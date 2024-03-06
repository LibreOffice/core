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
#include <osl/module.h>

#include <config_cairo_canvas.h>

#include <unx/gtk/gtkframe.hxx>
#include <unx/gtk/gtkdata.hxx>
#include <unx/gtk/gtkinst.hxx>
#include <unx/gtk/gtkgdi.hxx>
#include <unx/gtk/gtkbackend.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/decoview.hxx>
#include <vcl/settings.hxx>
#include <unx/fontmanager.hxx>
#include <o3tl/string_view.hxx>

#include "gtkcairo.hxx"
#include <optional>

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
GtkStyleContext* GtkSalGraphics::mpToolbarSeparatorStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpCheckButtonStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpCheckButtonCheckStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpRadioButtonStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpRadioButtonRadioStyle = nullptr;
GtkStyleContext* GtkSalGraphics::mpSpinStyle = nullptr;
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
gint GtkSalGraphics::mnVerticalSeparatorMinWidth = 0;

#if !GTK_CHECK_VERSION(4, 0, 0)
static void style_context_get_margin(GtkStyleContext *pContext, GtkBorder *pMargin)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_style_context_get_margin(pContext, pMargin);
#else
    gtk_style_context_get_margin(pContext, gtk_style_context_get_state(pContext), pMargin);
#endif
}
#endif

#if !GTK_CHECK_VERSION(4, 0, 0)
static void style_context_get_border(GtkStyleContext* pContext, GtkBorder* pBorder)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_style_context_get_border(pContext, pBorder);
#else
    gtk_style_context_get_border(pContext, gtk_style_context_get_state(pContext), pBorder);
#endif
}
#endif

#if !GTK_CHECK_VERSION(4, 0, 0)
static void style_context_get_padding(GtkStyleContext* pContext, GtkBorder* pPadding)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_style_context_get_padding(pContext, pPadding);
#else
    gtk_style_context_get_padding(pContext, gtk_style_context_get_state(pContext), pPadding);
#endif
}
#endif

bool GtkSalGraphics::style_loaded = false;
/************************************************************************
 * State conversion
 ************************************************************************/
#if !GTK_CHECK_VERSION(4, 0, 0)
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

namespace {

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

}

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
    style_context_get_padding(mpSpinUpStyle, &padding);
    style_context_get_border(mpSpinUpStyle, &border);

    gint buttonWidth = icon_size + padding.left + padding.right +
        border.left + border.right;

    tools::Rectangle buttonRect(Point(0, aAreaRect.Top()), Size(buttonWidth, 0));
    buttonRect.setHeight(aAreaRect.GetHeight());
    tools::Rectangle partRect(buttonRect);
    if ( nPart == ControlPart::ButtonUp )
    {
        if (AllSettings::GetLayoutRTL())
            partRect.SetPosX(aAreaRect.Left());
        else
            partRect.SetPosX(aAreaRect.Left() + (aAreaRect.GetWidth() - buttonRect.GetWidth()));
    }
    else if( nPart == ControlPart::ButtonDown )
    {
        if (AllSettings::GetLayoutRTL())
            partRect.SetPosX(aAreaRect.Left() + buttonRect.GetWidth());
        else
            partRect.SetPosX(aAreaRect.Left() + (aAreaRect.GetWidth() - 2 * buttonRect.GetWidth()));
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
#endif

#if !GTK_CHECK_VERSION(4, 0, 0)
namespace
{
    void QuerySize(GtkStyleContext *pContext, Size &rSize)
    {
        GtkBorder margin, border, padding;

        style_context_get_margin(pContext, &margin);
        style_context_get_border(pContext, &border);
        style_context_get_padding(pContext, &padding);

        int nMinWidth(0), nMinHeight(0);
        GtkStateFlags stateflags = gtk_style_context_get_state (pContext);
        gtk_style_context_get(pContext, stateflags,
                "min-width", &nMinWidth, "min-height", &nMinHeight, nullptr);
        nMinWidth += margin.left + margin.right + border.left + border.right + padding.left + padding.right;
        nMinHeight += margin.top + margin.bottom + border.top + border.bottom + padding.top + padding.bottom;

        rSize = Size(std::max<tools::Long>(rSize.Width(), nMinWidth), std::max<tools::Long>(rSize.Height(), nMinHeight));
    }
}
#endif

#if !GTK_CHECK_VERSION(4, 0, 0)
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

    gint nFirst = 0;
    gint nSecond = 0;

    if ( has_forward )   nSecond += 1;
    if ( has_forward2 )  nFirst  += 1;
    if ( has_backward )  nFirst  += 1;
    if ( has_backward2 ) nSecond += 1;

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
        buttonRect.SetLeft(aAreaRect.Left());
        buttonRect.SetTop(aAreaRect.Top());
    }
    else if (nPart == ControlPart::ButtonLeft)
    {
        aSize.setWidth( aSize.Width() * nFirst );
        buttonRect.SetLeft(aAreaRect.Left());
        buttonRect.SetTop(aAreaRect.Top());
    }
    else if (nPart == ControlPart::ButtonDown)
    {
        aSize.setHeight( aSize.Height() * nSecond );
        buttonRect.SetLeft(aAreaRect.Left());
        buttonRect.SetTop(aAreaRect.Top() + aAreaRect.GetHeight() - aSize.Height());
    }
    else if (nPart == ControlPart::ButtonRight)
    {
        aSize.setWidth( aSize.Width() * nSecond );
        buttonRect.SetLeft(aAreaRect.Left() + aAreaRect.GetWidth() - aSize.Width());
        buttonRect.SetTop(aAreaRect.Top());
    }

    buttonRect.SetSize(aSize);

    return buttonRect;
}
#endif

static GtkWidget* gCacheWindow;
static GtkWidget* gDumbContainer;
#if GTK_CHECK_VERSION(4, 0, 0)
static GtkWidget* gVScrollbar;
static GtkWidget* gTextView;
#else
static GtkWidget* gComboBox;
static GtkWidget* gListBox;
static GtkWidget* gSpinBox;
static GtkWidget* gTreeViewWidget;
#endif
static GtkWidget* gHScrollbar;
static GtkWidget* gEntryBox;

