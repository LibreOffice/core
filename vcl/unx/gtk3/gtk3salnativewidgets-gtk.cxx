/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <basegfx/range/b2ibox.hxx>
#include <unx/gtk/gtkframe.hxx>
#include <unx/gtk/gtkdata.hxx>
#include <unx/gtk/gtkinst.hxx>
#include <unx/gtk/gtkgdi.hxx>
#include <vcl/decoview.hxx>
#include <vcl/settings.hxx>
#include "fontmanager.hxx"
#include "cairo_gtk3_cairo.hxx"

#include <boost/optional.hpp>

GtkStyleContext* GtkSalGraphics::mpButtonStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpLinkButtonStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpEntryStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpTextViewStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpVScrollbarStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpVScrollbarTroughStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpVScrollbarSliderStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpVScrollbarButtonStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpHScrollbarStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpHScrollbarTroughStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpHScrollbarSliderStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpHScrollbarButtonStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpToolbarStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpToolButtonStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpToolbarSeperatorStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpCheckButtonStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpRadioButtonStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpMenuBarStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpMenuBarItemStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpMenuStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpMenuItemStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpMenuItemArrowStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpCheckMenuItemStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpRadioMenuItemStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpSeparatorMenuItemStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpSpinStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpSpinUpStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpSpinDownStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpComboboxStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpComboboxButtonStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpListboxStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpListboxButtonStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpFrameInStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpFrameOutStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpFixedHoriLineStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpFixedVertLineStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpTreeHeaderButtonStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpProgressBarStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpProgressBarTroughStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpProgressBarProgressStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpNotebookStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpNotebookStackStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpNotebookHeaderStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpNotebookHeaderTabsStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpNotebookHeaderTabsTabStyle = nullptr;

bool GtkSalGraphics::style_loaded = false;
/************************************************************************
 * State conversion
 ************************************************************************/
static GtkStateFlags NWConvertVCLStateToGTKState(ControlState nVCLState)
{
    GtkStateFlags nGTKState = GTK_STATE_FLAG_NORMAL;

    if (!( nVCLState & ControlState::ENABLED ))
    {
        nGTKState = GTK_STATE_FLAG_INSENSITIVE;
    }

    if ( nVCLState & ControlState::PRESSED )
    {
        nGTKState = (GtkStateFlags) (nGTKState | GTK_STATE_FLAG_ACTIVE);
    }

    if ( nVCLState & ControlState::ROLLOVER )
    {
        nGTKState = (GtkStateFlags) (nGTKState | GTK_STATE_FLAG_PRELIGHT);
    }

    if ( nVCLState & ControlState::SELECTED )
        nGTKState = (GtkStateFlags) (nGTKState | GTK_STATE_FLAG_SELECTED);

    if ( nVCLState & ControlState::FOCUSED )
        nGTKState = (GtkStateFlags) (nGTKState | GTK_STATE_FLAG_FOCUSED);

    if (AllSettings::GetLayoutRTL())
    {
        nGTKState = (GtkStateFlags) (nGTKState | GTK_STATE_FLAG_DIR_RTL);
    }
    else
    {
        nGTKState = (GtkStateFlags) (nGTKState | GTK_STATE_FLAG_DIR_LTR);
    }

    return nGTKState;
}

enum {
    RENDER_BACKGROUND_AND_FRAME = 1,
    RENDER_CHECK = 2,
    RENDER_BACKGROUND = 3,
    RENDER_MENU_SEPERATOR = 4,
    RENDER_TOOLBAR_SEPERATOR = 5,
    RENDER_SEPERATOR = 6,
    RENDER_ARROW = 7,
    RENDER_RADIO = 8,
    RENDER_SCROLLBAR = 9,
    RENDER_SPINBUTTON = 10,
    RENDER_COMBOBOX = 11,
    RENDER_EXPANDER = 12,
    RENDER_ICON = 13,
    RENDER_PROGRESS = 14,
    RENDER_FOCUS = 15,
};

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

Rectangle GtkSalGraphics::NWGetSpinButtonRect( ControlPart nPart, Rectangle aAreaRect)
{
    gint w, h;
    gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &w, &h);
    gint icon_size = std::max(w, h);

    GtkBorder padding, border;
    gtk_style_context_get_padding(mpSpinUpStyle, gtk_style_context_get_state(mpSpinUpStyle), &padding);
    gtk_style_context_get_border(mpSpinUpStyle, gtk_style_context_get_state(mpSpinUpStyle), &border);

    gint buttonWidth = icon_size + padding.left + padding.right +
        border.left + border.right;

    gint buttonHeight = icon_size + padding.top + padding.bottom +
        border.top + border.bottom;

    Rectangle buttonRect;
    buttonRect.SetSize(Size(buttonWidth, buttonHeight));
    buttonRect.setY(aAreaRect.Top());
    buttonRect.Bottom() = buttonRect.Top() + aAreaRect.GetHeight();
    Rectangle partRect(buttonRect);
    if ( nPart == PART_BUTTON_UP )
    {
        if (AllSettings::GetLayoutRTL())
            partRect.setX(aAreaRect.Left());
        else
            partRect.setX(aAreaRect.Left() + (aAreaRect.GetWidth() - buttonRect.GetWidth()));
    }
    else if( nPart == PART_BUTTON_DOWN )
    {
        if (AllSettings::GetLayoutRTL())
            partRect.setX(aAreaRect.Left() + buttonRect.GetWidth());
        else
            partRect.setX(aAreaRect.Left() + (aAreaRect.GetWidth() - 2 * buttonRect.GetWidth()));
    }
    else
    {
        if (AllSettings::GetLayoutRTL())
        {
            partRect.Right() = aAreaRect.Left() + aAreaRect.GetWidth();
            partRect.Left()  = aAreaRect.Left() + (2 * buttonRect.GetWidth()) - 1;
        }
        else
        {
            partRect.Right() = (aAreaRect.Left() + (aAreaRect.GetWidth() - 2 * buttonRect.GetWidth())) - 1;
            partRect.Left()  = aAreaRect.Left();
        }
        partRect.Top()    = aAreaRect.Top();
        partRect.Bottom() = aAreaRect.Bottom();
    }

    return partRect;
}

