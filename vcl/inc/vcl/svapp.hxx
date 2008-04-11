/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svapp.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _VOS_THREAD_HXX
#include <vos/thread.hxx>
#endif
#include <tools/string.hxx>
#include <tools/link.hxx>
#include <tools/unqid.hxx>
#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/apptypes.hxx>
#ifndef _VCL_SETTINGS_HXX
#include <vcl/settings.hxx>
#endif
#include <vcl/vclevent.hxx>
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

namespace vos { class IMutex; }
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/connection/XConnection.hpp>

namespace com {
namespace sun {
namespace star {
namespace lang {
    class XMultiServiceFactory;
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

#define SYSTEMWINDOW_MODE_NOAUTOMODE    ((USHORT)0x0001)
#define SYSTEMWINDOW_MODE_DIALOG        ((USHORT)0x0002)

// -------------
// - EventHook -
// -------------

typedef long (*VCLEventHookProc)( NotifyEvent& rEvt, void* pData );

// --------------------
// - ApplicationEvent -
// --------------------

// Erstmal wieder eingebaut, damit AppEvents auf dem MAC funktionieren
#ifdef UNX
// enum Doppelt in daemon.cxx unter unix Achtung !!!
enum Service { SERVICE_OLE, SERVICE_APPEVENT, SERVICE_IPC };
#endif

class VCL_DLLPUBLIC ApplicationAddress
{
friend class Application;
protected:
    UniString           aHostName;
    UniString           aDisplayName;
    UniString           aDomainName;
    int                 nPID;

public:
                        ApplicationAddress();
                        ApplicationAddress( const UniString& rDomain );
                        ApplicationAddress( const UniString& rHost,
                                            const UniString& rDisp,
                                            const UniString& rDomain );
                        ApplicationAddress( const UniString& rHost, int nPID );

    const UniString&    GetHost() const     { return aHostName; }
    const UniString&    GetDisplay() const  { return aDisplayName; }
    const UniString&    GetDomain() const   { return aDomainName; }
    int                 GetPID() const      { return nPID; }

    BOOL                IsConnectToSame( const ApplicationAddress& rAdr ) const;
};

inline ApplicationAddress::ApplicationAddress()
{
    nPID = 0;
}

inline ApplicationAddress::ApplicationAddress( const UniString& rDomain )
{
    aDomainName     = rDomain;
    nPID            = 0;
}

inline ApplicationAddress::ApplicationAddress( const UniString& rHost,
                                               const UniString& rDisp,
                                               const UniString& rDomain )
{
    aHostName       = rHost;
    aDisplayName    = rDisp;
    aDomainName     = rDomain;
    nPID            = 0;
}

inline ApplicationAddress::ApplicationAddress( const UniString& rHost, int nPIDPar )
{
    aHostName       = rHost;
    nPID            = nPIDPar;
}

inline BOOL ApplicationAddress::IsConnectToSame( const ApplicationAddress& rAdr ) const
{
    if ( nPID && ((nPID == rAdr.nPID) && (aHostName.Equals( rAdr.aHostName))) )
        return TRUE;
    else
        return FALSE;
}

#define APPEVENT_PARAM_DELIMITER        '\n'

#define APPEVENT_OPEN_STRING            "Open"
#define APPEVENT_PRINT_STRING           "Print"
#define APPEVENT_DISKINSERT_STRING      "DiskInsert"
#define APPEVENT_SAVEDOCUMENTS_STRING   "SaveDocuments"

class VCL_DLLPUBLIC ApplicationEvent
{
private:
    UniString           aSenderAppName; // Absender Applikationsname
    ByteString          aEvent;         // Event
    UniString           aData;          // Uebertragene Daten
    ApplicationAddress  aAppAddr;       // Absender Addresse

public:
                        ApplicationEvent() {}
                        ApplicationEvent( const UniString& rSenderAppName,
                                          const ApplicationAddress& rAppAddr,
                                          const ByteString& rEvent,
                                          const UniString& rData );

    const UniString&    GetSenderAppName() const { return aSenderAppName; }
    const ByteString&   GetEvent() const { return aEvent; }
    const UniString&    GetData() const { return aData; }
    const ApplicationAddress& GetAppAddress() const { return aAppAddr; }

    BOOL                IsOpenEvent() const;
    BOOL                IsPrintEvent() const;
    BOOL                IsDiskInsertEvent() const;

