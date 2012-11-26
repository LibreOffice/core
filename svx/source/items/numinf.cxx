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
#include "precompiled_svx.hxx"

#define _SVX_NUMINF_CXX

#include <svx/numinf.hxx>

// class SvxNumberInfoItem -----------------------------------------------

#define INIT(pNum,eVal,nDouble,rStr)    \
    SfxPoolItem     ( nId ),            \
                                        \
    pFormatter      ( pNum ),           \
    eValueType      ( eVal ),           \
    aStringVal      ( rStr ),           \
    nDoubleVal      ( nDouble ),        \
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

// -----------------------------------------------------------------------

SvxNumberInfoItem::SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                                      const double& rVal, const String& rValueStr,
                                      const sal_uInt16 nId ) :

    INIT( pNumFormatter, SVX_VALUE_TYPE_NUMBER, rVal, rValueStr )

{
}

#undef INIT

// -----------------------------------------------------------------------

SvxNumberInfoItem::SvxNumberInfoItem( const SvxNumberInfoItem& rItem ) :

    SfxPoolItem( rItem.Which() ),

    pFormatter   ( rItem.pFormatter ),
    eValueType   ( rItem.eValueType ),
    aStringVal   ( rItem.aStringVal ),
    nDoubleVal   ( rItem.nDoubleVal ),
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
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    String&             rText, const IntlWrapper *
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

SfxPoolItem* SvxNumberInfoItem::Create( SvStream& /*rStream*/, sal_uInt16 ) const
{
    return new SvxNumberInfoItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxNumberInfoItem::Store( SvStream &rStream, sal_uInt16 /*nItemVersion*/ ) const
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


