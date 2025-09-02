/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/config.h>

#include <editeng/crossedoutitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <o3tl/string_view.hxx>
#include <sal/log.hxx>
#include <sax/tools/converter.hxx>
#include <svl/itemiter.hxx>
#include <editeng/fontitem.hxx>
#include <comphelper/string.hxx>
#include <svl/urihelper.hxx>

#include <officecfg/Office/Writer.hxx>

#include <docary.hxx>
#include <fmtpdsc.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <mdiexp.hxx>
#include <ndtxt.hxx>
#include <poolfmt.hxx>
#include <redline.hxx>
#include <strings.hrc>
#include <txatbase.hxx>
#include <charatr.hxx>
#include <fmtcntnt.hxx>
#include <ndgrf.hxx>
#include "wrtmd.hxx"

#include <algorithm>
#include <unordered_map>
#include <vector>

namespace
{
/// Stores information about a to-be-exported linked image.
struct SwMDImageInfo
{
    OUString aURL;
    OUString aTitle;

    SwMDImageInfo(const OUString& rURL, const OUString& rTitle)
        : aURL(rURL)
        , aTitle(rTitle)
    {
    }

    bool operator<(const SwMDImageInfo& rOther) const
    {
        if (aURL < rOther.aURL)
            return true;
        if (rOther.aURL < aURL)
            return false;
        return aTitle < rOther.aTitle;
    }
};

struct FormattingStatus
{
    int nCrossedOutChange = 0;
    int nPostureChange = 0;
    int nUnderlineChange = 0;
    int nWeightChange = 0;
    int nCodeChange = 0;
    std::unordered_map<OUString, int> aHyperlinkChanges;
    std::unordered_map<const SwRangeRedline*, int> aRedlineChanges;
    std::set<SwMDImageInfo> aImages;
};

template <typename T> struct PosData
{
    using value_type = std::pair<sal_Int32, const T*>;
    static bool value_less(const value_type& l, const value_type& r) { return l.first < r.first; }
    std::vector<value_type> table;
    size_t cur = 0;
    const value_type* get(size_t n) const { return n < table.size() ? &table[n] : nullptr; }
    const value_type* current() const { return get(cur); }
    const value_type* next() { return get(++cur); }
    void add(sal_Int32 pos, const T* val) { table.emplace_back(pos, val); }
    void sort() { std::stable_sort(table.begin(), table.end(), value_less); }
};

struct NodePositions
{
    PosData<SfxPoolItem> hintStarts;
    PosData<SfxPoolItem> hintEnds;
    PosData<SwRangeRedline> redlineStarts;
    PosData<SwRangeRedline> redlineEnds;

