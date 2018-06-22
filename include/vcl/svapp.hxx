/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_VCL_SVAPP_HXX
#define INCLUDED_VCL_SVAPP_HXX

#include <sal/config.h>
#include <sal/types.h>

#include <cassert>
#include <stdexcept>
#include <vector>

#include <comphelper/solarmutex.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <osl/thread.hxx>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/inputtypes.hxx>
#include <vcl/exceptiontypes.hxx>
#include <vcl/keycod.hxx>
#include <vcl/vclevent.hxx>
#include <vcl/metric.hxx>
#include <unotools/localedatawrapper.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/connection/XConnection.hpp>


class BitmapEx;
namespace weld
{
    class Builder;
    class Container;
    class MessageDialog;
    class Widget;
    class Window;
}
class AllSettings;
class DataChangedEvent;
class Accelerator;
class Help;
class OutputDevice;
namespace vcl { class Window; }
class WorkWindow;
class MenuBar;
class UnoWrapperBase;
class Reflection;
class NotifyEvent;
class KeyEvent;
class MouseEvent;
struct ImplSVEvent;
struct ConvertData;

namespace com {
namespace sun {
namespace star {
namespace uno {
    class XComponentContext;
}
namespace ui {
    namespace dialogs {
        class XFilePicker2;
        class XFolderPicker2;
    }
}
namespace awt {
    class XToolkit;
    class XDisplayConnection;
    class XWindow;
}
} } }

// helper needed by SalLayout implementations as well as svx/source/dialog/svxbmpnumbalueset.cxx
VCL_DLLPUBLIC sal_UCS4 GetMirroredChar( sal_UCS4 );
VCL_DLLPUBLIC sal_UCS4 GetLocalizedChar( sal_UCS4, LanguageType );

enum class SystemWindowFlags {
    NOAUTOMODE    = 0x0001,
    DIALOG        = 0x0002
};
namespace o3tl
{
    template<> struct typed_flags<SystemWindowFlags> : is_typed_flags<SystemWindowFlags, 0x03> {};
}

typedef long (*VCLEventHookProc)( NotifyEvent& rEvt, void* pData );

/** An application can be notified of a number of different events:
    - Type::Accept       - listen for connection to the application (a connection
                          string is passed via the event)
    - Type::Unaccept     - stops listening for a connection to the app (determined by
                          a connection string passed via the event)
    - Type::Appear       - brings the app to the front (i.e. makes it "appear")
    - Type::Version      - display the app version
    - Type::Help         - opens a help topic (help topic passed as string)
    - Type::OpenHELP_URL - opens a help URL (URL passed as a string)
    - Type::ShowDialog   - shows a dialog (dialog passed as a string)
    - Type::Open         - opens a document or group of documents (documents passed
                          as an array of strings)
    - Type::Print        - print a document or group of documents (documents passed
                          as an array of strings
    - Type::PrivateDoShutdown - shutdown the app
*/

class VCL_DLLPUBLIC ApplicationEvent
{
public:
    enum class Type {
        Accept,                ///< Listen for connections
        Appear,                ///< Make application appear
        Help,                  ///< Bring up help options (command-line help)
        Version,               ///< Display product version
        Open,                  ///< Open a document
        OpenHelpUrl,           ///< Open a help URL
        Print,                 ///< Print document
        PrivateDoShutdown,    ///< Shutdown application
        QuickStart,            ///< Start QuickStart
        ShowDialog,            ///< Show a dialog
        Unaccept               ///< Stop listening for connections
    };

    /** Explicit constructor for ApplicationEvent.

     @attention Type::Appear, Type::Version, Type::PrivateDoShutdown and
        Type::QuickStart are the \em only events that don't need to include
        a data string with the event. No other events should use this
        constructor!
    */
    explicit ApplicationEvent(Type type): aEvent(type)
    {
        assert(
            type == Type::Appear || type == Type::Version
            || type == Type::PrivateDoShutdown || type == Type::QuickStart);
    }

    /** Constructor for ApplicationEvent, accepts a string for the data
     associated with the event.

     @attention Type::Accept, Type::Help, Type::OpenHelpUrl, Type::ShowDialog
        and Type::Unaccept are the \em only events that accept a single
        string as event data. No other events should use this constructor!
    */
    ApplicationEvent(Type type, OUString const & data): aEvent(type)
    {
        assert(
            type == Type::Accept || type == Type::Help || type == Type::OpenHelpUrl
            || type == Type::ShowDialog || type == Type::Unaccept);
        aData.push_back(data);
    }

    /** Constructor for ApplicationEvent, accepts an array of strings for
     the data associated with the event.

     @attention Type::Open and Type::Print can apply to multiple documents,
        and are the \em only events that accept an array of strings. No other
        events should use this constructor.
    */
    ApplicationEvent(Type type, std::vector<OUString> const & data):
        aEvent(type), aData(data)
    {
        assert(type == Type::Open || type == Type::Print);
    }

    /** Get the type of event.

     @returns The type of event.
    */
    Type GetEvent() const
    {
        return aEvent;
    }

