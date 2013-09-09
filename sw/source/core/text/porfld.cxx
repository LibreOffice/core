/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <hintids.hxx>

#include <com/sun/star/i18n/ScriptType.hpp>
#include <vcl/graph.hxx>
#include <editeng/brushitem.hxx>
#include <vcl/metric.hxx>
#include <vcl/outdev.hxx>
#include <viewopt.hxx>  // SwViewOptions
#include <SwPortionHandler.hxx>
#include <porlay.hxx>
#include <porfld.hxx>
#include <inftxt.hxx>
#include <blink.hxx>    // pBlink
#include <frmtool.hxx>  // DrawGraphic
#include <viewsh.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include "rootfrm.hxx"
#include <breakit.hxx>
#include <porrst.hxx>
#include <porftn.hxx>   // SwFtnPortion
#include <accessibilityoptions.hxx>
#include <editeng/lrspitem.hxx>

#include <unicode/ubidi.h>

using namespace ::com::sun::star;

/*************************************************************************
 *                      class SwFldPortion
 *************************************************************************/

SwLinePortion *SwFldPortion::Compress()
{ return (GetLen() || !aExpand.isEmpty() || SwLinePortion::Compress()) ? this : 0; }

SwFldPortion *SwFldPortion::Clone( const OUString &rExpand ) const
{
    SwFont *pNewFnt;
    if( 0 != ( pNewFnt = pFnt ) )
    {
        pNewFnt = new SwFont( *pFnt );
    }
    // #i107143#
    // pass placeholder property to created <SwFldPortion> instance.
    SwFldPortion* pClone = new SwFldPortion( rExpand, pNewFnt, bPlaceHolder );
    pClone->SetNextOffset( nNextOffset );
    pClone->m_bNoLength = this->m_bNoLength;
    return pClone;
}

void SwFldPortion::TakeNextOffset( const SwFldPortion* pFld )
{
    OSL_ENSURE( pFld, "TakeNextOffset: Missing Source" );
    nNextOffset = pFld->GetNextOffset();
    aExpand = aExpand.replaceAt( 0, nNextOffset, "" );
    bFollow = sal_True;
}

SwFldPortion::SwFldPortion( const OUString &rExpand, SwFont *pFont, bool bPlaceHold )
    : aExpand(rExpand), pFnt(pFont), nNextOffset(0), nNextScriptChg(STRING_LEN), nViewWidth(0),
      bFollow( sal_False ), bHasFollow( sal_False ), bPlaceHolder( bPlaceHold )
    , m_bNoLength( sal_False )
{
    SetWhichPor( POR_FLD );
}

SwFldPortion::SwFldPortion( const SwFldPortion& rFld )
    : SwExpandPortion( rFld ),
      aExpand( rFld.GetExp() ),
      nNextOffset( rFld.GetNextOffset() ),
      nNextScriptChg( rFld.GetNextScriptChg() ),
      bFollow( rFld.IsFollow() ),
      bLeft( rFld.IsLeft() ),
      bHide( rFld.IsHide() ),
      bCenter( rFld.IsCenter() ),
      bHasFollow( rFld.HasFollow() ),
      bPlaceHolder( rFld.bPlaceHolder )
    , m_bNoLength( rFld.m_bNoLength )
{
    if ( rFld.HasFont() )
        pFnt = new SwFont( *rFld.GetFont() );
    else
        pFnt = 0;

    SetWhichPor( POR_FLD );
}

SwFldPortion::~SwFldPortion()
{
    delete pFnt;
    if( pBlink )
        pBlink->Delete( this );
}

/*************************************************************************
 *               virtual SwFldPortion::GetViewWidth()
 *************************************************************************/

KSHORT SwFldPortion::GetViewWidth( const SwTxtSizeInfo &rInf ) const
{
    // Wir stehen zwar im const, aber nViewWidth sollte erst im letzten
    // Moment errechnet werden:
    SwFldPortion* pThis = (SwFldPortion*)this;
    if( !Width() && rInf.OnWin() && !rInf.GetOpt().IsPagePreview() &&
            !rInf.GetOpt().IsReadonly() && SwViewOption::IsFieldShadings() )
    {
        if( !nViewWidth )
            pThis->nViewWidth = rInf.GetTxtSize(OUString(' ')).Width();
    }
    else
        pThis->nViewWidth = 0;
    return nViewWidth;
}

/*************************************************************************
 *                 virtual SwFldPortion::Format()
 *************************************************************************/

// 8653: in keinem Fall nur SetLen(0);

/*************************************************************************
 *   Hilfsklasse SwFldSlot
 **************************************************************************/

class SwFldSlot
{
    const OUString *pOldTxt;
    OUString aTxt;
    sal_Int32 nIdx;
    sal_Int32 nLen;
    sal_Bool bOn;
    SwTxtFormatInfo *pInf;
public:
    SwFldSlot( const SwTxtFormatInfo* pNew, const SwFldPortion *pPor );
    ~SwFldSlot();
};

SwFldSlot::SwFldSlot( const SwTxtFormatInfo* pNew, const SwFldPortion *pPor )
{
    bOn = pPor->GetExpTxt( *pNew, aTxt );

    // Der Text wird ausgetauscht...
    if( bOn )
    {
        pInf = (SwTxtFormatInfo*)pNew;
        nIdx = pInf->GetIdx();
        nLen = pInf->GetLen();
        pOldTxt = &(pInf->GetTxt());
        pInf->SetLen( aTxt.getLength() );
        if( pPor->IsFollow() )
        {
            pInf->SetFakeLineStart( nIdx > pInf->GetLineStart() );
            pInf->SetIdx( 0 );
        }
        else
        {
            aTxt = (*pOldTxt).replaceAt(nIdx, 1, aTxt);
        }
        pInf->SetTxt( aTxt );
    }
}

SwFldSlot::~SwFldSlot()
{
    if( bOn )
    {
        pInf->SetTxt( *pOldTxt );
        pInf->SetIdx( nIdx );
        pInf->SetLen( nLen );
        pInf->SetFakeLineStart( sal_False );
    }
}

