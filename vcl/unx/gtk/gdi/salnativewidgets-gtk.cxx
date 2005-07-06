/*************************************************************************
 *
 *  $RCSfile: salnativewidgets-gtk.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-06 09:21:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *
 ************************************************************************/

#include <plugins/gtk/gtkframe.hxx>
#include <plugins/gtk/gtkdata.hxx>
#include <plugins/gtk/gtkinst.hxx>
#include <plugins/gtk/gtkgdi.hxx>

#include <pspgraphics.h>

#include <cstdio>
#include <cmath>
#include <vector>
#include <algorithm>
#include <hash_map>

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif

#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif


#ifndef _OSL_CONDITN_HXX_
#include <osl/mutex.hxx>
#endif

// initialize statics
BOOL GtkSalGraphics::bThemeChanged = TRUE;
BOOL GtkSalGraphics::bNeedPixmapPaint = FALSE;

GtkSalGraphics::~GtkSalGraphics()
{
}


using namespace rtl;

/*************************************
 * Cached native widget objects
 *************************************/
static GtkWidget *  gCacheWindow = NULL;
static GtkWidget *  gDumbContainer = NULL;

static GtkWidget *  gBtnWidget              = NULL;
static GtkWidget *  gRadioWidget            = NULL;
static GtkWidget *  gRadioWidgetSibling     = NULL;
static GtkWidget *  gCheckWidget            = NULL;
static GtkWidget *  gScrollHorizWidget      = NULL;
static GtkWidget *  gScrollVertWidget       = NULL;
static GtkWidget *  gArrowWidget            = NULL;
static GtkWidget *  gDropdownWidget         = NULL;
static GtkWidget *  gEditBoxWidget          = NULL;
static GtkWidget *  gSpinButtonWidget       = NULL;
static GtkWidget *  gNotebookWidget         = NULL;
static GtkWidget *  gOptionMenuWidget       = NULL;
static GtkWidget *  gComboWidget            = NULL;
static GtkWidget *  gScrolledWindowWidget   = NULL;
static GtkWidget *  gToolbarWidget          = NULL;
static GtkWidget *  gToolbarButtonWidget    = NULL;
static GtkWidget *  gToolbarToggleWidget    = NULL;
static GtkWidget *  gHandleBoxWidget        = NULL;
static GtkWidget *  gMenubarWidget          = NULL;
static GtkWidget *  gMenuItemMenubarWidget  = NULL;
static GtkWidget *  gMenuWidget             = NULL;
static GtkWidget *  gMenuItemMenuWidget     = NULL;
static GtkWidget *  gTooltipPopup           = NULL;

osl::Mutex  * pWidgetMutex;

class NWPixmapCacheList;
static NWPixmapCacheList* gNWPixmapCacheList = NULL;

// Keep a hash table of Widgets->default flags so that we can
// easily and quickly reset each to a default state before using
// them
static std::hash_map<long, guint>   gWidgetDefaultFlags;

static const GtkBorder aDefDefBorder        = { 1, 1, 1, 1 };
static const GtkBorder aDefDefOutsideBorder = { 0, 0, 0, 0 };

// Some GTK defaults
#define MIN_ARROW_SIZE                  11
#define BTN_CHILD_SPACING               1
#define MIN_SPIN_ARROW_WIDTH                6


static void NWEnsureGTKRadio            ( void );
static void NWEnsureGTKButton           ( void );
static void NWEnsureGTKCheck            ( void );
static void NWEnsureGTKScrollbars       ( void );
static void NWEnsureGTKArrow            ( void );
static void NWEnsureGTKEditBox          ( void );
static void NWEnsureGTKSpinButton       ( void );
static void NWEnsureGTKNotebook         ( void );
static void NWEnsureGTKOptionMenu       ( void );
static void NWEnsureGTKCombo            ( void );
static void NWEnsureGTKScrolledWindow   ( void );
static void NWEnsureGTKToolbar          ( void );
static void NWEnsureGTKMenubar          ( void );
static void NWEnsureGTKMenu             ( void );
static void NWEnsureGTKTooltip          ( void );

static void NWConvertVCLStateToGTKState( ControlState nVCLState, GtkStateType* nGTKState, GtkShadowType* nGTKShadow );
static void NWAddWidgetToCacheWindow( GtkWidget* widget );
static void NWSetWidgetState( GtkWidget* widget, ControlState nState, GtkStateType nGtkState );

static void NWCalcArrowRect( const Rectangle& rButton, Rectangle& rArrow );

/*
 * Individual helper functions
 *
 */

//---
static Rectangle NWGetButtonArea( ControlType nType, ControlPart nPart, Rectangle aAreaRect, ControlState nState,
                                const ImplControlValue& aValue, SalControlHandle& rControlHandle, OUString aCaption );

//---
static Rectangle NWGetEditBoxPixmapRect( ControlType nType, ControlPart nPart, Rectangle aAreaRect, ControlState nState,
                            const ImplControlValue& aValue, SalControlHandle& rControlHandle, OUString aCaption );

static void NWPaintOneEditBox( GdkDrawable * gdkDrawable, GdkRectangle *gdkRect,
                               ControlType nType, ControlPart nPart, Rectangle aEditBoxRect,
                               ControlState nState, const ImplControlValue& aValue,
                               SalControlHandle& rControlHandle, OUString aCaption );

//---
static Rectangle NWGetSpinButtonRect( ControlType nType, ControlPart nPart, Rectangle aAreaRect, ControlState nState,
                            const ImplControlValue& aValue, SalControlHandle& rControlHandle, OUString aCaption );

static void NWPaintOneSpinButton( GdkPixmap * pixmap, ControlType nType, ControlPart nPart, Rectangle aAreaRect,
                            ControlState nState, const ImplControlValue& aValue, SalControlHandle& rControlHandle,
                            OUString aCaption );
//---
static Rectangle NWGetComboBoxButtonRect( ControlType nType, ControlPart nPart, Rectangle aAreaRect, ControlState nState,
                            const ImplControlValue& aValue, SalControlHandle& rControlHandle, OUString aCaption );

//---
static Rectangle NWGetListBoxButtonRect( ControlType nType, ControlPart nPart, Rectangle aAreaRect, ControlState nState,
                            const ImplControlValue& aValue, SalControlHandle& rControlHandle, OUString aCaption );

static Rectangle NWGetListBoxIndicatorRect( ControlType nType, ControlPart nPart, Rectangle aAreaRect, ControlState nState,
                            const ImplControlValue& aValue, SalControlHandle& rControlHandle, OUString aCaption );

static Rectangle NWGetToolbarRect( ControlType nType,
                                   ControlPart nPart,
                                   Rectangle aAreaRect,
                                   ControlState nState,
                                   const ImplControlValue& aValue,
                                   SalControlHandle& rControlHandle,
                                   const OUString& aCaption );
//---

static Rectangle NWGetScrollButtonRect( ControlPart nPart, Rectangle aAreaRect );
//---

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
    Rectangle   m_pixmapRect;
    GdkPixmap*  m_pixmap;

    NWPixmapCacheData()
        { m_pixmap = NULL; };
    ~NWPixmapCacheData()
        { SetPixmap( NULL ); };
    void SetPixmap( GdkPixmap* pPixmap );
};

class NWPixmapCache
{
    int m_size;
    int m_idx;
    NWPixmapCacheData* pData;
public:
    NWPixmapCache();
    ~NWPixmapCache();

    void SetSize( int n)
        { delete [] pData; m_idx = 0; m_size = n; pData = new NWPixmapCacheData[m_size]; }
    int GetSize() { return m_size; }

    BOOL Find( ControlType aType, ControlState aState, const Rectangle& r_pixmapRect, GdkPixmap** pPixmap );
    void Fill( ControlType aType, ControlState aState, const Rectangle& r_pixmapRect, GdkPixmap* pPixmap );

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

void NWPixmapCacheData::SetPixmap( GdkPixmap* pPixmap )
{
    if( m_pixmap )
        g_object_unref( m_pixmap );

    m_pixmap = pPixmap;

    if( m_pixmap )
        g_object_ref( m_pixmap );
}


NWPixmapCache::NWPixmapCache()
{
    m_idx = 0;
    m_size = 0;
    pData = NULL;
    if( gNWPixmapCacheList )
        gNWPixmapCacheList->AddCache(this);
}
NWPixmapCache::~NWPixmapCache()
{
    if( gNWPixmapCacheList )
        gNWPixmapCacheList->RemoveCache(this);
    delete[] pData;
}
void NWPixmapCache::ThemeChanged()
{
    // throw away cached pixmaps
    int i;
    for(i=0; i<m_size; i++)
        pData[i].SetPixmap( NULL );
}

BOOL  NWPixmapCache::Find( ControlType aType, ControlState aState, const Rectangle& r_pixmapRect, GdkPixmap** pPixmap )
{
    aState &= ~CTRL_CACHING_ALLOWED; // mask clipping flag
    int i;
    for(i=0; i<m_size; i++)
    {
        if( pData[i].m_nType == aType &&
            pData[i].m_nState == aState &&
            pData[i].m_pixmapRect.GetWidth() == r_pixmapRect.GetWidth() &&
            pData[i].m_pixmapRect.GetHeight() == r_pixmapRect.GetHeight() &&
            pData[i].m_pixmap != NULL )
        {
            *pPixmap = pData[i].m_pixmap;
            return TRUE;
        }
    }
    return FALSE;
}

void NWPixmapCache::Fill( ControlType aType, ControlState aState, const Rectangle& r_pixmapRect, GdkPixmap* pPixmap )
{
    if( !(aState & CTRL_CACHING_ALLOWED) )
        return;

    aState &= ~CTRL_CACHING_ALLOWED; // mask clipping flag
    m_idx = (m_idx+1) % m_size; // just wrap
    pData[m_idx].m_nType = aType;
    pData[m_idx].m_nState = aState;
    pData[m_idx].m_pixmapRect = r_pixmapRect;
    pData[m_idx].SetPixmap( pPixmap );
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
        p++;
    }
}


/*********************************************************
 * Make border manipulation easier
 *********************************************************/
inline void NW_gtk_border_set_from_border( GtkBorder& aDst, const GtkBorder * pSrc )
{
    aDst.left       = pSrc->left;
    aDst.top        = pSrc->top;
    aDst.right  = pSrc->right;
    aDst.bottom = pSrc->bottom;
}


/*********************************************************
 * Initialize GTK and local stuff
 *********************************************************/
void GtkData::initNWF( void )
{
    pWidgetMutex = new osl::Mutex;
    gNWPixmapCacheList = new NWPixmapCacheList;

    ImplSVData* pSVData = ImplGetSVData();

    // draw no border for popup menus (NWF draws its own)
    pSVData->maNWFData.mbFlatMenu = true;

    // draw separate buttons for toolbox dropdown items
    pSVData->maNWFData.mbToolboxDropDownSeparate = true;

    // small extra border around menu items
    pSVData->maNWFData.mnMenuFormatExtraBorder = 1;

    // draw toolbars in separate lines
    pSVData->maNWFData.mbDockingAreaSeparateTB = true;

    if( SalGetDesktopEnvironment().equalsAscii( "KDE" ) )
        // KDE 3.3 invented a bug in the qt<->gtk theme engine
        // that makes direct rendering impossible: they totally
        // ignore the clip rectangle passed to the paint methods
        GtkSalGraphics::bNeedPixmapPaint = true;

    #if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "GtkPlugin: using %s NWF\n",
             GtkSalGraphics::bNeedPixmapPaint ? "offscreen" : "direct" );
    #endif
}


/*********************************************************
 * Release GTK and local stuff
 *********************************************************/
void GtkData::deInitNWF( void )
{
    // free up global widgets
    // gtk_widget_destroy will in turn destroy the child hierarchy
    // so only destroy disjunct hierachies
    if( gCacheWindow )
        gtk_widget_destroy( gCacheWindow );
    if( gMenuWidget )
        gtk_widget_destroy( gMenuWidget );
    if( gTooltipPopup )
        gtk_widget_destroy( gTooltipPopup );

    delete pWidgetMutex;
    delete gNWPixmapCacheList;
    gNWPixmapCacheList = NULL;
}


/**********************************************************
 * track clip region
 **********************************************************/
void GtkSalGraphics::ResetClipRegion()
{
    m_aClipRegion.SetNull();
    X11SalGraphics::ResetClipRegion();
}

void GtkSalGraphics::BeginSetClipRegion( ULONG nCount )
{
    m_aClipRegion.SetNull();
    X11SalGraphics::BeginSetClipRegion( nCount );
}

BOOL GtkSalGraphics::unionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    Rectangle aRect( Point( nX, nY ), Size( nWidth, nHeight ) );
    m_aClipRegion.Union( aRect );
    return X11SalGraphics::unionClipRegion( nX, nY, nWidth, nHeight );
}

void GtkSalGraphics::EndSetClipRegion()
{
    if( m_aClipRegion.IsEmpty() )
        m_aClipRegion.SetNull();
    X11SalGraphics::EndSetClipRegion();
}

/*
 * IsNativeControlSupported()
 *
 *  Returns TRUE if the platform supports native
 *  drawing of the control defined by nPart
 */
BOOL GtkSalGraphics::IsNativeControlSupported( ControlType nType, ControlPart nPart )
{
    if (
        ((nType==CTRL_PUSHBUTTON)  && (nPart==PART_ENTIRE_CONTROL))     ||
         ((nType==CTRL_RADIOBUTTON) && (nPart==PART_ENTIRE_CONTROL))        ||
        ((nType==CTRL_CHECKBOX)    && (nPart==PART_ENTIRE_CONTROL))     ||
        ((nType==CTRL_SCROLLBAR) &&
                (  (nPart==PART_DRAW_BACKGROUND_HORZ)
                || (nPart==PART_DRAW_BACKGROUND_VERT)
                || (nPart==PART_ENTIRE_CONTROL)
                || (nPart==HAS_THREE_BUTTONS) )                 )   ||
        ((nType==CTRL_EDITBOX) &&
                (  (nPart==PART_ENTIRE_CONTROL)
                || (nPart==HAS_BACKGROUND_TEXTURE) )            )   ||
        ((nType==CTRL_MULTILINE_EDITBOX) &&
                (  (nPart==PART_ENTIRE_CONTROL)
                || (nPart==HAS_BACKGROUND_TEXTURE) )            )   ||
        ((nType==CTRL_SPINBOX) &&
                (  (nPart==PART_ENTIRE_CONTROL)
                || (nPart==PART_ALL_BUTTONS)
                || (nPart==HAS_BACKGROUND_TEXTURE) )            )   ||
        ((nType==CTRL_SPINBUTTONS) &&
                (  (nPart==PART_ENTIRE_CONTROL)
                || (nPart==PART_ALL_BUTTONS)    )               )   ||
        ((nType==CTRL_COMBOBOX) &&
                (  (nPart==PART_ENTIRE_CONTROL)
                || (nPart==HAS_BACKGROUND_TEXTURE)  )           )   ||
        (((nType==CTRL_TAB_ITEM) || (nType==CTRL_TAB_PANE) ||
          (nType==CTRL_TAB_BODY) || (nType==CTRL_FIXEDBORDER)) &&
                (  (nPart==PART_ENTIRE_CONTROL)
                || (nPart==PART_TABS_DRAW_RTL) )                )   ||
        ((nType==CTRL_LISTBOX) &&
                (  (nPart==PART_ENTIRE_CONTROL)
                || (nPart==PART_WINDOW)
                || (nPart==HAS_BACKGROUND_TEXTURE) )            )   ||
        ((nType == CTRL_TOOLBAR) &&
                 (  (nPart==PART_ENTIRE_CONTROL)
                ||  (nPart==PART_DRAW_BACKGROUND_HORZ)
                ||  (nPart==PART_DRAW_BACKGROUND_VERT)
                ||  (nPart==PART_THUMB_HORZ)
                ||  (nPart==PART_THUMB_VERT)
                ||  (nPart==PART_BUTTON)
                )
                                                                )   ||
        ((nType == CTRL_MENUBAR) &&
                (   (nPart==PART_ENTIRE_CONTROL) )              )   ||
        ((nType == CTRL_TOOLTIP) &&
                (   (nPart==PART_ENTIRE_CONTROL) )              )   ||
        ((nType == CTRL_MENU_POPUP) &&
                (   (nPart==PART_ENTIRE_CONTROL) )
                ||  (nPart==PART_MENU_ITEM)
                )
        )
        return( TRUE );

    return( FALSE );
}


