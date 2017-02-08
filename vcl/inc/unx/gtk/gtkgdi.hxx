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
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>

#include <unx/gtk/gtkframe.hxx>
#include "ControlCacheKey.hxx"

#if GTK_CHECK_VERSION(3,0,0)

#include <headless/svpgdi.hxx>
#include "textrender.hxx"

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

typedef void (*gtk_widget_path_iter_set_object_nameFunc)(GtkWidgetPath *, guint, const char*);

class GtkSalGraphics : public SvpSalGraphics
{
    GtkSalFrame *mpFrame;
public:
    GtkSalGraphics( GtkSalFrame *pFrame, GtkWidget *pWindow );
    virtual bool        drawNativeControl( ControlType nType, ControlPart nPart,
                                               const Rectangle& rControlRegion,
                                               ControlState nState, const ImplControlValue& aValue,
                                               const OUString& rCaption ) override;
    virtual bool        IsNativeControlSupported( ControlType nType, ControlPart nPart ) override;
    virtual bool        getNativeControlRegion( ControlType nType, ControlPart nPart,
                                                    const Rectangle& rControlRegion,
                                                    ControlState nState,
                                                    const ImplControlValue& aValue,
                                                    const OUString& rCaption,
                                                    Rectangle &rNativeBoundingRegion,
                                                    Rectangle &rNativeContentRegion ) override;
#if ENABLE_CAIRO_CANVAS

    virtual bool        SupportsCairo() const override;
    virtual cairo::SurfaceSharedPtr CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const override;
    virtual cairo::SurfaceSharedPtr CreateSurface(const OutputDevice& rRefDevice, int x, int y, int width, int height) const override;

#endif

    void WidgetQueueDraw() const;

    void updateSettings( AllSettings& rSettings );
    static void refreshFontconfig( GtkSettings *pSettings );
    static void signalSettingsNotify( GObject*, GParamSpec *pSpec, gpointer );

    virtual void GetResolution(sal_Int32& rDPIX, sal_Int32& rDPIY) override;

    GtkStyleContext* createStyleContext(gtk_widget_path_iter_set_object_nameFunc set_object_name, GtkControlPart ePart);
    GtkStyleContext* createNewContext(GtkControlPart ePart, gtk_widget_path_iter_set_object_nameFunc set_object_name);
    GtkStyleContext* createOldContext(GtkControlPart ePart);
    GtkStyleContext* makeContext(GtkWidgetPath *pPath, GtkStyleContext *pParent);
private:
    GtkWidget       *mpWindow;
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
    static GtkStyleContext *mpToolbarSeperatorStyle;
    static GtkStyleContext *mpCheckButtonStyle;
    static GtkStyleContext *mpCheckButtonCheckStyle;
    static GtkStyleContext *mpRadioButtonStyle;
    static GtkStyleContext *mpRadioButtonRadioStyle;
    static GtkStyleContext *mpSpinStyle;
    static GtkStyleContext *mpSpinEntryStyle;
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

    static Rectangle NWGetScrollButtonRect( ControlPart nPart, Rectangle aAreaRect );
    static Rectangle NWGetSpinButtonRect( ControlPart nPart, Rectangle aAreaRect);
    static Rectangle NWGetComboBoxButtonRect( ControlType nType, ControlPart nPart, Rectangle aAreaRect );

    static void PaintScrollbar(GtkStyleContext *context,
                        cairo_t *cr,
                        const Rectangle& rControlRectangle,
                        ControlType nType,
                        ControlPart nPart,
                        const ImplControlValue& aValue );
           void PaintOneSpinButton( GtkStyleContext *context,
                             cairo_t *cr,
                             ControlType nType,
                             ControlPart nPart,
                             Rectangle aAreaRect,
                             ControlState nState );
           void PaintSpinButton(GtkStateFlags flags,
                         cairo_t *cr,
                         const Rectangle& rControlRectangle,
                         ControlType nType,
                         ControlPart nPart,
                         const ImplControlValue& aValue);
    static void PaintCombobox(GtkStateFlags flags,
                        cairo_t *cr,
                        const Rectangle& rControlRectangle,
                        ControlType nType,
                        ControlPart nPart,
                        const ImplControlValue& aValue);
    static void PaintCheckOrRadio(cairo_t *cr, GtkStyleContext *context,
                                  const Rectangle& rControlRectangle,
                                  bool bIsCheck, bool bInMenu);

    static void PaintCheck(cairo_t *cr, GtkStyleContext *context,
                           const Rectangle& rControlRectangle, bool bInMenu);

    static void PaintRadio(cairo_t *cr, GtkStyleContext *context,
                           const Rectangle& rControlRectangle, bool bInMenu);


    static bool style_loaded;
};

#else
#include <unx/salgdi.h>

class GdkX11Pixmap;
class GtkSalGraphics : public X11SalGraphics
{
    GtkWidget           *m_pWindow;
    vcl::Region               m_aClipRegion;

public:
    GtkSalGraphics(GtkSalFrame *, GtkWidget *window, SalX11Screen nXScreen);
    virtual ~GtkSalGraphics() override;

