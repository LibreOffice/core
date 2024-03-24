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

#include <sal/config.h>

#include <string_view>

#include "atkwrapper.hxx"

#include <com/sun/star/accessibility/XAccessibleImage.hpp>

using namespace ::com::sun::star;

// FIXME
static const gchar *
getAsConst( std::u16string_view rString )
{
    static const int nMax = 10;
    static OString aUgly[nMax];
    static int nIdx = 0;
    nIdx = (nIdx + 1) % nMax;
    aUgly[nIdx] = OUStringToOString( rString, RTL_TEXTENCODING_UTF8 );
    return aUgly[ nIdx ].getStr();
}

/// @throws uno::RuntimeException
static css::uno::Reference<css::accessibility::XAccessibleImage>
    getImage( AtkImage *pImage )
{
    AtkObjectWrapper *pWrap = ATK_OBJECT_WRAPPER( pImage );
    if( pWrap )
    {
        if( !pWrap->mpImage.is() )
        {
            pWrap->mpImage.set(pWrap->mpContext, css::uno::UNO_QUERY);
        }

        return pWrap->mpImage;
    }

    return css::uno::Reference<css::accessibility::XAccessibleImage>();
}

extern "C" {

static const gchar *
image_get_image_description( AtkImage *image )
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleImage> pImage
            = getImage( image );
        if( pImage.is() )
            return getAsConst( pImage->getAccessibleImageDescription() );
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in getAccessibleImageDescription()" );
    }

    return nullptr;
}

static void
image_get_image_position( AtkImage     *image,
                          gint         *x,
                          gint         *y,
                          AtkCoordType  coord_type )
{
    *x = *y = -1;
    if( ATK_IS_COMPONENT( image ) )
    {
        gint nWidth = -1;
        gint nHeight = -1;
        atk_component_get_extents(ATK_COMPONENT(image), x, y, &nWidth, &nHeight, coord_type);
    }
    else
        g_warning( "FIXME: no image position information" );
}

static void
image_get_image_size( AtkImage *image,
                      gint     *width,
                      gint     *height )
{
    *width = *height = -1;
    try {
        css::uno::Reference<css::accessibility::XAccessibleImage> pImage
            = getImage( image );
        if( pImage.is() )
        {
            *width = pImage->getAccessibleImageWidth();
            *height = pImage->getAccessibleImageHeight();
        }
    }
    catch(const uno::Exception&) {
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
imageIfaceInit (gpointer iface_, gpointer)
{
  auto const iface = static_cast<AtkImageIface *>(iface_);
  g_return_if_fail (iface != nullptr);

  iface->set_image_description = image_set_image_description;
  iface->get_image_description = image_get_image_description;
  iface->get_image_position = image_get_image_position;
  iface->get_image_size = image_get_image_size;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