    sal_Int32 getEndOfCurrent(sal_Int32 end)
    {
        auto pos_of = [](const auto* v) { return v ? v->first : SAL_MAX_INT32; };
        return std::min({
            end,
            pos_of(hintEnds.current()),
            pos_of(hintStarts.current()),
            pos_of(redlineEnds.current()),
            pos_of(redlineStarts.current()),
        });
    }
};

void ApplyItem(SwMDWriter& rWrt, FormattingStatus& rChange, const SfxPoolItem& rItem, int increment)
{
    auto IterateItemSet = [&rWrt, &rChange, increment](const SfxItemSet& set) {
        SfxItemIter iter(set);
        while (!iter.IsAtEnd())
        {
            if (const auto* pNestedItem = iter.GetCurItem())
                ApplyItem(rWrt, rChange, *pNestedItem, increment);
            iter.NextItem();
        }
    };

    auto HandleEnumItem = [&rItem, increment ]<class ItemType>(
        TypedWhichId<ItemType> id, decltype(std::declval<ItemType>().GetValue()) noneValue)
    {
        auto& rItem2 = rItem.StaticWhichCast(id);
        if (rItem2.GetValue() == noneValue)
            return -increment;
        else
            return +increment;
    };

    switch (rItem.Which())
    {
        case RES_CHRATR_CROSSEDOUT:
            rChange.nCrossedOutChange += HandleEnumItem(RES_CHRATR_CROSSEDOUT, STRIKEOUT_NONE);
            break;
        case RES_CHRATR_POSTURE:
            rChange.nPostureChange += HandleEnumItem(RES_CHRATR_POSTURE, ITALIC_NONE);
            break;
        case RES_CHRATR_UNDERLINE:
            rChange.nUnderlineChange += HandleEnumItem(RES_CHRATR_UNDERLINE, LINESTYLE_NONE);
            break;
        case RES_CHRATR_WEIGHT:
            rChange.nWeightChange += HandleEnumItem(RES_CHRATR_WEIGHT, WEIGHT_DONTKNOW);
            break;
        case RES_TXTATR_AUTOFMT:
            if (auto& pStyle = rItem.StaticWhichCast(RES_TXTATR_AUTOFMT).GetStyleHandle())
                IterateItemSet(*pStyle);
            break;
        case RES_TXTATR_INETFMT:
            if (auto& url = rItem.StaticWhichCast(RES_TXTATR_INETFMT).GetValue(); !url.isEmpty())
                rChange.aHyperlinkChanges[url] += increment;
            // TODO: can we store anchors?
            break;
        case RES_TXTATR_CHARFMT:
            if (auto pStyle = rItem.StaticWhichCast(RES_TXTATR_CHARFMT).GetCharFormat())
                IterateItemSet(pStyle->GetAttrSet());
            break;
        case RES_CHRATR_FONT:
        {
            const SvxFontItem& rFontItem = rItem.StaticWhichCast(RES_CHRATR_FONT);
            SwDoc* pDoc = rWrt.m_pDoc;
            IDocumentStylePoolAccess& rIDSPA = pDoc->getIDocumentStylePoolAccess();
            SwTextFormatColl* pColl = rIDSPA.GetTextCollFromPool(RES_POOLCOLL_HTML_PRE);
            if (rFontItem == pColl->GetFont())
            {
                // We know the import uses this font for code spans, so map this back to a code
                // span.
                rChange.nCodeChange += increment;
            }
            break;
        }
        case RES_TXTATR_FLYCNT:
        {
            // Inline image.
            const SwFormatFlyCnt& rFormatFlyCnt = rItem.StaticWhichCast(RES_TXTATR_FLYCNT);
            const SwFrameFormat& rFrameFormat = *rFormatFlyCnt.GetFrameFormat();
            const SwFormatContent& rFlyContent = rFrameFormat.GetContent();
            SwNodeOffset nStart = rFlyContent.GetContentIdx()->GetIndex() + 1;
            SwGrfNode* pGrfNode = rWrt.m_pDoc->GetNodes()[nStart]->GetGrfNode();
            Graphic aGraphic = pGrfNode->GetGraphic();
            if (!pGrfNode->IsLinkedFile())
            {
                // Not linked, ignore for now.
                break;
            }

            // Try to extract a relative URL and a title.
            OUString aGraphicURL;
            pGrfNode->GetFileFilterNms(&aGraphicURL, /*pFilterNm=*/nullptr);
            const OUString& rBaseURL = rWrt.GetBaseURL();
            if (!rBaseURL.isEmpty())
            {
                aGraphicURL = URIHelper::simpleNormalizedMakeRelative(rBaseURL, aGraphicURL);
            }
            OUString aTitle = pGrfNode->GetTitle();
            rChange.aImages.emplace(aGraphicURL, aTitle);
            break;
        }
    }
}

void ApplyItem(FormattingStatus& rChange, const SwRangeRedline* pItem, int increment)
{
    rChange.aRedlineChanges[pItem] += increment;
}

FormattingStatus CalculateFormattingChange(SwMDWriter& rWrt, NodePositions& positions,
                                           sal_Int32 pos, const FormattingStatus& currentFormatting)
{
    FormattingStatus result(currentFormatting);
    // 1. Output closing attributes
    for (auto* p = positions.hintEnds.current(); p && p->first == pos;
         p = positions.hintEnds.next())
        ApplyItem(rWrt, result, *p->second, -1);

    // 2. Output opening attributes
    for (auto* p = positions.hintStarts.current(); p && p->first == pos;
         p = positions.hintStarts.next())
        ApplyItem(rWrt, result, *p->second, +1);

    // 3. Output closing redlines
    for (auto* p = positions.redlineEnds.current(); p && p->first == pos;
         p = positions.redlineEnds.next())
        ApplyItem(result, p->second, -1);

    // 4. Output opening redlines
    for (auto* p = positions.redlineStarts.current(); p && p->first == pos;
         p = positions.redlineStarts.next())
        ApplyItem(result, p->second, +1);

    return result;
}

// Closing redlines may happen in a following paragraph; there it will change from 0 to -1.
// Account for that possibility in ShouldCloseIt.
bool ShouldCloseIt(int prev, int curr) { return prev != curr && prev >= 0 && curr <= 0; }
bool ShouldOpenIt(int prev, int curr) { return prev != curr && prev <= 0 && curr > 0; }

void OutEscapedChars(SwMDWriter& rWrt, std::u16string_view chars);
void OutFormattingChange(SwMDWriter& rWrt, NodePositions& positions, sal_Int32 pos,
                         FormattingStatus& current)
{
    FormattingStatus result = CalculateFormattingChange(rWrt, positions, pos, current);

    // Closing stuff

    // TODO/FIXME: the closing characters must be right-flanking

    if (ShouldCloseIt(current.nCodeChange, result.nCodeChange))
    {
        rWrt.Strm().WriteUnicodeOrByteText(u"`");
    }

    // Not in CommonMark
    if (ShouldCloseIt(current.nCrossedOutChange, result.nCrossedOutChange))
        rWrt.Strm().WriteUnicodeOrByteText(u"~~");

    if (ShouldCloseIt(current.nPostureChange, result.nPostureChange))
        rWrt.Strm().WriteUnicodeOrByteText(u"*");

    if (ShouldCloseIt(current.nUnderlineChange, result.nUnderlineChange))
    {
        //rWrt.Strm().WriteUnicodeOrByteText(u"[/u]");
    }

    if (ShouldCloseIt(current.nWeightChange, result.nWeightChange))
        rWrt.Strm().WriteUnicodeOrByteText(u"**");

    for (const auto & [ url, curr ] : result.aHyperlinkChanges)
    {
        if (ShouldCloseIt(current.aHyperlinkChanges[url], curr))
        {
            rWrt.Strm().WriteUnicodeOrByteText(u"](");
            rWrt.Strm().WriteUnicodeOrByteText(url);
            rWrt.Strm().WriteUnicodeOrByteText(u")");
        }
    }

    for (const auto & [ pRedline, curr ] : result.aRedlineChanges)
    {
        if (ShouldCloseIt(current.aRedlineChanges[pRedline], curr))
        {
            // </ins>
            rWrt.Strm().WriteUnicodeOrByteText(u"</");
            if (pRedline->GetType() == RedlineType::Insert)
                rWrt.Strm().WriteUnicodeOrByteText(u"ins");
            else if (pRedline->GetType() == RedlineType::Delete)
                rWrt.Strm().WriteUnicodeOrByteText(u"del");
            rWrt.Strm().WriteUnicodeOrByteText(u">");
        }
    }

    // Opening stuff

    // TODO/FIXME: the opening characters must be left-flanking

    for (const auto & [ pRedline, curr ] : result.aRedlineChanges)
    {
        if (ShouldOpenIt(current.aRedlineChanges[pRedline], curr))
        {
            // <ins title="Author: John Doe" datetime="2025-07-10T20:00:00">
            rWrt.Strm().WriteUnicodeOrByteText(u"<");
            if (pRedline->GetType() == RedlineType::Insert)
                rWrt.Strm().WriteUnicodeOrByteText(u"ins");
            else if (pRedline->GetType() == RedlineType::Delete)
                rWrt.Strm().WriteUnicodeOrByteText(u"del");
            rWrt.Strm().WriteUnicodeOrByteText(u" title=\"Author: ");
            rWrt.Strm().WriteUnicodeOrByteText(pRedline->GetAuthorString());
            rWrt.Strm().WriteUnicodeOrByteText(u"\" datetime=\"");
            OUStringBuffer buf;
            sax::Converter::convertDateTime(buf, pRedline->GetTimeStamp().GetUNODateTime(),
                                            nullptr);
            rWrt.Strm().WriteUnicodeOrByteText(buf);
            rWrt.Strm().WriteUnicodeOrByteText(u"\">");
        }
    }

    // Not in CommonMark
    if (ShouldOpenIt(current.nCrossedOutChange, result.nCrossedOutChange))
        rWrt.Strm().WriteUnicodeOrByteText(u"~~");

    if (ShouldOpenIt(current.nPostureChange, result.nPostureChange))
        rWrt.Strm().WriteUnicodeOrByteText(u"*");

    if (ShouldOpenIt(current.nUnderlineChange, result.nUnderlineChange))
    {
        //rWrt.Strm().WriteUnicodeOrByteText(u"[u]");
    }

    if (ShouldOpenIt(current.nWeightChange, result.nWeightChange))
        rWrt.Strm().WriteUnicodeOrByteText(u"**");

    for (const auto & [ url, curr ] : result.aHyperlinkChanges)
    {
        if (ShouldOpenIt(current.aHyperlinkChanges[url], curr))
        {
            rWrt.Strm().WriteUnicodeOrByteText(u"[");
        }
    }

    if (ShouldOpenIt(current.nCodeChange, result.nCodeChange))
    {
        rWrt.Strm().WriteUnicodeOrByteText(u"`");
    }

    // Images: write the complete markup on start.
    for (const auto& rImageInfo : result.aImages)
    {
        // 'current' is the old state and 'result' is the new state, so only write images which are
        // new in 'result'.
        if (current.aImages.contains(rImageInfo))
        {
            continue;
        }

        rWrt.Strm().WriteUnicodeOrByteText(u"![");
        OutEscapedChars(rWrt, rImageInfo.aTitle);
        rWrt.Strm().WriteUnicodeOrByteText(u"](");
        rWrt.Strm().WriteUnicodeOrByteText(rImageInfo.aURL);
        rWrt.Strm().WriteUnicodeOrByteText(u")");
    }

    current = result;
}

void OutEscapedChars(SwMDWriter& rWrt, std::u16string_view chars)
{
    for (size_t pos = 0; pos < chars.size();)
    {
        size_t oldpos = pos;
        sal_uInt32 ch = o3tl::iterateCodePoints(chars, &pos);
        switch (ch)
        {
            // dummy characters: anchors, comments, etc. TODO: handle their attributes / content.
            case CH_TXTATR_BREAKWORD:
            case CH_TXTATR_INWORD:
            case CH_TXT_ATR_INPUTFIELDSTART:
            case CH_TXT_ATR_INPUTFIELDEND:
            case CH_TXT_ATR_FORMELEMENT:
            case CH_TXT_ATR_FIELDSTART:
            case CH_TXT_ATR_FIELDSEP:
            case CH_TXT_ATR_FIELDEND:
            case CH_TXT_TRACKED_DUMMY_CHAR:
                break;

                // TODO: line breaks

            case '\\':
            case '`':
            case '*':
            case '_':
            case '{':
            case '}':
            case '[':
            case ']':
            case '<':
            case '>':
            case '#':
                // TODO: should we escape '+', '-', '.', '|'?
                rWrt.Strm().WriteUniOrByteChar('\\');
                [[fallthrough]];
            default:
                rWrt.Strm().WriteUnicodeOrByteText(chars.substr(oldpos, pos - oldpos));
        }
    }
}

/* Output of the nodes*/
void OutMarkdown_SwTextNode(SwMDWriter& rWrt, const SwTextNode& rNode, bool bFirst)
{
    std::optional<SwMDCellInfo> oCellInfo;
    std::stack<SwMDTableInfo>& rTableInfos = rWrt.GetTableInfos();
    if (!rTableInfos.empty())
    {
        SwMDTableInfo& aTableInfo = rTableInfos.top();
        auto it = aTableInfo.aCellInfos.find(rNode.GetIndex());
        if (it != aTableInfo.aCellInfos.end())
        {
            // This text node is at the start or end of a table cell.
            oCellInfo = it->second;
        }
    }

    if (oCellInfo && oCellInfo->bCellStart)
    {
        // Cell start, separate by " | " from the previous cell, see
        // <https://github.github.com/gfm/#tables-extension->.
        if (!oCellInfo->bRowStart)
        {
            rWrt.Strm().WriteUnicodeOrByteText(u" ");
        }

        rWrt.Strm().WriteUnicodeOrByteText(u"| ");
    }

    const OUString& rNodeText = rNode.GetText();
    if (!rNodeText.isEmpty())
    {
        // Paragraphs separate by empty lines
        if (!bFirst && !oCellInfo)
            rWrt.Strm().WriteUnicodeOrByteText(u"" SAL_NEWLINE_STRING);

        int nHeadingLevel = 0;
        for (const SwFormat* pFormat = &rNode.GetAnyFormatColl(); pFormat;
             pFormat = pFormat->DerivedFrom())
        {
            sal_uInt16 nPoolId = pFormat->GetPoolFormatId();
            switch (nPoolId)
            {
                case RES_POOLCOLL_HEADLINE1:
                    if (!nHeadingLevel)
                        nHeadingLevel = 1;
                    break;
                case RES_POOLCOLL_HEADLINE2:
                    if (!nHeadingLevel)
                        nHeadingLevel = 2;
                    break;
                case RES_POOLCOLL_HEADLINE3:
                    if (!nHeadingLevel)
                        nHeadingLevel = 3;
                    break;
                case RES_POOLCOLL_HEADLINE4:
                    if (!nHeadingLevel)
                        nHeadingLevel = 4;
                    break;
                case RES_POOLCOLL_HEADLINE5:
                    if (!nHeadingLevel)
                        nHeadingLevel = 5;
                    break;
                case RES_POOLCOLL_HEADLINE6:
                    if (!nHeadingLevel)
                        nHeadingLevel = 6;
                    break;
                case RES_POOLCOLL_HTML_HR:
                    if (rNodeText.isEmpty())
                    {
                        rWrt.Strm().WriteUnicodeOrByteText(u"___\n");
                        return;
                    }
                    break;
            }
        }

        if (nHeadingLevel > 0)
        {
            for (int i = 0; i < nHeadingLevel; ++i)
                rWrt.Strm().WriteUniOrByteChar('#');
            rWrt.Strm().WriteUniOrByteChar(' ');
        }
        else if (rNode.GetNumRule())
        {
            // <https://spec.commonmark.org/0.31.2/#list-items>, the amount of indent we have to use
            // depends on the parent's prefix size.
            OUStringBuffer aLevel;
            auto it = rWrt.GetListLevelPrefixSizes().find(rNode.GetActualListLevel() - 1);
            if (it != rWrt.GetListLevelPrefixSizes().end())
            {
                comphelper::string::padToLength(aLevel, it->second, ' ');
            }

            // In "1." form, should be one of "1." or "1)".
            OUString aNumString(rNode.GetNumString());
            if (aNumString.isEmpty() && rNode.HasBullet())
            {
                // Should be one of -, +, or *.
                aNumString = u"-"_ustr;
            }

            if (!aLevel.isEmpty() || !aNumString.isEmpty())
            {
                OUString aPrefix = aLevel + aNumString + " ";
                rWrt.Strm().WriteUnicodeOrByteText(aPrefix);
                rWrt.SetListLevelPrefixSize(rNode.GetActualListLevel(), aPrefix.getLength());
            }
        }

        sal_Int32 nStrPos = rWrt.m_pCurrentPam->GetPoint()->GetContentIndex();
        sal_Int32 nEnd = rNodeText.getLength();
        if (rWrt.m_pCurrentPam->GetPoint()->GetNode() == rWrt.m_pCurrentPam->GetMark()->GetNode())
            nEnd = rWrt.m_pCurrentPam->GetMark()->GetContentIndex();

        NodePositions positions;

        // Start paragraph properties
        for (SfxItemIter iter(rNode.GetSwAttrSet()); !iter.IsAtEnd(); iter.NextItem())
            positions.hintStarts.add(nStrPos, iter.GetCurItem());

        // Store character formatting
        const size_t nCntAttr = rNode.HasHints() ? rNode.GetSwpHints().Count() : 0;
        for (size_t i = 0; i < nCntAttr; ++i)
        {
            const SwTextAttr* pHint = rNode.GetSwpHints().Get(i);
            const sal_Int32 nHintStart = pHint->GetStart();
            if (nHintStart >= nEnd)
                break;
            const sal_Int32 nHintEnd = pHint->GetAnyEnd();
            if (nHintStart >= nStrPos && pHint->Which() == RES_TXTATR_FLYCNT)
            {
                // SwTextFlyCnt is a hint that has no end, but still output it.
                positions.hintStarts.add(std::max(nHintStart, nStrPos), &pHint->GetAttr());
                continue;
            }
            else if (nHintEnd == nHintStart || nHintEnd <= nStrPos)
                continue; // no output of zero-length hints and hints ended before output started yet
            positions.hintStarts.add(std::max(nHintStart, nStrPos), &pHint->GetAttr());
            positions.hintEnds.add(std::min(nHintEnd, nEnd), &pHint->GetAttr());
        }

        positions.hintEnds.sort();

        // End paragraph properties
        for (SfxItemIter iter(rNode.GetSwAttrSet()); !iter.IsAtEnd(); iter.NextItem())
            positions.hintEnds.add(nEnd, iter.GetCurItem());

        if (const SwRedlineTable& rRedlines
            = rNode.GetDoc().getIDocumentRedlineAccess().GetRedlineTable();
            !rRedlines.empty() && rRedlines.GetMaxEndPos() >= SwPosition(rNode))
        {
            for (const SwRangeRedline* pRedline : rRedlines)
            {
                const auto[redlineStart, redlineEnd] = pRedline->StartEnd();
                if (redlineStart->GetContentNode()->GetIndex() > rNode.GetIndex()
                    || (redlineStart->GetContentNode()->GetIndex() == rNode.GetIndex()
                        && redlineStart->GetContentIndex() > nEnd))
                    break;
                if (redlineEnd->GetContentNode()->GetIndex() < rNode.GetIndex()
                    || (redlineEnd->GetContentNode()->GetIndex() == rNode.GetIndex()
                        && redlineEnd->GetContentIndex() < nStrPos))
                    continue;

                if (pRedline->GetType() != RedlineType::Insert
                    && pRedline->GetType() != RedlineType::Delete)
                    continue;

                if (*redlineStart->GetContentNode() == rNode
                    && redlineStart->GetContentIndex() >= nStrPos)
                    positions.redlineStarts.add(redlineStart->GetContentIndex(), pRedline);

                if (*redlineEnd->GetContentNode() == rNode && redlineEnd->GetContentIndex() <= nEnd)
                    positions.redlineEnds.add(redlineEnd->GetContentIndex(), pRedline);
            }
        }

        positions.redlineEnds.sort();

        FormattingStatus currentStatus;
        while (nStrPos < nEnd)
        {
            // 1. Output attributes
            OutFormattingChange(rWrt, positions, nStrPos, currentStatus);

            // 2. Escape and output the character. This relies on hints not appearing in the middle of
            // a surrogate pair.
            sal_Int32 nEndOfChunk = positions.getEndOfCurrent(nEnd);
            OutEscapedChars(rWrt, rNodeText.subView(nStrPos, nEndOfChunk - nStrPos));
            nStrPos = nEndOfChunk;
        }
        assert(positions.hintStarts.current() == nullptr);
        // Output final closing attributes
        OutFormattingChange(rWrt, positions, nEnd, currentStatus);
    }

    bool bRowEnd = oCellInfo && oCellInfo->bRowEnd;
    if (bRowEnd)
    {
        // Cell ends are implicit, but row end has its own marker.
        rWrt.Strm().WriteUnicodeOrByteText(u" |");

        if (oCellInfo->bFirstRowEnd)
        {
            // First row has a separator from the remaining rows, which is written even for
            // single-row tables.
            rWrt.Strm().WriteUnicodeOrByteText(u"" SAL_NEWLINE_STRING);
            for (size_t nBox = 0; nBox < oCellInfo->nFirstRowBoxCount; ++nBox)
            {
                rWrt.Strm().WriteUnicodeOrByteText(u"|-");
            }
            rWrt.Strm().WriteUnicodeOrByteText(u"|");
        }
    }

    if (!oCellInfo || bRowEnd)
    {
        rWrt.Strm().WriteUnicodeOrByteText(u"" SAL_NEWLINE_STRING);
    }
}

void OutMarkdown_SwTableNode(SwMDWriter& rWrt, const SwTableNode& rTableNode)
{
    // Scan the SwTable for the interesting start/end nodes, so we know when to write the various
    // separators when the caller will actually traverse the content of the table.
    SwMDTableInfo aTableInfo;
    const SwTable& rTable = rTableNode.GetTable();
    for (size_t nLine = 0; nLine < rTable.GetTabLines().size(); ++nLine)
    {
        const SwTableLine* pLine = rTable.GetTabLines()[nLine];
        for (size_t nBox = 0; nBox < pLine->GetTabBoxes().size(); ++nBox)
        {
            const SwTableBox* pBox = pLine->GetTabBoxes()[nBox];
            const SwStartNode* pStart = pBox->GetSttNd();
            SwMDCellInfo& rStartInfo = aTableInfo.aCellInfos[pStart->GetIndex() + 1];
            const SwEndNode* pEnd = pStart->EndOfSectionNode();
            rStartInfo.bCellStart = true;
            if (nBox == 0)
            {
                rStartInfo.bRowStart = true;
            }
            if (nBox == pLine->GetTabBoxes().size() - 1)
            {
                SwMDCellInfo& rEndInfo = aTableInfo.aCellInfos[pEnd->GetIndex() - 1];
                rEndInfo.bRowEnd = true;
                if (nLine == 0)
                {
                    rEndInfo.bFirstRowEnd = true;
                    rEndInfo.nFirstRowBoxCount = pLine->GetTabBoxes().size();
                }
            }
        }
    }
    aTableInfo.pEndNode = rTableNode.EndOfSectionNode();
    rWrt.GetTableInfos().push(aTableInfo);

    // Separator between the table and the previous content.
    rWrt.Strm().WriteUnicodeOrByteText(u"" SAL_NEWLINE_STRING);
}
}

