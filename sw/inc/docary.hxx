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
#ifndef INCLUDED_SW_INC_DOCARY_HXX
#define INCLUDED_SW_INC_DOCARY_HXX

#include <com/sun/star/i18n/ForbiddenCharacters.hpp>
#include <vector>
#include <set>
#include <algorithm>
#include <o3tl/sorted_vector.hxx>

class SwFieldType;
class SwFmt;
class SwFrmFmt;
class SwCharFmt;
class SwTOXType;
class SwUndo;
class SwSectionFmt;
class SwNumRule;
class SwRangeRedline;
class SwExtraRedline;
class SwUnoCrsr;
class SwOLENode;
class SwTxtFmtColl;
class SwGrfFmtColl;

namespace com { namespace sun { namespace star { namespace i18n {
    struct ForbiddenCharacters;    ///< comes from the I18N UNO interface
}}}}

#include <swtypes.hxx>
#include <ndarr.hxx>

/** provides some methods for generic operations on lists that contain
SwFmt* subclasses. */
class SwFmtsBase
{
public:
    virtual size_t GetFmtCount() const = 0;
    virtual SwFmt* GetFmt(size_t idx) const = 0;
    virtual ~SwFmtsBase() = 0;
};

class SwGrfFmtColls : public std::vector<SwGrfFmtColl*>, public SwFmtsBase
{
public:
    virtual size_t GetFmtCount() const { return size(); }
    virtual SwFmt* GetFmt(size_t idx) const { return (SwFmt*)operator[](idx); }
    sal_uInt16 GetPos(const SwGrfFmtColl* pFmt) const;
    /// free's any remaining child objects
    virtual ~SwGrfFmtColls() {}
};

/// stupid base class to work around MSVC dllexport mess
class SAL_DLLPUBLIC_TEMPLATE SwFrmFmts_Base : public std::vector<SwFrmFmt*> {};

/// Specific frame formats (frames, DrawObjects).
class SW_DLLPUBLIC SwFrmFmts : public SwFrmFmts_Base, public SwFmtsBase
{
public:
    virtual size_t GetFmtCount() const { return size(); }
    virtual SwFmt* GetFmt(size_t idx) const { return (SwFmt*)operator[](idx); }
    sal_uInt16 GetPos(const SwFrmFmt* pFmt) const;
    bool Contains(const SwFrmFmt* pFmt) const;
    /// free's any remaining child objects
    virtual ~SwFrmFmts();
};

class SwCharFmts : public std::vector<SwCharFmt*>, public SwFmtsBase
{
public:
    virtual size_t GetFmtCount() const { return size(); }
    virtual SwFmt* GetFmt(size_t idx) const { return (SwFmt*)operator[](idx); }
    sal_uInt16 GetPos(const SwCharFmt* pFmt) const;
    bool Contains(const SwCharFmt* pFmt) const;
    void dumpAsXml(xmlTextWriterPtr w);
    /// free's any remaining child objects
    virtual ~SwCharFmts();
};

class SwTxtFmtColls : public std::vector<SwTxtFmtColl*>, public SwFmtsBase
{
public:
    virtual size_t GetFmtCount() const { return size(); }
    virtual SwFmt* GetFmt(size_t idx) const { return (SwFmt*)operator[](idx); }
    sal_uInt16 GetPos(const SwTxtFmtColl* pFmt) const;
    void dumpAsXml(xmlTextWriterPtr w);
    virtual ~SwTxtFmtColls() {}
};

/// Array of Undo-history.
class SW_DLLPUBLIC SwSectionFmts : public std::vector<SwSectionFmt*>, public SwFmtsBase
{
public:
    virtual size_t GetFmtCount() const { return size(); }
    virtual SwFmt* GetFmt(size_t idx) const { return (SwFmt*)operator[](idx); }
    sal_uInt16 GetPos(const SwSectionFmt* pFmt) const;
    bool Contains(const SwSectionFmt* pFmt) const;
    /// free's any remaining child objects
    virtual ~SwSectionFmts();
};