/*
 * HitTestNativeControl()
 *
 *  bIsInside is set to TRUE if aPos is contained within the
 *  given part of the control, whose bounding region is
 *  given by rControlRegion (in VCL frame coordinates).
 *
 *  returns whether bIsInside was really set.
 */
BOOL GtkSalGraphics::hitTestNativeControl( ControlType      nType,
                                ControlPart     nPart,
                                const Region&       rControlRegion,
                                const Point&        aPos,
                                SalControlHandle&   rControlHandle,
                                BOOL&           rIsInside )
{
    if ( ( nType == CTRL_SCROLLBAR ) &&
         ( ( nPart == PART_BUTTON_UP ) ||
           ( nPart == PART_BUTTON_DOWN ) ||
           ( nPart == PART_BUTTON_LEFT ) ||
           ( nPart == PART_BUTTON_RIGHT ) ) )
    {
        NWEnsureGTKScrollbars();

        // Grab some button style attributes
        gboolean has_forward;
        gboolean has_forward2;
        gboolean has_backward;
        gboolean has_backward2;

        gtk_widget_style_get( gScrollHorizWidget, "has-forward-stepper", &has_forward,
                                        "has-secondary-forward-stepper", &has_forward2,
                                        "has-backward-stepper", &has_backward,
                                           "has-secondary-backward-stepper", &has_backward2, NULL );
        Rectangle aForward;
        Rectangle aBackward;

        rIsInside = FALSE;

        ControlPart nCounterPart = 0;
        if ( nPart == PART_BUTTON_UP )
            nCounterPart = PART_BUTTON_DOWN;
        else if ( nPart == PART_BUTTON_DOWN )
            nCounterPart = PART_BUTTON_UP;
        else if ( nPart == PART_BUTTON_LEFT )
            nCounterPart = PART_BUTTON_RIGHT;
        else if ( nPart == PART_BUTTON_RIGHT )
            nCounterPart = PART_BUTTON_LEFT;

        aBackward = NWGetScrollButtonRect( nPart, rControlRegion.GetBoundRect() );
        aForward = NWGetScrollButtonRect( nCounterPart, rControlRegion.GetBoundRect() );

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
        return ( TRUE );
    }

    if( IsNativeControlSupported(nType, nPart) )
    {
        rIsInside = rControlRegion.IsInside( aPos );
        return( TRUE );
    }
    else
    {
        return( FALSE );
    }
}


/*
 * DrawNativeControl()
 *
 *  Draws the requested control described by nPart/nState.
 *
 *  rControlRegion: The bounding region of the complete control in VCL frame coordinates.
 *  aValue:         An optional value (tristate/numerical/string)
 *  rControlHandle: Carries platform dependent data and is maintained by the SalFrame implementation.
 *  aCaption:   A caption or title string (like button text etc)
 */
BOOL GtkSalGraphics::drawNativeControl( ControlType nType,
                            ControlPart nPart,
                            const Region& rControlRegion,
                            ControlState nState,
                            const ImplControlValue& aValue,
                            SalControlHandle& rControlHandle,
                            OUString aCaption )
{
    BOOL            returnVal = FALSE;
    // get a GC with current clipping region set
    SelectFont();

    // theme changed ?
    if( GtkSalGraphics::bThemeChanged )
    {
        // invalidate caches
        gNWPixmapCacheList->ThemeChanged();
        GtkSalGraphics::bThemeChanged = FALSE;
    }

    if ( pWidgetMutex->acquire() )
    {
        Rectangle aCtrlRect = rControlRegion.GetBoundRect();
        Region aClipRegion( m_aClipRegion );
        if( aClipRegion.IsNull() )
            aClipRegion = aCtrlRect;

        clipList aClip;
        GdkDrawable* gdkDrawable = GDK_DRAWABLE( GetGdkWindow() );
        GdkPixmap* pixmap = NULL;
        Rectangle aPixmapRect;
        if( bNeedPixmapPaint &&
            nType != CTRL_SCROLLBAR &&
            nType != CTRL_SPINBOX &&
            nType != CTRL_TAB_ITEM &&
            nType != CTRL_TAB_PANE
            )
        {
            // make pixmap a little larger since some themes draw decoration
            // outside the rectangle, see e.g. checkbox
            aPixmapRect = Rectangle( Point( aCtrlRect.Left()-1, aCtrlRect.Top()-1 ),
                                     Size( aCtrlRect.GetWidth()+2, aCtrlRect.GetHeight()+2) );
            pixmap = NWGetPixmapFromScreen( aPixmapRect );
            if( ! pixmap )
            {
                pWidgetMutex->release();
                return FALSE;
            }
            gdkDrawable = GDK_DRAWABLE( pixmap );
            aCtrlRect = Rectangle( Point(1,1), aCtrlRect.GetSize() );
            aClip.push_back( aCtrlRect );
        }
        else
        {
            RegionHandle aHdl = aClipRegion.BeginEnumRects();
            Rectangle aPaintRect;
            while( aClipRegion.GetNextEnumRect( aHdl, aPaintRect ) )
            {
                aPaintRect = aCtrlRect.GetIntersection( aPaintRect );
                if( aPaintRect.IsEmpty() )
                    continue;
                aClip.push_back( aPaintRect );
            }
            aClipRegion.EndEnumRects( aHdl );
        }

        if ( (nType==CTRL_PUSHBUTTON) && (nPart==PART_ENTIRE_CONTROL) )
        {
            returnVal = NWPaintGTKButton( gdkDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rControlHandle, aCaption );
        }
        else if ( (nType==CTRL_RADIOBUTTON) && (nPart==PART_ENTIRE_CONTROL) )
        {
            returnVal = NWPaintGTKRadio( gdkDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rControlHandle, aCaption );
        }
        else if ( (nType==CTRL_CHECKBOX) && (nPart==PART_ENTIRE_CONTROL) )
        {
            returnVal = NWPaintGTKCheck( gdkDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rControlHandle, aCaption );
        }
        else if ( (nType==CTRL_SCROLLBAR) && ((nPart==PART_DRAW_BACKGROUND_HORZ) || (nPart==PART_DRAW_BACKGROUND_VERT)) )
        {
            returnVal = NWPaintGTKScrollbar( nType, nPart, aCtrlRect, aClip, nState, aValue, rControlHandle, aCaption );
        }
        else if ( ((nType==CTRL_EDITBOX) && ((nPart==PART_ENTIRE_CONTROL) || (nPart==HAS_BACKGROUND_TEXTURE)) )
            || ((nType==CTRL_SPINBOX) && (nPart==HAS_BACKGROUND_TEXTURE))
        || ((nType==CTRL_COMBOBOX) && (nPart==HAS_BACKGROUND_TEXTURE))
        || ((nType==CTRL_LISTBOX) && (nPart==HAS_BACKGROUND_TEXTURE)) )
        {
            returnVal = NWPaintGTKEditBox( gdkDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rControlHandle, aCaption );
        }
        else if ( ((nType==CTRL_MULTILINE_EDITBOX) && ((nPart==PART_ENTIRE_CONTROL) || (nPart==HAS_BACKGROUND_TEXTURE)) ) )
        {
            returnVal = NWPaintGTKEditBox( gdkDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rControlHandle, aCaption );
        }
        else if ( ((nType==CTRL_SPINBOX) || (nType==CTRL_SPINBUTTONS))
            && ((nPart==PART_ENTIRE_CONTROL) || (nPart==PART_ALL_BUTTONS)) )
        {
            returnVal = NWPaintGTKSpinBox( nType, nPart, aCtrlRect, aClip, nState, aValue, rControlHandle, aCaption );
        }
        else if ( (nType == CTRL_COMBOBOX) &&
            ( (nPart==PART_ENTIRE_CONTROL)
            ||(nPart==PART_BUTTON_DOWN)
            ) )
        {
            returnVal = NWPaintGTKComboBox( gdkDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rControlHandle, aCaption );
        }
        else if ( (nType==CTRL_TAB_ITEM) || (nType==CTRL_TAB_PANE) || (nType==CTRL_TAB_BODY) || (nType==CTRL_FIXEDBORDER) )
        {
            if ( nType == CTRL_TAB_BODY )
                returnVal = TRUE;
            else
                returnVal = NWPaintGTKTabItem( nType, nPart, aCtrlRect, aClip, nState, aValue, rControlHandle, aCaption);
        }
        else if ( (nType==CTRL_LISTBOX) && ((nPart==PART_ENTIRE_CONTROL) || (nPart==PART_WINDOW)) )
        {
            returnVal = NWPaintGTKListBox( gdkDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rControlHandle, aCaption );
        }
        else if ( (nType== CTRL_TOOLBAR) )
        {
            returnVal = NWPaintGTKToolbar( gdkDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rControlHandle, aCaption );
        }
        else if ( (nType== CTRL_MENUBAR) )
        {
            returnVal = NWPaintGTKMenubar( gdkDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rControlHandle, aCaption );
        }
        else if(    (nType == CTRL_MENU_POPUP)
            && (  (nPart == PART_ENTIRE_CONTROL)
        || (nPart == PART_MENU_ITEM)
        )
        )
        {
            returnVal = NWPaintGTKPopupMenu( gdkDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rControlHandle, aCaption );
        }
        else if( (nType == CTRL_TOOLTIP) && (nPart == PART_ENTIRE_CONTROL) )
        {
            returnVal = NWPaintGTKTooltip( gdkDrawable, nType, nPart, aCtrlRect, aClip, nState, aValue, rControlHandle, aCaption );
        }
        if( pixmap )
        {
            returnVal = NWRenderPixmapToScreen( pixmap, aPixmapRect );
            g_object_unref( pixmap );
        }

        pWidgetMutex->release();
    }

    return( returnVal );
}


/*
 * DrawNativeControlText()
 *
 *  OPTIONAL.  Draws the requested text for the control described by nPart/nState.
 *     Used if text not drawn by DrawNativeControl().
 *
 *  rControlRegion: The bounding region of the complete control in VCL frame coordinates.
 *  aValue:         An optional value (tristate/numerical/string)
 *  rControlHandle: Carries platform dependent data and is maintained by the SalFrame implementation.
 *  aCaption:   A caption or title string (like button text etc)
 */
BOOL GtkSalGraphics::drawNativeControlText( ControlType nType,
                                ControlPart nPart,
                                const Region& rControlRegion,
                                ControlState nState,
                                const ImplControlValue& aValue,
                                SalControlHandle& rControlHandle,
                                OUString aCaption )
{
    return( FALSE );
}


/*
 * GetNativeControlRegion()
 *
 *  If the return value is TRUE, rNativeBoundingRegion
 *  contains the TRUE bounding region covered by the control
 *  including any adornment, while rNativeContentRegion contains the area
 *  within the control that can be safely drawn into without drawing over
 *  the borders of the control.
 *
 *  rControlRegion: The bounding region of the control in VCL frame coordinates.
 *  aValue:     An optional value (tristate/numerical/string)
 *  rControlHandle: Carries platform dependent data and is maintained by the SalFrame implementation.
 *  aCaption:       A caption or title string (like button text etc)
 */
BOOL GtkSalGraphics::getNativeControlRegion(  ControlType nType,
                                ControlPart nPart,
                                const Region& rControlRegion,
                                ControlState nState,
                                const ImplControlValue& aValue,
                                SalControlHandle& rControlHandle,
                                OUString aCaption,
                                Region &rNativeBoundingRegion,
                                Region &rNativeContentRegion )
{
    BOOL returnVal = FALSE;

    if ( pWidgetMutex->acquire() )
    {
        if ( (nType==CTRL_PUSHBUTTON) && (nPart==PART_ENTIRE_CONTROL)
            && (rControlRegion.GetBoundRect().GetWidth() > 16)
        && (rControlRegion.GetBoundRect().GetHeight() > 16) )
        {
            rNativeBoundingRegion = NWGetButtonArea( nType, nPart, rControlRegion.GetBoundRect(),
            nState, aValue, rControlHandle, aCaption );
            rNativeContentRegion = rControlRegion;

            returnVal = TRUE;
        }
        if ( (nType==CTRL_SPINBOX) && ((nPart==PART_BUTTON_UP) || (nPart==PART_BUTTON_DOWN)) )
        {
            rNativeBoundingRegion = NWGetSpinButtonRect( nType, nPart, rControlRegion.GetBoundRect(),
            nState, aValue, rControlHandle, aCaption );
            rNativeContentRegion = rNativeBoundingRegion;

            returnVal = TRUE;
        }
        if ( (nType==CTRL_COMBOBOX) && (nPart==PART_BUTTON_DOWN) )
        {
            rNativeBoundingRegion = NWGetComboBoxButtonRect( nType, nPart, rControlRegion.GetBoundRect(), nState,
            aValue, rControlHandle, aCaption );
            rNativeContentRegion = rNativeBoundingRegion;

            returnVal = TRUE;
        }
        if ( (nType==CTRL_SPINBOX) && ((nPart==PART_BUTTON_UP) || (nPart==PART_BUTTON_DOWN)) )
        {

            rNativeBoundingRegion = NWGetSpinButtonRect( nType, nPart, rControlRegion.GetBoundRect(), nState,
            aValue, rControlHandle, aCaption );
            rNativeContentRegion = rNativeBoundingRegion;

            returnVal = TRUE;
        }
        if ( (nType==CTRL_LISTBOX) && ((nPart==PART_BUTTON_DOWN) || (nPart==PART_SUB_EDIT)) )
        {
            rNativeBoundingRegion = NWGetListBoxButtonRect( nType, nPart, rControlRegion.GetBoundRect(), nState,
            aValue, rControlHandle, aCaption );
            rNativeContentRegion = rNativeBoundingRegion;

            returnVal = TRUE;
        }
        if ( (nType==CTRL_TOOLBAR) &&
            ((nPart==PART_DRAW_BACKGROUND_HORZ) ||
            (nPart==PART_DRAW_BACKGROUND_VERT)  ||
            (nPart==PART_THUMB_HORZ)            ||
            (nPart==PART_THUMB_VERT)            ||
            (nPart==PART_BUTTON)
            ))
        {
            rNativeBoundingRegion = NWGetToolbarRect( nType, nPart, rControlRegion.GetBoundRect(), nState, aValue, rControlHandle, aCaption );
            rNativeContentRegion = rNativeBoundingRegion;
            returnVal = TRUE;
        }
        if ( (nType==CTRL_SCROLLBAR) && ((nPart==PART_BUTTON_LEFT) || (nPart==PART_BUTTON_RIGHT) ||
            (nPart==PART_BUTTON_UP) || (nPart==PART_BUTTON_DOWN)  ) )
        {
            rNativeBoundingRegion = NWGetScrollButtonRect( nPart, rControlRegion.GetBoundRect() );
            rNativeContentRegion = rNativeBoundingRegion;

            returnVal = TRUE;
        }
        if( (nType == CTRL_MENUBAR) && (nPart == PART_ENTIRE_CONTROL) )
        {
            NWEnsureGTKMenubar();
            GtkRequisition aReq;
            gtk_widget_size_request( gMenubarWidget, &aReq );
            Rectangle aMenuBarRect = rControlRegion.GetBoundRect();
            aMenuBarRect = Rectangle( aMenuBarRect.TopLeft(),
                                      Size( aMenuBarRect.GetWidth(), aReq.height+1 ) );
            rNativeBoundingRegion = Region( aMenuBarRect );
            rNativeContentRegion = rNativeBoundingRegion;
            returnVal = TRUE;
        }
        if( (nType == CTRL_RADIOBUTTON || nType == CTRL_CHECKBOX) )
        {
            NWEnsureGTKRadio();
            NWEnsureGTKCheck();
            GtkWidget* widget = (nType == CTRL_RADIOBUTTON) ? gRadioWidget : gCheckWidget;
            gint indicator_size, indicator_spacing;
            gtk_widget_style_get( widget,
                                  "indicator_size", &indicator_size,
                                  "indicator_spacing", &indicator_spacing,
                                  NULL);
            indicator_size += 2*indicator_spacing; // guess overpaint of theme
            rNativeBoundingRegion = rControlRegion;
            Rectangle aIndicatorRect( Point( 0,
                                             (rControlRegion.GetBoundRect().GetHeight()-indicator_size)/2),
                                      Size( indicator_size, indicator_size ) );
            rNativeContentRegion = Region( aIndicatorRect );
            returnVal = TRUE;
        }

        pWidgetMutex->release();
    }

    return( returnVal );
}


