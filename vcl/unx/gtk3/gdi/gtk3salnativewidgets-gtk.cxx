/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2010 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
 *  (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

// Gross inclusion hacks for now ...

#include "../../../headless/svpgdi.cxx"
#include <unx/gtk/gtkframe.hxx>
#include <unx/gtk/gtkdata.hxx>
#include <unx/gtk/gtkinst.hxx>
#include <unx/gtk/gtkgdi.hxx>
#include <vcl/fontmanager.hxx>

GtkStyleContext* GtkSalGraphics::mpButtonStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpEntryStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpScrollbarStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpToolbarStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpToolButtonStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpCheckButtonStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpMenuBarStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpMenuBarItemStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpMenuStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpMenuItemStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpSpinStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpComboboxStyle = NULL;
GtkStyleContext* GtkSalGraphics::mpListboxStyle = NULL;

bool GtkSalGraphics::style_loaded = false;
/************************************************************************
 * State conversion
 ************************************************************************/
static void NWConvertVCLStateToGTKState( ControlState nVCLState,
            GtkStateFlags* nGTKState, GtkShadowType* nGTKShadow )
{
    *nGTKShadow = GTK_SHADOW_OUT;
    *nGTKState = GTK_STATE_FLAG_NORMAL;

    if (!( nVCLState & CTRL_STATE_ENABLED ))
        *nGTKState = GTK_STATE_FLAG_INSENSITIVE;

    if ( nVCLState & CTRL_STATE_PRESSED )
    {
        *nGTKState = (GtkStateFlags) (*nGTKState | GTK_STATE_FLAG_ACTIVE);
        *nGTKShadow = GTK_SHADOW_IN;
    }

    if ( nVCLState & CTRL_STATE_ROLLOVER )
        *nGTKState = (GtkStateFlags) (*nGTKState | GTK_STATE_FLAG_PRELIGHT);

    if ( nVCLState & CTRL_STATE_SELECTED )
        *nGTKState = (GtkStateFlags) (*nGTKState | GTK_STATE_FLAG_SELECTED);

    if ( nVCLState & CTRL_STATE_FOCUSED )
        *nGTKState = (GtkStateFlags) (*nGTKState | GTK_STATE_FLAG_FOCUSED);
}

enum {
    RENDER_BACKGROUND_AND_FRAME = 1,
    RENDER_CHECK = 2,
    RENDER_BACKGROUND = 3,
    RENDER_LINE = 4,
    RENDER_ARROW = 5,
    RENDER_RADIO = 6,
    RENDER_SCROLLBAR = 7,
    RENDER_SPINBUTTON = 8,
    RENDER_COMBOBOX = 9,
};

static void PrepareComboboxStyle( GtkStyleContext *context,
                                  gboolean forEntry)
{
    GtkWidgetPath *path, *siblingsPath;

    path = gtk_widget_path_new();
    siblingsPath = gtk_widget_path_new();
    gtk_widget_path_append_type(path, GTK_TYPE_COMBO_BOX);
    gtk_widget_path_iter_add_class(path, 0, GTK_STYLE_CLASS_COMBOBOX_ENTRY);

    gtk_widget_path_append_type(siblingsPath, GTK_TYPE_ENTRY);
    gtk_widget_path_append_type(siblingsPath, GTK_TYPE_BUTTON);
    gtk_widget_path_iter_add_class(siblingsPath, 0, GTK_STYLE_CLASS_ENTRY);
    gtk_widget_path_iter_add_class(siblingsPath, 1, GTK_STYLE_CLASS_BUTTON);

    if (forEntry)
    {
        gtk_widget_path_append_with_siblings(path, siblingsPath, 1);
        gtk_widget_path_append_with_siblings(path, siblingsPath, 0);
    }
    else
    {
        gtk_widget_path_append_with_siblings(path, siblingsPath, 0);
        gtk_widget_path_append_with_siblings(path, siblingsPath, 1);
    }

    gtk_style_context_set_path(context, path);
    gtk_widget_path_free(path);
    gtk_widget_path_free(siblingsPath);
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

    return( buttonRect );
}

Rectangle GtkSalGraphics::NWGetScrollButtonRect( ControlPart nPart, Rectangle aAreaRect )
{
    gint slider_width;
    gint stepper_size;
    gint stepper_spacing;
    gint trough_border;

    // Grab some button style attributes
    gtk_style_context_get_style( mpScrollbarStyle,
                                 "slider-width", &slider_width,
                                 "stepper-size", &stepper_size,
                                 "trough-border", &trough_border,
                                 "stepper-spacing", &stepper_spacing, (char *)NULL );

    gboolean has_forward;
    gboolean has_forward2;
    gboolean has_backward;
    gboolean has_backward2;

    gtk_style_context_get_style( mpScrollbarStyle,
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

    return( buttonRect );
}

void GtkSalGraphics::PaintScrollbar(GtkStyleContext *context,
                                    cairo_t *cr,
                                    const Rectangle& rControlRectangle,
                                    ControlType nType,
                                    ControlPart nPart,
                                    const ImplControlValue& aValue )
{
    (void)nType;
    OSL_ASSERT( aValue.getType() == CTRL_SCROLLBAR );
    const ScrollbarValue* pScrollbarVal = static_cast<const ScrollbarValue *>(&aValue);
    Rectangle        scrollbarRect;
    GtkStateFlags    stateFlags;
    GtkShadowType    shadowType;
    GtkOrientation    scrollbarOrientation;
    Rectangle        thumbRect = pScrollbarVal->maThumbRect;
    Rectangle        button11BoundRect = pScrollbarVal->maButton1Rect;   // backward
    Rectangle        button22BoundRect = pScrollbarVal->maButton2Rect;   // forward
    Rectangle        button12BoundRect = pScrollbarVal->maButton1Rect;   // secondary forward
    Rectangle        button21BoundRect = pScrollbarVal->maButton2Rect;   // secondary backward
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
    gtk_style_context_get_style( mpScrollbarStyle,
                                 "slider_width", &slider_width,
                                 "stepper_size", &stepper_size,
                                 "trough_border", &trough_border,
                                 "stepper_spacing", &stepper_spacing,
                                 "min_slider_length", &min_slider_length, (char *)NULL );
    gboolean has_forward;
    gboolean has_forward2;
    gboolean has_backward;
    gboolean has_backward2;

    gtk_style_context_get_style( mpScrollbarStyle,
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

    sal_Bool has_slider = ( thumbRect.GetWidth() > 0 && thumbRect.GetHeight() > 0 );

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
        NWConvertVCLStateToGTKState( pScrollbarVal->mnThumbState, &stateFlags, &shadowType );
        if ( pScrollbarVal->mnThumbState & CTRL_STATE_PRESSED )
            stateFlags = (GtkStateFlags) (stateFlags | GTK_STATE_PRELIGHT);

        gtk_style_context_save(context);
        gtk_style_context_set_state(context, stateFlags);
        gtk_style_context_add_class(context, GTK_STYLE_CLASS_SLIDER);

        gtk_render_slider(context, cr,
                          thumbRect.Left(), thumbRect.Top(),
                          thumbRect.GetWidth(), thumbRect.GetHeight(), scrollbarOrientation);

        gtk_style_context_restore(context);
    }
    // ----------------- BUTTON 1 //
    if ( has_backward )
    {
        NWConvertVCLStateToGTKState( pScrollbarVal->mnButton1State, &stateFlags, &shadowType );

        gtk_style_context_save(context);
        gtk_style_context_set_state(context, stateFlags);
        gtk_style_context_add_class(context, GTK_STYLE_CLASS_BUTTON);

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
        NWConvertVCLStateToGTKState( pScrollbarVal->mnButton2State, &stateFlags, &shadowType );

        gtk_style_context_save(context);
        gtk_style_context_set_state(context, stateFlags);
        gtk_style_context_add_class(context, GTK_STYLE_CLASS_BUTTON);

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
        NWConvertVCLStateToGTKState( pScrollbarVal->mnButton1State, &stateFlags, &shadowType );

        gtk_style_context_save(context);
        gtk_style_context_set_state(context, stateFlags);
        gtk_style_context_add_class(context, GTK_STYLE_CLASS_BUTTON);

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
        NWConvertVCLStateToGTKState( pScrollbarVal->mnButton2State, &stateFlags, &shadowType );

        gtk_style_context_save(context);
        gtk_style_context_set_state(context, stateFlags);
        gtk_style_context_add_class(context, GTK_STYLE_CLASS_BUTTON);

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
                                     const ImplControlValue& aValue )
{
    (void)nPart;
    Rectangle            areaRect;
    GtkShadowType        shadowType;
    const SpinbuttonValue *    pSpinVal = (aValue.getType() == CTRL_SPINBUTTONS) ? static_cast<const SpinbuttonValue *>(&aValue) : NULL;
    Rectangle            upBtnRect;
    ControlPart        upBtnPart = PART_BUTTON_UP;
    ControlState        upBtnState = CTRL_STATE_ENABLED;
    Rectangle            downBtnRect;
    ControlPart        downBtnPart = PART_BUTTON_DOWN;
    ControlState        downBtnState = CTRL_STATE_ENABLED;

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
        gint adjust_x = (gint) ((padding.left + padding.right) / 2) + nFocusWidth + nFocusPad;
        gint adjust_y = (gint) ((padding.top + padding.bottom) / 2) + nFocusWidth + nFocusPad;

        aButtonRect.SetSize( Size( aAreaRect.GetWidth() - nButtonWidth - 2 * adjust_x,
                                   aAreaRect.GetHeight() - 2 * adjust_y ) );
        Point aEditPos = aAreaRect.TopLeft();
        aEditPos.X() += adjust_x;
        aEditPos.Y() += adjust_y;
        aButtonRect.SetPos( aEditPos );
    }

    return( aButtonRect );
}

void GtkSalGraphics::PaintCombobox( GtkStyleContext *context,
                                    cairo_t *cr,
                                    const Rectangle& rControlRectangle,
                                    ControlType nType,
                                    ControlPart nPart,
                                    const ImplControlValue& aValue )
{
    (void)aValue;
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

    if ( nType == CTRL_COMBOBOX )
    {
        if( nPart == PART_ENTIRE_CONTROL )
        {
            PrepareComboboxStyle(context, true);
            gtk_render_background(context, cr,
                                  0, 0,
                                  aEditBoxRect.GetWidth(), aEditBoxRect.GetHeight() );
            gtk_render_frame(context, cr,
                             0, 0,
                             aEditBoxRect.GetWidth(), aEditBoxRect.GetHeight() );
        }

        PrepareComboboxStyle(context, false);
        gtk_render_background(context, cr,
                              (buttonRect.Left() - areaRect.Left()),
                              (buttonRect.Top() - areaRect.Top()),
                              buttonRect.GetWidth(), buttonRect.GetHeight() );
        gtk_render_frame(context, cr,
                         (buttonRect.Left() - areaRect.Left()),
                         (buttonRect.Top() - areaRect.Top()),
                         buttonRect.GetWidth(), buttonRect.GetHeight() );
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
            gtk_render_background(context, cr,
                                  0, 0,
                                  areaRect.GetWidth(), areaRect.GetHeight());
            gtk_render_frame(context, cr,
                             0, 0,
                             areaRect.GetWidth(), areaRect.GetHeight());
        }
    }

    arrowRect.SetSize( Size( (gint)(ARROW_SIZE),
                             (gint)(ARROW_SIZE) ) );
    arrowRect.SetPos( Point( buttonRect.Left() + (gint)((buttonRect.GetWidth() - arrowRect.GetWidth()) / 2),
                             buttonRect.Top() + (gint)((buttonRect.GetHeight() - arrowRect.GetHeight()) / 2) ) );
    gtk_render_arrow(context, cr,
                     G_PI,
                     (arrowRect.Left() - areaRect.Left()), (arrowRect.Top() - areaRect.Top()),
                     arrowRect.GetWidth() );
}

sal_Bool GtkSalGraphics::drawNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                            ControlState nState, const ImplControlValue& aValue,
                                            const rtl::OUString& )
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
        switch (nPart)
        {
        case PART_ENTIRE_CONTROL:
            context = mpListboxStyle;
            renderType = RENDER_COMBOBOX;
            break;
        default:
            return sal_False;
        }
        break;
    case CTRL_MENU_POPUP:
        /* FIXME: missing ENTIRE_CONTROL, as it doesn't seem to work */
        switch(nPart)
        {
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
            renderType = RENDER_LINE;
            break;
        case PART_MENU_SUBMENU_ARROW:
            context = mpMenuStyle;
            renderType = RENDER_ARROW;
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
                    ( (aValue.getTristateVal() == BUTTONVALUE_ON) ? GTK_STATE_FLAG_ACTIVE : GTK_STATE_FLAG_NORMAL));
            context = mpToolButtonStyle;
            break;
        default:
            return sal_False;
        }
        break;
    case CTRL_CHECKBOX:
        flags = (GtkStateFlags)(flags |
                ( (aValue.getTristateVal() == BUTTONVALUE_ON) ? GTK_STATE_FLAG_ACTIVE : GTK_STATE_FLAG_NORMAL));
        context = mpCheckButtonStyle;
        styleClass = GTK_STYLE_CLASS_CHECK;
        renderType = RENDER_CHECK;
        break;
    case CTRL_RADIOBUTTON:
        flags = (GtkStateFlags)(flags |
                ( (aValue.getTristateVal() == BUTTONVALUE_ON) ? GTK_STATE_FLAG_ACTIVE : GTK_STATE_FLAG_NORMAL));
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
            context = mpScrollbarStyle;
            renderType = RENDER_SCROLLBAR;
            break;
        }
        break;
    default:
        return sal_False;
    }

    cairo_rectangle_int_t translatedRegion = { (int) rControlRegion.Left() - 1, (int) rControlRegion.Top() - 1,
                                               (int) rControlRegion.GetWidth() + 2, (int) rControlRegion.GetHeight() + 2 };
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                                          translatedRegion.width, translatedRegion.height);
    cairo_t *cr = cairo_create(surface);
    cairo_surface_destroy(surface); // unref

    gtk_style_context_save(context);
    gtk_style_context_set_state(context, flags);
    if (styleClass)
        gtk_style_context_add_class(context, styleClass);

    cairo_translate(cr, 1, 1);

    switch(renderType)
    {
    case RENDER_BACKGROUND:
    case RENDER_BACKGROUND_AND_FRAME:
        gtk_render_background(context, cr,
                              0, 0,
                              rControlRegion.GetWidth(), rControlRegion.GetHeight());
        if (renderType == RENDER_BACKGROUND_AND_FRAME)
            gtk_render_frame(context, cr,
                             0, 0,
                             rControlRegion.GetWidth(), rControlRegion.GetHeight());
        break;
    case RENDER_CHECK:
        gtk_render_check(context, cr,
                         0, 0,
                         rControlRegion.GetWidth(), rControlRegion.GetHeight());
        break;
    case RENDER_RADIO:
        gtk_render_option(context, cr,
                          0, 0,
                          rControlRegion.GetWidth(), rControlRegion.GetHeight());
        break;
    case RENDER_LINE:
        gtk_render_line(context, cr,
                        3, rControlRegion.GetHeight() / 2,
                        rControlRegion.GetWidth() - 3, rControlRegion.GetHeight() / 2);
        break;
    case RENDER_ARROW:
        gtk_render_arrow(context, cr,
                         G_PI / 2, 0, 0,
                         MIN(rControlRegion.GetWidth(), 1 + rControlRegion.GetHeight()));
        break;
    case RENDER_SCROLLBAR:
        PaintScrollbar(context, cr, rControlRegion, nType, nPart, aValue);
        break;
    case RENDER_SPINBUTTON:
        PaintSpinButton(context, cr, rControlRegion, nType, nPart, aValue);
        break;
    case RENDER_COMBOBOX:
        PaintCombobox(context, cr, rControlRegion, nType, nPart, aValue);
        break;
    default:
        break;
    }

    gtk_style_context_restore(context);

    renderAreaToPix(cr, &translatedRegion);
    cairo_destroy(cr); // unref

    return sal_True;
}