namespace
{
    void style_context_set_state(GtkStyleContext* context, GtkStateFlags flags)
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        do
        {
            gtk_style_context_set_state(context, flags);
        }
        while ((context = gtk_style_context_get_parent(context)));
#else
        gtk_style_context_set_state(context, flags);
#endif
    }

    class StyleContextSave
    {
    private:
        std::vector<std::pair<GtkStyleContext*, GtkStateFlags>> m_aStates;
    public:
        void save(GtkStyleContext* context)
        {
#if !GTK_CHECK_VERSION(4, 0, 0)
            do
            {
                m_aStates.emplace_back(context, gtk_style_context_get_state(context));
            }
            while ((context = gtk_style_context_get_parent(context)));
#else
            m_aStates.emplace_back(context, gtk_style_context_get_state(context));
#endif
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

#if !GTK_CHECK_VERSION(4, 0, 0)
    tools::Rectangle render_common(GtkStyleContext *pContext, cairo_t *cr, const tools::Rectangle &rIn, GtkStateFlags flags)
    {
        if (!pContext)
            return rIn;

        gtk_style_context_set_state(pContext, flags);

        tools::Rectangle aRect(rIn);
        GtkBorder margin;
        style_context_get_margin(pContext, &margin);

        aRect.AdjustLeft(margin.left );
        aRect.AdjustTop(margin.top );
        aRect.AdjustRight( -(margin.right) );
        aRect.AdjustBottom( -(margin.bottom) );

        gtk_render_background(pContext, cr, aRect.Left(), aRect.Top(),
                                            aRect.GetWidth(), aRect.GetHeight());
        gtk_render_frame(pContext, cr, aRect.Left(), aRect.Top(),
                                       aRect.GetWidth(), aRect.GetHeight());

        GtkBorder border, padding;
        style_context_get_border(pContext, &border);
        style_context_get_padding(pContext, &padding);

        aRect.AdjustLeft(border.left + padding.left );
        aRect.AdjustTop(border.top + padding.top );
        aRect.AdjustRight( -(border.right + padding.right) );
        aRect.AdjustBottom( -(border.bottom + padding.bottom) );

        return aRect;
    }
#endif
}

#if !GTK_CHECK_VERSION(4, 0, 0)
void GtkSalGraphics::PaintScrollbar(GtkStyleContext *context,
                                    cairo_t *cr,
                                    const tools::Rectangle& rControlRectangle,
                                    ControlPart nPart,
                                    const ImplControlValue& rValue )
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
    if (scrollbarRect.IsEmpty())
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

    bool has_slider = !thumbRect.IsEmpty();

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
        if (!aBtn1Rect.IsWidthEmpty())
            aTrackRect.SetLeft( aBtn1Rect.Right() );
        if (!aBtn2Rect.IsWidthEmpty())
            aTrackRect.SetRight( aBtn2Rect.Left() );
    }
    else
    {
        tools::Rectangle aBtn1Rect = NWGetScrollButtonRect(ControlPart::ButtonUp, aTrackRect);
        tools::Rectangle aBtn2Rect = NWGetScrollButtonRect(ControlPart::ButtonDown, aTrackRect);
        if (!aBtn1Rect.IsHeightEmpty())
            aTrackRect.SetTop( aBtn1Rect.Bottom() + 1 );
        if (!aBtn2Rect.IsHeightEmpty())
            aTrackRect.SetBottom( aBtn2Rect.Top() );
    }

    GtkStyleContext* pScrollbarTroughStyle = scrollbarOrientation == GTK_ORIENTATION_VERTICAL ?
                                              mpVScrollbarTroughStyle : mpHScrollbarTroughStyle;
    gtk_render_background(pScrollbarTroughStyle, cr, aTrackRect.Left(), aTrackRect.Top(),
                          aTrackRect.GetWidth(), aTrackRect.GetHeight() );
    gtk_render_frame(pScrollbarTroughStyle, cr, aTrackRect.Left(), aTrackRect.Top(),
                     aTrackRect.GetWidth(), aTrackRect.GetHeight() );

    // ----------------- THUMB
    if ( !has_slider )
        return;

    stateFlags = NWConvertVCLStateToGTKState(rScrollbarVal.mnThumbState);
    if ( rScrollbarVal.mnThumbState & ControlState::PRESSED )
        stateFlags = static_cast<GtkStateFlags>(stateFlags | GTK_STATE_FLAG_PRELIGHT);

    GtkStyleContext* pScrollbarSliderStyle = scrollbarOrientation == GTK_ORIENTATION_VERTICAL ?
                                              mpVScrollbarSliderStyle : mpHScrollbarSliderStyle;

    gtk_style_context_set_state(pScrollbarSliderStyle, stateFlags);

    GtkBorder margin;
    style_context_get_margin(pScrollbarSliderStyle, &margin);

    gtk_render_background(pScrollbarSliderStyle, cr,
                      thumbRect.Left() + margin.left, thumbRect.Top() + margin.top,
                      thumbRect.GetWidth() - margin.left - margin.right,
                      thumbRect.GetHeight() - margin.top - margin.bottom);

    gtk_render_frame(pScrollbarSliderStyle, cr,
                      thumbRect.Left() + margin.left, thumbRect.Top() + margin.top,
                      thumbRect.GetWidth() - margin.left - margin.right,
                      thumbRect.GetHeight() - margin.top - margin.bottom);
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

    style_context_get_padding(context, &padding);
    style_context_get_border(context, &border);

    gtk_render_background(context, cr,
                          buttonRect.Left(), buttonRect.Top(),
                          buttonRect.GetWidth(), buttonRect.GetHeight() );

    gint iconWidth = buttonRect.GetWidth() - padding.left - padding.right - border.left - border.right;
    gint iconHeight = buttonRect.GetHeight() - padding.top - padding.bottom - border.top - border.bottom;

    const char* icon = (nPart == ControlPart::ButtonUp) ? "list-add-symbolic" : "list-remove-symbolic";
    GtkIconTheme *pIconTheme = gtk_icon_theme_get_for_screen(gtk_widget_get_screen(mpWindow));

    gint scale = gtk_style_context_get_scale (context);
    GtkIconInfo *info = gtk_icon_theme_lookup_icon_for_scale(pIconTheme, icon, std::min(iconWidth, iconHeight), scale,
                                                   static_cast<GtkIconLookupFlags>(0));

    GdkPixbuf *pixbuf = gtk_icon_info_load_symbolic_for_context(info, context, nullptr, nullptr);
    g_object_unref(info);

    iconWidth = gdk_pixbuf_get_width(pixbuf)/scale;
    iconHeight = gdk_pixbuf_get_height(pixbuf)/scale;
    tools::Rectangle arrowRect(buttonRect.Center() - Point(iconWidth / 2, iconHeight / 2),
                               Size(iconWidth, iconHeight));

    gtk_style_context_save (context);
    gtk_style_context_set_scale (context, 1);
    gtk_render_icon(context, cr, pixbuf, arrowRect.Left(), arrowRect.Top());
    gtk_style_context_restore (context);
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
        style_context_get_padding(mpListboxButtonStyle, &padding);
    else
        style_context_get_padding(mpButtonStyle, &padding);

    gint nArrowWidth = FALLBACK_ARROW_SIZE;
    gtk_style_context_get(mpComboboxButtonArrowStyle,
        gtk_style_context_get_state(mpComboboxButtonArrowStyle),
        "min-width", &nArrowWidth, nullptr);

    gint nButtonWidth = nArrowWidth + padding.left + padding.right;
    if( nPart == ControlPart::ButtonDown )
    {
        Point aPos(aAreaRect.Left() + aAreaRect.GetWidth() - nButtonWidth, aAreaRect.Top());
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
            render_common(mpListboxButtonStyle, cr, aRect, flags);
            render_common(mpListboxBoxStyle, cr, aRect, flags);

            gtk_render_arrow(mpListboxButtonArrowStyle, cr,
                             G_PI,
                             (arrowRect.Left() - areaRect.Left()), (arrowRect.Top() - areaRect.Top()),
                             arrowRect.GetWidth() );
        }
    }
}

static void appendComboEntry(GtkWidgetPath* pSiblingsPath)
{
    gtk_widget_path_append_type(pSiblingsPath, GTK_TYPE_ENTRY);
    gtk_widget_path_iter_set_object_name(pSiblingsPath, -1, "entry");
    gtk_widget_path_iter_add_class(pSiblingsPath, -1, "combo");
}

static void appendComboButton(GtkWidgetPath* pSiblingsPath)
{
    gtk_widget_path_append_type(pSiblingsPath, GTK_TYPE_BUTTON);
    gtk_widget_path_iter_set_object_name(pSiblingsPath, -1, "button");
    gtk_widget_path_iter_add_class(pSiblingsPath, -1, "combo");
}

static GtkWidgetPath* buildLTRComboSiblingsPath()
{
    GtkWidgetPath* pSiblingsPath = gtk_widget_path_new();

    appendComboEntry(pSiblingsPath);
    appendComboButton(pSiblingsPath);

    return pSiblingsPath;
}

static GtkWidgetPath* buildRTLComboSiblingsPath()
{
    GtkWidgetPath* pSiblingsPath = gtk_widget_path_new();

    appendComboButton(pSiblingsPath);
    appendComboEntry(pSiblingsPath);

    return pSiblingsPath;
}


GtkStyleContext* GtkSalGraphics::makeContext(GtkWidgetPath *pPath, GtkStyleContext *pParent)
{
    GtkStyleContext* context = gtk_style_context_new();
    gtk_style_context_set_screen(context, gtk_widget_get_screen(mpWindow));
    gtk_style_context_set_path(context, pPath);
    if (pParent == nullptr)
    {
        GtkWidget* pTopLevel = widget_get_toplevel(mpWindow);
        GtkStyleContext* pStyle = gtk_widget_get_style_context(pTopLevel);
        gtk_style_context_set_parent(context, pStyle);
        gtk_style_context_set_scale (context, gtk_style_context_get_scale (pStyle));
    }
    else
    {
        gtk_style_context_set_parent(context, pParent);
        gtk_style_context_set_scale (context, gtk_style_context_get_scale (pParent));
    }
    gtk_widget_path_unref(pPath);
    return context;
}

