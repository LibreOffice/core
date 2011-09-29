/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *        Caolán McNamara <caolanm@redhat.com> (Red Hat, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Caolán McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

// Gross inclusion hacks for now ...

#include "../../headless/svpgdi.cxx"
#include <unx/gtk/gtkframe.hxx>
#include <unx/gtk/gtkdata.hxx>
#include <unx/gtk/gtkinst.hxx>
#include <unx/gtk/gtkgdi.hxx>

/************************************************************************
 * State conversion
 ************************************************************************/
static void NWConvertVCLStateToGTKState( ControlState nVCLState,
            GtkStateFlags* nGTKState, GtkShadowType* nGTKShadow )
{
    *nGTKShadow = GTK_SHADOW_OUT;
    *nGTKState = GTK_STATE_FLAG_INSENSITIVE;

    if ( nVCLState & CTRL_STATE_ENABLED )
    {
        if ( nVCLState & CTRL_STATE_PRESSED )
        {
            *nGTKState = GTK_STATE_FLAG_ACTIVE;
            *nGTKShadow = GTK_SHADOW_IN;
        }
        else if ( nVCLState & CTRL_STATE_ROLLOVER )
        {
            *nGTKState = GTK_STATE_FLAG_PRELIGHT;
            *nGTKShadow = GTK_SHADOW_OUT;
        }
        else
        {
            *nGTKState = GTK_STATE_FLAG_NORMAL;
            *nGTKShadow = GTK_SHADOW_OUT;
        }
    }
}

sal_Bool GtkSalGraphics::drawNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                            ControlState nState, const ImplControlValue& aValue,
                                            const rtl::OUString& rCaption )
{
    GtkStateFlags flags;
    GtkShadowType shadow;
    NWConvertVCLStateToGTKState(nState, &flags, &shadow);
    cairo_t* cr = gdk_cairo_create(gtk_widget_get_window(mpWindow));
    switch(nType)
    {
    case CTRL_PUSHBUTTON:
        if(!GTK_IS_STYLE_CONTEXT(mpButtonStyle))
        {
            mpButtonStyle = gtk_widget_get_style_context(gtk_button_new());
        }

        gtk_style_context_set_state(mpButtonStyle, flags);

        gtk_render_background(mpButtonStyle, cr,
                rControlRegion.Left(), rControlRegion.Top(),
                rControlRegion.GetWidth(), rControlRegion.GetHeight());
        gtk_render_frame(mpButtonStyle, cr,
                rControlRegion.Left(), rControlRegion.Top(),
                rControlRegion.GetWidth(), rControlRegion.GetHeight());

        return sal_True;
    default:
        return sal_False;
    }
    cairo_destroy(cr);
}

sal_Bool GtkSalGraphics::getNativeControlRegion( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion, ControlState nState,
                                                const ImplControlValue& aValue, const rtl::OUString& rCaption,
                                                Rectangle &rNativeBoundingRegion, Rectangle &rNativeContentRegion )
{
    switch(nType)
    {
    default:
        return sal_False;
    }
}

sal_Bool GtkSalGraphics::IsNativeControlSupported( ControlType nType, ControlPart nPart )
{
#if 0
    if(nType == CTRL_PUSHBUTTON)
        return sal_True;
#endif
    return sal_False;
}

void GtkData::initNWF() {}
void GtkData::deInitNWF() {}

GtkSalGraphics::GtkSalGraphics( GtkSalFrame *pFrame, GtkWidget *pWindow )
    : SvpSalGraphics(),
      mpFrame( pFrame ),
      mpWindow( pWindow ),
      mpButtonStyle( NULL )
{
}

void GtkSalGraphics::updateSettings( AllSettings& rSettings )
{
    g_warning ("unimplemented GtkSalGraphics update");
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
    mpFrame->pushIgnoreDamage();
    SvpSalGraphics::copyArea( nDestX, nDestY, nSrcX, nSrcY, nSrcWidth, nSrcHeight, nFlags );
    mpFrame->popIgnoreDamage();

    cairo_rectangle_int_t rect = { (int)nSrcX, (int)nSrcY, (int)nSrcWidth, (int)nSrcHeight };
    cairo_region_t *region = cairo_region_create_rectangle( &rect );

    print_update_area( gtk_widget_get_window( mpFrame->getWindow() ), "before copy area" );

//    print_cairo_region( mpFrame->m_pRegion, "extremely odd SalFrame: shape combine region! - ");

    g_warning( "FIXME: copy area delta: %d %d needs clip intersect\n",
               nDestX - nSrcX, nDestY - nSrcY );

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
                                            (int)aClipRect.Right(), (int)aClipRect.Bottom() };
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
