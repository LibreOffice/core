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

#ifndef INCLUDED_VCL_INC_SVDATA_HXX
#define INCLUDED_VCL_INC_SVDATA_HXX

#include "sal/types.h"

#include <osl/thread.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include "tools/link.hxx"
#include "tools/fldunit.hxx"
#include "tools/color.hxx"
#include "tools/debug.hxx"
#include "tools/solar.h"
#include "vcl/bitmapex.hxx"
#include "vcl/idle.hxx"
#include "vcl/dllapi.h"
#include "vcl/keycod.hxx"
#include "vcl/svapp.hxx"
#include "vcl/vclevent.hxx"
#include "vcleventlisteners.hxx"

#include "unotools/options.hxx"

#include "xconnection.hxx"

#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/uno/Reference.hxx"

#include <unordered_map>

#include <config_version.h>

struct ImplTimerData;
struct ImplIdleData;
struct ImplConfigData;
class ImplDirectFontSubstitution;
struct ImplHotKey;
struct ImplEventHook;
class Point;
class ResMgr;
class ImplAccelManager;
class PhysicalFontCollection;
class ImplFontCache;
class HelpTextWindow;
class ImplTBDragMgr;
class ImplIdleMgr;
class FloatingWindow;
class AllSettings;
class NotifyEvent;
class Timer;
class AutoTimer;
class Idle;
class Help;
class ImageList;
class Image;
class PopupMenu;
class Application;
class OutputDevice;
namespace vcl { class Window; }
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
class OpenGLContext;

namespace vcl { class DisplayConnection; class SettingsConfigItem; class DeleteOnDeinitBase; }

class LocaleConfigurationListener : public utl::ConfigurationListener
{
public:
    virtual void ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 ) override;
};

typedef std::vector<Link<VclWindowEvent&,bool> > SVAppKeyListeners;

struct SVAppPostYieldListeners : public vcl::DeletionNotifier
{
    std::vector<Link<LinkParamNone*,void>>   m_aListeners;
};

struct ImplSVAppData
{
    enum ImeStatusWindowMode
    {
        ImeStatusWindowMode_UNKNOWN,
        ImeStatusWindowMode_HIDE,
        ImeStatusWindowMode_SHOW
    };

    AllSettings*            mpSettings;                     // Application settings
    LocaleConfigurationListener* mpCfgListener;
    VclEventListeners*      mpEventListeners;               // listeners for vcl events (eg, extended toolkit)
    SVAppKeyListeners*      mpKeyListeners;                 // listeners for key events only (eg, extended toolkit)
    ImplAccelManager*       mpAccelMgr;                     // Accelerator Manager
    OUString*               mpAppName;                      // Application name
    OUString*               mpAppFileName;                  // Abs. Application FileName
    OUString*               mpDisplayName;                  // Application Display Name
    OUString*               mpFontPath;                     // Additional Fontpath
    Help*                   mpHelp;                         // Application help
    PopupMenu*              mpActivePopupMenu;              // Actives Popup-Menu (in Execute)
    ImplIdleMgr*            mpIdleMgr;                      // Idle-Manager
    VclPtr<ImplWheelWindow> mpWheelWindow;                  // WheelWindow
    ImplHotKey*             mpFirstHotKey;                  // HotKey-Verwaltung
    ImplEventHook*          mpFirstEventHook;               // Event-Hooks
    SVAppPostYieldListeners* mpPostYieldListeners;           // post yield listeners
    sal_uInt64              mnLastInputTime;                // GetLastInputTime()
    sal_uInt16              mnDispatchLevel;                // DispatchLevel
    sal_uInt16              mnModalMode;                    // ModalMode Count
    sal_uInt16              mnModalDialog;                  // ModalDialog Count
    sal_uInt16              mnAccessCount;                  // AccessHdl Count
    SystemWindowFlags       mnSysWinMode;                   // Mode, when SystemWindows should be created
    short                   mnDialogScaleX;                 // Scale X-Positions and sizes in Dialogs
    bool                    mbInAppMain;                    // is Application::Main() on stack
    bool                    mbInAppExecute;                 // is Application::Execute() on stack
    bool                    mbAppQuit;                      // is Application::Quit() called
    bool                    mbSettingsInit;                 // true: Settings are initialized
    bool                    mbNoYield;                      // Application::Yield will not wait for events if the queue is empty
                                                            // essentially that makes it the same as Application::Reschedule
    Application::DialogCancelMode meDialogCancel;           // true: All Dialog::Execute() calls will be terminated immediately with return false

