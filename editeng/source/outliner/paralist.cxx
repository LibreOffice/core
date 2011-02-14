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
#include "precompiled_editeng.hxx"

#include <paralist.hxx>
#include <editeng/outliner.hxx>     // nur wegen Paragraph, muss geaendert werden!
#include <editeng/numdef.hxx>

DBG_NAME(Paragraph)

ParagraphData::ParagraphData()
: nDepth( -1 )
, mnNumberingStartValue( -1 )
, mbParaIsNumberingRestart( sal_False )
{
}

ParagraphData::ParagraphData( const ParagraphData& r )
: nDepth( r.nDepth )
, mnNumberingStartValue( r.mnNumberingStartValue )
, mbParaIsNumberingRestart( r.mbParaIsNumberingRestart )
{
}

ParagraphData& ParagraphData::operator=( const ParagraphData& r)
{
    nDepth = r.nDepth;
    mnNumberingStartValue = r.mnNumberingStartValue;
    mbParaIsNumberingRestart = r.mbParaIsNumberingRestart;
    return *this;
}

bool ParagraphData::operator==(const ParagraphData& rCandidate) const
{
    return (nDepth == rCandidate.nDepth
        && mnNumberingStartValue == rCandidate.mnNumberingStartValue
        && mbParaIsNumberingRestart == rCandidate.mbParaIsNumberingRestart);
}

Paragraph::Paragraph( sal_Int16 nDDepth )
: aBulSize( -1, -1)
{
    DBG_CTOR( Paragraph, 0 );

    DBG_ASSERT(  ( nDDepth >= -1 ) && ( nDDepth < SVX_MAX_NUM ), "Paragraph-CTOR: nDepth invalid!" );

    nDepth = nDDepth;
    nFlags = 0;
    bVisible = sal_True;
}

Paragraph::Paragraph( const Paragraph& rPara )
: ParagraphData( rPara )
, aBulText( rPara.aBulText )
, aBulSize( rPara.aBulSize )
{
    DBG_CTOR( Paragraph, 0 );

    nDepth = rPara.nDepth;
    nFlags = rPara.nFlags;
    bVisible = rPara.bVisible;
}

Paragraph::Paragraph( const ParagraphData& rData )
: nFlags( 0 )
, aBulSize( -1, -1)
, bVisible( sal_True )
{
    DBG_CTOR( Paragraph, 0 );

    nDepth = rData.nDepth;
    mnNumberingStartValue = rData.mnNumberingStartValue;
    mbParaIsNumberingRestart = rData.mbParaIsNumberingRestart;
}

Paragraph::~Paragraph()
{
    DBG_DTOR( Paragraph, 0 );
}

void Paragraph::SetNumberingStartValue( sal_Int16 nNumberingStartValue )
{
    mnNumberingStartValue = nNumberingStartValue;
    if( mnNumberingStartValue != -1 )
        mbParaIsNumberingRestart = true;
}

void Paragraph::SetParaIsNumberingRestart( sal_Bool bParaIsNumberingRestart )
{
    mbParaIsNumberingRestart = bParaIsNumberingRestart;
    if( !mbParaIsNumberingRestart )
        mnNumberingStartValue = -1;
}

void ParagraphList::Clear( sal_Bool bDestroyParagraphs )
{
    if ( bDestroyParagraphs )
    {
        for ( sal_uLong n = GetParagraphCount(); n; )
        {
            Paragraph* pPara = GetParagraph( --n );
            delete pPara;
        }
    }
    List::Clear();
}

void ParagraphList::MoveParagraphs( sal_uLong nStart, sal_uLong nDest, sal_uLong _nCount )
{
    if ( ( nDest < nStart ) || ( nDest >= ( nStart + _nCount ) ) )
    {
        sal_uLong n;
        ParagraphList aParas;
        for ( n = 0; n < _nCount; n++ )
        {
            Paragraph* pPara = GetParagraph( nStart );
            aParas.Insert( pPara, LIST_APPEND );
            Remove( nStart );
        }

        if ( nDest > nStart )
            nDest -= _nCount;

        for ( n = 0; n < _nCount; n++ )
        {
            Paragraph* pPara = aParas.GetParagraph( n );
            Insert( pPara, nDest++ );
        }
    }
    else
    {
        DBG_ERROR( "MoveParagraphs: Invalid Parameters" );
    }
}

