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
#include <calbck.hxx>
#include <editeng/numitem.hxx>
#include <fmtcol.hxx>

class SwTextFormatColl;
class SwPageDesc;
class SwCharFormat;
class SwDoc;

class SW_DLLPUBLIC SwEndNoteInfo : public SwClient
{
    SwDepend    aPageDescDep;
    SwDepend    aCharFormatDep, aAnchorCharFormatDep;
    OUString sPrefix;
    OUString sSuffix;
protected:
    bool        m_bEndNote;
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew ) override;

public:
    SvxNumberType aFormat;
    sal_uInt16    nFootnoteOffset;

    void        ChgPageDesc( SwPageDesc *pDesc );
    SwPageDesc* GetPageDesc( SwDoc &rDoc ) const;
    bool        KnowsPageDesc() const;
    bool        DependsOn( const SwPageDesc* ) const;

    void SetFootnoteTextColl(SwTextFormatColl& rColl);
    SwTextFormatColl* GetFootnoteTextColl() const { return const_cast<SwTextFormatColl*>(static_cast<const SwTextFormatColl*>(GetRegisteredIn())); } // can be 0.

    SwCharFormat* GetCharFormat(SwDoc &rDoc) const;
    void SetCharFormat( SwCharFormat* );
    SwClient   *GetCharFormatDep() const { return const_cast<SwClient*>(static_cast<SwClient const *>(&aCharFormatDep)); }

    SwCharFormat* GetAnchorCharFormat(SwDoc &rDoc) const;
    void SetAnchorCharFormat( SwCharFormat* );
    SwClient   *GetAnchorCharFormatDep() const { return const_cast<SwClient*>(static_cast<SwClient const *>(&aAnchorCharFormatDep)); }

    SwEndNoteInfo & operator=(const SwEndNoteInfo&);
    bool operator==( const SwEndNoteInfo &rInf ) const;

    SwEndNoteInfo( SwTextFormatColl *pTextColl = nullptr);
    SwEndNoteInfo(const SwEndNoteInfo&);

    const OUString& GetPrefix() const  { return sPrefix; }
    const OUString& GetSuffix() const  { return sSuffix; }

    void SetPrefix(const OUString& rSet) { sPrefix = rSet; }
    void SetSuffix(const OUString& rSet) { sSuffix = rSet; }
    void ReleaseCollection() { if ( GetRegisteredInNonConst() ) GetRegisteredInNonConst()->Remove( this ); }
};

enum SwFootnotePos
{
    // Momentarily only PAGE and CHAPTER. CHAPTER == document-endnotes.
    FTNPOS_PAGE = 1,
    FTNPOS_CHAPTER = 8
};

enum SwFootnoteNum
{
    FTNNUM_PAGE, FTNNUM_CHAPTER, FTNNUM_DOC
};

class SW_DLLPUBLIC SwFootnoteInfo: public SwEndNoteInfo
{
    using SwEndNoteInfo::operator ==;

public:
    OUString  aQuoVadis;
    OUString  aErgoSum;
    SwFootnotePos  ePos;
    SwFootnoteNum  eNum;

    SwFootnoteInfo& operator=(const SwFootnoteInfo&);

    bool operator==( const SwFootnoteInfo &rInf ) const;

    SwFootnoteInfo(SwTextFormatColl* pTextColl = nullptr);
    SwFootnoteInfo(const SwFootnoteInfo&);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