class SwFldTypes : public std::vector<SwFieldType*> {
public:
    /// the destructor will free all objects still in the vector
    ~SwFldTypes();
    sal_uInt16 GetPos(const SwFieldType* pFieldType) const;
    void dumpAsXml(xmlTextWriterPtr w);
};

class SwTOXTypes : public std::vector<SwTOXType*> {
public:
    /// the destructor will free all objects still in the vector
    ~SwTOXTypes();
    sal_uInt16 GetPos(const SwTOXType* pTOXType) const;
};

class SW_DLLPUBLIC SwNumRuleTbl : public std::vector<SwNumRule*> {
public:
    /// the destructor will free all objects still in the vector
    ~SwNumRuleTbl();
    sal_uInt16 GetPos(const SwNumRule* pRule) const;
    void dumpAsXml(xmlTextWriterPtr w);
};

struct CompareSwRedlineTbl
{
    bool operator()(SwRangeRedline* const &lhs, SwRangeRedline* const &rhs) const;
};
class _SwRedlineTbl
    : public o3tl::sorted_vector<SwRangeRedline*, CompareSwRedlineTbl,
                o3tl::find_partialorder_ptrequals>
{
public:
    ~_SwRedlineTbl();
};

class SwRedlineTbl : private _SwRedlineTbl
{
public:
    bool Contains(const SwRangeRedline* p) const { return find(const_cast<SwRangeRedline* const>(p)) != end(); }
    sal_uInt16 GetPos(const SwRangeRedline* p) const;

    bool Insert( SwRangeRedline* p, bool bIns = true );
    bool Insert( SwRangeRedline* p, sal_uInt16& rInsPos, bool bIns = true );
    bool InsertWithValidRanges( SwRangeRedline* p, sal_uInt16* pInsPos = 0 );

    void Remove( sal_uInt16 nPos );
    bool Remove( const SwRangeRedline* p );
    void DeleteAndDestroy( sal_uInt16 nPos, sal_uInt16 nLen = 1 );
    void DeleteAndDestroyAll();

    void dumpAsXml(xmlTextWriterPtr w);

    /** Search next or previous Redline with the same Seq. No.
       Search can be restricted via Lookahaed.
       Using 0 or USHRT_MAX makes search the whole array. */
    sal_uInt16 FindNextOfSeqNo( sal_uInt16 nSttPos, sal_uInt16 nLookahead = 20 ) const;
    sal_uInt16 FindPrevOfSeqNo( sal_uInt16 nSttPos, sal_uInt16 nLookahead = 20 ) const;
    sal_uInt16 FindNextSeqNo( sal_uInt16 nSeqNo, sal_uInt16 nSttPos,
                            sal_uInt16 nLookahead = 20 ) const;
    sal_uInt16 FindPrevSeqNo( sal_uInt16 nSeqNo, sal_uInt16 nSttPos,
                            sal_uInt16 nLookahead = 20 ) const;

    using _SwRedlineTbl::size;
    using _SwRedlineTbl::operator[];
    using _SwRedlineTbl::empty;
};

/// Table that holds 'extra' redlines, such as 'table row insert\delete', 'paragraph moves' etc...
class SwExtraRedlineTbl
{
private:
    std::vector<SwExtraRedline*>    m_aExtraRedlines;

public:
    sal_uInt16 GetPos(const SwExtraRedline* p) const;

    bool Insert( SwExtraRedline* p );

    void Remove( sal_uInt16 nPos );
    void DeleteAndDestroy( sal_uInt16 nPos, sal_uInt16 nLen = 1 );
    void DeleteAndDestroyAll();

    void dumpAsXml(xmlTextWriterPtr w);

    sal_uInt16 GetSize() const                              {     return m_aExtraRedlines.size();                }
    SwExtraRedline* GetRedline( sal_uInt16 uIndex ) const   {     return m_aExtraRedlines.operator[]( uIndex );  }
    bool IsEmpty() const                              {     return m_aExtraRedlines.empty();               }
};

class SwUnoCrsrTbl : public std::set<SwUnoCrsr*> {
public:
    /// the destructor will free all objects still in the set
    ~SwUnoCrsrTbl();
};

class SwOLENodes : public std::vector<SwOLENode*> {};

#endif // INCLUDED_SW_INC_DOCARY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
