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
    SwTextNode *const pTextNode = 0 );

SwTextAttr * MakeTextAttr(
    SwDoc & rDoc,
    const SfxItemSet & rSet,
    sal_Int32 nStt,
    sal_Int32 nEnd );

/// create redline dummy text hint that must not be inserted into hints array
SwTextAttr* MakeRedlineTextAttr(
    SwDoc & rDoc,
    SfxPoolItem& rAttr );

/** Class SwpHints is derived indirectly via SwpHts, because only the
   class SwTextNode should be allowed to insert and remove attributes.
   Other classes like the Frames are given only reading access via
   the index-operator.
   Size when created is 1 because an array is created only if
   also a hint is inserted. */

 /// Class SwpHtStart/End

struct CompareSwpHtStart
{
    bool operator()(SwTextAttr* const lhs, SwTextAttr* const rhs) const;
};
class SwpHtStart : public o3tl::sorted_vector<SwTextAttr*, CompareSwpHtStart,
    o3tl::find_partialorder_ptrequals> {};

struct CompareSwpHtEnd
{
    bool operator()(SwTextAttr* const lhs, SwTextAttr* const rhs) const;
};
class SwpHtEnd : public o3tl::sorted_vector<SwTextAttr*, CompareSwpHtEnd,
    o3tl::find_partialorder_ptrequals> {};

/// Class SwpHintsArr

/// the Hints array
class SwpHintsArray
{

protected:
    SwpHtStart m_HintStarts;
    SwpHtEnd   m_HintEnds;

    // SAL_MAX_SIZE is used by GetStartOf to return
    // failure, so just allow SAL_MAX_SIZE-1 hints
    static const size_t MAX_HINTS = SAL_MAX_SIZE-1;

    //FIXME: why are the non-const methods public?
public:
    void Insert( const SwTextAttr *pHt );
    void DeleteAtPos( const size_t nPosInStart );
    void Resort();
    SwTextAttr * Cut( const size_t nPosInStart )
    {
        SwTextAttr *pHt = GetTextHint(nPosInStart);
        DeleteAtPos( nPosInStart );
        return pHt;
    }
    const SwTextAttr * GetStart( const size_t nPos ) const
    {
        assert(nPos < m_HintStarts.size());
        return m_HintStarts[nPos];
    }
    const SwTextAttr * GetEnd( const size_t nPos ) const
    {
        assert(nPos < m_HintEnds.size());
        return m_HintEnds[nPos];
    }
    SwTextAttr * GetStart( const size_t nPos )
    {
        assert(nPos < m_HintStarts.size());
        return m_HintStarts[nPos];
    }
    SwTextAttr * GetEnd( const size_t nPos )
    {
        assert(nPos < m_HintEnds.size());
        return m_HintEnds[nPos];
    }

    size_t GetEndCount()   const { return m_HintEnds.size(); }
    size_t GetStartCount() const { return m_HintStarts.size(); }

    size_t GetStartOf( const SwTextAttr *pHt ) const
    {
        SwpHtStart::const_iterator const it =
            m_HintStarts.find(const_cast<SwTextAttr*>(pHt));
        if ( it == m_HintStarts.end() )
        {
            return SAL_MAX_SIZE;
        }
        return it - m_HintStarts.begin();
    }

    bool Contains( const SwTextAttr *pHt ) const;

    const SwTextAttr * GetTextHint( const size_t nIdx ) const
        { return GetStart(nIdx); }
    SwTextAttr * GetTextHint( const size_t nIdx )
        { return GetStart(nIdx); }
    const SwTextAttr * operator[]( const size_t nIdx ) const
        { return GetStart(nIdx); }
    size_t Count() const { return GetStartCount(); }

#ifdef DBG_UTIL
    bool Check(bool) const;
#endif
};

// Class SwpHints

/// public interface
class SwpHints : public SwpHintsArray
{
private:
    SwRegHistory* m_pHistory;          ///< for Undo

    bool m_bFontChange          : 1;   ///< font change
    /// true: the Node is in Split and Frames are moved
    bool m_bInSplitNode         : 1;
    /// m_bHasHiddenParaField is invalid, call CalcHiddenParaField()
    bool m_bCalcHiddenParaField : 1;
    bool m_bHasHiddenParaField  : 1;   ///< HiddenParaField
    bool m_bFootnote            : 1;   ///< footnotes
    bool m_bDDEFields           : 1;   ///< the TextNode has DDE fields

    /// records a new attibute in m_pHistory.
    void NoteInHistory( SwTextAttr *pAttr, const bool bNew = false );

    void CalcFlags( );

    /** Delete methods may only be called by the TextNode!
       Because the TextNode also guarantees removal of the Character for
       attributes without an end. */
    friend class SwTextNode;
    void DeleteAtPos( const size_t nPos );
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

public:
    SwpHints();

    bool CanBeDeleted() const    { return !Count(); }

    /// register a History, which receives all attribute changes (for Undo)
    void Register( SwRegHistory* pHist ) { m_pHistory = pHist; }
    /// deregister the currently registered History
    void DeRegister() { Register(0); }
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
