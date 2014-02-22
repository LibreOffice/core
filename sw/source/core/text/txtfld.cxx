/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "hintids.hxx"
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <charfmt.hxx>

#include "viewsh.hxx"
#include "doc.hxx"
#include "rootfrm.hxx"
#include "pagefrm.hxx"
#include "ndtxt.hxx"
#include "fldbas.hxx"
#include "viewopt.hxx"
#include "flyfrm.hxx"
#include "viewimp.hxx"
#include "txtatr.hxx"
#include "swfont.hxx"
#include "fntcache.hxx"
#include "porfld.hxx"
#include "porftn.hxx"
#include "porref.hxx"
#include "portox.hxx"
#include "porhyph.hxx"
#include "porfly.hxx"
#include "itrform2.hxx"
#include "chpfld.hxx"
#include "dbfld.hxx"
#include "expfld.hxx"
#include "docufld.hxx"
#include "pagedesc.hxx"
#include <pormulti.hxx>
#include "fmtmeta.hxx"
#include "reffld.hxx"
#include "flddat.hxx"

/*************************************************************************
 *                      SwTxtFormatter::NewFldPortion()
 *************************************************************************/


static bool lcl_IsInBody( SwFrm *pFrm )
{
    if ( pFrm->IsInDocBody() )
        return true;
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
    SwField *pFld = (SwField*)pHint->GetFmtFld().GetField();
    const sal_Bool bName = rInf.GetOpt().IsFldName();

    SwCharFmt* pChFmt = 0;
    bool bNewFlyPor = false;
    sal_uInt16 subType = 0;

    
    ((SwTxtFormatter*)this)->SeekAndChg( rInf );
    if (pFld->GetLanguage() != GetFnt()->GetLanguage())
    {
        pFld->SetLanguage( GetFnt()->GetLanguage() );
        
        if (pFld->GetTyp()->Which()==RES_POSTITFLD)
            const_cast<SwFmtFld*> (&pHint->GetFmtFld())->Broadcast( SwFmtFldHint( &pHint->GetFmtFld(), SWFMTFLD_LANGUAGE ) );
    }

    SwViewShell *pSh = rInf.GetVsh();
    SwDoc *const pDoc( (pSh) ? pSh->GetDoc() : 0 );
    bool const bInClipboard( (pDoc) ? pDoc->IsClipBoard() : true );
    sal_Bool bPlaceHolder = false;

    switch( pFld->GetTyp()->Which() )
    {
        case RES_SCRIPTFLD:
        case RES_POSTITFLD:
            pRet = new SwPostItsPortion( RES_SCRIPTFLD == pFld->GetTyp()->Which() );
            break;

        case RES_COMBINED_CHARS:
            {
                if( bName )
                    pRet = new SwFldPortion( pFld->GetFieldName() );
                else
                    pRet = new SwCombinedPortion( pFld->ExpandField(bInClipboard) );
            }
            break;

        case RES_HIDDENTXTFLD:
            {
                OUString const aStr( (bName)
                        ? pFld->GetFieldName()
                        : pFld->ExpandField(bInClipboard) );
                pRet = new SwHiddenPortion(aStr);
            }
            break;

        case RES_CHAPTERFLD:
            if( !bName && pSh && !pSh->Imp()->IsUpdateExpFlds() )
            {
                ((SwChapterField*)pFld)->ChangeExpansion( pFrame,
                                        &((SwTxtFld*)pHint)->GetTxtNode() );
            }
            {
                OUString const aStr( (bName)
                        ? pFld->GetFieldName()
                        : pFld->ExpandField(bInClipboard) );
                pRet = new SwFldPortion( aStr );
            }
            break;

        case RES_DOCSTATFLD:
            if( !bName && pSh && !pSh->Imp()->IsUpdateExpFlds() )
            {
                ((SwDocStatField*)pFld)->ChangeExpansion( pFrame );
            }
            {
                OUString const aStr( (bName)
                        ? pFld->GetFieldName()
                        : pFld->ExpandField(bInClipboard) );
                pRet = new SwFldPortion( aStr );
            }
            if(pRet)
                ((SwFldPortion*)pRet)->m_nAttrFldType= ATTR_PAGECOOUNTFLD;
            break;

        case RES_PAGENUMBERFLD:
        {
            if( !bName && pSh && pSh->GetLayout() && !pSh->Imp()->IsUpdateExpFlds() )
            {
                SwPageNumberFieldType *pPageNr = (SwPageNumberFieldType *)pFld->GetTyp();

                const SwRootFrm* pTmpRootFrm = pSh->GetLayout();
                const sal_Bool bVirt = pTmpRootFrm->IsVirtPageNum();

                MSHORT nVirtNum = pFrame->GetVirtPageNum();
                MSHORT nNumPages = pTmpRootFrm->GetPageNum();
                sal_Int16 nNumFmt = -1;
                if(SVX_NUM_PAGEDESC == pFld->GetFormat())
                    nNumFmt = pFrame->FindPageFrm()->GetPageDesc()->GetNumType().GetNumberingType();
                static_cast<SwPageNumberField*>(pFld)
                    ->ChangeExpansion(nVirtNum, nNumPages);
                pPageNr->ChangeExpansion(pDoc,
                                            bVirt, nNumFmt > -1 ? &nNumFmt : 0);
            }
            {
                OUString const aStr( (bName)
                        ? pFld->GetFieldName()
                        : pFld->ExpandField(bInClipboard) );
                pRet = new SwFldPortion( aStr );
            }
            if(pRet)
                ((SwFldPortion*)pRet)->m_nAttrFldType= ATTR_PAGENUMBERFLD;
            break;
        }
        case RES_GETEXPFLD:
        {
            if( !bName && pSh && !pSh->Imp()->IsUpdateExpFlds() )
            {
                SwGetExpField* pExpFld = (SwGetExpField*)pFld;
                if( !::lcl_IsInBody( pFrame ) )
                {
                    pExpFld->ChgBodyTxtFlag( false );
                    pExpFld->ChangeExpansion( *pFrame, *((SwTxtFld*)pHint) );
                }
                else if( !pExpFld->IsInBodyTxt() )
                {
                    
                    pExpFld->ChangeExpansion( *pFrame, *((SwTxtFld*)pHint) );
                    pExpFld->ChgBodyTxtFlag( true );
                }
            }
            {
                OUString const aStr( (bName)
                        ? pFld->GetFieldName()
                        : pFld->ExpandField(bInClipboard) );
                pRet = new SwFldPortion( aStr );
            }
            break;
        }
        case RES_DBFLD:
        {
            if( !bName )
            {
                SwDBField* pDBFld = (SwDBField*)pFld;
                pDBFld->ChgBodyTxtFlag( ::lcl_IsInBody( pFrame ) );
            }
            {
                OUString const aStr( (bName)
                        ? pFld->GetFieldName()
                        : pFld->ExpandField(bInClipboard) );
                pRet = new SwFldPortion(aStr);
            }
            break;
        }
        case RES_REFPAGEGETFLD:
            if( !bName && pSh && !pSh->Imp()->IsUpdateExpFlds() )
            {
                ((SwRefPageGetField*)pFld)->ChangeExpansion( pFrame, (SwTxtFld*)pHint );
            }
            {
                OUString const aStr( (bName)
                        ? pFld->GetFieldName()
                        : pFld->ExpandField(bInClipboard) );
                pRet = new SwFldPortion(aStr);
            }
            break;

        case RES_JUMPEDITFLD:
            if( !bName )
                pChFmt =  ((SwJumpEditField*)pFld)->GetCharFmt();
            bNewFlyPor = true;
            bPlaceHolder = true;
            break;
        case RES_GETREFFLD:
            subType = ((SwGetRefField*)pFld)->GetSubType();
            {
                OUString const str( (bName)
                        ? pFld->GetFieldName()
                        : pFld->ExpandField(bInClipboard) );
                pRet = new SwFldPortion(str);
            }
            if(pRet)
            {
                if( subType == REF_BOOKMARK  )
                    ((SwFldPortion*)pRet)->m_nAttrFldType = ATTR_BOOKMARKFLD;
                else if( subType == REF_SETREFATTR )
                    ((SwFldPortion*)pRet)->m_nAttrFldType = ATTR_SETREFATTRFLD;
                break;
            }
        case RES_DATETIMEFLD:
            subType = ((SwDateTimeField*)pFld)->GetSubType();
            {
                OUString const str( (bName)
                        ? pFld->GetFieldName()
                        : pFld->ExpandField(bInClipboard) );
                pRet = new SwFldPortion(str);
            }
            if(pRet)
            {
                if( subType & DATEFLD  )
                    ((SwFldPortion*)pRet)->m_nAttrFldType= ATTR_DATEFLD;
                else if( subType & TIMEFLD )
                    ((SwFldPortion*)pRet)->m_nAttrFldType = ATTR_TIMEFLD;
                break;
            }
        default:
            {
                OUString const aStr( (bName)
                        ? pFld->GetFieldName()
                        : pFld->ExpandField(bInClipboard) );
                pRet = new SwFldPortion(aStr);
            }
    }

    if( bNewFlyPor )
    {
        SwFont *pTmpFnt = 0;
        if( !bName )
        {
            pTmpFnt = new SwFont( *pFnt );
            pTmpFnt->SetDiffFnt( &pChFmt->GetAttrSet(), pFrm->GetTxtNode()->getIDocumentSettingAccess() );
        }
        {
            OUString const aStr( (bName)
                    ? pFld->GetFieldName()
                    : pFld->ExpandField(bInClipboard) );
            pRet = new SwFldPortion(aStr, pTmpFnt, bPlaceHolder);
        }
    }

    return pRet;
}

