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

#ifndef _SV_SVDATA_HXX
#define _SV_SVDATA_HXX

#include "sal/types.h"

#include <osl/thread.hxx>
#include <rtl/ref.hxx>

#include "tools/shl.hxx"
#include "tools/link.hxx"
#include "tools/fldunit.hxx"
#include "tools/string.hxx"
#include "tools/color.hxx"
#include "tools/debug.hxx"

#include "vcl/vclevent.hxx"
#include "tools/solar.h"
#include "vcl/svapp.hxx"
#include "vcl/dllapi.h"

#include "unotools/options.hxx"

#include "xconnection.hxx"

#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/uno/Reference.hxx"

#include <boost/unordered_map.hpp>

#include <config_version.h>

struct ImplTimerData;
struct ImplConfigData;
class ImplDirectFontSubstitution;
struct ImplHotKey;
struct ImplEventHook;
class Point;
class ResMgr;
class ImplAccelManager;
class ImplDevFontList;
class ImplFontCache;
class HelpTextWindow;
class ImplTBDragMgr;
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
class Image;
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
class ImplPrnQueueList;
class UnoWrapperBase;
class GraphicConverter;
class ImplWheelWindow;
class SalTimer;
class SalI18NImeStatus;
class DockingManager;
class VclEventListeners2;
class SalData;

namespace vcl { class DisplayConnection; class SettingsConfigItem; class DeleteOnDeinitBase; }
namespace utl { class DefaultFontConfiguration; class FontSubstConfiguration; }

class LocaleConfigurationListener : public utl::ConfigurationListener
{
public:
    virtual void ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 );
};

struct ImplSVAppData
{
    enum ImeStatusWindowMode
    {
        ImeStatusWindowMode_UNKNOWN,
        ImeStatusWindowMode_HIDE,
        ImeStatusWindowMode_SHOW
    };

    AllSettings*            mpSettings;         // Application settings
    LocaleConfigurationListener* mpCfgListener;
    VclEventListeners*      mpEventListeners;   // listeners for vcl events (eg, extended toolkit)
    VclEventListeners*      mpKeyListeners;     // listeners for key events only (eg, extended toolkit)
    ImplAccelManager*       mpAccelMgr;         // Accelerator Manager
    OUString*               mpAppName;          // Application name
    OUString*               mpAppFileName;      // Abs. Application FileName
    OUString*               mpDisplayName;      // Application Display Name
    OUString*               mpFontPath;         // Additional Fontpath
    Help*                   mpHelp;             // Application help
    PopupMenu*              mpActivePopupMenu;  // Actives Popup-Menu (in Execute)
    ImplIdleMgr*            mpIdleMgr;          // Idle-Manager
    ImplWheelWindow*        mpWheelWindow;      // WheelWindow
    ImplHotKey*             mpFirstHotKey;      // HotKey-Verwaltung
    ImplEventHook*          mpFirstEventHook;   // Event-Hooks
    VclEventListeners2*     mpPostYieldListeners;           // post yield listeners
    sal_uLong               mnLastInputTime;                // GetLastInputTime()
    sal_uInt16              mnDispatchLevel;                // DispatchLevel
    sal_uInt16              mnModalMode;                    // ModalMode Count
    sal_uInt16              mnModalDialog;                  // ModalDialog Count
    sal_uInt16              mnAccessCount;                  // AccessHdl Count
    sal_uInt16              mnSysWinMode;                   // Mode, when SystemWindows should be created
    sal_uInt16              mnLayout;                       // --- RTL-Flags --- currently not used, only for testing
    short                   mnDialogScaleX;                 // Scale X-Positions and sizes in Dialogs
    sal_Bool                mbInAppMain;                    // is Application::Main() on stack
    sal_Bool                mbInAppExecute;                 // is Application::Execute() on stack
    sal_Bool                mbAppQuit;                      // is Application::Quit() called
    sal_Bool                mbSettingsInit;                 // sal_True: Settings are initialized
    sal_Bool                mbNoYield;                      // Application::Yield will not wait for events if the queue is empty
                                                            // essentially that makes it the same as Application::Reschedule
    Application::DialogCancelMode meDialogCancel;           // sal_True: All Dialog::Execute() calls will be terminated immediately with return sal_False
    long                    mnDefaultLayoutBorder;          // default value in pixel for layout distances used
                                                            // in window arrangers

