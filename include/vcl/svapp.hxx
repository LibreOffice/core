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

#include <config_features.h>

#include <sal/config.h>

#include <cassert>
#include <stdexcept>
#include <vector>

#include <comphelper/solarmutex.hxx>
#include <rtl/ustring.hxx>
#include <osl/thread.hxx>
#include <tools/link.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/apptypes.hxx>
#include <vcl/settings.hxx>
#include <vcl/vclevent.hxx>

class BitmapEx;
class Link;
class AllSettings;
class DataChangedEvent;
class Accelerator;
class Help;
class OutputDevice;
class Window;
class WorkWindow;
class MenuBar;
class UnoWrapperBase;
class Reflection;
class KeyCode;
class NotifyEvent;
class KeyEvent;
class MouseEvent;
class ZoomEvent;
class ScrollEvent;

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/connection/XConnection.hpp>

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
}
} } }

// helper needed by SalLayout implementations as well as svx/source/dialog/svxbmpnumbalueset.cxx
VCL_DLLPUBLIC sal_UCS4 GetMirroredChar( sal_UCS4 );
VCL_DLLPUBLIC sal_UCS4 GetLocalizedChar( sal_UCS4, LanguageType );

#define SYSTEMWINDOW_MODE_NOAUTOMODE    ((sal_uInt16)0x0001)
#define SYSTEMWINDOW_MODE_DIALOG        ((sal_uInt16)0x0002)

typedef long (*VCLEventHookProc)( NotifyEvent& rEvt, void* pData );

// ATTENTION: ENUM duplicate in daemon.cxx under Unix!

#ifdef UNX
enum Service { SERVICE_OLE, SERVICE_APPEVENT, SERVICE_IPC };
#endif

class VCL_DLLPUBLIC ApplicationEvent
{
public:
    enum Type {
        TYPE_ACCEPT, TYPE_APPEAR, TYPE_HELP, TYPE_VERSION, TYPE_OPEN,
        TYPE_OPENHELPURL, TYPE_PRINT, TYPE_PRIVATE_DOSHUTDOWN, TYPE_QUICKSTART,
        TYPE_SHOWDIALOG, TYPE_UNACCEPT
    };

    explicit ApplicationEvent(Type type): aEvent(type) {
        assert(
            type == TYPE_APPEAR || type == TYPE_VERSION
            || type == TYPE_PRIVATE_DOSHUTDOWN || type == TYPE_QUICKSTART);
    }

    ApplicationEvent(Type type, OUString const & data): aEvent(type) {
        assert(
            type == TYPE_ACCEPT || type == TYPE_HELP || type == TYPE_OPENHELPURL
            || type == TYPE_SHOWDIALOG || type == TYPE_UNACCEPT);
        aData.push_back(data);
    }

    ApplicationEvent(Type type, std::vector<OUString> const & data):
        aEvent(type), aData(data)
    { assert(type == TYPE_OPEN || type == TYPE_PRINT); }

    Type GetEvent() const { return aEvent; }

    OUString GetStringData() const {
        assert(
            aEvent == TYPE_ACCEPT || aEvent == TYPE_HELP
            || aEvent == TYPE_OPENHELPURL || aEvent == TYPE_SHOWDIALOG
            || aEvent == TYPE_UNACCEPT);
        assert(aData.size() == 1);
        return aData[0];
    }

    std::vector<OUString> const & GetStringsData() const {
        assert(aEvent == TYPE_OPEN || aEvent == TYPE_PRINT);
        return aData;
    }

private:
    Type aEvent;
    std::vector<OUString> aData;
};

/**
 @brief Abstract base class used mainly for the LibreOffice Desktop class.

 The Application class is an abstract class with one pure virtual
 function: Main(), however, there are a \em lot of static functions
 that are vital for applications. It is really meant to be subclassed
 to provide a global singleton, and heavily relies on a single data
 structure ImplSVData

 The reason Application exists is because the VCL used to be a
 standalone framework, long since abandoned by anything other than
 our application.

 @see   Desktop, ImplSVData
 */
