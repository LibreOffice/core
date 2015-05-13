/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Gross inclusion hacks for now ...

#include "../../../headless/svpgdi.cxx"
#include <unx/gtk/gtkframe.hxx>
#include <unx/gtk/gtkdata.hxx>
#include <unx/gtk/gtkinst.hxx>
#include <unx/gtk/gtkgdi.hxx>
#include <vcl/decoview.hxx>
#include <vcl/settings.hxx>
#include "fontmanager.hxx"
#include "gtk3cairotextrender.hxx"
#include "cairo_gtk3_cairo.hxx"

GtkStyleContext* GtkSalGraphics::mpButtonStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpEntryStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpVScrollbarStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpHScrollbarStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpToolbarStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpToolButtonStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpToolbarSeperatorStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpCheckButtonStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpMenuBarStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpMenuStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpMenuItemStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpSpinStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpComboboxStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpComboboxEntryStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpComboboxButtonStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpListboxStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpListboxButtonStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpNoteBookStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpFrameInStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpFrameOutStyle = NULL;

bool GtkSalGraphics::style_loaded = false;
/************************************************************************
 * State conversion
 ************************************************************************/
static void NWConvertVCLStateToGTKState( ControlState nVCLState,
            GtkStateFlags* nGTKState, GtkShadowType* nGTKShadow )
{
    *nGTKShadow = GTK_SHADOW_OUT;
    *nGTKState = GTK_STATE_FLAG_NORMAL;

    if (!( nVCLState & ControlState::ENABLED ))
        *nGTKState = GTK_STATE_FLAG_INSENSITIVE;

    if ( nVCLState & ControlState::PRESSED )
    {
        *nGTKState = (GtkStateFlags) (*nGTKState | GTK_STATE_FLAG_ACTIVE);
        *nGTKShadow = GTK_SHADOW_IN;
    }

    if ( nVCLState & ControlState::ROLLOVER )
        *nGTKState = (GtkStateFlags) (*nGTKState | GTK_STATE_FLAG_PRELIGHT);

    if ( nVCLState & ControlState::SELECTED )
        *nGTKState = (GtkStateFlags) (*nGTKState | GTK_STATE_FLAG_SELECTED);

    if ( nVCLState & ControlState::FOCUSED )
        *nGTKState = (GtkStateFlags) (*nGTKState | GTK_STATE_FLAG_FOCUSED);
}

enum {
    RENDER_BACKGROUND_AND_FRAME = 1,
    RENDER_CHECK = 2,
    RENDER_BACKGROUND = 3,
    RENDER_MENU_SEPERATOR = 4,
    RENDER_TOOLBAR_SEPERATOR = 5,
    RENDER_ARROW = 6,
    RENDER_RADIO = 7,
    RENDER_SCROLLBAR = 8,
    RENDER_SPINBUTTON = 9,
    RENDER_COMBOBOX = 10,
    RENDER_EXTENSION = 11,
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

#define MIN_SPIN_ARROW_WIDTH 6

Rectangle GtkSalGraphics::NWGetSpinButtonRect( ControlPart nPart, Rectangle aAreaRect)
{
    gint            buttonSize;
    Rectangle        buttonRect;
    const PangoFontDescription *fontDesc;
    GtkBorder padding;

    gtk_style_context_save(mpSpinStyle);
    gtk_style_context_add_class(mpSpinStyle, GTK_STYLE_CLASS_BUTTON);

    fontDesc = gtk_style_context_get_font( mpSpinStyle, GTK_STATE_FLAG_NORMAL);
    gtk_style_context_get_padding(mpSpinStyle, GTK_STATE_FLAG_NORMAL, &padding);

    buttonSize = MAX( PANGO_PIXELS(pango_font_description_get_size(fontDesc) ),
                      MIN_SPIN_ARROW_WIDTH );
    buttonSize -= buttonSize % 2 - 1; /* force odd */
    buttonRect.SetSize( Size( buttonSize + padding.left + padding.right,
                              buttonRect.GetHeight() ) );
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
        buttonRect.Right()  = buttonRect.Left()-1;
        buttonRect.Left()   = aAreaRect.Left();
        buttonRect.Top()    = aAreaRect.Top();
        buttonRect.Bottom() = aAreaRect.Bottom();
    }

    gtk_style_context_restore(mpSpinStyle);

    return buttonRect;
}

