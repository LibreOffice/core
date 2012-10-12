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

#include <hintids.hxx>
#include <vcl/metric.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <paratr.hxx>
#include <txtfrm.hxx>   // Format()
#include <charfmt.hxx>
#include <viewopt.hxx>  // SwViewOption
#include <viewsh.hxx>   // ViewShell
#include <pordrop.hxx>
#include <itrform2.hxx>
#include <txtpaint.hxx> // SwSaveClip
#include <blink.hxx>    // pBlink
#include <breakit.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <editeng/langitem.hxx>
#include <charatr.hxx>
#include <editeng/fhgtitem.hxx>
#include <switerator.hxx>

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star;

/*************************************************************************
 *                lcl_IsDropFlyInter
 *
 *  Calculates if a drop caps portion intersects with a fly
 *  The width and height of the drop caps portion are passed as arguments,
 *  the position is calculated from the values in rInf
 *************************************************************************/

static sal_Bool lcl_IsDropFlyInter( const SwTxtFormatInfo &rInf,
                             sal_uInt16 nWidth, sal_uInt16 nHeight )
{
    const SwTxtFly *pTxtFly = rInf.GetTxtFly();
    if( pTxtFly && pTxtFly->IsOn() )
    {
        SwRect aRect( rInf.GetTxtFrm()->Frm().Pos(), Size( nWidth, nHeight) );
        aRect.Pos() += rInf.GetTxtFrm()->Prt().Pos();
        aRect.Pos().X() += rInf.X();
        aRect.Pos().Y() = rInf.Y();
        aRect = pTxtFly->GetFrm( aRect );
        return aRect.HasArea();
    }

    return sal_False;
}

/*************************************************************************
 *                class SwDropSave
 *************************************************************************/

class SwDropSave
{
    SwTxtPaintInfo* pInf;
    xub_StrLen nIdx;
    xub_StrLen nLen;
    long nX;
    long nY;

public:
    SwDropSave( const SwTxtPaintInfo &rInf );
    ~SwDropSave();
};

SwDropSave::SwDropSave( const SwTxtPaintInfo &rInf ) :
        pInf( ((SwTxtPaintInfo*)&rInf) ), nIdx( rInf.GetIdx() ),
        nLen( rInf.GetLen() ), nX( rInf.X() ), nY( rInf.Y() )
{
}

SwDropSave::~SwDropSave()
{
    pInf->SetIdx( nIdx );
    pInf->SetLen( nLen );
    pInf->X( nX );
    pInf->Y( nY );
}

/*************************************************************************
 *                SwDropPortionPart DTor
 *************************************************************************/

SwDropPortionPart::~SwDropPortionPart()
{
    delete pFollow;
    delete pFnt;
}

/*************************************************************************
 *                SwDropPortion CTor, DTor
 *************************************************************************/

SwDropPortion::SwDropPortion( const MSHORT nLineCnt,
                              const KSHORT nDrpHeight,
                              const KSHORT nDrpDescent,
                              const KSHORT nDist )
  : pPart( 0 ),
    nLines( nLineCnt ),
    nDropHeight(nDrpHeight),
    nDropDescent(nDrpDescent),
    nDistance(nDist),
    nFix(0),
    nX(0)
{
    SetWhichPor( POR_DROP );
}

SwDropPortion::~SwDropPortion()
{
    delete pPart;
    if( pBlink )
        pBlink->Delete( this );
}

sal_Bool SwTxtSizeInfo::_HasHint( const SwTxtNode* pTxtNode, xub_StrLen nPos )
{
    return 0 != pTxtNode->GetTxtAttrForCharAt(nPos);
}

/*************************************************************************
 *                    SwTxtNode::GetDropLen()
 *
 * nWishLen = 0 indicates that we want a whole word
 *************************************************************************/

