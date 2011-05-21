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
#include "precompiled_svl.hxx"

#include <svl/slstitm.hxx>
#include <svl/poolitem.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/stream.hxx>

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxStringListItem)

// -----------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SfxStringListItem, SfxPoolItem);

class SfxImpStringList
{
public:
    sal_uInt16  nRefCount;
    std::vector<String> aList;

            SfxImpStringList() { nRefCount = 1; }
            ~SfxImpStringList();
    void    Sort( sal_Bool bAscending);
};

//------------------------------------------------------------------------

SfxImpStringList::~SfxImpStringList()
{
    DBG_ASSERT(nRefCount!=0xffff,"ImpList already deleted");
    nRefCount = 0xffff;
}

//------------------------------------------------------------------------

void SfxImpStringList::Sort( sal_Bool bAscending)
{
    sal_uLong nCount = aList.size();
    if( nCount > 1 )
    {
        nCount -= 2;
        // Bubble Dir Einen
        sal_Bool bSwapped = sal_True;
        while( bSwapped )
        {
            bSwapped = sal_False;
            for( sal_uLong nCur = 0; nCur <= nCount; nCur++ )
            {
                String aStr1 = aList[nCur];
                String aStr2 = aList[nCur+1];
                // COMPARE_GREATER => pStr2 ist groesser als pStr1
                StringCompare eCompare = aStr1.CompareIgnoreCaseToAscii( aStr2 ); //@@@
                sal_Bool bSwap = sal_False;
                if( bAscending )
                {
                    if( eCompare == COMPARE_LESS )
                        bSwap = sal_True;
                }
                else if( eCompare == COMPARE_GREATER )
                    bSwap = sal_True;

                if( bSwap )
                {
                    bSwapped = sal_True;
                    aList[nCur+1] = aStr1;
                    aList[nCur] = aStr2;
                }
            }
        }
    }
}

// class SfxStringListItem -----------------------------------------------

SfxStringListItem::SfxStringListItem() :
    pImp(NULL)
{
}

//------------------------------------------------------------------------

SfxStringListItem::SfxStringListItem( sal_uInt16 which, const std::vector<String>* pList ) :
    SfxPoolItem( which ),
    pImp(NULL)
{
    // PB: das Putten einer leeren Liste funktionierte nicht,
    // deshalb habe ich hier die Abfrage nach dem Count auskommentiert
    if( pList /*!!! && pList->Count() */ )
    {
        pImp = new SfxImpStringList;

        if (pImp)
            pImp->aList = *pList;
    }
}

//------------------------------------------------------------------------

SfxStringListItem::SfxStringListItem( sal_uInt16 which, SvStream& rStream ) :
    SfxPoolItem( which ),
    pImp(NULL)
{
    long nEntryCount;
    rStream >> nEntryCount;

    if( nEntryCount )
        pImp = new SfxImpStringList;

    if (pImp)
    {
        long   i;
        String  aStr;
        for( i=0; i < nEntryCount; i++ )
        {
            readByteString(rStream, aStr);
            pImp->aList.push_back(aStr);
        }
    }
}

//------------------------------------------------------------------------

SfxStringListItem::SfxStringListItem( const SfxStringListItem& rItem ) :
    SfxPoolItem( rItem ),
    pImp(rItem.pImp)
{
    if( pImp )
    {
        DBG_ASSERT(pImp->nRefCount!=0xffff,"ImpList not valid");
        pImp->nRefCount++;
    }
}

//------------------------------------------------------------------------

SfxStringListItem::~SfxStringListItem()
{
    if( pImp )
    {
        DBG_ASSERT(pImp->nRefCount!=0xffff,"ImpList not valid");
        if( pImp->nRefCount > 1 )
            pImp->nRefCount--;
        else
            delete pImp;
    }
}

//------------------------------------------------------------------------

std::vector<String>& SfxStringListItem::GetList()
{
    if( !pImp )
        pImp = new SfxImpStringList;
    DBG_ASSERT(pImp->nRefCount!=0xffff,"ImpList not valid");
    return pImp->aList;
}

const std::vector<String>& SfxStringListItem::GetList () const
{
    return SAL_CONST_CAST(SfxStringListItem *, this)->GetList();
}

//------------------------------------------------------------------------

int SfxStringListItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );

    SfxStringListItem* pItem = (SfxStringListItem*)&rItem;

    return pImp == pItem->pImp;
}

//------------------------------------------------------------------------

