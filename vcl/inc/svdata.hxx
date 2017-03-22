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

#include <config_version.h>

#include <tools/fldunit.hxx>
#include <unotools/options.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/lang/XComponent.hpp>

#include "vcleventlisteners.hxx"
#include "impfontcache.hxx"
#include "salwtype.hxx"
#include "displayconnectiondispatch.hxx"

#include <list>
#include <unordered_map>
#include <boost/functional/hash.hpp>
#include "ControlCacheKey.hxx"

struct ImplPostEventData;
struct ImplTimerData;
struct ImplIdleData;
struct ImplConfigData;
class ImplDirectFontSubstitution;
struct ImplHotKey;
struct ImplEventHook;
struct ImplSchedulerData;
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
class Image;
class PopupMenu;
class Application;
class OutputDevice;
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

namespace vcl
{
    class DisplayConnectionDispatch;
    class SettingsConfigItem;
    class DeleteOnDeinitBase;
    class Window;
}

class LocaleConfigurationListener : public utl::ConfigurationListener
{
public:
    virtual void ConfigurationChanged( utl::ConfigurationBroadcaster*, ConfigurationHints ) override;
};

typedef std::vector<Link<VclWindowEvent&,bool> > SVAppKeyListeners;

typedef std::pair<VclPtr<vcl::Window>, ImplPostEventData *> ImplPostEventPair;

struct ImplSVAppData
{
    enum ImeStatusWindowMode
    {
        ImeStatusWindowMode_UNKNOWN,
        ImeStatusWindowMode_HIDE,
        ImeStatusWindowMode_SHOW
    };

    AllSettings*            mpSettings = nullptr;           // Application settings
    LocaleConfigurationListener* mpCfgListener = nullptr;
    VclEventListeners*      mpEventListeners = nullptr;     // listeners for vcl events (eg, extended toolkit)
    SVAppKeyListeners*      mpKeyListeners = nullptr;       // listeners for key events only (eg, extended toolkit)
    std::list<ImplPostEventPair> maPostedEventList;
    ImplAccelManager*       mpAccelMgr = nullptr;           // Accelerator Manager
    OUString*               mpAppName = nullptr;            // Application name
    OUString*               mpAppFileName = nullptr;        // Abs. Application FileName
    OUString*               mpDisplayName = nullptr;        // Application Display Name
    OUString*               mpToolkitName = nullptr;        // Toolkit Name
    Help*                   mpHelp = nullptr;               // Application help
    VclPtr<PopupMenu>       mpActivePopupMenu;              // Actives Popup-Menu (in Execute)
    VclPtr<ImplWheelWindow> mpWheelWindow;                  // WheelWindow
    ImplHotKey*             mpFirstHotKey = nullptr;        // HotKey-Verwaltung
    ImplEventHook*          mpFirstEventHook = nullptr;     // Event-Hooks
    sal_uInt64              mnLastInputTime = 0;            // GetLastInputTime()
    sal_uInt16              mnDispatchLevel = 0;            // DispatchLevel
    sal_uInt16              mnModalMode = 0;                // ModalMode Count
    SystemWindowFlags       mnSysWinMode = SystemWindowFlags(0); // Mode, when SystemWindows should be created
    bool                    mbInAppMain = false;            // is Application::Main() on stack
    bool                    mbInAppExecute = false;         // is Application::Execute() on stack
    bool                    mbAppQuit = false;              // is Application::Quit() called
    bool                    mbSettingsInit = false;         // true: Settings are initialized
    Application::DialogCancelMode meDialogCancel = Application::DialogCancelMode::Off; // true: All Dialog::Execute() calls will be terminated immediately with return false

    /** Controls whether showing any IME status window is toggled on or off.

        Only meaningful if showing IME status windows can be toggled on and off
        externally (see Application::CanToggleImeStatusWindow).
     */
    ImeStatusWindowMode meShowImeStatusWindow = ImeStatusWindowMode_UNKNOWN;

    SvFileStream*       mpEventTestInput = nullptr;
    Idle*               mpEventTestingIdle = nullptr;
    int                 mnEventTestLimit = 0;

