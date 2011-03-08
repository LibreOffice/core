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
#include "precompiled_sw.hxx"


#include "hintids.hxx"
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <charfmt.hxx>

#include "viewsh.hxx"   // NewFldPortion, GetDoc()
#include "doc.hxx"      // NewFldPortion, GetSysFldType()
#include "rootfrm.hxx"  // Info ueber virt. PageNumber
#include "pagefrm.hxx"  // NewFldPortion, GetVirtPageNum()
#include "ndtxt.hxx"    // NewNumberPortion, pHints->GetNum()
#include "fldbas.hxx"      // SwField
#include "viewopt.hxx"  // SwViewOptions
#include "flyfrm.hxx"   //IsInBody()
#include "viewimp.hxx"
#include "txtatr.hxx"   // SwTxtFld
#include "swfont.hxx"   // NewFldPortion, new SwFont
#include "fntcache.hxx"   // NewFldPortion, SwFntAccess
#include "porfld.hxx"
#include "porftn.hxx"   // NewExtraPortion
#include "porref.hxx"   // NewExtraPortion
#include "portox.hxx"   // NewExtraPortion
#include "porhyph.hxx"   // NewExtraPortion
#include "porfly.hxx"   // NewExtraPortion
#include "itrform2.hxx"   // SwTxtFormatter
#include "chpfld.hxx"
#include "dbfld.hxx"
#include "expfld.hxx"
#include "docufld.hxx"
#include "pagedesc.hxx"  // NewFldPortion, GetNum()
#include <pormulti.hxx>     // SwMultiPortion
#include "fmtmeta.hxx" // lcl_NewMetaPortion


/*************************************************************************
 *                      SwTxtFormatter::NewFldPortion()
 *************************************************************************/


sal_Bool lcl_IsInBody( SwFrm *pFrm )
{
    if ( pFrm->IsInDocBody() )
        return sal_True;
    else
    {
        const SwFrm *pTmp = pFrm;
        const SwFlyFrm *pFly;
        while ( 0 != (pFly = pTmp->FindFlyFrm()) )
            pTmp = pFly->GetAnchorFrm();
        return pTmp->IsInDocBody();
    }
}