void SwFldPortion::CheckScript( const SwTxtSizeInfo &rInf )
{
    OUString aTxt;
    if( GetExpTxt( rInf, aTxt ) && !aTxt.isEmpty() && g_pBreakIt->GetBreakIter().is() )
    {
        sal_uInt8 nActual = pFnt ? pFnt->GetActual() : rInf.GetFont()->GetActual();
        sal_uInt16 nScript;
        {
            nScript = g_pBreakIt->GetBreakIter()->getScriptType( aTxt, 0 );
            xub_StrLen nChg = 0;
            if( i18n::ScriptType::WEAK == nScript )
            {
                nChg =(xub_StrLen)g_pBreakIt->GetBreakIter()->endOfScript(aTxt,0,nScript);
                if( nChg < aTxt.getLength() )
                    nScript = g_pBreakIt->GetBreakIter()->getScriptType( aTxt, nChg );
            }

            //
            // nNextScriptChg will be evaluated during SwFldPortion::Format()
            //
            if ( nChg < aTxt.getLength() )
                nNextScriptChg = (xub_StrLen)g_pBreakIt->GetBreakIter()->endOfScript( aTxt, nChg, nScript );
            else
                nNextScriptChg = aTxt.getLength();

        }
        sal_uInt8 nTmp;
        switch ( nScript ) {
            case i18n::ScriptType::LATIN : nTmp = SW_LATIN; break;
            case i18n::ScriptType::ASIAN : nTmp = SW_CJK; break;
            case i18n::ScriptType::COMPLEX : nTmp = SW_CTL; break;
            default: nTmp = nActual;
        }

        // #i16354# Change script type for RTL text to CTL.
        const SwScriptInfo& rSI = rInf.GetParaPortion()->GetScriptInfo();
        // #i98418#
        const sal_uInt8 nFldDir = ( IsNumberPortion() || IsFtnNumPortion() ) ?
                             rSI.GetDefaultDir() :
                             rSI.DirType( IsFollow() ? rInf.GetIdx() - 1 : rInf.GetIdx() );

        bool bPerformUBA = UBIDI_LTR != nFldDir ? true : i18n::ScriptType::COMPLEX == nScript;
        if (bPerformUBA)
        {
            UErrorCode nError = U_ZERO_ERROR;
            UBiDi* pBidi = ubidi_openSized( aTxt.getLength(), 0, &nError );
            ubidi_setPara( pBidi, reinterpret_cast<const UChar *>(aTxt.getStr()), aTxt.getLength(), nFldDir, NULL, &nError );
            int32_t nEnd;
            UBiDiLevel nCurrDir;
            ubidi_getLogicalRun( pBidi, 0, &nEnd, &nCurrDir );
            ubidi_close( pBidi );
            const xub_StrLen nNextDirChg = (xub_StrLen)nEnd;
            nNextScriptChg = std::min( nNextScriptChg, nNextDirChg );

            // #i89825# change the script type also to CTL
            // if there is no strong LTR char in the LTR run (numbers)
            if ( nCurrDir != UBIDI_RTL )
            {
                nCurrDir = UBIDI_RTL;
                for ( xub_StrLen nCharIdx = 0; nCharIdx < nEnd; ++nCharIdx )
                {
                    UCharDirection nCharDir = u_charDirection ( aTxt[ nCharIdx ]);
                    if ( nCharDir == U_LEFT_TO_RIGHT ||
                         nCharDir == U_LEFT_TO_RIGHT_EMBEDDING ||
                         nCharDir == U_LEFT_TO_RIGHT_OVERRIDE )
                    {
                        nCurrDir = UBIDI_LTR;
                        break;
                    }
                }
            }

            if (nCurrDir == UBIDI_RTL)
            {
                nTmp = SW_CTL;
                //If we decided that this range was RTL after all and the
                //previous range was complex but clipped to the start of this
                //range, then extend it to be complex over the additional RTL
                //range
                if (nScript == i18n::ScriptType::COMPLEX)
                    nNextScriptChg = nNextDirChg;
            }
        }

        // #i98418#
        // keep determined script type for footnote portions as preferred script type.
        // For footnote portions a font can not be created directly - see footnote
        // portion format method.
        if ( IsFtnPortion() )
        {
            dynamic_cast<SwFtnPortion*>(this)->SetPreferredScriptType( nTmp );
        }
        else if ( nTmp != nActual )
        {
            if( !pFnt )
                pFnt = new SwFont( *rInf.GetFont() );
            pFnt->SetActual( nTmp );
        }
    }
}

