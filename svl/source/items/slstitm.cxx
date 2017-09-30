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


#include <svl/slstitm.hxx>
#include <svl/poolitem.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/diagnose.h>
#include <tools/stream.hxx>
#include <stringio.hxx>

SfxPoolItem* SfxStringListItem::CreateDefault() { return new SfxStringListItem; }

class SfxImpStringList
{
public:
    std::vector<OUString>  aList;

    SfxImpStringList() {}
};


SfxStringListItem::SfxStringListItem()
{
}


SfxStringListItem::SfxStringListItem( sal_uInt16 which, const std::vector<OUString>* pList ) :
    SfxPoolItem( which )
{
    // FIXME: Putting an empty list does not work
    // Therefore the query after the count is commented out
    if( pList /*!!! && pList->Count() */ )
    {
        pImpl.reset(new SfxImpStringList);
        pImpl->aList = *pList;
    }
}


SfxStringListItem::SfxStringListItem( sal_uInt16 which, SvStream& rStream ) :
    SfxPoolItem( which )
{
    sal_Int32 nEntryCount;
    rStream.ReadInt32( nEntryCount );

    if( nEntryCount )
        pImpl.reset(new SfxImpStringList);

    if (pImpl)
    {
        for( sal_Int32 i=0; i < nEntryCount; i++ )
        {
            pImpl->aList.push_back( readByteString(rStream) );
        }
    }
}

std::vector<OUString>& SfxStringListItem::GetList()
{
    if( !pImpl )
        pImpl.reset( new SfxImpStringList );
    return pImpl->aList;
}

const std::vector<OUString>& SfxStringListItem::GetList () const
{
    return const_cast< SfxStringListItem * >(this)->GetList();
}


bool SfxStringListItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));

    const SfxStringListItem& rSSLItem = static_cast<const SfxStringListItem&>(rItem);

    return pImpl == rSSLItem.pImpl;
}


bool SfxStringListItem::GetPresentation
(
    SfxItemPresentation     /*ePresentation*/,
    MapUnit                 /*eCoreMetric*/,
    MapUnit                 /*ePresentationMetric*/,
    OUString&               rText,
    const IntlWrapper&
)   const
{
    rText = "(List)";
    return false;
}


SfxPoolItem* SfxStringListItem::Clone( SfxItemPool *) const
{
    return new SfxStringListItem( *this );
}


SfxPoolItem* SfxStringListItem::Create( SvStream & rStream, sal_uInt16 ) const
{
    return new SfxStringListItem( Which(), rStream );
}


SvStream& SfxStringListItem::Store( SvStream & rStream, sal_uInt16 ) const
{
    if( !pImpl )
    {
        rStream.WriteInt32( 0 );
        return rStream;
    }

    sal_uInt32 nCount = pImpl->aList.size();
    rStream.WriteUInt32( nCount );

    for( sal_uInt32 i=0; i < nCount; i++ )
        writeByteString(rStream, pImpl->aList[i]);

    return rStream;
}


void SfxStringListItem::SetString( const OUString& rStr )
{
    pImpl.reset( new SfxImpStringList );

    sal_Int32 nStart = 0;
    OUString aStr(convertLineEnd(rStr, LINEEND_CR));
    for (;;)
    {
        const sal_Int32 nDelimPos = aStr.indexOf( '\r', nStart );
        if ( nDelimPos < 0 )
        {
            if (nStart<aStr.getLength())
            {
                // put last string only if not empty
                pImpl->aList.push_back(aStr.copy(nStart));
            }
            break;
        }

        pImpl->aList.push_back(aStr.copy(nStart, nDelimPos-nStart));

        // skip both inserted string and delimiter
        nStart = nDelimPos + 1 ;
    }
}


OUString SfxStringListItem::GetString()
{
    OUString aStr;
    if ( pImpl )
    {
        std::vector<OUString>::const_iterator iter = pImpl->aList.begin();
        for (;;)
        {
            aStr += *iter;
            ++iter;

            if (iter == pImpl->aList.end())
                break;

            aStr += "\r";
        }
    }
    return convertLineEnd(aStr, GetSystemLineEnd());
}


void SfxStringListItem::SetStringList( const css::uno::Sequence< OUString >& rList )
{
    pImpl.reset(new SfxImpStringList);

    // String belongs to the list
    for ( sal_Int32 n = 0; n < rList.getLength(); n++ )
        pImpl->aList.push_back(rList[n]);
}

void SfxStringListItem::GetStringList( css::uno::Sequence< OUString >& rList ) const
{
    long nCount = pImpl->aList.size();

    rList.realloc( nCount );
    for( long i=0; i < nCount; i++ )
        rList[i] = pImpl->aList[i];
}

// virtual
bool SfxStringListItem::PutValue( const css::uno::Any& rVal, sal_uInt8 )
{
    css::uno::Sequence< OUString > aValue;
    if ( rVal >>= aValue )
    {
        SetStringList( aValue );
        return true;
    }

    OSL_FAIL( "SfxStringListItem::PutValue - Wrong type!" );
    return false;
}

// virtual
bool SfxStringListItem::QueryValue( css::uno::Any& rVal, sal_uInt8 ) const
{
    // GetString() is not const!!!
    SfxStringListItem* pThis = const_cast< SfxStringListItem * >( this );

    css::uno::Sequence< OUString > aStringList;
    pThis->GetStringList( aStringList );
    rVal <<= aStringList;
    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
