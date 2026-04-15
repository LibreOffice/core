/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "markdownparser.hxx"

#include <editeng/editeng.hxx>
#include <editeng/flditem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/colritem.hxx>
#include <svl/intitem.hxx>

namespace editeng
{
MarkdownParser::MarkdownParser(EditEngine* pEditEngine, const EditPaM& rPaM)
    : mpEditEngine(pEditEngine)
    , maCurSel(rPaM)
    , mbInParagraph(false)
    , mbNeedParaBreak(false)
    , mnListDepth(-1)
    , mbBold(false)
    , mbItalic(false)
    , mbStrikethrough(false)
    , mbCode(false)
    , mbInCodeBlock(false)
    , mnHeadingLevel(0)
    , mnBlockQuoteDepth(0)
    , mnLinkStart(-1)
{
}

EditPaM MarkdownParser::Parse(const OString& rMarkdown)
{
    MD_PARSER parser
        = { 0,      MD_DIALECT_GITHUB, EnterBlockCb, LeaveBlockCb, EnterSpanCb, LeaveSpanCb,
            TextCb, nullptr,           nullptr };

    int nRet = md_parse(rMarkdown.getStr(), rMarkdown.getLength(), &parser, this);
    SAL_WARN_IF(nRet != 0, "editeng", "md_parse failed with error " << nRet);

    return maCurSel.Max();
}

int MarkdownParser::EnterBlockCb(MD_BLOCKTYPE nType, void* pDetail, void* pUserData)
{
    static_cast<MarkdownParser*>(pUserData)->EnterBlock(nType, pDetail);
    return 0;
}

int MarkdownParser::LeaveBlockCb(MD_BLOCKTYPE nType, void* pDetail, void* pUserData)
{
    static_cast<MarkdownParser*>(pUserData)->LeaveBlock(nType, pDetail);
    return 0;
}

int MarkdownParser::EnterSpanCb(MD_SPANTYPE nType, void* pDetail, void* pUserData)
{
    static_cast<MarkdownParser*>(pUserData)->EnterSpan(nType, pDetail);
    return 0;
}

int MarkdownParser::LeaveSpanCb(MD_SPANTYPE nType, void* pDetail, void* pUserData)
{
    static_cast<MarkdownParser*>(pUserData)->LeaveSpan(nType, pDetail);
    return 0;
}

int MarkdownParser::TextCb(MD_TEXTTYPE nType, const MD_CHAR* pText, MD_SIZE nSize, void* pUserData)
{
    static_cast<MarkdownParser*>(pUserData)->HandleText(nType, pText, nSize);
    return 0;
}

void MarkdownParser::EnterBlock(MD_BLOCKTYPE nType, void* pDetail)
{
    switch (nType)
    {
        case MD_BLOCK_DOC:
            break;

        case MD_BLOCK_P:
        {
            if (mbNeedParaBreak)
            {
                InsertParaBreak();
                mbNeedParaBreak = false;
            }
            mbInParagraph = true;
            break;
        }

        case MD_BLOCK_H:
        {
            if (mbNeedParaBreak)
            {
                InsertParaBreak();
                mbNeedParaBreak = false;
            }
            mbInParagraph = true;
            auto* pHDetail = static_cast<MD_BLOCK_H_DETAIL*>(pDetail);
            mnHeadingLevel = static_cast<sal_Int16>(pHDetail->level);
            break;
        }

        case MD_BLOCK_CODE:
        {
            if (mbNeedParaBreak)
            {
                InsertParaBreak();
                mbNeedParaBreak = false;
            }
            mbInParagraph = true;
            mbInCodeBlock = true;
            break;
        }

        case MD_BLOCK_QUOTE:
        {
            mnBlockQuoteDepth++;
            if (mbNeedParaBreak)
            {
                InsertParaBreak();
                mbNeedParaBreak = false;
            }
            mbInParagraph = true;
            break;
        }

        case MD_BLOCK_UL:
        {
            ListInfo aInfo;
            aInfo.bOrdered = false;
            aInfo.nStart = 1;
            maListStack.push_back(aInfo);
            mnListDepth++;
            break;
        }

        case MD_BLOCK_OL:
        {
            ListInfo aInfo;
            aInfo.bOrdered = true;
            auto* pOLDetail = static_cast<MD_BLOCK_OL_DETAIL*>(pDetail);
            aInfo.nStart = static_cast<sal_Int32>(pOLDetail->start);
            maListStack.push_back(aInfo);
            mnListDepth++;
            break;
        }

        case MD_BLOCK_LI:
        {
            // Insert paragraph break if previous block requested one, or if we're
            // already inside a paragraph (e.g. nested list item in a tight list where
            // md4c doesn't wrap text in P blocks).
            if (mbNeedParaBreak || mbInParagraph)
            {
                InsertParaBreak();
                mbNeedParaBreak = false;
            }
            mbInParagraph = true;

            // Set bullet/numbering for this paragraph
            if (mnListDepth >= 0 && !maListStack.empty())
            {
                sal_Int32 nPara = mpEditEngine->GetEditDoc().GetPos(maCurSel.Max().GetNode());

                // Set outline level
                SfxItemSet aItems(mpEditEngine->GetParaAttribs(nPara));
                aItems.Put(SfxInt16Item(EE_PARA_OUTLLEVEL, static_cast<sal_Int16>(mnListDepth)));

                // Set numbering rule
                SvxNumRule aRule(SvxNumRuleFlags::BULLET_REL_SIZE, 10, false);
                for (sal_uInt16 nLevel = 0;
                     nLevel < 10 && nLevel < static_cast<sal_uInt16>(maListStack.size()); nLevel++)
                {
                    SvxNumberFormat aFmt(SVX_NUM_CHAR_SPECIAL);
                    if (maListStack[nLevel].bOrdered)
                    {
                        aFmt.SetNumberingType(SVX_NUM_ARABIC);
                        aFmt.SetLabelFollowedBy(SvxNumberFormat::LabelFollowedBy::LISTTAB);
                        aFmt.SetStart(static_cast<sal_uInt16>(maListStack[nLevel].nStart));
                    }
                    else
                    {
                        aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
                        aFmt.SetBulletChar(0x2022); // bullet
                    }
                    aFmt.SetFirstLineIndent(0);
                    aFmt.SetAbsLSpace(720 * (nLevel + 1));
                    aRule.SetLevel(nLevel, aFmt);
                }
                aItems.Put(SvxNumBulletItem(std::move(aRule), EE_PARA_NUMBULLET));
                mpEditEngine->SetParaAttribsOnly(nPara, aItems);
            }
            break;
        }

        case MD_BLOCK_HR:
        {
            if (mbNeedParaBreak)
            {
                InsertParaBreak();
                mbNeedParaBreak = false;
            }
            // Insert a visual separator using a line of dashes
            InsertText(u"\u2015\u2015\u2015"_ustr); // horizontal bar characters
            mbNeedParaBreak = true;
            break;
        }

        default:
            break;
    }
}

void MarkdownParser::LeaveBlock(MD_BLOCKTYPE nType, void* /*pDetail*/)
{
    switch (nType)
    {
        case MD_BLOCK_DOC:
            break;

        case MD_BLOCK_P:
        case MD_BLOCK_LI:
        {
            mbInParagraph = false;
            mbNeedParaBreak = true;
            break;
        }

        case MD_BLOCK_H:
        {
            // Apply heading formatting to the current paragraph
            if (mnHeadingLevel > 0)
            {
                sal_Int32 nPara = mpEditEngine->GetEditDoc().GetPos(maCurSel.Max().GetNode());
                sal_Int32 nParaLen = mpEditEngine->GetTextLen(nPara);

                // Apply bold and larger font size based on heading level
                if (nParaLen > 0)
                {
                    SfxItemSet aSet(mpEditEngine->GetEmptyItemSet());
                    aSet.Put(SvxWeightItem(WEIGHT_BOLD, EE_CHAR_WEIGHT));

                    // Scale font size: H1=180%, H2=150%, H3=130%, H4=115%, H5/H6=100%
                    sal_uInt32 nPct = 100;
                    switch (mnHeadingLevel)
                    {
                        case 1:
                            nPct = 180;
                            break;
                        case 2:
                            nPct = 150;
                            break;
                        case 3:
                            nPct = 130;
                            break;
                        case 4:
                            nPct = 115;
                            break;
                        default:
                            break;
                    }
                    if (nPct > 100)
                    {
                        SvxFontHeightItem aHeight(
                            mpEditEngine->GetEmptyItemSet().Get(EE_CHAR_FONTHEIGHT).GetHeight()
                                * nPct / 100,
                            100, EE_CHAR_FONTHEIGHT);
                        aSet.Put(aHeight);
                    }

                    EditSelection aParaSel(EditPaM(maCurSel.Max().GetNode(), 0),
                                           EditPaM(maCurSel.Max().GetNode(), nParaLen));
                    mpEditEngine->SetAttribs(aParaSel, aSet);
                }
            }
            mnHeadingLevel = 0;
            mbInParagraph = false;
            mbNeedParaBreak = true;
            break;
        }

        case MD_BLOCK_CODE:
        {
            mbInCodeBlock = false;
            mbInParagraph = false;
            mbNeedParaBreak = true;
            break;
        }

        case MD_BLOCK_QUOTE:
        {
            if (mnBlockQuoteDepth > 0)
                mnBlockQuoteDepth--;
            mbInParagraph = false;
            mbNeedParaBreak = true;
            break;
        }

        case MD_BLOCK_UL:
        case MD_BLOCK_OL:
        {
            if (!maListStack.empty())
                maListStack.pop_back();
            if (mnListDepth >= 0)
                mnListDepth--;
            break;
        }

        default:
            break;
    }
}

void MarkdownParser::EnterSpan(MD_SPANTYPE nType, void* pDetail)
{
    switch (nType)
    {
        case MD_SPAN_STRONG:
            mbBold = true;
            break;

        case MD_SPAN_EM:
            mbItalic = true;
            break;

        case MD_SPAN_DEL:
            mbStrikethrough = true;
            break;

        case MD_SPAN_CODE:
            mbCode = true;
            break;

        case MD_SPAN_A:
        {
            auto* pADetail = static_cast<MD_SPAN_A_DETAIL*>(pDetail);
            if (pADetail->href.text && pADetail->href.size > 0)
            {
                maLinkURL
                    = OUString(pADetail->href.text, pADetail->href.size, RTL_TEXTENCODING_UTF8);
            }
            mnLinkStart = maCurSel.Max().GetIndex();
            break;
        }

        default:
            break;
    }
}

void MarkdownParser::LeaveSpan(MD_SPANTYPE nType, void* /*pDetail*/)
{
    switch (nType)
    {
        case MD_SPAN_STRONG:
            mbBold = false;
            break;

        case MD_SPAN_EM:
            mbItalic = false;
            break;

        case MD_SPAN_DEL:
            mbStrikethrough = false;
            break;

        case MD_SPAN_CODE:
            mbCode = false;
            break;

        case MD_SPAN_A:
        {
            if (!maLinkURL.isEmpty())
            {
                // Insert URL field covering the link text
                sal_Int32 nEnd = maCurSel.Max().GetIndex();
                OUString aRepresentation
                    = EditDoc::GetParaAsString(maCurSel.Max().GetNode(), mnLinkStart, nEnd);

                // Delete the plain text we inserted for the link
                EditPaM aStart(maCurSel.Max().GetNode(), mnLinkStart);
                EditPaM aEnd(maCurSel.Max());
                EditSelection aDelSel(aStart, aEnd);
                maCurSel = EditSelection(mpEditEngine->DeleteSelection(aDelSel));

                // Insert the URL field
                SvxURLField aURLField(maLinkURL, aRepresentation, SvxURLFormat::Repr);
                SvxFieldItem aFieldItem(aURLField, EE_FEATURE_FIELD);
                maCurSel = EditSelection(mpEditEngine->InsertField(maCurSel, aFieldItem));
            }
            maLinkURL.clear();
            mnLinkStart = -1;
            break;
        }

        default:
            break;
    }
}

void MarkdownParser::HandleText(MD_TEXTTYPE nType, const MD_CHAR* pText, MD_SIZE nSize)
{
    switch (nType)
    {
        case MD_TEXT_NORMAL:
        case MD_TEXT_CODE:
        {
            OUString aText(pText, nSize, RTL_TEXTENCODING_UTF8);
            InsertText(aText);
            break;
        }

        case MD_TEXT_BR:
        case MD_TEXT_SOFTBR:
        {
            if (nType == MD_TEXT_SOFTBR)
                InsertText(u" "_ustr);
            else
                InsertParaBreak();
            break;
        }

        case MD_TEXT_ENTITY:
        {
            OString aEntity(pText, nSize);
            OUString aText;
            if (aEntity == "&amp;")
                aText = u"&"_ustr;
            else if (aEntity == "&lt;")
                aText = u"<"_ustr;
            else if (aEntity == "&gt;")
                aText = u">"_ustr;
            else if (aEntity == "&quot;")
                aText = u"\""_ustr;
            else if (aEntity == "&apos;")
                aText = u"'"_ustr;
            else if (aEntity == "&nbsp;")
                aText = u"\u00A0"_ustr;
            else if (aEntity.startsWith("&#x") || aEntity.startsWith("&#X"))
            {
                // Hex numeric entity: &#xHHHH;
                OString aHex = aEntity.copy(3, aEntity.getLength() - 4); // strip &#x and ;
                sal_uInt32 nCodePoint = aHex.toUInt32(16);
                if (nCodePoint > 0 && nCodePoint <= 0x10FFFF)
                    aText = OUString(&nCodePoint, 1);
                else
                    aText = OUString(pText, nSize, RTL_TEXTENCODING_UTF8);
            }
            else if (aEntity.startsWith("&#"))
            {
                // Decimal numeric entity: &#NNN;
                OString aDec = aEntity.copy(2, aEntity.getLength() - 3); // strip &# and ;
                sal_uInt32 nCodePoint = aDec.toUInt32();
                if (nCodePoint > 0 && nCodePoint <= 0x10FFFF)
                    aText = OUString(&nCodePoint, 1);
                else
                    aText = OUString(pText, nSize, RTL_TEXTENCODING_UTF8);
            }
            else
                aText = OUString(pText, nSize, RTL_TEXTENCODING_UTF8);
            InsertText(aText);
            break;
        }

        default:
            break;
    }
}

void MarkdownParser::InsertText(const OUString& rText)
{
    EditPaM aStartPaM = maCurSel.Max();
    sal_Int32 nStartIndex = aStartPaM.GetIndex();

    maCurSel = EditSelection(mpEditEngine->InsertText(maCurSel, rText));

    // Always apply explicit character formatting to prevent attribute bleeding
    // from adjacent formatted text via ContentNode::ExpandAttribs
    bool bNeedMonospace = mbCode || mbInCodeBlock;
    EditPaM aAttrStart(aStartPaM.GetNode(), nStartIndex);
    EditPaM aAttrEnd(maCurSel.Max());
    EditSelection aAttrSel(aAttrStart, aAttrEnd);

    SfxItemSet aSet(mpEditEngine->GetEmptyItemSet());
    aSet.Put(SvxWeightItem(mbBold ? WEIGHT_BOLD : WEIGHT_NORMAL, EE_CHAR_WEIGHT));
    aSet.Put(SvxPostureItem(mbItalic ? ITALIC_NORMAL : ITALIC_NONE, EE_CHAR_ITALIC));
    aSet.Put(
        SvxCrossedOutItem(mbStrikethrough ? STRIKEOUT_SINGLE : STRIKEOUT_NONE, EE_CHAR_STRIKEOUT));
    aSet.Put(SvxColorItem(bNeedMonospace ? Color(225, 225, 225) : COL_AUTO, EE_CHAR_BKGCOLOR));
    if (bNeedMonospace)
    {
        SvxFontItem aFont(FAMILY_MODERN, u"Courier New"_ustr, u""_ustr, PITCH_FIXED,
                          RTL_TEXTENCODING_DONTKNOW, EE_CHAR_FONTINFO);
        aSet.Put(aFont);
    }

    mpEditEngine->SetAttribs(aAttrSel, aSet);

    // Apply blockquote indent if we're inside a blockquote
    if (mnBlockQuoteDepth > 0)
    {
        sal_Int32 nPara = mpEditEngine->GetEditDoc().GetPos(maCurSel.Max().GetNode());
        SfxItemSet aParaItems(mpEditEngine->GetParaAttribs(nPara));
        SvxLRSpaceItem aLR(EE_PARA_LRSPACE);
        aLR.SetTextLeft(SvxIndentValue::twips(720 * mnBlockQuoteDepth));
        aParaItems.Put(aLR);
        mpEditEngine->SetParaAttribsOnly(nPara, aParaItems);
    }
}

void MarkdownParser::InsertParaBreak()
{
    maCurSel = EditSelection(mpEditEngine->InsertParaBreak(maCurSel));
}

} // namespace editeng

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