SwExpandPortion *SwTxtFormatter::NewFldPortion( SwTxtFormatInfo &rInf,
                                                const SwTxtAttr *pHint ) const
{
    SwExpandPortion *pRet = 0;
    SwFrm *pFrame = (SwFrm*)pFrm;
    SwField *pFld = (SwField*)pHint->GetFld().GetFld();
    const sal_Bool bName = rInf.GetOpt().IsFldName();

    SwCharFmt* pChFmt = 0;
    sal_Bool bNewFlyPor = sal_False,
         bINet = sal_False;

    // set language
    ((SwTxtFormatter*)this)->SeekAndChg( rInf );
    if (pFld->GetLanguage() != GetFnt()->GetLanguage())
    {
        pFld->SetLanguage( GetFnt()->GetLanguage() );
        // let the visual note know about its new language
        if (pFld->GetTyp()->Which()==RES_POSTITFLD)
            const_cast<SwFmtFld*> (&pHint->GetFld())->Broadcast( SwFmtFldHint( &pHint->GetFld(), SWFMTFLD_LANGUAGE ) );
    }

    ViewShell *pSh = rInf.GetVsh();
    sal_Bool bPlaceHolder = sal_False;

    switch( pFld->GetTyp()->Which() )
    {
        case RES_SCRIPTFLD:
        case RES_POSTITFLD:
            pRet = new SwPostItsPortion( RES_SCRIPTFLD == pFld->GetTyp()->Which() );
            break;

        case RES_COMBINED_CHARS:
            {
                String sStr( pFld->GetCntnt( bName ));
                if( bName )
                    pRet = new SwFldPortion( sStr );
                else
                    pRet = new SwCombinedPortion( sStr );
            }
            break;

        case RES_HIDDENTXTFLD:
            pRet = new SwHiddenPortion(pFld->GetCntnt( bName ));
            break;

        case RES_CHAPTERFLD:
            if( !bName && pSh && !pSh->Imp()->IsUpdateExpFlds() )
            {
                ((SwChapterField*)pFld)->ChangeExpansion( pFrame,
                                        &((SwTxtFld*)pHint)->GetTxtNode() );
            }
            pRet = new SwFldPortion( pFld->GetCntnt( bName ) );
            break;

        case RES_DOCSTATFLD:
            if( !bName && pSh && !pSh->Imp()->IsUpdateExpFlds() )
                ((SwDocStatField*)pFld)->ChangeExpansion( pFrame );
            pRet = new SwFldPortion( pFld->GetCntnt( bName ) );
            break;

        case RES_PAGENUMBERFLD:
        {
            if( !bName && pSh && !pSh->Imp()->IsUpdateExpFlds() )
            {
                SwPageNumberFieldType *pPageNr = (SwPageNumberFieldType *)pFld->GetTyp();

                const SwRootFrm* pTmpRootFrm = pSh->GetLayout();
                const sal_Bool bVirt = pTmpRootFrm->IsVirtPageNum();

                SwDoc* pDoc = pSh->GetDoc();
                MSHORT nVirtNum = pFrame->GetVirtPageNum();
                MSHORT nNumPages = pTmpRootFrm->GetPageNum();
                sal_Int16 nNumFmt = -1;
                if(SVX_NUM_PAGEDESC == pFld->GetFormat())
                    nNumFmt = pFrame->FindPageFrm()->GetPageDesc()->GetNumType().GetNumberingType();

                pPageNr->ChangeExpansion( pDoc, nVirtNum, nNumPages,
                                            bVirt, nNumFmt > -1 ? &nNumFmt : 0);
            }
            pRet = new SwFldPortion( pFld->GetCntnt( bName ) );
            break;
        }
        case RES_GETEXPFLD:
        {
            if( !bName && pSh && !pSh->Imp()->IsUpdateExpFlds() )
            {
                SwGetExpField* pExpFld = (SwGetExpField*)pFld;
                if( !::lcl_IsInBody( pFrame ) )
                {
                    pExpFld->ChgBodyTxtFlag( sal_False );
                    pExpFld->ChangeExpansion( *pFrame, *((SwTxtFld*)pHint) );
                }
                else if( !pExpFld->IsInBodyTxt() )
                {
                    // war vorher anders, also erst expandieren, dann umsetzen!!
                    pExpFld->ChangeExpansion( *pFrame, *((SwTxtFld*)pHint) );
                    pExpFld->ChgBodyTxtFlag( sal_True );
                }
            }
            pRet = new SwFldPortion( pFld->GetCntnt( bName ) );
            break;
        }
        case RES_DBFLD:
        {
            if( !bName )
            {
                SwDBField* pDBFld = (SwDBField*)pFld;
                pDBFld->ChgBodyTxtFlag( ::lcl_IsInBody( pFrame ) );
            }
            pRet = new SwFldPortion( pFld->GetCntnt( bName ) );
            break;
        }
        case RES_REFPAGEGETFLD:
            if( !bName && pSh && !pSh->Imp()->IsUpdateExpFlds() )
                ((SwRefPageGetField*)pFld)->ChangeExpansion( pFrame, (SwTxtFld*)pHint );
            pRet = new SwFldPortion( pFld->GetCntnt( bName ) );
            break;

        case RES_JUMPEDITFLD:
            if( !bName )
                pChFmt =  ((SwJumpEditField*)pFld)->GetCharFmt();
            bNewFlyPor = sal_True;
            bPlaceHolder = sal_True;
            break;

        default:
        {
            pRet = new SwFldPortion(pFld->GetCntnt( bName ) );
        }
    }

    if( bNewFlyPor )
    {
        SwFont *pTmpFnt = 0;
        if( !bName )
        {
            pTmpFnt = new SwFont( *pFnt );
            if( bINet )
            {
                SwAttrPool* pPool = pChFmt->GetAttrSet().GetPool();
                SfxItemSet aSet( *pPool, RES_CHRATR_BEGIN, RES_CHRATR_END );
                SfxItemSet aTmpSet( aSet );
                pFrm->GetTxtNode()->GetAttr(aSet,rInf.GetIdx(),rInf.GetIdx()+1);
                aTmpSet.Set( pChFmt->GetAttrSet() );
                aTmpSet.Differentiate( aSet );
                if( aTmpSet.Count() )
                    pTmpFnt->SetDiffFnt( &aTmpSet, pFrm->GetTxtNode()->getIDocumentSettingAccess() );
            }
            else
                pTmpFnt->SetDiffFnt( &pChFmt->GetAttrSet(), pFrm->GetTxtNode()->getIDocumentSettingAccess() );
        }
        pRet = new SwFldPortion( pFld->GetCntnt( bName ), pTmpFnt, bPlaceHolder );
    }

    return pRet;
}