/************************************************************************
 * Individual control drawing functions
 ************************************************************************/
BOOL GtkSalGraphics::NWPaintGTKButton(
            GdkDrawable* gdkDrawable,
            ControlType nType, ControlPart nPart,
            const Rectangle& rControlRectangle,
            const clipList& rClipList,
            ControlState nState, const ImplControlValue& aValue,
            SalControlHandle& rControlHandle, OUString aCaption )
{
    GtkStateType    stateType;
    GtkShadowType   shadowType;
    gboolean        interiorFocus;
    gint            focusWidth;
    gint            focusPad;
    BOOL            bDrawFocus = TRUE;
    gint            x, y, w, h;
    GtkBorder       aDefBorder;
    GtkBorder       aDefOutsideBorder;
    GtkBorder*      pBorder;
    GtkBorder*      pOutBorder;
    GdkRectangle    clipRect;

    NWEnsureGTKButton();
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    x = rControlRectangle.Left();
    y = rControlRectangle.Top();
    w = rControlRectangle.GetWidth();
    h = rControlRectangle.GetHeight();

    // Grab some button style attributes
    gtk_widget_style_get( gBtnWidget,   "focus-line-width", &focusWidth,
                                "focus-padding",    &focusPad,
                                 "interior_focus",  &interiorFocus,
                                "default_border",   &pBorder,
                                "default_outside_border", &pOutBorder, NULL );

    // Make sure the border values exist, otherwise use some defaults
    if ( pBorder )
    {
        NW_gtk_border_set_from_border( aDefBorder, pBorder );
        gtk_border_free( pBorder );
    }
    else NW_gtk_border_set_from_border( aDefBorder, &aDefDefBorder );

    if ( pOutBorder )
    {
        NW_gtk_border_set_from_border( aDefOutsideBorder, pOutBorder );
        gtk_border_free( pOutBorder );
    }
    else NW_gtk_border_set_from_border( aDefOutsideBorder, &aDefDefOutsideBorder );

    // If the button is too small, don't ever draw focus or grab more space
    if ( (w < 16) || (h < 16) )
        bDrawFocus = FALSE;

    NWSetWidgetState( gBtnWidget, nState, stateType );

    gint xi = x, yi = y, wi = w, hi = h;
    if ( (nState & CTRL_STATE_DEFAULT) && bDrawFocus )
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
        gtk_paint_flat_box( gBtnWidget->style, gdkDrawable, GTK_STATE_NORMAL, GTK_SHADOW_NONE,
                            &clipRect, m_pWindow, "base", x, y, w, h );

        if ( (nState & CTRL_STATE_DEFAULT) && (GTK_BUTTON(gBtnWidget)->relief == GTK_RELIEF_NORMAL) )
        {
            gtk_paint_box( gBtnWidget->style, gdkDrawable, GTK_STATE_NORMAL, GTK_SHADOW_IN,
                           &clipRect, gBtnWidget, "buttondefault", x, y, w, h );
        }

        if ( (GTK_BUTTON(gBtnWidget)->relief != GTK_RELIEF_NONE)
            || (nState & CTRL_STATE_PRESSED)
            || (nState & CTRL_STATE_ROLLOVER) )
        {
            gtk_paint_box( gBtnWidget->style, gdkDrawable, stateType, shadowType,
                           &clipRect, gBtnWidget, "button", xi, yi, wi, hi );
        }
    }
#if 0 // VCL draws focus rects
    // Draw focus rect
    if ( (nState & CTRL_STATE_FOCUSED) && (nState & CTRL_STATE_ENABLED) && bDrawFocus )
    {
        if (interiorFocus)
        {
            x += gBtnWidget->style->xthickness + focusPad;
            y += gBtnWidget->style->ythickness + focusPad;
            w -= 2 * (gBtnWidget->style->xthickness + focusPad);
            h -=  2 * (gBtnWidget->style->xthickness + focusPad);
        }
        else
        {
            x -= focusWidth + focusPad;
            y -= focusWidth + focusPad;
            w += 2 * (focusWidth + focusPad);
            h += 2 * (focusWidth + focusPad);
        }
        if ( !interiorFocus )
            gtk_paint_focus( gBtnWidget->style, gdkDrawable, stateType, &clipRect,
                             gBtnWidget, "button", x, y, w, h );
    }
#endif

    return( TRUE );
}

static Rectangle NWGetButtonArea( ControlType nType, ControlPart nPart, Rectangle aAreaRect, ControlState nState,
                            const ImplControlValue& aValue, SalControlHandle& rControlHandle, OUString aCaption )
{
    gboolean        interiorFocus;
    gint            focusWidth;
    gint            focusPad;
    GtkBorder       aDefBorder;
    GtkBorder       aDefOutsideBorder;
    GtkBorder * pBorder;
    GtkBorder * pOutBorder;
    BOOL            bDrawFocus = TRUE;
    Rectangle       aRect;
    gint            x, y, w, h;

    NWEnsureGTKButton();
    gtk_widget_style_get( gBtnWidget,   "focus-line-width", &focusWidth,
                                "focus-padding",    &focusPad,
                                 "interior_focus",  &interiorFocus,
                                "default_border",   &pBorder,
                                "default_outside_border", &pOutBorder, NULL );

    // Make sure the border values exist, otherwise use some defaults
    if ( pBorder )
    {
        NW_gtk_border_set_from_border( aDefBorder, pBorder );
        gtk_border_free( pBorder );
    }
    else NW_gtk_border_set_from_border( aDefBorder, &aDefDefBorder );

    if ( pOutBorder )
    {
        NW_gtk_border_set_from_border( aDefOutsideBorder, pOutBorder );
        gtk_border_free( pOutBorder );
    }
    else NW_gtk_border_set_from_border( aDefOutsideBorder, &aDefDefOutsideBorder );

    x = aAreaRect.Left();
    y = aAreaRect.Top();
    w = aAreaRect.GetWidth();
    h = aAreaRect.GetHeight();

    // If the button is too small, don't ever draw focus or grab more space
    if ( (w < 16) || (h < 16) )
        bDrawFocus = FALSE;

    if ( (nState & CTRL_STATE_DEFAULT) && bDrawFocus )
    {
        x -= aDefBorder.left;
        y -= aDefBorder.top;
        w += aDefBorder.left + aDefBorder.right;
        h += aDefBorder.top + aDefBorder.bottom;
    }

    aRect = Rectangle( Point( x, y ), Size( w, h ) );

    return( aRect );
}

//-------------------------------------

BOOL GtkSalGraphics::NWPaintGTKRadio( GdkDrawable* gdkDrawable,
                                      ControlType nType, ControlPart nPart,
                                      const Rectangle& rControlRectangle,
                                      const clipList& rClipList,
                                      ControlState nState,
                                      const ImplControlValue& aValue, SalControlHandle& rControlHandle,
                                      OUString aCaption )
{
    GtkStateType    stateType;
    GtkShadowType   shadowType;
    BOOL            isChecked = (aValue.getTristateVal()==BUTTONVALUE_ON);
    gint            x, y;
    GdkRectangle    clipRect;

    NWEnsureGTKButton();
    NWEnsureGTKRadio();
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    gint indicator_size;
    gtk_widget_style_get( gRadioWidget, "indicator_size", &indicator_size, NULL);

    x = rControlRectangle.Left() + (rControlRectangle.GetWidth()-indicator_size)/2;
    y = rControlRectangle.Top() + (rControlRectangle.GetHeight()-indicator_size)/2;

    // Set the shadow based on if checked or not so we get a freakin checkmark.
    shadowType = isChecked ? GTK_SHADOW_IN : GTK_SHADOW_OUT;
    NWSetWidgetState( gRadioWidget, nState, stateType );
    NWSetWidgetState( gRadioWidgetSibling, nState, stateType );

    // GTK enforces radio groups, so that if we don't have 2 buttons in the group,
    // the single button will always be active.  So we have to have 2 buttons.
    if (!isChecked)
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(gRadioWidgetSibling), TRUE );
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(gRadioWidget), isChecked );

    for( clipList::const_iterator it = rClipList.begin(); it != rClipList.end(); ++it )
    {
        clipRect.x = it->Left();
        clipRect.y = it->Top();
        clipRect.width = it->GetWidth();
        clipRect.height = it->GetHeight();

        gtk_paint_option( gRadioWidget->style, gdkDrawable, stateType, shadowType,
                          &clipRect, gRadioWidget, "radiobutton",
                          x, y, indicator_size, indicator_size );
    }

    return( TRUE );
}

//-------------------------------------

BOOL GtkSalGraphics::NWPaintGTKCheck( GdkDrawable* gdkDrawable,
                                      ControlType nType, ControlPart nPart,
                                      const Rectangle& rControlRectangle,
                                      const clipList& rClipList,
                                      ControlState nState,
                                      const ImplControlValue& aValue,
                                      SalControlHandle& rControlHandle, OUString aCaption )
{
    GtkStateType    stateType;
    GtkShadowType   shadowType;
    BOOL            isChecked = (aValue.getTristateVal()==BUTTONVALUE_ON) ? TRUE : FALSE;
    GdkRectangle    clipRect;
    gint            x,y;

    NWEnsureGTKButton();
    NWEnsureGTKCheck();
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    gint indicator_size;
    gtk_widget_style_get( gCheckWidget, "indicator_size", &indicator_size, NULL);

    x = rControlRectangle.Left() + (rControlRectangle.GetWidth()-indicator_size)/2;
    y = rControlRectangle.Top() + (rControlRectangle.GetHeight()-indicator_size)/2;

    // Set the shadow based on if checked or not so we get a checkmark.
    shadowType = isChecked ? GTK_SHADOW_IN : GTK_SHADOW_OUT;
    NWSetWidgetState( gCheckWidget, nState, stateType );
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(gCheckWidget), isChecked );

    for( clipList::const_iterator it = rClipList.begin(); it != rClipList.end(); ++it )
    {
        clipRect.x = it->Left();
        clipRect.y = it->Top();
        clipRect.width = it->GetWidth();
        clipRect.height = it->GetHeight();

        gtk_paint_check( gCheckWidget->style, gdkDrawable, stateType, shadowType,
                         &clipRect, gCheckWidget, "checkbutton",
                         x, y, indicator_size, indicator_size );
    }

    return( TRUE );
}

//-------------------------------------
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

