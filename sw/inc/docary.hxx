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
class SwTOXType;
class SwUndo;
class SwNumRule;
class SwRangeRedline;
class SwExtraRedline;
class SwUnoCrsr;
class SwOLENode;
class SwTable;
class SwTableLine;
class SwTableBox;

namespace com { namespace sun { namespace star { namespace i18n {
    struct ForbiddenCharacters;    ///< comes from the I18N UNO interface
}}}}

#include <swtypes.hxx>
#include <ndarr.hxx>
#include <charfmt.hxx>
#include <fmtcol.hxx>
#include <frmfmt.hxx>
#include <section.hxx>

/** provides some methods for generic operations on lists that contain
SwFmt* subclasses. */
class SwFmtsBase
{
public:
    virtual size_t GetFmtCount() const = 0;
    virtual const SwFmt* GetFmt(size_t idx) const = 0;
    virtual SwFmt* GetFmt(size_t idx) = 0;
    virtual ~SwFmtsBase() = 0;
};

class SwGrfFmtColls : public SwFmtsBase
{
private:
    std::vector<SwGrfFmtColl*> mvColls;

public:
    virtual size_t GetFmtCount() const SAL_OVERRIDE { return size(); }
    virtual const SwGrfFmtColl* GetFmt(size_t idx) const SAL_OVERRIDE { return operator[](idx); }
    virtual SwGrfFmtColl* GetFmt(size_t idx) SAL_OVERRIDE { return operator[](idx); }
    size_t size() const { return mvColls.size(); }
    SwGrfFmtColl *operator[](size_t idx) const { return mvColls[idx]; }
    void push_back(SwGrfFmtColl* pColl) { mvColls.push_back(pColl); }
    void DeleteAndDestroy(int nStartIdx, int nEndIdx);
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
    virtual size_t GetFmtCount() const SAL_OVERRIDE { return size(); }
    virtual const SwFrmFmt* GetFmt(size_t idx) const SAL_OVERRIDE { return operator[](idx); }
    virtual SwFrmFmt* GetFmt(size_t idx) SAL_OVERRIDE { return operator[](idx); }
    sal_uInt16 GetPos(const SwFrmFmt* pFmt) const;
    bool Contains(const SwFrmFmt* pFmt) const;
    void dumpAsXml(xmlTextWriterPtr w, const char* pName) const;
    /// free's any remaining child objects
    virtual ~SwFrmFmts();
};

class SwCharFmts : public std::vector<SwCharFmt*>, public SwFmtsBase
{
public:
    virtual size_t GetFmtCount() const SAL_OVERRIDE { return size(); }
    virtual const SwCharFmt* GetFmt(size_t idx) const SAL_OVERRIDE { return operator[](idx); }
    virtual SwCharFmt* GetFmt(size_t idx) SAL_OVERRIDE { return operator[](idx); }
    sal_uInt16 GetPos(const SwCharFmt* pFmt) const;
    bool Contains(const SwCharFmt* pFmt) const;
    void dumpAsXml(xmlTextWriterPtr w) const;
    /// free's any remaining child objects
    virtual ~SwCharFmts();
};

class SwTxtFmtColls : public std::vector<SwTxtFmtColl*>, public SwFmtsBase
{
public:
    virtual size_t GetFmtCount() const SAL_OVERRIDE { return size(); }
    virtual const SwTxtFmtColl* GetFmt(size_t idx) const SAL_OVERRIDE { return operator[](idx); }
    virtual SwTxtFmtColl* GetFmt(size_t idx) SAL_OVERRIDE { return operator[](idx); }
    sal_uInt16 GetPos(const SwTxtFmtColl* pFmt) const;
    void dumpAsXml(xmlTextWriterPtr w) const;
    virtual ~SwTxtFmtColls() {}
};

