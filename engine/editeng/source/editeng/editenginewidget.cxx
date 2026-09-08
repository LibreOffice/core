/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <editeng/editenginewidget.hxx>

#include <editeng/adjustitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/misspellrange.hxx>
#include <editeng/outliner.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <o3tl/unit_conversion.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svtools/colorcfg.hxx>
#include <tools/json_writer.hxx>
#include <tools/mapunit.hxx>
#include <vcl/event.hxx>
#include <vcl/keycod.hxx>
#include <vcl/svapp.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <algorithm>
#include <optional>
#include <sstream>
#include <vector>

namespace
{
std::optional<boost::property_tree::ptree> parseEventJson(std::u16string_view rData)
{
    std::stringstream aStream(OUStringToOString(rData, RTL_TEXTENCODING_UTF8).getStr());
    boost::property_tree::ptree aTree;
    try
    {
        boost::property_tree::read_json(aStream, aTree);
    }
    catch (const std::exception&)
    {
        SAL_WARN("editeng", "EditEngineWidgetController: malformed JSON event payload");
        return std::nullopt;
    }
    return aTree;
}

OUString toHexColor(const Color& rColor) { return u"#"_ustr + rColor.AsRGBHexString(); }

/// The alignment names match the CSS text-align values the client applies.
OUString adjustName(SvxAdjust eAdjust)
{
    switch (eAdjust)
    {
        case SvxAdjust::Right:
            return u"right"_ustr;
        case SvxAdjust::Center:
            return u"center"_ustr;
        case SvxAdjust::Block:
            return u"justify"_ustr;
        default:
            return u"left"_ustr;
    }
}

OUString underlineName(FontLineStyle eStyle)
{
    switch (eStyle)
    {
        case LINESTYLE_NONE:
            return OUString();
        case LINESTYLE_DOUBLE:
            return u"double"_ustr;
        case LINESTYLE_DOTTED:
        case LINESTYLE_BOLDDOTTED:
            return u"dotted"_ustr;
        case LINESTYLE_DASH:
        case LINESTYLE_LONGDASH:
        case LINESTYLE_DASHDOT:
        case LINESTYLE_DASHDOTDOT:
        case LINESTYLE_BOLDDASH:
        case LINESTYLE_BOLDLONGDASH:
        case LINESTYLE_BOLDDASHDOT:
        case LINESTYLE_BOLDDASHDOTDOT:
            return u"dashed"_ustr;
        case LINESTYLE_WAVE:
        case LINESTYLE_SMALLWAVE:
        case LINESTYLE_DOUBLEWAVE:
        case LINESTYLE_BOLDWAVE:
            return u"wave"_ustr;
        default:
            return u"single"_ustr;
    }
}

/// Escapement is stored as a percentage of the font height, positive up.
OUString escapementName(short nEsc)
{
    if (nEsc > 0)
        return u"super"_ustr;
    if (nEsc < 0)
        return u"sub"_ustr;
    return OUString();
}

/// Converts a font height, held in the item pool's map unit, to the points the client wants.
/// Rounds to the nearest point. Empty when the pool metric has no physical length.
std::optional<sal_Int32> fontSizeInPoints(const SvxFontHeightItem& rItem, const SfxItemPool* pPool)
{
    if (!pPool)
        return std::nullopt;

    const o3tl::Length eLength = MapToO3tlLength(pPool->GetMetric(EE_CHAR_FONTHEIGHT));
    if (eLength == o3tl::Length::invalid)
        return std::nullopt;

    return static_cast<sal_Int32>(o3tl::convert(rItem.GetHeight(), eLength, o3tl::Length::pt));
}

/// A character bullet often comes from a symbol font, such as OpenSymbol, whose glyph sits in the
/// Unicode private use area. A client without that font draws nothing there, so such a glyph is
/// replaced with a plain bullet that renders in any font. Standard characters, like the black
/// circle the presentation defaults use, are kept.
OUString sanitizeBulletText(const OUString& rText)
{
    OUStringBuffer aBuffer(rText.getLength());
    for (sal_Int32 nIndex = 0; nIndex < rText.getLength();)
    {
        const sal_uInt32 nChar = rText.iterateCodePoints(&nIndex);
        if (nChar >= 0xE000 && nChar <= 0xF8FF)
            aBuffer.append(u'\x2022');
        else
            aBuffer.appendUtf32(nChar);
    }
    return aBuffer.makeStringAndClear();
}

/// Collects every position where a run has to be split, so that one run carries one uniform
/// set of attributes. Both ends of the paragraph are always included.
void collectRunBoundaries(std::vector<sal_Int32>& rBoundaries, sal_Int32 nParaLen,
                          const std::vector<EECharAttrib>& rAttribs,
                          const std::vector<editeng::MisspellRange>* pMisspells)
{
    rBoundaries.push_back(0);
    rBoundaries.push_back(nParaLen);

    for (const EECharAttrib& rAttrib : rAttribs)
    {
        rBoundaries.push_back(rAttrib.nStart);
        rBoundaries.push_back(rAttrib.nEnd);
    }

    if (pMisspells)
    {
        for (const editeng::MisspellRange& rRange : *pMisspells)
        {
            rBoundaries.push_back(static_cast<sal_Int32>(rRange.mnStart));
            rBoundaries.push_back(static_cast<sal_Int32>(rRange.mnEnd));
        }
    }

    std::sort(rBoundaries.begin(), rBoundaries.end());
    rBoundaries.erase(std::unique(rBoundaries.begin(), rBoundaries.end()), rBoundaries.end());
    std::erase_if(rBoundaries, [nParaLen](sal_Int32 nPos) { return nPos < 0 || nPos > nParaLen; });
}

bool isMisspelled(const std::vector<editeng::MisspellRange>* pMisspells, sal_Int32 nStart,
                  sal_Int32 nEnd)
{
    if (!pMisspells)
        return false;

    return std::any_of(pMisspells->begin(), pMisspells->end(),
                       [nStart, nEnd](const editeng::MisspellRange& rRange) {
                           return static_cast<sal_Int32>(
                               rRange.mnStart)<nEnd&& static_cast<sal_Int32>(rRange.mnEnd)>
                               nStart;
                       });
}
}

