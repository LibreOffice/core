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

#include <prex.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <postx.h>

#include <unx/salgdi.h>

#if GTK_CHECK_VERSION(3,0,0)

#include <headless/svpgdi.hxx>

class GtkSalFrame;
class GtkSalGraphics : public SvpSalGraphics
{
    GtkSalFrame *mpFrame;
public:
    GtkSalGraphics( GtkSalFrame *pFrame, GtkWidget *pWindow );
    virtual void copyArea( long nDestX, long nDestY,
                           long nSrcX, long nSrcY,
                           long nSrcWidth, long nSrcHeight,
                           sal_uInt16 /*nFlags*/ );
    virtual bool        drawNativeControl( ControlType nType, ControlPart nPart,
                                               const Rectangle& rControlRegion,
                                               ControlState nState, const ImplControlValue& aValue,
                                               const OUString& rCaption );
    virtual bool        IsNativeControlSupported( ControlType nType, ControlPart nPart );
    virtual bool        getNativeControlRegion( ControlType nType, ControlPart nPart,
                                                    const Rectangle& rControlRegion,
                                                    ControlState nState,
                                                    const ImplControlValue& aValue,
                                                    const OUString& rCaption,
                                                    Rectangle &rNativeBoundingRegion,
                                                    Rectangle &rNativeContentRegion );
    void updateSettings( AllSettings& rSettings );
    static void refreshFontconfig( GtkSettings *pSettings );
    static void signalSettingsNotify( GObject*, GParamSpec *pSpec, gpointer );
private:
    GtkWidget       *mpWindow;
    static GtkStyleContext *mpButtonStyle;
    static GtkStyleContext *mpEntryStyle;
    static GtkStyleContext *mpVScrollbarStyle;
    static GtkStyleContext *mpHScrollbarStyle;
    static GtkStyleContext *mpToolbarStyle;
    static GtkStyleContext *mpToolButtonStyle;
    static GtkStyleContext *mpCheckButtonStyle;
    static GtkStyleContext *mpMenuBarStyle;
    static GtkStyleContext *mpMenuBarItemStyle;
    static GtkStyleContext *mpMenuStyle;
    static GtkStyleContext *mpMenuItemStyle;
    static GtkStyleContext *mpSpinStyle;
    static GtkStyleContext *mpComboboxStyle;
    static GtkStyleContext *mpListboxStyle;

    void renderAreaToPix( cairo_t* cr, cairo_rectangle_int_t *region );
    void getStyleContext( GtkStyleContext** style, GtkWidget* widget );
    Rectangle NWGetScrollButtonRect( ControlPart nPart, Rectangle aAreaRect );
    Rectangle NWGetSpinButtonRect( ControlPart nPart, Rectangle aAreaRect);
    Rectangle NWGetComboBoxButtonRect( ControlType nType, ControlPart nPart, Rectangle aAreaRect );

    void PaintScrollbar(GtkStyleContext *context,
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
    void PaintSpinButton(GtkStyleContext *context,
                         cairo_t *cr,
                         const Rectangle& rControlRectangle,
                         ControlType nType,
                         ControlPart nPart,
                         const ImplControlValue& aValue );
    void PaintCombobox( GtkStyleContext *context,
                        cairo_t *cr,
                        const Rectangle& rControlRectangle,
                        ControlType nType,
                        ControlPart nPart,
                        const ImplControlValue& aValue );
    void PaintCheckOrRadio(GtkStyleContext *context,
                           cairo_t *cr,
                           const Rectangle& rControlRectangle,
                           ControlType nType);

    static bool style_loaded;
};

#else

class GtkSalGraphics : public X11SalGraphics
{
    GtkWidget           *m_pWindow;
    Region               m_aClipRegion;

public:
                        GtkSalGraphics( GtkSalFrame *, GtkWidget *window );
    virtual             ~GtkSalGraphics();

