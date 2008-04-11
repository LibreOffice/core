/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svdata.hxx,v $
 * $Revision: 1.10 $
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

#ifndef _SV_SVDATA_HXX
#define _SV_SVDATA_HXX

#ifndef _VOS_THREAD_HXX
#include <vos/thread.hxx>
#endif
#include <tools/string.hxx>
#include <tools/gen.hxx>
#include <tools/shl.hxx>
#include <tools/link.hxx>
#include <vcl/vclevent.hxx>
#include <vcl/sv.h>
#include <tools/color.hxx>
#include <tools/debug.hxx>
#include <vcl/dllapi.h>
#include <com/sun/star/uno/Reference.hxx>

namespace com {
namespace sun {
namespace star {
namespace lang {
    class XMultiServiceFactory;
}
namespace frame {
    class XSessionManagerClient;
}
namespace awt {
    class XDisplayConnection;
}
}}}

struct ImplTimerData;
struct ImplFileImageCacheData;
struct ImplConfigData;
class ImplDirectFontSubstitution;
struct ImplHotKey;
struct ImplEventHook;
class Point;
class Rectangle;
class ImpResMgr;
class ResMgr;
class UniqueIndex;
class ImplAccelManager;
class ImplDevFontList;
class ImplFontCache;
class HelpTextWindow;
class ImplTBDragMgr;
class ImplButtonList;
class ImplIdleMgr;
class DbgWindow;
class FloatingWindow;
class AllSettings;
class KeyCode;
class NotifyEvent;
class Timer;
class AutoTimer;
class Help;
class ImageList;
class PopupMenu;
class Application;
class OutputDevice;
class Window;
class SystemWindow;
class WorkWindow;
class Dialog;
class VirtualDevice;
class Printer;
class SalFrame;
class SalInstance;
class SalSystem;
class SalProcessWindowList;
class SalTrayList;
class UniqueIdContainer;
class List;
class ImplPrnQueueList;
class ImplVDevCache;
class UnoWrapperBase;
class GraphicConverter;
class ImplWheelWindow;
class SalTimer;
class SalI18NImeStatus;
class DockingManager;

namespace vos { class OMutex; }
namespace vos { class OCondition; }
namespace vcl { class DisplayConnection; class FontSubstConfiguration; class SettingsConfigItem; class DefaultFontConfiguration; }

// -----------------
// - ImplSVAppData -
// -----------------

struct ImplSVAppData
{
    enum ImeStatusWindowMode
    {
        ImeStatusWindowMode_UNKNOWN,
        ImeStatusWindowMode_HIDE,
        ImeStatusWindowMode_SHOW
    };

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    mxMSF;
    String*                 mpMSFTempFileName;
    AllSettings*            mpSettings;         // Application settings
    VclEventListeners*      mpEventListeners;   // listeners for vcl events (eg, extended toolkit)
    VclEventListeners*      mpKeyListeners;     // listeners for key events only (eg, extended toolkit)
    ImplAccelManager*       mpAccelMgr;         // Accelerator Manager
    XubString*              mpAppName;          // Application name
    XubString*              mpAppFileName;      // Abs. Application FileName
    XubString*              mpDisplayName;      // Application Display Name
    String*                 mpFontPath;         // Additional Fontpath
    Help*                   mpHelp;             // Application help
    PopupMenu*              mpActivePopupMenu;  // Actives Popup-Menu (in Execute)
    UniqueIdContainer*      mpUniqueIdCont;     // Fuer Eindeutige Id's
    ImplIdleMgr*            mpIdleMgr;          // Idle-Manager
    ImplWheelWindow*        mpWheelWindow;      // WheelWindow
    ImplHotKey*             mpFirstHotKey;      // HotKey-Verwaltung
    ImplEventHook*          mpFirstEventHook;   // Event-Hooks
    ULONG                   mnLastInputTime;    // GetLastInputTime()
    USHORT                  mnDispatchLevel;    // DispatchLevel
    USHORT                  mnModalMode;        // ModalMode Count
    USHORT                  mnModalDialog;      // ModalDialog Count
    USHORT                  mnAccessCount;      // AccessHdl Count
    USHORT                  mnSysWinMode;       // Modus, wann SystemWindows erzeugt werden sollen
    USHORT                  mnLayout;           // --- RTL-Flags --- currently not used, only for testing
    short                   mnDialogScaleX;     // Scale X-Positions and sizes in Dialogs
    BOOL                    mbInAppMain;        // is Application::Main() on stack
    BOOL                    mbInAppExecute;     // is Application::Execute() on stack
    BOOL                    mbAppQuit;          // is Application::Quit() called
    BOOL                    mbSettingsInit;     // TRUE: Settings are init
    BOOL                    mbDialogCancel;     // TRUE: Alle Dialog::Execute()-Aufrufe werden mit return FALSE sofort beendet

