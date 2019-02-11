/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <sal/log.hxx>

#include <config_cairo_canvas.h>

#include <basegfx/range/b2ibox.hxx>
#include <unx/gtk/gtkframe.hxx>
#include <unx/gtk/gtkdata.hxx>
#include <unx/gtk/gtkinst.hxx>
#include <unx/gtk/gtkgdi.hxx>
#include <unx/gtk/gtkbackend.hxx>
#include <vcl/decoview.hxx>
#include <vcl/settings.hxx>
#include <unx/fontmanager.hxx>
#include <headless/CustomWidgetDraw.hxx>

#include "cairo_gtk3_cairo.hxx"
#include <boost/optional.hpp>

GtkStyleContext* GtkSalGraphics::mpWindowStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpButtonStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpLinkButtonStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpEntryStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpTextViewStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpVScrollbarStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpVScrollbarContentsStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpVScrollbarTroughStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpVScrollbarSliderStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpVScrollbarButtonStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpHScrollbarStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpHScrollbarContentsStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpHScrollbarTroughStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpHScrollbarSliderStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpHScrollbarButtonStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpToolbarStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpToolButtonStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpToolbarSeperatorStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpCheckButtonStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpCheckButtonCheckStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpRadioButtonStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpRadioButtonRadioStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpSpinStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpSpinEntryStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpSpinUpStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpSpinDownStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpComboboxStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpComboboxBoxStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpComboboxEntryStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpComboboxButtonStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpComboboxButtonBoxStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpComboboxButtonArrowStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpListboxStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpListboxBoxStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpListboxButtonStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpListboxButtonBoxStyle= nullptr;
GtkStyleContext* GtkSalGraphics::mpListboxButtonArrowStyle = nullptr;
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
GtkStyleContext* GtkSalGraphics::mpNotebookHeaderTabsTabLabelStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpNotebookHeaderTabsTabActiveLabelStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpNotebookHeaderTabsTabHoverLabelStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpMenuBarStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpMenuBarItemStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpMenuWindowStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpMenuStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpMenuItemStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpMenuItemArrowStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpMenuItemLabelStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpCheckMenuItemStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpCheckMenuItemCheckStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpRadioMenuItemStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpRadioMenuItemRadioStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpSeparatorMenuItemStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpSeparatorMenuItemSeparatorStyle = nullptr;

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
        nGTKState = static_cast<GtkStateFlags>(nGTKState | GTK_STATE_FLAG_ACTIVE);
    }

    if ( nVCLState & ControlState::ROLLOVER )
    {
        nGTKState = static_cast<GtkStateFlags>(nGTKState | GTK_STATE_FLAG_PRELIGHT);
    }

    if ( nVCLState & ControlState::SELECTED )
        nGTKState = static_cast<GtkStateFlags>(nGTKState | GTK_STATE_FLAG_SELECTED);

    if ( nVCLState & ControlState::FOCUSED )
        nGTKState = static_cast<GtkStateFlags>(nGTKState | GTK_STATE_FLAG_FOCUSED);

    if (AllSettings::GetLayoutRTL())
    {
        nGTKState = static_cast<GtkStateFlags>(nGTKState | GTK_STATE_FLAG_DIR_RTL);
    }
    else
    {
        nGTKState = static_cast<GtkStateFlags>(nGTKState | GTK_STATE_FLAG_DIR_LTR);
    }

    return nGTKState;
}

enum class RenderType {
    BackgroundAndFrame = 1,
    Check,
    Background,
    MenuSeparator,
    ToolbarSeparator,
    Separator,
    Arrow,
    Radio,
    Scrollbar,
    Spinbutton,
    Combobox,
    Expander,
    Icon,
    Progress,
    TabItem,
    Focus
};

static void NWCalcArrowRect( const tools::Rectangle& rButton, tools::Rectangle& rArrow )
{
    // Size the arrow appropriately
    Size aSize( rButton.GetWidth()/2, rButton.GetHeight()/2 );
    rArrow.SetSize( aSize );

    rArrow.SetPos( Point(
        rButton.Left() + ( rButton.GetWidth()  - rArrow.GetWidth()  ) / 2,
        rButton.Top() + ( rButton.GetHeight() - rArrow.GetHeight() ) / 2
        ) );
}

tools::Rectangle GtkSalGraphics::NWGetSpinButtonRect( ControlPart nPart, tools::Rectangle aAreaRect)
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

    tools::Rectangle buttonRect;
    buttonRect.SetSize(Size(buttonWidth, buttonHeight));
    buttonRect.setY(aAreaRect.Top());
    buttonRect.SetBottom( buttonRect.Top() + aAreaRect.GetHeight() );
    tools::Rectangle partRect(buttonRect);
    if ( nPart == ControlPart::ButtonUp )
    {
        if (AllSettings::GetLayoutRTL())
            partRect.setX(aAreaRect.Left());
        else
            partRect.setX(aAreaRect.Left() + (aAreaRect.GetWidth() - buttonRect.GetWidth()));
    }
    else if( nPart == ControlPart::ButtonDown )
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
            partRect.SetRight( aAreaRect.Left() + aAreaRect.GetWidth() );
            partRect.SetLeft( aAreaRect.Left() + (2 * buttonRect.GetWidth()) - 1 );
        }
        else
        {
            partRect.SetRight( (aAreaRect.Left() + (aAreaRect.GetWidth() - 2 * buttonRect.GetWidth())) - 1 );
            partRect.SetLeft( aAreaRect.Left() );
        }
        partRect.SetTop( aAreaRect.Top() );
        partRect.SetBottom( aAreaRect.Bottom() );
    }

    return partRect;
}

namespace
{
    void QuerySize(GtkStyleContext *pContext, Size &rSize)
    {
        GtkBorder margin, border, padding;

        gtk_style_context_get_margin(pContext, gtk_style_context_get_state(pContext), &margin);
        gtk_style_context_get_border(pContext, gtk_style_context_get_state(pContext), &border);
        gtk_style_context_get_padding(pContext, gtk_style_context_get_state(pContext), &padding);

        int nMinWidth, nMinHeight;
        gtk_style_context_get(pContext, gtk_style_context_get_state(pContext),
                "min-width", &nMinWidth, "min-height", &nMinHeight, nullptr);

        nMinWidth += margin.left + margin.right + border.left + border.right + padding.left + padding.right;
        nMinHeight += margin.top + margin.bottom + border.top + border.bottom + padding.top + padding.bottom;

        rSize = Size(std::max<long>(rSize.Width(), nMinWidth), std::max<long>(rSize.Height(), nMinHeight));
    }
}