Rectangle GtkSalGraphics::NWGetScrollButtonRect( ControlPart nPart, Rectangle aAreaRect )
{
    GtkStyleContext* pScrollbarStyle = NULL;
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
                                 "stepper-spacing", &stepper_spacing, (char *)NULL );

    gboolean has_forward;
    gboolean has_forward2;
    gboolean has_backward;
    gboolean has_backward2;

    gtk_style_context_get_style( pScrollbarStyle,
                                 "has-forward-stepper", &has_forward,
                                 "has-secondary-forward-stepper", &has_forward2,
                                 "has-backward-stepper", &has_backward,
                                 "has-secondary-backward-stepper", &has_backward2, (char *)NULL );
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
    GtkShadowType    shadowType;
    GtkOrientation    scrollbarOrientation;
    Rectangle        thumbRect = rScrollbarVal.maThumbRect;
    Rectangle        button11BoundRect = rScrollbarVal.maButton1Rect;   // backward
    Rectangle        button22BoundRect = rScrollbarVal.maButton2Rect;   // forward
    Rectangle        button12BoundRect = rScrollbarVal.maButton1Rect;   // secondary forward
    Rectangle        button21BoundRect = rScrollbarVal.maButton2Rect;   // secondary backward
    gdouble          arrow1Angle;                                        // backward
    gdouble          arrow2Angle;                                        // forward
    Rectangle        arrowRect;
    const gchar*     button1StyleClass = NULL;
    const gchar*     button2StyleClass = NULL;
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
                                 "min_slider_length", &min_slider_length, (char *)NULL );
    gboolean has_forward;
    gboolean has_forward2;
    gboolean has_backward;
    gboolean has_backward2;

    gtk_style_context_get_style( context,
                                 "has-forward-stepper", &has_forward,
                                 "has-secondary-forward-stepper", &has_forward2,
                                 "has-backward-stepper", &has_backward,
                                 "has-secondary-backward-stepper", &has_backward2, (char *)NULL );
    gint magic = trough_border ? 1 : 0;
    gint nFirst = 0;
    gint slider_side = slider_width + (trough_border * 2);

    if ( has_backward )  nFirst  += 1;
    if ( has_forward2 )  nFirst  += 1;

    if ( nPart == PART_DRAW_BACKGROUND_HORZ )
    {
        scrollbarRect.Move( 0, (scrollbarRect.GetHeight() - slider_side) / 2 );
        scrollbarRect.SetSize( Size( scrollbarRect.GetWidth(), slider_side ) );

        scrollbarOrientation = GTK_ORIENTATION_HORIZONTAL;
        arrow1Angle = G_PI * 3 / 2;
        arrow2Angle = G_PI / 2;
        button1StyleClass = GTK_STYLE_CLASS_LEFT;
        button2StyleClass = GTK_STYLE_CLASS_RIGHT;

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
        button1StyleClass = GTK_STYLE_CLASS_TOP;
        button2StyleClass = GTK_STYLE_CLASS_BOTTOM;

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

    // ----------------- TROUGH
    gtk_style_context_save(context);
    gtk_style_context_add_class(context, GTK_STYLE_CLASS_TROUGH);
    gtk_render_background(context, cr, 0, 0,
                          scrollbarRect.GetWidth(), scrollbarRect.GetHeight() );
    gtk_render_frame(context, cr, 0, 0,
                     scrollbarRect.GetWidth(), scrollbarRect.GetHeight() );

    gtk_style_context_restore(context);

    // ----------------- THUMB
    if ( has_slider )
    {
        NWConvertVCLStateToGTKState( rScrollbarVal.mnThumbState, &stateFlags, &shadowType );
        if ( rScrollbarVal.mnThumbState & ControlState::PRESSED )
            stateFlags = (GtkStateFlags) (stateFlags | GTK_STATE_PRELIGHT);

        gtk_style_context_save(context);
        gtk_style_context_set_state(context, stateFlags);
        gtk_style_context_add_class(context, GTK_STYLE_CLASS_SLIDER);

        gtk_render_slider(context, cr,
                          thumbRect.Left(), thumbRect.Top(),
                          thumbRect.GetWidth(), thumbRect.GetHeight(), scrollbarOrientation);

        gtk_style_context_restore(context);
    }

    bool backwardButtonInsensitive =
        rScrollbarVal.mnCur == rScrollbarVal.mnMin;
    bool forwardButtonInsensitive = rScrollbarVal.mnMax == 0 ||
        rScrollbarVal.mnCur + rScrollbarVal.mnVisibleSize >= rScrollbarVal.mnMax;

    // ----------------- BUTTON 1
    if ( has_backward )
    {
        NWConvertVCLStateToGTKState( rScrollbarVal.mnButton1State, &stateFlags, &shadowType );
        if ( backwardButtonInsensitive )
            stateFlags = GTK_STATE_FLAG_INSENSITIVE;

        gtk_style_context_save(context);
        gtk_style_context_set_state(context, stateFlags);
        gtk_style_context_add_class(context, GTK_STYLE_CLASS_BUTTON);
        gtk_style_context_add_class(context, button1StyleClass);

        gtk_render_background(context, cr,
                              button11BoundRect.Left(), button11BoundRect.Top(),
                              button11BoundRect.GetWidth(), button11BoundRect.GetHeight() );
        gtk_render_frame(context, cr,
                         button11BoundRect.Left(), button11BoundRect.Top(),
                         button11BoundRect.GetWidth(), button11BoundRect.GetHeight() );

        // ----------------- ARROW 1
        NWCalcArrowRect( button11BoundRect, arrowRect );
        gtk_render_arrow(context, cr,
                         arrow1Angle,
                         arrowRect.Left(), arrowRect.Top(),
                         MIN(arrowRect.GetWidth(), arrowRect.GetHeight()) );

        gtk_style_context_restore(context);
    }
    if ( has_forward2 )
    {
        NWConvertVCLStateToGTKState( rScrollbarVal.mnButton2State, &stateFlags, &shadowType );
        if ( forwardButtonInsensitive )
            stateFlags = GTK_STATE_FLAG_INSENSITIVE;

        gtk_style_context_save(context);
        gtk_style_context_set_state(context, stateFlags);
        gtk_style_context_add_class(context, GTK_STYLE_CLASS_BUTTON);
        gtk_style_context_add_class(context, button1StyleClass);

        gtk_render_background(context, cr,
                              button12BoundRect.Left(), button12BoundRect.Top(),
                              button12BoundRect.GetWidth(), button12BoundRect.GetHeight() );
        gtk_render_frame(context, cr,
                         button12BoundRect.Left(), button12BoundRect.Top(),
                         button12BoundRect.GetWidth(), button12BoundRect.GetHeight() );

        // ----------------- ARROW 1
        NWCalcArrowRect( button12BoundRect, arrowRect );
        gtk_render_arrow(context, cr,
                         arrow2Angle,
                         arrowRect.Left(), arrowRect.Top(),
                         MIN(arrowRect.GetWidth(), arrowRect.GetHeight()) );

        gtk_style_context_restore(context);
    }
    // ----------------- BUTTON 2
    if ( has_backward2 )
    {
        NWConvertVCLStateToGTKState( rScrollbarVal.mnButton1State, &stateFlags, &shadowType );
        if ( backwardButtonInsensitive )
            stateFlags = GTK_STATE_FLAG_INSENSITIVE;

        gtk_style_context_save(context);
        gtk_style_context_set_state(context, stateFlags);
        gtk_style_context_add_class(context, GTK_STYLE_CLASS_BUTTON);
        gtk_style_context_add_class(context, button2StyleClass);

        gtk_render_background(context, cr,
                              button21BoundRect.Left(), button21BoundRect.Top(),
                              button21BoundRect.GetWidth(), button21BoundRect.GetHeight() );
        gtk_render_frame(context, cr,
                         button21BoundRect.Left(), button21BoundRect.Top(),
                         button21BoundRect.GetWidth(), button21BoundRect.GetHeight() );

        // ----------------- ARROW 2
        NWCalcArrowRect( button21BoundRect, arrowRect );
        gtk_render_arrow(context, cr,
                         arrow1Angle,
                         arrowRect.Left(), arrowRect.Top(),
                         MIN(arrowRect.GetWidth(), arrowRect.GetHeight()) );

        gtk_style_context_restore(context);
    }
    if ( has_forward )
    {
        NWConvertVCLStateToGTKState( rScrollbarVal.mnButton2State, &stateFlags, &shadowType );
        if ( forwardButtonInsensitive )
            stateFlags = GTK_STATE_FLAG_INSENSITIVE;

        gtk_style_context_save(context);
        gtk_style_context_set_state(context, stateFlags);
        gtk_style_context_add_class(context, GTK_STYLE_CLASS_BUTTON);
        gtk_style_context_add_class(context, button2StyleClass);

        gtk_render_background(context, cr,
                       button22BoundRect.Left(), button22BoundRect.Top(),
                       button22BoundRect.GetWidth(), button22BoundRect.GetHeight() );
        gtk_render_frame(context, cr,
                       button22BoundRect.Left(), button22BoundRect.Top(),
                       button22BoundRect.GetWidth(), button22BoundRect.GetHeight() );

        // ----------------- ARROW 2
        NWCalcArrowRect( button22BoundRect, arrowRect );
        gtk_render_arrow(context, cr,
                         arrow2Angle,
                         arrowRect.Left(), arrowRect.Top(),
                         MIN(arrowRect.GetWidth(), arrowRect.GetHeight()) );

        gtk_style_context_restore(context);
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
    Rectangle            buttonRect;
    GtkStateFlags        stateFlags;
    GtkShadowType        shadowType;
    Rectangle            arrowRect;
    gint                 arrowSize;
    GtkBorder            padding;

    NWConvertVCLStateToGTKState( nState, &stateFlags,  &shadowType );
    buttonRect = NWGetSpinButtonRect( nPart, aAreaRect );

    gtk_style_context_save(context);
    gtk_style_context_set_state(context, stateFlags);
    gtk_style_context_add_class(context, GTK_STYLE_CLASS_BUTTON);

    gtk_style_context_get_padding(context, GTK_STATE_FLAG_NORMAL, &padding);

    gtk_render_background(context, cr,
                          (buttonRect.Left() - aAreaRect.Left()), (buttonRect.Top() - aAreaRect.Top()),
                          buttonRect.GetWidth(), buttonRect.GetHeight() );
    gtk_render_frame(context, cr,
                     (buttonRect.Left() - aAreaRect.Left()), (buttonRect.Top() - aAreaRect.Top()),
                     buttonRect.GetWidth(), buttonRect.GetHeight() );

    arrowSize = (gint) floor((buttonRect.GetWidth() - padding.left - padding.right) * 0.45);
    arrowSize -= arrowSize % 2 - 1; /* force odd */
    arrowRect.SetSize( Size( arrowSize, arrowSize ) );
    arrowRect.setX( buttonRect.Left() + (buttonRect.GetWidth() - arrowRect.GetWidth()) / 2 );
    if ( nPart == PART_BUTTON_UP )
        arrowRect.setY( buttonRect.Top() + (buttonRect.GetHeight() - arrowRect.GetHeight()) / 2 + 1);
    else
        arrowRect.setY( buttonRect.Top() + (buttonRect.GetHeight() - arrowRect.GetHeight()) / 2 - 1);

    gtk_render_arrow(context, cr,
                     (nPart == PART_BUTTON_UP) ? 0 : G_PI,
                     (arrowRect.Left() - aAreaRect.Left()), (arrowRect.Top() - aAreaRect.Top()),
                     arrowSize);

    gtk_style_context_restore(context);
}

void GtkSalGraphics::PaintSpinButton(GtkStyleContext *context,
                                     cairo_t *cr,
                                     const Rectangle& rControlRectangle,
                                     ControlType nType,
                                     ControlPart nPart,
                                     const ImplControlValue& rValue )
{
    (void)nPart;
    Rectangle            areaRect;
    GtkShadowType        shadowType;
    const SpinbuttonValue *    pSpinVal = (rValue.getType() == CTRL_SPINBUTTONS) ? static_cast<const SpinbuttonValue *>(&rValue) : NULL;
    ControlPart        upBtnPart = PART_BUTTON_UP;
    ControlState        upBtnState = ControlState::ENABLED;
    ControlPart        downBtnPart = PART_BUTTON_DOWN;
    ControlState        downBtnState = ControlState::ENABLED;

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
            SAL_WARN( "vcl.gtk", "Tried to draw CTRL_SPINBUTTONS, but the SpinButtons data structure didn't exist!" );
            return;
        }
        areaRect = pSpinVal->maUpperRect;
        areaRect.Union( pSpinVal->maLowerRect );
    }
    else
        areaRect = rControlRectangle;

    gtk_style_context_get_style( context,
                                 "shadow-type", &shadowType,
                                 NULL );

    if ( shadowType != GTK_SHADOW_NONE )
    {
        gtk_render_background(context, cr,
                              0, 0,
                              areaRect.GetWidth(), areaRect.GetHeight() );
        gtk_render_frame(context, cr,
                         0, 0,
                         areaRect.GetWidth(), areaRect.GetHeight() );
   }

    PaintOneSpinButton(context, cr, nType, upBtnPart, areaRect, upBtnState );
    PaintOneSpinButton(context, cr, nType, downBtnPart, areaRect, downBtnState );
}