MSHORT SwTxtNode::GetDropLen( MSHORT nWishLen ) const
{
    xub_StrLen nEnd = GetTxt().Len();
    if( nWishLen && nWishLen < nEnd )
        nEnd = nWishLen;

    if ( ! nWishLen && pBreakIt->GetBreakIter().is() )
    {
        // find first word
        const SwAttrSet& rAttrSet = GetSwAttrSet();
        const sal_uInt16 nTxtScript = pBreakIt->GetRealScriptOfText( GetTxt(), 0 );

        LanguageType eLanguage;

        switch ( nTxtScript )
        {
        case i18n::ScriptType::ASIAN :
            eLanguage = rAttrSet.GetCJKLanguage().GetLanguage();
            break;
        case i18n::ScriptType::COMPLEX :
            eLanguage = rAttrSet.GetCTLLanguage().GetLanguage();
            break;
        default :
            eLanguage = rAttrSet.GetLanguage().GetLanguage();
            break;
        }

        Boundary aBound =
            pBreakIt->GetBreakIter()->getWordBoundary( GetTxt(), 0,
            pBreakIt->GetLocale( eLanguage ), WordType::DICTIONARY_WORD, sal_True );

        nEnd = (xub_StrLen)aBound.endPos;
    }

    xub_StrLen i = 0;
    for( ; i < nEnd; ++i )
    {
        xub_Unicode cChar = GetTxt().GetChar( i );
        if( CH_TAB == cChar || CH_BREAK == cChar ||
            (( CH_TXTATR_BREAKWORD == cChar || CH_TXTATR_INWORD == cChar )
                && SwTxtSizeInfo::_HasHint( this, i ) ) )
            break;
    }
    return i;
}

/*************************************************************************
 *                    SwTxtNode::GetDropSize()
 *
 *  If a dropcap is found the return value is true otherwise false. The
 *  drop cap sizes passed back by reference are font height, drop height
 *  and drop descent.
 *************************************************************************/
bool SwTxtNode::GetDropSize(int& rFontHeight, int& rDropHeight, int& rDropDescent) const
{
    rFontHeight = 0;
    rDropHeight = 0;
    rDropDescent =0;

    const SwAttrSet& rSet = GetSwAttrSet();
    const SwFmtDrop& rDrop = rSet.GetDrop();

    // Return (0,0) if there is no drop cap at this paragraph
    if( 1 >= rDrop.GetLines() ||
        ( !rDrop.GetChars() && !rDrop.GetWholeWord() ) )
    {
        return false;
    }

    // get text frame
    SwIterator<SwTxtFrm,SwTxtNode> aIter( *this );
    for( SwTxtFrm* pLastFrm = aIter.First(); pLastFrm; pLastFrm = aIter.Next() )
    {
        // Only (master-) text frames can have a drop cap.
        if ( !pLastFrm->IsFollow() )
        {

            if( !pLastFrm->HasPara() )
                pLastFrm->GetFormatted();

            if ( !pLastFrm->IsEmpty() )
            {
                const SwParaPortion* pPara = pLastFrm->GetPara();
                OSL_ENSURE( pPara, "GetDropSize could not find the ParaPortion, I'll guess the drop cap size" );

                if ( pPara )
                {
                    const SwLinePortion* pFirstPor = pPara->GetFirstPortion();
                    if (pFirstPor && pFirstPor->IsDropPortion())
                    {
                        const SwDropPortion* pDrop = (const SwDropPortion*)pFirstPor;
                        rDropHeight = pDrop->GetDropHeight();
                        rDropDescent = pDrop->GetDropDescent();
                        if (const SwFont *pFont = pDrop->GetFnt())
                            rFontHeight = pFont->GetSize(pFont->GetActual()).Height();
                        else
                        {
                            const SvxFontHeightItem& rItem = (SvxFontHeightItem&)rSet.Get(RES_CHRATR_FONTSIZE);
                            rFontHeight = rItem.GetHeight();
                        }
                    }
                }
            }
            break;
        }
    }

    if (rFontHeight==0 && rDropHeight==0 && rDropDescent==0)
    {
        const sal_uInt16 nLines = rDrop.GetLines();

        const SvxFontHeightItem& rItem = (SvxFontHeightItem&)rSet.Get( RES_CHRATR_FONTSIZE );
        rFontHeight = rItem.GetHeight();
        rDropHeight = nLines * rFontHeight;
        rDropDescent = rFontHeight / 5;
        return false;
    }

    return true;
}

/*************************************************************************
 *                    SwDropPortion::PaintTxt()
 *************************************************************************/

// Die Breite manipulieren, sonst werden die Buchstaben gestretcht