    /** Controls whether showing any IME status window is toggled on or off.

        Only meaningful if showing IME status windows can be toggled on and off
        externally (see Application::CanToggleImeStatusWindow).
     */
    ImeStatusWindowMode meShowImeStatusWindow;

                            DECL_STATIC_LINK( ImplSVAppData, ImplQuitMsg, void* );

};


// -----------------
// - ImplSVGDIData -
// -----------------

struct ImplSVGDIData
{
    OutputDevice*           mpFirstWinGraphics; // First OutputDevice with a Frame Graphics
    OutputDevice*           mpLastWinGraphics;  // Last OutputDevice with a Frame Graphics
    OutputDevice*           mpFirstVirGraphics; // First OutputDevice with a VirtualDevice Graphics
    OutputDevice*           mpLastVirGraphics;  // Last OutputDevice with a VirtualDevice Graphics
    OutputDevice*           mpFirstPrnGraphics; // First OutputDevice with a InfoPrinter Graphics
    OutputDevice*           mpLastPrnGraphics;  // Last OutputDevice with a InfoPrinter Graphics
    VirtualDevice*          mpFirstVirDev;      // First VirtualDevice
    VirtualDevice*          mpLastVirDev;       // Last VirtualDevice
    Printer*                mpFirstPrinter;     // First Printer
    Printer*                mpLastPrinter;      // Last Printer
    ImplPrnQueueList*       mpPrinterQueueList; // List of all printer queue
    ImplDevFontList*        mpScreenFontList;   // Screen-Font-List
    ImplFontCache*          mpScreenFontCache;  // Screen-Font-Cache
    ImplDirectFontSubstitution* mpDirectFontSubst;// Font-Substitutons defined in Tools->Options->Fonts
    GraphicConverter*       mpGrfConverter;     // Converter for graphics
    long                    mnRealAppFontX;     // AppFont X-Numenator for 40/tel Width
    long                    mnAppFontX;         // AppFont X-Numenator for 40/tel Width + DialogScaleX
    long                    mnAppFontY;         // AppFont Y-Numenator for 80/tel Height
    BOOL                    mbFontSubChanged;   // TRUE: FontSubstitution wurde zwischen Begin/End geaendert
    vcl::DefaultFontConfiguration* mpDefaultFontConfiguration;
    vcl::FontSubstConfiguration* mpFontSubstConfiguration;
    bool                    mbPrinterPullModel; // true: use pull model instead of normal push model when printing
    bool                    mbNativeFontConfig; // true: do not override UI font
};


// -----------------
// - ImplSVWinData -
// -----------------

struct ImplSVWinData
{
    Window*                 mpFirstFrame;       // First FrameWindow
    Window*                 mpDefDialogParent;  // Default Dialog Parent
    WorkWindow*             mpAppWin;           // Application-Window
    Window*                 mpFocusWin;         // window, that has the focus
    Window*                 mpActiveApplicationFrame; // the last active application frame, can be used as DefModalDialogParent if no focuswin set
    Window*                 mpCaptureWin;       // window, that has the mouse capture
    Window*                 mpLastDeacWin;      // Window, that need a deactivate (FloatingWindow-Handling)
    DbgWindow*              mpDbgWin;           // debug window
    FloatingWindow*         mpFirstFloat;       // First FloatingWindow in PopupMode
    Dialog*                 mpLastExecuteDlg;   // Erster Dialog, der sich in Execute befindet
    Window*                 mpExtTextInputWin;  // Window, which is in ExtTextInput
    Window*                 mpTrackWin;         // window, that is in tracking mode
    AutoTimer*              mpTrackTimer;       // tracking timer
    ImageList*              mpMsgBoxImgList;    // ImageList for MessageBoxen
    Window*                 mpAutoScrollWin;    // window, that is in AutoScrollMode mode
    USHORT                  mnTrackFlags;       // tracking flags
    USHORT                  mnAutoScrollFlags;  // auto scroll flags
    BOOL                    mbNoDeactivate;     // TRUE: keine Deactivate durchfuehren
    BOOL                    mbNoSaveFocus;      // TRUE: menues must not save/restore focus
};


// ------------------
// - ImplSVCtrlData -
// ------------------