Paragraph* ParagraphList::NextVisible( Paragraph* pPara ) const
{
    sal_uLong n = GetAbsPos( pPara );

    Paragraph* p = GetParagraph( ++n );
    while ( p && !p->IsVisible() )
        p = GetParagraph( ++n );

    return p;
}

Paragraph* ParagraphList::PrevVisible( Paragraph* pPara ) const
{
    sal_uLong n = GetAbsPos( pPara );

    Paragraph* p = n ? GetParagraph( --n ) : NULL;
    while ( p && !p->IsVisible() )
        p = n ? GetParagraph( --n ) : NULL;

    return p;
}

Paragraph* ParagraphList::LastVisible() const
{
    sal_uLong n = GetParagraphCount();

    Paragraph* p = n ? GetParagraph( --n ) : NULL;
    while ( p && !p->IsVisible() )
        p = n ? GetParagraph( --n ) : NULL;

    return p;
}

sal_Bool ParagraphList::HasChilds( Paragraph* pParagraph ) const
{
    sal_uLong n = GetAbsPos( pParagraph );
    Paragraph* pNext = GetParagraph( ++n );
    return ( pNext && ( pNext->GetDepth() > pParagraph->GetDepth() ) ) ? sal_True : sal_False;
}

sal_Bool ParagraphList::HasHiddenChilds( Paragraph* pParagraph ) const
{
    sal_uLong n = GetAbsPos( pParagraph );
    Paragraph* pNext = GetParagraph( ++n );
    return ( pNext && ( pNext->GetDepth() > pParagraph->GetDepth() ) && !pNext->IsVisible() ) ? sal_True : sal_False;
}

sal_Bool ParagraphList::HasVisibleChilds( Paragraph* pParagraph ) const
{
    sal_uLong n = GetAbsPos( pParagraph );
    Paragraph* pNext = GetParagraph( ++n );
    return ( pNext && ( pNext->GetDepth() > pParagraph->GetDepth() ) && pNext->IsVisible() ) ? sal_True : sal_False;
}

sal_uLong ParagraphList::GetChildCount( Paragraph* pParent ) const
{
    sal_uLong nChildCount = 0;
    sal_uLong n = GetAbsPos( pParent );
    Paragraph* pPara = GetParagraph( ++n );
    while ( pPara && ( pPara->GetDepth() > pParent->GetDepth() ) )
    {
        nChildCount++;
        pPara = GetParagraph( ++n );
    }
    return nChildCount;
}

Paragraph* ParagraphList::GetParent( Paragraph* pParagraph /*, sal_uInt16& rRelPos */ ) const
{
    /* rRelPos = 0 */;
    sal_uLong n = GetAbsPos( pParagraph );
    Paragraph* pPrev = GetParagraph( --n );
    while ( pPrev && ( pPrev->GetDepth() >= pParagraph->GetDepth() ) )
    {
//      if ( pPrev->GetDepth() == pParagraph->GetDepth() )
//          rRelPos++;
        pPrev = GetParagraph( --n );
    }

    return pPrev;
}

void ParagraphList::Expand( Paragraph* pParent )
{
    sal_uLong nChildCount = GetChildCount( pParent );
    sal_uLong nPos = GetAbsPos( pParent );

    for ( sal_uLong n = 1; n <= nChildCount; n++  )
    {
        Paragraph* pPara = GetParagraph( nPos+n );
        if ( !( pPara->IsVisible() ) )
        {
            pPara->bVisible = sal_True;
            aVisibleStateChangedHdl.Call( pPara );
        }
    }
}

void ParagraphList::Collapse( Paragraph* pParent )
{
    sal_uLong nChildCount = GetChildCount( pParent );
    sal_uLong nPos = GetAbsPos( pParent );

    for ( sal_uLong n = 1; n <= nChildCount; n++  )
    {
        Paragraph* pPara = GetParagraph( nPos+n );
        if ( pPara->IsVisible() )
        {
            pPara->bVisible = sal_False;
            aVisibleStateChangedHdl.Call( pPara );
        }
    }
}

sal_uLong ParagraphList::GetVisPos( Paragraph* pPara )
{
    sal_uLong nVisPos = 0;
    sal_uLong nPos = GetAbsPos( pPara );
    for ( sal_uLong n = 0; n < nPos; n++ )
    {
        Paragraph* _pPara = GetParagraph( n );
        if ( _pPara->IsVisible() )
            nVisPos++;
    }
    return nVisPos;
}