EditEngineWidgetController::EditEngineWidgetController(EditView& rEditView, Outliner* pOutliner)
    : m_rEditView(rEditView)
    , m_rEditEngine(rEditView.getEditEngine())
    , m_pOutliner(pOutliner)
    , m_aUpdateIdle("EditEngineWidgetController update")
{
    m_aUpdateIdle.SetPriority(TaskPriority::HIGHEST);
    m_aUpdateIdle.SetInvokeHandler(LINK(this, EditEngineWidgetController, UpdateIdleHdl));
}

EditEngineWidgetController::~EditEngineWidgetController() { m_aUpdateIdle.Stop(); }

void EditEngineWidgetController::QueueUpdate() { m_aUpdateIdle.Start(); }

IMPL_LINK_NOARG(EditEngineWidgetController, UpdateIdleHdl, Timer*, void) { SendUpdateAction(); }

void EditEngineWidgetController::DumpRuns(tools::JsonWriter& rWriter, sal_Int32 nPara,
                                         const SfxItemSet& rParagraphBaseline)
{
    const sal_Int32 nParaLen = m_rEditEngine.GetTextLen(nPara);

    std::vector<EECharAttrib> aAttribs;
    m_rEditEngine.GetCharAttribs(nPara, aAttribs);

    std::vector<editeng::MisspellRanges> aMisspellRanges;
    m_rEditEngine.GetAllMisspellRanges(aMisspellRanges);
    const std::vector<editeng::MisspellRange>* pMisspells = nullptr;
    for (const editeng::MisspellRanges& rRanges : aMisspellRanges)
    {
        if (rRanges.mnParagraph == nPara)
        {
            pMisspells = &rRanges.maRanges;
            break;
        }
    }

    std::vector<sal_Int32> aBoundaries;
    collectRunBoundaries(aBoundaries, nParaLen, aAttribs, pMisspells);

    auto aRunsNode = rWriter.startArray("runs");

    for (size_t i = 0; i + 1 < aBoundaries.size(); ++i)
    {
        const sal_Int32 nStart = aBoundaries[i];
        const sal_Int32 nEnd = aBoundaries[i + 1];
        if (nStart == nEnd)
            continue;

        // Asking for the effective set rather than only the hard formatting, so that what the
        // style sheet and the pool defaults contribute is rendered too.
        const SfxItemSet aSet
            = m_rEditEngine.GetAttribs(nPara, nStart, nEnd, GetAttribsFlags::CHARATTRIBS);

        auto aRunNode = rWriter.startStruct();
        rWriter.put("start", nStart);
        rWriter.put("end", nEnd);

        if (const SvxWeightItem* pItem = aSet.GetItemIfSet(EE_CHAR_WEIGHT))
        {
            if (pItem->GetWeight() > WEIGHT_NORMAL)
                rWriter.put("bold", true);
        }
        if (const SvxPostureItem* pItem = aSet.GetItemIfSet(EE_CHAR_ITALIC))
        {
            if (pItem->GetPosture() != ITALIC_NONE)
                rWriter.put("italic", true);
        }
        if (const SvxUnderlineItem* pItem = aSet.GetItemIfSet(EE_CHAR_UNDERLINE))
        {
            const OUString sUnderline = underlineName(pItem->GetLineStyle());
            if (!sUnderline.isEmpty())
                rWriter.put("underline", sUnderline);
        }
        if (const SvxCrossedOutItem* pItem = aSet.GetItemIfSet(EE_CHAR_STRIKEOUT))
        {
            if (pItem->GetStrikeout() != STRIKEOUT_NONE)
                rWriter.put("strikeout", true);
        }
        // The paragraph carries its own family, size and colour, and the client applies them to
        // the whole paragraph, so a run only repeats one of them when it actually differs.
        if (const SvxColorItem* pItem = aSet.GetItemIfSet(EE_CHAR_COLOR))
        {
            const SvxColorItem* pBase = rParagraphBaseline.GetItemIfSet(EE_CHAR_COLOR);
            if (!pItem->GetValue().IsTransparent()
                && (!pBase || pBase->GetValue() != pItem->GetValue()))
                rWriter.put("color", toHexColor(pItem->GetValue()));
        }
        if (const SvxFontItem* pItem = aSet.GetItemIfSet(EE_CHAR_FONTINFO))
        {
            const SvxFontItem* pBase = rParagraphBaseline.GetItemIfSet(EE_CHAR_FONTINFO);
            if (!pBase || pBase->GetFamilyName() != pItem->GetFamilyName())
                rWriter.put("family", pItem->GetFamilyName());
        }
        if (const SvxFontHeightItem* pItem = aSet.GetItemIfSet(EE_CHAR_FONTHEIGHT))
        {
            const SvxFontHeightItem* pBase = rParagraphBaseline.GetItemIfSet(EE_CHAR_FONTHEIGHT);
            if (!pBase || pBase->GetHeight() != pItem->GetHeight())
            {
                if (std::optional<sal_Int32> oPoints
                    = fontSizeInPoints(*pItem, m_rEditEngine.GetItemPool()))
                    rWriter.put("size", *oPoints);
            }
        }
        if (const SvxEscapementItem* pItem = aSet.GetItemIfSet(EE_CHAR_ESCAPEMENT))
        {
            const OUString sEscapement = escapementName(pItem->GetEsc());
            if (!sEscapement.isEmpty())
                rWriter.put("escapement", sEscapement);
        }

        if (isMisspelled(pMisspells, nStart, nEnd))
            rWriter.put("spellError", true);
    }
}