/*************************************************************************
 *                      SwTxtFormatter::TryNewNoLengthPortion()
 *************************************************************************/

static SwFldPortion * lcl_NewMetaPortion(SwTxtAttr & rHint, const bool bPrefix)
{
    ::sw::Meta *const pMeta(
        static_cast<SwFmtMeta &>(rHint.GetAttr()).GetMeta() );
    OUString fix;
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
        const sal_Int32 nIdx(rInfo.GetIdx());
        while (m_nHintEndIndex < pHints->GetEndCount())
        {
            SwTxtAttr & rHint( *pHints->GetEnd(m_nHintEndIndex) );
            sal_Int32 const nEnd( *rHint.GetAnyEnd() );
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
                    pPortion->SetNoLength(); 
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
    case RES_TXTATR_ANNOTATION :
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
        const OUString aNothing;
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

    
    if( pTxtNd->IsNumbered() && pTxtNd->IsCountedInList())
    {
        int nLevel = pTxtNd->GetActualListLevel();

        if (nLevel < 0)
            nLevel = 0;

        if (nLevel >= MAXLEVEL)
            nLevel = MAXLEVEL - 1;

        const SwNumFmt &rNumFmt = pNumRule->Get( nLevel );
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
            
            
            
            SwFont *pNumFnt = 0;
            const SwAttrSet* pFmt = rNumFmt.GetCharFmt() ?
                                    &rNumFmt.GetCharFmt()->GetAttrSet() :
                                    NULL;
            const IDocumentSettingAccess* pIDSA = pTxtNd->getIDocumentSettingAccess();

            if( SVX_NUM_CHAR_SPECIAL == rNumFmt.GetNumberingType() )
            {
                const Font *pFmtFnt = rNumFmt.GetBulletFont();

                //
                
                //
                pNumFnt = new SwFont( &rInf.GetCharAttr(), pIDSA );

                
                if ( !pIDSA->get(IDocumentSettingAccess::DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT) )
                {
                    
                    
                    
                    
                    
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
                
                
                //
                if( pFmt )
                    pNumFnt->SetDiffFnt( pFmt, pIDSA );

                if ( pFmtFnt )
                {
                    const sal_uInt8 nAct = pNumFnt->GetActual();
                    pNumFnt->SetFamily( pFmtFnt->GetFamily(), nAct );
                    pNumFnt->SetName( pFmtFnt->GetName(), nAct );
                    pNumFnt->SetStyleName( pFmtFnt->GetStyleName(), nAct );
                    pNumFnt->SetCharSet( pFmtFnt->GetCharSet(), nAct );
                    pNumFnt->SetPitch( pFmtFnt->GetPitch(), nAct );
                }

                
                pNumFnt->SetVertical( pNumFnt->GetOrientation(),
                                      pFrm->IsVertical() );

                
                pRet = new SwBulletPortion( rNumFmt.GetBulletChar(),
                                            pTxtNd->GetLabelFollowedBy(),
                                            pNumFnt,
                                            bLeft, bCenter, nMinDist,
                                            bLabelAlignmentPosAndSpaceModeActive );
            }
            else
            {
                OUString aTxt( pTxtNd->GetNumString() );
                if ( !aTxt.isEmpty() )
                {
                    aTxt += pTxtNd->GetLabelFollowedBy();
                }

                
                
                
                
                
                if( !aTxt.isEmpty() )
                {
                    //
                    
                    //
                    pNumFnt = new SwFont( &rInf.GetCharAttr(), pIDSA );

                    
                    if ( !pIDSA->get(IDocumentSettingAccess::DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT) )
                    {
                        
                        
                        pNumFnt->SetUnderline( UNDERLINE_NONE );
                        
                        pNumFnt->SetOverline( UNDERLINE_NONE );
                    }


                    //
                    
                    
                    //
                    if( pFmt )
                        pNumFnt->SetDiffFnt( pFmt, pIDSA );

                    
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
