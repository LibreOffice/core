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
#ifndef INCLUDED_SW_INC_FTNINFO_HXX
#define INCLUDED_SW_INC_FTNINFO_HXX

#include <rtl/ustring.hxx>
#include "swdllapi.h"
#include "calbck.hxx"
#include <editeng/numitem.hxx>

class SwTextFormatColl;
class SwPageDesc;
class SwCharFormat;
class SwDoc;
class SwSection;

class SW_DLLPUBLIC SwEndNoteInfo : public SwClient
{
    mutable sw::WriterMultiListener m_aDepends;
    mutable SwTextFormatColl* m_pTextFormatColl;
    mutable SwPageDesc* m_pPageDesc;
    mutable std::unique_ptr<SwSection> m_pSwSection;
    mutable SwCharFormat* m_pCharFormat;
    mutable SwCharFormat* m_pAnchorFormat;
    OUString m_sPrefix;
    OUString m_sSuffix;
protected:
    bool        m_bEndNote;
    virtual void SwClientNotify( const SwModify&, const SfxHint&) override;

public:
    SvxNumberType m_aFormat;
    sal_uInt16    m_nFootnoteOffset;

    void ChgPageDesc(SwPageDesc* pDesc);
    SwPageDesc* GetPageDesc(SwDoc& rDoc) const;
    bool KnowsPageDesc() const;
    bool DependsOn(const SwPageDesc*) const;

    SwSection* GetSwSection(SwDoc& rDoc) const;
    void ResetSwSection();

    void SetFootnoteTextColl(SwTextFormatColl& rColl);
    SwTextFormatColl* GetFootnoteTextColl() const { return m_pTextFormatColl; } // can be 0.

    SwCharFormat* GetCharFormat(SwDoc &rDoc) const;
    void SetCharFormat( SwCharFormat* );

    SwCharFormat* GetAnchorCharFormat(SwDoc &rDoc) const;
    void SetAnchorCharFormat(SwCharFormat*);
    SwCharFormat* GetCurrentCharFormat(const bool bAnchor) const;

    SwEndNoteInfo & operator=(const SwEndNoteInfo&);
    bool operator==( const SwEndNoteInfo &rInf ) const;

    SwEndNoteInfo();
    SwEndNoteInfo(const SwEndNoteInfo&);

    const OUString& GetPrefix() const  { return m_sPrefix; }
    const OUString& GetSuffix() const  { return m_sSuffix; }

    void SetPrefix(const OUString& rSet) { m_sPrefix = rSet; }
    void SetSuffix(const OUString& rSet) { m_sSuffix = rSet; }
    void UpdateFormatOrAttr();
};

enum SwFootnotePos
{
    // Momentarily only PAGE and CHAPTER. CHAPTER == document-endnotes.
    FTNPOS_PAGE = 1,
    FTNPOS_CHAPTER = 8
};

enum SwFootnoteNum : unsigned
{
    FTNNUM_PAGE, FTNNUM_CHAPTER, FTNNUM_DOC
};

class SW_DLLPUBLIC SwFootnoteInfo final : public SwEndNoteInfo
{
    using SwEndNoteInfo::operator ==;

public:
    OUString  m_aQuoVadis;
    OUString  m_aErgoSum;
    SwFootnotePos  m_ePos;
    SwFootnoteNum  m_eNum;

    SwFootnoteInfo& operator=(const SwFootnoteInfo&);

    bool operator==( const SwFootnoteInfo &rInf ) const;

    SwFootnoteInfo();
    SwFootnoteInfo(const SwFootnoteInfo&);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