sal_Bool SwFldPortion::Format( SwTxtFormatInfo &rInf )
{
    // Scope wegen aDiffTxt::DTOR!
    xub_StrLen nRest;
    sal_Bool bFull;
    bool bEOL = false;
    long nTxtRest = rInf.GetTxt().getLength() - rInf.GetIdx();
    {
        SwFldSlot aDiffTxt( &rInf, this );
        SwLayoutModeModifier aLayoutModeModifier( *rInf.GetOut() );
        aLayoutModeModifier.SetAuto();

        // Field portion has to be split in several parts if
        // 1. There are script/direction changes inside the field
        // 2. There are portion breaks (tab, break) inside the field:
        const xub_StrLen nOldFullLen = rInf.GetLen();
        xub_StrLen nFullLen = rInf.ScanPortionEnd( rInf.GetIdx(), rInf.GetIdx() + nOldFullLen ) - rInf.GetIdx();
        if ( nNextScriptChg < nFullLen )
        {
            nFullLen = nNextScriptChg;
            rInf.SetHookChar( 0 );
        }
        rInf.SetLen( nFullLen );

        if ( STRING_LEN != rInf.GetUnderScorePos() &&
             rInf.GetUnderScorePos() > rInf.GetIdx() )
             rInf.SetUnderScorePos( rInf.GetIdx() );

        if( pFnt )
            pFnt->GoMagic( rInf.GetVsh(), pFnt->GetActual() );

        SwFontSave aSave( rInf, pFnt );

        // 8674: Laenge muss 0 sein, bei bFull nach Format ist die Laenge
        // gesetzt und wird in nRest uebertragen. Ansonsten bleibt die
        // Laenge erhalten und wuerde auch in nRest einfliessen!
        SetLen(0);
           const MSHORT nFollow = IsFollow() ? 0 : 1;

        // So komisch es aussieht, die Abfrage auf GetLen() muss wegen der
        // ExpandPortions _hinter_ aDiffTxt (vgl. SoftHyphs)
        // sal_False returnen wegen SetFull ...
        if( !nFullLen )
        {
            // nicht Init(), weil wir Hoehe und Ascent brauchen
            Width(0);
            bFull = rInf.Width() <= rInf.GetPos().X();
        }
        else
        {
            xub_StrLen nOldLineStart = rInf.GetLineStart();
            if( IsFollow() )
                rInf.SetLineStart( 0 );
            rInf.SetNotEOL( nFullLen == nOldFullLen && nTxtRest > nFollow );

            // the height depending on the fields font is set,
            // this is required for SwTxtGuess::Guess
            Height( rInf.GetTxtHeight() + rInf.GetFont()->GetTopBorderSpace() +
                    rInf.GetFont()->GetBottomBorderSpace() );
            // If a kerning portion is inserted after our field portion,
            // the ascent and height must be known
            SetAscent( rInf.GetAscent() + rInf.GetFont()->GetTopBorderSpace() );
            bFull = SwTxtPortion::Format( rInf );
            rInf.SetNotEOL( false );
            rInf.SetLineStart( nOldLineStart );
        }
        xub_StrLen nTmpLen = GetLen();
        bEOL = !nTmpLen && nFollow && bFull;
        nRest = nOldFullLen - nTmpLen;

        // Das Zeichen wird in der ersten Portion gehalten.
        // Unbedingt nach Format!
        SetLen( (m_bNoLength) ? 0 : nFollow );

        if( nRest )
        {
            // aExpand ist noch nicht gekuerzt worden, der neue Ofst
            // ergibt sich durch nRest.
            sal_Int32 nNextOfst = aExpand.getLength() - nRest;

            if ( IsQuoVadisPortion() )
                nNextOfst = nNextOfst + ((SwQuoVadisPortion*)this)->GetContTxt().getLength();

            OUString aNew( aExpand.copy( nNextOfst ) );
            aExpand = aExpand.copy( 0, nNextOfst );

            // These characters should not be contained in the follow
            // field portion. They are handled via the HookChar mechanism.
            switch( aNew[0] )
            {
                case CH_BREAK  : bFull = sal_True;
                            // kein break;
                case ' ' :
                case CH_TAB    :
                case CHAR_HARDHYPHEN:               // non-breaking hyphen
                case CHAR_SOFTHYPHEN:
                case CHAR_HARDBLANK:
                // #i59759# Erase additional control
                // characters from field string, otherwise we get stuck in
                // a loop.
                case CHAR_ZWSP :
                case CHAR_ZWNBSP :
                // #i111750#
                // - Erasing further control characters from field string in
                // to avoid loop.
                case CH_TXTATR_BREAKWORD:
                case CH_TXTATR_INWORD:
                {
                    aNew = aNew.copy( 1 );
                    ++nNextOfst;
                    break;
                }
                default: ;
            }

            // Even if there is no more text left for a follow field,
            // we have to build a follow field portion (without font),
            // otherwise the HookChar mechanism would not work.
            SwFldPortion *pFld = Clone( aNew );
            if( !aNew.isEmpty() && !pFld->GetFont() )
            {
                SwFont *pNewFnt = new SwFont( *rInf.GetFont() );
                pFld->SetFont( pNewFnt );
            }
            pFld->SetFollow( sal_True );
            SetHasFollow( sal_True );
            // In nNextOffset steht bei einem neuangelegten Feld zunaechst
            // der Offset, an dem es selbst im Originalstring beginnt.
            // Wenn beim Formatieren ein FollowFeld angelegt wird, wird
            // der Offset dieses FollowFelds in nNextOffset festgehalten.
            nNextOffset = nNextOffset + nNextOfst;
            pFld->SetNextOffset( nNextOffset );
            rInf.SetRest( pFld );
        }
    }

    if( bEOL && rInf.GetLast() && !rInf.GetUnderFlow() )
        rInf.GetLast()->FormatEOL( rInf );
    return bFull;
}

/*************************************************************************
 *               virtual SwFldPortion::Paint()
 *************************************************************************/

void SwFldPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    SwFontSave aSave( rInf, pFnt );

    OSL_ENSURE( GetLen() <= 1, "SwFldPortion::Paint: rest-portion polution?" );
    if( Width() && ( !bPlaceHolder || rInf.GetOpt().IsShowPlaceHolderFields() ) )
    {
        // Dies ist eine freizuegige Auslegung der Hintergrundbelegung ...
        rInf.DrawViewOpt( *this, POR_FLD );
        SwExpandPortion::Paint( rInf );
    }
}

/*************************************************************************
 *              virtual SwFldPortion::GetExpTxt()
 *************************************************************************/

sal_Bool SwFldPortion::GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const
{
    rTxt = aExpand;
    if( rTxt.isEmpty() && rInf.OnWin() &&
        !rInf.GetOpt().IsPagePreview() && !rInf.GetOpt().IsReadonly() &&
            SwViewOption::IsFieldShadings() &&
            !HasFollow() )
        rTxt = OUString(' ');
    return sal_True;
}

/*************************************************************************
 *              virtual SwFldPortion::HandlePortion()
 *************************************************************************/

void SwFldPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    sal_Int32 nH = 0;
    if (pFnt)
        nH = pFnt->GetSize(pFnt->GetActual()).Height();
    rPH.Special( GetLen(), aExpand, GetWhichPor(), nH );
}

/*************************************************************************
 *                virtual SwFldPortion::GetTxtSize()
 *************************************************************************/

SwPosSize SwFldPortion::GetTxtSize( const SwTxtSizeInfo &rInf ) const
{
    SwFontSave aSave( rInf, pFnt );
    SwPosSize aSize( SwExpandPortion::GetTxtSize( rInf ) );
    return aSize;
}

/*************************************************************************
 *                      class SwHiddenPortion
 *************************************************************************/

SwFldPortion *SwHiddenPortion::Clone(const OUString &rExpand ) const
{
    SwFont *pNewFnt;
    if( 0 != ( pNewFnt = pFnt ) )
        pNewFnt = new SwFont( *pFnt );
    return new SwHiddenPortion( rExpand, pNewFnt );
}

/*************************************************************************
 *               virtual SwHiddenPortion::Paint()
 *************************************************************************/

void SwHiddenPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( Width() )
    {
        SwFontSave aSave( rInf, pFnt );
        rInf.DrawViewOpt( *this, POR_HIDDEN );
        SwExpandPortion::Paint( rInf );
    }
}

/*************************************************************************
 *              virtual SwHiddenPortion::GetExpTxt()
 *************************************************************************/

sal_Bool SwHiddenPortion::GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const
{
    // Nicht auf IsHidden() abfragen !
    return SwFldPortion::GetExpTxt( rInf, rTxt );
}

/*************************************************************************
 *                      class SwNumberPortion
 *************************************************************************/