void SwDropPortion::PaintTxt( const SwTxtPaintInfo &rInf ) const
{
    if ( rInf.OnWin() &&
        !rInf.GetOpt().IsPagePreview() && !rInf.GetOpt().IsReadonly() && SwViewOption::IsFieldShadings()    )
        rInf.DrawBackground( *this );

    OSL_ENSURE( nDropHeight && pPart && nLines != 1, "Drop Portion painted twice" );

    const SwDropPortionPart* pCurrPart = GetPart();
    const xub_StrLen nOldLen = GetLen();

    const SwTwips nBasePosY  = rInf.Y();
    ((SwTxtPaintInfo&)rInf).Y( nBasePosY + nY );
    SwDropSave aSave( rInf );
    // for text inside drop portions we let vcl handle the text directions
    SwLayoutModeModifier aLayoutModeModifier( *rInf.GetOut() );
    aLayoutModeModifier.SetAuto();

    while ( pCurrPart )
    {
        ((SwDropPortion*)this)->SetLen( pCurrPart->GetLen() );
        ((SwTxtPaintInfo&)rInf).SetLen( pCurrPart->GetLen() );
        SwFontSave aFontSave( rInf, &pCurrPart->GetFont() );

        SwTxtPortion::Paint( rInf );

        ((SwTxtPaintInfo&)rInf).SetIdx( rInf.GetIdx() + pCurrPart->GetLen() );
        ((SwTxtPaintInfo&)rInf).X( rInf.X() + pCurrPart->GetWidth() );
        pCurrPart = pCurrPart->GetFollow();
    }

    ((SwTxtPaintInfo&)rInf).Y( nBasePosY );
    ((SwDropPortion*)this)->SetLen( nOldLen );
}

/*************************************************************************
 *                   SwDropPortion::Paint()
 *************************************************************************/

void SwDropPortion::PaintDrop( const SwTxtPaintInfo &rInf ) const
{
    // ganz normale Ausgabe wird w?hrend des normalen Paints erledigt
    if( ! nDropHeight || ! pPart || nLines == 1 )
        return;

    // Luegenwerte einstellen!
    const KSHORT nOldHeight = Height();
    const KSHORT nOldWidth  = Width();
    const KSHORT nOldAscent = GetAscent();
    const SwTwips nOldPosY  = rInf.Y();
    const KSHORT nOldPosX   = (KSHORT)rInf.X();
    const SwParaPortion *pPara = rInf.GetParaPortion();
    const Point aOutPos( nOldPosX + nX, nOldPosY - pPara->GetAscent()
                         - pPara->GetRealHeight() + pPara->Height() );
    // Retusche nachholen.

    // Set baseline
    ((SwTxtPaintInfo&)rInf).Y( aOutPos.Y() + nDropHeight );

    // for background
    ((SwDropPortion*)this)->Height( nDropHeight + nDropDescent );
    ((SwDropPortion*)this)->Width( Width() - nX );
    ((SwDropPortion*)this)->SetAscent( nDropHeight );

    // Clipregion auf uns einstellen!
    // Und zwar immer, und nie mit dem bestehenden ClipRect
    // verrechnen, weil dies auf die Zeile eingestellt sein koennte.

    SwRect aClipRect;
    if ( rInf.OnWin() )
    {
        aClipRect = SwRect( aOutPos, SvLSize() );
        aClipRect.Intersection( rInf.GetPaintRect() );
    }
    SwSaveClip aClip( (OutputDevice*)rInf.GetOut() );
    aClip.ChgClip( aClipRect, rInf.GetTxtFrm() );
    // Das machen, was man sonst nur macht ...
    PaintTxt( rInf );

    // Alte Werte sichern
    ((SwDropPortion*)this)->Height( nOldHeight );
    ((SwDropPortion*)this)->Width( nOldWidth );
    ((SwDropPortion*)this)->SetAscent( nOldAscent );
    ((SwTxtPaintInfo&)rInf).Y( nOldPosY );
}

/*************************************************************************
 *              virtual SwDropPortion::Paint()
 *************************************************************************/

void SwDropPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    // ganz normale Ausgabe wird hier erledigt.
    if( ! nDropHeight || ! pPart || 1 == nLines )
    {
        if ( rInf.OnWin() &&
            !rInf.GetOpt().IsPagePreview() && !rInf.GetOpt().IsReadonly() && SwViewOption::IsFieldShadings()       )
            rInf.DrawBackground( *this );

        // make sure that font is not rotated
        SwFont* pTmpFont = 0;
        if ( rInf.GetFont()->GetOrientation( rInf.GetTxtFrm()->IsVertical() ) )
        {
            pTmpFont = new SwFont( *rInf.GetFont() );
            pTmpFont->SetVertical( 0, rInf.GetTxtFrm()->IsVertical() );
        }

        SwFontSave aFontSave( rInf, pTmpFont );
        // for text inside drop portions we let vcl handle the text directions
        SwLayoutModeModifier aLayoutModeModifier( *rInf.GetOut() );
        aLayoutModeModifier.SetAuto();

        SwTxtPortion::Paint( rInf );
        delete pTmpFont;
    }
}

/*************************************************************************
 *                virtual Format()
 *************************************************************************/