Rectangle GtkSalGraphics::NWGetScrollButtonRect( ControlPart nPart, Rectangle aAreaRect )
{
    GtkStyleContext* pScrollbarStyle = nullptr;
    if ((nPart == PART_BUTTON_LEFT) || (nPart == PART_BUTTON_RIGHT))
        pScrollbarStyle = mpHScrollbarStyle;
    else // (nPart == PART_BUTTON_UP) || (nPart == PART_BUTTON_DOWN)
        pScrollbarStyle = mpVScrollbarStyle;

    gint slider_width;
    gint stepper_size;
    gint stepper_spacing;
    gint trough_border;

    // Grab some button style attributes
    gtk_style_context_get_style( pScrollbarStyle,
                                 "slider-width", &slider_width,
                                 "stepper-size", &stepper_size,
                                 "trough-border", &trough_border,
                                 "stepper-spacing", &stepper_spacing, nullptr );

    gboolean has_forward;
    gboolean has_forward2;
    gboolean has_backward;
    gboolean has_backward2;

    gtk_style_context_get_style( pScrollbarStyle,
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

static GtkWidget* gCacheWindow;
static GtkWidget* gDumbContainer;
static GtkWidget* gSpinBox;
static GtkWidget* gEntryBox;
static GtkWidget* gComboBox;
static GtkWidget* gListBox;
static GtkWidget* gMenuBarWidget;
static GtkWidget* gMenuItemMenuBarWidget;
static GtkWidget* gCheckMenuItemWidget;
static GtkWidget* gTreeViewWidget;

void GtkSalGraphics::PaintScrollbar(GtkStyleContext *context,
                                    cairo_t *cr,
                                    const Rectangle& rControlRectangle,
                                    ControlType nType,
                                    ControlPart nPart,
                                    const ImplControlValue& rValue )
{
    (void)nType;
    OSL_ASSERT( rValue.getType() == CTRL_SCROLLBAR );
    const ScrollbarValue& rScrollbarVal = static_cast<const ScrollbarValue&>(rValue);
    Rectangle        scrollbarRect;
    GtkStateFlags    stateFlags;
    GtkOrientation    scrollbarOrientation;
    Rectangle        thumbRect = rScrollbarVal.maThumbRect;
    Rectangle        button11BoundRect = rScrollbarVal.maButton1Rect;   // backward
    Rectangle        button22BoundRect = rScrollbarVal.maButton2Rect;   // forward
    Rectangle        button12BoundRect = rScrollbarVal.maButton1Rect;   // secondary forward
    Rectangle        button21BoundRect = rScrollbarVal.maButton2Rect;   // secondary backward
    gdouble          arrow1Angle;                                        // backward
    gdouble          arrow2Angle;                                        // forward
    Rectangle        arrowRect;
    gint            slider_width = 0;
    gint            stepper_size = 0;
    gint            stepper_spacing = 0;
    gint            trough_border = 0;
    gint            min_slider_length = 0;

    // make controlvalue rectangles relative to area
    thumbRect.Move( -rControlRectangle.Left(), -rControlRectangle.Top() );
    button11BoundRect.Move( -rControlRectangle.Left(), -rControlRectangle.Top() );
    button22BoundRect.Move( -rControlRectangle.Left(), -rControlRectangle.Top() );
    button12BoundRect.Move( -rControlRectangle.Left(), -rControlRectangle.Top() );
    button21BoundRect.Move( -rControlRectangle.Left(), -rControlRectangle.Top() );

    // Find the overall bounding rect of the control
    scrollbarRect = rControlRectangle;
    scrollbarRect.SetSize( Size( scrollbarRect.GetWidth() + 1,
                                 scrollbarRect.GetHeight() + 1 ) );

    if ( (scrollbarRect.GetWidth() <= 1) || (scrollbarRect.GetHeight() <= 1) )
        return;

    // Grab some button style attributes
    gtk_style_context_get_style( context,
                                 "slider_width", &slider_width,
                                 "stepper_size", &stepper_size,
                                 "trough_border", &trough_border,
                                 "stepper_spacing", &stepper_spacing,
                                 "min_slider_length", &min_slider_length, nullptr );
    gboolean has_forward;
    gboolean has_forward2;
    gboolean has_backward;
    gboolean has_backward2;

    gtk_style_context_get_style( context,
                                 "has-forward-stepper", &has_forward,
                                 "has-secondary-forward-stepper", &has_forward2,
                                 "has-backward-stepper", &has_backward,
                                 "has-secondary-backward-stepper", &has_backward2, nullptr );
    gint magic = trough_border ? 1 : 0;
    gint slider_side = slider_width + (trough_border * 2);

    if ( nPart == PART_DRAW_BACKGROUND_HORZ )
    {
        scrollbarRect.Move( 0, (scrollbarRect.GetHeight() - slider_side) / 2 );
        scrollbarRect.SetSize( Size( scrollbarRect.GetWidth(), slider_side ) );

        scrollbarOrientation = GTK_ORIENTATION_HORIZONTAL;
        arrow1Angle = G_PI * 3 / 2;
        arrow2Angle = G_PI / 2;

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
        scrollbarRect.Move( (scrollbarRect.GetWidth() - slider_side) / 2, 0 );
        scrollbarRect.SetSize( Size( slider_side, scrollbarRect.GetHeight() ) );

        scrollbarOrientation = GTK_ORIENTATION_VERTICAL;
        arrow1Angle = 0;
        arrow2Angle = G_PI;

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

    gtk_render_background(gtk_widget_get_style_context(gCacheWindow), cr, 0, 0,
                          scrollbarRect.GetWidth(), scrollbarRect.GetHeight() );

    // ----------------- TROUGH
    GtkStyleContext* pScrollbarTroughStyle = scrollbarOrientation == GTK_ORIENTATION_VERTICAL ?
                                              mpVScrollbarTroughStyle : mpHScrollbarTroughStyle;
    gtk_render_background(pScrollbarTroughStyle, cr, 0, 0,
                          scrollbarRect.GetWidth(), scrollbarRect.GetHeight() );
    gtk_render_frame(pScrollbarTroughStyle, cr, 0, 0,
                     scrollbarRect.GetWidth(), scrollbarRect.GetHeight() );

    // ----------------- THUMB
    if ( has_slider )
    {
        stateFlags = NWConvertVCLStateToGTKState(rScrollbarVal.mnThumbState);
        if ( rScrollbarVal.mnThumbState & ControlState::PRESSED )
            stateFlags = (GtkStateFlags) (stateFlags | GTK_STATE_PRELIGHT);

        GtkStyleContext* pScrollbarSliderStyle = scrollbarOrientation == GTK_ORIENTATION_VERTICAL ?
                                                  mpVScrollbarSliderStyle : mpHScrollbarSliderStyle;

        gtk_style_context_set_state(pScrollbarSliderStyle, stateFlags);

        GtkBorder margin;
        gtk_style_context_get_margin(pScrollbarSliderStyle, stateFlags, &margin);


        gtk_render_slider(pScrollbarSliderStyle, cr,
                          thumbRect.Left() + margin.left, thumbRect.Top() + margin.top,
                          thumbRect.GetWidth() - margin.left - margin.right,
                          thumbRect.GetHeight() - margin.top - margin.bottom,
                          scrollbarOrientation);
    }

    bool backwardButtonInsensitive =
        rScrollbarVal.mnCur == rScrollbarVal.mnMin;
    bool forwardButtonInsensitive = rScrollbarVal.mnMax == 0 ||
        rScrollbarVal.mnCur + rScrollbarVal.mnVisibleSize >= rScrollbarVal.mnMax;

    // ----------------- BUTTON 1
    if ( has_backward )
    {
        stateFlags = NWConvertVCLStateToGTKState(rScrollbarVal.mnButton1State);
        if ( backwardButtonInsensitive )
            stateFlags = GTK_STATE_FLAG_INSENSITIVE;

        GtkStyleContext* pScrollbarButtonStyle = scrollbarOrientation == GTK_ORIENTATION_VERTICAL ?
                                                 mpVScrollbarButtonStyle : mpHScrollbarButtonStyle;

        gtk_style_context_set_state(pScrollbarButtonStyle, stateFlags);

        gtk_render_background(pScrollbarButtonStyle, cr,
                              button11BoundRect.Left(), button11BoundRect.Top(),
                              button11BoundRect.GetWidth(), button11BoundRect.GetHeight() );
        gtk_render_frame(pScrollbarButtonStyle, cr,
                         button11BoundRect.Left(), button11BoundRect.Top(),
                         button11BoundRect.GetWidth(), button11BoundRect.GetHeight() );

        // ----------------- ARROW 1
        NWCalcArrowRect( button11BoundRect, arrowRect );
        gtk_render_arrow(pScrollbarButtonStyle, cr,
                         arrow1Angle,
                         arrowRect.Left(), arrowRect.Top(),
                         MIN(arrowRect.GetWidth(), arrowRect.GetHeight()) );
    }
    if ( has_forward2 )
    {
        stateFlags = NWConvertVCLStateToGTKState(rScrollbarVal.mnButton2State);
        if ( forwardButtonInsensitive )
            stateFlags = GTK_STATE_FLAG_INSENSITIVE;

        GtkStyleContext* pScrollbarButtonStyle = scrollbarOrientation == GTK_ORIENTATION_VERTICAL ?
                                                 mpVScrollbarButtonStyle : mpHScrollbarButtonStyle;

        gtk_style_context_set_state(pScrollbarButtonStyle, stateFlags);

        gtk_render_background(pScrollbarButtonStyle, cr,
                              button12BoundRect.Left(), button12BoundRect.Top(),
                              button12BoundRect.GetWidth(), button12BoundRect.GetHeight() );
        gtk_render_frame(pScrollbarButtonStyle, cr,
                         button12BoundRect.Left(), button12BoundRect.Top(),
                         button12BoundRect.GetWidth(), button12BoundRect.GetHeight() );

        // ----------------- ARROW 1
        NWCalcArrowRect( button12BoundRect, arrowRect );
        gtk_render_arrow(pScrollbarButtonStyle, cr,
                         arrow2Angle,
                         arrowRect.Left(), arrowRect.Top(),
                         MIN(arrowRect.GetWidth(), arrowRect.GetHeight()) );
    }
    // ----------------- BUTTON 2
    if ( has_backward2 )
    {
        stateFlags = NWConvertVCLStateToGTKState(rScrollbarVal.mnButton1State);
        if ( backwardButtonInsensitive )
            stateFlags = GTK_STATE_FLAG_INSENSITIVE;

        GtkStyleContext* pScrollbarButtonStyle = scrollbarOrientation == GTK_ORIENTATION_VERTICAL ?
                                                 mpVScrollbarButtonStyle : mpHScrollbarButtonStyle;

        gtk_style_context_set_state(pScrollbarButtonStyle, stateFlags);

        gtk_render_background(pScrollbarButtonStyle, cr,
                              button21BoundRect.Left(), button21BoundRect.Top(),
                              button21BoundRect.GetWidth(), button21BoundRect.GetHeight() );
        gtk_render_frame(pScrollbarButtonStyle, cr,
                         button21BoundRect.Left(), button21BoundRect.Top(),
                         button21BoundRect.GetWidth(), button21BoundRect.GetHeight() );

        // ----------------- ARROW 2
        NWCalcArrowRect( button21BoundRect, arrowRect );
        gtk_render_arrow(pScrollbarButtonStyle, cr,
                         arrow1Angle,
                         arrowRect.Left(), arrowRect.Top(),
                         MIN(arrowRect.GetWidth(), arrowRect.GetHeight()) );
    }
    if ( has_forward )
    {
        stateFlags = NWConvertVCLStateToGTKState(rScrollbarVal.mnButton2State);
        if ( forwardButtonInsensitive )
            stateFlags = GTK_STATE_FLAG_INSENSITIVE;

        GtkStyleContext* pScrollbarButtonStyle = scrollbarOrientation == GTK_ORIENTATION_VERTICAL ?
                                                 mpVScrollbarButtonStyle : mpHScrollbarButtonStyle;

        gtk_style_context_set_state(pScrollbarButtonStyle, stateFlags);

        gtk_render_background(pScrollbarButtonStyle, cr,
                       button22BoundRect.Left(), button22BoundRect.Top(),
                       button22BoundRect.GetWidth(), button22BoundRect.GetHeight() );
        gtk_render_frame(pScrollbarButtonStyle, cr,
                       button22BoundRect.Left(), button22BoundRect.Top(),
                       button22BoundRect.GetWidth(), button22BoundRect.GetHeight() );

        // ----------------- ARROW 2
        NWCalcArrowRect( button22BoundRect, arrowRect );
        gtk_render_arrow(pScrollbarButtonStyle, cr,
                         arrow2Angle,
                         arrowRect.Left(), arrowRect.Top(),
                         MIN(arrowRect.GetWidth(), arrowRect.GetHeight()) );
    }
}

void GtkSalGraphics::PaintOneSpinButton( GtkStyleContext *context,
                                         cairo_t *cr,
                                         ControlType nType,
                                         ControlPart nPart,
                                         Rectangle aAreaRect,
                                         ControlState nState )
{
    (void)nType;
    GtkBorder            padding, border;

    GtkStateFlags stateFlags = NWConvertVCLStateToGTKState(nState);
    Rectangle buttonRect = NWGetSpinButtonRect( nPart, aAreaRect );

    gtk_style_context_set_state(context, stateFlags);

    gtk_style_context_get_padding(context, gtk_style_context_get_state(context), &padding);
    gtk_style_context_get_border(context, gtk_style_context_get_state(context), &border);

    gtk_render_background(context, cr,
                          buttonRect.Left(), buttonRect.Top(),
                          buttonRect.GetWidth(), buttonRect.GetHeight() );
    gtk_render_frame(context, cr,
                     buttonRect.Left(), buttonRect.Top(),
                     buttonRect.GetWidth(), buttonRect.GetHeight() );

    gint iconWidth = (buttonRect.GetWidth() - padding.left - padding.right - border.left - border.right);
    gint iconHeight = (buttonRect.GetHeight() - padding.top - padding.bottom - border.top - border.bottom);

    const char* icon = (nPart == PART_BUTTON_UP) ? "list-add-symbolic" : "list-remove-symbolic";
    GtkIconTheme *pIconTheme = gtk_icon_theme_get_for_screen(gtk_widget_get_screen(mpWindow));

    GtkIconInfo *info = gtk_icon_theme_lookup_icon(pIconTheme, icon, std::min(iconWidth, iconHeight),
                                                   static_cast<GtkIconLookupFlags>(0));

    GdkPixbuf *pixbuf = gtk_icon_info_load_symbolic_for_context
                                                        (info,
                                                         context,
                                                         nullptr,
                                                         nullptr);
    iconWidth = gdk_pixbuf_get_width(pixbuf);
    iconHeight = gdk_pixbuf_get_height(pixbuf);
    Rectangle arrowRect;
    arrowRect.SetSize(Size(iconWidth, iconHeight));
    arrowRect.setX( buttonRect.Left() + (buttonRect.GetWidth() - arrowRect.GetWidth()) / 2 );
    arrowRect.setY( buttonRect.Top() + (buttonRect.GetHeight() - arrowRect.GetHeight()) / 2 );

    gtk_render_icon(context, cr, pixbuf, arrowRect.Left(), arrowRect.Top());
    g_object_unref(pixbuf);
    gtk_icon_info_free(info);
}

void GtkSalGraphics::PaintSpinButton(GtkStyleContext *context,
                                     cairo_t *cr,
                                     const Rectangle& rControlRectangle,
                                     ControlType nType,
                                     ControlPart nPart,
                                     const ImplControlValue& rValue )
{
    const SpinbuttonValue *pSpinVal = (rValue.getType() == CTRL_SPINBUTTONS) ? static_cast<const SpinbuttonValue *>(&rValue) : nullptr;
    ControlPart upBtnPart = PART_BUTTON_UP;
    ControlState upBtnState = ControlState::NONE;
    ControlPart downBtnPart = PART_BUTTON_DOWN;
    ControlState downBtnState = ControlState::NONE;

    if ( pSpinVal )
    {
        upBtnPart = pSpinVal->mnUpperPart;
        upBtnState = pSpinVal->mnUpperState;

        downBtnPart = pSpinVal->mnLowerPart;
        downBtnState = pSpinVal->mnLowerState;
    }

    if (nPart == PART_ENTIRE_CONTROL)
    {
        gtk_render_background(context, cr,
                              0, 0,
                              rControlRectangle.GetWidth(), rControlRectangle.GetHeight() );
        gtk_render_frame(context, cr,
                         0, 0,
                         rControlRectangle.GetWidth(), rControlRectangle.GetHeight() );
    }

    cairo_translate(cr, -rControlRectangle.Left(), -rControlRectangle.Top());
    PaintOneSpinButton(mpSpinUpStyle, cr, nType, upBtnPart, rControlRectangle, upBtnState );
    PaintOneSpinButton(mpSpinDownStyle, cr, nType, downBtnPart, rControlRectangle, downBtnState );
    cairo_translate(cr, rControlRectangle.Left(), rControlRectangle.Top());
}

#define ARROW_SIZE 11 * 0.85
Rectangle GtkSalGraphics::NWGetComboBoxButtonRect( ControlType nType,
                                                   ControlPart nPart,
                                                   Rectangle aAreaRect )
{
    (void)nType;
    (void)nPart;
    Rectangle    aButtonRect;

    GtkBorder padding;
    gtk_style_context_get_padding( mpButtonStyle, gtk_style_context_get_state(mpButtonStyle), &padding);

    gint nArrowWidth = ARROW_SIZE;
    gint nButtonWidth = nArrowWidth + padding.left + padding.right;
    if( nPart == PART_BUTTON_DOWN )
    {
        Point aPos = Point(aAreaRect.Left() + aAreaRect.GetWidth() - nButtonWidth, aAreaRect.Top());
        if (AllSettings::GetLayoutRTL())
            aPos.X() = aAreaRect.Left();
        aButtonRect.SetSize( Size( nButtonWidth, aAreaRect.GetHeight() ) );
        aButtonRect.SetPos(aPos);
    }
    else if( nPart == PART_SUB_EDIT )
    {
        gint adjust_left = padding.left;
        gint adjust_top = padding.top;
        gint adjust_right = padding.right;
        gint adjust_bottom = padding.bottom;

        aButtonRect.SetSize( Size( aAreaRect.GetWidth() - nButtonWidth - (adjust_left + adjust_right),
                                   aAreaRect.GetHeight() - (adjust_top + adjust_bottom)) );
        Point aEditPos = aAreaRect.TopLeft();
        if (AllSettings::GetLayoutRTL())
            aEditPos.X() += nButtonWidth;
        else
            aEditPos.X() += adjust_left;
        aEditPos.Y() += adjust_top;
        aButtonRect.SetPos( aEditPos );
    }

    return aButtonRect;
}

void GtkSalGraphics::PaintCombobox( GtkStateFlags flags, cairo_t *cr,
                                    const Rectangle& rControlRectangle,
                                    ControlType nType,
                                    ControlPart nPart,
                                    const ImplControlValue& /*rValue*/ )
{
    Rectangle        areaRect;
    Rectangle        buttonRect;
    Rectangle        arrowRect;

    // Find the overall bounding rect of the buttons's drawing area,
    // plus its actual draw rect excluding adornment
    areaRect = rControlRectangle;

    buttonRect = NWGetComboBoxButtonRect( nType, PART_BUTTON_DOWN, areaRect );
    if( nPart == PART_BUTTON_DOWN )
        buttonRect.Left() += 1;

    Rectangle        aEditBoxRect( areaRect );
    aEditBoxRect.SetSize( Size( areaRect.GetWidth() - buttonRect.GetWidth(), aEditBoxRect.GetHeight() ) );
    if (AllSettings::GetLayoutRTL())
        aEditBoxRect.SetPos( Point( areaRect.Left() + buttonRect.GetWidth(), areaRect.Top() ) );

    arrowRect.SetSize( Size( (gint)(ARROW_SIZE),
                             (gint)(ARROW_SIZE) ) );
    arrowRect.SetPos( Point( buttonRect.Left() + (gint)((buttonRect.GetWidth() - arrowRect.GetWidth()) / 2),
                             buttonRect.Top() + (gint)((buttonRect.GetHeight() - arrowRect.GetHeight()) / 2) ) );


    if ( nType == CTRL_COMBOBOX )
    {
        gtk_style_context_save(mpComboboxButtonStyle);
        gtk_style_context_set_state(mpComboboxButtonStyle, flags);

        if( nPart == PART_ENTIRE_CONTROL )
        {
            GtkJunctionSides eJuncSides = gtk_style_context_get_junction_sides(mpEntryStyle);
            gtk_style_context_set_state(mpEntryStyle, flags);
            if (AllSettings::GetLayoutRTL())
                gtk_style_context_set_junction_sides(mpEntryStyle, GTK_JUNCTION_LEFT);
            else
                gtk_style_context_set_junction_sides(mpEntryStyle, GTK_JUNCTION_RIGHT);

            gtk_render_background(mpComboboxStyle, cr,
                                  0, 0,
                                  areaRect.GetWidth(), areaRect.GetHeight());
            gtk_render_frame(mpComboboxStyle, cr,
                             0, 0,
                             areaRect.GetWidth(), areaRect.GetHeight());
            gtk_render_background(mpEntryStyle, cr,
                                  (aEditBoxRect.Left() - areaRect.Left()),
                                  (aEditBoxRect.Top() - areaRect.Top()),
                                  aEditBoxRect.GetWidth(), aEditBoxRect.GetHeight() );
            gtk_render_frame(mpEntryStyle, cr,
                             (aEditBoxRect.Left() - areaRect.Left()),
                             (aEditBoxRect.Top() - areaRect.Top()),
                             aEditBoxRect.GetWidth(), aEditBoxRect.GetHeight() );
            gtk_style_context_set_junction_sides(mpEntryStyle, eJuncSides);
        }

        gtk_render_background(mpComboboxButtonStyle, cr,
                              (buttonRect.Left() - areaRect.Left()),
                              (buttonRect.Top() - areaRect.Top()),
                              buttonRect.GetWidth(), buttonRect.GetHeight() );
        gtk_render_frame(mpComboboxButtonStyle, cr,
                         (buttonRect.Left() - areaRect.Left()),
                         (buttonRect.Top() - areaRect.Top()),
                         buttonRect.GetWidth(), buttonRect.GetHeight() );

        gtk_render_arrow(mpComboboxStyle, cr,
                         G_PI,
                         (arrowRect.Left() - areaRect.Left()), (arrowRect.Top() - areaRect.Top()),
                         arrowRect.GetWidth() );

        gtk_style_context_restore(mpComboboxButtonStyle);
    }
    else if (nType == CTRL_LISTBOX)
    {
        if( nPart == PART_WINDOW )
        {
            /* render the popup window with the menu style */
            gtk_render_frame(mpMenuStyle, cr,
                             0, 0,
                             areaRect.GetWidth(), areaRect.GetHeight());
        }
        else
        {
            gtk_style_context_save(mpListboxButtonStyle);
            gtk_style_context_set_state(mpListboxButtonStyle, flags);

            gtk_render_background(mpListboxStyle, cr,
                                  0, 0,
                                  areaRect.GetWidth(), areaRect.GetHeight());
            gtk_render_frame(mpListboxStyle, cr,
                             0, 0,
                             areaRect.GetWidth(), areaRect.GetHeight());

            gtk_render_background(mpListboxButtonStyle, cr,
                                  0, 0,
                                  areaRect.GetWidth(), areaRect.GetHeight());
            gtk_render_frame(mpListboxButtonStyle, cr,
                             0, 0,
                             areaRect.GetWidth(), areaRect.GetHeight());

            gtk_render_arrow(mpListboxStyle, cr,
                             G_PI,
                             (arrowRect.Left() - areaRect.Left()), (arrowRect.Top() - areaRect.Top()),
                             arrowRect.GetWidth() );

            gtk_style_context_restore(mpListboxButtonStyle);
        }
    }
}

static GtkStyleContext* createStyleContext(GtkControlPart ePart, GtkStyleContext* parent = nullptr)
{
    GtkWidgetPath *path = parent ? gtk_widget_path_copy(gtk_style_context_get_path(parent)) : gtk_widget_path_new();
    switch (ePart)
    {
        case GtkControlPart::Button:
            gtk_widget_path_append_type(path, GTK_TYPE_BUTTON);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "button");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_BUTTON);
#endif
            break;
        case GtkControlPart::LinkButton:
            gtk_widget_path_append_type(path, GTK_TYPE_BUTTON);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "button");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_BUTTON);
#endif
            gtk_widget_path_iter_add_class(path, -1, "link");
            break;
        case GtkControlPart::CheckButton:
            gtk_widget_path_append_type(path, GTK_TYPE_CHECK_BUTTON);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "checkbutton");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_CHECK);
