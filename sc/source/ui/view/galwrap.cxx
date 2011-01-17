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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"




// INCLUDE ---------------------------------------------------------------

#include <vcl/graph.hxx>
#include <svx/gallery.hxx>
#include <sfx2/app.hxx>

// -----------------------------------------------------------------------

Graphic GalleryGetGraphic()
{
    GalleryExplorer* pGal = SVX_GALLERY();
    DBG_ASSERT( pGal, "Wo ist die Gallery?" );
    return pGal->GetGraphic();
}

sal_uInt16 GallerySGA_FORMAT_GRAPHIC()
{
    return SGA_FORMAT_GRAPHIC;
}

sal_Bool GalleryIsLinkage()
{
    GalleryExplorer* pGal = SVX_GALLERY();
    DBG_ASSERT( pGal, "Wo ist die Gallery?" );
    return pGal->IsLinkage();
}

String GalleryGetFullPath()
{
    GalleryExplorer* pGal = SVX_GALLERY();
    DBG_ASSERT( pGal, "Wo ist die Gallery?" );
//  return pGal->GetPath().GetFull();
    return pGal->GetURL().GetMainURL(INetURLObject::NO_DECODE);
    // URL as stored in GraphicLink must be encoded
}

String GalleryGetFilterName()
{
    GalleryExplorer* pGal = SVX_GALLERY();
    DBG_ASSERT( pGal, "Wo ist die Gallery?" );
    return pGal->GetFilterName();
}