class VCL_DLLPUBLIC Application
{
public:
    enum DialogCancelMode {
        DIALOG_CANCEL_OFF,      ///< do not automatically cancel dialogs
        DIALOG_CANCEL_SILENT,   ///< silently cancel any dialogs
        DIALOG_CANCEL_FATAL     ///< cancel any dialogs by std::abort
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
            ImplInitSVData initializes the global instance data
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
                TheWindow(Window *parent, WinBits windowStyle) :
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
    virtual int                 Main() = 0;

    /** Exit from the application

     @returns true if exited successfully, false if not able to fully exit
    */
    virtual sal_Bool           QueryExit();

    /** Send user event.

    @param  nEvent      The numeric ID of the event
    @param  pEventData  Pointer to a data associated with the event. Use
                        a reinterpret_cast<void*> to pass in the parameter.
    */
    virtual void                UserEvent( sal_uLong nEvent, void* pEventData );

    /** @name Change Notification Functions

        Functions that notify when changes occur in the application.
    */
    ///@{

    /** Notify that the application is no longer the "focused" (or current)
        application - needed for Windowing systems where an end user can switch
        from one application to another.

     @see DataChanged
    */
    virtual void                FocusChanged();

    /** Notify the application that data has changed via an event.

     @param rDCEvt      Const reference to a DataChangedEvent object

     @see FocusChanged, NotifyAllWindows
    */
    virtual void                DataChanged( const DataChangedEvent& rDCEvt );

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

    /** Handles an error code.

     @remark This is not actually an exception. It merely takes an
        error code, then in most cases aborts. The list of exception
        identifiers can be found at include/vcl/apptypes.hxx - each
        one starts with EXC_*

     @param nError      The error code identifier

     @returns sal_uInt16 value - if it is 0, then the error wasn't handled.

     @see Abort
    */
    virtual sal_uInt16          Exception( sal_uInt16 nError );

    /** Ends the program prematurely with an error message.

     If the \code --norestore \endcode command line argument is given (assuming
     this process is run by developers who are interested in cores,
     vs. end users who are not) then it does a coredump.

     @param rErrorText  The error message to report.

     @see Exception
    */
    static void                 Abort( const OUString& rErrorText );

    ///@}

    /** @name Event Loop

        Event loop functions

        Functions that handle the LibreOffice main event loop are here,
        including a global lock called the Solar Mutex.
    */
    ///@{

    /** Run the main event processing loop until it is quit by Quit().

     @see Quit, Reschedule, Yield, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
          EnableNoYieldMode, DisableNoYieldMode, AddPostYieldListener,
          RemovePostYieldListener
    */
    static void                 Execute();

    /** Quit the program

     @see Execute, Reschedule, Yield, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
          EnableNoYieldMode, DisableNoYieldMode, AddPostYieldListener,
          RemovePostYieldListener
    */
    static void                 Quit();

    /** Attempt to reschedule in processing of current event(s)

     @param bAllEvents  If set to true, then try to process all the
        events. If set to false, then only process the current
        event. Defaults to false.

     @see Execute, Quit, Yield, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
          EnableNoYieldMode, DisableNoYieldMode, AddPostYieldListener,
          RemovePostYieldListener
     */
    static void                 Reschedule( bool bAllEvents = false );

    /** Allow processing of the next event.

     @see Execute, Quit, Reschedule, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
          EnableNoYieldMode, DisableNoYieldMode, AddPostYieldListener,
          RemovePostYieldListener
    */
    static void                 Yield();

    /**

     @see Execute, Quit, Reschedule, Yield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
          EnableNoYieldMode, DisableNoYieldMode, AddPostYieldListener,
          RemovePostYieldListener
    */
    static void                 EndYield();

    /** @brief Get the Solar Mutex for this thread.

     Get the Solar Mutex that prevents other threads from accessing VCL
     concurrently.

     @returns SolarMutex reference

     @see Execute, Quit, Reschedule, Yield, EndYield,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
          EnableNoYieldMode, DisableNoYieldMode, AddPostYieldListener,
          RemovePostYieldListener
    */
    static comphelper::SolarMutex& GetSolarMutex();