#define ARROW_SIZE 11 * 0.85
Rectangle GtkSalGraphics::NWGetComboBoxButtonRect( ControlType nType,
                                                   ControlPart nPart,
                                                   Rectangle aAreaRect )
{
    (void)nType;
    (void)nPart;
    Rectangle    aButtonRect;
    gint        nArrowWidth;
    gint        nButtonWidth;
    gint        nFocusWidth;
    gint        nFocusPad;
    GtkBorder   padding;

    // Grab some button style attributes
    gtk_style_context_get_style( mpButtonStyle,
                                 "focus-line-width", &nFocusWidth,
                                 "focus-padding", &nFocusPad,
                                 NULL );
    gtk_style_context_get_padding( mpButtonStyle, GTK_STATE_FLAG_NORMAL, &padding);

    nArrowWidth = ARROW_SIZE;
    nButtonWidth = nArrowWidth + padding.left + padding.right + (2 * (nFocusWidth+nFocusPad));
    if( nPart == PART_BUTTON_DOWN )
    {
        aButtonRect.SetSize( Size( nButtonWidth, aAreaRect.GetHeight() ) );
        aButtonRect.SetPos( Point( aAreaRect.Left() + aAreaRect.GetWidth() - nButtonWidth,
                                   aAreaRect.Top() ) );
    }
    else if( nPart == PART_SUB_EDIT )
    {
        gint adjust_left = padding.left + nFocusWidth + nFocusPad;
        gint adjust_top = padding.top + nFocusWidth + nFocusPad;
        gint adjust_right = padding.right + nFocusWidth + nFocusPad;
        gint adjust_bottom = padding.bottom + nFocusWidth + nFocusPad;

        aButtonRect.SetSize( Size( aAreaRect.GetWidth() - nButtonWidth - (adjust_left + adjust_right),
                                   aAreaRect.GetHeight() - (adjust_top + adjust_bottom)) );
        Point aEditPos = aAreaRect.TopLeft();
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
            gtk_render_background(mpComboboxStyle, cr,
                                  0, 0,
                                  areaRect.GetWidth(), areaRect.GetHeight());
            gtk_render_frame(mpComboboxStyle, cr,
                             0, 0,
                             areaRect.GetWidth(), areaRect.GetHeight());

            gtk_render_background(mpComboboxEntryStyle, cr,
                                  0, 0,
                                  aEditBoxRect.GetWidth(), aEditBoxRect.GetHeight() );
            gtk_render_frame(mpComboboxEntryStyle, cr,
                             0, 0,
                             aEditBoxRect.GetWidth(), aEditBoxRect.GetHeight() );
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

void GtkSalGraphics::PaintCheckOrRadio(GtkStyleContext *context,
                                       cairo_t *cr,
                                       const Rectangle& rControlRectangle,
                                       ControlType nType)
{
    gint x, y, indicator_size;
    gtk_style_context_get_style(mpCheckButtonStyle,
                                "indicator-size", &indicator_size,
                                NULL );

    x = (rControlRectangle.GetWidth() - indicator_size) / 2;
    y = (rControlRectangle.GetHeight() - indicator_size) / 2;
    if (nType == CTRL_CHECKBOX)
        gtk_render_check(context, cr, x, y, indicator_size, indicator_size);
    else if (nType == CTRL_RADIOBUTTON)
        gtk_render_option(context, cr, x, y, indicator_size, indicator_size);
}

#if GTK_CHECK_VERSION(3,14,0)
#   define CHECKED_AND_ACTIVE (GTK_STATE_FLAG_CHECKED | GTK_STATE_FLAG_ACTIVE)
#else
#   define CHECKED_AND_ACTIVE GTK_STATE_FLAG_ACTIVE
#endif

bool GtkSalGraphics::drawNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                            ControlState nState, const ImplControlValue& rValue,
                                            const OUString& )
{
    GtkStateFlags flags;
    GtkShadowType shadow;
    gint renderType = RENDER_BACKGROUND_AND_FRAME;
    GtkStyleContext *context = NULL;
    const gchar *styleClass = NULL;

    NWConvertVCLStateToGTKState(nState, &flags, &shadow);

    switch(nType)
    {
    case CTRL_SPINBOX:
        switch (nPart)
        {
        case PART_ENTIRE_CONTROL:
            context = mpSpinStyle;
            renderType = RENDER_SPINBUTTON;
            break;
        }
        break;
    case CTRL_EDITBOX:
        context = mpEntryStyle;
        break;
    case CTRL_COMBOBOX:
        context = mpComboboxStyle;
        renderType = RENDER_COMBOBOX;
        break;
    case CTRL_LISTBOX:
        context = mpListboxStyle;
        renderType = RENDER_COMBOBOX;
        break;
    case CTRL_MENU_POPUP:

        // map selected menu entries in vcl parlance to gtk prelight
        if (nPart >= PART_MENU_ITEM && nPart <= PART_MENU_SUBMENU_ARROW && (nState & ControlState::SELECTED))
            flags = (GtkStateFlags) (flags | GTK_STATE_FLAG_PRELIGHT);

        switch(nPart)
        {
        case PART_MENU_ITEM:
            styleClass = GTK_STYLE_CLASS_MENUITEM;
            context = mpMenuItemStyle;
            renderType = RENDER_BACKGROUND_AND_FRAME;
            break;
        case PART_MENU_ITEM_CHECK_MARK:
            styleClass = GTK_STYLE_CLASS_CHECK;
            context = mpMenuItemStyle;
            renderType = RENDER_CHECK;
            break;
        case PART_MENU_ITEM_RADIO_MARK:
            styleClass = GTK_STYLE_CLASS_RADIO;
            context = mpMenuItemStyle;
            renderType = RENDER_RADIO;
            break;
        case PART_MENU_SEPARATOR:
            styleClass = GTK_STYLE_CLASS_SEPARATOR;
            context = mpMenuItemStyle;
            renderType = RENDER_MENU_SEPERATOR;
            break;
        case PART_MENU_SUBMENU_ARROW:
            context = mpMenuStyle;
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
    case CTRL_CHECKBOX:
        flags = (GtkStateFlags)(flags |
                ( (rValue.getTristateVal() == BUTTONVALUE_ON) ? CHECKED_AND_ACTIVE :
                  (rValue.getTristateVal() == BUTTONVALUE_MIXED) ? GTK_STATE_FLAG_INCONSISTENT :
                  GTK_STATE_FLAG_NORMAL));
        context = mpCheckButtonStyle;
        styleClass = GTK_STYLE_CLASS_CHECK;
        renderType = RENDER_CHECK;
        break;
    case CTRL_RADIOBUTTON:
        flags = (GtkStateFlags)(flags |
                ( (rValue.getTristateVal() == BUTTONVALUE_ON) ? CHECKED_AND_ACTIVE : GTK_STATE_FLAG_NORMAL));
        context = mpCheckButtonStyle;
        styleClass = GTK_STYLE_CLASS_RADIO;
        renderType = RENDER_RADIO;
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
    case CTRL_TAB_BODY:
        return true;
        break;
    case CTRL_TAB_PANE:
        context = mpNoteBookStyle;
        break;
    case CTRL_TAB_HEADER:
        context = mpNoteBookStyle;
        styleClass = GTK_STYLE_CLASS_HEADER;
        break;
    case CTRL_TAB_ITEM:
        context = mpNoteBookStyle;
        if (nState & ControlState::SELECTED)
            flags = (GtkStateFlags) (flags | GTK_STATE_FLAG_ACTIVE);

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
    default:
        return false;
    }

    cairo_t *cr = getCairoContext();
    clipRegion(cr);
    cairo_translate(cr, rControlRegion.Left(), rControlRegion.Top());

    long nX = 0;
    long nY = 0;
    long nWidth = rControlRegion.GetWidth();
    long nHeight = rControlRegion.GetHeight();

    gtk_style_context_save(context);
    gtk_style_context_set_state(context, flags);
    if (nType == CTRL_TAB_ITEM)
    {

        const TabitemValue& rTabitemValue = static_cast<const TabitemValue&>(rValue);

        GtkRegionFlags eFlags(GTK_REGION_EVEN);
        if (rTabitemValue.isFirst() && rTabitemValue.isLast())
            eFlags = GTK_REGION_ONLY;
        else if (rTabitemValue.isFirst())
            eFlags = GTK_REGION_FIRST;
        else if (rTabitemValue.isLast())
            eFlags = GTK_REGION_LAST;

        gtk_style_context_add_region(mpNoteBookStyle, GTK_STYLE_REGION_TAB, eFlags);
        gtk_style_context_add_class(context, GTK_STYLE_CLASS_TOP);

        gint initial_gap(0);
        gtk_style_context_get_style(mpNoteBookStyle,
                                "initial-gap", &initial_gap,
                                NULL);

        nX += initial_gap/2;
        nWidth -= initial_gap;
    }
    if (styleClass)
    {
        gtk_style_context_add_class(context, styleClass);
    }
    if (nType == CTRL_TAB_HEADER || nType == CTRL_TAB_PANE)
    {
        gtk_style_context_add_class(context, GTK_STYLE_CLASS_FRAME);
    }

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
    case RENDER_EXTENSION:
        gtk_render_extension(context, cr, nX, nY, nWidth, nHeight, GTK_POS_BOTTOM);
        break;
    case RENDER_CHECK:
    case RENDER_RADIO:
        PaintCheckOrRadio(context, cr, rControlRegion, nType);
        break;
    case RENDER_MENU_SEPERATOR:
        gtk_render_line(context, cr,
                        3, rControlRegion.GetHeight() / 2,
                        rControlRegion.GetWidth() - 3, rControlRegion.GetHeight() / 2);
        break;
    case RENDER_TOOLBAR_SEPERATOR:
        gtk_render_line(context, cr,
                        rControlRegion.GetWidth() / 2, rControlRegion.GetHeight() * 0.2,
                        rControlRegion.GetWidth() / 2, rControlRegion.GetHeight() * 0.8 );
        break;
    case RENDER_ARROW:
        gtk_render_arrow(context, cr,
                         G_PI / 2, 0, 0,
                         MIN(rControlRegion.GetWidth(), 1 + rControlRegion.GetHeight()));
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
    default:
        break;
    }

    gtk_style_context_restore(context);
    cairo_surface_flush(cairo_get_target(cr));
    cairo_destroy(cr); // unref
    mpFrame->damaged(basegfx::B2IBox(rControlRegion.Left(), rControlRegion.Top(), rControlRegion.Right(), rControlRegion.Bottom()));

    return true;
}

Rectangle GetWidgetSize(const Rectangle& rControlRegion, GtkWidget* widget)
{
    GtkRequisition aReq;
    gtk_widget_get_preferred_size(widget, NULL, &aReq);
    long nHeight = (rControlRegion.GetHeight() > aReq.height) ? rControlRegion.GetHeight() : aReq.height;
    return Rectangle(rControlRegion.TopLeft(), Size(rControlRegion.GetWidth(), nHeight));
}

Rectangle AdjustRectForTextBordersPadding(GtkStyleContext* pStyle, long nContentWidth, long nContentHeight, const Rectangle& rControlRegion)
{
    gtk_style_context_save(pStyle);

    GtkBorder border;
    gtk_style_context_get_border(pStyle, GTK_STATE_FLAG_NORMAL, &border);

    GtkBorder padding;
    gtk_style_context_get_padding(pStyle, GTK_STATE_FLAG_NORMAL, &padding);

    gint nWidgetHeight = nContentHeight + padding.top + padding.bottom + border.top + border.bottom;
    nWidgetHeight = std::max<gint>(nWidgetHeight, rControlRegion.GetHeight());

    gint nWidgetWidth = nContentWidth + padding.left + padding.right + border.left + border.right;
    nWidgetWidth = std::max<gint>(nWidgetWidth, rControlRegion.GetWidth());

    Rectangle aEditRect(rControlRegion.TopLeft(), Size(nWidgetWidth, nWidgetHeight));

    gtk_style_context_restore(pStyle);

    return aEditRect;
}

static GtkWidget* gCacheWindow;
static GtkWidget* gDumbContainer;
static GtkWidget* gSpinBox;
static GtkWidget* gEntryBox;
static GtkWidget* gComboBox;
static GtkWidget* gComboBoxButtonWidget;
static GtkWidget* gComboBoxEntryWidget;
static GtkWidget* gListBox;
static GtkWidget* gListBoxButtonWidget;
static GtkWidget* gFrameIn;
static GtkWidget* gFrameOut;

bool GtkSalGraphics::getNativeControlRegion( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion, ControlState,
                                                const ImplControlValue& rValue, const OUString&,
                                                Rectangle &rNativeBoundingRegion, Rectangle &rNativeContentRegion )
{
    /* TODO: all this funcions needs improvements */
    Rectangle aEditRect = rControlRegion;
    gint indicator_size, indicator_spacing, point;

    if(((nType == CTRL_CHECKBOX) || (nType == CTRL_RADIOBUTTON)) &&
       nPart == PART_ENTIRE_CONTROL)
    {
        gtk_style_context_get_style( mpCheckButtonStyle,
                                     "indicator-size", &indicator_size,
                                     "indicator-spacing", &indicator_spacing,
                                     (char *)NULL );

        gint size = indicator_size + indicator_spacing*2;

        point = MAX(0, rControlRegion.GetHeight() - size);
        aEditRect = Rectangle( Point( 0, point / 2),
                               Size( size, size ) );
    }
    else if( nType == CTRL_MENU_POPUP)
    {
        if (((nPart == PART_MENU_ITEM_CHECK_MARK) ||
              (nPart == PART_MENU_ITEM_RADIO_MARK) ))
        {
            indicator_size = 0;

            gtk_style_context_get_style( mpMenuItemStyle,
                                         "indicator-size", &indicator_size,
                                         (char *)NULL );

            point = MAX(0, rControlRegion.GetHeight() - indicator_size);
            aEditRect = Rectangle( Point( 0, point / 2),
                                   Size( indicator_size, indicator_size ) );
        }
        else if (nPart == PART_MENU_SEPARATOR)
        {
            gint separator_height, separator_width, wide_separators;

            gtk_style_context_get_style (mpMenuItemStyle,
                                         "wide-separators",  &wide_separators,
                                         "separator-width",  &separator_width,
                                         "separator-height", &separator_height,
                                         NULL);

            aEditRect = Rectangle( aEditRect.TopLeft(),
                                   Size( aEditRect.GetWidth(), wide_separators ? separator_height : 1 ) );
        }
        else if (nPart == PART_MENU_SUBMENU_ARROW)
        {
            gfloat arrow_scaling, arrow_size;

            arrow_scaling = 0;
            gtk_style_context_get_style (mpMenuStyle,
                                         "arrow-scaling",  &arrow_scaling,
                                         NULL);

            arrow_size = 11 * arrow_scaling;
            aEditRect = Rectangle( aEditRect.TopLeft(),
                                   Size( arrow_size, arrow_size ) );
        }
    }
    else if ( (nType==CTRL_SCROLLBAR) &&
              ((nPart==PART_BUTTON_LEFT) || (nPart==PART_BUTTON_RIGHT) ||
               (nPart==PART_BUTTON_UP) || (nPart==PART_BUTTON_DOWN)  ) )
    {
        aEditRect = NWGetScrollButtonRect( nPart, rControlRegion );
        if (!aEditRect.GetWidth())
            aEditRect.Right() = aEditRect.Left() + 1;
        if (!aEditRect.GetHeight())
            aEditRect.Bottom() = aEditRect.Top() + 1;
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
        gtk_style_context_save(mpNoteBookStyle);

        gtk_style_context_add_region(mpNoteBookStyle, GTK_STYLE_REGION_TAB, GTK_REGION_ONLY);
        gtk_style_context_add_class(mpNoteBookStyle, GTK_STYLE_CLASS_TOP);

        const TabitemValue& rTabitemValue = static_cast<const TabitemValue&>(rValue);
        const Rectangle& rTabitemRect = rTabitemValue.getContentRect();

        aEditRect = AdjustRectForTextBordersPadding(mpNoteBookStyle, rTabitemRect.GetWidth(),
                                                    rTabitemRect.GetHeight(), rControlRegion);

        gtk_style_context_restore(mpNoteBookStyle);
    }
    else if (nType == CTRL_FRAME && nPart == PART_BORDER)
    {
        aEditRect = rControlRegion;
        DrawFrameFlags nStyle = static_cast<DrawFrameFlags>(rValue.getNumericVal() & 0xfff0);
        if (nStyle & DrawFrameFlags::NoDraw)
        {
            gtk_style_context_save(mpFrameInStyle);
            gtk_style_context_add_class(mpFrameInStyle, GTK_STYLE_CLASS_FRAME);

            GtkBorder padding;
            gtk_style_context_get_padding(mpFrameInStyle, GTK_STATE_FLAG_NORMAL, &padding);

            int x1 = aEditRect.Left();
            int y1 = aEditRect.Top();
            int x2 = aEditRect.Right();
            int y2 = aEditRect.Bottom();

            rNativeBoundingRegion = aEditRect;
            rNativeContentRegion = Rectangle(x1 + padding.left,
                                             y1 + padding.top,
                                             x2 - padding.right,
                                             y2 - padding.bottom);

            gtk_style_context_restore(mpFrameInStyle);
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

static inline ::Color getColorFromColor( const GdkColor& rCol )
{
    return ::Color( rCol.red >> 8, rCol.green >> 8, rCol.blue >> 8 );
}

void GtkSalGraphics::updateSettings( AllSettings& rSettings )
{
    GdkScreen* pScreen = gtk_widget_get_screen( mpWindow );
    GtkStyleContext* pStyle = gtk_widget_get_style_context( mpWindow );
    GtkSettings* pSettings = gtk_widget_get_settings( mpWindow );
    StyleSettings aStyleSet = rSettings.GetStyleSettings();
    GdkRGBA color;

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

    // mouse over text colors
    gtk_style_context_get_color(pStyle, GTK_STATE_FLAG_PRELIGHT, &text_color);
    aTextColor = getColor( text_color );
    aStyleSet.SetButtonRolloverTextColor( aTextColor );
    aStyleSet.SetFieldRolloverTextColor( aTextColor );

    // background colors
    GdkRGBA background_color;
    gtk_style_context_get_background_color(pStyle, GTK_STATE_FLAG_NORMAL, &background_color);

    ::Color aBackColor = getColor( background_color );
    aStyleSet.Set3DColors( aBackColor );
    aStyleSet.SetFaceColor( aBackColor );
    aStyleSet.SetDialogColor( aBackColor );
    aStyleSet.SetWorkspaceColor( aBackColor );
    aStyleSet.SetCheckedColorSpecialCase( );

    // tooltip colors
    GdkRGBA tooltip_bg_color, tooltip_fg_color;
    gtk_style_context_save (pStyle);
    gtk_style_context_add_class (pStyle, GTK_STYLE_CLASS_TOOLTIP);
    gtk_style_context_get_color (pStyle, GTK_STATE_FLAG_NORMAL, &tooltip_fg_color);
    gtk_style_context_get_background_color (pStyle, GTK_STATE_FLAG_NORMAL, &tooltip_bg_color);
    gtk_style_context_restore (pStyle);

    aStyleSet.SetHelpColor( getColor( tooltip_bg_color ));
    aStyleSet.SetHelpTextColor( getColor( tooltip_fg_color ));

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
        gtk_style_context_get_background_color(pCStyle, GTK_STATE_FLAG_SELECTED, &text_color);
        ::Color aHighlightColor = getColor( text_color );
        gtk_style_context_get_color(pCStyle, GTK_STATE_FLAG_SELECTED, &text_color);
        ::Color aHighlightTextColor = getColor( text_color );
        aStyleSet.SetHighlightColor( aHighlightColor );
        aStyleSet.SetHighlightTextColor( aHighlightTextColor );

        // field background color
        GdkRGBA field_background_color;
        gtk_style_context_get_background_color(pCStyle, GTK_STATE_FLAG_NORMAL, &field_background_color);
        g_object_unref( pCStyle );

        ::Color aBackFieldColor = getColor( field_background_color );
        aStyleSet.SetFieldColor( aBackFieldColor );
        // This baby is the default page/paper color
        aStyleSet.SetWindowColor( aBackFieldColor );
    }

    // menu disabled entries handling
    aStyleSet.SetSkipDisabledInMenus( true );
    aStyleSet.SetAcceleratorsInContextMenus( false );

    // menu colors
    gtk_style_context_get_background_color( mpMenuStyle, GTK_STATE_FLAG_NORMAL, &background_color );
    aBackColor = getColor( background_color );
    aStyleSet.SetMenuColor( aBackColor );

    // menu bar
    gtk_style_context_get_background_color( mpMenuBarStyle, GTK_STATE_FLAG_NORMAL, &background_color );
    aBackColor = getColor( background_color );
    aStyleSet.SetMenuBarColor( aBackColor );
    aStyleSet.SetMenuBarRolloverColor( aBackColor );

    gtk_style_context_get_color( mpMenuBarStyle, GTK_STATE_FLAG_NORMAL, &text_color );
    aTextColor = aStyleSet.GetPersonaMenuBarTextColor().get_value_or( getColor( text_color ) );
    aStyleSet.SetMenuBarTextColor( aTextColor );
    aStyleSet.SetMenuBarRolloverTextColor( aTextColor );

    // menu items
    gtk_style_context_get_color( mpMenuStyle, GTK_STATE_FLAG_NORMAL, &color );
    aTextColor = getColor( color );
    aStyleSet.SetMenuTextColor( aTextColor );

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

    gtk_style_context_get_background_color( mpMenuItemStyle, GTK_STATE_FLAG_PRELIGHT, &background_color );
    ::Color aHighlightColor = getColor( background_color );

    gtk_style_context_get_color( mpMenuItemStyle, GTK_STATE_FLAG_PRELIGHT, &color );
    ::Color aHighlightTextColor = getColor( color );
    if( aHighlightColor == aHighlightTextColor )
        aHighlightTextColor = (aHighlightColor.GetLuminance() < 128) ? ::Color( COL_WHITE ) : ::Color( COL_BLACK );
    aStyleSet.SetMenuHighlightColor( aHighlightColor );
    aStyleSet.SetMenuHighlightTextColor( aHighlightTextColor );

    // hyperlink colors
    GdkColor *link_color = NULL;
    gtk_style_context_get_style(pStyle,
                                 "link-color", &link_color,
                                 NULL);
    if (link_color) {
        aStyleSet.SetLinkColor(getColorFromColor(*link_color));
        gdk_color_free(link_color);
    }

    link_color = NULL;
    gtk_style_context_get_style(pStyle,
                                "visited-link-color", &link_color,
                                NULL);
    if (link_color) {
        aStyleSet.SetVisitedLinkColor(getColorFromColor(*link_color));
        gdk_color_free(link_color);
    }

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
#else
        gtk_widget_path_iter_set_name(pCPath, pos, "first tab label");
#endif
        pCStyle = gtk_style_context_new();
        gtk_style_context_set_path(pCStyle, pCPath);
        gtk_widget_path_free(pCPath);

        gtk_style_context_get_color(pCStyle, GTK_STATE_FLAG_NORMAL, &text_color);
        aTextColor = getColor( text_color );
        aStyleSet.SetTabTextColor(aTextColor);

        // mouse over text colors
        gtk_style_context_add_class(pCStyle, "prelight-page");
        gtk_style_context_get_color(pCStyle, GTK_STATE_FLAG_PRELIGHT, &text_color);
        gtk_style_context_remove_class(pCStyle, "prelight-page");
        aTextColor = getColor( text_color );
        aStyleSet.SetTabRolloverTextColor(aTextColor);

        gtk_style_context_add_class(pCStyle, "active-page");
        gtk_style_context_get_color(pCStyle, GTK_STATE_FLAG_ACTIVE, &text_color);
        gtk_style_context_remove_class(pCStyle, "active-page");
        aTextColor = getColor( text_color );
        aStyleSet.SetTabHighlightTextColor(aTextColor);

        g_object_unref( pCStyle );
    }

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
    // get cursor blink time
    gboolean blink = false;

    g_object_get( pSettings, "gtk-cursor-blink", &blink, (char *)NULL );
    if( blink )
    {
        gint blink_time = static_cast<gint>(STYLE_CURSOR_NOBLINKTIME);
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

    gboolean showmenuicons = true, primarybuttonwarps = false;
    g_object_get( pSettings,
        "gtk-menu-images", &showmenuicons,
        "gtk-primary-button-warps-slider", &primarybuttonwarps,
        (char *)NULL );
    aStyleSet.SetPreferredUseImagesInMenus(showmenuicons);
    aStyleSet.SetPrimaryButtonWarpsSlider(primarybuttonwarps);

    // set scrollbar settings
    gint slider_width = 14;
    gint trough_border = 1;
    gint min_slider_length = 21;

    // Grab some button style attributes
    gtk_style_context_get_style( mpVScrollbarStyle,
                                 "slider-width", &slider_width,
                                 "trough-border", &trough_border,
                                 "min-slider-length", &min_slider_length,
                                 (char *)NULL );
    gint magic = trough_border ? 1 : 0;
    aStyleSet.SetScrollBarSize( slider_width + 2*trough_border );
    aStyleSet.SetMinThumbSize( min_slider_length - magic );

    // preferred icon style
    gchar* pIconThemeName = NULL;
    g_object_get( pSettings, "gtk-icon-theme-name", &pIconThemeName, (char *)NULL );
    aStyleSet.SetPreferredIconTheme( OUString::createFromAscii( pIconThemeName ) );
    g_free( pIconThemeName );

    aStyleSet.SetToolbarIconSize( STYLE_TOOLBAR_ICONSIZE_LARGE );

    const cairo_font_options_t* pNewOptions = gdk_screen_get_font_options(pScreen);
    aStyleSet.SetCairoFontOptions( pNewOptions );
    // finally update the collected settings
    rSettings.SetStyleSettings( aStyleSet );
    gchar* pThemeName = NULL;
    g_object_get( pSettings, "gtk-theme-name", &pThemeName, (char *)NULL );
    #if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "Theme name is \"%s\"\n", pThemeName );
    #endif

}

bool GtkSalGraphics::IsNativeControlSupported( ControlType nType, ControlPart nPart )
{
    switch(nType)
    {
        case CTRL_PUSHBUTTON:
        case CTRL_RADIOBUTTON:
        case CTRL_CHECKBOX:
//        case CTRL_TOOLTIP:
//        case CTRL_PROGRESS:
//        case CTRL_LISTNODE:
//        case CTRL_LISTNET:
            if(nPart==PART_ENTIRE_CONTROL)
                return true;
            break;

        case CTRL_SCROLLBAR:
            if(nPart==PART_DRAW_BACKGROUND_HORZ || nPart==PART_DRAW_BACKGROUND_VERT ||
               nPart==PART_ENTIRE_CONTROL       || nPart==HAS_THREE_BUTTONS)
                return true;
            break;

        case CTRL_EDITBOX:
//        case CTRL_MULTILINE_EDITBOX:
        case CTRL_COMBOBOX:
            if(nPart==PART_ENTIRE_CONTROL || nPart==HAS_BACKGROUND_TEXTURE || nPart == PART_ALL_BUTTONS)
                return true;
            break;

        case CTRL_SPINBOX:
            if(nPart==PART_ENTIRE_CONTROL || nPart==PART_ALL_BUTTONS || nPart==HAS_BACKGROUND_TEXTURE)
                return true;
            break;

//        case CTRL_SPINBUTTONS:
//            if(nPart==PART_ENTIRE_CONTROL || nPart==PART_ALL_BUTTONS)
//                return true;
//            break;

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
            if (nPart==PART_ENTIRE_CONTROL || nPart==PART_WINDOW || nPart==HAS_BACKGROUND_TEXTURE)
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

//        case CTRL_MENUBAR:
//            if(nPart==PART_ENTIRE_CONTROL || nPart==PART_MENU_ITEM)
//                return true;
//            break;

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

//        case CTRL_FIXEDLINE:
//            if(nPart == PART_SEPARATOR_VERT || nPart == PART_SEPARATOR_HORZ)
//                return true;
//            break;

//        case CTRL_LISTHEADER:
//            if(nPart == PART_BUTTON || nPart == PART_ARROW)
//                return true;
//            break;
    }

    printf( "Unhandled is native supported for Type: %d, Part %d\n",
            (int)nType, (int)nPart );

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
    pSVData->maNWFData.mbCanDrawWidgetAnySize = true;
    pSVData->maNWFData.mbDDListBoxNoTextArea = true;
}

void GtkData::deInitNWF()
{
    if (gCacheWindow)
        gtk_widget_destroy(gCacheWindow);
}

static void get_combo_box_entry_inner_widgets(GtkWidget *widget, gpointer)
{
    if (GTK_IS_TOGGLE_BUTTON(widget))
    {
        gComboBoxButtonWidget = widget;
    }
    else if (GTK_IS_ENTRY(widget))
    {
        gComboBoxEntryWidget = widget;
    }
}

void get_combo_box_inner_button(GtkWidget *widget, gpointer)
{
    if (GTK_IS_TOGGLE_BUTTON(widget))
    {
        gListBoxButtonWidget = widget;
    }
}

GtkSalGraphics::GtkSalGraphics( GtkSalFrame *pFrame, GtkWidget *pWindow )
    : SvpSalGraphics(),
      mpFrame( pFrame ),
      mpWindow( pWindow )
{
    m_xTextRenderImpl.reset(new GtkCairoTextRender(*this));

    if(style_loaded)
        return;

    GtkWidgetPath* path;

    style_loaded = true;
    gtk_init(NULL, NULL);
    /* Load the GtkStyleContexts, it might be a bit slow, but usually,
     * gtk apps create a lot of widgets at startup, so, it shouldn't be
     * too slow */
    gCacheWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gDumbContainer = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(gCacheWindow), gDumbContainer);
    gtk_widget_realize(gDumbContainer);
    gtk_widget_realize(gCacheWindow);

    gEntryBox = gtk_entry_new();
    getStyleContext(&mpEntryStyle, gEntryBox);
    getStyleContext(&mpButtonStyle, gtk_button_new());

    getStyleContext(&mpToolbarStyle, gtk_toolbar_new());
    gtk_style_context_add_class(mpToolbarStyle, GTK_STYLE_CLASS_PRIMARY_TOOLBAR);
    gtk_style_context_add_class(mpToolbarStyle, GTK_STYLE_CLASS_TOOLBAR);

    getStyleContext(&mpToolbarSeperatorStyle, GTK_WIDGET(gtk_separator_tool_item_new()));

    getStyleContext(&mpToolButtonStyle, gtk_button_new());

    /* Create a widget path for our toolbutton widget */
    path = gtk_widget_path_new();
    gtk_widget_path_append_type(path, GTK_TYPE_TOOLBAR);
    gtk_widget_path_append_type(path, GTK_TYPE_TOOL_BUTTON);
    gtk_widget_path_append_type(path, GTK_TYPE_BUTTON);
    gtk_widget_path_iter_add_class (path, 0, GTK_STYLE_CLASS_PRIMARY_TOOLBAR);
    gtk_widget_path_iter_add_class (path, 1, GTK_STYLE_CLASS_TOOLBAR);
    gtk_widget_path_iter_add_class (path, 2, GTK_STYLE_CLASS_BUTTON);
    gtk_style_context_set_path(mpToolButtonStyle, path);
    gtk_widget_path_free (path);

    getStyleContext(&mpVScrollbarStyle, gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, NULL));
    gtk_style_context_add_class(mpVScrollbarStyle, GTK_STYLE_CLASS_SCROLLBAR);
    getStyleContext(&mpHScrollbarStyle, gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, NULL));
    gtk_style_context_add_class(mpHScrollbarStyle, GTK_STYLE_CLASS_SCROLLBAR);

    getStyleContext(&mpCheckButtonStyle, gtk_check_button_new());

    /* Menu */
    GtkWidget *menu = gtk_menu_new();
    mpMenuStyle = gtk_widget_get_style_context(menu);
    g_object_ref_sink(menu);

    /* Menu Items */
    path = gtk_widget_path_new();
    gtk_widget_path_append_type(path, GTK_TYPE_MENU);
    gtk_widget_path_append_type(path, GTK_TYPE_CHECK_MENU_ITEM);
    gtk_widget_path_iter_add_class(path, 0, GTK_STYLE_CLASS_MENU);
    gtk_widget_path_iter_add_class(path, 1, GTK_STYLE_CLASS_MENUITEM);
    mpMenuItemStyle = gtk_style_context_new();
    gtk_style_context_set_path(mpMenuItemStyle, path);
    gtk_widget_path_free(path);

    /* Menu bar */
    getStyleContext(&mpMenuBarStyle, gtk_menu_bar_new());
    gtk_style_context_add_class(mpMenuBarStyle, GTK_STYLE_CLASS_MENU);

    /* Spinbutton */
    gSpinBox = gtk_spin_button_new(NULL, 0, 0);
    getStyleContext(&mpSpinStyle, gSpinBox);

    /* NoteBook */
    getStyleContext(&mpNoteBookStyle, gtk_notebook_new());

    /* Combobox */
    gComboBox = gtk_combo_box_text_new_with_entry();
    getStyleContext(&mpComboboxStyle, gComboBox);
    /* Get ComboBox Entry and Button */
    gtk_container_forall(GTK_CONTAINER(gComboBox),
                         get_combo_box_entry_inner_widgets,
                         NULL);
    mpComboboxEntryStyle = gtk_widget_get_style_context(gComboBoxEntryWidget);
    mpComboboxButtonStyle = gtk_widget_get_style_context(gComboBoxButtonWidget);

    /* Listbox */
    gListBox = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(gListBox), "sample");
    getStyleContext(&mpListboxStyle, gListBox);
    /* Get ComboBox Button */
    gtk_container_forall(GTK_CONTAINER(gListBox),
                         get_combo_box_inner_button,
                         NULL);
    mpListboxButtonStyle = gtk_widget_get_style_context(gListBoxButtonWidget);

    /* Frames */
    gFrameIn = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(gFrameIn), GTK_SHADOW_IN);

    gFrameOut = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(gFrameOut), GTK_SHADOW_OUT);

    getStyleContext(&mpFrameInStyle, gFrameIn);
    getStyleContext(&mpFrameOutStyle, gFrameOut);

    gtk_widget_show_all(gDumbContainer);
}

cairo_t* GtkSalGraphics::getCairoContext() const
{
    return mpFrame->getCairoContext();
}

void GtkSalGraphics::clipRegion(cairo_t* cr)
{
    if (!m_aClipRegion.IsEmpty())
    {
        RectangleVector aRectangles;
        m_aClipRegion.GetRegionRectangles(aRectangles);
        for (RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
        {
            cairo_rectangle(cr, aRectIter->Left(), aRectIter->Top(), aRectIter->GetWidth(), aRectIter->GetHeight());
        }
        cairo_clip(cr);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
