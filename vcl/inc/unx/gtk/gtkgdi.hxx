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

#ifndef _VCL_GTKGDI_HXX
#define _VCL_GTKGDI_HXX

#include <tools/prex.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <tools/postx.h>

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
    virtual sal_Bool        drawNativeControl( ControlType nType, ControlPart nPart,
                                               const Rectangle& rControlRegion,
                                               ControlState nState, const ImplControlValue& aValue,
                                               const OUString& rCaption );
    virtual sal_Bool        IsNativeControlSupported( ControlType nType, ControlPart nPart );
    virtual sal_Bool        getNativeControlRegion( ControlType nType, ControlPart nPart,
                                                    const Rectangle& rControlRegion,
                                                    ControlState nState,
                                                    const ImplControlValue& aValue,
                                                    const OUString& rCaption,
                                                    Rectangle &rNativeBoundingRegion,
                                                    Rectangle &rNativeContentRegion );
    void updateSettings( AllSettings& rSettings );
private:
    GtkWidget       *mpWindow;
    static GtkStyleContext *mpButtonStyle;
    static GtkStyleContext *mpEntryStyle;
    static GtkStyleContext *mpScrollbarStyle;
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
    static  sal_Bool        bThemeChanged;
    static  sal_Bool        bNeedPixmapPaint;

    // native widget methods
    virtual sal_Bool        IsNativeControlSupported( ControlType nType, ControlPart nPart );
    virtual sal_Bool        hitTestNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                              const Point& aPos, sal_Bool& rIsInside );
    virtual sal_Bool        drawNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                           ControlState nState, const ImplControlValue& aValue,
                                           const OUString& rCaption );
    virtual sal_Bool        getNativeControlRegion( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion, ControlState nState,
                                                const ImplControlValue& aValue, const OUString& rCaption,
                                                Rectangle &rNativeBoundingRegion, Rectangle &rNativeContentRegion );

    //helper methods for frame's UpdateSettings
    void updateSettings( AllSettings& rSettings );

    virtual bool            setClipRegion( const Region& );
    virtual void            ResetClipRegion();

    // some themes set the background pixmap of our window EVERY time
    // a control is painted; but presentation effects need
    // the background set to None; workaround: set the background
    // before copyBits
    virtual void            copyBits( const SalTwoRect* pPosAry,
                                      SalGraphics* pSrcGraphics );

protected:
    typedef std::list< Rectangle > clipList;

    GdkPixmap* NWGetPixmapFromScreen( Rectangle srcRect );
    sal_Bool NWRenderPixmapToScreen( GdkPixmap* pPixmap, Rectangle dstRect );

    sal_Bool NWPaintGTKArrow( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const clipList& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    sal_Bool NWPaintGTKListHeader( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const clipList& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    sal_Bool NWPaintGTKFixedLine( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const clipList& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    sal_Bool NWPaintGTKFrame( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const clipList& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    sal_Bool NWPaintGTKWindowBackground( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const clipList& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    sal_Bool NWPaintGTKButtonReal( GtkWidget* button, GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const clipList& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    sal_Bool NWPaintGTKButton( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const clipList& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    sal_Bool NWPaintGTKRadio( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                          const Rectangle& rControlRectangle,
                          const clipList& rClipList,
                          ControlState nState, const ImplControlValue& aValue,
                          const OUString& rCaption );
    sal_Bool NWPaintGTKCheck( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                          const Rectangle& rControlRectangle,
                          const clipList& rClipList,
                          ControlState nState, const ImplControlValue& aValue,
                          const OUString& rCaption );
    sal_Bool NWPaintGTKScrollbar( ControlType nType, ControlPart nPart,
                              const Rectangle& rControlRectangle,
                              const clipList& rClipList,
                              ControlState nState, const ImplControlValue& aValue,
                              const OUString& rCaption );
    sal_Bool NWPaintGTKEditBox( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    sal_Bool NWPaintGTKSpinBox( ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    sal_Bool NWPaintGTKComboBox( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                             const Rectangle& rControlRectangle,
                             const clipList& rClipList,
                             ControlState nState, const ImplControlValue& aValue,
                             const OUString& rCaption );
    sal_Bool NWPaintGTKTabItem( ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    sal_Bool NWPaintGTKListBox( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );

    sal_Bool NWPaintGTKToolbar( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    sal_Bool NWPaintGTKMenubar( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    sal_Bool NWPaintGTKPopupMenu( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                              const Rectangle& rControlRectangle,
                              const clipList& rClipList,
                              ControlState nState, const ImplControlValue& aValue,
                              const OUString& rCaption );
    sal_Bool NWPaintGTKTooltip( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    sal_Bool NWPaintGTKProgress( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    sal_Bool NWPaintGTKSlider( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const clipList& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    sal_Bool NWPaintGTKListNode( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
};

#endif // !gtk3

#endif // _VCL_GTKGDI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
