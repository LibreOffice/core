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

#include <config_version.h>

#include "vcl/svapp.hxx"

#include "unx/gtk/gtkframe.hxx"
#include "unx/gtk/gtkdata.hxx"
#include "unx/gtk/gtkinst.hxx"
#include "unx/gtk/gtkgdi.hxx"

#include "unx/pixmap.hxx"
#include "unx/saldata.hxx"
#include "unx/saldisp.hxx"

#include <cstdio>
#include <cmath>
#include <vector>
#include <algorithm>
#include <unordered_map>

#include <boost/optional.hpp>

#include "vcl/vclenum.hxx"
#include <vcl/settings.hxx>
#include "fontmanager.hxx"
#include <vcl/decoview.hxx>

#include <vcl/opengl/OpenGLHelper.hxx>

typedef struct _cairo_font_options cairo_font_options_t;
const char* const tabPrelitDataName="libreoffice-tab-is-prelit";

// initialize statics
bool GtkSalGraphics::bThemeChanged = true;
bool GtkSalGraphics::bNeedPixmapPaint = false;
bool GtkSalGraphics::bNeedTwoPasses = false;

enum
{
    BG_NONE = 0,
    BG_FILL,
    BG_WHITE,
    BG_BLACK
};

GtkSalGraphics::GtkSalGraphics( GtkSalFrame *pFrame, GtkWidget *pWindow )
    : X11SalGraphics(),
      m_pWindow( pWindow ),
      m_aClipRegion(true)
{
    Init( pFrame, GDK_WINDOW_XID( widget_get_window( pWindow ) ),
          SalX11Screen( gdk_x11_screen_get_screen_number(
                                gtk_widget_get_screen( pWindow ) ) ) );
}

GtkSalGraphics::GtkSalGraphics( GtkSalFrame *pFrame, GtkWidget *pWindow,
                                SalX11Screen nXScreen )
    : X11SalGraphics(),
      m_pWindow( pWindow ),
      m_aClipRegion(true)
{
    Init( pFrame, GDK_WINDOW_XID( widget_get_window( pWindow ) ), nXScreen );
}

GtkSalGraphics::~GtkSalGraphics()
{
}

/*************************************
 * Cached native widget objects
 *************************************/
class NWPixmapCacheList;
class NWPixmapCache;
struct NWFWidgetData
{
    GtkWidget *  gCacheWindow;
    GtkWidget *  gDumbContainer;

    GtkWidget *  gBtnWidget;
    GtkWidget *  gRadioWidget;
    GtkWidget *  gRadioWidgetSibling;
    GtkWidget *  gCheckWidget;
    GtkWidget *  gScrollHorizWidget;
    GtkWidget *  gScrollVertWidget;
    GtkWidget *  gArrowWidget;
    GtkWidget *  gDropdownWidget;
    GtkWidget *  gEditBoxWidget;
    GtkWidget *  gSpinButtonWidget;
    GtkWidget *  gNotebookWidget;
    GtkWidget *  gOptionMenuWidget;
    GtkWidget *  gComboWidget;
    GtkWidget *  gScrolledWindowWidget;
    GtkWidget *  gToolbarWidget;
    GtkWidget *  gToolbarButtonWidget;
    GtkWidget *  gHandleBoxWidget;
    GtkWidget *  gMenubarWidget;
    GtkWidget *  gMenuItemMenubarWidget;
    GtkWidget *  gMenuWidget;
    GtkWidget *  gMenuItemMenuWidget;
    GtkWidget *  gMenuItemCheckMenuWidget;
    GtkWidget *  gMenuItemRadioMenuWidget;
    GtkWidget *  gMenuItemSeparatorMenuWidget;
    GtkWidget *  gImageMenuItem;
    GtkWidget *  gTooltipPopup;
    GtkWidget *  gProgressBar;
    GtkWidget *  gTreeView;
    GtkWidget *  gHScale;
    GtkWidget *  gVScale;
    GtkWidget *  gSeparator;
    GtkWidget *  gDialog;
    GtkWidget *  gFrame;

    NWPixmapCacheList* gNWPixmapCacheList;
    NWPixmapCache* gCacheTabItems;
    NWPixmapCache* gCacheTabPages;

    NWFWidgetData() :
        gCacheWindow( nullptr ),
        gDumbContainer( nullptr ),
        gBtnWidget( nullptr ),
        gRadioWidget( nullptr ),
        gRadioWidgetSibling( nullptr ),
        gCheckWidget( nullptr ),
        gScrollHorizWidget( nullptr ),
        gScrollVertWidget( nullptr ),
        gArrowWidget( nullptr ),
        gDropdownWidget( nullptr ),
        gEditBoxWidget( nullptr ),
        gSpinButtonWidget( nullptr ),
        gNotebookWidget( nullptr ),
        gOptionMenuWidget( nullptr ),
        gComboWidget( nullptr ),
        gScrolledWindowWidget( nullptr ),
        gToolbarWidget( nullptr ),
        gToolbarButtonWidget( nullptr ),
        gHandleBoxWidget( nullptr ),
        gMenubarWidget( nullptr ),
        gMenuItemMenubarWidget( nullptr ),
        gMenuWidget( nullptr ),
        gMenuItemMenuWidget( nullptr ),
        gMenuItemCheckMenuWidget( nullptr ),
        gMenuItemRadioMenuWidget( nullptr ),
        gMenuItemSeparatorMenuWidget( nullptr ),
        gImageMenuItem( nullptr ),
        gTooltipPopup( nullptr ),
        gProgressBar( nullptr ),
        gTreeView( nullptr ),
        gHScale( nullptr ),
        gVScale( nullptr ),
        gSeparator( nullptr ),
        gDialog( nullptr ),
        gFrame( nullptr ),
        gNWPixmapCacheList( nullptr ),
        gCacheTabItems( nullptr ),
        gCacheTabPages( nullptr )
    {}
};

// Keep a hash table of Widgets->default flags so that we can
// easily and quickly reset each to a default state before using
// them
static std::unordered_map<long, guint>    gWidgetDefaultFlags;
class WidgetDataVector
{
private:
    std::vector<NWFWidgetData> mData;

public:
    explicit WidgetDataVector(size_t nElems = 0) : mData( nElems ) {}
    size_t size() const { return mData.size(); }
    NWFWidgetData &operator [](size_t i) { return mData.at(i); }
    NWFWidgetData &operator [](const SalX11Screen &s) { return mData.at(s.getXScreen()); }
};
static WidgetDataVector gWidgetData;

static const GtkBorder aDefDefBorder        = { 1, 1, 1, 1 };

// Some GTK defaults
#define MIN_ARROW_SIZE                    11
#define BTN_CHILD_SPACING                1
#define MIN_SPIN_ARROW_WIDTH                6

static void NWEnsureGTKRadio             ( SalX11Screen nScreen );
static void NWEnsureGTKButton            ( SalX11Screen nScreen );
static void NWEnsureGTKCheck             ( SalX11Screen nScreen );
static void NWEnsureGTKScrollbars        ( SalX11Screen nScreen );
static void NWEnsureGTKArrow             ( SalX11Screen nScreen );
static void NWEnsureGTKEditBox           ( SalX11Screen nScreen );
static void NWEnsureGTKSpinButton        ( SalX11Screen nScreen );
static void NWEnsureGTKNotebook          ( SalX11Screen nScreen );
static void NWEnsureGTKOptionMenu        ( SalX11Screen nScreen );
static void NWEnsureGTKCombo             ( SalX11Screen nScreen );
static void NWEnsureGTKScrolledWindow    ( SalX11Screen nScreen );
static void NWEnsureGTKToolbar           ( SalX11Screen nScreen );
static void NWEnsureGTKMenubar           ( SalX11Screen nScreen );
static void NWEnsureGTKMenu              ( SalX11Screen nScreen );
static void NWEnsureGTKTooltip           ( SalX11Screen nScreen );
static void NWEnsureGTKDialog            ( SalX11Screen nScreen );
static void NWEnsureGTKFrame             ( SalX11Screen nScreen );
static void NWEnsureGTKProgressBar       ( SalX11Screen nScreen );
static void NWEnsureGTKTreeView          ( SalX11Screen nScreen );
static void NWEnsureGTKSlider            ( SalX11Screen nScreen );

static void NWConvertVCLStateToGTKState( ControlState nVCLState, GtkStateType* nGTKState, GtkShadowType* nGTKShadow );
static void NWAddWidgetToCacheWindow( GtkWidget* widget, SalX11Screen nScreen );
static void NWSetWidgetState( GtkWidget* widget, ControlState nState, GtkStateType nGtkState );

static void NWCalcArrowRect( const Rectangle& rButton, Rectangle& rArrow );

/*
 * Individual helper functions
 *
 */

static Rectangle NWGetButtonArea( SalX11Screen nScreen, ControlType nType, ControlPart nPart, Rectangle aAreaRect, ControlState nState,
                                const ImplControlValue& aValue, const OUString& rCaption );

static Rectangle NWGetTabItemRect( SalX11Screen nScreen, Rectangle aAreaRect );

static Rectangle NWGetEditBoxPixmapRect( SalX11Screen nScreen, ControlType nType, ControlPart nPart, Rectangle aAreaRect, ControlState nState,
                            const ImplControlValue& aValue, const OUString& rCaption );

static void NWPaintOneEditBox( SalX11Screen nScreen, GdkDrawable * gdkDrawable, GdkRectangle *gdkRect,
                               ControlType nType, ControlPart nPart, Rectangle aEditBoxRect,
                               ControlState nState, const ImplControlValue& aValue,
                               const OUString& rCaption );

static Rectangle NWGetSpinButtonRect( SalX11Screen nScreen, ControlType nType, ControlPart nPart, Rectangle aAreaRect, ControlState nState,
                            const ImplControlValue& aValue, const OUString& rCaption );

static void NWPaintOneSpinButton( SalX11Screen nScreen, GdkPixmap * pixmap, ControlType nType, ControlPart nPart, Rectangle aAreaRect,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );

static Rectangle NWGetComboBoxButtonRect( SalX11Screen nScreen, ControlType nType, ControlPart nPart, Rectangle aAreaRect, ControlState nState,
                            const ImplControlValue& aValue, const OUString& rCaption );

static Rectangle NWGetListBoxButtonRect( SalX11Screen nScreen, ControlType nType, ControlPart nPart, Rectangle aAreaRect, ControlState nState,
                            const ImplControlValue& aValue, const OUString& rCaption );

static Rectangle NWGetListBoxIndicatorRect( SalX11Screen nScreen, ControlType nType, ControlPart nPart, Rectangle aAreaRect, ControlState nState,
                            const ImplControlValue& aValue, const OUString& rCaption );

static Rectangle NWGetToolbarRect( SalX11Screen nScreen,
                                   ControlType nType,
                                   ControlPart nPart,
                                   Rectangle aAreaRect,
                                   ControlState nState,
                                   const ImplControlValue& aValue,
                                   const OUString& rCaption );

static int getFrameWidth(GtkWidget* widget);

static Rectangle NWGetScrollButtonRect(    SalX11Screen nScreen, ControlPart nPart, Rectangle aAreaRect );


/************************************************************************
 * GDK implementation of X11Pixmap
 ************************************************************************/

class GdkX11Pixmap : public X11Pixmap
{
public:
    GdkX11Pixmap( int nWidth, int nHeight, int nDepth );
    virtual ~GdkX11Pixmap();

    virtual int          GetDepth() const override;
    virtual SalX11Screen GetScreen() const override;
    virtual Pixmap       GetPixmap() const override;
    GdkPixmap*           GetGdkPixmap() const;
    GdkDrawable*         GetGdkDrawable() const;

protected:
    GdkPixmap* mpGdkPixmap;
    int        mnDepth;
};

GdkX11Pixmap::GdkX11Pixmap( int nWidth, int nHeight, int nDepth )
: X11Pixmap( nWidth, nHeight )
{
    mpGdkPixmap = gdk_pixmap_new( nullptr, nWidth, nHeight, nDepth );
    mnDepth = gdk_drawable_get_depth( GDK_DRAWABLE( mpGdkPixmap ) );

    GdkScreen *pScreen = gdk_drawable_get_screen( GDK_DRAWABLE( mpGdkPixmap ) );
    gdk_drawable_set_colormap( GDK_DRAWABLE( mpGdkPixmap ), gdk_screen_get_default_colormap( pScreen ) );
}

GdkX11Pixmap::~GdkX11Pixmap()
{
    g_object_unref( mpGdkPixmap );
}

int GdkX11Pixmap::GetDepth() const
{
    return mnDepth;
}

SalX11Screen GdkX11Pixmap::GetScreen() const
{
    return SalX11Screen( gdk_screen_get_number( gdk_drawable_get_screen( GDK_DRAWABLE(mpGdkPixmap) ) ) );
}

Pixmap GdkX11Pixmap::GetPixmap() const
{
    return GDK_PIXMAP_XID( mpGdkPixmap );
}

GdkPixmap* GdkX11Pixmap::GetGdkPixmap() const
{
    return mpGdkPixmap;
}

GdkDrawable* GdkX11Pixmap::GetGdkDrawable() const
{
    return GDK_DRAWABLE( mpGdkPixmap );
}


/*********************************************************
 * PixmapCache
 *********************************************************/

// as some native widget drawing operations are pretty slow
// with certain themes (eg tabpages)
// this cache can be used to cache the corresponding pixmap
// see NWPaintGTKTabItem

class NWPixmapCacheData
{
public:
    ControlType m_nType;
    ControlState m_nState;
    Rectangle      m_pixmapRect;
    GdkX11Pixmap*  m_pixmap;
    GdkX11Pixmap*  m_mask;

    NWPixmapCacheData() : m_nType(0), m_nState(ControlState::NONE), m_pixmap(nullptr), m_mask(nullptr) {}
    ~NWPixmapCacheData()
        { SetPixmap( nullptr, nullptr ); };
    void SetPixmap( GdkX11Pixmap* pPixmap, GdkX11Pixmap* pMask );
};

class NWPixmapCache
{
    int m_size;
    int m_idx;
    int m_screen;
    NWPixmapCacheData* pData;
public:
    explicit NWPixmapCache( SalX11Screen nScreen );
    ~NWPixmapCache();

    void SetSize( int n)
        { delete [] pData; m_idx = 0; m_size = n; pData = new NWPixmapCacheData[m_size]; }
    int GetSize() const { return m_size; }

    bool Find( ControlType aType, ControlState aState, const Rectangle& r_pixmapRect, GdkX11Pixmap** pPixmap, GdkX11Pixmap** pMask );
    void Fill( ControlType aType, ControlState aState, const Rectangle& r_pixmapRect, GdkX11Pixmap* pPixmap, GdkX11Pixmap* pMask );

    void ThemeChanged();
};

class NWPixmapCacheList
{
public:
    ::std::vector< NWPixmapCache* > mCaches;

    void AddCache( NWPixmapCache *pCache );
    void RemoveCache( NWPixmapCache *pCache );
    void ThemeChanged();
};

// --- implementation ---

void NWPixmapCacheData::SetPixmap( GdkX11Pixmap* pPixmap, GdkX11Pixmap* pMask )
{
    delete m_pixmap;
    delete m_mask;

    m_pixmap = pPixmap;
    m_mask = pMask;
}

NWPixmapCache::NWPixmapCache( SalX11Screen nScreen )
{
    m_idx = 0;
    m_size = 0;
    m_screen = nScreen.getXScreen();
    pData = nullptr;
    if( gWidgetData[m_screen].gNWPixmapCacheList )
        gWidgetData[m_screen].gNWPixmapCacheList->AddCache(this);
}
NWPixmapCache::~NWPixmapCache()
{
    if( gWidgetData[m_screen].gNWPixmapCacheList )
        gWidgetData[m_screen].gNWPixmapCacheList->RemoveCache(this);
    delete[] pData;
}
void NWPixmapCache::ThemeChanged()
{
    // throw away cached pixmaps
    int i;
    for(i=0; i<m_size; i++)
        pData[i].SetPixmap( nullptr, nullptr );
}

bool  NWPixmapCache::Find( ControlType aType, ControlState aState, const Rectangle& r_pixmapRect, GdkX11Pixmap** pPixmap, GdkX11Pixmap** pMask )
{
    aState &= ~ControlState::CACHING_ALLOWED; // mask clipping flag
    int i;
    for(i=0; i<m_size; i++)
    {
        if( pData[i].m_nType == aType &&
            pData[i].m_nState == aState &&
            pData[i].m_pixmapRect.GetWidth() == r_pixmapRect.GetWidth() &&
            pData[i].m_pixmapRect.GetHeight() == r_pixmapRect.GetHeight() &&
            pData[i].m_pixmap != nullptr )
        {
            *pPixmap = pData[i].m_pixmap;
            *pMask = pData[i].m_mask;
            return true;
        }
    }
    return false;
}

void NWPixmapCache::Fill( ControlType aType, ControlState aState, const Rectangle& r_pixmapRect, GdkX11Pixmap* pPixmap, GdkX11Pixmap* pMask )
{
    if( !(aState & ControlState::CACHING_ALLOWED) )
        return;

    aState &= ~ControlState::CACHING_ALLOWED; // mask clipping flag
    m_idx = (m_idx+1) % m_size; // just wrap
    pData[m_idx].m_nType = aType;
    pData[m_idx].m_nState = aState;
    pData[m_idx].m_pixmapRect = r_pixmapRect;
    pData[m_idx].SetPixmap( pPixmap, pMask );
}

void NWPixmapCacheList::AddCache( NWPixmapCache* pCache )
{
    mCaches.push_back( pCache );
}
void NWPixmapCacheList::RemoveCache( NWPixmapCache* pCache )
{
    ::std::vector< NWPixmapCache* >::iterator p;
    p = ::std::find( mCaches.begin(), mCaches.end(), pCache );
    if( p != mCaches.end() )
        mCaches.erase( p );
}
void NWPixmapCacheList::ThemeChanged( )
{
    ::std::vector< NWPixmapCache* >::iterator p = mCaches.begin();
    while( p != mCaches.end() )
    {
        (*p)->ThemeChanged();
        ++p;
    }
}

/*********************************************************
 * Make border manipulation easier
 *********************************************************/
inline void NW_gtk_border_set_from_border( GtkBorder& aDst, const GtkBorder * pSrc )
{
    aDst.left        = pSrc->left;
    aDst.top        = pSrc->top;
    aDst.right    = pSrc->right;
    aDst.bottom    = pSrc->bottom;
}

/*********************************************************
 * Initialize GTK and local stuff
 *********************************************************/
void GtkData::initNWF()
{
    ImplSVData* pSVData = ImplGetSVData();

    // draw no border for popup menus (NWF draws its own)
    pSVData->maNWFData.mbFlatMenu = true;

    // draw separate buttons for toolbox dropdown items
    pSVData->maNWFData.mbToolboxDropDownSeparate = true;

    // draw toolbars in separate lines
    pSVData->maNWFData.mbDockingAreaSeparateTB = true;

    // open first menu on F10
    pSVData->maNWFData.mbOpenMenuOnF10 = true;

    // omit GetNativeControl while painting (see brdwin.cxx)
    pSVData->maNWFData.mbCanDrawWidgetAnySize = true;

    pSVData->maNWFData.mbDDListBoxNoTextArea = true;

    // use offscreen rendering when using OpenGL backend
    if( OpenGLHelper::isVCLOpenGLEnabled() )
    {
        GtkSalGraphics::bNeedPixmapPaint = true;
        GtkSalGraphics::bNeedTwoPasses = true;
    }

    int nScreens = GetGtkSalData()->GetGtkDisplay()->GetXScreenCount();
    gWidgetData = WidgetDataVector( nScreens );
    for( int i = 0; i < nScreens; i++ )
        gWidgetData[i].gNWPixmapCacheList = new NWPixmapCacheList;

    // small extra border around menu items
    NWEnsureGTKMenu( SalX11Screen( 0 ) );
    gint horizontal_padding = 1;
    gint vertical_padding = 1;
    gint separator_padding = 1;
    gtk_widget_style_get( gWidgetData[0].gMenuWidget,
            "horizontal-padding", &horizontal_padding,
            nullptr);
    gtk_widget_style_get( gWidgetData[0].gMenuWidget,
            "vertical-padding", &vertical_padding,
            nullptr);
    gtk_widget_style_get( gWidgetData[0].gMenuItemSeparatorMenuWidget,
            "horizontal-padding", &separator_padding,
            nullptr);
    gint xthickness = gWidgetData[0].gMenuWidget->style->xthickness;
    gint ythickness = gWidgetData[0].gMenuWidget->style->ythickness;
    pSVData->maNWFData.mnMenuFormatBorderX = xthickness + horizontal_padding;
    pSVData->maNWFData.mnMenuFormatBorderY = ythickness + vertical_padding;
    pSVData->maNWFData.mnMenuSeparatorBorderX = separator_padding;

    pSVData->maNWFData.mbCheckBoxNeedsErase = true;

    if( SalGetDesktopEnvironment() == "KDE" )
    {
        // #i97196# ensure a widget exists and the style engine was loaded
        NWEnsureGTKButton( SalX11Screen( 0 ) );
        if( g_type_from_name( "QtEngineStyle" ) )
        {
            // KDE 3.3 invented a bug in the qt<->gtk theme engine
            // that makes direct rendering impossible: they totally
            // ignore the clip rectangle passed to the paint methods
            GtkSalGraphics::bNeedPixmapPaint = true;
        }
    }
    static const char* pEnv = getenv( "SAL_GTK_USE_PIXMAPPAINT" );
    if( pEnv && *pEnv )
        GtkSalGraphics::bNeedPixmapPaint = true;

    #if OSL_DEBUG_LEVEL > 1
    std::fprintf( stderr, "GtkPlugin: using %s NWF\n",
             GtkSalGraphics::bNeedPixmapPaint ? "offscreen" : "direct" );
    #endif

    GtkSettings *gtks = gtk_settings_get_default ();
    gint val;
    g_object_get (gtks, "gtk-auto-mnemonics", &val, NULL);
    if (val) pSVData->maNWFData.mbAutoAccel = true;
    else pSVData->maNWFData.mbAutoAccel = false;
    g_object_get (gtks, "gtk-enable-mnemonics", &val, NULL);
    if (val) pSVData->maNWFData.mbEnableAccel = true;
    else pSVData->maNWFData.mbEnableAccel = false;
}

/*********************************************************
 * Release GTK and local stuff
 *********************************************************/
void GtkData::deInitNWF()
{
    for( size_t i = 0; i < gWidgetData.size(); i++ )
    {
        // free up global widgets
        // gtk_widget_destroy will in turn destroy the child hierarchy
        // so only destroy disjunct hierarchies
        if( gWidgetData[i].gCacheWindow )
            gtk_widget_destroy( gWidgetData[i].gCacheWindow );
        if( gWidgetData[i].gMenuWidget )
            g_object_unref (gWidgetData[i].gMenuWidget);
        if( gWidgetData[i].gTooltipPopup )
            gtk_widget_destroy( gWidgetData[i].gTooltipPopup );
        if( gWidgetData[i].gDialog )
            gtk_widget_destroy( gWidgetData[i].gDialog );
        delete gWidgetData[i].gCacheTabPages;
        gWidgetData[i].gCacheTabPages = nullptr;
        delete gWidgetData[i].gCacheTabItems;
        gWidgetData[i].gCacheTabItems = nullptr;
        delete gWidgetData[i].gNWPixmapCacheList;
        gWidgetData[i].gNWPixmapCacheList = nullptr;
    }
}

/**********************************************************
 * track clip region
 **********************************************************/
void GtkSalGraphics::ResetClipRegion()
{
    m_aClipRegion.SetNull();
    X11SalGraphics::ResetClipRegion();
}

bool GtkSalGraphics::setClipRegion( const vcl::Region& i_rClip )
{
    m_aClipRegion = i_rClip;
    bool bRet = X11SalGraphics::setClipRegion( m_aClipRegion );
    if( m_aClipRegion.IsEmpty() )
        m_aClipRegion.SetNull();
    return bRet;
}

void GtkSalGraphics::copyBits( const SalTwoRect& rPosAry,
                               SalGraphics* pSrcGraphics )
{
    GtkSalFrame* pFrame = GetGtkFrame();
    ::Window aWin = None;
    if( pFrame && m_pWindow )
    {
        /* #i64117# some themes set the background pixmap VERY frequently */
        GdkWindow* pWin = GTK_WIDGET(m_pWindow)->window;
        if( pWin )
        {
            aWin = GDK_WINDOW_XWINDOW(pWin);
            if( aWin != None )
                XSetWindowBackgroundPixmap( GtkSalFrame::getDisplay()->GetDisplay(),
                                            aWin,
                                            None );
        }
    }
    X11SalGraphics::copyBits( rPosAry, pSrcGraphics );
    if( pFrame && pFrame->getBackgroundPixmap() != None )
        XSetWindowBackgroundPixmap( GtkSalFrame::getDisplay()->GetDisplay(),
                                    aWin,
                                    pFrame->getBackgroundPixmap() );
}

/*
 * IsNativeControlSupported()
 *
 *  Returns true if the platform supports native
 *  drawing of the control defined by nPart
 */
