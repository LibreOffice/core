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
#ifndef INCLUDED_SW_INC_NDHINTS_HXX
#define INCLUDED_SW_INC_NDHINTS_HXX

#include <tools/mempool.hxx>
#include <o3tl/sorted_vector.hxx>

#include "swtypes.hxx"

class SwTextNode;
class SwRegHistory;                 // Is in RolBck.hxx.
class SwTextAttr;
class SwTextAttrNesting;

class SfxPoolItem;
class SfxItemSet;
class SwDoc;

typedef enum {
    COPY = int(true),
    NEW  = int(false),
} CopyOrNew_t;

/// if COPY then pTextNode must be given!
SwTextAttr * MakeTextAttr(
    SwDoc & rDoc,
    SfxPoolItem & rNew,
    sal_Int32 const nStt,
    sal_Int32 const nEnd,
    CopyOrNew_t const bIsCopy = NEW,
    SwTextNode *const pTextNode = nullptr );

SwTextAttr * MakeTextAttr(
    SwDoc & rDoc,
    const SfxItemSet & rSet,
    sal_Int32 nStt,
    sal_Int32 nEnd );

/// create redline dummy text hint that must not be inserted into hints array
SwTextAttr* MakeRedlineTextAttr(
    SwDoc & rDoc,
    SfxPoolItem& rAttr );


/// SwTextAttr's, sorted by start
struct CompareSwpHtStart
{
    bool operator()(SwTextAttr* const lhs, SwTextAttr* const rhs) const;
};
class SwpHtStart : public o3tl::sorted_vector<SwTextAttr*, CompareSwpHtStart,
    o3tl::find_partialorder_ptrequals> {};

/// SwTextAttr's, sorted by end
struct CompareSwpHtEnd
{
    bool operator()(SwTextAttr* const lhs, SwTextAttr* const rhs) const;
};
class SwpHtEnd : public o3tl::sorted_vector<SwTextAttr*, CompareSwpHtEnd,
    o3tl::find_partialorder_ptrequals> {};



class SwpHints
{
private:
    // SAL_MAX_SIZE is used by GetStartOf to return
    // failure, so just allow SAL_MAX_SIZE-1 hints
    static const size_t MAX_HINTS = SAL_MAX_SIZE-1;

    SwpHtStart    m_HintsByStart;
    SwpHtEnd      m_HintsByEnd;

    SwRegHistory* m_pHistory;                   ///< for Undo

    bool          m_bFontChange          : 1;   ///< font change
    /// true: the Node is in Split and Frames are moved
    bool          m_bInSplitNode         : 1;
    /// m_bHasHiddenParaField is invalid, call CalcHiddenParaField()
    bool          m_bCalcHiddenParaField : 1;
    bool          m_bHasHiddenParaField  : 1;   ///< HiddenParaField
    bool          m_bFootnote            : 1;   ///< footnotes
    bool          m_bDDEFields           : 1;   ///< the TextNode has DDE fields

    /// records a new attibute in m_pHistory.
    void NoteInHistory( SwTextAttr *pAttr, const bool bNew = false );

    void CalcFlags( );

    /** Delete methods may only be called by the TextNode!
       Because the TextNode also guarantees removal of the Character for
       attributes without an end. */
    friend class SwTextNode;
    void DeleteAtPos( size_t nPos );
    /// Delete the given Hint. The Hint must actually be in the array!
    void Delete( SwTextAttr* pTextHt );

    void SetInSplitNode(bool bInSplit) { m_bInSplitNode = bInSplit; }
    void SetCalcHiddenParaField() { m_bCalcHiddenParaField = true; }
    void SetHiddenParaField( const bool bNew ) { m_bHasHiddenParaField = bNew; }
    bool HasHiddenParaField() const
    {
        if ( m_bCalcHiddenParaField )
        {
            (const_cast<SwpHints*>(this))->CalcHiddenParaField();
        }
        return m_bHasHiddenParaField;
    }

    void InsertNesting(SwTextAttrNesting & rNewHint);
    bool TryInsertNesting(SwTextNode & rNode, SwTextAttrNesting & rNewHint);
    void BuildPortions( SwTextNode& rNode, SwTextAttr& rNewHint,
            const SetAttrMode nMode );
    bool MergePortions( SwTextNode& rNode );

    void Insert( const SwTextAttr *pHt );
    void Resort();

    size_t GetIndexOf( const SwTextAttr *pHt ) const
    {
        SwpHtStart::const_iterator const it =
            m_HintsByStart.find(const_cast<SwTextAttr*>(pHt));
        if ( it == m_HintsByStart.end() )
        {
            return SAL_MAX_SIZE;
        }
        return it - m_HintsByStart.begin();
    }

#ifdef DBG_UTIL
    bool Check(bool) const;
#endif

public:
    SwpHints();

    size_t Count() const { return m_HintsByStart.size(); }
    bool Contains( const SwTextAttr *pHt ) const;
    SwTextAttr * Get( size_t nPos ) const
    {
        return m_HintsByStart[nPos];
    }
    SwTextAttr * GetSortedByEnd( size_t nPos ) const
    {
        return m_HintsByEnd[nPos];
    }
    SwTextAttr * Cut( const size_t nPosInStart )
    {
        SwTextAttr *pHt = m_HintsByStart[nPosInStart];
        DeleteAtPos( nPosInStart );
        return pHt;
    }

    bool CanBeDeleted() const    { return m_HintsByStart.empty(); }

    /// register a History, which receives all attribute changes (for Undo)
    void Register( SwRegHistory* pHist ) { m_pHistory = pHist; }
    /// deregister the currently registered History
    void DeRegister() { Register(nullptr); }
    SwRegHistory* GetHistory() const    { return m_pHistory; }

    /// try to insert the hint
    /// @return true iff hint successfully inserted
    bool TryInsertHint( SwTextAttr * const pHint, SwTextNode & rNode,
            const SetAttrMode nMode = SetAttrMode::DEFAULT );

    bool HasFootnote() const          { return m_bFootnote; }
    bool IsInSplitNode() const   { return m_bInSplitNode; }

    /// calc current value of m_bHasHiddenParaField, returns true iff changed
    bool CalcHiddenParaField();

    DECL_FIXEDMEMPOOL_NEWDEL(SwpHints)
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