// FIXME: This is incredibly lame ... but so is cairo's insistance on -exactly-
// its own stride - neither more nor less - particularly not more aligned
// we like 8byte aligned, it likes 4 - most odd.
void GtkSalGraphics::renderAreaToPix( cairo_t *cr,
                                      cairo_rectangle_int_t *region)
{
    if( !mpFrame->m_aFrame.get() )
        return;

    basebmp::RawMemorySharedArray data = mpFrame->m_aFrame->getBuffer();
    basegfx::B2IVector size = mpFrame->m_aFrame->getSize();
    sal_Int32 nStride = mpFrame->m_aFrame->getScanlineStride();
    long ax = region->x;
    long ay = region->y;
    long awidth = region->width;
    long aheight = region->height;

    /* Get tje cairo surface and the data */
    cairo_surface_t* surface = cairo_get_target(cr);
    g_assert(surface != NULL);
    cairo_surface_flush(surface);
    unsigned char* cairo_data = cairo_image_surface_get_data(surface);
    g_assert(cairo_data != NULL);
    int cairo_stride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, awidth);

    unsigned char *src = data.get();
    src += (int)ay * nStride + (int)ax * 3;
    awidth = MIN (region->width, size.getX() - ax);
    aheight = MIN (region->height, size.getY() - ay);

    for (int y = 0; y < aheight; ++y)
    {
        for (int x = 0; x < awidth && y < aheight; ++x)
        {
            double alpha = ((float)cairo_data[x*4 + 3])/255.0;
            src[x*3 + 0] = src[x*3 + 0] * (1.0 - alpha) + cairo_data[x*4+0];
            src[x*3 + 1] = src[x*3 + 1] * (1.0 - alpha) + cairo_data[x*4+1];
            src[x*3 + 2] = src[x*3 + 2] * (1.0 - alpha) + cairo_data[x*4+2];
        }
        src += nStride;
        cairo_data += cairo_stride;
    }
    if ( !mpFrame->isDuringRender() )
        gtk_widget_queue_draw_area( mpFrame->getWindow(), ax, ay, awidth, aheight );
}