bool GtkSalGraphics::IsNativeControlSupported( ControlType nType, ControlPart nPart )
{
    switch(nType)
    {
        case CTRL_PUSHBUTTON:
        case CTRL_RADIOBUTTON:
        case CTRL_CHECKBOX:
        case CTRL_TOOLTIP:
        case CTRL_PROGRESS:
        case CTRL_LISTNODE:
        case CTRL_LISTNET:
            if(nPart==PART_ENTIRE_CONTROL)
                return true;
            break;

        case CTRL_SCROLLBAR:
            if(nPart==PART_DRAW_BACKGROUND_HORZ || nPart==PART_DRAW_BACKGROUND_VERT ||
               nPart==PART_ENTIRE_CONTROL       || nPart==HAS_THREE_BUTTONS)
                return true;
            break;

        case CTRL_EDITBOX:
        case CTRL_MULTILINE_EDITBOX:
        case CTRL_COMBOBOX:
            if(nPart==PART_ENTIRE_CONTROL || nPart==HAS_BACKGROUND_TEXTURE)
                return true;
            break;

        case CTRL_SPINBOX:
            if(nPart==PART_ENTIRE_CONTROL || nPart==PART_ALL_BUTTONS || nPart==HAS_BACKGROUND_TEXTURE)
                return true;
            break;

        case CTRL_SPINBUTTONS:
            if(nPart==PART_ENTIRE_CONTROL || nPart==PART_ALL_BUTTONS)
                return true;
            break;

        case CTRL_FRAME:
        case CTRL_WINDOW_BACKGROUND:
            return true;

        case CTRL_TAB_ITEM:
        case CTRL_TAB_PANE:
        case CTRL_TAB_BODY:
            if(nPart==PART_ENTIRE_CONTROL || nPart==PART_TABS_DRAW_RTL)
                return true;
            break;

        case CTRL_LISTBOX:
            if(nPart==PART_ENTIRE_CONTROL || nPart==PART_WINDOW || nPart==HAS_BACKGROUND_TEXTURE)
                return true;
            break;

        case CTRL_TOOLBAR:
            if( nPart==PART_ENTIRE_CONTROL
                ||  nPart==PART_DRAW_BACKGROUND_HORZ
                ||  nPart==PART_DRAW_BACKGROUND_VERT
                ||  nPart==PART_THUMB_HORZ
                ||  nPart==PART_THUMB_VERT
                ||  nPart==PART_BUTTON
                ||  nPart==PART_SEPARATOR_HORZ
                ||  nPart==PART_SEPARATOR_VERT
                )
                return true;
            break;

        case CTRL_MENUBAR:
            if(nPart==PART_ENTIRE_CONTROL || nPart==PART_MENU_ITEM)
                return true;
            break;

        case CTRL_MENU_POPUP:
            if (nPart==PART_ENTIRE_CONTROL
                ||  nPart==PART_MENU_ITEM
                ||  nPart==PART_MENU_ITEM_CHECK_MARK
                ||  nPart==PART_MENU_ITEM_RADIO_MARK
                ||  nPart==PART_MENU_SEPARATOR
                ||  nPart==PART_MENU_SUBMENU_ARROW
            )
                return true;
            break;

        case CTRL_SLIDER:
            if(nPart == PART_TRACK_HORZ_AREA || nPart == PART_TRACK_VERT_AREA)
                return true;
            break;

        case CTRL_FIXEDLINE:
            if(nPart == PART_SEPARATOR_VERT || nPart == PART_SEPARATOR_HORZ)
                return true;
            break;

        case CTRL_LISTHEADER:
            if(nPart == PART_BUTTON || nPart == PART_ARROW)
                return true;
            break;
    }

    return false;
}

/*
 * HitTestNativeControl()
 *
 *  bIsInside is set to true if aPos is contained within the
 *  given part of the control, whose bounding region is
 *  given by rControlRegion (in VCL frame coordinates).
 *
 *  returns whether bIsInside was really set.
 */
bool GtkSalGraphics::hitTestNativeControl( ControlType        nType,
                                ControlPart        nPart,
                                const Rectangle&        rControlRegion,
                                const Point&        aPos,
                                bool&            rIsInside )
{
    if ( ( nType == CTRL_SCROLLBAR ) &&
         ( ( nPart == PART_BUTTON_UP ) ||
           ( nPart == PART_BUTTON_DOWN ) ||
           ( nPart == PART_BUTTON_LEFT ) ||
           ( nPart == PART_BUTTON_RIGHT ) ) )
    {
        NWEnsureGTKScrollbars( m_nXScreen );

        // Grab some button style attributes
        gboolean has_forward;
        gboolean has_forward2;
        gboolean has_backward;
        gboolean has_backward2;

        gtk_widget_style_get( gWidgetData[m_nXScreen].gScrollHorizWidget,
                              "has-forward-stepper", &has_forward,
                              "has-secondary-forward-stepper", &has_forward2,
                              "has-backward-stepper", &has_backward,
                              "has-secondary-backward-stepper", &has_backward2,
                              nullptr );
        Rectangle aForward;
        Rectangle aBackward;

        rIsInside = false;

        ControlPart nCounterPart = 0;
        if ( nPart == PART_BUTTON_UP )
            nCounterPart = PART_BUTTON_DOWN;
        else if ( nPart == PART_BUTTON_DOWN )
            nCounterPart = PART_BUTTON_UP;
        else if ( nPart == PART_BUTTON_LEFT )
            nCounterPart = PART_BUTTON_RIGHT;
        else if ( nPart == PART_BUTTON_RIGHT )
            nCounterPart = PART_BUTTON_LEFT;

        aBackward = NWGetScrollButtonRect( m_nXScreen, nPart, rControlRegion );
        aForward = NWGetScrollButtonRect( m_nXScreen, nCounterPart, rControlRegion );

        if ( has_backward && has_forward2 )
        {
            Size aSize( aBackward.GetSize() );
            if ( ( nPart == PART_BUTTON_UP ) || ( nPart == PART_BUTTON_DOWN ) )
                aSize.setHeight( aBackward.GetHeight() / 2 );
            else
                aSize.setWidth( aBackward.GetWidth() / 2 );
            aBackward.SetSize( aSize );

            if ( nPart == PART_BUTTON_DOWN )
                aBackward.Move( 0, aBackward.GetHeight() / 2 );
            else if ( nPart == PART_BUTTON_RIGHT )
                aBackward.Move( aBackward.GetWidth() / 2, 0 );
        }

        if ( has_backward2 && has_forward )
        {
            Size aSize( aForward.GetSize() );
            if ( ( nPart == PART_BUTTON_UP ) || ( nPart == PART_BUTTON_DOWN ) )
                aSize.setHeight( aForward.GetHeight() / 2 );
            else
                aSize.setWidth( aForward.GetWidth() / 2 );
            aForward.SetSize( aSize );

            if ( nPart == PART_BUTTON_DOWN )
                aForward.Move( 0, aForward.GetHeight() / 2 );
            else if ( nPart == PART_BUTTON_RIGHT )
                aForward.Move( aForward.GetWidth() / 2, 0 );
        }

        if ( ( nPart == PART_BUTTON_UP ) || ( nPart == PART_BUTTON_LEFT ) )
        {
            if ( has_backward )
                rIsInside |= aBackward.IsInside( aPos );
            if ( has_backward2 )
                rIsInside |= aForward.IsInside( aPos );
        }
        else
        {
            if ( has_forward )
                rIsInside |= aBackward.IsInside( aPos );
            if ( has_forward2 )
                rIsInside |= aForward.IsInside( aPos );
        }
        return true;
    }

    if( IsNativeControlSupported(nType, nPart) )
    {
        rIsInside = rControlRegion.IsInside( aPos );
        return true;
    }
    else
    {
        return false;
    }
}

/*
 * DrawNativeControl()
 *
 *  Draws the requested control described by nPart/nState.
 *
 *  rControlRegion:    The bounding region of the complete control in VCL frame coordinates.
 *  aValue:          An optional value (tristate/numerical/string)
 *  rCaption:      A caption or title string (like button text etc)
 */
bool GtkSalGraphics::drawNativeControl(ControlType nType, ControlPart nPart,
        const Rectangle& rControlRegion, ControlState nState,
        const ImplControlValue& aValue, const OUString& rCaption)
{
    // get a GC with current clipping region set
    GetFontGC();

    // theme changed ?
    if( GtkSalGraphics::bThemeChanged )
    {
        // invalidate caches
        for( size_t i = 0; i < gWidgetData.size(); i++ )
            if( gWidgetData[i].gNWPixmapCacheList )
                gWidgetData[i].gNWPixmapCacheList->ThemeChanged();
        GtkSalGraphics::bThemeChanged = false;
    }

    Rectangle aCtrlRect( rControlRegion );
    vcl::Region aClipRegion( m_aClipRegion );
    if( aClipRegion.IsNull() )
        aClipRegion = aCtrlRect;

    Rectangle aPixmapRect;

    // make pixmap a little larger since some themes draw decoration
    // outside the rectangle, see e.g. checkbox
    aPixmapRect = Rectangle(Point( aCtrlRect.Left()-1, aCtrlRect.Top()-1 ),
                            Size( aCtrlRect.GetWidth()+2, aCtrlRect.GetHeight()+2) );

    ControlCacheKey aControlCacheKey(nType, nPart, nState, aPixmapRect.GetSize());
    if (aControlCacheKey.canCacheControl()
        && TryRenderCachedNativeControl(aControlCacheKey, aPixmapRect.Left(), aPixmapRect.Top()))
    {
        return true;
    }

    clipList aClip;
    int nPasses = 0;
    GdkDrawable* gdkDrawable[2];
    std::unique_ptr<GdkX11Pixmap> xPixmap;
    std::unique_ptr<GdkX11Pixmap> xMask;

    if ((bNeedPixmapPaint || (nState & ControlState::DOUBLEBUFFERING))
        && nType != CTRL_SCROLLBAR
        && nType != CTRL_SPINBOX
        && nType != CTRL_TAB_ITEM
        && nType != CTRL_TAB_PANE
        && nType != CTRL_PROGRESS
        && ! (nType == CTRL_TOOLBAR && (nPart == PART_THUMB_HORZ || nPart == PART_THUMB_VERT) )
        )
    {
        if( bNeedTwoPasses )
        {
            xPixmap.reset( NWGetPixmapFromScreen( aPixmapRect, BG_WHITE ) );
            xMask.reset( NWGetPixmapFromScreen( aPixmapRect, BG_BLACK ) );
            if( !xPixmap || !xMask )
                return false;
            nPasses = 2;
            gdkDrawable[0] = xPixmap->GetGdkDrawable();
            gdkDrawable[1] = xMask->GetGdkDrawable();
        }
        else
        {
            xPixmap.reset( NWGetPixmapFromScreen( aPixmapRect, BG_FILL ) );
            if( !xPixmap )
                return false;
            nPasses = 1;
            gdkDrawable[0] = xPixmap->GetGdkDrawable();
        }

        aCtrlRect = Rectangle( Point(1,1), aCtrlRect.GetSize() );
        aClip.push_back( aCtrlRect );
    }
    else
    {
        nPasses = 1;
        gdkDrawable[0] = GDK_DRAWABLE( GetGdkWindow() );
        RectangleVector aRectangles;
        aClipRegion.GetRegionRectangles(aRectangles);

        for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
        {
            Rectangle aPaintRect = aCtrlRect.GetIntersection(*aRectIter);
            if( aPaintRect.IsEmpty() )
                continue;
            aClip.push_back( aPaintRect );
        }
    }

    bool returnVal = false;

    for( int i = 0; i < nPasses; ++i )
    {
        assert(gdkDrawable[i] && "rhbz#1050162");
        if( gdkDrawable[i] == nullptr )
            return false;

        returnVal = DoDrawNativeControl(gdkDrawable[i], nType, nPart, aCtrlRect, aClip,
                                        nState, aValue, rCaption, aControlCacheKey);
        if( !returnVal )
            break;
    }

    if( xPixmap )
        returnVal = returnVal && RenderAndCacheNativeControl(xPixmap.get(), xMask.get(),
                                                             aPixmapRect.Left(), aPixmapRect.Top(),
                                                             aControlCacheKey);

    return returnVal;
}


bool GtkSalGraphics::DoDrawNativeControl(
                            GdkDrawable* pDrawable,
                            ControlType nType,
                            ControlPart nPart,
                            const Rectangle& aCtrlRect,
                            const clipList& aClip,
                            ControlState nState,
                            const ImplControlValue& aValue,
                            const OUString& rCaption,
                            ControlCacheKey& rControlCacheKey)
{
    if ( (nType==CTRL_PUSHBUTTON) && (nPart==PART_ENTIRE_CONTROL) )
    {
        return NWPaintGTKButton( pDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption );
    }
    else if ( (nType==CTRL_RADIOBUTTON) && (nPart==PART_ENTIRE_CONTROL) )
    {
        return NWPaintGTKRadio( pDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption );
    }
    else if ( (nType==CTRL_CHECKBOX) && (nPart==PART_ENTIRE_CONTROL) )
    {
        return NWPaintGTKCheck( pDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption );
    }
    else if ( (nType==CTRL_SCROLLBAR) && ((nPart==PART_DRAW_BACKGROUND_HORZ) || (nPart==PART_DRAW_BACKGROUND_VERT)) )
    {
        return NWPaintGTKScrollbar( nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption );
    }
    else if ( ((nType==CTRL_EDITBOX) && ((nPart==PART_ENTIRE_CONTROL) || (nPart==HAS_BACKGROUND_TEXTURE)) )
        || ((nType==CTRL_SPINBOX) && (nPart==HAS_BACKGROUND_TEXTURE))
    || ((nType==CTRL_COMBOBOX) && (nPart==HAS_BACKGROUND_TEXTURE))
    || ((nType==CTRL_LISTBOX) && (nPart==HAS_BACKGROUND_TEXTURE)) )
    {
        return NWPaintGTKEditBox( pDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption );
    }
    else if ( ((nType==CTRL_MULTILINE_EDITBOX) && ((nPart==PART_ENTIRE_CONTROL) || (nPart==HAS_BACKGROUND_TEXTURE)) ) )
    {
        return NWPaintGTKEditBox( pDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption );
    }
    else if ( ((nType==CTRL_SPINBOX) || (nType==CTRL_SPINBUTTONS))
        && ((nPart==PART_ENTIRE_CONTROL) || (nPart==PART_ALL_BUTTONS)) )
    {
        return NWPaintGTKSpinBox(nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption, rControlCacheKey);
    }
    else if ( (nType == CTRL_COMBOBOX) &&
        ( (nPart==PART_ENTIRE_CONTROL)
        ||(nPart==PART_BUTTON_DOWN)
        ) )
    {
        return NWPaintGTKComboBox( pDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption );
    }
    else if ( (nType==CTRL_TAB_ITEM) || (nType==CTRL_TAB_PANE) || (nType==CTRL_TAB_BODY) )
    {
        if ( nType == CTRL_TAB_BODY )
            return true;
        else
            return NWPaintGTKTabItem( nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption);
    }
    else if ( (nType==CTRL_LISTBOX) && ((nPart==PART_ENTIRE_CONTROL) || (nPart==PART_WINDOW)) )
    {
        return NWPaintGTKListBox( pDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption );
    }
    else if ( nType== CTRL_TOOLBAR )
    {
        return NWPaintGTKToolbar( pDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption );
    }
    else if ( nType== CTRL_MENUBAR )
    {
        return NWPaintGTKMenubar( pDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption );
    }
    else if(    (nType == CTRL_MENU_POPUP)
        && (  (nPart == PART_ENTIRE_CONTROL)
    || (nPart == PART_MENU_ITEM)
    || (nPart == PART_MENU_ITEM_CHECK_MARK)
    || (nPart == PART_MENU_ITEM_RADIO_MARK)
    || (nPart == PART_MENU_SEPARATOR)
    || (nPart == PART_MENU_SUBMENU_ARROW)
    )
    )
    {
        return NWPaintGTKPopupMenu( pDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption );
    }
    else if( (nType == CTRL_TOOLTIP) && (nPart == PART_ENTIRE_CONTROL) )
    {
        return NWPaintGTKTooltip( pDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption );
    }
    else if( (nType == CTRL_PROGRESS) && (nPart == PART_ENTIRE_CONTROL) )
    {
        return NWPaintGTKProgress( pDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption );
    }
    else if( (nType == CTRL_LISTNODE) && (nPart == PART_ENTIRE_CONTROL) )
    {
        return NWPaintGTKListNode( pDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption );
    }
    else if( (nType == CTRL_LISTNET) && (nPart == PART_ENTIRE_CONTROL) )
    {
        // don't actually draw anything; gtk treeviews do not draw lines
        return TRUE;
    }
    else if( nType == CTRL_SLIDER )
    {
        return NWPaintGTKSlider( pDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption );
    }
    else if( nType == CTRL_WINDOW_BACKGROUND )
    {
        return NWPaintGTKWindowBackground( pDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption );
    }
    else if( nType == CTRL_FIXEDLINE )
    {
        return NWPaintGTKFixedLine( pDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption );
    }
    else if(nType==CTRL_FRAME)
    {
        return NWPaintGTKFrame( pDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption);
    }
    else if(nType==CTRL_LISTHEADER)
    {
        if(nPart == PART_BUTTON)
            return NWPaintGTKListHeader( pDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption );
        else if(nPart == PART_ARROW)
            return NWPaintGTKArrow( pDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rCaption );
    }

    return false;
}

/*
 * GetNativeControlRegion()
 *
 *  If the return value is true, rNativeBoundingRegion
 *  contains the true bounding region covered by the control
 *  including any adornment, while rNativeContentRegion contains the area
 *  within the control that can be safely drawn into without drawing over
 *  the borders of the control.
 *
 *  rControlRegion:    The bounding region of the control in VCL frame coordinates.
 *  aValue:        An optional value (tristate/numerical/string)
 *  rCaption:        A caption or title string (like button text etc)
 */
bool GtkSalGraphics::getNativeControlRegion(  ControlType nType,
                                ControlPart nPart,
                                const Rectangle& rControlRegion,
                                ControlState nState,
                                const ImplControlValue& aValue,
                                const OUString& rCaption,
                                Rectangle &rNativeBoundingRegion,
                                Rectangle &rNativeContentRegion )
{
    bool returnVal = false;

    if ( (nType==CTRL_PUSHBUTTON) && (nPart==PART_ENTIRE_CONTROL)
        && (rControlRegion.GetWidth() > 16)
    && (rControlRegion.GetHeight() > 16) )
    {
        rNativeBoundingRegion = NWGetButtonArea( m_nXScreen, nType, nPart, rControlRegion,
        nState, aValue, rCaption );
        rNativeContentRegion = rControlRegion;

        returnVal = true;
    }
    if (nType == CTRL_TAB_ITEM && nPart == PART_ENTIRE_CONTROL)
    {
        rNativeBoundingRegion = NWGetTabItemRect(m_nXScreen, rControlRegion);
        rNativeContentRegion = rNativeBoundingRegion;
        returnVal = true;
    }
    if ( (nType==CTRL_COMBOBOX) && ((nPart==PART_BUTTON_DOWN) || (nPart==PART_SUB_EDIT)) )
    {
        rNativeBoundingRegion = NWGetComboBoxButtonRect( m_nXScreen, nType, nPart, rControlRegion, nState,
        aValue, rCaption );
        rNativeContentRegion = rNativeBoundingRegion;

        returnVal = true;
    }
    if ( (nType==CTRL_SPINBOX) && ((nPart==PART_BUTTON_UP) || (nPart==PART_BUTTON_DOWN) || (nPart==PART_SUB_EDIT)) )
    {

        rNativeBoundingRegion = NWGetSpinButtonRect( m_nXScreen, nType, nPart, rControlRegion, nState,
        aValue, rCaption );
        rNativeContentRegion = rNativeBoundingRegion;

        returnVal = true;
    }
    if ( (nType==CTRL_LISTBOX) && ((nPart==PART_BUTTON_DOWN) || (nPart==PART_SUB_EDIT)) )
    {
        rNativeBoundingRegion = NWGetListBoxButtonRect( m_nXScreen, nType, nPart, rControlRegion, nState,
        aValue, rCaption );
        rNativeContentRegion = rNativeBoundingRegion;

        returnVal = true;
    }
    if ( (nType==CTRL_TOOLBAR) &&
        ((nPart==PART_DRAW_BACKGROUND_HORZ)    ||
        (nPart==PART_DRAW_BACKGROUND_VERT)    ||
        (nPart==PART_THUMB_HORZ)            ||
        (nPart==PART_THUMB_VERT)            ||
        (nPart==PART_BUTTON)
        ))
    {
        rNativeBoundingRegion = NWGetToolbarRect( m_nXScreen, nType, nPart, rControlRegion, nState, aValue, rCaption );
        rNativeContentRegion = rNativeBoundingRegion;
        returnVal = true;
    }
    if ( (nType==CTRL_SCROLLBAR) && ((nPart==PART_BUTTON_LEFT) || (nPart==PART_BUTTON_RIGHT) ||
        (nPart==PART_BUTTON_UP) || (nPart==PART_BUTTON_DOWN)  ) )
    {
        rNativeBoundingRegion = NWGetScrollButtonRect( m_nXScreen, nPart, rControlRegion );
        rNativeContentRegion = rNativeBoundingRegion;

        //See fdo#33523, possibly makes sense to do this test for all return values
        if (!rNativeContentRegion.GetWidth())
            rNativeContentRegion.Right() = rNativeContentRegion.Left() + 1;
        if (!rNativeContentRegion.GetHeight())
            rNativeContentRegion.Bottom() = rNativeContentRegion.Top() + 1;
        returnVal = true;
    }
    if( (nType == CTRL_MENUBAR) && (nPart == PART_ENTIRE_CONTROL) )
    {
        NWEnsureGTKMenubar( m_nXScreen );
        GtkRequisition aReq;
        gtk_widget_size_request( gWidgetData[m_nXScreen].gMenubarWidget, &aReq );
        Rectangle aMenuBarRect = rControlRegion;
        aMenuBarRect = Rectangle( aMenuBarRect.TopLeft(),
                                  Size( aMenuBarRect.GetWidth(), aReq.height+1 ) );
        rNativeBoundingRegion = aMenuBarRect;
        rNativeContentRegion = rNativeBoundingRegion;
        returnVal = true;
    }
    if( nType == CTRL_MENU_POPUP )
    {
        if( (nPart == PART_MENU_ITEM_CHECK_MARK) ||
            (nPart == PART_MENU_ITEM_RADIO_MARK) )
        {
            NWEnsureGTKMenu( m_nXScreen );

            gint indicator_size = 0;
            GtkWidget* pWidget = (nPart == PART_MENU_ITEM_CHECK_MARK) ?
                gWidgetData[m_nXScreen].gMenuItemCheckMenuWidget : gWidgetData[m_nXScreen].gMenuItemRadioMenuWidget;
            gtk_widget_style_get( pWidget,
                                  "indicator_size", &indicator_size,
                                  nullptr );
            rNativeBoundingRegion = rControlRegion;
            Rectangle aIndicatorRect( Point( 0,
                                             (rControlRegion.GetHeight()-indicator_size)/2),
                                      Size( indicator_size, indicator_size ) );
            rNativeContentRegion = aIndicatorRect;
            returnVal = true;
        }
        else if( nPart == PART_MENU_SUBMENU_ARROW )
        {
            GtkWidget* widget = gWidgetData[m_nXScreen].gMenuItemMenuWidget;
            GtkWidget* child;
            PangoContext *context;
            PangoFontMetrics *metrics;
            gint arrow_size;
            gint arrow_extent;
            guint horizontal_padding;
            gfloat arrow_scaling = 0.4; // Default for early GTK versions

            gtk_widget_style_get( widget,
                                  "horizontal-padding", &horizontal_padding,
                                  NULL );

            // Use arrow-scaling property if available (2.15+), avoid warning otherwise
            if ( gtk_widget_class_find_style_property( GTK_WIDGET_GET_CLASS( widget ),
                                                       "arrow-scaling" ) )
            {
                gtk_widget_style_get( widget,
                                      "arrow-scaling", &arrow_scaling,
                                      NULL );
            }

            child = GTK_BIN( widget )->child;

            context = gtk_widget_get_pango_context( child );
            metrics = pango_context_get_metrics( context,
                                                 child->style->font_desc,
                                                 pango_context_get_language( context ) );

            arrow_size = ( PANGO_PIXELS( pango_font_metrics_get_ascent( metrics ) +
                                         pango_font_metrics_get_descent( metrics ) ));

            pango_font_metrics_unref( metrics );

            arrow_extent = static_cast<gint>(arrow_size * arrow_scaling);

            rNativeContentRegion = Rectangle( Point( 0, 0 ),
                                              Size( arrow_extent, arrow_extent ));
            rNativeBoundingRegion = Rectangle( Point( 0, 0 ),
                                               Size( arrow_extent + horizontal_padding, arrow_extent ));
            returnVal = true;
        }
    }
    if( (nType == CTRL_RADIOBUTTON || nType == CTRL_CHECKBOX) )
    {
        NWEnsureGTKRadio( m_nXScreen );
        NWEnsureGTKCheck( m_nXScreen );
        GtkWidget* widget = (nType == CTRL_RADIOBUTTON) ? gWidgetData[m_nXScreen].gRadioWidget : gWidgetData[m_nXScreen].gCheckWidget;
        gint indicator_size, indicator_spacing, focusPad, focusWidth;
        gtk_widget_style_get( widget,
                              "indicator_size", &indicator_size,
                              "indicator_spacing", &indicator_spacing,
                              "focus-line-width", &focusWidth,
                              "focus-padding", &focusPad,
                              nullptr);
        indicator_size += 2*indicator_spacing + 2*(focusWidth + focusWidth);
        rNativeBoundingRegion = rControlRegion;
        Rectangle aIndicatorRect( Point( 0,
                                         (rControlRegion.GetHeight()-indicator_size)/2),
                                  Size( indicator_size, indicator_size ) );
        rNativeContentRegion = aIndicatorRect;
        returnVal = true;
    }
    if( (nType == CTRL_EDITBOX || nType == CTRL_SPINBOX || nType == CTRL_COMBOBOX) && nPart == PART_ENTIRE_CONTROL )
    {
        NWEnsureGTKEditBox( m_nXScreen );
        GtkWidget* widget = gWidgetData[m_nXScreen].gEditBoxWidget;
        GtkRequisition aReq;
        gtk_widget_size_request( widget, &aReq );
        Rectangle aEditRect = rControlRegion;
        long nHeight = (aEditRect.GetHeight() > aReq.height) ? aEditRect.GetHeight() : aReq.height;
        aEditRect = Rectangle( aEditRect.TopLeft(),
                               Size( aEditRect.GetWidth(), nHeight ) );
        rNativeBoundingRegion = aEditRect;
        rNativeContentRegion = rNativeBoundingRegion;
        returnVal = true;
    }
    if( (nType == CTRL_SLIDER) && (nPart == PART_THUMB_HORZ || nPart == PART_THUMB_VERT) )
    {
        NWEnsureGTKSlider( m_nXScreen );
        GtkWidget* widget = (nPart == PART_THUMB_HORZ) ? gWidgetData[m_nXScreen].gHScale : gWidgetData[m_nXScreen].gVScale;
        gint slider_length = 10;
        gint slider_width = 10;
        gtk_widget_style_get( widget,
                              "slider-width", &slider_width,
                              "slider-length", &slider_length,
                              nullptr);
        Rectangle aRect( rControlRegion );
        if( nPart == PART_THUMB_HORZ )
        {
            aRect.Right() = aRect.Left() + slider_length - 1;
            aRect.Bottom() = aRect.Top() + slider_width - 1;
        }
        else
        {
            aRect.Bottom() = aRect.Top() + slider_length - 1;
            aRect.Right() = aRect.Left() + slider_width - 1;
        }
        rNativeBoundingRegion = rNativeContentRegion = aRect;
        returnVal = true;
    }
    if( nType == CTRL_FRAME && nPart == PART_BORDER )
    {
        int frameWidth = getFrameWidth(gWidgetData[m_nXScreen].gFrame);
        rNativeBoundingRegion = rControlRegion;
        DrawFrameFlags nStyle = static_cast<DrawFrameFlags>(aValue.getNumericVal() & 0xfff0);
        int x1=rControlRegion.Left();
        int y1=rControlRegion.Top();
        int x2=rControlRegion.Right();
        int y2=rControlRegion.Bottom();

        if( nStyle & DrawFrameFlags::NoDraw )
        {
            rNativeContentRegion = Rectangle(x1+frameWidth,
                                             y1+frameWidth,
                                             x2-frameWidth,
                                             y2-frameWidth);
        }
        else
            rNativeContentRegion = rControlRegion;
        returnVal=true;
    }

    return returnVal;
}

