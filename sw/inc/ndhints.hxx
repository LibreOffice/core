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

#include "swtypes.hxx"

class SwTextNode;
class SwRegHistory;                 // Is in RolBck.hxx.
class SwTextAttr;
class SwTextAttrNesting;

class SfxPoolItem;
class SfxItemSet;
class SwDoc;

enum class CopyOrNewType { Copy, New };

/// if COPY then pTextNode must be given!
SwTextAttr * MakeTextAttr(
    SwDoc & rDoc,
    SfxPoolItem & rNew,
    sal_Int32 const nStt,
    sal_Int32 const nEnd,
    CopyOrNewType const bIsCopy = CopyOrNewType::New,
    SwTextNode *const pTextNode = nullptr );

SwTextAttr * MakeTextAttr(
    SwDoc & rDoc,
    const SfxItemSet & rSet,
    sal_Int32 nStt,
    sal_Int32 nEnd );

/// create redline dummy text hint that must not be inserted into hints array
SwTextAttr* MakeRedlineTextAttr(
    SwDoc & rDoc,
    SfxPoolItem const & rAttr );

struct CompareSwpHtEnd
{
    bool operator()( sal_Int32 nEndPos, const SwTextAttr* rhs ) const;
    bool operator()( const SwTextAttr* lhs, const SwTextAttr* rhs ) const;
};
typedef std::pair<sal_Int32, sal_Int32> WhichStartPair;
struct CompareSwpHtWhichStart
{
    bool operator()( const SwTextAttr* lhs, const sal_uInt16 nWhich ) const;
    bool operator()( const SwTextAttr* lhs, const SwTextAttr* rhs ) const;
    // used when we sort only a partial range of the map
    bool operator()( const SwTextAttr* lhs, const WhichStartPair nWhich ) const;
    bool operator()( const WhichStartPair nWhich, const SwTextAttr* lhs ) const;
};

/// An SwTextAttr container, stores all directly formatted text portions for a text node.
class SwpHints
{
private:
    const SwTextNode& m_rParent;

    // SAL_MAX_SIZE is used by GetStartOf to return
    // failure, so just allow SAL_MAX_SIZE-1 hints
    static const size_t MAX_HINTS = SAL_MAX_SIZE-1;

    std::vector<SwTextAttr*> m_HintsByStart;
    std::vector<SwTextAttr*> m_HintsByEnd;
    std::vector<SwTextAttr*> m_HintsByWhichAndStart;

    SwRegHistory* m_pHistory;                   ///< for Undo

    /// true: the Node is in Split and Frames are moved
    bool          m_bInSplitNode         : 1;
    // m_bHiddenByParaField is invalid, call CalcHiddenParaField()
    mutable bool  m_bCalcHiddenParaField : 1;
    // if all fields controlling visibility of the paragraph require to hide it
    // (if there's no such fields, or if any field requires to show, then this is false)
    mutable bool  m_bHiddenByParaField   : 1;
    bool          m_bFootnote            : 1;   ///< footnotes
    bool          m_bDDEFields           : 1;   ///< the TextNode has DDE fields
    // Sort on demand to avoid O(n^2) behaviour
    // SAL_MAX_INT32 means nothing needs sorting, -1 means everything needs sorting
    mutable std::pair<sal_Int32, sal_Int32> m_StartMapNeedsSortingRange { SAL_MAX_INT32, -1 };
    mutable std::pair<sal_Int32, sal_Int32> m_EndMapNeedsSortingRange { SAL_MAX_INT32, -1 };
    // we are storing pairs of { Which, Start } here
    mutable std::pair<WhichStartPair, WhichStartPair> m_WhichMapNeedsSortingRange { { SAL_MAX_INT32, -1 }, { -1, -1 } };

    /// records a new attribute in m_pHistory.
    void NoteInHistory( SwTextAttr *pAttr, const bool bNew = false );

    void CalcFlags( );

    /** Delete methods may only be called by the TextNode!
       Because the TextNode also guarantees removal of the Character for
       attributes without an end. */
    friend class SwTextNode;
    void DeleteAtPos( size_t nPos );
    /// Delete the given Hint. The Hint must actually be in the array!
    void Delete( SwTextAttr const * pTextHt );

    void SetInSplitNode(bool bInSplit) { m_bInSplitNode = bInSplit; }
    void SetCalcHiddenParaField() const { m_bCalcHiddenParaField = true; }
    void SetHiddenByParaField( const bool bNew ) const { m_bHiddenByParaField = bNew; }
    bool IsHiddenByParaField() const
    {
        if ( m_bCalcHiddenParaField )
        {
            CalcHiddenParaField();
        }
        return m_bHiddenByParaField;
    }

