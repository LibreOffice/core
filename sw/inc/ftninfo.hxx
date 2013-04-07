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
#ifndef _FTNINFO_HXX
#define _FTNINFO_HXX

#include <tools/string.hxx>
#include "swdllapi.h"
#include <calbck.hxx>
#include <editeng/numitem.hxx>

class SwTxtFmtColl;
class SwPageDesc;
class SwCharFmt;
class SwDoc;

class SW_DLLPUBLIC SwEndNoteInfo : public SwClient
{
    SwDepend    aPageDescDep;
    SwDepend    aCharFmtDep, aAnchorCharFmtDep;
    OUString sPrefix;
    OUString sSuffix;
protected:
    bool        m_bEndNote;
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew );

public:
    SvxNumberType aFmt;
    sal_uInt16    nFtnOffset;

    void        ChgPageDesc( SwPageDesc *pDesc );
    SwPageDesc* GetPageDesc( SwDoc &rDoc ) const;
    bool        KnowsPageDesc() const;
    bool        DependsOn( const SwPageDesc* ) const;

    void SetFtnTxtColl(SwTxtFmtColl& rColl);
    SwTxtFmtColl* GetFtnTxtColl() const { return  (SwTxtFmtColl*) GetRegisteredIn(); } // can be 0.

    SwCharFmt* GetCharFmt(SwDoc &rDoc) const;
    void SetCharFmt( SwCharFmt* );
    SwClient   *GetCharFmtDep() const { return (SwClient*)&aCharFmtDep; }

    SwCharFmt* GetAnchorCharFmt(SwDoc &rDoc) const;
    void SetAnchorCharFmt( SwCharFmt* );
    SwClient   *GetAnchorCharFmtDep() const { return (SwClient*)&aAnchorCharFmtDep; }

    SwEndNoteInfo & operator=(const SwEndNoteInfo&);
    bool operator==( const SwEndNoteInfo &rInf ) const;

    SwEndNoteInfo( SwTxtFmtColl *pTxtColl = 0);
    SwEndNoteInfo(const SwEndNoteInfo&);

    const OUString& GetPrefix() const  { return sPrefix; }
    const OUString& GetSuffix() const  { return sSuffix; }

    void SetPrefix(const OUString& rSet) { sPrefix = rSet; }
    void SetSuffix(const OUString& rSet) { sSuffix = rSet; }
    void ReleaseCollection() { if ( GetRegisteredInNonConst() ) GetRegisteredInNonConst()->Remove( this ); }
};

enum SwFtnPos
{
    // Momentarily only PAGE and CHAPTER. CHAPTER == document-endnotes.
    FTNPOS_PAGE = 1,
    FTNPOS_CHAPTER = 8
};

enum SwFtnNum
{
    FTNNUM_PAGE, FTNNUM_CHAPTER, FTNNUM_DOC
};

class SW_DLLPUBLIC SwFtnInfo: public SwEndNoteInfo
{
    using SwEndNoteInfo::operator ==;

public:
    String    aQuoVadis;
    String    aErgoSum;
    SwFtnPos  ePos;
    SwFtnNum  eNum;


    SwFtnInfo& operator=(const SwFtnInfo&);

    bool operator==( const SwFtnInfo &rInf ) const;

    SwFtnInfo(SwTxtFmtColl* pTxtColl = 0);
    SwFtnInfo(const SwFtnInfo&);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
