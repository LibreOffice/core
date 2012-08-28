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
#ifndef _DOCARY_HXX
#define _DOCARY_HXX

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
class SwRedline;
class SwUnoCrsr;
class SwOLENode;
class SwTxtFmtColl;
class SwGrfFmtColl;

namespace com { namespace sun { namespace star { namespace i18n {
    struct ForbiddenCharacters;    ///< comes from the I18N UNO interface
}}}}

#include <swtypes.hxx>

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

/// Specific frame formats (frames, DrawObjects).
class SW_DLLPUBLIC SwFrmFmts : public std::vector<SwFrmFmt*>, public SwFmtsBase
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
    /// free's any remaining child objects
    virtual ~SwCharFmts();
};

class SwTxtFmtColls : public std::vector<SwTxtFmtColl*>, public SwFmtsBase
{
public:
    virtual size_t GetFmtCount() const { return size(); }
    virtual SwFmt* GetFmt(size_t idx) const { return (SwFmt*)operator[](idx); }
    sal_uInt16 GetPos(const SwTxtFmtColl* pFmt) const;
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
};

struct CompareSwRedlineTbl
{
    bool operator()(SwRedline* const &lhs, SwRedline* const &rhs) const;
};
class _SwRedlineTbl
    : public o3tl::sorted_vector<SwRedline*, CompareSwRedlineTbl,
                o3tl::find_partialorder_ptrequals>
{
public:
    ~_SwRedlineTbl();
};

class SwRedlineTbl : private _SwRedlineTbl
{
public:
    bool Contains(const SwRedline* p) const { return find(const_cast<SwRedline* const>(p)) != end(); }
    sal_uInt16 GetPos(const SwRedline* p) const;

    bool Insert( SwRedline* p, bool bIns = true );
    bool Insert( SwRedline* p, sal_uInt16& rInsPos, bool bIns = true );
    bool InsertWithValidRanges( SwRedline* p, sal_uInt16* pInsPos = 0 );

    void Remove( sal_uInt16 nPos );
    bool Remove( const SwRedline* p );
    void DeleteAndDestroy( sal_uInt16 nPos, sal_uInt16 nLen = 1 );
    void DeleteAndDestroyAll();

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

class SwUnoCrsrTbl : public std::set<SwUnoCrsr*> {
public:
    /// the destructor will free all objects still in the set
    ~SwUnoCrsrTbl();
};

class SwOLENodes : public std::vector<SwOLENode*> {};


#endif  //_DOCARY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