#endif
            break;
        case GtkControlPart::CheckButtonCheck:
            gtk_widget_path_append_type(path, GTK_TYPE_CHECK_BUTTON);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "check");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_CHECK);
#endif
            break;
        case GtkControlPart::RadioButton:
            gtk_widget_path_append_type(path, GTK_TYPE_RADIO_BUTTON);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "radiobutton");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_RADIO);
#endif
            break;
        case GtkControlPart::RadioButtonRadio:
            gtk_widget_path_append_type(path, GTK_TYPE_RADIO_BUTTON);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "radio");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_RADIO);
#endif
        break;
    case GtkControlPart::Entry:
            gtk_widget_path_append_type(path, GTK_TYPE_ENTRY);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "entry");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_ENTRY);
#endif
            break;
        case GtkControlPart::SpinButton:
            gtk_widget_path_append_type(path, GTK_TYPE_SPIN_BUTTON);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "spinbutton");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_SPINBUTTON);
#endif
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_HORIZONTAL);
            break;
        case GtkControlPart::SpinButtonUpButton:
        case GtkControlPart::SpinButtonDownButton:
            gtk_widget_path_append_type(path, GTK_TYPE_SPIN_BUTTON);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "button");
            gtk_widget_path_iter_add_class(path, -1, ePart == GtkControlPart::SpinButtonUpButton ? "up" : "down");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_SPINBUTTON);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_BUTTON);
#endif
            break;
        case GtkControlPart::ScrollbarVertical:
        case GtkControlPart::ScrollbarHorizontal:
            gtk_widget_path_append_type(path, GTK_TYPE_SCROLLBAR);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "scrollbar");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_SCROLLBAR);
#endif
            gtk_widget_path_iter_add_class(path, -1, ePart == GtkControlPart::ScrollbarVertical ? "vertical" : "horizontal");
            break;
        case GtkControlPart::ScrollbarTrough:
            gtk_widget_path_append_type(path, GTK_TYPE_SCROLLBAR);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "trough");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_SCROLLBAR);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_TROUGH);
#endif
            break;
        case GtkControlPart::ScrollbarSlider:
            gtk_widget_path_append_type(path, GTK_TYPE_SCROLLBAR);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "slider");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_SCROLLBAR);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_SLIDER);
#endif
            break;
        case GtkControlPart::ScrollbarButton:
            gtk_widget_path_append_type(path, GTK_TYPE_SCROLLBAR);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "button");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_SCROLLBAR);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_BUTTON);
#endif
            break;
        case GtkControlPart::ProgressBar:
            gtk_widget_path_append_type(path, GTK_TYPE_PROGRESS_BAR);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "progressbar");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_PROGRESSBAR);
#endif
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_HORIZONTAL);
            break;
        case GtkControlPart::ProgressBarTrough:
            gtk_widget_path_append_type(path, GTK_TYPE_PROGRESS_BAR);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "trough");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_PROGRESSBAR);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_TROUGH);
#endif
            break;
        case GtkControlPart::ProgressBarProgress:
            gtk_widget_path_append_type(path, GTK_TYPE_PROGRESS_BAR);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "progress");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_PROGRESSBAR);
#endif
            break;
        case GtkControlPart::MenuBar:
            gtk_widget_path_append_type(path, GTK_TYPE_MENU_BAR);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "menubar");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_MENUBAR);
#endif
            break;
        case GtkControlPart::MenuItem:
            gtk_widget_path_append_type(path, GTK_TYPE_MENU_ITEM);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "menuitem");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_MENUITEM);
#endif
            break;
        case GtkControlPart::MenuItemArrow:
            gtk_widget_path_append_type(path, GTK_TYPE_MENU_ITEM);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "arrow");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_ARROW);
#endif
            break;
        case GtkControlPart::Menu:
            gtk_widget_path_append_type(path, GTK_TYPE_MENU);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "menu");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_MENU);
#endif
            break;
        case GtkControlPart::CheckMenuItem:
            gtk_widget_path_append_type(path, GTK_TYPE_CHECK_MENU_ITEM);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "menuitem");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_MENUITEM);
#endif
            break;
        case GtkControlPart::CheckMenuItemCheck:
            gtk_widget_path_append_type(path, GTK_TYPE_CHECK_MENU_ITEM);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "check");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_CHECK);
#endif
            break;
        case GtkControlPart::RadioMenuItem:
            gtk_widget_path_append_type(path, GTK_TYPE_RADIO_MENU_ITEM);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "menuitem");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_MENUITEM);
#endif
            break;
        case GtkControlPart::RadioMenuItemRadio:
            gtk_widget_path_append_type(path, GTK_TYPE_RADIO_MENU_ITEM);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "radio");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_RADIO);
#endif
            break;
        case GtkControlPart::SeparatorMenuItem:
            gtk_widget_path_append_type(path, GTK_TYPE_SEPARATOR_MENU_ITEM);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "menuitem");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_MENUITEM);
