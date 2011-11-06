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
#include "precompiled_sfx2.hxx"

// INCLUDE ---------------------------------------------------------------

#ifndef GCC
#endif

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
            aFilterList.Insert(pTupel, LIST_APPEND);
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

// Destruktor; gibt interne Daten frei

SfxStyleFamilyItem::~SfxStyleFamilyItem()
{
    SfxFilterTupel *pTupel = aFilterList.First();
    while(pTupel)
    {
        delete pTupel;
        pTupel = aFilterList.Next();
    }
}

// -----------------------------------------------------------------------

// Implementierung des Resource-Konstruktors

SfxStyleFamilies::SfxStyleFamilies( const ResId& rResId ) :

    Resource( rResId.SetRT( RSC_SFX_STYLE_FAMILIES ).SetAutoRelease( sal_False ) ),
    aEntryList( 4, 1 )
{
    sal_uIntPtr nCount = ReadLongRes();
    for( sal_uIntPtr i = 0; i < nCount; i++ )
    {
        const ResId aResId((RSHEADER_TYPE *)GetClassRes(), *rResId.GetResMgr());
        SfxStyleFamilyItem *pItem = new SfxStyleFamilyItem(aResId);
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
        aEntryList.Insert(pItem, LIST_APPEND);
    }

    FreeResource();

    updateImages( rResId, BMP_COLOR_NORMAL );
}

// -----------------------------------------------------------------------

// Destruktor; gibt interne Daten frei

SfxStyleFamilies::~SfxStyleFamilies()
{
    SfxStyleFamilyItem *pItem = aEntryList.First();

    while(pItem)
    {
        delete pItem;
        pItem = aEntryList.Next();
    }
}


// -----------------------------------------------------------------------

sal_Bool SfxStyleFamilies::updateImages( const ResId& _rId, const BmpColorMode _eMode )
{
    sal_Bool bSuccess = sal_False;

    {
        ::svt::OLocalResourceAccess aLocalRes( _rId );

        // check if the image list is present
        ResId aImageListId( (sal_uInt16)_eMode + 1, *_rId.GetResMgr() );
        aImageListId.SetRT( RSC_IMAGELIST );

        if ( aLocalRes.IsAvailableRes( aImageListId ) )
        {   // there is such a list
            ImageList aImages( aImageListId );

            // number of styles items/images
            sal_uInt16 nCount = aImages.GetImageCount( );
            DBG_ASSERT( Count() == nCount, "SfxStyleFamilies::updateImages: found the image list, but missing some bitmaps!" );
            if ( nCount > Count() )
                nCount = Count();

            // set the images on the items
            for ( sal_uInt16 i = 0; i < nCount; ++i )
            {
                SfxStyleFamilyItem* pItem = static_cast< SfxStyleFamilyItem* >( aEntryList.GetObject( i ) );
                pItem->SetImage( aImages.GetImage( aImages.GetImageId( i ) ) );
            }

            bSuccess = sal_True;
        }
    }

    return bSuccess;
}