    /** Controls whether showing any IME status window is toggled on or off.

        Only meaningful if showing IME status windows can be toggled on and off
        externally (see Application::CanToggleImeStatusWindow).
     */
    ImeStatusWindowMode meShowImeStatusWindow;

    DECL_STATIC_LINK( ImplSVAppData, ImplQuitMsg, void* );
};


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
    sal_Bool                    mbFontSubChanged;   // sal_True: FontSubstitution was changed between Begin/End
    utl::DefaultFontConfiguration* mpDefaultFontConfiguration;
    utl::FontSubstConfiguration* mpFontSubstConfiguration;
    bool                    mbNativeFontConfig; // true: do not override UI font
    bool                    mbNoXORClipping;    // true: do not use XOR to achieve clipping effects
};


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
    Dialog*                 mpLastExecuteDlg;   // First Dialog that is in Execute
    Window*                 mpExtTextInputWin;  // Window, which is in ExtTextInput
    Window*                 mpTrackWin;         // window, that is in tracking mode
    AutoTimer*              mpTrackTimer;       // tracking timer
    ImageList*              mpMsgBoxImgList;    // ImageList for MessageBox
    Window*                 mpAutoScrollWin;    // window, that is in AutoScrollMode mode
    sal_uInt16              mnTrackFlags;       // tracking flags
    sal_uInt16              mnAutoScrollFlags;  // auto scroll flags
    sal_Bool                mbNoDeactivate;     // sal_True: do not execute Deactivate
    sal_Bool                mbNoSaveFocus;      // sal_True: menus must not save/restore focus
    sal_Bool                mbNoSaveBackground; // sal_True: save background is unnecessary or even less performant
};


typedef std::vector< std::pair< OUString, FieldUnit > > FieldUnitStringList;

struct ImplSVCtrlData
{
    ImageList*              mpCheckImgList;     // ImageList for CheckBoxes
    ImageList*              mpRadioImgList;     // ImageList for RadioButtons
    ImageList*              mpPinImgList;       // ImageList for PIN
    ImageList*              mpSplitHPinImgList; // ImageList for Horizontale SplitWindows
    ImageList*              mpSplitVPinImgList; // ImageList for Vertikale SplitWindows (PIN's)
    ImageList*              mpSplitHArwImgList; // ImageList for Horizontale SplitWindows (Arrows)
    ImageList*              mpSplitVArwImgList; // ImageList for Vertikale SplitWindows (Arrows)
    Image*                  mpDisclosurePlus;
    Image*                  mpDisclosureMinus;
    ImplTBDragMgr*          mpTBDragMgr;        // DragMgr for ToolBox
    sal_uInt16                  mnCheckStyle;       // CheckBox-Style for ImageList-Update
    sal_uInt16                  mnRadioStyle;       // Radio-Style for ImageList-Update
    sal_uLong                   mnLastCheckFColor;  // Letzte FaceColor fuer CheckImage
    sal_uLong                   mnLastCheckWColor;  // Letzte WindowColor fuer CheckImage
    sal_uLong                   mnLastCheckWTextColor;  // Letzte WindowTextColor fuer CheckImage
    sal_uLong                   mnLastCheckLColor;  // Letzte LightColor fuer CheckImage
    sal_uLong                   mnLastRadioFColor;  // Letzte FaceColor fuer RadioImage
    sal_uLong                   mnLastRadioWColor;  // Letzte WindowColor fuer RadioImage
    sal_uLong                   mnLastRadioLColor;  // Letzte LightColor fuer RadioImage
    FieldUnitStringList*    mpFieldUnitStrings; // list with field units
    FieldUnitStringList*    mpCleanUnitStrings; // same list but with some "fluff" like spaces removed
};