    /** Get the main thread ID.

     @returns oslThreadIdentifier that contains the thread ID

     @see Execute, Quit, Reschedule, Yield, EndYield, GetSolarMutex,
          ReleaseSolarMutex, AcquireSolarMutex,
          EnableNoYieldMode, DisableNoYieldMode, AddPostYieldListener,
          RemovePostYieldListener
    */
    static oslThreadIdentifier  GetMainThreadIdentifier();

    /** @brief Release Solar Mutex(es) for this thread

     Release the Solar Mutex(es) that prevents other threads from accessing
     VCL concurrently.

     @returns The number of mutexes that were acquired by this thread.

     @see Execute, Quit, Reschedule, Yield, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, AcquireSolarMutex,
          EnableNoYieldMode, DisableNoYieldMode, AddPostYieldListener,
          RemovePostYieldListener
    */
    static sal_uLong            ReleaseSolarMutex();

    /** @brief Acquire Solar Mutex(es) for this thread.

     Acquire the Solar Mutex(es) that prevents other threads from accessing
     VCL concurrently.

     @see Execute, Quit, Reschedule, Yield, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex,
          EnableNoYieldMode, DisableNoYieldMode, AddPostYieldListener,
          RemovePostYieldListener
    */
    static void                 AcquireSolarMutex( sal_uLong nCount );

    /** @brief Enables "no yield" mode

     "No yield" mode prevents Yield() from waiting for events.

     @remarks This was originally implemented in OOo bug 98792 to improve
        Impress slideshows.

     @see DisableNoYieldMode, Execute, Quit, Reschedule, Yield, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
          DisableNoYield, AddPostYieldListener, RemovePostYieldListener
    */
    static void                 EnableNoYieldMode();

    /** @brief Disables "no yield" mode

     "No yield" mode prevents Yield() from waiting for events.

     @remarks This was originally implemented in OOo bug 98792 to improve
        Impress slideshows.

     @see EnableNoYieldMode, Execute, Quit, Reschedule, Yield, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
          EnableNoYield, AddPostYieldListener, RemovePostYieldListener
    */

    static void                 DisableNoYieldMode();

    /** Add a listener for yield events

     @param  i_rListener     Listener to add

     @see Execute, Quit, Reschedule, Yield, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
          EnableNoYieldMode, DisableNoYieldMode, RemovePostYieldListener
    */
    static void                 AddPostYieldListener( const Link& i_rListener );

    /** Remove listener for yield events

     @param  i_rListener     Listener to remove

     @see Execute, Quit, Reschedule, Yield, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
          AddPostYieldListener, EnableNoYieldMode, DisableNoYieldMode
    */
    static void                 RemovePostYieldListener( const Link& i_rListener );


    /** Queries whether the application is in "main", i.e. not yet in
        the event loop

     @returns   true if in main, false if not in main

     @see IsInExecute, IsInModalMode
    */
    static sal_Bool             IsInMain();

    /** Queries whether the application is in the event loop

     @returns   true if in the event loop, false if not

     @see IsInMain, IsInModalMode
    */
    static sal_Bool             IsInExecute();

    /** Queries whether application has a modal dialog active.

     @returns   true if a modal dialog is active, false if not

     @see IsInMain, IsInExecute
    */
    static sal_Bool             IsInModalMode();

    /** Return how many events are being dispatched.

     @returns   the number of events currently being dispatched
    */
    static sal_uInt16           GetDispatchLevel();

    /** Determine if there are any pending input events.

     @param     nType   input identifier, defined in include/vcl/apptypes.hxx
                        The default is VCL_INPUT_ANY.

     @returns   true if there are pending events, false if not.

     @see GetLastInputInterval
    */
    static bool                 AnyInput( sal_uInt16 nType = VCL_INPUT_ANY );

    /** The interval from the last time that input was received.

     @returns   system ticks - last input time

     @see AnyInput
    */
    static sal_uLong            GetLastInputInterval();

    ///@}