/************************************************************************
 * Individual control drawing functions
 ************************************************************************/

// macros to call before and after the rendering code for a widget
// it takes care of creating the needed pixmaps
#define BEGIN_PIXMAP_RENDER(aRect, gdkPixmap) \
    std::unique_ptr<GdkX11Pixmap> _pixmap, _mask; \
    int _nPasses = 0; \
    if( bNeedTwoPasses ) \
    { \
        _nPasses = 2; \
        _pixmap.reset( NWGetPixmapFromScreen( aRect, BG_WHITE ) ); \
        _mask.reset( NWGetPixmapFromScreen( aRect, BG_BLACK ) ); \
    } \
    else \
    { \
        _nPasses = 1; \
        _pixmap.reset( NWGetPixmapFromScreen( aRect, BG_FILL ) ); \
    } \
    if( !_pixmap || ( bNeedTwoPasses && !_mask ) ) \
        return false; \
    for( int i = 0; i < _nPasses; ++i ) \
    { \
        GdkPixmap* gdkPixmap = (i == 0) ? _pixmap->GetGdkPixmap() \
                                        : _mask->GetGdkPixmap();

#define END_PIXMAP_RENDER(aRect) \
    } \
    if( !NWRenderPixmapToScreen( _pixmap.get(), _mask.get(), aRect ) ) \
        return false;

#define END_PIXMAP_RENDER_WITH_CONTROL_KEY(aRect, aControlKey) \
    } \
    if( !RenderAndCacheNativeControl( _pixmap.get(), _mask.get(), aRect.Left(), aRect.Top(), aControlKey ) ) \
        return false;

// same as above but with pixmaps that should be kept for caching
#define BEGIN_CACHE_PIXMAP_RENDER(aRect, pixmap, mask, gdkPixmap) \
    int _nPasses = 0; \
    if( bNeedTwoPasses ) \
    { \
        _nPasses = 2; \
        pixmap = NWGetPixmapFromScreen( aRect, BG_WHITE ); \
        mask = NWGetPixmapFromScreen( aRect, BG_BLACK ); \
    } \
    else \
    { \
        _nPasses = 1; \
        pixmap = NWGetPixmapFromScreen( aRect, BG_FILL ); \
        mask = nullptr; \
    } \
    if( !pixmap || ( bNeedTwoPasses && !mask ) ) \
        return false; \
    for( int i = 0; i < _nPasses; ++i ) \
    { \
        GdkPixmap* gdkPixmap = (i == 0) ? pixmap->GetGdkPixmap() \
                                        : mask->GetGdkPixmap();

#define END_CACHE_PIXMAP_RENDER(aRect, pixmap, mask) \
    } \
    if( !NWRenderPixmapToScreen( pixmap, mask, aRect ) ) \
        return false;

bool GtkSalGraphics::NWPaintGTKArrow(
            GdkDrawable* gdkDrawable,
            ControlType, ControlPart,
            const Rectangle& rControlRectangle,
            const clipList& rClipList,
            ControlState nState, const ImplControlValue& aValue,
            const OUString& )
{
    GtkArrowType arrowType(aValue.getNumericVal()&1?GTK_ARROW_DOWN:GTK_ARROW_UP);
    GtkStateType stateType(nState&ControlState::PRESSED?GTK_STATE_ACTIVE:GTK_STATE_NORMAL);

    GdkRectangle clipRect;
    for( clipList::const_iterator it = rClipList.begin(); it != rClipList.end(); ++it )
    {
        clipRect.x = it->Left();
        clipRect.y = it->Top();
        clipRect.width = it->GetWidth();
        clipRect.height = it->GetHeight();

        gtk_paint_arrow(m_pWindow->style,gdkDrawable,stateType,GTK_SHADOW_NONE,&clipRect,
                m_pWindow,"arrow",arrowType,true,
                rControlRectangle.Left(),
                rControlRectangle.Top(),
                rControlRectangle.GetWidth(),
                rControlRectangle.GetHeight());
    }
    return true;
}

bool GtkSalGraphics::NWPaintGTKListHeader(
            GdkDrawable* gdkDrawable,
            ControlType, ControlPart,
            const Rectangle& rControlRectangle,
            const clipList& rClipList,
            ControlState nState, const ImplControlValue&,
            const OUString& )
{
    GtkStateType    stateType;
    GtkShadowType    shadowType;
    NWEnsureGTKTreeView( m_nXScreen );
    GtkWidget* &treeview(gWidgetData[m_nXScreen].gTreeView);
    GtkTreeViewColumn* column=gtk_tree_view_get_column(GTK_TREE_VIEW(treeview),0);
    GtkWidget* button=gtk_tree_view_column_get_widget(column);
    while(button && !GTK_IS_BUTTON(button))
        button=gtk_widget_get_parent(button);
    if(!button)
        // Shouldn't ever happen
        return false;
    gtk_widget_realize(button);
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );
    NWSetWidgetState( button, nState, stateType );

    GdkRectangle clipRect;
    for( clipList::const_iterator it = rClipList.begin(); it != rClipList.end(); ++it )
    {
        clipRect.x = it->Left();
        clipRect.y = it->Top();
        clipRect.width = it->GetWidth();
        clipRect.height = it->GetHeight();

        gtk_paint_box(button->style,gdkDrawable,stateType,shadowType,&clipRect,
                button,"button",
                rControlRectangle.Left()-1,
                rControlRectangle.Top(),
                rControlRectangle.GetWidth()+1,
                rControlRectangle.GetHeight());
    }
    return true;
}

bool GtkSalGraphics::NWPaintGTKFixedLine(
            GdkDrawable* gdkDrawable,
            ControlType, ControlPart nPart,
            const Rectangle& rControlRectangle,
            const clipList&,
            ControlState, const ImplControlValue&,
            const OUString& )
{
    if(nPart == PART_SEPARATOR_HORZ)
        gtk_paint_hline(m_pWindow->style,gdkDrawable,GTK_STATE_NORMAL,nullptr,m_pWindow,"hseparator",rControlRectangle.Left(),rControlRectangle.Right(),rControlRectangle.Top());
    else
        gtk_paint_vline(m_pWindow->style,gdkDrawable,GTK_STATE_NORMAL,nullptr,m_pWindow,"vseparator",rControlRectangle.Top(),rControlRectangle.Bottom(),rControlRectangle.Left());

    return true;
}

bool GtkSalGraphics::NWPaintGTKFrame(
            GdkDrawable* gdkDrawable,
            ControlType, ControlPart,
            const Rectangle& rControlRectangle,
            const clipList& rClipList,
            ControlState /* nState */, const ImplControlValue& aValue,
            const OUString& )
{
    GdkRectangle clipRect;
    int frameWidth=getFrameWidth(gWidgetData[m_nXScreen].gFrame);
    GtkShadowType shadowType=GTK_SHADOW_IN;
    DrawFrameStyle nStyle = static_cast<DrawFrameStyle>(aValue.getNumericVal() & 0x0f);
    if( nStyle == DrawFrameStyle::In )
        shadowType=GTK_SHADOW_OUT;
    if( nStyle == DrawFrameStyle::Out )
        shadowType=GTK_SHADOW_IN;

    for( clipList::const_iterator it = rClipList.begin(); it != rClipList.end(); ++it )
    {
        clipRect.x = it->Left();
        clipRect.y = it->Top();
        clipRect.width = it->GetWidth();
        clipRect.height = it->GetHeight();

        // Draw background first

        // Top
        gtk_paint_flat_box(m_pWindow->style,gdkDrawable,GTK_STATE_NORMAL,GTK_SHADOW_OUT,&clipRect,
                         m_pWindow,"base",
                         rControlRectangle.Left(),
                         rControlRectangle.Top(),
                         rControlRectangle.GetWidth(),
                         frameWidth);
        // Bottom
        gtk_paint_flat_box(m_pWindow->style,gdkDrawable,GTK_STATE_NORMAL,GTK_SHADOW_OUT,&clipRect,
                         m_pWindow,"base",
                         rControlRectangle.Left(),
                         rControlRectangle.Top()+rControlRectangle.GetHeight()-frameWidth,
                         rControlRectangle.GetWidth(),
                         frameWidth);
        // Left
        gtk_paint_flat_box(m_pWindow->style,gdkDrawable,GTK_STATE_NORMAL,GTK_SHADOW_OUT,&clipRect,
                         m_pWindow,"base",
                         rControlRectangle.Left(),
                         rControlRectangle.Top(),
                         2*frameWidth,
                         rControlRectangle.GetHeight());
        // Right
        gtk_paint_flat_box(m_pWindow->style,gdkDrawable,GTK_STATE_NORMAL,GTK_SHADOW_OUT,&clipRect,
                         m_pWindow,"base",
                         rControlRectangle.Left()+rControlRectangle.GetWidth()-frameWidth,
                         rControlRectangle.Top(),
                         2*frameWidth,
                         rControlRectangle.GetHeight());

        // Now render the frame
        gtk_paint_shadow(gWidgetData[m_nXScreen].gFrame->style,gdkDrawable,GTK_STATE_NORMAL,shadowType,&clipRect,
                         gWidgetData[m_nXScreen].gFrame,"base",
                         rControlRectangle.Left(),
                         rControlRectangle.Top(),
                         rControlRectangle.GetWidth(),
                         rControlRectangle.GetHeight());
    }

    return true;
}

bool GtkSalGraphics::NWPaintGTKWindowBackground(
            GdkDrawable* gdkDrawable,
            ControlType, ControlPart,
            const Rectangle& rControlRectangle,
            const clipList& rClipList,
            ControlState /* nState */, const ImplControlValue&,
            const OUString& )
{
    GdkRectangle clipRect;
    for( clipList::const_iterator it = rClipList.begin(); it != rClipList.end(); ++it )
    {
        clipRect.x = it->Left();
        clipRect.y = it->Top();
        clipRect.width = it->GetWidth();
        clipRect.height = it->GetHeight();

        gtk_paint_flat_box(m_pWindow->style,gdkDrawable,GTK_STATE_NORMAL,GTK_SHADOW_NONE,&clipRect,
                           m_pWindow,"base",
                           rControlRectangle.Left(),
                           rControlRectangle.Top(),
                           rControlRectangle.GetWidth(),
                           rControlRectangle.GetHeight());
    }

    return true;
}

bool GtkSalGraphics::NWPaintGTKButtonReal(
            GtkWidget* button,
            GdkDrawable* gdkDrawable,
            ControlType, ControlPart,
            const Rectangle& rControlRectangle,
            const clipList& rClipList,
            ControlState nState, const ImplControlValue&,
            const OUString& )
{
    GtkStateType    stateType;
    GtkShadowType    shadowType;
    gboolean        interiorFocus;
    gint            focusWidth;
    gint            focusPad;
    bool            bDrawFocus = true;
    gint            x, y, w, h;
    GtkBorder        aDefBorder;
    GtkBorder*        pBorder;
    GdkRectangle    clipRect;

    NWEnsureGTKButton( m_nXScreen );
    NWEnsureGTKToolbar( m_nXScreen );

    // Flat toolbutton has a bit bigger variety of states than normal buttons, so handle it differently
    if(GTK_IS_TOGGLE_BUTTON(button))
    {
       if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
           shadowType=GTK_SHADOW_IN;
       else
           shadowType=GTK_SHADOW_OUT;

       if(nState & ControlState::ROLLOVER)
           stateType=GTK_STATE_PRELIGHT;
       else
           stateType=GTK_STATE_NORMAL;

       if(nState & ControlState::PRESSED)
       {
           stateType=GTK_STATE_ACTIVE;
           shadowType=GTK_SHADOW_IN;
       }
    }
    else
    {
        NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );
        NWSetWidgetState( gWidgetData[m_nXScreen].gBtnWidget, nState, stateType );
    }

    x = rControlRectangle.Left();
    y = rControlRectangle.Top();
    w = rControlRectangle.GetWidth();
    h = rControlRectangle.GetHeight();

    gint internal_padding = 0;
    if(GTK_IS_TOOL_ITEM(button))
    {
        gtk_widget_style_get (GTK_WIDGET (gWidgetData[m_nXScreen].gToolbarWidget),
                "internal-padding", &internal_padding,
                NULL);
        x += internal_padding/2;
        w -= internal_padding;
        stateType = GTK_STATE_PRELIGHT;
    }

    // Grab some button style attributes
    gtk_widget_style_get( gWidgetData[m_nXScreen].gBtnWidget,    "focus-line-width",    &focusWidth,
                                "focus-padding",     &focusPad,
                                 "interior_focus",    &interiorFocus,
                                nullptr );
    gtk_widget_style_get( gWidgetData[m_nXScreen].gBtnWidget,
                                "default_border",    &pBorder,
                                nullptr );

    // Make sure the border values exist, otherwise use some defaults
    if ( pBorder )
    {
        NW_gtk_border_set_from_border( aDefBorder, pBorder );
        gtk_border_free( pBorder );
    }
    else NW_gtk_border_set_from_border( aDefBorder, &aDefDefBorder );

    // If the button is too small, don't ever draw focus or grab more space
    if ( (w < 16) || (h < 16) )
        bDrawFocus = false;

    gint xi = x, yi = y, wi = w, hi = h;
    if ( (nState & ControlState::DEFAULT) && bDrawFocus )
    {
        xi += aDefBorder.left;
        yi += aDefBorder.top;
        wi -= aDefBorder.left + aDefBorder.right;
        hi -= aDefBorder.top + aDefBorder.bottom;
    }

    if ( !interiorFocus && bDrawFocus )
    {
        xi += focusWidth + focusPad;
        yi += focusWidth + focusPad;
        wi -= 2 * (focusWidth + focusPad);
        hi -= 2 * (focusWidth + focusPad);
    }
    for( clipList::const_iterator it = rClipList.begin(); it != rClipList.end(); ++it)
    {
        clipRect.x = it->Left();
        clipRect.y = it->Top();
        clipRect.width = it->GetWidth();
        clipRect.height = it->GetHeight();

        // Buttons must paint opaque since some themes have alpha-channel enabled buttons
        if(button == gWidgetData[m_nXScreen].gToolbarButtonWidget)
        {
            gtk_paint_box( gWidgetData[m_nXScreen].gToolbarWidget->style, gdkDrawable, GTK_STATE_NORMAL, GTK_SHADOW_NONE,
                                &clipRect, gWidgetData[m_nXScreen].gToolbarWidget, "toolbar", x, y, w, h );
        }
        else
        {
            gtk_paint_box( m_pWindow->style, gdkDrawable, GTK_STATE_NORMAL, GTK_SHADOW_NONE,
                                &clipRect, m_pWindow, "base", x, y, w, h );
        }

        if ( GTK_IS_BUTTON(button) )
        {
            if ( (nState & ControlState::DEFAULT) )
                gtk_paint_box( button->style, gdkDrawable, GTK_STATE_NORMAL, GTK_SHADOW_IN,
                               &clipRect, button, "buttondefault", x, y, w, h );

            /* don't draw "button", because it can be a tool_button, and
             * it causes some weird things, so, the default button is
             * just fine */
            gtk_paint_box( button->style, gdkDrawable, stateType, shadowType,
                           &clipRect, button, "button", xi, yi, wi, hi );
        }
    }

    return true;
}

bool GtkSalGraphics::NWPaintGTKButton(
            GdkDrawable* gdkDrawable,
            ControlType type, ControlPart part,
            const Rectangle& rControlRectangle,
            const clipList& rClipList,
            ControlState nState, const ImplControlValue& value,
            const OUString& string)
{
        return NWPaintGTKButtonReal(
            gWidgetData[m_nXScreen].gBtnWidget,
            gdkDrawable,
            type, part,
            rControlRectangle,
            rClipList,
            nState, value,
            string );
}

static Rectangle NWGetButtonArea( SalX11Screen nScreen,
                                  ControlType, ControlPart, Rectangle aAreaRect, ControlState nState,
                                  const ImplControlValue&, const OUString& )
{
    gboolean        interiorFocus;
    gint            focusWidth;
    gint            focusPad;
    GtkBorder        aDefBorder;
    GtkBorder *    pBorder;
    bool            bDrawFocus = true;
    Rectangle        aRect;
    gint            x, y, w, h;

    NWEnsureGTKButton( nScreen );
    gtk_widget_style_get( gWidgetData[nScreen].gBtnWidget,
                                "focus-line-width",    &focusWidth,
                                "focus-padding",     &focusPad,
                                 "interior_focus",    &interiorFocus,
                                "default_border",    &pBorder,
                                nullptr );

    // Make sure the border values exist, otherwise use some defaults
    if ( pBorder )
    {
        NW_gtk_border_set_from_border( aDefBorder, pBorder );
        gtk_border_free( pBorder );
    }
    else NW_gtk_border_set_from_border( aDefBorder, &aDefDefBorder );

    x = aAreaRect.Left();
    y = aAreaRect.Top();
    w = aAreaRect.GetWidth();
    h = aAreaRect.GetHeight();

    // If the button is too small, don't ever draw focus or grab more space
    if ( (w < 16) || (h < 16) )
        bDrawFocus = false;

    if ( (nState & ControlState::DEFAULT) && bDrawFocus )
    {
        x -= aDefBorder.left;
        y -= aDefBorder.top;
        w += aDefBorder.left + aDefBorder.right;
        h += aDefBorder.top + aDefBorder.bottom;
    }

    aRect = Rectangle( Point( x, y ), Size( w, h ) );

    return aRect;
}

static Rectangle NWGetTabItemRect( SalX11Screen nScreen, Rectangle aAreaRect )
{
    NWEnsureGTKNotebook( nScreen );

    gint            x, y, w, h;

    x = aAreaRect.Left();
    y = aAreaRect.Top();
    w = aAreaRect.GetWidth();
    h = aAreaRect.GetHeight();

    gint xthickness = gWidgetData[nScreen].gNotebookWidget->style->xthickness;
    gint ythickness = gWidgetData[nScreen].gNotebookWidget->style->ythickness;

    x -= xthickness;
    y -= ythickness;
    w += xthickness*2;
    h += ythickness*2;

    return Rectangle( Point( x, y ), Size( w, h ) );
}

bool GtkSalGraphics::NWPaintGTKRadio( GdkDrawable* gdkDrawable,
                                      ControlType, ControlPart,
                                      const Rectangle& rControlRectangle,
                                      const clipList& rClipList,
                                      ControlState nState,
                                      const ImplControlValue& aValue,
                                      const OUString& )
{
    GtkStateType    stateType;
    GtkShadowType    shadowType;
    bool            isChecked = (aValue.getTristateVal()==BUTTONVALUE_ON);
    gint            x, y;
    GdkRectangle    clipRect;

    NWEnsureGTKButton( m_nXScreen );
    NWEnsureGTKRadio( m_nXScreen );
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    gint indicator_size;
    gtk_widget_style_get( gWidgetData[m_nXScreen].gRadioWidget, "indicator_size", &indicator_size, nullptr);

    x = rControlRectangle.Left() + (rControlRectangle.GetWidth()-indicator_size)/2;
    y = rControlRectangle.Top() + (rControlRectangle.GetHeight()-indicator_size)/2;

    // Set the shadow based on if checked or not so we get a freakin checkmark.
    shadowType = isChecked ? GTK_SHADOW_IN : GTK_SHADOW_OUT;
    NWSetWidgetState( gWidgetData[m_nXScreen].gRadioWidget, nState, stateType );
    NWSetWidgetState( gWidgetData[m_nXScreen].gRadioWidgetSibling, nState, stateType );

    // GTK enforces radio groups, so that if we don't have 2 buttons in the group,
    // the single button will always be active.  So we have to have 2 buttons.

    // #i59666# set the members directly where we should use
    // gtk_toggle_button_set_active. reason: there are animated themes
    // which are in active state only after a while leading to painting
    // intermediate states between active/inactive. Let's hope that
    // GtkToggleButtone stays binary compatible.
    if (!isChecked)
        GTK_TOGGLE_BUTTON(gWidgetData[m_nXScreen].gRadioWidgetSibling)->active = true;
    GTK_TOGGLE_BUTTON(gWidgetData[m_nXScreen].gRadioWidget)->active = isChecked;

    for( clipList::const_iterator it = rClipList.begin(); it != rClipList.end(); ++it )
    {
        clipRect.x = it->Left();
        clipRect.y = it->Top();
        clipRect.width = it->GetWidth();
        clipRect.height = it->GetHeight();

        gtk_paint_option( gWidgetData[m_nXScreen].gRadioWidget->style, gdkDrawable, stateType, shadowType,
                          &clipRect, gWidgetData[m_nXScreen].gRadioWidget, "radiobutton",
                          x, y, indicator_size, indicator_size );
    }

    return true;
}

bool GtkSalGraphics::NWPaintGTKCheck( GdkDrawable* gdkDrawable,
                                      ControlType, ControlPart,
                                      const Rectangle& rControlRectangle,
                                      const clipList& rClipList,
                                      ControlState nState,
                                      const ImplControlValue& aValue,
                                      const OUString& )
{
    GtkStateType    stateType;
    GtkShadowType    shadowType;
    bool            isChecked = (aValue.getTristateVal() == BUTTONVALUE_ON);
    bool            isInconsistent = (aValue.getTristateVal() == BUTTONVALUE_MIXED);
    GdkRectangle    clipRect;
    gint            x,y;

    NWEnsureGTKButton( m_nXScreen );
    NWEnsureGTKCheck( m_nXScreen );
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    gint indicator_size;
    gtk_widget_style_get( gWidgetData[m_nXScreen].gCheckWidget, "indicator_size", &indicator_size, nullptr);

    x = rControlRectangle.Left() + (rControlRectangle.GetWidth()-indicator_size)/2;
    y = rControlRectangle.Top() + (rControlRectangle.GetHeight()-indicator_size)/2;

    // Set the shadow based on if checked or not so we get a checkmark.
    shadowType = isChecked ? GTK_SHADOW_IN : isInconsistent ? GTK_SHADOW_ETCHED_IN : GTK_SHADOW_OUT;
    NWSetWidgetState( gWidgetData[m_nXScreen].gCheckWidget, nState, stateType );
    GTK_TOGGLE_BUTTON(gWidgetData[m_nXScreen].gCheckWidget)->active = isChecked;

    for( clipList::const_iterator it = rClipList.begin(); it != rClipList.end(); ++it )
    {
        clipRect.x = it->Left();
        clipRect.y = it->Top();
        clipRect.width = it->GetWidth();
        clipRect.height = it->GetHeight();

        gtk_paint_check( gWidgetData[m_nXScreen].gCheckWidget->style, gdkDrawable, stateType, shadowType,
                         &clipRect, gWidgetData[m_nXScreen].gCheckWidget, "checkbutton",
                         x, y, indicator_size, indicator_size );
    }

    return true;
}

