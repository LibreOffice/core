/*************************************************************************
 *
 *  $RCSfile: numinf.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:21 $
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

// include ---------------------------------------------------------------

#pragma hdrstop

#define _SVX_NUMINF_CXX
#define ITEMID_NUMBERINFO   0

#include "numinf.hxx"

// -----------------------------------------------------------------------

TYPEINIT1(SvxNumberInfoItem, SfxPoolItem);

// class SvxNumberInfoItem -----------------------------------------------

#define INIT(pNum,eVal,nDouble,rStr)    \
    SfxPoolItem     ( nId ),            \
                                        \
    pFormatter      ( pNum ),           \
    eValueType      ( eVal ),           \
    nDoubleVal      ( nDouble ),        \
    aStringVal      ( rStr ),           \
    pDelFormatArr   ( NULL ),           \
    nDelCount       ( 0 )

SvxNumberInfoItem::SvxNumberInfoItem( const sal_uInt16 nId ) :

    INIT( NULL, SVX_VALUE_TYPE_UNDEFINED, 0, String() )

{
}

// -----------------------------------------------------------------------

SvxNumberInfoItem::SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                                      const sal_uInt16 nId ) :

    INIT( pNumFormatter, SVX_VALUE_TYPE_UNDEFINED, 0, String() )

{
}

// -----------------------------------------------------------------------

SvxNumberInfoItem::SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                                      const String& rVal, const sal_uInt16 nId ) :

    INIT( pNumFormatter, SVX_VALUE_TYPE_STRING, 0, rVal )

{
}

// -----------------------------------------------------------------------

SvxNumberInfoItem::SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                                      const double& rVal, const sal_uInt16 nId ) :

    INIT( pNumFormatter, SVX_VALUE_TYPE_NUMBER, rVal, String() )

{
}

#undef INIT

// -----------------------------------------------------------------------

SvxNumberInfoItem::SvxNumberInfoItem( const SvxNumberInfoItem& rItem ) :

    SfxPoolItem( rItem.Which() ),

    pFormatter   ( rItem.pFormatter ),
    eValueType   ( rItem.eValueType ),
    nDoubleVal   ( rItem.nDoubleVal ),
    aStringVal   ( rItem.aStringVal ),
    pDelFormatArr( NULL ),
    nDelCount    ( rItem.nDelCount )

{
    if ( rItem.nDelCount > 0 )
    {
        pDelFormatArr = new sal_uInt32[ rItem.nDelCount ];

        for ( sal_uInt16 i = 0; i < rItem.nDelCount; ++i )
            pDelFormatArr[i] = rItem.pDelFormatArr[i];
    }
}

// -----------------------------------------------------------------------

SvxNumberInfoItem::~SvxNumberInfoItem()
{
    if ( pDelFormatArr )
        delete []pDelFormatArr;
}

//------------------------------------------------------------------------

SfxItemPresentation SvxNumberInfoItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText, const International *
)   const
{
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

int SvxNumberInfoItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal which or type" );

    SvxNumberInfoItem& rOther = (SvxNumberInfoItem&)rItem;

    sal_Bool bEqual = sal_False;

    if ( nDelCount == rOther.nDelCount )
    {
        if ( nDelCount > 0 )
        {
            if ( pDelFormatArr != NULL && rOther.pDelFormatArr != NULL )
            {
                bEqual = sal_True;

                for ( sal_uInt16 i = 0; i < nDelCount && bEqual; ++i )
                    bEqual = ( pDelFormatArr[i] == rOther.pDelFormatArr[i] );
            }
        }
        else if ( nDelCount == 0 )
            bEqual = ( pDelFormatArr == NULL && rOther.pDelFormatArr == NULL );

        bEqual = bEqual &&
                 pFormatter == rOther.pFormatter &&
                 eValueType == rOther.eValueType &&
                 nDoubleVal == rOther.nDoubleVal &&
                 aStringVal == rOther.aStringVal;
    }
    return bEqual;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxNumberInfoItem::Clone( SfxItemPool * ) const
{
    return new SvxNumberInfoItem( *this );
}

// Laden/Speichern wird nicht gebraucht!
// -----------------------------------------------------------------------

SfxPoolItem* SvxNumberInfoItem::Create( SvStream& rStream, sal_uInt16 ) const
{
    return new SvxNumberInfoItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxNumberInfoItem::Store( SvStream &rStream, sal_uInt16 nItemVersion ) const
{
    return rStream;
}

// -----------------------------------------------------------------------

void SvxNumberInfoItem::SetNumberFormatter( SvNumberFormatter* pNumFormatter )
{
    pFormatter = pNumFormatter;
}

// -----------------------------------------------------------------------

void SvxNumberInfoItem::SetStringValue( const String& rNewVal )
{
    aStringVal = rNewVal;
    eValueType = SVX_VALUE_TYPE_STRING;
}

// -----------------------------------------------------------------------

void SvxNumberInfoItem::SetDoubleValue( const double& rNewVal )
{
    nDoubleVal = rNewVal;
    eValueType = SVX_VALUE_TYPE_NUMBER;
}

// -----------------------------------------------------------------------

void SvxNumberInfoItem::SetDelFormatArray( const sal_uInt32* pData,
                                           const sal_uInt32 nCount )
{
    if ( pDelFormatArr )
    {
        delete []pDelFormatArr;
        pDelFormatArr = NULL;
    }

    nDelCount = nCount;

    if ( nCount > 0 )
    {
        pDelFormatArr = new sal_uInt32[ nCount ];

        if ( pData != NULL )
        {
            for ( sal_uInt16 i = 0; i < nCount; ++i )
                pDelFormatArr[i] = pData[i];
        }
    }
}


