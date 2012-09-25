/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_SVAPP_HXX
#define _SV_SVAPP_HXX

#include <sal/config.h>

#include <stdexcept>

#include <osl/thread.hxx>
#include <osl/mutex.hxx>
#include <tools/string.hxx>
#include <tools/link.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/apptypes.hxx>
#include <vcl/settings.hxx>
#include <vcl/vclevent.hxx>
#include <vcl/rendergraphicrasterizer.hxx>
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
        class XFolderPicker;
    }
}
namespace awt {
    class XToolkit;
    class XDisplayConnection;
}
} } }

// helper needed by SalLayout implementations as well as svx/source/dialog/svxbmpnumbalueset.cxx
VCL_DLLPUBLIC sal_UCS4 GetMirroredChar( sal_UCS4 );

// --------------------
// - SystemWindowMode -
// --------------------

#define SYSTEMWINDOW_MODE_NOAUTOMODE    ((sal_uInt16)0x0001)
#define SYSTEMWINDOW_MODE_DIALOG        ((sal_uInt16)0x0002)

// -------------
// - EventHook -
// -------------

typedef long (*VCLEventHookProc)( NotifyEvent& rEvt, void* pData );

// --------------------
// - ApplicationEvent -
// --------------------

// Build in again, in order to make AppEvents work on MACs

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

    ApplicationEvent() {}

    explicit ApplicationEvent(
        Type rEvent, const rtl::OUString& rData = rtl::OUString()):
        aEvent(rEvent),
        aData(rData)
    {}

    Type GetEvent() const { return aEvent; }
    const rtl::OUString& GetData() const { return aData; }

private:
    Type aEvent;
    rtl::OUString aData;
};

class VCL_DLLPUBLIC PropertyHandler
{
public:
    virtual void                Property( ApplicationProperty& ) = 0;

protected:
    ~PropertyHandler() {}
};

// ---------------
// - Application -
// ---------------

class VCL_DLLPUBLIC Application
{
public:
    enum DialogCancelMode {
        DIALOG_CANCEL_OFF, ///< do not automatically cancel dialogs
        DIALOG_CANCEL_SILENT, ///< silently cancel any dialogs
        DIALOG_CANCEL_FATAL
            ///< cancel any dialogs by throwing a DialogCancelledException
    };

    class VCL_DLLPUBLIC DialogCancelledException:
        virtual public std::runtime_error
    {
    public:
        explicit DialogCancelledException(char const * what_arg):
            runtime_error(what_arg) {}

        virtual ~DialogCancelledException() throw ();
    };

                                Application();
    virtual                     ~Application();

    virtual int                 Main() = 0;

    virtual sal_Bool                QueryExit();

    virtual void                UserEvent( sal_uLong nEvent, void* pEventData );

    virtual void                ActivateExtHelp();
    virtual void                DeactivateExtHelp();

    virtual void                FocusChanged();
    virtual void                DataChanged( const DataChangedEvent& rDCEvt );

    virtual void                Init();
    virtual void                InitFinished();
    virtual void                DeInit();

    static sal_uInt16               GetCommandLineParamCount();
    static XubString            GetCommandLineParam( sal_uInt16 nParam );
    static const XubString&     GetAppFileName();

    virtual sal_uInt16              Exception( sal_uInt16 nError );
    static void                 Abort( const XubString& rErrorText );

    static void                 Execute();
    static void                 Quit();
    static void                 Reschedule( bool bAllEvents = false );
    static void                 Yield( bool bAllEvents = false );
    static void                 EndYield();
    static osl::SolarMutex&     GetSolarMutex();
    static oslThreadIdentifier  GetMainThreadIdentifier();
    static sal_uLong                ReleaseSolarMutex();
    static void                 AcquireSolarMutex( sal_uLong nCount );
    static void                 EnableNoYieldMode( bool i_bNoYield );
    static void                 AddPostYieldListener( const Link& i_rListener );
    static void                 RemovePostYieldListener( const Link& i_rListener );

    static sal_Bool                 IsInMain();
    static sal_Bool                 IsInExecute();
    static sal_Bool                 IsInModalMode();

    static sal_uInt16               GetDispatchLevel();
    static sal_Bool                 AnyInput( sal_uInt16 nType = VCL_INPUT_ANY );
    static sal_uLong                GetLastInputInterval();
    static sal_Bool                 IsUICaptured();