static void NWCalcArrowRect( const Rectangle& rButton, Rectangle& rArrow )
{
    // Size the arrow appropriately
    Size aSize( rButton.GetWidth()/2, rButton.GetHeight()/2 );
    rArrow.SetSize( aSize );

    rArrow.SetPos( Point(
        rButton.Left() + ( rButton.GetWidth()  - rArrow.GetWidth()  ) / 2,
        rButton.Top() + ( rButton.GetHeight() - rArrow.GetHeight() ) / 2
        ) );
}

bool GtkSalGraphics::NWPaintGTKScrollbar( ControlType, ControlPart nPart,
                                          const Rectangle& rControlRectangle,
                                          const clipList&,
                                          ControlState nState,
                                          const ImplControlValue& aValue,
                                          const OUString& )
{
    assert(aValue.getType() == CTRL_SCROLLBAR);
    const ScrollbarValue& rScrollbarVal = static_cast<const ScrollbarValue&>(aValue);
    GdkX11Pixmap*    pixmap = nullptr;
    Rectangle        pixmapRect, scrollbarRect;
    GtkStateType    stateType;
    GtkShadowType    shadowType;
    GtkScrollbar *    scrollbarWidget;
    GtkStyle *    style;
    GtkAdjustment* scrollbarValues = nullptr;
    GtkOrientation    scrollbarOrientation;
    Rectangle        thumbRect = rScrollbarVal.maThumbRect;
    Rectangle        button11BoundRect = rScrollbarVal.maButton1Rect;   // backward
    Rectangle        button22BoundRect = rScrollbarVal.maButton2Rect;   // forward
    Rectangle        button12BoundRect = rScrollbarVal.maButton1Rect;   // secondary forward
    Rectangle        button21BoundRect = rScrollbarVal.maButton2Rect;   // secondary backward
    GtkArrowType    button1Type;                                        // backward
    GtkArrowType    button2Type;                                        // forward
    gchar *        scrollbarTagH = const_cast<gchar *>("hscrollbar");
    gchar *        scrollbarTagV = const_cast<gchar *>("vscrollbar");
    gchar *        scrollbarTag = nullptr;
    Rectangle        arrowRect;
    gint            slider_width = 0;
    gint            stepper_size = 0;
    gint            stepper_spacing = 0;
    gint            trough_border = 0;
    gint            min_slider_length = 0;
    gint            vShim = 0;
    gint            hShim = 0;
    gint            x,y,w,h;

    // make controlvalue rectangles relative to area
    thumbRect.Move( -rControlRectangle.Left(), -rControlRectangle.Top() );
    button11BoundRect.Move( -rControlRectangle.Left(), -rControlRectangle.Top() );
    button22BoundRect.Move( -rControlRectangle.Left(), -rControlRectangle.Top() );
    button12BoundRect.Move( -rControlRectangle.Left(), -rControlRectangle.Top() );
    button21BoundRect.Move( -rControlRectangle.Left(), -rControlRectangle.Top() );

    NWEnsureGTKButton( m_nXScreen );
    NWEnsureGTKScrollbars( m_nXScreen );
    NWEnsureGTKArrow( m_nXScreen );

    // Find the overall bounding rect of the control
    pixmapRect = rControlRectangle;
    scrollbarRect = pixmapRect;

    if ( (scrollbarRect.GetWidth() <= 1) || (scrollbarRect.GetHeight() <= 1) )
        return true;

    // Grab some button style attributes
    gtk_widget_style_get( gWidgetData[m_nXScreen].gScrollHorizWidget,
                                      "slider_width", &slider_width,
                                      "stepper_size", &stepper_size,
                                      "trough_border", &trough_border,
                                      "stepper_spacing", &stepper_spacing,
                                      "min_slider_length", &min_slider_length, nullptr );
    gboolean has_forward;
    gboolean has_forward2;
    gboolean has_backward;
    gboolean has_backward2;

    gtk_widget_style_get( gWidgetData[m_nXScreen].gScrollHorizWidget, "has-forward-stepper", &has_forward,
                                      "has-secondary-forward-stepper", &has_forward2,
                                      "has-backward-stepper", &has_backward,
                                         "has-secondary-backward-stepper", &has_backward2, nullptr );
    gint magic = trough_border ? 1 : 0;

    if ( nPart == PART_DRAW_BACKGROUND_HORZ )
    {
        unsigned int sliderHeight = slider_width + (trough_border * 2);
        vShim = (pixmapRect.GetHeight() - sliderHeight) / 2;
        bool bRTLSwap = button11BoundRect.Left() > button22BoundRect.Left();

        scrollbarRect.Move( 0, vShim );
        scrollbarRect.SetSize( Size( scrollbarRect.GetWidth(), sliderHeight ) );

        scrollbarWidget = GTK_SCROLLBAR( gWidgetData[m_nXScreen].gScrollHorizWidget );
        scrollbarOrientation = GTK_ORIENTATION_HORIZONTAL;
        scrollbarTag = scrollbarTagH;
        button1Type = bRTLSwap? GTK_ARROW_RIGHT: GTK_ARROW_LEFT;
        button2Type = bRTLSwap? GTK_ARROW_LEFT: GTK_ARROW_RIGHT;

        if ( has_backward )
        {
            button12BoundRect.Move( stepper_size - trough_border,
                                    (scrollbarRect.GetHeight() - slider_width) / 2 );
        }

        button11BoundRect.Move( trough_border, (scrollbarRect.GetHeight() - slider_width) / 2 );
        button11BoundRect.SetSize( Size( stepper_size, slider_width ) );
        button12BoundRect.SetSize( Size( stepper_size, slider_width ) );

        if ( has_backward2 )
        {
            button22BoundRect.Move( stepper_size+(trough_border+1)/2, (scrollbarRect.GetHeight() - slider_width) / 2 );
            button21BoundRect.Move( (trough_border+1)/2, (scrollbarRect.GetHeight() - slider_width) / 2 );
        }
        else
        {
            button22BoundRect.Move( (trough_border+1)/2, (scrollbarRect.GetHeight() - slider_width) / 2 );
        }

        button21BoundRect.SetSize( Size( stepper_size, slider_width ) );
        button22BoundRect.SetSize( Size( stepper_size, slider_width ) );

        thumbRect.Bottom() = thumbRect.Top() + slider_width - 1;
        // Make sure the thumb is at least the default width (so we don't get tiny thumbs),
        // but if the VCL gives us a size smaller than the theme's default thumb size,
        // honor the VCL size
        thumbRect.Right() += magic;
        // Center vertically in the track
        thumbRect.Move( 0, (scrollbarRect.GetHeight() - slider_width) / 2 );
    }
    else
    {
        unsigned int sliderWidth = slider_width + (trough_border * 2);
        hShim = (pixmapRect.GetWidth() - sliderWidth) / 2;

        scrollbarRect.Move( hShim, 0 );
        scrollbarRect.SetSize( Size( sliderWidth, scrollbarRect.GetHeight() ) );

        scrollbarWidget = GTK_SCROLLBAR( gWidgetData[m_nXScreen].gScrollVertWidget );
        scrollbarOrientation = GTK_ORIENTATION_VERTICAL;
        scrollbarTag = scrollbarTagV;
        button1Type = GTK_ARROW_UP;
        button2Type = GTK_ARROW_DOWN;

        if ( has_backward )
        {
            button12BoundRect.Move( (scrollbarRect.GetWidth() - slider_width) / 2,
                                    stepper_size + trough_border );
        }
        button11BoundRect.Move( (scrollbarRect.GetWidth() - slider_width) / 2, trough_border );
        button11BoundRect.SetSize( Size( slider_width, stepper_size ) );
        button12BoundRect.SetSize( Size( slider_width, stepper_size ) );

        if ( has_backward2 )
        {
            button22BoundRect.Move( (scrollbarRect.GetWidth() - slider_width) / 2, stepper_size+(trough_border+1)/2 );
            button21BoundRect.Move( (scrollbarRect.GetWidth() - slider_width) / 2, (trough_border+1)/2 );
        }
        else
        {
            button22BoundRect.Move( (scrollbarRect.GetWidth() - slider_width) / 2, (trough_border+1)/2 );
        }

        button21BoundRect.SetSize( Size( slider_width, stepper_size ) );
        button22BoundRect.SetSize( Size( slider_width, stepper_size ) );

        thumbRect.Right() = thumbRect.Left() + slider_width - 1;

        thumbRect.Bottom() += magic;
        // Center horizontally in the track
        thumbRect.Move( (scrollbarRect.GetWidth() - slider_width) / 2, 0 );
    }

    bool has_slider = ( thumbRect.GetWidth() > 0 && thumbRect.GetHeight() > 0 );

    scrollbarValues = gtk_range_get_adjustment( GTK_RANGE(scrollbarWidget) );
    if ( scrollbarValues == nullptr )
        scrollbarValues = GTK_ADJUSTMENT( gtk_adjustment_new(0, 0, 0, 0, 0, 0) );
    if ( nPart == PART_DRAW_BACKGROUND_HORZ )
    {
        scrollbarValues->lower = rScrollbarVal.mnMin;
        scrollbarValues->upper = rScrollbarVal.mnMax;
        scrollbarValues->value = rScrollbarVal.mnCur;
        scrollbarValues->page_size = scrollbarRect.GetWidth() / 2;
    }
    else
    {
        scrollbarValues->lower = rScrollbarVal.mnMin;
        scrollbarValues->upper = rScrollbarVal.mnMax;
        scrollbarValues->value = rScrollbarVal.mnCur;
        scrollbarValues->page_size = scrollbarRect.GetHeight() / 2;
    }
    gtk_adjustment_changed( scrollbarValues );

    // as multiple paints are required for the scrollbar
    // painting them directly to the window flickers
    pixmap = NWGetPixmapFromScreen( pixmapRect );
    if( ! pixmap )
        return false;
    x = y = 0;

    w = pixmapRect.GetWidth();
    h = pixmapRect.GetHeight();

    GdkDrawable* const &gdkDrawable = pixmap->GetGdkDrawable();
    GdkRectangle* gdkRect = nullptr;

    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );
    NWSetWidgetState( GTK_WIDGET(scrollbarWidget), nState, stateType );
    NWSetWidgetState( gWidgetData[m_nXScreen].gBtnWidget, nState, stateType );
    style = GTK_WIDGET( scrollbarWidget )->style;

    gtk_style_apply_default_background( m_pWindow->style, gdkDrawable, TRUE,
                                        GTK_STATE_NORMAL, gdkRect,
                                        x, y, w, h );

    // ----------------- TROUGH
    // Pass coordinates of draw rect: window(0,0) -> widget(bottom-right) (coords relative to widget)
    gtk_paint_flat_box(m_pWindow->style, gdkDrawable,
                        GTK_STATE_NORMAL, GTK_SHADOW_NONE, gdkRect,
                        m_pWindow, "base", x-pixmapRect.Left(),y-pixmapRect.Top(),x+pixmapRect.Right(),y+pixmapRect.Bottom());

    gtk_paint_box( style, gdkDrawable, GTK_STATE_ACTIVE, GTK_SHADOW_IN,
                   gdkRect, GTK_WIDGET(scrollbarWidget), "trough",
                   x, y,
                   scrollbarRect.GetWidth(), scrollbarRect.GetHeight() );

    if ( nState & ControlState::FOCUSED )
    {
        gtk_paint_focus( style, gdkDrawable, GTK_STATE_ACTIVE,
                         gdkRect, GTK_WIDGET(scrollbarWidget), "trough",
                         x, y,
                         scrollbarRect.GetWidth(), scrollbarRect.GetHeight() );
    }

    // ----------------- THUMB
    if ( has_slider )
    {
        NWConvertVCLStateToGTKState( rScrollbarVal.mnThumbState, &stateType, &shadowType );
        gtk_paint_slider( style, gdkDrawable, stateType, GTK_SHADOW_OUT,
                        gdkRect, GTK_WIDGET(scrollbarWidget), "slider",
                        x+hShim+thumbRect.Left(), y+vShim+thumbRect.Top(),
                        thumbRect.GetWidth(), thumbRect.GetHeight(), scrollbarOrientation );
    }

    // Some engines require allocation, e.g. Clearlooks uses it to identify
    // positions of the buttons, whereupon the first and the last button will
    // have rounded corners.
    GTK_WIDGET(scrollbarWidget)->allocation.x = x;
    GTK_WIDGET(scrollbarWidget)->allocation.y = y;
    GTK_WIDGET(scrollbarWidget)->allocation.width = w;
    GTK_WIDGET(scrollbarWidget)->allocation.height = h;

    bool backwardButtonInsensitive =
        rScrollbarVal.mnCur == rScrollbarVal.mnMin;
    bool forwardButtonInsensitive = rScrollbarVal.mnMax == 0 ||
        rScrollbarVal.mnCur + rScrollbarVal.mnVisibleSize >= rScrollbarVal.mnMax;

    // ----------------- BUTTON 1
    if ( has_backward )
    {
        NWConvertVCLStateToGTKState( rScrollbarVal.mnButton1State, &stateType, &shadowType );
        if ( backwardButtonInsensitive )
            stateType = GTK_STATE_INSENSITIVE;
        gtk_paint_box( style, gdkDrawable, stateType, shadowType,
                       gdkRect, GTK_WIDGET(scrollbarWidget), scrollbarTag,
                       x+hShim+button11BoundRect.Left(), y+vShim+button11BoundRect.Top(),
                       button11BoundRect.GetWidth(), button11BoundRect.GetHeight() );
        // ----------------- ARROW 1
        NWCalcArrowRect( button11BoundRect, arrowRect );
        gtk_paint_arrow( style, gdkDrawable, stateType, shadowType,
                         gdkRect, GTK_WIDGET(scrollbarWidget), scrollbarTag, button1Type, true,
                         x+hShim+arrowRect.Left(), y+vShim+arrowRect.Top(),
                         arrowRect.GetWidth(), arrowRect.GetHeight() );
    }
    if ( has_forward2 )
    {
        NWConvertVCLStateToGTKState( rScrollbarVal.mnButton2State, &stateType, &shadowType );
        if ( forwardButtonInsensitive )
            stateType = GTK_STATE_INSENSITIVE;
        gtk_paint_box( style, gdkDrawable, stateType, shadowType,
                       gdkRect, GTK_WIDGET(scrollbarWidget), scrollbarTag,
                       x+hShim+button12BoundRect.Left(), y+vShim+button12BoundRect.Top(),
                       button12BoundRect.GetWidth(), button12BoundRect.GetHeight() );
        // ----------------- ARROW 1
        NWCalcArrowRect( button12BoundRect, arrowRect );
        gtk_paint_arrow( style, gdkDrawable, stateType, shadowType,
                         gdkRect, GTK_WIDGET(scrollbarWidget), scrollbarTag, button2Type, true,
                         x+hShim+arrowRect.Left(), y+vShim+arrowRect.Top(),
                         arrowRect.GetWidth(), arrowRect.GetHeight() );
    }
    // ----------------- BUTTON 2
    if ( has_backward2 )
    {
        NWConvertVCLStateToGTKState( rScrollbarVal.mnButton1State, &stateType, &shadowType );
        if ( backwardButtonInsensitive )
            stateType = GTK_STATE_INSENSITIVE;
        gtk_paint_box( style, gdkDrawable, stateType, shadowType, gdkRect,
                       GTK_WIDGET(scrollbarWidget), scrollbarTag,
                       x+hShim+button21BoundRect.Left(), y+vShim+button21BoundRect.Top(),
                       button21BoundRect.GetWidth(), button21BoundRect.GetHeight() );
        // ----------------- ARROW 2
        NWCalcArrowRect( button21BoundRect, arrowRect );
        gtk_paint_arrow( style, gdkDrawable, stateType, shadowType,
                         gdkRect, GTK_WIDGET(scrollbarWidget), scrollbarTag, button1Type, true,
                         x+hShim+arrowRect.Left(), y+vShim+arrowRect.Top(),
                         arrowRect.GetWidth(), arrowRect.GetHeight() );
    }
    if ( has_forward )
    {
        NWConvertVCLStateToGTKState( rScrollbarVal.mnButton2State, &stateType, &shadowType );
        if ( forwardButtonInsensitive )
            stateType = GTK_STATE_INSENSITIVE;
        gtk_paint_box( style, gdkDrawable, stateType, shadowType, gdkRect,
                       GTK_WIDGET(scrollbarWidget), scrollbarTag,
                       x+hShim+button22BoundRect.Left(), y+vShim+button22BoundRect.Top(),
                       button22BoundRect.GetWidth(), button22BoundRect.GetHeight() );
        // ----------------- ARROW 2
        NWCalcArrowRect( button22BoundRect, arrowRect );
        gtk_paint_arrow( style, gdkDrawable, stateType, shadowType,
                         gdkRect, GTK_WIDGET(scrollbarWidget), scrollbarTag, button2Type, true,
                         x+hShim+arrowRect.Left(), y+vShim+arrowRect.Top(),
                         arrowRect.GetWidth(), arrowRect.GetHeight() );
    }

    bool bRet = NWRenderPixmapToScreen( pixmap, nullptr, pixmapRect );
    delete pixmap;

    return bRet;
}

static Rectangle NWGetScrollButtonRect(    SalX11Screen nScreen, ControlPart nPart, Rectangle aAreaRect )
{
    gint slider_width;
    gint stepper_size;
    gint stepper_spacing;
    gint trough_border;

    NWEnsureGTKScrollbars( nScreen );

    // Grab some button style attributes
    gtk_widget_style_get( gWidgetData[nScreen].gScrollHorizWidget,
                                      "slider-width", &slider_width,
                                      "stepper-size", &stepper_size,
                                      "trough-border", &trough_border,
                                         "stepper-spacing", &stepper_spacing, nullptr );

    gboolean has_forward;
    gboolean has_forward2;
    gboolean has_backward;
    gboolean has_backward2;

    gtk_widget_style_get( gWidgetData[nScreen].gScrollHorizWidget,
                                      "has-forward-stepper", &has_forward,
                                      "has-secondary-forward-stepper", &has_forward2,
                                      "has-backward-stepper", &has_backward,
                                         "has-secondary-backward-stepper", &has_backward2, nullptr );
    gint       buttonWidth;
    gint       buttonHeight;
    Rectangle  buttonRect;

    gint nFirst = 0;
    gint nSecond = 0;

    if ( has_forward )   nSecond += 1;
    if ( has_forward2 )  nFirst  += 1;
    if ( has_backward )  nFirst  += 1;
    if ( has_backward2 ) nSecond += 1;

    if ( ( nPart == PART_BUTTON_UP ) || ( nPart == PART_BUTTON_DOWN ) )
    {
        buttonWidth = slider_width + 2 * trough_border;
        buttonHeight = stepper_size + trough_border + stepper_spacing;
    }
    else
    {
        buttonWidth = stepper_size + trough_border + stepper_spacing;
        buttonHeight = slider_width + 2 * trough_border;
    }

    if ( nPart == PART_BUTTON_UP )
    {
        buttonHeight *= nFirst;
        buttonHeight -= 1;
        buttonRect.setX( aAreaRect.Left() );
        buttonRect.setY( aAreaRect.Top() );
    }
    else if ( nPart == PART_BUTTON_LEFT )
    {
        buttonWidth *= nFirst;
        buttonWidth -= 1;
        buttonRect.setX( aAreaRect.Left() );
        buttonRect.setY( aAreaRect.Top() );
    }
    else if ( nPart == PART_BUTTON_DOWN )
    {
        buttonHeight *= nSecond;
        buttonRect.setX( aAreaRect.Left() );
        buttonRect.setY( aAreaRect.Top() + aAreaRect.GetHeight() - buttonHeight );
    }
    else if ( nPart == PART_BUTTON_RIGHT )
    {
        buttonWidth *= nSecond;
        buttonRect.setX( aAreaRect.Left() + aAreaRect.GetWidth() - buttonWidth );
        buttonRect.setY( aAreaRect.Top() );
    }

    buttonRect.SetSize( Size( buttonWidth, buttonHeight ) );

    return buttonRect;
}

bool GtkSalGraphics::NWPaintGTKEditBox( GdkDrawable* gdkDrawable,
                                        ControlType nType, ControlPart nPart,
                                        const Rectangle& rControlRectangle,
                                        const clipList& rClipList,
                                        ControlState nState,
                                        const ImplControlValue& aValue,
                                        const OUString& rCaption )
{
    Rectangle        pixmapRect;
    GdkRectangle    clipRect;

    // Find the overall bounding rect of the buttons's drawing area,
    // plus its actual draw rect excluding adornment
    pixmapRect = NWGetEditBoxPixmapRect( m_nXScreen, nType, nPart, rControlRectangle,
                                         nState, aValue, rCaption );
    for( clipList::const_iterator it = rClipList.begin(); it != rClipList.end(); ++it )
    {
        clipRect.x = it->Left();
        clipRect.y = it->Top();
        clipRect.width = it->GetWidth();
        clipRect.height = it->GetHeight();

        NWPaintOneEditBox( m_nXScreen, gdkDrawable, &clipRect, nType, nPart, pixmapRect, nState, aValue, rCaption );
    }

    return true;
}

/* Take interior/exterior focus into account and return
 * the bounding rectangle of the edit box including
 * any focus requirements.
 */
static Rectangle NWGetEditBoxPixmapRect(SalX11Screen nScreen,
                                        ControlType,
                                        ControlPart,
                                        Rectangle aAreaRect,
                                        ControlState,
                                        const ImplControlValue&,
                                        const OUString& )
{
    Rectangle        pixmapRect = aAreaRect;
    gboolean        interiorFocus;
    gint            focusWidth;

    NWEnsureGTKEditBox( nScreen );

    // Grab some entry style attributes
    gtk_widget_style_get( gWidgetData[nScreen].gEditBoxWidget,
                                    "focus-line-width",    &focusWidth,
                                     "interior-focus",    &interiorFocus, nullptr );

    if ( !interiorFocus )
    {
        pixmapRect.Move( -(focusWidth), -(focusWidth) );
        pixmapRect.SetSize( Size( pixmapRect.GetWidth() + (2*(focusWidth)),
                                  pixmapRect.GetHeight() + (2*(focusWidth)) ) );
    }

    return pixmapRect;
}

/* Paint a GTK Entry widget into the specified GdkPixmap.
 * All coordinates should be local to the Pixmap, NOT
 * screen/window coordinates.
 */
static void NWPaintOneEditBox(    SalX11Screen nScreen,
                                GdkDrawable * gdkDrawable,
                                GdkRectangle *    gdkRect,
                                ControlType            nType,
                                ControlPart,
                                Rectangle                aEditBoxRect,
                                ControlState            nState,
                                const ImplControlValue&,
                                const OUString& )
{
    GtkStateType    stateType;
    GtkShadowType    shadowType;
    GtkWidget      *widget;

    NWEnsureGTKButton( nScreen );
    NWEnsureGTKEditBox( nScreen );
    NWEnsureGTKSpinButton( nScreen );
    NWEnsureGTKCombo( nScreen );
    NWEnsureGTKScrolledWindow( nScreen );
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    switch ( nType )
    {
        case CTRL_SPINBOX:
            widget = gWidgetData[nScreen].gSpinButtonWidget;
            break;

        case CTRL_MULTILINE_EDITBOX:
            widget = gWidgetData[nScreen].gScrolledWindowWidget;
            break;
        case CTRL_COMBOBOX:
            widget = GTK_COMBO(gWidgetData[nScreen].gComboWidget)->entry;
            break;

        default:
            widget = gWidgetData[nScreen].gEditBoxWidget;
            break;
    }

    if ( stateType == GTK_STATE_PRELIGHT )
        stateType = GTK_STATE_NORMAL;

    NWSetWidgetState( widget, nState, stateType );

    gint xborder = widget->style->xthickness;
    gint yborder = widget->style->ythickness;
    gint bInteriorFocus, nFocusLineWidth;
    gtk_widget_style_get( widget,
        "interior-focus",   &bInteriorFocus,
        "focus-line-width", &nFocusLineWidth,
        nullptr);
    if ( !bInteriorFocus )
    {
        xborder += nFocusLineWidth;
        yborder += nFocusLineWidth;
    }

    gtk_paint_flat_box( widget->style, gdkDrawable, stateType, GTK_SHADOW_NONE,
                        gdkRect, widget, "entry_bg",
                        aEditBoxRect.Left() + xborder, aEditBoxRect.Top() + yborder,
                        aEditBoxRect.GetWidth() - 2*xborder, aEditBoxRect.GetHeight() - 2*yborder );
    gtk_paint_shadow( widget->style, gdkDrawable, GTK_STATE_NORMAL, GTK_SHADOW_IN,
                      gdkRect, widget, "entry",
                      aEditBoxRect.Left(), aEditBoxRect.Top(),
                      aEditBoxRect.GetWidth(), aEditBoxRect.GetHeight() );

}