SwNumberPortion::SwNumberPortion( const OUString &rExpand,
                                  SwFont *pFont,
                                  const sal_Bool bLft,
                                  const sal_Bool bCntr,
                                  const KSHORT nMinDst,
                                  const bool bLabelAlignmentPosAndSpaceModeActive )
        : SwFldPortion( rExpand, pFont ),
          nFixWidth(0),
          nMinDist( nMinDst ),
          mbLabelAlignmentPosAndSpaceModeActive( bLabelAlignmentPosAndSpaceModeActive )
{
    SetWhichPor( POR_NUMBER );
    SetLeft( bLft );
    SetHide( sal_False );
    SetCenter( bCntr );
}

xub_StrLen SwNumberPortion::GetCrsrOfst( const MSHORT ) const
{
    return 0;
}

SwFldPortion *SwNumberPortion::Clone( const OUString &rExpand ) const
{
    SwFont *pNewFnt;
    if( 0 != ( pNewFnt = pFnt ) )
        pNewFnt = new SwFont( *pFnt );

    return new SwNumberPortion( rExpand, pNewFnt, IsLeft(), IsCenter(),
                                nMinDist, mbLabelAlignmentPosAndSpaceModeActive );
}

/*************************************************************************
 *                 virtual SwNumberPortion::Format()
 *************************************************************************/

// 5010: Wir sind in der Lage, mehrzeilige NumFelder anzulegen!
// 3689: Fies ist, wenn man in der Dialogbox soviel Davor-Text
// eingibt, bis die Zeile ueberlaeuft.
// Man muss die Fly-Ausweichmanoever beachten!

sal_Bool SwNumberPortion::Format( SwTxtFormatInfo &rInf )
{
    SetHide( sal_False );
    const sal_Bool bFull = SwFldPortion::Format( rInf );
    SetLen( 0 );
    // a numbering portion can be contained in a rotated portion!!!
    nFixWidth = rInf.IsMulti() ? Height() : Width();
    rInf.SetNumDone( !rInf.GetRest() );
    if( rInf.IsNumDone() )
    {
//        SetAscent( rInf.GetAscent() );
        OSL_ENSURE( Height() && nAscent, "NumberPortions without Height | Ascent" );

        long nDiff( 0 );

        if ( !mbLabelAlignmentPosAndSpaceModeActive )
        {
            if ( !rInf.GetTxtFrm()->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING) &&
                 // #i32902#
                 !IsFtnNumPortion() )
            {
                nDiff = rInf.Left()
                    + rInf.GetTxtFrm()->GetTxtNode()->
                    GetSwAttrSet().GetLRSpace().GetTxtFirstLineOfst()
                    - rInf.First()
                    + rInf.ForcedLeftMargin();
            }
            else
            {
                nDiff = rInf.Left() - rInf.First() + rInf.ForcedLeftMargin();
            }
        }
        // Ein Vorschlag von Juergen und Volkmar:
        // Der Textteil hinter der Numerierung sollte immer
        // mindestens beim linken Rand beginnen.
        if( nDiff < 0 )
            nDiff = 0;
        else if ( nDiff > rInf.X() )
            nDiff -= rInf.X();
        else
            nDiff = 0;

        if( nDiff < nFixWidth + nMinDist )
            nDiff = nFixWidth + nMinDist;
        // 2739: Numerierung weicht Fly aus, kein nDiff in der zweiten Runde
        // fieser Sonderfall: FlyFrm liegt in dem Bereich,
        // den wir uns gerade unter den Nagel reissen wollen.
        // Die NumberPortion wird als verborgen markiert.
        const bool bFly = rInf.GetFly() ||
            ( rInf.GetLast() && rInf.GetLast()->IsFlyPortion() );
        if( nDiff > rInf.Width() )
        {
            nDiff = rInf.Width();
            if ( bFly )
                SetHide( sal_True );
        }

        // A numbering portion can be inside a SwRotatedPortion. Then the
        // Height has to be changed
        if ( rInf.IsMulti() )
        {
            if ( Height() < nDiff )
                Height( KSHORT( nDiff ) );
        }
        else if( Width() < nDiff )
            Width( KSHORT(nDiff) );
    }
    return bFull;
}

void SwNumberPortion::FormatEOL( SwTxtFormatInfo& )
{
/*  Ein FormatEOL deutet daraufhin, dass der folgende Text
 *  nicht mit auf die Zeile passte. Damit die Numerierung mitwandert,
 *  wird diese NumberPortion verborgen.
 */

    // This caused trouble with flys anchored as characters.
    // If one of these is numbered but does not fit to the line,
    // it calls this function, causing a loop because both the number
    // portion and the fly portion go to the next line
//    SetHide( sal_True );
}

/*************************************************************************
 *               virtual SwNumberPortion::Paint()
 *************************************************************************/

void SwNumberPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
/*  Eine verborgene NumberPortion wird nicht angezeigt, es sei denn, es gibt
 *  Textportions in dieser Zeile oder es gibt ueberhaupt nur eine einzige Zeile.
 */

    if ( IsHide() && rInf.GetParaPortion() && rInf.GetParaPortion()->GetNext() )
    {
        SwLinePortion *pTmp = GetPortion();
        while ( pTmp && !pTmp->InTxtGrp() )
            pTmp = pTmp->GetPortion();
        if ( !pTmp )
            return;
    }

    // calculate the width of the number portion, including follows
    const KSHORT nOldWidth = Width();
    sal_uInt16 nSumWidth = 0;
    sal_uInt16 nOffset = 0;

    const SwLinePortion* pTmp = this;
    while ( pTmp && pTmp->InNumberGrp() )
    {
        nSumWidth = nSumWidth + pTmp->Width();
        if ( ((SwNumberPortion*)pTmp)->HasFollow() )
            pTmp = pTmp->GetPortion();
        else
        {
            nOffset = pTmp->Width() - ((SwNumberPortion*)pTmp)->nFixWidth;
            break;
        }
    }

    // The master portion takes care for painting the background of the
    // follow field portions
    if ( ! IsFollow() )
    {
        SwLinePortion *pThis = (SwLinePortion*)this;
        pThis->Width( nSumWidth );
        rInf.DrawViewOpt( *this, POR_NUMBER );
        pThis->Width( nOldWidth );
    }

    if( !aExpand.isEmpty() )
    {
        const SwFont *pTmpFnt = rInf.GetFont();
        bool bPaintSpace = ( UNDERLINE_NONE != pTmpFnt->GetUnderline() ||
                                 UNDERLINE_NONE != pTmpFnt->GetOverline()  ||
                                 STRIKEOUT_NONE != pTmpFnt->GetStrikeout() ) &&
                                 !pTmpFnt->IsWordLineMode();
        if( bPaintSpace && pFnt )
            bPaintSpace = ( UNDERLINE_NONE != pFnt->GetUnderline() ||
                            UNDERLINE_NONE != pFnt->GetOverline()  ||
                            STRIKEOUT_NONE != pFnt->GetStrikeout() ) &&
                            !pFnt->IsWordLineMode();

        SwFontSave aSave( rInf, pFnt );

        if( nFixWidth == Width() && ! HasFollow() )
            SwExpandPortion::Paint( rInf );
        else
        {
            // logisches const: Width wird wieder zurueckgesetzt
            SwLinePortion *pThis = (SwLinePortion*)this;
            bPaintSpace = bPaintSpace && nFixWidth < nOldWidth;
            KSHORT nSpaceOffs = nFixWidth;
            pThis->Width( nFixWidth );

            if( ( IsLeft() && ! rInf.GetTxtFrm()->IsRightToLeft() ) ||
                ( ! IsLeft() && ! IsCenter() && rInf.GetTxtFrm()->IsRightToLeft() ) )
                SwExpandPortion::Paint( rInf );
            else
            {
                SwTxtPaintInfo aInf( rInf );
                if( nOffset < nMinDist )
                    nOffset = 0;
                else
                {
                    if( IsCenter() )
                    {
                        /* #110778# a / 2 * 2 == a is not a tautology */
                        KSHORT nTmpOffset = nOffset;
                        nOffset /= 2;
                        if( nOffset < nMinDist )
                            nOffset = nTmpOffset - nMinDist;
                    }
                    else
                        nOffset = nOffset - nMinDist;
                }
                aInf.X( aInf.X() + nOffset );
                SwExpandPortion::Paint( aInf );
                if( bPaintSpace )
                    nSpaceOffs = nSpaceOffs + nOffset;
            }
            if( bPaintSpace && nOldWidth > nSpaceOffs )
            {
                SwTxtPaintInfo aInf( rInf );
static sal_Char const sDoubleSpace[] = "  ";
                aInf.X( aInf.X() + nSpaceOffs );

                // #i53199# Adjust position of underline:
                if ( rInf.GetUnderFnt() )
                {
                    const Point aNewPos( aInf.GetPos().X(), rInf.GetUnderFnt()->GetPos().Y() );
                    rInf.GetUnderFnt()->SetPos( aNewPos );
                }

                pThis->Width( nOldWidth - nSpaceOffs + 12 );
                {
                    SwTxtSlot aDiffTxt( &aInf, this, true, false, sDoubleSpace );
                    aInf.DrawText( *this, aInf.GetLen(), sal_True );
                }
            }
            pThis->Width( nOldWidth );
        }
    }
}


/*************************************************************************
 *                      class SwBulletPortion
 *************************************************************************/

SwBulletPortion::SwBulletPortion( const sal_Unicode cBullet,
                                  const OUString& rBulletFollowedBy,
                                  SwFont *pFont,
                                  const sal_Bool bLft,
                                  const sal_Bool bCntr,
                                  const KSHORT nMinDst,
                                  const bool bLabelAlignmentPosAndSpaceModeActive )
    : SwNumberPortion( OUString(cBullet) + rBulletFollowedBy,
                       pFont, bLft, bCntr, nMinDst,
                       bLabelAlignmentPosAndSpaceModeActive )
{
    SetWhichPor( POR_BULLET );
}

/*************************************************************************
 *                      class SwGrfNumPortion
 *************************************************************************/

#define GRFNUM_SECURE 10

SwGrfNumPortion::SwGrfNumPortion(
        SwFrm*,
        const OUString& rGraphicFollowedBy,
        const SvxBrushItem* pGrfBrush,
        const SwFmtVertOrient* pGrfOrient, const Size& rGrfSize,
        const sal_Bool bLft, const sal_Bool bCntr, const KSHORT nMinDst,
        const bool bLabelAlignmentPosAndSpaceModeActive ) :
    SwNumberPortion( rGraphicFollowedBy, NULL, bLft, bCntr, nMinDst,
                     bLabelAlignmentPosAndSpaceModeActive ),
    pBrush( new SvxBrushItem(RES_BACKGROUND) ), nId( 0 )
{
    SetWhichPor( POR_GRFNUM );
    SetAnimated( sal_False );
    bReplace = sal_False;
    if( pGrfBrush )
    {
        *pBrush = *pGrfBrush;
        const Graphic* pGraph = pGrfBrush->GetGraphic();
        if( pGraph )
            SetAnimated( pGraph->IsAnimated() );
        else
            bReplace = sal_True;
    }
    if( pGrfOrient )
    {
        nYPos = pGrfOrient->GetPos();
        eOrient = pGrfOrient->GetVertOrient();
    }
    else
    {
        nYPos = 0;
        eOrient = text::VertOrientation::TOP;
    }
    Width( static_cast<sal_uInt16>(rGrfSize.Width() + 2 * GRFNUM_SECURE) );
    nFixWidth = Width();
    nGrfHeight = rGrfSize.Height() + 2 * GRFNUM_SECURE;
    Height( KSHORT(nGrfHeight) );
    bNoPaint = sal_False;
}

SwGrfNumPortion::~SwGrfNumPortion()
{
    if ( IsAnimated() )
        ( (Graphic*) pBrush->GetGraphic() )->StopAnimation( 0, nId );
    delete pBrush;
}

void SwGrfNumPortion::StopAnimation( OutputDevice* pOut )
{
    if ( IsAnimated() )
        ( (Graphic*) pBrush->GetGraphic() )->StopAnimation( pOut, nId );
}

