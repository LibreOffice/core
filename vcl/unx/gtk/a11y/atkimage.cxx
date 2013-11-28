/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    return aUgly[ nIdx ].getStr();
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
