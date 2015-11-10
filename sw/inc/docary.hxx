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
#include <fldbas.hxx>
#include <tox.hxx>
#include <numrule.hxx>

/** provides some methods for generic operations on lists that contain
SwFormat* subclasses. */
class SwFormatsBase
{
public:
    virtual size_t GetFormatCount() const = 0;
    virtual SwFormat* GetFormat(size_t idx) const = 0;
    virtual ~SwFormatsBase() {};
};

template<typename Value>
class SwVectorModifyBase : public std::vector<Value>
{
public:
    typedef typename std::vector<Value>::const_iterator const_iterator;

protected:
    enum class DestructorPolicy {
        KeepElements,
        FreeElements,
    };

private:
    const DestructorPolicy mPolicy;

protected:
    // default destructor deletes all contained elements
    SwVectorModifyBase(DestructorPolicy policy = DestructorPolicy::FreeElements)
        : mPolicy(policy) {}

public:
    using std::vector<Value>::begin;
    using std::vector<Value>::end;

    // free any remaining child objects based on mPolicy
    virtual ~SwVectorModifyBase()
    {
        if (mPolicy == DestructorPolicy::FreeElements)
            for(const_iterator it = begin(); it != end(); ++it)
                delete *it;
    }

    void DeleteAndDestroy(int aStartIdx, int aEndIdx)
    {
        if (aEndIdx < aStartIdx)
            return;
        for (const_iterator it = begin() + aStartIdx;
                            it != begin() + aEndIdx; ++it)
            delete *it;
        this->erase( begin() + aStartIdx, begin() + aEndIdx);
    }

    size_t GetPos(Value const& p) const
    {
        const_iterator const it = std::find(begin(), end(), p);
        return it == end() ? SIZE_MAX : it - begin();
    }

    bool Contains(Value const& p) const
        { return std::find(begin(), end(), p) != end(); }

    static void dumpAsXml(struct _xmlTextWriter* /*pWriter*/) {};
};

template<typename Value>
class SwFormatsModifyBase : public SwVectorModifyBase<Value>, public SwFormatsBase
{
protected:
    SwFormatsModifyBase(typename SwVectorModifyBase<Value>::DestructorPolicy
            policy = SwVectorModifyBase<Value>::DestructorPolicy::FreeElements)
        : SwVectorModifyBase<Value>(policy) {}

public:
    virtual size_t GetFormatCount() const override
        { return std::vector<Value>::size(); }

    virtual Value GetFormat(size_t idx) const override
        { return std::vector<Value>::operator[](idx); }

    inline size_t GetPos(const SwFormat *p) const
        { return SwVectorModifyBase<Value>::GetPos( static_cast<Value>( const_cast<SwFormat*>( p ) ) ); }
    inline bool Contains(const SwFormat *p) const {
        Value p2 = dynamic_cast<Value>(const_cast<SwFormat*>(p));
        return p2 != nullptr && SwVectorModifyBase<Value>::Contains(p2);
    }
};

class SwGrfFormatColls : public SwFormatsModifyBase<SwGrfFormatColl*>
{
public:
    SwGrfFormatColls() : SwFormatsModifyBase( DestructorPolicy::KeepElements ) {}
};

/// Specific frame formats (frames, DrawObjects).
class SW_DLLPUBLIC SwFrameFormats : public SwFormatsModifyBase<SwFrameFormat*>
{
public:
    void dumpAsXml(struct _xmlTextWriter* pWriter, const char* pName) const;
};

class SwCharFormats : public SwFormatsModifyBase<SwCharFormat*>
{
public:
    void dumpAsXml(struct _xmlTextWriter* pWriter) const;
};

class SwTextFormatColls : public SwFormatsModifyBase<SwTextFormatColl*>
{
public:
    SwTextFormatColls() : SwFormatsModifyBase( DestructorPolicy::KeepElements ) {}
    void dumpAsXml(struct _xmlTextWriter* pWriter) const;
};

/// Array of Undo-history.
class SW_DLLPUBLIC SwSectionFormats : public SwFormatsModifyBase<SwSectionFormat*>
{
public:
    void dumpAsXml(struct _xmlTextWriter* pWriter) const;
};

class SwFieldTypes : public SwVectorModifyBase<SwFieldType*> {
public:
    void dumpAsXml(struct _xmlTextWriter* pWriter) const;
};

class SwTOXTypes : public SwVectorModifyBase<SwTOXType*> {};

