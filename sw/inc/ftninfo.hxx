/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
    rtl::OUString sPrefix;
    rtl::OUString sSuffix;
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

    const rtl::OUString& GetPrefix() const  { return sPrefix; }
    const rtl::OUString& GetSuffix() const  { return sSuffix; }

    void SetPrefix(const rtl::OUString& rSet) { sPrefix = rSet; }
    void SetSuffix(const rtl::OUString& rSet) { sSuffix = rSet; }
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