#endif
            break;
        case GtkControlPart::SeparatorMenuItemSeparator:
            gtk_widget_path_append_type(path, GTK_TYPE_SEPARATOR_MENU_ITEM);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "separator");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_SEPARATOR);
#endif
            break;
        case GtkControlPart::Notebook:
            gtk_widget_path_append_type(path, GTK_TYPE_NOTEBOOK);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "notebook");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_NOTEBOOK);
#endif
            gtk_widget_path_iter_add_class(path, -1, "frame");
            break;
        case GtkControlPart::NotebookStack:
            gtk_widget_path_append_type(path, GTK_TYPE_NOTEBOOK);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "stack");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_NOTEBOOK);
#endif
            break;
        case GtkControlPart::NotebookHeader:
            gtk_widget_path_append_type(path, GTK_TYPE_NOTEBOOK);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "header");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_HEADER);
#endif
            gtk_widget_path_iter_add_class(path, -1, "frame");
            gtk_widget_path_iter_add_class(path, -1, "top");
            break;
        case GtkControlPart::NotebookHeaderTabs:
            gtk_widget_path_append_type(path, GTK_TYPE_NOTEBOOK);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "tabs");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_HEADER);
#endif
            gtk_widget_path_iter_add_class(path, -1, "top");
            break;
        case GtkControlPart::NotebookHeaderTabsTab:
            gtk_widget_path_append_type(path, GTK_TYPE_NOTEBOOK);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "tab");
#else
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_HEADER);
#endif
            gtk_widget_path_iter_add_class(path, -1, "top");
            break;
        case GtkControlPart::FrameBorder:
            gtk_widget_path_append_type(path, GTK_TYPE_FRAME);
#if GTK_CHECK_VERSION(3, 19, 2)
            gtk_widget_path_iter_set_object_name(path, -1, "frame");
#endif
            gtk_widget_path_iter_add_class(path, -1, "frame");
            break;
    }

    GtkStyleContext* context = gtk_style_context_new();
    gtk_style_context_set_path(context, path);
    gtk_style_context_set_parent(context, parent);
    gtk_widget_path_unref (path);

#if !GTK_CHECK_VERSION(3, 19, 2)
    if (ePart == GtkControlPart::NotebookHeaderTabsTab)
    {
        gtk_style_context_add_region(context, GTK_STYLE_REGION_TAB, GTK_REGION_ONLY);
    }
#endif

    return context;
}

#if GTK_CHECK_VERSION(3,13,7)
#   define CHECKED GTK_STATE_FLAG_CHECKED
#else
#   define CHECKED GTK_STATE_FLAG_ACTIVE
#endif

#if GTK_CHECK_VERSION(3,19,11)
#   define ACTIVE_TAB GTK_STATE_FLAG_CHECKED
#else
#   define ACTIVE_TAB GTK_STATE_FLAG_ACTIVE
#endif

void GtkSalGraphics::PaintCheckOrRadio(cairo_t *cr, GtkStyleContext *context,
                                       const Rectangle& rControlRectangle, bool bIsCheck, bool bInMenu)
{
    gint indicator_size;
    gtk_style_context_get_style(context, "indicator-size", &indicator_size, nullptr);

    gint x = (rControlRectangle.GetWidth() - indicator_size) / 2;
    gint y = (rControlRectangle.GetHeight() - indicator_size) / 2;

    if (!bInMenu)
        gtk_render_background(context, cr, x, y, indicator_size, indicator_size);
    gtk_render_frame(context, cr, x, y, indicator_size, indicator_size);

    if (bIsCheck)
        gtk_render_check(context, cr, x, y, indicator_size, indicator_size);
    else
        gtk_render_option(context, cr, x, y, indicator_size, indicator_size);
}

void GtkSalGraphics::PaintCheck(cairo_t *cr, GtkStyleContext *context,
                                const Rectangle& rControlRectangle, bool bInMenu)
{
    PaintCheckOrRadio(cr, context, rControlRectangle, true, bInMenu);
}

void GtkSalGraphics::PaintRadio(cairo_t *cr, GtkStyleContext *context,
                                const Rectangle& rControlRectangle, bool bInMenu)
{
    PaintCheckOrRadio(cr, context, rControlRectangle, false, bInMenu);
}

void parent_styles_context_set_state(GtkStyleContext* context, GtkStateFlags flags)
{
    while ((context = gtk_style_context_get_parent(context)))
    {
        gtk_style_context_set_state(context, flags);
    }
}

static gfloat getArrowSize(GtkStyleContext* context)
{
    gfloat arrow_scaling = 1.0;
    gtk_style_context_get_style(context, "arrow-scaling", &arrow_scaling, nullptr);
    gfloat arrow_size = 11 * arrow_scaling;
    return arrow_size;
}

bool GtkSalGraphics::drawNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                            ControlState nState, const ImplControlValue& rValue,
                                            const OUString& )
{
    gint renderType = nPart == PART_FOCUS ? RENDER_FOCUS : RENDER_BACKGROUND_AND_FRAME;
    GtkStyleContext *context = nullptr;
#if !GTK_CHECK_VERSION(3,19,2)
    const gchar *styleClass = nullptr;
#endif
    GdkPixbuf *pixbuf = nullptr;
    bool bInMenu = false;

    GtkStateFlags flags = NWConvertVCLStateToGTKState(nState);

    switch(nType)
    {
    case CTRL_SPINBOX:
    case CTRL_SPINBUTTONS:
        context = mpEntryStyle;
        renderType = RENDER_SPINBUTTON;
        break;
    case CTRL_EDITBOX:
        context = mpEntryStyle;
        break;
    case CTRL_MULTILINE_EDITBOX:
        context = mpTextViewStyle;
        break;
    case CTRL_COMBOBOX:
        context = mpComboboxStyle;
        renderType = RENDER_COMBOBOX;
        break;
    case CTRL_LISTBOX:
        context = mpListboxStyle;
        renderType = nPart == PART_FOCUS ? RENDER_FOCUS : RENDER_COMBOBOX;
        break;
    case CTRL_MENU_POPUP:
        bInMenu = true;

        // map selected menu entries in vcl parlance to gtk prelight
        if (nPart >= PART_MENU_ITEM && nPart <= PART_MENU_SUBMENU_ARROW && (nState & ControlState::SELECTED))
            flags = (GtkStateFlags) (flags | GTK_STATE_FLAG_PRELIGHT);
        flags = (GtkStateFlags)(flags & ~GTK_STATE_FLAG_ACTIVE);
        switch(nPart)
        {
        case PART_MENU_ITEM:
            context = mpMenuItemStyle;
            renderType = RENDER_BACKGROUND_AND_FRAME;
            break;
        case PART_MENU_ITEM_CHECK_MARK:
#if GTK_CHECK_VERSION(3,19,2)
            context = mpCheckMenuItemStyle;
#else
            context = gtk_widget_get_style_context(gCheckMenuItemWidget);
            styleClass = GTK_STYLE_CLASS_CHECK;
#endif
            renderType = RENDER_CHECK;
            nType = CTRL_CHECKBOX;
            if (nState & ControlState::PRESSED)
            {
                flags = (GtkStateFlags)(flags | CHECKED);
            }
            break;
        case PART_MENU_ITEM_RADIO_MARK:
#if GTK_CHECK_VERSION(3,19,2)
            context = mpRadioMenuItemStyle;
#else
            context = gtk_widget_get_style_context(gCheckMenuItemWidget);
            styleClass = GTK_STYLE_CLASS_RADIO;
#endif
            renderType = RENDER_RADIO;
            nType = CTRL_RADIOBUTTON;
            if (nState & ControlState::PRESSED)
            {
                flags = (GtkStateFlags)(flags | CHECKED);
            }
            break;
        case PART_MENU_SEPARATOR:
            context = mpSeparatorMenuItemStyle;
            flags = (GtkStateFlags)(GTK_STATE_FLAG_BACKDROP | GTK_STATE_FLAG_INSENSITIVE); //GTK_STATE_FLAG_BACKDROP hack ?
            renderType = RENDER_MENU_SEPERATOR;
            break;
        case PART_MENU_SUBMENU_ARROW:
#if GTK_CHECK_VERSION(3,19,2)
            context = mpMenuItemArrowStyle;
#else
            context = gtk_widget_get_style_context(gCheckMenuItemWidget);
            styleClass = GTK_STYLE_CLASS_ARROW;
#endif
            renderType = RENDER_ARROW;
            break;
        case PART_ENTIRE_CONTROL:
            context = mpMenuStyle;
            renderType = RENDER_BACKGROUND;
            break;
        }
        break;
    case CTRL_TOOLBAR:
        switch(nPart)
        {
        case PART_DRAW_BACKGROUND_HORZ:
        case PART_DRAW_BACKGROUND_VERT:
            context = mpToolbarStyle;
            break;
        case PART_BUTTON:
            /* For all checkbuttons in the toolbars */
            flags = (GtkStateFlags)(flags |
                    ( (rValue.getTristateVal() == BUTTONVALUE_ON) ? GTK_STATE_FLAG_ACTIVE : GTK_STATE_FLAG_NORMAL));
            context = mpToolButtonStyle;
            break;
        case PART_SEPARATOR_VERT:
            context = mpToolbarSeperatorStyle;
            renderType = RENDER_TOOLBAR_SEPERATOR;
            break;
        default:
            return false;
        }
        break;
    case CTRL_RADIOBUTTON:
        flags = (GtkStateFlags)(flags |
                ( (rValue.getTristateVal() == BUTTONVALUE_ON) ? CHECKED : GTK_STATE_FLAG_NORMAL));
        context = mpRadioButtonStyle;
        renderType = nPart == PART_FOCUS ? RENDER_FOCUS : RENDER_RADIO;
        break;
    case CTRL_CHECKBOX:
        flags = (GtkStateFlags)(flags |
                ( (rValue.getTristateVal() == BUTTONVALUE_ON) ? CHECKED :
                  (rValue.getTristateVal() == BUTTONVALUE_MIXED) ? GTK_STATE_FLAG_INCONSISTENT :
                  GTK_STATE_FLAG_NORMAL));
        context = mpCheckButtonStyle;
        renderType = nPart == PART_FOCUS ? RENDER_FOCUS : RENDER_CHECK;
        break;
    case CTRL_PUSHBUTTON:
        context = mpButtonStyle;
        break;
    case CTRL_SCROLLBAR:
        switch(nPart)
        {
        case PART_DRAW_BACKGROUND_VERT:
        case PART_DRAW_BACKGROUND_HORZ:
            context = (nPart == PART_DRAW_BACKGROUND_VERT)
                ? mpVScrollbarStyle : mpHScrollbarStyle;
            renderType = RENDER_SCROLLBAR;
            break;
        }
        break;
    case CTRL_LISTNET:
        return true;
        break;
    case CTRL_TAB_PANE:
        context = mpNotebookStyle;
        break;
    case CTRL_TAB_BODY:
        context = mpNotebookStackStyle;
        break;
    case CTRL_TAB_HEADER:
        context = mpNotebookHeaderStyle;
        break;
    case CTRL_TAB_ITEM:
        context = mpNotebookHeaderTabsTabStyle;
        if (nState & ControlState::SELECTED)
            flags = (GtkStateFlags) (flags | ACTIVE_TAB);
        break;
    case CTRL_WINDOW_BACKGROUND:
        context = gtk_widget_get_style_context(mpWindow);
        break;
    case CTRL_FRAME:
    {
        DrawFrameStyle nStyle = static_cast<DrawFrameStyle>(rValue.getNumericVal() & 0x0f);
        if (nStyle == DrawFrameStyle::In)
            context = mpFrameOutStyle;
        else
            context = mpFrameInStyle;
        break;
    }
    case CTRL_MENUBAR:
        if (nPart == PART_MENU_ITEM)
        {
            context = mpMenuBarItemStyle;

            flags = (!(nState & ControlState::ENABLED)) ? GTK_STATE_FLAG_INSENSITIVE : GTK_STATE_FLAG_NORMAL;
            if (nState & ControlState::SELECTED)
                flags = (GtkStateFlags) (flags | GTK_STATE_FLAG_PRELIGHT);
        }
        else
        {
            // context = mpMenuBarStyle; why does this not work
            context = gtk_widget_get_style_context(mpWindow);
        }
        break;
    case CTRL_FIXEDLINE:
        context = nPart == PART_SEPARATOR_HORZ ? mpFixedHoriLineStyle : mpFixedVertLineStyle;
        renderType = RENDER_SEPERATOR;
        break;
    case CTRL_LISTNODE:
    {
        context = mpTreeHeaderButtonStyle;
        ButtonValue aButtonValue = rValue.getTristateVal();
        if (aButtonValue == BUTTONVALUE_ON)
            flags = (GtkStateFlags) (flags | CHECKED);
        renderType = RENDER_EXPANDER;
        break;
    }
    case CTRL_LISTHEADER:
        context = mpTreeHeaderButtonStyle;
        if (nPart == PART_ARROW)
        {
            const char* icon = (rValue.getNumericVal() & 1) ? "pan-down-symbolic" : "pan-up-symbolic";
            GtkIconTheme *pIconTheme = gtk_icon_theme_get_for_screen(gtk_widget_get_screen(mpWindow));
            pixbuf = gtk_icon_theme_load_icon(pIconTheme, icon,
                                              std::max(rControlRegion.GetWidth(), rControlRegion.GetHeight()),
                                              static_cast<GtkIconLookupFlags>(0), nullptr);
            flags = GTK_STATE_FLAG_SELECTED;
            renderType = RENDER_ICON;
        }
        break;
    case CTRL_PROGRESS:
        context = mpProgressBarProgressStyle;
        renderType = RENDER_PROGRESS;
        break;
    default:
        return false;
    }

    cairo_t *cr = getCairoContext();
    clipRegion(cr);
    cairo_translate(cr, rControlRegion.Left(), rControlRegion.Top());

    Rectangle aDamageRect(rControlRegion);

    long nX = 0;
    long nY = 0;
    long nWidth = rControlRegion.GetWidth();
    long nHeight = rControlRegion.GetHeight();

    gtk_style_context_set_state(context, flags);
    parent_styles_context_set_state(context, flags);
    if (nType == CTRL_TAB_ITEM)
    {
        GtkBorder margin;
#if GTK_CHECK_VERSION(3,19,2)
        gtk_style_context_get_margin(mpNotebookHeaderTabsTabStyle, gtk_style_context_get_state(mpNotebookHeaderTabsTabStyle), &margin);
#else
        gint initial_gap(0);
        gtk_style_context_get_style(mpNotebookStyle,
                                "initial-gap", &initial_gap,
                                nullptr);

        margin.left = margin.right = initial_gap/2;
#endif
        nX += margin.left;
        nWidth -= (margin.left + margin.right);
    }

#if !GTK_CHECK_VERSION(3,19,2)
    if (styleClass)
    {
        gtk_style_context_add_class(context, styleClass);
    }
#endif

    switch(renderType)
    {
    case RENDER_BACKGROUND:
    case RENDER_BACKGROUND_AND_FRAME:
        gtk_render_background(context, cr, nX, nY, nWidth, nHeight);
        if (renderType == RENDER_BACKGROUND_AND_FRAME)
        {
            gtk_render_frame(context, cr, nX, nY, nWidth, nHeight);
        }
        break;
    case RENDER_CHECK:
    {
        PaintCheck(cr, context, rControlRegion, bInMenu);
        break;
    }
    case RENDER_RADIO:
    {
        PaintRadio(cr, context, rControlRegion, bInMenu);
        break;
    }
    case RENDER_MENU_SEPERATOR:
        gtk_render_line(context, cr,
                        0, rControlRegion.GetHeight() / 2,
                        rControlRegion.GetWidth() - 1, rControlRegion.GetHeight() / 2);
        break;
    case RENDER_TOOLBAR_SEPERATOR:
        gtk_render_line(context, cr,
                        rControlRegion.GetWidth() / 2, rControlRegion.GetHeight() * 0.2,
                        rControlRegion.GetWidth() / 2, rControlRegion.GetHeight() * 0.8 );
        break;
    case RENDER_SEPERATOR:
        if (nPart == PART_SEPARATOR_HORZ)
            gtk_render_line(context, cr, 0, nHeight / 2, nWidth - 1, nHeight / 2);
        else
            gtk_render_line(context, cr, nWidth / 2, 0, nWidth / 2, nHeight - 1);
        break;
    case RENDER_ARROW:
        gtk_render_arrow(context, cr,
                         G_PI / 2, 0, 0,
                         MIN(rControlRegion.GetWidth(), 1 + rControlRegion.GetHeight()));
        break;
    case RENDER_EXPANDER:
        gtk_render_expander(context, cr, -2, -2, nWidth+4, nHeight+4);
        break;
    case RENDER_SCROLLBAR:
        PaintScrollbar(context, cr, rControlRegion, nType, nPart, rValue);
        break;
    case RENDER_SPINBUTTON:
        PaintSpinButton(context, cr, rControlRegion, nType, nPart, rValue);
        break;
    case RENDER_COMBOBOX:
        PaintCombobox(flags, cr, rControlRegion, nType, nPart, rValue);
        break;
    case RENDER_ICON:
        gtk_render_icon(context, cr, pixbuf, nX, nY);
        g_object_unref(pixbuf);
        break;
    case RENDER_FOCUS:
    {
        if (nType != CTRL_CHECKBOX)
        {
            GtkBorder border;

            gtk_style_context_get_border(context, flags, &border);

            nX += border.left;
            nY += border.top;
            nWidth -= border.left + border.right;
            nHeight -= border.top + border.bottom;
        }

        gtk_render_focus(context, cr, nX, nY, nWidth, nHeight);

        break;
    }
    case RENDER_PROGRESS:
    {
        gtk_render_background(mpProgressBarTroughStyle, cr, nX, nY, nWidth, nHeight);
        gtk_render_frame(mpProgressBarTroughStyle, cr, nX, nY, nWidth, nHeight);
        long nProgressWidth = rValue.getNumericVal();
        if (nProgressWidth)
        {
            GtkBorder padding;
            gtk_style_context_get_padding(context, gtk_style_context_get_state(context), &padding);

            nX += padding.left;
            nY += padding.top;
            nHeight -= (padding.top + padding.bottom);
            nProgressWidth -= (padding.left + padding.right);
            gtk_render_background(context, cr, nX, nY, nProgressWidth, nHeight);
            gtk_render_frame(context, cr, nX, nY, nProgressWidth, nHeight);
        }

        break;
    }
    default:
        break;
    }

#if !GTK_CHECK_VERSION(3,19,2)
    if (styleClass)
    {
        gtk_style_context_remove_class(context, styleClass);
    }
#endif

    cairo_surface_flush(cairo_get_target(cr));
    cairo_destroy(cr); // unref

    if (!aDamageRect.IsEmpty())
        mpFrame->damaged(basegfx::B2IBox(aDamageRect.Left(), aDamageRect.Top(), aDamageRect.Right(), aDamageRect.Bottom()));

    return true;
}