    /** Gets the application event's data string.

     @attention The \em only events that need a single string Type::Accept,
        Type::Help, Type::OpenHelpUrl, Type::ShowDialog and Type::Unaccept

     @returns The event's data string.
    */
    OUString const & GetStringData() const
    {
        assert(
            aEvent == Type::Accept || aEvent == Type::Help
            || aEvent == Type::OpenHelpUrl || aEvent == Type::ShowDialog
            || aEvent == Type::Unaccept);
        assert(aData.size() == 1);
        return aData[0];
    }

    /** Gets the event's array of strings.

     @attention The \em only events that need an array of strings
        are Type::Open and Type::Print.
    */
    std::vector<OUString> const & GetStringsData() const
    {
        assert(aEvent == Type::Open || aEvent == Type::Print);
        return aData;
    }

private:
    Type aEvent;
    std::vector<OUString> aData;
};

/**
 @brief Base class used mainly for the LibreOffice Desktop class.

 The Application class is a base class mainly used by the Desktop
 class. It is really meant to be subclassed, and the Main() function
 should be overridden. Many of the ImplSVData members should be
 moved to this class.

 The reason Application exists is because the VCL used to be a
 standalone framework, long since abandoned by anything other than
 our application.

 @see   Desktop, ImplSVData
 */
class VCL_DLLPUBLIC Application
{
public:
    enum class DialogCancelMode {
        Off,      ///< do not automatically cancel dialogs
        Silent,   ///< silently cancel any dialogs
        Fatal     ///< cancel any dialogs by std::abort
    };

    /** @name Initialization
        The following functions perform initialization and deinitialization
        of the application.
    */
    ///@{

    /** Default constructor for Application class.

    Initializes the LibreOffice global instance data structure if needed,
    and then sets itself to be the Application class. Also initializes any
    platform specific data structures.

    @attention The initialization of the application itself is done in Init()

    @see    InitSalData is implemented by platform specific code.
    */
                                Application();

    /** Virtual destructor for Application class.

     Deinitializes the LibreOffice global instance data structure, then
     deinitializes any platform specific data structures.

     @see   ImplDeInitSVData deinitializes the global instance data,
            DeInitSalData is implemented by platform specific code
    */
    virtual                     ~Application();

    /** Initialize the application itself.

     @attention Note that the global data structures and platform specific
        initialization is done in the constructor.

     @see InitFinished, DeInit
    */
    virtual void                Init();

    /** Finish initialization of the application.

     @see Init, DeInit
    */
    virtual void                InitFinished();

    /** Deinitialized the application itself.

     @attention Note that the global data structures and platform specific
         deinitialization is done in the destructor.

     @see Init, InitFinished
    */
    virtual void                DeInit();

    ///@}

    /** @brief Pure virtual entrypoint to the application.

     Main() is the pure virtual entrypoint to your application. You
     inherit your class from Application and subclass this function to
     implement an application.

     The Main() function does not pass in command line parameters,
     you must use the functions GetCommandLineParamCount() and
     GetCommandLineParam() to get these values as these are platform
     independent ways of getting the command line (use GetAppFileName()
     to get the invoked executable filename).

     Once in this function, you create windows, etc. then call on
     Execute() to start the application's main event loop.

     An example code snippet follows (it won't compile, this just gives the
     general flavour of the framework and is adapted from an old HelloWorld
     example program that Star Division used to provide as part of their
     library).

     \code{.cpp}
        class TheApplication : public Application
        {
            public:
                virtual void Main();
        };

        class TheWindow : public WorkWindow
        {
            public:
                TheWindow(vcl::Window *parent, WinBits windowStyle) :
                        WorkWindow(parent, windowStyle) {}

                virtual void Paint(const Rectangle &);
        };

        void TheWindow::Paint(const Rectangle&)
        {
                DrawText(Point(100,100), String("Hello World!"));
        }

        void TheApplication::Main()
        {
                TheWindow aWindow(NULL, WB_APP | WB_STDWORK);
                    aWindow.Show();
                        Execute();
        }

        TheApplication anApplication;
    \endcode

    Some examples in the source tree can be found here:

    vcl/workben/svdem.cxx

    This is an example of how to use the Application and WorkWindow. Unfortunately, it
    no longer compiles.

    vcl/fpicker/test/svdem.cxx
    */
    virtual int                 Main();

    /** Exit from the application

     @returns true if exited successfully, false if not able to fully exit
    */
    virtual bool                QueryExit();

    /** @name Change Notification Functions

        Functions that notify when changes occur in the application.
    */
    ///@{

    /** Notify all windows that the application has changed data.

     @param rDCEvt     Reference to a DataChangedEvent object

     @see DataChanged
    */
    static void                 NotifyAllWindows( DataChangedEvent& rDCEvt );

    ///@}

    /** @name Command Line Processing

        Command line processing is done via the following functions. They
        give the number of parameters, the parameters themselves and a way
        to get the name of the invoking application.
    */

    ///@{

    /** Gets the number of command line parameters passed to the application

     @return sal_uInt16 - the number of parameters

     @see GetCommandLineParam, GetAppFileName
    */
    static sal_uInt16           GetCommandLineParamCount();

