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
#ifndef _DOCARY_HXX
#define _DOCARY_HXX

#include <com/sun/star/i18n/ForbiddenCharacters.hpp>
#include <vector>
#include <set>
#include <algorithm>
#include <o3tl/sorted_vector.hxx>

#include "charfmt.hxx"
#include "fldbas.hxx"
#include "fmtcol.hxx"
#include "frmfmt.hxx"
#include "numrule.hxx"
#include "section.hxx"
#include "tox.hxx"
#include "unocrsr.hxx"
#include "redline.hxx"

class SwRangeRedline;
class SwExtraRedline;

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
    virtual SwFmt* GetFmt(size_t) const = 0;
    virtual ~SwFmtsBase() {}
};

template<typename Value>
class SwFmtsBaseModify : public std::vector<Value>, public SwFmtsBase
{
public:
    typedef typename std::vector<Value>::const_iterator const_iterator;

private:
    const bool mCleanup;

public:
    SwFmtsBaseModify(bool cleanup = true) : mCleanup(cleanup) {}

    using std::vector<Value>::begin;
    using std::vector<Value>::end;

    // free any remaining child objects based on mCleanup
    virtual ~SwFmtsBaseModify()
    {
        if (mCleanup)
            for(const_iterator it = begin(); it != end(); ++it)
                delete *it;
    }

    sal_uInt16 GetPos(Value const& p) const
    {
        const_iterator const it = std::find(begin(), end(), p);
        return it == end() ? USHRT_MAX : it - begin();
    }
    bool Contains(Value const& p) const
        { return std::find(begin(), end(), p) != end(); }
    virtual size_t GetFmtCount() const SAL_OVERRIDE
        { return std::vector<Value>::size(); }
    virtual SwFmt* GetFmt(size_t idx) const SAL_OVERRIDE
        { return (SwFmt*) std::vector<Value>::operator[](idx); }
    void dumpAsXml(xmlTextWriterPtr) {};
};

class SwGrfFmtColls : public SwFmtsBaseModify<SwGrfFmtColl*>
{
public:
    SwGrfFmtColls() : SwFmtsBaseModify( false ) {}
    virtual ~SwGrfFmtColls() {}
};

struct SwFrmFmtSearch
{
    sal_uInt16 type;
    const OUString& name;
    sal_Int32 length;

    SwFrmFmtSearch( sal_uInt16 _type,
                    const OUString& _name, sal_Int32 _length )
        :type( _type ), name( _name ), length( _length ) {}
};

struct CompareSwFrmFmts
{
    bool operator()(SwFrmFmt* const& lhs, SwFrmFmt* const& rhs) const;
    bool operator()(SwFrmFmt* const& lhs, SwFrmFmtSearch const& rhs) const;
    bool operator()(SwFrmFmtSearch const& lhs, SwFrmFmt* const& rhs) const;
};

struct PrefixCompareSwFrmFmts
{
#if (defined(DBG_UTIL) || defined(_DEBUG)) && (defined(WNT) || defined(_WIN32))
    bool operator()(SwFrmFmt* const& lhs, SwFrmFmt* const& rhs) const;
#endif
    bool operator()(SwFrmFmt* const& lhs, SwFrmFmtSearch const& rhs) const;
    bool operator()(SwFrmFmtSearch const& lhs, SwFrmFmt* const& rhs) const;
};

typedef o3tl::sorted_vector<SwFrmFmt*, CompareSwFrmFmts,
                            o3tl::find_partialorder_ptrequals> SwFrmFmtsBase;

/// Specific frame formats (frames, DrawObjects).
/// Mimics o3tl::sorted_vector interface
class SW_DLLPUBLIC SwFrmFmts : public SwFrmFmtsBase, public SwFmtsBase
{
public:
    typedef SwFrmFmtsBase::const_iterator const_iterator;
    typedef SwFrmFmtsBase::size_type size_type;
    typedef SwFrmFmtsBase::value_type value_type;
    typedef SwFrmFmtsBase::find_insert_type find_insert_type;

private:
    find_insert_type insert( const value_type& x, bool isNewRoot );

public:
    SwFrmFmts();
    virtual ~SwFrmFmts();

    find_insert_type insert( const value_type& x );
    size_type erase( const value_type& x );
    void erase( size_type index );
    void erase( const_iterator const& position );

    const_iterator find( const value_type& x ) const;
    std::pair<const_iterator,const_iterator>
        findRange( const value_type& x,
                   bool& root, sal_Int32 length=-1 ) const;
    std::pair<const_iterator,const_iterator>
        findRange( sal_uInt16 type, const OUString& name,
                   bool& root, sal_Int32 length=-1 ) const;

    bool Contains( const value_type& x ) const;

    virtual size_t GetFmtCount() const SAL_OVERRIDE
        { return SwFrmFmtsBase::size(); }
    virtual SwFmt* GetFmt(size_t idx) const SAL_OVERRIDE
        { return (SwFmt*) SwFrmFmtsBase::operator[](idx); }

    void dumpAsXml(xmlTextWriterPtr w, const char* pName);

    bool newDefault( const value_type& x );
};

/// Unsorted, undeleting SwFrmFmt vector
class SwFrmFmtsV : public SwFmtsBaseModify<SwFrmFmt*>
{
public:
    virtual ~SwFrmFmtsV() {}
};

class SwCharFmts : public SwFmtsBaseModify<SwCharFmt*>
{
public:
    virtual ~SwCharFmts() {}
    void dumpAsXml(xmlTextWriterPtr w);
};

class SwTxtFmtColls : public SwFmtsBaseModify<SwTxtFmtColl*>
{
public:
    SwTxtFmtColls() : SwFmtsBaseModify( false ) {}
    virtual ~SwTxtFmtColls() {}
    void dumpAsXml(xmlTextWriterPtr w);
};

/// Array of Undo-history.
class SW_DLLPUBLIC SwSectionFmts : public SwFmtsBaseModify<SwSectionFmt*>
{
public:
    virtual ~SwSectionFmts() {}
    void dumpAsXml(xmlTextWriterPtr w);
};

class SwFldTypes : public SwFmtsBaseModify<SwFieldType*>
{
public:
    virtual ~SwFldTypes() {}
    void dumpAsXml(xmlTextWriterPtr w);
};

class SwTOXTypes : public SwFmtsBaseModify<SwTOXType*>
{
public:
    virtual ~SwTOXTypes() {}
};

class SW_DLLPUBLIC SwNumRuleTbl : public SwFmtsBaseModify<SwNumRule*> {
public:
    virtual ~SwNumRuleTbl() {}
    void dumpAsXml(xmlTextWriterPtr w);
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
