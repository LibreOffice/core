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
    List    aList;

            SfxImpStringList() { nRefCount = 1; }
            ~SfxImpStringList();
    void    Sort( sal_Bool bAscending, List* );
};

//------------------------------------------------------------------------

SfxImpStringList::~SfxImpStringList()
{
    DBG_ASSERT(nRefCount!=0xffff,"ImpList already deleted");
    String* pStr = (String*)aList.First();
    while( pStr )
    {
        delete pStr;
        pStr = (String*)aList.Next();
    }
    nRefCount = 0xffff;
}

//------------------------------------------------------------------------

void SfxImpStringList::Sort( sal_Bool bAscending, List* pParallelList )
{
    DBG_ASSERT(!pParallelList || pParallelList->Count() >= aList.Count(),"Sort:ParallelList too small");
    sal_uLong nCount = aList.Count();
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
                String* pStr1 = (String*)aList.GetObject( nCur );
                String* pStr2 = (String*)aList.GetObject( nCur+1 );
                // COMPARE_GREATER => pStr2 ist groesser als pStr1
                StringCompare eCompare = pStr1->CompareIgnoreCaseToAscii( *pStr2 ); //@@@
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
                    aList.Replace( pStr1, nCur + 1 );
                    aList.Replace( pStr2, nCur );
                    if( pParallelList )
                    {
                        void* p1 = pParallelList->GetObject( nCur );
                        void* p2 = pParallelList->GetObject( nCur + 1 );
                        pParallelList->Replace( p1, nCur + 1 );
                        pParallelList->Replace( p2, nCur );
                    }
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

SfxStringListItem::SfxStringListItem( sal_uInt16 which, const List* pList ) :
    SfxPoolItem( which ),
    pImp(NULL)
{
    // PB: das Putten einer leeren Liste funktionierte nicht,
    // deshalb habe ich hier die Abfrage nach dem Count auskommentiert
    if( pList /*!!! && pList->Count() */ )
    {
        pImp = new SfxImpStringList;

        long i, nCount = pList->Count();
        String  *pStr1, *pStr2;
        for( i=0; i < nCount; i++ )
        {
            pStr1 = (String*)pList->GetObject(i);
            pStr2 = new String( *pStr1 );
            pImp->aList.Insert( pStr2, LIST_APPEND );
        }
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

    long   i;
    String*  pStr;
    for( i=0; i < nEntryCount; i++ )
    {
        pStr = new String;
        readByteString(rStream, *pStr);
        pImp->aList.Insert( pStr, LIST_APPEND );
    }
}

//------------------------------------------------------------------------

SfxStringListItem::SfxStringListItem( const SfxStringListItem& rItem ) :
    SfxPoolItem( rItem ),
    pImp(NULL)
{
    pImp = rItem.pImp;

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

List* SfxStringListItem::GetList()
{
    if( !pImp )
        pImp = new SfxImpStringList;
    DBG_ASSERT(pImp->nRefCount!=0xffff,"ImpList not valid");
    return &(pImp->aList);
}

//------------------------------------------------------------------------

int SfxStringListItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );

    SfxStringListItem* pItem = (SfxStringListItem*)&rItem;

    if( pImp == pItem->pImp )
        return sal_True;
    else
        return sal_False;
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

    long nCount = pImp->aList.Count();
    rStream << nCount;

    long i;
    String* pStr;
    for( i=0; i < nCount; i++ )
    {
        pStr = (String*)(pImp->aList.GetObject( i ));
        writeByteString(rStream, *pStr);
    }

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

        XubString* pStr = new XubString(aStr.Copy(nStart, nLen));
        // String gehoert der Liste
        pImp->aList.Insert( pStr, LIST_APPEND );

        nStart += nLen + 1 ;    // delimiter ueberspringen
    } while( nDelimPos != STRING_NOTFOUND );

    // Kein Leerstring am Ende
    if( pImp->aList.Last() &&
        !((XubString*)pImp->aList.Last())->Len() )
        delete (XubString*)pImp->aList.Remove( pImp->aList.Count()-1 );
}

//------------------------------------------------------------------------

XubString SfxStringListItem::GetString()
{
    XubString aStr;
    if ( pImp )
    {
        DBG_ASSERT(pImp->nRefCount!=0xffff,"ImpList not valid");
        XubString* pStr = (XubString*)(pImp->aList.First());
        while( pStr )
        {
            aStr += *pStr;
            pStr = (XubString*)(pImp->aList.Next());
            if ( pStr )
                aStr += '\r';
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

void SfxStringListItem::Sort( sal_Bool bAscending, List* pParallelList )
{
    DBG_ASSERT(GetRefCount()==0,"Sort:RefCount!=0");
    if( pImp )
        pImp->Sort( bAscending, pParallelList );
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

    for ( sal_Int32 n = 0; n < rList.getLength(); n++ )
    {
        XubString* pStr = new XubString( rList[n] );
        // String gehoert der Liste
        pImp->aList.Insert( pStr, LIST_APPEND );
    }
}

//----------------------------------------------------------------------------
void SfxStringListItem::GetStringList( com::sun::star::uno::Sequence< rtl::OUString >& rList ) const
{
    long nCount = pImp->aList.Count();

    rList.realloc( nCount );
    for( long i=0; i < nCount; i++ )
        rList[i] = *(String*)(pImp->aList.GetObject( i ));
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