GtkStyleContext* GtkSalGraphics::createStyleContext(GtkControlPart ePart)
{
    switch (ePart)
    {
        case GtkControlPart::ToplevelWindow:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, G_TYPE_NONE);
            gtk_widget_path_iter_set_object_name(path, -1, "window");
            gtk_widget_path_iter_add_class(path, -1, "background");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::Button:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_BUTTON);
            gtk_widget_path_iter_set_object_name(path, -1, "button");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::LinkButton:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_BUTTON);
            gtk_widget_path_iter_set_object_name(path, -1, "button");
            gtk_widget_path_iter_add_class(path, -1, "link");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::CheckButton:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_CHECK_BUTTON);
            gtk_widget_path_iter_set_object_name(path, -1, "checkbutton");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::CheckButtonCheck:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpCheckButtonStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_CHECK_BUTTON);
            gtk_widget_path_iter_set_object_name(path, -1, "check");
            return makeContext(path, mpCheckButtonStyle);
        }
        case GtkControlPart::RadioButton:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_RADIO_BUTTON);
            gtk_widget_path_iter_set_object_name(path, -1, "radiobutton");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::RadioButtonRadio:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpRadioButtonStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_RADIO_BUTTON);
            gtk_widget_path_iter_set_object_name(path, -1, "radio");
            return makeContext(path, mpRadioButtonStyle);
        }
        case GtkControlPart::ComboboxBoxButtonBoxArrow:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpComboboxButtonBoxStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_RADIO_BUTTON);
            gtk_widget_path_append_type(path, GTK_TYPE_BUTTON);
            gtk_widget_path_iter_set_object_name(path, -1, "arrow");
            return makeContext(path, mpComboboxButtonBoxStyle);
        }
        case GtkControlPart::ListboxBoxButtonBoxArrow:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpListboxButtonBoxStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_RADIO_BUTTON);
            gtk_widget_path_append_type(path, GTK_TYPE_BUTTON);
            gtk_widget_path_iter_set_object_name(path, -1, "arrow");
            return makeContext(path, mpListboxButtonBoxStyle);
        }
        case GtkControlPart::Entry:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_ENTRY);
            gtk_widget_path_iter_set_object_name(path, -1, "entry");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::Combobox:
        case GtkControlPart::Listbox:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, G_TYPE_NONE);
            gtk_widget_path_iter_set_object_name(path, -1, "combobox");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::ComboboxBox:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpComboboxStyle));
            gtk_widget_path_append_type(path, G_TYPE_NONE);
            gtk_widget_path_iter_set_object_name(path, -1, "box");
            gtk_widget_path_iter_add_class(path, -1, "horizontal");
            gtk_widget_path_iter_add_class(path, -1, "linked");
            return makeContext(path, mpComboboxStyle);
        }
        case GtkControlPart::ListboxBox:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpListboxStyle));
            gtk_widget_path_append_type(path, G_TYPE_NONE);
            gtk_widget_path_iter_set_object_name(path, -1, "box");
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
                pSiblingsPath = buildRTLComboSiblingsPath();
                gtk_widget_path_append_with_siblings(path, pSiblingsPath, 1);
            }
            else
            {
                pSiblingsPath = buildLTRComboSiblingsPath();
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
                pSiblingsPath = buildRTLComboSiblingsPath();
                gtk_widget_path_append_with_siblings(path, pSiblingsPath, 0);
            }
            else
            {
                pSiblingsPath = buildLTRComboSiblingsPath();
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
            gtk_widget_path_iter_set_object_name(pSiblingsPath, -1, "button");
            gtk_widget_path_iter_add_class(pSiblingsPath, -1, "combo");

            gtk_widget_path_append_with_siblings(path, pSiblingsPath, 0);
            gtk_widget_path_unref(pSiblingsPath);
            return makeContext(path, mpListboxBoxStyle);
        }
        case GtkControlPart::ComboboxBoxButtonBox:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpComboboxButtonStyle));
            gtk_widget_path_append_type(path, G_TYPE_NONE);
            gtk_widget_path_iter_set_object_name(path, -1, "box");
            gtk_widget_path_iter_add_class(path, -1, "horizontal");
            return makeContext(path, mpComboboxButtonStyle);
        }
        case GtkControlPart::ListboxBoxButtonBox:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpListboxButtonStyle));
            gtk_widget_path_append_type(path, G_TYPE_NONE);
            gtk_widget_path_iter_set_object_name(path, -1, "box");
            gtk_widget_path_iter_add_class(path, -1, "horizontal");
            return makeContext(path, mpListboxButtonStyle);
        }
        case GtkControlPart::SpinButton:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpWindowStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_SPIN_BUTTON);
            gtk_widget_path_iter_set_object_name(path, -1, "spinbutton");
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_HORIZONTAL);
            return makeContext(path, mpWindowStyle);
        }
        case GtkControlPart::SpinButtonUpButton:
        case GtkControlPart::SpinButtonDownButton:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpSpinStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_SPIN_BUTTON);
            gtk_widget_path_iter_set_object_name(path, -1, "button");
            gtk_widget_path_iter_add_class(path, -1, ePart == GtkControlPart::SpinButtonUpButton ? "up" : "down");
            return makeContext(path, mpSpinStyle);
        }
        case GtkControlPart::ScrollbarVertical:
        case GtkControlPart::ScrollbarHorizontal:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_SCROLLBAR);
            gtk_widget_path_iter_set_object_name(path, -1, "scrollbar");
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
            gtk_widget_path_iter_set_object_name(path, -1, "contents");
            return makeContext(path, pParent);
        }
        case GtkControlPart::ScrollbarVerticalTrough:
        case GtkControlPart::ScrollbarHorizontalTrough:
        {
            GtkStyleContext *pParent =
                (ePart == GtkControlPart::ScrollbarVerticalTrough) ? mpVScrollbarContentsStyle : mpHScrollbarContentsStyle;
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(pParent));
            gtk_widget_path_append_type(path, GTK_TYPE_SCROLLBAR);
            gtk_widget_path_iter_set_object_name(path, -1, "trough");
            return makeContext(path, pParent);
        }
        case GtkControlPart::ScrollbarVerticalSlider:
        case GtkControlPart::ScrollbarHorizontalSlider:
        {
            GtkStyleContext *pParent =
                (ePart == GtkControlPart::ScrollbarVerticalSlider) ? mpVScrollbarTroughStyle : mpHScrollbarTroughStyle;
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(pParent));
            gtk_widget_path_append_type(path, GTK_TYPE_SCROLLBAR);
            gtk_widget_path_iter_set_object_name(path, -1, "slider");
            return makeContext(path, pParent);
        }
        case GtkControlPart::ScrollbarVerticalButton:
        case GtkControlPart::ScrollbarHorizontalButton:
        {
            GtkStyleContext *pParent =
                (ePart == GtkControlPart::ScrollbarVerticalButton) ? mpVScrollbarStyle : mpHScrollbarStyle;
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(pParent));
            gtk_widget_path_append_type(path, GTK_TYPE_SCROLLBAR);
            gtk_widget_path_iter_set_object_name(path, -1, "button");
            return makeContext(path, pParent);
        }
        case GtkControlPart::ProgressBar:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_PROGRESS_BAR);
            gtk_widget_path_iter_set_object_name(path, -1, "progressbar");
            gtk_widget_path_iter_add_class(path, -1, GTK_STYLE_CLASS_HORIZONTAL);
            return makeContext(path, nullptr);
        }
        case GtkControlPart::ProgressBarTrough:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpProgressBarStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_PROGRESS_BAR);
            gtk_widget_path_iter_set_object_name(path, -1, "trough");
            return makeContext(path, mpProgressBarStyle);
        }
        case GtkControlPart::ProgressBarProgress:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpProgressBarTroughStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_PROGRESS_BAR);
            gtk_widget_path_iter_set_object_name(path, -1, "progress");
            return makeContext(path, mpProgressBarTroughStyle);
        }
        case GtkControlPart::Notebook:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpWindowStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_NOTEBOOK);
            gtk_widget_path_iter_set_object_name(path, -1, "notebook");
            return makeContext(path, mpWindowStyle);
        }
        case GtkControlPart::NotebookStack:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpNotebookStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_NOTEBOOK);
            gtk_widget_path_iter_set_object_name(path, -1, "stack");
            return makeContext(path, mpNotebookStyle);
        }
        case GtkControlPart::NotebookHeader:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpNotebookStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_NOTEBOOK);
            gtk_widget_path_iter_set_object_name(path, -1, "header");
            gtk_widget_path_iter_add_class(path, -1, "frame");
            gtk_widget_path_iter_add_class(path, -1, "top");
            return makeContext(path, mpNotebookStyle);
        }
        case GtkControlPart::NotebookHeaderTabs:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpNotebookHeaderStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_NOTEBOOK);
            gtk_widget_path_iter_set_object_name(path, -1, "tabs");
            gtk_widget_path_iter_add_class(path, -1, "top");
            return makeContext(path, mpNotebookHeaderStyle);
        }
        case GtkControlPart::NotebookHeaderTabsTab:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpNotebookHeaderTabsStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_NOTEBOOK);
            gtk_widget_path_iter_set_object_name(path, -1, "tab");
            gtk_widget_path_iter_add_class(path, -1, "top");
            return makeContext(path, mpNotebookHeaderTabsStyle);
        }
        case GtkControlPart::NotebookHeaderTabsTabLabel:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpNotebookHeaderTabsTabStyle));
            gtk_widget_path_append_type(path, G_TYPE_NONE);
            gtk_widget_path_iter_set_object_name(path, -1, "label");
            return makeContext(path, mpNotebookHeaderTabsTabStyle);
        }
        case GtkControlPart::NotebookHeaderTabsTabActiveLabel:
        case GtkControlPart::NotebookHeaderTabsTabHoverLabel:
            return mpNotebookHeaderTabsTabLabelStyle;
        case GtkControlPart::FrameBorder:
        {
            GtkWidgetPath *path = gtk_widget_path_new();
            gtk_widget_path_append_type(path, GTK_TYPE_FRAME);
            gtk_widget_path_iter_set_object_name(path, -1, "frame");
            gtk_widget_path_iter_add_class(path, -1, "frame");
            return makeContext(path, nullptr);
        }
        case GtkControlPart::MenuBar:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpWindowStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_MENU_BAR);
            gtk_widget_path_iter_set_object_name(path, -1, "menubar");
            return makeContext(path, mpWindowStyle);
        }
        case GtkControlPart::MenuBarItem:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuBarStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_MENU_ITEM);
            gtk_widget_path_iter_set_object_name(path, -1, "menuitem");
            return makeContext(path, mpMenuBarStyle);
        }
        case GtkControlPart::MenuWindow:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuBarItemStyle));
            gtk_widget_path_append_type(path, G_TYPE_NONE);
            gtk_widget_path_iter_set_object_name(path, -1, "window");
            gtk_widget_path_iter_add_class(path, -1, "background");
            gtk_widget_path_iter_add_class(path, -1, "popup");
            return makeContext(path, mpMenuBarItemStyle);
        }
        case GtkControlPart::Menu:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuWindowStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_MENU);
            gtk_widget_path_iter_set_object_name(path, -1, "menu");
            return makeContext(path, mpMenuWindowStyle);
        }
        case GtkControlPart::MenuItem:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_MENU_ITEM);
            gtk_widget_path_iter_set_object_name(path, -1, "menuitem");
            return makeContext(path, mpMenuStyle);
        }
        case GtkControlPart::MenuItemLabel:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuItemStyle));
            gtk_widget_path_append_type(path, G_TYPE_NONE);
            gtk_widget_path_iter_set_object_name(path, -1, "label");
            return makeContext(path, mpMenuItemStyle);
        }
        case GtkControlPart::MenuItemArrow:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuItemStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_MENU_ITEM);
            gtk_widget_path_iter_set_object_name(path, -1, "arrow");
            return makeContext(path, mpMenuItemStyle);
        }
        case GtkControlPart::CheckMenuItem:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_CHECK_MENU_ITEM);
            gtk_widget_path_iter_set_object_name(path, -1, "menuitem");
            return makeContext(path, mpMenuStyle);
        }
        case GtkControlPart::CheckMenuItemCheck:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpCheckMenuItemStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_CHECK_MENU_ITEM);
            gtk_widget_path_iter_set_object_name(path, -1, "check");
            return makeContext(path, mpCheckMenuItemStyle);
        }
        case GtkControlPart::RadioMenuItem:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_RADIO_MENU_ITEM);
            gtk_widget_path_iter_set_object_name(path, -1, "menuitem");
            return makeContext(path, mpMenuStyle);
        }
        case GtkControlPart::RadioMenuItemRadio:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpRadioMenuItemStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_RADIO_MENU_ITEM);
            gtk_widget_path_iter_set_object_name(path, -1, "radio");
            return makeContext(path, mpRadioMenuItemStyle);
        }
        case GtkControlPart::SeparatorMenuItem:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpMenuStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_SEPARATOR_MENU_ITEM);
            gtk_widget_path_iter_set_object_name(path, -1, "menuitem");
            return makeContext(path, mpMenuStyle);
        }
        case GtkControlPart::SeparatorMenuItemSeparator:
        {
            GtkWidgetPath *path = gtk_widget_path_copy(gtk_style_context_get_path(mpSeparatorMenuItemStyle));
            gtk_widget_path_append_type(path, GTK_TYPE_SEPARATOR_MENU_ITEM);
            gtk_widget_path_iter_set_object_name(path, -1, "separator");
            return makeContext(path, mpSeparatorMenuItemStyle);
        }
    }

    return nullptr;
}