SfxItemPresentation SfxStringListItem::GetPresentation
(
    SfxItemPresentation     /*ePresentation*/,
    SfxMapUnit              /*eCoreMetric*/,
    SfxMapUnit              /*ePresentationMetric*/,
    XubString&              rText,
    const IntlWrapper *
)   const
{
    rText.AssignAscii(RTL_CONSTASCII_STRINGPARAM("(List)"));
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

SfxPoolItem* SfxStringListItem::Clone( SfxItemPool *) const
{
    return new SfxStringListItem( *this );
    /*
    if( pImp )
        return new SfxStringListItem( Which(), &(pImp->aList) );
    else
        return new SfxStringListItem( Which(), NULL );
    */

}

//------------------------------------------------------------------------

SfxPoolItem* SfxStringListItem::Create( SvStream & rStream, sal_uInt16 ) const
{
    return new SfxStringListItem( Which(), rStream );
}

//------------------------------------------------------------------------

SvStream& SfxStringListItem::Store( SvStream & rStream, sal_uInt16 ) const
{
    if( !pImp )
    {
        rStream << 0L;
        return rStream;
    }

    DBG_ASSERT(pImp->nRefCount!=0xffff,"ImpList not valid");

    sal_uInt32 nCount = pImp->aList.size();
    rStream << nCount;

    for( sal_uInt32 i=0; i < nCount; i++ )
        writeByteString(rStream, pImp->aList[i]);

    return rStream;
}

//------------------------------------------------------------------------

void SfxStringListItem::SetString( const XubString& rStr )
{
    DBG_ASSERT(GetRefCount()==0,"SetString:RefCount!=0");

    if ( pImp && (pImp->nRefCount == 1) )
        delete pImp;
    else
    if( pImp )
        pImp->nRefCount--;
    pImp = new SfxImpStringList;

    xub_StrLen nStart = 0;
    xub_StrLen nDelimPos;
    XubString aStr(rStr);
    aStr.ConvertLineEnd(LINEEND_CR);
    do
    {
        nDelimPos = aStr.Search( _CR, nStart );
        xub_StrLen nLen;
        if ( nDelimPos == STRING_NOTFOUND )
            nLen = 0xffff;
        else
            nLen = nDelimPos - nStart;

        // String gehoert der Liste
        pImp->aList.push_back(aStr.Copy(nStart, nLen));

        nStart += nLen + 1 ;    // delimiter ueberspringen
    } while( nDelimPos != STRING_NOTFOUND );

    // Kein Leerstring am Ende
    if (!pImp->aList.empty() && !(pImp->aList.rbegin())->Len())
        pImp->aList.pop_back();
}

//------------------------------------------------------------------------

XubString SfxStringListItem::GetString()
{
    XubString aStr;
    if ( pImp )
    {
        DBG_ASSERT(pImp->nRefCount!=0xffff,"ImpList not valid");

        std::vector<String>::iterator iter;
        for (iter = pImp->aList.begin();;)
        {
            aStr += *iter;
            ++iter;

            if (iter != pImp->aList.end())
                aStr += '\r';
            else
                break;
        }
    }
    aStr.ConvertLineEnd();
    return aStr;
}

//------------------------------------------------------------------------

#ifndef TF_POOLABLE

int SfxStringListItem::IsPoolable() const
{
    return sal_False;
}

#endif

//------------------------------------------------------------------------

void SfxStringListItem::Sort( sal_Bool bAscending)
{
    DBG_ASSERT(GetRefCount()==0,"Sort:RefCount!=0");
    if( pImp )
        pImp->Sort( bAscending);
}

//----------------------------------------------------------------------------
void SfxStringListItem::SetStringList( const com::sun::star::uno::Sequence< rtl::OUString >& rList )
{
    DBG_ASSERT(GetRefCount()==0,"SetString:RefCount!=0");

    if ( pImp && (pImp->nRefCount == 1) )
        delete pImp;
    else
    if( pImp )
        pImp->nRefCount--;
    pImp = new SfxImpStringList;

    if (pImp)
    {
        // String gehoert der Liste
        for ( sal_Int32 n = 0; n < rList.getLength(); n++ )
            pImp->aList.push_back(XubString(rList[n]));
    }
}

//----------------------------------------------------------------------------
void SfxStringListItem::GetStringList( com::sun::star::uno::Sequence< rtl::OUString >& rList ) const
{
    long nCount = pImp->aList.size();

    rList.realloc( nCount );
    for( long i=0; i < nCount; i++ )
        rList[i] = pImp->aList[i];
}

//----------------------------------------------------------------------------
// virtual
bool SfxStringListItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 )
{
    com::sun::star::uno::Sequence< rtl::OUString > aValue;
    if ( rVal >>= aValue )
    {
        SetStringList( aValue );
        return true;
    }

    OSL_FAIL( "SfxStringListItem::PutValue - Wrong type!" );
    return false;
}

//----------------------------------------------------------------------------
// virtual
bool SfxStringListItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 ) const
{
    // GetString() is not const!!!
    SfxStringListItem* pThis = const_cast< SfxStringListItem * >( this );

    com::sun::star::uno::Sequence< rtl::OUString > aStringList;
    pThis->GetStringList( aStringList );
    rVal = ::com::sun::star::uno::makeAny( aStringList );
    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
