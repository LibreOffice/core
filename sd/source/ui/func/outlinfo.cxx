/*************************************************************************
 *
 *  $RCSfile: outlinfo.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: aw $ $Date: 2002-08-01 15:01:05 $
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
#include <algorithm>

// #101500#
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif

#ifndef _COM_SUN_STAR_I18N_XBREAKITERATOR_HPP_
#include <com/sun/star/i18n/XBreakIterator.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_CHARACTERITERATORMODE_HDL_
#include <com/sun/star/i18n/CharacterIteratorMode.hdl>
#endif

#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;

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

/*
IMPL_LINK(OutlinerInfo, DrawPortionHdl, DrawPortionInfo*, pInfo)
{
    // #101500#
    Point aStart;
    sal_Bool bIsVertical(IsVertical());
    sal_Bool bUseBreakIterator(sal_False);

    pOut->SetFont((const Font&)pInfo->rFont);

    FontMetric aFontMetric(pOut->GetFontMetric());

    // initialize BreakIterator
    Reference < com::sun::star::i18n::XBreakIterator > xBreak;
    Reference < XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    Reference < XInterface > xInterface = xMSF->createInstance(::rtl::OUString::createFromAscii("com.sun.star.i18n.BreakIterator"));
    ::com::sun::star::lang::Locale aFontLocale = SvxCreateLocale(pInfo->rFont.GetLanguage());

    if(xInterface.is())
    {
        Any x = xInterface->queryInterface(::getCppuType((const Reference< XBreakIterator >*)0));
        x >>= xBreak;
    }

    if(xBreak.is())
    {
        bUseBreakIterator = sal_True;
    }

    if(bIsVertical)
    {
        aStart.X() = pInfo->rStartPos.X() + aTextOffset.X() - aFontMetric.GetDescent();
        aStart.Y() = pInfo->rStartPos.Y() + aTextOffset.Y();

        const Point aTopLeft(aStart.X(), aParaBound.Top());
        const Point aBottomRight(aStart.X() + aFontMetric.GetLineHeight(), aParaBound.Bottom());
        const Rectangle aCurRect(aTopLeft, aBottomRight);

        if(pInfo->nPara != nCurPara)
        {
            nCurPara = pInfo->nPara;
            pParagraphs[nCurPara].aRect = aCurRect;
        }
        else
        {
            pParagraphs[nCurPara].aRect.Union(aCurRect);
        }
    }
    else
    {
        aStart.X() = pInfo->rStartPos.X() + aTextOffset.X();
        aStart.Y() = pInfo->rStartPos.Y() + aTextOffset.Y() - aFontMetric.GetAscent();

        const Point aTopLeft(aParaBound.Left(), aStart.Y());
        const Point aBottomRight(aParaBound.Right(), aStart.Y() + aFontMetric.GetLineHeight());
        const Rectangle aCurRect(aTopLeft, aBottomRight);

        if(pInfo->nPara != nCurPara)
        {
            nCurPara = pInfo->nPara;
            pParagraphs[nCurPara].aRect = aCurRect;
        }
        else
        {
            pParagraphs[nCurPara].aRect.Union(aCurRect);
        }
    }

    if(pInfo->nTextLen && (0xFFFF != pInfo->nIndex))
    {
        pParagraphs[nCurPara].nCharCount += pInfo->nTextLen;

        for(sal_uInt16 nCharIndex(0); nCharIndex < pInfo->nTextLen;)
        {
            xub_StrLen nNextGlyphLen(1);
            sal_Bool bIsSingleSpace(sal_False);

            if(bUseBreakIterator)
            {
                sal_Int32 nDone(0L);
                nNextGlyphLen = (xub_StrLen)xBreak->nextCharacters( pInfo->rText, pInfo->nTextStart, aFontLocale,
                    CharacterIteratorMode::SKIPCELL, 1, nDone) - (pInfo->nTextStart);
            }

            if(!bIsSingleSpace)
            {
                Rectangle aTextRect;
                sal_Bool bDidWork = pOut->GetTextBoundRect(aTextRect, pInfo->rText,
                    pInfo->nTextStart, pInfo->nTextStart + nCharIndex, nNextGlyphLen);

                if(bDidWork)
                {
//                  if(bIsVertical)
//                      aTextRect.SetPos(aStart + Point(-aFontMetric.GetDescent(), 0));
//                  else
//                      aTextRect.SetPos(aStart + Point(0, aFontMetric.GetAscent()));
                    aTextRect.Move(0, aFontMetric.GetAscent());
                    aTextRect.Move(aStart.X(), aStart.Y());

                    aCharacterList.Insert(
                        new OutlinerCharacter(
                            aTextRect,
                            pInfo->nPara,
                            pInfo->rFont.GetColor()
                            // #101500# CharCode is no longer used.
                            //,pInfo->rText.GetChar(nCharIndex + pInfo->nTextStart)
                            ),
                        LIST_APPEND);
                }
            }

            // use glyph lengh for loop
            nCharIndex += nNextGlyphLen;
        }
    }

    return 0L;
}
*/

