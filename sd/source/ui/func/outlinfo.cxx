/*************************************************************************
 *
 *  $RCSfile: outlinfo.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ka $ $Date: 2002-04-04 16:14:24 $
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

#include <vcl/metric.hxx>
#include <svx/outliner.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svxfont.hxx>
#include "drawdoc.hxx"
#include "outlinfo.hxx"

// ----------------
// - OutlinerInfo -
// ----------------

OutlinerInfo::OutlinerInfo() :
            pParagraphs ( NULL ),
            nParaCount  ( 0UL ),
            nExtraData  ( 0L )
{
}

// -----------------------------------------------------------------------------

OutlinerInfo::~OutlinerInfo()
{
    Clear();
}

// -----------------------------------------------------------------------------

void OutlinerInfo::SetTextObj( SdDrawDocument* pDoc, SdrRectObj* pObj, OutputDevice* _pOut )
{
    Clear();

    SdrOutliner& rOutliner = pDoc->GetDrawOutliner();

    pOut = _pOut;
    rOutliner.SetText( *pObj->GetOutlinerParaObject() );

    aObjBound = pObj->GetBoundRect();
    nParaCount = rOutliner.GetParagraphCount();
    nExtraData = 0L;

    if( nParaCount )
    {
        mbVertical = rOutliner.IsVertical();
        pObj->TakeTextRect( rOutliner, aParaBound, TRUE );

        if( IsVertical() )
            aTextOffset = aParaBound.TopRight();
        else
            aTextOffset = aParaBound.TopLeft();

        nCurPara = 0;
        bInit = TRUE;

        rOutliner.SetDrawPortionHdl( LINK( this, OutlinerInfo, DrawPortionHdl ) );
        pParagraphs = new OutlinerParagraph[ nParaCount ];
        rOutliner.StripPortions();
        rOutliner.SetDrawPortionHdl( Link() );

        if( 1 == nParaCount )
            pParagraphs[ 0 ].aRect = aParaBound;
        else if( IsVertical() )
        {
            pParagraphs[ 0 ].aRect.Right() = aParaBound.Right();

            for( USHORT i = 0; i < ( nParaCount - 1 ); i++ )
                if( pParagraphs[ i ].aRect.Left() > pParagraphs[ i + 1 ].aRect.Right() )
                    pParagraphs[ i ].aRect.Left() = pParagraphs[ i + 1 ].aRect.Right();

            pParagraphs[ nParaCount - 1 ].aRect.Left() = aParaBound.Left();
        }
        else
        {
            pParagraphs[ 0 ].aRect.Top() = aParaBound.Top();

            for( USHORT i = 0; i < ( nParaCount - 1 ); i++ )
                if( pParagraphs[ i ].aRect.Bottom() < pParagraphs[ i + 1 ].aRect.Top() )
                    pParagraphs[ i ].aRect.Bottom() = pParagraphs[ i + 1 ].aRect.Top();

            pParagraphs[ nParaCount - 1 ].aRect.Bottom() = aParaBound.Bottom();
        }
    }
    else
    {
        pParagraphs = NULL;
        aParaBound = Rectangle();
        aTextOffset = Point();
    }

    nCurPara = 0;
    bInit = FALSE;
}

// -----------------------------------------------------------------------------

void OutlinerInfo::Clear()
{
    for( void* pChar = aCharacterList.First(); pChar; pChar = aCharacterList.Next() )
        delete (OutlinerCharacter*) pChar;
    aCharacterList.Clear();

    delete[] pParagraphs;
    pParagraphs = NULL;

    nCurPara = nParaCount = 0UL;
    aObjBound = aParaBound = Rectangle();
    aTextOffset = Point();
}

// -----------------------------------------------------------------------------

const Rectangle& OutlinerInfo::GetParaRect( const ULONG nPara ) const
{
    DBG_ASSERT( nPara < nParaCount, "Para out of range!" );
    return pParagraphs[ nPara ].aRect;
}

// -----------------------------------------------------------------------------

BOOL OutlinerInfo::GetParaCharCount( const ULONG nPara ) const
{
    DBG_ASSERT( nPara < nParaCount, "Para out of range!" );
    return (0 != pParagraphs[ nPara ].nCharCount);
}

// -----------------------------------------------------------------------------

IMPL_LINK( OutlinerInfo, DrawPortionHdl, DrawPortionInfo*, pDInfo )
{
    const USHORT    nCharCount = pDInfo->rText.Len();
    Point           aStart;

    pOut->SetFont( (const Font&) pDInfo->rFont );

    if( IsVertical() )
    {
        aStart.X() = pDInfo->rStartPos.X() + aTextOffset.X() - pOut->GetFontMetric().GetDescent();
        aStart.Y() = pDInfo->rStartPos.Y() + aTextOffset.Y();

        const Point     aTopLeft( aStart.X(), aParaBound.Top() );
        const Point     aBottomRight( aStart.X() + pOut->GetFontMetric().GetLineHeight(), aParaBound.Bottom() );
        const Rectangle aCurRect( aTopLeft, aBottomRight );

        if( pDInfo->nPara != nCurPara )
            pParagraphs[ nCurPara = pDInfo->nPara ].aRect = aCurRect;
        else
            pParagraphs[ nCurPara ].aRect.Union( aCurRect );
    }
    else
    {
        aStart.X() = pDInfo->rStartPos.X() + aTextOffset.X();
        aStart.Y() = pDInfo->rStartPos.Y() + aTextOffset.Y() - pOut->GetFontMetric().GetAscent();

        const Point     aTopLeft( aParaBound.Left(), aStart.Y() );
        const Point     aBottomRight( aParaBound.Right(), aStart.Y() + pOut->GetFontMetric().GetLineHeight() );
        const Rectangle aCurRect( aTopLeft, aBottomRight );

        if( pDInfo->nPara != nCurPara )
            pParagraphs[ nCurPara = pDInfo->nPara ].aRect = aCurRect;
        else
            pParagraphs[ nCurPara ].aRect.Union( aCurRect );
    }

    if( nCharCount && ( pDInfo->nIndex != 0xFFFF ) )
    {
        pParagraphs[ nCurPara ].nCharCount += nCharCount;

        for( USHORT nCharIndex = 0; nCharIndex < nCharCount; nCharIndex++ )
        {
            Size aCharSize( pDInfo->rFont.GetPhysTxtSize( pOut, pDInfo->rText, nCharIndex, 1 ) );

            if( IsVertical() )
                ::std::swap( aCharSize.Width(), aCharSize.Height() );

            const Rectangle aRect( aStart, aCharSize );
            aCharacterList.Insert( new OutlinerCharacter( aRect, pDInfo->nPara,
                                                          pDInfo->rFont.GetColor(),
                                                          pDInfo->rText.GetChar( nCharIndex ) ), LIST_APPEND );

            if( nCharIndex < nCharCount - 1 )
            {
                if( IsVertical() )
                    aStart.Y() = pDInfo->rStartPos.Y() + aTextOffset.Y() + ( pDInfo->pDXArray )[ nCharIndex ];
                else
                    aStart.X() = pDInfo->rStartPos.X() + aTextOffset.X() + ( pDInfo->pDXArray )[ nCharIndex ];
            }
        }
    }

    return 0L;
}