BOOL GtkSalGraphics::NWPaintGTKScrollbar( ControlType nType, ControlPart nPart,
                                          const Rectangle& rControlRectangle,
                                          const clipList& rClipList,
                                          ControlState nState,
                                          const ImplControlValue& aValue, SalControlHandle& rControlHandle,
                                          OUString aCaption )
{
    ScrollbarValue* pScrollbarVal = (ScrollbarValue *)(aValue.getOptionalVal());
    GdkPixmap*      pixmap = NULL;
    Rectangle       pixmapRect, scrollbarRect;
    GtkStateType    stateType;
    GtkShadowType   shadowType;
    GtkScrollbar *  scrollbarWidget;
    GtkStyle *  style;
    GtkAdjustment* scrollbarValues = NULL;
    GtkOrientation  scrollbarOrientation;
    Rectangle       thumbRect = pScrollbarVal->maThumbRect;
    Rectangle       button11BoundRect = pScrollbarVal->maButton1Rect;   // backward
    Rectangle       button22BoundRect = pScrollbarVal->maButton2Rect;   // forward
    Rectangle       button12BoundRect = pScrollbarVal->maButton1Rect;   // secondary forward
    Rectangle       button21BoundRect = pScrollbarVal->maButton2Rect;   // secondary backward
    GtkArrowType    button1Type;                                        // backward
    GtkArrowType    button2Type;                                        // forward
    gchar *     scrollbarTagH = (gchar *) "hscrollbar";
    gchar *     scrollbarTagV = (gchar *) "vscrollbar";
    gchar *     scrollbarTag = NULL;
    Rectangle       arrowRect;
    gint            slider_width = 0;
    gint            stepper_size = 0;
    gint            stepper_spacing = 0;
    gint            trough_border = 0;
    gint            min_slider_length = 0;
    gint            vShim = 0;
    gint            hShim = 0;
    gint            x,y,w,h;

    NWEnsureGTKButton();
    NWEnsureGTKScrollbars();
    NWEnsureGTKArrow();

    // Find the overall bounding rect of the control
    pixmapRect = rControlRectangle;
    pixmapRect.SetSize( Size( pixmapRect.GetWidth() + 1,
                              pixmapRect.GetHeight() + 1 ) );
    scrollbarRect = pixmapRect;

    if ( (scrollbarRect.GetWidth() <= 1) || (scrollbarRect.GetHeight() <= 1) )
        return( TRUE );

    // Grab some button style attributes
    gtk_widget_style_get( gScrollHorizWidget, "slider_width", &slider_width,
                                      "stepper_size", &stepper_size,
                                      "trough_border", &trough_border,
                                      "stepper_spacing", &stepper_spacing,
                                      "min_slider_length", &min_slider_length, NULL );
    gboolean has_forward;
    gboolean has_forward2;
    gboolean has_backward;
    gboolean has_backward2;

    gtk_widget_style_get( gScrollHorizWidget, "has-forward-stepper", &has_forward,
                                      "has-secondary-forward-stepper", &has_forward2,
                                      "has-backward-stepper", &has_backward,
                                         "has-secondary-backward-stepper", &has_backward2, NULL );
    gint magic = trough_border ? 1 : 0;
    gint nFirst = 0;

    if ( has_backward )  nFirst  += 1;
    if ( has_forward2 )  nFirst  += 1;

    if ( nPart == PART_DRAW_BACKGROUND_HORZ )
    {
        unsigned int sliderHeight = slider_width + (trough_border * 2);
        vShim = (pixmapRect.GetHeight() - sliderHeight) / 2;

        if ( int(sliderHeight) < scrollbarRect.GetHeight() );
        {
            scrollbarRect.Move( 0, vShim );
            scrollbarRect.SetSize( Size( scrollbarRect.GetWidth(), sliderHeight ) );
        }

        scrollbarWidget = GTK_SCROLLBAR( gScrollHorizWidget );
        scrollbarOrientation = GTK_ORIENTATION_HORIZONTAL;
        scrollbarTag = scrollbarTagH;
        button1Type = GTK_ARROW_LEFT;
        button2Type = GTK_ARROW_RIGHT;

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
#if 0
        if ( (thumbRect.GetWidth() < min_slider_length)
            && ((scrollbarRect.GetWidth()-button1BoundRect.GetWidth()-button2BoundRect.GetWidth()) > min_slider_length) )
            thumbRect.SetSize( Size( min_slider_length, thumbRect.GetHeight() ) );
#endif

        thumbRect.Right() += magic;
        // Center vertically in the track
        thumbRect.Move( 0, (scrollbarRect.GetHeight() - slider_width) / 2 );
    }
    else
    {
        unsigned int sliderWidth = slider_width + (trough_border * 2);
        hShim = (pixmapRect.GetWidth() - sliderWidth) / 2;

        if ( int(sliderWidth) < scrollbarRect.GetWidth() );
        {
            scrollbarRect.Move( hShim, 0 );
            scrollbarRect.SetSize( Size( sliderWidth, scrollbarRect.GetHeight() ) );
        }

        scrollbarWidget = GTK_SCROLLBAR( gScrollVertWidget );
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
#if 0
        // Make sure the thumb is at least the default width (so we don't get tiny thumbs),
        // but if the VCL gives us a size smaller than the theme's default thumb size,
        // honor the VCL size
        if ( (thumbRect.GetHeight() < min_slider_length)
            && ((scrollbarRect.GetHeight()-button1BoundRect.GetHeight()-button2BoundRect.GetHeight()) > min_slider_length) )
            thumbRect.SetSize( Size( thumbRect.GetWidth(), min_slider_length ) );
#endif

        thumbRect.Bottom() += magic;
        // Center horizontally in the track
        thumbRect.Move( (scrollbarRect.GetWidth() - slider_width) / 2, 0 );
    }

    BOOL has_slider = ( thumbRect.GetWidth() > 0 && thumbRect.GetHeight() > 0 );

    scrollbarValues = gtk_range_get_adjustment( GTK_RANGE(scrollbarWidget) );
    if ( scrollbarValues == NULL )
        scrollbarValues = GTK_ADJUSTMENT( gtk_adjustment_new(0, 0, 0, 0, 0, 0) );
    if ( nPart == PART_DRAW_BACKGROUND_HORZ )
    {
        scrollbarValues->lower = pScrollbarVal->mnMin;
        scrollbarValues->upper = pScrollbarVal->mnMax;
        scrollbarValues->value = pScrollbarVal->mnCur;
        scrollbarValues->page_size = scrollbarRect.GetWidth() / 2;
    }
    else
    {
        scrollbarValues->lower = pScrollbarVal->mnMin;
        scrollbarValues->upper = pScrollbarVal->mnMax;
        scrollbarValues->value = pScrollbarVal->mnCur;
        scrollbarValues->page_size = scrollbarRect.GetHeight() / 2;
    }
    gtk_adjustment_changed( scrollbarValues );

    // as multiple paints are required for the scrollbar
    // painting them directly to the window flickers
    pixmap = NWGetPixmapFromScreen( pixmapRect );
    if( ! pixmap )
        return FALSE;
    x = y = 0;

    w = pixmapRect.GetWidth();
    h = pixmapRect.GetHeight();

    GdkDrawable* const &gdkDrawable = GDK_DRAWABLE( pixmap );
    GdkRectangle* gdkRect = NULL;

    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );
    NWSetWidgetState( GTK_WIDGET(scrollbarWidget), nState, stateType );
    NWSetWidgetState( gBtnWidget, nState, stateType );
    style = GTK_WIDGET( scrollbarWidget )->style;

    // ----------------- TROUGH
    gtk_paint_flat_box( gBtnWidget->style, gdkDrawable,
                        GTK_STATE_NORMAL, GTK_SHADOW_NONE, gdkRect,
                        m_pWindow, "base", x, y,
                        w, h );
    gtk_paint_box( style, gdkDrawable, GTK_STATE_ACTIVE, GTK_SHADOW_IN,
                   gdkRect, GTK_WIDGET(scrollbarWidget), "trough",
                   x, y,
                   scrollbarRect.GetWidth(), scrollbarRect.GetHeight() );

    if ( nState & CTRL_STATE_FOCUSED )
    {
        gtk_paint_focus( style, gdkDrawable, GTK_STATE_ACTIVE,
                         gdkRect, GTK_WIDGET(scrollbarWidget), "trough",
                         x, y,
                         scrollbarRect.GetWidth(), scrollbarRect.GetHeight() );
    }

    // ----------------- THUMB
    if ( has_slider )
    {
        NWConvertVCLStateToGTKState( pScrollbarVal->mnThumbState, &stateType, &shadowType );
        if ( pScrollbarVal->mnThumbState & CTRL_STATE_PRESSED )  stateType = GTK_STATE_PRELIGHT;
        gtk_paint_slider( style, gdkDrawable, stateType, GTK_SHADOW_OUT,
                        gdkRect, GTK_WIDGET(scrollbarWidget), "slider",
                        x+hShim+thumbRect.Left(), y+vShim+thumbRect.Top(),
                        thumbRect.GetWidth(), thumbRect.GetHeight(), scrollbarOrientation );
    }
    // ----------------- BUTTON 1 //
    if ( has_backward )
    {
        NWConvertVCLStateToGTKState( pScrollbarVal->mnButton1State, &stateType, &shadowType );
        if ( stateType == GTK_STATE_INSENSITIVE )   stateType = GTK_STATE_NORMAL;
        gtk_paint_box( style, gdkDrawable, stateType, shadowType,
                       gdkRect, GTK_WIDGET(scrollbarWidget), "stepper",
                       x+hShim+button11BoundRect.Left(), y+vShim+button11BoundRect.Top(),
                       button11BoundRect.GetWidth(), button11BoundRect.GetHeight() );
        // ----------------- ARROW 1
        NWCalcArrowRect( button11BoundRect, arrowRect );
        gtk_paint_arrow( style, gdkDrawable, stateType, shadowType,
                         gdkRect, GTK_WIDGET(scrollbarWidget), scrollbarTag, button1Type, TRUE,
                         x+hShim+arrowRect.Left(), y+vShim+arrowRect.Top(),
                         arrowRect.GetWidth(), arrowRect.GetHeight() );
    }
    if ( has_forward2 )
    {
        NWConvertVCLStateToGTKState( pScrollbarVal->mnButton2State, &stateType, &shadowType );
        if ( stateType == GTK_STATE_INSENSITIVE )   stateType = GTK_STATE_NORMAL;
        gtk_paint_box( style, gdkDrawable, stateType, shadowType,
                       gdkRect, GTK_WIDGET(scrollbarWidget), "stepper",
                       x+hShim+button12BoundRect.Left(), y+vShim+button12BoundRect.Top(),
                       button12BoundRect.GetWidth(), button12BoundRect.GetHeight() );
        // ----------------- ARROW 1
        NWCalcArrowRect( button12BoundRect, arrowRect );
        gtk_paint_arrow( style, gdkDrawable, stateType, shadowType,
                         gdkRect, GTK_WIDGET(scrollbarWidget), scrollbarTag, button2Type, TRUE,
                         x+hShim+arrowRect.Left(), y+vShim+arrowRect.Top(),
                         arrowRect.GetWidth(), arrowRect.GetHeight() );
    }
    // ----------------- BUTTON 2
    if ( has_backward2 )
    {
        NWConvertVCLStateToGTKState( pScrollbarVal->mnButton1State, &stateType, &shadowType );
        if ( stateType == GTK_STATE_INSENSITIVE )   stateType = GTK_STATE_NORMAL;
        gtk_paint_box( style, gdkDrawable, stateType, shadowType, gdkRect,
                       GTK_WIDGET(scrollbarWidget), "stepper",
                       x+hShim+button21BoundRect.Left(), y+vShim+button21BoundRect.Top(),
                       button21BoundRect.GetWidth(), button21BoundRect.GetHeight() );
        // ----------------- ARROW 2
        NWCalcArrowRect( button21BoundRect, arrowRect );
        gtk_paint_arrow( style, gdkDrawable, stateType, shadowType,
                         gdkRect, GTK_WIDGET(scrollbarWidget), scrollbarTag, button1Type, TRUE,
                         x+hShim+arrowRect.Left(), y+vShim+arrowRect.Top(),
                         arrowRect.GetWidth(), arrowRect.GetHeight() );
    }
    if ( has_forward )
    {
        NWConvertVCLStateToGTKState( pScrollbarVal->mnButton2State, &stateType, &shadowType );
        if ( stateType == GTK_STATE_INSENSITIVE )   stateType = GTK_STATE_NORMAL;
        gtk_paint_box( style, gdkDrawable, stateType, shadowType, gdkRect,
                       GTK_WIDGET(scrollbarWidget), "stepper",
                       x+hShim+button22BoundRect.Left(), y+vShim+button22BoundRect.Top(),
                       button22BoundRect.GetWidth(), button22BoundRect.GetHeight() );
        // ----------------- ARROW 2
        NWCalcArrowRect( button22BoundRect, arrowRect );
        gtk_paint_arrow( style, gdkDrawable, stateType, shadowType,
                         gdkRect, GTK_WIDGET(scrollbarWidget), scrollbarTag, button2Type, TRUE,
                         x+hShim+arrowRect.Left(), y+vShim+arrowRect.Top(),
                         arrowRect.GetWidth(), arrowRect.GetHeight() );
    }

    if( !NWRenderPixmapToScreen(pixmap, pixmapRect) )
    {
        g_object_unref( pixmap );
        return( FALSE );
    }
    g_object_unref( pixmap );

    return( TRUE );
}

//---

static Rectangle NWGetScrollButtonRect( ControlPart nPart, Rectangle aAreaRect )
{
    gint slider_width;
    gint stepper_size;
    gint stepper_spacing;
    gint trough_border;

    NWEnsureGTKScrollbars();

    // Grab some button style attributes
    gtk_widget_style_get( gScrollHorizWidget, "slider-width", &slider_width,
                                      "stepper-size", &stepper_size,
                                      "trough-border", &trough_border,
                                         "stepper-spacing", &stepper_spacing, NULL );

    gboolean has_forward;
    gboolean has_forward2;
    gboolean has_backward;
    gboolean has_backward2;

    gtk_widget_style_get( gScrollHorizWidget, "has-forward-stepper", &has_forward,
                                      "has-secondary-forward-stepper", &has_forward2,
                                      "has-backward-stepper", &has_backward,
                                         "has-secondary-backward-stepper", &has_backward2, NULL );
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

    return( buttonRect );
}

//-------------------------------------

BOOL GtkSalGraphics::NWPaintGTKEditBox( GdkDrawable* gdkDrawable,
                                        ControlType nType, ControlPart nPart,
                                        const Rectangle& rControlRectangle,
                                        const clipList& rClipList,
                                        ControlState nState,
                                        const ImplControlValue& aValue, SalControlHandle& rControlHandle,
                                        OUString aCaption )
{
    Rectangle       pixmapRect;
    GdkRectangle    clipRect;

    // Find the overall bounding rect of the buttons's drawing area,
    // plus its actual draw rect excluding adornment
    pixmapRect = NWGetEditBoxPixmapRect( nType, nPart, rControlRectangle,
                                         nState, aValue, rControlHandle, aCaption );
    for( clipList::const_iterator it = rClipList.begin(); it != rClipList.end(); ++it )
    {
        clipRect.x = it->Left();
        clipRect.y = it->Top();
        clipRect.width = it->GetWidth();
        clipRect.height = it->GetHeight();

        NWPaintOneEditBox( gdkDrawable, &clipRect, nType, nPart, pixmapRect, nState, aValue, rControlHandle, aCaption );
    }

    return( TRUE );
}


/* Take interior/exterior focus into account and return
 * the bounding rectangle of the edit box including
 * any focus requirements.
 */
static Rectangle NWGetEditBoxPixmapRect(ControlType         nType,
                                ControlPart         nPart,
                                Rectangle               aAreaRect,
                                ControlState            nState,
                                const ImplControlValue& aValue,
                                SalControlHandle&       rControlHandle,
                                OUString                aCaption )
{
    Rectangle       pixmapRect = aAreaRect;
    gboolean        interiorFocus;
    gint            focusWidth;

    NWEnsureGTKEditBox();

    // Grab some entry style attributes
    gtk_widget_style_get( gEditBoxWidget,   "focus-line-width", &focusWidth,
                                     "interior-focus",  &interiorFocus, NULL );

    if ( !interiorFocus )
    {
        pixmapRect.Move( -(focusWidth), -(focusWidth) );
        pixmapRect.SetSize( Size( pixmapRect.GetWidth() + (2*(focusWidth)),
                                  pixmapRect.GetHeight() + (2*(focusWidth)) ) );
    }

    return( pixmapRect );
}


/* Paint a GTK Entry widget into the specified GdkPixmap.
 * All coordinates should be local to the Pixmap, NOT
 * screen/window coordinates.
 */
static void NWPaintOneEditBox(  GdkDrawable * gdkDrawable,
                                GdkRectangle *  gdkRect,
                                ControlType         nType,
                                ControlPart         nPart,
                                Rectangle               aEditBoxRect,
                                ControlState            nState,
                                const ImplControlValue& aValue,
                                SalControlHandle&       rControlHandle,
                                OUString                aCaption )
{
    GtkStateType    stateType;
    GtkShadowType   shadowType;
    GtkWidget      *widget;

    NWEnsureGTKButton();
    NWEnsureGTKEditBox();
    NWEnsureGTKSpinButton();
    NWEnsureGTKCombo();
    NWEnsureGTKScrolledWindow();
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    /* border's shadowType for gtk entries is always GTK_SHADOW_IN (see gtkentry.c)
    shadowType = GTK_SHADOW_IN;
    */

    switch ( nType )
    {
        case CTRL_SPINBOX:
            widget = gSpinButtonWidget;
            break;

        case CTRL_MULTILINE_EDITBOX:
            widget = gScrolledWindowWidget;
            break;
        case CTRL_COMBOBOX:
            widget = GTK_COMBO(gComboWidget)->entry;
            break;

        default:
            widget = gEditBoxWidget;
            break;
    }

    if ( stateType == GTK_STATE_PRELIGHT )
        stateType = GTK_STATE_NORMAL;

    NWSetWidgetState( gBtnWidget, nState, stateType );
    NWSetWidgetState( widget, nState, stateType );

    // Blueprint needs to paint entry_bg with a Button widget, not an Entry widget to get
    // a nice white (or whatever default color) background
    gtk_paint_flat_box( gBtnWidget->style, gdkDrawable, stateType, GTK_SHADOW_NONE,
                        gdkRect, gBtnWidget, "entry_bg",
                        aEditBoxRect.Left(), aEditBoxRect.Top(),
                        aEditBoxRect.GetWidth(), aEditBoxRect.GetHeight() );
    gtk_paint_shadow( widget->style, gdkDrawable, GTK_STATE_NORMAL, GTK_SHADOW_IN,
                      gdkRect, widget, "entry",
                      aEditBoxRect.Left(), aEditBoxRect.Top(),
                      aEditBoxRect.GetWidth(), aEditBoxRect.GetHeight() );

}