    /* Determines if the UI is captured.

     The UI is considered captured if a system dialog is open (e.g. printer setup),
     a floating window, menu or toolbox dropdown is open, or a window has been
     captured by the mouse.

     @returns   true if UI is captured, false if not
     */
    static sal_Bool             IsUICaptured();

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
    virtual void                SystemSettingsChanging( AllSettings& rSettings );

    /** Set the settings object to the platform/desktop environment system
     settings.

     @param     rSettings       Reference to the settings object to change.

     @see SystemSettingsChanging, SetSettings, GetSettings
    */
    static void                 MergeSystemSettings( AllSettings& rSettings );

    /** Sets the application's settings and notifies all windows of the
     change.

     @param     rSettings       const reference to settings object used to
                                change the application's settings.

     @see SystemSettingsChanging, MergeSystemSettings, GetSettings
    */
    static void                 SetSettings( const AllSettings& rSettings );

    /** Gets the application's settings. If the application hasn't initialized
     it's settings, then it does so (lazy initialization).

     @returns AllSettings instance that contains the current settings of the
        application.

     @see SystemSettingsChanging, MergeSystemSettings, SetSettings
    */
    static const AllSettings&   GetSettings();

    /** Validate that the currently selected system UI font is suitable
     to display the application's UI.

     A localized test string will be checked if it can be displayed in the currently
     selected system UI font. If no glyphs are missing it can be assumed that the font
     is proper for display of the application's UI.

     @returns true if the system font is suitable for our UI and false if the test
       string could not be displayed with the system font.
    */
    static bool                 ValidateSystemFont();

    /** Get the application's locale data wrapper.

     @returns reference to a LocaleDataWrapper object
    */
    static const LocaleDataWrapper& GetAppLocaleDataWrapper();

    ///@}

    /** @name Event Listeners

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
    static void                 AddEventListener( const Link& rEventListener );

    /** Remove a VCL event listener from the application.

     @param     rEventListener  Const refernece to the event listener to be removed

     @see AddEventListener, AddKeyListener, RemoveKeyListener
    */
    static void                 RemoveEventListener( const Link& rEventListener );

    /** Add a keypress listener to the application. If keypress listener exists,
     then initialize the application's keypress event listener with a new one, then
     add the keypress listener.

     @param     rKeyListener    Const reference to the keypress event listener to add

     @see AddEventListener, RemoveEventListener, RemoveKeyListener
    */
    static void                 AddKeyListener( const Link& rKeyListener );

    /** Remove a keypress listener from the application.

     @param     rKeyListener    Const reference to the keypress event listener to be removed

     @see AddEventListener, RemoveEventListener, AddKeyListener
    */
    static void                 RemoveKeyListener( const Link& rKeyListener );

    /** Send event to all VCL application event listeners

     @param     nEvent          Event ID
     @param     pWin            Pointer to window to send event
     @param     pData           Pointer to data to send with event

     @see ImplCallEventListeners(VclSimpleEvent* pEvent)
    */
    static void                 ImplCallEventListeners( sal_uLong nEvent, Window* pWin, void* pData );

    /** Send event to all VCL application event listeners

     @param     pEvent          Pointer to VclSimpleEvent

     @see ImplCallEventListeners(sal_uLong nEvent, Windows* pWin, void* pData);
    */
    static void                 ImplCallEventListeners( VclSimpleEvent* pEvent );

    /** Handle keypress event

     @param     nEvent          Event ID for keypress
     @param     pWin            Pointer to window that receives the event
     @param     pKeyEvent       Received key event

     @see PostKeyEvent
    */
    static sal_Bool             HandleKey( sal_uLong nEvent, Window *pWin, KeyEvent* pKeyEvent );

    /** Send keypress event

     @param     nEvent          Event ID for keypress
     @param     pWin            Pointer to window to which the event is sent
     @param     pKeyEvent       Key event to send

     @see HandleKey
    */
    static sal_uLong            PostKeyEvent( sal_uLong nEvent, Window *pWin, KeyEvent* pKeyEvent );