struct ImplSVHelpData
{
    sal_Bool                    mbContextHelp       : 1;    // is ContextHelp enabled
    sal_Bool                    mbExtHelp           : 1;    // is ExtendedHelp enabled
    sal_Bool                    mbExtHelpMode       : 1;    // is in ExtendedHelp Mode
    sal_Bool                    mbOldBalloonMode    : 1;    // BallonMode, befor ExtHelpMode started
    sal_Bool                    mbBalloonHelp       : 1;    // is BalloonHelp enabled
    sal_Bool                    mbQuickHelp         : 1;    // is QuickHelp enabled
    sal_Bool                    mbSetKeyboardHelp   : 1;    // tiphelp was activated by keyboard
    sal_Bool                    mbKeyboardHelp      : 1;    // tiphelp was activated by keyboard
    sal_Bool                    mbAutoHelpId        : 1;    // generate HelpIds
    sal_Bool                    mbRequestingHelp    : 1;    // In Window::RequestHelp
    HelpTextWindow*         mpHelpWin;                  // HelpWindow
    sal_uLong                   mnLastHelpHideTime;         // ticks of last show
};

struct ImplSVNWFData
{
    int                     mnStatusBarLowerRightOffset;    // amount in pixel to avoid in the lower righthand corner
    int                     mnMenuFormatBorderX;            // horizontal inner popup menu border
    int                     mnMenuFormatBorderY;            // vertical inner popup menu border
    ::Color                 maMenuBarHighlightTextColor;    // override higlight text color
                                                            // in menubar if not transparent
    bool                    mbMenuBarDockingAreaCommonBG:1; // e.g. WinXP default theme
    bool                    mbDockingAreaSeparateTB:1;      // individual toolbar backgrounds
                                                            // instead of one for docking area
    bool                    mbDockingAreaAvoidTBFrames:1;   ///< don't draw frames around the individual toolbars if mbDockingAreaSeparateTB is false
    bool                    mbToolboxDropDownSeparate:1;    // two adjacent buttons for
                                                            // toolbox dropdown buttons
    bool                    mbFlatMenu:1;                   // no popup 3D border
    bool                    mbOpenMenuOnF10:1;              // on gnome the first menu opens on F10
    bool                    mbNoFocusRects:1;               // on Aqua focus rects are not used
    bool                    mbCenteredTabs:1;               // on Aqua, tabs are centered
    bool                    mbNoActiveTabTextRaise:1;       // on Aqua the text for the selected tab
                                                            // should not "jump up" a pixel
    bool                    mbProgressNeedsErase:1;         // set true for platforms that should draw the
                                                            // window background before drawing the native
                                                            // progress bar
    bool                    mbCheckBoxNeedsErase:1;         // set true for platforms that should draw the
                                                            // window background before drawing the native
                                                            // checkbox
    bool                    mbCanDrawWidgetAnySize:1;       // set to true currently on gtk

    /// entire drop down listbox resembles a button, no textarea/button parts (as currently on Windows)
    bool                    mbDDListBoxNoTextArea:1;
};

struct BlendFrameCache
{
    Size m_aLastSize;
    sal_uInt8 m_nLastAlpha;
    Color m_aLastColorTopLeft;
    Color m_aLastColorTopRight;
    Color m_aLastColorBottomRight;
    Color m_aLastColorBottomLeft;
    BitmapEx m_aLastResult;

    BlendFrameCache()
        : m_aLastSize(0, 0)
        , m_nLastAlpha(0)
        , m_aLastColorTopLeft(COL_BLACK)
        , m_aLastColorTopRight(COL_BLACK)
        , m_aLastColorBottomRight(COL_BLACK)
        , m_aLastColorBottomLeft(COL_BLACK)
    {
    }
};