bool GtkSalGraphics::NWPaintGTKSpinBox(ControlType nType, ControlPart nPart,
                                       const Rectangle& rControlRectangle,
                                       const clipList&,
                                       ControlState nState,
                                       const ImplControlValue& aValue,
                                       const OUString& rCaption,
                                       ControlCacheKey& rControlCacheKey)
{
    Rectangle            pixmapRect;
    GtkStateType        stateType;
    GtkShadowType        shadowType;
    const SpinbuttonValue *    pSpinVal = (aValue.getType() == CTRL_SPINBUTTONS) ? static_cast<const SpinbuttonValue *>(&aValue) : nullptr;
    Rectangle            upBtnRect;
    ControlPart        upBtnPart = PART_BUTTON_UP;
    ControlState        upBtnState = ControlState::ENABLED;
    Rectangle            downBtnRect;
    ControlPart        downBtnPart = PART_BUTTON_DOWN;
    ControlState        downBtnState = ControlState::ENABLED;

    NWEnsureGTKButton( m_nXScreen );
    NWEnsureGTKSpinButton( m_nXScreen );
    NWEnsureGTKArrow( m_nXScreen );

    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    if ( pSpinVal )
    {
        upBtnPart = pSpinVal->mnUpperPart;
        upBtnState = pSpinVal->mnUpperState;

        downBtnPart = pSpinVal->mnLowerPart;
        downBtnState = pSpinVal->mnLowerState;
    }

    // CTRL_SPINBUTTONS pass their area in pSpinVal, not in rControlRectangle
    if ( nType == CTRL_SPINBUTTONS )
    {
        if ( !pSpinVal )
        {
            std::fprintf( stderr, "Tried to draw CTRL_SPINBUTTONS, but the SpinButtons data structure didn't exist!\n" );
            return false;
        }
        pixmapRect = pSpinVal->maUpperRect;
        pixmapRect.Union( pSpinVal->maLowerRect );
    }
    else
        pixmapRect = rControlRectangle;

    BEGIN_PIXMAP_RENDER( pixmapRect, gdkPixmap )
    {
        // First render background
        gtk_paint_flat_box(m_pWindow->style,gdkPixmap,GTK_STATE_NORMAL,GTK_SHADOW_NONE,nullptr,m_pWindow,"base",
                -pixmapRect.Left(),
                -pixmapRect.Top(),
                pixmapRect.Right(),
                pixmapRect.Bottom() );

        upBtnRect = NWGetSpinButtonRect( m_nXScreen, nType, upBtnPart, pixmapRect, upBtnState, aValue, rCaption );
        downBtnRect = NWGetSpinButtonRect( m_nXScreen, nType, downBtnPart, pixmapRect, downBtnState, aValue, rCaption );

        if ( (nType==CTRL_SPINBOX) && (nPart!=PART_ALL_BUTTONS) )
        {
            // Draw an edit field for SpinBoxes and ComboBoxes
            Rectangle aEditBoxRect( pixmapRect );
            aEditBoxRect.SetSize( Size( pixmapRect.GetWidth() - upBtnRect.GetWidth(), aEditBoxRect.GetHeight() ) );
            if( AllSettings::GetLayoutRTL() )
                aEditBoxRect.setX( upBtnRect.GetWidth() );
            else
                aEditBoxRect.setX( 0 );
            aEditBoxRect.setY( 0 );

            NWPaintOneEditBox( m_nXScreen, gdkPixmap, nullptr, nType, nPart, aEditBoxRect, nState, aValue, rCaption );
        }

        NWSetWidgetState( gWidgetData[m_nXScreen].gSpinButtonWidget, nState, stateType );
        gtk_widget_style_get( gWidgetData[m_nXScreen].gSpinButtonWidget, "shadow_type", &shadowType, nullptr );

        if ( shadowType != GTK_SHADOW_NONE )
        {
            Rectangle        shadowRect( upBtnRect );

            shadowRect.Union( downBtnRect );
            gtk_paint_box( gWidgetData[m_nXScreen].gSpinButtonWidget->style, gdkPixmap, GTK_STATE_NORMAL, shadowType, nullptr,
                gWidgetData[m_nXScreen].gSpinButtonWidget, "spinbutton",
                (shadowRect.Left() - pixmapRect.Left()), (shadowRect.Top() - pixmapRect.Top()),
                shadowRect.GetWidth(), shadowRect.GetHeight() );
        }

        NWPaintOneSpinButton( m_nXScreen, gdkPixmap, nType, upBtnPart, pixmapRect, upBtnState, aValue, rCaption );
        NWPaintOneSpinButton( m_nXScreen, gdkPixmap, nType, downBtnPart, pixmapRect, downBtnState, aValue, rCaption );
    }
    END_PIXMAP_RENDER_WITH_CONTROL_KEY(pixmapRect, rControlCacheKey);

    return true;
}

static Rectangle NWGetSpinButtonRect( SalX11Screen nScreen,
                                      ControlType,
                                      ControlPart            nPart,
                                      Rectangle             aAreaRect,
                                      ControlState,
                                      const ImplControlValue&,
                                      const OUString& )
{
    gint            buttonSize;
    Rectangle        buttonRect;

    NWEnsureGTKSpinButton( nScreen );

    buttonSize = MAX( PANGO_PIXELS( pango_font_description_get_size(GTK_WIDGET(gWidgetData[nScreen].gSpinButtonWidget)->style->font_desc) ),
                   MIN_SPIN_ARROW_WIDTH );
    buttonSize -= buttonSize % 2 - 1; /* force odd */
    buttonRect.SetSize( Size( buttonSize + 2 * gWidgetData[nScreen].gSpinButtonWidget->style->xthickness,
                              buttonRect.GetHeight() ) );
    if( AllSettings::GetLayoutRTL() )
        buttonRect.setX( aAreaRect.Left() );
    else
        buttonRect.setX( aAreaRect.Left() + (aAreaRect.GetWidth() - buttonRect.GetWidth()) );
    if ( nPart == PART_BUTTON_UP )
    {
        buttonRect.setY( aAreaRect.Top() );
        buttonRect.Bottom() = buttonRect.Top() + (aAreaRect.GetHeight() / 2);
    }
    else if( nPart == PART_BUTTON_DOWN )
    {
        buttonRect.setY( aAreaRect.Top() + (aAreaRect.GetHeight() / 2) );
        buttonRect.Bottom() = aAreaRect.Bottom(); // cover area completely
    }
    else
    {
        if( AllSettings::GetLayoutRTL() ) {
            buttonRect.Left()   = buttonRect.Right()+1;
            buttonRect.Right()  = aAreaRect.Right();
        } else {
            buttonRect.Right()  = buttonRect.Left()-1;
            buttonRect.Left()   = aAreaRect.Left();
        }
        buttonRect.Top()    = aAreaRect.Top();
        buttonRect.Bottom() = aAreaRect.Bottom();
    }

    return buttonRect;
}

static void NWPaintOneSpinButton( SalX11Screen nScreen,
                                  GdkPixmap*            pixmap,
                                  ControlType            nType,
                                  ControlPart            nPart,
                                  Rectangle                aAreaRect,
                                  ControlState            nState,
                                  const ImplControlValue&    aValue,
                                  const OUString&                rCaption )
{
    Rectangle            buttonRect;
    GtkStateType        stateType;
    GtkShadowType        shadowType;
    Rectangle            arrowRect;
    gint                arrowSize;

    NWEnsureGTKSpinButton( nScreen );
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    buttonRect = NWGetSpinButtonRect( nScreen, nType, nPart, aAreaRect, nState, aValue, rCaption );

    NWSetWidgetState( gWidgetData[nScreen].gSpinButtonWidget, nState, stateType );
    gtk_paint_box( gWidgetData[nScreen].gSpinButtonWidget->style, pixmap, stateType, shadowType, nullptr, gWidgetData[nScreen].gSpinButtonWidget,
            (nPart == PART_BUTTON_UP) ? "spinbutton_up" : "spinbutton_down",
            (buttonRect.Left() - aAreaRect.Left()), (buttonRect.Top() - aAreaRect.Top()),
            buttonRect.GetWidth(), buttonRect.GetHeight() );

    arrowSize = (buttonRect.GetWidth() - (2 * gWidgetData[nScreen].gSpinButtonWidget->style->xthickness)) - 4;
    arrowSize -= arrowSize % 2 - 1; /* force odd */
    arrowRect.SetSize( Size( arrowSize, arrowSize ) );
    arrowRect.setX( buttonRect.Left() + (buttonRect.GetWidth() - arrowRect.GetWidth()) / 2 );
    if ( nPart == PART_BUTTON_UP )
        arrowRect.setY( buttonRect.Top() + (buttonRect.GetHeight() - arrowRect.GetHeight()) / 2 + 1);
    else
        arrowRect.setY( buttonRect.Top() + (buttonRect.GetHeight() - arrowRect.GetHeight()) / 2 - 1);

    gtk_paint_arrow( gWidgetData[nScreen].gSpinButtonWidget->style, pixmap, stateType, GTK_SHADOW_OUT, nullptr, gWidgetData[nScreen].gSpinButtonWidget,
            "spinbutton", (nPart == PART_BUTTON_UP) ? GTK_ARROW_UP : GTK_ARROW_DOWN, true,
            (arrowRect.Left() - aAreaRect.Left()), (arrowRect.Top() - aAreaRect.Top()),
            arrowRect.GetWidth(), arrowRect.GetHeight() );
}

bool GtkSalGraphics::NWPaintGTKComboBox( GdkDrawable* gdkDrawable,
                                         ControlType nType, ControlPart nPart,
                                         const Rectangle& rControlRectangle,
                                         const clipList& rClipList,
                                         ControlState nState,
                                         const ImplControlValue& aValue,
                                         const OUString& rCaption )
{
    Rectangle        pixmapRect;
    Rectangle        buttonRect;
    GtkStateType    stateType;
    GtkShadowType    shadowType;
    Rectangle        arrowRect;
    gint            x,y;
    GdkRectangle    clipRect;

    NWEnsureGTKButton( m_nXScreen );
    NWEnsureGTKArrow( m_nXScreen );
    NWEnsureGTKCombo( m_nXScreen );
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    // Find the overall bounding rect of the buttons's drawing area,
    // plus its actual draw rect excluding adornment
    pixmapRect = rControlRectangle;
    x = rControlRectangle.Left();
    y = rControlRectangle.Top();

    NWSetWidgetState( gWidgetData[m_nXScreen].gBtnWidget, nState, stateType );
    NWSetWidgetState( gWidgetData[m_nXScreen].gComboWidget, nState, stateType );
    NWSetWidgetState( gWidgetData[m_nXScreen].gArrowWidget, nState, stateType );

    buttonRect = NWGetComboBoxButtonRect( m_nXScreen, nType, PART_BUTTON_DOWN, pixmapRect, nState, aValue, rCaption );
    if( nPart == PART_BUTTON_DOWN )
        buttonRect.Left() += 1;

    Rectangle        aEditBoxRect( pixmapRect );
    aEditBoxRect.SetSize( Size( pixmapRect.GetWidth() - buttonRect.GetWidth(), aEditBoxRect.GetHeight() ) );
    if( AllSettings::GetLayoutRTL() )
        aEditBoxRect.SetPos( Point( x + buttonRect.GetWidth() , y ) );

    #define ARROW_EXTENT        0.7
    arrowRect.SetSize( Size( (gint)(MIN_ARROW_SIZE * ARROW_EXTENT),
                             (gint)(MIN_ARROW_SIZE * ARROW_EXTENT) ) );
    arrowRect.SetPos( Point( buttonRect.Left() + (gint)((buttonRect.GetWidth() - arrowRect.GetWidth()) / 2),
                             buttonRect.Top() + (gint)((buttonRect.GetHeight() - arrowRect.GetHeight()) / 2) ) );

    for( clipList::const_iterator it = rClipList.begin(); it != rClipList.end(); ++it )
    {
        clipRect.x = it->Left();
        clipRect.y = it->Top();
        clipRect.width = it->GetWidth();
        clipRect.height = it->GetHeight();

        if( nPart == PART_ENTIRE_CONTROL )
            NWPaintOneEditBox( m_nXScreen, gdkDrawable, &clipRect, nType, nPart, aEditBoxRect,
                               nState, aValue, rCaption );

        // Buttons must paint opaque since some themes have alpha-channel enabled buttons
        gtk_paint_flat_box( m_pWindow->style, gdkDrawable, GTK_STATE_NORMAL, GTK_SHADOW_NONE,
                            &clipRect, m_pWindow, "base",
                            x+(buttonRect.Left() - pixmapRect.Left()),
                            y+(buttonRect.Top() - pixmapRect.Top()),
                            buttonRect.GetWidth(), buttonRect.GetHeight() );
        gtk_paint_box( GTK_COMBO(gWidgetData[m_nXScreen].gComboWidget)->button->style, gdkDrawable, stateType, shadowType,
                       &clipRect, GTK_COMBO(gWidgetData[m_nXScreen].gComboWidget)->button, "button",
                       x+(buttonRect.Left() - pixmapRect.Left()),
                       y+(buttonRect.Top() - pixmapRect.Top()),
                       buttonRect.GetWidth(), buttonRect.GetHeight() );

        gtk_paint_arrow( gWidgetData[m_nXScreen].gArrowWidget->style, gdkDrawable, stateType, shadowType,
                         &clipRect, gWidgetData[m_nXScreen].gArrowWidget, "arrow", GTK_ARROW_DOWN, true,
                         x+(arrowRect.Left() - pixmapRect.Left()), y+(arrowRect.Top() - pixmapRect.Top()),
                         arrowRect.GetWidth(), arrowRect.GetHeight() );
    }

    return true;
}

static Rectangle NWGetComboBoxButtonRect( SalX11Screen nScreen,
                                          ControlType,
                                          ControlPart nPart,
                                          Rectangle                aAreaRect,
                                          ControlState,
                                          const ImplControlValue&,
                                          const OUString& )
{
    Rectangle    aButtonRect;
    gint        nArrowWidth;
    gint        nButtonWidth;
    gint        nFocusWidth;
    gint        nFocusPad;

    NWEnsureGTKArrow( nScreen );

    // Grab some button style attributes
    gtk_widget_style_get( gWidgetData[nScreen].gDropdownWidget,
                                      "focus-line-width",    &nFocusWidth,
                                    "focus-padding",     &nFocusPad, nullptr );

    nArrowWidth = MIN_ARROW_SIZE + (GTK_MISC(gWidgetData[nScreen].gArrowWidget)->xpad * 2);
    nButtonWidth = nArrowWidth +
                   ((BTN_CHILD_SPACING + gWidgetData[nScreen].gDropdownWidget->style->xthickness) * 2)
                   + (2 * (nFocusWidth+nFocusPad));
    if( nPart == PART_BUTTON_DOWN )
    {
        aButtonRect.SetSize( Size( nButtonWidth, aAreaRect.GetHeight() ) );
        if( AllSettings::GetLayoutRTL() )
            aButtonRect.SetPos( Point( aAreaRect.Left(), aAreaRect.Top() ) );
        else
            aButtonRect.SetPos( Point( aAreaRect.Left() + aAreaRect.GetWidth() - nButtonWidth,
                                       aAreaRect.Top() ) );
    }
    else if( nPart == PART_SUB_EDIT )
    {
        NWEnsureGTKCombo( nScreen );

        gint adjust_x = GTK_CONTAINER(gWidgetData[nScreen].gComboWidget)->border_width +
                        nFocusWidth +
                        nFocusPad;
        gint adjust_y = adjust_x + gWidgetData[nScreen].gComboWidget->style->ythickness;
        adjust_x     += gWidgetData[nScreen].gComboWidget->style->xthickness;
        aButtonRect.SetSize( Size( aAreaRect.GetWidth() - nButtonWidth - 2 * adjust_x,
                                   aAreaRect.GetHeight() - 2 * adjust_y ) );
        Point aEditPos = aAreaRect.TopLeft();
        aEditPos.X() += adjust_x;
        aEditPos.Y() += adjust_y;
        if( AllSettings::GetLayoutRTL() )
            aEditPos.X() += nButtonWidth;
        aButtonRect.SetPos( aEditPos );
    }

    return aButtonRect;
}

bool GtkSalGraphics::NWPaintGTKTabItem( ControlType nType, ControlPart,
                                        const Rectangle& rControlRectangle,
                                        const clipList&,
                                        ControlState nState,
                                        const ImplControlValue& aValue,
                                        const OUString& )
{
    OSL_ASSERT( nType != CTRL_TAB_ITEM || aValue.getType() == CTRL_TAB_ITEM );
    GdkX11Pixmap *   pixmap;
    GdkX11Pixmap *   mask;
    Rectangle        pixmapRect;
    Rectangle        tabRect;
    GtkStateType    stateType;
    GtkShadowType    shadowType;
    if( ! gWidgetData[ m_nXScreen ].gCacheTabItems )
    {
        gWidgetData[ m_nXScreen ].gCacheTabItems = new NWPixmapCache( m_nXScreen );
        gWidgetData[ m_nXScreen ].gCacheTabPages = new NWPixmapCache( m_nXScreen );
    }
    NWPixmapCache& aCacheItems = *gWidgetData[ m_nXScreen ].gCacheTabItems;
    NWPixmapCache& aCachePage = *gWidgetData[ m_nXScreen ].gCacheTabPages;

    if( !aCacheItems.GetSize() )
        aCacheItems.SetSize( 20 );
    if( !aCachePage.GetSize() )
        aCachePage.SetSize( 1 );

    if ( (nType == CTRL_TAB_ITEM) && (aValue.getType() != CTRL_TAB_ITEM) )
    {
        return false;
    }

    NWEnsureGTKButton( m_nXScreen );
    NWEnsureGTKNotebook( m_nXScreen );
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    // Find the overall bounding rect of the buttons's drawing area,
    // plus its actual draw rect excluding adornment
    pixmapRect = rControlRectangle;
    if ( nType == CTRL_TAB_ITEM )
    {
        const TabitemValue *    pTabitemValue = static_cast<const TabitemValue *>(&aValue);
        if ( !pTabitemValue->isFirst() )
        {
            // GTK+ tabs overlap on the right edge (the top tab obscures the
            // left edge of the tab right "below" it, so adjust the rectangle
            // to draw tabs slightly large so the overlap happens
            pixmapRect.Move( -2, 0 );
            pixmapRect.SetSize( Size( pixmapRect.GetWidth() + 2, pixmapRect.GetHeight() ) );
        }
        if ( nState & ControlState::SELECTED )
        {
            // In GTK+, the selected tab is 2px taller than all other tabs
            pixmapRect.Move( 0, -2 );
            pixmapRect.Bottom() += 2;
            tabRect = pixmapRect;
            // Only draw over 1 pixel of the tab pane that this tab is drawn on top of.
            tabRect.Bottom() -= 1;
        }
        else
            tabRect = pixmapRect;

        // Allow the tab to draw a right border if needed
        tabRect.Right() -= 1;

        // avoid degenerate cases which might lead to crashes
        if( tabRect.GetWidth() <= 1 || tabRect.GetHeight() <= 1 )
            return false;
    }

    if( nType == CTRL_TAB_ITEM )
    {
        if( aCacheItems.Find( nType, nState, pixmapRect, &pixmap, &mask ) )
            return NWRenderPixmapToScreen( pixmap, mask, pixmapRect );
    }
    else
    {
        if( aCachePage.Find( nType, nState, pixmapRect, &pixmap, &mask ) )
            return NWRenderPixmapToScreen( pixmap, mask, pixmapRect );
    }

    GdkRectangle paintRect;
    paintRect.x = paintRect.y = 0;
    paintRect.width = pixmapRect.GetWidth();
    paintRect.height = pixmapRect.GetHeight();

    BEGIN_CACHE_PIXMAP_RENDER( pixmapRect, pixmap, mask, gdkPixmap )
    {
        gtk_paint_flat_box( m_pWindow->style, gdkPixmap, GTK_STATE_NORMAL,
                            GTK_SHADOW_NONE, &paintRect, m_pWindow, "base",
                            -rControlRectangle.Left(),
                            -rControlRectangle.Top(),
                            pixmapRect.GetWidth()+rControlRectangle.Left(),
                            pixmapRect.GetHeight()+rControlRectangle.Top());

        NWSetWidgetState( gWidgetData[m_nXScreen].gNotebookWidget, nState, stateType );

        switch( nType )
        {
            case CTRL_TAB_BODY:
                break;

            case CTRL_TAB_PANE:
                gtk_paint_box_gap( gWidgetData[m_nXScreen].gNotebookWidget->style, gdkPixmap, GTK_STATE_NORMAL, GTK_SHADOW_OUT, nullptr, gWidgetData[m_nXScreen].gNotebookWidget,
                    "notebook", 0, 0, pixmapRect.GetWidth(), pixmapRect.GetHeight(), GTK_POS_TOP, 0, 0 );
                break;

            case CTRL_TAB_ITEM:
            {
                stateType = ( nState & ControlState::SELECTED ) ? GTK_STATE_NORMAL : GTK_STATE_ACTIVE;

                // First draw the background
                gtk_paint_flat_box(gWidgetData[m_nXScreen].gNotebookWidget->style, gdkPixmap,
                                       GTK_STATE_NORMAL, GTK_SHADOW_NONE, nullptr, m_pWindow, "base",
                                       -rControlRectangle.Left(),
                                       -rControlRectangle.Top(),
                                       pixmapRect.GetWidth()+rControlRectangle.Left(),
                                       pixmapRect.GetHeight()+rControlRectangle.Top());

                // Now the tab itself
                if( nState & ControlState::ROLLOVER )
                    g_object_set_data(G_OBJECT(gdkPixmap),tabPrelitDataName,reinterpret_cast<gpointer>(TRUE));

                gtk_paint_extension( gWidgetData[m_nXScreen].gNotebookWidget->style, gdkPixmap, stateType, GTK_SHADOW_OUT, nullptr, gWidgetData[m_nXScreen].gNotebookWidget,
                    "tab", (tabRect.Left() - pixmapRect.Left()), (tabRect.Top() - pixmapRect.Top()),
                    tabRect.GetWidth(), tabRect.GetHeight(), GTK_POS_BOTTOM );

                g_object_steal_data(G_OBJECT(gdkPixmap),tabPrelitDataName);

                if ( nState & ControlState::SELECTED )
                {
                    gtk_paint_flat_box( m_pWindow->style, gdkPixmap, stateType, GTK_SHADOW_NONE, nullptr, m_pWindow,
                        "base", 0, (pixmapRect.GetHeight() - 1), pixmapRect.GetWidth(), 1 );
                }
                break;
            }

            default:
                break;
        }
    }
    END_CACHE_PIXMAP_RENDER( pixmapRect, pixmap, mask )

    // cache data
    if( nType == CTRL_TAB_ITEM )
        aCacheItems.Fill( nType, nState, pixmapRect, pixmap, mask );
    else
        aCachePage.Fill( nType, nState, pixmapRect, pixmap, mask );

    return true;
}

bool GtkSalGraphics::NWPaintGTKListBox( GdkDrawable* gdkDrawable,
                                        ControlType nType, ControlPart nPart,
                                        const Rectangle& rControlRectangle,
                                        const clipList& rClipList,
                                        ControlState nState,
                                        const ImplControlValue& aValue,
                                        const OUString& rCaption )
{
    Rectangle        aIndicatorRect;
    GtkStateType    stateType;
    GtkShadowType    shadowType;
    gint            bInteriorFocus;
    gint            nFocusLineWidth;
    gint            nFocusPadding;
    gint            x,y,w,h;
    GdkRectangle    clipRect;

    NWEnsureGTKButton( m_nXScreen );
    NWEnsureGTKOptionMenu( m_nXScreen );
    NWEnsureGTKScrolledWindow( m_nXScreen );
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    // set up references to correct drawable and cliprect
    NWSetWidgetState( gWidgetData[m_nXScreen].gBtnWidget, nState, stateType );
    NWSetWidgetState( gWidgetData[m_nXScreen].gOptionMenuWidget, nState, stateType );
    NWSetWidgetState( gWidgetData[m_nXScreen].gScrolledWindowWidget, nState, stateType );

    x = rControlRectangle.Left();
    y = rControlRectangle.Top();
    w = rControlRectangle.GetWidth();
    h = rControlRectangle.GetHeight();

    if ( nPart != PART_WINDOW )
    {
        gtk_widget_style_get( gWidgetData[m_nXScreen].gOptionMenuWidget,
            "interior_focus",    &bInteriorFocus,
            "focus_line_width",    &nFocusLineWidth,
            "focus_padding",    &nFocusPadding,
            nullptr);
    }

    for( clipList::const_iterator it = rClipList.begin(); it != rClipList.end(); ++it )
    {
        clipRect.x = it->Left();
        clipRect.y = it->Top();
        clipRect.width = it->GetWidth();
        clipRect.height = it->GetHeight();

        if ( nPart != PART_WINDOW )
        {
            // Listboxes must paint opaque since some themes have alpha-channel enabled bodies
            gtk_paint_flat_box( m_pWindow->style, gdkDrawable, GTK_STATE_NORMAL, GTK_SHADOW_NONE,
                                &clipRect, m_pWindow, "base", x, y, w, h);
            gtk_paint_box( gWidgetData[m_nXScreen].gOptionMenuWidget->style, gdkDrawable, stateType, shadowType, &clipRect,
                           gWidgetData[m_nXScreen].gOptionMenuWidget, "optionmenu",
                           x, y, w, h);
            aIndicatorRect = NWGetListBoxIndicatorRect( m_nXScreen, nType, nPart, rControlRectangle, nState,
                                                        aValue, rCaption );
            gtk_paint_tab( gWidgetData[m_nXScreen].gOptionMenuWidget->style, gdkDrawable, stateType, shadowType, &clipRect,
                           gWidgetData[m_nXScreen].gOptionMenuWidget, "optionmenutab",
                           aIndicatorRect.Left(), aIndicatorRect.Top(),
                           aIndicatorRect.GetWidth(), aIndicatorRect.GetHeight() );
        }
        else
        {
            shadowType = GTK_SHADOW_IN;

            gtk_paint_shadow( gWidgetData[m_nXScreen].gScrolledWindowWidget->style, gdkDrawable, GTK_STATE_NORMAL, shadowType,
                &clipRect, gWidgetData[m_nXScreen].gScrolledWindowWidget, "scrolled_window",
                x, y, w, h );
        }
    }

    return true;
}