    USHORT              GetParamCount() const { return aData.GetTokenCount( APPEVENT_PARAM_DELIMITER ); }
    UniString           GetParam( USHORT nParam ) const { return aData.GetToken( nParam, APPEVENT_PARAM_DELIMITER ); }
};

inline ApplicationEvent::ApplicationEvent( const UniString& rSenderAppName,
                                           const ApplicationAddress& rAppAddr,
                                           const ByteString& rEvent,
                                           const UniString& rData ) :
    aSenderAppName( rSenderAppName ),
    aEvent( rEvent ),
    aData( rData ),
    aAppAddr( rAppAddr )
{
}

inline BOOL ApplicationEvent::IsOpenEvent() const
{
    if ( aEvent.Equals( APPEVENT_OPEN_STRING ))
        return TRUE;
    else
        return FALSE;
}

inline BOOL ApplicationEvent::IsPrintEvent() const
{
    if ( aEvent.Equals( APPEVENT_PRINT_STRING ))
        return TRUE;
    else
        return FALSE;
}

inline BOOL ApplicationEvent::IsDiskInsertEvent() const
{
    if ( aEvent.Equals( APPEVENT_DISKINSERT_STRING ))
        return TRUE;
    else
        return FALSE;
}

class VCL_DLLPUBLIC PropertyHandler
{
public:
    virtual void                Property( ApplicationProperty& ) = 0;
};

// ---------------
// - Application -
// ---------------

class VCL_DLLPUBLIC Application
{
public:
                                Application();
    virtual                     ~Application();

    virtual void                Main() = 0;

    virtual BOOL                QueryExit();

    virtual void                UserEvent( ULONG nEvent, void* pEventData );

    virtual void                ActivateExtHelp();
    virtual void                DeactivateExtHelp();

    virtual void                ShowStatusText( const XubString& rText );
    virtual void                HideStatusText();

    virtual void                ShowHelpStatusText( const XubString& rText );
    virtual void                HideHelpStatusText();

    virtual void                FocusChanged();
    virtual void                DataChanged( const DataChangedEvent& rDCEvt );

    virtual void                Init();
    virtual void                DeInit();

    static void                 InitAppRes( const ResId& rResId );

    static USHORT               GetCommandLineParamCount();
    static XubString            GetCommandLineParam( USHORT nParam );
    static const XubString&     GetAppFileName();

    virtual USHORT              Exception( USHORT nError );
    static void                 Abort( const XubString& rErrorText );

    static void                 Execute();
    static void                 Quit();
    static void                 Reschedule( bool bAllEvents = false );
    static void                 Yield( bool bAllEvents = false );
    static void                 EndYield();
    static vos::IMutex&                     GetSolarMutex();
    static vos::OThread::TThreadIdentifier  GetMainThreadIdentifier();
    static ULONG                ReleaseSolarMutex();
    static void                 AcquireSolarMutex( ULONG nCount );

    static BOOL                 IsInMain();
    static BOOL                 IsInExecute();
    static BOOL                 IsShutDown();
    static BOOL                 IsInModalMode();
    static USHORT               GetModalModeCount();

    static USHORT               GetDispatchLevel();
    static BOOL                 AnyInput( USHORT nType = INPUT_ANY );
    static ULONG                GetLastInputInterval();
    static BOOL                 IsUICaptured();
    static BOOL                 IsUserActive( USHORT nTest = USERACTIVE_ALL );

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
    static void                 ImplCallEventListeners( ULONG nEvent, Window* pWin, void* pData );
    static void                 ImplCallEventListeners( VclSimpleEvent* pEvent );
    static BOOL                 HandleKey( ULONG nEvent, Window *pWin, KeyEvent* pKeyEvent );

    static ULONG                PostKeyEvent( ULONG nEvent, Window *pWin, KeyEvent* pKeyEvent );
    static ULONG                PostMouseEvent( ULONG nEvent, Window *pWin, MouseEvent* pMouseEvent );
    static void                 RemoveMouseAndKeyEvents( Window *pWin );
    static BOOL                 IsProcessedMouseOrKeyEvent( ULONG nEventId );

    static ULONG                PostUserEvent( ULONG nEvent, void* pEventData = NULL );
    static ULONG                PostUserEvent( const Link& rLink, void* pCaller = NULL );
    static BOOL                 PostUserEvent( ULONG& rEventId, ULONG nEvent, void* pEventData = NULL );
    static BOOL                 PostUserEvent( ULONG& rEventId, const Link& rLink, void* pCaller = NULL );
    static void                 RemoveUserEvent( ULONG nUserEvent );

    static BOOL                 InsertIdleHdl( const Link& rLink, USHORT nPriority );
    static void                 RemoveIdleHdl( const Link& rLink );

    virtual void                AppEvent( const ApplicationEvent& rAppEvent );