    virtual void                SystemSettingsChanging( AllSettings& rSettings,
                                                        Window* pFrame );
    static void                 MergeSystemSettings( AllSettings& rSettings );
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
    static bool                 ValidateSystemFont();

    static void                 SetSettings( const AllSettings& rSettings );
    static const AllSettings&   GetSettings();
    static void                 NotifyAllWindows( DataChangedEvent& rDCEvt );

    static void                 AddEventListener( const Link& rEventListener );
    static void                 RemoveEventListener( const Link& rEventListener );
    static void                 AddKeyListener( const Link& rKeyListener );
    static void                 RemoveKeyListener( const Link& rKeyListener );
    static void                 ImplCallEventListeners( sal_uLong nEvent, Window* pWin, void* pData );
    static void                 ImplCallEventListeners( VclSimpleEvent* pEvent );
    static sal_Bool                 HandleKey( sal_uLong nEvent, Window *pWin, KeyEvent* pKeyEvent );

    static sal_uLong                PostKeyEvent( sal_uLong nEvent, Window *pWin, KeyEvent* pKeyEvent );
    static sal_uLong                PostMouseEvent( sal_uLong nEvent, Window *pWin, MouseEvent* pMouseEvent );
    static void                 RemoveMouseAndKeyEvents( Window *pWin );

    static sal_uLong                PostUserEvent( const Link& rLink, void* pCaller = NULL );
    static sal_Bool                 PostUserEvent( sal_uLong& rEventId, const Link& rLink, void* pCaller = NULL );
    static void                 RemoveUserEvent( sal_uLong nUserEvent );

    static sal_Bool                 InsertIdleHdl( const Link& rLink, sal_uInt16 nPriority );
    static void                 RemoveIdleHdl( const Link& rLink );

    virtual void                AppEvent( const ApplicationEvent& rAppEvent );

    virtual void                Property( ApplicationProperty& );

#ifndef NO_GETAPPWINDOW
    static WorkWindow*          GetAppWindow();
#endif

    static Window*              GetFocusWindow();
    static OutputDevice*        GetDefaultDevice();

    static Window*              GetFirstTopLevelWindow();
    static Window*              GetNextTopLevelWindow( Window* pWindow );

    static long                 GetTopWindowCount();
    static Window*              GetTopWindow( long nIndex );
    static Window*              GetActiveTopWindow();

    static void                 SetAppName( const String& rUniqueName );
    static String               GetAppName();
    static bool                 LoadBrandBitmap (const char* pName, BitmapEx &rBitmap);
    static vcl::RenderGraphicRasterizer  LoadBrandSVG (const char* pName);

    // default name of the application for message dialogs and printing
    static void                 SetDisplayName( const UniString& rDisplayName );
    static UniString            GetDisplayName();


    static unsigned int         GetScreenCount();
    static Rectangle            GetScreenPosSizePixel( unsigned int nScreen );

    // IsUnifiedDisplay returns:
    //        true:  screens form up one large display area
    //               windows can be moved between single screens
    //               (e.g. Xserver with Xinerama, Windows)
    //        false: different screens are separate and windows cannot be moved
    //               between them (e.g. Xserver with multiple screens)
    SAL_DLLPRIVATE static bool          IsUnifiedDisplay();
    // if IsUnifiedDisplay() == true the return value will be
    // nearest screen of the target rectangle
    // in case of IsUnifiedDisplay() == false the return value
    // will always be GetDisplayDefaultScreen()
    SAL_DLLPRIVATE static unsigned int  GetBestScreen( const Rectangle& );
    SAL_DLLPRIVATE static Rectangle     GetWorkAreaPosSizePixel( unsigned int nScreen );
    // This returns the LCD screen number for a laptop, or the primary
    // external VGA display for a desktop machine - it is where a presenter
    // console should be rendered if there are other (non-built-in) screens
    // present.
    SAL_DLLPRIVATE static unsigned int  GetDisplayBuiltInScreen();
    SAL_DLLPRIVATE static rtl::OUString GetDisplayScreenName( unsigned int nScreen );

    static const LocaleDataWrapper& GetAppLocaleDataWrapper();

    static sal_Bool                 InsertAccel( Accelerator* pAccel );
    static void                 RemoveAccel( Accelerator* pAccel );

    static long                 CallEventHooks( NotifyEvent& rEvt );

    static void                 SetHelp( Help* pHelp = NULL );
    static Help*                GetHelp();

