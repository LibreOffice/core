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
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <charfmt.hxx>

#include <viewsh.hxx>
#include <doc.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>
#include <viewopt.hxx>
#include <flyfrm.hxx>
#include <viewimp.hxx>
#include <txtatr.hxx>
#include <swfont.hxx>
#include <fntcache.hxx>
#include "porfld.hxx"
#include "porftn.hxx"
#include "porref.hxx"
#include "portox.hxx"
#include "porhyph.hxx"
#include "porfly.hxx"
#include "itrform2.hxx"
#include <chpfld.hxx>
#include <dbfld.hxx>
#include <expfld.hxx>
#include <docufld.hxx>
#include <pagedesc.hxx>
#include "pormulti.hxx"
#include <fmtmeta.hxx>
#include <reffld.hxx>
#include <flddat.hxx>
#include <fmtautofmt.hxx>
#include <IDocumentSettingAccess.hxx>
#include <svl/itemiter.hxx>

static bool lcl_IsInBody( SwFrame const *pFrame )
{
    if ( pFrame->IsInDocBody() )
        return true;
    else
    {
        const SwFrame *pTmp = pFrame;
        const SwFlyFrame *pFly;
        while ( nullptr != (pFly = pTmp->FindFlyFrame()) )
            pTmp = pFly->GetAnchorFrame();
        return pTmp->IsInDocBody();
    }
}