    void InsertNesting(SwTextAttrNesting & rNewHint);
    bool TryInsertNesting(SwTextNode & rNode, SwTextAttrNesting & rNewHint);
    void BuildPortions( SwTextNode& rNode, SwTextAttr& rNewHint,
            const SetAttrMode nMode );
    bool MergePortions( SwTextNode& rNode );

    void Insert(SwTextAttr* pHt);
    SW_DLLPUBLIC void Resort() const;
    SW_DLLPUBLIC void ResortStartMap() const;
    SW_DLLPUBLIC void ResortEndMap() const;
    SW_DLLPUBLIC void ResortWhichMap() const;

    size_t GetIndexOf( const SwTextAttr *pHt ) const;

#ifdef DBG_UTIL
    bool Check(bool) const;
#endif

public:
    SwpHints(const SwTextNode& rParent);

    size_t Count() const { return m_HintsByStart.size(); }
    bool Contains( const SwTextAttr *pHt ) const;
    SwTextAttr * Get( size_t nPos ) const
    {
        assert( !(nPos != 0 && m_StartMapNeedsSortingRange.first != SAL_MAX_INT32) && "going to trigger a resort in the middle of an iteration, that's bad" );
        if (m_StartMapNeedsSortingRange.first != SAL_MAX_INT32)
            ResortStartMap();
        return m_HintsByStart[nPos];
    }
    // Get without triggering resorting - useful if we are modifying start/end pos while iterating
    SwTextAttr * GetWithoutResorting( size_t nPos ) const
    {
        return m_HintsByStart[nPos];
    }

    int GetLastPosSortedByEnd(sal_Int32 nEndPos) const;
    SwTextAttr * GetSortedByEnd( size_t nPos ) const
    {
        assert( !(nPos != 0 && m_EndMapNeedsSortingRange.first != SAL_MAX_INT32) && "going to trigger a resort in the middle of an iteration, that's bad" );
        if (m_EndMapNeedsSortingRange.first != SAL_MAX_INT32)
            ResortEndMap();
        return m_HintsByEnd[nPos];
    }

    size_t GetFirstPosSortedByWhichAndStart(sal_uInt16 nWhich) const;
    SwTextAttr * GetSortedByWhichAndStart( size_t nPos ) const
    {
        assert( !(nPos != 0 && m_WhichMapNeedsSortingRange.first.first != SAL_MAX_INT32) && "going to trigger a resort in the middle of an iteration, that's bad" );
        if (m_WhichMapNeedsSortingRange.first.first != SAL_MAX_INT32)
            ResortWhichMap();
        return m_HintsByWhichAndStart[nPos];
    }

    /// Trigger the sorting if necessary
    void SortIfNeedBe() const
    {
        if (m_StartMapNeedsSortingRange.first != SAL_MAX_INT32)
            ResortStartMap();
        if (m_EndMapNeedsSortingRange.first != SAL_MAX_INT32)
            ResortEndMap();
        if (m_WhichMapNeedsSortingRange.first.first != SAL_MAX_INT32)
            ResortWhichMap();
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

    // calc current value of m_bHiddenByParaField, returns true iff changed
    bool CalcHiddenParaField() const; // changes mutable state

    // Marks the hint-maps as needing sorting because the position of something has changed
    void StartPosChanged() const
    {
        m_StartMapNeedsSortingRange = { -1, -1 };
        m_EndMapNeedsSortingRange = { -1, -1 };
        m_WhichMapNeedsSortingRange = { { -1, -1 }, { -1, -1 } };
    }
    void EndPosChanged(sal_uInt16 nWhich, sal_Int32 nStartPos, sal_Int32 nOldEndPos, sal_Int32 nNewEndPos) const
    {
        m_StartMapNeedsSortingRange = { std::min(nStartPos, m_StartMapNeedsSortingRange.first),
                                        std::max(nStartPos, m_StartMapNeedsSortingRange.second) };
        m_EndMapNeedsSortingRange = { std::min(std::min(nOldEndPos, nNewEndPos), m_EndMapNeedsSortingRange.first),
                                      std::max(std::max(nOldEndPos, nNewEndPos), m_EndMapNeedsSortingRange.second) };
        WhichStartPair aPair { sal_Int32(nWhich), nStartPos };
        m_WhichMapNeedsSortingRange = { std::min(aPair, m_WhichMapNeedsSortingRange.first),
                                        std::max(aPair, m_WhichMapNeedsSortingRange.second) };
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