tools::Rectangle GtkSalGraphics::NWGetScrollButtonRect( ControlPart nPart, tools::Rectangle aAreaRect )
{
    tools::Rectangle  buttonRect;

    gboolean has_forward;
    gboolean has_forward2;
    gboolean has_backward;
    gboolean has_backward2;

    GtkStyleContext* pScrollbarStyle = nullptr;
    if ((nPart == ControlPart::ButtonLeft) || (nPart == ControlPart::ButtonRight))
        pScrollbarStyle = mpHScrollbarStyle;
    else // (nPart == ControlPart::ButtonUp) || (nPart == ControlPart::ButtonDown)
        pScrollbarStyle = mpVScrollbarStyle;

    gtk_style_context_get_style( pScrollbarStyle,
                                 "has-forward-stepper", &has_forward,
                                 "has-secondary-forward-stepper", &has_forward2,
                                 "has-backward-stepper", &has_backward,
                                 "has-secondary-backward-stepper", &has_backward2, nullptr );
    gint       buttonWidth;
    gint       buttonHeight;

    gint nFirst = 0;
    gint nSecond = 0;

    if ( has_forward )   nSecond += 1;
    if ( has_forward2 )  nFirst  += 1;
    if ( has_backward )  nFirst  += 1;
    if ( has_backward2 ) nSecond += 1;

    if (gtk_check_version(3, 20, 0) == nullptr)
    {
        Size aSize;
        if (nPart == ControlPart::ButtonLeft || nPart == ControlPart::ButtonRight)
        {
            QuerySize(mpHScrollbarStyle, aSize);
            QuerySize(mpHScrollbarContentsStyle, aSize);
            QuerySize(mpHScrollbarButtonStyle, aSize);
        }
        else
        {
            QuerySize(mpVScrollbarStyle, aSize);
            QuerySize(mpVScrollbarContentsStyle, aSize);
            QuerySize(mpVScrollbarButtonStyle, aSize);
        }

        if (nPart == ControlPart::ButtonUp)
        {
            aSize.setHeight( aSize.Height() * nFirst );
            buttonRect.setX(aAreaRect.Left());
            buttonRect.setY(aAreaRect.Top());
        }
        else if (nPart == ControlPart::ButtonLeft)
        {
            aSize.setWidth( aSize.Width() * nFirst );
            buttonRect.setX(aAreaRect.Left());
            buttonRect.setY(aAreaRect.Top());
        }
        else if (nPart == ControlPart::ButtonDown)
        {
            aSize.setHeight( aSize.Height() * nSecond );
            buttonRect.setX(aAreaRect.Left());
            buttonRect.setY(aAreaRect.Top() + aAreaRect.GetHeight() - aSize.Height());
        }
        else if (nPart == ControlPart::ButtonRight)
        {
            aSize.setWidth( aSize.Width() * nSecond );
            buttonRect.setX(aAreaRect.Left() + aAreaRect.GetWidth() - aSize.Width());
            buttonRect.setY(aAreaRect.Top());
        }

        buttonRect.SetSize(aSize);

        return buttonRect;
    }

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

    if ( ( nPart == ControlPart::ButtonUp ) || ( nPart == ControlPart::ButtonDown ) )
    {
        buttonWidth = slider_width + 2 * trough_border;
        buttonHeight = stepper_size + trough_border + stepper_spacing;
    }
    else
    {
        buttonWidth = stepper_size + trough_border + stepper_spacing;
        buttonHeight = slider_width + 2 * trough_border;
    }

    if ( nPart == ControlPart::ButtonUp )
    {
        buttonHeight *= nFirst;
        buttonHeight -= 1;
        buttonRect.setX( aAreaRect.Left() );
        buttonRect.setY( aAreaRect.Top() );
    }
    else if ( nPart == ControlPart::ButtonLeft )
    {
        buttonWidth *= nFirst;
        buttonWidth -= 1;
        buttonRect.setX( aAreaRect.Left() );
        buttonRect.setY( aAreaRect.Top() );
    }
    else if ( nPart == ControlPart::ButtonDown )
    {
        buttonHeight *= nSecond;
        buttonRect.setX( aAreaRect.Left() );
        buttonRect.setY( aAreaRect.Top() + aAreaRect.GetHeight() - buttonHeight );
    }
    else if ( nPart == ControlPart::ButtonRight )
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

namespace
{
    void style_context_set_state(GtkStyleContext* context, GtkStateFlags flags)
    {
        do
        {
            gtk_style_context_set_state(context, flags);
        }
        while ((context = gtk_style_context_get_parent(context)));
    }

    class StyleContextSave
    {
    private:
        std::vector<std::pair<GtkStyleContext*, GtkStateFlags>> m_aStates;
    public:
        void save(GtkStyleContext* context)
        {
            do
            {
                m_aStates.emplace_back(context, gtk_style_context_get_state(context));
            }
            while ((context = gtk_style_context_get_parent(context)));
        }
        void restore()
        {
            for (auto a = m_aStates.rbegin(); a != m_aStates.rend(); ++a)
            {
                gtk_style_context_set_state(a->first, a->second);
            }
            m_aStates.clear();
        }
    };

    tools::Rectangle render_common(GtkStyleContext *pContext, cairo_t *cr, const tools::Rectangle &rIn, GtkStateFlags flags)
    {
        if (!pContext)
            return rIn;

        gtk_style_context_set_state(pContext, flags);

        tools::Rectangle aRect(rIn);
        GtkBorder margin;
        gtk_style_context_get_margin(pContext, gtk_style_context_get_state(pContext), &margin);

        aRect.AdjustLeft(margin.left );
        aRect.AdjustTop(margin.top );
        aRect.AdjustRight( -(margin.right) );
        aRect.AdjustBottom( -(margin.bottom) );

        gtk_render_background(pContext, cr, aRect.Left(), aRect.Top(),
                                            aRect.GetWidth(), aRect.GetHeight());
        gtk_render_frame(pContext, cr, aRect.Left(), aRect.Top(),
                                       aRect.GetWidth(), aRect.GetHeight());

        GtkBorder border, padding;
        gtk_style_context_get_border(pContext, gtk_style_context_get_state(pContext), &border);
        gtk_style_context_get_padding(pContext, gtk_style_context_get_state(pContext), &padding);

        aRect.AdjustLeft(border.left + padding.left );
        aRect.AdjustTop(border.top + padding.top );
        aRect.AdjustRight( -(border.right + padding.right) );
        aRect.AdjustBottom( -(border.bottom + padding.bottom) );

        return aRect;
    }
}

void GtkSalGraphics::PaintScrollbar(GtkStyleContext *context,
                                    cairo_t *cr,
                                    const tools::Rectangle& rControlRectangle,
                                    ControlPart nPart,
                                    const ImplControlValue& rValue )
{
    if (gtk_check_version(3, 20, 0) == nullptr)
    {
        assert(rValue.getType() == ControlType::Scrollbar);
        const ScrollbarValue& rScrollbarVal = static_cast<const ScrollbarValue&>(rValue);
        tools::Rectangle        scrollbarRect;
        GtkStateFlags    stateFlags;
        GtkOrientation    scrollbarOrientation;
        tools::Rectangle        thumbRect = rScrollbarVal.maThumbRect;
        tools::Rectangle        button11BoundRect = rScrollbarVal.maButton1Rect;   // backward
        tools::Rectangle        button22BoundRect = rScrollbarVal.maButton2Rect;   // forward
        tools::Rectangle        button12BoundRect = rScrollbarVal.maButton1Rect;   // secondary forward
        tools::Rectangle        button21BoundRect = rScrollbarVal.maButton2Rect;   // secondary backward
        gdouble          arrow1Angle;                                        // backward
        gdouble          arrow2Angle;                                        // forward
        tools::Rectangle        arrowRect;
        gint            slider_width = 0;
        gint            stepper_size = 0;

        // make controlvalue rectangles relative to area
        thumbRect.Move( -rControlRectangle.Left(), -rControlRectangle.Top() );
        button11BoundRect.Move( -rControlRectangle.Left(), -rControlRectangle.Top() );
        button22BoundRect.Move( -rControlRectangle.Left(), -rControlRectangle.Top() );
        button12BoundRect.Move( -rControlRectangle.Left(), -rControlRectangle.Top() );
        button21BoundRect.Move( -rControlRectangle.Left(), -rControlRectangle.Top() );

        // Find the overall bounding rect of the control
        scrollbarRect = rControlRectangle;
        if (scrollbarRect.GetWidth() <= 0 || scrollbarRect.GetHeight() <= 0)
            return;

        gint slider_side;
        Size aSize;
        if (nPart == ControlPart::DrawBackgroundHorz)
        {
            QuerySize(mpHScrollbarStyle, aSize);
            QuerySize(mpHScrollbarContentsStyle, aSize);
            QuerySize(mpHScrollbarTroughStyle, aSize);
            QuerySize(mpHScrollbarSliderStyle, aSize);
            slider_side = aSize.Height();
            gtk_style_context_get(mpHScrollbarButtonStyle,
                                  gtk_style_context_get_state(mpHScrollbarButtonStyle),
                                  "min-height", &slider_width,
                                  "min-width", &stepper_size, nullptr);
        }
        else
        {
            QuerySize(mpVScrollbarStyle, aSize);
            QuerySize(mpVScrollbarContentsStyle, aSize);
            QuerySize(mpVScrollbarTroughStyle, aSize);
            QuerySize(mpVScrollbarSliderStyle, aSize);
            slider_side = aSize.Width();
            gtk_style_context_get(mpVScrollbarButtonStyle,
                                  gtk_style_context_get_state(mpVScrollbarButtonStyle),
                                  "min-width", &slider_width,
                                  "min-height", &stepper_size, nullptr);
        }

        gboolean has_forward;
        gboolean has_forward2;
        gboolean has_backward;
        gboolean has_backward2;

        gtk_style_context_get_style( context,
                                     "has-forward-stepper", &has_forward,
                                     "has-secondary-forward-stepper", &has_forward2,
                                     "has-backward-stepper", &has_backward,
                                     "has-secondary-backward-stepper", &has_backward2, nullptr );

        if ( nPart == ControlPart::DrawBackgroundHorz )
        {
            // Center vertically in the track
            scrollbarRect.Move( 0, (scrollbarRect.GetHeight() - slider_side) / 2 );
            scrollbarRect.SetSize( Size( scrollbarRect.GetWidth(), slider_side ) );
            thumbRect.Move( 0, (scrollbarRect.GetHeight() - slider_side) / 2 );
            thumbRect.SetSize( Size( thumbRect.GetWidth(), slider_side ) );

            scrollbarOrientation = GTK_ORIENTATION_HORIZONTAL;
            arrow1Angle = G_PI * 3 / 2;
            arrow2Angle = G_PI / 2;

            if ( has_backward )
            {
                button12BoundRect.Move( stepper_size,
                                        (scrollbarRect.GetHeight() - slider_width) / 2 );
            }

            button11BoundRect.Move( 0, (scrollbarRect.GetHeight() - slider_width) / 2 );
            button11BoundRect.SetSize( Size( stepper_size, slider_width ) );
            button12BoundRect.SetSize( Size( stepper_size, slider_width ) );

            if ( has_backward2 )
            {
                button22BoundRect.Move( stepper_size, (scrollbarRect.GetHeight() - slider_width) / 2 );
                button21BoundRect.Move( 0, (scrollbarRect.GetHeight() - slider_width) / 2 );
            }
            else
            {
                button22BoundRect.Move( 0, (scrollbarRect.GetHeight() - slider_width) / 2 );
            }

            button21BoundRect.SetSize( Size( stepper_size, slider_width ) );
            button22BoundRect.SetSize( Size( stepper_size, slider_width ) );
        }
        else
        {
            // Center horizontally in the track
            scrollbarRect.Move( (scrollbarRect.GetWidth() - slider_side) / 2, 0 );
            scrollbarRect.SetSize( Size( slider_side, scrollbarRect.GetHeight() ) );
            thumbRect.Move( (scrollbarRect.GetWidth() - slider_side) / 2, 0 );
            thumbRect.SetSize( Size( slider_side, thumbRect.GetHeight() ) );

            scrollbarOrientation = GTK_ORIENTATION_VERTICAL;
            arrow1Angle = 0;
            arrow2Angle = G_PI;

            if ( has_backward )
            {
                button12BoundRect.Move( (scrollbarRect.GetWidth() - slider_width) / 2,
                                        stepper_size );
            }
            button11BoundRect.Move( (scrollbarRect.GetWidth() - slider_width) / 2, 0 );
            button11BoundRect.SetSize( Size( slider_width, stepper_size ) );
            button12BoundRect.SetSize( Size( slider_width, stepper_size ) );

            if ( has_backward2 )
            {
                button22BoundRect.Move( (scrollbarRect.GetWidth() - slider_width) / 2, stepper_size );
                button21BoundRect.Move( (scrollbarRect.GetWidth() - slider_width) / 2, 0 );
            }
            else
            {
                button22BoundRect.Move( (scrollbarRect.GetWidth() - slider_width) / 2, 0 );
            }

            button21BoundRect.SetSize( Size( slider_width, stepper_size ) );
            button22BoundRect.SetSize( Size( slider_width, stepper_size ) );
        }

        bool has_slider = ( thumbRect.GetWidth() > 0 && thumbRect.GetHeight() > 0 );

        // ----------------- CONTENTS
        GtkStyleContext* pScrollbarContentsStyle = scrollbarOrientation == GTK_ORIENTATION_VERTICAL ?
                                                  mpVScrollbarContentsStyle : mpHScrollbarContentsStyle;

        gtk_render_background(gtk_widget_get_style_context(gCacheWindow), cr, 0, 0,
                              scrollbarRect.GetWidth(), scrollbarRect.GetHeight() );

        gtk_render_background(context, cr, 0, 0,
                              scrollbarRect.GetWidth(), scrollbarRect.GetHeight() );
        gtk_render_frame(context, cr, 0, 0,
                         scrollbarRect.GetWidth(), scrollbarRect.GetHeight() );

        gtk_render_background(pScrollbarContentsStyle, cr, 0, 0,
                              scrollbarRect.GetWidth(), scrollbarRect.GetHeight() );
        gtk_render_frame(pScrollbarContentsStyle, cr, 0, 0,
                         scrollbarRect.GetWidth(), scrollbarRect.GetHeight() );

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

        // ----------------- TROUGH
        // trackrect matches that of ScrollBar::ImplCalc
        tools::Rectangle aTrackRect(Point(0, 0), scrollbarRect.GetSize());
        if (nPart == ControlPart::DrawBackgroundHorz)
        {
            tools::Rectangle aBtn1Rect = NWGetScrollButtonRect(ControlPart::ButtonLeft, aTrackRect);
            tools::Rectangle aBtn2Rect = NWGetScrollButtonRect(ControlPart::ButtonRight, aTrackRect);
            aTrackRect.SetLeft( aBtn1Rect.Right() );
            aTrackRect.SetRight( aBtn2Rect.Left() );
        }
        else
        {
            tools::Rectangle aBtn1Rect = NWGetScrollButtonRect(ControlPart::ButtonUp, aTrackRect);
            tools::Rectangle aBtn2Rect = NWGetScrollButtonRect(ControlPart::ButtonDown, aTrackRect);
            aTrackRect.SetTop( aBtn1Rect.Bottom() + 1 );
            aTrackRect.SetBottom( aBtn2Rect.Top() );
        }

        GtkStyleContext* pScrollbarTroughStyle = scrollbarOrientation == GTK_ORIENTATION_VERTICAL ?
                                                  mpVScrollbarTroughStyle : mpHScrollbarTroughStyle;
        gtk_render_background(pScrollbarTroughStyle, cr, aTrackRect.Left(), aTrackRect.Top(),
                              aTrackRect.GetWidth(), aTrackRect.GetHeight() );
        gtk_render_frame(pScrollbarTroughStyle, cr, aTrackRect.Left(), aTrackRect.Top(),
                         aTrackRect.GetWidth(), aTrackRect.GetHeight() );

        // ----------------- THUMB
        if ( has_slider )
        {
            stateFlags = NWConvertVCLStateToGTKState(rScrollbarVal.mnThumbState);
            if ( rScrollbarVal.mnThumbState & ControlState::PRESSED )
                stateFlags = static_cast<GtkStateFlags>(stateFlags | GTK_STATE_PRELIGHT);

            GtkStyleContext* pScrollbarSliderStyle = scrollbarOrientation == GTK_ORIENTATION_VERTICAL ?
                                                      mpVScrollbarSliderStyle : mpHScrollbarSliderStyle;

            gtk_style_context_set_state(pScrollbarSliderStyle, stateFlags);

            GtkBorder margin;
            gtk_style_context_get_margin(pScrollbarSliderStyle, stateFlags, &margin);

            gtk_render_background(pScrollbarSliderStyle, cr,
                              thumbRect.Left() + margin.left, thumbRect.Top() + margin.top,
                              thumbRect.GetWidth() - margin.left - margin.right,
                              thumbRect.GetHeight() - margin.top - margin.bottom);

            gtk_render_frame(pScrollbarSliderStyle, cr,
                              thumbRect.Left() + margin.left, thumbRect.Top() + margin.top,
                              thumbRect.GetWidth() - margin.left - margin.right,
                              thumbRect.GetHeight() - margin.top - margin.bottom);
        }

        return;
    }

    OSL_ASSERT( rValue.getType() == ControlType::Scrollbar );
    const ScrollbarValue& rScrollbarVal = static_cast<const ScrollbarValue&>(rValue);
    tools::Rectangle        scrollbarRect;
    GtkStateFlags    stateFlags;
    GtkOrientation    scrollbarOrientation;
    tools::Rectangle        thumbRect = rScrollbarVal.maThumbRect;
    tools::Rectangle        button11BoundRect = rScrollbarVal.maButton1Rect;   // backward
    tools::Rectangle        button22BoundRect = rScrollbarVal.maButton2Rect;   // forward
    tools::Rectangle        button12BoundRect = rScrollbarVal.maButton1Rect;   // secondary forward
    tools::Rectangle        button21BoundRect = rScrollbarVal.maButton2Rect;   // secondary backward
    gdouble          arrow1Angle;                                        // backward
    gdouble          arrow2Angle;                                        // forward
    tools::Rectangle        arrowRect;
    gint            slider_width = 0;
    gint            stepper_size = 0;
    gint            trough_border = 0;

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
                                 "trough_border", &trough_border, nullptr );
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

    if ( nPart == ControlPart::DrawBackgroundHorz )
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

        thumbRect.SetBottom( thumbRect.Top() + slider_width - 1 );
        // Make sure the thumb is at least the default width (so we don't get tiny thumbs),
        // but if the VCL gives us a size smaller than the theme's default thumb size,
        // honor the VCL size
        thumbRect.AdjustRight(magic );
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

        thumbRect.SetRight( thumbRect.Left() + slider_width - 1 );

        thumbRect.AdjustBottom(magic );
        // Center horizontally in the track
        thumbRect.Move( (scrollbarRect.GetWidth() - slider_width) / 2, 0 );
    }

    bool has_slider = ( thumbRect.GetWidth() > 0 && thumbRect.GetHeight() > 0 );

    // ----------------- CONTENTS
    GtkStyleContext* pScrollbarContentsStyle = scrollbarOrientation == GTK_ORIENTATION_VERTICAL ?
                                              mpVScrollbarContentsStyle : mpHScrollbarContentsStyle;

    gtk_render_background(gtk_widget_get_style_context(gCacheWindow), cr, 0, 0,
                          scrollbarRect.GetWidth(), scrollbarRect.GetHeight() );

    gtk_render_background(context, cr, 0, 0,
                          scrollbarRect.GetWidth(), scrollbarRect.GetHeight() );
    gtk_render_frame(context, cr, 0, 0,
                     scrollbarRect.GetWidth(), scrollbarRect.GetHeight() );

    gtk_render_background(pScrollbarContentsStyle, cr, 0, 0,
                          scrollbarRect.GetWidth(), scrollbarRect.GetHeight() );
    gtk_render_frame(pScrollbarContentsStyle, cr, 0, 0,
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
            stateFlags = static_cast<GtkStateFlags>(stateFlags | GTK_STATE_PRELIGHT);

        GtkStyleContext* pScrollbarSliderStyle = scrollbarOrientation == GTK_ORIENTATION_VERTICAL ?
                                                  mpVScrollbarSliderStyle : mpHScrollbarSliderStyle;

        gtk_style_context_set_state(pScrollbarSliderStyle, stateFlags);

        GtkBorder margin;
        gtk_style_context_get_margin(pScrollbarSliderStyle, stateFlags, &margin);

        gtk_render_background(pScrollbarSliderStyle, cr,
                          thumbRect.Left() + margin.left, thumbRect.Top() + margin.top,
                          thumbRect.GetWidth() - margin.left - margin.right,
                          thumbRect.GetHeight() - margin.top - margin.bottom);

        gtk_render_frame(pScrollbarSliderStyle, cr,
                          thumbRect.Left() + margin.left, thumbRect.Top() + margin.top,
                          thumbRect.GetWidth() - margin.left - margin.right,
                          thumbRect.GetHeight() - margin.top - margin.bottom);
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
                                         ControlPart nPart,
                                         tools::Rectangle aAreaRect,
                                         ControlState nState )
{
    GtkBorder            padding, border;

    GtkStateFlags stateFlags = NWConvertVCLStateToGTKState(nState);
    tools::Rectangle buttonRect = NWGetSpinButtonRect( nPart, aAreaRect );

    gtk_style_context_set_state(context, stateFlags);

    gtk_style_context_get_padding(context, gtk_style_context_get_state(context), &padding);
    gtk_style_context_get_border(context, gtk_style_context_get_state(context), &border);

    gtk_render_background(context, cr,
                          buttonRect.Left(), buttonRect.Top(),
                          buttonRect.GetWidth(), buttonRect.GetHeight() );

    gint iconWidth = buttonRect.GetWidth() - padding.left - padding.right - border.left - border.right;
    gint iconHeight = buttonRect.GetHeight() - padding.top - padding.bottom - border.top - border.bottom;

    const char* icon = (nPart == ControlPart::ButtonUp) ? "list-add-symbolic" : "list-remove-symbolic";
    GtkIconTheme *pIconTheme = gtk_icon_theme_get_for_screen(gtk_widget_get_screen(mpWindow));

    GtkIconInfo *info = gtk_icon_theme_lookup_icon(pIconTheme, icon, std::min(iconWidth, iconHeight),
                                                   static_cast<GtkIconLookupFlags>(0));

    GdkPixbuf *pixbuf = gtk_icon_info_load_symbolic_for_context(info, context, nullptr, nullptr);
    g_object_unref(info);

    iconWidth = gdk_pixbuf_get_width(pixbuf);
    iconHeight = gdk_pixbuf_get_height(pixbuf);
    tools::Rectangle arrowRect;
    arrowRect.SetSize(Size(iconWidth, iconHeight));
    arrowRect.setX( buttonRect.Left() + (buttonRect.GetWidth() - arrowRect.GetWidth()) / 2 );
    arrowRect.setY( buttonRect.Top() + (buttonRect.GetHeight() - arrowRect.GetHeight()) / 2 );

    gtk_render_icon(context, cr, pixbuf, arrowRect.Left(), arrowRect.Top());
    g_object_unref(pixbuf);

    gtk_render_frame(context, cr,
                     buttonRect.Left(), buttonRect.Top(),
                     buttonRect.GetWidth(), buttonRect.GetHeight() );
}