bool GtkSalGraphics::NWPaintGTKToolbar(
            GdkDrawable* gdkDrawable,
            ControlType, ControlPart nPart,
            const Rectangle& rControlRectangle,
            const clipList& rClipList,
            ControlState nState, const ImplControlValue& aValue,
            const OUString& string)
{
    GtkStateType    stateType;
    GtkShadowType    shadowType;
    gint            x, y, w, h;
    gint            g_x=0, g_y=0, g_w=10, g_h=10;
    GtkWidget*      pButtonWidget = gWidgetData[m_nXScreen].gToolbarButtonWidget;
    GdkRectangle    clipRect;

    NWEnsureGTKToolbar( m_nXScreen );
    if( nPart == PART_BUTTON ) // toolbar buttons cannot focus in gtk
        nState &= ~ControlState::FOCUSED;
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    x = rControlRectangle.Left();
    y = rControlRectangle.Top();
    w = rControlRectangle.GetWidth();
    h = rControlRectangle.GetHeight();

    // handle toolbar
    if( nPart == PART_DRAW_BACKGROUND_HORZ || nPart == PART_DRAW_BACKGROUND_VERT )
    {
        NWSetWidgetState( gWidgetData[m_nXScreen].gToolbarWidget, nState, stateType );

        GTK_WIDGET_UNSET_FLAGS( gWidgetData[m_nXScreen].gToolbarWidget, GTK_SENSITIVE );
        if ( nState & ControlState::ENABLED )
            GTK_WIDGET_SET_FLAGS( gWidgetData[m_nXScreen].gToolbarWidget, GTK_SENSITIVE );

        if( nPart == PART_DRAW_BACKGROUND_HORZ )
            gtk_toolbar_set_orientation( GTK_TOOLBAR(gWidgetData[m_nXScreen].gToolbarWidget), GTK_ORIENTATION_HORIZONTAL );
        else
            gtk_toolbar_set_orientation( GTK_TOOLBAR(gWidgetData[m_nXScreen].gToolbarWidget), GTK_ORIENTATION_VERTICAL );
    }
    // handle grip
    else if( nPart == PART_THUMB_HORZ || nPart == PART_THUMB_VERT )
    {
        NWSetWidgetState( gWidgetData[m_nXScreen].gHandleBoxWidget, nState, stateType );

        GTK_WIDGET_UNSET_FLAGS( gWidgetData[m_nXScreen].gHandleBoxWidget, GTK_SENSITIVE );
        if ( nState & ControlState::ENABLED )
            GTK_WIDGET_SET_FLAGS( gWidgetData[m_nXScreen].gHandleBoxWidget, GTK_SENSITIVE );

        gtk_handle_box_set_shadow_type( GTK_HANDLE_BOX(gWidgetData[m_nXScreen].gHandleBoxWidget), shadowType );

        // evaluate grip rect
        if( aValue.getType() == CTRL_TOOLBAR )
        {
            const ToolbarValue* pVal = static_cast<const ToolbarValue*>(&aValue);
            g_x = pVal->maGripRect.Left();
            g_y = pVal->maGripRect.Top();
            g_w = pVal->maGripRect.GetWidth();
            g_h = pVal->maGripRect.GetHeight();
        }
    }
    // handle button
    else if( nPart == PART_BUTTON )
    {
        bool bPaintButton = (nState & ControlState::PRESSED)
            || (nState & ControlState::ROLLOVER);
        if( aValue.getTristateVal() == BUTTONVALUE_ON )
        {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pButtonWidget),TRUE);
            bPaintButton = true;
        }
        else
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pButtonWidget),FALSE);

        NWSetWidgetState( pButtonWidget, nState, stateType );
        gtk_widget_ensure_style( pButtonWidget );
        if(bPaintButton)
            NWPaintGTKButtonReal(pButtonWidget, gdkDrawable, 0, 0, rControlRectangle, rClipList, nState, aValue, string);
    }

    if( nPart != PART_BUTTON )
    {
        for( clipList::const_iterator it = rClipList.begin(); it != rClipList.end(); ++it )
        {
            clipRect.x = it->Left();
            clipRect.y = it->Top();
            clipRect.width = it->GetWidth();
            clipRect.height = it->GetHeight();

            // draw toolbar
            if( nPart == PART_DRAW_BACKGROUND_HORZ || nPart == PART_DRAW_BACKGROUND_VERT )
            {
                gtk_paint_flat_box( gWidgetData[m_nXScreen].gToolbarWidget->style,
                                    gdkDrawable,
                                    (GtkStateType)GTK_STATE_NORMAL,
                                    GTK_SHADOW_NONE,
                                    &clipRect,
                                    gWidgetData[m_nXScreen].gToolbarWidget,
                                    "base",
                                    x, y, w, h );
                gtk_paint_box( gWidgetData[m_nXScreen].gToolbarWidget->style,
                               gdkDrawable,
                               stateType,
                               shadowType,
                               &clipRect,
                               gWidgetData[m_nXScreen].gToolbarWidget,
                               "toolbar",
                               x, y, w, h );
            }
            // draw grip
            else if( nPart == PART_THUMB_HORZ || nPart == PART_THUMB_VERT )
            {
                gtk_paint_handle( gWidgetData[m_nXScreen].gHandleBoxWidget->style,
                                  gdkDrawable,
                                  GTK_STATE_NORMAL,
                                  GTK_SHADOW_OUT,
                                  &clipRect,
                                  gWidgetData[m_nXScreen].gHandleBoxWidget,
                                  "handlebox",
                                  g_x, g_y, g_w, g_h,
                                  nPart == PART_THUMB_HORZ ?
                                  GTK_ORIENTATION_HORIZONTAL :
                                  GTK_ORIENTATION_VERTICAL
                                  );
            }
            else if( nPart == PART_SEPARATOR_HORZ || nPart == PART_SEPARATOR_VERT )
            {
                const double shim = 0.2;

                gint separator_height, separator_width, wide_separators = 0;

                gtk_widget_style_get (gWidgetData[m_nXScreen].gSeparator,
                                      "wide-separators",  &wide_separators,
                                      "separator-width",  &separator_width,
                                      "separator-height", &separator_height,
                                      NULL);

                if (wide_separators)
                {
                    if (nPart == PART_SEPARATOR_VERT)
                        gtk_paint_box (gWidgetData[m_nXScreen].gSeparator->style, gdkDrawable,
                                       GTK_STATE_NORMAL, GTK_SHADOW_ETCHED_OUT,
                                       &clipRect, gWidgetData[m_nXScreen].gSeparator, "vseparator",
                                       x + (w - separator_width) / 2, y + h * shim,
                                       separator_width, h * (1 - 2*shim));
                    else
                        gtk_paint_box (gWidgetData[m_nXScreen].gSeparator->style, gdkDrawable,
                                       GTK_STATE_NORMAL, GTK_SHADOW_ETCHED_OUT,
                                       &clipRect, gWidgetData[m_nXScreen].gSeparator, "hseparator",
                                       x + w * shim, y + (h - separator_width) / 2,
                                       w * (1 - 2*shim), separator_width);
                }
                else
                {
                    if (nPart == PART_SEPARATOR_VERT)
                        gtk_paint_vline (gWidgetData[m_nXScreen].gSeparator->style, gdkDrawable,
                                         GTK_STATE_NORMAL,
                                         &clipRect, gWidgetData[m_nXScreen].gSeparator, "vseparator",
                                         y + h * shim, y + h * (1 - shim), x + w/2 - 1);
                    else
                        gtk_paint_hline (gWidgetData[m_nXScreen].gSeparator->style, gdkDrawable,
                                         GTK_STATE_NORMAL,
                                         &clipRect, gWidgetData[m_nXScreen].gSeparator, "hseparator",
                                         x + w * shim, x + w * (1 - shim), y + h/2 - 1);
                }
            }
        }
    }

    return true;
}

/// Converts a VCL Rectangle to a GdkRectangle.
static void lcl_rectangleToGdkRectangle(const Rectangle& rRectangle, GdkRectangle& rGdkRectangle)
{
    rGdkRectangle.x = rRectangle.Left();
    rGdkRectangle.y = rRectangle.Top();
    rGdkRectangle.width = rRectangle.GetWidth();
    rGdkRectangle.height = rRectangle.GetHeight();
}

bool GtkSalGraphics::NWPaintGTKMenubar(
            GdkDrawable* gdkDrawable,
            ControlType, ControlPart nPart,
            const Rectangle& rControlRectangle,
            const clipList& rClipList,
            ControlState nState, const ImplControlValue&,
            const OUString& )
{
    GtkStateType    stateType;
    GtkShadowType    shadowType;
    GtkShadowType   selected_shadow_type = GTK_SHADOW_OUT;
    gint            x, y, w, h;
    GdkRectangle    clipRect;

    NWEnsureGTKMenubar( m_nXScreen );
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    x = rControlRectangle.Left();
    y = rControlRectangle.Top();
    w = rControlRectangle.GetWidth();
    h = rControlRectangle.GetHeight();

    if( nPart == PART_MENU_ITEM )
    {
        if( nState & ControlState::SELECTED )
        {
            gtk_widget_style_get( gWidgetData[m_nXScreen].gMenuItemMenubarWidget,
                                  "selected_shadow_type", &selected_shadow_type,
                                  nullptr);
        }
    }

    for( clipList::const_iterator it = rClipList.begin(); it != rClipList.end(); ++it )
    {
        lcl_rectangleToGdkRectangle(*it, clipRect);

        // handle Menubar
        if( nPart == PART_ENTIRE_CONTROL )
        {
            NWSetWidgetState( gWidgetData[m_nXScreen].gMenubarWidget, nState, stateType );

            GTK_WIDGET_UNSET_FLAGS( gWidgetData[m_nXScreen].gMenubarWidget, GTK_SENSITIVE );
            if ( nState & ControlState::ENABLED )
                GTK_WIDGET_SET_FLAGS( gWidgetData[m_nXScreen].gMenubarWidget, GTK_SENSITIVE );

            // for translucent menubar styles paint background first
            gtk_paint_flat_box( gWidgetData[m_nXScreen].gMenubarWidget->style,
                                gdkDrawable,
                                GTK_STATE_NORMAL,
                                GTK_SHADOW_NONE,
                                &clipRect,
                                GTK_WIDGET(m_pWindow),
                                "base",
                                x, y, w, h );

            // Do the conversion again, in case clipRect has been modified.
            lcl_rectangleToGdkRectangle(*it, clipRect);

            gtk_paint_box( gWidgetData[m_nXScreen].gMenubarWidget->style,
                           gdkDrawable,
                           stateType,
                           shadowType,
                           &clipRect,
                           gWidgetData[m_nXScreen].gMenubarWidget,
                           "menubar",
                           x, y, w, h );
        }

        else if( nPart == PART_MENU_ITEM )
        {
            if( nState & ControlState::SELECTED )
            {
                gtk_paint_box( gWidgetData[m_nXScreen].gMenuItemMenubarWidget->style,
                               gdkDrawable,
                               GTK_STATE_PRELIGHT,
                               selected_shadow_type,
                               &clipRect,
                               gWidgetData[m_nXScreen].gMenuItemMenubarWidget,
                               "menuitem",
                               x, y, w, h);
            }
        }
    }

    return true;
}

bool GtkSalGraphics::NWPaintGTKPopupMenu(
            GdkDrawable* gdkDrawable,
            ControlType, ControlPart nPart,
            const Rectangle& rControlRectangle,
            const clipList& rClipList,
            ControlState nState, const ImplControlValue&,
            const OUString& )
{
    // #i50745# gtk does not draw disabled menu entries (and crux theme
    // even crashes) in very old (Fedora Core 4 vintage) gtk's
    if (gtk_major_version <= 2 && gtk_minor_version <= 8 &&
        nPart == PART_MENU_ITEM && ! (nState & ControlState::ENABLED) )
        return true;

    GtkStateType    stateType;
    GtkShadowType    shadowType;
    GtkShadowType   selected_shadow_type = GTK_SHADOW_OUT;
    gint            x, y, w, h;
    GdkRectangle    clipRect;

    NWEnsureGTKMenu( m_nXScreen );
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    x = rControlRectangle.Left();
    y = rControlRectangle.Top();
    w = rControlRectangle.GetWidth();
    h = rControlRectangle.GetHeight();

    if( nPart == PART_MENU_ITEM &&
        ( nState & (ControlState::SELECTED|ControlState::ROLLOVER) ) )
    {
        gtk_widget_style_get( gWidgetData[m_nXScreen].gMenuItemMenuWidget,
                              "selected_shadow_type", &selected_shadow_type,
                              nullptr);
    }

    NWSetWidgetState( gWidgetData[m_nXScreen].gMenuWidget, nState, stateType );

    GTK_WIDGET_UNSET_FLAGS( gWidgetData[m_nXScreen].gMenuWidget, GTK_SENSITIVE );
    if ( nState & ControlState::ENABLED )
        GTK_WIDGET_SET_FLAGS( gWidgetData[m_nXScreen].gMenuWidget, GTK_SENSITIVE );

    for( clipList::const_iterator it = rClipList.begin(); it != rClipList.end(); ++it )
    {
        clipRect.x = it->Left();
        clipRect.y = it->Top();
        clipRect.width = it->GetWidth();
        clipRect.height = it->GetHeight();

        if( nPart == PART_ENTIRE_CONTROL )
        {
            // for translucent menubar styles paint background first
            gtk_paint_flat_box( gWidgetData[m_nXScreen].gMenuWidget->style,
                                gdkDrawable,
                                GTK_STATE_NORMAL,
                                GTK_SHADOW_NONE,
                                &clipRect,
                                GTK_WIDGET(m_pWindow),
                                "base",
                                x, y, w, h );
            gtk_paint_box( gWidgetData[m_nXScreen].gMenuWidget->style,
                           gdkDrawable,
                           GTK_STATE_NORMAL,
                           GTK_SHADOW_OUT,
                           &clipRect,
                           gWidgetData[m_nXScreen].gMenuWidget,
                           "menu",
                           x, y, w, h );
        }
        else if( nPart == PART_MENU_ITEM )
        {
            if( nState & (ControlState::SELECTED|ControlState::ROLLOVER) )
            {
                if( nState & ControlState::ENABLED )
                gtk_paint_box( gWidgetData[m_nXScreen].gMenuItemMenuWidget->style,
                               gdkDrawable,
                               GTK_STATE_PRELIGHT,
                               selected_shadow_type,
                               &clipRect,
                               gWidgetData[m_nXScreen].gMenuItemMenuWidget,
                               "menuitem",
                               x, y, w, h);
            }
        }
        else if( nPart == PART_MENU_ITEM_CHECK_MARK || nPart == PART_MENU_ITEM_RADIO_MARK )
        {
            GtkWidget* pWidget = (nPart == PART_MENU_ITEM_CHECK_MARK) ?
                                 gWidgetData[m_nXScreen].gMenuItemCheckMenuWidget :
                                 gWidgetData[m_nXScreen].gMenuItemRadioMenuWidget;

            GtkStateType nStateType;
            GtkShadowType nShadowType;
            NWConvertVCLStateToGTKState( nState, &nStateType, &nShadowType );

            if ( (nState & ControlState::SELECTED) && (nState & ControlState::ENABLED) )
                nStateType = GTK_STATE_PRELIGHT;

            NWSetWidgetState( pWidget, nState, nStateType );

            if ( nPart == PART_MENU_ITEM_CHECK_MARK )
            {
                gtk_paint_check( pWidget->style,
                                 gdkDrawable,
                                 nStateType,
                                 nShadowType,
                                 &clipRect,
                                 gWidgetData[m_nXScreen].gMenuItemMenuWidget,
                                 "check",
                                 x, y, w, h );
            }
            else
            {
                gtk_paint_option( pWidget->style,
                                  gdkDrawable,
                                  nStateType,
                                  nShadowType,
                                  &clipRect,
                                  gWidgetData[m_nXScreen].gMenuItemMenuWidget,
                                  "option",
                                  x, y, w, h );
            }
        }
        else if( nPart == PART_MENU_SEPARATOR )
        {
            gtk_paint_hline( gWidgetData[m_nXScreen].gMenuItemSeparatorMenuWidget->style,
                             gdkDrawable,
                             GTK_STATE_NORMAL,
                             &clipRect,
                             gWidgetData[m_nXScreen].gMenuItemSeparatorMenuWidget,
                             "menuitem",
                             x, x + w, y + h / 2);
        }
        else if( nPart == PART_MENU_SUBMENU_ARROW )
        {
            GtkStateType nStateType;
            GtkShadowType nShadowType;
            NWConvertVCLStateToGTKState( nState, &nStateType, &nShadowType );

            if ( (nState & ControlState::SELECTED) && (nState & ControlState::ENABLED) )
                nStateType = GTK_STATE_PRELIGHT;

            NWSetWidgetState( gWidgetData[m_nXScreen].gMenuItemMenuWidget,
                              nState, nStateType );

            GtkArrowType eArrow;
            if( AllSettings::GetLayoutRTL() )
                eArrow = GTK_ARROW_LEFT;
            else
                eArrow = GTK_ARROW_RIGHT;

            gtk_paint_arrow( gWidgetData[m_nXScreen].gMenuItemMenuWidget->style,
                             gdkDrawable,
                             nStateType,
                             nShadowType,
                             &clipRect,
                             gWidgetData[m_nXScreen].gMenuItemMenuWidget,
                             "menuitem",
                             eArrow, TRUE,
                             x, y, w, h);
        }
    }

    return true;
}

bool GtkSalGraphics::NWPaintGTKTooltip(
            GdkDrawable* gdkDrawable,
            ControlType, ControlPart,
            const Rectangle& rControlRectangle,
            const clipList& rClipList,
            ControlState, const ImplControlValue&,
            const OUString& )
{
    NWEnsureGTKTooltip( m_nXScreen );

    gint            x, y, w, h;
    GdkRectangle    clipRect;

    x = rControlRectangle.Left();
    y = rControlRectangle.Top();
    w = rControlRectangle.GetWidth();
    h = rControlRectangle.GetHeight();

    for( clipList::const_iterator it = rClipList.begin(); it != rClipList.end(); ++it )
    {
        clipRect.x = it->Left();
        clipRect.y = it->Top();
        clipRect.width = it->GetWidth();
        clipRect.height = it->GetHeight();

        gtk_paint_flat_box( gWidgetData[m_nXScreen].gTooltipPopup->style,
                            gdkDrawable,
                            GTK_STATE_NORMAL,
                            GTK_SHADOW_OUT,
                            &clipRect,
                            gWidgetData[m_nXScreen].gTooltipPopup,
                            "tooltip",
                            x, y, w, h );
    }

    return true;
}

bool GtkSalGraphics::NWPaintGTKListNode(
            GdkDrawable*,
            ControlType, ControlPart,
            const Rectangle& rControlRectangle,
            const clipList&,
            ControlState nState, const ImplControlValue& rValue,
            const OUString& )
{
    NWEnsureGTKTreeView( m_nXScreen );

    Rectangle aRect( rControlRectangle );
    aRect.Left() -= 2;
    aRect.Right() += 2;
    aRect.Top() -= 2;
    aRect.Bottom() += 2;
    gint            w, h;
    w = aRect.GetWidth();
    h = aRect.GetHeight();

    GtkStateType    stateType;
    GtkShadowType    shadowType;
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    ButtonValue aButtonValue = rValue.getTristateVal();
    GtkExpanderStyle eStyle = GTK_EXPANDER_EXPANDED;

    switch( aButtonValue )
    {
        case BUTTONVALUE_ON: eStyle = GTK_EXPANDER_EXPANDED;break;
        case BUTTONVALUE_OFF: eStyle = GTK_EXPANDER_COLLAPSED; break;
        default:
            break;
    }

    BEGIN_PIXMAP_RENDER( aRect, pixDrawable )
    {
        gtk_paint_expander( gWidgetData[m_nXScreen].gTreeView->style,
                            pixDrawable,
                            stateType,
                            nullptr,
                            gWidgetData[m_nXScreen].gTreeView,
                            "treeview",
                            w/2, h/2,
                            eStyle );
    }
    END_PIXMAP_RENDER( aRect )

    return true;
}

bool GtkSalGraphics::NWPaintGTKProgress(
            GdkDrawable*,
            ControlType, ControlPart,
            const Rectangle& rControlRectangle,
            const clipList&,
            ControlState, const ImplControlValue& rValue,
            const OUString& )
{
    NWEnsureGTKProgressBar( m_nXScreen );

    gint            w, h;
    w = rControlRectangle.GetWidth();
    h = rControlRectangle.GetHeight();
    Rectangle aRect( Point( 0, 0 ), Size( w, h ) );

    long nProgressWidth = rValue.getNumericVal();

    BEGIN_PIXMAP_RENDER( aRect, pixDrawable )
    {
        // paint background
        gtk_paint_flat_box(gWidgetData[m_nXScreen].gProgressBar->style, pixDrawable,
                               GTK_STATE_NORMAL, GTK_SHADOW_NONE, nullptr, m_pWindow, "base",
                               -rControlRectangle.Left(),-rControlRectangle.Top(),
                               rControlRectangle.Left()+w,rControlRectangle.Top()+h);

        gtk_paint_flat_box( gWidgetData[m_nXScreen].gProgressBar->style,
                            pixDrawable,
                            GTK_STATE_NORMAL,
                            GTK_SHADOW_NONE,
                            nullptr,
                            gWidgetData[m_nXScreen].gProgressBar,
                            "trough",
                            0, 0, w, h );
        if( nProgressWidth > 0 )
        {
            // paint progress
            if( AllSettings::GetLayoutRTL() )
            {
                gtk_paint_box( gWidgetData[m_nXScreen].gProgressBar->style,
                               pixDrawable,
                               GTK_STATE_PRELIGHT, GTK_SHADOW_OUT,
                               nullptr,
                               gWidgetData[m_nXScreen].gProgressBar,
                               "bar",
                               w-nProgressWidth, 0, nProgressWidth, h
                               );
            }
            else
            {
                gtk_paint_box( gWidgetData[m_nXScreen].gProgressBar->style,
                               pixDrawable,
                               GTK_STATE_PRELIGHT, GTK_SHADOW_OUT,
                               nullptr,
                               gWidgetData[m_nXScreen].gProgressBar,
                               "bar",
                               0, 0, nProgressWidth, h
                               );
            }
        }
    }
    END_PIXMAP_RENDER( rControlRectangle )

    return true;
}

