/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: styfitem.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:32:39 $
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

// INCLUDE ---------------------------------------------------------------

#ifndef GCC
#pragma hdrstop
#endif

#include "styfitem.hxx"

#ifndef _SVTOOLS_LOCALRESACCESS_HXX_
#include <svtools/localresaccess.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

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
    ULONG nMask = ReadLongRes();

    if(nMask & RSC_SFX_STYLE_ITEM_LIST)
    {
        ULONG nCount = ReadLongRes();
        for( ULONG i = 0; i < nCount; i++ )
        {
            SfxFilterTupel *pTupel = new SfxFilterTupel;
            pTupel->aName = ReadStringRes();
            long lFlags = ReadLongRes();
            pTupel->nFlags = (USHORT)lFlags;
            aFilterList.Insert(pTupel, LIST_APPEND);
        }
    }
    if(nMask & RSC_SFX_STYLE_ITEM_BITMAP)
    {
        aBitmap = Bitmap(ResId((RSHEADER_TYPE *)GetClassRes()));
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
        nFamily = (USHORT)ReadLongRes();
    }
    else
        nFamily = SFX_STYLE_FAMILY_PARA;
    if(nMask & RSC_SFX_STYLE_ITEM_IMAGE)
    {
        aImage = Image(ResId((RSHEADER_TYPE *)GetClassRes()));
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

    Resource( rResId.SetRT( RSC_SFX_STYLE_FAMILIES ).SetAutoRelease( FALSE ) ),
    aEntryList( 4, 1 )
{
    ULONG nCount = ReadLongRes();
    for( ULONG i = 0; i < nCount; i++ )
    {
        const ResId aResId((RSHEADER_TYPE *)GetClassRes());
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
        ResId aImageListId( (sal_uInt16)_eMode + 1 );
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