    virtual void                Property( ApplicationProperty& );
    void                        SetPropertyHandler( PropertyHandler* pHandler );

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

    static void                 SetDisplayName( const UniString& rDisplayName );
    static UniString            GetDisplayName();

    static unsigned int         GetScreenCount();
    // IsMultiDisplay returns:
    //        true:  different screens are separate and windows cannot be moved
    //               between them (e.g. Xserver with multiple screens)
    //        false: screens form up one large display area
    //               windows can be moved between single screens
    //               (e.g. Xserver with Xinerama, Windows)
    static bool                 IsMultiDisplay();
    static Rectangle            GetScreenPosSizePixel( unsigned int nScreen );
    static Rectangle            GetWorkAreaPosSizePixel( unsigned int nScreen );
    static rtl::OUString        GetScreenName( unsigned int nScreen );
    static unsigned int         GetDefaultDisplayNumber();
    // if IsMultiDisplay() == false the return value will be
    // nearest screen of the target rectangle
    // in case of IsMultiDisplay() == true the return value
    // will always be GetDefaultDisplayNumber()
    static unsigned int         GetBestScreen( const Rectangle& );

    static const LocaleDataWrapper& GetAppLocaleDataWrapper();

    static BOOL                 InsertAccel( Accelerator* pAccel );
    static void                 RemoveAccel( Accelerator* pAccel );
    static void                 FlushAccel();
    static BOOL                 CallAccel( const KeyCode& rKeyCode, USHORT nRepeat = 0 );

    static ULONG                AddHotKey( const KeyCode& rKeyCode, const Link& rLink, void* pData = NULL );
    static void                 RemoveHotKey( ULONG nId );
    static ULONG                AddEventHook( VCLEventHookProc pProc, void* pData = NULL );
    static void                 RemoveEventHook( ULONG nId );
    static long                 CallEventHooks( NotifyEvent& rEvt );
    static long                 CallPreNotify( NotifyEvent& rEvt );
    static long                 CallEvent( NotifyEvent& rEvt );

    static void                 SetHelp( Help* pHelp = NULL );
    static Help*                GetHelp();

    static void                 EnableAutoHelpId( BOOL bEnabled = TRUE );
    static BOOL                 IsAutoHelpIdEnabled();

    static void                 EnableAutoMnemonic( BOOL bEnabled = TRUE );
    static BOOL                 IsAutoMnemonicEnabled();

    static ULONG                GetReservedKeyCodeCount();
    static const KeyCode*       GetReservedKeyCode( ULONG i );
    static String               GetReservedKeyCodeDescription( ULONG i );

    static void                 SetDefDialogParent( Window* pWindow );
    static Window*              GetDefDialogParent();

    static void                 EnableDialogCancel( BOOL bDialogCancel = TRUE );
    static BOOL                 IsDialogCancelEnabled();

    static void                 SetSystemWindowMode( USHORT nMode );
    static USHORT               GetSystemWindowMode();

    static void                 SetDialogScaleX( short nScale );
    static short                GetDialogScaleX();

    static void                 SetFontPath( const String& rPath );
    static const String&        GetFontPath();

    static UniqueItemId         CreateUniqueId();

    static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDisplayConnection > GetDisplayConnection();

    // The global service manager has to be created before!
    static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > GetVCLToolkit();
    static UnoWrapperBase*      GetUnoWrapper( BOOL bCreateIfNotExists = TRUE );
    static void                 SetUnoWrapper( UnoWrapperBase* pWrapper );

    static void                 SetFilterHdl( const Link& rLink );
    static const Link&          GetFilterHdl();

    static BOOL                 IsAccessibilityEnabled();

    static void                 EnableHeadlessMode( BOOL bEnable = TRUE );
    static BOOL                 IsHeadlessModeEnabled();

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

private:

    DECL_STATIC_LINK( Application, PostEventHandler, void* );
};

VCL_DLLPUBLIC Application* GetpApp();

VCL_DLLPUBLIC BOOL InitVCL( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & );
VCL_DLLPUBLIC void DeInitVCL();

VCL_DLLPUBLIC BOOL InitAccessBridge( BOOL bAllowCancel, BOOL &rCancelled );

// only allowed to call, if no thread is running. You must call JoinMainLoopThread to free all memory.
VCL_DLLPUBLIC void CreateMainLoopThread( oslWorkerFunction pWorker, void * pThreadData );
VCL_DLLPUBLIC void JoinMainLoopThread();

inline void Application::EndYield()
{
    PostUserEvent( Link() );
}

#endif // _APP_HXX