    /** Controls whether showing any IME status window is toggled on or off.

        Only meaningful if showing IME status windows can be toggled on and off
        externally (see Application::CanToggleImeStatusWindow).
     */
    ImeStatusWindowMode meShowImeStatusWindow;

    SvFileStream*       mpEventTestInput;
    Idle*               mpEventTestingIdle;
    int                 mnEventTestLimit;

    DECL_STATIC_LINK_TYPED(ImplSVAppData, ImplQuitMsg, void*, void);
    DECL_STATIC_LINK_TYPED(ImplSVAppData, ImplPrepareExitMsg, void*, void);
    DECL_STATIC_LINK_TYPED(ImplSVAppData, ImplEndAllDialogsMsg, void*, void);
    DECL_STATIC_LINK_TYPED(ImplSVAppData, ImplEndAllPopupsMsg, void*, void);
    DECL_STATIC_LINK_TYPED(ImplSVAppData, ImplVclEventTestingHdl, void*, void);
    DECL_LINK_TYPED(VclEventTestingHdl, Idle*, void);
};

struct ImplSVGDIData
{
    ~ImplSVGDIData();

    VclPtr<OutputDevice>    mpFirstWinGraphics;             // First OutputDevice with a Frame Graphics
    VclPtr<OutputDevice>    mpLastWinGraphics;              // Last OutputDevice with a Frame Graphics
    VclPtr<OutputDevice>    mpFirstVirGraphics;             // First OutputDevice with a VirtualDevice Graphics
    VclPtr<OutputDevice>    mpLastVirGraphics;              // Last OutputDevice with a VirtualDevice Graphics
    VclPtr<OutputDevice>    mpFirstPrnGraphics;             // First OutputDevice with a InfoPrinter Graphics
    VclPtr<OutputDevice>    mpLastPrnGraphics;              // Last OutputDevice with a InfoPrinter Graphics
    VclPtr<VirtualDevice>   mpFirstVirDev;                  // First VirtualDevice
    VclPtr<VirtualDevice>   mpLastVirDev;                   // Last VirtualDevice
    OpenGLContext*          mpFirstContext;                 // First OpenGLContext
    OpenGLContext*          mpLastContext;                  // Last OpenGLContext
    VclPtr<Printer>         mpFirstPrinter;                 // First Printer
    VclPtr<Printer>         mpLastPrinter;                  // Last Printer
    ImplPrnQueueList*       mpPrinterQueueList;             // List of all printer queue
    PhysicalFontCollection* mpScreenFontList;               // Screen-Font-List
    ImplFontCache*          mpScreenFontCache;              // Screen-Font-Cache
    ImplDirectFontSubstitution* mpDirectFontSubst;          // Font-Substitutons defined in Tools->Options->Fonts
    GraphicConverter*       mpGrfConverter;                 // Converter for graphics
    long                    mnRealAppFontX;                 // AppFont X-Numenator for 40/tel Width
    long                    mnAppFontX;                     // AppFont X-Numenator for 40/tel Width + DialogScaleX
    long                    mnAppFontY;                     // AppFont Y-Numenator for 80/tel Height
    bool                    mbFontSubChanged;               // true: FontSubstitution was changed between Begin/End
    bool                    mbNativeFontConfig;             // true: do not override UI font
};

struct ImplSVWinData
{
    VclPtr<vcl::Window>     mpFirstFrame;                   // First FrameWindow
    VclPtr<vcl::Window>     mpDefDialogParent;              // Default Dialog Parent
    VclPtr<WorkWindow>      mpAppWin;                       // Application-Window
    VclPtr<vcl::Window>     mpFocusWin;                     // window, that has the focus
    VclPtr<vcl::Window>     mpActiveApplicationFrame;       // the last active application frame, can be used as DefModalDialogParent if no focuswin set
    VclPtr<vcl::Window>     mpCaptureWin;                   // window, that has the mouse capture
    VclPtr<vcl::Window>     mpLastDeacWin;                  // Window, that need a deactivate (FloatingWindow-Handling)
    VclPtr<FloatingWindow>  mpFirstFloat;                   // First FloatingWindow in PopupMode
    VclPtr<Dialog>          mpLastExecuteDlg;               // First Dialog that is in Execute
    VclPtr<vcl::Window>     mpExtTextInputWin;              // Window, which is in ExtTextInput
    VclPtr<vcl::Window>     mpTrackWin;                     // window, that is in tracking mode
    AutoTimer*              mpTrackTimer;                   // tracking timer
    ImageList*              mpMsgBoxImgList;                // ImageList for MessageBox
    VclPtr<vcl::Window>     mpAutoScrollWin;                // window, that is in AutoScrollMode mode
    StartTrackingFlags      mnTrackFlags;                   // tracking flags
    StartAutoScrollFlags    mnAutoScrollFlags;              // auto scroll flags
    bool                    mbNoDeactivate;                 // true: do not execute Deactivate
    bool                    mbNoSaveFocus;                  // true: menus must not save/restore focus
    bool                    mbNoSaveBackground;             // true: save background is unnecessary or even less performant
};