void GtkSalGraphics::PaintSpinButton(GtkStateFlags flags,
                                     cairo_t *cr,
                                     const tools::Rectangle& rControlRectangle,
                                     ControlPart nPart,
                                     const ImplControlValue& rValue )
{
    const SpinbuttonValue *pSpinVal = (rValue.getType() == ControlType::SpinButtons) ? static_cast<const SpinbuttonValue *>(&rValue) : nullptr;
    ControlPart upBtnPart = ControlPart::ButtonUp;
    ControlState upBtnState = ControlState::NONE;
    ControlPart downBtnPart = ControlPart::ButtonDown;
    ControlState downBtnState = ControlState::NONE;

    if ( pSpinVal )
    {
        upBtnPart = pSpinVal->mnUpperPart;
        upBtnState = pSpinVal->mnUpperState;

        downBtnPart = pSpinVal->mnLowerPart;
        downBtnState = pSpinVal->mnLowerState;
    }

    if (nPart == ControlPart::Entire)
    {
        gtk_style_context_set_state(mpWindowStyle, flags);

        gtk_render_background(mpWindowStyle, cr,
                              0, 0,
                              rControlRectangle.GetWidth(), rControlRectangle.GetHeight());

        gtk_style_context_set_state(mpSpinStyle, flags);

        gtk_render_background(mpSpinStyle, cr,
                              0, 0,
                              rControlRectangle.GetWidth(), rControlRectangle.GetHeight());
    }

    cairo_translate(cr, -rControlRectangle.Left(), -rControlRectangle.Top());
    PaintOneSpinButton(mpSpinUpStyle, cr, upBtnPart, rControlRectangle, upBtnState );
    PaintOneSpinButton(mpSpinDownStyle, cr, downBtnPart, rControlRectangle, downBtnState );
    cairo_translate(cr, rControlRectangle.Left(), rControlRectangle.Top());

    if (nPart == ControlPart::Entire)
    {
        gtk_render_frame(mpSpinStyle, cr,
                         0, 0,
                         rControlRectangle.GetWidth(), rControlRectangle.GetHeight() );
    }
}

#define FALLBACK_ARROW_SIZE gint(11 * 0.85)

tools::Rectangle GtkSalGraphics::NWGetComboBoxButtonRect(ControlType nType,
                                                   ControlPart nPart,
                                                   tools::Rectangle aAreaRect )
{
    tools::Rectangle    aButtonRect;

    GtkBorder padding;
    if (nType == ControlType::Listbox)
        gtk_style_context_get_padding(mpListboxButtonStyle, gtk_style_context_get_state(mpListboxButtonStyle), &padding);
    else
        gtk_style_context_get_padding(mpButtonStyle, gtk_style_context_get_state(mpButtonStyle), &padding);

    gint nArrowWidth = FALLBACK_ARROW_SIZE;
    if (gtk_check_version(3, 20, 0) == nullptr)
    {
        gtk_style_context_get(mpComboboxButtonArrowStyle,
            gtk_style_context_get_state(mpComboboxButtonArrowStyle),
            "min-width", &nArrowWidth, nullptr);
    }

    gint nButtonWidth = nArrowWidth + padding.left + padding.right;
    if( nPart == ControlPart::ButtonDown )
    {
        Point aPos = Point(aAreaRect.Left() + aAreaRect.GetWidth() - nButtonWidth, aAreaRect.Top());
        if (AllSettings::GetLayoutRTL())
            aPos.setX( aAreaRect.Left() );
        aButtonRect.SetSize( Size( nButtonWidth, aAreaRect.GetHeight() ) );
        aButtonRect.SetPos(aPos);
    }
    else if( nPart == ControlPart::SubEdit )
    {
        gint adjust_left = padding.left;
        gint adjust_top = padding.top;
        gint adjust_right = padding.right;
        gint adjust_bottom = padding.bottom;

        aButtonRect.SetSize( Size( aAreaRect.GetWidth() - nButtonWidth - (adjust_left + adjust_right),
                                   aAreaRect.GetHeight() - (adjust_top + adjust_bottom)) );
        Point aEditPos = aAreaRect.TopLeft();
        if (AllSettings::GetLayoutRTL())
            aEditPos.AdjustX(nButtonWidth );
        else
            aEditPos.AdjustX(adjust_left );
        aEditPos.AdjustY(adjust_top );
        aButtonRect.SetPos( aEditPos );
    }

    return aButtonRect;
}

void GtkSalGraphics::PaintCombobox( GtkStateFlags flags, cairo_t *cr,
                                    const tools::Rectangle& rControlRectangle,
                                    ControlType nType,
                                    ControlPart nPart )
{
    tools::Rectangle        areaRect;
    tools::Rectangle        buttonRect;
    tools::Rectangle        arrowRect;

    // Find the overall bounding rect of the buttons's drawing area,
    // plus its actual draw rect excluding adornment
    areaRect = rControlRectangle;

    buttonRect = NWGetComboBoxButtonRect(ControlType::Combobox, ControlPart::ButtonDown, areaRect);

    tools::Rectangle        aEditBoxRect( areaRect );
    aEditBoxRect.SetSize( Size( areaRect.GetWidth() - buttonRect.GetWidth(), aEditBoxRect.GetHeight() ) );
    if (AllSettings::GetLayoutRTL())
        aEditBoxRect.SetPos( Point( areaRect.Left() + buttonRect.GetWidth(), areaRect.Top() ) );

    gint arrow_width = FALLBACK_ARROW_SIZE, arrow_height = FALLBACK_ARROW_SIZE;
    if (gtk_check_version(3, 20, 0) == nullptr)
    {
        if (nType == ControlType::Combobox)
        {
            gtk_style_context_get(mpComboboxButtonArrowStyle,
                gtk_style_context_get_state(mpComboboxButtonArrowStyle),
                "min-width", &arrow_width, "min-height", &arrow_height, nullptr);
        }
        else if (nType == ControlType::Listbox)
        {
            gtk_style_context_get(mpListboxButtonArrowStyle,
                gtk_style_context_get_state(mpListboxButtonArrowStyle),
                "min-width", &arrow_width, "min-height", &arrow_height, nullptr);
        }
    }

    arrowRect.SetSize(Size(arrow_width, arrow_height));
    arrowRect.SetPos( Point( buttonRect.Left() + static_cast<gint>((buttonRect.GetWidth() - arrowRect.GetWidth()) / 2),
                             buttonRect.Top() + static_cast<gint>((buttonRect.GetHeight() - arrowRect.GetHeight()) / 2) ) );


    tools::Rectangle aRect(Point(0, 0), Size(areaRect.GetWidth(), areaRect.GetHeight()));

    if (nType == ControlType::Combobox)
    {
        if( nPart == ControlPart::Entire )
        {
            render_common(mpComboboxStyle, cr, aRect, flags);
            render_common(mpComboboxBoxStyle, cr, aRect, flags);
            tools::Rectangle aEntryRect(Point(aEditBoxRect.Left() - areaRect.Left(),
                                 aEditBoxRect.Top() - areaRect.Top()),
                                 Size(aEditBoxRect.GetWidth(), aEditBoxRect.GetHeight()));

            GtkJunctionSides eJuncSides = gtk_style_context_get_junction_sides(mpComboboxEntryStyle);
            if (AllSettings::GetLayoutRTL())
                gtk_style_context_set_junction_sides(mpComboboxEntryStyle, GTK_JUNCTION_LEFT);
            else
                gtk_style_context_set_junction_sides(mpComboboxEntryStyle, GTK_JUNCTION_RIGHT);
            render_common(mpComboboxEntryStyle, cr, aEntryRect, flags);
            gtk_style_context_set_junction_sides(mpComboboxEntryStyle, eJuncSides);
        }

        tools::Rectangle aButtonRect(Point(buttonRect.Left() - areaRect.Left(), buttonRect.Top() - areaRect.Top()),
                              Size(buttonRect.GetWidth(), buttonRect.GetHeight()));
        GtkJunctionSides eJuncSides = gtk_style_context_get_junction_sides(mpComboboxButtonStyle);
        if (AllSettings::GetLayoutRTL())
            gtk_style_context_set_junction_sides(mpComboboxButtonStyle, GTK_JUNCTION_RIGHT);
        else
            gtk_style_context_set_junction_sides(mpComboboxButtonStyle, GTK_JUNCTION_LEFT);
        render_common(mpComboboxButtonStyle, cr, aButtonRect, flags);
        gtk_style_context_set_junction_sides(mpComboboxButtonStyle, eJuncSides);

        gtk_render_arrow(mpComboboxButtonArrowStyle, cr,
                         G_PI,
                         (arrowRect.Left() - areaRect.Left()), (arrowRect.Top() - areaRect.Top()),
                         arrowRect.GetWidth() );
    }
    else if (nType == ControlType::Listbox)
    {
        if( nPart == ControlPart::ListboxWindow )
        {
            /* render the popup window with the menu style */
            gtk_render_frame(mpMenuStyle, cr,
                             0, 0,
                             areaRect.GetWidth(), areaRect.GetHeight());
        }
        else
        {
            render_common(mpListboxStyle, cr, aRect, flags);
            render_common(mpListboxBoxStyle, cr, aRect, flags);

            render_common(mpListboxButtonStyle, cr, aRect, flags);

            gtk_render_arrow(mpListboxButtonArrowStyle, cr,
                             G_PI,
                             (arrowRect.Left() - areaRect.Left()), (arrowRect.Top() - areaRect.Top()),
                             arrowRect.GetWidth() );
        }
    }
}

static void appendComboEntry(GtkWidgetPath* pSiblingsPath, gtk_widget_path_iter_set_object_nameFunc set_object_name)
{
    gtk_widget_path_append_type(pSiblingsPath, GTK_TYPE_ENTRY);
    set_object_name(pSiblingsPath, -1, "entry");
    gtk_widget_path_iter_add_class(pSiblingsPath, -1, "combo");
}

static void appendComboButton(GtkWidgetPath* pSiblingsPath, gtk_widget_path_iter_set_object_nameFunc set_object_name)
{
    gtk_widget_path_append_type(pSiblingsPath, GTK_TYPE_BUTTON);
    set_object_name(pSiblingsPath, -1, "button");
    gtk_widget_path_iter_add_class(pSiblingsPath, -1, "combo");
}

static GtkWidgetPath* buildLTRComboSiblingsPath(gtk_widget_path_iter_set_object_nameFunc set_object_name)
{
    GtkWidgetPath* pSiblingsPath = gtk_widget_path_new();

    appendComboEntry(pSiblingsPath, set_object_name);
    appendComboButton(pSiblingsPath, set_object_name);

    return pSiblingsPath;
}

static GtkWidgetPath* buildRTLComboSiblingsPath(gtk_widget_path_iter_set_object_nameFunc set_object_name)
{
    GtkWidgetPath* pSiblingsPath = gtk_widget_path_new();

    appendComboButton(pSiblingsPath, set_object_name);
    appendComboEntry(pSiblingsPath, set_object_name);

    return pSiblingsPath;
}

GtkStyleContext* GtkSalGraphics::makeContext(GtkWidgetPath *pPath, GtkStyleContext *pParent)
{
    GtkStyleContext* context = gtk_style_context_new();
    gtk_style_context_set_screen(context, gtk_window_get_screen(GTK_WINDOW(mpWindow)));
    gtk_style_context_set_path(context, pPath);
    gtk_style_context_set_parent(context, pParent);
    gtk_widget_path_unref(pPath);
    return context;
}