    /** Send mouse event

     @param     nEvent          Event ID for mouse event
     @param     pWin            Pointer to window to which the event is sent
     @param     pKeyEvent       Mouse event to send
    */
    static sal_uLong            PostMouseEvent( sal_uLong nEvent, Window *pWin, MouseEvent* pMouseEvent );

#if !HAVE_FEATURE_DESKTOP
    /** Send zoom event

     @param     nEvent          Event ID for zoom event
     @param     pWin            Pointer to window to which the event is sent
     @param     pZoomEvent      Zoom event to send
    */
    static sal_uLong            PostZoomEvent( sal_uLong nEvent, Window *pWin, ZoomEvent* pZoomEvent );

    /* Send scroll event

     @param      nEvent          Event ID for scroll event
     @param      pWin            Pointer to window to which the event is sent
     @param      pScrollEvent    Scroll event to send
    */
    static sal_uLong            PostScrollEvent( sal_uLong nEvent, Window *pWin, ScrollEvent* pScrollEvent );
#endif

    /** Remove mouse and keypress events from a window... any also zoom and scroll events
     if the platform supports it.

     @param     pWin            Window to remove events from

     @see HandleKey, PostKeyEvent, PostMouseEvent, PostZoomEvent, PostScrollEvent
    */
    static void                 RemoveMouseAndKeyEvents( Window *pWin );

    /** Post a user event to the default window.

     User events allow for the deferral of work to later in the main-loop - at idle.

     @param     rLink           Link to event callback function
     @param     pCaller         Pointer to data sent to the event by the caller. Optional.

     @return the event ID used to post the event.
    */
    static sal_uLong            PostUserEvent( const Link& rLink, void* pCaller = NULL );

    /** Post a user event to the default window.

     @param     rEventID        Reference to event ID to be posted
     @param     rLink           Link to event callback function
     @param     pCaller         Pointer to data sent to teh event by the caller. Optional.

     @return true if there is a default window and the event could be posted to it successfully.
    */
    static sal_Bool             PostUserEvent( sal_uLong& rEventId, const Link& rLink, void* pCaller = NULL );

    /** Remove user event based on event ID

     @param     nUserEvent      Numeric user event to remove
    */
    static void                 RemoveUserEvent( sal_uLong nUserEvent );

    /** Insert an idle handler into the application.

     If the idle event manager doesn't exist, then initialize it.

     @param     rLink           const reference to the idle handler
     @param     nPrio           The priority of the idle handler - idle handlers of a higher
                                priority will be processed before this handler.

     @return true if the handler was inserted successfully, false if it couldn't be inserted.
    */
    static sal_Bool             InsertIdleHdl( const Link& rLink, sal_uInt16 nPriority );

    /** Remove an idle handler from the application.

     @param     rLink           const reference to the idle handler to remove
    */
    static void                 RemoveIdleHdl( const Link& rLink );


    /** @deprecated AppEvent is used only in the Desktop class now. However, it is
     intended to notify the application that an event has occured. It was in oldsv.cxx,
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

    /** Get the currently focussed window.

     @returns Pointer to focused window.

     @see GetAppWindow, GetDefaultDevice
    */
    static Window*              GetFocusWindow();

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
    static Window*              GetFirstTopLevelWindow();

    /** Get the next top level window.

     @param     pWindow     Pointer to Window object you wish to get the next
                            window from.

     @returns Pointer to next top window.
    */
    static Window*              GetNextTopLevelWindow( Window* pWindow );

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
    static Window*              GetTopWindow( long nIndex );

    /** Get the "active" top window.

     An "active" top window is one that has a child window that has the
     application's focus.

     @returns the active top window

     @see GetFirstTopLevelWindow, GetNextTopLevelWindow, GetTopWindowCount,
          GetTopWindow
    */
    static Window*              GetActiveTopWindow();

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

    /** Load a localized branding PNG file as a bitmap.

     @param     pName           Name of the bitmap to load.
     @param     rBitmap         Reference to BitmapEx object to load PNG into

     @returns true if the PNG could be loaded, otherwise returns false.
    */
    static bool                 LoadBrandBitmap (const char* pName, BitmapEx &rBitmap);

    ///*}

    /** @name Display and Screen
    */
    ///*{

