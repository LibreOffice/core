/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *        Caolán McNamara <caolanm@redhat.com> (Red Hat, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Caolán McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

// Gross inclusion hacks for now ...

#include "../../headless/svpgdi.cxx"
#include <unx/gtk/gtkframe.hxx>
#include <unx/gtk/gtkdata.hxx>
#include <unx/gtk/gtkinst.hxx>
#include <unx/gtk/gtkgdi.hxx>

/************************************************************************
 * State conversion
 ************************************************************************/
static void NWConvertVCLStateToGTKState( ControlState nVCLState,
            GtkStateFlags* nGTKState, GtkShadowType* nGTKShadow )
{
    *nGTKShadow = GTK_SHADOW_OUT;
    *nGTKState = GTK_STATE_FLAG_INSENSITIVE;

    if ( nVCLState & CTRL_STATE_ENABLED )
    {
        if ( nVCLState & CTRL_STATE_PRESSED )
        {
            *nGTKState = GTK_STATE_FLAG_ACTIVE;
            *nGTKShadow = GTK_SHADOW_IN;
        }
        else if ( nVCLState & CTRL_STATE_ROLLOVER )
        {
            *nGTKState = GTK_STATE_FLAG_PRELIGHT;
            *nGTKShadow = GTK_SHADOW_OUT;
        }
        else
        {
            *nGTKState = GTK_STATE_FLAG_NORMAL;
            *nGTKShadow = GTK_SHADOW_OUT;
        }
    }
}


/**
 * Draw a GtkStyleContext in the given area, gtk_render_frame and
 * gtk_render_background are called. At the end of the method, the
 * cairo surface is drawn in the real winet dow area.
 *
 * @param style the GtkStyleContext you want to draw
 * @param flags the state of the context
 * @param rControlRegion the real region you want to draw
 * @param render_background wether we must clean the area first or not
 * (may be disabled for optimization).
 **/
void GtkSalGraphics::drawStyleContext( GtkStyleContext* style, GtkStateFlags flags,
                                       const Rectangle& rControlRegion, bool render_background )
{
    /* Set up the cairo context and surface to draw the widgets. */
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
            rControlRegion.GetWidth() + 2, /* make it a bit larger because some
                                            * themes doesn't like drawing directly
                                            * on the corners (for blur methods for instance). */
            rControlRegion.GetHeight() + 2);
    cairo_t* cr = cairo_create(surface);
    cairo_rectangle_t rect;
    rect.x = rControlRegion.Left() - 1;
    rect.y = rControlRegion.Top() - 1;
    rect.height = rControlRegion.GetHeight() + 2;
    rect.width = rControlRegion.GetWidth() + 2;

    if(render_background)
        gtk_render_background(gtk_widget_get_style_context(mpWindow), cr, 0, 0, rControlRegion.GetWidth() + 2, rControlRegion.GetHeight() + 2);

    gtk_style_context_set_state(style, flags);

    gtk_render_background(style, cr,
            1, 1,
            rControlRegion.GetWidth(), rControlRegion.GetHeight());
    gtk_render_frame(style, cr,
            1, 1,
            rControlRegion.GetWidth(), rControlRegion.GetHeight());

    renderAreaToPix(cr, &rect);
    cairo_destroy(cr);
}

sal_Bool GtkSalGraphics::drawNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                            ControlState nState, const ImplControlValue&,
                                            const rtl::OUString& )
{
    GtkStateFlags flags;
    GtkShadowType shadow;
    NWConvertVCLStateToGTKState(nState, &flags, &shadow);

    switch(nType)
    {
    case CTRL_EDITBOX:
        drawStyleContext(mpEntryStyle, flags, rControlRegion);
        return sal_True;
    case CTRL_TOOLBAR:
        switch(nPart)
        {
        case PART_DRAW_BACKGROUND_HORZ:
            drawStyleContext(mpToolbarStyle, flags, rControlRegion);
            return sal_True;
        case PART_BUTTON:
            drawStyleContext(mpToolButtonStyle, flags, rControlRegion);
            return sal_True;
        }
        break;
    case CTRL_PUSHBUTTON:
        drawStyleContext(mpButtonStyle, flags, rControlRegion);
        return sal_True;
    case CTRL_SCROLLBAR:
        drawStyleContext(mpScrollbarStyle, flags, rControlRegion);
        return sal_True;
    }
    return sal_False;
}