    /** Gets a particular command line parameter

     @param  nParam      The index of the parameter to return.

     @return The command line parameter as an OUString

     @see GetCommandLineParamCount, GetAppFileName
    */
    static OUString             GetCommandLineParam( sal_uInt16 nParam );

    /** Get the name of the file used to start the application

     @return The filename as an OUString

     @see GetCommandLineParamCount, GetCommandLineParam
    */
    static OUString             GetAppFileName();

    ///@}

    /** @name Error Handling

        \em Very rudimentary error handling is done by these
        functions.

        @{
    */

    /** Handles an error.

     @param nCategory    The error category, see include/vcl/exceptiontypes.hxx

     @see Abort
    */
    virtual void          Exception( ExceptionCategory nCategory );

    /** Ends the program prematurely with an error message.

     If the \code --norestore \endcode command line argument is given (assuming
     this process is run by developers who are interested in cores,
     vs. end users who are not) then it does a coredump.

     @param rErrorText  The error message to report.

     @see Exception
    */
    static void                 Abort( const OUString& rErrorText );

    ///@}

    /** @name Event Loop Functions

        Functions that handle the LibreOffice main event loop are here,
        including a global lock called the Solar Mutex.
    */
    ///@{

    /** Run the main event processing loop until it is quit by Quit().

     @see Quit, Reschedule, Yield, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
    */
    static void                 Execute();

    /** Quit the program

     @see Execute, Reschedule, Yield, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
    */
    static void                 Quit();

    /** Attempt to process current pending event(s)

     It doesn't sleep if no events are available for processing.
     This doesn't process any events generated after invoking the function.
     So in contrast to Scheduler::ProcessEventsToIdle, this cannot become
     busy-locked by an event-generating event in the event queue.

     @param bHandleAllCurrentEvents  If set to true, then try to process all
        the current events. If set to false, then only process one event.
        Defaults to false.

     @returns true if any event was processed.

     @see Yield, Scheduler::ProcessEventsToIdle
     */
    static bool                 Reschedule( bool bHandleAllCurrentEvents = false );

    /** Process the next event.

     It sleeps if no event is available for processing and just returns
     if an event was processed.

     @see Execute, Quit, Reschedule, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
    */
    static void                 Yield();

    /**

     @see Execute, Quit, Reschedule, Yield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
    */
    static void                 EndYield();

    /** @brief Get the Solar Mutex for this thread.

     Get the Solar Mutex that prevents other threads from accessing VCL
     concurrently.

     @returns SolarMutex reference

     @see Execute, Quit, Reschedule, Yield, EndYield,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
    */
    static comphelper::SolarMutex& GetSolarMutex();

    /** Get the main thread ID.

     @returns oslThreadIdentifier that contains the thread ID

     @see Execute, Quit, Reschedule, Yield, EndYield, GetSolarMutex,
          ReleaseSolarMutex, AcquireSolarMutex,
    */
    static oslThreadIdentifier  GetMainThreadIdentifier();

    /** @brief Release Solar Mutex(es) for this thread

     Release the Solar Mutex(es) that prevents other threads from accessing
     VCL concurrently.

     @returns The number of mutexes that were acquired by this thread.

     @see Execute, Quit, Reschedule, Yield, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, AcquireSolarMutex,
    */
    static sal_uInt32           ReleaseSolarMutex();

    /** @brief Acquire Solar Mutex(es) for this thread.

     Acquire the Solar Mutex(es) that prevents other threads from accessing
     VCL concurrently.

     @see Execute, Quit, Reschedule, Yield, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex,
    */
    static void                 AcquireSolarMutex( sal_uInt32 nCount );

    /** Queries whether the application is in "main", i.e. not yet in
        the event loop

     @returns   true if in main, false if not in main

     @see IsInExecute, IsInModalMode
    */
    static bool                 IsInMain();

    /** Queries whether the application is in the event loop

     @returns   true if in the event loop, false if not

     @see IsInMain, IsInModalMode
    */
    static bool                 IsInExecute();

    /** Queries whether application has a modal dialog active.

     @returns   true if a modal dialog is active, false if not

     @see IsInMain, IsInExecute
    */
    static bool                 IsInModalMode();

    /** Return how many events are being dispatched.

     @returns   the number of events currently being dispatched
    */
    static sal_uInt16           GetDispatchLevel();

    /** Determine if there are any pending input events.

     @param     nType   input identifier, defined in include/vcl/inputtypes.hxx
                        The default is VCL_INPUT_ANY.

     @returns   true if there are pending events, false if not.

     @see GetLastInputInterval
    */
    static bool                 AnyInput( VclInputFlags nType = VCL_INPUT_ANY );

    /** The interval from the last time that input was received.

     @returns   system ticks - last input time

     @see AnyInput
    */
    static sal_uInt64           GetLastInputInterval();

    ///@}

    /* Determines if the UI is captured.

     The UI is considered captured if a system dialog is open (e.g. printer setup),
     a floating window, menu or toolbox dropdown is open, or a window has been
     captured by the mouse.

     @returns   true if UI is captured, false if not
     */
    static bool                 IsUICaptured();

    /** @name Settings

        The following functions set system settings (e.g. tab color, etc.). There are functions
        that set settings objects, and functions that set and get the actual system settings for
        the application.
    */
    ///@{