SwExpandPortion *SwTextFormatter::NewFieldPortion( SwTextFormatInfo &rInf,
                                                const SwTextAttr *pHint ) const
{
    SwExpandPortion *pRet = nullptr;
    SwFrame *pFrame = m_pFrame;
    SwField *pField = const_cast<SwField*>(pHint->GetFormatField().GetField());
    const bool bName = rInf.GetOpt().IsFieldName();

    SwCharFormat* pChFormat = nullptr;
    bool bNewFlyPor = false;
    sal_uInt16 subType = 0;

    // set language
    const_cast<SwTextFormatter*>(this)->SeekAndChg( rInf );
    if (pField->GetLanguage() != GetFnt()->GetLanguage())
    {
        pField->SetLanguage( GetFnt()->GetLanguage() );
        // let the visual note know about its new language
        if (pField->GetTyp()->Which()==SwFieldIds::Postit)
            const_cast<SwFormatField*> (&pHint->GetFormatField())->Broadcast( SwFormatFieldHint( &pHint->GetFormatField(), SwFormatFieldHintWhich::LANGUAGE ) );
    }

    SwViewShell *pSh = rInf.GetVsh();
    SwDoc *const pDoc( pSh ? pSh->GetDoc() : nullptr );
    bool const bInClipboard( pDoc == nullptr || pDoc->IsClipBoard() );
    bool bPlaceHolder = false;

    switch( pField->GetTyp()->Which() )
    {
        case SwFieldIds::Script:
        case SwFieldIds::Postit:
            pRet = new SwPostItsPortion( SwFieldIds::Script == pField->GetTyp()->Which() );
            break;

        case SwFieldIds::CombinedChars:
            {
                if( bName )
                    pRet = new SwFieldPortion( pField->GetFieldName() );
                else
                    pRet = new SwCombinedPortion( pField->ExpandField(bInClipboard, pFrame->getRootFrame()) );
            }
            break;

        case SwFieldIds::HiddenText:
            {
                OUString const aStr( bName
                    ? pField->GetFieldName()
                    : pField->ExpandField(bInClipboard, pFrame->getRootFrame()) );
                pRet = new SwHiddenPortion(aStr);
            }
            break;

        case SwFieldIds::Chapter:
            if( !bName && pSh && !pSh->Imp()->IsUpdateExpFields() )
            {
                static_cast<SwChapterField*>(pField)->ChangeExpansion(*pFrame,
                    &static_txtattr_cast<SwTextField const*>(pHint)->GetTextNode());
            }
            {
                OUString const aStr( bName
                    ? pField->GetFieldName()
                    : pField->ExpandField(bInClipboard, pFrame->getRootFrame()) );
                pRet = new SwFieldPortion( aStr );
            }
            break;

        case SwFieldIds::DocStat:
            if( !bName && pSh && !pSh->Imp()->IsUpdateExpFields() )
            {
                static_cast<SwDocStatField*>(pField)->ChangeExpansion( pFrame );
            }
            {
                OUString const aStr( bName
                    ? pField->GetFieldName()
                    : pField->ExpandField(bInClipboard, pFrame->getRootFrame()) );
                pRet = new SwFieldPortion( aStr );
            }
            static_cast<SwFieldPortion*>(pRet)->m_nAttrFieldType= ATTR_PAGECOOUNTFLD;
            break;

        case SwFieldIds::PageNumber:
        {
            if( !bName && pSh && pSh->GetLayout() && !pSh->Imp()->IsUpdateExpFields() )
            {
                SwPageNumberFieldType *pPageNr = static_cast<SwPageNumberFieldType *>(pField->GetTyp());

                const SwRootFrame* pTmpRootFrame = pSh->GetLayout();
                const bool bVirt = pTmpRootFrame->IsVirtPageNum();

                sal_uInt16 nVirtNum = pFrame->GetVirtPageNum();
                sal_uInt16 nNumPages = pTmpRootFrame->GetPageNum();
                SvxNumType nNumFormat = SvxNumType(-1);
                if(SVX_NUM_PAGEDESC == pField->GetFormat())
                    nNumFormat = pFrame->FindPageFrame()->GetPageDesc()->GetNumType().GetNumberingType();
                static_cast<SwPageNumberField*>(pField)
                    ->ChangeExpansion(nVirtNum, nNumPages);
                pPageNr->ChangeExpansion(pDoc,
                                            bVirt, nNumFormat != SvxNumType(-1) ? &nNumFormat : nullptr);
            }
            {
                OUString const aStr( bName
                    ? pField->GetFieldName()
                    : pField->ExpandField(bInClipboard, pFrame->getRootFrame()) );
                pRet = new SwFieldPortion( aStr );
            }
            static_cast<SwFieldPortion*>(pRet)->m_nAttrFieldType= ATTR_PAGENUMBERFLD;
            break;
        }
        case SwFieldIds::GetExp:
        {
            if( !bName && pSh && !pSh->Imp()->IsUpdateExpFields() )
            {
                SwGetExpField* pExpField = static_cast<SwGetExpField*>(pField);
                if( !::lcl_IsInBody( pFrame ) )
                {
                    pExpField->ChgBodyTextFlag( false );
                    pExpField->ChangeExpansion(*pFrame,
                            *static_txtattr_cast<SwTextField const*>(pHint));
                }
                else if( !pExpField->IsInBodyText() )
                {
                    // Was something else previously, thus: expand first, then convert it!
                    pExpField->ChangeExpansion(*pFrame,
                            *static_txtattr_cast<SwTextField const*>(pHint));
                    pExpField->ChgBodyTextFlag( true );
                }
            }
            {
                OUString const aStr( bName
                    ? pField->GetFieldName()
                    : pField->ExpandField(bInClipboard, pFrame->getRootFrame()) );
                pRet = new SwFieldPortion( aStr );
            }
            break;
        }
        case SwFieldIds::Database:
        {
            if( !bName )
            {
                SwDBField* pDBField = static_cast<SwDBField*>(pField);
                pDBField->ChgBodyTextFlag( ::lcl_IsInBody( pFrame ) );
            }
            {
                OUString const aStr( bName
                    ? pField->GetFieldName()
                    : pField->ExpandField(bInClipboard, pFrame->getRootFrame()) );
                pRet = new SwFieldPortion(aStr);
            }
            break;
        }
        case SwFieldIds::RefPageGet:
            if( !bName && pSh && !pSh->Imp()->IsUpdateExpFields() )
            {
                static_cast<SwRefPageGetField*>(pField)->ChangeExpansion(*pFrame,
                        static_txtattr_cast<SwTextField const*>(pHint));
            }
            {
                OUString const aStr( bName
                    ? pField->GetFieldName()
                    : pField->ExpandField(bInClipboard, pFrame->getRootFrame()) );
                pRet = new SwFieldPortion(aStr);
            }
            break;

        case SwFieldIds::JumpEdit:
            if( !bName )
                pChFormat = static_cast<SwJumpEditField*>(pField)->GetCharFormat();
            bNewFlyPor = true;
            bPlaceHolder = true;
            break;
        case SwFieldIds::GetRef:
            subType = static_cast<SwGetRefField*>(pField)->GetSubType();
            {
                OUString const str( bName
                    ? pField->GetFieldName()
                    : pField->ExpandField(bInClipboard, pFrame->getRootFrame()) );
                pRet = new SwFieldPortion(str);
            }
            if( subType == REF_BOOKMARK  )
                static_cast<SwFieldPortion*>(pRet)->m_nAttrFieldType = ATTR_BOOKMARKFLD;
            else if( subType == REF_SETREFATTR )
                static_cast<SwFieldPortion*>(pRet)->m_nAttrFieldType = ATTR_SETREFATTRFLD;
            break;
        case SwFieldIds::DateTime:
            subType = static_cast<SwDateTimeField*>(pField)->GetSubType();
            {
                OUString const str( bName
                    ? pField->GetFieldName()
                    : pField->ExpandField(bInClipboard, pFrame->getRootFrame()) );
                pRet = new SwFieldPortion(str);
            }
            if( subType & DATEFLD  )
                static_cast<SwFieldPortion*>(pRet)->m_nAttrFieldType= ATTR_DATEFLD;
            else if( subType & TIMEFLD )
                static_cast<SwFieldPortion*>(pRet)->m_nAttrFieldType = ATTR_TIMEFLD;
            break;
        default:
            {
                OUString const aStr( bName
                    ? pField->GetFieldName()
                    : pField->ExpandField(bInClipboard, pFrame->getRootFrame()) );
                pRet = new SwFieldPortion(aStr);
            }
    }

    if( bNewFlyPor )
    {
        std::unique_ptr<SwFont> pTmpFnt;
        if( !bName )
        {
            pTmpFnt.reset(new SwFont( *m_pFont ));
            pTmpFnt->SetDiffFnt(&pChFormat->GetAttrSet(), &m_pFrame->GetDoc().getIDocumentSettingAccess());
        }
        OUString const aStr( bName
            ? pField->GetFieldName()
            : pField->ExpandField(bInClipboard, pFrame->getRootFrame()) );
        pRet = new SwFieldPortion(aStr, std::move(pTmpFnt), bPlaceHolder);
    }

    return pRet;
}

