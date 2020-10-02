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

#ifndef INCLUDED_VCL_INC_UNX_GTK_GTKGDI_HXX
#define INCLUDED_VCL_INC_UNX_GTK_GTKGDI_HXX

#include <config_cairo_canvas.h>

#include <gtk/gtk.h>
#include "gtkbackend.hxx"
#include <gdk/gdkkeysyms.h>

#include <unx/gtk/gtkframe.hxx>
#include <ControlCacheKey.hxx>

#include <headless/svpgdi.hxx>
#include <textrender.hxx>

enum class GtkControlPart
{
    ToplevelWindow,
    Button,
    LinkButton,
    CheckButton,
    CheckButtonCheck,
    RadioButton,
    RadioButtonRadio,
    Entry,
    Combobox,
    ComboboxBox,
    ComboboxBoxEntry,
    ComboboxBoxButton,
    ComboboxBoxButtonBox,
    ComboboxBoxButtonBoxArrow,
    Listbox,
    ListboxBox,
    ListboxBoxButton,
    ListboxBoxButtonBox,
    ListboxBoxButtonBoxArrow,
    SpinButton,
    SpinButtonEntry,
    SpinButtonUpButton,
    SpinButtonDownButton,
    ScrollbarVertical,
    ScrollbarVerticalContents,
    ScrollbarVerticalTrough,
    ScrollbarVerticalSlider,
    ScrollbarVerticalButton,
    ScrollbarHorizontal,
    ScrollbarHorizontalContents,
    ScrollbarHorizontalTrough,
    ScrollbarHorizontalSlider,
    ScrollbarHorizontalButton,
    ProgressBar,
    ProgressBarTrough,
    ProgressBarProgress,
    Notebook,
    NotebookHeader,
    NotebookStack,
    NotebookHeaderTabs,
    NotebookHeaderTabsTab,
    NotebookHeaderTabsTabLabel,
    NotebookHeaderTabsTabActiveLabel,
    NotebookHeaderTabsTabHoverLabel,
    FrameBorder,
    MenuBar,
    MenuBarItem,
    MenuWindow,
    Menu,
    MenuItem,
    MenuItemLabel,
    MenuItemArrow,
    CheckMenuItem,
    CheckMenuItemCheck,
    RadioMenuItem,
    RadioMenuItemRadio,
    SeparatorMenuItem,
    SeparatorMenuItemSeparator,
};

class GtkSalGraphics : public SvpSalGraphics
{
    GtkSalFrame * const mpFrame;

protected:
    bool isNativeControlSupported(ControlType, ControlPart) override;
    virtual bool        drawNativeControl( ControlType nType, ControlPart nPart,
                                               const tools::Rectangle& rControlRegion,
                                               ControlState nState, const ImplControlValue& aValue,
                                               const OUString& rCaption,
                                               const Color& rBackgroundColor ) override;
    virtual bool        getNativeControlRegion( ControlType nType, ControlPart nPart,
                                                    const tools::Rectangle& rControlRegion,
                                                    ControlState nState,
                                                    const ImplControlValue& aValue,
                                                    const OUString& rCaption,
                                                    tools::Rectangle &rNativeBoundingRegion,
                                                    tools::Rectangle &rNativeContentRegion ) override;
    bool updateSettings(AllSettings&) override;
    void handleDamage(const tools::Rectangle&) override;

public:
    GtkSalGraphics( GtkSalFrame *pFrame, GtkWidget *pWindow );

#if ENABLE_CAIRO_CANVAS

    virtual bool        SupportsCairo() const override;
    virtual cairo::SurfaceSharedPtr CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const override;
    virtual cairo::SurfaceSharedPtr CreateSurface(const OutputDevice& rRefDevice, int x, int y, int width, int height) const override;

#endif

    void WidgetQueueDraw() const;

    virtual void GetResolution(sal_Int32& rDPIX, sal_Int32& rDPIY) override;

    virtual OUString getRenderBackendName() const override { return "gtk3svp"; }