    /** Set the default name of the application for message dialogs and printing.

     @param     rDisplayName    const reference to string to set the Display name to.

     @see GetDisplayName
    */
    static void                 SetDisplayName( const OUString& rDisplayName );

    /** Get the default name of the application for messag dialogs and printing.

     @returns The display name of the application.
    */
    static OUString             GetDisplayName();

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
    static Rectangle            GetScreenPosSizePixel( unsigned int nScreen );

    /** Determines if the screens that make up a display are seperate or
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
    SAL_DLLPRIVATE static unsigned int GetBestScreen( const Rectangle& );

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

    //@}

    /** Insert accelerator

     @param     pAccel          Pointer to an Accelerator object to insert

     @returns true if successful, false if otherwise
    */
    static sal_Bool             InsertAccel( Accelerator* pAccel );

    /** Remove accelerator

     @param     pAccel          Pointer to Accelerator object to remove
    */
    static void                 RemoveAccel( Accelerator* pAccel );

    static long                 CallEventHooks( NotifyEvent& rEvt );

    static void                 SetHelp( Help* pHelp = NULL );
    static Help*                GetHelp();

    static void                 EnableAutoHelpId( sal_Bool bEnabled = sal_True );
    static sal_Bool             IsAutoHelpIdEnabled();

    static void                 EnableAutoMnemonic( sal_Bool bEnabled = sal_True );
    static sal_Bool             IsAutoMnemonicEnabled();

    static sal_uLong            GetReservedKeyCodeCount();
    static const KeyCode*       GetReservedKeyCode( sal_uLong i );

    static void                 SetDefDialogParent( Window* pWindow );
    static Window*              GetDefDialogParent();

    static DialogCancelMode     GetDialogCancelMode();
    static void                 SetDialogCancelMode( DialogCancelMode mode );
    static sal_Bool             IsDialogCancelEnabled();

    static void                 SetSystemWindowMode( sal_uInt16 nMode );
    static sal_uInt16           GetSystemWindowMode();

    static void                 SetDialogScaleX( short nScale );

    static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDisplayConnection > GetDisplayConnection();

    // The global service manager has to be created before!
    static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > GetVCLToolkit();
    static UnoWrapperBase*      GetUnoWrapper( sal_Bool bCreateIfNotExists = sal_True );
    static void                 SetUnoWrapper( UnoWrapperBase* pWrapper );

    static void                 SetFilterHdl( const Link& rLink );
    static const Link&          GetFilterHdl();

    static void                 EnableHeadlessMode( bool dialogsAreFatal );
    static sal_Bool             IsHeadlessModeEnabled();
    /// check command line arguments for --headless
    static bool                 IsHeadlessModeRequested();
    /// used to disable Mac specific app init that requires an app bundle
    static void                 EnableConsoleOnly();
    /// used to see if Mac specific app init has been disabled
    static bool                 IsConsoleOnly();

    static void                 ShowNativeErrorBox(const OUString& sTitle  ,
                                                   const OUString& sMessage);

    // IME Status Window Control:

    /** Return true if any IME status window can be toggled on and off
        externally.

        Must only be called with the Solar mutex locked.
     */
    static bool                 CanToggleImeStatusWindow();

    /** Toggle any IME status window on and off.

        This only works if CanToggleImeStatusWinodw returns true (otherwise,
        any calls of this method are ignored).

        Must only be called with the Solar mutex locked.
     */
    static void                 ShowImeStatusWindow(bool bShow);

    /** Return true if any IME status window should be turned on by default
        (this decision can be locale dependent, for example).

        Can be called without the Solar mutex locked.
     */
    static bool                 GetShowImeStatusWindowDefault();

    /** Returns a string representing the desktop environment
        the process is currently running in.
     */
    static const OUString&      GetDesktopEnvironment();

    /** Add a file to the system shells recent document list if there is any.
          This function may have no effect under Unix because there is no
          standard API among the different desktop managers.

          @param rFileUrl
                    The file url of the document.

          @param rMimeType
          The mime content type of the document specified by aFileUrl.
          If an empty string will be provided "application/octet-stream"
          will be used.
    */
    static void                 AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService);

