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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

// INCLUDE ---------------------------------------------------------------

#include <sfx2/styfitem.hxx>
#include <svtools/localresaccess.hxx>
#include <tools/debug.hxx>

// -----------------------------------------------------------------------

class SfxStyleFamilyItem_Impl
{
    Bitmap aBitmap;
    Image   aImage;
};

// -----------------------------------------------------------------------

// Implementierung des Resource-Konstruktors

SfxStyleFamilyItem::SfxStyleFamilyItem( const ResId &rResId ) :

    Resource( rResId.SetRT( RSC_SFX_STYLE_FAMILY_ITEM ) )

{
    sal_uIntPtr nMask = ReadLongRes();

    if(nMask & RSC_SFX_STYLE_ITEM_LIST)
    {
        sal_uIntPtr nCount = ReadLongRes();
        for( sal_uIntPtr i = 0; i < nCount; i++ )
        {
            SfxFilterTupel *pTupel = new SfxFilterTupel;
            pTupel->aName = ReadStringRes();
            long lFlags = ReadLongRes();
            pTupel->nFlags = (sal_uInt16)lFlags;
            aFilterList.push_back( pTupel );
        }
    }
    if(nMask & RSC_SFX_STYLE_ITEM_BITMAP)
    {
        aBitmap = Bitmap(ResId((RSHEADER_TYPE *)GetClassRes(),*rResId.GetResMgr()));
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
    }
    if(nMask & RSC_SFX_STYLE_ITEM_TEXT)
    {
        aText = ReadStringRes();
    }
    if(nMask & RSC_SFX_STYLE_ITEM_HELPTEXT)
    {
        aHelpText = ReadStringRes();
    }
    if(nMask & RSC_SFX_STYLE_ITEM_STYLEFAMILY)
    {
        nFamily = (sal_uInt16)ReadLongRes();
    }
    else
        nFamily = SFX_STYLE_FAMILY_PARA;
    if(nMask & RSC_SFX_STYLE_ITEM_IMAGE)
    {
        aImage = Image(ResId((RSHEADER_TYPE *)GetClassRes(),*rResId.GetResMgr()));
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
    }
    else
        aImage = Image(aBitmap);
}

// -----------------------------------------------------------------------

// Destructor; releases the internal data

SfxStyleFamilyItem::~SfxStyleFamilyItem()
{
    for ( size_t i = 0, n = aFilterList.size(); i < n; ++i )
        delete aFilterList[ i ];
    aFilterList.clear();
}

// -----------------------------------------------------------------------

// Implementation of the resource constructor

SfxStyleFamilies::SfxStyleFamilies( const ResId& rResId ) :
    Resource( rResId.SetRT( RSC_SFX_STYLE_FAMILIES ).SetAutoRelease( false ) )
{
    sal_uIntPtr nCount = ReadLongRes();
    for( sal_uIntPtr i = 0; i < nCount; i++ )
    {
        const ResId aResId((RSHEADER_TYPE *)GetClassRes(), *rResId.GetResMgr());
        SfxStyleFamilyItem *pItem = new SfxStyleFamilyItem(aResId);
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
        aEntryList.push_back( pItem );
    }

    FreeResource();

    updateImages( rResId );
}

// -----------------------------------------------------------------------

// Destructor; releases the internal data

SfxStyleFamilies::~SfxStyleFamilies()
{
    for ( size_t i = 0, n = aEntryList.size(); i < n; ++i )
        delete aEntryList[ i ];
    aEntryList.clear();
}


// -----------------------------------------------------------------------

sal_Bool SfxStyleFamilies::updateImages( const ResId& _rId )
{
    sal_Bool bSuccess = sal_False;

    {
        ::svt::OLocalResourceAccess aLocalRes( _rId );

        // check if the image list is present
        ResId aImageListId( (sal_uInt16) 1, *_rId.GetResMgr() );
        aImageListId.SetRT( RSC_IMAGELIST );

        if ( aLocalRes.IsAvailableRes( aImageListId ) )
        {   // there is such a list
            ImageList aImages( aImageListId );

            // number of styles items/images
            sal_uInt16 nCount = aImages.GetImageCount( );
            DBG_ASSERT( aEntryList.size() == nCount, "SfxStyleFamilies::updateImages: found the image list, but missing some bitmaps!" );
            if ( nCount > aEntryList.size() )
                nCount = aEntryList.size();

            // set the images on the items
            for ( size_t i = 0; i < nCount; ++i )
            {
                SfxStyleFamilyItem* pItem = aEntryList[ i ];
                pItem->SetImage( aImages.GetImage( aImages.GetImageId( i ) ) );
            }

            bSuccess = sal_True;
        }
    }

    return bSuccess;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
