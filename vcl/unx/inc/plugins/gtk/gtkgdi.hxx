/*************************************************************************
 *
 *  $RCSfile: gtkgdi.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 15:55:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _VCL_GTKGDI_HXX
#define _VCL_GTKGDI_HXX

#include <prex.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <postx.h>


#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif

class GtkSalGraphics : public X11SalGraphics
{
    GtkWidget           *m_pWindow;
public:
                        GtkSalGraphics( GtkWidget *window ) { m_pWindow = window; }
    virtual             ~GtkSalGraphics();

    inline GtkWidget*  GetGtkWidget() const { return m_pWindow; }
    inline GdkWindow*  GetGdkWindow() const { return m_pWindow->window; }
    inline GtkSalFrame* GetGtkFrame() const { return static_cast<GtkSalFrame*>(m_pFrame); }


    // will be set when UI theme was changed
    static  BOOL        bThemeChanged;

    // native widget methods
    virtual BOOL        IsNativeControlSupported( ControlType nType, ControlPart nPart );
    virtual BOOL        hitTestNativeControl( ControlType nType, ControlPart nPart, const Region& rControlRegion,
                                              const Point& aPos, SalControlHandle& rControlHandle, BOOL& rIsInside );
    virtual BOOL        drawNativeControl( ControlType nType, ControlPart nPart, const Region& rControlRegion,
                                           ControlState nState, const ImplControlValue& aValue, SalControlHandle& rControlHandle,
                                           rtl::OUString aCaption );
    virtual BOOL        drawNativeControlText( ControlType nType, ControlPart nPart, const Region& rControlRegion,
                                               ControlState nState, const ImplControlValue& aValue,
                                               SalControlHandle& rControlHandle, rtl::OUString aCaption );
    virtual BOOL        getNativeControlRegion( ControlType nType, ControlPart nPart, const Region& rControlRegion, ControlState nState,
                                                const ImplControlValue& aValue, SalControlHandle& rControlHandle, rtl::OUString aCaption,
                                                Region &rNativeBoundingRegion, Region &rNativeContentRegion );

protected:
    GdkPixmap* NWGetPixmapFromScreen( Rectangle srcRect );
    BOOL NWRenderPixmapToScreen( GdkPixmap* pPixmap, Rectangle dstRect );

    BOOL NWPaintGTKButton( ControlType nType, ControlPart nPart,
                           const Region& rControlRegion, ControlState nState, const ImplControlValue& aValue,
                           SalControlHandle& rControlHandle, OUString aCaption );
    BOOL NWPaintGTKRadio( ControlType nType, ControlPart nPart,
                            const Region& rControlRegion, ControlState nState, const ImplControlValue& aValue,
                            SalControlHandle& rControlHandle, OUString aCaption );
    BOOL NWPaintGTKCheck( ControlType nType, ControlPart nPart,
                            const Region& rControlRegion, ControlState nState, const ImplControlValue& aValue,
                            SalControlHandle& rControlHandle, OUString aCaption );
    BOOL NWPaintGTKScrollbar( ControlType nType, ControlPart nPart,
                            const Region& rControlRegion, ControlState nState, const ImplControlValue& aValue,
                            SalControlHandle& rControlHandle, OUString aCaption );
    BOOL NWPaintGTKEditBox( ControlType nType, ControlPart nPart,
                            const Region& rControlRegion, ControlState nState, const ImplControlValue& aValue,
                            SalControlHandle& rControlHandle, OUString aCaption );
    BOOL NWPaintGTKSpinBox( ControlType nType, ControlPart nPart,
                            const Region& rControlRegion, ControlState nState, const ImplControlValue& aValue,
                            SalControlHandle& rControlHandle, OUString aCaption );
    BOOL NWPaintGTKComboBox( ControlType nType, ControlPart nPart,
                            const Region& rControlRegion, ControlState nState, const ImplControlValue& aValue,
                            SalControlHandle& rControlHandle, OUString aCaption );
    BOOL NWPaintGTKTabItem( ControlType nType, ControlPart nPart,
                            const Region& rControlRegion, ControlState nState, const ImplControlValue& aValue,
                            SalControlHandle& rControlHandle, OUString aCaption );
    BOOL NWPaintGTKListBox( ControlType nType, ControlPart nPart,
                            const Region& rControlRegion, ControlState nState, const ImplControlValue& aValue,
                            SalControlHandle& rControlHandle, OUString aCaption );






};

#endif // _VCL_GTKGDI_HXX