struct ImplSVData
{
    SalData*                mpSalData;
    SalInstance*            mpDefInst;          // Default SalInstance
    Application*            mpApp;              // pApp
    WorkWindow*             mpDefaultWin;       // Default-Window
    sal_Bool                    mbDeInit;             // Is VCL deinitializing
    sal_uLong                   mnThreadCount;      // is VCL MultiThread enabled
    ImplConfigData*         mpFirstConfigData;  // Zeiger auf ersten Config-Block
    ImplTimerData*          mpFirstTimerData;   // list of all running timers
    SalTimer*               mpSalTimer;         // interface to sal event loop/timers
    SalI18NImeStatus*       mpImeStatus;        // interface to ime status window
    SalSystem*              mpSalSystem;        // SalSystem interface
    ResMgr*                 mpResMgr;           // SV-Resource-Manager
    sal_uLong                   mnTimerPeriod;      // current timer period
    sal_uLong                   mnTimerUpdate;      // TimerCallbackProcs on stack
    sal_Bool                    mbNotAllTimerCalled;// sal_True: Es muessen noch Timer abgearbeitet werden
    sal_Bool                    mbNoCallTimer;      // sal_True: No Timeout calls
    ImplSVAppData           maAppData;          // indepen data for class Application
    ImplSVGDIData           maGDIData;          // indepen data for Output classes
    ImplSVWinData           maWinData;          // indepen data for Windows classes
    ImplSVCtrlData          maCtrlData;         // indepen data for Control classes
    ImplSVHelpData          maHelpData;         // indepen data for Help classes
    ImplSVNWFData           maNWFData;
    UnoWrapperBase*         mpUnoWrapper;
    Window*                 mpIntroWindow;      // the splash screen
    DockingManager*         mpDockingManager;
    BlendFrameCache*        mpBlendFrameCache;
    sal_Bool                mbIsTestTool;

    oslThreadIdentifier                     mnMainThreadId;
    rtl::Reference< vcl::DisplayConnection >            mxDisplayConnection;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > mxAccessBridge;
    ::vcl::SettingsConfigItem*          mpSettingsConfigItem;
    std::list< vcl::DeleteOnDeinitBase* >*   mpDeinitDeleteList;
    boost::unordered_map< int, OUString >*     mpPaperNames;
};

void        ImplInitSVData();
void        ImplDeInitSVData();
void        ImplDestroySVData();
Window*     ImplGetDefaultWindow();
VCL_PLUGIN_PUBLIC ResMgr*     ImplGetResMgr();
VCL_PLUGIN_PUBLIC ResId VclResId( sal_Int32 nId ); // throws std::bad_alloc if no res mgr
DockingManager*     ImplGetDockingManager();
BlendFrameCache*    ImplGetBlendFrameCache();
void        ImplWindowAutoMnemonic( Window* pWindow );

void        ImplUpdateSystemProcessWindow();
Window*     ImplFindWindow( const SalFrame* pFrame, Point& rSalFramePos );

bool        ImplCallHotKey( const KeyCode& rKeyCode );
void        ImplFreeHotKeyData();
void        ImplFreeEventHookData();

long        ImplCallPreNotify( NotifyEvent& rEvt );
long        ImplCallEvent( NotifyEvent& rEvt );

extern VCL_PLUGIN_PUBLIC ImplSVData* pImplSVData;
inline VCL_PLUGIN_PUBLIC ImplSVData* ImplGetSVData() { return pImplSVData; }
VCL_PLUGIN_PUBLIC void ImplHideSplash();

bool ImplInitAccessBridge( bool bAllowCancel, bool &rCancelled );

FieldUnitStringList* ImplGetFieldUnits();
FieldUnitStringList* ImplGetCleanedFieldUnits();

// ImplDelData is used as a "dog tag" by a window when it
// does something that could indirectly destroy the window
// TODO: wild destruction of a window should not be possible

struct ImplDelData
{
    ImplDelData*    mpNext;
    const Window*   mpWindow;
    sal_Bool            mbDel;

                    ImplDelData( const Window* pWindow = NULL )
                    : mpNext( NULL ), mpWindow( NULL ), mbDel( sal_False )
                    { if( pWindow ) AttachToWindow( pWindow ); }

    virtual         ~ImplDelData();

    bool            IsDead() const
    {
        DBG_ASSERT( mbDel == sal_False, "object deleted while in use !" );
        return (mbDel!=sal_False);
    }

private:
    void            AttachToWindow( const Window* );
};

struct ImplSVEvent
{
    sal_uLong               mnEvent;
    void*               mpData;
    Link*               mpLink;
    Window*             mpWindow;
    ImplDelData         maDelData;
    sal_Bool                mbCall;
};

#endif  // _SV_SVDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