/// Array of Undo-history.
class SW_DLLPUBLIC SwSectionFmts : public std::vector<SwSectionFmt*>, public SwFmtsBase
{
public:
    virtual size_t GetFmtCount() const SAL_OVERRIDE { return size(); }
    virtual const SwSectionFmt* GetFmt(size_t idx) const SAL_OVERRIDE { return operator[](idx); }
    virtual SwSectionFmt* GetFmt(size_t idx) SAL_OVERRIDE { return operator[](idx); }
    sal_uInt16 GetPos(const SwSectionFmt* pFmt) const;
    bool Contains(const SwSectionFmt* pFmt) const;
    void dumpAsXml(xmlTextWriterPtr w) const;
    /// free's any remaining child objects
    virtual ~SwSectionFmts();
};

class SwFldTypes : public std::vector<SwFieldType*> {
public:
    /// the destructor will free all objects still in the vector
    ~SwFldTypes();
    sal_uInt16 GetPos(const SwFieldType* pFieldType) const;
    void dumpAsXml(xmlTextWriterPtr w) const;
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
    void dumpAsXml(xmlTextWriterPtr w) const;
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

    void dumpAsXml(xmlTextWriterPtr w) const;

    /** Search next or previous Redline with the same Seq. No.
       Search can be restricted via Lookahaed.
       Using 0 or USHRT_MAX makes search the whole array. */
    sal_uInt16 FindNextOfSeqNo( sal_uInt16 nSttPos, sal_uInt16 nLookahead = 20 ) const;
    sal_uInt16 FindPrevOfSeqNo( sal_uInt16 nSttPos, sal_uInt16 nLookahead = 20 ) const;
    sal_uInt16 FindNextSeqNo( sal_uInt16 nSeqNo, sal_uInt16 nSttPos,
                            sal_uInt16 nLookahead = 20 ) const;
    sal_uInt16 FindPrevSeqNo( sal_uInt16 nSeqNo, sal_uInt16 nSttPos,
                            sal_uInt16 nLookahead = 20 ) const;

    /**
     Find the redline at the given position.

     @param tableIndex position in SwRedlineTbl to start searching at, will be updated with the index of the returned
                       redline (or the next redline after the given position if not found)
     @param next true: redline starts at position and ends after, false: redline starts before position and ends at or after
    */
    const SwRangeRedline* FindAtPosition( const SwPosition& startPosition, sal_uInt16& tableIndex, bool next = true ) const;

    using _SwRedlineTbl::const_iterator;
    using _SwRedlineTbl::begin;
    using _SwRedlineTbl::end;
    using _SwRedlineTbl::size;
    using _SwRedlineTbl::operator[];
    using _SwRedlineTbl::empty;
    using _SwRedlineTbl::Resort;
};

/// Table that holds 'extra' redlines, such as 'table row insert\delete', 'paragraph moves' etc...
class SwExtraRedlineTbl
{
private:
    std::vector<SwExtraRedline*>    m_aExtraRedlines;

public:
    ~SwExtraRedlineTbl();

    bool Insert( SwExtraRedline* p );

    void DeleteAndDestroy( sal_uInt16 nPos, sal_uInt16 nLen = 1 );
    void DeleteAndDestroyAll();

    void dumpAsXml(xmlTextWriterPtr w) const;

    sal_uInt16 GetSize() const                              {     return m_aExtraRedlines.size();                }
    SwExtraRedline* GetRedline( sal_uInt16 uIndex ) const   {     return m_aExtraRedlines.operator[]( uIndex );  }
    bool IsEmpty() const                              {     return m_aExtraRedlines.empty();               }

    bool DeleteAllTableRedlines( SwDoc* pDoc, const SwTable& rTable, bool bSaveInUndo, sal_uInt16 nRedlineTypeToDelete );
    bool DeleteTableRowRedline ( SwDoc* pDoc, const SwTableLine& rTableLine, bool bSaveInUndo, sal_uInt16 nRedlineTypeToDelete );
    bool DeleteTableCellRedline( SwDoc* pDoc, const SwTableBox& rTableBox, bool bSaveInUndo, sal_uInt16 nRedlineTypeToDelete );
};

class SwUnoCrsrTbl : public std::set<SwUnoCrsr*> {
public:
    /// the destructor will free all objects still in the set
    ~SwUnoCrsrTbl();
};

typedef std::vector<SwOLENode*> SwOLENodes;

#endif // INCLUDED_SW_INC_DOCARY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