Rectangle GetWidgetSize(const Rectangle& rControlRegion, GtkWidget* widget)
{
    GtkRequisition aReq;
    gtk_widget_get_preferred_size(widget, nullptr, &aReq);
    long nHeight = (rControlRegion.GetHeight() > aReq.height) ? rControlRegion.GetHeight() : aReq.height;
    return Rectangle(rControlRegion.TopLeft(), Size(rControlRegion.GetWidth(), nHeight));
}

Rectangle AdjustRectForTextBordersPadding(GtkStyleContext* pStyle, long nContentWidth, long nContentHeight, const Rectangle& rControlRegion)
{
    GtkBorder border;
    gtk_style_context_get_border(pStyle, gtk_style_context_get_state(pStyle), &border);

    GtkBorder padding;
    gtk_style_context_get_padding(pStyle, gtk_style_context_get_state(pStyle), &padding);

    gint nWidgetHeight = nContentHeight + padding.top + padding.bottom + border.top + border.bottom;
    nWidgetHeight = std::max(std::max<gint>(nWidgetHeight, rControlRegion.GetHeight()), 34);

    gint nWidgetWidth = nContentWidth + padding.left + padding.right + border.left + border.right;
    nWidgetWidth = std::max<gint>(nWidgetWidth, rControlRegion.GetWidth());

    Rectangle aEditRect(rControlRegion.TopLeft(), Size(nWidgetWidth, nWidgetHeight));

    return aEditRect;
}