    inline GtkWidget*  GetGtkWidget() const { return m_pWindow; }
    inline GdkWindow*  GetGdkWindow() const { return m_pWindow->window; }
    inline GtkSalFrame* GetGtkFrame() const { return static_cast<GtkSalFrame*>(m_pFrame); }
    void SetWindow( GtkWidget* window ) { m_pWindow = window; }

    // will be set when UI theme was changed
    static  bool        bThemeChanged;
    static  bool        bNeedPixmapPaint;
    static  bool        bNeedTwoPasses;

    // native widget methods
    virtual bool        IsNativeControlSupported( ControlType nType, ControlPart nPart ) override;
    virtual bool        hitTestNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                              const Point& aPos, bool& rIsInside ) override;
    virtual bool        drawNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                           ControlState nState, const ImplControlValue& aValue,
                                           const OUString& rCaption ) override;
    virtual bool        getNativeControlRegion( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion, ControlState nState,
                                                const ImplControlValue& aValue, const OUString& rCaption,
                                                Rectangle &rNativeBoundingRegion, Rectangle &rNativeContentRegion ) override;

    //helper methods for frame's UpdateSettings
    void updateSettings( AllSettings& rSettings );
    static void refreshFontconfig( GtkSettings *pSettings );
    static void signalSettingsNotify( GObject*, GParamSpec *pSpec, gpointer );

    virtual bool            setClipRegion( const vcl::Region& ) override;
    virtual void            ResetClipRegion() override;

    // some themes set the background pixmap of our window EVERY time
    // a control is painted; but presentation effects need
    // the background set to None; workaround: set the background
    // before copyBits
    virtual void            copyBits( const SalTwoRect& rPosAry,
                                      SalGraphics* pSrcGraphics ) override;

protected:

    GdkX11Pixmap* NWGetPixmapFromScreen( Rectangle srcRect, int nBgColor = 0 );
    bool NWRenderPixmapToScreen( GdkX11Pixmap* pPixmap, GdkX11Pixmap* pMask, Rectangle dstRect );

    bool DoDrawNativeControl( GdkDrawable* pDrawable,
                           ControlType nType,
                           ControlPart nPart,
                           const Rectangle& aCtrlRect,
                           const std::list< Rectangle >& aClip,
                           ControlState nState,
                           const ImplControlValue& aValue,
                           const OUString& rCaption,
                           ControlCacheKey& rControlCacheKey);

    bool NWPaintGTKArrow( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const std::list< Rectangle >& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    bool NWPaintGTKListHeader( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const std::list< Rectangle >& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    bool NWPaintGTKFixedLine( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const std::list< Rectangle >& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    bool NWPaintGTKFrame( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const std::list< Rectangle >& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    bool NWPaintGTKWindowBackground( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const std::list< Rectangle >& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    bool NWPaintGTKButtonReal( GtkWidget* button, GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const std::list< Rectangle >& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    bool NWPaintGTKButton( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const std::list< Rectangle >& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    bool NWPaintGTKRadio( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                          const Rectangle& rControlRectangle,
                          const std::list< Rectangle >& rClipList,
                          ControlState nState, const ImplControlValue& aValue,
                          const OUString& rCaption );
    bool NWPaintGTKCheck( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                          const Rectangle& rControlRectangle,
                          const std::list< Rectangle >& rClipList,
                          ControlState nState, const ImplControlValue& aValue,
                          const OUString& rCaption );
    bool NWPaintGTKScrollbar( ControlType nType, ControlPart nPart,
                              const Rectangle& rControlRectangle,
                              const std::list< Rectangle >& rClipList,
                              ControlState nState, const ImplControlValue& aValue,
                              const OUString& rCaption );
    bool NWPaintGTKEditBox( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const std::list< Rectangle >& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    bool NWPaintGTKSpinBox(ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const std::list< Rectangle >& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption,
                           ControlCacheKey& rControlCacheKey);
    bool NWPaintGTKComboBox( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                             const Rectangle& rControlRectangle,
                             const std::list< Rectangle >& rClipList,
                             ControlState nState, const ImplControlValue& aValue,
                             const OUString& rCaption );
    bool NWPaintGTKTabItem( ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const std::list< Rectangle >& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    bool NWPaintGTKListBox( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const std::list< Rectangle >& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );

    bool NWPaintGTKToolbar( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const std::list< Rectangle >& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    bool NWPaintGTKMenubar( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const std::list< Rectangle >& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    bool NWPaintGTKPopupMenu( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                              const Rectangle& rControlRectangle,
                              const std::list< Rectangle >& rClipList,
                              ControlState nState, const ImplControlValue& aValue,
                              const OUString& rCaption );
    bool NWPaintGTKTooltip( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const std::list< Rectangle >& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    bool NWPaintGTKProgress( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const std::list< Rectangle >& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    bool NWPaintGTKSlider( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const std::list< Rectangle >& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    bool NWPaintGTKListNode( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const std::list< Rectangle >& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
};

#endif // !gtk3

#endif // INCLUDED_VCL_INC_UNX_GTK_GTKGDI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