sal_Bool SwDropPortion::FormatTxt( SwTxtFormatInfo &rInf )
{
    const xub_StrLen nOldLen = GetLen();
    const xub_StrLen nOldInfLen = rInf.GetLen();
    const sal_Bool bFull = SwTxtPortion::Format( rInf );
    if( bFull )
    {
        // sieht zwar Scheisse aus, aber was soll man schon machen?
        rInf.SetUnderFlow( 0 );
        Truncate();
        SetLen( nOldLen );
        rInf.SetLen( nOldInfLen );
    }
    return bFull;
}

/*************************************************************************
 *                virtual GetTxtSize()
 *************************************************************************/


SwPosSize SwDropPortion::GetTxtSize( const SwTxtSizeInfo &rInf ) const
{
    sal_uInt16 nMyX = 0;
    xub_StrLen nIdx = 0;

    const SwDropPortionPart* pCurrPart = GetPart();

    // skip parts
    while ( pCurrPart && nIdx + pCurrPart->GetLen() < rInf.GetLen() )
    {
        nMyX = nMyX + pCurrPart->GetWidth();
        nIdx = nIdx + pCurrPart->GetLen();
        pCurrPart = pCurrPart->GetFollow();
    }

    xub_StrLen nOldIdx = rInf.GetIdx();
    xub_StrLen nOldLen = rInf.GetLen();

    ((SwTxtSizeInfo&)rInf).SetIdx( nIdx );
    ((SwTxtSizeInfo&)rInf).SetLen( rInf.GetLen() - nIdx );

    // robust
    SwFontSave aFontSave( rInf, pCurrPart ? &pCurrPart->GetFont() : 0 );
    SwPosSize aPosSize( SwTxtPortion::GetTxtSize( rInf ) );
    aPosSize.Width( aPosSize.Width() + nMyX );

    ((SwTxtSizeInfo&)rInf).SetIdx( nOldIdx );
    ((SwTxtSizeInfo&)rInf).SetLen( nOldLen );

    return aPosSize;
}

/*************************************************************************
 *                virtual GetCrsrOfst()
 *************************************************************************/

xub_StrLen SwDropPortion::GetCrsrOfst( const KSHORT ) const
{
    return 0;
}

/*************************************************************************
 *                SwTxtFormatter::CalcDropHeight()
 *************************************************************************/

void SwTxtFormatter::CalcDropHeight( const MSHORT nLines )
{
    const SwLinePortion *const pOldCurr = GetCurr();
    KSHORT nDropHght = 0;
    KSHORT nAscent = 0;
    KSHORT nHeight = 0;
    KSHORT nDropLns = 0;
    sal_Bool bRegisterOld = IsRegisterOn();
    bRegisterOn = sal_False;

    Top();

    while( GetCurr()->IsDummy() )
    {
        if ( !Next() )
            break;
    }

    // Wenn wir nur eine Zeile haben returnen wir 0
    if( GetNext() || GetDropLines() == 1 )
    {
        for( ; nDropLns < nLines; nDropLns++ )
        {
            if ( GetCurr()->IsDummy() )
                break;
            else
            {
                CalcAscentAndHeight( nAscent, nHeight );
                nDropHght = nDropHght + nHeight;
                bRegisterOn = bRegisterOld;
            }
            if ( !Next() )
            {
                nDropLns++; // Fix: 11356
                break;
            }
        }

        // In der letzten Zeile plumpsen wir auf den Zeilenascent!
        nDropHght = nDropHght - nHeight;
        nDropHght = nDropHght + nAscent;
        Top();
    }
    bRegisterOn = bRegisterOld;
    SetDropDescent( nHeight - nAscent );
    SetDropHeight( nDropHght );
    SetDropLines( nDropLns );
    // Alte Stelle wiederfinden!
    while( pOldCurr != GetCurr() )
    {
        if( !Next() )
        {
            OSL_ENSURE( !this, "SwTxtFormatter::_CalcDropHeight: left Toulouse" );
            break;
        }
    }
}

/*************************************************************************
 *                SwTxtFormatter::GuessDropHeight()
 *
 *  Wir schaetzen mal, dass die Fonthoehe sich nicht aendert und dass
 *  erst mindestens soviele Zeilen gibt, wie die DropCap-Einstellung angibt.
 *
 *************************************************************************/



void SwTxtFormatter::GuessDropHeight( const MSHORT nLines )
{
    OSL_ENSURE( nLines, "GuessDropHeight: Give me more Lines!" );
    KSHORT nAscent = 0;
    KSHORT nHeight = 0;
    SetDropLines( nLines );
    if ( GetDropLines() > 1 )
    {
        CalcRealHeight();
        CalcAscentAndHeight( nAscent, nHeight );
    }
    SetDropDescent( nHeight - nAscent );
    SetDropHeight( nHeight * nLines - GetDropDescent() );
}