// FIXME: This is incredibly lame ... but so is cairo's insistance on -exactly-
// its own stride - neither more nor less - particularly not more aligned
// we like 8byte aligned, it likes 4 - most odd.
void GtkSalGraphics::renderAreaToPix( cairo_t *cr, cairo_rectangle_t *area )
{
    if( !mpFrame->m_aFrame.get() )
        return;



    basebmp::RawMemorySharedArray data = mpFrame->m_aFrame->getBuffer();
    basegfx::B2IVector size = mpFrame->m_aFrame->getSize();
    sal_Int32 nStride = mpFrame->m_aFrame->getScanlineStride();
    long ax = area->x;
    long ay = area->y;
    long awidth = area->width;
    long aheight = area->height;

    /* Get tje cairo surface and the data */
    cairo_surface_t* surface = cairo_get_target(cr);
    g_assert(surface != NULL);
    cairo_surface_flush(surface);
    unsigned char* cairo_data = cairo_image_surface_get_data(surface);
    g_assert(cairo_data != NULL);
    int cairo_stride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, area->width);

    unsigned char *src = data.get();
    src += (int)ay * nStride + (int)ax * 3;

    for (int y = 0; y < size.getY(); ++y)
    {
        for (int x = 0; x < awidth && y < aheight; ++x)
        {
            double alpha = ((float)cairo_data[x*4 + 3])/255.0;
            src[x*3 + 0] = src[x*3 + 0] * (1.0 - alpha) + cairo_data[x*4+0] * alpha;
            src[x*3 + 1] = src[x*3 + 1] * (1.0 - alpha) + cairo_data[x*4+1] * alpha;
            src[x*3 + 2] = src[x*3 + 2] * (1.0 - alpha) + cairo_data[x*4+2] * alpha;
        }
        src += nStride;
        cairo_data += cairo_stride;
    }
}
sal_Bool GtkSalGraphics::getNativeControlRegion( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion, ControlState,
                                                const ImplControlValue&, const rtl::OUString&,
                                                Rectangle &rNativeBoundingRegion, Rectangle &rNativeContentRegion )
{
    /* TODO: all this funcions needs improvements */
    Rectangle aEditRect = rControlRegion;
    GtkWidget* widget;
    GtkRequisition aReq;
    long nHeight;
    if( nPart == PART_ENTIRE_CONTROL)
    {
        switch(nType)
        {
        case CTRL_PUSHBUTTON:
            widget = gtk_button_new(); /* FIXME: too slow */

            gtk_widget_size_request( widget, &aReq );
            nHeight = MAX(aEditRect.GetHeight(), aReq.height+1);
            aEditRect = Rectangle( aEditRect.TopLeft(),
                                   Size( aEditRect.GetWidth(), nHeight ) );
            rNativeBoundingRegion = aEditRect;
            rNativeContentRegion = rNativeBoundingRegion;
            gtk_widget_destroy(widget);
            return sal_True;
        case CTRL_EDITBOX:
            widget = gtk_entry_new(); /* FIXME: too slow, must be cached */

            gtk_widget_size_request( widget, &aReq );
            nHeight = MAX(aEditRect.GetHeight(), aReq.height+1);
            aEditRect = Rectangle( aEditRect.TopLeft(),
                                   Size( aEditRect.GetWidth(), nHeight ) );
            rNativeBoundingRegion = aEditRect;
            rNativeContentRegion = rNativeBoundingRegion;
            gtk_widget_destroy(widget);
            return sal_True;
        default:
            return sal_False;
        }
    }
    return sal_False;
}
/************************************************************************
 * helper for GtkSalFrame
 ************************************************************************/
static inline ::Color getColor( const GdkRGBA& rCol )
{
    return ::Color( (int)(rCol.red * 0xFFFF) >> 8, (int)(rCol.green * 0xFFFF) >> 8, (int)(rCol.blue * 0xFFFF) >> 8 );
}