bool GtkSalGraphics::NWPaintGTKSlider(
            GdkDrawable*,
            ControlType, ControlPart nPart,
            const Rectangle& rControlRectangle,
            const clipList&,
            ControlState nState, const ImplControlValue& rValue,
            const OUString& )
{
    OSL_ASSERT( rValue.getType() == CTRL_SLIDER );
    NWEnsureGTKSlider( m_nXScreen );

    gint            w, h;
    w = rControlRectangle.GetWidth();
    h = rControlRectangle.GetHeight();

    const SliderValue* pVal = static_cast<const SliderValue*>(&rValue);

    BEGIN_PIXMAP_RENDER( rControlRectangle, pixDrawable )
    {
        GtkWidget* pWidget = (nPart == PART_TRACK_HORZ_AREA)
                             ? GTK_WIDGET(gWidgetData[m_nXScreen].gHScale)
                             : GTK_WIDGET(gWidgetData[m_nXScreen].gVScale);
        const gchar* pDetail = (nPart == PART_TRACK_HORZ_AREA) ? "hscale" : "vscale";
        GtkOrientation eOri = (nPart == PART_TRACK_HORZ_AREA) ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL;
        gint slider_width = 10;
        gint slider_length = 10;
        gint trough_border = 0;
        gtk_widget_style_get( pWidget,
                              "slider-width", &slider_width,
                              "slider-length", &slider_length,
                              "trough-border", &trough_border,
                              NULL);

        GtkStateType eState = (nState & ControlState::ENABLED) ? GTK_STATE_NORMAL : GTK_STATE_INSENSITIVE;
        if( nPart == PART_TRACK_HORZ_AREA )
        {
            gtk_paint_box( pWidget->style,
                           pixDrawable,
                           eState,
                           GTK_SHADOW_IN,
                           nullptr,
                           pWidget,
                           "trough",
                           0, (h-slider_width-2*trough_border)/2, w, slider_width + 2*trough_border);
            gint x = (w - slider_length + 1) * (pVal->mnCur - pVal->mnMin) / (pVal->mnMax - pVal->mnMin);
            gtk_paint_slider( pWidget->style,
                              pixDrawable,
                              eState,
                              GTK_SHADOW_OUT,
                              nullptr,
                              pWidget,
                              pDetail,
                              x, (h-slider_width)/2,
                              slider_length, slider_width,
                              eOri );
        }
        else
        {
            gtk_paint_box( pWidget->style,
                           pixDrawable,
                           eState,
                           GTK_SHADOW_IN,
                           nullptr,
                           pWidget,
                           "trough",
                           (w-slider_width-2*trough_border)/2, 0, slider_width + 2*trough_border, h);
            gint y = (h - slider_length + 1) * (pVal->mnCur - pVal->mnMin) / (pVal->mnMax - pVal->mnMin);
            gtk_paint_slider( pWidget->style,
                              pixDrawable,
                              eState,
                              GTK_SHADOW_OUT,
                              nullptr,
                              pWidget,
                              pDetail,
                              (w-slider_width)/2, y,
                              slider_width, slider_length,
                              eOri );
        }
    }
    END_PIXMAP_RENDER( rControlRectangle )

    return true;
}

static int getFrameWidth(GtkWidget* widget)
{
    return widget->style->xthickness;
}

static Rectangle NWGetListBoxButtonRect( SalX11Screen nScreen,
                                         ControlType,
                                         ControlPart    nPart,
                                         Rectangle      aAreaRect,
                                         ControlState,
                                         const ImplControlValue&,
                                         const OUString& )
{
    Rectangle       aPartRect;
    GtkRequisition *pIndicatorSize = nullptr;
    GtkBorder      *pIndicatorSpacing = nullptr;
    gint            width = 13;    // GTK+ default
    gint            right = 5;    // GTK+ default
    gint            nButtonAreaWidth = 0;
    gint            xthickness = 0;

    NWEnsureGTKOptionMenu( nScreen );

    gtk_widget_style_get( gWidgetData[nScreen].gOptionMenuWidget,
            "indicator_size",    &pIndicatorSize,
            "indicator_spacing",&pIndicatorSpacing, nullptr);

    if ( pIndicatorSize )
        width = pIndicatorSize->width;

    if ( pIndicatorSpacing )
        right = pIndicatorSpacing->right;

    Size aPartSize( 0, aAreaRect.GetHeight() );
    Point aPartPos ( 0, aAreaRect.Top() );

    xthickness = gWidgetData[nScreen].gOptionMenuWidget->style->xthickness;
    nButtonAreaWidth = width + right + (xthickness * 2);
    switch( nPart )
    {
        case PART_BUTTON_DOWN:
            aPartSize.Width() = nButtonAreaWidth;
            aPartPos.X() = aAreaRect.Left() + aAreaRect.GetWidth() - aPartSize.Width();
            break;

        case PART_SUB_EDIT:
            aPartSize.Width() = aAreaRect.GetWidth() - nButtonAreaWidth - xthickness;
            if( AllSettings::GetLayoutRTL() )
                aPartPos.X() = aAreaRect.Left() + nButtonAreaWidth;
            else
                aPartPos.X() = aAreaRect.Left() + xthickness;
            break;

        default:
            aPartSize.Width() = aAreaRect.GetWidth();
            aPartPos.X() = aAreaRect.Left();
            break;
    }
    aPartRect = Rectangle( aPartPos, aPartSize );

    if ( pIndicatorSize )
        gtk_requisition_free( pIndicatorSize );
    if ( pIndicatorSpacing )
        gtk_border_free( pIndicatorSpacing );

    return aPartRect;
}

static Rectangle NWGetListBoxIndicatorRect( SalX11Screen nScreen,
                                            ControlType,
                                            ControlPart,
                                            Rectangle                aAreaRect,
                                            ControlState,
                                            const ImplControlValue&,
                                            const OUString& )
{
    Rectangle       aIndicatorRect;
    GtkRequisition *pIndicatorSize = nullptr;
    GtkBorder      *pIndicatorSpacing = nullptr;
    gint            width = 13;    // GTK+ default
    gint            height = 13;    // GTK+ default
    gint            right = 5;    // GTK+ default
    gint            x;

    NWEnsureGTKOptionMenu( nScreen );

    gtk_widget_style_get( gWidgetData[nScreen].gOptionMenuWidget,
            "indicator_size",    &pIndicatorSize,
            "indicator_spacing",&pIndicatorSpacing, nullptr);

    if ( pIndicatorSize )
    {
        width = pIndicatorSize->width;
        height = pIndicatorSize->height;
    }

    if ( pIndicatorSpacing )
        right = pIndicatorSpacing->right;

    aIndicatorRect.SetSize( Size( width, height ) );
    if( AllSettings::GetLayoutRTL() )
        x = aAreaRect.Left() + right;
    else
        x = aAreaRect.Left() + aAreaRect.GetWidth() - width - right - gWidgetData[nScreen].gOptionMenuWidget->style->xthickness;
    aIndicatorRect.SetPos( Point( x, aAreaRect.Top() + ((aAreaRect.GetHeight() - height) / 2) ) );

    // If height is odd, move the indicator down 1 pixel
    if ( aIndicatorRect.GetHeight() % 2 )
        aIndicatorRect.Move( 0, 1 );

    if ( pIndicatorSize )
        gtk_requisition_free( pIndicatorSize );
    if ( pIndicatorSpacing )
        gtk_border_free( pIndicatorSpacing );

    return aIndicatorRect;
}

static Rectangle NWGetToolbarRect(  SalX11Screen nScreen,
                                    ControlType,
                                    ControlPart                nPart,
                                    Rectangle                aAreaRect,
                                    ControlState,
                                    const ImplControlValue&,
                                    const OUString& )
{
    Rectangle aRet;

    if( nPart == PART_DRAW_BACKGROUND_HORZ ||
        nPart == PART_DRAW_BACKGROUND_VERT )
        aRet = aAreaRect;
    else if( nPart == PART_THUMB_HORZ )
        aRet = Rectangle( Point( 0, 0 ), Size( aAreaRect.GetWidth(), 10 ) );
    else if( nPart == PART_THUMB_VERT )
        aRet = Rectangle( Point( 0, 0 ), Size( 10, aAreaRect.GetHeight() ) );
    else if( nPart == PART_BUTTON )
    {
        aRet = aAreaRect;

        NWEnsureGTKToolbar( nScreen );

        gint nMinWidth =
            2*gWidgetData[nScreen].gToolbarButtonWidget->style->xthickness
            + 1 // CHILD_SPACING constant, found in gtk_button.c
            + 3*gWidgetData[nScreen].gToolbarButtonWidget->style->xthickness; // Murphy factor
        gint nMinHeight =
            2*gWidgetData[nScreen].gToolbarButtonWidget->style->ythickness
            + 1 // CHILD_SPACING constant, found in gtk_button.c
            + 3*gWidgetData[nScreen].gToolbarButtonWidget->style->ythickness; // Murphy factor

        gtk_widget_ensure_style( gWidgetData[nScreen].gToolbarButtonWidget );
        if( aAreaRect.GetWidth() < nMinWidth )
            aRet.Right() = aRet.Left() + nMinWidth;
        if( aAreaRect.GetHeight() < nMinHeight  )
            aRet.Bottom() = aRet.Top() + nMinHeight;
    }

    return aRet;
}

/************************************************************************
 * helper for GtkSalFrame
 ************************************************************************/
static inline Color getColor( const GdkColor& rCol )
{
    return Color( rCol.red >> 8, rCol.green >> 8, rCol.blue >> 8 );
}

#if OSL_DEBUG_LEVEL > 1

void printColor( const char* name, const GdkColor& rCol )
{
    std::fprintf( stderr, "   %s = 0x%2x 0x%2x 0x%2x\n",
             name,
             rCol.red >> 8, rCol.green >> 8, rCol.blue >> 8 );
}

void printStyleColors( GtkStyle* pStyle )
{
    static const char* pStates[] = { "NORMAL", "ACTIVE", "PRELIGHT", "SELECTED", "INSENSITIVE" };

    for( int i = 0; i < 5; i++ )
    {
        std::fprintf( stderr, "state %s colors:\n", pStates[i] );
        printColor( "bg     ", pStyle->bg[i] );
        printColor( "fg     ", pStyle->fg[i] );
        printColor( "light  ", pStyle->light[i] );
        printColor( "dark   ", pStyle->dark[i] );
        printColor( "mid    ", pStyle->mid[i] );
        printColor( "text   ", pStyle->text[i] );
        printColor( "base   ", pStyle->base[i] );
        printColor( "text_aa", pStyle->text_aa[i] );
    }
}
#endif

void GtkSalGraphics::signalSettingsNotify( GObject *pSettings, GParamSpec *pSpec, gpointer )
{
    g_return_if_fail( pSpec != nullptr );

    if( !strcmp( pSpec->name, "gtk-fontconfig-timestamp" ) )
        GtkSalGraphics::refreshFontconfig( GTK_SETTINGS( pSettings ) );
}

void GtkSalGraphics::refreshFontconfig( GtkSettings *pSettings )
{
    guint latest_fontconfig_timestamp = 0;
    static guint our_fontconfig_timestamp = 0;
    g_object_get( pSettings, "gtk-fontconfig-timestamp", &latest_fontconfig_timestamp, nullptr );
    if (latest_fontconfig_timestamp != our_fontconfig_timestamp)
    {
        bool bFirstTime = our_fontconfig_timestamp == 0;
        our_fontconfig_timestamp = latest_fontconfig_timestamp;
        if (!bFirstTime)
        {
            psp::PrintFontManager::get().initialize();
        }
    }
}

void GtkSalGraphics::updateSettings( AllSettings& rSettings )
{
    gtk_widget_ensure_style( m_pWindow );
    GtkStyle* pStyle = gtk_widget_get_style( m_pWindow );
    GtkSettings* pSettings = gtk_widget_get_settings( m_pWindow );
    StyleSettings aStyleSet = rSettings.GetStyleSettings();

    // Listen for font changes
    if( !g_object_get_data( G_OBJECT( pSettings ), "libo:listening" ) )
    {
        g_object_set_data( G_OBJECT( pSettings ), "libo:listening",
                           GUINT_TO_POINTER( 1 ) );
        g_signal_connect_data( G_OBJECT( pSettings ), "notify",
                               G_CALLBACK( signalSettingsNotify ),
                               nullptr, nullptr, G_CONNECT_AFTER );
    }

    refreshFontconfig( pSettings );

    // get the widgets in place
    NWEnsureGTKMenu( m_nXScreen );
    NWEnsureGTKMenubar( m_nXScreen );
    NWEnsureGTKScrollbars( m_nXScreen );
    NWEnsureGTKEditBox( m_nXScreen );
    NWEnsureGTKTooltip( m_nXScreen );
    NWEnsureGTKDialog( m_nXScreen );
    NWEnsureGTKFrame( m_nXScreen );

#if OSL_DEBUG_LEVEL > 2
    printStyleColors( pStyle );
#endif

    // text colors
    Color aTextColor = getColor( pStyle->text[GTK_STATE_NORMAL] );
    aStyleSet.SetDialogTextColor( aTextColor );
    aStyleSet.SetWindowTextColor( aTextColor );
    aStyleSet.SetFieldTextColor( aTextColor );
    aTextColor = getColor( pStyle->fg[GTK_STATE_NORMAL] );
    aStyleSet.SetButtonTextColor( aTextColor );
    aStyleSet.SetRadioCheckTextColor( aTextColor );
    aStyleSet.SetGroupTextColor( aTextColor );
    aStyleSet.SetLabelTextColor( aTextColor );
    aStyleSet.SetInfoTextColor( aTextColor );
    aStyleSet.SetTabTextColor( aTextColor );
    aStyleSet.SetTabRolloverTextColor( aTextColor );
    aStyleSet.SetTabHighlightTextColor( aTextColor );

    // Tooltip colors
    GtkStyle* pTooltipStyle = gtk_widget_get_style( gWidgetData[m_nXScreen].gTooltipPopup );
    aTextColor = getColor( pTooltipStyle->fg[ GTK_STATE_NORMAL ] );
    aStyleSet.SetHelpTextColor( aTextColor );

    DialogStyle aDialogStyle(aStyleSet.GetDialogStyle());
    gtk_widget_style_get (gWidgetData[m_nXScreen].gDialog,
        "content-area-border", &aDialogStyle.content_area_border,
        "content-area-spacing", &aDialogStyle.content_area_spacing,
        "button-spacing", &aDialogStyle.button_spacing,
        "action-area-border", &aDialogStyle.action_area_border,
        NULL);
    aStyleSet.SetDialogStyle(aDialogStyle);

    FrameStyle aFrameStyle(aStyleSet.GetFrameStyle());
    aFrameStyle.left = aFrameStyle.right =
        gWidgetData[m_nXScreen].gFrame->style->xthickness;
    aFrameStyle.top = aFrameStyle.bottom =
        gWidgetData[m_nXScreen].gFrame->style->ythickness;
    aStyleSet.SetFrameStyle(aFrameStyle);

    // mouse over text colors
    aTextColor = getColor( pStyle->fg[ GTK_STATE_PRELIGHT ] );
    aStyleSet.SetButtonRolloverTextColor( aTextColor );
    aStyleSet.SetFieldRolloverTextColor( aTextColor );

    // background colors
    Color aBackColor = getColor( pStyle->bg[GTK_STATE_NORMAL] );
    Color aBackFieldColor = getColor( pStyle->base[ GTK_STATE_NORMAL ] );
    aStyleSet.Set3DColors( aBackColor );
    aStyleSet.SetFaceColor( aBackColor );
    aStyleSet.SetDialogColor( aBackColor );
    aStyleSet.SetWorkspaceColor( aBackColor );
    aStyleSet.SetFieldColor( aBackFieldColor );
    aStyleSet.SetWindowColor( aBackFieldColor );
    aStyleSet.SetCheckedColorSpecialCase( );

    // highlighting colors
    Color aHighlightColor = getColor( pStyle->base[GTK_STATE_SELECTED] );
    Color aHighlightTextColor = getColor( pStyle->text[GTK_STATE_SELECTED] );
    aStyleSet.SetHighlightColor( aHighlightColor );
    aStyleSet.SetHighlightTextColor( aHighlightTextColor );

    // hyperlink colors
    GdkColor *link_color = nullptr;
    gtk_widget_style_get (m_pWindow, "link-color", &link_color, NULL);
    if (link_color)
    {
        aStyleSet.SetLinkColor(getColor(*link_color));
        gdk_color_free (link_color);
        link_color = nullptr;
    }
    gtk_widget_style_get (m_pWindow, "visited-link-color", &link_color, NULL);
    if (link_color)
    {
        aStyleSet.SetVisitedLinkColor(getColor(*link_color));
        gdk_color_free (link_color);
    }

    // Tab colors
    aStyleSet.SetActiveTabColor( aBackFieldColor ); // same as the window color.
    Color aSelectedBackColor = getColor( pStyle->bg[GTK_STATE_ACTIVE] );
    aStyleSet.SetInactiveTabColor( aSelectedBackColor );

    // menu disabled entries handling
    aStyleSet.SetSkipDisabledInMenus( true );
    aStyleSet.SetAcceleratorsInContextMenus( false );
    // menu colors
    GtkStyle* pMenuStyle = gtk_widget_get_style( gWidgetData[m_nXScreen].gMenuWidget );
    GtkStyle* pMenuItemStyle = gtk_rc_get_style( gWidgetData[m_nXScreen].gMenuItemMenuWidget );
    GtkStyle* pMenubarStyle = gtk_rc_get_style( gWidgetData[m_nXScreen].gMenubarWidget );
    GtkStyle* pMenuTextStyle = gtk_rc_get_style( gtk_bin_get_child( GTK_BIN(gWidgetData[m_nXScreen].gMenuItemMenuWidget) ) );

    aBackColor = getColor( pMenubarStyle->bg[GTK_STATE_NORMAL] );
    aStyleSet.SetMenuBarColor( aBackColor );
    aStyleSet.SetMenuBarRolloverColor( aBackColor );
    aBackColor = getColor( pMenuStyle->bg[GTK_STATE_NORMAL] );
    aTextColor = getColor( pMenuTextStyle->fg[GTK_STATE_NORMAL] );
    aStyleSet.SetMenuColor( aBackColor );
    aStyleSet.SetMenuTextColor( aTextColor );

    aTextColor = aStyleSet.GetPersonaMenuBarTextColor().get_value_or( getColor( pMenubarStyle->fg[GTK_STATE_NORMAL] ) );
    aStyleSet.SetMenuBarTextColor( aTextColor );
    aStyleSet.SetMenuBarRolloverTextColor(getColor(pMenubarStyle->fg[GTK_STATE_PRELIGHT]));
    aStyleSet.SetMenuBarHighlightTextColor(getColor(pMenubarStyle->fg[GTK_STATE_SELECTED]));

#if OSL_DEBUG_LEVEL > 1
    std::fprintf( stderr, "==\n" );
    std::fprintf( stderr, "MenuColor = %x (%d)\n", (int)aStyleSet.GetMenuColor().GetColor(), aStyleSet.GetMenuColor().GetLuminance() );
    std::fprintf( stderr, "MenuTextColor = %x (%d)\n", (int)aStyleSet.GetMenuTextColor().GetColor(), aStyleSet.GetMenuTextColor().GetLuminance() );
    std::fprintf( stderr, "MenuBarColor = %x (%d)\n", (int)aStyleSet.GetMenuBarColor().GetColor(), aStyleSet.GetMenuBarColor().GetLuminance() );
    std::fprintf( stderr, "MenuBarRolloverColor = %x (%d)\n", (int)aStyleSet.GetMenuBarRolloverColor().GetColor(), aStyleSet.GetMenuBarRolloverColor().GetLuminance() );
    std::fprintf( stderr, "MenuBarTextColor = %x (%d)\n", (int)aStyleSet.GetMenuBarTextColor().GetColor(), aStyleSet.GetMenuBarTextColor().GetLuminance() );
    std::fprintf( stderr, "MenuBarRolloverTextColor = %x (%d)\n", (int)aStyleSet.GetMenuBarRolloverTextColor().GetColor(), aStyleSet.GetMenuBarRolloverTextColor().GetLuminance() );
    std::fprintf( stderr, "LightColor = %x (%d)\n", (int)aStyleSet.GetLightColor().GetColor(), aStyleSet.GetLightColor().GetLuminance() );
    std::fprintf( stderr, "ShadowColor = %x (%d)\n", (int)aStyleSet.GetShadowColor().GetColor(), aStyleSet.GetShadowColor().GetLuminance() );
    std::fprintf( stderr, "DarkShadowColor = %x (%d)\n", (int)aStyleSet.GetDarkShadowColor().GetColor(), aStyleSet.GetDarkShadowColor().GetLuminance() );
#endif

    // Awful hack for menu separators in the Sonar and similar themes.
    // If the menu color is not too dark, and the menu text color is lighter,
    // make the "light" color lighter than the menu color and the "shadow"
    // color darker than it.
    if ( aStyleSet.GetMenuColor().GetLuminance() >= 32 &&
     aStyleSet.GetMenuColor().GetLuminance() <= aStyleSet.GetMenuTextColor().GetLuminance() )
    {
      Color temp = aStyleSet.GetMenuColor();
      temp.IncreaseLuminance( 8 );
      aStyleSet.SetLightColor( temp );
      temp = aStyleSet.GetMenuColor();
      temp.DecreaseLuminance( 16 );
      aStyleSet.SetShadowColor( temp );
    }

    aHighlightColor = getColor( pMenuItemStyle->bg[ GTK_STATE_SELECTED ] );
    aHighlightTextColor = getColor( pMenuItemStyle->fg[ GTK_STATE_SELECTED ] );
    aStyleSet.SetMenuHighlightColor( aHighlightColor );
    aStyleSet.SetMenuHighlightTextColor( aHighlightTextColor );

    // UI font
    OString    aFamily        = pango_font_description_get_family( pStyle->font_desc );
    int nPangoHeight    = pango_font_description_get_size( pStyle->font_desc );
    PangoStyle    eStyle    = pango_font_description_get_style( pStyle->font_desc );
    PangoWeight    eWeight    = pango_font_description_get_weight( pStyle->font_desc );
    PangoStretch eStretch = pango_font_description_get_stretch( pStyle->font_desc );

    psp::FastPrintFontInfo aInfo;
    // set family name
    aInfo.m_aFamilyName = OStringToOUString( aFamily, RTL_TEXTENCODING_UTF8 );
    // set italic
    switch( eStyle )
    {
        case PANGO_STYLE_NORMAL:    aInfo.m_eItalic = ITALIC_NONE;break;
        case PANGO_STYLE_ITALIC:    aInfo.m_eItalic = ITALIC_NORMAL;break;
        case PANGO_STYLE_OBLIQUE:    aInfo.m_eItalic = ITALIC_OBLIQUE;break;
    }
    // set weight
    if( eWeight <= PANGO_WEIGHT_ULTRALIGHT )
        aInfo.m_eWeight = WEIGHT_ULTRALIGHT;
    else if( eWeight <= PANGO_WEIGHT_LIGHT )
        aInfo.m_eWeight = WEIGHT_LIGHT;
    else if( eWeight <= PANGO_WEIGHT_NORMAL )
        aInfo.m_eWeight = WEIGHT_NORMAL;
    else if( eWeight <= PANGO_WEIGHT_BOLD )
        aInfo.m_eWeight = WEIGHT_BOLD;
    else
        aInfo.m_eWeight = WEIGHT_ULTRABOLD;
    // set width
    switch( eStretch )
    {
        case PANGO_STRETCH_ULTRA_CONDENSED:    aInfo.m_eWidth = WIDTH_ULTRA_CONDENSED;break;
        case PANGO_STRETCH_EXTRA_CONDENSED:    aInfo.m_eWidth = WIDTH_EXTRA_CONDENSED;break;
        case PANGO_STRETCH_CONDENSED:        aInfo.m_eWidth = WIDTH_CONDENSED;break;
        case PANGO_STRETCH_SEMI_CONDENSED:    aInfo.m_eWidth = WIDTH_SEMI_CONDENSED;break;
        case PANGO_STRETCH_NORMAL:            aInfo.m_eWidth = WIDTH_NORMAL;break;
        case PANGO_STRETCH_SEMI_EXPANDED:    aInfo.m_eWidth = WIDTH_SEMI_EXPANDED;break;
        case PANGO_STRETCH_EXPANDED:        aInfo.m_eWidth = WIDTH_EXPANDED;break;
        case PANGO_STRETCH_EXTRA_EXPANDED:    aInfo.m_eWidth = WIDTH_EXTRA_EXPANDED;break;
        case PANGO_STRETCH_ULTRA_EXPANDED:    aInfo.m_eWidth = WIDTH_ULTRA_EXPANDED;break;
    }

#if OSL_DEBUG_LEVEL > 1
    std::fprintf( stderr, "font name BEFORE system match: \"%s\"\n", aFamily.getStr() );
#endif

    // match font to e.g. resolve "Sans"
    psp::PrintFontManager::get().matchFont( aInfo, rSettings.GetUILanguageTag().getLocale() );

#if OSL_DEBUG_LEVEL > 1
    std::fprintf( stderr, "font match %s, name AFTER: \"%s\"\n",
             aInfo.m_nID != 0 ? "succeeded" : "failed",
             OUStringToOString( aInfo.m_aFamilyName, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif

    sal_Int32 nDispDPIY = GetDisplay()->GetResolution().B();
    int nPointHeight = 0;
    static gboolean(*pAbso)(const PangoFontDescription*) =
        reinterpret_cast<gboolean(*)(const PangoFontDescription*)>(osl_getAsciiFunctionSymbol( GetSalData()->m_pPlugin, "pango_font_description_get_size_is_absolute" ));

    if( pAbso && pAbso( pStyle->font_desc ) )
        nPointHeight = (nPangoHeight * 72 + nDispDPIY*PANGO_SCALE/2) / (nDispDPIY * PANGO_SCALE);
    else
        nPointHeight = nPangoHeight/PANGO_SCALE;

    vcl::Font aFont( aInfo.m_aFamilyName, Size( 0, nPointHeight ) );
    if( aInfo.m_eWeight != WEIGHT_DONTKNOW )
        aFont.SetWeight( aInfo.m_eWeight );
    if( aInfo.m_eWidth != WIDTH_DONTKNOW )
        aFont.SetWidthType( aInfo.m_eWidth );
    if( aInfo.m_eItalic != ITALIC_DONTKNOW )
        aFont.SetItalic( aInfo.m_eItalic );
    if( aInfo.m_ePitch != PITCH_DONTKNOW )
        aFont.SetPitch( aInfo.m_ePitch );

    aStyleSet.SetAppFont( aFont );
    aStyleSet.SetHelpFont( aFont );
    aStyleSet.SetMenuFont( aFont );
    aStyleSet.SetToolFont( aFont );
    aStyleSet.SetLabelFont( aFont );
    aStyleSet.SetInfoFont( aFont );
    aStyleSet.SetRadioCheckFont( aFont );
    aStyleSet.SetPushButtonFont( aFont );
    aStyleSet.SetFieldFont( aFont );
    aStyleSet.SetIconFont( aFont );
    aStyleSet.SetTabFont( aFont );
    aStyleSet.SetGroupFont( aFont );

    aFont.SetWeight( WEIGHT_BOLD );
    aStyleSet.SetTitleFont( aFont );
    aStyleSet.SetFloatTitleFont( aFont );

    // get cursor blink time
    gboolean blink = false;

    g_object_get( pSettings, "gtk-cursor-blink", &blink, nullptr );
    if( blink )
    {
        gint blink_time = static_cast<gint>(STYLE_CURSOR_NOBLINKTIME);
        g_object_get( pSettings, "gtk-cursor-blink-time", &blink_time, nullptr );
        // set the blink_time if there is a setting and it is reasonable
        // else leave the default value
        if( blink_time > 100 && blink_time != gint(STYLE_CURSOR_NOBLINKTIME) )
            aStyleSet.SetCursorBlinkTime( blink_time/2 );
    }
    else
        aStyleSet.SetCursorBlinkTime( STYLE_CURSOR_NOBLINKTIME );

    MouseSettings aMouseSettings = rSettings.GetMouseSettings();
    int iDoubleClickTime, iDoubleClickDistance, iDragThreshold, iMenuPopupDelay;
    g_object_get( pSettings,
                  "gtk-double-click-time", &iDoubleClickTime,
                  "gtk-double-click-distance", &iDoubleClickDistance,
                  "gtk-dnd-drag-threshold", &iDragThreshold,
                  "gtk-menu-popup-delay", &iMenuPopupDelay,
                  nullptr );
    aMouseSettings.SetDoubleClickTime( iDoubleClickTime );
    aMouseSettings.SetDoubleClickWidth( iDoubleClickDistance );
    aMouseSettings.SetDoubleClickHeight( iDoubleClickDistance );
    aMouseSettings.SetStartDragWidth( iDragThreshold );
    aMouseSettings.SetStartDragHeight( iDragThreshold );
    aMouseSettings.SetMenuDelay( iMenuPopupDelay );
    rSettings.SetMouseSettings( aMouseSettings );

    gboolean showmenuicons = true, primarybuttonwarps = false;
    g_object_get( pSettings,
        "gtk-menu-images", &showmenuicons,
        nullptr );
    if( g_object_class_find_property(
            G_OBJECT_GET_CLASS(pSettings), "gtk-primary-button-warps-slider") )
    {
        g_object_get( pSettings,
            "gtk-primary-button-warps-slider", &primarybuttonwarps,
            nullptr );
    }
    aStyleSet.SetPreferredUseImagesInMenus(showmenuicons);
    aStyleSet.SetPrimaryButtonWarpsSlider(primarybuttonwarps);

    // set scrollbar settings
    gint slider_width = 14;
    gint trough_border = 1;
    gint min_slider_length = 21;

    // Grab some button style attributes
    gtk_widget_style_get( gWidgetData[m_nXScreen].gScrollHorizWidget,
                          "slider-width", &slider_width,
                          "trough-border", &trough_border,
                          "min-slider-length", &min_slider_length,
                          nullptr );
    gint magic = trough_border ? 1 : 0;
    aStyleSet.SetScrollBarSize( slider_width + 2*trough_border );
    aStyleSet.SetMinThumbSize( min_slider_length - magic );

    // preferred icon style
    gchar* pIconThemeName = nullptr;
    g_object_get( pSettings, "gtk-icon-theme-name", &pIconThemeName, nullptr );
    aStyleSet.SetPreferredIconTheme( OUString::createFromAscii( pIconThemeName ) );
    g_free( pIconThemeName );

    aStyleSet.SetToolbarIconSize( ToolbarIconSize::Large );

    // finally update the collected settings
    rSettings.SetStyleSettings( aStyleSet );
}

/************************************************************************
 * Create a GdkPixmap filled with the contents of an area of an Xlib window
 ************************************************************************/

GdkX11Pixmap* GtkSalGraphics::NWGetPixmapFromScreen( Rectangle srcRect, int nBgColor )
{
    GdkX11Pixmap* pPixmap;
    int nDepth = vcl_sal::getSalDisplay(GetGenericData())->GetVisual( m_nXScreen ).GetDepth();

    pPixmap = new GdkX11Pixmap( srcRect.GetWidth(), srcRect.GetHeight(), nDepth );

    if( nBgColor == BG_FILL )
    {
        FillPixmapFromScreen( pPixmap, srcRect.Left(), srcRect.Top() );
    }
    else if( nBgColor != BG_NONE )
    {
        cairo_t *cr = gdk_cairo_create( pPixmap->GetGdkDrawable() );
        if( nBgColor == BG_BLACK)
            cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
        else
            cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 1.0);
        cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
        cairo_paint (cr);
        cairo_destroy(cr);
    }

    return pPixmap;
}

/************************************************************************
 * Copy an alpha pixmap to screen using a gc with clipping
 ************************************************************************/

bool GtkSalGraphics::NWRenderPixmapToScreen( GdkX11Pixmap* pPixmap, GdkX11Pixmap* pMask, Rectangle dstRect )
{
    return RenderPixmapToScreen( pPixmap, pMask, dstRect.Left(), dstRect.Top() );
}

/************************************************************************
 * State conversion
 ************************************************************************/
static void NWConvertVCLStateToGTKState( ControlState nVCLState,
            GtkStateType* nGTKState, GtkShadowType* nGTKShadow )
{
    *nGTKShadow = GTK_SHADOW_OUT;
    *nGTKState = GTK_STATE_INSENSITIVE;

    if ( nVCLState & ControlState::ENABLED )
    {
        if ( nVCLState & ControlState::PRESSED )
        {
            *nGTKState = GTK_STATE_ACTIVE;
            *nGTKShadow = GTK_SHADOW_IN;
        }
        else if ( nVCLState & ControlState::ROLLOVER )
        {
            *nGTKState = GTK_STATE_PRELIGHT;
            *nGTKShadow = GTK_SHADOW_OUT;
        }
        else
        {
            *nGTKState = GTK_STATE_NORMAL;
            *nGTKShadow = GTK_SHADOW_OUT;
        }
    }
}

/************************************************************************
 * Set widget flags
 ************************************************************************/
static void NWSetWidgetState( GtkWidget* widget, ControlState nState, GtkStateType nGtkState )
{
    // Set to default state, then build up from there
    GTK_WIDGET_UNSET_FLAGS( widget, GTK_HAS_DEFAULT );
    GTK_WIDGET_UNSET_FLAGS( widget, GTK_HAS_FOCUS );
    GTK_WIDGET_UNSET_FLAGS( widget, GTK_SENSITIVE );
    GTK_WIDGET_SET_FLAGS( widget, gWidgetDefaultFlags[reinterpret_cast<long>(widget)] );

    if ( nState & ControlState::DEFAULT )
        GTK_WIDGET_SET_FLAGS( widget, GTK_HAS_DEFAULT );
    if ( !GTK_IS_TOGGLE_BUTTON(widget) && (nState & ControlState::FOCUSED) )
        GTK_WIDGET_SET_FLAGS( widget, GTK_HAS_FOCUS );
    if ( nState & ControlState::ENABLED )
        GTK_WIDGET_SET_FLAGS( widget, GTK_SENSITIVE );
    gtk_widget_set_state( widget, nGtkState );
}

/************************************************************************
 * Widget ensure functions - make sure cached objects are valid
 ************************************************************************/

static void NWAddWidgetToCacheWindow( GtkWidget* widget, SalX11Screen nScreen )
{
    NWFWidgetData& rData = gWidgetData[nScreen];
    if ( !rData.gCacheWindow || !rData.gDumbContainer )
    {
        if ( !rData.gCacheWindow )
        {
            rData.gCacheWindow = gtk_window_new( GTK_WINDOW_TOPLEVEL );
            g_object_set_data( G_OBJECT( rData.gCacheWindow ), "libo-version",
                               const_cast<char *>(LIBO_VERSION_DOTTED) );

            GdkScreen* pScreen = gdk_display_get_screen( gdk_display_get_default(),
                                                         nScreen.getXScreen() );
            if( pScreen )
                gtk_window_set_screen( GTK_WINDOW(rData.gCacheWindow), pScreen );
        }
        if ( !rData.gDumbContainer )
            rData.gDumbContainer = gtk_fixed_new();
        gtk_container_add( GTK_CONTAINER(rData.gCacheWindow), rData.gDumbContainer );
        gtk_widget_realize( rData.gDumbContainer );
        gtk_widget_realize( rData.gCacheWindow );
    }

    gtk_container_add( GTK_CONTAINER(rData.gDumbContainer), widget );
    gtk_widget_realize( widget );
    gtk_widget_ensure_style( widget );

    // Store widget's default flags
    gWidgetDefaultFlags[ reinterpret_cast<long>(widget) ] = GTK_WIDGET_FLAGS( widget );
}

static void NWEnsureGTKButton( SalX11Screen nScreen )
{
    if ( !gWidgetData[nScreen].gBtnWidget )
    {
        gWidgetData[nScreen].gBtnWidget = gtk_button_new_with_label( "" );
        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gBtnWidget, nScreen );
    }
}