#ifndef GTK_STYLE_CLASS_POPUP
constexpr OUStringLiteral GTK_STYLE_CLASS_POPUP = u"popup";
#endif
#ifndef GTK_STYLE_CLASS_LABEL
constexpr OUStringLiteral GTK_STYLE_CLASS_LABEL = u"label";
#endif

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
    gint min_width, min_weight;
    gtk_style_context_get_style(context, "min-width", &min_width, nullptr);
    gtk_style_context_get_style(context, "min-height", &min_weight, nullptr);
    gfloat arrow_size = 11 * MAX (min_width, min_weight);
    return arrow_size;
}

namespace
{
    void draw_vertical_separator(GtkStyleContext *context, cairo_t *cr, const tools::Rectangle& rControlRegion, gint nSeparatorWidth)
    {
        tools::Long nX = 0;
        tools::Long nY = 0;

        gint nHalfSeparatorWidth = nSeparatorWidth / 2;
        gint nHalfRegionWidth = rControlRegion.GetWidth() / 2;

        nX = nX + nHalfRegionWidth - nHalfSeparatorWidth;
        nY = rControlRegion.GetHeight() > 5 ? 1 : 0;
        int nHeight = rControlRegion.GetHeight() - (2 * nY);

        gtk_render_background(context, cr, nX, nY, nSeparatorWidth, nHeight);
        gtk_render_frame(context, cr, nX, nY, nSeparatorWidth, nHeight);
    }

    void draw_horizontal_separator(GtkStyleContext *context, cairo_t *cr, const tools::Rectangle& rControlRegion)
    {
        tools::Long nX = 0;
        tools::Long nY = 0;

        gint nSeparatorHeight = 1;

        gtk_style_context_get(context,
            gtk_style_context_get_state(context),
            "min-height", &nSeparatorHeight, nullptr);

        gint nHalfSeparatorHeight = nSeparatorHeight / 2;
        gint nHalfRegionHeight = rControlRegion.GetHeight() / 2;

        nY = nY + nHalfRegionHeight - nHalfSeparatorHeight;
        nX = rControlRegion.GetWidth() > 5 ? 1 : 0;
        int nWidth = rControlRegion.GetWidth() - (2 * nX);

        gtk_render_background(context, cr, nX, nY, nWidth, nSeparatorHeight);
        gtk_render_frame(context, cr, nX, nY, nWidth, nSeparatorHeight);
    }
}
#endif

void GtkSalGraphics::handleDamage(const tools::Rectangle& rDamagedRegion)
{
    assert(m_pWidgetDraw);
    assert(!rDamagedRegion.IsEmpty());
    mpFrame->damaged(rDamagedRegion.Left(), rDamagedRegion.Top(), rDamagedRegion.GetWidth(), rDamagedRegion.GetHeight());
}