void GtkSalGraphics::updateSettings( AllSettings& rSettings )
{
    GdkScreen* pScreen = gtk_widget_get_screen( mpWindow );
    GtkStyleContext* pStyle = gtk_widget_get_style_context( mpWindow );
    GtkSettings* pSettings = gtk_widget_get_settings( mpWindow );
    StyleSettings aStyleSet = rSettings.GetStyleSettings();

    // text colors
    GdkRGBA text_color;
    gtk_style_context_get_color(pStyle, GTK_STATE_FLAG_NORMAL, &text_color);
    ::Color aTextColor = getColor( text_color );
    aStyleSet.SetDialogTextColor( aTextColor );
    aStyleSet.SetButtonTextColor( aTextColor );
    aStyleSet.SetRadioCheckTextColor( aTextColor );
    aStyleSet.SetGroupTextColor( aTextColor );
    aStyleSet.SetLabelTextColor( aTextColor );
    aStyleSet.SetInfoTextColor( aTextColor );
    aStyleSet.SetWindowTextColor( aTextColor );
    aStyleSet.SetFieldTextColor( aTextColor );
#if 0
    // Tooltip colors
    GtkStyle* pTooltipStyle = gtk_widget_get_style( gWidgetData[m_nScreen].gTooltipPopup );
    aTextColor = getColor( pTooltipStyle->fg[ GTK_STATE_NORMAL ] );
    aStyleSet.SetHelpTextColor( aTextColor );

    // mouse over text colors
    aTextColor = getColor( pStyle->fg[ GTK_STATE_PRELIGHT ] );
    aStyleSet.SetButtonRolloverTextColor( aTextColor );
    aStyleSet.SetFieldRolloverTextColor( aTextColor );
#endif
    // background colors
    GdkRGBA background_color;
    gtk_style_context_get_background_color(pStyle, GTK_STATE_FLAG_NORMAL, &background_color);
    ::Color aBackColor = getColor( background_color );
    ::Color aBackFieldColor = getColor( background_color  );
    aStyleSet.Set3DColors( aBackColor );
    aStyleSet.SetFaceColor( aBackColor );
    aStyleSet.SetDialogColor( aBackColor );
    aStyleSet.SetWorkspaceColor( aBackColor );
    aStyleSet.SetFieldColor( aBackFieldColor );
    aStyleSet.SetWindowColor( aBackFieldColor );
    aStyleSet.SetCheckedColorSpecialCase( );
    // highlighting colors
    gtk_style_context_get_background_color(pStyle, GTK_STATE_FLAG_SELECTED, &text_color);
    ::Color aHighlightColor = getColor( text_color );
    gtk_style_context_get_color(pStyle, GTK_STATE_FLAG_SELECTED, &text_color);
    ::Color aHighlightTextColor = getColor( text_color );
    aStyleSet.SetHighlightColor( aHighlightColor );
    aStyleSet.SetHighlightTextColor( aHighlightTextColor );

#if 0
        // hyperlink colors
        gtk_widget_style_get (m_pWindow, "link-color", &text_color, NULL);
            aStyleSet.SetLinkColor(getColor(*text_color));
        gtk_widget_style_get (m_pWindow, "visited-link-color", &link_color, NULL);
            aStyleSet.SetVisitedLinkColor(getColor(*link_color));
#endif
#if 0

    // Tab colors
    aStyleSet.SetActiveTabColor( aBackFieldColor ); // same as the window color.
    Color aSelectedBackColor = getColor( pStyle->bg[GTK_STATE_ACTIVE] );
    aStyleSet.SetInactiveTabColor( aSelectedBackColor );

    // menu disabled entries handling
    aStyleSet.SetSkipDisabledInMenus( sal_True );
    aStyleSet.SetAcceleratorsInContextMenus( sal_False );
    // menu colors
    GtkStyle* pMenuStyle = gtk_widget_get_style( gWidgetData[m_nScreen].gMenuWidget );
    GtkStyle* pMenuItemStyle = gtk_rc_get_style( gWidgetData[m_nScreen].gMenuItemMenuWidget );
    GtkStyle* pMenubarStyle = gtk_rc_get_style( gWidgetData[m_nScreen].gMenubarWidget );
    GtkStyle* pMenuTextStyle = gtk_rc_get_style( gtk_bin_get_child( GTK_BIN(gWidgetData[m_nScreen].gMenuItemMenuWidget) ) );

    aBackColor = getColor( pMenubarStyle->bg[GTK_STATE_NORMAL] );
    aStyleSet.SetMenuBarColor( aBackColor );
    aBackColor = getColor( pMenuStyle->bg[GTK_STATE_NORMAL] );
    aTextColor = getColor( pMenuTextStyle->fg[GTK_STATE_NORMAL] );
    aStyleSet.SetMenuColor( aBackColor );
    aStyleSet.SetMenuTextColor( aTextColor );

    aTextColor = getColor( pMenubarStyle->fg[GTK_STATE_NORMAL] );
    aStyleSet.SetMenuBarTextColor( aTextColor );

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
    aHighlightTextColor = getColor( pMenuTextStyle->fg[ GTK_STATE_PRELIGHT ] );
    if( aHighlightColor == aHighlightTextColor )
        aHighlightTextColor = (aHighlightColor.GetLuminance() < 128) ? Color( COL_WHITE ) : Color( COL_BLACK );
    aStyleSet.SetMenuHighlightColor( aHighlightColor );
    aStyleSet.SetMenuHighlightTextColor( aHighlightTextColor );
#endif
    // UI font
    const PangoFontDescription* font = gtk_style_context_get_font(pStyle, GTK_STATE_FLAG_NORMAL);
    OString    aFamily        = pango_font_description_get_family( font );
    int nPangoHeight    = pango_font_description_get_size( font );
    PangoStyle    eStyle    = pango_font_description_get_style( font );
    PangoWeight    eWeight    = pango_font_description_get_weight( font );
    PangoStretch eStretch = pango_font_description_get_stretch( font );

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
    aInfo.m_eWidth = WIDTH_ULTRA_CONDENSED;

    std::fprintf( stderr, "font name BEFORE system match: \"%s\"\n", aFamily.getStr() );
    // match font to e.g. resolve "Sans"
    psp::PrintFontManager::get().matchFont( aInfo, rSettings.GetUILocale() );
    std::fprintf( stderr, "font match %s, name AFTER: \"%s\"\n",
             aInfo.m_nID != 0 ? "succeeded" : "failed",
             OUStringToOString( aInfo.m_aStyleName, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );

    int nPointHeight = 0;
    /*sal_Int32 nDispDPIY = GetDisplay()->GetResolution().B();
    static gboolean(*pAbso)(const PangoFontDescription*) =
        (gboolean(*)(const PangoFontDescription*))osl_getAsciiFunctionSymbol( GetSalData()->m_pPlugin, "pango_font_description_get_size_is_absolute" );

    if( pAbso && pAbso( font ) )
        nPointHeight = (nPangoHeight * 72 + nDispDPIY*PANGO_SCALE/2) / (nDispDPIY * PANGO_SCALE);
    else*/
        nPointHeight = nPangoHeight/PANGO_SCALE;

    Font aFont( aInfo.m_aFamilyName, Size( 0, nPointHeight ) );
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
    aStyleSet.SetGroupFont( aFont );

    aFont.SetWeight( WEIGHT_BOLD );
    aStyleSet.SetTitleFont( aFont );
    aStyleSet.SetFloatTitleFont( aFont );
    // get cursor blink time
    gboolean blink = false;

    g_object_get( pSettings, "gtk-cursor-blink", &blink, (char *)NULL );
    if( blink )
    {
        gint blink_time = STYLE_CURSOR_NOBLINKTIME;
        g_object_get( pSettings, "gtk-cursor-blink-time", &blink_time, (char *)NULL );
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
                  (char *)NULL );
    aMouseSettings.SetDoubleClickTime( iDoubleClickTime );
    aMouseSettings.SetDoubleClickWidth( iDoubleClickDistance );
    aMouseSettings.SetDoubleClickHeight( iDoubleClickDistance );
    aMouseSettings.SetStartDragWidth( iDragThreshold );
    aMouseSettings.SetStartDragHeight( iDragThreshold );
    aMouseSettings.SetMenuDelay( iMenuPopupDelay );
    rSettings.SetMouseSettings( aMouseSettings );

    gboolean showmenuicons = true;
    g_object_get( pSettings, "gtk-menu-images", &showmenuicons, (char *)NULL );
    aStyleSet.SetPreferredUseImagesInMenus( showmenuicons );
#if 0
    // set scrollbar settings
    gint slider_width = 14;
    gint trough_border = 1;
    gint min_slider_length = 21;

    // Grab some button style attributes
    gtk_widget_style_get( gWidgetData[m_nScreen].gScrollHorizWidget,
                          "slider-width", &slider_width,
                          "trough-border", &trough_border,
                          "min-slider-length", &min_slider_length,
                          (char *)NULL );
    gint magic = trough_border ? 1 : 0;
    aStyleSet.SetScrollBarSize( slider_width + 2*trough_border );
    aStyleSet.SetMinThumbSize( min_slider_length - magic );
#endif

    // preferred icon style
    gchar* pIconThemeName = NULL;
    g_object_get( pSettings, "gtk-icon-theme-name", &pIconThemeName, (char *)NULL );
    aStyleSet.SetPreferredSymbolsStyleName( OUString::createFromAscii( pIconThemeName ) );
    g_free( pIconThemeName );

    aStyleSet.SetToolbarIconSize( STYLE_TOOLBAR_ICONSIZE_LARGE );

    const cairo_font_options_t* pNewOptions = NULL;
    static cairo_font_options_t* (*gdk_screen_get_font_options)(GdkScreen*) =
        (cairo_font_options_t*(*)(GdkScreen*))osl_getAsciiFunctionSymbol( GetSalData()->m_pPlugin, "gdk_screen_get_font_options" );
    if( gdk_screen_get_font_options != NULL )
        pNewOptions = gdk_screen_get_font_options( pScreen );
    aStyleSet.SetCairoFontOptions( pNewOptions );
    // finally update the collected settings
    rSettings.SetStyleSettings( aStyleSet );
    gchar* pThemeName = NULL;
    g_object_get( pSettings, "gtk-theme-name", &pThemeName, (char *)NULL );
    #if OSL_DEBUG_LEVEL > 1
    std::fprintf( stderr, "Theme name is \"%s\"\n", pThemeName );
    #endif


}

sal_Bool GtkSalGraphics::IsNativeControlSupported( ControlType nType, ControlPart nPart )
{
    if((nType == CTRL_PUSHBUTTON && nPart == PART_ENTIRE_CONTROL) || nType == CTRL_SCROLLBAR || nType == CTRL_EDITBOX /*||
            segfault with recent code, needs investigating nType == CTRL_TOOLBAR*/ )
        return sal_True;
    return sal_False;
}

void GtkData::initNWF() {}
void GtkData::deInitNWF() {}

void GtkSalGraphics::getStyleContext(GtkStyleContext** style, GtkWidget* widget)
{
    *style = gtk_widget_get_style_context(widget);
    g_object_ref(*style);
    gtk_widget_destroy(widget);
}

GtkSalGraphics::GtkSalGraphics( GtkSalFrame *pFrame, GtkWidget *pWindow )
    : SvpSalGraphics(),
      mpFrame( pFrame ),
      mpWindow( pWindow )
{

    gtk_init(NULL, NULL);
    /* Load the GtkStyleContexts, it might be a bit slow, but usually,
     * gtk apps create a lot of widgets at startup, so, it shouldn't be
     * too slow */
    GtkWidget* toolbar = gtk_toolbar_new();
    GtkWidget* toolbutton = gtk_button_new();
    //gtk_container_add(GTK_CONTAINER(toolbar), GTK_WIDGET(toolbutton));
    getStyleContext(&mpEntryStyle, gtk_entry_new());
    getStyleContext(&mpButtonStyle, gtk_button_new());
    getStyleContext(&mpToolbarStyle, toolbar);
    gtk_style_context_add_class(mpToolbarStyle, "primary-toolbar");
    getStyleContext(&mpToolButtonStyle, GTK_WIDGET(toolbutton));
    gtk_style_context_add_class(mpToolButtonStyle, "button"); /* TODO */
    getStyleContext(&mpScrollbarStyle, gtk_vscrollbar_new(NULL));
}


static void print_cairo_region (cairo_region_t *region, const char *msg)
{
    if (!region) {
        fprintf (stderr, "%s - NULL\n", msg);
        return;
    }
    int numrect = cairo_region_num_rectangles (region);
    fprintf (stderr, "%s - %d rects\n", msg, numrect);
    for (int i = 0; i < numrect; i++) {
        cairo_rectangle_int_t rect;
        cairo_region_get_rectangle (region, i, &rect);
        fprintf( stderr, "\t%d -> %d,%d %dx%d\n", i,
                 rect.x, rect.y, rect.width, rect.height );
    }
}

static void print_update_area (GdkWindow *window, const char *msg)
{
    print_cairo_region (gdk_window_get_update_area (window), msg);
}

void GtkSalGraphics::copyArea( long nDestX, long nDestY,
                               long nSrcX, long nSrcY,
                               long nSrcWidth, long nSrcHeight,
                               sal_uInt16 nFlags )
{
    return;
    mpFrame->pushIgnoreDamage();
    SvpSalGraphics::copyArea( nDestX, nDestY, nSrcX, nSrcY, nSrcWidth, nSrcHeight, nFlags );
    mpFrame->popIgnoreDamage();
    cairo_rectangle_int_t rect = { (int)nSrcX, (int)nSrcY, (int)nSrcWidth, (int)nSrcHeight };
    cairo_region_t *region = cairo_region_create_rectangle( &rect );

    print_update_area( gtk_widget_get_window( mpFrame->getWindow() ), "before copy area" );

//    print_cairo_region( mpFrame->m_pRegion, "extremely odd SalFrame: shape combine region! - ");

    g_warning( "FIXME: copy area delta: %d %d needs clip intersect\n",
               (int)(nDestX - nSrcX), (int)(nDestY - nSrcY) );

    // get clip region and translate it in the opposite direction & intersect ...
    cairo_region_t *clip_region;

    if( m_aClipRegion.GetRectCount() <= 0)
    {
        basegfx::B2IVector aSize = GetSize();
        cairo_rectangle_int_t aCairoSize = { 0, 0, aSize.getX(), aSize.getY() };
        clip_region = cairo_region_create_rectangle( &aCairoSize );
    }
    else
    {
        clip_region = cairo_region_create();
        Rectangle aClipRect;
        RegionHandle aHnd = m_aClipRegion.BeginEnumRects();
        while( m_aClipRegion.GetNextEnumRect( aHnd, aClipRect ) )
        {
            cairo_rectangle_int_t aRect = { (int)aClipRect.Left(), (int)aClipRect.Top(),
                                            (int)aClipRect.Right(), (int)aClipRect.Bottom() };
            cairo_region_union_rectangle( clip_region, &aRect );
        }
        m_aClipRegion.EndEnumRects (aHnd);
    }
    print_cairo_region( clip_region, "pristine clip region" );
    cairo_region_translate( clip_region, - (nDestX - nSrcX), - (nDestY - nSrcY) );
    print_cairo_region( clip_region, "translated clip region" );
    cairo_region_intersect( region, clip_region );
    print_cairo_region( region, "reduced copy area region" );

    // FIXME: this will queue (duplicate) gtk+ re-rendering for the exposed area, c'est la vie
    gdk_window_move_region( gtk_widget_get_window( mpFrame->getWindow() ),
                            region, nDestX - nSrcX, nDestY - nSrcY );

    print_update_area( gtk_widget_get_window( mpFrame->getWindow() ), "after copy area" );
    cairo_region_destroy( clip_region );
    cairo_region_destroy( region );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