static void NWEnsureGTKRadio( SalX11Screen nScreen )
{
    if ( !gWidgetData[nScreen].gRadioWidget || !gWidgetData[nScreen].gRadioWidgetSibling )
    {
        gWidgetData[nScreen].gRadioWidget = gtk_radio_button_new( nullptr );
        gWidgetData[nScreen].gRadioWidgetSibling = gtk_radio_button_new_from_widget( GTK_RADIO_BUTTON(gWidgetData[nScreen].gRadioWidget) );
        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gRadioWidget, nScreen );
        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gRadioWidgetSibling, nScreen );
    }
}

static void NWEnsureGTKCheck( SalX11Screen nScreen )
{
    if ( !gWidgetData[nScreen].gCheckWidget )
    {
        gWidgetData[nScreen].gCheckWidget = gtk_check_button_new();
        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gCheckWidget, nScreen );
    }
}

static void NWEnsureGTKScrollbars( SalX11Screen nScreen )
{
    if ( !gWidgetData[nScreen].gScrollHorizWidget )
    {
        gWidgetData[nScreen].gScrollHorizWidget = gtk_hscrollbar_new( nullptr );
        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gScrollHorizWidget, nScreen );
    }

    if ( !gWidgetData[nScreen].gScrollVertWidget )
    {
        gWidgetData[nScreen].gScrollVertWidget = gtk_vscrollbar_new( nullptr );
        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gScrollVertWidget, nScreen );
    }
}

static void NWEnsureGTKArrow( SalX11Screen nScreen )
{
    if ( !gWidgetData[nScreen].gArrowWidget || !gWidgetData[nScreen].gDropdownWidget )
    {
        gWidgetData[nScreen].gDropdownWidget = gtk_toggle_button_new();
        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gDropdownWidget, nScreen );
        gWidgetData[nScreen].gArrowWidget = gtk_arrow_new( GTK_ARROW_DOWN, GTK_SHADOW_OUT );
        gtk_container_add( GTK_CONTAINER(gWidgetData[nScreen].gDropdownWidget), gWidgetData[nScreen].gArrowWidget );
        gtk_widget_set_rc_style( gWidgetData[nScreen].gArrowWidget );
        gtk_widget_realize( gWidgetData[nScreen].gArrowWidget );
    }
}

static void NWEnsureGTKEditBox( SalX11Screen nScreen )
{
    if ( !gWidgetData[nScreen].gEditBoxWidget )
    {
        gWidgetData[nScreen].gEditBoxWidget = gtk_entry_new();
        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gEditBoxWidget, nScreen );
    }
}

static void NWEnsureGTKSpinButton( SalX11Screen nScreen )
{
    if ( !gWidgetData[nScreen].gSpinButtonWidget )
    {
        GtkAdjustment *adj = GTK_ADJUSTMENT( gtk_adjustment_new(0, 0, 1, 1, 1, 0) );
        gWidgetData[nScreen].gSpinButtonWidget = gtk_spin_button_new( adj, 1, 2 );

        //Setting non-editable means it doesn't blink, so there's no timeouts
        //running around to nobble us
        gtk_editable_set_editable(GTK_EDITABLE(gWidgetData[nScreen].gSpinButtonWidget), false);

        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gSpinButtonWidget, nScreen );
    }
}

static void NWEnsureGTKNotebook( SalX11Screen nScreen )
{
    if ( !gWidgetData[nScreen].gNotebookWidget )
    {
        gWidgetData[nScreen].gNotebookWidget = gtk_notebook_new();
        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gNotebookWidget, nScreen );
    }
}

static void NWEnsureGTKOptionMenu( SalX11Screen nScreen )
{
    if ( !gWidgetData[nScreen].gOptionMenuWidget )
    {
        gWidgetData[nScreen].gOptionMenuWidget = gtk_option_menu_new();
        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gOptionMenuWidget, nScreen );
    }
}

static void NWEnsureGTKCombo( SalX11Screen nScreen )
{
    if ( !gWidgetData[nScreen].gComboWidget )
    {
        gWidgetData[nScreen].gComboWidget = gtk_combo_new();

        // #i59129# Setting non-editable means it doesn't blink, so
        // there are no timeouts running around to nobble us
        gtk_editable_set_editable(GTK_EDITABLE(GTK_COMBO(gWidgetData[nScreen].gComboWidget)->entry), false);

        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gComboWidget, nScreen );
        // Must realize the ComboBox's children, since GTK
        // does not do this for us in GtkCombo::gtk_widget_realize()
        gtk_widget_realize( GTK_COMBO(gWidgetData[nScreen].gComboWidget)->button );
        gtk_widget_realize( GTK_COMBO(gWidgetData[nScreen].gComboWidget)->entry );
    }
}

static void NWEnsureGTKScrolledWindow( SalX11Screen nScreen )
{
    if ( !gWidgetData[nScreen].gScrolledWindowWidget )
    {
        GtkAdjustment *hadj = GTK_ADJUSTMENT( gtk_adjustment_new(0, 0, 0, 0, 0, 0) );
        GtkAdjustment *vadj = GTK_ADJUSTMENT( gtk_adjustment_new(0, 0, 0, 0, 0, 0) );

        gWidgetData[nScreen].gScrolledWindowWidget = gtk_scrolled_window_new( hadj, vadj );
        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gScrolledWindowWidget, nScreen );
    }
}

static void NWEnsureGTKToolbar( SalX11Screen nScreen )
{
    if( !gWidgetData[nScreen].gToolbarWidget )
    {
        gWidgetData[nScreen].gToolbarWidget = gtk_toolbar_new();
        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gToolbarWidget, nScreen );
        gWidgetData[nScreen].gToolbarButtonWidget = GTK_WIDGET(gtk_toggle_button_new());
        gWidgetData[nScreen].gSeparator = GTK_WIDGET(gtk_separator_tool_item_new());
        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gSeparator, nScreen );

        GtkReliefStyle aRelief = GTK_RELIEF_NORMAL;
        gtk_widget_ensure_style( gWidgetData[nScreen].gToolbarWidget );
        gtk_widget_style_get( gWidgetData[nScreen].gToolbarWidget,
                              "button_relief", &aRelief,
                              nullptr);

        gtk_button_set_relief( GTK_BUTTON(gWidgetData[nScreen].gToolbarButtonWidget), aRelief );
        GTK_WIDGET_UNSET_FLAGS( gWidgetData[nScreen].gToolbarButtonWidget, GTK_CAN_FOCUS );
        GTK_WIDGET_UNSET_FLAGS( gWidgetData[nScreen].gToolbarButtonWidget, GTK_CAN_DEFAULT );
        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gToolbarButtonWidget, nScreen );

    }
    if( ! gWidgetData[nScreen].gHandleBoxWidget )
    {
        gWidgetData[nScreen].gHandleBoxWidget = gtk_handle_box_new();
        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gHandleBoxWidget, nScreen );
    }
}

static void NWEnsureGTKMenubar( SalX11Screen nScreen )
{
    if( !gWidgetData[nScreen].gMenubarWidget )
    {
        gWidgetData[nScreen].gMenubarWidget = gtk_menu_bar_new();
        gWidgetData[nScreen].gMenuItemMenubarWidget = gtk_menu_item_new_with_label( "b" );
        gtk_menu_shell_append( GTK_MENU_SHELL( gWidgetData[nScreen].gMenubarWidget ), gWidgetData[nScreen].gMenuItemMenubarWidget );
        gtk_widget_show( gWidgetData[nScreen].gMenuItemMenubarWidget );
        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gMenubarWidget, nScreen );
        gtk_widget_show( gWidgetData[nScreen].gMenubarWidget );

        // do what NWAddWidgetToCacheWindow does except adding to def container
        gtk_widget_realize( gWidgetData[nScreen].gMenuItemMenubarWidget );
        gtk_widget_ensure_style( gWidgetData[nScreen].gMenuItemMenubarWidget );

        gWidgetDefaultFlags[ reinterpret_cast<long>(gWidgetData[nScreen].gMenuItemMenubarWidget) ] = GTK_WIDGET_FLAGS( gWidgetData[nScreen].gMenuItemMenubarWidget );
    }
}

static void NWEnsureGTKMenu( SalX11Screen nScreen )
{
    if( !gWidgetData[nScreen].gMenuWidget )
    {
        gWidgetData[nScreen].gMenuWidget                  = gtk_menu_new();
        gWidgetData[nScreen].gMenuItemMenuWidget          = gtk_menu_item_new_with_label( "b" );
        gWidgetData[nScreen].gMenuItemCheckMenuWidget     = gtk_check_menu_item_new_with_label( "b" );
        gWidgetData[nScreen].gMenuItemRadioMenuWidget     = gtk_radio_menu_item_new_with_label( nullptr, "b" );
        gWidgetData[nScreen].gMenuItemSeparatorMenuWidget = gtk_separator_menu_item_new();
        gWidgetData[nScreen].gImageMenuItem               = gtk_image_menu_item_new();

        g_object_ref_sink (gWidgetData[nScreen].gMenuWidget);

        gtk_menu_shell_append( GTK_MENU_SHELL( gWidgetData[nScreen].gMenuWidget ), gWidgetData[nScreen].gMenuItemMenuWidget );
        gtk_menu_shell_append( GTK_MENU_SHELL( gWidgetData[nScreen].gMenuWidget ), gWidgetData[nScreen].gMenuItemCheckMenuWidget );
        gtk_menu_shell_append( GTK_MENU_SHELL( gWidgetData[nScreen].gMenuWidget ), gWidgetData[nScreen].gMenuItemRadioMenuWidget );
        gtk_menu_shell_append( GTK_MENU_SHELL( gWidgetData[nScreen].gMenuWidget ), gWidgetData[nScreen].gMenuItemSeparatorMenuWidget );
        gtk_menu_shell_append( GTK_MENU_SHELL( gWidgetData[nScreen].gMenuWidget ), gWidgetData[nScreen].gImageMenuItem );

        // do what NWAddWidgetToCacheWindow does except adding to def container
        gtk_widget_realize( gWidgetData[nScreen].gMenuWidget );
        gtk_widget_ensure_style( gWidgetData[nScreen].gMenuWidget );

        gtk_widget_realize( gWidgetData[nScreen].gMenuItemMenuWidget );
        gtk_widget_ensure_style( gWidgetData[nScreen].gMenuItemMenuWidget );

        gtk_widget_realize( gWidgetData[nScreen].gMenuItemCheckMenuWidget );
        gtk_widget_ensure_style( gWidgetData[nScreen].gMenuItemCheckMenuWidget );

        gtk_widget_realize( gWidgetData[nScreen].gMenuItemRadioMenuWidget );
        gtk_widget_ensure_style( gWidgetData[nScreen].gMenuItemRadioMenuWidget );

        gtk_widget_realize( gWidgetData[nScreen].gMenuItemSeparatorMenuWidget );
        gtk_widget_ensure_style( gWidgetData[nScreen].gMenuItemSeparatorMenuWidget );

        gtk_widget_realize( gWidgetData[nScreen].gImageMenuItem );
        gtk_widget_ensure_style( gWidgetData[nScreen].gImageMenuItem );

        gWidgetDefaultFlags[ reinterpret_cast<long>(gWidgetData[nScreen].gMenuWidget) ] = GTK_WIDGET_FLAGS( gWidgetData[nScreen].gMenuWidget );
        gWidgetDefaultFlags[ reinterpret_cast<long>(gWidgetData[nScreen].gMenuItemMenuWidget) ] = GTK_WIDGET_FLAGS( gWidgetData[nScreen].gMenuItemMenuWidget );
        gWidgetDefaultFlags[ reinterpret_cast<long>(gWidgetData[nScreen].gMenuItemCheckMenuWidget) ] = GTK_WIDGET_FLAGS( gWidgetData[nScreen].gMenuItemCheckMenuWidget );
        gWidgetDefaultFlags[ reinterpret_cast<long>(gWidgetData[nScreen].gMenuItemRadioMenuWidget) ] = GTK_WIDGET_FLAGS( gWidgetData[nScreen].gMenuItemRadioMenuWidget );
        gWidgetDefaultFlags[ reinterpret_cast<long>(gWidgetData[nScreen].gMenuItemSeparatorMenuWidget) ] = GTK_WIDGET_FLAGS( gWidgetData[nScreen].gMenuItemSeparatorMenuWidget );
        gWidgetDefaultFlags[ reinterpret_cast<long>(gWidgetData[nScreen].gImageMenuItem) ] = GTK_WIDGET_FLAGS( gWidgetData[nScreen].gImageMenuItem );
    }
}

static void NWEnsureGTKTooltip( SalX11Screen nScreen )
{
    if( !gWidgetData[nScreen].gTooltipPopup )
    {
        gWidgetData[nScreen].gTooltipPopup = gtk_window_new (GTK_WINDOW_POPUP);
        GdkScreen* pScreen = gdk_display_get_screen( gdk_display_get_default(),
                                                     nScreen.getXScreen() );
        if( pScreen )
            gtk_window_set_screen( GTK_WINDOW(gWidgetData[nScreen].gTooltipPopup), pScreen );
        gtk_widget_set_name( gWidgetData[nScreen].gTooltipPopup, "gtk-tooltips");
        gtk_widget_realize( gWidgetData[nScreen].gTooltipPopup );
        gtk_widget_ensure_style( gWidgetData[nScreen].gTooltipPopup );
    }
}

static void NWEnsureGTKDialog( SalX11Screen nScreen )
{
    if( !gWidgetData[nScreen].gDialog )
    {
        gWidgetData[nScreen].gDialog = gtk_dialog_new();
        GdkScreen* pScreen = gdk_display_get_screen( gdk_display_get_default(),
                                                     nScreen.getXScreen() );
        if( pScreen )
            gtk_window_set_screen( GTK_WINDOW(gWidgetData[nScreen].gDialog), pScreen );
        gtk_widget_realize(gWidgetData[nScreen].gDialog);
        gtk_widget_ensure_style(gWidgetData[nScreen].gDialog);
    }
}

static void NWEnsureGTKFrame( SalX11Screen nScreen )
{
    if( !gWidgetData[nScreen].gFrame )
    {
        gWidgetData[nScreen].gFrame = gtk_frame_new(nullptr);
        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gFrame, nScreen );
    }
}

static void NWEnsureGTKProgressBar( SalX11Screen nScreen )
{
    if( !gWidgetData[nScreen].gProgressBar )
    {
        gWidgetData[nScreen].gProgressBar = gtk_progress_bar_new ();
        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gProgressBar, nScreen );
    }
}

static void NWEnsureGTKTreeView( SalX11Screen nScreen )
{
    if( !gWidgetData[nScreen].gTreeView )
    {
        gWidgetData[nScreen].gTreeView = gtk_tree_view_new ();

        // Columns will be used for tree header rendering
        GtkCellRenderer* renderer=gtk_cell_renderer_text_new();
        GtkTreeViewColumn* column=gtk_tree_view_column_new_with_attributes("",renderer,"text",0,NULL);
        gtk_tree_view_column_set_widget(column,gtk_label_new(""));
        gtk_tree_view_append_column(GTK_TREE_VIEW(gWidgetData[nScreen].gTreeView), column);

        // Add one more column so that some engines like clearlooks did render separators between columns
        column=gtk_tree_view_column_new_with_attributes("",renderer,"text",0,NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(gWidgetData[nScreen].gTreeView), column);

        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gTreeView, nScreen );
    }
}

static void NWEnsureGTKSlider( SalX11Screen nScreen )
{
    if( !gWidgetData[nScreen].gHScale )
    {
        gWidgetData[nScreen].gHScale = gtk_hscale_new_with_range(0, 10, 1);
        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gHScale, nScreen );
    }
    if( !gWidgetData[nScreen].gVScale )
    {
        gWidgetData[nScreen].gVScale = gtk_vscale_new_with_range(0, 10, 1);
        NWAddWidgetToCacheWindow( gWidgetData[nScreen].gVScale, nScreen );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