    GtkStyleContext* createStyleContext(GtkControlPart ePart);
    GtkStyleContext* makeContext(GtkWidgetPath *pPath, GtkStyleContext *pParent);
private:
    GtkWidget              *mpWindow;
    static GtkStyleContext *mpWindowStyle;
    static GtkStyleContext *mpButtonStyle;
    static GtkStyleContext *mpLinkButtonStyle;
    static GtkStyleContext *mpEntryStyle;
    static GtkStyleContext *mpTextViewStyle;
    static GtkStyleContext *mpVScrollbarStyle;
    static GtkStyleContext *mpVScrollbarContentsStyle;
    static GtkStyleContext *mpVScrollbarTroughStyle;
    static GtkStyleContext *mpVScrollbarSliderStyle;
    static GtkStyleContext *mpVScrollbarButtonStyle;
    static GtkStyleContext *mpHScrollbarStyle;
    static GtkStyleContext *mpHScrollbarContentsStyle;
    static GtkStyleContext *mpHScrollbarTroughStyle;
    static GtkStyleContext *mpHScrollbarSliderStyle;
    static GtkStyleContext *mpHScrollbarButtonStyle;
    static GtkStyleContext *mpToolbarStyle;
    static GtkStyleContext *mpToolButtonStyle;
    static GtkStyleContext *mpToolbarSeparatorStyle;
    static GtkStyleContext *mpCheckButtonStyle;
    static GtkStyleContext *mpCheckButtonCheckStyle;
    static GtkStyleContext *mpRadioButtonStyle;
    static GtkStyleContext *mpRadioButtonRadioStyle;
    static GtkStyleContext *mpSpinStyle;
    static GtkStyleContext *mpSpinUpStyle;
    static GtkStyleContext *mpSpinDownStyle;
    static GtkStyleContext *mpComboboxStyle;
    static GtkStyleContext *mpComboboxBoxStyle;
    static GtkStyleContext *mpComboboxEntryStyle;
    static GtkStyleContext *mpComboboxButtonStyle;
    static GtkStyleContext *mpComboboxButtonBoxStyle;
    static GtkStyleContext *mpComboboxButtonArrowStyle;
    static GtkStyleContext *mpListboxStyle;
    static GtkStyleContext *mpListboxBoxStyle;
    static GtkStyleContext *mpListboxButtonStyle;
    static GtkStyleContext *mpListboxButtonBoxStyle;
    static GtkStyleContext *mpListboxButtonArrowStyle;
    static GtkStyleContext *mpFrameInStyle;
    static GtkStyleContext *mpFrameOutStyle;
    static GtkStyleContext *mpFixedHoriLineStyle;
    static GtkStyleContext *mpFixedVertLineStyle;
    static GtkStyleContext *mpTreeHeaderButtonStyle;
    static GtkStyleContext *mpProgressBarStyle;
    static GtkStyleContext *mpProgressBarTroughStyle;
    static GtkStyleContext *mpProgressBarProgressStyle;
    static GtkStyleContext *mpNotebookStyle;
    static GtkStyleContext *mpNotebookStackStyle;
    static GtkStyleContext *mpNotebookHeaderStyle;
    static GtkStyleContext *mpNotebookHeaderTabsStyle;
    static GtkStyleContext *mpNotebookHeaderTabsTabStyle;
    static GtkStyleContext *mpNotebookHeaderTabsTabLabelStyle;
    static GtkStyleContext *mpNotebookHeaderTabsTabActiveLabelStyle;
    static GtkStyleContext *mpNotebookHeaderTabsTabHoverLabelStyle;
    static GtkStyleContext *mpMenuBarStyle;
    static GtkStyleContext *mpMenuBarItemStyle;
    static GtkStyleContext *mpMenuWindowStyle;
    static GtkStyleContext *mpMenuStyle;
    static GtkStyleContext *mpMenuItemStyle;
    static GtkStyleContext *mpMenuItemLabelStyle;
    static GtkStyleContext *mpMenuItemArrowStyle;
    static GtkStyleContext *mpCheckMenuItemStyle;
    static GtkStyleContext *mpCheckMenuItemCheckStyle;
    static GtkStyleContext *mpRadioMenuItemStyle;
    static GtkStyleContext *mpRadioMenuItemRadioStyle;
    static GtkStyleContext *mpSeparatorMenuItemStyle;
    static GtkStyleContext *mpSeparatorMenuItemSeparatorStyle;

    static tools::Rectangle NWGetScrollButtonRect( ControlPart nPart, tools::Rectangle aAreaRect );
    static tools::Rectangle NWGetSpinButtonRect( ControlPart nPart, tools::Rectangle aAreaRect);
    static tools::Rectangle NWGetComboBoxButtonRect(ControlType nType, ControlPart nPart, tools::Rectangle aAreaRect);

    static void PaintScrollbar(GtkStyleContext *context,
                        cairo_t *cr,
                        const tools::Rectangle& rControlRectangle,
                        ControlPart nPart,
                        const ImplControlValue& aValue );
           void PaintOneSpinButton( GtkStyleContext *context,
                             cairo_t *cr,
                             ControlPart nPart,
                             tools::Rectangle aAreaRect,
                             ControlState nState );
           void PaintSpinButton(GtkStateFlags flags,
                         cairo_t *cr,
                         const tools::Rectangle& rControlRectangle,
                         ControlPart nPart,
                         const ImplControlValue& aValue);
    static void PaintCombobox(GtkStateFlags flags,
                        cairo_t *cr,
                        const tools::Rectangle& rControlRectangle,
                        ControlType nType,
                        ControlPart nPart);
    static void PaintCheckOrRadio(cairo_t *cr, GtkStyleContext *context,
                                  const tools::Rectangle& rControlRectangle,
                                  bool bIsCheck, bool bInMenu);

    static void PaintCheck(cairo_t *cr, GtkStyleContext *context,
                           const tools::Rectangle& rControlRectangle, bool bInMenu);

    static void PaintRadio(cairo_t *cr, GtkStyleContext *context,
                           const tools::Rectangle& rControlRectangle, bool bInMenu);


    static bool style_loaded;
};

#endif // INCLUDED_VCL_INC_UNX_GTK_GTKGDI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