    inline GtkWidget*  GetGtkWidget() const { return m_pWindow; }
    inline GdkWindow*  GetGdkWindow() const { return m_pWindow->window; }
    inline GtkSalFrame* GetGtkFrame() const { return static_cast<GtkSalFrame*>(m_pFrame); }
    void SetWindow( GtkWidget* window ) { m_pWindow = window; }


    // will be set when UI theme was changed
    static  bool        bThemeChanged;
    static  bool        bNeedPixmapPaint;

    // native widget methods
    virtual bool        IsNativeControlSupported( ControlType nType, ControlPart nPart ) SAL_OVERRIDE;
    virtual bool        hitTestNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                              const Point& aPos, bool& rIsInside ) SAL_OVERRIDE;
    virtual bool        drawNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                           ControlState nState, const ImplControlValue& aValue,
                                           const OUString& rCaption ) SAL_OVERRIDE;
    virtual bool        getNativeControlRegion( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion, ControlState nState,
                                                const ImplControlValue& aValue, const OUString& rCaption,
                                                Rectangle &rNativeBoundingRegion, Rectangle &rNativeContentRegion ) SAL_OVERRIDE;

    //helper methods for frame's UpdateSettings
    void updateSettings( AllSettings& rSettings );
    static void refreshFontconfig( GtkSettings *pSettings );
    static void signalSettingsNotify( GObject*, GParamSpec *pSpec, gpointer );

    virtual bool            setClipRegion( const Region& ) SAL_OVERRIDE;
    virtual void            ResetClipRegion() SAL_OVERRIDE;

    // some themes set the background pixmap of our window EVERY time
    // a control is painted; but presentation effects need
    // the background set to None; workaround: set the background
    // before copyBits
    virtual void            copyBits( const SalTwoRect& rPosAry,
                                      SalGraphics* pSrcGraphics ) SAL_OVERRIDE;

protected:
    typedef std::list< Rectangle > clipList;

    GdkPixmap* NWGetPixmapFromScreen( Rectangle srcRect );
    bool NWRenderPixmapToScreen( GdkPixmap* pPixmap, Rectangle dstRect );

    bool NWPaintGTKArrow( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const clipList& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    bool NWPaintGTKListHeader( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const clipList& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    bool NWPaintGTKFixedLine( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const clipList& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    bool NWPaintGTKFrame( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const clipList& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    bool NWPaintGTKWindowBackground( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const clipList& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    bool NWPaintGTKButtonReal( GtkWidget* button, GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const clipList& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    bool NWPaintGTKButton( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const clipList& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    bool NWPaintGTKRadio( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                          const Rectangle& rControlRectangle,
                          const clipList& rClipList,
                          ControlState nState, const ImplControlValue& aValue,
                          const OUString& rCaption );
    bool NWPaintGTKCheck( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                          const Rectangle& rControlRectangle,
                          const clipList& rClipList,
                          ControlState nState, const ImplControlValue& aValue,
                          const OUString& rCaption );
    bool NWPaintGTKScrollbar( ControlType nType, ControlPart nPart,
                              const Rectangle& rControlRectangle,
                              const clipList& rClipList,
                              ControlState nState, const ImplControlValue& aValue,
                              const OUString& rCaption );
    bool NWPaintGTKEditBox( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    bool NWPaintGTKSpinBox( ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    bool NWPaintGTKComboBox( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                             const Rectangle& rControlRectangle,
                             const clipList& rClipList,
                             ControlState nState, const ImplControlValue& aValue,
                             const OUString& rCaption );
    bool NWPaintGTKTabItem( ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    bool NWPaintGTKListBox( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );

    bool NWPaintGTKToolbar( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    bool NWPaintGTKMenubar( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    bool NWPaintGTKPopupMenu( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                              const Rectangle& rControlRectangle,
                              const clipList& rClipList,
                              ControlState nState, const ImplControlValue& aValue,
                              const OUString& rCaption );
    bool NWPaintGTKTooltip( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    bool NWPaintGTKProgress( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    bool NWPaintGTKSlider( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const clipList& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    bool NWPaintGTKListNode( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
};

#endif // !gtk3

#endif // INCLUDED_VCL_INC_UNX_GTK_GTKGDI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
