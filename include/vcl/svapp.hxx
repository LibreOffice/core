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
 @brief Abstract base class used for the LibreOffice Desktop class.

 Abstract base class used for the LibreOffice Desktop class.

 It is only actually used by the Desktop class, the reason that
 Application exists is because the VCL used to be a standalone
 framework, long since abandoned by anything other than
 OpenOffice and variants.

 @see   Desktop
 */
class VCL_DLLPUBLIC Application
{
public:
    enum DialogCancelMode {
        DIALOG_CANCEL_OFF,      ///< do not automatically cancel dialogs
        DIALOG_CANCEL_SILENT,   ///< silently cancel any dialogs
        DIALOG_CANCEL_FATAL     ///< cancel any dialogs by std::abort
    };

    /**
    Default constructor for Application class.

    Initializes the LibreOffice global instance data structure if needed,
    and then sets itself to be the Application class. Also initializes any
    platform specific data structures.

    @attention The initialization of the application itself is done in
            Init()

    @see    InitSalData() is implemented by platform specific code.
            ImplInitSVData() initializes the global instance data
    */
                                Application();

    /** Virtual destructor for Application class.

    Deinitializes the LibreOffice global instance data structure, then
    deinitializes any platform specific data structures.

     @see   ImplDeInitSVData() deinitializes the global instance data,
            DeInitSalData() is implemented by platform specific code
    */
    virtual                     ~Application();

    /** Pure virtual entrypoint to the application.
    */
    virtual int                 Main() = 0;

    /** Exit from the application

     @returns true if exited successfully, false if not able to fully exit
    */
    virtual sal_Bool                QueryExit();

    /** Send user event.

    @param  nEvent      The numeric ID of the event
    @param  pEventData  Pointer to a data associated with the event. Use
                        a reinterpret_cast<void*> to pass in the parameter.
    */
    virtual void                UserEvent( sal_uLong nEvent, void* pEventData );

    // Functions that notify when changes occur in the application

    /** Notify that the application is no longer the "focused" (or current)
        application - needed for Windowing systems where an end user can switch
        from one application to another.

     @see DataChanged
    */
    virtual void                FocusChanged();

    /** Notify the application that data has changed via an event.

     @param rDCEvt      Reference to a DataChangedEvent object - this will not
                        be changed

     @see FocusChanged
    */
    virtual void                DataChanged( const DataChangedEvent& rDCEvt );

    // Initialization functions

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

    // Command line processing:

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

    // Error handling
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

     If the --norestore command line argument is given (assuming
     this process is run by developers who are interested in cores,
     vs. end users who are not) then it does a coredump.

     @param rErrorText  The error message to report.

     @see Exception
    */
    static void                 Abort( const OUString& rErrorText );

    // Event loop functions:

    /** Start executing the program.

     @see Quit, Reschedule, Yield, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
          EnableNoYieldMode, AddPostYieldListener, RemovePostYieldListener
    */
    static void                 Execute();

    /** Quit the program

     @see Execute, Reschedule, Yield, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
          EnableNoYieldMode, AddPostYieldListener, RemovePostYieldListener
    */
    static void                 Quit();

    /** Attempt to reschedule in processing of current event(s)

     @param bAllEvents  If set to true, then try to process all the
        events. If set to false, then only process the current
        event. Defaults to false.

     @see Execute, Quit, Yield, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
          EnableNoYieldMode, AddPostYieldListener, RemovePostYieldListener

     */
    static void                 Reschedule( bool bAllEvents = false );

    /** Allow processing of the next event.

     @see Execute, Quit, Reschedule, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
          EnableNoYieldMode, AddPostYieldListener, RemovePostYieldListener
    */
    static void                 Yield();

    /**

     @see Execute, Quit, Reschedule, Yield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
          EnableNoYieldMode, AddPostYieldListener, RemovePostYieldListener
    */
    static void                 EndYield();

    /** @Brief Get the Solar Mutex for this thread.

     Get the Solar Mutex that prevents other threads from accessing VCL
     concurrently.

     @returns SolarMutex reference

     @see Execute, Quit, Reschedule, Yield, EndYield,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
          EnableNoYieldMode, AddPostYieldListener, RemovePostYieldListener
    */
    static comphelper::SolarMutex& GetSolarMutex();

    /** Get the main thread ID.

     @returns oslThreadIdentifier that contains the thread ID

     @see Execute, Quit, Reschedule, Yield, EndYield, GetSolarMutex,
          ReleaseSolarMutex, AcquireSolarMutex,
          EnableNoYieldMode, AddPostYieldListener, RemovePostYieldListener
    */
    static oslThreadIdentifier  GetMainThreadIdentifier();

    /** @Brief Release Solar Mutex(es) for this thread

     Release the Solar Mutex(es) that prevents other threads from accessing
     VCL concurrently.

     @returns The number of mutexes that were acquired by this thread.

     @see Execute, Quit, Reschedule, Yield, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, AcquireSolarMutex,
          EnableNoYieldMode, AddPostYieldListener, RemovePostYieldListener
    */

    static sal_uLong            ReleaseSolarMutex();

    /** @Brief Acquire Solar Mutex(es) for this thread.

     Acquire the Solar Mutex(es) that prevents other threads from accessing
     VCL concurrently.

     @see Execute, Quit, Reschedule, Yield, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex,
          EnableNoYieldMode, AddPostYieldListener, RemovePostYieldListener
    */
    static void                 AcquireSolarMutex( sal_uLong nCount );

