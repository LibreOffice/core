/*************************************************************************
 *
 *  $RCSfile: txtdrop.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-25 07:39:23 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SV_METRIC_HXX //autogen
#include <vcl/metric.hxx>
#endif
#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>   // Format()
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>  // SwViewOption
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>   // ViewShell
#endif
#ifndef _PORDROP_HXX
#include <pordrop.hxx>
#endif
#ifndef _ITRFORM2_HXX
#include <itrform2.hxx>
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>
#endif
#ifndef _TXTPAINT_HXX
#include <txtpaint.hxx> // SwSaveClip
#endif
#ifndef _TXTFLY_HXX
#include <txtfly.hxx>   // Format()
#endif
#ifndef _BLINK_HXX
#include <blink.hxx>    // pBlink
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>     // GetDoc()
#endif
#ifndef _SVX_FHGTITEM_HXX
#include <svx/fhgtitem.hxx>
#endif

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star;

/*************************************************************************
 *                lcl_IsDropFlyInter
 *
 *  Calculates if a drop caps portion intersects with a fly
 *  The width and height of the drop caps portion are passed as arguments,
 *  the position is calculated from the values in rInf
 *************************************************************************/

sal_Bool lcl_IsDropFlyInter( const SwTxtFormatInfo &rInf,
                             USHORT nWidth, USHORT nHeight )
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
    if ( pFollow )
        delete pFollow;
    delete pFnt;
}

/*************************************************************************
 *                SwDropPortion CTor, DTor
 *************************************************************************/

