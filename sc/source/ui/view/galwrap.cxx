/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: galwrap.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:55:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

USHORT GallerySGA_FORMAT_GRAPHIC()
{
    return SGA_FORMAT_GRAPHIC;
}

BOOL GalleryIsLinkage()
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




