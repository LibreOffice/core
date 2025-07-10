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

#pragma once

#include <memory>

#include <tools/stream.hxx>
#include <doc.hxx>
#include <pam.hxx>

#include <md4c.h>
#include <o3tl/unit_conversion.hxx>
#include <frozen/unordered_map.h>

#include "mdnum.hxx"

class MDTable;

constexpr tools::Long MD_PARSPACE = o3tl::toTwips(5, o3tl::Length::mm);
constexpr tools::Long MD_MAX_IMAGE_WIDTH_IN_TWIPS = 2500;
constexpr tools::Long MD_MAX_IMAGE_HEIGHT_IN_TWIPS = 2500;

constexpr frozen::unordered_map<MD_ALIGN, SvxAdjust, 4> adjustMap
    = { { MD_ALIGN_DEFAULT, SvxAdjust::Left },
        { MD_ALIGN_LEFT, SvxAdjust::Left },
        { MD_ALIGN_CENTER, SvxAdjust::Center },
        { MD_ALIGN_RIGHT, SvxAdjust::Right } };

constexpr Color COL_CODE_BLOCK = { 225, 225, 225 };

enum SwMdAppendMode
{
    AM_NORMAL, // no paragraph spacing handling
    AM_NOSPACE, // set spacing hard to 0cm
    AM_SPACE, // set spacing hard to 0.5cm
    AM_SOFTNOSPACE, // don't set spacing, but save 0cm
    AM_NONE // no append
};

typedef std::vector<std::unique_ptr<SfxPoolItem>> MDAttrStack;

class SwMarkdownParser
{
    rtl::Reference<SwDoc> m_xDoc;
    SwPaM* m_pPam;
    SvStream& m_rInput;
    // SfxMedium* m_pMedium;
    std::unique_ptr<char[]> m_pArr;
    std::unique_ptr<SwMdNumRuleInfo> m_pNumRuleInfo;
    tools::Long m_nFilesize;

    MDAttrStack m_aAttrStack;

    OUString m_htmlData;
    OUString m_sBaseURL;

    sal_Int32 m_nBlockQuoteDepth = -1;

    bool m_bNewDoc;
    bool m_bNoParSpace = true;
    bool m_bInsideImage = false;

    std::vector<MDTable*> m_aTables;
    std::shared_ptr<MDTable> m_xTable;

    SwMarkdownParser(const SwMarkdownParser&) = delete;
    SwMarkdownParser& operator=(const SwMarkdownParser&) = delete;

    void SetNodeNum(sal_uInt8 nLevel);

    sal_Int32 StripTrailingLF();

    bool AppendTextNode(SwMdAppendMode eMode, bool bUpdateNum = true);
    void AddParSpace();

    void AddBlockQuote();
    void EndBlockQuote();

    void AddHR();
    void EndHR();

    void StartPara();
    void EndPara();
    void StartHeading(sal_uInt8 nLvl);
    void EndHeading();

    void StartNumberedBulletList(MD_BLOCKTYPE aListType);
    void EndNumberedBulletList();
    void StartNumberedBulletListItem();
    void EndNumberedBulletListItem();

    void BeginHtmlBlock();
    void InsertHtmlData();
    void EndHtmlBlock();

    void BeginCodeBlock();
    void EndCodeBlock();

    void InsertText(OUString& aStr);
    void SetAttrs(SwPaM& rRange);
    void ClearAttrs();

    void InsertImage(const OUString& aURL, const OUString& rTitle);

    void StartTable(sal_Int32 nRow, sal_Int32 nCol);
    void EndTable();

    void StartRow();

    void StartCell(MD_ALIGN eAdjust);

public:
    void RegisterTable(MDTable* pTable);
    void DeRegisterTable(MDTable* pTable);

public:
    SwMarkdownParser(SwDoc& rD, SwPaM& rCursor, SvStream& rIn, OUString aBaseURL, bool bReadNewDoc);

    SwMdNumRuleInfo& GetNumInfo() const { return *m_pNumRuleInfo; }
    bool IsNewDoc() const { return m_bNewDoc; }
    ErrCode CallParser();

    ~SwMarkdownParser();

private:
    static int enter_block_callback(MD_BLOCKTYPE type, void* detail, void* userdata);
    static int leave_block_callback(MD_BLOCKTYPE type, void* detail, void* userdata);
    static int enter_span_callback(MD_SPANTYPE type, void* detail, void* userdata);
    static int leave_span_callback(MD_SPANTYPE type, void* detail, void* userdata);
    static int text_callback(MD_TEXTTYPE type, const MD_CHAR* text, MD_SIZE size, void* userdata);

    static void SanitizeAnchor(SfxItemSet& rFrameItemSet);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
