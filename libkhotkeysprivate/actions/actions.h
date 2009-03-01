/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef _ACTIONS_H_
#define _ACTIONS_H_


#include <QtCore/QList>
#include <QtCore/QTimer>

#include <kservice.h>

class KConfig;

#include "khotkeysglobal.h"

namespace KHotKeys
{

class ActionData;
class Windowdef_list;

// this one is a base for all "real" resulting actions, e.g. running a command,
// ActionData instances usually contain at least one Action
class KDE_EXPORT Action
    {
    Q_DISABLE_COPY( Action )

    public:

        /**
         * A enum for all possible action types.
         *
         * @see type()
         */
        enum ActionType
            {
            ActivateWindowActionType = 0x01, //!< @see ActivateWindowAction
            CommandUrlActionType     = 0x02, //!< @see CommandUrlAction
            DBusActionType           = 0x04, //!< @see DBusAction
            KeyboardInputActionType  = 0x08, //!< @see KeyboardInputAction
            MenuEntryActionType      = 0x10, //!< @see MenuEntryAction
            ActionListType           = 0x11, //!< @see ActionList
            AllTypes                 = 0xEF  //!< All types. For convenience
            };

        Q_DECLARE_FLAGS(ActionTypes, ActionType)

        /**
         * Create a action
         */
        Action( ActionData* data_P );
        Action( KConfigGroup& cfg_P, ActionData* data_P );

        virtual ~Action();

        /**
         * Execute the action.
         */
        virtual void execute() = 0;

        /**
         * Have a look what's inside.
         *
         * @return the type of the action.
         */
        virtual ActionType type() = 0;

        /**
         * Returns a short descriptions of the action.
         *
         * The description is generated and can't be set.
         *
         * @return a description for this action
         */
        virtual const QString description() const = 0;

        /**
         * Write the actions configuration to @p cfg_P.
         */
        virtual void cfg_write( KConfigGroup& cfg_P ) const;

        /**
         * Read the configuration from @p cfg_P and create a new action.
         *
         * It's is the responsibility of the caller to make sure that @p cfg_P
         * really holds the configuration for a action. But the code still
         * tries to be as smart as possible when interpreting that stuff.
         *
         * The new actions will have @p parent_P as the parent.
         *
         * @param cfg_P the configuration object to read from
         * @param data_P parent for the new object
         *
         * @returns the new action object.
         */
        static Action* create_cfg_read( KConfigGroup& cfg_P, ActionData* data_P );

        /**
         * Return a copy of the action. 
         *
         * This is a real deep copy.
         */
        virtual Action* copy( ActionData* data_P ) const = 0;

        /**
         * The action is about to be erased permanently
         */
        virtual void aboutToBeErased();

    protected:

        /**
         * The action data corresponding to this action.
         */
        ActionData* const data;
    };

Q_DECLARE_OPERATORS_FOR_FLAGS(Action::ActionTypes)


class KDE_EXPORT ActionList
    : public QList< Action* >
    {
    Q_DISABLE_COPY( ActionList )
    public:
        ActionList( const QString& comment_P ); // CHECKME nebo i data ?
        ActionList( KConfigGroup& cfg_P, ActionData* data_P );
        ~ActionList();
        void cfg_write( KConfigGroup& cfg_P ) const;
        //! Some convenience typedef
        typedef QList<Action*>::Iterator Iterator;
        typedef QList<Action*>::ConstIterator ConstIterator;
        const QString& comment() const;

        /**
         * @reimp
         */
        void aboutToBeErased();

    private:
        QString _comment;
    };

class KDE_EXPORT CommandUrlAction
    : public Action
    {
    typedef Action base;
    public:
        CommandUrlAction( ActionData* data_P, const QString& command_url_P = QString() );
        CommandUrlAction( KConfigGroup& cfg_P, ActionData* data_P );
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual void execute();
        virtual const QString description() const;

        //! The command url to trigger
        void set_command_url( const QString &command_url );
        QString command_url() const;

        virtual ActionType type() { return CommandUrlActionType; }
        virtual Action* copy( ActionData* data_P ) const;
    private:
        QString _command_url;
    };

class KDE_EXPORT MenuEntryAction
    : public CommandUrlAction
    {
    typedef CommandUrlAction base;
    public:
        MenuEntryAction( ActionData* data_P, const QString& menuentry_P = QString() );
        MenuEntryAction( KConfigGroup& cfg_P, ActionData* data_P );
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual void execute();

        // The service we trigger
        KService::Ptr service() const;
        void set_service( KService::Ptr );

        virtual const QString description() const;
        virtual Action* copy( ActionData* data_P ) const;
        virtual ActionType type() { return Action::MenuEntryActionType; }
    private:
        KService::Ptr _service;
    };

class KDE_EXPORT DBusAction
    : public Action
    {
    typedef Action base;
    public:
        DBusAction( 
            ActionData* data_P,
            const QString& app_P = QString(),
            const QString& obj_P = QString(),
            const QString& call_P= QString(),
            const QString& args_P= QString() );

        DBusAction( KConfigGroup& cfg_P, ActionData* data_P );
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual void execute();
        const QString remote_application() const;
        const QString remote_object() const;
        const QString called_function() const;
        const QString arguments() const;

        void set_remote_application( const QString &application );
        void set_remote_object( const QString &object );
        void set_called_function( const QString &function );
        void set_arguments( const QString &args );

        virtual const QString description() const;
        virtual Action* copy( ActionData* data_P ) const;
        virtual ActionType type() { return DBusActionType; }
    private:
        QString _application; // CHECKME QCString ?
        QString _object;
        QString _function;
        QString _arguments;
    };

class KDE_EXPORT KeyboardInputAction
    : public Action
    {
    typedef Action base;
    public:

        /**
         * Where should we send the data too
         */
        enum DestinationWindow
            {
            ActiveWindow,
            SpecificWindow,
            ActionWindow
            };

        KeyboardInputAction(
                ActionData* data_P,
                const QString& input_P = QString(),
                Windowdef_list* dest_window_P = NULL,
                bool active_window_P = true);

        KeyboardInputAction(
                KConfigGroup& cfg_P,
                ActionData* data_P);

        virtual ~KeyboardInputAction();
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual void execute();


        const QString& input() const;
        void setInput(const QString &input);

        // send to specific window: dest_window != NULL
        // send to active window: dest_window == NULL && activeWindow() == true
        // send to action window: dest_window == NULL && activeWindow() == false
        //

        DestinationWindow destination() const;
        void setDestination(const DestinationWindow &dest);

        const Windowdef_list* dest_window() const;
        Windowdef_list* dest_window();
        bool activeWindow() const;
        virtual const QString description() const;
        virtual Action* copy( ActionData* data_P ) const;
        virtual ActionType type() { return KeyboardInputActionType; }
    private:
        QString _input;
        Windowdef_list* _dest_window;

        //! Which window should get the input
        DestinationWindow _destination;
    };

class KDE_EXPORT ActivateWindowAction
    : public Action
    {
    typedef Action base;
    public:
        ActivateWindowAction( ActionData* data_P, const Windowdef_list* window_P );
        ActivateWindowAction( KConfigGroup& cfg_P, ActionData* data_P );
        virtual ~ActivateWindowAction();
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual void execute();
        const Windowdef_list* window() const;
        virtual const QString description() const;
        virtual Action* copy( ActionData* data_P ) const;
        virtual ActionType type() { return ActivateWindowActionType; }
    private:
        const Windowdef_list* _window;
    };

} // namespace KHotKeys

#endif
