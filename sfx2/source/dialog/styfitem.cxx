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

#include <sfx2/styfitem.hxx>
#include <svtools/localresaccess.hxx>
#include <tools/debug.hxx>


// Implementierung des Resource-Konstruktors

SfxStyleFamilyItem::SfxStyleFamilyItem( const ResId &rResId ) :

    Resource( rResId.SetRT( RSC_SFX_STYLE_FAMILY_ITEM ) )

{
    const sal_Int32 nMask = ReadLongRes();

    if(nMask & (sal_uInt32)SfxStyleItem::List)
    {
        const sal_Int32 nCount = ReadLongRes();
        for( sal_Int32 i = 0; i < nCount; ++i )
        {
            SfxFilterTupel *pTupel = new SfxFilterTupel;
            pTupel->aName = ReadStringRes();
            pTupel->nFlags = static_cast<sal_uInt16>(ReadLongRes());
            aFilterList.push_back( pTupel );
        }
    }
    if(nMask & (sal_uInt32)SfxStyleItem::Bitmap)
    {
        aBitmap = Bitmap(ResId(static_cast<RSHEADER_TYPE *>(GetClassRes()),*rResId.GetResMgr()));
        IncrementRes( GetObjSizeRes( static_cast<RSHEADER_TYPE *>(GetClassRes()) ) );
    }
    if(nMask & (sal_uInt32)SfxStyleItem::Text)
    {
        aText = ReadStringRes();
    }
    if(nMask & (sal_uInt32)SfxStyleItem::HelpText)
    {
        aHelpText = ReadStringRes();
    }
    if(nMask & (sal_uInt32)SfxStyleItem::StyleFamily)
    {
        nFamily = static_cast<SfxStyleFamily>(ReadLongRes());
    }
    else
        nFamily = SfxStyleFamily::Para;

    if(nMask & (sal_uInt32)SfxStyleItem::Image)
    {
        aImage = Image(ResId(static_cast<RSHEADER_TYPE *>(GetClassRes()),*rResId.GetResMgr()));
        IncrementRes( GetObjSizeRes( static_cast<RSHEADER_TYPE *>(GetClassRes()) ) );
    }
    else
        aImage = Image(aBitmap);
}


// Destructor; releases the internal data

SfxStyleFamilyItem::~SfxStyleFamilyItem()
{
    for (SfxFilterTupel* p : aFilterList)
        delete p;
    aFilterList.clear();
}


// Implementation of the resource constructor

SfxStyleFamilies::SfxStyleFamilies( const ResId& rResId ) :
    Resource( rResId.SetRT( RSC_SFX_STYLE_FAMILIES ).SetAutoRelease( false ) )
{
    const sal_Int32 nCount = ReadLongRes();
    for( sal_Int32 i = 0; i < nCount; ++i )
    {
        const ResId aResId(static_cast<RSHEADER_TYPE *>(GetClassRes()), *rResId.GetResMgr());
        SfxStyleFamilyItem *pItem = new SfxStyleFamilyItem(aResId);
        IncrementRes( GetObjSizeRes( static_cast<RSHEADER_TYPE *>(GetClassRes()) ) );
        aEntryList.push_back( pItem );
    }

    FreeResource();

    updateImages( rResId );
}


// Destructor; releases the internal data

SfxStyleFamilies::~SfxStyleFamilies()
{
    for (SfxStyleFamilyItem* p : aEntryList)
        delete p;
    aEntryList.clear();
}


void SfxStyleFamilies::updateImages( const ResId& _rId )
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
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
