/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: outlinfo.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:27:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    mpOut->SetFont((const Font&)pInfo->rFont);
    FontMetric aFontMetric(mpOut->GetFontMetric());
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
        sal_uInt16 nInsertIndex(0xffff);

        if(pInfo->IsRTL())
            nInsertIndex = (sal_uInt16)aCharacterList.Count();

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
                Size aGlyphSize(pInfo->rFont.GetPhysTxtSize(mpOut,
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

                long dx = 0;
                if( pInfo->pDXArray )
                    dx = (pInfo->pDXArray)[nCharIndex];

                if(bIsVertical)
                    aStart.Y() = pInfo->rStartPos.Y() + aTextOffset.Y() + dx;
                else
                    aStart.X() = pInfo->rStartPos.X() + aTextOffset.X() + dx;
            }

            nCharIndex = nCharIndex + nNextGlyphLen;
        }
    }

    return 0L;
}

// eof
