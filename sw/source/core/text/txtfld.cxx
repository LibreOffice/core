/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txtfld.cxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:07:58 $
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
#include "precompiled_sw.hxx"


#include "hintids.hxx"

#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif

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
#include "txtcfg.hxx"
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

#ifndef _PORMULTI_HXX
#include <pormulti.hxx>     // SwMultiPortion
#endif


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

    // Sprache setzen
    ((SwTxtFormatter*)this)->SeekAndChg( rInf );
    pFld->SetLanguage( GetFnt()->GetLanguage() );

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
/* Solange das ChangeExpansion auskommentiert ist.
 * Aktualisieren in Kopf/Fuszeilen geht aktuell nicht.
                if( !::lcl_IsInBody( pFrame ) )
                {
                    pDBFld->ChgBodyTxtFlag( sal_False );
                    pDBFld->ChangeExpansion( pFrame, (SwTxtFld*)pHint );
                }
                else if( !pDBFld->IsInBodyTxt() )
                {
                    // war vorher anders, also erst expandieren, dann umsetzen!!
                    pDBFld->ChangeExpansion( pFrame, (SwTxtFld*)pHint );
                    pDBFld->ChgBodyTxtFlag( sal_True );
                }
*/
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
 *                      SwTxtFormatter::NewExtraPortion()
 *************************************************************************/


SwLinePortion *SwTxtFormatter::NewExtraPortion( SwTxtFormatInfo &rInf )
{
    SwTxtAttr *pHint = GetAttr( rInf.GetIdx() );
    SwLinePortion *pRet = 0;
    if( !pHint )
    {
#if OSL_DEBUG_LEVEL > 1
//        aDbstream << "NewExtraPortion: hint not found?" << endl;
#endif
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
        case RES_TXTATR_SOFTHYPH :
        {
            pRet = new SwSoftHyphPortion;
            break;
        }
        case RES_TXTATR_HARDBLANK :
        {
            pRet = new SwBlankPortion( ((SwTxtHardBlank*)pHint)->GetChar() );
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
        default: ;
    }
    if( !pRet )
    {
#if OSL_DEBUG_LEVEL > 1
//        aDbstream << "NewExtraPortion: unknown hint" << endl;
#endif
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
    if( pTxtNd->IsNumbered() && pTxtNd->IsCounted())
    {
        const SwNumFmt &rNumFmt = pNumRule->Get( static_cast<USHORT>(pTxtNd->GetLevel()) );
        const sal_Bool bLeft = SVX_ADJUST_LEFT == rNumFmt.GetNumAdjust();
        const sal_Bool bCenter = SVX_ADJUST_CENTER == rNumFmt.GetNumAdjust();
        // --> OD 2008-01-23 #newlistlevelattrs#
        const bool bLabelAlignmentPosAndSpaceModeActive(
                rNumFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT );
        const KSHORT nMinDist = bLabelAlignmentPosAndSpaceModeActive
                                ? 0 : rNumFmt.GetCharTextDistance();
        // <--

        if( SVX_NUM_BITMAP == rNumFmt.GetNumberingType() )
        {
            // --> OD 2008-01-23 #newlistlevelattrs#
            pRet = new SwGrfNumPortion( (SwFrm*)GetTxtFrm(),
                                        pTxtNd->GetLabelFollowedBy(),
                                        rNumFmt.GetBrush(),
                                        rNumFmt.GetGraphicOrientation(),
                                        rNumFmt.GetGraphicSize(),
                                        bLeft, bCenter, nMinDist,
                                        bLabelAlignmentPosAndSpaceModeActive );
            // <--
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
                    // Weight style of paragraph font should not be considered
                    // Posture style of paragraph font should not be considered
                    pNumFnt->SetUnderline( UNDERLINE_NONE );
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
//                XubString aTxt( pTxtNd->GetNumString() );
                XubString aTxt( pNumRule->MakeNumString( *(pTxtNd->GetNum()) ) );
                // <--
                // --> OD 2008-01-23 #newlistlevelattrs#
                if ( aTxt.Len() > 0 )
                {
                    aTxt.Insert( pTxtNd->GetLabelFollowedBy() );
                }
                // <--

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
                    }


                    //
                    // Apply the explicit attributes from the character style
                    // associated with the numering to the new bullet font.
                    //
                    if( pFmt )
                        pNumFnt->SetDiffFnt( pFmt, pIDSA );

                    // we do not allow a vertical font
                    pNumFnt->SetVertical( pNumFnt->GetOrientation(), pFrm->IsVertical() );

                    // --> OD 2008-01-23 #newlistlevelattrs#
                    pRet = new SwNumberPortion( aTxt, pNumFnt,
                                                bLeft, bCenter, nMinDist,
                                                bLabelAlignmentPosAndSpaceModeActive );
                    // <--
                }
            }
        }
    }
    return pRet;
}
/* -----------------26.06.2003 13:54-----------------

 --------------------------------------------------*/
void SwTxtFld::NotifyContentChange(SwFmtFld& rFmtFld)
{
    //if not in undo section notify the change
    if(pMyTxtNd && pMyTxtNd->GetNodes().IsDocNodes())
        pMyTxtNd->Modify(0, &rFmtFld);
}

// #111840#
SwPosition * SwTxtFld::GetPosition() const
{
    return GetTxtNode().GetPosition(this);
}
