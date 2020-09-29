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

#include <vector>
#include <type_traits>
#include <o3tl/sorted_vector.hxx>

#include "fmtcol.hxx"
#include "frmfmt.hxx"
#include "section.hxx"
#include "tox.hxx"
#include "numrule.hxx"
#include "fldbas.hxx"

class SwRangeRedline;
class SwExtraRedline;
class SwOLENode;
class SwTable;
class SwTableLine;
class SwTableBox;
struct SwPosition;
enum class RedlineType : sal_uInt16;

/** provides some methods for generic operations on lists that contain SwFormat* subclasses. */
class SwFormatsBase
{
public:
    virtual size_t GetFormatCount() const = 0;
    virtual SwFormat* GetFormat(size_t idx) const = 0;
    virtual ~SwFormatsBase() {};

    SwFormatsBase() = default;
    SwFormatsBase(SwFormatsBase const &) = default;
    SwFormatsBase(SwFormatsBase &&) = default;
    SwFormatsBase & operator =(SwFormatsBase const &) = default;
    SwFormatsBase & operator =(SwFormatsBase &&) = default;
};

template<typename Value>
class SwVectorModifyBase
{
public:
    typedef typename std::vector<Value>::iterator iterator;
    typedef typename std::vector<Value>::const_iterator const_iterator;
    typedef typename std::vector<Value>::size_type size_type;
    typedef typename std::vector<Value>::value_type value_type;

protected:
    enum class DestructorPolicy {
        KeepElements,
        FreeElements,
    };

private:
    typename std::vector<Value> mvVals;
    const DestructorPolicy mPolicy;

protected:
    // default destructor deletes all contained elements
    SwVectorModifyBase(DestructorPolicy policy = DestructorPolicy::FreeElements)
        : mPolicy(policy) {}

public:
    bool empty() const { return mvVals.empty(); }
    Value const& front() const { return mvVals.front(); }
    size_t size() const { return mvVals.size(); }
    iterator begin() { return mvVals.begin(); }
    const_iterator begin() const { return mvVals.begin(); }
    iterator end() { return mvVals.end(); }
    const_iterator end() const { return mvVals.end(); }
    void clear() { mvVals.clear(); }
    iterator erase(iterator aIt) { return mvVals.erase(aIt); }
    iterator erase(iterator aFirst, iterator aLast) { return mvVals.erase(aFirst, aLast); }
    iterator insert(iterator aIt, Value const& rVal) { return mvVals.insert(aIt, rVal); }
    template<typename TInputIterator>
    void insert(iterator aIt, TInputIterator aFirst, TInputIterator aLast)
    {
        mvVals.insert(aIt, aFirst, aLast);
    }
    void push_back(Value const& rVal) { mvVals.push_back(rVal); }
    void reserve(size_type nSize) { mvVals.reserve(nSize); }
    Value const& at(size_type nPos) const { return mvVals.at(nPos); }
    Value const& operator[](size_type nPos) const { return mvVals[nPos]; }
    Value& operator[](size_type nPos) { return mvVals[nPos]; }

    // free any remaining child objects based on mPolicy
    virtual ~SwVectorModifyBase()
    {
        if (mPolicy == DestructorPolicy::FreeElements)
            for(const_iterator it = begin(); it != end(); ++it)
                delete *it;
    }

    //TODO: These functions are apparently brittle (but the copy functions are actually used by the
    // code; the move functions will be implicitly-defined as deleted anyway) and should probably
    // only be used with DestructorPolicy::KeepELements:
    SwVectorModifyBase(SwVectorModifyBase const &) = default;
    SwVectorModifyBase(SwVectorModifyBase &&) = default;
    SwVectorModifyBase & operator =(SwVectorModifyBase const &) = default;
    SwVectorModifyBase & operator =(SwVectorModifyBase &&) = default;

    void DeleteAndDestroy(int aStartIdx, int aEndIdx)
    {
        if (aEndIdx < aStartIdx)
            return;
        for (const_iterator it = begin() + aStartIdx;
                            it != begin() + aEndIdx; ++it)
            delete *it;
        erase( begin() + aStartIdx, begin() + aEndIdx);
    }

    size_t GetPos(Value const& p) const
    {
        const_iterator const it = std::find(begin(), end(), p);
        return it == end() ? SIZE_MAX : it - begin();
    }

    /// check that given format is still alive (i.e. contained here)
    bool IsAlive(typename std::remove_pointer<Value>::type const*const p) const
        { return std::find(begin(), end(), p) != end(); }

    static void dumpAsXml(xmlTextWriterPtr /*pWriter*/) {};
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
        { return SwVectorModifyBase<Value>::size(); }

    virtual Value GetFormat(size_t idx) const override
        { return SwVectorModifyBase<Value>::operator[](idx); }

    size_t GetPos(const SwFormat *p) const
        { return SwVectorModifyBase<Value>::GetPos( static_cast<Value>( const_cast<SwFormat*>( p ) ) ); }

    /// check if given format is contained here
    /// @precond pFormat must not have been deleted
    bool ContainsFormat(SwFormat const*const pFormat) const {
        Value p = dynamic_cast<Value>(const_cast<SwFormat*>(pFormat));
        return p != nullptr && SwVectorModifyBase<Value>::IsAlive(p);
    }
};