SwDropPortion::SwDropPortion( const MSHORT nLineCnt,
                              const KSHORT nDropHeight,
                              const KSHORT nDropDescent,
                              const KSHORT nDistance )
  : pPart( 0 ),
    nLines( nLineCnt ),
    nDropHeight(nDropHeight),
    nDropDescent(nDropDescent),
    nDistance(nDistance),
    nX(0),
    nFix(0)
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
    const SwpHints *pHints = pTxtNode->GetpSwpHints();
    if( !pHints )
        return sal_False;
    for( MSHORT i = 0; i < pHints->Count(); ++i )
    {
        const SwTxtAttr *pPos = (*pHints)[i];
        xub_StrLen nStart = *pPos->GetStart();
        if( nPos < nStart )
            return sal_False;
        if( nPos == nStart && !pPos->GetEnd() )
            return sal_True;
    }
    return sal_False;
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

    if ( ! nWishLen && pBreakIt->xBreak.is() )
    {
        // find first word
        const SwAttrSet& rAttrSet = GetSwAttrSet();
        const USHORT nTxtScript = pBreakIt->GetRealScriptOfText( GetTxt(), 0 );

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
            pBreakIt->xBreak->getWordBoundary( GetTxt(), 0,
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
    SwDropPortion *pRet = 0;
    rFontHeight = 0;
    rDropHeight = 0;
    rDropDescent =0;

    ASSERT( GetDoc() && GetDoc()->GetRootFrm(),
            "No layout available in GetDropSize(), I'll guess the drop cap size" )

    const SwAttrSet& rSet = GetSwAttrSet();
    const SwFmtDrop& rDrop = rSet.GetDrop();

    // Return (0,0) if there is no drop cap at this paragraph
    if( 1 >= rDrop.GetLines() ||
        ( !rDrop.GetChars() && !rDrop.GetWholeWord() ) )
    {
        return false;
    }

    // get text frame
    SwClientIter aClientIter( (SwTxtNode&)*this );
    SwClient* pLast = aClientIter.GoStart();

    while( pLast )
    {
        // Only (master-) text frames can have a drop cap.
        if ( pLast->ISA( SwTxtFrm ) && !((SwTxtFrm*)pLast)->IsFollow() )
        {

            if( !((SwTxtFrm*)pLast)->HasPara() )
                ((SwTxtFrm*)pLast)->GetFormatted();

            if ( !((SwTxtFrm*)pLast)->IsEmpty() )
            {
                const SwParaPortion* pPara = ((SwTxtFrm*)pLast)->GetPara();
                ASSERT( pPara, "GetDropSize could not find the ParaPortion, I'll guess the drop cap size" )

                if ( pPara )
                {
                    const SwLinePortion* pFirst = pPara->GetFirstPortion();
                    if ( pFirst->IsDropPortion() )
                    {
                        SwFont& rFont = ((SwDropPortion*)pFirst)->GetPart()->GetFont();
                        rFontHeight = rFont.GetSize(rFont.GetActual()).Height();
                        rDropHeight = ((SwDropPortion*)pFirst)->GetDropHeight();
                        rDropDescent = ((SwDropPortion*)pFirst)->GetDropDescent();
                    }
                }
            }
            break;
        }
        pLast = ++aClientIter;
    }

    if(rFontHeight==0 && rDropHeight==0 && rDropDescent==0)
    {
        const USHORT nLines = rDrop.GetLines();
        const USHORT nChars = rDrop.GetChars() ? rDrop.GetChars() : 1;

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

    ASSERT( nDropHeight && pPart && nLines != 1, "Drop Portion painted twice" );

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
        SwFontSave aSave( rInf, &pCurrPart->GetFont() );

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
    // ganz normale Ausgabe wird während des normalen Paints erledigt
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
#ifdef VERTICAL_LAYOUT
    aClip.ChgClip( aClipRect, rInf.GetTxtFrm() );
#else
    aClip.ChgClip( aClipRect );
#endif
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

        SwFontSave aSave( rInf, pTmpFont );
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
    USHORT nX = 0;
    xub_StrLen nIdx = 0;

    const SwDropPortionPart* pCurrPart = GetPart();

    // skip parts
    while ( pCurrPart && nIdx + pCurrPart->GetLen() < rInf.GetLen() )
    {
        nX += pCurrPart->GetWidth();
        nIdx += pCurrPart->GetLen();
        pCurrPart = pCurrPart->GetFollow();
    }

    xub_StrLen nOldIdx = rInf.GetIdx();
    xub_StrLen nOldLen = rInf.GetLen();

    ((SwTxtSizeInfo&)rInf).SetIdx( nIdx );
    ((SwTxtSizeInfo&)rInf).SetLen( rInf.GetLen() - nIdx );

    // robust
    SwFontSave aSave( rInf, pCurrPart ? &pCurrPart->GetFont() : 0 );
    SwPosSize aPosSize( SwTxtPortion::GetTxtSize( rInf ) );
    aPosSize.Width( aPosSize.Width() + nX );

    ((SwTxtSizeInfo&)rInf).SetIdx( nOldIdx );
    ((SwTxtSizeInfo&)rInf).SetLen( nOldLen );

    return aPosSize;
}

/*************************************************************************
 *                virtual GetCrsrOfst()
 *************************************************************************/

xub_StrLen SwDropPortion::GetCrsrOfst( const KSHORT nOfst ) const
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
                nDropHght += nHeight;
                bRegisterOn = bRegisterOld;
            }
            if ( !Next() )
            {
                nDropLns++; // Fix: 11356
                break;
            }
        }

        // In der letzten Zeile plumpsen wir auf den Zeilenascent!
        nDropHght -= nHeight;
        nDropHght += nAscent;
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
            ASSERT( !this, "SwTxtFormatter::_CalcDropHeight: left Toulouse" );
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
    ASSERT( nLines, "GuessDropHeight: Give me more Lines!" );
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
    ASSERT( ! rInf.GetIdx(), "Drop Portion not at 0 position!" );
    xub_StrLen nIdx = rInf.GetIdx();
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
            pTmpFnt->SetDiffFnt( &rSet, rInf.GetDoc() );
        }

        // we do not allow a vertical font for the drop portion
#ifdef VERTICAL_LAYOUT
        pTmpFnt->SetVertical( 0, rInf.GetTxtFrm()->IsVertical() );
#else
        pTmpFnt->SetVertical( 0 );