/*************************************************************************
 *                      SwTxtFormatter::TryNewNoLengthPortion()
 *************************************************************************/

SwFldPortion * lcl_NewMetaPortion(SwTxtAttr & rHint, const bool bPrefix)
{
    ::sw::Meta *const pMeta(
        static_cast<SwFmtMeta &>(rHint.GetAttr()).GetMeta() );
    ::rtl::OUString fix;
    ::sw::MetaField *const pField( dynamic_cast< ::sw::MetaField * >(pMeta) );
    OSL_ENSURE(pField, "lcl_NewMetaPortion: no meta field?");
    if (pField)
    {
        pField->GetPrefixAndSuffix((bPrefix) ? &fix : 0, (bPrefix) ? 0 : &fix);
    }
    return new SwFldPortion( fix );
}

/** Try to create a new portion with zero length, for an end of a hint
    (where there is no CH_TXTATR). Because there may be multiple hint ends at a
    given index, m_nHintEndIndex is used to keep track of the already created
    portions. But the portions created here may actually be deleted again,
    due to UnderFlow. In that case, m_nHintEndIndex must be decremented,
    so the portion will be created again on the next line.
 */
SwExpandPortion *
SwTxtFormatter::TryNewNoLengthPortion(SwTxtFormatInfo & rInfo)
{
    if (pHints)
    {
        const xub_StrLen nIdx(rInfo.GetIdx());
        while (m_nHintEndIndex < pHints->GetEndCount())
        {
            SwTxtAttr & rHint( *pHints->GetEnd(m_nHintEndIndex) );
            xub_StrLen const nEnd( *rHint.GetAnyEnd() );
            if (nEnd > nIdx)
            {
                break;
            }
            ++m_nHintEndIndex;
            if (nEnd == nIdx)
            {
                if (RES_TXTATR_METAFIELD == rHint.Which())
                {
                    SwFldPortion *const pPortion(
                            lcl_NewMetaPortion(rHint, false));
                    pPortion->SetNoLength(); // no CH_TXTATR at hint end!
                    return pPortion;
                }
            }
        }
    }
    return 0;
}

/*************************************************************************
 *                      SwTxtFormatter::NewExtraPortion()
 *************************************************************************/

SwLinePortion *SwTxtFormatter::NewExtraPortion( SwTxtFormatInfo &rInf )
{
    SwTxtAttr *pHint = GetAttr( rInf.GetIdx() );
    SwLinePortion *pRet = 0;
    if( !pHint )
    {
        pRet = new SwTxtPortion;
        pRet->SetLen( 1 );
        rInf.SetLen( 1 );
        return pRet;
    }

    switch( pHint->Which() )
    {
        case RES_TXTATR_FLYCNT :
        {
            pRet = NewFlyCntPortion( rInf, pHint );
            break;
        }
        case RES_TXTATR_FTN :
        {
            pRet = NewFtnPortion( rInf, pHint );
            break;
        }
        case RES_TXTATR_FIELD :
        {
            pRet = NewFldPortion( rInf, pHint );
            break;
        }
        case RES_TXTATR_REFMARK :
        {
            pRet = new SwIsoRefPortion;
            break;
        }
        case RES_TXTATR_TOXMARK :
        {
            pRet = new SwIsoToxPortion;
            break;
        }
        case RES_TXTATR_METAFIELD:
        {
            pRet = lcl_NewMetaPortion( *pHint, true );
            break;
        }
        default: ;
    }
    if( !pRet )
    {
        const XubString aNothing;
        pRet = new SwFldPortion( aNothing );
        rInf.SetLen( 1 );
    }
    return pRet;
}