struct ImplSVCtrlData
{
    ImageList*              mpCheckImgList;     // ImageList for CheckBoxes
    ImageList*              mpRadioImgList;     // ImageList for RadioButtons
    ImageList*              mpPinImgList;       // ImageList for PIN
    ImageList*              mpSplitHPinImgList; // ImageList for Horizontale SplitWindows
    ImageList*              mpSplitVPinImgList; // ImageList for Vertikale SplitWindows (PIN's)
    ImageList*              mpSplitHArwImgList; // ImageList for Horizontale SplitWindows (Arrows)
    ImageList*              mpSplitVArwImgList; // ImageList for Vertikale SplitWindows (Arrows)
    ImplTBDragMgr*          mpTBDragMgr;        // DragMgr for ToolBox
    USHORT                  mnCheckStyle;       // CheckBox-Style for ImageList-Update
    USHORT                  mnRadioStyle;       // Radio-Style for ImageList-Update
    ULONG                   mnLastCheckFColor;  // Letzte FaceColor fuer CheckImage
    ULONG                   mnLastCheckWColor;  // Letzte WindowColor fuer CheckImage
    ULONG                   mnLastCheckWTextColor;  // Letzte WindowTextColor fuer CheckImage
    ULONG                   mnLastCheckLColor;  // Letzte LightColor fuer CheckImage
    ULONG                   mnLastRadioFColor;  // Letzte FaceColor fuer RadioImage
    ULONG                   mnLastRadioWColor;  // Letzte WindowColor fuer RadioImage
    ULONG                   mnLastRadioLColor;  // Letzte LightColor fuer RadioImage
};


// ------------------
// - ImplSVHelpData -
// ------------------

struct ImplSVHelpData
{
    BOOL                    mbContextHelp       : 1;    // is ContextHelp enabled
    BOOL                    mbExtHelp           : 1;    // is ExtendedHelp enabled
    BOOL                    mbExtHelpMode       : 1;    // is in ExtendedHelp Mode
    BOOL                    mbOldBalloonMode    : 1;    // BallonMode, befor ExtHelpMode started
    BOOL                    mbBalloonHelp       : 1;    // is BalloonHelp enabled
    BOOL                    mbQuickHelp         : 1;    // is QuickHelp enabled
    BOOL                    mbSetKeyboardHelp   : 1;    // tiphelp was activated by keyboard
    BOOL                    mbKeyboardHelp      : 1;    // tiphelp was activated by keyboard
    BOOL                    mbAutoHelpId        : 1;    // generate HelpIds
    BOOL                    mbRequestingHelp    : 1;    // In Window::RequestHelp
    HelpTextWindow*         mpHelpWin;                  // HelpWindow
    ULONG                   mnLastHelpHideTime;         // ticks of last show
};

struct ImplSVNWFData
{
    bool                    mbMenuBarDockingAreaCommonBG; // e.g. WinXP default theme
    bool                    mbDockingAreaSeparateTB;      // individual toolbar backgrounds
                                                          // instead of one for docking area
    bool                    mbToolboxDropDownSeparate;    // two adjacent buttons for
                                                          // toolbox dropdown buttons
    int                     mnMenuFormatExtraBorder;      // inner popup menu border
    bool                    mbFlatMenu;                   // no popup 3D border
    Color                   maMenuBarHighlightTextColor;  // override higlight text color
                                                          // in menubar if not transparent
    bool                    mbOpenMenuOnF10;              // on gnome the first menu opens on F10
    bool                    mbNoFocusRects;               // on Aqua focus rects are not used
    bool                    mbNoBoldTabFocus;             // on Aqua and Gnome the focused tab has not bold text
    bool                    mbCenteredTabs;               // on Aqua, tabs are centered
    bool                    mbProgressNeedsErase;         // set true for platforms that should draw the
                                                          // window background before drawing the native
                                                          // progress bar
    bool                    mbCheckBoxNeedsErase;         // set true for platforms that should draw the
                                                          // window background before drawing the native
                                                          // checkbox
    bool                    mbScrollbarJumpPage;          // true for "jump to here" behavior
};


// --------------
// - ImplSVData -
// --------------

