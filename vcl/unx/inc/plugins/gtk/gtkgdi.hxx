/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: gtkgdi.hxx,v $
 * $Revision: 1.14 $
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

#include <prex.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <postx.h>

#include <salgdi.h>

class GtkSalGraphics : public X11SalGraphics
{
    GtkWidget           *m_pWindow;
    Region               m_aClipRegion;

public:
                        GtkSalGraphics( GtkWidget *window )
                            : m_pWindow( window ),
                              m_aClipRegion( REGION_NULL )
                              {}
    virtual             ~GtkSalGraphics();

    inline GtkWidget*  GetGtkWidget() const { return m_pWindow; }
    inline GdkWindow*  GetGdkWindow() const { return m_pWindow->window; }
    inline GtkSalFrame* GetGtkFrame() const { return static_cast<GtkSalFrame*>(m_pFrame); }
    void SetWindow( GtkWidget* window ) { m_pWindow = window; }


    // will be set when UI theme was changed
    static  BOOL        bThemeChanged;
    static  BOOL        bNeedPixmapPaint;
    static  BOOL        bGlobalNeedPixmapPaint;
    static  BOOL        bToolbarGripWorkaround;
    static  BOOL        bNeedButtonStyleAsEditBackgroundWorkaround;

    // native widget methods
    virtual BOOL        IsNativeControlSupported( ControlType nType, ControlPart nPart );
    virtual BOOL        hitTestNativeControl( ControlType nType, ControlPart nPart, const Region& rControlRegion,
                                              const Point& aPos, SalControlHandle& rControlHandle, BOOL& rIsInside );
    virtual BOOL        drawNativeControl( ControlType nType, ControlPart nPart, const Region& rControlRegion,
                                           ControlState nState, const ImplControlValue& aValue, SalControlHandle& rControlHandle,
                                           const rtl::OUString& rCaption );
    virtual BOOL        drawNativeControlText( ControlType nType, ControlPart nPart, const Region& rControlRegion,
                                               ControlState nState, const ImplControlValue& aValue,
                                               SalControlHandle& rControlHandle, const rtl::OUString& rCaption );
    virtual BOOL        getNativeControlRegion( ControlType nType, ControlPart nPart, const Region& rControlRegion, ControlState nState,
                                                const ImplControlValue& aValue, SalControlHandle& rControlHandle, const rtl::OUString& rCaption,
                                                Region &rNativeBoundingRegion, Region &rNativeContentRegion );

    //helper methods for frame's UpdateSettings
    void updateSettings( AllSettings& rSettings );

    virtual void            ResetClipRegion();
    virtual void            BeginSetClipRegion( ULONG nCount );
    virtual BOOL            unionClipRegion( long nX, long nY, long nWidth, long nHeight );
    virtual void            EndSetClipRegion();

    // some themes set the background pixmap of our window EVERY time
    // a control is painted; but presentation effects need
    // the background set to None; workaround: set the background
    // before copyBits
    virtual void            copyBits( const SalTwoRect* pPosAry,
                                      SalGraphics* pSrcGraphics );

protected:
    typedef std::list< Rectangle > clipList;

    GdkPixmap* NWGetPixmapFromScreen( Rectangle srcRect );
    BOOL NWRenderPixmapToScreen( GdkPixmap* pPixmap, Rectangle dstRect );

    BOOL NWPaintGTKButton( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const clipList& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           SalControlHandle& rControlHandle, const OUString& rCaption );
    BOOL NWPaintGTKRadio( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                          const Rectangle& rControlRectangle,
                          const clipList& rClipList,
                          ControlState nState, const ImplControlValue& aValue,
                          SalControlHandle& rControlHandle, const OUString& rCaption );
    BOOL NWPaintGTKCheck( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                          const Rectangle& rControlRectangle,
                          const clipList& rClipList,
                          ControlState nState, const ImplControlValue& aValue,
                          SalControlHandle& rControlHandle, const OUString& rCaption );
    BOOL NWPaintGTKScrollbar( ControlType nType, ControlPart nPart,
                              const Rectangle& rControlRectangle,
                              const clipList& rClipList,
                              ControlState nState, const ImplControlValue& aValue,
                              SalControlHandle& rControlHandle, const OUString& rCaption );
    BOOL NWPaintGTKEditBox( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            SalControlHandle& rControlHandle, const OUString& rCaption );
    BOOL NWPaintGTKSpinBox( ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            SalControlHandle& rControlHandle, const OUString& rCaption );
    BOOL NWPaintGTKComboBox( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                             const Rectangle& rControlRectangle,
                             const clipList& rClipList,
                             ControlState nState, const ImplControlValue& aValue,
                             SalControlHandle& rControlHandle, const OUString& rCaption );
    BOOL NWPaintGTKTabItem( ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            SalControlHandle& rControlHandle, const OUString& rCaption );
    BOOL NWPaintGTKListBox( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            SalControlHandle& rControlHandle, const OUString& rCaption );

    BOOL NWPaintGTKToolbar( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            SalControlHandle& rControlHandle, const OUString& rCaption );
    BOOL NWPaintGTKMenubar( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            SalControlHandle& rControlHandle, const OUString& rCaption );
    BOOL NWPaintGTKPopupMenu( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                              const Rectangle& rControlRectangle,
                              const clipList& rClipList,
                              ControlState nState, const ImplControlValue& aValue,
                              SalControlHandle& rControlHandle, const OUString& rCaption );
    BOOL NWPaintGTKTooltip( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            SalControlHandle& rControlHandle, const OUString& rCaption );
    BOOL NWPaintGTKProgress( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            SalControlHandle& rControlHandle, const OUString& rCaption );

    BOOL drawNativeMixedStateCheck( ControlType nType, ControlPart nPart, const Region& rControlRegion,
                                    ControlState nState, const ImplControlValue& aValue, SalControlHandle& rControlHandle,
                                    const rtl::OUString& rCaption );
};

#endif // _VCL_GTKGDI_HXX