sal_Bool GtkSalGraphics::getNativeControlRegion( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion, ControlState,
                                                const ImplControlValue&, const rtl::OUString&,
                                                Rectangle &rNativeBoundingRegion, Rectangle &rNativeContentRegion )
{
    /* TODO: all this funcions needs improvements */
    Rectangle aEditRect = rControlRegion;
    gint indicator_size, point;

    if(((nType == CTRL_CHECKBOX) || (nType == CTRL_RADIOBUTTON)) &&
       nPart == PART_ENTIRE_CONTROL)
    {
        gtk_style_context_get_style( mpCheckButtonStyle,
                                     "indicator-size", &indicator_size,
                                     (char *)NULL );

        point = MAX(0, rControlRegion.GetHeight() - indicator_size);
        aEditRect = Rectangle( Point( 0, point / 2),
                               Size( indicator_size, indicator_size ) );
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
        aEditRect = NWGetSpinButtonRect( nPart, rControlRegion );
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
    else
    {
        return sal_False;
    }

    rNativeBoundingRegion = aEditRect;
    rNativeContentRegion = rNativeBoundingRegion;

    return sal_True;
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

    // highlighting colors
    gtk_style_context_get_background_color(pStyle, GTK_STATE_FLAG_SELECTED, &text_color);
    ::Color aHighlightColor = getColor( text_color );
    gtk_style_context_get_color(pStyle, GTK_STATE_FLAG_SELECTED, &text_color);
    ::Color aHighlightTextColor = getColor( text_color );
    aStyleSet.SetHighlightColor( aHighlightColor );
    aStyleSet.SetHighlightTextColor( aHighlightTextColor );

    // tooltip colors
    GdkRGBA tooltip_bg_color, tooltip_fg_color;
    gtk_style_context_save (pStyle);
    gtk_style_context_add_class (pStyle, GTK_STYLE_CLASS_TOOLTIP);
    gtk_style_context_get_color (pStyle, GTK_STATE_FLAG_NORMAL, &tooltip_fg_color);
    gtk_style_context_get_background_color (pStyle, GTK_STATE_FLAG_NORMAL, &tooltip_bg_color);
    gtk_style_context_restore (pStyle);

    aStyleSet.SetHelpColor( getColor( tooltip_bg_color ));
    aStyleSet.SetHelpTextColor( getColor( tooltip_fg_color ));

{ // FIXME: turn me into a helper function ...
    // construct style context for text view
    GtkStyleContext *pCStyle = gtk_style_context_new();
    gtk_style_context_set_screen( pCStyle, gtk_window_get_screen( GTK_WINDOW( mpWindow ) ) );
    GtkWidgetPath *pCPath = gtk_widget_path_new();
    gtk_widget_path_append_type( pCPath, GTK_TYPE_TEXT_VIEW );
    gtk_widget_path_iter_add_class( pCPath, -1, GTK_STYLE_CLASS_VIEW );
    gtk_style_context_set_path( pCStyle, pCPath );
    gtk_widget_path_free( pCPath );
    GdkRGBA field_background_color;
    gtk_style_context_get_background_color(pCStyle, GTK_STATE_FLAG_NORMAL, &field_background_color);
    g_object_unref( pCStyle );

    ::Color aBackFieldColor = getColor( field_background_color );
    aStyleSet.SetFieldColor( aBackFieldColor );
    // This baby is the default page/paper color
    aStyleSet.SetWindowColor( aBackFieldColor );
}

    // menu disabled entries handling
    aStyleSet.SetSkipDisabledInMenus( sal_True );
    aStyleSet.SetAcceleratorsInContextMenus( sal_False );

    // menu colors
    gtk_style_context_get_background_color( mpMenuStyle, GTK_STATE_FLAG_NORMAL, &background_color );
    aBackColor = getColor( background_color );
    aStyleSet.SetMenuColor( aBackColor );

    // menu bar
    gtk_style_context_get_background_color( mpMenuBarStyle, GTK_STATE_FLAG_NORMAL, &background_color );
    aBackColor = getColor( background_color );
    aStyleSet.SetMenuBarColor( aBackColor );

    gtk_style_context_get_color( mpMenuBarStyle, GTK_STATE_FLAG_NORMAL, &text_color );
    aTextColor = getColor( text_color );
    aStyleSet.SetMenuBarTextColor( aTextColor );

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
    aHighlightColor = getColor( background_color );

    gtk_style_context_get_color( mpMenuItemStyle, GTK_STATE_FLAG_PRELIGHT, &color );
    aHighlightTextColor = getColor( color );
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

#if 0

    // Tab colors
    aStyleSet.SetActiveTabColor( aBackFieldColor ); // same as the window color.
    Color aSelectedBackColor = getColor( pStyle->bg[GTK_STATE_ACTIVE] );
    aStyleSet.SetInactiveTabColor( aSelectedBackColor );

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

    fprintf( stderr, "font name BEFORE system match: \"%s\"\n", aFamily.getStr() );
    // match font to e.g. resolve "Sans"
    psp::PrintFontManager::get().matchFont( aInfo, rSettings.GetUILocale() );
    fprintf( stderr, "font match %s, name AFTER: \"%s\"\n",
                  aInfo.m_nID != 0 ? "succeeded" : "failed",
                  rtl::OUStringToOString( aInfo.m_aStyleName, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );

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

    // set scrollbar settings
    gint slider_width = 14;
    gint trough_border = 1;
    gint min_slider_length = 21;

    // Grab some button style attributes
    gtk_style_context_get_style( mpScrollbarStyle,
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
    fprintf( stderr, "Theme name is \"%s\"\n", pThemeName );
    #endif


}

sal_Bool GtkSalGraphics::IsNativeControlSupported( ControlType nType, ControlPart nPart )
{
    if(
       (nType == CTRL_EDITBOX) ||
       (nType == CTRL_PUSHBUTTON && nPart == PART_ENTIRE_CONTROL) ||
       (nType == CTRL_CHECKBOX && nPart == PART_ENTIRE_CONTROL) ||
       (nType == CTRL_RADIOBUTTON && nPart == PART_ENTIRE_CONTROL) |
       (nType == CTRL_TOOLBAR &&
        (nPart == PART_BUTTON || nPart == PART_ENTIRE_CONTROL)) ||
        ((nType == CTRL_SPINBOX) &&
         ((nPart == PART_ENTIRE_CONTROL) || (nPart == PART_ALL_BUTTONS) ||
          (nPart == HAS_BACKGROUND_TEXTURE))) ||
        ((nType == CTRL_COMBOBOX) &&
         ((nPart == PART_ENTIRE_CONTROL) || (nPart == PART_ALL_BUTTONS) ||
          (nPart == HAS_BACKGROUND_TEXTURE))) ||
        ((nType==CTRL_LISTBOX) &&
         ((nPart==PART_ENTIRE_CONTROL) || (nPart == PART_WINDOW) || (nPart == PART_BUTTON_DOWN) ||
          (nPart==HAS_BACKGROUND_TEXTURE))) ||
       ((nType == CTRL_SCROLLBAR) &&
        ( (nPart == PART_DRAW_BACKGROUND_HORZ) || (nPart == PART_DRAW_BACKGROUND_VERT) ||
          (nPart == PART_ENTIRE_CONTROL) || (nPart == HAS_THREE_BUTTONS))) ||
       (nType == CTRL_MENU_POPUP &&
        ((nPart == PART_MENU_ITEM_CHECK_MARK) || (nPart == PART_MENU_ITEM_RADIO_MARK) ||
         (nPart == PART_MENU_SEPARATOR) || (nPart == PART_MENU_SUBMENU_ARROW))))
        return sal_True;

    printf( "Unhandled is native supported for Type: %d, Part %d\n",
            (int)nType, (int)nPart );

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
    if(style_loaded)
        return;

    GtkWidgetPath* path;

    style_loaded = true;
    gtk_init(NULL, NULL);
    /* Load the GtkStyleContexts, it might be a bit slow, but usually,
     * gtk apps create a lot of widgets at startup, so, it shouldn't be
     * too slow */
    getStyleContext(&mpEntryStyle, gtk_entry_new());
    getStyleContext(&mpButtonStyle, gtk_button_new());

    getStyleContext(&mpToolbarStyle, gtk_toolbar_new());
    gtk_style_context_add_class(mpToolbarStyle, GTK_STYLE_CLASS_PRIMARY_TOOLBAR);
    gtk_style_context_add_class(mpToolbarStyle, GTK_STYLE_CLASS_TOOLBAR);

    getStyleContext(&mpToolButtonStyle, gtk_button_new());

    /* Create a widget path for our toolbutton widget */
    path = gtk_widget_path_new();
    gtk_widget_path_append_type(path, GTK_TYPE_TOOLBAR);
    gtk_widget_path_append_type(path, GTK_TYPE_TOOL_BUTTON);
    gtk_widget_path_append_type(path, GTK_TYPE_BUTTON);
    gtk_widget_path_iter_add_class (path, 0, GTK_STYLE_CLASS_PRIMARY_TOOLBAR);
    gtk_widget_path_iter_add_class (path, 0, GTK_STYLE_CLASS_TOOLBAR);
    gtk_widget_path_iter_add_class (path, 2, GTK_STYLE_CLASS_BUTTON);
    gtk_style_context_set_path(mpToolButtonStyle, path);
    gtk_widget_path_free (path);

    getStyleContext(&mpScrollbarStyle, gtk_vscrollbar_new(NULL));
    gtk_style_context_add_class(mpScrollbarStyle, GTK_STYLE_CLASS_SCROLLBAR);

    getStyleContext(&mpCheckButtonStyle, gtk_check_button_new());

    /* Menu */
    getStyleContext(&mpMenuStyle, gtk_menu_new());

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

    /* Menu items in a menu bar */
    path = gtk_widget_path_new();
    gtk_widget_path_append_type(path, GTK_TYPE_MENU_BAR);
    gtk_widget_path_append_type(path, GTK_TYPE_MENU_ITEM);
    gtk_widget_path_iter_add_class(path, 0, GTK_STYLE_CLASS_MENUBAR);
    gtk_widget_path_iter_add_class(path, 1, GTK_STYLE_CLASS_MENUITEM);
    mpMenuBarItemStyle = gtk_style_context_new();
    gtk_style_context_set_path(mpMenuBarItemStyle, path);
    gtk_widget_path_free(path);

    /* Spinbutton */
    getStyleContext(&mpSpinStyle, gtk_spin_button_new(NULL, 0, 0));

    /* Combobox */
    mpComboboxStyle = gtk_style_context_new();
    PrepareComboboxStyle(mpComboboxStyle, true);

    /* Listbox */
    mpListboxStyle = gtk_style_context_new();
    path = gtk_widget_path_new();
    gtk_widget_path_append_type(path, GTK_TYPE_COMBO_BOX);
    gtk_widget_path_append_type(path, GTK_TYPE_BUTTON);
    gtk_widget_path_iter_add_class(path, 1, GTK_STYLE_CLASS_BUTTON);
    gtk_style_context_set_path(mpListboxStyle, path);
    gtk_widget_path_free(path);
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
#ifndef DISABLE_CLEVER_COPYAREA
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
                                            (int)aClipRect.GetWidth(), (int)aClipRect.GetHeight() };
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
#else
    SvpSalGraphics::copyArea( nDestX, nDestY, nSrcX, nSrcY, nSrcWidth, nSrcHeight, nFlags );
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