    DECL_STATIC_LINK(ImplSVAppData, ImplQuitMsg, void*, void);
    DECL_STATIC_LINK(ImplSVAppData, ImplPrepareExitMsg, void*, void);
    DECL_STATIC_LINK(ImplSVAppData, ImplEndAllDialogsMsg, void*, void);
    DECL_STATIC_LINK(ImplSVAppData, ImplEndAllPopupsMsg, void*, void);
    DECL_STATIC_LINK(ImplSVAppData, ImplVclEventTestingHdl, void*, void);
    DECL_LINK(VclEventTestingHdl, Timer*, void);
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
    OpenGLContext*          mpFirstContext = nullptr;       // First OpenGLContext
    OpenGLContext*          mpLastContext = nullptr;        // Last OpenGLContext
    VclPtr<Printer>         mpFirstPrinter;                 // First Printer
    VclPtr<Printer>         mpLastPrinter;                  // Last Printer
    ImplPrnQueueList*       mpPrinterQueueList = nullptr;   // List of all printer queue
    PhysicalFontCollection* mpScreenFontList = nullptr;     // Screen-Font-List
    ImplFontCache*          mpScreenFontCache = nullptr;    // Screen-Font-Cache
    ImplDirectFontSubstitution* mpDirectFontSubst = nullptr; // Font-Substitutons defined in Tools->Options->Fonts
    GraphicConverter*       mpGrfConverter = nullptr;       // Converter for graphics
    long                    mnAppFontX = 0;                 // AppFont X-Numenator for 40/tel Width
    long                    mnAppFontY = 0;                 // AppFont Y-Numenator for 80/tel Height
    bool                    mbFontSubChanged = false;       // true: FontSubstitution was changed between Begin/End
    bool                    mbNativeFontConfig = false;     // true: do not override UI font
};

struct ImplSVWinData
{
    VclPtr<vcl::Window>     mpFirstFrame;                   // First FrameWindow
    VclPtr<WorkWindow>      mpAppWin;                       // Application-Window
    VclPtr<vcl::Window>     mpFocusWin;                     // window, that has the focus
    VclPtr<vcl::Window>     mpActiveApplicationFrame;       // the last active application frame, can be used as DefModalDialogParent if no focuswin set
    VclPtr<vcl::Window>     mpCaptureWin;                   // window, that has the mouse capture
    VclPtr<vcl::Window>     mpLastDeacWin;                  // Window, that need a deactivate (FloatingWindow-Handling)
    VclPtr<FloatingWindow>  mpFirstFloat;                   // First FloatingWindow in PopupMode
    VclPtr<Dialog>          mpLastExecuteDlg;               // First Dialog that is in Execute
    VclPtr<vcl::Window>     mpExtTextInputWin;              // Window, which is in ExtTextInput
    VclPtr<vcl::Window>     mpTrackWin;                     // window, that is in tracking mode
    AutoTimer*              mpTrackTimer = nullptr;         // tracking timer
    std::vector<Image>      maMsgBoxImgList;                // ImageList for MessageBox
    VclPtr<vcl::Window>     mpAutoScrollWin;                // window, that is in AutoScrollMode mode
    VclPtr<vcl::Window>     mpLastWheelWindow;              // window, that last received a mouse wheel event
    SalWheelMouseEvent      maLastWheelEvent;               // the last received mouse whell event

    StartTrackingFlags      mnTrackFlags = StartTrackingFlags::NONE; // tracking flags
    StartAutoScrollFlags    mnAutoScrollFlags = StartAutoScrollFlags::NONE; // auto scroll flags
    bool                    mbNoDeactivate = false;         // true: do not execute Deactivate
    bool                    mbNoSaveFocus = false;          // true: menus must not save/restore focus
};

typedef std::vector< std::pair< OUString, FieldUnit > > FieldUnitStringList;