    /** @Brief Enables "no yield" mode

     "No yield" mode prevents \code Yield() \endcode from waiting for events.

     @remarks This was originally implemented in OOo bug 98792 to improve
        Impress slideshows. For some reason, to \em disable no yield mode, you
        call on EnableNoYieldMode

     @param i_bNoYield      If set to false, then "no yield" mode is turned off.
                            If set to true, then "no yield" mode is turned on.

     @see Execute, Quit, Reschedule, Yield, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
          AddPostYieldListener, RemovePostYieldListener
    */
    static void                 EnableNoYieldMode( bool i_bNoYield );

    /** Add a listener for yield events

     @param  i_rListener     Listener to add

     @see Execute, Quit, Reschedule, Yield, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
          RemovePostYieldListener
    */
    static void                 AddPostYieldListener( const Link& i_rListener );


    /** Remove listener for yield events

     @param  i_rListener     Listener to remove

     @see Execute, Quit, Reschedule, Yield, EndYield, GetSolarMutex,
          GetMainThreadIdentifier, ReleaseSolarMutex, AcquireSolarMutex,
          AddPostYieldListener
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

    /* Determines if the UI is captured.

     The UI is considered captured if a system dialog is open (e.g. printer setup),
     a floating window, menu or toolbox dropdown is open, or a window has been
     captured by the mouse.

     @returns   true if UI is captured, false if not
     */
    static sal_Bool             IsUICaptured();

    virtual void                SystemSettingsChanging( AllSettings& rSettings,
                                                        Window* pFrame );
    static void                 MergeSystemSettings( AllSettings& rSettings );

    static void                 SetSettings( const AllSettings& rSettings );
    static const AllSettings&   GetSettings();
    static void                 NotifyAllWindows( DataChangedEvent& rDCEvt );

    static void                 AddEventListener( const Link& rEventListener );
    static void                 RemoveEventListener( const Link& rEventListener );
    static void                 AddKeyListener( const Link& rKeyListener );
    static void                 RemoveKeyListener( const Link& rKeyListener );
    static void                 ImplCallEventListeners( sal_uLong nEvent, Window* pWin, void* pData );
    static void                 ImplCallEventListeners( VclSimpleEvent* pEvent );
    static sal_Bool             HandleKey( sal_uLong nEvent, Window *pWin, KeyEvent* pKeyEvent );

    static sal_uLong            PostKeyEvent( sal_uLong nEvent, Window *pWin, KeyEvent* pKeyEvent );
    static sal_uLong            PostMouseEvent( sal_uLong nEvent, Window *pWin, MouseEvent* pMouseEvent );
#if !HAVE_FEATURE_DESKTOP
    static sal_uLong            PostZoomEvent( sal_uLong nEvent, Window *pWin, ZoomEvent* pZoomEvent );
    static sal_uLong            PostScrollEvent( sal_uLong nEvent, Window *pWin, ScrollEvent* pScrollEvent );
#endif
    static void                 RemoveMouseAndKeyEvents( Window *pWin );

    static sal_uLong            PostUserEvent( const Link& rLink, void* pCaller = NULL );
    static sal_Bool             PostUserEvent( sal_uLong& rEventId, const Link& rLink, void* pCaller = NULL );
    static void                 RemoveUserEvent( sal_uLong nUserEvent );

    static sal_Bool             InsertIdleHdl( const Link& rLink, sal_uInt16 nPriority );
    static void                 RemoveIdleHdl( const Link& rLink );

    virtual void                AppEvent( const ApplicationEvent& rAppEvent );

    static WorkWindow*          GetAppWindow();
    static Window*              GetFocusWindow();
    static OutputDevice*        GetDefaultDevice();

    static Window*              GetFirstTopLevelWindow();
    static Window*              GetNextTopLevelWindow( Window* pWindow );

    static long                 GetTopWindowCount();
    static Window*              GetTopWindow( long nIndex );
    static Window*              GetActiveTopWindow();

    static void                 SetAppName( const OUString& rUniqueName );
    static OUString             GetAppName();
    static bool                 LoadBrandBitmap (const char* pName, BitmapEx &rBitmap);

    /** validate that the currently selected system UI font is suitable
        to display the application's UI.

        A localized test string will be checked if it can be displayed
        in the currently selected system UI font. If no glyphs are
        missing it can be assumed that the font is proper for display
        of the application's UI.

        @returns
        <TRUE/> if the system font is suitable for our UI
        <FALSE/> if the test string could not be displayed with the system font
     */

    // default name of the application for message dialogs and printing
    static bool                 ValidateSystemFont();

    static void                 SetDisplayName( const OUString& rDisplayName );
    static OUString             GetDisplayName();

    static unsigned int         GetScreenCount();
    static Rectangle            GetScreenPosSizePixel( unsigned int nScreen );

    // IsUnifiedDisplay returns:
    //        true:  screens form up one large display area
    //               windows can be moved between single screens
    //               (e.g. Xserver with Xinerama, Windows)
    //        false: different screens are separate and windows cannot be moved
    //               between them (e.g. Xserver with multiple screens)
    static bool                 IsUnifiedDisplay();
    // if IsUnifiedDisplay() == true the return value will be
    // nearest screen of the target rectangle
    // in case of IsUnifiedDisplay() == false the return value
    // will always be GetDisplayDefaultScreen()
    SAL_DLLPRIVATE static unsigned int  GetBestScreen( const Rectangle& );
    // This returns the LCD screen number for a laptop, or the primary
    // external VGA display for a desktop machine - it is where a presenter
    // console should be rendered if there are other (non-built-in) screens
    // present.
    static unsigned int         GetDisplayBuiltInScreen();
    // Practically, this means - Get the screen we should run a presentation on.
    static unsigned int         GetDisplayExternalScreen();

    static const LocaleDataWrapper& GetAppLocaleDataWrapper();

    static sal_Bool             InsertAccel( Accelerator* pAccel );
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
