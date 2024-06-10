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

#include <doc.hxx>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <vcl/help.hxx>
#include <tools/json_writer.hxx>
#include <tools/urlobj.hxx>
#include <fmtrfmrk.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/sfxhelp.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <swmodule.hxx>
#include <modcfg.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <edtwin.hxx>
#include <dpage.hxx>
#include <docufld.hxx>
#include <reffld.hxx>
#include <cellatr.hxx>
#include <shdwcrsr.hxx>
#include <fmtinfmt.hxx>
#include <fmtftn.hxx>
#include <redline.hxx>
#include <tox.hxx>
#include <txatbase.hxx>
#include <uitool.hxx>
#include <viewopt.hxx>
#include <strings.hrc>

#include <IDocumentMarkAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <txtfrm.hxx>
#include <ndtxt.hxx>
#include <comphelper/lok.hxx>
#include <authfld.hxx>

#include <com/sun/star/text/XTextRange.hpp>
#include <unotextrange.hxx>
#include <SwStyleNameMapper.hxx>
#include <unoprnms.hxx>
#include <editeng/unoprnms.hxx>
#include <rootfrm.hxx>
#include <unomap.hxx>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

namespace {

bool HasValidPropertyValue(const uno::Any& rAny)
{
    if (bool bValue; rAny >>= bValue)
    {
        return true;
    }
    else if (OUString aValue; (rAny >>= aValue) && !(aValue.isEmpty()))
    {
        return true;
    }
    else if (awt::FontSlant eValue; rAny >>= eValue)
    {
        return true;
    }
    else if (tools::Long nValueLong; rAny >>= nValueLong)
    {
        return true;
    }
    else if (double fValue; rAny >>= fValue)
    {
        return true;
    }
    else if (short nValueShort; rAny >>= nValueShort)
    {
        return true;
    }
    else
        return false;
}

bool PSCSDFPropsQuickHelp(const HelpEvent &rEvt, SwWrtShell& rSh)
{
    OUString sText;
    SwView& rView = rSh.GetView();

    if (rView.IsHighlightCharDF() || rView.GetStylesHighlighterParaColorMap().size()
            || rView.GetStylesHighlighterCharColorMap().size())
    {
        SwPosition aPos(rSh.GetDoc()->GetNodes());
        Point aPt(rSh.GetWin()->PixelToLogic(
                      rSh.GetWin()->ScreenToOutputPixel(rEvt.GetMousePosPixel())));

        rSh.GetLayout()->GetModelPositionForViewPoint(&aPos, aPt);

        if (!aPos.GetContentNode()->IsTextNode())
            return false;

        rtl::Reference<SwXTextRange> xRange(
                    SwXTextRange::CreateXTextRange(*(rView.GetDocShell()->GetDoc()),
                                                   aPos, &aPos));
        if (!xRange)
            throw uno::RuntimeException();

        SwContentFrame* pContentFrame = aPos.GetContentNode()->GetTextNode()->getLayoutFrame(
                            rSh.GetLayout());

        SwRect aFrameAreaRect;

        bool bContainsPt = false;
        do
        {
            aFrameAreaRect = pContentFrame->getFrameArea();
            if (aFrameAreaRect.Contains(aPt))
            {
                bContainsPt = true;
                break;
            }
        } while((pContentFrame = pContentFrame->GetFollow()));

        if (bContainsPt)
        {
            if (rView.GetStylesHighlighterCharColorMap().size())
            {
                // check if in CS formatting highlighted area
                OUString sCharStyle;
                xRange->getPropertyValue(u"CharStyleName"_ustr) >>= sCharStyle;
                if (!sCharStyle.isEmpty())
                    sText = SwStyleNameMapper::GetUIName(sCharStyle, SwGetPoolIdFromName::ChrFmt);
            }

            if (sText.isEmpty() && rView.IsHighlightCharDF())
            {
                // check if in direct formatting highlighted area
                const std::vector<OUString> aHiddenProperties{ UNO_NAME_RSID,
                            UNO_NAME_PARA_IS_NUMBERING_RESTART,
                            UNO_NAME_PARA_STYLE_NAME,
                            UNO_NAME_PARA_CONDITIONAL_STYLE_NAME,
                            UNO_NAME_PAGE_STYLE_NAME,
                            UNO_NAME_NUMBERING_START_VALUE,
                            UNO_NAME_NUMBERING_IS_NUMBER,
                            UNO_NAME_PARA_CONTINUEING_PREVIOUS_SUB_TREE,
                            UNO_NAME_CHAR_STYLE_NAME,
                            UNO_NAME_NUMBERING_LEVEL,
                            UNO_NAME_SORTED_TEXT_ID,
                            UNO_NAME_PARRSID,
                            UNO_NAME_CHAR_COLOR_THEME,
                            UNO_NAME_CHAR_COLOR_TINT_OR_SHADE };

                SfxItemPropertySet const& rPropSet(
                            *aSwMapProvider.GetPropertySet(PROPERTY_MAP_CHAR_AUTO_STYLE));
                SfxItemPropertyMap const& rMap(rPropSet.getPropertyMap());

                const uno::Sequence<beans::Property> aProperties
                        = xRange->getPropertySetInfo()->getProperties();

                for (const beans::Property& rProperty : aProperties)
                {
                    const OUString& rPropName = rProperty.Name;

                    if (!rMap.hasPropertyByName(rPropName))
                        continue;

                    if (std::find(aHiddenProperties.begin(), aHiddenProperties.end(), rPropName)
                            != aHiddenProperties.end())
                        continue;

                    if (xRange->getPropertyState(rPropName)
                            == beans::PropertyState_DIRECT_VALUE)
                    {
                        const uno::Any aAny = xRange->getPropertyValue(rPropName);
                        if (HasValidPropertyValue(aAny))
                        {
                            sText = SwResId(STR_CHARACTER_DIRECT_FORMATTING);
                            break;
                        }
                    }
                }
            }
        }
        else if (rView.GetStylesHighlighterParaColorMap().size())
        {
            // check if in paragraph style formatting highlighted area
            pContentFrame = aPos.GetContentNode()->GetTextNode()->getLayoutFrame(
                        rSh.GetLayout());
            do
            {
                aFrameAreaRect = pContentFrame->getFrameArea();
                if (pContentFrame->IsRightToLeft())
                {
                    aFrameAreaRect.AddRight(375);
                    aFrameAreaRect.Left(aFrameAreaRect.Right() - 300);
                }
                else
                {
                    aFrameAreaRect.AddLeft(-375);
                    aFrameAreaRect.Right(aFrameAreaRect.Left() + 300);
                }
                if (aFrameAreaRect.Contains(aPt))
                {
                    OUString sParaStyle;
                    xRange->getPropertyValue(u"ParaStyleName"_ustr) >>= sParaStyle;
                    sText = SwStyleNameMapper::GetUIName(sParaStyle, SwGetPoolIdFromName::TxtColl);
                    // check for paragraph direct formatting
                    if (SwDoc::HasParagraphDirectFormatting(aPos))
                        sText = sText + " + " + SwResId(STR_PARAGRAPH_DIRECT_FORMATTING);
                    break;
                }
            } while((pContentFrame = pContentFrame->GetFollow()));
        }
    }

    if (!sText.isEmpty())
    {
        tools::Rectangle aRect(rSh.GetWin()->PixelToLogic(
                                   rSh.GetWin()->ScreenToOutputPixel(rEvt.GetMousePosPixel())),
                               Size(1, 1));
        Point aPt(rSh.GetWin()->OutputToScreenPixel(rSh.GetWin()->LogicToPixel(aRect.TopLeft())));
        aRect.SetLeft(aPt.X());
        aRect.SetTop(aPt.Y());
        aPt = rSh.GetWin()->OutputToScreenPixel(rSh.GetWin()->LogicToPixel(aRect.BottomRight()));
        aRect.SetRight(aPt.X());
        aRect.SetBottom(aPt.Y());

        // tdf#136336 ensure tooltip area surrounds the current mouse position with at least a pixel margin
        aRect.Union(tools::Rectangle(rEvt.GetMousePosPixel(), Size(1, 1)));
        aRect.AdjustLeft(-1);
        aRect.AdjustRight(1);
        aRect.AdjustTop(-1);
        aRect.AdjustBottom(1);

        QuickHelpFlags nStyle = QuickHelpFlags::NONE; //TipStyleBalloon;
        Help::ShowQuickHelp(rSh.GetWin(), aRect, sText, nStyle);
    }

    return !sText.isEmpty();
}
}

