/*************************************************************************
 *
 *  $RCSfile: txtfld.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ama $ $Date: 2000-10-17 10:22:50 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

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
        SwFrm *pTmp = pFrm;
        SwFlyFrm *pFly;
        while ( 0 != (pFly = pTmp->FindFlyFrm()) )
            pTmp = pFly->GetAnchor();
        return pTmp->IsInDocBody();
    }
}


SwExpandPortion *SwTxtFormatter::NewFldPortion( SwTxtFormatInfo &rInf,
                                                SwTxtAttr *pHint ) const
{
    SwExpandPortion *pRet;
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

    switch( pFld->GetTyp()->Which() )
    {
        case RES_SCRIPTFLD:
        case RES_POSTITFLD:
        {
            pRet = new SwPostItsPortion( RES_SCRIPTFLD == pFld->GetTyp()->Which() );
            break;
        }
        case RES_HIDDENTXTFLD:
        {
            pRet = new SwHiddenPortion(pFld->GetCntnt( bName ));
            break;
        }
        case RES_CHAPTERFLD:
        {
            if( !bName && pSh && !pSh->Imp()->IsUpdateExpFlds() )
            {
                ((SwChapterField*)pFld)->ChangeExpansion( pFrame,
                                        &((SwTxtFld*)pHint)->GetTxtNode() );
            }
            pRet = new SwFldPortion( pFld->GetCntnt( bName ) );
            break;
        }

        case RES_DOCSTATFLD:
            if( !bName && pSh && !pSh->Imp()->IsUpdateExpFlds() )
                ((SwDocStatField*)pFld)->ChangeExpansion( pFrame );
            pRet = new SwFldPortion( pFld->GetCntnt( bName ) );
            break;

        case RES_PAGENUMBERFLD:
        {
            if( !bName && pSh && !pSh->Imp()->IsUpdateExpFlds() )
            {
                SwDoc* pDoc = pSh->GetDoc();
                SwPageNumberFieldType *pPageNr = (SwPageNumberFieldType *)
                                pFld->GetTyp();
//???                               pDoc->GetSysFldType( RES_PAGENUMBERFLD );

//              SwPageFrm *pPage = pFrm->FindPageFrm();
//              sal_Bool bVirt = pPage && pPage->GetNext();
                sal_Bool bVirt = pSh->GetLayout()->IsVirtPageNum();

                MSHORT nVirtNum = pFrame->GetVirtPageNum(),
                       nNumPages = pDoc->GetRootFrm()->GetPageNum();
                const SvxExtNumType* pNumFmt = SVX_NUM_PAGEDESC == pFld->GetFormat()
                    ? &pFrame->FindPageFrm()->GetPageDesc()->GetNumType().eType
                    : 0;

                pPageNr->ChangeExpansion( pDoc, nVirtNum, nNumPages,
                                            bVirt, pNumFmt );
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
                    pTmpFnt->SetDiffFnt( &aTmpSet );
            }
            else
                pTmpFnt->SetDiffFnt( &pChFmt->GetAttrSet() );
        }
        pRet = new SwFldPortion( pFld->GetCntnt( bName ), pTmpFnt );
    }

    return pRet;
}


SwFldPortion *SwTxtFormatter::GetFieldRest( SwTxtFormatInfo &rInf ) const
{
    if( !nStart )
        return NULL;
    SwTxtAttr *pHint = GetAttr( nStart - 1 );
    SwFldPortion *pRet = NULL;
    if( !pHint )
        return pRet;

    if( pHint->Which() == RES_TXTATR_FIELD )
    {
        SwExpandPortion* pNew = NewFldPortion( rInf, pHint );
        if( pNew->InFldGrp() )
            pRet = (SwFldPortion*)pNew;
        else
            delete pNew;
    }
    return pRet;
}


void SwTxtFormatter::MakeRestPortion()
{
    const SwFldPortion* pFld = NULL;
    const SwLinePortion* pLine = GetCurr();
    while( pLine )
    {
        if( pLine->InFldGrp() )
            pFld = (SwFldPortion*)pLine;
        pLine = pLine->GetPortion();
    }
    if( pFld && pFld->HasFollow() )
    {
        xub_StrLen nNext = nStart + GetCurr()->GetLen();
        if( !nNext )
            return;
        SwTxtAttr *pHint = GetAttr( nNext - 1 );
        if( !pHint )
            return;
        SwFldPortion *pRest = NULL;

        if( pHint->Which() == RES_TXTATR_FIELD )
        {
            SwExpandPortion* pNew = NewFldPortion( GetInfo(), pHint );
            if( pNew->InFldGrp() )
                pRest = (SwFldPortion*)pNew;
            else
                delete pNew;
        }
        if( pRest )
        {
            pRest->TakeNextOffset( pFld );
            GetInfo().SetRest( pRest );
        }
    }
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
#ifdef DEBUG
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
#ifdef DEBUG
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
    const SwNodeNum* pNum = pTxtNd->GetNum();

    if( !pNumRule )     // oder sollte OutlineNum an sein?
    {
        pNum = pTxtNd->GetOutlineNum();
        if( pNum )
            pNumRule = pTxtNd->GetDoc()->GetOutlineNumRule();
    }
    // hat ein "gueltige" Nummer ?
    if( pNumRule && pNum && MAXLEVEL > pNum->GetLevel() )
    {
        CONST SwNumFmt &rNumFmt = pNumRule->Get( pNum->GetLevel() );
        const sal_Bool bLeft = SVX_ADJUST_LEFT == rNumFmt.GetAdjust();
        const sal_Bool bCenter = SVX_ADJUST_CENTER == rNumFmt.GetAdjust();
        const KSHORT nMinDist = rNumFmt.GetCharTextOffset();

        if( SVX_NUM_BITMAP == rNumFmt.eType )
        {
            pRet = new SwGrfNumPortion( (SwFrm*)GetTxtFrm(),rNumFmt.GetGrfBrush(),
                rNumFmt.GetGrfOrient(), rNumFmt.GetGrfSize(),
                bLeft, bCenter, nMinDist );
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
                &rNumFmt.GetCharFmt()->GetAttrSet() : NULL;
            if( SVX_NUM_CHAR_SPECIAL == rNumFmt.eType )
            {
                const Font *pFmtFnt = rNumFmt.GetBulletFont();
                pNumFnt = new SwFont( &rInf.GetCharAttr() );
                if( pFmt )
                    pNumFnt->SetDiffFnt( pFmt );
                if ( pFmtFnt )
                {
                    const BYTE nAct = pNumFnt->GetActual();
                    pNumFnt->SetFamily( pFmtFnt->GetFamily(), nAct );
                    pNumFnt->SetName( pFmtFnt->GetName(), nAct );
                    pNumFnt->SetStyleName( pFmtFnt->GetStyleName(), nAct );
                    pNumFnt->SetCharSet( pFmtFnt->GetCharSet(), nAct );
                    pNumFnt->SetPitch( pFmtFnt->GetPitch(), nAct );
                }
                pRet = new SwBulletPortion( rNumFmt.GetBulletChar(), pNumFnt, bLeft,
                            bCenter, nMinDist );
            }
            else
            {
                XubString aTxt( pNumRule->MakeNumString( *pNum ) );

                // 7974: Nicht nur eine Optimierung...
                // Eine Numberportion ohne Text wird die Breite von 0
                // erhalten. Die nachfolgende Textportion wird im BreakLine
                // in das BreakCut laufen, obwohl rInf.GetLast()->GetFlyPortion()
                // vorliegt!
                if( aTxt.Len() )
                {
                    pNumFnt = new SwFont( &rInf.GetCharAttr() );
                    if( pFmt )
                        pNumFnt->SetDiffFnt( pFmt );
                    // Die SSize muss erhalten bleiben
                    // pNumFnt->ChangeSize( rInf.GetFont()->GetSize() );
                    pRet = new SwNumberPortion( aTxt, pNumFnt, bLeft, bCenter,
                                                nMinDist );
                }
            }
        }
    }
    return pRet;
}

/*-----------------16.10.00 09:55-------------------
 * SwTxtFrm::GetRestPortion() returns a field portion,
 * if the last portion in the last line is a field portion,
 * which is not complete. So the following part of the text frame
 * has to start with the rest of the field.
 * If the field portion is inside a multi-portion, the next text frame
 * starts with a multi-portion, too.
 * --------------------------------------------------*/

const SwFldPortion* SwTxtFrm::GetRestPortion()
{
    if( !HasPara() )
        GetFormatted();
    SwTxtSizeInfo aInf( this );
    SwTxtIter aLine( this, &aInf );
    aLine.Bottom();
    SwFldPortion* pRet = NULL;
    const SwLinePortion* pLine = aLine.GetCurr();
    const SwMultiPortion *pMulti = NULL;
    while( pLine )
    {
        if( pLine->InFldGrp() )
        {
            pMulti = NULL;
            pRet = (SwFldPortion*)pLine;
        }
        else if( pLine->IsMultiPortion() )
        {
            pRet = NULL;
            pMulti = (SwMultiPortion*)pLine;
        }
        pLine = pLine->GetPortion();
        // If the last portion is a multi-portion, we enter it
        // and look for a field portion inside.
        if( !pLine && pMulti )
            pLine = pMulti->GetRoot().GetNext();
    }
    if( pRet && !pRet->HasFollow() )
        pRet = NULL;
    return pRet;
}