    /** Do we have a native / system file selector available ?
     */
    static bool                 hasNativeFileSelection();

    /** Create a platform specific file picker, if one is available,
        otherwise return an empty reference
    */
    static com::sun::star::uno::Reference< com::sun::star::ui::dialogs::XFilePicker2 >
        createFilePicker( const com::sun::star::uno::Reference<
                              com::sun::star::uno::XComponentContext >& rServiceManager );

    /** Create a platform specific folder picker, if one is available,
        otherwise return an empty reference
    */
    static com::sun::star::uno::Reference< com::sun::star::ui::dialogs::XFolderPicker2 >
        createFolderPicker( const com::sun::star::uno::Reference<
                              com::sun::star::uno::XComponentContext >& rServiceManager );

    static bool                 IsEnableAccessInterface() {return true;}

private:

    static void InitSettings();

    DECL_STATIC_LINK( Application, PostEventHandler, void* );
};


class VCL_DLLPUBLIC SolarMutexGuard
{
    private:
        SolarMutexGuard( const SolarMutexGuard& );
        const SolarMutexGuard& operator = ( const SolarMutexGuard& );
        comphelper::SolarMutex& m_solarMutex;

    public:

        /** Acquires the object specified as parameter.
         */
        SolarMutexGuard() :
        m_solarMutex(Application::GetSolarMutex())
    {
        m_solarMutex.acquire();
    }

    /** Releases the mutex or interface. */
    ~SolarMutexGuard()
    {
        m_solarMutex.release();
    }
};

class VCL_DLLPUBLIC SolarMutexClearableGuard
{
    SolarMutexClearableGuard( const SolarMutexClearableGuard& );
    const SolarMutexClearableGuard& operator = ( const SolarMutexClearableGuard& );
    bool m_bCleared;
public:
    /** Acquires mutex
        @param pMutex pointer to mutex which is to be acquired  */
    SolarMutexClearableGuard()
        : m_bCleared(false)
        , m_solarMutex( Application::GetSolarMutex() )
        {
            m_solarMutex.acquire();
        }

    /** Releases mutex. */
    virtual ~SolarMutexClearableGuard()
        {
            if( !m_bCleared )
            {
                m_solarMutex.release();
            }
        }

    /** Releases mutex. */
    void SAL_CALL clear()
        {
            if( !m_bCleared )
            {
                m_solarMutex.release();
                m_bCleared = true;
            }
        }
protected:
    comphelper::SolarMutex& m_solarMutex;
};

class VCL_DLLPUBLIC SolarMutexResettableGuard
{
    SolarMutexResettableGuard( const SolarMutexResettableGuard& );
    const SolarMutexResettableGuard& operator = ( const SolarMutexResettableGuard& );
    bool m_bCleared;
public:
    /** Acquires mutex
        @param pMutex pointer to mutex which is to be acquired  */
    SolarMutexResettableGuard()
        : m_bCleared(false)
        , m_solarMutex( Application::GetSolarMutex() )
        {
            m_solarMutex.acquire();
        }

    /** Releases mutex. */
    virtual ~SolarMutexResettableGuard()
        {
            if( !m_bCleared )
            {
                m_solarMutex.release();
            }
        }

    /** Releases mutex. */
    void SAL_CALL clear()
        {
            if( !m_bCleared)
            {
                m_solarMutex.release();
                m_bCleared = true;
            }
        }
    /** Releases mutex. */
    void SAL_CALL reset()
        {
            if( m_bCleared)
            {
                m_solarMutex.acquire();
                m_bCleared = false;
            }
        }
protected:
    comphelper::SolarMutex& m_solarMutex;
};


/**
 A helper class that calls Application::ReleaseSolarMutex() in its constructor
 and restores the mutex in its destructor.
*/
class SolarMutexReleaser
{
    sal_uLong mnReleased;

public:
    SolarMutexReleaser(): mnReleased(Application::ReleaseSolarMutex()) {}

    ~SolarMutexReleaser()
    {
        Application::AcquireSolarMutex( mnReleased );
    }
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
    PostUserEvent( Link() );
}

#endif // _APP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