bool GtkSalGraphics::getNativeControlRegion( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion, ControlState,
                                                const ImplControlValue& rValue, const OUString&,
                                                Rectangle &rNativeBoundingRegion, Rectangle &rNativeContentRegion )
{
    /* TODO: all this functions needs improvements */
    Rectangle aEditRect = rControlRegion;
    gint indicator_size, indicator_spacing, point;

    if(((nType == CTRL_CHECKBOX) || (nType == CTRL_RADIOBUTTON)) &&
       nPart == PART_ENTIRE_CONTROL)
    {
        rNativeBoundingRegion = rControlRegion;

        GtkStyleContext *pButtonStyle = (nType == CTRL_CHECKBOX) ? mpCheckButtonStyle : mpRadioButtonStyle;


        gtk_style_context_get_style( pButtonStyle,
                                     "indicator-size", &indicator_size,
                                     "indicator-spacing", &indicator_spacing,
                                     nullptr );

        GtkBorder border;
        gtk_style_context_get_border(pButtonStyle, gtk_style_context_get_state(pButtonStyle), &border);

        GtkBorder padding;
        gtk_style_context_get_padding(pButtonStyle, gtk_style_context_get_state(pButtonStyle), &padding);


        indicator_size += 2*indicator_spacing + border.left + padding.left + border.right + padding.right;
        Rectangle aIndicatorRect( Point( 0,
                                         (rControlRegion.GetHeight()-indicator_size)/2),
                                  Size( indicator_size, indicator_size ) );
        rNativeContentRegion = aIndicatorRect;

        return true;
    }
    else if( nType == CTRL_MENU_POPUP)
    {
        if (((nPart == PART_MENU_ITEM_CHECK_MARK) ||
              (nPart == PART_MENU_ITEM_RADIO_MARK) ))
        {
            indicator_size = 0;

            GtkStyleContext *pMenuItemStyle = (nType == PART_MENU_ITEM_CHECK_MARK ) ? mpCheckMenuItemStyle : mpRadioMenuItemStyle;

            gtk_style_context_get_style( pMenuItemStyle,
                                         "indicator-size", &indicator_size,
                                         nullptr );

            point = MAX(0, rControlRegion.GetHeight() - indicator_size);
            aEditRect = Rectangle( Point( 0, point / 2),
                                   Size( indicator_size, indicator_size ) );
        }
        else if (nPart == PART_MENU_SEPARATOR)
        {
            gint separator_height, separator_width, wide_separators;

            gtk_style_context_get_style (mpSeparatorMenuItemStyle,
                                         "wide-separators",  &wide_separators,
                                         "separator-width",  &separator_width,
                                         "separator-height", &separator_height,
                                         nullptr);

            aEditRect = Rectangle( aEditRect.TopLeft(),
                                   Size( aEditRect.GetWidth(), wide_separators ? separator_height : 1 ) );
        }
        else if (nPart == PART_MENU_SUBMENU_ARROW)
        {
            gfloat arrow_size = getArrowSize(mpMenuItemArrowStyle);
            aEditRect = Rectangle( aEditRect.TopLeft(),
                                   Size( arrow_size, arrow_size ) );
        }
    }
    else if ( (nType==CTRL_SCROLLBAR) &&
              ((nPart==PART_BUTTON_LEFT) || (nPart==PART_BUTTON_RIGHT) ||
               (nPart==PART_BUTTON_UP) || (nPart==PART_BUTTON_DOWN)  ) )
    {
        rNativeBoundingRegion = NWGetScrollButtonRect( nPart, rControlRegion );
        rNativeContentRegion = rNativeBoundingRegion;

        if (!rNativeContentRegion.GetWidth())
            rNativeContentRegion.Right() = rNativeContentRegion.Left() + 1;
        if (!rNativeContentRegion.GetHeight())
            rNativeContentRegion.Bottom() = rNativeContentRegion.Top() + 1;

        return true;
    }
    if( (nType == CTRL_MENUBAR) && (nPart == PART_ENTIRE_CONTROL) )
    {
        aEditRect = GetWidgetSize(rControlRegion, gMenuBarWidget);
    }
    else if ( (nType==CTRL_SPINBOX) &&
              ((nPart==PART_BUTTON_UP) || (nPart==PART_BUTTON_DOWN) ||
               (nPart==PART_SUB_EDIT)) )
    {
        Rectangle aControlRegion(GetWidgetSize(rControlRegion, gSpinBox));
        aEditRect = NWGetSpinButtonRect(nPart, aControlRegion);
    }
    else if ( (nType==CTRL_COMBOBOX) &&
              ((nPart==PART_BUTTON_DOWN) || (nPart==PART_SUB_EDIT)) )
    {
        aEditRect = NWGetComboBoxButtonRect( nType, nPart, rControlRegion );
    }
    else if ( (nType==CTRL_LISTBOX) &&
              ((nPart==PART_BUTTON_DOWN) || (nPart==PART_SUB_EDIT)) )
    {
        aEditRect = NWGetComboBoxButtonRect( nType, nPart, rControlRegion );
    }
    else if (nType == CTRL_EDITBOX && nPart == PART_ENTIRE_CONTROL)
    {
        aEditRect = GetWidgetSize(rControlRegion, gEntryBox);
    }
    else if (nType == CTRL_LISTBOX && nPart == PART_ENTIRE_CONTROL)
    {
        aEditRect = GetWidgetSize(rControlRegion, gListBox);
    }
    else if (nType == CTRL_COMBOBOX && nPart == PART_ENTIRE_CONTROL)
    {
        aEditRect = GetWidgetSize(rControlRegion, gComboBox);
    }
    else if (nType == CTRL_SPINBOX && nPart == PART_ENTIRE_CONTROL)
    {
        aEditRect = GetWidgetSize(rControlRegion, gSpinBox);
    }
    else if (nType == CTRL_TAB_ITEM && nPart == PART_ENTIRE_CONTROL)
    {
        const TabitemValue& rTabitemValue = static_cast<const TabitemValue&>(rValue);
        const Rectangle& rTabitemRect = rTabitemValue.getContentRect();

        aEditRect = AdjustRectForTextBordersPadding(mpNotebookHeaderTabsTabStyle, rTabitemRect.GetWidth(),
                                                    rTabitemRect.GetHeight(), rControlRegion);
    }
    else if (nType == CTRL_FRAME && nPart == PART_BORDER)
    {
        aEditRect = rControlRegion;
        DrawFrameFlags nStyle = static_cast<DrawFrameFlags>(rValue.getNumericVal() & 0xfff0);
        if (nStyle & DrawFrameFlags::NoDraw)
        {
            GtkBorder padding;
            gtk_style_context_get_padding(mpFrameInStyle, gtk_style_context_get_state(mpFrameInStyle), &padding);

            GtkBorder border;
            gtk_style_context_get_border(mpFrameInStyle, gtk_style_context_get_state(mpFrameInStyle), &border);

            int x1 = aEditRect.Left();
            int y1 = aEditRect.Top();
            int x2 = aEditRect.Right();
            int y2 = aEditRect.Bottom();

            rNativeBoundingRegion = aEditRect;
            rNativeContentRegion = Rectangle(x1 + (padding.left + border.left),
                                             y1 + (padding.top + border.top),
                                             x2 - (padding.right + border.right),
                                             y2 - (padding.bottom + border.bottom));

            return true;
        }
        else
            rNativeContentRegion = rControlRegion;
    }
    else
    {
        return false;
    }

    rNativeBoundingRegion = aEditRect;
    rNativeContentRegion = rNativeBoundingRegion;

    return true;
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
    GtkStyleContext* pStyle = gtk_widget_get_style_context( mpWindow );
    GtkSettings* pSettings = gtk_widget_get_settings( mpWindow );
    StyleSettings aStyleSet = rSettings.GetStyleSettings();
    GdkRGBA color;

    // text colors
    GdkRGBA text_color;
    gtk_style_context_set_state(pStyle, GTK_STATE_FLAG_NORMAL);
    gtk_style_context_get_color(pStyle, gtk_style_context_get_state(pStyle), &text_color);
    ::Color aTextColor = getColor( text_color );
    aStyleSet.SetDialogTextColor( aTextColor );
    aStyleSet.SetButtonTextColor( aTextColor );
    aStyleSet.SetRadioCheckTextColor( aTextColor );
    aStyleSet.SetGroupTextColor( aTextColor );
    aStyleSet.SetLabelTextColor( aTextColor );
    aStyleSet.SetInfoTextColor( aTextColor );
    aStyleSet.SetWindowTextColor( aTextColor );
    aStyleSet.SetFieldTextColor( aTextColor );

    // background colors
    GdkRGBA background_color;
    gtk_style_context_get_background_color(pStyle, gtk_style_context_get_state(pStyle), &background_color);

    ::Color aBackColor = getColor( background_color );
    aStyleSet.Set3DColors( aBackColor );
    aStyleSet.SetFaceColor( aBackColor );
    aStyleSet.SetDialogColor( aBackColor );
    aStyleSet.SetWorkspaceColor( aBackColor );
    aStyleSet.SetCheckedColorSpecialCase( );

    // UI font
    const PangoFontDescription* font = gtk_style_context_get_font(pStyle, gtk_style_context_get_state(pStyle));
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

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "font name BEFORE system match: \"%s\"\n", aFamily.getStr() );
#endif

    // match font to e.g. resolve "Sans"
    psp::PrintFontManager::get().matchFont( aInfo, rSettings.GetUILanguageTag().getLocale() );

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "font match %s, name AFTER: \"%s\"\n",
                  aInfo.m_nID != 0 ? "succeeded" : "failed",
                  OUStringToOString( aInfo.m_aFamilyName, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif

    int nPointHeight = 0;
    /*sal_Int32 nDispDPIY = GetDisplay()->GetResolution().B();
    static gboolean(*pAbso)(const PangoFontDescription*) =
        (gboolean(*)(const PangoFontDescription*))osl_getAsciiFunctionSymbol( GetSalData()->m_pPlugin, "pango_font_description_get_size_is_absolute" );

    if( pAbso && pAbso( font ) )
        nPointHeight = (nPangoHeight * 72 + nDispDPIY*PANGO_SCALE/2) / (nDispDPIY * PANGO_SCALE);
    else*/
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
    aStyleSet.SetGroupFont( aFont );

    aFont.SetWeight( WEIGHT_BOLD );
    aStyleSet.SetTabFont( aFont );  //pull from notebook style + GTK_STYLE_REGION_TAB ?
    aStyleSet.SetTitleFont( aFont );
    aStyleSet.SetFloatTitleFont( aFont );

    // mouse over text colors
    gtk_style_context_set_state(pStyle, GTK_STATE_FLAG_PRELIGHT);
    gtk_style_context_get_color(pStyle, gtk_style_context_get_state(pStyle), &text_color);
    aTextColor = getColor( text_color );
    aStyleSet.SetButtonRolloverTextColor( aTextColor );
    aStyleSet.SetFieldRolloverTextColor( aTextColor );

    // tooltip colors
    {
        GtkStyleContext *pCStyle = gtk_style_context_new();
        gtk_style_context_set_screen( pCStyle, gtk_window_get_screen( GTK_WINDOW( mpWindow ) ) );
        GtkWidgetPath *pCPath = gtk_widget_path_new();
        guint pos = gtk_widget_path_append_type(pCPath, GTK_TYPE_WINDOW);
        gtk_widget_path_iter_add_class(pCPath, pos, GTK_STYLE_CLASS_TOOLTIP);
        pos = gtk_widget_path_append_type (pCPath, GTK_TYPE_LABEL);
#if GTK_CHECK_VERSION(3,16,0)
        gtk_widget_path_iter_add_class(pCPath, pos, GTK_STYLE_CLASS_LABEL);
#endif
        pCStyle = gtk_style_context_new();
        gtk_style_context_set_path(pCStyle, pCPath);
        gtk_widget_path_free(pCPath);

        GdkRGBA tooltip_bg_color, tooltip_fg_color;
        gtk_style_context_set_state(pCStyle, GTK_STATE_FLAG_NORMAL);
        gtk_style_context_get_color(pCStyle, gtk_style_context_get_state(pCStyle), &tooltip_fg_color);
        gtk_style_context_get_background_color(pCStyle, gtk_style_context_get_state(pCStyle), &tooltip_bg_color);
        g_object_unref( pCStyle );

        aStyleSet.SetHelpColor( getColor( tooltip_bg_color ));
        aStyleSet.SetHelpTextColor( getColor( tooltip_fg_color ));
    }

    {
        // construct style context for text view
        GtkStyleContext *pCStyle = gtk_style_context_new();
        gtk_style_context_set_screen( pCStyle, gtk_window_get_screen( GTK_WINDOW( mpWindow ) ) );
        GtkWidgetPath *pCPath = gtk_widget_path_new();
        gtk_widget_path_append_type( pCPath, GTK_TYPE_TEXT_VIEW );
        gtk_widget_path_iter_add_class( pCPath, -1, GTK_STYLE_CLASS_VIEW );
        gtk_style_context_set_path( pCStyle, pCPath );
        gtk_widget_path_free( pCPath );

        // highlighting colors
        gtk_style_context_set_state(pCStyle, GTK_STATE_FLAG_SELECTED);
        gtk_style_context_get_background_color(pCStyle, gtk_style_context_get_state(pCStyle), &text_color);
        ::Color aHighlightColor = getColor( text_color );
        gtk_style_context_get_color(pCStyle, gtk_style_context_get_state(pCStyle), &text_color);
        ::Color aHighlightTextColor = getColor( text_color );
        aStyleSet.SetHighlightColor( aHighlightColor );
        aStyleSet.SetHighlightTextColor( aHighlightTextColor );

        // field background color
        GdkRGBA field_background_color;
        gtk_style_context_set_state(pCStyle, GTK_STATE_FLAG_NORMAL);
        gtk_style_context_get_background_color(pCStyle, gtk_style_context_get_state(pCStyle), &field_background_color);
        g_object_unref( pCStyle );

        ::Color aBackFieldColor = getColor( field_background_color );
        aStyleSet.SetFieldColor( aBackFieldColor );
        // This baby is the default page/paper color
        aStyleSet.SetWindowColor( aBackFieldColor );

        // Tab colors
        aStyleSet.SetActiveTabColor( aBackFieldColor ); // same as the window color.
        aStyleSet.SetInactiveTabColor( aBackColor );
    }

    // menu disabled entries handling
    aStyleSet.SetSkipDisabledInMenus( true );
    aStyleSet.SetAcceleratorsInContextMenus( false );

    // menu colors
    gtk_style_context_set_state(mpMenuStyle, GTK_STATE_FLAG_NORMAL);
    gtk_style_context_get_background_color( mpMenuStyle, gtk_style_context_get_state(mpMenuStyle), &background_color );
    aBackColor = getColor( background_color );
    aStyleSet.SetMenuColor( aBackColor );

    // menu items
    gtk_style_context_get_color( mpMenuStyle, gtk_style_context_get_state(mpMenuStyle), &color );
    aTextColor = getColor( color );
    aStyleSet.SetMenuTextColor( aTextColor );

    // menu bar
    gtk_style_context_set_state(mpMenuBarStyle, GTK_STATE_FLAG_NORMAL);
    gtk_style_context_get_background_color( mpMenuBarStyle, gtk_style_context_get_state(mpMenuBarStyle), &background_color );
    aBackColor = getColor( background_color );
    aStyleSet.SetMenuBarColor( aBackColor );
    aStyleSet.SetMenuBarRolloverColor( aBackColor );

    gtk_style_context_set_state(mpMenuBarItemStyle, GTK_STATE_FLAG_NORMAL);
    gtk_style_context_get_color( mpMenuBarItemStyle, gtk_style_context_get_state(mpMenuBarItemStyle), &text_color );
    aTextColor = aStyleSet.GetPersonaMenuBarTextColor().get_value_or( getColor( text_color ) );
    aStyleSet.SetMenuBarTextColor( aTextColor );
    aStyleSet.SetMenuBarRolloverTextColor( aTextColor );

    gtk_style_context_set_state(mpMenuBarItemStyle, GTK_STATE_FLAG_PRELIGHT);
    gtk_style_context_get_color( mpMenuBarItemStyle, gtk_style_context_get_state(mpMenuBarItemStyle), &text_color );
    aTextColor = aStyleSet.GetPersonaMenuBarTextColor().get_value_or( getColor( text_color ) );
    aStyleSet.SetMenuBarHighlightTextColor( aTextColor );

    // Awful hack for menu separators in the Sonar and similar themes.
    // If the menu color is not too dark, and the menu text color is lighter,
    // make the "light" color lighter than the menu color and the "shadow"
    // color darker than it.
    if ( aStyleSet.GetMenuColor().GetLuminance() >= 32 &&
     aStyleSet.GetMenuColor().GetLuminance() <= aStyleSet.GetMenuTextColor().GetLuminance() )
    {
        ::Color temp = aStyleSet.GetMenuColor();
        temp.IncreaseLuminance( 8 );
        aStyleSet.SetLightColor( temp );
        temp = aStyleSet.GetMenuColor();
        temp.DecreaseLuminance( 16 );
        aStyleSet.SetShadowColor( temp );
    }

    gtk_style_context_set_state(mpMenuItemStyle, GTK_STATE_FLAG_PRELIGHT);
    gtk_style_context_get_background_color( mpMenuItemStyle, gtk_style_context_get_state(mpMenuItemStyle), &background_color );
    ::Color aHighlightColor = getColor( background_color );
    aStyleSet.SetMenuHighlightColor( aHighlightColor );

    gtk_style_context_get_color( mpMenuItemStyle, gtk_style_context_get_state(mpMenuItemStyle), &color );
    ::Color aHighlightTextColor = getColor( color );
    aStyleSet.SetMenuHighlightTextColor( aHighlightTextColor );

#if GTK_CHECK_VERSION(3, 12, 0)
    // hyperlink colors
    gtk_style_context_set_state(mpLinkButtonStyle, GTK_STATE_FLAG_LINK);
    gtk_style_context_get_color(mpLinkButtonStyle, gtk_style_context_get_state(mpLinkButtonStyle), &text_color);
    aStyleSet.SetLinkColor(getColor(text_color));
    gtk_style_context_set_state(mpLinkButtonStyle, GTK_STATE_FLAG_VISITED);
    gtk_style_context_get_color(mpLinkButtonStyle, gtk_style_context_get_state(mpLinkButtonStyle), &text_color);
    aStyleSet.SetVisitedLinkColor(getColor(text_color));
#endif

#if GTK_CHECK_VERSION(3, 19, 2)
    {
        GtkStyleContext *pCStyle = mpNotebookHeaderTabsTabStyle;

        gtk_style_context_set_state(pCStyle, GTK_STATE_FLAG_NORMAL);
        gtk_style_context_get_color(pCStyle, gtk_style_context_get_state(pCStyle), &text_color);
        aTextColor = getColor( text_color );
        aStyleSet.SetTabTextColor(aTextColor);

        // mouse over text colors
        gtk_style_context_set_state(pCStyle, GTK_STATE_FLAG_PRELIGHT);
        gtk_style_context_get_color(pCStyle, gtk_style_context_get_state(pCStyle), &text_color);
        aTextColor = getColor( text_color );
        aStyleSet.SetTabRolloverTextColor(aTextColor);

        gtk_style_context_set_state(pCStyle, ACTIVE_TAB);
        gtk_style_context_get_color(pCStyle, gtk_style_context_get_state(pCStyle), &text_color);
        aTextColor = getColor( text_color );
        aStyleSet.SetTabHighlightTextColor(aTextColor);
    }
#else
    {
        GtkStyleContext *pCStyle = gtk_style_context_new();
        gtk_style_context_set_screen( pCStyle, gtk_window_get_screen( GTK_WINDOW( mpWindow ) ) );
        GtkWidgetPath *pCPath = gtk_widget_path_new();
        guint pos = gtk_widget_path_append_type(pCPath, GTK_TYPE_NOTEBOOK);
        gtk_widget_path_iter_add_class(pCPath, pos, GTK_STYLE_CLASS_NOTEBOOK);
        gtk_widget_path_iter_add_region(pCPath, pos, GTK_STYLE_REGION_TAB, static_cast<GtkRegionFlags>(GTK_REGION_EVEN | GTK_REGION_FIRST));
        pos = gtk_widget_path_append_type (pCPath, GTK_TYPE_LABEL);
#if GTK_CHECK_VERSION(3,16,0)
        gtk_widget_path_iter_add_class(pCPath, pos, GTK_STYLE_CLASS_LABEL);
#endif
        pCStyle = gtk_style_context_new();
        gtk_style_context_set_path(pCStyle, pCPath);
        gtk_widget_path_free(pCPath);

        gtk_style_context_set_state(pCStyle, GTK_STATE_FLAG_NORMAL);
        gtk_style_context_get_color(pCStyle, gtk_style_context_get_state(pCStyle), &text_color);
        aTextColor = getColor( text_color );
        aStyleSet.SetTabTextColor(aTextColor);

        // mouse over text colors
        gtk_style_context_add_class(pCStyle, "prelight-page");
        gtk_style_context_set_state(pCStyle, GTK_STATE_FLAG_PRELIGHT);
        gtk_style_context_get_color(pCStyle, gtk_style_context_get_state(pCStyle), &text_color);
        gtk_style_context_remove_class(pCStyle, "prelight-page");
        aTextColor = getColor( text_color );
        aStyleSet.SetTabRolloverTextColor(aTextColor);

        gtk_style_context_add_class(pCStyle, "active-page");
        gtk_style_context_set_state(pCStyle, GTK_STATE_FLAG_ACTIVE);
        gtk_style_context_get_color(pCStyle, gtk_style_context_get_state(pCStyle), &text_color);
        gtk_style_context_remove_class(pCStyle, "active-page");
        aTextColor = getColor( text_color );
        aStyleSet.SetTabHighlightTextColor(aTextColor);

        g_object_unref( pCStyle );
     }
#endif

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
    int iDoubleClickTime, iDoubleClickDistance, iDragThreshold;
    static const int MENU_POPUP_DELAY = 225;
    g_object_get( pSettings,
                  "gtk-double-click-time", &iDoubleClickTime,
                  "gtk-double-click-distance", &iDoubleClickDistance,
                  "gtk-dnd-drag-threshold", &iDragThreshold,
                  nullptr );
    aMouseSettings.SetDoubleClickTime( iDoubleClickTime );
    aMouseSettings.SetDoubleClickWidth( iDoubleClickDistance );
    aMouseSettings.SetDoubleClickHeight( iDoubleClickDistance );
    aMouseSettings.SetStartDragWidth( iDragThreshold );
    aMouseSettings.SetStartDragHeight( iDragThreshold );
    aMouseSettings.SetMenuDelay( MENU_POPUP_DELAY );
    rSettings.SetMouseSettings( aMouseSettings );

    gboolean primarybuttonwarps = false;
    g_object_get( pSettings,
        "gtk-primary-button-warps-slider", &primarybuttonwarps,
        nullptr );
    aStyleSet.SetPreferredUseImagesInMenus(false);
    aStyleSet.SetPrimaryButtonWarpsSlider(primarybuttonwarps);

    // set scrollbar settings
    gint slider_width = 14;
    gint trough_border = 1;
    gint min_slider_length = 21;

    // Grab some button style attributes
    gtk_style_context_get_style(mpVScrollbarStyle,
                                "slider-width", &slider_width,
                                "trough-border", &trough_border,
                                nullptr);
    aStyleSet.SetScrollBarSize(slider_width + 2*trough_border);
    if (gtk_check_version(3, 20, 0) == nullptr)
    {
        gtk_style_context_get(mpVScrollbarSliderStyle, gtk_style_context_get_state(mpVScrollbarSliderStyle),
                              "min-height", &min_slider_length,
                              nullptr);
        aStyleSet.SetMinThumbSize(min_slider_length);
    }
    else
    {
        gtk_style_context_get_style(mpVScrollbarStyle,
                                    "min-slider-length", &min_slider_length,
                                    nullptr);
        gint magic = trough_border ? 1 : 0;
        aStyleSet.SetMinThumbSize(min_slider_length - magic);
    }

    // preferred icon style
    gchar* pIconThemeName = nullptr;
    g_object_get( pSettings, "gtk-icon-theme-name", &pIconThemeName, nullptr );
    aStyleSet.SetPreferredIconTheme( OUString::createFromAscii( pIconThemeName ) );
    g_free( pIconThemeName );

    aStyleSet.SetToolbarIconSize( ToolbarIconSize::Large );

    // finally update the collected settings
    rSettings.SetStyleSettings( aStyleSet );
#if OSL_DEBUG_LEVEL > 1
    gchar* pThemeName = NULL;
    g_object_get( pSettings, "gtk-theme-name", &pThemeName, nullptr );
    fprintf( stderr, "Theme name is \"%s\"\n", pThemeName );
    g_free(pThemeName);
#endif
}