    /** Sets user settings in settings object to override system settings

     The system settings that can be overridden are:
        - window dragging options (on or off, including live scrolling!)
        - style settings (e.g. checkbox color, border color, 3D colors,
          button rollover colors, etc.)
        - mouse settings
        - menu options, including the mouse follows the menu and whether menu
          icons are used

     @param      rSettings      Reference to the settings object to change.

     @see MergeSystemSettings, SetSettings, GetSettings
    */
    virtual void                OverrideSystemSettings( AllSettings& rSettings );

    /** Set the settings object to the platform/desktop environment system
     settings.

     @param     rSettings       Reference to the settings object to change.

     @see OverrideSystemSettings, SetSettings, GetSettings
    */
    static void                 MergeSystemSettings( AllSettings& rSettings );

    /** Sets the application's settings and notifies all windows of the
     change.

     @param     rSettings       const reference to settings object used to
                                change the application's settings.

     @see OverrideSystemSettings, MergeSystemSettings, GetSettings
    */
    static void                 SetSettings( const AllSettings& rSettings );

    /** Gets the application's settings. If the application hasn't initialized
     it's settings, then it does so (lazy initialization).

     @returns AllSettings instance that contains the current settings of the
        application.

     @see OverrideSystemSettings, MergeSystemSettings, SetSettings
    */
    static const AllSettings&   GetSettings();

    /** Get the application's locale data wrapper.

     @returns reference to a LocaleDataWrapper object
    */
    static const LocaleDataWrapper& GetAppLocaleDataWrapper();

    ///@}

    /** @name Event Listeners/Handlers

        A set of event listeners and callers. Note that in this code there is
        platform specific functions - namely for zoom and scroll events.
    */
    ///@{


    /** Add a VCL event listener to the application. If no event listener exists,
     then initialize the application's event listener with a new one, then add
     the event listener.

     @param     rEventListener  Const reference to the event listener to add.

     @see RemoveEventListener, AddKeyListener, RemoveKeyListener
    */
    static void                 AddEventListener( const Link<VclSimpleEvent&,void>& rEventListener );

    /** Remove a VCL event listener from the application.

     @param     rEventListener  Const reference to the event listener to be removed

     @see AddEventListener, AddKeyListener, RemoveKeyListener
    */
    static void                 RemoveEventListener( const Link<VclSimpleEvent&,void>& rEventListener );

    /** Add a keypress listener to the application. If keypress listener exists,
     then initialize the application's keypress event listener with a new one, then
     add the keypress listener.

     @param     rKeyListener    Const reference to the keypress event listener to add

     @see AddEventListener, RemoveEventListener, RemoveKeyListener
    */
    static void                 AddKeyListener( const Link<VclWindowEvent&,bool>& rKeyListener );

    /** Remove a keypress listener from the application.

     @param     rKeyListener    Const reference to the keypress event listener to be removed

     @see AddEventListener, RemoveEventListener, AddKeyListener
    */
    static void                 RemoveKeyListener( const Link<VclWindowEvent&,bool>& rKeyListener );

    /** Send event to all VCL application event listeners

     @param     pWin            Pointer to window to send event
     @param     pData           Pointer to data to send with event

     @see ImplCallEventListeners(VclSimpleEvent* pEvent)
    */
    static void                 ImplCallEventListenersApplicationDataChanged( void* pData );

    /** Send event to all VCL application event listeners

     @param     rEvent          Reference to VclSimpleEvent

     @see ImplCallEventListeners(sal_uLong nEvent, Windows* pWin, void* pData);
    */
    static void                 ImplCallEventListeners( VclSimpleEvent& rEvent );

    /** Handle keypress event

     @param     nEvent          Event ID for keypress
     @param     pWin            Pointer to window that receives the event
     @param     pKeyEvent       Received key event

     @see PostKeyEvent
    */
    static bool                 HandleKey( VclEventId nEvent, vcl::Window *pWin, KeyEvent* pKeyEvent );

    /** Send keypress event

     @param     nEvent          Event ID for keypress
     @param     pWin            Pointer to window to which the event is sent
     @param     pKeyEvent       Key event to send

     @see HandleKey
    */
    static ImplSVEvent *        PostKeyEvent( VclEventId nEvent, vcl::Window *pWin, KeyEvent const * pKeyEvent );

    /** Send mouse event

     @param     nEvent          Event ID for mouse event
     @param     pWin            Pointer to window to which the event is sent
     @param     pMouseEvent     Mouse event to send
    */
    static ImplSVEvent *        PostMouseEvent( VclEventId nEvent, vcl::Window *pWin, MouseEvent const * pMouseEvent );

    /** Remove mouse and keypress events from a window... any also zoom and scroll events
     if the platform supports it.

     @param     pWin            Window to remove events from

     @see HandleKey, PostKeyEvent, PostMouseEvent
    */
    static void                 RemoveMouseAndKeyEvents( vcl::Window *pWin );

