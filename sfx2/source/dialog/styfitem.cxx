/*************************************************************************
 *
 *  $RCSfile: styfitem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// INCLUDE ---------------------------------------------------------------

#pragma hdrstop

#include "styfitem.hxx"

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
    USHORT nMask = (USHORT) ReadShortRes();

    if(nMask & RSC_SFX_STYLE_ITEM_LIST)
    {
        USHORT nCount = (USHORT) ReadShortRes();
        for( USHORT i = 0; i < nCount; i++ )
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
        nFamily = (USHORT)ReadShortRes();
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

    Resource( rResId.SetRT( RSC_SFX_STYLE_FAMILIES ) ),
    aEntryList( 4, 1 )
{
    USHORT nCount = (USHORT)ReadShortRes();
    for( USHORT i = 0; i < nCount; i++ )
    {
        const ResId aResId((RSHEADER_TYPE *)GetClassRes());
        SfxStyleFamilyItem *pItem = new SfxStyleFamilyItem(aResId);
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
        aEntryList.Insert(pItem, LIST_APPEND);
    }
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