#if !GTK_CHECK_VERSION(4, 0, 0)
bool GtkSalGraphics::drawNativeControl( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion,
                                            ControlState nState, const ImplControlValue& rValue,
                                            const OUString&, const Color& rBackgroundColor)
{
    RenderType renderType = nPart == ControlPart::Focus ? RenderType::Focus : RenderType::BackgroundAndFrame;
    GtkStyleContext *context = nullptr;
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
            context = mpCheckMenuItemCheckStyle;
            renderType = RenderType::Check;
            nType = ControlType::Checkbox;
            if (nState & ControlState::PRESSED)
            {
                flags = static_cast<GtkStateFlags>(flags | GTK_STATE_FLAG_CHECKED);
            }
            break;
        case ControlPart::MenuItemRadioMark:
            context = mpRadioMenuItemRadioStyle;
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
            context = mpMenuItemArrowStyle;
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
                    ( (rValue.getTristateVal() == ButtonValue::On) ? GTK_STATE_FLAG_CHECKED : GTK_STATE_FLAG_NORMAL));
            context = mpToolButtonStyle;
            break;
        case ControlPart::SeparatorVert:
            context = mpToolbarSeparatorStyle;
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
            flags = static_cast<GtkStateFlags>(flags | GTK_STATE_FLAG_CHECKED);
        renderType = RenderType::TabItem;
        break;
    case ControlType::WindowBackground:
        context = gtk_widget_get_style_context(widget_get_toplevel(mpWindow));
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
            pixbuf = gtk_icon_theme_load_icon_for_scale(pIconTheme, icon,
                                                        std::max(rControlRegion.GetWidth(), rControlRegion.GetHeight()),
                                                        gtk_style_context_get_scale (context),
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

    cairo_t *cr = getCairoContext();
    clipRegion(cr);
    cairo_translate(cr, rControlRegion.Left(), rControlRegion.Top());

    tools::Long nX = 0;
    tools::Long nY = 0;
    tools::Long nWidth = rControlRegion.GetWidth();
    tools::Long nHeight = rControlRegion.GetHeight();

    StyleContextSave aContextState;
    aContextState.save(context);
    style_context_set_state(context, flags);

    // apply background in style, if explicitly set
    // note: for more complex controls that use multiple styles for their elements,
    // background may have to be applied for more of those as well (s. case RenderType::Combobox below)
    GtkCssProvider* pBgCssProvider = nullptr;
    if (rBackgroundColor != COL_AUTO)
    {
        const OUString sColorCss = "* { background-color: #" + rBackgroundColor.AsRGBHexString() + "; }";
        const OString aResult = OUStringToOString(sColorCss, RTL_TEXTENCODING_UTF8);
        pBgCssProvider =  gtk_css_provider_new();
        css_provider_load_from_data(pBgCssProvider, aResult.getStr(), aResult.getLength());
        gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(pBgCssProvider),
                                       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
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
        draw_vertical_separator(context, cr, rControlRegion, mnVerticalSeparatorMinWidth);
        break;
    }
    case RenderType::Separator:
        if (nPart == ControlPart::SeparatorHorz)
            draw_horizontal_separator(context, cr, rControlRegion);
        else
            draw_vertical_separator(context, cr, rControlRegion, mnVerticalSeparatorMinWidth);
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
        if (pBgCssProvider)
        {
            if (nType == ControlType::Combobox)
            {
                gtk_style_context_add_provider(mpComboboxEntryStyle, GTK_STYLE_PROVIDER(pBgCssProvider),
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            }
            else if (nType == ControlType::Listbox)
            {
                gtk_style_context_add_provider(mpListboxBoxStyle, GTK_STYLE_PROVIDER(pBgCssProvider),
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            }
        }
        PaintCombobox(flags, cr, rControlRegion, nType, nPart);
        if (pBgCssProvider)
        {
            if (nType == ControlType::Combobox)
                gtk_style_context_remove_provider(mpComboboxEntryStyle, GTK_STYLE_PROVIDER(pBgCssProvider));
            else if (nType == ControlType::Listbox)
                gtk_style_context_remove_provider(mpListboxBoxStyle, GTK_STYLE_PROVIDER(pBgCssProvider));
        }
        break;
    case RenderType::Icon:
        gtk_style_context_save (context);
        gtk_style_context_set_scale (context, 1);
        gtk_render_icon(context, cr, pixbuf, nX, nY);
        gtk_style_context_restore (context);
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

            style_context_get_border(context, &border);

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

        tools::Long nProgressWidth = rValue.getNumericVal();
        if (nProgressWidth)
        {
            GtkBorder padding;
            style_context_get_padding(context, &padding);

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
        gint initial_gap(0);
        gtk_style_context_get_style(mpNotebookStyle,
                                "initial-gap", &initial_gap,
                                nullptr);

        nX += initial_gap/2;
        nWidth -= initial_gap;
        tools::Rectangle aRect(Point(nX, nY), Size(nWidth, nHeight));
        render_common(mpNotebookHeaderTabsTabStyle, cr, aRect, flags);
        break;
    }
    default:
        break;
    }

    if (pBgCssProvider)
    {
        gtk_style_context_remove_provider(context, GTK_STYLE_PROVIDER(pBgCssProvider));
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
    tools::Long nHeight = std::max<tools::Long>(rControlRegion.GetHeight(), aReq.height);
    return tools::Rectangle(rControlRegion.TopLeft(), Size(rControlRegion.GetWidth(), nHeight));
}

static tools::Rectangle AdjustRectForTextBordersPadding(GtkStyleContext* pStyle, tools::Long nContentWidth, tools::Long nContentHeight, const tools::Rectangle& rControlRegion)
{
    GtkBorder border;
    style_context_get_border(pStyle, &border);

    GtkBorder padding;
    style_context_get_padding(pStyle, &padding);

    gint nWidgetHeight = nContentHeight + padding.top + padding.bottom + border.top + border.bottom;
    nWidgetHeight = std::max(std::max<gint>(nWidgetHeight, rControlRegion.GetHeight()), 34);

    gint nWidgetWidth = nContentWidth + padding.left + padding.right + border.left + border.right;
    nWidgetWidth = std::max<gint>(nWidgetWidth, rControlRegion.GetWidth());

    tools::Rectangle aEditRect(rControlRegion.TopLeft(), Size(nWidgetWidth, nWidgetHeight));

    return aEditRect;
}

bool GtkSalGraphics::getNativeControlRegion( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion, ControlState,
                                                const ImplControlValue& rValue, const OUString&,
                                                tools::Rectangle &rNativeBoundingRegion, tools::Rectangle &rNativeContentRegion )
{
    /* TODO: all this functions needs improvements */
    tools::Rectangle aEditRect = rControlRegion;
    gint indicator_size, indicator_spacing;

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
        style_context_get_border(pButtonStyle, &border);

        GtkBorder padding;
        style_context_get_padding(pButtonStyle, &padding);


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

            gint point = MAX(0, rControlRegion.GetHeight() - indicator_size);
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

        GtkBorder padding;
        style_context_get_padding(mpFrameInStyle, &padding);

        GtkBorder border;
        style_context_get_border(mpFrameInStyle, &border);

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
    {
        return false;
    }

    rNativeBoundingRegion = aEditRect;
    rNativeContentRegion = rNativeBoundingRegion;

    return true;
}
#endif

/************************************************************************
 * helper for GtkSalFrame
 ************************************************************************/
static ::Color getColor( const GdkRGBA& rCol )
{
    return ::Color( static_cast<int>(rCol.red * 0xFFFF) >> 8, static_cast<int>(rCol.green * 0xFFFF) >> 8, static_cast<int>(rCol.blue * 0xFFFF) >> 8 );
}

static ::Color style_context_get_background_color(GtkStyleContext* pStyle)
{
#if !GTK_CHECK_VERSION(4, 0, 0)
    GdkRGBA background_color;
    gtk_style_context_get_background_color(pStyle, gtk_style_context_get_state(pStyle), &background_color);
    return getColor(background_color);
#else
    cairo_surface_t *target = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
    cairo_t* cr = cairo_create(target);
    gtk_render_background(pStyle, cr, 0, 0, 1, 1);
    cairo_destroy(cr);

    cairo_surface_flush(target);
    vcl::bitmap::lookup_table const & unpremultiply_table = vcl::bitmap::get_unpremultiply_table();
    unsigned char *data = cairo_image_surface_get_data(target);
    sal_uInt8 a = data[SVP_CAIRO_ALPHA];
    sal_uInt8 b = unpremultiply_table[a][data[SVP_CAIRO_BLUE]];
    sal_uInt8 g = unpremultiply_table[a][data[SVP_CAIRO_GREEN]];
    sal_uInt8 r = unpremultiply_table[a][data[SVP_CAIRO_RED]];
    Color aColor(r, g, b);
    cairo_surface_destroy(target);

    return aColor;
#endif
}

#if !GTK_CHECK_VERSION(4, 0, 0)
static vcl::Font getFont(GtkStyleContext* pStyle, const css::lang::Locale& rLocale)
{
    const PangoFontDescription* font = gtk_style_context_get_font(pStyle, gtk_style_context_get_state(pStyle));
    return pango_to_vcl(font, rLocale);
}
#endif

vcl::Font pango_to_vcl(const PangoFontDescription* font, const css::lang::Locale& rLocale)
{
    OString    aFamily        = pango_font_description_get_family( font );
    PangoStyle    eStyle    = pango_font_description_get_style( font );
    PangoWeight    eWeight    = pango_font_description_get_weight( font );
    PangoStretch eStretch = pango_font_description_get_stretch( font );

    FontAttributes aDFA;

    // set family name
    aDFA.SetFamilyName(OStringToOUString(aFamily, RTL_TEXTENCODING_UTF8));

    // set italic
    switch( eStyle )
    {
        case PANGO_STYLE_NORMAL:    aDFA.SetItalic(ITALIC_NONE);break;
        case PANGO_STYLE_ITALIC:    aDFA.SetItalic(ITALIC_NORMAL);break;
        case PANGO_STYLE_OBLIQUE:    aDFA.SetItalic(ITALIC_OBLIQUE);break;
    }

    // set weight
    if( eWeight <= PANGO_WEIGHT_ULTRALIGHT )
        aDFA.SetWeight(WEIGHT_ULTRALIGHT);
    else if( eWeight <= PANGO_WEIGHT_LIGHT )
        aDFA.SetWeight(WEIGHT_LIGHT);
    else if( eWeight <= PANGO_WEIGHT_NORMAL )
        aDFA.SetWeight(WEIGHT_NORMAL);
    else if( eWeight <= PANGO_WEIGHT_BOLD )
        aDFA.SetWeight(WEIGHT_BOLD);
    else
        aDFA.SetWeight(WEIGHT_ULTRABOLD);

    // set width
    switch( eStretch )
    {
        case PANGO_STRETCH_ULTRA_CONDENSED:    aDFA.SetWidthType(WIDTH_ULTRA_CONDENSED);break;
        case PANGO_STRETCH_EXTRA_CONDENSED:    aDFA.SetWidthType(WIDTH_EXTRA_CONDENSED);break;
        case PANGO_STRETCH_CONDENSED:        aDFA.SetWidthType(WIDTH_CONDENSED);break;
        case PANGO_STRETCH_SEMI_CONDENSED:    aDFA.SetWidthType(WIDTH_SEMI_CONDENSED);break;
        case PANGO_STRETCH_NORMAL:            aDFA.SetWidthType(WIDTH_NORMAL);break;
        case PANGO_STRETCH_SEMI_EXPANDED:    aDFA.SetWidthType(WIDTH_SEMI_EXPANDED);break;
        case PANGO_STRETCH_EXPANDED:        aDFA.SetWidthType(WIDTH_EXPANDED);break;
        case PANGO_STRETCH_EXTRA_EXPANDED:    aDFA.SetWidthType(WIDTH_EXTRA_EXPANDED);break;
        case PANGO_STRETCH_ULTRA_EXPANDED:    aDFA.SetWidthType(WIDTH_ULTRA_EXPANDED);break;
    }

#if OSL_DEBUG_LEVEL > 1
    SAL_INFO("vcl.gtk3", "font name BEFORE system match: \""
            << aFamily << "\".");
#endif

    // match font to e.g. resolve "Sans"
    bool bFound = psp::PrintFontManager::get().matchFont(aDFA, rLocale);

#if OSL_DEBUG_LEVEL > 1
    SAL_INFO("vcl.gtk3", "font match "
            << (bFound ? "succeeded" : "failed")
            << ", name AFTER: \""
            << aDFA.GetFamilyName()
            << "\".");
#else
    (void) bFound;
#endif

    int nPangoHeight = pango_font_description_get_size(font) / PANGO_SCALE;

    if (pango_font_description_get_size_is_absolute(font))
    {
        const sal_Int32 nDPIY = 96;
        nPangoHeight = nPangoHeight * 72;
        nPangoHeight = nPangoHeight + nDPIY / 2;
        nPangoHeight = nPangoHeight / nDPIY;
    }

    vcl::Font aFont(aDFA.GetFamilyName(), Size(0, nPangoHeight));
    if (aDFA.GetWeight() != WEIGHT_DONTKNOW)
        aFont.SetWeight(aDFA.GetWeight());
    if (aDFA.GetWidthType() != WIDTH_DONTKNOW)
        aFont.SetWidthType(aDFA.GetWidthType());
    if (aDFA.GetItalic() != ITALIC_DONTKNOW)
        aFont.SetItalic(aDFA.GetItalic());
    if (aDFA.GetPitch() != PITCH_DONTKNOW)
        aFont.SetPitch(aDFA.GetPitch());
    return aFont;
}

bool GtkSalGraphics::updateSettings(AllSettings& rSettings)
{
    GtkWidget* pTopLevel = widget_get_toplevel(mpWindow);
    GtkStyleContext* pStyle = gtk_widget_get_style_context(pTopLevel);
    StyleContextSave aContextState;
    aContextState.save(pStyle);
    GtkSettings* pSettings = gtk_widget_get_settings(pTopLevel);
    StyleSettings aStyleSet = rSettings.GetStyleSettings();

    // text colors
    GdkRGBA text_color;
    style_context_set_state(pStyle, GTK_STATE_FLAG_NORMAL);
    style_context_get_color(pStyle, &text_color);
    ::Color aTextColor = getColor( text_color );
    aStyleSet.SetDialogTextColor( aTextColor );
    aStyleSet.SetButtonTextColor( aTextColor );
    aStyleSet.SetDefaultActionButtonTextColor(aTextColor);
    aStyleSet.SetActionButtonTextColor(aTextColor);
    aStyleSet.SetListBoxWindowTextColor( aTextColor );
    aStyleSet.SetRadioCheckTextColor( aTextColor );
    aStyleSet.SetGroupTextColor( aTextColor );
    aStyleSet.SetLabelTextColor( aTextColor );
    aStyleSet.SetWindowTextColor( aTextColor );
    aStyleSet.SetFieldTextColor( aTextColor );

    // background colors
    ::Color aBackColor = style_context_get_background_color(pStyle);
    aStyleSet.BatchSetBackgrounds( aBackColor );

    // UI font
#if GTK_CHECK_VERSION(4, 0, 0)
    gchar* pFontname = nullptr;
    g_object_get(pSettings, "gtk-font-name", &pFontname, nullptr);
    PangoFontDescription* pFontDesc = pango_font_description_from_string(pFontname);
    g_free(pFontname);
    vcl::Font aFont(pango_to_vcl(pFontDesc, rSettings.GetUILanguageTag().getLocale()));
    pango_font_description_free(pFontDesc);
#else
    vcl::Font aFont(getFont(pStyle, rSettings.GetUILanguageTag().getLocale()));
#endif

    aStyleSet.BatchSetFonts( aFont, aFont);

    aFont.SetWeight( WEIGHT_BOLD );
    aStyleSet.SetTitleFont( aFont );
    aStyleSet.SetFloatTitleFont( aFont );

    // mouse over text colors
    style_context_set_state(pStyle, GTK_STATE_FLAG_PRELIGHT);
    style_context_get_color(pStyle, &text_color);
    aTextColor = getColor(text_color);
    aStyleSet.SetDefaultButtonTextColor(aTextColor);
    aStyleSet.SetDefaultButtonRolloverTextColor(aTextColor);
    aStyleSet.SetDefaultButtonPressedRolloverTextColor(aTextColor);
    aStyleSet.SetButtonRolloverTextColor(aTextColor);
    aStyleSet.SetDefaultActionButtonRolloverTextColor(aTextColor);
    aStyleSet.SetDefaultActionButtonPressedRolloverTextColor(aTextColor);
    aStyleSet.SetActionButtonRolloverTextColor(aTextColor);
    aStyleSet.SetActionButtonPressedRolloverTextColor(aTextColor);
    aStyleSet.SetFlatButtonTextColor(aTextColor);
    aStyleSet.SetFlatButtonPressedRolloverTextColor(aTextColor);
    aStyleSet.SetFlatButtonRolloverTextColor(aTextColor);
    aStyleSet.SetFieldRolloverTextColor(aTextColor);

    aContextState.restore();

    // button mouse over colors
    {
        GdkRGBA normal_button_rollover_text_color, pressed_button_rollover_text_color;
        aContextState.save(mpButtonStyle);
        style_context_set_state(mpButtonStyle, GTK_STATE_FLAG_PRELIGHT);
        style_context_get_color(mpButtonStyle, &normal_button_rollover_text_color);
        aTextColor = getColor(normal_button_rollover_text_color);
        aStyleSet.SetButtonRolloverTextColor( aTextColor );
        style_context_set_state(mpButtonStyle, static_cast<GtkStateFlags>(GTK_STATE_FLAG_PRELIGHT | GTK_STATE_FLAG_ACTIVE));
        style_context_get_color(mpButtonStyle, &pressed_button_rollover_text_color);
        aTextColor = getColor(pressed_button_rollover_text_color);
        style_context_set_state(mpButtonStyle, GTK_STATE_FLAG_NORMAL);
        aStyleSet.SetButtonPressedRolloverTextColor( aTextColor );
        aContextState.restore();
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    // tooltip colors
    {
        GtkWidgetPath *pCPath = gtk_widget_path_new();
        guint pos = gtk_widget_path_append_type(pCPath, GTK_TYPE_WINDOW);
        gtk_widget_path_iter_add_class(pCPath, pos, GTK_STYLE_CLASS_TOOLTIP);
        pos = gtk_widget_path_append_type (pCPath, GTK_TYPE_LABEL);
        gtk_widget_path_iter_add_class(pCPath, pos, GTK_STYLE_CLASS_LABEL);
        GtkStyleContext *pCStyle = makeContext (pCPath, nullptr);
        aContextState.save(pCStyle);

        GdkRGBA tooltip_fg_color;
        style_context_set_state(pCStyle, GTK_STATE_FLAG_NORMAL);
        style_context_get_color(pCStyle, &tooltip_fg_color);
        ::Color aTooltipBgColor = style_context_get_background_color(pCStyle);

        aContextState.restore();
        g_object_unref( pCStyle );

        aStyleSet.SetHelpColor(aTooltipBgColor);
        aStyleSet.SetHelpTextColor( getColor( tooltip_fg_color ));
    }
#endif

    GdkRGBA color;
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        // construct style context for text view
        GtkWidgetPath *pCPath = gtk_widget_path_new();
        gtk_widget_path_append_type( pCPath, GTK_TYPE_TEXT_VIEW );
        gtk_widget_path_iter_add_class( pCPath, -1, GTK_STYLE_CLASS_VIEW );
        GtkStyleContext *pCStyle = makeContext( pCPath, nullptr );
#else
        GtkStyleContext *pCStyle = gtk_widget_get_style_context(gTextView);
#endif
        aContextState.save(pCStyle);

        // highlighting colors
        style_context_set_state(pCStyle, GTK_STATE_FLAG_SELECTED);
        ::Color aHighlightColor = style_context_get_background_color(pCStyle);
        style_context_get_color(pCStyle, &text_color);
        ::Color aHighlightTextColor = getColor( text_color );
        aStyleSet.SetAccentColor( aHighlightColor ); // https://debugpointnews.com/gnome-native-accent-colour-announcement/
        aStyleSet.SetHighlightColor( aHighlightColor );
        aStyleSet.SetHighlightTextColor( aHighlightTextColor );
        aStyleSet.SetListBoxWindowHighlightColor( aHighlightColor );
        aStyleSet.SetListBoxWindowHighlightTextColor( aHighlightTextColor );
        // make active like highlight, except with a small contrast. Note, see
        // a GtkListBoxRow in a GtkStackSidebar for a gtk widget with a
        // difference between highlighted and highlighted with focus.
        aHighlightColor.IncreaseLuminance(16);
        aStyleSet.SetActiveColor( aHighlightColor );
        aStyleSet.SetActiveTextColor( aHighlightTextColor );

        // warning color
        GdkRGBA warning_color;
        if (gtk_style_context_lookup_color(pCStyle, "warning_color", &warning_color))
            aStyleSet.SetWarningColor(getColor(warning_color));

        // field background color
        style_context_set_state(pCStyle, GTK_STATE_FLAG_NORMAL);
        ::Color aBackFieldColor = style_context_get_background_color(pCStyle);
        aStyleSet.SetFieldColor( aBackFieldColor );
        // This baby is the default page/paper color
        aStyleSet.SetWindowColor( aBackFieldColor );
        // listbox background color
        aStyleSet.SetListBoxWindowBackgroundColor( aBackFieldColor );

#if GTK_CHECK_VERSION(4, 0, 0)
        double caretAspectRatio = 0.04f;
        g_object_get(pSettings, "gtk-cursor-aspect-ratio", &caretAspectRatio, nullptr);
#else
        // Cursor width
        gfloat caretAspectRatio = 0.04f;
        gtk_style_context_get_style( pCStyle, "cursor-aspect-ratio", &caretAspectRatio, nullptr );
#endif
        // Assume 20px tall for the ratio computation, which should give reasonable results
        aStyleSet.SetCursorSize(20 * caretAspectRatio + 1);

        // Dark shadow color
        style_context_set_state(pCStyle, GTK_STATE_FLAG_INSENSITIVE);
        style_context_get_color(pCStyle, &color);
        ::Color aDarkShadowColor = getColor( color );
        aStyleSet.SetDarkShadowColor( aDarkShadowColor );

        ::Color aShadowColor(aBackColor);
        if (aDarkShadowColor.GetLuminance() > aBackColor.GetLuminance())
            aShadowColor.IncreaseLuminance(64);
        else
            aShadowColor.DecreaseLuminance(64);
        aStyleSet.SetShadowColor(aShadowColor);

        aContextState.restore();
#if !GTK_CHECK_VERSION(4, 0, 0)
        g_object_unref( pCStyle );
#endif

        // Tab colors
        aStyleSet.SetActiveTabColor( aBackFieldColor ); // same as the window color.
        aStyleSet.SetInactiveTabColor( aBackColor );
    }

    // menu disabled entries handling
    aStyleSet.SetSkipDisabledInMenus( true );
    aStyleSet.SetPreferredContextMenuShortcuts( false );

#if !GTK_CHECK_VERSION(4, 0, 0)
    aContextState.save(mpMenuItemLabelStyle);

    // menu colors
    style_context_set_state(mpMenuStyle, GTK_STATE_FLAG_NORMAL);
    aBackColor = style_context_get_background_color(mpMenuStyle);
    aStyleSet.SetMenuColor( aBackColor );

    // menu bar
    style_context_set_state(mpMenuBarStyle, GTK_STATE_FLAG_NORMAL);
    aBackColor = style_context_get_background_color(mpMenuBarStyle);
    aStyleSet.SetMenuBarColor( aBackColor );
    aStyleSet.SetMenuBarRolloverColor( aBackColor );

    style_context_set_state(mpMenuBarItemStyle, GTK_STATE_FLAG_NORMAL);
    style_context_get_color(mpMenuBarItemStyle, &text_color);
    aTextColor = aStyleSet.GetPersonaMenuBarTextColor().value_or( getColor( text_color ) );
    aStyleSet.SetMenuBarTextColor( aTextColor );
    aStyleSet.SetMenuBarRolloverTextColor( aTextColor );

    style_context_set_state(mpMenuBarItemStyle, GTK_STATE_FLAG_PRELIGHT);
    style_context_get_color(mpMenuBarItemStyle, &text_color);
    aTextColor = aStyleSet.GetPersonaMenuBarTextColor().value_or( getColor( text_color ) );
    aStyleSet.SetMenuBarHighlightTextColor( aTextColor );

    // menu items
    style_context_set_state(mpMenuItemLabelStyle, GTK_STATE_FLAG_NORMAL);
    style_context_get_color(mpMenuItemLabelStyle, &color);
    aTextColor = getColor(color);
    aStyleSet.SetMenuTextColor(aTextColor);

    style_context_set_state(mpMenuItemLabelStyle, GTK_STATE_FLAG_PRELIGHT);
    ::Color aHighlightColor = style_context_get_background_color(mpMenuItemLabelStyle);
    aStyleSet.SetMenuHighlightColor( aHighlightColor );

    style_context_get_color(mpMenuItemLabelStyle, &color);
    ::Color aHighlightTextColor = getColor( color );
    aStyleSet.SetMenuHighlightTextColor( aHighlightTextColor );

    aContextState.restore();
#endif

    // hyperlink colors
    aContextState.save(mpLinkButtonStyle);
    style_context_set_state(mpLinkButtonStyle, GTK_STATE_FLAG_LINK);
    style_context_get_color(mpLinkButtonStyle, &text_color);
    aStyleSet.SetLinkColor(getColor(text_color));
    style_context_set_state(mpLinkButtonStyle, GTK_STATE_FLAG_VISITED);
    style_context_get_color(mpLinkButtonStyle, &text_color);
    aStyleSet.SetVisitedLinkColor(getColor(text_color));
    aContextState.restore();

#if !GTK_CHECK_VERSION(4, 0, 0)
    {
        GtkStyleContext *pCStyle = mpNotebookHeaderTabsTabLabelStyle;
        aContextState.save(pCStyle);
        style_context_set_state(pCStyle, GTK_STATE_FLAG_NORMAL);
        style_context_get_color(pCStyle, &text_color);
        aTextColor = getColor( text_color );
        aStyleSet.SetTabTextColor(aTextColor);
        aStyleSet.SetTabFont(getFont(mpNotebookHeaderTabsTabLabelStyle, rSettings.GetUILanguageTag().getLocale()));
        aContextState.restore();
    }

    {
        GtkStyleContext *pCStyle = mpToolButtonStyle;
        aContextState.save(pCStyle);
        style_context_set_state(pCStyle, GTK_STATE_FLAG_NORMAL);
        style_context_get_color(pCStyle, &text_color);
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
        style_context_get_color(pCStyle, &text_color);
        aTextColor = getColor( text_color );
        aStyleSet.SetTabRolloverTextColor(aTextColor);
        aContextState.restore();
    }

    {
        GtkStyleContext *pCStyle = mpNotebookHeaderTabsTabActiveLabelStyle;
        aContextState.save(pCStyle);
        style_context_set_state(pCStyle, GTK_STATE_FLAG_CHECKED);
        style_context_get_color(pCStyle, &text_color);
        aTextColor = getColor( text_color );
        aStyleSet.SetTabHighlightTextColor(aTextColor);
        aContextState.restore();
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

    GtkRequisition natural_horz_scroll_size;
    gtk_widget_get_preferred_size(gHScrollbar, nullptr, &natural_horz_scroll_size);

#if GTK_CHECK_VERSION(4, 0, 0)
    aStyleSet.SetScrollBarSize(natural_horz_scroll_size.height);
#else
    // Grab some button style attributes
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

    // Recent breeze (Mar 2024) has 17 vs 10, while Adwaita still reports 14 vs 14.
    if (natural_horz_scroll_size.height > aSize.Height())
        aSize.setHeight(natural_horz_scroll_size.height);

    aStyleSet.SetScrollBarSize(aSize.Height());

    gtk_style_context_get(mpVScrollbarSliderStyle, gtk_style_context_get_state(mpVScrollbarSliderStyle),
                          "min-height", &min_slider_length,
                          nullptr);
#endif
    aStyleSet.SetMinThumbSize(min_slider_length);

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

    gchar* pThemeName = nullptr;
    g_object_get( pSettings, "gtk-theme-name", &pThemeName, nullptr );
    SAL_INFO("vcl.gtk3", "Theme name is \""
            << pThemeName
            << "\".");
    // High contrast
    aStyleSet.SetHighContrastMode(g_strcmp0(pThemeName, "HighContrast") == 0);
    g_free(pThemeName);

    // finally update the collected settings
    rSettings.SetStyleSettings( aStyleSet );

    return true;
}

#if !GTK_CHECK_VERSION(4, 0, 0)
bool GtkSalGraphics::isNativeControlSupported( ControlType nType, ControlPart nPart )
{
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
#endif

#if ENABLE_CAIRO_CANVAS

bool GtkSalGraphics::SupportsCairo() const
{
    return true;
}

cairo::SurfaceSharedPtr GtkSalGraphics::CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const
{
    return std::make_shared<cairo::Gtk3Surface>(rSurface);
}

cairo::SurfaceSharedPtr GtkSalGraphics::CreateSurface(const OutputDevice& /*rRefDevice*/, int x, int y, int width, int height) const
{
    return std::make_shared<cairo::Gtk3Surface>(this, x, y, width, height);
}

#endif

void GtkSalGraphics::WidgetQueueDraw() const
{
    //request gtk to sync the entire contents
    mpFrame->queue_draw();
}

namespace {

void getStyleContext(GtkStyleContext** style, GtkWidget* widget)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_fixed_put(GTK_FIXED(gDumbContainer), widget, 0, 0);
#else
    gtk_container_add(GTK_CONTAINER(gDumbContainer), widget);
#endif
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
#if GTK_CHECK_VERSION(4, 0, 0)
    pSVData->maNWFData.mbNoFrameJunctionForPopups = true;
#endif

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
#if !GTK_CHECK_VERSION(4, 0, 0)
    if (gCacheWindow)
        gtk_widget_destroy(gCacheWindow);
#endif
}

GtkSalGraphics::GtkSalGraphics( GtkSalFrame *pFrame, GtkWidget *pWindow )
    : mpFrame( pFrame ),
      mpWindow( pWindow )
{
    if (style_loaded)
        return;

    style_loaded = true;

    /* Load the GtkStyleContexts, it might be a bit slow, but usually,
     * gtk apps create a lot of widgets at startup, so, it shouldn't be
     * too slow */

#if GTK_CHECK_VERSION(4, 0, 0)
    gCacheWindow = gtk_window_new();
#else
    gCacheWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#endif
    gDumbContainer = gtk_fixed_new();
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_window_set_child(GTK_WINDOW(gCacheWindow), gDumbContainer);
#else
    gtk_container_add(GTK_CONTAINER(gCacheWindow), gDumbContainer);
#endif
    gtk_widget_realize(gDumbContainer);
    gtk_widget_realize(gCacheWindow);

    gEntryBox = gtk_entry_new();
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_fixed_put(GTK_FIXED(gDumbContainer), gEntryBox, 0, 0);
#else
    gtk_container_add(GTK_CONTAINER(gDumbContainer), gEntryBox);
#endif

#if !GTK_CHECK_VERSION(4, 0, 0)
    mpWindowStyle = createStyleContext(GtkControlPart::ToplevelWindow);
    mpEntryStyle = createStyleContext(GtkControlPart::Entry);
#else
    mpWindowStyle = gtk_widget_get_style_context(gCacheWindow);
    getStyleContext(&mpEntryStyle, gtk_entry_new());
#endif

    getStyleContext(&mpTextViewStyle, gtk_text_view_new());

#if !GTK_CHECK_VERSION(4, 0, 0)
    mpButtonStyle = createStyleContext(GtkControlPart::Button);
    mpLinkButtonStyle = createStyleContext(GtkControlPart::LinkButton);
#else
    getStyleContext(&mpButtonStyle, gtk_button_new());
    getStyleContext(&mpLinkButtonStyle, gtk_link_button_new("https://www.libreoffice.org"));
#endif

#if !GTK_CHECK_VERSION(4, 0, 0)
    GtkWidget* pToolbar = gtk_toolbar_new();
    mpToolbarStyle = gtk_widget_get_style_context(pToolbar);
    gtk_style_context_add_class(mpToolbarStyle, GTK_STYLE_CLASS_TOOLBAR);

    GtkToolItem *item = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), item, -1);
    mpToolbarSeparatorStyle = gtk_widget_get_style_context(GTK_WIDGET(item));
    gtk_style_context_get(mpToolbarSeparatorStyle,
        gtk_style_context_get_state(mpToolbarSeparatorStyle),
        "min-width", &mnVerticalSeparatorMinWidth, nullptr);

    GtkWidget *pButton = gtk_button_new();
    item = gtk_tool_button_new(pButton, nullptr);
    gtk_toolbar_insert(GTK_TOOLBAR(pToolbar), item, -1);
    mpToolButtonStyle = gtk_widget_get_style_context(GTK_WIDGET(pButton));
#endif

    gHScrollbar = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, nullptr);
    gtk_fixed_put(GTK_FIXED(gDumbContainer), gHScrollbar, 0, 0);
    gtk_widget_show(gHScrollbar);

#if GTK_CHECK_VERSION(4, 0, 0)
    gVScrollbar = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, nullptr);
    gtk_fixed_put(GTK_FIXED(gDumbContainer), gVScrollbar, 0, 0);
    gtk_widget_show(gVScrollbar);
    mpVScrollbarStyle = gtk_widget_get_style_context(gVScrollbar);

    mpHScrollbarStyle = gtk_widget_get_style_context(gHScrollbar);

    gTextView = gtk_text_view_new();
    gtk_fixed_put(GTK_FIXED(gDumbContainer), gTextView, 0, 0);
    gtk_widget_show(gTextView);
#else
    mpVScrollbarStyle = createStyleContext(GtkControlPart::ScrollbarVertical);
    mpVScrollbarContentsStyle = createStyleContext(GtkControlPart::ScrollbarVerticalContents);
    mpVScrollbarTroughStyle = createStyleContext(GtkControlPart::ScrollbarVerticalTrough);
    mpVScrollbarSliderStyle = createStyleContext(GtkControlPart::ScrollbarVerticalSlider);
    mpVScrollbarButtonStyle = createStyleContext(GtkControlPart::ScrollbarVerticalButton);
    mpHScrollbarStyle = createStyleContext(GtkControlPart::ScrollbarHorizontal);
    mpHScrollbarContentsStyle = createStyleContext(GtkControlPart::ScrollbarHorizontalContents);
    mpHScrollbarTroughStyle = createStyleContext(GtkControlPart::ScrollbarHorizontalTrough);
    mpHScrollbarSliderStyle = createStyleContext(GtkControlPart::ScrollbarHorizontalSlider);
    mpHScrollbarButtonStyle = createStyleContext(GtkControlPart::ScrollbarHorizontalButton);
#endif

#if !GTK_CHECK_VERSION(4, 0, 0)
    mpCheckButtonStyle = createStyleContext(GtkControlPart::CheckButton);
    mpCheckButtonCheckStyle = createStyleContext(GtkControlPart::CheckButtonCheck);

    mpRadioButtonStyle = createStyleContext(GtkControlPart::RadioButton);
    mpRadioButtonRadioStyle = createStyleContext(GtkControlPart::RadioButtonRadio);

    /* Spinbutton */
    gSpinBox = gtk_spin_button_new(nullptr, 0, 0);
    gtk_container_add(GTK_CONTAINER(gDumbContainer), gSpinBox);
    mpSpinStyle = createStyleContext(GtkControlPart::SpinButton);
    mpSpinUpStyle = createStyleContext(GtkControlPart::SpinButtonUpButton);
    mpSpinDownStyle = createStyleContext(GtkControlPart::SpinButtonDownButton);

    /* NoteBook */
    mpNotebookStyle = createStyleContext(GtkControlPart::Notebook);
    mpNotebookStackStyle = createStyleContext(GtkControlPart::NotebookStack);
    mpNotebookHeaderStyle = createStyleContext(GtkControlPart::NotebookHeader);
    mpNotebookHeaderTabsStyle = createStyleContext(GtkControlPart::NotebookHeaderTabs);
    mpNotebookHeaderTabsTabStyle = createStyleContext(GtkControlPart::NotebookHeaderTabsTab);
    mpNotebookHeaderTabsTabLabelStyle = createStyleContext(GtkControlPart::NotebookHeaderTabsTabLabel);
    mpNotebookHeaderTabsTabActiveLabelStyle = createStyleContext(GtkControlPart::NotebookHeaderTabsTabActiveLabel);
    mpNotebookHeaderTabsTabHoverLabelStyle = createStyleContext(GtkControlPart::NotebookHeaderTabsTabHoverLabel);

    /* Combobox */
    gComboBox = gtk_combo_box_text_new_with_entry();
    gtk_container_add(GTK_CONTAINER(gDumbContainer), gComboBox);
    mpComboboxStyle = createStyleContext(GtkControlPart::Combobox);
    mpComboboxBoxStyle = createStyleContext(GtkControlPart::ComboboxBox);
    mpComboboxEntryStyle = createStyleContext(GtkControlPart::ComboboxBoxEntry);
    mpComboboxButtonStyle = createStyleContext(GtkControlPart::ComboboxBoxButton);
    mpComboboxButtonBoxStyle = createStyleContext(GtkControlPart::ComboboxBoxButtonBox);
    mpComboboxButtonArrowStyle = createStyleContext(GtkControlPart::ComboboxBoxButtonBoxArrow);

    /* Listbox */
    gListBox = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(gListBox), "sample");
    gtk_container_add(GTK_CONTAINER(gDumbContainer), gListBox);
    mpListboxStyle = createStyleContext(GtkControlPart::Listbox);
    mpListboxBoxStyle = createStyleContext(GtkControlPart::ListboxBox);
    mpListboxButtonStyle = createStyleContext(GtkControlPart::ListboxBoxButton);
    mpListboxButtonBoxStyle = createStyleContext(GtkControlPart::ListboxBoxButtonBox);
    mpListboxButtonArrowStyle = createStyleContext(GtkControlPart::ListboxBoxButtonBoxArrow);

    mpMenuBarStyle = createStyleContext(GtkControlPart::MenuBar);
    mpMenuBarItemStyle = createStyleContext(GtkControlPart::MenuBarItem);

    /* Menu */
    mpMenuWindowStyle = createStyleContext(GtkControlPart::MenuWindow);
    mpMenuStyle = createStyleContext(GtkControlPart::Menu);

    mpMenuItemStyle = createStyleContext(GtkControlPart::MenuItem);
    mpMenuItemLabelStyle = createStyleContext(GtkControlPart::MenuItemLabel);
    mpMenuItemArrowStyle = createStyleContext(GtkControlPart::MenuItemArrow);
    mpCheckMenuItemStyle = createStyleContext(GtkControlPart::CheckMenuItem);
    mpCheckMenuItemCheckStyle = createStyleContext(GtkControlPart::CheckMenuItemCheck);
    mpRadioMenuItemStyle = createStyleContext(GtkControlPart::RadioMenuItem);
    mpRadioMenuItemRadioStyle = createStyleContext(GtkControlPart::RadioMenuItemRadio);
    mpSeparatorMenuItemStyle = createStyleContext(GtkControlPart::SeparatorMenuItem);
    mpSeparatorMenuItemSeparatorStyle = createStyleContext(GtkControlPart::SeparatorMenuItemSeparator);

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
    mpProgressBarTroughStyle = createStyleContext(GtkControlPart::ProgressBarTrough);
    mpProgressBarProgressStyle = createStyleContext(GtkControlPart::ProgressBarProgress);

    gtk_widget_show_all(gDumbContainer);
#endif
}

void GtkSalGraphics::GetResolution(sal_Int32& rDPIX, sal_Int32& rDPIY)
{
    char* pForceDpi;
    if ((pForceDpi = getenv("SAL_FORCEDPI")))
    {
        rDPIX = rDPIY = o3tl::toInt32(std::string_view(pForceDpi));
        return;
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    GdkScreen* pScreen = gtk_widget_get_screen(mpWindow);
    double fResolution = -1.0;
    g_object_get(pScreen, "resolution", &fResolution, nullptr);

    if (fResolution > 0.0)
        rDPIX = rDPIY = sal_Int32(fResolution);
    else
        rDPIX = rDPIY = 96;
#else
    rDPIX = rDPIY = 96;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
