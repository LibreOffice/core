/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: outlinfo.cxx,v $
 * $Revision: 1.16 $
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
#include "precompiled_sd.hxx"

#include <vcl/metric.hxx>
#include <svx/outliner.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svxfont.hxx>
#include "drawdoc.hxx"
#include "outlinfo.hxx"
#include <algorithm>

// #101500#
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <comphelper/processfactory.hxx>

#ifndef _COM_SUN_STAR_I18N_CHARACTERITERATORMODE_HDL_
#include <com/sun/star/i18n/CharacterIteratorMode.hdl>
#endif
#include <svx/unolingu.hxx>

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

void OutlinerInfo::SetTextObj( SdDrawDocument* pDoc, SdrRectObj* pObj, OutputDevice* pOut )
{
    Clear();

    SdrOutliner& rOutliner = pDoc->GetDrawOutliner();

    mpOut = pOut;
    rOutliner.SetText( *pObj->GetOutlinerParaObject() );

    aObjBound = pObj->GetCurrentBoundRect();
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

            for( USHORT i = 0; i < nParaCount; i++ )
            {
                if( i > 0 )
                    pParagraphs[i].aRect.Right() = pParagraphs[ i - 1 ].aRect.Left();

                pParagraphs[i].aRect.Left() = pParagraphs[i].aRect.Right() - rOutliner.GetTextHeight( i );
            }
        }
        else
        {
            pParagraphs[ 0 ].aRect.Top() = aParaBound.Top();

            for( USHORT i = 0; i < nParaCount; i++ )
            {
                if( i > 0 )
                    pParagraphs[ i ].aRect.Top() = pParagraphs[ i - 1 ].aRect.Bottom();

                pParagraphs[ i ].aRect.Bottom() = pParagraphs[ i ].aRect.Top() + rOutliner.GetTextHeight( i );
            }
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

IMPL_LINK(OutlinerInfo, DrawPortionHdl, DrawPortionInfo*, pInfo)
{
    // #101500#
    Point aStart;
    sal_Bool bIsVertical(IsVertical());
    mpOut->SetFont((const Font&)pInfo->mrFont);
    FontMetric aFontMetric(mpOut->GetFontMetric());
    sal_Bool bUseBreakIterator(sal_False);

    // initialize BreakIterator
    Reference < com::sun::star::i18n::XBreakIterator > xBreak;
    Reference < XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    Reference < XInterface > xInterface = xMSF->createInstance(::rtl::OUString::createFromAscii("com.sun.star.i18n.BreakIterator"));
    ::com::sun::star::lang::Locale aFontLocale = SvxCreateLocale(pInfo->mrFont.GetLanguage());

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
        aStart.X() = pInfo->mrStartPos.X() + aTextOffset.X() - aFontMetric.GetDescent();
        aStart.Y() = pInfo->mrStartPos.Y() + aTextOffset.Y();

        const Point aTopLeft(aStart.X(), aParaBound.Top());
        const Point aBottomRight(aStart.X() + aFontMetric.GetLineHeight(), aParaBound.Bottom());
        const Rectangle aCurRect(aTopLeft, aBottomRight);

        if(pInfo->mnPara != nCurPara)
        {
            nCurPara = pInfo->mnPara;
            pParagraphs[nCurPara].aRect = aCurRect;
        }
        else
        {
            pParagraphs[nCurPara].aRect.Union(aCurRect);
        }
    }
    else
    {
        aStart.X() = pInfo->mrStartPos.X() + aTextOffset.X();
        aStart.Y() = pInfo->mrStartPos.Y() + aTextOffset.Y() - aFontMetric.GetAscent();

        const Point aTopLeft(aParaBound.Left(), aStart.Y());
        const Point aBottomRight(aParaBound.Right(), aStart.Y() + aFontMetric.GetLineHeight());
        const Rectangle aCurRect(aTopLeft, aBottomRight);

        if(pInfo->mnPara != nCurPara)
        {
            nCurPara = pInfo->mnPara;
            pParagraphs[nCurPara].aRect = aCurRect;
        }
        else
        {
            pParagraphs[nCurPara].aRect.Union(aCurRect);
        }
    }

    if(pInfo->mnTextLen && (0xFFFF != pInfo->mnIndex))
    {
        pParagraphs[nCurPara].nCharCount += pInfo->mnTextLen;
        sal_uInt16 nInsertIndex(0xffff);

        if(pInfo->IsRTL())
            nInsertIndex = (sal_uInt16)aCharacterList.Count();

        for(sal_uInt16 nCharIndex(0); nCharIndex < pInfo->mnTextLen; )
        {
            xub_StrLen nNextGlyphLen(1);
            sal_Bool bIsSingleSpace(sal_False);

            if(bUseBreakIterator)
            {
                sal_Int32 nDone(0L);
                nNextGlyphLen = (xub_StrLen)xBreak->nextCharacters( pInfo->mrText, pInfo->mnTextStart, aFontLocale,
                    CharacterIteratorMode::SKIPCELL, 1, nDone) - (pInfo->mnTextStart);
            }

            if(!bIsSingleSpace)
            {
                Size aGlyphSize(pInfo->mrFont.GetPhysTxtSize(mpOut,
                    pInfo->mrText, nCharIndex + pInfo->mnTextStart, nNextGlyphLen));

                if(bIsVertical)
                {
                    ::std::swap(aGlyphSize.Width(), aGlyphSize.Height());
                }

                const Rectangle aRect(aStart, aGlyphSize);

                aCharacterList.Insert(new OutlinerCharacter(
                    aRect,
                    pInfo->mnPara,
                    pInfo->mrFont.GetColor()),
                    nInsertIndex);

                long dx = 0;
                if( pInfo->mpDXArray )
                    dx = (pInfo->mpDXArray)[nCharIndex];

                if(bIsVertical)
                    aStart.Y() = pInfo->mrStartPos.Y() + aTextOffset.Y() + dx;
                else
                    aStart.X() = pInfo->mrStartPos.X() + aTextOffset.X() + dx;
            }

            nCharIndex = nCharIndex + nNextGlyphLen;
        }
    }

    return 0L;
}

// eof