//-------------------------------------

BOOL GtkSalGraphics::NWPaintGTKSpinBox( ControlType nType, ControlPart nPart,
                                        const Rectangle& rControlRectangle,
                                        const clipList& rClipList,
                                        ControlState nState,
                                        const ImplControlValue& aValue,
                                        SalControlHandle& rControlHandle, OUString aCaption )
{
    GdkPixmap   *       pixmap;
    Rectangle           pixmapRect;
    GtkStateType        stateType;
    GtkShadowType       shadowType;
    SpinbuttonValue *   pSpinVal = (SpinbuttonValue *)(aValue.getOptionalVal());
    Rectangle           upBtnRect;
    ControlPart     upBtnPart = PART_BUTTON_UP;
    ControlState        upBtnState = CTRL_STATE_ENABLED;
    Rectangle           downBtnRect;
    ControlPart     downBtnPart = PART_BUTTON_DOWN;
    ControlState        downBtnState = CTRL_STATE_ENABLED;

    NWEnsureGTKButton();
    NWEnsureGTKSpinButton();
    NWEnsureGTKArrow();

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
            fprintf( stderr, "Tried to draw CTRL_SPINBUTTONS, but the SpinButtons data structure didn't exist!\n" );
            return( false );
        }
        pixmapRect = pSpinVal->maUpperRect;
        pixmapRect.Union( pSpinVal->maLowerRect );
    }
    else
        pixmapRect = rControlRectangle;


    pixmap = NWGetPixmapFromScreen( pixmapRect );
    if ( !pixmap )
        return( FALSE );

    upBtnRect = NWGetSpinButtonRect( nType, upBtnPart, pixmapRect, upBtnState, aValue, rControlHandle, aCaption );
    downBtnRect = NWGetSpinButtonRect( nType, downBtnPart, pixmapRect, downBtnState, aValue, rControlHandle, aCaption );

    if ( (nType==CTRL_SPINBOX) && (nPart!=PART_ALL_BUTTONS) )
    {
        // Draw an edit field for SpinBoxes and ComboBoxes
        Rectangle aEditBoxRect( pixmapRect );
        aEditBoxRect.SetSize( Size( upBtnRect.Left() - pixmapRect.Left(), aEditBoxRect.GetHeight() ) );
        aEditBoxRect.setX( 0 );
        aEditBoxRect.setY( 0 );

        NWPaintOneEditBox( pixmap, NULL, nType, nPart, aEditBoxRect, nState, aValue, rControlHandle, aCaption );
    }

    NWSetWidgetState( gSpinButtonWidget, nState, stateType );
    gtk_widget_style_get( gSpinButtonWidget, "shadow_type", &shadowType, NULL );

    if ( shadowType != GTK_SHADOW_NONE )
    {
        Rectangle       shadowRect( upBtnRect );

        shadowRect.Union( downBtnRect );
        gtk_paint_box( gSpinButtonWidget->style, pixmap, GTK_STATE_NORMAL, shadowType, NULL,
            gSpinButtonWidget, "spinbutton",
            (shadowRect.Left() - pixmapRect.Left()), (shadowRect.Top() - pixmapRect.Top()),
            shadowRect.GetWidth(), shadowRect.GetHeight() );
    }

    NWPaintOneSpinButton( pixmap, nType, upBtnPart, pixmapRect, upBtnState, aValue, rControlHandle, aCaption );
    NWPaintOneSpinButton( pixmap, nType, downBtnPart, pixmapRect, downBtnState, aValue, rControlHandle, aCaption );

    if( !NWRenderPixmapToScreen(pixmap, pixmapRect) )
    {
        g_object_unref( pixmap );
        return( FALSE );
    }

    g_object_unref( pixmap );
    return( TRUE );
}

//---

static Rectangle NWGetSpinButtonRect(   ControlType         nType,
                                ControlPart         nPart,
                                Rectangle           aAreaRect,
                                ControlState            nState,
                                const ImplControlValue& aValue,
                                SalControlHandle&       rControlHandle,
                                OUString                aCaption )
{
    gint            buttonSize;
    Rectangle       buttonRect;

    NWEnsureGTKSpinButton();

    buttonSize = MAX( PANGO_PIXELS( pango_font_description_get_size(GTK_WIDGET(gSpinButtonWidget)->style->font_desc) ),
                   MIN_SPIN_ARROW_WIDTH );
    buttonSize -= buttonSize % 2 - 1; /* force odd */
    buttonRect.SetSize( Size( buttonSize + 2 * gSpinButtonWidget->style->xthickness,
                              buttonRect.GetHeight() ) );
    buttonRect.setX( aAreaRect.Left() + (aAreaRect.GetWidth() - buttonRect.GetWidth()) );
    if ( nPart == PART_BUTTON_UP )
    {
        buttonRect.setY( aAreaRect.Top() );
        buttonRect.Bottom() = buttonRect.Top() + (aAreaRect.GetHeight() / 2);
    }
    else
    {
        buttonRect.setY( aAreaRect.Top() + (aAreaRect.GetHeight() / 2) );
        buttonRect.Bottom() = aAreaRect.Bottom(); // cover area completely
    }

    return( buttonRect );
}

//---

static void NWPaintOneSpinButton(   GdkPixmap   *           pixmap,
                            ControlType         nType,
                            ControlPart         nPart,
                            Rectangle               aAreaRect,
                            ControlState            nState,
                            const ImplControlValue& aValue,
                            SalControlHandle&       rControlHandle,
                            OUString                aCaption )
{
    Rectangle           buttonRect;
    GtkStateType        stateType;
    GtkShadowType       shadowType;
    Rectangle           arrowRect;
    gint                arrowSize;

    NWEnsureGTKSpinButton();
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    buttonRect = NWGetSpinButtonRect( nType, nPart, aAreaRect, nState, aValue, rControlHandle, aCaption );

    NWSetWidgetState( gSpinButtonWidget, nState, stateType );
    gtk_paint_box( gSpinButtonWidget->style, pixmap, stateType, shadowType, NULL, gSpinButtonWidget,
            (nPart == PART_BUTTON_UP) ? "spinbutton_up" : "spinbutton_down",
            (buttonRect.Left() - aAreaRect.Left()), (buttonRect.Top() - aAreaRect.Top()),
            buttonRect.GetWidth(), buttonRect.GetHeight() );

    arrowSize = (buttonRect.GetWidth() - (2 * gSpinButtonWidget->style->xthickness)) - 4;
    arrowSize -= arrowSize % 2 - 1; /* force odd */
    arrowRect.SetSize( Size( arrowSize, arrowSize ) );
    arrowRect.setX( buttonRect.Left() + (buttonRect.GetWidth() - arrowRect.GetWidth()) / 2 );
    if ( nPart == PART_BUTTON_UP )
        arrowRect.setY( buttonRect.Top() + (buttonRect.GetHeight() - arrowRect.GetHeight()) / 2 + 1);
    else
        arrowRect.setY( buttonRect.Top() + (buttonRect.GetHeight() - arrowRect.GetHeight()) / 2 - 1);

    gtk_paint_arrow( gSpinButtonWidget->style, pixmap, GTK_STATE_NORMAL, GTK_SHADOW_OUT, NULL, gSpinButtonWidget,
            "spinbutton", (nPart == PART_BUTTON_UP) ? GTK_ARROW_UP : GTK_ARROW_DOWN, TRUE,
            (arrowRect.Left() - aAreaRect.Left()), (arrowRect.Top() - aAreaRect.Top()),
            arrowRect.GetWidth(), arrowRect.GetHeight() );
}


//-------------------------------------

BOOL GtkSalGraphics::NWPaintGTKComboBox( GdkDrawable* gdkDrawable,
                                         ControlType nType, ControlPart nPart,
                                         const Rectangle& rControlRectangle,
                                         const clipList& rClipList,
                                         ControlState nState,
                                         const ImplControlValue& aValue,
                                         SalControlHandle& rControlHandle, OUString aCaption )
{
    Rectangle       pixmapRect;
    Rectangle       buttonRect;
    GtkStateType    stateType;
    GtkShadowType   shadowType;
    Rectangle       arrowRect;
    gint            x,y;
    GdkRectangle    clipRect;

    NWEnsureGTKButton();
    NWEnsureGTKArrow();
    NWEnsureGTKCombo();
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    // Find the overall bounding rect of the buttons's drawing area,
    // plus its actual draw rect excluding adornment
    pixmapRect = rControlRectangle;
    x = rControlRectangle.Left();
    y = rControlRectangle.Top();

    NWSetWidgetState( gBtnWidget, nState, stateType );
    NWSetWidgetState( gComboWidget, nState, stateType );
    NWSetWidgetState( gArrowWidget, nState, stateType );

    buttonRect = NWGetComboBoxButtonRect( nType, nPart, pixmapRect, nState, aValue, rControlHandle, aCaption );
    if( nPart == PART_BUTTON_DOWN )
        buttonRect.Left() += 1;

    Rectangle       aEditBoxRect( pixmapRect );
    aEditBoxRect.SetSize( Size( pixmapRect.GetWidth() - buttonRect.GetWidth(), aEditBoxRect.GetHeight() ) );

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
            NWPaintOneEditBox( gdkDrawable, &clipRect, nType, nPart, aEditBoxRect,
                               nState, aValue, rControlHandle, aCaption );

        // Buttons must paint opaque since some themes have alpha-channel enabled buttons
        gtk_paint_flat_box( gBtnWidget->style, gdkDrawable, GTK_STATE_NORMAL, GTK_SHADOW_NONE,
                            &clipRect, m_pWindow, "base",
                            x+(buttonRect.Left() - pixmapRect.Left()),
                            y+(buttonRect.Top() - pixmapRect.Top()),
                            buttonRect.GetWidth(), buttonRect.GetHeight() );
        gtk_paint_box( GTK_COMBO(gComboWidget)->button->style, gdkDrawable, stateType, shadowType,
                       &clipRect, GTK_COMBO(gComboWidget)->button, "button",
                       x+(buttonRect.Left() - pixmapRect.Left()),
                       y+(buttonRect.Top() - pixmapRect.Top()),
                       buttonRect.GetWidth(), buttonRect.GetHeight() );

        gtk_paint_arrow( gArrowWidget->style, gdkDrawable, stateType, shadowType,
                         &clipRect, gArrowWidget, "arrow", GTK_ARROW_DOWN, TRUE,
                         x+(arrowRect.Left() - pixmapRect.Left()), y+(arrowRect.Top() - pixmapRect.Top()),
                         arrowRect.GetWidth(), arrowRect.GetHeight() );
    }

    return( TRUE );
}

//----

static Rectangle NWGetComboBoxButtonRect(   ControlType         nType,
                                    ControlPart         nPart,
                                    Rectangle               aAreaRect,
                                    ControlState            nState,
                                    const ImplControlValue& aValue,
                                    SalControlHandle&       rControlHandle,
                                    OUString                aCaption )
{
    Rectangle   aButtonRect;
    gint        nArrowWidth;
    gint        nFocusWidth;
    gint        nFocusPad;

    NWEnsureGTKArrow();

    // Grab some button style attributes
    gtk_widget_style_get( gDropdownWidget,  "focus-line-width", &nFocusWidth,
                                    "focus-padding",    &nFocusPad, NULL );

    nArrowWidth = MIN_ARROW_SIZE + (GTK_MISC(gArrowWidget)->xpad * 2);
    aButtonRect.SetSize( Size( nArrowWidth + ((BTN_CHILD_SPACING + gDropdownWidget->style->xthickness) * 2)
                               + (2 * (nFocusWidth+nFocusPad)),
                               aAreaRect.GetHeight() ) );
    aButtonRect.SetPos( Point( aAreaRect.Left() + aAreaRect.GetWidth() - aButtonRect.GetWidth(),
                        aAreaRect.Top() ) );

    return( aButtonRect );
}

//-------------------------------------



BOOL GtkSalGraphics::NWPaintGTKTabItem( ControlType nType, ControlPart nPart,
                                        const Rectangle& rControlRectangle,
                                        const clipList& rClipList,
                                        ControlState nState,
                                        const ImplControlValue& aValue,
                                        SalControlHandle& rControlHandle, OUString aCaption )
{
    GdkPixmap * pixmap;
    Rectangle       pixmapRect;
    Rectangle       tabRect;
    TabitemValue *  pTabitemValue = (TabitemValue *)(aValue.getOptionalVal());
    GtkStateType    stateType;
    GtkShadowType   shadowType;
    static NWPixmapCache aCacheItems;
    static NWPixmapCache aCachePage;

    if( !aCacheItems.GetSize() )
        aCacheItems.SetSize( 20 );
    if( !aCachePage.GetSize() )
        aCachePage.SetSize( 1 );

    if ( !pTabitemValue && (nType==CTRL_TAB_ITEM) )
    {
        fprintf( stderr, "NWPaintGTKTabItem() received a NULL TabitemValue.  Cannot draw native tab\n" );
        return( false );
    }

    NWEnsureGTKButton();
    NWEnsureGTKNotebook();
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    // Find the overall bounding rect of the buttons's drawing area,
    // plus its actual draw rect excluding adornment
    pixmapRect = rControlRectangle;
    if ( nType == CTRL_TAB_ITEM )
    {
        if ( !pTabitemValue->isFirst() )
        {
            // GTK+ tabs overlap on the right edge (the top tab obscures the
            // left edge of the tab right "below" it, so adjust the rectangle
            // to draw tabs slightly large so the overlap happens
            pixmapRect.Move( -2, 0 );
            pixmapRect.SetSize( Size( pixmapRect.GetWidth() + 2, pixmapRect.GetHeight() ) );
        }
        if ( nState & CTRL_STATE_SELECTED )
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
    }

    if( nType == CTRL_TAB_ITEM )
    {
        if( aCacheItems.Find( nType, nState, pixmapRect, &pixmap ) )
            return NWRenderPixmapToScreen( pixmap, pixmapRect );
    }
    else
    {
        if( aCachePage.Find( nType, nState, pixmapRect, &pixmap ) )
            return NWRenderPixmapToScreen( pixmap, pixmapRect );
    }


//  gtk_widget_set_state( gNotebookWidget, stateType );

    pixmap = NWGetPixmapFromScreen( pixmapRect );
    if ( !pixmap )
        return( FALSE );

    NWSetWidgetState( gNotebookWidget, nState, stateType );
    switch( nType )
    {
        case CTRL_TAB_BODY:
            break;

        case CTRL_FIXEDBORDER:
        case CTRL_TAB_PANE:
            gtk_paint_box_gap( gNotebookWidget->style, pixmap, GTK_STATE_NORMAL, GTK_SHADOW_OUT, NULL, gNotebookWidget,
                (char *)"notebook", 0, 0, pixmapRect.GetWidth(), pixmapRect.GetHeight(), GTK_POS_TOP, 0, 0 );
            break;

        case CTRL_TAB_ITEM:
            stateType = ( nState & CTRL_STATE_SELECTED ) ? GTK_STATE_NORMAL : GTK_STATE_ACTIVE;

            gtk_paint_extension( gNotebookWidget->style, pixmap, stateType, GTK_SHADOW_OUT, NULL, gNotebookWidget,
                (char *)"tab", (tabRect.Left() - pixmapRect.Left()), (tabRect.Top() - pixmapRect.Top()),
                tabRect.GetWidth(), tabRect.GetHeight(), GTK_POS_BOTTOM );

            if ( nState & CTRL_STATE_SELECTED )
            {
                gtk_paint_flat_box( gNotebookWidget->style, pixmap, stateType, GTK_SHADOW_NONE, NULL, m_pWindow,
                    (char *)"base", 0, (pixmapRect.GetHeight() - 1), pixmapRect.GetWidth(), 1 );
            }
            break;

        default:
            break;
    }

    // Crux seems to think it can make the pane without a left edge
    if ( nType == CTRL_FIXEDBORDER )
        pixmapRect.Move( 1, 0 );

    // cache data
    if( GetGtkFrame()->getVisibilityState() == GDK_VISIBILITY_UNOBSCURED )
    {
        if( nType == CTRL_TAB_ITEM )
            aCacheItems.Fill( nType, nState, pixmapRect, pixmap );
        else
            aCachePage.Fill( nType, nState, pixmapRect, pixmap );
    }

    if( !NWRenderPixmapToScreen(pixmap, pixmapRect) )
    {
        g_object_unref( pixmap );
        return( FALSE );
    }

    g_object_unref( pixmap );
    return( TRUE );
}