/*************************************************************************
 *                SwTxtFormatter::NewDropPortion
 *************************************************************************/

SwDropPortion *SwTxtFormatter::NewDropPortion( SwTxtFormatInfo &rInf )
{
    if( !pDropFmt )
        return 0;

    xub_StrLen nPorLen = pDropFmt->GetWholeWord() ? 0 : pDropFmt->GetChars();
    nPorLen = pFrm->GetTxtNode()->GetDropLen( nPorLen );
    if( !nPorLen )
    {
        ((SwTxtFormatter*)this)->ClearDropFmt();
        return 0;
    }

    SwDropPortion *pDropPor = 0;

    // erste oder zweite Runde?
    if ( !( GetDropHeight() || IsOnceMore() ) )
    {
        if ( GetNext() )
            CalcDropHeight( pDropFmt->GetLines() );
        else
            GuessDropHeight( pDropFmt->GetLines() );
    }

    // the DropPortion
    if( GetDropHeight() )
        pDropPor = new SwDropPortion( GetDropLines(), GetDropHeight(),
                                      GetDropDescent(), pDropFmt->GetDistance() );
    else
        pDropPor = new SwDropPortion( 0,0,0,pDropFmt->GetDistance() );

    pDropPor->SetLen( nPorLen );

    // If it was not possible to create a proper drop cap portion
    // due to avoiding endless loops. We return a drop cap portion
    // with an empty SwDropCapPart. For these portions the current
    // font is used.
    if ( GetDropLines() < 2 )
    {
        ((SwTxtFormatter*)this)->SetPaintDrop( sal_True );
        return pDropPor;
    }

    // build DropPortionParts:
    OSL_ENSURE( ! rInf.GetIdx(), "Drop Portion not at 0 position!" );
    xub_StrLen nNextChg = 0;
    const SwCharFmt* pFmt = pDropFmt->GetCharFmt();
    SwDropPortionPart* pCurrPart = 0;

    while ( nNextChg  < nPorLen )
    {
        // check for attribute changes and if the portion has to split:
        Seek( nNextChg );

        // the font is deleted in the destructor of the drop portion part
        SwFont* pTmpFnt = new SwFont( *rInf.GetFont() );
        if ( pFmt )
        {
            const SwAttrSet& rSet = pFmt->GetAttrSet();
            pTmpFnt->SetDiffFnt( &rSet, pFrm->GetTxtNode()->getIDocumentSettingAccess() );
        }

        // we do not allow a vertical font for the drop portion
        pTmpFnt->SetVertical( 0, rInf.GetTxtFrm()->IsVertical() );

        // find next attribute change / script change
        const xub_StrLen nTmpIdx = nNextChg;
        xub_StrLen nNextAttr = Min( GetNextAttr(), rInf.GetTxt().Len() );
        nNextChg = pScriptInfo->NextScriptChg( nTmpIdx );
        if( nNextChg > nNextAttr )
            nNextChg = nNextAttr;
        if ( nNextChg > nPorLen )
            nNextChg = nPorLen;

        SwDropPortionPart* pPart =
                new SwDropPortionPart( *pTmpFnt, nNextChg - nTmpIdx );

        if ( ! pCurrPart )
            pDropPor->SetPart( pPart );
        else
            pCurrPart->SetFollow( pPart );

        pCurrPart = pPart;
    }

    ((SwTxtFormatter*)this)->SetPaintDrop( sal_True );
    return pDropPor;
}

/*************************************************************************
 *                SwTxtPainter::PaintDropPortion()
 *************************************************************************/



void SwTxtPainter::PaintDropPortion()
{
    const SwDropPortion *pDrop = GetInfo().GetParaPortion()->FindDropPortion();
    OSL_ENSURE( pDrop, "DrapCop-Portion not available." );
    if( !pDrop )
        return;

    const SwTwips nOldY = GetInfo().Y();

    Top();

    GetInfo().SetpSpaceAdd( pCurr->GetpLLSpaceAdd() );
    GetInfo().ResetSpaceIdx();
    GetInfo().SetKanaComp( pCurr->GetpKanaComp() );
    GetInfo().ResetKanaIdx();

    // 8047: Drops und Dummies
    while( !pCurr->GetLen() && Next() )
        ;

    // MarginPortion und Adjustment!
    const SwLinePortion *pPor = pCurr->GetFirstPortion();
    KSHORT nX = 0;
    while( pPor && !pPor->IsDropPortion() )
    {
        nX = nX + pPor->Width();
        pPor = pPor->GetPortion();
    }
    Point aLineOrigin( GetTopLeft() );

    aLineOrigin.X() += nX;
    KSHORT nTmpAscent, nTmpHeight;
    CalcAscentAndHeight( nTmpAscent, nTmpHeight );
    aLineOrigin.Y() += nTmpAscent;
    GetInfo().SetIdx( GetStart() );
    GetInfo().SetPos( aLineOrigin );
    GetInfo().SetLen( pDrop->GetLen() );

    pDrop->PaintDrop( GetInfo() );

    GetInfo().Y( nOldY );
}