struct ImplSVCtrlData
{
    std::vector<Image>      maCheckImgList;                 // ImageList for CheckBoxes
    std::vector<Image>      maRadioImgList;                 // ImageList for RadioButtons
    Image*                  mpDisclosurePlus = nullptr;
    Image*                  mpDisclosureMinus = nullptr;
    ImplTBDragMgr*          mpTBDragMgr = nullptr;          // DragMgr for ToolBox
    sal_uInt16              mnCheckStyle = 0;               // CheckBox-Style for ImageList-Update
    sal_uInt16              mnRadioStyle = 0;               // Radio-Style for ImageList-Update
    sal_uLong               mnLastCheckFColor = 0;          // Letzte FaceColor fuer CheckImage
    sal_uLong               mnLastCheckWColor = 0;          // Letzte WindowColor fuer CheckImage
    sal_uLong               mnLastCheckLColor = 0;          // Letzte LightColor fuer CheckImage
    sal_uLong               mnLastRadioFColor = 0;          // Letzte FaceColor fuer RadioImage
    sal_uLong               mnLastRadioWColor = 0;          // Letzte WindowColor fuer RadioImage
    sal_uLong               mnLastRadioLColor = 0;          // Letzte LightColor fuer RadioImage
    FieldUnitStringList*    mpFieldUnitStrings = nullptr;   // list with field units
    FieldUnitStringList*    mpCleanUnitStrings = nullptr;   // same list but with some "fluff" like spaces removed
};

struct ImplSVHelpData
{
    bool                    mbContextHelp = false;          // is ContextHelp enabled
    bool                    mbExtHelp = false;              // is ExtendedHelp enabled
    bool                    mbExtHelpMode = false;          // is in ExtendedHelp Mode
    bool                    mbOldBalloonMode = false;       // BalloonMode, before ExtHelpMode started
    bool                    mbBalloonHelp = false;          // is BalloonHelp enabled
    bool                    mbQuickHelp = false;            // is QuickHelp enabled
    bool                    mbSetKeyboardHelp = false;      // tiphelp was activated by keyboard
    bool                    mbKeyboardHelp = false;         // tiphelp was activated by keyboard
    bool                    mbRequestingHelp = false;       // In Window::RequestHelp
    VclPtr<HelpTextWindow>  mpHelpWin;                      // HelpWindow
    sal_uInt64              mnLastHelpHideTime = 0;         // ticks of last show
};

// "NWF" means "Native Widget Framework" and was the term used for the
// idea that StarView/OOo "widgets" should *look* (and feel) like the
// "native widgets" on each platform, even if not at all implemented
// using them. See http://people.redhat.com/dcbw/ooo-nwf.html .

struct ImplSVNWFData
{
    int                     mnStatusBarLowerRightOffset = 0; // amount in pixel to avoid in the lower righthand corner
    int                     mnMenuFormatBorderX = 0;        // horizontal inner popup menu border
    int                     mnMenuFormatBorderY = 0;        // vertical inner popup menu border
    int                     mnMenuSeparatorBorderX = 0;     // gap at each side of separator
    ::Color                 maMenuBarHighlightTextColor = Color( COL_TRANSPARENT ); // override highlight text color
                                                            // in menubar if not transparent
    bool                    mbMenuBarDockingAreaCommonBG = false; // e.g. WinXP default theme
    bool                    mbDockingAreaSeparateTB = false; // individual toolbar backgrounds
                                                            // instead of one for docking area
    bool                    mbDockingAreaAvoidTBFrames = false; ///< don't draw frames around the individual toolbars if mbDockingAreaSeparateTB is false
    bool                    mbToolboxDropDownSeparate = false; // two adjacent buttons for
                                                            // toolbox dropdown buttons
    bool                    mbFlatMenu = false;             // no popup 3D border
    bool                    mbOpenMenuOnF10 = false;        // on gnome the first menu opens on F10
    bool                    mbNoFocusRects = false;         // on Aqua/Gtk3 use native focus rendering, except for flat buttons
    bool                    mbNoFocusRectsForFlatButtons = false; // on Gtk3 native focusing is also preferred for flat buttons
    bool                    mbCenteredTabs = false;         // on Aqua, tabs are centered
    bool                    mbNoActiveTabTextRaise = false; // on Aqua the text for the selected tab
                                                            // should not "jump up" a pixel
    bool                    mbProgressNeedsErase = false;   // set true for platforms that should draw the
                                                            // window background before drawing the native
                                                            // progress bar
    bool                    mbCheckBoxNeedsErase = false;   // set true for platforms that should draw the
                                                            // window background before drawing the native
                                                            // checkbox
    bool                    mbCanDrawWidgetAnySize = false; // set to true currently on gtk

