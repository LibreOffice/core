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
#include "fmtautofmt.hxx"
#include <IDocumentSettingAccess.hxx>
#include <svl/itemiter.hxx>

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
    SwFrm *pFrame = pFrm;
    SwField *pFld = const_cast<SwField*>(pHint->GetFmtFld().GetField());
    const bool bName = rInf.GetOpt().IsFldName();

    SwCharFmt* pChFmt = 0;
    bool bNewFlyPor = false;
    sal_uInt16 subType = 0;

    // set language
    const_cast<SwTxtFormatter*>(this)->SeekAndChg( rInf );
    if (pFld->GetLanguage() != GetFnt()->GetLanguage())
    {
        pFld->SetLanguage( GetFnt()->GetLanguage() );
        // let the visual note know about its new language
        if (pFld->GetTyp()->Which()==RES_POSTITFLD)
            const_cast<SwFmtFld*> (&pHint->GetFmtFld())->Broadcast( SwFmtFldHint( &pHint->GetFmtFld(), SwFmtFldHintWhich::LANGUAGE ) );
    }

    SwViewShell *pSh = rInf.GetVsh();
    SwDoc *const pDoc( (pSh) ? pSh->GetDoc() : 0 );
    bool const bInClipboard( pDoc == nullptr || pDoc->IsClipBoard() );
    bool bPlaceHolder = false;

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
                static_cast<SwChapterField*>(pFld)->ChangeExpansion( pFrame,
                    &static_txtattr_cast<SwTxtFld const*>(pHint)->GetTxtNode());
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
                static_cast<SwDocStatField*>(pFld)->ChangeExpansion( pFrame );
            }
            {
                OUString const aStr( (bName)
                        ? pFld->GetFieldName()
                        : pFld->ExpandField(bInClipboard) );
                pRet = new SwFldPortion( aStr );
            }
            static_cast<SwFldPortion*>(pRet)->m_nAttrFldType= ATTR_PAGECOOUNTFLD;
            break;

        case RES_PAGENUMBERFLD:
        {
            if( !bName && pSh && pSh->GetLayout() && !pSh->Imp()->IsUpdateExpFlds() )
            {
                SwPageNumberFieldType *pPageNr = static_cast<SwPageNumberFieldType *>(pFld->GetTyp());

                const SwRootFrm* pTmpRootFrm = pSh->GetLayout();
                const bool bVirt = pTmpRootFrm->IsVirtPageNum();

                sal_uInt16 nVirtNum = pFrame->GetVirtPageNum();
                sal_uInt16 nNumPages = pTmpRootFrm->GetPageNum();
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
            static_cast<SwFldPortion*>(pRet)->m_nAttrFldType= ATTR_PAGENUMBERFLD;
            break;
        }
        case RES_GETEXPFLD:
        {
            if( !bName && pSh && !pSh->Imp()->IsUpdateExpFlds() )
            {
                SwGetExpField* pExpFld = static_cast<SwGetExpField*>(pFld);
                if( !::lcl_IsInBody( pFrame ) )
                {
                    pExpFld->ChgBodyTxtFlag( false );
                    pExpFld->ChangeExpansion(*pFrame,
                            *static_txtattr_cast<SwTxtFld const*>(pHint));
                }
                else if( !pExpFld->IsInBodyTxt() )
                {
                    // Was something else previously, thus: expand first, then convert it!
                    pExpFld->ChangeExpansion(*pFrame,
                            *static_txtattr_cast<SwTxtFld const*>(pHint));
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
                SwDBField* pDBFld = static_cast<SwDBField*>(pFld);
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
                static_cast<SwRefPageGetField*>(pFld)->ChangeExpansion(pFrame,
                        static_txtattr_cast<SwTxtFld const*>(pHint));
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
                pChFmt = static_cast<SwJumpEditField*>(pFld)->GetCharFmt();
            bNewFlyPor = true;
            bPlaceHolder = true;
            break;
        case RES_GETREFFLD:
            subType = static_cast<SwGetRefField*>(pFld)->GetSubType();
            {
                OUString const str( (bName)
                        ? pFld->GetFieldName()
                        : pFld->ExpandField(bInClipboard) );
                pRet = new SwFldPortion(str);
            }
            if( subType == REF_BOOKMARK  )
                static_cast<SwFldPortion*>(pRet)->m_nAttrFldType = ATTR_BOOKMARKFLD;
            else if( subType == REF_SETREFATTR )
                static_cast<SwFldPortion*>(pRet)->m_nAttrFldType = ATTR_SETREFATTRFLD;
            break;
        case RES_DATETIMEFLD:
            subType = static_cast<SwDateTimeField*>(pFld)->GetSubType();
            {
                OUString const str( (bName)
                        ? pFld->GetFieldName()
                        : pFld->ExpandField(bInClipboard) );
                pRet = new SwFldPortion(str);
            }
            if( subType & DATEFLD  )
                static_cast<SwFldPortion*>(pRet)->m_nAttrFldType= ATTR_DATEFLD;
            else if( subType & TIMEFLD )
                static_cast<SwFldPortion*>(pRet)->m_nAttrFldType = ATTR_TIMEFLD;
            break;
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

/**
 * Try to create a new portion with zero length, for an end of a hint
 * (where there is no CH_TXTATR). Because there may be multiple hint ends at a
 * given index, m_nHintEndIndex is used to keep track of the already created
 * portions. But the portions created here may actually be deleted again,
 * due to Underflow. In that case, m_nHintEndIndex must be decremented,
 * so the portion will be created again on the next line.
 */
SwExpandPortion * SwTxtFormatter::TryNewNoLengthPortion(SwTxtFormatInfo & rInfo)
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
                    pPortion->SetNoLength(); // no CH_TXTATR at hint end!
                    return pPortion;
                }
            }
        }
    }
    return 0;
}

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

