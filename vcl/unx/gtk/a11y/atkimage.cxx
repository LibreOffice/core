/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: atkimage.cxx,v $
 * $Revision: 1.4 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "atkwrapper.hxx"

#include <com/sun/star/accessibility/XAccessibleImage.hpp>

#include <stdio.h>

using namespace ::com::sun::star;

// FIXME
static G_CONST_RETURN gchar *
getAsConst( rtl::OUString rString )
{
    static const int nMax = 10;
    static rtl::OString aUgly[nMax];
    static int nIdx = 0;
    nIdx = (nIdx + 1) % nMax;
    aUgly[nIdx] = rtl::OUStringToOString( rString, RTL_TEXTENCODING_UTF8 );
    return aUgly[ nIdx ];
}

static accessibility::XAccessibleImage*
    getImage( AtkImage *pImage ) throw (uno::RuntimeException)
{
    AtkObjectWrapper *pWrap = ATK_OBJECT_WRAPPER( pImage );
    if( pWrap )
    {
        if( !pWrap->mpImage && pWrap->mpContext )
        {
            uno::Any any = pWrap->mpContext->queryInterface( accessibility::XAccessibleImage::static_type(NULL) );
            pWrap->mpImage = reinterpret_cast< accessibility::XAccessibleImage * > (any.pReserved);
            pWrap->mpImage->acquire();
        }

        return pWrap->mpImage;
    }

    return NULL;
}

extern "C" {

static G_CONST_RETURN gchar *
image_get_image_description( AtkImage *image )
{
    try {
        accessibility::XAccessibleImage* pImage = getImage( image );
        if( pImage )
            return getAsConst( pImage->getAccessibleImageDescription() );
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getAccessibleImageDescription()" );
    }

    return NULL;
}

static void
image_get_image_position( AtkImage     *image,
                          gint         *x,
                          gint         *y,
                          AtkCoordType  coord_type )
{
    *x = *y = 0;
    if( ATK_IS_COMPONENT( image ) )
        atk_component_get_position( ATK_COMPONENT( image ), x, y, coord_type );
    else
        g_warning( "FIXME: no image position information" );
}

static void
image_get_image_size( AtkImage *image,
                      gint     *width,
                      gint     *height )
{
    *width = 0;
    *height = 0;
    try {
        accessibility::XAccessibleImage* pImage = getImage( image );
        if( pImage )
        {
            *width = pImage->getAccessibleImageWidth();
            *height = pImage->getAccessibleImageHeight();
        }
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getAccessibleImageHeight() or Width" );
    }
}

static gboolean
image_set_image_description( AtkImage *, const gchar * )
{
    g_warning ("FIXME: no set image description");
    return FALSE;
}

} // extern "C"

void
imageIfaceInit (AtkImageIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->set_image_description = image_set_image_description;
  iface->get_image_description = image_get_image_description;
  iface->get_image_position = image_get_image_position;
  iface->get_image_size = image_get_image_size;
}