    /// entire drop down listbox resembles a button, no textarea/button parts (as currently on Windows)
    bool                    mbDDListBoxNoTextArea = false;
    bool                    mbEnableAccel = true;           // whether or not accelerators are shown
    bool                    mbAutoAccel = false;            // whether accelerators are only shown when Alt is held down
    bool                    mbRolloverMenubar = false;      // theming engine supports rollover in menubar
    // gnome#768128 I cannot see a route under wayland at present to support
    // floating toolbars that can be redocked because there's no way to track
    // that the toolbar is over a dockable area.
    bool                    mbCanDetermineWindowPosition = true;
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

struct ImplSchedulerContext
{
    ImplSchedulerData*      mpFirstSchedulerData = nullptr; ///< list of all active tasks
    ImplSchedulerData*      mpLastSchedulerData = nullptr;  ///< last item of the mpFirstSchedulerData list
    ImplSchedulerData*      mpSchedulerStack = nullptr;     ///< stack of invoked tasks
    SalTimer*               mpSalTimer = nullptr;           ///< interface to sal event loop / system timer
    sal_uInt64              mnTimerStart = 0;               ///< start time of the timer
    sal_uInt64              mnTimerPeriod = SAL_MAX_UINT64; ///< current timer period
};

struct ImplSVData
{
    SalData*                mpSalData = nullptr;
    SalInstance*            mpDefInst = nullptr;            // Default SalInstance
    Application*            mpApp = nullptr;                // pApp
    VclPtr<WorkWindow>      mpDefaultWin;                   // Default-Window
    bool                    mbDeInit = false;               // Is VCL deinitializing
    SalI18NImeStatus*       mpImeStatus = nullptr;          // interface to ime status window
    SalSystem*              mpSalSystem = nullptr;          // SalSystem interface
    ResMgr*                 mpResMgr = nullptr;             // SV-Resource-Manager
    ImplSchedulerContext    maSchedCtx;                     // indepen data for class Scheduler
    ImplSVAppData           maAppData;                      // indepen data for class Application
    ImplSVGDIData           maGDIData;                      // indepen data for Output classes
    ImplSVWinData           maWinData;                      // indepen data for Windows classes
    ImplSVCtrlData          maCtrlData;                     // indepen data for Control classes
    ImplSVHelpData          maHelpData;                     // indepen data for Help classes
    ImplSVNWFData           maNWFData;
    UnoWrapperBase*         mpUnoWrapper = nullptr;
    VclPtr<vcl::Window>     mpIntroWindow;                  // the splash screen
    DockingManager*         mpDockingManager = nullptr;
    BlendFrameCache*        mpBlendFrameCache = nullptr;

    oslThreadIdentifier     mnMainThreadId = 0;
    rtl::Reference< vcl::DisplayConnectionDispatch > mxDisplayConnection;

    css::uno::Reference< css::lang::XComponent > mxAccessBridge;
    vcl::SettingsConfigItem* mpSettingsConfigItem = nullptr;
    std::list< vcl::DeleteOnDeinitBase* >* mpDeinitDeleteList = nullptr;
    std::unordered_map< int, OUString >* mpPaperNames = nullptr;

    Link<LinkParamNone*,void> maDeInitHook;
};

void        ImplDeInitSVData();
VCL_PLUGIN_PUBLIC vcl::Window* ImplGetDefaultWindow();
VCL_PLUGIN_PUBLIC vcl::Window* ImplGetDefaultContextWindow();
VCL_PLUGIN_PUBLIC ResMgr*     ImplGetResMgr();
VCL_PLUGIN_PUBLIC ResId VclResId( sal_Int32 nId ); // throws std::bad_alloc if no res mgr
DockingManager*     ImplGetDockingManager();
BlendFrameCache*    ImplGetBlendFrameCache();

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

struct ImplSVEvent
{
    void*               mpData;
    Link<void*,void>    maLink;
    VclPtr<vcl::Window> mpInstanceRef;
    VclPtr<vcl::Window> mpWindow;
    bool                mbCall;
};

struct ControlCacheHashFunction
{
    std::size_t operator()(ControlCacheKey const& aCache) const
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, aCache.mnType);
        boost::hash_combine(seed, aCache.mnPart);
        boost::hash_combine(seed, aCache.mnState);
        boost::hash_combine(seed, aCache.maSize.Width());
        boost::hash_combine(seed, aCache.maSize.Height());
        return seed;
    }
};

extern int nImplSysDialog;

#endif // INCLUDED_VCL_INC_SVDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