    /** Post a user event to the default window.

     User events allow for the deferral of work to later in the main-loop - at idle.

     Execution of the deferred work is thread-safe which means all the tasks are executed
     serially, so no thread-safety locks between tasks are necessary.

     @param     rLink           Link to event callback function
     @param     pCaller         Pointer to data sent to the event by the caller. Optional.
     @param     bReferenceLink  If true - hold a VclPtr<> reference on the Link's instance.
                                Taking the reference is guarded by a SolarMutexGuard.

     @return the event ID used to post the event.
    */
    static ImplSVEvent * PostUserEvent( const Link<void*,void>& rLink, void* pCaller = nullptr,
                                        bool bReferenceLink = false );

    /** Remove user event based on event ID

     @param     nUserEvent      User event to remove
    */
    static void                 RemoveUserEvent( ImplSVEvent * nUserEvent );

    /*** Get the DisplayConnection.

     It is a reference to XDisplayConnection, which allows toolkits to send display
     events to the application.

     @returns UNO reference to an object that implements the css:awt:XDisplayConnection
        interface.
    */
    static css::uno::Reference< css::awt::XDisplayConnection > GetDisplayConnection();

    /** @deprecated AppEvent is used only in the Desktop class now. However, it is
     intended to notify the application that an event has occurred. It was in oldsv.cxx,
     but is still needed by a number of functions.

     @param     rAppEvent       const reference to ApplicationEvent event
    */
    virtual void                AppEvent( const ApplicationEvent& rAppEvent );

    ///@}

    /** @name Application Window Functions

        Functions that deal with the application's windows
    */
    ///@{

    /** Get the main application window.

     @remark the main application window (or App window) has a style of WB_APP,
        there can only be on WorkWindow with this style, if a dialog or floating
        window cannot find a parent, then the parent becomes the app window.

        It also becomes the "default window", is used for help, is a fallback if
        the application has no name, and a number of other things.

     returns Pointer to main application window.

     @see GetFocusWindow, GetDefaultDevice
    */
    static WorkWindow*          GetAppWindow();

    /** Get the currently focused window.

     @returns Pointer to focused window.

     @see GetAppWindow, GetDefaultDevice
    */
    static vcl::Window*              GetFocusWindow();

    /** Get the default "device" (in this case the default window).

     @returns Pointer to an OutputDevice. However, it is a Window object -
             Window class subclasses OutputDevice.

     @see GetAppWindow, GetFocusWindow
    */
    static OutputDevice*        GetDefaultDevice();

    /** Get the first top-level window of the application.

     @returns Pointer to top-level window (a Window object)

     @see GetNextTopLevelWindow, GetTopWindowCount, GetTopWindow,
          GetActiveTopWindow
    */
    static vcl::Window*              GetFirstTopLevelWindow();

    /** Get the next top level window.

     @param     pWindow     Pointer to Window object you wish to get the next
                            window from.

     @returns Pointer to next top window.
    */
    static vcl::Window*              GetNextTopLevelWindow( vcl::Window const * pWindow );

    /** Return the number of top-level windows being used by the application

     @returns the number of top-level windows

     @see GetFirstTopLevelWindow, GetNextTopLevelWindow, GetTopWindow,
          GetActiveTopWindow

    */
    static long                 GetTopWindowCount();

    /** Get the nth top window.

     @remark Top windows are actually implemented in a one-way linked list.
         This iterates through top level windows n times.

     @param    nIndex      The index of the top-level window

     @returns The nth top-level window of the application

     @see GetFirstTopLevelWindow, GetNextTopLevelWindow, GetTopWindowCount,
          GetActiveTopWindow
    */
    static vcl::Window*              GetTopWindow( long nIndex );

    /** Get the "active" top window.

     An "active" top window is one that has a child window that has the
     application's focus.

     @returns the active top window

     @see GetFirstTopLevelWindow, GetNextTopLevelWindow, GetTopWindowCount,
          GetTopWindow
    */
    static vcl::Window*              GetActiveTopWindow();

    ///@}

    /** Set the application's name.

     @param     rUniqueName     What to set the application name to

     @see       GetAppName
    */
    static void                 SetAppName( const OUString& rUniqueName );


    /** @name Application Name, Branding
    */
    ///@{

    /** Get the application's name.

     @returns The application name.
    */
    static OUString             GetAppName();

    /** Get useful OS, Hardware and configuration information,
     * cf. Help->About, and User-Agent
     */
    static OUString             GetHWOSConfInfo();

    /** Load a localized branding PNG file as a bitmap.

     @param     pName           Name of the bitmap to load.
     @param     rBitmap         Reference to BitmapEx object to load PNG into

     @returns true if the PNG could be loaded, otherwise returns false.
    */
    static bool                 LoadBrandBitmap (const char* pName, BitmapEx &rBitmap);

    ///@}

    /** @name Display and Screen
    */
    ///@{

    /** Set the default name of the application for message dialogs and printing.

     @param     rDisplayName    const reference to string to set the Display name to.

     @see GetDisplayName
    */
    static void                 SetDisplayName( const OUString& rDisplayName );

    /** Get the default name of the application for message dialogs and printing.

     @returns The display name of the application.
    */
    static OUString             GetDisplayName();

    /** Get the toolkit's name. e.g. gtk3

     @returns The toolkit name.
    */
    static OUString             GetToolkitName();