static SwFieldPortion * lcl_NewMetaPortion(SwTextAttr & rHint, const bool bPrefix)
{
    ::sw::Meta *const pMeta(
        static_cast<SwFormatMeta &>(rHint.GetAttr()).GetMeta() );
    OUString fix;
    ::sw::MetaField *const pField( dynamic_cast< ::sw::MetaField * >(pMeta) );
    OSL_ENSURE(pField, "lcl_NewMetaPortion: no meta field?");
    if (pField)
    {
        pField->GetPrefixAndSuffix(bPrefix ? &fix : nullptr, bPrefix ? nullptr : &fix);
    }
    return new SwFieldPortion( fix );
}

/**
 * Try to create a new portion with zero length, for an end of a hint
 * (where there is no CH_TXTATR). Because there may be multiple hint ends at a
 * given index, m_pByEndIter is used to keep track of the already created
 * portions. But the portions created here may actually be deleted again,
 * due to Underflow. In that case, m_pByEndIter must be decremented,
 * so the portion will be created again on the next line.
 */
SwExpandPortion * SwTextFormatter::TryNewNoLengthPortion(SwTextFormatInfo const & rInfo)
{
    const TextFrameIndex nIdx(rInfo.GetIdx());

    // sw_redlinehide: because there is a dummy character at the start of these
    // hints, it's impossible to have ends of hints from different nodes at the
    // same view position, so it's sufficient to check the hints of the current
    // node.  However, m_pByEndIter exists for the whole text frame, so
    // it's necessary to iterate all hints for that purpose...
    if (!m_pByEndIter)
    {
        m_pByEndIter.reset(new sw::MergedAttrIterByEnd(*rInfo.GetTextFrame()));
    }
    SwTextNode const* pNode(nullptr);
    for (SwTextAttr const* pHint = m_pByEndIter->NextAttr(pNode); pHint;
         pHint = m_pByEndIter->NextAttr(pNode))
    {
        SwTextAttr & rHint(const_cast<SwTextAttr&>(*pHint));
        TextFrameIndex const nEnd(
            rInfo.GetTextFrame()->MapModelToView(pNode, *rHint.GetAnyEnd()));
        if (nEnd > nIdx)
        {
            m_pByEndIter->PrevAttr();
            break;
        }
        if (nEnd == nIdx)
        {
            if (RES_TXTATR_METAFIELD == rHint.Which())
            {
                SwFieldPortion *const pPortion(
                        lcl_NewMetaPortion(rHint, false));
                pPortion->SetNoLength(); // no CH_TXTATR at hint end!
                return pPortion;
            }
        }
    }
    return nullptr;
}