sal_Bool SwGrfNumPortion::Format( SwTxtFormatInfo &rInf )
{
    SetHide( sal_False );
//    Width( nFixWidth );
    KSHORT nFollowedByWidth( 0 );
    if ( mbLabelAlignmentPosAndSpaceModeActive )
    {
        SwFldPortion::Format( rInf );
        nFollowedByWidth = Width();
        SetLen( 0 );
    }
    Width( nFixWidth + nFollowedByWidth );
    const sal_Bool bFull = rInf.Width() < rInf.X() + Width();
    const bool bFly = rInf.GetFly() ||
        ( rInf.GetLast() && rInf.GetLast()->IsFlyPortion() );
    SetAscent( static_cast<sal_uInt16>(GetRelPos() > 0 ? GetRelPos() : 0) );
    if( GetAscent() > Height() )
        Height( GetAscent() );

    if( bFull )
    {
        Width( rInf.Width() - (KSHORT)rInf.X() );
        if( bFly )
        {
            SetLen( 0 );
            SetNoPaint( sal_True );
            rInf.SetNumDone( sal_False );
            return sal_True;
        }
    }
    rInf.SetNumDone( sal_True );
//    long nDiff = rInf.Left() - rInf.First() + rInf.ForcedLeftMargin();
    long nDiff = mbLabelAlignmentPosAndSpaceModeActive
                 ? 0
                 : rInf.Left() - rInf.First() + rInf.ForcedLeftMargin();
    // Ein Vorschlag von Juergen und Volkmar:
    // Der Textteil hinter der Numerierung sollte immer
    // mindestens beim linken Rand beginnen.
    if( nDiff < 0 )
        nDiff = 0;
    else if ( nDiff > rInf.X() )
        nDiff -= rInf.X();
    if( nDiff < nFixWidth + nMinDist )
        nDiff = nFixWidth + nMinDist;
    // 2739: Numerierung weicht Fly aus, kein nDiff in der zweiten Runde
    // fieser Sonderfall: FlyFrm liegt in dem Bereich,
    // den wir uns gerade unter den Nagel reissen wollen.
    // Die NumberPortion wird als verborgen markiert.
    if( nDiff > rInf.Width() )
    {
        nDiff = rInf.Width();
        if( bFly )
            SetHide( sal_True );
    }

    if( Width() < nDiff )
        Width( KSHORT(nDiff) );
    return bFull;
}

void SwGrfNumPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( DontPaint() )
        return;
/*  Eine verborgene NumberPortion wird nicht angezeigt, es sei denn, es gibt
 *  Textportions in dieser Zeile oder es gibt ueberhaupt nur eine einzige Zeile.
 */
    if ( IsHide() && rInf.GetParaPortion() && rInf.GetParaPortion()->GetNext() )
    {
        SwLinePortion *pTmp = GetPortion();
        while ( pTmp && !pTmp->InTxtGrp() )
            pTmp = pTmp->GetPortion();
        if ( !pTmp )
            return;
    }
    Point aPos( rInf.X() + GRFNUM_SECURE, rInf.Y() - GetRelPos() + GRFNUM_SECURE );
    long nTmpWidth = std::max( (long)0, (long)(nFixWidth - 2 * GRFNUM_SECURE) );
    Size aSize( nTmpWidth, GetGrfHeight() - 2 * GRFNUM_SECURE );

    const bool bTmpLeft = mbLabelAlignmentPosAndSpaceModeActive ||
                              ( IsLeft() && ! rInf.GetTxtFrm()->IsRightToLeft() ) ||
                              ( ! IsLeft() && ! IsCenter() && rInf.GetTxtFrm()->IsRightToLeft() );

    if( nFixWidth < Width() && !bTmpLeft )
    {
        KSHORT nOffset = Width() - nFixWidth;
        if( nOffset < nMinDist )
            nOffset = 0;
        else
        {
            if( IsCenter() )
            {
                nOffset /= 2;
                if( nOffset < nMinDist )
                    nOffset = Width() - nFixWidth - nMinDist;
            }
            else
                nOffset = nOffset - nMinDist;
        }
        aPos.X() += nOffset;
    }

    if( bReplace )
    {
        KSHORT nTmpH = GetPortion() ? GetPortion()->GetAscent() : 120;
        aSize = Size( nTmpH, nTmpH );
        aPos.Y() = rInf.Y() - nTmpH;
    }
    SwRect aTmp( aPos, aSize );

    bool bDraw = true;

    if ( IsAnimated() )
    {
        bDraw = !rInf.GetOpt().IsGraphic();
        if( !nId )
        {
            SetId( sal_IntPtr( rInf.GetTxtFrm() ) );
            rInf.GetTxtFrm()->SetAnimation();
        }
        if( aTmp.IsOver( rInf.GetPaintRect() ) && !bDraw )
        {
            rInf.NoteAnimation();
            const ViewShell* pViewShell = rInf.GetVsh();

            // virtual device, not pdf export
            if( OUTDEV_VIRDEV == rInf.GetOut()->GetOutDevType() &&
                pViewShell && pViewShell->GetWin()  )
            {
                ( (Graphic*) pBrush->GetGraphic() )->StopAnimation(0,nId);
                rInf.GetTxtFrm()->getRootFrm()->GetCurrShell()->InvalidateWindows( aTmp );
            }


            else if ( pViewShell &&
                     !pViewShell->GetAccessibilityOptions()->IsStopAnimatedGraphics() &&
                     !pViewShell->IsPreView() &&
                      // #i9684# Stop animation during printing/pdf export.
                      pViewShell->GetWin() )
            {
                ( (Graphic*) pBrush->GetGraphic() )->StartAnimation(
                    (OutputDevice*)rInf.GetOut(), aPos, aSize, nId );
            }

            // pdf export, printing, preview, stop animations...
            else
                bDraw = true;
        }
        if( bDraw )
            ( (Graphic*) pBrush->GetGraphic() )->StopAnimation( 0, nId );
    }

    SwRect aRepaint( rInf.GetPaintRect() );
    const SwTxtFrm& rFrm = *rInf.GetTxtFrm();
    if( rFrm.IsVertical() )
    {
        rFrm.SwitchHorizontalToVertical( aTmp );
        rFrm.SwitchHorizontalToVertical( aRepaint );
    }

    if( rFrm.IsRightToLeft() )
    {
        rFrm.SwitchLTRtoRTL( aTmp );
        rFrm.SwitchLTRtoRTL( aRepaint );
    }

    if( bDraw && aTmp.HasArea() )
        DrawGraphic( pBrush, 0, 0, (OutputDevice*)rInf.GetOut(),
            aTmp, aRepaint, bReplace ? GRFNUM_REPLACE : GRFNUM_YES );
}