class SwGrfFormatColls final : public SwFormatsModifyBase<SwGrfFormatColl*>
{
public:
    SwGrfFormatColls() : SwFormatsModifyBase( DestructorPolicy::KeepElements ) {}
};


/// Unsorted, undeleting SwFrameFormat vector
class SwFrameFormatsV final : public SwFormatsModifyBase<SwFrameFormat*>
{
public:
    SwFrameFormatsV() : SwFormatsModifyBase( DestructorPolicy::KeepElements ) {}
};

class SwCharFormats final : public SwFormatsModifyBase<SwCharFormat*>
{
public:
    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

class SwTextFormatColls final : public SwFormatsModifyBase<SwTextFormatColl*>
{
public:
    SwTextFormatColls() : SwFormatsModifyBase( DestructorPolicy::KeepElements ) {}
    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

/// Array of Undo-history.
class SwSectionFormats final : public SwFormatsModifyBase<SwSectionFormat*>
{
public:
    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

class SwFieldTypes : public std::vector<std::unique_ptr<SwFieldType>> {
public:
    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

class SwTOXTypes : public std::vector<std::unique_ptr<SwTOXType>> {};

class SwNumRuleTable final : public SwVectorModifyBase<SwNumRule*> {
public:
    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

struct CompareSwRedlineTable
{
    bool operator()(SwRangeRedline* const &lhs, SwRangeRedline* const &rhs) const;
};

// Notification type for notifying about redlines to LOK clients
enum class RedlineNotification { Add, Remove, Modify };

class SwRedlineTable
{
public:
    typedef o3tl::sorted_vector<SwRangeRedline*, CompareSwRedlineTable,
                o3tl::find_partialorder_ptrequals> vector_type;
    typedef vector_type::size_type size_type;
    static constexpr size_type npos = SAL_MAX_INT32;
private:
    vector_type maVector;
public:
    ~SwRedlineTable();
    bool Contains(const SwRangeRedline* p) const { return maVector.find(const_cast<SwRangeRedline*>(p)) != maVector.end(); }
    size_type GetPos(const SwRangeRedline* p) const;

    bool Insert(SwRangeRedline*& p);
    bool Insert(SwRangeRedline*& p, size_type& rInsPos);
    bool InsertWithValidRanges(SwRangeRedline*& p, size_type* pInsPos = nullptr);

    void Remove( size_type nPos );
    void Remove( const SwRangeRedline* p );
    void DeleteAndDestroy(size_type nPos);
    void DeleteAndDestroyAll();

    void dumpAsXml(xmlTextWriterPtr pWriter) const;

    size_type FindNextOfSeqNo( size_type nSttPos ) const;
    size_type FindPrevOfSeqNo( size_type nSttPos ) const;
    /** Search next or previous Redline with the same Seq. No.
       Search can be restricted via Lookahead.
       Using 0 makes search the whole array. */
    size_type FindNextSeqNo( sal_uInt16 nSeqNo, size_type nSttPos ) const;
    size_type FindPrevSeqNo( sal_uInt16 nSeqNo, size_type nSttPos ) const;

    /**
     Find the redline at the given position.

     @param tableIndex position in SwRedlineTable to start searching at, will be updated with the index of the returned
                       redline (or the next redline after the given position if not found)
     @param next true: redline starts at position and ends after, false: redline starts before position and ends at or after
    */
    const SwRangeRedline* FindAtPosition( const SwPosition& startPosition, size_type& tableIndex, bool next = true ) const;

    bool                        empty() const { return maVector.empty(); }
    size_type                   size() const { return maVector.size(); }
    SwRangeRedline*             operator[]( size_type idx ) const { return maVector[idx]; }
    vector_type::const_iterator begin() const { return maVector.begin(); }
    vector_type::const_iterator end() const { return maVector.end(); }
    void                        Resort() { maVector.Resort(); }

    // Notifies all LOK clients when redlines are added/modified/removed
    static void                 LOKRedlineNotification(RedlineNotification eType, SwRangeRedline* pRedline);
};

/// Table that holds 'extra' redlines, such as 'table row insert/delete', 'paragraph moves' etc...
class SwExtraRedlineTable
{
private:
    std::vector<SwExtraRedline*>    m_aExtraRedlines;

public:
    ~SwExtraRedlineTable();

    void Insert( SwExtraRedline* p );

    void DeleteAndDestroy( sal_uInt16 nPos);
    void DeleteAndDestroyAll();

    void dumpAsXml(xmlTextWriterPtr pWriter) const;

    sal_uInt16 GetSize() const                              {     return m_aExtraRedlines.size();                }
    SwExtraRedline* GetRedline( sal_uInt16 uIndex ) const   {     return m_aExtraRedlines.operator[]( uIndex );  }

    SW_DLLPUBLIC bool DeleteAllTableRedlines( SwDoc& rDoc, const SwTable& rTable, bool bSaveInUndo, RedlineType nRedlineTypeToDelete );
    bool DeleteTableRowRedline ( SwDoc* pDoc, const SwTableLine& rTableLine, bool bSaveInUndo, RedlineType nRedlineTypeToDelete );
    bool DeleteTableCellRedline( SwDoc* pDoc, const SwTableBox& rTableBox, bool bSaveInUndo, RedlineType nRedlineTypeToDelete );
};

typedef std::vector<SwOLENode*> SwOLENodes;

#endif // INCLUDED_SW_INC_DOCARY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