SwLinePortion *SwTextFormatter::NewExtraPortion( SwTextFormatInfo &rInf )
{
    SwTextAttr *pHint = GetAttr( rInf.GetIdx() );
    SwLinePortion *pRet = nullptr;
    if( !pHint )
    {
        pRet = new SwTextPortion;
        pRet->SetLen(TextFrameIndex(1));
        rInf.SetLen(TextFrameIndex(1));
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
            pRet = NewFootnotePortion( rInf, pHint );
            break;
        }
    case RES_TXTATR_FIELD :
    case RES_TXTATR_ANNOTATION :
        {
            pRet = NewFieldPortion( rInf, pHint );
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
        pRet = new SwFieldPortion( aNothing );
        rInf.SetLen(TextFrameIndex(1));
    }
    return pRet;
}

/**
 * OOXML spec says that w:rPr inside w:pPr specifies formatting for the paragraph mark symbol (i.e. the control
 * character than can be configured to be shown). However, in practice MSO also uses it as direct formatting
 * for numbering in that paragraph. I don't know if the problem is in the spec or in MSWord.
 */
static void checkApplyParagraphMarkFormatToNumbering(SwFont* pNumFnt, SwTextFormatInfo& rInf,
                                                     const IDocumentSettingAccess* pIDSA,
                                                     const SwAttrSet* pFormat)
{
    if( !pIDSA->get(DocumentSettingId::APPLY_PARAGRAPH_MARK_FORMAT_TO_NUMBERING ))
        return;
    TextFrameIndex const nTextLen(rInf.GetTextFrame()->GetText().getLength());
    SwTextNode const* pNode(nullptr);
    sw::MergedAttrIterReverse iter(*rInf.GetTextFrame());
    for (SwTextAttr const* pHint = iter.PrevAttr(&pNode); pHint;
         pHint = iter.PrevAttr(&pNode))
    {
        TextFrameIndex const nHintEnd(
            rInf.GetTextFrame()->MapModelToView(pNode, *pHint->GetAnyEnd()));
        if (nHintEnd < nTextLen)
        {
            break; // only those at para end are interesting
        }
        // Formatting for the paragraph mark is set to apply only to the
        // (non-existent) extra character at end of the text node.
        if (pHint->Which() == RES_TXTATR_AUTOFMT
            && pHint->GetStart() == *pHint->End())
        {
            std::shared_ptr<SfxItemSet> pSet(pHint->GetAutoFormat().GetStyleHandle());

            // Check each item and in case it should be ignored, then clear it.
            std::unique_ptr<SfxItemSet> pCleanedSet;
            if (pSet.get())
            {
                pCleanedSet = pSet->Clone();

                SfxItemIter aIter(*pSet);
                const SfxPoolItem* pItem = aIter.GetCurItem();
                while (true)
                {
                    if (SwTextNode::IsIgnoredCharFormatForNumbering(pItem->Which()))
                        pCleanedSet->ClearItem(pItem->Which());
                    else if (pFormat && pFormat->HasItem(pItem->Which()))
                        pCleanedSet->ClearItem(pItem->Which());

                    if (aIter.IsAtEnd())
                        break;

                    pItem = aIter.NextItem();
                }
            }

            // Highlightcolor also needed to be untouched, but we can't have that just by clearing the item
            Color nSaveHighlight = pNumFnt->GetHighlightColor();

            pNumFnt->SetDiffFnt(pCleanedSet.get(), pIDSA);
            pNumFnt->SetHighlightColor(nSaveHighlight);
        }
    }
}