typedef std::vector< std::pair< OUString, FieldUnit > > FieldUnitStringList;

struct ImplSVCtrlData
{
    ImageList*              mpCheckImgList;                 // ImageList for CheckBoxes
    ImageList*              mpRadioImgList;                 // ImageList for RadioButtons
    ImageList*              mpPinImgList;                   // ImageList for PIN
    ImageList*              mpSplitHPinImgList;             // ImageList for Horizontale SplitWindows
    ImageList*              mpSplitVPinImgList;             // ImageList for Vertikale SplitWindows (PIN's)
    ImageList*              mpSplitHArwImgList;             // ImageList for Horizontale SplitWindows (Arrows)
    ImageList*              mpSplitVArwImgList;             // ImageList for Vertikale SplitWindows (Arrows)
    Image*                  mpDisclosurePlus;
    Image*                  mpDisclosureMinus;
    ImplTBDragMgr*          mpTBDragMgr;                    // DragMgr for ToolBox
    sal_uInt16              mnCheckStyle;                   // CheckBox-Style for ImageList-Update
    sal_uInt16              mnRadioStyle;                   // Radio-Style for ImageList-Update
    sal_uLong               mnLastCheckFColor;              // Letzte FaceColor fuer CheckImage
    sal_uLong               mnLastCheckWColor;              // Letzte WindowColor fuer CheckImage
    sal_uLong               mnLastCheckWTextColor;          // Letzte WindowTextColor fuer CheckImage
    sal_uLong               mnLastCheckLColor;              // Letzte LightColor fuer CheckImage
    sal_uLong               mnLastRadioFColor;              // Letzte FaceColor fuer RadioImage
    sal_uLong               mnLastRadioWColor;              // Letzte WindowColor fuer RadioImage
    sal_uLong               mnLastRadioLColor;              // Letzte LightColor fuer RadioImage
    FieldUnitStringList*    mpFieldUnitStrings;             // list with field units
    FieldUnitStringList*    mpCleanUnitStrings;             // same list but with some "fluff" like spaces removed
};

struct ImplSVHelpData
{
    bool                    mbContextHelp       : 1;        // is ContextHelp enabled
    bool                    mbExtHelp           : 1;        // is ExtendedHelp enabled
    bool                    mbExtHelpMode       : 1;        // is in ExtendedHelp Mode
    bool                    mbOldBalloonMode    : 1;        // BalloonMode, before ExtHelpMode started
    bool                    mbBalloonHelp       : 1;        // is BalloonHelp enabled
    bool                    mbQuickHelp         : 1;        // is QuickHelp enabled
    bool                    mbSetKeyboardHelp   : 1;        // tiphelp was activated by keyboard
    bool                    mbKeyboardHelp      : 1;        // tiphelp was activated by keyboard
    bool                    mbAutoHelpId        : 1;        // generate HelpIds
    bool                    mbRequestingHelp    : 1;        // In Window::RequestHelp
    VclPtr<HelpTextWindow>  mpHelpWin;                      // HelpWindow
    sal_uInt64              mnLastHelpHideTime;             // ticks of last show
};

// "NWF" means "Native Widget Framework" and was the term used for the
// idea that StarView/OOo "widgets" should *look* (and feel) like the
// "native widgets" on each platform, even if not at all implemented
// using them. See http://people.redhat.com/dcbw/ooo-nwf.html .

struct ImplSVNWFData
{
    int                     mnStatusBarLowerRightOffset;    // amount in pixel to avoid in the lower righthand corner
    int                     mnMenuFormatBorderX;            // horizontal inner popup menu border
    int                     mnMenuFormatBorderY;            // vertical inner popup menu border
    int                     mnMenuSeparatorBorderX;         // gap at each side of separator
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
    bool                    mbNoFocusRects:1;               // on Aqua/Gtk3 use native focus rendering, except for flat buttons
    bool                    mbNoFocusRectsForFlatButtons:1; // on Gtk3 native focusing is also preferred for flat buttons
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
    bool                    mbEnableAccel:1;                // whether or not accelerators are shown
    bool                    mbAutoAccel:1;                  // whether accelerators are only shown when Alt is held down
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
    ImplSVData();