GtkStyleContext* GtkSalGraphics::createNewContext(GtkControlPart ePart, gtk_widget_path_iter_set_object_nameFunc set_object_name)
{
    switch (ePart)
    {
        case GtkControlPart::ToplevelWindow:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, G_TYPE_NONE);
            set_object_name(path, -1, "window");
            gtk_widget_path_iter_add_class(path, -1, "background");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::Button:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_BUTTON);
            set_object_name(path, -1, "button");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::LinkButton:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_BUTTON);
            set_object_name(path, -1, "button");
            gtk_widget_path_iter_add_class(path, -1, "link");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::CheckButton:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_CHECK_BUTTON);
            set_object_name(path, -1, "checkbutton");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::CheckButtonCheck:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpCheckButtonStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_CHECK_BUTTON);
            set_object_name(path, -1, "check");
            return makeContext(path, mpCheckButtonStyle);
        }
        case GtkControlPart::RadioButton:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_RADIO_BUTTON);
            set_object_name(path, -1, "radiobutton");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::RadioButtonRadio:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpRadioButtonStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_RADIO_BUTTON);
            set_object_name(path, -1, "radio");
            return makeContext(path, mpRadioButtonStyle);
        }
        case GtkControlPart::ComboboxBoxButtonBoxArrow:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpComboboxButtonBoxStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_RADIO_BUTTON);
            gtk_widget_path_append_type(path, GTK_TYPE_BUTTON);
            set_object_name(path, -1, "arrow");
            return makeContext(path, mpComboboxButtonBoxStyle);
        }
        case GtkControlPart::ListboxBoxButtonBoxArrow:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpListboxButtonBoxStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_RADIO_BUTTON);
            gtk_widget_path_append_type(path, GTK_TYPE_BUTTON);
            set_object_name(path, -1, "arrow");
            return makeContext(path, mpListboxButtonBoxStyle);
        }
        case GtkControlPart::Entry:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_ENTRY);
            set_object_name(path, -1, "entry");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::Combobox:
        case GtkControlPart::Listbox:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, G_TYPE_NONE);
            set_object_name(path, -1, "combobox");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::ComboboxBox:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpComboboxStyle));
            gtk_widget_path_append_type(path, G_TYPE_NONE);
            set_object_name(path, -1, "box");
            gtk_widget_path_iter_add_class(path, -1, "horizontal");
            gtk_widget_path_iter_add_class(path, -1, "linked");
            return makeContext(path, mpComboboxStyle);
        }
        case GtkControlPart::ListboxBox:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpListboxStyle));
            gtk_widget_path_append_type(path, G_TYPE_NONE);
            set_object_name(path, -1, "box");
            gtk_widget_path_iter_add_class(path, -1, "horizontal");
            gtk_widget_path_iter_add_class(path, -1, "linked");
            return makeContext(path, mpListboxStyle);
        }
        case GtkControlPart::ComboboxBoxEntry:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpComboboxBoxStyle));
            GtkWidgetPath* pSiblingsPath;
            if (AllSettings::GetLayoutRTL())
            {
                pSiblingsPath = buildRTLComboSiblingsPath(set_object_name);
                gtk_widget_path_append_with_siblings(path, pSiblingsPath, 1);
            }
            else
            {
                pSiblingsPath = buildLTRComboSiblingsPath(set_object_name);
                gtk_widget_path_append_with_siblings(path, pSiblingsPath, 0);
            }
            gtk_widget_path_unref(pSiblingsPath);
            return makeContext(path, mpComboboxBoxStyle);
        }
        case GtkControlPart::ComboboxBoxButton:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpComboboxBoxStyle));
            GtkWidgetPath* pSiblingsPath;
            if (AllSettings::GetLayoutRTL())
            {
                pSiblingsPath = buildRTLComboSiblingsPath(set_object_name);
                gtk_widget_path_append_with_siblings(path, pSiblingsPath, 0);
            }
            else
            {
                pSiblingsPath = buildLTRComboSiblingsPath(set_object_name);
                gtk_widget_path_append_with_siblings(path, pSiblingsPath, 1);
            }
            gtk_widget_path_unref(pSiblingsPath);
            return makeContext(path, mpComboboxBoxStyle);
        }
        case GtkControlPart::ListboxBoxButton:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpListboxBoxStyle));
            GtkWidgetPath* pSiblingsPath = gtk_widget_path_new();

            gtk_widget_path_append_type(pSiblingsPath, GTK_TYPE_BUTTON);
            set_object_name(pSiblingsPath, -1, "button");
            gtk_widget_path_iter_add_class(pSiblingsPath, -1, "combo");

            gtk_widget_path_append_with_siblings(path, pSiblingsPath, 0);
            gtk_widget_path_unref(pSiblingsPath);
            return makeContext(path, mpListboxBoxStyle);
        }
        case GtkControlPart::ComboboxBoxButtonBox:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpComboboxButtonStyle));
            gtk_widget_path_append_type(path, G_TYPE_NONE);
            set_object_name(path, -1, "box");
            gtk_widget_path_iter_add_class(path, -1, "horizontal");
            return makeContext(path, mpComboboxButtonStyle);
        }
        case GtkControlPart::ListboxBoxButtonBox:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpListboxButtonStyle));
            gtk_widget_path_append_type(path, G_TYPE_NONE);
            set_object_name(path, -1, "box");
            gtk_widget_path_iter_add_class(path, -1, "horizontal");
            return makeContext(path, mpListboxButtonStyle);
        }
        case GtkControlPart::SpinButton:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpWindowStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_SPIN_BUTTON);
            set_object_name(path, -1, "spinbutton");
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_HORIZONTAL);
            return makeContext(path, mpWindowStyle);
        }
        case GtkControlPart::SpinButtonEntry:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpSpinStyle));
            gtk_widget_path_append_type(path, G_TYPE_NONE);
            set_object_name(path, -1, "entry");
            return makeContext(path, mpSpinStyle);
        }
        case GtkControlPart::SpinButtonUpButton:
        case GtkControlPart::SpinButtonDownButton:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpSpinStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_SPIN_BUTTON);
            set_object_name(path, -1, "button");
            gtk_widget_path_iter_add_class(path, -1, ePart == GtkControlPart::SpinButtonUpButton ? "up" : "down");
            return makeContext(path, mpSpinStyle);
        }
        case GtkControlPart::ScrollbarVertical:
        case GtkControlPart::ScrollbarHorizontal:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_SCROLLBAR);
            set_object_name(path, -1, "scrollbar");
            gtk_widget_path_iter_add_class(path, -1, ePart == GtkControlPart::ScrollbarVertical ? "vertical" : "horizontal");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::ScrollbarVerticalContents:
        case GtkControlPart::ScrollbarHorizontalContents:
        {
            GtkStyleContext *pParent =
                (ePart == GtkControlPart::ScrollbarVerticalContents) ? mpVScrollbarStyle : mpHScrollbarStyle;
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(pParent));
            gtk_widget_path_append_type(path, GTK_TYPE_SCROLLBAR);
            set_object_name(path, -1, "contents");
            return makeContext(path, pParent);
        }
        case GtkControlPart::ScrollbarVerticalTrough:
        case GtkControlPart::ScrollbarHorizontalTrough:
        {
            GtkStyleContext *pParent =
                (ePart == GtkControlPart::ScrollbarVerticalTrough) ? mpVScrollbarContentsStyle : mpHScrollbarContentsStyle;
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(pParent));
            gtk_widget_path_append_type(path, GTK_TYPE_SCROLLBAR);
            set_object_name(path, -1, "trough");
            return makeContext(path, pParent);
        }
        case GtkControlPart::ScrollbarVerticalSlider:
        case GtkControlPart::ScrollbarHorizontalSlider:
        {
            GtkStyleContext *pParent =
                (ePart == GtkControlPart::ScrollbarVerticalSlider) ? mpVScrollbarTroughStyle : mpHScrollbarTroughStyle;
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(pParent));
            gtk_widget_path_append_type(path, GTK_TYPE_SCROLLBAR);
            set_object_name(path, -1, "slider");
            return makeContext(path, pParent);
        }
        case GtkControlPart::ScrollbarVerticalButton:
        case GtkControlPart::ScrollbarHorizontalButton:
        {
            GtkStyleContext *pParent =
                (ePart == GtkControlPart::ScrollbarVerticalButton) ? mpVScrollbarStyle : mpHScrollbarStyle;
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(pParent));
            gtk_widget_path_append_type(path, GTK_TYPE_SCROLLBAR);
            set_object_name(path, -1, "button");
            return makeContext(path, pParent);
        }
        case GtkControlPart::ProgressBar:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_PROGRESS_BAR);
            set_object_name(path, -1, "progressbar");
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_HORIZONTAL);
            return makeContext(path, nullptr);
        }
        case GtkControlPart::ProgressBarTrough:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpProgressBarStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_PROGRESS_BAR);
            set_object_name(path, -1, "trough");
            return makeContext(path, mpProgressBarStyle);
        }
        case GtkControlPart::ProgressBarProgress:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpProgressBarTroughStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_PROGRESS_BAR);
            set_object_name(path, -1, "progress");
            return makeContext(path, mpProgressBarTroughStyle);
        }
        case GtkControlPart::Notebook:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpWindowStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_NOTEBOOK);
            set_object_name(path, -1, "notebook");
            return makeContext(path, mpWindowStyle);
        }
        case GtkControlPart::NotebookStack:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpNotebookStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_NOTEBOOK);
            set_object_name(path, -1, "stack");
            return makeContext(path, mpNotebookStyle);
        }
        case GtkControlPart::NotebookHeader:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpNotebookStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_NOTEBOOK);
            set_object_name(path, -1, "header");
            gtk_widget_path_iter_add_class(path, -1, "frame");
            gtk_widget_path_iter_add_class(path, -1, "top");
            return makeContext(path, mpNotebookStyle);
        }
        case GtkControlPart::NotebookHeaderTabs:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpNotebookHeaderStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_NOTEBOOK);
            set_object_name(path, -1, "tabs");
            gtk_widget_path_iter_add_class(path, -1, "top");
            return makeContext(path, mpNotebookHeaderStyle);
        }
        case GtkControlPart::NotebookHeaderTabsTab:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpNotebookHeaderTabsStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_NOTEBOOK);
            set_object_name(path, -1, "tab");
            gtk_widget_path_iter_add_class(path, -1, "top");
            return makeContext(path, mpNotebookHeaderTabsStyle);
        }
        case GtkControlPart::NotebookHeaderTabsTabLabel:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpNotebookHeaderTabsTabStyle));
            gtk_widget_path_append_type(path, G_TYPE_NONE);
            set_object_name(path, -1, "label");
            return makeContext(path, mpNotebookHeaderTabsTabStyle);
        }
        case GtkControlPart::NotebookHeaderTabsTabActiveLabel:
        case GtkControlPart::NotebookHeaderTabsTabHoverLabel:
            return mpNotebookHeaderTabsTabLabelStyle;
        case GtkControlPart::FrameBorder:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_FRAME);
            set_object_name(path, -1, "frame");
            gtk_widget_path_iter_add_class(path, -1, "frame");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::MenuBar:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpWindowStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_MENU_BAR);
            set_object_name(path, -1, "menubar");
            return makeContext(path, mpWindowStyle);
        }
        case GtkControlPart::MenuBarItem:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuBarStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_MENU_ITEM);
            set_object_name(path, -1, "menuitem");
            return makeContext(path, mpMenuBarStyle);
        }
        case GtkControlPart::MenuWindow:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuBarItemStyle));
            gtk_widget_path_append_type(path, G_TYPE_NONE);
            set_object_name(path, -1, "window");
            gtk_widget_path_iter_add_class(path, -1, "background");
            gtk_widget_path_iter_add_class(path, -1, "popup");
            return makeContext(path, mpMenuBarItemStyle);
        }
        case GtkControlPart::Menu:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuWindowStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_MENU);
            set_object_name(path, -1, "menu");
            return makeContext(path, mpMenuWindowStyle);
        }
        case GtkControlPart::MenuItem:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_MENU_ITEM);
            set_object_name(path, -1, "menuitem");
            return makeContext(path, mpMenuStyle);
        }
        case GtkControlPart::MenuItemLabel:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuItemStyle));
            gtk_widget_path_append_type(path, G_TYPE_NONE);
            set_object_name(path, -1, "label");
            return makeContext(path, mpMenuItemStyle);
        }
        case GtkControlPart::MenuItemArrow:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuItemStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_MENU_ITEM);
            set_object_name(path, -1, "arrow");
            return makeContext(path, mpMenuItemStyle);
        }
        case GtkControlPart::CheckMenuItem:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_CHECK_MENU_ITEM);
            set_object_name(path, -1, "menuitem");
            return makeContext(path, mpMenuStyle);
        }
        case GtkControlPart::CheckMenuItemCheck:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpCheckMenuItemStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_CHECK_MENU_ITEM);
            set_object_name(path, -1, "check");
            return makeContext(path, mpCheckMenuItemStyle);
        }
        case GtkControlPart::RadioMenuItem:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_RADIO_MENU_ITEM);
            set_object_name(path, -1, "menuitem");
            return makeContext(path, mpMenuStyle);
        }
        case GtkControlPart::RadioMenuItemRadio:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpRadioMenuItemStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_RADIO_MENU_ITEM);
            set_object_name(path, -1, "radio");
            return makeContext(path, mpRadioMenuItemStyle);
        }
        case GtkControlPart::SeparatorMenuItem:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_SEPARATOR_MENU_ITEM);
            set_object_name(path, -1, "menuitem");
            return makeContext(path, mpMenuStyle);
        }
        case GtkControlPart::SeparatorMenuItemSeparator:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpSeparatorMenuItemStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_SEPARATOR_MENU_ITEM);
            set_object_name(path, -1, "separator");
            return makeContext(path, mpSeparatorMenuItemStyle);
        }
    }

    return nullptr;
}

#ifndef GTK_STYLE_CLASS_POPUP
#define GTK_STYLE_CLASS_POPUP "popup"
#endif
#ifndef GTK_STYLE_CLASS_LABEL
#define GTK_STYLE_CLASS_LABEL "label"
#endif

GtkStyleContext* GtkSalGraphics::createOldContext(GtkControlPart ePart)
{
    switch (ePart)
    {
        case GtkControlPart::ToplevelWindow:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_WINDOW);
            gtk_widget_path_iter_add_class(path, -1, "background");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::Button:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_BUTTON);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_BUTTON);
            gtk_widget_path_iter_add_class(path, -1, "button");
            gtk_widget_path_iter_add_class(path, -1, "text-button");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::LinkButton:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_LINK_BUTTON);
            gtk_widget_path_iter_add_class(path, -1, "text-button");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::CheckButton:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_CHECK_BUTTON);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_CHECK);
            gtk_widget_path_iter_add_class(path, -1, "text-button");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::CheckButtonCheck:
            return mpCheckButtonStyle;
        case GtkControlPart::RadioButton:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_RADIO_BUTTON);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_RADIO);
            gtk_widget_path_iter_add_class(path, -1, "text-button");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::RadioButtonRadio:
            return mpRadioButtonStyle;
        case GtkControlPart::ComboboxBoxButtonBoxArrow:
        case GtkControlPart::ListboxBoxButtonBoxArrow:
        {
            return (ePart == GtkControlPart::ComboboxBoxButtonBoxArrow)
                ? mpComboboxButtonStyle : mpListboxButtonStyle;
        }
        case GtkControlPart::Entry:
        case GtkControlPart::ComboboxBoxEntry:
        case GtkControlPart::SpinButtonEntry:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_ENTRY);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_ENTRY);
            return makeContext(path, nullptr);
        }
        case GtkControlPart::Combobox:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_COMBO_BOX_TEXT);
            return makeContext(path, nullptr);
        }
        case GtkControlPart::Listbox:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_COMBO_BOX);
            return makeContext(path, nullptr);
        }
        case GtkControlPart::ComboboxBoxButton:
        case GtkControlPart::ListboxBoxButton:
        {
            GtkStyleContext *pParent =
                (ePart == GtkControlPart::ComboboxBoxButton ) ? mpComboboxStyle : mpListboxStyle;
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(pParent));
            gtk_widget_path_append_type(path, GTK_TYPE_TOGGLE_BUTTON);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_BUTTON);
            gtk_widget_path_iter_add_class(path, -1, "the-button-in-the-combobox");
            return makeContext(path, pParent);
        }
        case GtkControlPart::SpinButton:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_SPIN_BUTTON);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_ENTRY);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_HORIZONTAL);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_SPINBUTTON);
            return makeContext(path, nullptr);
        }
        case GtkControlPart::SpinButtonUpButton:
        case GtkControlPart::SpinButtonDownButton:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpSpinStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_SPIN_BUTTON);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_SPINBUTTON);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_BUTTON);
            return makeContext(path, mpSpinStyle);
        }
        case GtkControlPart::ScrollbarVertical:
        case GtkControlPart::ScrollbarHorizontal:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_SCROLLBAR);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_SCROLLBAR);
            gtk_widget_path_iter_add_class(path, -1, ePart == GtkControlPart::ScrollbarVertical ? "vertical" : "horizontal");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::ScrollbarVerticalContents:
        case GtkControlPart::ScrollbarHorizontalContents:
        {
            GtkStyleContext *pParent =
                (ePart == GtkControlPart::ScrollbarVerticalContents) ? mpVScrollbarStyle : mpHScrollbarStyle;
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(pParent));
            gtk_widget_path_append_type(path, GTK_TYPE_SCROLLBAR);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_SCROLLBAR);
            gtk_widget_path_iter_add_class(path, -1, "contents");
            return makeContext(path, pParent);
        }
        case GtkControlPart::ScrollbarHorizontalTrough:
        case GtkControlPart::ScrollbarVerticalTrough:
        {
            GtkStyleContext *pParent =
                (ePart == GtkControlPart::ScrollbarVerticalTrough) ? mpVScrollbarContentsStyle : mpHScrollbarContentsStyle;
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(pParent));
            gtk_widget_path_append_type(path, GTK_TYPE_SCROLLBAR);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_SCROLLBAR);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_TROUGH);
            return makeContext(path, pParent);
        }
        case GtkControlPart::ScrollbarHorizontalSlider:
        case GtkControlPart::ScrollbarVerticalSlider:
        {
            GtkStyleContext *pParent =
                (ePart == GtkControlPart::ScrollbarVerticalSlider) ? mpVScrollbarContentsStyle : mpHScrollbarContentsStyle;
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(pParent));
            gtk_widget_path_append_type(path, GTK_TYPE_SCROLLBAR);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_SCROLLBAR);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_SLIDER);
            return makeContext(path, pParent);
        }
        case GtkControlPart::ScrollbarHorizontalButton:
        case GtkControlPart::ScrollbarVerticalButton:
        {
            GtkStyleContext *pParent =
                (ePart == GtkControlPart::ScrollbarVerticalButton) ? mpVScrollbarStyle : mpHScrollbarStyle;
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(pParent));
            gtk_widget_path_append_type(path, GTK_TYPE_SCROLLBAR);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_SCROLLBAR);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_BUTTON);
            return makeContext(path, pParent);
        }
        case GtkControlPart::ProgressBar:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_PROGRESS_BAR);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_PROGRESSBAR);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_HORIZONTAL);
            return makeContext(path, nullptr);
        }
        case GtkControlPart::ProgressBarTrough:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpProgressBarStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_PROGRESS_BAR);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_TROUGH);
            return makeContext(path, mpProgressBarStyle);
        }
        case GtkControlPart::ProgressBarProgress:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpProgressBarTroughStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_PROGRESS_BAR);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_PROGRESSBAR);
            return makeContext(path, mpProgressBarTroughStyle);
        }
        case GtkControlPart::Notebook:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpWindowStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_NOTEBOOK);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_NOTEBOOK);
            gtk_widget_path_iter_add_class(path, -1, "frame");
            return makeContext(path, mpWindowStyle);
        }
        case GtkControlPart::NotebookStack:
            return mpNotebookStyle;
        case GtkControlPart::NotebookHeader:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpNotebookStyle));
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_HEADER);
            gtk_widget_path_iter_add_class(path, -1, "top");
            return makeContext(path, gtk_style_context_get_parent(mpNotebookStyle));
        }
        case GtkControlPart::NotebookHeaderTabs:
            return mpNotebookHeaderStyle;
        case GtkControlPart::NotebookHeaderTabsTab:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpNotebookHeaderTabsStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_NOTEBOOK);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_HEADER);
            gtk_widget_path_iter_add_class(path, -1, "top");
            gtk_widget_path_iter_add_region(path, -1, GTK_STYLE_REGION_TAB, static_cast<GtkRegionFlags>(GTK_REGION_EVEN | GTK_REGION_FIRST));
            return makeContext(path, mpNotebookHeaderTabsStyle);
        }
        case GtkControlPart::NotebookHeaderTabsTabLabel:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpNotebookHeaderTabsTabStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_LABEL);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_LABEL);
            return makeContext(path, mpNotebookHeaderTabsTabStyle);
        }
        case GtkControlPart::NotebookHeaderTabsTabActiveLabel:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpNotebookHeaderTabsTabStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_LABEL);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_LABEL);
            gtk_widget_path_iter_add_class(path, -1, "active-page");
            return makeContext(path, mpNotebookHeaderTabsTabStyle);
        }
        case GtkControlPart::NotebookHeaderTabsTabHoverLabel:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpNotebookHeaderTabsTabStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_LABEL);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_LABEL);
            gtk_widget_path_iter_add_class(path, -1, "prelight-page");
            return makeContext(path, mpNotebookHeaderTabsTabStyle);
        }
        case GtkControlPart::FrameBorder:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_FRAME);
            gtk_widget_path_iter_add_class(path, -1, "frame");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::MenuBar:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpWindowStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_MENU_BAR);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_MENUBAR);
            return makeContext(path, mpWindowStyle);
        }
        case GtkControlPart::MenuBarItem:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuBarStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_MENU_ITEM);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_MENUITEM);
            return makeContext(path, mpMenuBarStyle);
        }
        case GtkControlPart::MenuWindow:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuBarItemStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_WINDOW);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_POPUP);
            gtk_widget_path_iter_add_class(path, -1, "background");
            return makeContext(path, mpMenuBarItemStyle);
        }
        case GtkControlPart::Menu:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuWindowStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_MENU);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_MENU);
            return makeContext(path, mpMenuWindowStyle);
        }
        case GtkControlPart::MenuItem:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_MENU_ITEM);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_MENUITEM);
            return makeContext(path, mpMenuStyle);
        }
        case GtkControlPart::MenuItemLabel:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuItemStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_LABEL);
            return makeContext(path, mpMenuItemStyle);
        }
        case GtkControlPart::MenuItemArrow:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuItemStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_MENU_ITEM);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_ARROW);
            return makeContext(path, mpMenuItemStyle);
        }
        case GtkControlPart::CheckMenuItem:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_CHECK_MENU_ITEM);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_CHECK);
            return makeContext(path, mpMenuStyle);
        }
        case GtkControlPart::CheckMenuItemCheck:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpCheckMenuItemStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_CHECK_MENU_ITEM);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_CHECK);
            return makeContext(path, mpCheckMenuItemStyle);
        }
        case GtkControlPart::RadioMenuItem:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_RADIO_MENU_ITEM);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_MENUITEM);
            return makeContext(path, mpMenuStyle);
        }
        case GtkControlPart::RadioMenuItemRadio:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpRadioMenuItemStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_RADIO_MENU_ITEM);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_RADIO);
            return makeContext(path, mpRadioMenuItemStyle);
        }
        case GtkControlPart::SeparatorMenuItem:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_SEPARATOR_MENU_ITEM);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_MENUITEM);
            return makeContext(path, mpMenuStyle);
        }
        case GtkControlPart::SeparatorMenuItemSeparator:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpSeparatorMenuItemStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_SEPARATOR_MENU_ITEM);
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_SEPARATOR);
            return makeContext(path, mpSeparatorMenuItemStyle);
        }
        case GtkControlPart::ComboboxBox:
        case GtkControlPart::ListboxBox:
        case GtkControlPart::ComboboxBoxButtonBox:
        case GtkControlPart::ListboxBoxButtonBox:
            return nullptr;
        default:
            break;
    }

    return mpButtonStyle;
}

