/*************************************************************************
 *
 *  $RCSfile: paralist.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:23 $
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

#pragma hdrstop

#include <paralist.hxx>
#include <outliner.hxx>     // nur wegen Paragraph, muss geaendert werden!

DBG_NAME(Paragraph);

Paragraph::Paragraph( USHORT nDDepth )
    : aBulSize( -1, -1)
{
    DBG_CTOR( Paragraph, 0 );

    nDepth = nDDepth;
    nFlags = 0;
    bVisible = TRUE;
}

Paragraph::Paragraph( const Paragraph& rPara )
    : aBulSize( rPara.aBulSize ), aBulText( rPara.aBulText )
{
    DBG_CTOR( Paragraph, 0 );

    nDepth = rPara.nDepth;
    nFlags = rPara.nFlags;
    bVisible = rPara.bVisible;
}

Paragraph::~Paragraph()
{
    DBG_DTOR( Paragraph, 0 );
}

void ParagraphList::Clear( BOOL bDestroyParagraphs )
{
    if ( bDestroyParagraphs )
    {
        for ( ULONG n = GetParagraphCount(); n; )
        {
            Paragraph* pPara = GetParagraph( --n );
            delete pPara;
        }
    }
    List::Clear();
}

void ParagraphList::MoveParagraphs( ULONG nStart, ULONG nDest, ULONG nCount )
{
    if ( ( nDest < nStart ) || ( nDest >= ( nStart + nCount ) ) )
    {
        ULONG n;
        ParagraphList aParas;
        for ( n = 0; n < nCount; n++ )
        {
            Paragraph* pPara = GetParagraph( nStart );
            aParas.Insert( pPara, LIST_APPEND );
            Remove( nStart );
        }

        if ( nDest > nStart )
            nDest -= nCount;

        for ( n = 0; n < nCount; n++ )
        {
            Paragraph* pPara = aParas.GetParagraph( n );
            Insert( pPara, nDest++ );
        }
    }
    else
        DBG_ERROR( "MoveParagraphs: Invalid Parameters" );
}

Paragraph* ParagraphList::NextVisible( Paragraph* pPara ) const
{
    ULONG n = GetAbsPos( pPara );

    Paragraph* p = GetParagraph( ++n );
    while ( p && !p->IsVisible() )
        p = GetParagraph( ++n );

    return p;
}

Paragraph* ParagraphList::PrevVisible( Paragraph* pPara ) const
{
    ULONG n = GetAbsPos( pPara );

    Paragraph* p = n ? GetParagraph( --n ) : NULL;
    while ( p && !p->IsVisible() )
        p = n ? GetParagraph( --n ) : NULL;

    return p;
}

Paragraph* ParagraphList::LastVisible() const
{
    ULONG n = GetParagraphCount();

    Paragraph* p = n ? GetParagraph( --n ) : NULL;
    while ( p && !p->IsVisible() )
        p = n ? GetParagraph( --n ) : NULL;

    return p;
}

BOOL ParagraphList::HasChilds( Paragraph* pParagraph ) const
{
    ULONG n = GetAbsPos( pParagraph );
    Paragraph* pNext = GetParagraph( ++n );
    return ( pNext && ( pNext->GetDepth() > pParagraph->GetDepth() ) ) ? TRUE : FALSE;
}

BOOL ParagraphList::HasHiddenChilds( Paragraph* pParagraph ) const
{
    ULONG n = GetAbsPos( pParagraph );
    Paragraph* pNext = GetParagraph( ++n );
    return ( pNext && ( pNext->GetDepth() > pParagraph->GetDepth() ) && !pNext->IsVisible() ) ? TRUE : FALSE;
}

BOOL ParagraphList::HasVisibleChilds( Paragraph* pParagraph ) const
{
    ULONG n = GetAbsPos( pParagraph );
    Paragraph* pNext = GetParagraph( ++n );
    return ( pNext && ( pNext->GetDepth() > pParagraph->GetDepth() ) && pNext->IsVisible() ) ? TRUE : FALSE;
}

ULONG ParagraphList::GetChildCount( Paragraph* pParent ) const
{
    ULONG nChildCount = 0;
    ULONG n = GetAbsPos( pParent );
    Paragraph* pPara = GetParagraph( ++n );
    while ( pPara && ( pPara->GetDepth() > pParent->GetDepth() ) )
    {
        nChildCount++;
        pPara = GetParagraph( ++n );
    }
    return nChildCount;
}

Paragraph* ParagraphList::GetParent( Paragraph* pParagraph, USHORT& rRelPos ) const
{
    rRelPos = 0;
    ULONG n = GetAbsPos( pParagraph );
    Paragraph* pPrev = GetParagraph( --n );
    while ( pPrev && ( pPrev->GetDepth() >= pParagraph->GetDepth() ) )
    {
        if ( pPrev->GetDepth() == pParagraph->GetDepth() )
            rRelPos++;
        pPrev = GetParagraph( --n );
    }

    return pPrev;
}

void ParagraphList::Expand( Paragraph* pParent )
{
    ULONG nChildCount = GetChildCount( pParent );
    ULONG nPos = GetAbsPos( pParent );

    for ( ULONG n = 1; n <= nChildCount; n++  )
    {
        Paragraph* pPara = GetParagraph( nPos+n );
        if ( !( pPara->IsVisible() ) )
        {
            pPara->bVisible = TRUE;
            aVisibleStateChangedHdl.Call( pPara );
        }
    }
}

void ParagraphList::Collapse( Paragraph* pParent )
{
    ULONG nChildCount = GetChildCount( pParent );
    ULONG nPos = GetAbsPos( pParent );

    for ( ULONG n = 1; n <= nChildCount; n++  )
    {
        Paragraph* pPara = GetParagraph( nPos+n );
        if ( pPara->IsVisible() )
        {
            pPara->bVisible = FALSE;
            aVisibleStateChangedHdl.Call( pPara );
        }
    }
}

ULONG ParagraphList::GetVisPos( Paragraph* pPara )
{
    ULONG nVisPos = 0;
    ULONG nPos = GetAbsPos( pPara );
    for ( ULONG n = 0; n < nPos; n++ )
    {
        Paragraph* pPara = GetParagraph( n );
        if ( pPara->IsVisible() )
            nVisPos++;
    }
    return nVisPos;
}