#endif

        // find next attribute change / script change
        const xub_StrLen nIdx = nNextChg;
        xub_StrLen nNextAttr = Min( GetNextAttr(), rInf.GetTxt().Len() );
        nNextChg = pScriptInfo->NextScriptChg( nIdx );
        if( nNextChg > nNextAttr )
            nNextChg = nNextAttr;
        if ( nNextChg > nPorLen )
            nNextChg = nPorLen;

        SwDropPortionPart* pPart =
                new SwDropPortionPart( *pTmpFnt, nNextChg - nIdx );

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
    ASSERT( pDrop, "DrapCop-Portion not available." );
    if( !pDrop )
        return;

    const SwTwips nOldY = GetInfo().Y();

    Top();

    GetInfo().SetSpaceAdd( pCurr->GetpSpaceAdd() );
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
        nX += pPor->Width();
        pPor = pPor->GetPortion();
    }
    Point aLineOrigin( GetTopLeft() );

#ifdef NIE
    // Retusche nachholen...
    if( nX )
    {
        const Point aPoint( Left(), Y() );
        const Size  aSize( nX - 1, GetDropHeight()+GetDropDescent() );
        SwRect aRetouche( aPoint, aSize );
        GetInfo().DrawRect( aRetouche );
    }
#endif

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
    USHORT aFactor[ DROP_CACHE_SIZE ];
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
    const void* pFntNo;
    MSHORT nTmpIdx = 0;

    ASSERT( pDrop->GetPart(),"DropPortion without part during font calculation");

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
            aFactor[ nTmpIdx ] = (USHORT)nFactor;
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
        OutputDevice *pOut = rInf.GetOut();
        OutputDevice *pWin;
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
                const USHORT nCurrHeight =
                         (USHORT)rFnt.GetHeight( rFnt.GetActual() );

                // Get without proportion
                const BYTE nOldProp = rFnt.GetPropr();
                rFnt.SetProportion( 100 );
                Size aOldSize = Size( 0, rFnt.GetHeight( rFnt.GetActual() ) );

                Size aNewSize( 0, ( nFactor * nCurrHeight ) / 1000 );
                rFnt.SetSize( aNewSize, rFnt.GetActual() );
                rFnt.ChgPhysFnt( rInf.GetVsh(), pOut );
                nAscent = rFnt.GetAscent( rInf.GetVsh(), pOut ) -
                          rFnt.GetLeading( rInf.GetVsh(), pOut );

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

                nIdx += pCurrPart->GetLen();
                pCurrPart = pCurrPart->GetFollow();
            }

            // now we have a union ( aCommonRect ) of all glyphs with
            // respect to a common baseline : 0

            // get descent and ascent from union
#ifdef VERTICAL_LAYOUT
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
#else
            nDescent = aCommonRect.Bottom();
            nAscent = aCommonRect.Top();
#endif
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
                        aFactor[ nTmpIdx ] = (USHORT)nFactor;
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

        const BYTE nOldProp = rFnt.GetPropr();
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
    Fix( (USHORT)rInf.X() );

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
                    SwFontSave aSave( rInf, &rFnt );
                    bFull = FormatTxt( rInf );

                    if ( bFull )
                        break;
                }

                SwTwips nTmpWidth =
                        ( InSpaceGrp() && rInf.GetSpaceAdd() ) ?
                        Width() + CalcSpacing( rInf.GetSpaceAdd(), rInf ) :
                        Width();

                // set values
                pCurrPart->SetWidth( (USHORT)nTmpWidth );

                // Move
                rInf.SetIdx( rInf.GetIdx() + pCurrPart->GetLen() );
                rInf.X( rInf.X() + nTmpWidth );
                pCurrPart = pCurrPart->GetFollow();
            }

            Width( (USHORT)(rInf.X() - nOldX) );
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
        const KSHORT nRest = (USHORT)(rInf.Width() - rInf.X());
        if( ( nWant > nRest ) ||
            lcl_IsDropFlyInter( rInf, Width() + GetDistance(), nDropHeight ) )
            nDistance = 0;

        Width( Width() + nDistance );
    }
    return bFull;
}