class SW_DLLPUBLIC SwNumRuleTable : public SwVectorModifyBase<SwNumRule*> {
public:
    void dumpAsXml(struct _xmlTextWriter* pWriter) const;
};

struct CompareSwRedlineTable
{
    bool operator()(SwRangeRedline* const &lhs, SwRangeRedline* const &rhs) const;
};

class SwRedlineTable
{
public:
    typedef o3tl::sorted_vector<SwRangeRedline*, CompareSwRedlineTable,
                o3tl::find_partialorder_ptrequals> vector_type;
    typedef vector_type::size_type size_type;
private:
    vector_type maVector;
public:
    ~SwRedlineTable();
    bool Contains(const SwRangeRedline* p) const { return maVector.find(const_cast<SwRangeRedline* const>(p)) != maVector.end(); }
    sal_uInt16 GetPos(const SwRangeRedline* p) const;

    bool Insert( SwRangeRedline* p, bool bIns = true );
    bool Insert( SwRangeRedline* p, sal_uInt16& rInsPos, bool bIns = true );
    bool InsertWithValidRanges( SwRangeRedline* p, sal_uInt16* pInsPos = nullptr );

    void Remove( sal_uInt16 nPos );
    bool Remove( const SwRangeRedline* p );
    void DeleteAndDestroy( sal_uInt16 nPos, sal_uInt16 nLen = 1 );
    void DeleteAndDestroyAll();

    void dumpAsXml(struct _xmlTextWriter* pWriter) const;

    sal_uInt16 FindNextOfSeqNo( sal_uInt16 nSttPos ) const;
    sal_uInt16 FindPrevOfSeqNo( sal_uInt16 nSttPos ) const;
    /** Search next or previous Redline with the same Seq. No.
       Search can be restricted via Lookahead.
       Using 0 makes search the whole array. */
    sal_uInt16 FindNextSeqNo( sal_uInt16 nSeqNo, sal_uInt16 nSttPos,
                            sal_uInt16 nLookahead = 20 ) const;
    sal_uInt16 FindPrevSeqNo( sal_uInt16 nSeqNo, sal_uInt16 nSttPos,
                            sal_uInt16 nLookahead = 20 ) const;

    /**
     Find the redline at the given position.

     @param tableIndex position in SwRedlineTable to start searching at, will be updated with the index of the returned
                       redline (or the next redline after the given position if not found)
     @param next true: redline starts at position and ends after, false: redline starts before position and ends at or after
    */
    const SwRangeRedline* FindAtPosition( const SwPosition& startPosition, sal_uInt16& tableIndex, bool next = true ) const;

    bool                        empty() const { return maVector.empty(); }
    size_type                   size() const { return maVector.size(); }
    SwRangeRedline*             operator[]( size_type idx ) const { return maVector[idx]; }
    vector_type::const_iterator begin() const { return maVector.begin(); }
    vector_type::const_iterator end() const { return maVector.end(); }
    void                        Resort() { maVector.Resort(); }
};

/// Table that holds 'extra' redlines, such as 'table row insert\delete', 'paragraph moves' etc...
class SwExtraRedlineTable
{
private:
    std::vector<SwExtraRedline*>    m_aExtraRedlines;

public:
    ~SwExtraRedlineTable();

    bool Insert( SwExtraRedline* p );

    void DeleteAndDestroy( sal_uInt16 nPos, sal_uInt16 nLen = 1 );
    void DeleteAndDestroyAll();

    void dumpAsXml(struct _xmlTextWriter* pWriter) const;

    sal_uInt16 GetSize() const                              {     return m_aExtraRedlines.size();                }
    SwExtraRedline* GetRedline( sal_uInt16 uIndex ) const   {     return m_aExtraRedlines.operator[]( uIndex );  }
    bool IsEmpty() const                              {     return m_aExtraRedlines.empty();               }

    SW_DLLPUBLIC bool DeleteAllTableRedlines( SwDoc* pDoc, const SwTable& rTable, bool bSaveInUndo, sal_uInt16 nRedlineTypeToDelete );
    bool DeleteTableRowRedline ( SwDoc* pDoc, const SwTableLine& rTableLine, bool bSaveInUndo, sal_uInt16 nRedlineTypeToDelete );
    bool DeleteTableCellRedline( SwDoc* pDoc, const SwTableBox& rTableBox, bool bSaveInUndo, sal_uInt16 nRedlineTypeToDelete );
};

typedef std::vector<SwOLENode*> SwOLENodes;

#endif // INCLUDED_SW_INC_DOCARY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