GtkStyleContext* GtkSalGraphics::createStyleContext(gtk_widget_path_iter_set_object_nameFunc set_object_name,
                                                    GtkControlPart ePart)
{
    if (set_object_name)
        return createNewContext(ePart, set_object_name);
    return createOldContext(ePart);
}

namespace
{
    GtkStateFlags ACTIVE_TAB()
    {
#if GTK_CHECK_VERSION(3,20,0)
        if (gtk_check_version(3, 20, 0) == nullptr)
            return GTK_STATE_FLAG_CHECKED;
#endif
        return GTK_STATE_FLAG_ACTIVE;
    }
}

void GtkSalGraphics::PaintCheckOrRadio(cairo_t *cr, GtkStyleContext *context,
                                       const tools::Rectangle& rControlRectangle, bool bIsCheck, bool bInMenu)
{
    gint indicator_size;
    gtk_style_context_get_style(context, "indicator-size", &indicator_size, nullptr);

    gint x = (rControlRectangle.GetWidth() - indicator_size) / 2;
    gint y = (rControlRectangle.GetHeight() - indicator_size) / 2;

    if (!bInMenu)
        gtk_render_background(context, cr, x, y, indicator_size, indicator_size);

    if (bIsCheck)
        gtk_render_check(context, cr, x, y, indicator_size, indicator_size);
    else
        gtk_render_option(context, cr, x, y, indicator_size, indicator_size);

    gtk_render_frame(context, cr, x, y, indicator_size, indicator_size);
}

void GtkSalGraphics::PaintCheck(cairo_t *cr, GtkStyleContext *context,
                                const tools::Rectangle& rControlRectangle, bool bInMenu)
{
    PaintCheckOrRadio(cr, context, rControlRectangle, true, bInMenu);
}

void GtkSalGraphics::PaintRadio(cairo_t *cr, GtkStyleContext *context,
                                const tools::Rectangle& rControlRectangle, bool bInMenu)
{
    PaintCheckOrRadio(cr, context, rControlRectangle, false, bInMenu);
}

static gfloat getArrowSize(GtkStyleContext* context)
{
    gfloat arrow_scaling = 1.0;
    gtk_style_context_get_style(context, "arrow-scaling", &arrow_scaling, nullptr);
    gfloat arrow_size = 11 * arrow_scaling;
    return arrow_size;
}

namespace
{
    void draw_vertical_separator(GtkStyleContext *context, cairo_t *cr, const tools::Rectangle& rControlRegion)
    {
        long nX = 0;
        long nY = 0;

        const bool bNewStyle = gtk_check_version(3, 20, 0) == nullptr;

        gint nSeparatorWidth = 1;

        if (bNewStyle)
        {
            gtk_style_context_get(context,
                gtk_style_context_get_state(context),
                "min-width", &nSeparatorWidth, nullptr);
        }

        gint nHalfSeparatorWidth = nSeparatorWidth / 2;
        gint nHalfRegionWidth = rControlRegion.GetWidth() / 2;

        nX = nX + nHalfRegionWidth - nHalfSeparatorWidth;
        nY = rControlRegion.GetHeight() > 5 ? 1 : 0;
        int nHeight = rControlRegion.GetHeight() - (2 * nY);

        if (bNewStyle)
        {
            gtk_render_background(context, cr, nX, nY, nSeparatorWidth, nHeight);
            gtk_render_frame(context, cr, nX, nY, nSeparatorWidth, nHeight);
        }
        else
        {
            gtk_render_line(context, cr, nX, nY, nX, nY + nHeight);
        }
    }

    void draw_horizontal_separator(GtkStyleContext *context, cairo_t *cr, const tools::Rectangle& rControlRegion)
    {
        long nX = 0;
        long nY = 0;

        const bool bNewStyle = gtk_check_version(3, 20, 0) == nullptr;

        gint nSeparatorHeight = 1;

        if (bNewStyle)
        {
            gtk_style_context_get(context,
                gtk_style_context_get_state(context),
                "min-height", &nSeparatorHeight, nullptr);
        }

        gint nHalfSeparatorHeight = nSeparatorHeight / 2;
        gint nHalfRegionHeight = rControlRegion.GetHeight() / 2;

        nY = nY + nHalfRegionHeight - nHalfSeparatorHeight;
        nX = rControlRegion.GetWidth() > 5 ? 1 : 0;
        int nWidth = rControlRegion.GetWidth() - (2 * nX);

        if (bNewStyle)
        {
            gtk_render_background(context, cr, nX, nY, nWidth, nSeparatorHeight);
            gtk_render_frame(context, cr, nX, nY, nWidth, nSeparatorHeight);
        }
        else
        {
            gtk_render_line(context, cr, nX, nY, nX + nWidth, nY);
        }
    }
}