/*************************************************************************
 *                      SwTxtFormatter::NewNumberPortion()
 *************************************************************************/


SwNumberPortion *SwTxtFormatter::NewNumberPortion( SwTxtFormatInfo &rInf ) const
{
    if( rInf.IsNumDone() || rInf.GetTxtStart() != nStart
                || rInf.GetTxtStart() != rInf.GetIdx() )
        return 0;

    SwNumberPortion *pRet = 0;
    const SwTxtNode* pTxtNd = GetTxtFrm()->GetTxtNode();
    const SwNumRule* pNumRule = pTxtNd->GetNumRule();

    // hat ein "gueltige" Nummer ?
    if( pTxtNd->IsNumbered() && pTxtNd->IsCountedInList())
    {
        const SwNumFmt &rNumFmt = pNumRule->Get( static_cast<USHORT>(pTxtNd->GetActualListLevel()) );
        const sal_Bool bLeft = SVX_ADJUST_LEFT == rNumFmt.GetNumAdjust();
        const sal_Bool bCenter = SVX_ADJUST_CENTER == rNumFmt.GetNumAdjust();
        const bool bLabelAlignmentPosAndSpaceModeActive(
                rNumFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT );
        const KSHORT nMinDist = bLabelAlignmentPosAndSpaceModeActive
                                ? 0 : rNumFmt.GetCharTextDistance();

        if( SVX_NUM_BITMAP == rNumFmt.GetNumberingType() )
        {
            pRet = new SwGrfNumPortion( (SwFrm*)GetTxtFrm(),
                                        pTxtNd->GetLabelFollowedBy(),
                                        rNumFmt.GetBrush(),
                                        rNumFmt.GetGraphicOrientation(),
                                        rNumFmt.GetGraphicSize(),
                                        bLeft, bCenter, nMinDist,
                                        bLabelAlignmentPosAndSpaceModeActive );
            long nTmpA = rInf.GetLast()->GetAscent();
            long nTmpD = rInf.GetLast()->Height() - nTmpA;
            if( !rInf.IsTest() )
                ((SwGrfNumPortion*)pRet)->SetBase( nTmpA, nTmpD, nTmpA, nTmpD );
        }
        else
        {
            // Der SwFont wird dynamisch angelegt und im CTOR uebergeben,
            // weil das CharFmt nur einen SV-Font zurueckliefert.
            // Im Dtor vom SwNumberPortion wird der SwFont deletet.
            SwFont *pNumFnt = 0;
            const SwAttrSet* pFmt = rNumFmt.GetCharFmt() ?
                                    &rNumFmt.GetCharFmt()->GetAttrSet() :
                                    NULL;
            const IDocumentSettingAccess* pIDSA = pTxtNd->getIDocumentSettingAccess();

            if( SVX_NUM_CHAR_SPECIAL == rNumFmt.GetNumberingType() )
            {
                const Font *pFmtFnt = rNumFmt.GetBulletFont();

                //
                // Build a new bullet font basing on the current paragraph font:
                //
                pNumFnt = new SwFont( &rInf.GetCharAttr(), pIDSA );

                // --> FME 2005-08-11 #i53199#
                if ( !pIDSA->get(IDocumentSettingAccess::DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT) )
                {
                    // i18463:
                    // Underline style of paragraph font should not be considered
                    // Overline style of paragraph font should not be considered
                    // Weight style of paragraph font should not be considered
                    // Posture style of paragraph font should not be considered
                    pNumFnt->SetUnderline( UNDERLINE_NONE );
                    pNumFnt->SetOverline( UNDERLINE_NONE );
                    pNumFnt->SetItalic( ITALIC_NONE, SW_LATIN );
                    pNumFnt->SetItalic( ITALIC_NONE, SW_CJK );
                    pNumFnt->SetItalic( ITALIC_NONE, SW_CTL );
                    pNumFnt->SetWeight( WEIGHT_NORMAL, SW_LATIN );
                    pNumFnt->SetWeight( WEIGHT_NORMAL, SW_CJK );
                    pNumFnt->SetWeight( WEIGHT_NORMAL, SW_CTL );
                }

                //
                // Apply the explicit attributes from the character style
                // associated with the numering to the new bullet font.
                //
                if( pFmt )
                    pNumFnt->SetDiffFnt( pFmt, pIDSA );

                if ( pFmtFnt )
                {
                    const BYTE nAct = pNumFnt->GetActual();
                    pNumFnt->SetFamily( pFmtFnt->GetFamily(), nAct );
                    pNumFnt->SetName( pFmtFnt->GetName(), nAct );
                    pNumFnt->SetStyleName( pFmtFnt->GetStyleName(), nAct );
                    pNumFnt->SetCharSet( pFmtFnt->GetCharSet(), nAct );
                    pNumFnt->SetPitch( pFmtFnt->GetPitch(), nAct );
                }

                // we do not allow a vertical font
                pNumFnt->SetVertical( pNumFnt->GetOrientation(),
                                      pFrm->IsVertical() );

                // --> OD 2008-01-23 #newlistelevelattrs#
                pRet = new SwBulletPortion( rNumFmt.GetBulletChar(),
                                            pTxtNd->GetLabelFollowedBy(),
                                            pNumFnt,
                                            bLeft, bCenter, nMinDist,
                                            bLabelAlignmentPosAndSpaceModeActive );
                // <--
            }
            else
            {
                // --> OD 2006-06-02 #b6432095#
                // use method <SwNumRule::MakeNumString(..)> instead of
                // method <SwTxtNode::GetNumString()>, because for levels with
                // numbering none the prefix and the suffix strings have to be provided.
                XubString aTxt( pNumRule->MakeNumString( *(pTxtNd->GetNum()) ) );
                // <--
                if ( aTxt.Len() > 0 )
                {
                    aTxt.Insert( pTxtNd->GetLabelFollowedBy() );
                }

                // 7974: Nicht nur eine Optimierung...
                // Eine Numberportion ohne Text wird die Breite von 0
                // erhalten. Die nachfolgende Textportion wird im BreakLine
                // in das BreakCut laufen, obwohl rInf.GetLast()->GetFlyPortion()
                // vorliegt!
                if( aTxt.Len() )
                {
                    //
                    // Build a new numbering font basing on the current paragraph font:
                    //
                    pNumFnt = new SwFont( &rInf.GetCharAttr(), pIDSA );

                    // --> FME 2005-08-11 #i53199#
                    if ( !pIDSA->get(IDocumentSettingAccess::DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT) )
                    {
                        // i18463:
                        // Underline style of paragraph font should not be considered
                        pNumFnt->SetUnderline( UNDERLINE_NONE );
                        // Overline style of paragraph font should not be considered
                        pNumFnt->SetOverline( UNDERLINE_NONE );
                    }


                    //
                    // Apply the explicit attributes from the character style
                    // associated with the numering to the new bullet font.
                    //
                    if( pFmt )
                        pNumFnt->SetDiffFnt( pFmt, pIDSA );

                    // we do not allow a vertical font
                    pNumFnt->SetVertical( pNumFnt->GetOrientation(), pFrm->IsVertical() );

                    pRet = new SwNumberPortion( aTxt, pNumFnt,
                                                bLeft, bCenter, nMinDist,
                                                bLabelAlignmentPosAndSpaceModeActive );
                }
            }
        }
    }
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
