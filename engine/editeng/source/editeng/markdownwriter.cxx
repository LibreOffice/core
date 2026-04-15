/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "markdownwriter.hxx"

#include <ParagraphPortion.hxx>
#include <ParagraphPortionList.hxx>
#include <TextPortion.hxx>

#include <editeng/editeng.hxx>
#include <editeng/eeitem.hxx>

#include <editeng/crossedoutitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/flditem.hxx>

#include <svl/intitem.hxx>
#include <svl/itemset.hxx>

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

namespace editeng
{
MarkdownWriter::MarkdownWriter(const EditEngine& rEngine)
    : mrEngine(rEngine)
{
}

ErrCode MarkdownWriter::Write(SvStream& rOutput, EditSelection aSel) const
{
    aSel.Adjust(mrEngine.GetEditDoc());

    sal_Int32 nStartNode = mrEngine.GetEditDoc().GetPos(aSel.Min().GetNode());
    sal_Int32 nEndNode = mrEngine.GetEditDoc().GetPos(aSel.Max().GetNode());

    WriteContent([&rOutput](std::string_view s) { rOutput.WriteOString(s); }, nStartNode, nEndNode,
                 aSel.Min().GetIndex(), aSel.Max().GetIndex());

    return rOutput.GetError();
}

OString MarkdownWriter::GetString() const
{
    sal_Int32 nEndNode = mrEngine.GetEditDoc().Count() - 1;
    if (nEndNode == 0 && mrEngine.GetEditDoc().GetObject(0)->Len() == 0)
        return OString();

    OStringBuffer aOutput;
    sal_Int32 nEndPos = mrEngine.GetEditDoc().GetObject(nEndNode)->Len();

    WriteContent([&aOutput](std::string_view s) { aOutput.append(s); }, 0, nEndNode, 0, nEndPos);

    return aOutput.makeStringAndClear();
}

bool MarkdownWriter::IsMarkdownSpecial(sal_Unicode c)
{
    switch (c)
    {
        case '\\':
        case '*':
        case '_':
        case '~':
        case '[':
        case ']':
        case '(':
        case ')':
        case '`':
        case '|':
        case '!':
            return true;
        default:
            return false;
    }
}

bool MarkdownWriter::IsStartOfLineSpecial(sal_Unicode c)
{
    // These chars only need escaping at the start of a line
    return c == '#' || c == '>' || c == '+' || c == '-';
}

OString MarkdownWriter::EscapeMarkdown(const OUString& rText)
{
    OUStringBuffer aBuf;
    bool bAtLineStart = true;
    sal_Int32 nLineStart = 0;

    for (sal_Int32 i = 0; i < rText.getLength(); ++i)
    {
        sal_Unicode c = rText[i];

        if (c == '\n')
        {
            aBuf.append(c);
            bAtLineStart = true;
            nLineStart = i + 1;
            continue;
        }

        // Escape . after digits at start of line (prevents "1. " becoming a list)
        if (c == '.' && i > nLineStart)
        {
            bool bAllDigits = true;
            for (sal_Int32 j = nLineStart; j < i; ++j)
            {
                if (rText[j] < '0' || rText[j] > '9')
                {
                    bAllDigits = false;
                    break;
                }
            }
            if (bAllDigits)
            {
                aBuf.append(OUStringChar(u'\\') + OUStringChar(c));
                bAtLineStart = false;
                continue;
            }
        }

        if (IsStartOfLineSpecial(c))
        {
            if (bAtLineStart)
            {
                aBuf.append(OUStringChar(u'\\') + OUStringChar(c));
            }
            else
            {
                aBuf.append(c);
            }
        }
        else if (IsMarkdownSpecial(c))
        {
            aBuf.append(OUStringChar(u'\\') + OUStringChar(c));
        }
        else
        {
            aBuf.append(c);
        }

        bAtLineStart = false;
    }
    return OUStringToOString(aBuf, RTL_TEXTENCODING_UTF8);
}

bool MarkdownWriter::IsAllCodeParagraph(sal_Int32 nNode, const ParaPortion* pParaPortion,
                                        sal_Int32 nParaStartPos, sal_Int32 nParaEndPos) const
{
    if (nParaStartPos >= nParaEndPos)
        return false;

    bool bHasPortion = false;
    sal_Int32 nIndex = 0;
    sal_Int32 nPortionCount = pParaPortion->GetTextPortions().Count();

    for (sal_Int32 nPortion = 0; nPortion < nPortionCount; nPortion++)
    {
        const TextPortion& rTextPortion = pParaPortion->GetTextPortions()[nPortion];
        sal_Int32 nPortionStart = nIndex;
        sal_Int32 nPortionEnd = nIndex + rTextPortion.GetLen();
        nIndex = nPortionEnd;

        if (nPortionEnd <= nParaStartPos)
            continue;
        if (nPortionStart >= nParaEndPos)
            break;

        sal_Int32 nEffStart = std::max(nPortionStart, nParaStartPos);
        sal_Int32 nEffEnd = std::min(nPortionEnd, nParaEndPos);

        if (nEffStart >= nEffEnd)
            continue;

        bHasPortion = true;

        SfxItemSet aAttribs
            = mrEngine.GetAttribs(nNode, nEffStart, nEffEnd, GetAttribsFlags::CHARATTRIBS);
        const SvxFontItem& rFont = aAttribs.Get(EE_CHAR_FONTINFO);
        if (rFont.GetFamily() != FAMILY_MODERN && rFont.GetPitch() != PITCH_FIXED)
            return false;
    }

    return bHasPortion;
}

void MarkdownWriter::WriteContent(const std::function<void(std::string_view)>& rOut,
                                  sal_Int32 nStartNode, sal_Int32 nEndNode, sal_Int32 nStartPos,
                                  sal_Int32 nEndPos) const
{
    bool bPrevWasListItem = false;
    bool bPrevIsCode = false;

    for (sal_Int32 nNode = nStartNode; nNode <= nEndNode; nNode++)
    {
        const ContentNode* pNode = mrEngine.GetEditDoc().GetObject(nNode);
        const ParaPortion* pParaPortion = mrEngine.GetParaPortions().SafeGetObject(nNode);
        if (!pParaPortion)
            continue;

        bool bIsListItem = false;
        OString aListPrefix;

        // Check for list/bullet paragraph attributes
        const SfxInt16Item& rLevelItem = mrEngine.GetParaAttrib(nNode, EE_PARA_OUTLLEVEL);
        sal_Int16 nOutlLevel = rLevelItem.GetValue();
        if (nOutlLevel >= 0 && nOutlLevel <= 9)
        {
            bIsListItem = true;
            OStringBuffer aIndent;
            for (sal_Int16 i = 0; i < nOutlLevel; i++)
                aIndent.append("  ");

            const SvxNumBulletItem& rNumBullet = mrEngine.GetParaAttrib(nNode, EE_PARA_NUMBULLET);
            const SvxNumRule& rRule = rNumBullet.GetNumRule();
            const SvxNumberFormat* pFmt = rRule.Get(nOutlLevel);

            bool bOrdered = false;
            if (pFmt)
            {
                SvxNumType eType = pFmt->GetNumberingType();
                if (eType == SVX_NUM_ARABIC || eType == SVX_NUM_ROMAN_UPPER
                    || eType == SVX_NUM_ROMAN_LOWER || eType == SVX_NUM_CHARS_UPPER_LETTER
                    || eType == SVX_NUM_CHARS_LOWER_LETTER)
                {
                    bOrdered = true;
                }
            }

            // Markdown allows ordered-list markers like "1." for every item.
            // Most renderers auto-number by position, so constant "1." is valid
            // and avoids computing/exporting explicit sequence numbers.
            if (bOrdered)
                aListPrefix = aIndent.makeStringAndClear() + "1. ";
            else
                aListPrefix = aIndent.makeStringAndClear() + "- ";
        }

        // Determine selection range within this paragraph
        sal_Int32 nParaStartPos = 0;
        sal_Int32 nParaEndPos = pNode->Len();
        if (nNode == nStartNode)
            nParaStartPos = nStartPos;
        if (nNode == nEndNode)
            nParaEndPos = nEndPos;

        // Compute bIsCode inline (no pre-scan needed)
        bool bIsCode
            = !bIsListItem && IsAllCodeParagraph(nNode, pParaPortion, nParaStartPos, nParaEndPos);

        // Close previous code block if transitioning out
        if (bPrevIsCode && !bIsCode && nNode > nStartNode)
            rOut("\n```");

        // Paragraph separator
        if (nNode > nStartNode)
        {
            if (bPrevIsCode && bIsCode)
                rOut("\n");
            else if (bIsListItem || bPrevWasListItem)
                rOut("\n");
            else
                rOut("\n\n");
        }

        // Open code block if transitioning in
        if (bIsCode && !bPrevIsCode)
            rOut("```\n");

        if (bIsListItem)
            rOut(std::string_view(aListPrefix));

        bPrevWasListItem = bIsListItem;

        // Short-circuit code block paragraphs: emit raw text, no formatting
        if (bIsCode)
        {
            OUString aText = EditDoc::GetParaAsString(pNode, nParaStartPos, nParaEndPos);
            rOut(std::string_view(OUStringToOString(aText, RTL_TEXTENCODING_UTF8)));
            bPrevIsCode = true;
            continue;
        }
        bPrevIsCode = false;

        // Iterate text portions
        sal_Int32 nIndex = 0;
        sal_Int32 nPortionCount = pParaPortion->GetTextPortions().Count();

        for (sal_Int32 nPortion = 0; nPortion < nPortionCount; nPortion++)
        {
            const TextPortion& rTextPortion = pParaPortion->GetTextPortions()[nPortion];
            sal_Int32 nPortionStart = nIndex;
            sal_Int32 nPortionEnd = nIndex + rTextPortion.GetLen();
            nIndex = nPortionEnd;

            // Skip portions outside selection
            if (nPortionEnd <= nParaStartPos)
                continue;
            if (nPortionStart >= nParaEndPos)
                break;

            // Clamp to selection
            sal_Int32 nEffStart = std::max(nPortionStart, nParaStartPos);
            sal_Int32 nEffEnd = std::min(nPortionEnd, nParaEndPos);

            // Check for URL field
            const SvxURLField* pURLField = nullptr;
            if (rTextPortion.GetKind() == PortionKind::FIELD)
            {
                const EditCharAttrib* pAttr = pNode->GetCharAttribs().FindFeature(nPortionStart);
                if (pAttr)
                {
                    const SvxFieldItem* pFieldItem
                        = dynamic_cast<const SvxFieldItem*>(pAttr->GetItem());
                    if (pFieldItem)
                    {
                        const SvxFieldData* pFieldData = pFieldItem->GetField();
                        pURLField = dynamic_cast<const SvxURLField*>(pFieldData);
                    }
                }
            }

            // Get text for this portion
            OUString aText = EditDoc::GetParaAsString(pNode, nEffStart, nEffEnd);

            // Check character attributes
            bool bBold = false;
            bool bItalic = false;
            bool bStrikethrough = false;
            bool bCode = false;

            SfxItemSet aAttribs
                = mrEngine.GetAttribs(nNode, nEffStart, nEffEnd, GetAttribsFlags::CHARATTRIBS);

            const SvxWeightItem& rWeight = aAttribs.Get(EE_CHAR_WEIGHT);
            if (rWeight.GetWeight() == WEIGHT_BOLD)
                bBold = true;

            const SvxPostureItem& rPosture = aAttribs.Get(EE_CHAR_ITALIC);
            if (rPosture.GetPosture() == ITALIC_NORMAL || rPosture.GetPosture() == ITALIC_OBLIQUE)
                bItalic = true;

            const SvxCrossedOutItem& rStrikeout = aAttribs.Get(EE_CHAR_STRIKEOUT);
            if (rStrikeout.GetStrikeout() != STRIKEOUT_NONE)
                bStrikethrough = true;

            const SvxFontItem& rFont = aAttribs.Get(EE_CHAR_FONTINFO);
            if (rFont.GetFamily() == FAMILY_MODERN || rFont.GetPitch() == PITCH_FIXED)
                bCode = true;

            // Build markdown text
            OStringBuffer aPortionBuf;

            if (bCode)
            {
                aPortionBuf.append("`");
            }
            else
            {
                if (bStrikethrough)
                    aPortionBuf.append("~~");
                if (bBold)
                    aPortionBuf.append("**");
                if (bItalic)
                    aPortionBuf.append("*");
            }

            if (pURLField)
            {
                OString aRepr
                    = OUStringToOString(pURLField->GetRepresentation(), RTL_TEXTENCODING_UTF8);
                OString aUrl = OUStringToOString(pURLField->GetURL(), RTL_TEXTENCODING_UTF8);
                // Escape ] in link text and ) in URL
                aRepr = aRepr.replaceAll("]"_ostr, "\\]"_ostr);
                aUrl = aUrl.replaceAll(")"_ostr, "\\)"_ostr);
                aPortionBuf.append("[" + aRepr + "](" + aUrl + ")");
            }
            else if (bCode)
            {
                // Code spans: no escaping, content is literal
                aPortionBuf.append(OUStringToOString(aText, RTL_TEXTENCODING_UTF8));
            }
            else
            {
                aPortionBuf.append(EscapeMarkdown(aText));
            }

            if (bCode)
            {
                aPortionBuf.append("`");
            }
            else
            {
                if (bItalic)
                    aPortionBuf.append("*");
                if (bBold)
                    aPortionBuf.append("**");
                if (bStrikethrough)
                    aPortionBuf.append("~~");
            }

            rOut(std::string_view(aPortionBuf));
        }
    }

    // Close any open code block
    if (bPrevIsCode)
        rOut("\n```");
}

} // namespace editeng

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