void SwGrfNumPortion::SetBase( long nLnAscent, long nLnDescent,
                               long nFlyAsc, long nFlyDesc )
{
    if ( GetOrient() != text::VertOrientation::NONE )
    {
        SetRelPos( 0 );
        if ( GetOrient() == text::VertOrientation::CENTER )
            SetRelPos( GetGrfHeight() / 2 );
        else if ( GetOrient() == text::VertOrientation::TOP )
            SetRelPos( GetGrfHeight() - GRFNUM_SECURE );
        else if ( GetOrient() == text::VertOrientation::BOTTOM )
            ;
        else if ( GetOrient() == text::VertOrientation::CHAR_CENTER )
            SetRelPos( ( GetGrfHeight() + nLnAscent - nLnDescent ) / 2 );
        else if ( GetOrient() == text::VertOrientation::CHAR_TOP )
            SetRelPos( nLnAscent );
        else if ( GetOrient() == text::VertOrientation::CHAR_BOTTOM )
            SetRelPos( GetGrfHeight() - nLnDescent );
        else
        {
            if( GetGrfHeight() >= nFlyAsc + nFlyDesc )
            {
                // wenn ich genauso gross bin wie die Zeile, brauche ich mich
                // nicht an der Zeile nicht weiter ausrichten, ich lasse
                // dann auch den max. Ascent der Zeile unveraendert

                SetRelPos( nFlyAsc );
            }
            else if ( GetOrient() == text::VertOrientation::LINE_CENTER )
                SetRelPos( ( GetGrfHeight() + nFlyAsc - nFlyDesc ) / 2 );
            else if ( GetOrient() == text::VertOrientation::LINE_TOP )
                SetRelPos( nFlyAsc );
            else if ( GetOrient() == text::VertOrientation::LINE_BOTTOM )
                SetRelPos( GetGrfHeight() - nFlyDesc );
        }
    }
}

void SwTxtFrm::StopAnimation( OutputDevice* pOut )
{
    OSL_ENSURE( HasAnimation(), "SwTxtFrm::StopAnimation: Which Animation?" );
    if( HasPara() )
    {
        SwLineLayout *pLine = GetPara();
        while( pLine )
        {
            SwLinePortion *pPor = pLine->GetPortion();
            while( pPor )
            {
                if( pPor->IsGrfNumPortion() )
                    ((SwGrfNumPortion*)pPor)->StopAnimation( pOut );
                // Die Numerierungsportion sitzt immer vor dem ersten Zeichen,
                // deshalb koennen wir abbrechen, sobald wir eine Portion mit
                // einer Laenge > 0 erreicht haben.
                pPor = pPor->GetLen() ? 0 : pPor->GetPortion();
            }
            pLine = pLine->GetLen() ? 0 : pLine->GetNext();
        }
    }
}

/*************************************************************************
 * SwCombinedPortion::SwCombinedPortion(..)
 * initializes the script array and clears the width array
 *************************************************************************/

SwCombinedPortion::SwCombinedPortion( const OUString &rTxt )
     : SwFldPortion( rTxt )
{
    SetLen(1);
    SetWhichPor( POR_COMBINED );
    if( aExpand.getLength() > 6 )
        aExpand = aExpand.copy( 0, 6 );
    // Initialization of the scripttype array,
    // the arrays of width and position are filled by the format function
    if( g_pBreakIt->GetBreakIter().is() )
    {
        sal_uInt8 nScr = SW_SCRIPTS;
        for( sal_Int32 i = 0; i < rTxt.getLength(); ++i )
        {
            sal_uInt16 nScript = g_pBreakIt->GetBreakIter()->getScriptType( rTxt, i );
            switch ( nScript ) {
                case i18n::ScriptType::LATIN : nScr = SW_LATIN; break;
                case i18n::ScriptType::ASIAN : nScr = SW_CJK; break;
                case i18n::ScriptType::COMPLEX : nScr = SW_CTL; break;
            }
            aScrType[i] = nScr;
        }
    }
    else
    {
        for( sal_uInt16 i = 0; i < 6; ++i )
            aScrType[i] = 0;
    }
    memset( &aWidth, 0, sizeof(aWidth) );
}

/*************************************************************************
 * SwCombinedPortion::Paint(..)
 *************************************************************************/

void SwCombinedPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    OSL_ENSURE( GetLen() <= 1, "SwFldPortion::Paint: rest-portion polution?" );
    if( Width() )
    {
        rInf.DrawBackBrush( *this );
        rInf.DrawViewOpt( *this, POR_FLD );

        // do we have to repaint a post it portion?
        if( rInf.OnWin() && pPortion && !pPortion->Width() )
            pPortion->PrePaint( rInf, this );

        sal_Int32 nCount = aExpand.getLength();
        if( !nCount )
            return;
        OSL_ENSURE( nCount < 7, "Too much combined characters" );

        // the first character of the second row
        sal_uInt16 nTop = ( nCount + 1 ) / 2;

        SwFont aTmpFont( *rInf.GetFont() );
        aTmpFont.SetProportion( nProportion );  // a smaller font
        SwFontSave aFontSave( rInf, &aTmpFont );

        sal_uInt16 i = 0;
        Point aOldPos = rInf.GetPos();
        Point aOutPos( aOldPos.X(), aOldPos.Y() - nUpPos );// Y of the first row
        while( i < nCount )
        {
            if( i == nTop ) // change the row
                aOutPos.Y() = aOldPos.Y() + nLowPos;    // Y of the second row
            aOutPos.X() = aOldPos.X() + aPos[i];        // X position
            const sal_uInt8 nAct = aScrType[i];             // script type
            aTmpFont.SetActual( nAct );
            // if there're more than 4 characters to display, we choose fonts
            // with 2/3 of the original font width.
            if( aWidth[ nAct ] )
            {
                Size aTmpSz = aTmpFont.GetSize( nAct );
                if( aTmpSz.Width() != aWidth[ nAct ] )
                {
                    aTmpSz.Width() = aWidth[ nAct ];
                    aTmpFont.SetSize( aTmpSz, nAct );
                }
            }
            ((SwTxtPaintInfo&)rInf).SetPos( aOutPos );
            rInf.DrawText( aExpand, *this, i, 1 );
            ++i;
        }
        // rInf is const, so we have to take back our manipulations
        ((SwTxtPaintInfo&)rInf).SetPos( aOldPos );
    }
}

/*************************************************************************
 * SwCombinedPortion::Format(..)
 *************************************************************************/