    /** Get the number of screens available for the display.

     @returns The number of screens available.

     @see GetScreenPosSizePixel
    */
    static unsigned int         GetScreenCount();

    /** Get a screen's rectangular area.

     @param     nScreen         The number of the screen requested.

     @returns The area of the screen in a Rectangle object.

     @see GetScreenCount
    */
    static tools::Rectangle            GetScreenPosSizePixel( unsigned int nScreen );

    /** Determines if the screens that make up a display are separate or
     form one large display area.

     @returns true when screens form up one large display area windows can be
        moved between single screens (e.g. Xserver with Xinerama, Windows)
        and false when different screens are separate and windows cannot be moved
        between them (e.g. Xserver with multiple screens)

     @see GetBestScreen, GetDisplayBuiltInScreen
    */
    static bool                 IsUnifiedDisplay();

    /** Get the "best" screen.

     @returns If IsUnifiedDisplay() == true the return value will be
       nearest screen of the target rectangle.

       In case of IsUnifiedDisplay() == false the return value
       will always be GetDisplayDefaultScreen().

     @see IsUnifiedDisplay, GetDisplayBuiltInScreen
    */
    SAL_DLLPRIVATE static unsigned int GetBestScreen( const tools::Rectangle& );

    /** Get the built-in screen.

     @return
       This returns the LCD screen number for a laptop, or the primary
       external VGA display for a desktop machine - it is where a presenter
       console should be rendered if there are other (non-built-in) screens
       present.

     @see IsUnifiedDisplay, GetBestScreen
    */
    static unsigned int         GetDisplayBuiltInScreen();

    /** Get the display's external screen.

     Practically, this means - Get the screen we should run a presentation on.

     @returns 0 or 1 currently, will fallback to the first available screen if
        there are more than one external screens. May be changed in the future.
    */
    static unsigned int         GetDisplayExternalScreen();

    ///@}

    /** @name Accelerators and Mnemonics

     Accelerators allow a user to hold down Ctrl+key (or CMD+key on OS X)
     combination to gain quick access to functionality.

     Mnemonics are underline letters in things like menus and dialog boxes
     that allow a user to type in the letter to activate the menu or option.
    */
    ///@{

    /** Insert accelerator

     @param     pAccel          Pointer to an Accelerator object to insert

     @returns true if successful, false if otherwise

     @see RemoveAccel
    */
    static bool                 InsertAccel( Accelerator* pAccel );

    /** Remove accelerator

     @param     pAccel          Pointer to Accelerator object to remove

     @see InsertAccel
    */
    static void                 RemoveAccel( Accelerator const * pAccel );

    /** Get the number of reserved key codes used by the application.

     @returns number of reserved key codes

     @see GetReservedKeyCode
    */
    static sal_uLong            GetReservedKeyCodeCount();

    /** Get the reserved key code.

     @param     i               The keycode number to retrieve

     @returns Const pointer to a KeyCode object

     @see GetReservedKeyCodeCount
    */
    static const vcl::KeyCode*  GetReservedKeyCode( sal_uLong i );

    ///@}

    /** @name Application Help

     Deals with the help system, and "auto-help", where a user hovers a mouse above
     a UI element and a tooltip with an explanation pops up.
    */
    ///@{

    /** Sets up help

     @param     pHelp           Pointer to a Help object (optional, can by NULL)

     @see GetHelp
    */
    static void                 SetHelp( Help* pHelp = nullptr );

    /** Gets the application's help

     @returns Pointer to application's help object. Note that the application may
        not have a help object, so it might return NULL.

     @see SetHelp
    */
    static Help*                GetHelp();

    ///@}

    /** @name Dialogs

        @remark "Dialog cancel mode" tells a headless install whether to
                cancel dialogs when they appear. See the DialogCancelMode
                enumerator.
    */
    ///@{

    /** Get the default parent window for dialog boxes.

     @remark GetDefDialogParent does all sorts of things find a useful parent
             window for dialogs. It first uses the topmost parent of the active
             window to avoid using floating windows or other dialog boxes. If
             there are no active windows, then it will take a random stab and
             choose the first visible top window. Otherwise, it defaults to
             the desktop.

     @returns Pointer to the default window.
    */
    static vcl::Window*              GetDefDialogParent();


    /** Gets the dialog cancel mode for headless environments.

     @return DialogCancelMode value

     @see SetDialogCancelMode, IsDialogCancelEnabled
    */
    static DialogCancelMode     GetDialogCancelMode();

    /** Sets the dialog cancel mode for headless environments.

     @param     mode            DialogCancel mode value

     @see GetDialogCancelMode, IsDialogCancelEnabled
    */
    static void                 SetDialogCancelMode( DialogCancelMode mode );

    /** Determines if dialog cancel mode is enabled.

     @returns True if dialog cancel mode is enabled, false if disabled.

     @see GetDialogCancelMode, SetDialogCancelMode
    */
    static bool                 IsDialogCancelEnabled();


    /** Make a dialog box a system window or not.

     @param     nMode           Can be either: SystemWindowFlags::NOAUTOMODE (0x0001) or
                                SystemWindowFlags::DIALOG (0x0002)

     @see GetSystemWindowMode
    */
    static void                 SetSystemWindowMode( SystemWindowFlags nMode );

