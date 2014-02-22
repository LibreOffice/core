/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <sfx2/styfitem.hxx>
#include <svtools/localresaccess.hxx>
#include <tools/debug.hxx>



class SfxStyleFamilyItem_Impl
{
    Bitmap aBitmap;
    Image   aImage;
};





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





SfxStyleFamilyItem::~SfxStyleFamilyItem()
{
    for ( size_t i = 0, n = aFilterList.size(); i < n; ++i )
        delete aFilterList[ i ];
    aFilterList.clear();
}





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





SfxStyleFamilies::~SfxStyleFamilies()
{
    for ( size_t i = 0, n = aEntryList.size(); i < n; ++i )
        delete aEntryList[ i ];
    aEntryList.clear();
}




sal_Bool SfxStyleFamilies::updateImages( const ResId& _rId )
{
    sal_Bool bSuccess = sal_False;

    {
        ::svt::OLocalResourceAccess aLocalRes( _rId );

        
        ResId aImageListId( (sal_uInt16) 1, *_rId.GetResMgr() );
        aImageListId.SetRT( RSC_IMAGELIST );

        if ( aLocalRes.IsAvailableRes( aImageListId ) )
        {   
            ImageList aImages( aImageListId );

            
            sal_uInt16 nCount = aImages.GetImageCount( );
            DBG_ASSERT( aEntryList.size() == nCount, "SfxStyleFamilies::updateImages: found the image list, but missing some bitmaps!" );
            if ( nCount > aEntryList.size() )
                nCount = aEntryList.size();

            
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