SwMDWriter::SwMDWriter(const OUString& rBaseURL) { SetBaseURL(rBaseURL); }

ErrCode SwMDWriter::WriteStream()
{
    Strm().SetStreamCharSet(RTL_TEXTENCODING_UTF8);
    if (m_bShowProgress)
        ::StartProgress(STR_STATSTR_W4WWRITE, 0, sal_Int32(m_pDoc->GetNodes().Count()),
                        m_pDoc->GetDocShell());

    // respect table and section at document beginning
    {
        if (m_bWriteAll)
        {
            while (const SwStartNode* pTNd = m_pCurrentPam->GetPointNode().FindTableBoxStartNode())
            {
                // start with table node !!
                m_pCurrentPam->GetPoint()->Assign(*pTNd->FindTableNode());

                if (m_bWriteOnlyFirstTable)
                    m_pCurrentPam->GetMark()->Assign(
                        *m_pCurrentPam->GetPointNode().EndOfSectionNode());
            }
        }

        // first node (which can contain a page break)
        m_nStartNodeIndex = m_pCurrentPam->GetPoint()->GetNode().GetIndex();

        for (SwSectionNode* pSNd = m_pCurrentPam->GetPointNode().FindSectionNode(); pSNd;
             pSNd = pSNd->StartOfSectionNode()->FindSectionNode())
        {
            if (m_bWriteAll)
            {
                // start with section node !!
                m_pCurrentPam->GetPoint()->Assign(*pSNd);
            }
        }
    }

    Out_SwDoc(m_pOrigPam);

    if (m_bShowProgress)
        ::EndProgress(m_pDoc->GetDocShell());
    return ERRCODE_NONE;
}

