/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
                                               const rtl::OUString& rCaption );
    virtual sal_Bool        IsNativeControlSupported( ControlType nType, ControlPart nPart );
    virtual sal_Bool        getNativeControlRegion( ControlType nType, ControlPart nPart,
                                                    const Rectangle& rControlRegion,
                                                    ControlState nState,
                                                    const ImplControlValue& aValue,
                                                    const rtl::OUString& rCaption,
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
                                           const rtl::OUString& rCaption );
    virtual sal_Bool        getNativeControlRegion( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion, ControlState nState,
                                                const ImplControlValue& aValue, const rtl::OUString& rCaption,
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