/**
 * OOXML spec says that w:rPr inside w:pPr specifies formatting for the paragraph mark symbol (i.e. the control
 * character than can be configured to be shown). However, in practice MSO also uses it as direct formatting
 * for numbering in that paragraph. I don't know if the problem is in the spec or in MSWord.
 */
static void checkApplyParagraphMarkFormatToNumbering( SwFont* pNumFnt, SwTxtFormatInfo& rInf, const IDocumentSettingAccess* pIDSA )
{
    SwTxtNode* node = rInf.GetTxtFrm()->GetTxtNode();
    if( !pIDSA->get(DocumentSettingId::APPLY_PARAGRAPH_MARK_FORMAT_TO_NUMBERING ))
        return;
    if( SwpHints* hints = node->GetpSwpHints())
    {
        for( size_t i = 0; i < hints->Count(); ++i )
        {
            SwTxtAttr* hint = hints->GetTextHint( i );
            // Formatting for the paragraph mark is set to apply only to the (non-existent) extra character
            // the at end of the txt node.
            if( hint->Which() == RES_TXTATR_AUTOFMT && hint->GetEnd() != NULL
                && hint->GetStart() == *hint->GetEnd() && hint->GetStart() == node->Len())
            {
                std::shared_ptr<SfxItemSet> pSet(hint->GetAutoFmt().GetStyleHandle());

                // Check each item and in case it should be ignored, then clear it.
                std::shared_ptr<SfxItemSet> pCleanedSet;
                if (pSet.get())
                {
                    pCleanedSet.reset(pSet->Clone());

                    SfxItemIter aIter(*pSet);
                    const SfxPoolItem* pItem = aIter.GetCurItem();
                    while (true)
                    {
                        if (SwTxtNode::IsIgnoredCharFmtForNumbering(pItem->Which()))
                            pCleanedSet->ClearItem(pItem->Which());

                        if (aIter.IsAtEnd())
                            break;

                        pItem = aIter.NextItem();
                    }
                }
                pNumFnt->SetDiffFnt(pCleanedSet.get(), pIDSA);
            }
        }
    }
}