bool GtkSalGraphics::IsNativeControlSupported( ControlType nType, ControlPart nPart )
{
    switch(nType)
    {
        case CTRL_PUSHBUTTON:
        case CTRL_RADIOBUTTON:
        case CTRL_CHECKBOX:
        case CTRL_PROGRESS:
        case CTRL_LISTNODE:
        case CTRL_LISTNET:
            if (nPart==PART_ENTIRE_CONTROL || nPart == PART_FOCUS)
                return true;
            break;

        case CTRL_SCROLLBAR:
            if(nPart==PART_DRAW_BACKGROUND_HORZ || nPart==PART_DRAW_BACKGROUND_VERT ||
               nPart==PART_ENTIRE_CONTROL       || nPart==HAS_THREE_BUTTONS)
                return true;
            break;

        case CTRL_EDITBOX:
        case CTRL_MULTILINE_EDITBOX:
            if (nPart==PART_ENTIRE_CONTROL || nPart==HAS_BACKGROUND_TEXTURE)
                return true;
            break;

        case CTRL_COMBOBOX:
            if (nPart==PART_ENTIRE_CONTROL || nPart==HAS_BACKGROUND_TEXTURE || nPart == PART_ALL_BUTTONS)
                return true;
            break;

        case CTRL_SPINBOX:
            if (nPart==PART_ENTIRE_CONTROL || nPart==HAS_BACKGROUND_TEXTURE || nPart == PART_ALL_BUTTONS || nPart == PART_BUTTON_UP || nPart == PART_BUTTON_DOWN)
                return true;
            break;

        case CTRL_SPINBUTTONS:
            if (nPart==PART_ENTIRE_CONTROL || nPart==PART_ALL_BUTTONS)
                return true;
            break;

        case CTRL_FRAME:
        case CTRL_WINDOW_BACKGROUND:
            return true;

        case CTRL_TAB_ITEM:
        case CTRL_TAB_HEADER:
        case CTRL_TAB_PANE:
        case CTRL_TAB_BODY:
            if(nPart==PART_ENTIRE_CONTROL || nPart==PART_TABS_DRAW_RTL)
                return true;
            break;

        case CTRL_LISTBOX:
            if (nPart==PART_ENTIRE_CONTROL || nPart==PART_WINDOW || nPart==HAS_BACKGROUND_TEXTURE || nPart == PART_FOCUS)
                return true;
            break;

        case CTRL_TOOLBAR:
            if( nPart==PART_ENTIRE_CONTROL
//                ||  nPart==PART_DRAW_BACKGROUND_HORZ
//                ||  nPart==PART_DRAW_BACKGROUND_VERT
//                ||  nPart==PART_THUMB_HORZ
//                ||  nPart==PART_THUMB_VERT
                ||  nPart==PART_BUTTON
//                ||  nPart==PART_SEPARATOR_HORZ
                ||  nPart==PART_SEPARATOR_VERT
                )
                return true;
            break;

        case CTRL_MENUBAR:
            if (nPart==PART_ENTIRE_CONTROL || nPart==PART_MENU_ITEM)
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

//        case CTRL_SLIDER:
//            if(nPart == PART_TRACK_HORZ_AREA || nPart == PART_TRACK_VERT_AREA)
//                return true;
//            break;

        case CTRL_FIXEDLINE:
            if (nPart == PART_SEPARATOR_VERT || nPart == PART_SEPARATOR_HORZ)
                return true;
            break;

        case CTRL_LISTHEADER:
            if (nPart == PART_BUTTON || nPart == PART_ARROW)
                return true;
            break;
    }

    SAL_INFO("vcl.gtk", "Unhandled is native supported for Type:" << nType << ", Part" << nPart);

    return false;
}