/*************************************************************************
 *                      clas SwDropCapCache
 *
 * Da die Berechnung der Fontgroesse der Initialen ein teures Geschaeft ist,
 * wird dies durch einen DropCapCache geschleust.
 *************************************************************************/

#define DROP_CACHE_SIZE 10

class SwDropCapCache
{
    long aMagicNo[ DROP_CACHE_SIZE ];
    XubString aTxt[ DROP_CACHE_SIZE ];
    sal_uInt16 aFactor[ DROP_CACHE_SIZE ];
    KSHORT aWishedHeight[ DROP_CACHE_SIZE ];
    short aDescent[ DROP_CACHE_SIZE ];
    MSHORT nIndex;
public:
    SwDropCapCache();
    ~SwDropCapCache(){}
    void CalcFontSize( SwDropPortion* pDrop, SwTxtFormatInfo &rInf );
};

/*************************************************************************
 *                  SwDropCapCache Ctor / Dtor
 *************************************************************************/

SwDropCapCache::SwDropCapCache() : nIndex( 0 )
{
    memset( &aMagicNo, 0, sizeof(aMagicNo) );
    memset( &aWishedHeight, 0, sizeof(aWishedHeight) );
}

void SwDropPortion::DeleteDropCapCache()
{
    delete pDropCapCache;
}

/*************************************************************************
 *                  SwDropCapCache::CalcFontSize
 *************************************************************************/