SwNumberPortion *SwTextFormatter::NewNumberPortion( SwTextFormatInfo &rInf ) const
{
    if( rInf.IsNumDone() || rInf.GetTextStart() != m_nStart
                || rInf.GetTextStart() != rInf.GetIdx() )
        return nullptr;

    SwNumberPortion *pRet = nullptr;
    // sw_redlinehide: at this point it's certain that pTextNd is the node with
    // the numbering of the frame; only the actual number-vector (GetNumString)
    // depends on the hide-mode in the layout so other calls don't need to care
    const SwTextNode *const pTextNd = GetTextFrame()->GetTextNodeForParaProps();
    const SwNumRule* pNumRule = pTextNd->GetNumRule();

    // Has a "valid" number?
    // sw_redlinehide: check that pParaPropsNode is the correct one
    assert(pTextNd->IsNumbered(m_pFrame->getRootFrame()) == pTextNd->IsNumbered(nullptr));
    if (pTextNd->IsNumbered(m_pFrame->getRootFrame()) && pTextNd->IsCountedInList())
    {
        int nLevel = pTextNd->GetActualListLevel();

        if (nLevel < 0)
            nLevel = 0;

        if (nLevel >= MAXLEVEL)
            nLevel = MAXLEVEL - 1;

        const SwNumFormat &rNumFormat = pNumRule->Get( nLevel );
        const bool bLeft = SvxAdjust::Left == rNumFormat.GetNumAdjust();
        const bool bCenter = SvxAdjust::Center == rNumFormat.GetNumAdjust();
        const bool bLabelAlignmentPosAndSpaceModeActive(
                rNumFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT );
        const sal_uInt16 nMinDist = bLabelAlignmentPosAndSpaceModeActive
                                ? 0 : rNumFormat.GetCharTextDistance();

        if( SVX_NUM_BITMAP == rNumFormat.GetNumberingType() )
        {
            pRet = new SwGrfNumPortion( pTextNd->GetLabelFollowedBy(),
                                        rNumFormat.GetBrush(),
                                        rNumFormat.GetGraphicOrientation(),
                                        rNumFormat.GetGraphicSize(),
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
            // as the CharFormat only returns an SV-Font.
            // In the dtor of SwNumberPortion, the SwFont is deleted.
            const SwAttrSet* pFormat = rNumFormat.GetCharFormat() ?
                                    &rNumFormat.GetCharFormat()->GetAttrSet() :
                                    nullptr;
            const IDocumentSettingAccess* pIDSA = pTextNd->getIDocumentSettingAccess();

            if( SVX_NUM_CHAR_SPECIAL == rNumFormat.GetNumberingType() )
            {
                const vcl::Font *pFormatFnt = rNumFormat.GetBulletFont();

                // Build a new bullet font basing on the current paragraph font:
                std::unique_ptr<SwFont> pNumFnt(new SwFont( &rInf.GetCharAttr(), pIDSA ));

                // #i53199#
                if ( !pIDSA->get(DocumentSettingId::DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT) )
                {
                    // i18463:
                    // Underline style of paragraph font should not be considered
                    // Overline style of paragraph font should not be considered
                    // Weight style of paragraph font should not be considered
                    // Posture style of paragraph font should not be considered
                    pNumFnt->SetUnderline( LINESTYLE_NONE );
                    pNumFnt->SetOverline( LINESTYLE_NONE );
                    pNumFnt->SetItalic( ITALIC_NONE, SwFontScript::Latin );
                    pNumFnt->SetItalic( ITALIC_NONE, SwFontScript::CJK );
                    pNumFnt->SetItalic( ITALIC_NONE, SwFontScript::CTL );
                    pNumFnt->SetWeight( WEIGHT_NORMAL, SwFontScript::Latin );
                    pNumFnt->SetWeight( WEIGHT_NORMAL, SwFontScript::CJK );
                    pNumFnt->SetWeight( WEIGHT_NORMAL, SwFontScript::CTL );
                }

                // Apply the explicit attributes from the character style
                // associated with the numbering to the new bullet font.
                if( pFormat )
                    pNumFnt->SetDiffFnt( pFormat, pIDSA );

                checkApplyParagraphMarkFormatToNumbering(pNumFnt.get(), rInf, pIDSA, pFormat);

                if ( pFormatFnt )
                {
                    const SwFontScript nAct = pNumFnt->GetActual();
                    pNumFnt->SetFamily( pFormatFnt->GetFamilyType(), nAct );
                    pNumFnt->SetName( pFormatFnt->GetFamilyName(), nAct );
                    pNumFnt->SetStyleName( pFormatFnt->GetStyleName(), nAct );
                    pNumFnt->SetCharSet( pFormatFnt->GetCharSet(), nAct );
                    pNumFnt->SetPitch( pFormatFnt->GetPitch(), nAct );
                }

                // we do not allow a vertical font
                pNumFnt->SetVertical( pNumFnt->GetOrientation(),
                                      m_pFrame->IsVertical() );

                // --> OD 2008-01-23 #newlistelevelattrs#
                pRet = new SwBulletPortion( rNumFormat.GetBulletChar(),
                                            pTextNd->GetLabelFollowedBy(),
                                            std::move(pNumFnt),
                                            bLeft, bCenter, nMinDist,
                                            bLabelAlignmentPosAndSpaceModeActive );
            }
            else
            {
                OUString aText( pTextNd->GetNumString(true, MAXLEVEL, m_pFrame->getRootFrame()) );
                if ( !aText.isEmpty() )
                {
                    aText += pTextNd->GetLabelFollowedBy();
                }

                // Not just an optimization ...
                // A number portion without text will be assigned a width of 0.
                // The succeeding text portion will flow into the BreakCut in the BreakLine,
                // although  we have rInf.GetLast()->GetFlyPortion()!
                if( !aText.isEmpty() )
                {

                    // Build a new numbering font basing on the current paragraph font:
                    std::unique_ptr<SwFont> pNumFnt(new SwFont( &rInf.GetCharAttr(), pIDSA ));

                    // #i53199#
                    if ( !pIDSA->get(DocumentSettingId::DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT) )
                    {
                        // i18463:
                        // Underline style of paragraph font should not be considered
                        pNumFnt->SetUnderline( LINESTYLE_NONE );
                        // Overline style of paragraph font should not be considered
                        pNumFnt->SetOverline( LINESTYLE_NONE );
                    }

                    // Apply the explicit attributes from the character style
                    // associated with the numbering to the new bullet font.
                    if( pFormat )
                        pNumFnt->SetDiffFnt( pFormat, pIDSA );

                    checkApplyParagraphMarkFormatToNumbering(pNumFnt.get(), rInf, pIDSA, pFormat);

                    // we do not allow a vertical font
                    pNumFnt->SetVertical( pNumFnt->GetOrientation(), m_pFrame->IsVertical() );

                    pRet = new SwNumberPortion( aText, std::move(pNumFnt),
                                                bLeft, bCenter, nMinDist,
                                                bLabelAlignmentPosAndSpaceModeActive );
                }
            }
        }
    }
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