static OUString lcl_GetRedlineHelp( const SwRangeRedline& rRedl, bool bBalloon,
                                    bool bTableChange, bool bTableColChange )
{
    TranslateId pResId;
    switch( rRedl.GetType() )
    {
    case RedlineType::Insert:   pResId = bTableChange
        ? !bTableColChange
            ? STR_REDLINE_TABLE_ROW_INSERT
            : STR_REDLINE_TABLE_COLUMN_INSERT
        :  rRedl.IsMoved()
            ? STR_REDLINE_INSERT_MOVED
            : STR_REDLINE_INSERT;
        break;
    case RedlineType::Delete:   pResId = bTableChange
        ? !bTableColChange
            ? STR_REDLINE_TABLE_ROW_DELETE
            : STR_REDLINE_TABLE_COLUMN_DELETE
        : rRedl.IsMoved()
            ? STR_REDLINE_DELETE_MOVED
            : STR_REDLINE_DELETE;
        break;
    case RedlineType::Format:   pResId = STR_REDLINE_FORMAT; break;
    case RedlineType::Table:    pResId = STR_REDLINE_TABLE; break;
    case RedlineType::FmtColl:  pResId = STR_REDLINE_FMTCOLL; break;
    case RedlineType::ParagraphFormat: pResId = STR_REDLINE_PARAGRAPH_FORMAT; break;
    case RedlineType::TableRowInsert: pResId = STR_REDLINE_TABLE_ROW_INSERT; break;
    case RedlineType::TableRowDelete: pResId = STR_REDLINE_TABLE_ROW_DELETE; break;
    case RedlineType::TableCellInsert: pResId = STR_REDLINE_TABLE_CELL_INSERT; break;
    case RedlineType::TableCellDelete: pResId = STR_REDLINE_TABLE_CELL_DELETE; break;
    default: break;
    }

    if (!pResId)
        return OUString();
    OUStringBuffer sBuf(SwResId(pResId)
            + ": "
            + rRedl.GetAuthorString()
            + " - "
            + GetAppLangDateTimeString(rRedl.GetTimeStamp()));
    if( bBalloon && !rRedl.GetComment().isEmpty() )
        sBuf.append("\n" + rRedl.GetComment());
    return sBuf.makeStringAndClear();
}