void SwDropCapCache::CalcFontSize( SwDropPortion* pDrop, SwTxtFormatInfo &rInf )
{
    const void* pFntNo = 0;
    MSHORT nTmpIdx = 0;

    OSL_ENSURE( pDrop->GetPart(),"DropPortion without part during font calculation");

    SwDropPortionPart* pCurrPart = pDrop->GetPart();
    const sal_Bool bUseCache = ! pCurrPart->GetFollow();
    xub_StrLen nIdx = rInf.GetIdx();
    XubString aStr( rInf.GetTxt(), nIdx, pCurrPart->GetLen() );

    long nAscent = 0;
    long nDescent = 0;
    long nFactor = -1;

    if ( bUseCache )
    {
        SwFont& rFnt = pCurrPart->GetFont();
        rFnt.ChkMagic( rInf.GetVsh(), rFnt.GetActual() );
        rFnt.GetMagic( pFntNo, nTmpIdx, rFnt.GetActual() );

        nTmpIdx = 0;

        while( nTmpIdx < DROP_CACHE_SIZE &&
            ( aTxt[ nTmpIdx ] != aStr || aMagicNo[ nTmpIdx ] != long(pFntNo) ||
            aWishedHeight[ nTmpIdx ] != pDrop->GetDropHeight() ) )
            ++nTmpIdx;
    }

    // we have to calculate a new font scaling factor if
    // 1. we did not find a scaling factor in the cache or
    // 2. we are not allowed to use the cache because the drop portion
    //    consists of more than one part
    if( nTmpIdx >= DROP_CACHE_SIZE || ! bUseCache )
    {
        ++nIndex;
        nIndex %= DROP_CACHE_SIZE;
        nTmpIdx = nIndex;

        long nWishedHeight = pDrop->GetDropHeight();

        // find out biggest font size for initial scaling factor
        long nMaxFontHeight = 0;
        while ( pCurrPart )
        {
            const SwFont& rFnt = pCurrPart->GetFont();
            const long nCurrHeight = rFnt.GetHeight( rFnt.GetActual() );
            if ( nCurrHeight > nMaxFontHeight )
                nMaxFontHeight = nCurrHeight;

            pCurrPart = pCurrPart->GetFollow();
        }

        nFactor = ( 1000 * nWishedHeight ) / nMaxFontHeight;

        if ( bUseCache )
        {
            // save keys for cache
            aMagicNo[ nTmpIdx ] = long(pFntNo);
            aTxt[ nTmpIdx ] = aStr;
            aWishedHeight[ nTmpIdx ] = KSHORT(nWishedHeight);
            // save initial scaling factor
            aFactor[ nTmpIdx ] = (sal_uInt16)nFactor;
        }

        sal_Bool bGrow = ( pDrop->GetLen() != 0 );

        // for growing controll
        long nMax = KSHRT_MAX;
        long nMin = nFactor / 2;
#if OSL_DEBUG_LEVEL > 1
        long nGrow = 0;
#endif

        sal_Bool bWinUsed = sal_False;
        Font aOldFnt;
        MapMode aOldMap( MAP_TWIP );
        OutputDevice* pOut = rInf.GetOut();
        OutputDevice* pWin;
        if( rInf.GetVsh() && rInf.GetVsh()->GetWin() )
            pWin = rInf.GetVsh()->GetWin();
        else
            pWin = GetpApp()->GetDefaultDevice();

        while( bGrow )
        {
            // reset pCurrPart to first part
            pCurrPart = pDrop->GetPart();
            sal_Bool bFirstGlyphRect = sal_True;
            sal_Bool bHaveGlyphRect = sal_False;
            Rectangle aCommonRect, aRect;

            while ( pCurrPart )
            {
                // current font
                SwFont& rFnt = pCurrPart->GetFont();

                // Get height including proportion
                const sal_uInt16 nCurrHeight =
                         (sal_uInt16)rFnt.GetHeight( rFnt.GetActual() );

                // Get without proportion
                const sal_uInt8 nOldProp = rFnt.GetPropr();
                rFnt.SetProportion( 100 );
                Size aOldSize = Size( 0, rFnt.GetHeight( rFnt.GetActual() ) );

                Size aNewSize( 0, ( nFactor * nCurrHeight ) / 1000 );
                rFnt.SetSize( aNewSize, rFnt.GetActual() );
                rFnt.ChgPhysFnt( rInf.GetVsh(), *pOut );

                nAscent = rFnt.GetAscent( rInf.GetVsh(), *pOut );

                // Wir besorgen uns das alle Buchstaben umfassende Rechteck:
                bHaveGlyphRect = pOut->GetTextBoundRect( aRect, rInf.GetTxt(), 0,
                                     nIdx, pCurrPart->GetLen() ) &&
                                 ! aRect.IsEmpty();

                if ( ! bHaveGlyphRect )
                {
                    // getting glyph boundaries failed for some reason,
                    // we take the window for calculating sizes
                    if ( pWin )
                    {
                        if ( ! bWinUsed )
                        {
                            bWinUsed = sal_True;
                            aOldMap = pWin->GetMapMode( );
                            pWin->SetMapMode( MapMode( MAP_TWIP ) );
                            aOldFnt = pWin->GetFont();
                        }
                        pWin->SetFont( rFnt.GetActualFont() );

                        bHaveGlyphRect = pWin->GetTextBoundRect( aRect, rInf.GetTxt(), 0,
                                            nIdx, pCurrPart->GetLen() ) &&
                                        ! aRect.IsEmpty();
                    }
                    if ( bHaveGlyphRect )
                    {
                        FontMetric aWinMet( pWin->GetFontMetric() );
                        nAscent = (KSHORT) aWinMet.GetAscent();
                    }
                    else
                    // We do not have a window or our window could not
                    // give us glyph boundaries.
                        aRect = Rectangle( Point( 0, 0 ), Size( 0, nAscent ) );
                }

                // Now we (hopefully) have a bounding rectangle for the
                // glyphs of the current portion and the ascent of the current
                // font

                // reset font size and proportion
                rFnt.SetSize( aOldSize, rFnt.GetActual() );
                rFnt.SetProportion( nOldProp );

                if ( bFirstGlyphRect )
                {
                    aCommonRect = aRect;
                    bFirstGlyphRect = sal_False;
                }
                else
                    aCommonRect.Union( aRect );

                nIdx = nIdx + pCurrPart->GetLen();
                pCurrPart = pCurrPart->GetFollow();
            }

            // now we have a union ( aCommonRect ) of all glyphs with
            // respect to a common baseline : 0

            // get descent and ascent from union
            if ( rInf.GetTxtFrm()->IsVertical() )
            {
                nDescent = aCommonRect.Left();
                nAscent = aCommonRect.Right();

                if ( nDescent < 0 )
                    nDescent = -nDescent;
            }
            else
            {
                nDescent = aCommonRect.Bottom();
                nAscent = aCommonRect.Top();
            }
            if ( nAscent < 0 )
                nAscent = -nAscent;

            const long nHght = nAscent + nDescent;
            if ( nHght )
            {
                if ( nHght > nWishedHeight )
                    nMax = nFactor;
                else
                {
                    if ( bUseCache )
                        aFactor[ nTmpIdx ] = (sal_uInt16)nFactor;
                    nMin = nFactor;
                }

                nFactor = ( nFactor * nWishedHeight ) / nHght;
                bGrow = ( nFactor > nMin ) && ( nFactor < nMax );
#if OSL_DEBUG_LEVEL > 1
                if ( bGrow )
                    nGrow++;
#endif
                nIdx = rInf.GetIdx();
            }
            else
                bGrow = sal_False;
        }

        if ( bWinUsed )
        {
            // reset window if it has been used
            pWin->SetMapMode( aOldMap );
            pWin->SetFont( aOldFnt );
        }

        if ( bUseCache )
            aDescent[ nTmpIdx ] = -short( nDescent );
    }

    pCurrPart = pDrop->GetPart();

    // did made any new calculations or did we use the cache?
    if ( -1 == nFactor )
    {
        nFactor = aFactor[ nTmpIdx ];
        nDescent = aDescent[ nTmpIdx ];
    }
    else
        nDescent = -nDescent;

    while ( pCurrPart )
    {
        // scale current font
        SwFont& rFnt = pCurrPart->GetFont();
        Size aNewSize( 0, ( nFactor * rFnt.GetHeight( rFnt.GetActual() ) ) / 1000 );

        const sal_uInt8 nOldProp = rFnt.GetPropr();
        rFnt.SetProportion( 100 );
        rFnt.SetSize( aNewSize, rFnt.GetActual() );
        rFnt.SetProportion( nOldProp );

        pCurrPart = pCurrPart->GetFollow();
    }
    pDrop->SetY( (short)nDescent );
}