bool GtkSalGraphics::drawNativeControl( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion,
                                            ControlState nState, const ImplControlValue& rValue,
                                            const OUString& aCaptions)
{
    if (m_pWidgetDraw)
    {
        bool bReturn = SvpSalGraphics::drawNativeControl(nType, nPart, rControlRegion,
                                                         nState, rValue, aCaptions);

        if (bReturn && !rControlRegion.IsEmpty())
            mpFrame->damaged(rControlRegion.Left(), rControlRegion.Top(), rControlRegion.GetWidth(), rControlRegion.GetHeight());

        return bReturn;
    }

    RenderType renderType = nPart == ControlPart::Focus ? RenderType::Focus : RenderType::BackgroundAndFrame;
    GtkStyleContext *context = nullptr;
    const gchar *styleClass = nullptr;
    GdkPixbuf *pixbuf = nullptr;
    bool bInMenu = false;

    GtkStateFlags flags = NWConvertVCLStateToGTKState(nState);

    switch(nType)
    {
    case ControlType::Spinbox:
    case ControlType::SpinButtons:
        context = mpSpinStyle;
        renderType = RenderType::Spinbutton;
        break;
    case ControlType::Editbox:
        context = mpEntryStyle;
        break;
    case ControlType::MultilineEditbox:
        context = mpTextViewStyle;
        break;
    case ControlType::Combobox:
        context = mpComboboxStyle;
        renderType = RenderType::Combobox;
        break;
    case ControlType::Listbox:
        if (nPart == ControlPart::Focus)
        {
            renderType = RenderType::Focus;
            context = mpListboxButtonStyle;
        }
        else
        {
            renderType = RenderType::Combobox;
            context = mpListboxStyle;
        }
        break;
    case ControlType::MenuPopup:
        bInMenu = true;

        // map selected menu entries in vcl parlance to gtk prelight
        if (nPart >= ControlPart::MenuItem && nPart <= ControlPart::SubmenuArrow && (nState & ControlState::SELECTED))
            flags = static_cast<GtkStateFlags>(flags | GTK_STATE_FLAG_PRELIGHT);
        flags = static_cast<GtkStateFlags>(flags & ~GTK_STATE_FLAG_ACTIVE);
        switch(nPart)
        {
        case ControlPart::MenuItem:
            context = mpMenuItemStyle;
            renderType = RenderType::BackgroundAndFrame;
            break;
        case ControlPart::MenuItemCheckMark:
            if (gtk_check_version(3, 20, 0) == nullptr)
                context = mpCheckMenuItemCheckStyle;
            else
            {
                context = gtk_widget_get_style_context(gCheckMenuItemWidget);
                styleClass = GTK_STYLE_CLASS_CHECK;
            }
            renderType = RenderType::Check;
            nType = ControlType::Checkbox;
            if (nState & ControlState::PRESSED)
            {
                flags = static_cast<GtkStateFlags>(flags | GTK_STATE_FLAG_CHECKED);
            }
            break;
        case ControlPart::MenuItemRadioMark:
            if (gtk_check_version(3, 20, 0) == nullptr)
                context = mpRadioMenuItemRadioStyle;
            else
            {
                context = gtk_widget_get_style_context(gCheckMenuItemWidget);
                styleClass = GTK_STYLE_CLASS_RADIO;
            }
            renderType = RenderType::Radio;
            nType = ControlType::Radiobutton;
            if (nState & ControlState::PRESSED)
            {
                flags = static_cast<GtkStateFlags>(flags | GTK_STATE_FLAG_CHECKED);
            }
            break;
        case ControlPart::Separator:
            context = mpSeparatorMenuItemSeparatorStyle;
            flags = GtkStateFlags(GTK_STATE_FLAG_BACKDROP | GTK_STATE_FLAG_INSENSITIVE); //GTK_STATE_FLAG_BACKDROP hack ?
            renderType = RenderType::MenuSeparator;
            break;
        case ControlPart::SubmenuArrow:
            if (gtk_check_version(3, 20, 0) == nullptr)
                context = mpMenuItemArrowStyle;
            else
            {
                context = gtk_widget_get_style_context(gCheckMenuItemWidget);
                styleClass = GTK_STYLE_CLASS_ARROW;
            }
            renderType = RenderType::Arrow;
            break;
        case ControlPart::Entire:
            context = mpMenuStyle;
            renderType = RenderType::Background;
            break;
        default: break;
        }
        break;
    case ControlType::Toolbar:
        switch(nPart)
        {
        case ControlPart::DrawBackgroundHorz:
        case ControlPart::DrawBackgroundVert:
            context = mpToolbarStyle;
            break;
        case ControlPart::Button:
            /* For all checkbuttons in the toolbars */
            flags = static_cast<GtkStateFlags>(flags |
                    ( (rValue.getTristateVal() == ButtonValue::On) ? GTK_STATE_FLAG_ACTIVE : GTK_STATE_FLAG_NORMAL));
            context = mpToolButtonStyle;
            break;
        case ControlPart::SeparatorVert:
            context = mpToolbarSeperatorStyle;
            renderType = RenderType::ToolbarSeparator;
            break;
        default:
            return false;
        }
        break;
    case ControlType::Radiobutton:
        flags = static_cast<GtkStateFlags>(flags |
                ( (rValue.getTristateVal() == ButtonValue::On) ? GTK_STATE_FLAG_CHECKED : GTK_STATE_FLAG_NORMAL));
        context = mpRadioButtonRadioStyle;
        renderType = nPart == ControlPart::Focus ? RenderType::Focus : RenderType::Radio;
        break;
    case ControlType::Checkbox:
        flags = static_cast<GtkStateFlags>(flags |
                ( (rValue.getTristateVal() == ButtonValue::On) ? GTK_STATE_FLAG_CHECKED :
                  (rValue.getTristateVal() == ButtonValue::Mixed) ? GTK_STATE_FLAG_INCONSISTENT :
                  GTK_STATE_FLAG_NORMAL));
        context = mpCheckButtonCheckStyle;
        renderType = nPart == ControlPart::Focus ? RenderType::Focus : RenderType::Check;
        break;
    case ControlType::Pushbutton:
        context = mpButtonStyle;
        break;
    case ControlType::Scrollbar:
        switch(nPart)
        {
        case ControlPart::DrawBackgroundVert:
        case ControlPart::DrawBackgroundHorz:
            context = (nPart == ControlPart::DrawBackgroundVert)
                ? mpVScrollbarStyle : mpHScrollbarStyle;
            renderType = RenderType::Scrollbar;
            break;
        default: break;
        }
        break;
    case ControlType::ListNet:
        return true;
        break;
    case ControlType::TabPane:
        context = mpNotebookStyle;
        break;
    case ControlType::TabBody:
        context = mpNotebookStackStyle;
        break;
    case ControlType::TabHeader:
        context = mpNotebookHeaderStyle;
        break;
    case ControlType::TabItem:
        context = mpNotebookHeaderTabsTabStyle;
        if (nState & ControlState::SELECTED)
            flags = static_cast<GtkStateFlags>(flags | ACTIVE_TAB());
        renderType = RenderType::TabItem;
        break;
    case ControlType::WindowBackground:
        context = gtk_widget_get_style_context(mpWindow);
        break;
    case ControlType::Frame:
    {
        DrawFrameStyle nStyle = static_cast<DrawFrameStyle>(rValue.getNumericVal() & 0x0f);
        if (nStyle == DrawFrameStyle::In)
            context = mpFrameOutStyle;
        else
            context = mpFrameInStyle;
        break;
    }
    case ControlType::Menubar:
        if (nPart == ControlPart::MenuItem)
        {
            context = mpMenuBarItemStyle;

            flags = (!(nState & ControlState::ENABLED)) ? GTK_STATE_FLAG_INSENSITIVE : GTK_STATE_FLAG_NORMAL;
            if (nState & ControlState::SELECTED)
                flags = static_cast<GtkStateFlags>(flags | GTK_STATE_FLAG_PRELIGHT);
        }
        else
        {
            context = mpMenuBarStyle;
        }
        break;
    case ControlType::Fixedline:
        context = nPart == ControlPart::SeparatorHorz ? mpFixedHoriLineStyle : mpFixedVertLineStyle;
        renderType = RenderType::Separator;
        break;
    case ControlType::ListNode:
    {
        context = mpTreeHeaderButtonStyle;
        ButtonValue aButtonValue = rValue.getTristateVal();
        if (aButtonValue == ButtonValue::On)
            flags = static_cast<GtkStateFlags>(flags | GTK_STATE_FLAG_CHECKED);
        renderType = RenderType::Expander;
        break;
    }
    case ControlType::ListHeader:
        context = mpTreeHeaderButtonStyle;
        if (nPart == ControlPart::Arrow)
        {
            const char* icon = (rValue.getNumericVal() & 1) ? "pan-down-symbolic" : "pan-up-symbolic";
            GtkIconTheme *pIconTheme = gtk_icon_theme_get_for_screen(gtk_widget_get_screen(mpWindow));
            pixbuf = gtk_icon_theme_load_icon(pIconTheme, icon,
                                              std::max(rControlRegion.GetWidth(), rControlRegion.GetHeight()),
                                              static_cast<GtkIconLookupFlags>(0), nullptr);
            flags = GTK_STATE_FLAG_SELECTED;
            renderType = RenderType::Icon;
        }
        break;
    case ControlType::Progress:
        context = mpProgressBarProgressStyle;
        renderType = RenderType::Progress;
        break;
    default:
        return false;
    }

    cairo_t *cr = getCairoContext(false);
    clipRegion(cr);
    cairo_translate(cr, rControlRegion.Left(), rControlRegion.Top());

    long nX = 0;
    long nY = 0;
    long nWidth = rControlRegion.GetWidth();
    long nHeight = rControlRegion.GetHeight();

    StyleContextSave aContextState;
    aContextState.save(context);
    style_context_set_state(context, flags);

    if (styleClass)
    {
        gtk_style_context_add_class(context, styleClass);
    }

    switch(renderType)
    {
    case RenderType::Background:
    case RenderType::BackgroundAndFrame:
        gtk_render_background(context, cr, nX, nY, nWidth, nHeight);
        if (renderType == RenderType::BackgroundAndFrame)
        {
            gtk_render_frame(context, cr, nX, nY, nWidth, nHeight);
        }
        break;
    case RenderType::Check:
    {
        PaintCheck(cr, context, rControlRegion, bInMenu);
        break;
    }
    case RenderType::Radio:
    {
        PaintRadio(cr, context, rControlRegion, bInMenu);
        break;
    }
    case RenderType::MenuSeparator:
        gtk_render_line(context, cr,
                        0, rControlRegion.GetHeight() / 2,
                        rControlRegion.GetWidth() - 1, rControlRegion.GetHeight() / 2);
        break;
    case RenderType::ToolbarSeparator:
    {
        draw_vertical_separator(context, cr, rControlRegion);
        break;
    }
    case RenderType::Separator:
        if (nPart == ControlPart::SeparatorHorz)
            draw_horizontal_separator(context, cr, rControlRegion);
        else
            draw_vertical_separator(context, cr, rControlRegion);
        break;
    case RenderType::Arrow:
        gtk_render_arrow(context, cr,
                         G_PI / 2, 0, 0,
                         MIN(rControlRegion.GetWidth(), 1 + rControlRegion.GetHeight()));
        break;
    case RenderType::Expander:
        gtk_render_expander(context, cr, -2, -2, nWidth+4, nHeight+4);
        break;
    case RenderType::Scrollbar:
        PaintScrollbar(context, cr, rControlRegion, nPart, rValue);
        break;
    case RenderType::Spinbutton:
        PaintSpinButton(flags, cr, rControlRegion, nPart, rValue);
        break;
    case RenderType::Combobox:
        PaintCombobox(flags, cr, rControlRegion, nType, nPart);
        break;
    case RenderType::Icon:
        gtk_render_icon(context, cr, pixbuf, nX, nY);
        g_object_unref(pixbuf);
        break;
    case RenderType::Focus:
    {
        if (nType == ControlType::Checkbox ||
            nType == ControlType::Radiobutton)
        {
            nX -= 2; nY -=2;
            nHeight += 4; nWidth += 4;
        }
        else
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
    case RenderType::Progress:
    {
        gtk_render_background(mpProgressBarTroughStyle, cr, nX, nY, nWidth, nHeight);

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

        gtk_render_frame(mpProgressBarTroughStyle, cr, nX, nY, nWidth, nHeight);

        break;
    }
    case RenderType::TabItem:
    {
        if (gtk_check_version(3, 20, 0) != nullptr)
        {
            gint initial_gap(0);
            gtk_style_context_get_style(mpNotebookStyle,
                                    "initial-gap", &initial_gap,
                                    nullptr);

            nX += initial_gap/2;
            nWidth -= initial_gap;
        }
        tools::Rectangle aRect(Point(nX, nY), Size(nWidth, nHeight));
        render_common(mpNotebookHeaderTabsTabStyle, cr, aRect, flags);
        break;
    }
    default:
        break;
    }

    if (styleClass)
    {
        gtk_style_context_remove_class(context, styleClass);
    }
    aContextState.restore();

    cairo_destroy(cr); // unref

    if (!rControlRegion.IsEmpty())
        mpFrame->damaged(rControlRegion.Left(), rControlRegion.Top(), rControlRegion.GetWidth(), rControlRegion.GetHeight());

    return true;
}

static tools::Rectangle GetWidgetSize(const tools::Rectangle& rControlRegion, GtkWidget* widget)
{
    GtkRequisition aReq;
    gtk_widget_get_preferred_size(widget, nullptr, &aReq);
    long nHeight = std::max<long>(rControlRegion.GetHeight(), aReq.height);
    return tools::Rectangle(rControlRegion.TopLeft(), Size(rControlRegion.GetWidth(), nHeight));
}

static tools::Rectangle AdjustRectForTextBordersPadding(GtkStyleContext* pStyle, long nContentWidth, long nContentHeight, const tools::Rectangle& rControlRegion)
{
    GtkBorder border;
    gtk_style_context_get_border(pStyle, gtk_style_context_get_state(pStyle), &border);

    GtkBorder padding;
    gtk_style_context_get_padding(pStyle, gtk_style_context_get_state(pStyle), &padding);

    gint nWidgetHeight = nContentHeight + padding.top + padding.bottom + border.top + border.bottom;
    nWidgetHeight = std::max(std::max<gint>(nWidgetHeight, rControlRegion.GetHeight()), 34);

    gint nWidgetWidth = nContentWidth + padding.left + padding.right + border.left + border.right;
    nWidgetWidth = std::max<gint>(nWidgetWidth, rControlRegion.GetWidth());

    tools::Rectangle aEditRect(rControlRegion.TopLeft(), Size(nWidgetWidth, nWidgetHeight));

    return aEditRect;
}

bool GtkSalGraphics::getNativeControlRegion( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion, ControlState eState,
                                                const ImplControlValue& rValue, const OUString& aCaption,
                                                tools::Rectangle &rNativeBoundingRegion, tools::Rectangle &rNativeContentRegion )
{
    if (hasWidgetDraw())
    {
        return m_pWidgetDraw->getNativeControlRegion(nType, nPart, rControlRegion,
                                                     eState, rValue, aCaption,
                                                     rNativeBoundingRegion, rNativeContentRegion);
    }

    /* TODO: all this functions needs improvements */
    tools::Rectangle aEditRect = rControlRegion;
    gint indicator_size, indicator_spacing, point;

    if(((nType == ControlType::Checkbox) || (nType == ControlType::Radiobutton)) &&
       nPart == ControlPart::Entire)
    {
        rNativeBoundingRegion = rControlRegion;

        GtkStyleContext *pButtonStyle = (nType == ControlType::Checkbox) ? mpCheckButtonCheckStyle : mpRadioButtonRadioStyle;


        gtk_style_context_get_style( pButtonStyle,
                                     "indicator-size", &indicator_size,
                                     "indicator-spacing", &indicator_spacing,
                                     nullptr );

        GtkBorder border;
        gtk_style_context_get_border(pButtonStyle, gtk_style_context_get_state(pButtonStyle), &border);

        GtkBorder padding;
        gtk_style_context_get_padding(pButtonStyle, gtk_style_context_get_state(pButtonStyle), &padding);


        indicator_size += 2*indicator_spacing + border.left + padding.left + border.right + padding.right;
        tools::Rectangle aIndicatorRect( Point( 0,
                                         (rControlRegion.GetHeight()-indicator_size)/2),
                                  Size( indicator_size, indicator_size ) );
        rNativeContentRegion = aIndicatorRect;

        return true;
    }
    else if( nType == ControlType::MenuPopup)
    {
        if ((nPart == ControlPart::MenuItemCheckMark) ||
            (nPart == ControlPart::MenuItemRadioMark) )
        {
            indicator_size = 0;

            GtkStyleContext *pMenuItemStyle = (nPart == ControlPart::MenuItemCheckMark ) ? mpCheckMenuItemCheckStyle
                                                                                         : mpRadioMenuItemRadioStyle;

            gtk_style_context_get_style( pMenuItemStyle,
                                         "indicator-size", &indicator_size,
                                         nullptr );

            point = MAX(0, rControlRegion.GetHeight() - indicator_size);
            aEditRect = tools::Rectangle( Point( 0, point / 2),
                                   Size( indicator_size, indicator_size ) );
        }
        else if (nPart == ControlPart::Separator)
        {
            gint separator_height, separator_width, wide_separators;

            gtk_style_context_get_style (mpSeparatorMenuItemSeparatorStyle,
                                         "wide-separators",  &wide_separators,
                                         "separator-width",  &separator_width,
                                         "separator-height", &separator_height,
                                         nullptr);

            aEditRect = tools::Rectangle( aEditRect.TopLeft(),
                                   Size( aEditRect.GetWidth(), wide_separators ? separator_height : 1 ) );
        }
        else if (nPart == ControlPart::SubmenuArrow)
        {
            gfloat arrow_size = getArrowSize(mpMenuItemArrowStyle);
            aEditRect = tools::Rectangle( aEditRect.TopLeft(),
                                   Size( arrow_size, arrow_size ) );
        }
    }
    else if ( (nType==ControlType::Scrollbar) &&
              ((nPart==ControlPart::ButtonLeft) || (nPart==ControlPart::ButtonRight) ||
               (nPart==ControlPart::ButtonUp) || (nPart==ControlPart::ButtonDown)  ) )
    {
        rNativeBoundingRegion = NWGetScrollButtonRect( nPart, rControlRegion );
        rNativeContentRegion = rNativeBoundingRegion;

        if (!rNativeContentRegion.GetWidth())
            rNativeContentRegion.SetRight( rNativeContentRegion.Left() + 1 );
        if (!rNativeContentRegion.GetHeight())
            rNativeContentRegion.SetBottom( rNativeContentRegion.Top() + 1 );

        return true;
    }
    else if ( (nType==ControlType::Spinbox) &&
              ((nPart==ControlPart::ButtonUp) || (nPart==ControlPart::ButtonDown) ||
               (nPart==ControlPart::SubEdit)) )
    {
        tools::Rectangle aControlRegion(GetWidgetSize(rControlRegion, gSpinBox));
        aEditRect = NWGetSpinButtonRect(nPart, aControlRegion);
    }
    else if ( (nType==ControlType::Combobox) &&
              ((nPart==ControlPart::ButtonDown) || (nPart==ControlPart::SubEdit)) )
    {
        aEditRect = NWGetComboBoxButtonRect(nType, nPart, rControlRegion);
    }
    else if ( (nType==ControlType::Listbox) &&
              ((nPart==ControlPart::ButtonDown) || (nPart==ControlPart::SubEdit)) )
    {
        aEditRect = NWGetComboBoxButtonRect(nType, nPart, rControlRegion);
    }
    else if (nType == ControlType::Editbox && nPart == ControlPart::Entire)
    {
        aEditRect = GetWidgetSize(rControlRegion, gEntryBox);
    }
    else if (nType == ControlType::Listbox && nPart == ControlPart::Entire)
    {
        aEditRect = GetWidgetSize(rControlRegion, gListBox);
    }
    else if (nType == ControlType::Combobox && nPart == ControlPart::Entire)
    {
        aEditRect = GetWidgetSize(rControlRegion, gComboBox);
    }
    else if (nType == ControlType::Spinbox && nPart == ControlPart::Entire)
    {
        aEditRect = GetWidgetSize(rControlRegion, gSpinBox);
    }
    else if (nType == ControlType::TabItem && nPart == ControlPart::Entire)
    {
        const TabitemValue& rTabitemValue = static_cast<const TabitemValue&>(rValue);
        const tools::Rectangle& rTabitemRect = rTabitemValue.getContentRect();

        aEditRect = AdjustRectForTextBordersPadding(mpNotebookHeaderTabsTabStyle, rTabitemRect.GetWidth(),
                                                    rTabitemRect.GetHeight(), rControlRegion);
    }
    else if (nType == ControlType::Frame && nPart == ControlPart::Border)
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
            rNativeContentRegion = tools::Rectangle(x1 + (padding.left + border.left),
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
static ::Color getColor( const GdkRGBA& rCol )
{
    return ::Color( static_cast<int>(rCol.red * 0xFFFF) >> 8, static_cast<int>(rCol.green * 0xFFFF) >> 8, static_cast<int>(rCol.blue * 0xFFFF) >> 8 );
}

static vcl::Font getFont(GtkStyleContext* pStyle, const css::lang::Locale& rLocale)
{
    const PangoFontDescription* font = gtk_style_context_get_font(pStyle, gtk_style_context_get_state(pStyle));
    return pango_to_vcl(font, rLocale);
}

vcl::Font pango_to_vcl(const PangoFontDescription* font, const css::lang::Locale& rLocale)
{
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
    psp::PrintFontManager::get().matchFont(aInfo, rLocale);

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "font match %s, name AFTER: \"%s\"\n",
                  aInfo.m_nID != 0 ? "succeeded" : "failed",
                  OUStringToOString( aInfo.m_aFamilyName, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif

    int nPointHeight = nPangoHeight/PANGO_SCALE;

    vcl::Font aFont( aInfo.m_aFamilyName, Size( 0, nPointHeight ) );
    if( aInfo.m_eWeight != WEIGHT_DONTKNOW )
        aFont.SetWeight( aInfo.m_eWeight );
    if( aInfo.m_eWidth != WIDTH_DONTKNOW )
        aFont.SetWidthType( aInfo.m_eWidth );
    if( aInfo.m_eItalic != ITALIC_DONTKNOW )
        aFont.SetItalic( aInfo.m_eItalic );
    if( aInfo.m_ePitch != PITCH_DONTKNOW )
        aFont.SetPitch( aInfo.m_ePitch );
    return aFont;
}

void GtkSalGraphics::updateSettings(AllSettings& rSettings)
{
    if (m_pWidgetDraw)
    {
        m_pWidgetDraw->updateSettings(rSettings);
        return;
    }

    GtkStyleContext* pStyle = gtk_widget_get_style_context( mpWindow );
    StyleContextSave aContextState;
    aContextState.save(pStyle);
    GtkSettings* pSettings = gtk_widget_get_settings( mpWindow );
    StyleSettings aStyleSet = rSettings.GetStyleSettings();
    GdkRGBA color;

    // text colors
    GdkRGBA text_color;
    style_context_set_state(pStyle, GTK_STATE_FLAG_NORMAL);
    gtk_style_context_get_color(pStyle, gtk_style_context_get_state(pStyle), &text_color);
    ::Color aTextColor = getColor( text_color );
    aStyleSet.SetDialogTextColor( aTextColor );
    aStyleSet.SetButtonTextColor( aTextColor );
    aStyleSet.SetRadioCheckTextColor( aTextColor );
    aStyleSet.SetGroupTextColor( aTextColor );
    aStyleSet.SetLabelTextColor( aTextColor );
    aStyleSet.SetWindowTextColor( aTextColor );
    aStyleSet.SetFieldTextColor( aTextColor );

    // background colors
    GdkRGBA background_color;
    gtk_style_context_get_background_color(pStyle, gtk_style_context_get_state(pStyle), &background_color);

    ::Color aBackColor = getColor( background_color );
    aStyleSet.BatchSetBackgrounds( aBackColor );

    // UI font
    vcl::Font aFont(getFont(pStyle, rSettings.GetUILanguageTag().getLocale()));

    aStyleSet.BatchSetFonts( aFont, aFont);

    aFont.SetWeight( WEIGHT_BOLD );
    aStyleSet.SetTitleFont( aFont );
    aStyleSet.SetFloatTitleFont( aFont );

    // mouse over text colors
    style_context_set_state(pStyle, GTK_STATE_FLAG_PRELIGHT);
    gtk_style_context_get_color(pStyle, gtk_style_context_get_state(pStyle), &text_color);
    aTextColor = getColor( text_color );
    aStyleSet.SetFieldRolloverTextColor( aTextColor );

    aContextState.restore();

    // button mouse over colors
    {
        GdkRGBA normal_button_rollover_text_color, pressed_button_rollover_text_color;
        aContextState.save(mpButtonStyle);
        style_context_set_state(mpButtonStyle, GTK_STATE_FLAG_PRELIGHT);
        gtk_style_context_get_color(mpButtonStyle, gtk_style_context_get_state(mpButtonStyle), &normal_button_rollover_text_color);
        aTextColor = getColor(normal_button_rollover_text_color);
        aStyleSet.SetButtonRolloverTextColor( aTextColor );
        style_context_set_state(mpButtonStyle, static_cast<GtkStateFlags>(GTK_STATE_FLAG_PRELIGHT | GTK_STATE_FLAG_ACTIVE));
        gtk_style_context_get_color(mpButtonStyle, gtk_style_context_get_state(mpButtonStyle), &pressed_button_rollover_text_color);
        aTextColor = getColor(pressed_button_rollover_text_color);
        style_context_set_state(mpButtonStyle, GTK_STATE_FLAG_NORMAL);
        aStyleSet.SetButtonPressedRolloverTextColor( aTextColor );
        aContextState.restore();
    }

    // tooltip colors
    {
        GtkStyleContext *pCStyle = gtk_style_context_new();
        gtk_style_context_set_screen( pCStyle, gtk_window_get_screen( GTK_WINDOW( mpWindow ) ) );
        GtkWidgetPath *pCPath = gtk_widget_path_new();
        guint pos = gtk_widget_path_append_type(pCPath, GTK_TYPE_WINDOW);
        gtk_widget_path_iter_add_class(pCPath, pos, GTK_STYLE_CLASS_TOOLTIP);
        pos = gtk_widget_path_append_type (pCPath, GTK_TYPE_LABEL);
        gtk_widget_path_iter_add_class(pCPath, pos, GTK_STYLE_CLASS_LABEL);
        pCStyle = gtk_style_context_new();
        gtk_style_context_set_path(pCStyle, pCPath);
        gtk_widget_path_free(pCPath);

        GdkRGBA tooltip_bg_color, tooltip_fg_color;
        style_context_set_state(pCStyle, GTK_STATE_FLAG_NORMAL);
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
        style_context_set_state(pCStyle, GTK_STATE_FLAG_SELECTED);
        gtk_style_context_get_background_color(pCStyle, gtk_style_context_get_state(pCStyle), &text_color);
        ::Color aHighlightColor = getColor( text_color );
        gtk_style_context_get_color(pCStyle, gtk_style_context_get_state(pCStyle), &text_color);
        ::Color aHighlightTextColor = getColor( text_color );
        aStyleSet.SetHighlightColor( aHighlightColor );
        aStyleSet.SetHighlightTextColor( aHighlightTextColor );

        // field background color
        GdkRGBA field_background_color;
        style_context_set_state(pCStyle, GTK_STATE_FLAG_NORMAL);
        gtk_style_context_get_background_color(pCStyle, gtk_style_context_get_state(pCStyle), &field_background_color);

        ::Color aBackFieldColor = getColor( field_background_color );
        aStyleSet.SetFieldColor( aBackFieldColor );
        // This baby is the default page/paper color
        aStyleSet.SetWindowColor( aBackFieldColor );

        // Cursor width
        gfloat caretAspectRatio = 0.04f;
        gtk_style_context_get_style( pCStyle, "cursor-aspect-ratio", &caretAspectRatio, nullptr );
        // Assume 20px tall for the ratio computation, which should give reasonable results
        aStyleSet.SetCursorSize( 20 * caretAspectRatio + 1 );

        // Dark shadow color
        style_context_set_state(pCStyle, GTK_STATE_FLAG_INSENSITIVE);
        gtk_style_context_get_color(pCStyle, gtk_style_context_get_state(pCStyle), &color);
        ::Color aDarkShadowColor = getColor( color );
        aStyleSet.SetDarkShadowColor( aDarkShadowColor );

        ::Color aShadowColor(aBackColor);
        if (aDarkShadowColor.GetLuminance() > aBackColor.GetLuminance())
            aShadowColor.IncreaseLuminance(64);
        else
            aShadowColor.DecreaseLuminance(64);
        aStyleSet.SetShadowColor(aShadowColor);

        g_object_unref( pCStyle );

        // Tab colors
        aStyleSet.SetActiveTabColor( aBackFieldColor ); // same as the window color.
        aStyleSet.SetInactiveTabColor( aBackColor );
    }

    // menu disabled entries handling
    aStyleSet.SetSkipDisabledInMenus( true );
    aStyleSet.SetPreferredContextMenuShortcuts( false );

    aContextState.save(mpMenuItemLabelStyle);

    // menu colors
    style_context_set_state(mpMenuStyle, GTK_STATE_FLAG_NORMAL);
    gtk_style_context_get_background_color( mpMenuStyle, gtk_style_context_get_state(mpMenuStyle), &background_color );
    aBackColor = getColor( background_color );
    aStyleSet.SetMenuColor( aBackColor );

    // menu bar
    style_context_set_state(mpMenuBarStyle, GTK_STATE_FLAG_NORMAL);
    gtk_style_context_get_background_color( mpMenuBarStyle, gtk_style_context_get_state(mpMenuBarStyle), &background_color );
    aBackColor = getColor( background_color );
    aStyleSet.SetMenuBarColor( aBackColor );
    aStyleSet.SetMenuBarRolloverColor( aBackColor );

    style_context_set_state(mpMenuBarItemStyle, GTK_STATE_FLAG_NORMAL);
    gtk_style_context_get_color( mpMenuBarItemStyle, gtk_style_context_get_state(mpMenuBarItemStyle), &text_color );
    aTextColor = aStyleSet.GetPersonaMenuBarTextColor().get_value_or( getColor( text_color ) );
    aStyleSet.SetMenuBarTextColor( aTextColor );
    aStyleSet.SetMenuBarRolloverTextColor( aTextColor );

    style_context_set_state(mpMenuBarItemStyle, GTK_STATE_FLAG_PRELIGHT);
    gtk_style_context_get_color( mpMenuBarItemStyle, gtk_style_context_get_state(mpMenuBarItemStyle), &text_color );
    aTextColor = aStyleSet.GetPersonaMenuBarTextColor().get_value_or( getColor( text_color ) );
    aStyleSet.SetMenuBarHighlightTextColor( aTextColor );

    // menu items
    style_context_set_state(mpMenuItemLabelStyle, GTK_STATE_FLAG_NORMAL);
    gtk_style_context_get_color(mpMenuItemLabelStyle, gtk_style_context_get_state(mpMenuItemLabelStyle), &color);
    aTextColor = getColor(color);
    aStyleSet.SetMenuTextColor(aTextColor);

    style_context_set_state(mpMenuItemLabelStyle, GTK_STATE_FLAG_PRELIGHT);
    gtk_style_context_get_background_color( mpMenuItemLabelStyle, gtk_style_context_get_state(mpMenuItemLabelStyle), &background_color );
    ::Color aHighlightColor = getColor( background_color );
    aStyleSet.SetMenuHighlightColor( aHighlightColor );

    gtk_style_context_get_color( mpMenuItemLabelStyle, gtk_style_context_get_state(mpMenuItemLabelStyle), &color );
    ::Color aHighlightTextColor = getColor( color );
    aStyleSet.SetMenuHighlightTextColor( aHighlightTextColor );

    aContextState.restore();

    // hyperlink colors
    aContextState.save(mpLinkButtonStyle);
    style_context_set_state(mpLinkButtonStyle, GTK_STATE_FLAG_LINK);
    gtk_style_context_get_color(mpLinkButtonStyle, gtk_style_context_get_state(mpLinkButtonStyle), &text_color);
    aStyleSet.SetLinkColor(getColor(text_color));
    style_context_set_state(mpLinkButtonStyle, GTK_STATE_FLAG_VISITED);
    gtk_style_context_get_color(mpLinkButtonStyle, gtk_style_context_get_state(mpLinkButtonStyle), &text_color);
    aStyleSet.SetVisitedLinkColor(getColor(text_color));
    aContextState.restore();

    {
        GtkStyleContext *pCStyle = mpNotebookHeaderTabsTabLabelStyle;
        aContextState.save(pCStyle);
        style_context_set_state(pCStyle, GTK_STATE_FLAG_NORMAL);
        gtk_style_context_get_color(pCStyle, gtk_style_context_get_state(pCStyle), &text_color);
        aTextColor = getColor( text_color );
        aStyleSet.SetTabTextColor(aTextColor);
        aStyleSet.SetTabFont(getFont(mpNotebookHeaderTabsTabLabelStyle, rSettings.GetUILanguageTag().getLocale()));
        aContextState.restore();
    }

    {
        GtkStyleContext *pCStyle = mpToolButtonStyle;
        aContextState.save(pCStyle);
        style_context_set_state(pCStyle, GTK_STATE_FLAG_NORMAL);
        gtk_style_context_get_color(pCStyle, gtk_style_context_get_state(pCStyle), &text_color);
        aTextColor = getColor( text_color );
        aStyleSet.SetToolTextColor(aTextColor);
        aStyleSet.SetToolFont(getFont(mpToolButtonStyle, rSettings.GetUILanguageTag().getLocale()));
        aContextState.restore();
    }

    // mouse over text colors
    {
        GtkStyleContext *pCStyle = mpNotebookHeaderTabsTabHoverLabelStyle;
        aContextState.save(pCStyle);
        style_context_set_state(pCStyle, GTK_STATE_FLAG_PRELIGHT);
        gtk_style_context_get_color(pCStyle, gtk_style_context_get_state(pCStyle), &text_color);
        aTextColor = getColor( text_color );
        aStyleSet.SetTabRolloverTextColor(aTextColor);
        aContextState.restore();
    }

    {
        GtkStyleContext *pCStyle = mpNotebookHeaderTabsTabActiveLabelStyle;
        aContextState.save(pCStyle);
        style_context_set_state(pCStyle, ACTIVE_TAB());
        gtk_style_context_get_color(pCStyle, gtk_style_context_get_state(pCStyle), &text_color);
        aTextColor = getColor( text_color );
        aStyleSet.SetTabHighlightTextColor(aTextColor);
        aContextState.restore();
    }

    // get cursor blink time
    gboolean blink = false;

    g_object_get( pSettings, "gtk-cursor-blink", &blink, nullptr );
    if( blink )
    {
        gint blink_time = static_cast<gint>(STYLE_CURSOR_NOBLINKTIME);
        g_object_get( pSettings, "gtk-cursor-blink-time", &blink_time, nullptr );
        // set the blink_time if there is a setting and it is reasonable
        // else leave the default value
        if( blink_time > 100 )
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
    gint min_slider_length = 21;

    // Grab some button style attributes
    if (gtk_check_version(3, 20, 0) == nullptr)
    {
        Size aSize;
        QuerySize(mpHScrollbarStyle, aSize);
        QuerySize(mpHScrollbarContentsStyle, aSize);
        QuerySize(mpHScrollbarTroughStyle, aSize);
        QuerySize(mpHScrollbarSliderStyle, aSize);

        gboolean has_forward, has_forward2, has_backward, has_backward2;
        gtk_style_context_get_style(mpHScrollbarStyle,
                                    "has-forward-stepper", &has_forward,
                                    "has-secondary-forward-stepper", &has_forward2,
                                    "has-backward-stepper", &has_backward,
                                    "has-secondary-backward-stepper", &has_backward2, nullptr);
        if (has_forward || has_backward || has_forward2 || has_backward2)
            QuerySize(mpHScrollbarButtonStyle, aSize);

        aStyleSet.SetScrollBarSize(aSize.Height());

        gtk_style_context_get(mpVScrollbarSliderStyle, gtk_style_context_get_state(mpVScrollbarSliderStyle),
                              "min-height", &min_slider_length,
                              nullptr);
        aStyleSet.SetMinThumbSize(min_slider_length);
    }
    else
    {
        gint slider_width = 14;
        gint trough_border = 1;

        gtk_style_context_get_style(mpVScrollbarStyle,
                                    "slider-width", &slider_width,
                                    "trough-border", &trough_border,
                                    "min-slider-length", &min_slider_length,
                                    nullptr);
        aStyleSet.SetScrollBarSize(slider_width + 2*trough_border);
        gint magic = trough_border ? 1 : 0;
        aStyleSet.SetMinThumbSize(min_slider_length - magic);
    }

    // preferred icon style
    gchar* pIconThemeName = nullptr;
    gboolean bDarkIconTheme = false;
    g_object_get(pSettings, "gtk-icon-theme-name", &pIconThemeName,
                            "gtk-application-prefer-dark-theme", &bDarkIconTheme,
                            nullptr );
    OUString sIconThemeName(OUString::createFromAscii(pIconThemeName));
    aStyleSet.SetPreferredIconTheme(sIconThemeName, bDarkIconTheme);
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
    if (m_pWidgetDraw)
    {
        return m_pWidgetDraw->isNativeControlSupported(nType, nPart);
    }

    switch(nType)
    {
        case ControlType::Pushbutton:
        case ControlType::Radiobutton:
        case ControlType::Checkbox:
        case ControlType::Progress:
        case ControlType::ListNode:
        case ControlType::ListNet:
            if (nPart==ControlPart::Entire || nPart == ControlPart::Focus)
                return true;
            break;

        case ControlType::Scrollbar:
            if(nPart==ControlPart::DrawBackgroundHorz || nPart==ControlPart::DrawBackgroundVert ||
               nPart==ControlPart::Entire       || nPart==ControlPart::HasThreeButtons)
                return true;
            break;

        case ControlType::Editbox:
        case ControlType::MultilineEditbox:
            if (nPart==ControlPart::Entire || nPart==ControlPart::HasBackgroundTexture)
                return true;
            break;

        case ControlType::Combobox:
            if (nPart==ControlPart::Entire || nPart==ControlPart::HasBackgroundTexture || nPart == ControlPart::AllButtons)
                return true;
            break;

        case ControlType::Spinbox:
            if (nPart==ControlPart::Entire || nPart==ControlPart::HasBackgroundTexture || nPart == ControlPart::AllButtons || nPart == ControlPart::ButtonUp || nPart == ControlPart::ButtonDown)
                return true;
            break;

        case ControlType::SpinButtons:
            if (nPart==ControlPart::Entire || nPart==ControlPart::AllButtons)
                return true;
            break;

        case ControlType::Frame:
        case ControlType::WindowBackground:
            return true;

        case ControlType::TabItem:
        case ControlType::TabHeader:
        case ControlType::TabPane:
        case ControlType::TabBody:
            if(nPart==ControlPart::Entire || nPart==ControlPart::TabsDrawRtl)
                return true;
            break;

        case ControlType::Listbox:
            if (nPart==ControlPart::Entire || nPart==ControlPart::ListboxWindow || nPart==ControlPart::HasBackgroundTexture || nPart == ControlPart::Focus)
                return true;
            break;

        case ControlType::Toolbar:
            if( nPart==ControlPart::Entire
//                ||  nPart==ControlPart::DrawBackgroundHorz
//                ||  nPart==ControlPart::DrawBackgroundVert
//                ||  nPart==ControlPart::ThumbHorz
//                ||  nPart==ControlPart::ThumbVert
                ||  nPart==ControlPart::Button
//                ||  nPart==ControlPart::SeparatorHorz
                ||  nPart==ControlPart::SeparatorVert
                )
                return true;
            break;

        case ControlType::Menubar:
            if (nPart==ControlPart::Entire || nPart==ControlPart::MenuItem)
                return true;
            break;

        case ControlType::MenuPopup:
            if (nPart==ControlPart::Entire
                ||  nPart==ControlPart::MenuItem
                ||  nPart==ControlPart::MenuItemCheckMark
                ||  nPart==ControlPart::MenuItemRadioMark
                ||  nPart==ControlPart::Separator
                ||  nPart==ControlPart::SubmenuArrow
            )
                return true;
            break;

//        case ControlType::Slider:
//            if(nPart == ControlPart::TrackHorzArea || nPart == ControlPart::TrackVertArea)
//                return true;
//            break;

        case ControlType::Fixedline:
            if (nPart == ControlPart::SeparatorVert || nPart == ControlPart::SeparatorHorz)
                return true;
            break;

        case ControlType::ListHeader:
            if (nPart == ControlPart::Button || nPart == ControlPart::Arrow)
                return true;
            break;
        default: break;
    }

    SAL_INFO("vcl.gtk", "Unhandled is native supported for Type:" << static_cast<int>(nType) << ", Part" << static_cast<int>(nPart));

    return false;
}

#if ENABLE_CAIRO_CANVAS

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

#endif

void GtkSalGraphics::WidgetQueueDraw() const
{
    //request gtk to sync the entire contents
    GtkWidget *pWidget = GTK_WIDGET(mpFrame->getFixedContainer());
    gtk_widget_queue_draw(pWidget);
}

namespace {

void getStyleContext(GtkStyleContext** style, GtkWidget* widget)
{
    gtk_container_add(GTK_CONTAINER(gDumbContainer), widget);
    *style = gtk_widget_get_style_context(widget);
    g_object_ref(*style);
}

}

void GtkSalData::initNWF()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maNWFData.mbFlatMenu = true;
    pSVData->maNWFData.mbDockingAreaAvoidTBFrames = true;
    pSVData->maNWFData.mbCanDrawWidgetAnySize = true;
    pSVData->maNWFData.mbDDListBoxNoTextArea = true;
    pSVData->maNWFData.mbNoFocusRects = true;
    pSVData->maNWFData.mbNoFocusRectsForFlatButtons = true;
    pSVData->maNWFData.mbAutoAccel = true;
    pSVData->maNWFData.mbEnableAccel = true;

#if defined(GDK_WINDOWING_WAYLAND)
    //gnome#768128 for the car crash that is wayland
    //and floating dockable toolbars
    GdkDisplay *pDisplay = gdk_display_get_default();
    if (DLSYM_GDK_IS_WAYLAND_DISPLAY(pDisplay))
        pSVData->maNWFData.mbCanDetermineWindowPosition = false;
#endif
}

void GtkSalData::deInitNWF()
{
    if (gCacheWindow)
        gtk_widget_destroy(gCacheWindow);
}

GtkSalGraphics::GtkSalGraphics( GtkSalFrame *pFrame, GtkWidget *pWindow )
    : SvpSalGraphics(),
      mpFrame( pFrame ),
      mpWindow( pWindow )
{
    if (style_loaded)
        return;

    style_loaded = true;

    /* Load the GtkStyleContexts, it might be a bit slow, but usually,
     * gtk apps create a lot of widgets at startup, so, it shouldn't be
     * too slow */
    gtk_widget_path_iter_set_object_nameFunc set_object_name =
        reinterpret_cast<gtk_widget_path_iter_set_object_nameFunc>(osl_getAsciiFunctionSymbol(nullptr,
            "gtk_widget_path_iter_set_object_name"));

    gCacheWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gDumbContainer = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(gCacheWindow), gDumbContainer);
    gtk_widget_realize(gDumbContainer);
    gtk_widget_realize(gCacheWindow);

    gEntryBox = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(gDumbContainer), gEntryBox);

    mpWindowStyle = createStyleContext(set_object_name, GtkControlPart::ToplevelWindow);
    mpEntryStyle = createStyleContext(set_object_name, GtkControlPart::Entry);

    getStyleContext(&mpTextViewStyle, gtk_text_view_new());

    mpButtonStyle = createStyleContext(set_object_name, GtkControlPart::Button);
    mpLinkButtonStyle = createStyleContext(set_object_name, GtkControlPart::LinkButton);

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

    mpVScrollbarStyle = createStyleContext(set_object_name, GtkControlPart::ScrollbarVertical);
    mpVScrollbarContentsStyle = createStyleContext(set_object_name, GtkControlPart::ScrollbarVerticalContents);
    mpVScrollbarTroughStyle = createStyleContext(set_object_name, GtkControlPart::ScrollbarVerticalTrough);
    mpVScrollbarSliderStyle = createStyleContext(set_object_name, GtkControlPart::ScrollbarVerticalSlider);
    mpVScrollbarButtonStyle = createStyleContext(set_object_name, GtkControlPart::ScrollbarVerticalButton);
    mpHScrollbarStyle = createStyleContext(set_object_name, GtkControlPart::ScrollbarHorizontal);
    mpHScrollbarContentsStyle = createStyleContext(set_object_name, GtkControlPart::ScrollbarHorizontalContents);
    mpHScrollbarTroughStyle = createStyleContext(set_object_name, GtkControlPart::ScrollbarHorizontalTrough);
    mpHScrollbarSliderStyle = createStyleContext(set_object_name, GtkControlPart::ScrollbarHorizontalSlider);
    mpHScrollbarButtonStyle = createStyleContext(set_object_name, GtkControlPart::ScrollbarHorizontalButton);

    mpCheckButtonStyle = createStyleContext(set_object_name, GtkControlPart::CheckButton);
    mpCheckButtonCheckStyle = createStyleContext(set_object_name, GtkControlPart::CheckButtonCheck);

    mpRadioButtonStyle = createStyleContext(set_object_name, GtkControlPart::RadioButton);
    mpRadioButtonRadioStyle = createStyleContext(set_object_name, GtkControlPart::RadioButtonRadio);

    /* Spinbutton */
    gSpinBox = gtk_spin_button_new(nullptr, 0, 0);
    gtk_container_add(GTK_CONTAINER(gDumbContainer), gSpinBox);
    mpSpinStyle = createStyleContext(set_object_name, GtkControlPart::SpinButton);
    mpSpinEntryStyle = createStyleContext(set_object_name, GtkControlPart::SpinButtonEntry);
    mpSpinUpStyle = createStyleContext(set_object_name, GtkControlPart::SpinButtonUpButton);
    mpSpinDownStyle = createStyleContext(set_object_name, GtkControlPart::SpinButtonDownButton);

    /* NoteBook */
    mpNotebookStyle = createStyleContext(set_object_name, GtkControlPart::Notebook);
    mpNotebookStackStyle = createStyleContext(set_object_name, GtkControlPart::NotebookStack);
    mpNotebookHeaderStyle = createStyleContext(set_object_name, GtkControlPart::NotebookHeader);
    mpNotebookHeaderTabsStyle = createStyleContext(set_object_name, GtkControlPart::NotebookHeaderTabs);
    mpNotebookHeaderTabsTabStyle = createStyleContext(set_object_name, GtkControlPart::NotebookHeaderTabsTab);
    mpNotebookHeaderTabsTabLabelStyle = createStyleContext(set_object_name, GtkControlPart::NotebookHeaderTabsTabLabel);
    mpNotebookHeaderTabsTabActiveLabelStyle = createStyleContext(set_object_name, GtkControlPart::NotebookHeaderTabsTabActiveLabel);
    mpNotebookHeaderTabsTabHoverLabelStyle = createStyleContext(set_object_name, GtkControlPart::NotebookHeaderTabsTabHoverLabel);

    /* Combobox */
    gComboBox = gtk_combo_box_text_new_with_entry();
    gtk_container_add(GTK_CONTAINER(gDumbContainer), gComboBox);
    mpComboboxStyle = createStyleContext(set_object_name, GtkControlPart::Combobox);
    mpComboboxBoxStyle = createStyleContext(set_object_name, GtkControlPart::ComboboxBox);
    mpComboboxEntryStyle = createStyleContext(set_object_name, GtkControlPart::ComboboxBoxEntry);
    mpComboboxButtonStyle = createStyleContext(set_object_name, GtkControlPart::ComboboxBoxButton);
    mpComboboxButtonBoxStyle = createStyleContext(set_object_name, GtkControlPart::ComboboxBoxButtonBox);
    mpComboboxButtonArrowStyle = createStyleContext(set_object_name, GtkControlPart::ComboboxBoxButtonBoxArrow);

    /* Listbox */
    gListBox = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(gListBox), "sample");
    gtk_container_add(GTK_CONTAINER(gDumbContainer), gListBox);
    mpListboxStyle = createStyleContext(set_object_name, GtkControlPart::Listbox);
    mpListboxBoxStyle = createStyleContext(set_object_name, GtkControlPart::ListboxBox);
    mpListboxButtonStyle = createStyleContext(set_object_name, GtkControlPart::ListboxBoxButton);
    mpListboxButtonBoxStyle = createStyleContext(set_object_name, GtkControlPart::ListboxBoxButtonBox);
    mpListboxButtonArrowStyle = createStyleContext(set_object_name, GtkControlPart::ListboxBoxButtonBoxArrow);

    /* Menu bar */
    gMenuBarWidget = gtk_menu_bar_new();
    gMenuItemMenuBarWidget = gtk_menu_item_new_with_label( "b" );
    gtk_menu_shell_append(GTK_MENU_SHELL(gMenuBarWidget), gMenuItemMenuBarWidget);
    gtk_container_add(GTK_CONTAINER(gDumbContainer), gMenuBarWidget);

    mpMenuBarStyle = createStyleContext(set_object_name, GtkControlPart::MenuBar);
    mpMenuBarItemStyle = createStyleContext(set_object_name, GtkControlPart::MenuBarItem);

    /* Menu */
    mpMenuWindowStyle = createStyleContext(set_object_name, GtkControlPart::MenuWindow);
    mpMenuStyle = createStyleContext(set_object_name, GtkControlPart::Menu);
    GtkWidget *menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(gMenuItemMenuBarWidget), menu);

    /* Menu Items */
    gCheckMenuItemWidget = gtk_check_menu_item_new_with_label("M");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gCheckMenuItemWidget);

    mpMenuItemStyle = createStyleContext(set_object_name, GtkControlPart::MenuItem);
    mpMenuItemLabelStyle = createStyleContext(set_object_name, GtkControlPart::MenuItemLabel);
    mpMenuItemArrowStyle = createStyleContext(set_object_name, GtkControlPart::MenuItemArrow);
    mpCheckMenuItemStyle = createStyleContext(set_object_name, GtkControlPart::CheckMenuItem);
    mpCheckMenuItemCheckStyle = createStyleContext(set_object_name, GtkControlPart::CheckMenuItemCheck);
    mpRadioMenuItemStyle = createStyleContext(set_object_name, GtkControlPart::RadioMenuItem);
    mpRadioMenuItemRadioStyle = createStyleContext(set_object_name, GtkControlPart::RadioMenuItemRadio);
    mpSeparatorMenuItemStyle = createStyleContext(set_object_name, GtkControlPart::SeparatorMenuItem);
    mpSeparatorMenuItemSeparatorStyle = createStyleContext(set_object_name, GtkControlPart::SeparatorMenuItemSeparator);

    /* Frames */
    mpFrameOutStyle = mpFrameInStyle = createStyleContext(set_object_name, GtkControlPart::FrameBorder);
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
    mpProgressBarStyle = createStyleContext(set_object_name, GtkControlPart::ProgressBar);
    mpProgressBarTroughStyle = createStyleContext(set_object_name, GtkControlPart::ProgressBarTrough);
    mpProgressBarProgressStyle = createStyleContext(set_object_name, GtkControlPart::ProgressBarProgress);

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

    if (fResolution > 0.0)
        rDPIX = rDPIY = sal_Int32(fResolution);
    else
        rDPIX = rDPIY = 96;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