struct ImplSVData
{
    void*                   mpSalData;          // SalData
    SalInstance*            mpDefInst;          // Default SalInstance
    Application*            mpApp;              // pApp
    WorkWindow*             mpDefaultWin;       // Default-Window
    BOOL                    mbDeInit;             // Is VCL deinitializing
    ULONG                   mnThreadCount;      // is VCL MultiThread enabled
    ImplConfigData*         mpFirstConfigData;  // Zeiger auf ersten Config-Block
    ImplTimerData*          mpFirstTimerData;   // list of all running timers
    SalTimer*               mpSalTimer;         // interface to sal event loop/timers
    SalI18NImeStatus*       mpImeStatus;        // interface to ime status window
    SalSystem*              mpSalSystem;        // SalSystem interface
    ResMgr*                 mpResMgr;           // SV-Resource-Manager
    ULONG                   mnTimerPeriod;      // current timer period
    ULONG                   mnTimerUpdate;      // TimerCallbackProcs on stack
    BOOL                    mbNotAllTimerCalled;// TRUE: Es muessen noch Timer abgearbeitet werden
    BOOL                    mbNoCallTimer;      // TRUE: No Timeout calls
    ImplSVAppData           maAppData;          // indepen data for class Application
    ImplSVGDIData           maGDIData;          // indepen data for Output classes
    ImplSVWinData           maWinData;          // indepen data for Windows classes
    ImplSVCtrlData          maCtrlData;         // indepen data for Control classes
    ImplSVHelpData          maHelpData;         // indepen data for Help classes
    ImplSVNWFData           maNWFData;
    UnoWrapperBase*         mpUnoWrapper;
    Window*                 mpIntroWindow;      // the splash screen
    DockingManager*         mpDockingManager;

    vos::OThread::TThreadIdentifier                     mnMainThreadId;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XDisplayConnection >     mxDisplayConnection;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxAccessBridge;
    com::sun::star::uno::Reference< com::sun::star::frame::XSessionManagerClient > xSMClient;
    ::vcl::SettingsConfigItem*          mpSettingsConfigItem;
};

void        ImplInitSVData();
void        ImplDeInitSVData();
void        ImplDestroySVData();
Window*     ImplGetDefaultWindow();
VCL_DLLPUBLIC ResMgr*     ImplGetResMgr();
DockingManager*     ImplGetDockingManager();
void        ImplWindowAutoMnemonic( Window* pWindow );

void        ImplUpdateSystemProcessWindow();
Window*     ImplFindWindow( const SalFrame* pFrame, Point& rSalFramePos );

// SVAPP.CXX
BOOL        ImplCallHotKey( const KeyCode& rKeyCode );
void        ImplFreeHotKeyData();
void        ImplFreeEventHookData();

// WINPROC.CXX
long        ImplCallPreNotify( NotifyEvent& rEvt );
long        ImplCallEvent( NotifyEvent& rEvt );

extern VCL_DLLPUBLIC ImplSVData* pImplSVData;
inline VCL_DLLPUBLIC ImplSVData* ImplGetSVData() { return pImplSVData; }
inline ImplSVData* ImplGetAppSVData() { return ImplGetSVData(); }

bool ImplInitAccessBridge( BOOL bAllowCancel, BOOL &rCancelled );

// -----------------------------------------------------------------------

// -----------------
// - ImplSVEmpyStr -
// -----------------

// Empty-SV-String

inline const String& ImplGetSVEmptyStr()
    { return String::EmptyString(); }
inline const ByteString& ImplGetSVEmptyByteStr()
    { return ByteString::EmptyString(); }

// -----------------------------------------------------------------------

// ----------------------
// - struct ImplDelData -
// ----------------------
// ImplDelData is used as a "dog tag" by a window when it
// does something that could indirectly destroy the window
// TODO: wild destruction of a window should not be possible

struct ImplDelData
{
    ImplDelData*    mpNext;
    const Window*   mpWindow;
    BOOL            mbDel;

                    ImplDelData( const Window* pWindow = NULL )
                    : mpNext( NULL ), mpWindow( NULL ), mbDel( FALSE )
                    { if( pWindow ) AttachToWindow( pWindow ); }

    virtual         ~ImplDelData();

    bool            IsDead() const
    {
        DBG_ASSERT( mbDel == FALSE, "object deleted while in use !" );
        return (mbDel!=FALSE);
    }
    BOOL /*deprecated */IsDelete() const { return (BOOL)IsDead(); }

private:
    void            AttachToWindow( const Window* );
};

// ---------------
// - ImplSVEvent -
// ---------------

struct ImplSVEvent
{
    ULONG               mnEvent;
    void*               mpData;
    Link*               mpLink;
    Window*             mpWindow;
    ImplDelData         maDelData;
    BOOL                mbCall;
};

#endif  // _SV_SVDATA_HXX