IMPL_LINK(OutlinerInfo, DrawPortionHdl, DrawPortionInfo*, pInfo)
{
    // #101500#
    Point aStart;
    sal_Bool bIsVertical(IsVertical());
    pOut->SetFont((const Font&)pInfo->rFont);
    FontMetric aFontMetric(pOut->GetFontMetric());
    sal_Bool bUseBreakIterator(sal_False);

    // initialize BreakIterator
    Reference < com::sun::star::i18n::XBreakIterator > xBreak;
    Reference < XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    Reference < XInterface > xInterface = xMSF->createInstance(::rtl::OUString::createFromAscii("com.sun.star.i18n.BreakIterator"));
    ::com::sun::star::lang::Locale aFontLocale = SvxCreateLocale(pInfo->rFont.GetLanguage());

    if(xInterface.is())
    {
        Any x = xInterface->queryInterface(::getCppuType((const Reference< XBreakIterator >*)0));
        x >>= xBreak;
    }

    if(xBreak.is())
    {
        bUseBreakIterator = sal_True;
    }

    if(bIsVertical)
    {
        aStart.X() = pInfo->rStartPos.X() + aTextOffset.X() - aFontMetric.GetDescent();
        aStart.Y() = pInfo->rStartPos.Y() + aTextOffset.Y();

        const Point aTopLeft(aStart.X(), aParaBound.Top());
        const Point aBottomRight(aStart.X() + aFontMetric.GetLineHeight(), aParaBound.Bottom());
        const Rectangle aCurRect(aTopLeft, aBottomRight);

        if(pInfo->nPara != nCurPara)
        {
            nCurPara = pInfo->nPara;
            pParagraphs[nCurPara].aRect = aCurRect;
        }
        else
        {
            pParagraphs[nCurPara].aRect.Union(aCurRect);
        }
    }
    else
    {
        aStart.X() = pInfo->rStartPos.X() + aTextOffset.X();
        aStart.Y() = pInfo->rStartPos.Y() + aTextOffset.Y() - aFontMetric.GetAscent();

        const Point aTopLeft(aParaBound.Left(), aStart.Y());
        const Point aBottomRight(aParaBound.Right(), aStart.Y() + aFontMetric.GetLineHeight());
        const Rectangle aCurRect(aTopLeft, aBottomRight);

        if(pInfo->nPara != nCurPara)
        {
            nCurPara = pInfo->nPara;
            pParagraphs[nCurPara].aRect = aCurRect;
        }
        else
        {
            pParagraphs[nCurPara].aRect.Union(aCurRect);
        }
    }

    if(pInfo->nTextLen && (0xFFFF != pInfo->nIndex))
    {
        pParagraphs[nCurPara].nCharCount += pInfo->nTextLen;
        sal_uInt16 nInsertIndex(LIST_APPEND);

        if(pInfo->IsRTL())
            nInsertIndex = aCharacterList.Count();

        for(sal_uInt16 nCharIndex(0); nCharIndex < pInfo->nTextLen; )
        {
            xub_StrLen nNextGlyphLen(1);
            sal_Bool bIsSingleSpace(sal_False);

            if(bUseBreakIterator)
            {
                sal_Int32 nDone(0L);
                nNextGlyphLen = (xub_StrLen)xBreak->nextCharacters( pInfo->rText, pInfo->nTextStart, aFontLocale,
                    CharacterIteratorMode::SKIPCELL, 1, nDone) - (pInfo->nTextStart);
            }

            if(!bIsSingleSpace)
            {
                Size aGlyphSize(pInfo->rFont.GetPhysTxtSize(pOut,
                    pInfo->rText, nCharIndex + pInfo->nTextStart, nNextGlyphLen));

                if(bIsVertical)
                {
                    ::std::swap(aGlyphSize.Width(), aGlyphSize.Height());
                }

                const Rectangle aRect(aStart, aGlyphSize);

                aCharacterList.Insert(new OutlinerCharacter(
                    aRect,
                    pInfo->nPara,
                    pInfo->rFont.GetColor()),
                    nInsertIndex);

                if(bIsVertical)
                    aStart.Y() = pInfo->rStartPos.Y() + aTextOffset.Y() + (pInfo->pDXArray)[nCharIndex];
                else
                    aStart.X() = pInfo->rStartPos.X() + aTextOffset.X() + (pInfo->pDXArray)[nCharIndex];
            }

            nCharIndex += nNextGlyphLen;
        }
    }

    return 0L;
}

// eof