void EditEngineWidgetController::DumpParagraph(tools::JsonWriter& rWriter, sal_Int32 nPara)
{
    auto aParaNode = rWriter.startStruct();

    rWriter.put("text", m_rEditEngine.GetText(nPara));

    const SfxItemSet& rParaSet = m_rEditEngine.GetParaAttribs(nPara);
    if (const SvxAdjustItem* pItem = rParaSet.GetItemIfSet(EE_PARA_JUST))
        rWriter.put("align", adjustName(pItem->GetAdjust()));

    if (m_pOutliner)
    {
        rWriter.put("depth", m_pOutliner->GetDepth(nPara));

        const EBulletInfo aBullet = m_pOutliner->GetBulletInfo(nPara);
        if (aBullet.bVisible && !aBullet.aText.isEmpty())
            rWriter.put("bulletText", sanitizeBulletText(aBullet.aText));
    }

    // The character formatting shared by the whole paragraph. The client applies it to the
    // paragraph and each run inherits it, so a run only carries the attributes it changes. For a
    // paragraph in more than one font or size the shared item is not set, and the runs carry it.
    const SfxItemSet aBaseline = m_rEditEngine.GetAttribs(
        nPara, 0, m_rEditEngine.GetTextLen(nPara), GetAttribsFlags::CHARATTRIBS);
    if (const SvxFontItem* pItem = aBaseline.GetItemIfSet(EE_CHAR_FONTINFO))
        rWriter.put("family", pItem->GetFamilyName());
    if (const SvxFontHeightItem* pItem = aBaseline.GetItemIfSet(EE_CHAR_FONTHEIGHT))
    {
        if (std::optional<sal_Int32> oPoints = fontSizeInPoints(*pItem, m_rEditEngine.GetItemPool()))
            rWriter.put("size", *oPoints);
    }
    if (const SvxColorItem* pItem = aBaseline.GetItemIfSet(EE_CHAR_COLOR))
    {
        if (!pItem->GetValue().IsTransparent())
            rWriter.put("color", toHexColor(pItem->GetValue()));
    }

    DumpRuns(rWriter, nPara, aBaseline);
}