    SalData*                mpSalData;
    SalInstance*            mpDefInst;                      // Default SalInstance
    Application*            mpApp;                          // pApp
    VclPtr<WorkWindow>      mpDefaultWin;                   // Default-Window
    bool                    mbDeInit;                       // Is VCL deinitializing
    sal_uLong               mnThreadCount;                  // is VCL MultiThread enabled
    ImplConfigData*         mpFirstConfigData;              // pointer to the first config block
    ImplSchedulerData*      mpFirstSchedulerData;           // list of all running tasks
    SalTimer*               mpSalTimer;                     // interface to sal event loop/timers
    SalI18NImeStatus*       mpImeStatus;                    // interface to ime status window
    SalSystem*              mpSalSystem;                    // SalSystem interface
    ResMgr*                 mpResMgr;                       // SV-Resource-Manager
    sal_uInt64              mnTimerPeriod;                  // current timer period
    sal_uInt32              mnUpdateStack;                  // Scheduler on stack
    ImplSVAppData           maAppData;                      // indepen data for class Application
    ImplSVGDIData           maGDIData;                      // indepen data for Output classes
    ImplSVWinData           maWinData;                      // indepen data for Windows classes
    ImplSVCtrlData          maCtrlData;                     // indepen data for Control classes
    ImplSVHelpData          maHelpData;                     // indepen data for Help classes
    ImplSVNWFData           maNWFData;
    UnoWrapperBase*         mpUnoWrapper;
    VclPtr<vcl::Window>     mpIntroWindow;                  // the splash screen
    DockingManager*         mpDockingManager;
    BlendFrameCache*        mpBlendFrameCache;

    oslThreadIdentifier     mnMainThreadId;
    rtl::Reference< vcl::DisplayConnection > mxDisplayConnection;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > mxAccessBridge;
    vcl::SettingsConfigItem* mpSettingsConfigItem;
    std::list< vcl::DeleteOnDeinitBase* >* mpDeinitDeleteList;
    std::unordered_map< int, OUString >* mpPaperNames;

    Link<LinkParamNone*,void> maDeInitHook;
};

void        ImplDeInitSVData();
VCL_PLUGIN_PUBLIC vcl::Window* ImplGetDefaultWindow();
VCL_PLUGIN_PUBLIC ResMgr*     ImplGetResMgr();
VCL_PLUGIN_PUBLIC ResId VclResId( sal_Int32 nId ); // throws std::bad_alloc if no res mgr
DockingManager*     ImplGetDockingManager();
BlendFrameCache*    ImplGetBlendFrameCache();
void        ImplWindowAutoMnemonic( vcl::Window* pWindow );

bool        ImplCallHotKey( const vcl::KeyCode& rKeyCode );
void        ImplFreeHotKeyData();
void        ImplFreeEventHookData();

bool        ImplCallPreNotify( NotifyEvent& rEvt );

VCL_PLUGIN_PUBLIC ImplSVData* ImplGetSVData();
VCL_PLUGIN_PUBLIC void ImplHideSplash();

#ifdef _WIN32
bool ImplInitAccessBridge();
#endif

FieldUnitStringList* ImplGetFieldUnits();
FieldUnitStringList* ImplGetCleanedFieldUnits();

// ImplDelData is used as a "dog tag" by a window when it
// does something that could indirectly destroy the window
// TODO: wild destruction of a window should not be possible

struct ImplDelData
{
    ImplDelData*        mpNext;
    VclPtr<vcl::Window> mpWindow;
    bool                mbDel;

                        ImplDelData( vcl::Window* pWindow = NULL );
    virtual             ~ImplDelData();

    bool                IsDead() const
    {
        DBG_ASSERT( !mbDel, "object deleted while in use !" );
        return mbDel;
    }

private:
    void                AttachToWindow( const vcl::Window* );

};

struct ImplFocusDelData : public ImplDelData
{
    VclPtr<vcl::Window> mpFocusWin;
};

struct ImplSVEvent
{
    void*               mpData;
    Link<void*,void>    maLink;
    VclPtr<vcl::Window> mpInstanceRef;
    VclPtr<vcl::Window> mpWindow;
    ImplDelData         maDelData;
    bool                mbCall;
};

#endif // INCLUDED_VCL_INC_SVDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