OUString SwEditWin::ClipLongToolTip(const OUString& rText)
{
    OUString sDisplayText(rText);
    tools::Long nTextWidth = GetTextWidth(sDisplayText);
    tools::Long nMaxWidth = GetDesktopRectPixel().GetWidth() * 2 / 3;
    nMaxWidth = PixelToLogic(Size(nMaxWidth, 0)).Width();
    if (nTextWidth > nMaxWidth)
        sDisplayText = GetOutDev()->GetEllipsisString(sDisplayText, nMaxWidth, DrawTextFlags::CenterEllipsis);
    return sDisplayText;
}

static OString getTooltipPayload(const OUString& tooltip, const SwRect& rect)
{
    tools::JsonWriter writer;
    {
        writer.put("type", "generaltooltip");
        writer.put("text", tooltip);
        writer.put("rectangle", rect.SVRect().toString());
    }
    return writer.finishAndGetAsOString();
}

void SwEditWin::RequestHelp(const HelpEvent &rEvt)
{
    SwWrtShell &rSh = m_rView.GetWrtShell();

    if (PSCSDFPropsQuickHelp(rEvt, rSh))
        return;

    bool bQuickBalloon = bool(rEvt.GetMode() & ( HelpEventMode::QUICK | HelpEventMode::BALLOON ));
    if(bQuickBalloon && !rSh.GetViewOptions()->IsShowContentTips())
        return;
    bool bContinue = true;
    bool bScreenTip = false;
    CurrShell aCurr(&rSh);
    OUString sText;
    Point aPt( PixelToLogic( ScreenToOutputPixel( rEvt.GetMousePosPixel() ) ));
    bool bBalloon = bool(rEvt.GetMode() & HelpEventMode::BALLOON);

    SdrView *pSdrView = rSh.GetDrawView();

    if( bQuickBalloon && pSdrView )
    {
        SdrPageView* pPV = pSdrView->GetSdrPageView();
        SwDPage* pPage = pPV ? static_cast<SwDPage*>(pPV->GetPage()) : nullptr;
        bContinue = pPage && pPage->RequestHelp(this, pSdrView, rEvt);
    }

    if( bContinue && bQuickBalloon)
    {
        SwRect aFieldRect;
        SwContentAtPos aContentAtPos( IsAttrAtPos::Field |
                                    IsAttrAtPos::InetAttr |
                                    IsAttrAtPos::Ftn |
                                    IsAttrAtPos::Redline |
                                    IsAttrAtPos::ToxMark |
                                    IsAttrAtPos::RefMark |
                                    IsAttrAtPos::SmartTag |
#ifdef DBG_UTIL
                                    IsAttrAtPos::TableBoxValue |
                                    ( bBalloon ? IsAttrAtPos::CurrAttrs : IsAttrAtPos::NONE) |
#endif
                                    IsAttrAtPos::TableBoxFml |
                                    IsAttrAtPos::TableRedline |
                                    IsAttrAtPos::TableColRedline );

        if( rSh.GetContentAtPos( aPt, aContentAtPos, false, &aFieldRect ) )
        {
            QuickHelpFlags nStyle = QuickHelpFlags::NONE; // style of quick help
            switch( aContentAtPos.eContentAtPos )
            {
            case IsAttrAtPos::TableBoxFml:
                sText = "= " + static_cast<const SwTableBoxFormula*>(aContentAtPos.aFnd.pAttr)->GetFormula();
                break;
#ifdef DBG_UTIL
            case IsAttrAtPos::TableBoxValue:
            {
                if(aContentAtPos.aFnd.pAttr)
                {
                    sText = OUString::number(
                                static_cast<const SwTableBoxValue*>(aContentAtPos.aFnd.pAttr)->GetValue());
                }
                break;
            }
            case IsAttrAtPos::CurrAttrs:
                sText = aContentAtPos.sStr;
                break;
#endif

            case IsAttrAtPos::InetAttr:
            {
                sText = static_cast<const SwFormatINetFormat*>(aContentAtPos.aFnd.pAttr)->GetValue();
                sText = URIHelper::removePassword( sText,
                                        INetURLObject::EncodeMechanism::WasEncoded,
                                           INetURLObject::DecodeMechanism::Unambiguous);
                //#i63832# remove the link target type
                sal_Int32 nFound = sText.indexOf(cMarkSeparator);
                if( nFound != -1 && (++nFound) < sText.getLength() )
                {
                    std::u16string_view sSuffix( sText.subView(nFound) );
                    if( sSuffix == u"table" ||
                        sSuffix == u"frame" ||
                        sSuffix == u"region" ||
                        sSuffix == u"outline" ||
                        sSuffix == u"text" ||
                        sSuffix == u"graphic" ||
                        sSuffix == u"ole" ||
                        sSuffix == u"drawingobject" )
                        sText = sText.copy( 0, nFound - 1);
                }
                // #i104300#
                // special handling if target is a cross-reference bookmark
                {
                    OUString sTmpSearchStr = sText.copy( 1 );
                    IDocumentMarkAccess* pMarkAccess = rSh.getIDocumentMarkAccess();
                    IDocumentMarkAccess::const_iterator_t ppBkmk =
                                    pMarkAccess->findBookmark( sTmpSearchStr );
                    if ( ppBkmk != pMarkAccess->getBookmarksEnd() &&
                         IDocumentMarkAccess::GetType(**ppBkmk)
                            == IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK )
                    {
                        SwTextNode* pTextNode = (*ppBkmk)->GetMarkStart().GetNode().GetTextNode();
                        if ( pTextNode )
                        {
                            sText = sw::GetExpandTextMerged(rSh.GetLayout(), *pTextNode, true, false, ExpandMode(0));

                            if( !sText.isEmpty() )
                            {
                                OUStringBuffer sTmp(sText.replaceAll(u"\u00ad", ""));
                                for (sal_Int32 i = 0; i < sTmp.getLength(); ++i)
                                {
                                    if (sTmp[i] < 0x20)
                                        sTmp[i] = 0x20;
                                    else if (sTmp[i] == 0x2011)
                                        sTmp[i] = '-';
                                }
                                sText = sTmp.makeStringAndClear();
                            }
                        }
                    }
                }
                // #i80029#
                bool bExecHyperlinks = m_rView.GetDocShell()->IsReadOnly();
                if ( !bExecHyperlinks )
                {
                    sText = SfxHelp::GetURLHelpText(sText);

                    SwPosition aPos(rSh.GetDoc()->GetNodes());
                    rSh.GetLayout()->GetModelPositionForViewPoint(&aPos, aPt);
                    rtl::Reference<SwXTextRange> xRange(SwXTextRange::CreateXTextRange(
                        *(m_rView.GetDocShell()->GetDoc()), aPos, &aPos));

                    OUString sName;
                    xRange->getPropertyValue("HyperLinkName") >>= sName;
                    if (!sName.isEmpty())
                    {
                        bScreenTip = true;
                        OUStringBuffer sStrBuffer(sName);
                        sal_Int32 nTextLen = sText.getLength();
                        sal_Int32 nNameLen = sName.getLength();
                        if (nNameLen > nTextLen)
                        {
                            for (sal_Int32 i = nTextLen - 1; i < nNameLen; i += nTextLen)
                                sStrBuffer.insert(i + 1, std::u16string_view(u"\n"));
                        }
                        sText = sStrBuffer.makeStringAndClear() + "\n" + sText;
                    }
                }
                break;
            }
            case IsAttrAtPos::SmartTag:
            {
                vcl::KeyCode aCode( KEY_SPACE );
                vcl::KeyCode aModifiedCode( KEY_SPACE, KEY_MOD1 );
                OUString aModStr( aModifiedCode.GetName() );
                aModStr = aModStr.replaceFirst(aCode.GetName(), "");
                aModStr = aModStr.replaceAll("+", "");
                sText = SwResId(STR_SMARTTAG_CLICK).replaceAll("%s", aModStr);
                break;
            }

            case IsAttrAtPos::Ftn:
                if( aContentAtPos.pFndTextAttr && aContentAtPos.aFnd.pAttr )
                {
                    const SwFormatFootnote* pFootnote = static_cast<const SwFormatFootnote*>(aContentAtPos.aFnd.pAttr);
                    OUString sTmp(pFootnote->GetFootnoteText(*rSh.GetLayout()));
                    sText = SwResId( pFootnote->IsEndNote()
                                    ? STR_ENDNOTE : STR_FTNNOTE ) + sTmp;
                    bBalloon = true;
                    if( aContentAtPos.IsInRTLText() )
                        nStyle |= QuickHelpFlags::BiDiRtl;
                }
                break;

            case IsAttrAtPos::TableRedline:
            case IsAttrAtPos::TableColRedline:
            case IsAttrAtPos::Redline:
            {
                const bool bShowTrackChanges = IDocumentRedlineAccess::IsShowChanges( m_rView.GetDocShell()->GetDoc()->getIDocumentRedlineAccess().GetRedlineFlags() );
                const bool bShowInlineTooltips = rSh.GetViewOptions()->IsShowInlineTooltips();
                if ( bShowTrackChanges && bShowInlineTooltips )
                {
                     sText = lcl_GetRedlineHelp(*aContentAtPos.aFnd.pRedl, bBalloon,
                         IsAttrAtPos::TableRedline == aContentAtPos.eContentAtPos ||
                         IsAttrAtPos::TableColRedline == aContentAtPos.eContentAtPos,
                         IsAttrAtPos::TableColRedline == aContentAtPos.eContentAtPos);
                }
                break;
            }

            case IsAttrAtPos::ToxMark:
                sText = aContentAtPos.sStr;
                if( !sText.isEmpty() && aContentAtPos.pFndTextAttr )
                {
                    const SwTOXType* pTType = aContentAtPos.pFndTextAttr->
                                        GetTOXMark().GetTOXType();
                    if( pTType && !pTType->GetTypeName().isEmpty() )
                    {
                        sText = ": " + sText;
                        sText = pTType->GetTypeName() + sText;
                    }
                }
                break;

            case IsAttrAtPos::RefMark:
                if(aContentAtPos.aFnd.pAttr)
                {
                    sText = SwResId(STR_CONTENT_TYPE_SINGLE_REFERENCE) + ": " +
                        static_cast<const SwFormatRefMark*>(aContentAtPos.aFnd.pAttr)->GetRefName();
                }
            break;

            default:
                {
                    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
                    if(!pModOpt->IsHideFieldTips())
                    {
                        const SwField* pField = aContentAtPos.aFnd.pField;
                        switch( pField->Which() )
                        {
                        case SwFieldIds::SetExp:
                        case SwFieldIds::Table:
                        case SwFieldIds::GetExp:
                        {
                            sal_uInt16 nOldSubType = pField->GetSubType();
                            const_cast<SwField*>(pField)->SetSubType(nsSwExtendedSubType::SUB_CMD);
                            sText = pField->ExpandField(true, rSh.GetLayout());
                            const_cast<SwField*>(pField)->SetSubType(nOldSubType);
                        }
                        break;

                        case SwFieldIds::Postit:
                            {
                                break;
                            }
                        case SwFieldIds::Input:  // BubbleHelp, because the suggestion could be quite long
                            bBalloon = true;
                            [[fallthrough]];
                        case SwFieldIds::Dropdown:
                        case SwFieldIds::JumpEdit:
                            sText = pField->GetPar2();
                            break;

                        case SwFieldIds::Database:
                            sText = pField->GetFieldName();
                            break;

                        case SwFieldIds::User:
                        {
                            OUString aTitle = pField->GetTitle();
                            if (!aTitle.isEmpty())
                            {
                                sText = aTitle;
                            }
                            else
                            {
                                sText = pField->GetPar1();
                            }
                            break;
                        }
                        case SwFieldIds::HiddenText:
                            sText = pField->GetPar1();
                            break;

                        case SwFieldIds::DocStat:
                            break;

                        case SwFieldIds::Macro:
                            sText = static_cast<const SwMacroField*>(pField)->GetMacro();
                            break;

                        case SwFieldIds::GetRef:
                        {
                            // #i85090#
                            const SwGetRefField* pRefField( dynamic_cast<const SwGetRefField*>(pField) );
                            OSL_ENSURE( pRefField,
                                    "<SwEditWin::RequestHelp(..)> - unexpected type of <pField>" );
                            if ( pRefField )
                            {
                                if ( pRefField->IsRefToHeadingCrossRefBookmark() ||
                                     pRefField->IsRefToNumItemCrossRefBookmark() )
                                {
                                    sText = pRefField->GetExpandedTextOfReferencedTextNode(*rSh.GetLayout());
                                    if ( sText.getLength() > 80  )
                                    {
                                        sText = OUString::Concat(sText.subView(0, 80)) + "...";
                                    }
                                }
                                else
                                {
                                    sText = pRefField->GetSetRefName();
                                }
                            }
                            break;
                        }
                        case SwFieldIds::TableOfAuthorities:
                        {
                            const auto pAuthorityField = static_cast<const SwAuthorityField*>(pField);
                            sText = pAuthorityField->GetAuthority(rSh.GetLayout());

                            if (auto t = pAuthorityField->GetTargetType();
                                t == SwAuthorityField::TargetType::UseDisplayURL
                                || t == SwAuthorityField::TargetType::UseTargetURL)
                            {
                                const OUString& rURL = pAuthorityField->GetAbsoluteURL();
                                sText += "\n" + SfxHelp::GetURLHelpText(rURL);
                            }

                            break;
                        }

                        default: break;
                        }
                    }

                    if( sText.isEmpty() )
                    {
                        const bool bShowTrackChanges = IDocumentRedlineAccess::IsShowChanges( m_rView.GetDocShell()->GetDoc()->getIDocumentRedlineAccess().GetRedlineFlags() );
                        const bool bShowInlineTooltips = rSh.GetViewOptions()->IsShowInlineTooltips();
                        if ( bShowTrackChanges && bShowInlineTooltips )
                        {
                            aContentAtPos.eContentAtPos = IsAttrAtPos::Redline;
                            if( rSh.GetContentAtPos( aPt, aContentAtPos, false, &aFieldRect ) )
                                sText = lcl_GetRedlineHelp(*aContentAtPos.aFnd.pRedl, bBalloon, /*bTableChange=*/false, /*bTableColChange=*/false);
                        }
                    }
                }
            }
            if (!sText.isEmpty())
            {
                if (comphelper::LibreOfficeKit::isActive())
                {
                    m_rView.libreOfficeKitViewCallback(
                        LOK_CALLBACK_TOOLTIP, getTooltipPayload(sText, aFieldRect));
                }
                else
                {
                    tools::Rectangle aRect(aFieldRect.SVRect());
                    Point aRectPt(OutputToScreenPixel(LogicToPixel(aRect.TopLeft())));
                    aRect.SetLeft(aRectPt.X());
                    aRect.SetTop(aRectPt.Y());
                    aRectPt = OutputToScreenPixel(LogicToPixel(aRect.BottomRight()));
                    aRect.SetRight(aRectPt.X());
                    aRect.SetBottom(aRectPt.Y());

                    // tdf#136336 ensure tooltip area surrounds the current mouse position with at least a pixel margin
                    aRect.Union(tools::Rectangle(rEvt.GetMousePosPixel(), Size(1, 1)));
                    aRect.AdjustLeft(-1);
                    aRect.AdjustRight(1);
                    aRect.AdjustTop(-1);
                    aRect.AdjustBottom(1);

                    if (bBalloon)
                        Help::ShowBalloon(this, rEvt.GetMousePosPixel(), aRect, sText);
                    else
                    {
                        // the show the help
                        OUString sDisplayText(sText);
                        if (!bScreenTip)
                            sDisplayText = ClipLongToolTip(sText);
                        Help::ShowQuickHelp(this, aRect, sDisplayText, nStyle);
                    }
                }
            }

            bContinue = false;
        }

    }

    if( bContinue )
        Window::RequestHelp( rEvt );
}

void SwEditWin::PrePaint(vcl::RenderContext& /*rRenderContext*/)
{
    if (SwWrtShell* pWrtShell = GetView().GetWrtShellPtr())
        pWrtShell->PrePaint();
}

void SwEditWin::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    SwWrtShell* pWrtShell = GetView().GetWrtShellPtr();
    if(!pWrtShell)
        return;

    if ( GetView().GetVisArea().GetWidth()  <= 0 ||
              GetView().GetVisArea().GetHeight() <= 0 )
        Invalidate( rRect );
    else
    {
        pWrtShell->setOutputToWindow(true);
        bool bTiledPainting = false;
        if (comphelper::LibreOfficeKit::isActive())
        {
            bTiledPainting = comphelper::LibreOfficeKit::isTiledPainting();
            comphelper::LibreOfficeKit::setTiledPainting(true);
        }
        pWrtShell->Paint(rRenderContext, rRect);
        if (comphelper::LibreOfficeKit::isActive())
        {
            comphelper::LibreOfficeKit::setTiledPainting(bTiledPainting);
        }
        pWrtShell->setOutputToWindow(false);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