void EditEngineWidgetController::DumpWidgetData(tools::JsonWriter& rWriter)
{
    {
        auto aParagraphsNode = rWriter.startArray("paragraphs");
        const sal_Int32 nParaCount = m_rEditEngine.GetParagraphCount();
        for (sal_Int32 nPara = 0; nPara < nParaCount; ++nPara)
            DumpParagraph(rWriter, nPara);
    }

    {
        ESelection aSelection = m_rEditView.GetSelection();
        aSelection.Adjust();

        auto aSelectionNode = rWriter.startNode("selection");
        rWriter.put("startPara", aSelection.start.nPara);
        rWriter.put("startIndex", aSelection.start.nIndex);
        rWriter.put("endPara", aSelection.end.nPara);
        rWriter.put("endIndex", aSelection.end.nIndex);
    }

    svtools::ColorConfig aColorConfig;
    rWriter.put("backgroundColor",
                toHexColor(aColorConfig.GetColorValue(svtools::DOCCOLOR).nColor));
    rWriter.put("readOnly", m_rEditView.IsReadOnly());

    {
        auto aExtraNode = rWriter.startNode("extra");
        DumpExtraData(rWriter);
    }
}

bool EditEngineWidgetController::HandleKey(std::u16string_view rData)
{
    std::optional<boost::property_tree::ptree> oTree = parseEventJson(rData);
    if (!oTree)
        return true;

    // The key code arrives with its modifier bits already folded in, the same value the client
    // sends for the document itself, so vcl::KeyCode takes it whole.
    const sal_uInt16 nKeyCode = oTree->get<sal_uInt16>("keyCode", 0);
    const sal_uInt16 nCharCode = oTree->get<sal_uInt16>("charCode", 0);
    const sal_uInt16 nRepeat = oTree->get<sal_uInt16>("repeat", 0);

    const KeyEvent aKeyEvent(nCharCode, vcl::KeyCode(nKeyCode), nRepeat);

    // An outliner handles Return, Tab and numbering at the outline level and keeps its paragraph
    // list in step with the edit engine, so the key goes through its view when there is one. Posting
    // straight to the edit view would split a paragraph behind the outliner's back.
    OutlinerView* pOutlinerView = m_pOutliner ? m_pOutliner->GetView(0) : nullptr;
    if (pOutlinerView)
        pOutlinerView->PostKeyEvent(aKeyEvent);
    else
        m_rEditView.PostKeyEvent(aKeyEvent);

    // A caret move alone leaves the model untouched, so the modify handler will not fire.
    QueueUpdate();
    return true;
}

bool EditEngineWidgetController::HandleSelection(std::u16string_view rData)
{
    std::optional<boost::property_tree::ptree> oTree = parseEventJson(rData);
    if (!oTree)
        return true;

    // The selection comes from the client, and the edit engine clamps a position that reaches
    // past the end of a paragraph but not one below its start, so a negative value would edit
    // at an index of its own.
    const sal_Int32 nStartPara = std::max<sal_Int32>(0, oTree->get<sal_Int32>("startPara", 0));
    const sal_Int32 nStartIndex = std::max<sal_Int32>(0, oTree->get<sal_Int32>("startIndex", 0));
    const sal_Int32 nEndPara = std::max<sal_Int32>(0, oTree->get<sal_Int32>("endPara", nStartPara));
    const sal_Int32 nEndIndex
        = std::max<sal_Int32>(0, oTree->get<sal_Int32>("endIndex", nStartIndex));

    ESelection aSelection(nStartPara, nStartIndex, nEndPara, nEndIndex);
    aSelection.Adjust();
    m_rEditView.SetSelection(aSelection);
    return true;
}

bool EditEngineWidgetController::HandleText(std::u16string_view rData)
{
    // Posted key events go through the edit engine, which honours the read-only state itself, but
    // InsertText does not, so the check has to happen here to match.
    if (m_rEditView.IsReadOnly())
        return true;

    std::optional<boost::property_tree::ptree> oTree = parseEventJson(rData);
    if (!oTree)
        return true;

    const OUString sText = OUString::fromUtf8(oTree->get<std::string>("text", std::string()));
    if (sText.isEmpty())
        return true;

    m_rEditView.InsertText(sText);
    QueueUpdate();
    return true;
}

bool EditEngineWidgetController::HandleCustomEvent(const OUString& rCmd, const OUString& rData)
{
    SolarMutexGuard aGuard;

    if (rCmd == u"key")
        return HandleKey(rData);
    if (rCmd == u"selection")
        return HandleSelection(rData);
    if (rCmd == u"text")
        return HandleText(rData);

    if (HandleExtraEvent(rCmd, rData))
        return true;

    SAL_WARN("editeng", "EditEngineWidgetController: unknown custom event '" << rCmd << "'");
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
