/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <editdoc.hxx>
#include <EditSelection.hxx>
#include <rtl/string.hxx>
#include <vector>

#include <md4c.h>

class EditEngine;

namespace editeng
{
/** Parses Markdown text and inserts it into an EditEngine document.
 *
 * Uses the md4c library (GitHub Flavored Markdown dialect) to parse the
 * input and drive insertions into the edit engine at the given position.
 * Supports headings, bold/italic/strikethrough, inline code, fenced code
 * blocks, ordered/unordered lists, block quotes, hyperlinks, and HTML entities.
 */
class MarkdownParser
{
public:
    MarkdownParser(EditEngine* pEditEngine, const EditPaM& rPaM);

    EditPaM Parse(const OString& rMarkdown);

    const EditSelection& GetCurSelection() const { return maCurSel; }

private:
    static int EnterBlockCb(MD_BLOCKTYPE nType, void* pDetail, void* pUserData);
    static int LeaveBlockCb(MD_BLOCKTYPE nType, void* pDetail, void* pUserData);
    static int EnterSpanCb(MD_SPANTYPE nType, void* pDetail, void* pUserData);
    static int LeaveSpanCb(MD_SPANTYPE nType, void* pDetail, void* pUserData);
    static int TextCb(MD_TEXTTYPE nType, const MD_CHAR* pText, MD_SIZE nSize, void* pUserData);

    void EnterBlock(MD_BLOCKTYPE nType, void* pDetail);
    void LeaveBlock(MD_BLOCKTYPE nType, void* pDetail);
    void EnterSpan(MD_SPANTYPE nType, void* pDetail);
    void LeaveSpan(MD_SPANTYPE nType, void* pDetail);
    void HandleText(MD_TEXTTYPE nType, const MD_CHAR* pText, MD_SIZE nSize);

    void InsertText(const OUString& rText);
    void InsertParaBreak();

    EditEngine* mpEditEngine;
    EditSelection maCurSel;

    bool mbInParagraph;
    bool mbNeedParaBreak;
    sal_Int32 mnListDepth;
    bool mbBold;
    bool mbItalic;
    bool mbStrikethrough;
    bool mbCode;
    bool mbInCodeBlock;
    sal_Int16 mnHeadingLevel;
    sal_Int32 mnBlockQuoteDepth;

    struct ListInfo
    {
        bool bOrdered;
        sal_Int32 nStart;
    };
    std::vector<ListInfo> maListStack;

    OUString maLinkURL;
    sal_Int32 mnLinkStart;
};

} // namespace editeng

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