//-------------------------------------

BOOL GtkSalGraphics::NWPaintGTKListBox( GdkDrawable* gdkDrawable,
                                        ControlType nType, ControlPart nPart,
                                        const Rectangle& rControlRectangle,
                                        const clipList& rClipList,
                                        ControlState nState,
                                        const ImplControlValue& aValue,
                                        SalControlHandle& rControlHandle, OUString aCaption )
{
    Rectangle       pixmapRect;
    Rectangle       widgetRect;
    Rectangle       aIndicatorRect;
    GtkStateType    stateType;
    GtkShadowType   shadowType;
    gint            bInteriorFocus;
    gint            nFocusLineWidth;
    gint            nFocusPadding;
    gint            x,y;
    GdkRectangle    clipRect;

    NWEnsureGTKButton();
    NWEnsureGTKOptionMenu();
    NWEnsureGTKScrolledWindow();
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    // Find the overall bounding rect of the buttons's drawing area,
    // plus its actual draw rect excluding adornment
    pixmapRect = rControlRectangle;
    if ( nPart == PART_WINDOW )
    {
        // Make the widget a _bit_ bigger
        pixmapRect.SetPos( Point( pixmapRect.Left() - 1,
                                  pixmapRect.Top() - 1 ) );
        pixmapRect.SetSize( Size( pixmapRect.GetWidth() + 2,
                                  pixmapRect.GetHeight() + 2 ) );
    }

    widgetRect = pixmapRect;
    x = pixmapRect.Left();
    y = pixmapRect.Top();

    // set up references to correct drawable and cliprect
    NWSetWidgetState( gBtnWidget, nState, stateType );
    NWSetWidgetState( gOptionMenuWidget, nState, stateType );
    NWSetWidgetState( gScrolledWindowWidget, nState, stateType );

    if ( nPart != PART_WINDOW )
    {
        gtk_widget_style_get( gOptionMenuWidget,
            "interior_focus",   &bInteriorFocus,
            "focus_line_width", &nFocusLineWidth,
            "focus_padding",    &nFocusPadding,
            NULL);
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
            gtk_paint_flat_box( gBtnWidget->style, gdkDrawable, GTK_STATE_NORMAL, GTK_SHADOW_NONE,
                                &clipRect, m_pWindow, "base", x, y,
                                pixmapRect.GetWidth(), pixmapRect.GetHeight() );
            gtk_paint_box( gOptionMenuWidget->style, gdkDrawable, stateType, shadowType, &clipRect,
                           gOptionMenuWidget, "optionmenu",
                           x+(widgetRect.Left() - pixmapRect.Left()),
                           y+(widgetRect.Top() - pixmapRect.Top()),
                           widgetRect.GetWidth(), widgetRect.GetHeight() );
            aIndicatorRect = NWGetListBoxIndicatorRect( nType, nPart, widgetRect, nState,
                                                        aValue, rControlHandle, aCaption );
            gtk_paint_tab( gOptionMenuWidget->style, gdkDrawable, stateType, shadowType, &clipRect,
                           gOptionMenuWidget, "optionmenutab",
                           x+(aIndicatorRect.Left() - pixmapRect.Left()),
                           y+(aIndicatorRect.Top() - pixmapRect.Top()),
                           aIndicatorRect.GetWidth(), aIndicatorRect.GetHeight() );
        }
        else
        {
            shadowType = GTK_SHADOW_IN;

            gtk_paint_shadow( gScrolledWindowWidget->style, gdkDrawable, GTK_STATE_NORMAL, shadowType,
                &clipRect, gScrolledWindowWidget, "scrolled_window",
                x+(widgetRect.Left() - pixmapRect.Left()), y+(widgetRect.Top() - pixmapRect.Top()),
                widgetRect.GetWidth(), widgetRect.GetHeight() );
        }
    }

    return( TRUE );
}

BOOL GtkSalGraphics::NWPaintGTKToolbar(
            GdkDrawable* gdkDrawable,
            ControlType nType, ControlPart nPart,
            const Rectangle& rControlRectangle,
            const clipList& rClipList,
            ControlState nState, const ImplControlValue& aValue,
            SalControlHandle& rControlHandle, OUString aCaption )
{
    GtkStateType    stateType;
    GtkShadowType   shadowType;
    gint            x, y, w, h;
    gint            g_x=0, g_y=0, g_w=10, g_h=10;
    bool            bPaintButton = true;
    GtkWidget*      pButtonWidget = gToolbarButtonWidget;
    gchar*          pButtonDetail = "button";
    GdkRectangle    clipRect;

    NWEnsureGTKToolbar();
    if( nPart == PART_BUTTON ) // toolbar buttons cannot focus in gtk
        nState &= ~CTRL_STATE_FOCUSED;
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    x = rControlRectangle.Left();
    y = rControlRectangle.Top();
    w = rControlRectangle.GetWidth();
    h = rControlRectangle.GetHeight();

    // handle toolbar
    if( nPart == PART_DRAW_BACKGROUND_HORZ || nPart == PART_DRAW_BACKGROUND_VERT )
    {
        NWSetWidgetState( gToolbarWidget, nState, stateType );

        GTK_WIDGET_UNSET_FLAGS( gToolbarWidget, GTK_SENSITIVE );
        if ( nState & CTRL_STATE_ENABLED )
            GTK_WIDGET_SET_FLAGS( gToolbarWidget, GTK_SENSITIVE );

        if( nPart == PART_DRAW_BACKGROUND_HORZ )
            gtk_toolbar_set_orientation( GTK_TOOLBAR(gToolbarWidget), GTK_ORIENTATION_HORIZONTAL );
        else
            gtk_toolbar_set_orientation( GTK_TOOLBAR(gToolbarWidget), GTK_ORIENTATION_VERTICAL );
    }
    // handle grip
    else if( nPart == PART_THUMB_HORZ || nPart == PART_THUMB_VERT )
    {
        NWSetWidgetState( gHandleBoxWidget, nState, stateType );

        GTK_WIDGET_UNSET_FLAGS( gHandleBoxWidget, GTK_SENSITIVE );
        if ( nState & CTRL_STATE_ENABLED )
            GTK_WIDGET_SET_FLAGS( gHandleBoxWidget, GTK_SENSITIVE );

        gtk_handle_box_set_shadow_type( GTK_HANDLE_BOX(gHandleBoxWidget), shadowType );

        // evaluate grip rect
        ToolbarValue* pVal = (ToolbarValue*)aValue.getOptionalVal();
        if( pVal )
        {
            g_x = rControlRectangle.Left() + pVal->maGripRect.Left();
            g_y = rControlRectangle.Top()  + pVal->maGripRect.Top();
            g_w = pVal->maGripRect.GetWidth();
            g_h = pVal->maGripRect.GetHeight();
        }
    }
    // handle button
    else if( nPart == PART_BUTTON )
    {
        bPaintButton =
            (GTK_BUTTON(pButtonWidget)->relief != GTK_RELIEF_NONE)
            || (nState & CTRL_STATE_PRESSED)
            || (nState & CTRL_STATE_ROLLOVER);
        if( aValue.getTristateVal() == BUTTONVALUE_ON )
        {
            pButtonWidget = gToolbarToggleWidget;
            shadowType = GTK_SHADOW_IN;
            // special case stateType value for depressed toggle buttons
            // cf. gtk+/gtk/gtktogglebutton.c (gtk_toggle_button_update_state)
            if( ! (nState & (CTRL_STATE_PRESSED|CTRL_STATE_ROLLOVER)) )
                stateType = GTK_STATE_ACTIVE;
            pButtonDetail = "togglebutton";
            bPaintButton = true;
        }

        NWSetWidgetState( pButtonWidget, nState, stateType );
        gtk_widget_ensure_style( pButtonWidget );
    }

    for( clipList::const_iterator it = rClipList.begin(); it != rClipList.end(); ++it )
    {
        clipRect.x = it->Left();
        clipRect.y = it->Top();
        clipRect.width = it->GetWidth();
        clipRect.height = it->GetHeight();

        // draw toolbar
        if( nPart == PART_DRAW_BACKGROUND_HORZ || nPart == PART_DRAW_BACKGROUND_VERT )
        {
            gtk_paint_box( gToolbarWidget->style,
                           gdkDrawable,
                           stateType,
                           shadowType,
                           &clipRect,
                           gToolbarWidget,
                           "toolbar",
                           x, y, w, h );
        }
        // draw grip
        else if( nPart == PART_THUMB_HORZ || nPart == PART_THUMB_VERT )
        {
            gtk_paint_handle( gHandleBoxWidget->style,
                              gdkDrawable,
                              GTK_STATE_NORMAL,
                              GTK_SHADOW_OUT,
                              &clipRect,
                              gHandleBoxWidget,
                              "handlebox",
                              g_x, g_y, g_w, g_h,
                              nPart == PART_THUMB_HORZ ?
                              GTK_ORIENTATION_HORIZONTAL :
                              GTK_ORIENTATION_VERTICAL
                              );
        }
        // draw button
        else if( nPart == PART_BUTTON )
        {
            if( bPaintButton )
            {
                gtk_paint_box( pButtonWidget->style, gdkDrawable,
                               stateType,
                               shadowType,
                               &clipRect,
                               pButtonWidget, pButtonDetail, x, y, w, h );
            }
        }
    }

    return( TRUE );
}

//----

BOOL GtkSalGraphics::NWPaintGTKMenubar(
            GdkDrawable* gdkDrawable,
            ControlType nType, ControlPart nPart,
            const Rectangle& rControlRectangle,
            const clipList& rClipList,
            ControlState nState, const ImplControlValue& aValue,
            SalControlHandle& rControlHandle, OUString aCaption )
{
    GtkStateType    stateType;
    GtkShadowType   shadowType;
    GtkShadowType   selected_shadow_type = GTK_SHADOW_OUT;
    gint            x, y, w, h;
    GdkRectangle    clipRect;

    NWEnsureGTKMenubar();
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    x = rControlRectangle.Left();
    y = rControlRectangle.Top();
    w = rControlRectangle.GetWidth();
    h = rControlRectangle.GetHeight();

    if( nPart == PART_MENU_ITEM )
    {
        if( nState & (CTRL_STATE_SELECTED|CTRL_STATE_ROLLOVER) )
        {
            gtk_widget_style_get( gMenuItemMenubarWidget,
                                  "selected_shadow_type", &selected_shadow_type,
                                  NULL);
        }
    }

    for( clipList::const_iterator it = rClipList.begin(); it != rClipList.end(); ++it )
    {
        clipRect.x = it->Left();
        clipRect.y = it->Top();
        clipRect.width = it->GetWidth();
        clipRect.height = it->GetHeight();

        // handle Menubar
        if( nPart == PART_ENTIRE_CONTROL )
        {
            NWSetWidgetState( gMenubarWidget, nState, stateType );

            GTK_WIDGET_UNSET_FLAGS( gMenubarWidget, GTK_SENSITIVE );
            if ( nState & CTRL_STATE_ENABLED )
                GTK_WIDGET_SET_FLAGS( gMenubarWidget, GTK_SENSITIVE );

            // #118704# for translucent menubar styles paint background first
            gtk_paint_flat_box( gMenubarWidget->style,
                                gdkDrawable,
                                GTK_STATE_NORMAL,
                                GTK_SHADOW_NONE,
                                &clipRect,
                                GTK_WIDGET(m_pWindow),
                                "base",
                                x, y, w, h );
            gtk_paint_box( gMenubarWidget->style,
                           gdkDrawable,
                           stateType,
                           shadowType,
                           &clipRect,
                           gMenubarWidget,
                           "menubar",
                           x, y, w, h );
        }
        else if( nPart == PART_MENU_ITEM )
        {
            if( nState & (CTRL_STATE_SELECTED|CTRL_STATE_ROLLOVER) )
            {
                gtk_paint_box( gMenuItemMenubarWidget->style,
                               gdkDrawable,
                               GTK_STATE_PRELIGHT,
                               selected_shadow_type,
                               &clipRect,
                               gMenuItemMenubarWidget,
                               "menuitem",
                               x, y, w, h);
            }
        }
    }

    return( TRUE );
}

BOOL GtkSalGraphics::NWPaintGTKPopupMenu(
            GdkDrawable* gdkDrawable,
            ControlType nType, ControlPart nPart,
            const Rectangle& rControlRectangle,
            const clipList& rClipList,
            ControlState nState, const ImplControlValue& aValue,
            SalControlHandle& rControlHandle, OUString aCaption )
{
    // #i50745# gtk does not draw disabled menu entries (and crux theme
    // even crashes), draw them using vcl functionality.
    if( nPart == PART_MENU_ITEM && ! (nState & CTRL_STATE_ENABLED) )
        return FALSE;

    GtkStateType    stateType;
    GtkShadowType   shadowType;
    GtkShadowType   selected_shadow_type = GTK_SHADOW_OUT;
    gint            x, y, w, h;
    GdkRectangle    clipRect;

    NWEnsureGTKMenu();
    NWConvertVCLStateToGTKState( nState, &stateType, &shadowType );

    x = rControlRectangle.Left();
    y = rControlRectangle.Top();
    w = rControlRectangle.GetWidth();
    h = rControlRectangle.GetHeight();

    if( nPart == PART_MENU_ITEM &&
        ( nState & (CTRL_STATE_SELECTED|CTRL_STATE_ROLLOVER) ) )
    {
        gtk_widget_style_get( gMenuItemMenuWidget,
                              "selected_shadow_type", &selected_shadow_type,
                              NULL);
    }

    NWSetWidgetState( gMenuWidget, nState, stateType );

    GTK_WIDGET_UNSET_FLAGS( gMenuWidget, GTK_SENSITIVE );
    if ( nState & CTRL_STATE_ENABLED )
        GTK_WIDGET_SET_FLAGS( gMenuWidget, GTK_SENSITIVE );

    for( clipList::const_iterator it = rClipList.begin(); it != rClipList.end(); ++it )
    {
        clipRect.x = it->Left();
        clipRect.y = it->Top();
        clipRect.width = it->GetWidth();
        clipRect.height = it->GetHeight();

        if( nPart == PART_ENTIRE_CONTROL )
        {
            // #118704# for translucent menubar styles paint background first
            gtk_paint_flat_box( gMenuWidget->style,
                                gdkDrawable,
                                GTK_STATE_NORMAL,
                                GTK_SHADOW_NONE,
                                &clipRect,
                                GTK_WIDGET(m_pWindow),
                                "base",
                                x, y, w, h );
            gtk_paint_box( gMenuWidget->style,
                           gdkDrawable,
                           GTK_STATE_NORMAL,
                           GTK_SHADOW_OUT,
                           &clipRect,
                           gMenuWidget,
                           "menu",
                           x, y, w, h );
        }
        else if( nPart == PART_MENU_ITEM )
        {
            if( nState & (CTRL_STATE_SELECTED|CTRL_STATE_ROLLOVER) )
            {
                if( nState & CTRL_STATE_ENABLED )
                gtk_paint_box( gMenuItemMenuWidget->style,
                               gdkDrawable,
                               GTK_STATE_PRELIGHT,
                               selected_shadow_type,
                               &clipRect,
                               gMenuItemMenuWidget,
                               "menuitem",
                               x, y, w, h);
            }
        }
    }

    return( TRUE );
}