void SwMDWriter::Out_SwDoc(SwPaM* pPam)
{
    bool bSaveWriteAll = m_bWriteAll;
    bool bIncludeHidden
        = officecfg::Office::Writer::FilterFlags::Markdown::IncludeHiddenText::get();
    bool bFirstLine = true;

    do
    {
        m_bWriteAll = bSaveWriteAll;

        while (*m_pCurrentPam->GetPoint() <= *m_pCurrentPam->GetMark())
        {
            SwNode& rNd = m_pCurrentPam->GetPointNode();

            SAL_WARN_IF(rNd.IsGrfNode() || rNd.IsOLENode(), "sw.md",
                        "Unexpected Grf- or OLE-Node here");

            if (SwTextNode* pTextNd = rNd.GetTextNode())
            {
                if (bIncludeHidden || !pTextNd->IsHidden())
                {
                    if (!bFirstLine)
                        m_pCurrentPam->GetPoint()->SetContent(0);

                    OutMarkdown_SwTextNode(*this, *pTextNd, bFirstLine);
                }
            }
            else if (rNd.IsTableNode())
            {
                OutMarkdown_SwTableNode(*this, *rNd.GetTableNode());
            }
            else if (rNd.IsEndNode() && !m_aTableInfos.empty()
                     && &rNd == m_aTableInfos.top().pEndNode)
            {
                m_aTableInfos.pop();
            }
            else if (rNd.IsSectionNode())
            {
                SwSectionNode* pSectionNode = rNd.GetSectionNode();
                if (!bIncludeHidden && pSectionNode->GetSection().IsHiddenFlag())
                {
                    // Skip
                    m_pCurrentPam->GetPoint()->Assign(*pSectionNode->EndOfSectionNode());
                }
            }
            else if (&rNd == &m_pDoc->GetNodes().GetEndOfContent())
                break;

            m_pCurrentPam->GetPoint()->Adjust(SwNodeOffset(+1)); // move
            SwNodeOffset nPos = m_pCurrentPam->GetPoint()->GetNodeIndex();

            if (m_bShowProgress)
                ::SetProgressState(sal_Int32(nPos), m_pDoc->GetDocShell()); // How far ?

            /* If only the selected area should be saved, so only the complete
             * nodes should be saved, this means the first and n-th node
             * partly, the 2nd till n-1 node complete. (complete means with
             * all formats!)
             */
            m_bWriteAll = bSaveWriteAll || nPos != m_pCurrentPam->GetMark()->GetNodeIndex();
            bFirstLine = false;
        }
    } while (CopyNextPam(&pPam)); // until all PaM's processed

    m_bWriteAll = bSaveWriteAll; // reset to old values
}

void SwMDWriter::SetListLevelPrefixSize(int nListLevel, int nPrefixSize)
{
    m_aListLevelPrefixSizes[nListLevel] = nPrefixSize;
}

void GetMDWriter(std::u16string_view /*rFilterOptions*/, const OUString& rBaseURL, WriterRef& xRet)
{
    xRet = new SwMDWriter(rBaseURL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