    static void                 EnableAutoHelpId( sal_Bool bEnabled = sal_True );
    static sal_Bool                 IsAutoHelpIdEnabled();

    static void                 EnableAutoMnemonic( sal_Bool bEnabled = sal_True );
    static sal_Bool                 IsAutoMnemonicEnabled();

    static sal_uLong                GetReservedKeyCodeCount();
    static const KeyCode*       GetReservedKeyCode( sal_uLong i );

    static void                 SetDefDialogParent( Window* pWindow );
    static Window*              GetDefDialogParent();

    static DialogCancelMode GetDialogCancelMode();
    static void SetDialogCancelMode( DialogCancelMode mode );
    static sal_Bool                 IsDialogCancelEnabled();

    static void                 SetSystemWindowMode( sal_uInt16 nMode );
    static sal_uInt16               GetSystemWindowMode();

    static void                 SetDialogScaleX( short nScale );

    static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDisplayConnection > GetDisplayConnection();

    // The global service manager has to be created before!
    static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > GetVCLToolkit();
    static UnoWrapperBase*      GetUnoWrapper( sal_Bool bCreateIfNotExists = sal_True );
    static void                 SetUnoWrapper( UnoWrapperBase* pWrapper );

    static void                 SetFilterHdl( const Link& rLink );
    static const Link&          GetFilterHdl();

    static void                 EnableHeadlessMode( bool dialogsAreFatal );
    static sal_Bool                 IsHeadlessModeEnabled();

    static bool IsHeadlessModeRequested();
        ///< check command line arguments for --headless

    static void                 ShowNativeErrorBox(const String& sTitle  ,
                                                   const String& sMessage);

    // IME Status Window Control:

    /** Return true if any IME status window can be toggled on and off
        externally.

        Must only be called with the Solar mutex locked.
     */
    static bool CanToggleImeStatusWindow();

    /** Toggle any IME status window on and off.

        This only works if CanToggleImeStatusWinodw returns true (otherwise,
        any calls of this method are ignored).

        Must only be called with the Solar mutex locked.
     */
    static void ShowImeStatusWindow(bool bShow);

    /** Return true if any IME status window should be turned on by default
        (this decision can be locale dependent, for example).

        Can be called without the Solar mutex locked.
     */
    static bool GetShowImeStatusWindowDefault();

    /** Returns a string representing the desktop environment
        the process is currently running in.
     */
    static const ::rtl::OUString& GetDesktopEnvironment();

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
    static void AddToRecentDocumentList(const rtl::OUString& rFileUrl, const rtl::OUString& rMimeType);

    /** Do we have a native / system file selector available ?
     */
    static bool hasNativeFileSelection();

    /** Create a platform specific file picker, if one is available,
        otherwise return an empty reference
    */
    static com::sun::star::uno::Reference< com::sun::star::ui::dialogs::XFilePicker2 >
        createFilePicker( const com::sun::star::uno::Reference<
                              com::sun::star::uno::XComponentContext >& rServiceManager );

    /** Create a platform specific folder picker, if one is available,
        otherwise return an empty reference
    */
    static com::sun::star::uno::Reference< com::sun::star::ui::dialogs::XFolderPicker >
        createFolderPicker( const com::sun::star::uno::Reference<
                              com::sun::star::uno::XComponentContext >& rServiceManager );

private:

    DECL_STATIC_LINK( Application, PostEventHandler, void* );
};


class VCL_DLLPUBLIC SolarMutexGuard
{
    private:
        SolarMutexGuard( const SolarMutexGuard& );
        const SolarMutexGuard& operator = ( const SolarMutexGuard& );
        ::osl::SolarMutex& m_solarMutex;

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
    osl::SolarMutex& m_solarMutex;
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
    osl::SolarMutex& m_solarMutex;
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

VCL_DLLPUBLIC sal_Bool InitVCL( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & );
VCL_DLLPUBLIC void DeInitVCL();

VCL_DLLPUBLIC bool InitAccessBridge( bool bAllowCancel, bool &rCancelled );

// only allowed to call, if no thread is running. You must call JoinMainLoopThread to free all memory.
VCL_DLLPUBLIC void CreateMainLoopThread( oslWorkerFunction pWorker, void * pThreadData );
VCL_DLLPUBLIC void JoinMainLoopThread();

inline void Application::EndYield()
{
    PostUserEvent( Link() );
}

#endif // _APP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