    /** Get the system window mode of dialogs.

     @returns SystemWindowFlags::NOAUTOMODE (0x0001) or SystemWindowFlags::DIALOG (0x0002)

     @see SetSystemWindowMode
    */
    static SystemWindowFlags    GetSystemWindowMode();

    ///@}

    /** @name VCL Toolkit and UNO Wrapper

      The VCL Toolkit implements the UNO XToolkit interface, which specifies a
      factory interface for the window toolkit. It is similar to the abstract window
      toolkit (AWT) in Java.

    */
    ///@{

    /** Gets the VCL toolkit.

     @attention The global service manager has to be created before getting the toolkit!

     @returns UNO reference to VCL toolkit
    */
    static css::uno::Reference< css::awt::XToolkit > GetVCLToolkit();

    /** Get the application's UNO wrapper object.

     Note that this static function will only ever try to create UNO wrapper object once, and
     if it fails then it will not ever try again, even if the function is called multiple times.

     @param     bCreateIfNotExists  Create the UNO wrapper object if it doesn't exist when true.

     @return UNO wrapper object.
    */
    static UnoWrapperBase*      GetUnoWrapper( bool bCreateIfNotExists = true );

    /** Sets the application's UNO Wrapper object.

     @param     pWrapper        Pointer to UNO wrapper object.
    */
    static void                 SetUnoWrapper( UnoWrapperBase* pWrapper );

    ///@}


    /*** @name Graphic Filters
    */
    ///@{

    /** Setup a new graphics filter

     @param     rLink           Const reference to a Link object, which the filter calls upon.

     @see GetFilterHdl
    */
    static void                 SetFilterHdl( const Link<ConvertData&,bool>& rLink );

    ///@}

    /** @name Headless Mode
    */

    /** Enables headless mode.

     @param dialogsAreFatal     Set to true if a dialog ends the session, false if not.
    */
    static void                 EnableHeadlessMode( bool dialogsAreFatal );

    /** Determines if headless mode is enabled

     @return True if headless mode is enabled, false if not.
    */
    static bool                 IsHeadlessModeEnabled();

    /** Enable Console Only mode

     Used to disable Mac specific app init that requires an app bundle.
    */
    static void                 EnableConsoleOnly();

    /** Determines if console only mode is enabled.

     Used to see if Mac specific app init has been disabled.

     @returns True if console only mode is on, false if not.

     @see EnableConsoleOnly
    */
    static bool                 IsConsoleOnly();

    ///@}

    /** @name Event Testing Mode
    */

    /** Enables event testing mode.

    */
    static void                 EnableEventTestingMode();

    /** Determines if event testing mode is enabled

     @return True if event testing mode is enabled, false if not.
    */
    static bool                 IsEventTestingModeEnabled();

    /** Set safe mode to enabled */
    static void                 EnableSafeMode();

    /** Determines if safe mode is enabled */
    static bool                 IsSafeModeEnabled();

    ///@}

    /** @name IME Status Window Control
    */
    ///@{

    /** Determine application can toggle the IME status window on and off.

      @attention Must only be called with the Solar mutex locked.

      @return true if any IME status window can be toggled on and off
            externally.

      @see ShowImeStatusWindow, GetShowImeStatusWindowDefault,
           GetShowImeStatusWindowDefault
     */
    static bool                 CanToggleImeStatusWindow();

    /** Toggle any IME status window on and off.

     This only works if CanToggleImeStatusWindow returns true (otherwise,
     any calls of this method are ignored).

     @remark Can be called without the Solar mutex locked.

     @param      bShow       If true, then show the IME status window

     @see GetShowImeStatusWindowDefault, CanToggleImeStatusWindow,
          GetShowImeStatusWindow
    */
    static void                 ShowImeStatusWindow(bool bShow);

    /** Determines if the IME status window should be turned of by default.

      @return true if any IME status window should be turned on by default
      (this decision can be locale dependent, for example).

      @see ShowImeStatusWindow, GetShowImeStatusWindowDefault,
           CanToggleImeStatusWindow
     */
    static bool                 GetShowImeStatusWindowDefault();

    ///@}

    /** Get the desktop environment the process is currently running in

     @returns String representing the desktop environment
    */
    static const OUString&      GetDesktopEnvironment();

    /*** @name Platform Functionality
    */
    ///@{