SwNumberPortion *SwTxtFormatter::NewNumberPortion( SwTxtFormatInfo &rInf ) const
{
    if( rInf.IsNumDone() || rInf.GetTxtStart() != nStart
                || rInf.GetTxtStart() != rInf.GetIdx() )
        return 0;

    SwNumberPortion *pRet = 0;
    const SwTxtNode* pTxtNd = GetTxtFrm()->GetTxtNode();
    const SwNumRule* pNumRule = pTxtNd->GetNumRule();

    // Has a "valid" number?
    if( pTxtNd->IsNumbered() && pTxtNd->IsCountedInList())
    {
        int nLevel = pTxtNd->GetActualListLevel();

        if (nLevel < 0)
            nLevel = 0;

        if (nLevel >= MAXLEVEL)
            nLevel = MAXLEVEL - 1;

        const SwNumFmt &rNumFmt = pNumRule->Get( nLevel );
        const bool bLeft = SVX_ADJUST_LEFT == rNumFmt.GetNumAdjust();
        const bool bCenter = SVX_ADJUST_CENTER == rNumFmt.GetNumAdjust();
        const bool bLabelAlignmentPosAndSpaceModeActive(
                rNumFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT );
        const sal_uInt16 nMinDist = bLabelAlignmentPosAndSpaceModeActive
                                ? 0 : rNumFmt.GetCharTextDistance();

        if( SVX_NUM_BITMAP == rNumFmt.GetNumberingType() )
        {
            pRet = new SwGrfNumPortion( const_cast<SwTxtFrm*>(GetTxtFrm()),
                                        pTxtNd->GetLabelFollowedBy(),
                                        rNumFmt.GetBrush(),
                                        rNumFmt.GetGraphicOrientation(),
                                        rNumFmt.GetGraphicSize(),
                                        bLeft, bCenter, nMinDist,
                                        bLabelAlignmentPosAndSpaceModeActive );
            long nTmpA = rInf.GetLast()->GetAscent();
            long nTmpD = rInf.GetLast()->Height() - nTmpA;
            if( !rInf.IsTest() )
                static_cast<SwGrfNumPortion*>(pRet)->SetBase( nTmpA, nTmpD, nTmpA, nTmpD );
        }
        else
        {
            // The SwFont is created dynamically and passed in the ctor,
            // as the CharFmt only returns an SV-Font.
            // In the dtor of SwNumberPortion, the SwFont is deleted.
            SwFont *pNumFnt = 0;
            const SwAttrSet* pFmt = rNumFmt.GetCharFmt() ?
                                    &rNumFmt.GetCharFmt()->GetAttrSet() :
                                    NULL;
            const IDocumentSettingAccess* pIDSA = pTxtNd->getIDocumentSettingAccess();

            if( SVX_NUM_CHAR_SPECIAL == rNumFmt.GetNumberingType() )
            {
                const vcl::Font *pFmtFnt = rNumFmt.GetBulletFont();

                // Build a new bullet font basing on the current paragraph font:
                pNumFnt = new SwFont( &rInf.GetCharAttr(), pIDSA );

                // #i53199#
                if ( !pIDSA->get(DocumentSettingId::DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT) )
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

                // Apply the explicit attributes from the character style
                // associated with the numering to the new bullet font.
                if( pFmt )
                    pNumFnt->SetDiffFnt( pFmt, pIDSA );

                checkApplyParagraphMarkFormatToNumbering( pNumFnt, rInf, pIDSA );

                if ( pFmtFnt )
                {
                    const sal_uInt8 nAct = pNumFnt->GetActual();
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
            }
            else
            {
                OUString aTxt( pTxtNd->GetNumString() );
                if ( !aTxt.isEmpty() )
                {
                    aTxt += pTxtNd->GetLabelFollowedBy();
                }

                // Not just an optimization ...
                // A number portion without text will be assigned a width of 0.
                // The succeeding text portion will flow into the BreakCut in the BreakLine,
                // although  we have rInf.GetLast()->GetFlyPortion()!
                if( !aTxt.isEmpty() )
                {

                    // Build a new numbering font basing on the current paragraph font:
                    pNumFnt = new SwFont( &rInf.GetCharAttr(), pIDSA );

                    // #i53199#
                    if ( !pIDSA->get(DocumentSettingId::DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT) )
                    {
                        // i18463:
                        // Underline style of paragraph font should not be considered
                        pNumFnt->SetUnderline( UNDERLINE_NONE );
                        // Overline style of paragraph font should not be considered
                        pNumFnt->SetOverline( UNDERLINE_NONE );
                    }

                    // Apply the explicit attributes from the character style
                    // associated with the numering to the new bullet font.
                    if( pFmt )
                        pNumFnt->SetDiffFnt( pFmt, pIDSA );

                    checkApplyParagraphMarkFormatToNumbering( pNumFnt, rInf, pIDSA );

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