BOOL GtkSalGraphics::NWPaintGTKTooltip(
            GdkDrawable* gdkDrawable,
            ControlType nType, ControlPart nPart,
            const Rectangle& rControlRectangle,
            const clipList& rClipList,
            ControlState nState, const ImplControlValue& aValue,
            SalControlHandle& rControlHandle, OUString aCaption )
{
    NWEnsureGTKTooltip();

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

        gtk_paint_flat_box( gTooltipPopup->style,
                            gdkDrawable,
                            GTK_STATE_NORMAL,
                            GTK_SHADOW_OUT,
                            &clipRect,
                            gTooltipPopup,
                            "tooltip",
                            x, y, w, h );
    }

    return( TRUE );
}

//----

static Rectangle NWGetListBoxButtonRect( ControlType    nType,
                                         ControlPart    nPart,
                                         Rectangle      aAreaRect,
                                         ControlState   nState,
                                         const ImplControlValue&    aValue,
                                         SalControlHandle&          rControlHandle,
                                         OUString       aCaption )
{
    Rectangle       aPartRect;
    GtkRequisition *pIndicatorSize = NULL;
    GtkBorder      *pIndicatorSpacing = NULL;
    gint            width = 13; // GTK+ default
    gint            right = 5;  // GTK+ default
    gint            nButtonAreaWidth = 0;
    gint            xthickness = 0;

    NWEnsureGTKOptionMenu();

    gtk_widget_style_get( gOptionMenuWidget,
            "indicator_size",   &pIndicatorSize,
            "indicator_spacing",&pIndicatorSpacing, NULL);

    if ( pIndicatorSize )
        width = pIndicatorSize->width;

    if ( pIndicatorSpacing )
        right = pIndicatorSpacing->right;

    Size aPartSize( 0, aAreaRect.GetHeight() );
    Point aPartPos ( 0, aAreaRect.Top() );

    xthickness = gOptionMenuWidget->style->xthickness;
    nButtonAreaWidth = width + right + (xthickness * 2);
    switch( nPart )
    {
        case PART_BUTTON_DOWN:
            aPartSize.Width() = nButtonAreaWidth;
            aPartPos.X() = aAreaRect.Left() + aAreaRect.GetWidth() - aPartSize.Width();
            break;

        case PART_SUB_EDIT:
            aPartSize.Width() = aAreaRect.GetWidth() - nButtonAreaWidth - xthickness;
            aPartPos.X() = aAreaRect.Left() + xthickness;
            break;

        default:
            aPartSize.Width() = aAreaRect.GetWidth();
            aPartPos.X() = aAreaRect.Left();
            break;
    }
    aPartRect = Rectangle( aPartPos, aPartSize );

    if ( pIndicatorSize )
        g_free( pIndicatorSize );
    if ( pIndicatorSpacing )
        g_free( pIndicatorSpacing );

    return( aPartRect );
}

//----

static Rectangle NWGetListBoxIndicatorRect( ControlType         nType,
                                    ControlPart         nPart,
                                    Rectangle               aAreaRect,
                                    ControlState            nState,
                                    const ImplControlValue& aValue,
                                    SalControlHandle&       rControlHandle,
                                    OUString                aCaption )
{
    Rectangle       aIndicatorRect;
    GtkRequisition *pIndicatorSize = NULL;
    GtkBorder      *pIndicatorSpacing = NULL;
    gint            width = 13; // GTK+ default
    gint            height = 13;    // GTK+ default
    gint            right = 5;  // GTK+ default

    NWEnsureGTKOptionMenu();

    gtk_widget_style_get( gOptionMenuWidget,
            "indicator_size",   &pIndicatorSize,
            "indicator_spacing",&pIndicatorSpacing, NULL);

    if ( pIndicatorSize )
    {
        width = pIndicatorSize->width;
        height = pIndicatorSize->height;
    }

    if ( pIndicatorSpacing )
        right = pIndicatorSpacing->right;

    aIndicatorRect.SetSize( Size( width, height ) );
    aIndicatorRect.SetPos( Point( aAreaRect.Left() + aAreaRect.GetWidth() - width - right - gOptionMenuWidget->style->xthickness,
                                  aAreaRect.Top() + ((aAreaRect.GetHeight() - height) / 2) ) );

    // If height is odd, move the indicator down 1 pixel
    if ( aIndicatorRect.GetHeight() % 2 )
        aIndicatorRect.Move( 0, 1 );

    if ( pIndicatorSize )
        g_free( pIndicatorSize );
    if ( pIndicatorSpacing )
        g_free( pIndicatorSpacing );

    return( aIndicatorRect );
}

static Rectangle NWGetToolbarRect(  ControlType             nType,
                                    ControlPart             nPart,
                                    Rectangle               aAreaRect,
                                    ControlState            nState,
                                    const ImplControlValue& aValue,
                                    SalControlHandle&       rControlHandle,
                                    const OUString&         rCaption )
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

        NWEnsureGTKToolbar();

        gint nMinWidth =
            2*gToolbarButtonWidget->style->xthickness
            + 1 // CHILD_SPACING constant, found in gtk_button.c
            + 3*gToolbarButtonWidget->style->xthickness; // Murphy factor
        gint nMinHeight =
            2*gToolbarButtonWidget->style->ythickness
            + 1 // CHILD_SPACING constant, found in gtk_button.c
            + 3*gToolbarButtonWidget->style->ythickness; // Murphy factor

        gtk_widget_ensure_style( gToolbarButtonWidget );
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
    fprintf( stderr, "   %s = 0x%2x 0x%2x 0x%2x\n",
             name,
             rCol.red >> 8, rCol.green >> 8, rCol.blue >> 8 );
}