bool GtkSalGraphics::SupportsCairo() const
{
    return true;
}

cairo::SurfaceSharedPtr GtkSalGraphics::CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const
{
    return cairo::SurfaceSharedPtr(new cairo::Gtk3Surface(rSurface));
}

cairo::SurfaceSharedPtr GtkSalGraphics::CreateSurface(const OutputDevice& /*rRefDevice*/, int x, int y, int width, int height) const
{
    return cairo::SurfaceSharedPtr(new cairo::Gtk3Surface(this, x, y, width, height));
}

void GtkSalGraphics::WidgetQueueDraw() const
{
    //request gtk to sync the entire contents
    gtk_widget_queue_draw(mpWindow);
}

namespace {

void getStyleContext(GtkStyleContext** style, GtkWidget* widget)
{
    gtk_container_add(GTK_CONTAINER(gDumbContainer), widget);
    *style = gtk_widget_get_style_context(widget);
    g_object_ref(*style);
}

}

void GtkData::initNWF()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maNWFData.mbFlatMenu = true;
    pSVData->maNWFData.mbCheckBoxNeedsErase = true;
    pSVData->maNWFData.mbDockingAreaAvoidTBFrames = true;
    pSVData->maNWFData.mbCanDrawWidgetAnySize = true;
    pSVData->maNWFData.mbDDListBoxNoTextArea = true;
    pSVData->maNWFData.mbNoFocusRects = true;
    pSVData->maNWFData.mbNoFocusRectsForFlatButtons = true;
    pSVData->maNWFData.mbAutoAccel = true;
    pSVData->maNWFData.mbEnableAccel = true;
}

void GtkData::deInitNWF()
{
    if (gCacheWindow)
        gtk_widget_destroy(gCacheWindow);
}

GtkSalGraphics::GtkSalGraphics( GtkSalFrame *pFrame, GtkWidget *pWindow )
    : SvpSalGraphics(),
      mpFrame( pFrame ),
      mpWindow( pWindow )
{
    if(style_loaded)
        return;

    style_loaded = true;
    gtk_init(nullptr, nullptr);
    /* Load the GtkStyleContexts, it might be a bit slow, but usually,
     * gtk apps create a lot of widgets at startup, so, it shouldn't be
     * too slow */
    gCacheWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gDumbContainer = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(gCacheWindow), gDumbContainer);
    gtk_widget_realize(gDumbContainer);
    gtk_widget_realize(gCacheWindow);

    gEntryBox = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(gDumbContainer), gEntryBox);
    mpEntryStyle = createStyleContext(GtkControlPart::Entry);

    getStyleContext(&mpTextViewStyle, gtk_text_view_new());

    mpButtonStyle = createStyleContext(GtkControlPart::Button);
    mpLinkButtonStyle = createStyleContext(GtkControlPart::LinkButton);

    GtkWidget* pToolbar = gtk_toolbar_new();
    mpToolbarStyle = gtk_widget_get_style_context(pToolbar);
    gtk_style_context_add_class(mpToolbarStyle, GTK_STYLE_CLASS_TOOLBAR);

    GtkToolItem *item = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), item, -1);
    mpToolbarSeperatorStyle = gtk_widget_get_style_context(GTK_WIDGET(item));

    GtkWidget *pButton = gtk_button_new();
    item = gtk_tool_button_new(pButton, nullptr);
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), item, -1);
    mpToolButtonStyle = gtk_widget_get_style_context(GTK_WIDGET(pButton));

    mpVScrollbarStyle = createStyleContext(GtkControlPart::ScrollbarVertical);
    mpVScrollbarTroughStyle = createStyleContext(GtkControlPart::ScrollbarTrough, mpVScrollbarStyle);
    mpVScrollbarSliderStyle = createStyleContext(GtkControlPart::ScrollbarSlider, mpVScrollbarTroughStyle);
    mpVScrollbarButtonStyle = createStyleContext(GtkControlPart::ScrollbarButton, mpVScrollbarStyle);
    mpHScrollbarStyle = createStyleContext(GtkControlPart::ScrollbarHorizontal);
    mpHScrollbarTroughStyle = createStyleContext(GtkControlPart::ScrollbarTrough, mpHScrollbarStyle);
    mpHScrollbarSliderStyle = createStyleContext(GtkControlPart::ScrollbarSlider, mpHScrollbarTroughStyle);
    mpHScrollbarButtonStyle = createStyleContext(GtkControlPart::ScrollbarButton, mpHScrollbarStyle);

    {
        GtkStyleContext* parentContext = createStyleContext(GtkControlPart::CheckButton);
        mpCheckButtonStyle = createStyleContext(GtkControlPart::CheckButtonCheck, parentContext);
        g_object_unref(parentContext);
    }

    {
        GtkStyleContext* parentContext = createStyleContext(GtkControlPart::RadioButton);
        mpRadioButtonStyle = createStyleContext(GtkControlPart::RadioButtonRadio, parentContext);
        g_object_unref(parentContext);
    }

    /* Menu bar */
    gMenuBarWidget = gtk_menu_bar_new();
    gMenuItemMenuBarWidget = gtk_menu_item_new_with_label( "b" );
    gtk_menu_shell_append(GTK_MENU_SHELL(gMenuBarWidget), gMenuItemMenuBarWidget);
    gtk_container_add(GTK_CONTAINER(gDumbContainer), gMenuBarWidget);

    mpMenuBarStyle = createStyleContext(GtkControlPart::MenuBar, gtk_widget_get_style_context(mpWindow));
    mpMenuBarItemStyle = createStyleContext(GtkControlPart::MenuItem, mpMenuBarStyle);

    /* Menu */
    mpMenuStyle = createStyleContext(GtkControlPart::Menu, gtk_widget_get_style_context(mpWindow));
    GtkWidget *menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(gMenuItemMenuBarWidget), menu);

    /* Menu Items */
    gCheckMenuItemWidget = gtk_check_menu_item_new_with_label("M");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gCheckMenuItemWidget);

    mpMenuItemStyle = createStyleContext(GtkControlPart::MenuItem, mpMenuStyle);
    mpMenuItemArrowStyle = createStyleContext(GtkControlPart::MenuItemArrow, mpMenuItemStyle);
    mpCheckMenuItemStyle = createStyleContext(GtkControlPart::CheckMenuItemCheck, mpMenuItemStyle);
    mpRadioMenuItemStyle = createStyleContext(GtkControlPart::RadioMenuItemRadio, mpMenuItemStyle);
    mpSeparatorMenuItemStyle = createStyleContext(GtkControlPart::SeparatorMenuItemSeparator, mpMenuItemStyle);

    /* Spinbutton */
    gSpinBox = gtk_spin_button_new(nullptr, 0, 0);
    gtk_container_add(GTK_CONTAINER(gDumbContainer), gSpinBox);
    mpSpinStyle = createStyleContext(GtkControlPart::SpinButton);
    mpSpinUpStyle = createStyleContext(GtkControlPart::SpinButtonUpButton, mpSpinStyle);
    mpSpinDownStyle = createStyleContext(GtkControlPart::SpinButtonDownButton, mpSpinStyle);

    /* NoteBook */
    mpNotebookStyle = createStyleContext(GtkControlPart::Notebook);
    mpNotebookStackStyle = createStyleContext(GtkControlPart::NotebookStack, mpNotebookStyle);
    mpNotebookHeaderStyle = createStyleContext(GtkControlPart::NotebookHeader, mpNotebookStyle);
    mpNotebookHeaderTabsStyle = createStyleContext(GtkControlPart::NotebookHeaderTabs, mpNotebookHeaderStyle);
    mpNotebookHeaderTabsTabStyle = createStyleContext(GtkControlPart::NotebookHeaderTabsTab, mpNotebookHeaderTabsStyle);

    /* Combobox */
    gComboBox = gtk_combo_box_text_new_with_entry();
    getStyleContext(&mpComboboxStyle, gComboBox);
    mpComboboxButtonStyle = createStyleContext(GtkControlPart::Button, mpComboboxStyle);

    /* Listbox */
    gListBox = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(gListBox), "sample");
    getStyleContext(&mpListboxStyle, gListBox);
    mpListboxButtonStyle = createStyleContext(GtkControlPart::Button, mpListboxStyle);

    /* Frames */
    mpFrameOutStyle = mpFrameInStyle = createStyleContext(GtkControlPart::FrameBorder);
    getStyleContext(&mpFixedHoriLineStyle, gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
    getStyleContext(&mpFixedVertLineStyle, gtk_separator_new(GTK_ORIENTATION_VERTICAL));


    /* Tree List */
    gTreeViewWidget = gtk_tree_view_new();
    gtk_container_add(GTK_CONTAINER(gDumbContainer), gTreeViewWidget);

    GtkTreeViewColumn* firstTreeViewColumn = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(firstTreeViewColumn, "M");
    gtk_tree_view_append_column(GTK_TREE_VIEW(gTreeViewWidget), firstTreeViewColumn);

    GtkTreeViewColumn* middleTreeViewColumn = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(middleTreeViewColumn, "M");
    gtk_tree_view_append_column(GTK_TREE_VIEW(gTreeViewWidget), middleTreeViewColumn);
    gtk_tree_view_set_expander_column(GTK_TREE_VIEW(gTreeViewWidget), middleTreeViewColumn);

    GtkTreeViewColumn* lastTreeViewColumn = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(lastTreeViewColumn, "M");
    gtk_tree_view_append_column(GTK_TREE_VIEW(gTreeViewWidget), lastTreeViewColumn);

    /* Use the middle column's header for our button */
    GtkWidget* pTreeHeaderCellWidget = gtk_tree_view_column_get_button(middleTreeViewColumn);
    mpTreeHeaderButtonStyle = gtk_widget_get_style_context(pTreeHeaderCellWidget);

    /* Progress Bar */
    mpProgressBarStyle = createStyleContext(GtkControlPart::ProgressBar);
    mpProgressBarTroughStyle = createStyleContext(GtkControlPart::ProgressBar, mpProgressBarStyle);
    mpProgressBarProgressStyle = createStyleContext(GtkControlPart::ProgressBarProgress, mpProgressBarTroughStyle);

    gtk_widget_show_all(gDumbContainer);
}

void GtkSalGraphics::GetResolution(sal_Int32& rDPIX, sal_Int32& rDPIY)
{
    char* pForceDpi;
    if ((pForceDpi = getenv("SAL_FORCEDPI")))
    {
        OString sForceDPI(pForceDpi);
        rDPIX = rDPIY = sForceDPI.toInt32();
        return;
    }

    GdkScreen* pScreen = gtk_widget_get_screen(mpWindow);
    double fResolution = -1.0;
    g_object_get(pScreen, "resolution", &fResolution, nullptr);

    int nScaleFactor = 1;

#if GTK_CHECK_VERSION(3, 10, 0)
    nScaleFactor = gdk_window_get_scale_factor(widget_get_window(mpWindow));
#endif

    if (fResolution > 0.0)
        rDPIX = rDPIY = sal_Int32(fResolution * nScaleFactor);
    else
        rDPIX = rDPIY = 96;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