/*************************************************************************
 *                virtual Format()
 *************************************************************************/

sal_Bool SwDropPortion::Format( SwTxtFormatInfo &rInf )
{
    sal_Bool bFull = sal_False;
    Fix( (sal_uInt16)rInf.X() );

    SwLayoutModeModifier aLayoutModeModifier( *rInf.GetOut() );
    aLayoutModeModifier.SetAuto();

    if( nDropHeight && pPart && nLines!=1 )
    {
        if( !pDropCapCache )
            pDropCapCache = new SwDropCapCache();

        // adjust font sizes to fit into the rectangle
        pDropCapCache->CalcFontSize( this, rInf );

        const long nOldX = rInf.X();
        {
            SwDropSave aSave( rInf );
            SwDropPortionPart* pCurrPart = pPart;

            while ( pCurrPart )
            {
                rInf.SetLen( pCurrPart->GetLen() );
                SwFont& rFnt = pCurrPart->GetFont();
                {
                    SwFontSave aFontSave( rInf, &rFnt );
                    bFull = FormatTxt( rInf );

                    if ( bFull )
                        break;
                }

                const SwTwips nTmpWidth =
                        ( InSpaceGrp() && rInf.GetSpaceAdd() ) ?
                        Width() + CalcSpacing( rInf.GetSpaceAdd(), rInf ) :
                        Width();

                // set values
                pCurrPart->SetWidth( (sal_uInt16)nTmpWidth );

                // Move
                rInf.SetIdx( rInf.GetIdx() + pCurrPart->GetLen() );
                rInf.X( rInf.X() + nTmpWidth );
                pCurrPart = pCurrPart->GetFollow();
            }

            Width( (sal_uInt16)(rInf.X() - nOldX) );
        }

        // reset my length
        SetLen( rInf.GetLen() );

        // 7631, 7633: bei Ueberlappungen mit Flys ist Schluss.
        if( ! bFull )
            bFull = lcl_IsDropFlyInter( rInf, Width(), nDropHeight );

        if( bFull )
        {
            // Durch FormatTxt kann nHeight auf 0 gesetzt worden sein
            if ( !Height() )
                Height( rInf.GetTxtHeight() );

            // Jetzt noch einmal der ganze Spass
            nDropHeight = nLines = 0;
            delete pPart;
            pPart = NULL;

            // meanwhile use normal formatting
            bFull = SwTxtPortion::Format( rInf );
        }
        else
            rInf.SetDropInit( sal_True );

        Height( rInf.GetTxtHeight() );
        SetAscent( rInf.GetAscent() );
    }
    else
        bFull = SwTxtPortion::Format( rInf );

    if( bFull )
        nDistance = 0;
    else
    {
        const KSHORT nWant = Width() + GetDistance();
        const KSHORT nRest = (sal_uInt16)(rInf.Width() - rInf.X());
        if( ( nWant > nRest ) ||
            lcl_IsDropFlyInter( rInf, Width() + GetDistance(), nDropHeight ) )
            nDistance = 0;

        Width( Width() + nDistance );
    }
    return bFull;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