void printStyleColors( GtkStyle* pStyle )
{
    static const char* pStates[] = { "NORMAL", "ACTIVE", "PRELIGHT", "SELECTED", "INSENSITIVE" };

    for( int i = 0; i < 5; i++ )
    {
        fprintf( stderr, "state %s colors:\n", pStates[i] );
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

void GtkSalGraphics::updateSettings( AllSettings& rSettings )
{
    // get the widgets in place
    NWEnsureGTKMenu();
    NWEnsureGTKMenubar();
    NWEnsureGTKScrollbars();

    gtk_widget_ensure_style( m_pWindow );
    GtkStyle* pStyle = gtk_widget_get_style( m_pWindow );

    StyleSettings aStyleSet = rSettings.GetStyleSettings();

#if OSL_DEBUG_LEVEL > 2
    printStyleColors( pStyle );
#endif

    // text colors
    Color aTextColor = getColor( pStyle->text[GTK_STATE_NORMAL] );
    aStyleSet.SetDialogTextColor( aTextColor );
    aStyleSet.SetButtonTextColor( aTextColor );
    aStyleSet.SetRadioCheckTextColor( aTextColor );
    aStyleSet.SetGroupTextColor( aTextColor );
    aStyleSet.SetLabelTextColor( aTextColor );
    aStyleSet.SetInfoTextColor( aTextColor );
    aStyleSet.SetWindowTextColor( aTextColor );
    aStyleSet.SetFieldTextColor( aTextColor );
    aStyleSet.SetHelpTextColor( aTextColor );

    // background colors
    Color aBackColor = getColor( pStyle->bg[GTK_STATE_NORMAL] );
    Color aBackFieldColor = getColor( pStyle->base[ GTK_STATE_NORMAL ] );
    aStyleSet.Set3DColors( aBackColor );
    aStyleSet.SetFaceColor( aBackColor );
    aStyleSet.SetDialogColor( aBackColor );
    aStyleSet.SetWorkspaceColor( aBackColor );
    aStyleSet.SetFieldColor( aBackFieldColor );
    aStyleSet.SetWindowColor( aBackFieldColor );
    aStyleSet.SetHelpColor( aBackColor );
   // ancient wisdom tells us a mystic algorithm how to set checked color
    if( aBackColor == COL_LIGHTGRAY )
        aStyleSet.SetCheckedColor( Color( 0xCC, 0xCC, 0xCC ) );
    else
    {
        Color aColor2 = aStyleSet.GetLightColor();
        Color aCheck( (BYTE)(((USHORT)aBackColor.GetRed()+(USHORT)aColor2.GetRed())/2),
                      (BYTE)(((USHORT)aBackColor.GetGreen()+(USHORT)aColor2.GetGreen())/2),
                      (BYTE)(((USHORT)aBackColor.GetBlue()+(USHORT)aColor2.GetBlue())/2)
                      );
        aStyleSet.SetCheckedColor( aCheck );
    }

    // highlighting colors
    Color aHighlightColor = getColor( pStyle->base[GTK_STATE_SELECTED] );
    Color aHighlightTextColor = getColor( pStyle->text[GTK_STATE_SELECTED] );
    aStyleSet.SetHighlightColor( aHighlightColor );
    aStyleSet.SetHighlightTextColor( aHighlightTextColor );

    // menu colors
    gtk_widget_ensure_style( gMenuWidget );
    gtk_widget_ensure_style( gMenuItemMenuWidget );
    GtkStyle* pMenuStyle = gtk_widget_get_style( gMenuWidget );
    GtkStyle* pMenuItemStyle = gtk_rc_get_style( gMenuItemMenuWidget );
    GtkStyle* pMenubarStyle = gtk_rc_get_style( gMenubarWidget );
    GtkStyle* pMenuTextStyle = gtk_rc_get_style( gtk_bin_get_child( GTK_BIN(gMenuItemMenuWidget) ) );

    aBackColor = getColor( pMenubarStyle->bg[GTK_STATE_NORMAL] );
    aStyleSet.SetMenuBarColor( aBackColor );
    aBackColor = getColor( pMenuStyle->bg[GTK_STATE_NORMAL] );
    aTextColor = getColor( pMenuTextStyle->text[GTK_STATE_NORMAL] );
    if( aBackColor == aTextColor )
        aTextColor = (aBackColor.GetLuminance() < 128) ? Color( COL_WHITE ) : Color( COL_BLACK );
    aStyleSet.SetMenuColor( aBackColor );
    aStyleSet.SetMenuTextColor( aTextColor );

    aHighlightColor = getColor( pMenuItemStyle->bg[ GTK_STATE_SELECTED ] );
    aHighlightTextColor = getColor( pMenuTextStyle->fg[ GTK_STATE_PRELIGHT ] );
    if( aHighlightColor == aHighlightTextColor )
        aHighlightTextColor = (aHighlightColor.GetLuminance() < 128) ? Color( COL_WHITE ) : Color( COL_BLACK );
    aStyleSet.SetMenuHighlightColor( aHighlightColor );
    aStyleSet.SetMenuHighlightTextColor( aHighlightTextColor );

    // UI font
    OString aFamily     = pango_font_description_get_family( pStyle->font_desc );
    int nPixelHeight    = pango_font_description_get_size( pStyle->font_desc )/PANGO_SCALE;
    PangoStyle  eStyle  = pango_font_description_get_style( pStyle->font_desc );
    PangoWeight eWeight = pango_font_description_get_weight( pStyle->font_desc );
    PangoStretch eStretch = pango_font_description_get_stretch( pStyle->font_desc );

    psp::FastPrintFontInfo aInfo;
    // set family name
    aInfo.m_aFamilyName = OStringToOUString( aFamily, RTL_TEXTENCODING_UTF8 );
    // set italic
    switch( eStyle )
    {
        case PANGO_STYLE_NORMAL:    aInfo.m_eItalic = psp::italic::Upright;break;
        case PANGO_STYLE_ITALIC:    aInfo.m_eItalic = psp::italic::Italic;break;
        case PANGO_STYLE_OBLIQUE:   aInfo.m_eItalic = psp::italic::Oblique;break;
    }
    // set weight
    if( eWeight <= PANGO_WEIGHT_ULTRALIGHT )
        aInfo.m_eWeight = psp::weight::UltraLight;
    else if( eWeight <= PANGO_WEIGHT_LIGHT )
        aInfo.m_eWeight = psp::weight::Light;
    else if( eWeight <= PANGO_WEIGHT_NORMAL )
        aInfo.m_eWeight = psp::weight::Normal;
    else if( eWeight <= PANGO_WEIGHT_BOLD )
        aInfo.m_eWeight = psp::weight::Bold;
    else
        aInfo.m_eWeight = psp::weight::UltraBold;
    // set width
    switch( eStretch )
    {
        case PANGO_STRETCH_ULTRA_CONDENSED: aInfo.m_eWidth = psp::width::UltraCondensed;break;
        case PANGO_STRETCH_EXTRA_CONDENSED: aInfo.m_eWidth = psp::width::ExtraCondensed;break;
        case PANGO_STRETCH_CONDENSED:       aInfo.m_eWidth = psp::width::Condensed;break;
        case PANGO_STRETCH_SEMI_CONDENSED:  aInfo.m_eWidth = psp::width::SemiCondensed;break;
        case PANGO_STRETCH_NORMAL:          aInfo.m_eWidth = psp::width::Normal;break;
        case PANGO_STRETCH_SEMI_EXPANDED:   aInfo.m_eWidth = psp::width::SemiExpanded;break;
        case PANGO_STRETCH_EXPANDED:        aInfo.m_eWidth = psp::width::Expanded;break;
        case PANGO_STRETCH_EXTRA_EXPANDED:  aInfo.m_eWidth = psp::width::ExtraExpanded;break;
        case PANGO_STRETCH_ULTRA_EXPANDED:  aInfo.m_eWidth = psp::width::UltraExpanded;break;
    }

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "font name BEFORE system match: \"%s\"\n", aFamily.getStr() );
#endif

    // match font to e.g. resolve "Sans"
    psp::PrintFontManager::get().matchFont( aInfo, rSettings.GetUILocale() );

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "font match %s, name AFTER: \"%s\"\n",
             aInfo.m_nID != 0 ? "succeeded" : "failed",
             OUStringToOString( aInfo.m_aFamilyName, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif

    sal_Int32 nDPIX, nDPIY;
    sal_Int32 nDispDPIY = GetDisplay()->GetResolution().B();
    GetDisplay()->GetScreenFontResolution( nDPIX, nDPIY );
    int nHeight = nPixelHeight * nDispDPIY / nDPIY;
    // allow for rounding in back conversion (at SetFont)
    while( (nHeight * nDPIY / nDispDPIY) > nPixelHeight )
        nHeight--;
    while( (nHeight * nDPIY / nDispDPIY) < nPixelHeight )
        nHeight++;

    Font aFont( aInfo.m_aFamilyName, Size( 0, nHeight ) );
    if( aInfo.m_eWeight != psp::weight::Unknown )
        aFont.SetWeight( PspGraphics::ToFontWeight( aInfo.m_eWeight ) );
    if( aInfo.m_eWidth != psp::width::Unknown )
        aFont.SetWidthType( PspGraphics::ToFontWidth( aInfo.m_eWidth ) );
    if( aInfo.m_eItalic != psp::italic::Unknown )
        aFont.SetItalic( PspGraphics::ToFontItalic( aInfo.m_eItalic ) );
    if( aInfo.m_ePitch != psp::pitch::Unknown )
        aFont.SetPitch( PspGraphics::ToFontPitch( aInfo.m_ePitch ) );

    aStyleSet.SetAppFont( aFont );
    aStyleSet.SetHelpFont( aFont );
    aStyleSet.SetTitleFont( aFont );
    aStyleSet.SetFloatTitleFont( aFont );
    aStyleSet.SetMenuFont( aFont );
    aStyleSet.SetToolFont( aFont );
    aStyleSet.SetLabelFont( aFont );
    aStyleSet.SetInfoFont( aFont );
    aStyleSet.SetRadioCheckFont( aFont );
    aStyleSet.SetPushButtonFont( aFont );
    aStyleSet.SetFieldFont( aFont );
    aStyleSet.SetIconFont( aFont );
    aStyleSet.SetGroupFont( aFont );

    // set scrollbar settings
    gint slider_width = 14;
    gint trough_border = 1;
    gint min_slider_length = 21;

    // Grab some button style attributes
    gtk_widget_style_get( gScrollHorizWidget,
                          "slider-width", &slider_width,
                          "trough-border", &trough_border,
                          "min-slider-length", &min_slider_length,
                          NULL );
    gint magic = trough_border ? 1 : 0;
    aStyleSet.SetScrollBarSize( slider_width + 2*trough_border );
    aStyleSet.SetMinThumbSize( min_slider_length - magic );

    //  FIXME: need some way of fetching toolbar icon size.
//  aStyleSet.SetToolbarIconSize( STYLE_TOOLBAR_ICONSIZE_SMALL );

    // finally update the collected settings
    rSettings.SetStyleSettings( aStyleSet );
}


/************************************************************************
 * Create a GdkPixmap filled with the contents of an area of an Xlib window
 ************************************************************************/

GdkPixmap* GtkSalGraphics::NWGetPixmapFromScreen( Rectangle srcRect )
{
    // Create a new pixmap to hold the composite of the window background and the control
    GdkPixmap * pPixmap     = gdk_pixmap_new( NULL, srcRect.GetWidth(), srcRect.GetHeight(),
                                              GetSalData()->GetDisplay()->GetVisual()->GetDepth() );
    GdkGC *  pPixmapGC  = gdk_gc_new( pPixmap );

    if( !pPixmap || !pPixmapGC )
    {
        if ( pPixmap )
            g_object_unref( pPixmap );
        if ( pPixmapGC )
            g_object_unref( pPixmapGC );
        fprintf( stderr, "salnativewidgets-gtk.cxx: could not get valid pixmap from screen\n" );
        return( NULL );
    }

    // Copy the background of the screen into a composite pixmap
    XCopyArea( GetXDisplay(), GetDrawable(), gdk_x11_drawable_get_xid(pPixmap), gdk_x11_gc_get_xgc(pPixmapGC),
               srcRect.Left(), srcRect.Top(), srcRect.GetWidth(), srcRect.GetHeight(), 0, 0 );

    g_object_unref( pPixmapGC );
    return( pPixmap );
}




/************************************************************************
 * Copy an alpha pixmap to screen using a gc with clipping
 ************************************************************************/

BOOL GtkSalGraphics::NWRenderPixmapToScreen( GdkPixmap* pPixmap, Rectangle dstRect )
{
    // The GC can't be null, otherwise we'd have no clip region
    if( SelectFont() == NULL )
    {
        fprintf(stderr, "salnativewidgets.cxx: no valid GC\n" );
        return( FALSE );
    }

    if ( !pPixmap )
        return( FALSE );

    // Copy the background of the screen into a composite pixmap
    XCopyArea( GetXDisplay(), GDK_DRAWABLE_XID(pPixmap), GetDrawable(), SelectFont(),
               0, 0, dstRect.GetWidth(), dstRect.GetHeight(), dstRect.Left(), dstRect.Top() );

    X11SalGraphics::YieldGraphicsExpose( GetXDisplay(), NULL, GetDrawable() );

    return( TRUE );
}


/************************************************************************
 * State conversion
 ************************************************************************/
static void NWConvertVCLStateToGTKState( ControlState nVCLState,
            GtkStateType* nGTKState, GtkShadowType* nGTKShadow )
{
    *nGTKShadow = GTK_SHADOW_OUT;
    *nGTKState = GTK_STATE_INSENSITIVE;

    if ( nVCLState & CTRL_STATE_ENABLED )
    {
        if ( nVCLState & CTRL_STATE_PRESSED )
        {
            *nGTKState = GTK_STATE_ACTIVE;
            *nGTKShadow = GTK_SHADOW_IN;
        }
        else if ( nVCLState & CTRL_STATE_ROLLOVER )
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
    GTK_WIDGET_SET_FLAGS( widget, gWidgetDefaultFlags[(long)widget] );

    if ( nState & CTRL_STATE_DEFAULT )
        GTK_WIDGET_SET_FLAGS( widget, GTK_HAS_DEFAULT );
    if ( !GTK_IS_TOGGLE_BUTTON(widget) && (nState & CTRL_STATE_FOCUSED) )
        GTK_WIDGET_SET_FLAGS( widget, GTK_HAS_FOCUS );
    if ( nState & CTRL_STATE_ENABLED )
        GTK_WIDGET_SET_FLAGS( widget, GTK_SENSITIVE );
    gtk_widget_set_state( widget, nGtkState );
}

/************************************************************************
 * Widget ensure functions - make sure cached objects are valid
 ************************************************************************/

//-------------------------------------

static void NWAddWidgetToCacheWindow( GtkWidget* widget )
{
    if ( !gCacheWindow || !gDumbContainer )
    {
        if ( !gCacheWindow )
            gCacheWindow = gtk_window_new( GTK_WINDOW_TOPLEVEL );
        if ( !gDumbContainer )
            gDumbContainer = gtk_fixed_new();
        gtk_container_add( GTK_CONTAINER(gCacheWindow), gDumbContainer );
        gtk_widget_realize( gDumbContainer );
        gtk_widget_realize( gCacheWindow );
    }

    gtk_container_add( GTK_CONTAINER(gDumbContainer), widget );
    gtk_widget_realize( widget );
    gtk_widget_ensure_style( widget );

    // Store widget's default flags
    gWidgetDefaultFlags[ (long)widget ] = GTK_WIDGET_FLAGS( widget );
}

//-------------------------------------

static void NWEnsureGTKButton( void )
{
    if ( !gBtnWidget )
    {
        gBtnWidget = gtk_button_new_with_label( "" );
        NWAddWidgetToCacheWindow( gBtnWidget );
    }
}

//-------------------------------------

static void NWEnsureGTKRadio( void )
{
    if ( !gRadioWidget || !gRadioWidgetSibling )
    {
        gRadioWidget = gtk_radio_button_new( NULL );
        gRadioWidgetSibling = gtk_radio_button_new_from_widget( GTK_RADIO_BUTTON(gRadioWidget) );
        NWAddWidgetToCacheWindow( gRadioWidget );
        NWAddWidgetToCacheWindow( gRadioWidgetSibling );
    }
}

//-------------------------------------

static void NWEnsureGTKCheck( void )
{
    if ( !gCheckWidget )
    {
        gCheckWidget = gtk_check_button_new();
        NWAddWidgetToCacheWindow( gCheckWidget );
    }
}

//-------------------------------------

static void NWEnsureGTKScrollbars( void )
{
    if ( !gScrollHorizWidget )
    {
        gScrollHorizWidget = gtk_hscrollbar_new( NULL );
        NWAddWidgetToCacheWindow( gScrollHorizWidget );
    }

    if ( !gScrollVertWidget )
    {
        gScrollVertWidget = gtk_vscrollbar_new( NULL );
        NWAddWidgetToCacheWindow( gScrollVertWidget );
    }
}

//-------------------------------------

static void NWEnsureGTKArrow( void )
{
    if ( !gArrowWidget || !gDropdownWidget )
    {
        gDropdownWidget = gtk_toggle_button_new();
        NWAddWidgetToCacheWindow( gDropdownWidget );
        gArrowWidget = gtk_arrow_new( GTK_ARROW_DOWN, GTK_SHADOW_OUT );
        gtk_container_add( GTK_CONTAINER(gDropdownWidget), gArrowWidget );
        gtk_widget_set_rc_style( gArrowWidget );
        gtk_widget_realize( gArrowWidget );
    }
}

//-------------------------------------

static void NWEnsureGTKEditBox( void )
{
    if ( !gEditBoxWidget )
    {
        gEditBoxWidget = gtk_entry_new();
        NWAddWidgetToCacheWindow( gEditBoxWidget );
    }
}

//-------------------------------------

static void NWEnsureGTKSpinButton( void )
{
    if ( !gSpinButtonWidget )
    {
        GtkAdjustment *adj = GTK_ADJUSTMENT( gtk_adjustment_new(0, 0, 2, 1, 1, 1) );
        gSpinButtonWidget = gtk_spin_button_new( adj, 1, 2 );
        NWAddWidgetToCacheWindow( gSpinButtonWidget );
    }
}

//-------------------------------------

static void NWEnsureGTKNotebook( void )
{
    if ( !gNotebookWidget )
    {
        gNotebookWidget = gtk_notebook_new();
        NWAddWidgetToCacheWindow( gNotebookWidget );
    }
}

//-------------------------------------

static void NWEnsureGTKOptionMenu( void )
{
    if ( !gOptionMenuWidget )
    {
        gOptionMenuWidget = gtk_option_menu_new();
        NWAddWidgetToCacheWindow( gOptionMenuWidget );
    }
}

//-------------------------------------

static void NWEnsureGTKCombo( void )
{
    if ( !gComboWidget )
    {
        gComboWidget = gtk_combo_new();
        NWAddWidgetToCacheWindow( gComboWidget );
        // Must realize the ComboBox's children, since GTK
        // does not do this for us in GtkCombo::gtk_widget_realize()
        gtk_widget_realize( GTK_COMBO(gComboWidget)->button );
        gtk_widget_realize( GTK_COMBO(gComboWidget)->entry );
    }
}

//-------------------------------------

static void NWEnsureGTKScrolledWindow( void )
{
    if ( !gScrolledWindowWidget )
    {
        GtkAdjustment *hadj = GTK_ADJUSTMENT( gtk_adjustment_new(0, 0, 0, 0, 0, 0) );
        GtkAdjustment *vadj = GTK_ADJUSTMENT( gtk_adjustment_new(0, 0, 0, 0, 0, 0) );

        gScrolledWindowWidget = gtk_scrolled_window_new( hadj, vadj );
        NWAddWidgetToCacheWindow( gScrolledWindowWidget );
    }
}

//-------------------------------------

static void NWEnsureGTKToolbar(void)
{
    if( !gToolbarWidget )
    {
        gToolbarWidget = gtk_toolbar_new();
        NWAddWidgetToCacheWindow( gToolbarWidget );
        gToolbarButtonWidget = gtk_button_new();
        gToolbarToggleWidget = gtk_toggle_button_new();

        GtkReliefStyle aRelief = GTK_RELIEF_NORMAL;
        gtk_widget_ensure_style( gToolbarWidget );
        gtk_widget_style_get( gToolbarWidget,
                              "button_relief", &aRelief,
                              NULL);

        gtk_button_set_relief( GTK_BUTTON(gToolbarButtonWidget), aRelief );
        GTK_WIDGET_UNSET_FLAGS( gToolbarButtonWidget, GTK_CAN_FOCUS );
        GTK_WIDGET_UNSET_FLAGS( gToolbarButtonWidget, GTK_CAN_DEFAULT );
        NWAddWidgetToCacheWindow( gToolbarButtonWidget );

        gtk_button_set_relief( GTK_BUTTON(gToolbarToggleWidget), aRelief );
        GTK_WIDGET_UNSET_FLAGS( gToolbarToggleWidget, GTK_CAN_FOCUS );
        GTK_WIDGET_UNSET_FLAGS( gToolbarToggleWidget, GTK_CAN_DEFAULT );
        NWAddWidgetToCacheWindow( gToolbarToggleWidget );
    }
    if( ! gHandleBoxWidget )
    {
        gHandleBoxWidget = gtk_handle_box_new();
        NWAddWidgetToCacheWindow( gHandleBoxWidget );
    }
}

//-------------------------------------

static void NWEnsureGTKMenubar(void)
{
    if( !gMenubarWidget )
    {
        gMenubarWidget = gtk_menu_bar_new();
        gMenuItemMenubarWidget = gtk_menu_item_new_with_label( "b" );
        gtk_menu_shell_append( GTK_MENU_SHELL( gMenubarWidget ), gMenuItemMenubarWidget );
        gtk_widget_show( gMenuItemMenubarWidget );
        NWAddWidgetToCacheWindow( gMenubarWidget );
        gtk_widget_show( gMenubarWidget );

        // do what NWAddWidgetToCacheWindow does except adding to def container
        gtk_widget_realize( gMenuItemMenubarWidget );
        gtk_widget_ensure_style( gMenuItemMenubarWidget );

        gWidgetDefaultFlags[ (long)gMenuItemMenubarWidget ] = GTK_WIDGET_FLAGS( gMenuItemMenubarWidget );
    }
}

static void NWEnsureGTKMenu(void)
{
    if( !gMenuWidget )
    {
        gMenuWidget         = gtk_menu_new();
        gMenuItemMenuWidget = gtk_menu_item_new_with_label( "b" );
        gtk_menu_shell_append( GTK_MENU_SHELL( gMenuWidget ), gMenuItemMenuWidget );

        // do what NWAddWidgetToCacheWindow does except adding to def container
        gtk_widget_realize( gMenuWidget );
        gtk_widget_ensure_style( gMenuWidget );

        gtk_widget_realize( gMenuItemMenuWidget );
        gtk_widget_ensure_style( gMenuItemMenuWidget );

        gWidgetDefaultFlags[ (long)gMenuWidget ] = GTK_WIDGET_FLAGS( gMenuWidget );
        gWidgetDefaultFlags[ (long)gMenuItemMenuWidget ] = GTK_WIDGET_FLAGS( gMenuItemMenuWidget );
    }
}

static void NWEnsureGTKTooltip(void)
{
    if( !gTooltipPopup )
    {
        gTooltipPopup = gtk_window_new (GTK_WINDOW_POPUP);
        gtk_widget_set_name (gTooltipPopup, "gtk-tooltips");
        gtk_widget_realize( gTooltipPopup );
        gtk_widget_ensure_style( gTooltipPopup );
    }
}