sal_Bool SwCombinedPortion::Format( SwTxtFormatInfo &rInf )
{
    sal_Int32 nCount = aExpand.getLength();
    if( !nCount )
    {
        Width( 0 );
        return sal_False;
    }

    OSL_ENSURE( nCount < 7, "Too much combined characters" );
    // If there are leading "weak"-scripttyped characters in this portion,
    // they get the actual scripttype.
    sal_uInt16 i = 0;
    while( i < nCount && SW_SCRIPTS == aScrType[i] )
        aScrType[i++] = rInf.GetFont()->GetActual();
    if( nCount > 4 )
    {
        // more than four? Ok, then we need the 2/3 font width
        i = 0;
        while( i < aExpand.getLength() )
        {
            OSL_ENSURE( aScrType[i] < SW_SCRIPTS, "Combined: Script fault" );
            if( !aWidth[ aScrType[i] ] )
            {
                rInf.GetOut()->SetFont( rInf.GetFont()->GetFnt( aScrType[i] ) );
                aWidth[ aScrType[i] ] =
                        static_cast<sal_uInt16>(2 * rInf.GetOut()->GetFontMetric().GetSize().Width() / 3);
            }
            ++i;
        }
    }

    sal_uInt16 nTop = ( nCount + 1 ) / 2; // the first character of the second line
    ViewShell *pSh = rInf.GetTxtFrm()->getRootFrm()->GetCurrShell();
    SwFont aTmpFont( *rInf.GetFont() );
    SwFontSave aFontSave( rInf, &aTmpFont );
    nProportion = 55;
    // In nMainAscent/Descent we store the ascent and descent
    // of the original surrounding font
    sal_uInt16 nMaxDescent, nMaxAscent, nMaxWidth;
    sal_uInt16 nMainDescent = rInf.GetFont()->GetHeight( pSh, *rInf.GetOut() );
    const sal_uInt16 nMainAscent = rInf.GetFont()->GetAscent( pSh, *rInf.GetOut() );
    nMainDescent = nMainDescent - nMainAscent;
    // we start with a 50% font, but if we notice that the combined portion
    // becomes bigger than the surrounding font, we check 45% and maybe 40%.
    do
    {
        nProportion -= 5;
        aTmpFont.SetProportion( nProportion );
        i = 0;
        memset( &aPos, 0, sizeof(aPos) );
        nMaxDescent = 0;
        nMaxAscent = 0;
        nMaxWidth = 0;
        nUpPos = nLowPos = 0;

        // Now we get the width of all characters.
        // The ascent and the width of the first line are stored in the
        // ascent member of the portion, the descent in nLowPos.
        // The ascent, descent and width of the second line are stored in the
        // local nMaxAscent, nMaxDescent and nMaxWidth variables.
        while( i < nCount )
        {
            sal_uInt8 nScrp = aScrType[i];
            aTmpFont.SetActual( nScrp );
            if( aWidth[ nScrp ] )
            {
                Size aFontSize( aTmpFont.GetSize( nScrp ) );
                aFontSize.Width() = aWidth[ nScrp ];
                aTmpFont.SetSize( aFontSize, nScrp );
            }

            SwDrawTextInfo aDrawInf( pSh, *rInf.GetOut(), 0, aExpand, i, 1 );
            Size aSize = aTmpFont._GetTxtSize( aDrawInf );
            sal_uInt16 nAsc = aTmpFont.GetAscent( pSh, *rInf.GetOut() );
            aPos[ i ] = (sal_uInt16)aSize.Width();
            if( i == nTop ) // enter the second line
            {
                nLowPos = nMaxDescent;
                Height( nMaxDescent + nMaxAscent );
                Width( nMaxWidth );
                SetAscent( nMaxAscent );
                nMaxAscent = 0;
                nMaxDescent = 0;
                nMaxWidth = 0;
            }
            nMaxWidth = nMaxWidth + aPos[ i++ ];
            if( nAsc > nMaxAscent )
                nMaxAscent = nAsc;
            if( aSize.Height() - nAsc > nMaxDescent )
                nMaxDescent = static_cast<sal_uInt16>(aSize.Height() - nAsc);
        }
        // for one or two characters we double the width of the portion
        if( nCount < 3 )
        {
            nMaxWidth *= 2;
            Width( 2*Width() );
            if( nCount < 2 )
            {
                Height( nMaxAscent + nMaxDescent );
                nLowPos = nMaxDescent;
            }
        }
        Height( Height() + nMaxDescent + nMaxAscent );
        nUpPos = nMaxAscent;
        SetAscent( Height() - nMaxDescent - nLowPos );
    } while( nProportion > 40 && ( GetAscent() > nMainAscent ||
                                    Height() - GetAscent() > nMainDescent ) );
    // if the combined portion is smaller than the surrounding text,
    // the portion grows. This looks better, if there's a character background.
    if( GetAscent() < nMainAscent )
    {
        Height( Height() + nMainAscent - GetAscent() );
        SetAscent( nMainAscent );
    }
    if( Height() < nMainAscent + nMainDescent )
        Height( nMainAscent + nMainDescent );

    // We calculate the x positions of the characters in both lines..
    sal_uInt16 nTopDiff = 0;
    sal_uInt16 nBotDiff = 0;
    if( nMaxWidth > Width() )
    {
        nTopDiff = ( nMaxWidth - Width() ) / 2;
        Width( nMaxWidth );
    }
    else
        nBotDiff = ( Width() - nMaxWidth ) / 2;
    switch( nTop)
    {
        case 3: aPos[1] = aPos[0] + nTopDiff;  // no break
        case 2: aPos[nTop-1] = Width() - aPos[nTop-1];
    }
    aPos[0] = 0;
    switch( nCount )
    {
        case 5: aPos[4] = aPos[3] + nBotDiff;   // no break
        case 3: aPos[nTop] = nBotDiff;          break;
        case 6: aPos[4] = aPos[3] + nBotDiff;   // no break
        case 4: aPos[nTop] = 0;                 // no break
        case 2: aPos[nCount-1] = Width() - aPos[nCount-1];
    }

    // Does the combined portion fit the line?
    const sal_Bool bFull = rInf.Width() < rInf.X() + Width();
    if( bFull )
    {
        if( rInf.GetLineStart() == rInf.GetIdx() && (!rInf.GetLast()->InFldGrp()
            || !((SwFldPortion*)rInf.GetLast())->IsFollow() ) )
            Width( (sal_uInt16)( rInf.Width() - rInf.X() ) );
        else
        {
            Truncate();
            Width( 0 );
            SetLen( 0 );
            if( rInf.GetLast() )
                rInf.GetLast()->FormatEOL( rInf );
        }
    }
    return bFull;
}

/*************************************************************************
 * SwCombinedPortion::GetViewWidth(..)
 *************************************************************************/

KSHORT SwCombinedPortion::GetViewWidth( const SwTxtSizeInfo &rInf ) const
{
    if( !GetLen() ) // for the dummy part at the end of the line, where
        return 0;   // the combined portion doesn't fit.
    return SwFldPortion::GetViewWidth( rInf );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