    /** Add a file to the system shells recent document list if there is any.
     This function may have no effect under Unix because there is no standard
     API among the different desktop managers.

     @param     rFileUrl        The file url of the document.

     @param     rMimeType       The mime content type of the document specified by aFileUrl.
                                If an empty string will be provided "application/octet-stream"
                                will be used.

     @param     rDocumentService The app (or "document service") you will be adding the file to
                                e.g. com.sun.star.text.TextDocument
    */
    static void                 AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService);

    /*** Show a native error messagebox

     @param     sTitle          Title of error messagebox

     @param     sMessage        Message displayed in messagebox
    */
    static void                 ShowNativeErrorBox(const OUString& sTitle  ,
                                                   const OUString& sMessage);

    /** Update main thread identifier */
    static void                 UpdateMainThread();

    /** Do we have a native / system file selector available?

     @returns True if native file selector is available, false otherwise.
     */
    static bool                 hasNativeFileSelection();

    /** Create a platform specific file picker, if one is available, otherwise return an
     empty reference.

     @param    rServiceManager Const reference to a UNO component context (service manager).

     @returns File picker if available, otherwise an empty reference.
    */
    static css::uno::Reference< css::ui::dialogs::XFilePicker2 >
        createFilePicker( const css::uno::Reference< css::uno::XComponentContext >& rServiceManager );

    /** Create a platform specific folder picker, if one is available, otherwise return an
     empty reference

     @param    rServiceManager Const reference to a UNO component context (service manager).

     @returns Folder picker if available, otherwise an empty reference.
    */
    static css::uno::Reference< css::ui::dialogs::XFolderPicker2 >
        createFolderPicker( const css::uno::Reference< css::uno::XComponentContext >& rServiceManager );

    /** Cancel all open dialogs
    */
    static void                 EndAllDialogs();

    /** Cancel all open popups
    */
    static void                 EndAllPopups();

    ///@}

    // For vclbootstrapprotector:
    static void setDeInitHook(Link<LinkParamNone*,void> const & hook);

    static weld::Builder* CreateBuilder(weld::Widget* pParent, const OUString &rUIFile);
    static weld::Builder* CreateInterimBuilder(vcl::Window* pParent, const OUString &rUIFile); //for the duration of same SfxTabPages in mixed parent types
    static weld::Builder* CreateInterimBuilder(weld::Widget* pParent, const OUString &rUIFile); //for the duration of same SfxTabPages in mixed parent types

    static weld::MessageDialog* CreateMessageDialog(weld::Widget* pParent, VclMessageType eMessageType,
                                                    VclButtonsType eButtonType, const OUString& rPrimaryMessage);

    static weld::Window* GetFrameWeld(const css::uno::Reference<css::awt::XWindow>& rWindow);
private:
    DECL_STATIC_LINK( Application, PostEventHandler, void*, void );
};

class SolarMutexGuard
    : public osl::Guard<comphelper::SolarMutex>
{
public:
    SolarMutexGuard()
        : osl::Guard<comphelper::SolarMutex>( Application::GetSolarMutex() ) {}
};

class SolarMutexClearableGuard
    : public osl::ClearableGuard<comphelper::SolarMutex>
{
public:
    SolarMutexClearableGuard()
        : osl::ClearableGuard<comphelper::SolarMutex>( Application::GetSolarMutex() ) {}
};

class SolarMutexResettableGuard
    : public osl::ResettableGuard<comphelper::SolarMutex>
{
public:
    SolarMutexResettableGuard()
        : osl::ResettableGuard<comphelper::SolarMutex>( Application::GetSolarMutex() ) {}
};

namespace vcl
{

/** guard class that uses tryToAcquire() and has isAcquired() to check
 */
class SolarMutexTryAndBuyGuard
{
private:
    bool m_isAcquired;
#ifdef DBG_UTIL
    bool m_isChecked;
#endif
    comphelper::SolarMutex& m_rSolarMutex;

    SolarMutexTryAndBuyGuard(const SolarMutexTryAndBuyGuard&) = delete;
    SolarMutexTryAndBuyGuard& operator=(const SolarMutexTryAndBuyGuard&) = delete;

public:

    SolarMutexTryAndBuyGuard()
        : m_isAcquired(false)
#ifdef DBG_UTIL
        , m_isChecked(false)
#endif
        , m_rSolarMutex(Application::GetSolarMutex())

    {
        m_isAcquired = m_rSolarMutex.tryToAcquire();
    }

    ~SolarMutexTryAndBuyGuard()
    {
#ifdef DBG_UTIL
        assert(m_isChecked);
#endif
        if (m_isAcquired)
            m_rSolarMutex.release();
    }

    bool isAcquired()
    {
#ifdef DBG_UTIL
        m_isChecked = true;
#endif
        return m_isAcquired;
    }
};

} // namespace vcl

/**
 A helper class that calls Application::ReleaseSolarMutex() in its constructor
 and restores the mutex in its destructor.
*/
class SolarMutexReleaser
{
    const sal_uInt32 mnReleased;
public:
    SolarMutexReleaser(): mnReleased(Application::ReleaseSolarMutex()) {}
    ~SolarMutexReleaser() { Application::AcquireSolarMutex( mnReleased ); }
};

VCL_DLLPUBLIC Application* GetpApp();

VCL_DLLPUBLIC bool InitVCL();
VCL_DLLPUBLIC void DeInitVCL();

VCL_DLLPUBLIC bool InitAccessBridge();

// only allowed to call, if no thread is running. You must call JoinMainLoopThread to free all memory.
VCL_DLLPUBLIC void CreateMainLoopThread( oslWorkerFunction pWorker, void * pThreadData );
VCL_DLLPUBLIC void JoinMainLoopThread();

inline void Application::EndYield()
{
    PostUserEvent( Link<void*,void>() );
}

#endif // _APP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
