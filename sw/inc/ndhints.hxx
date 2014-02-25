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

class SwTxtNode;
class SwRegHistory;                 // Is in RolBck.hxx.
class SwTxtAttr;
class SwTxtAttrNesting;

class SfxPoolItem;
class SfxItemSet;
class SwDoc;

typedef enum {
    COPY = int(true),
    NEW  = int(false),
} CopyOrNew_t;

/// if COPY then pTxtNode must be given!
SwTxtAttr * MakeTxtAttr(
    SwDoc & rDoc,
    SfxPoolItem & rNew,
    sal_Int32 const nStt,
    sal_Int32 const nEnd,
    CopyOrNew_t const bIsCopy = NEW,
    SwTxtNode *const pTxtNode = 0 );

SwTxtAttr * MakeTxtAttr(
    SwDoc & rDoc,
    const SfxItemSet & rSet,
    sal_Int32 nStt,
    sal_Int32 nEnd );

/// create redline dummy text hint that must not be inserted into hints array
SwTxtAttr* MakeRedlineTxtAttr(
    SwDoc & rDoc,
    SfxPoolItem& rAttr );

/** Class SwpHints is derived indirectly via SwpHts, because only the
   class SwTxtNode should be allowed to insert and remove attributes.
   Other classes like the Frames are given only reading access via
   the index-operator.
   Size when created is 1 because an array is created only if
   also a hint is inserted. */

 /// Class SwpHtStart/End

struct CompareSwpHtStart
{
    bool operator()(SwTxtAttr* const lhs, SwTxtAttr* const rhs) const;
};
class SwpHtStart : public o3tl::sorted_vector<SwTxtAttr*, CompareSwpHtStart,
    o3tl::find_partialorder_ptrequals> {};

struct CompareSwpHtEnd
{
    bool operator()(SwTxtAttr* const lhs, SwTxtAttr* const rhs) const;
};
class SwpHtEnd : public o3tl::sorted_vector<SwTxtAttr*, CompareSwpHtEnd,
    o3tl::find_partialorder_ptrequals> {};

/// Class SwpHintsArr

/// the Hints array
class SwpHintsArray
{

protected:
    SwpHtStart m_HintStarts;
    SwpHtEnd   m_HintEnds;

    //FIXME: why are the non-const methods public?
public:
    void Insert( const SwTxtAttr *pHt );
    void DeleteAtPos( const sal_uInt16 nPosInStart );
    void Resort();
    SwTxtAttr * Cut( const sal_uInt16 nPosInStart );

    inline const SwTxtAttr * GetStart( const sal_uInt16 nPos ) const
        { return m_HintStarts[nPos]; }
    inline const SwTxtAttr * GetEnd  ( const sal_uInt16 nPos ) const
        { return m_HintEnds  [nPos]; }
    inline       SwTxtAttr * GetStart( const sal_uInt16 nPos )
        { return m_HintStarts[nPos]; }
    inline       SwTxtAttr * GetEnd  ( const sal_uInt16 nPos )
        { return m_HintEnds  [nPos]; }

    inline sal_uInt16 GetEndCount()   const { return m_HintEnds  .size(); }
    inline sal_uInt16 GetStartCount() const { return m_HintStarts.size(); }

    inline sal_uInt16 GetStartOf( const SwTxtAttr *pHt ) const;
    sal_uInt16 GetPos( const SwTxtAttr *pHt ) const;

    inline SwTxtAttr * GetTextHint( const sal_uInt16 nIdx )
        { return GetStart(nIdx); }
    inline const SwTxtAttr * operator[]( const sal_uInt16 nIdx ) const
        { return m_HintStarts[nIdx]; }
    inline sal_uInt16 Count() const { return m_HintStarts.size(); }

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
    bool m_bHasHiddenParaField  : 1;   ///< HiddenParaFld
    bool m_bFootnote            : 1;   ///< footnotes
    bool m_bDDEFields           : 1;   ///< the TextNode has DDE fields

    /// records a new attibute in m_pHistory.
    void NoteInHistory( SwTxtAttr *pAttr, const bool bNew = false );

    void CalcFlags( );

    /** Delete methods may only be called by the TextNode!
       Because the TextNode also guarantees removal of the Character for
       attributes without an end. */
    friend class SwTxtNode;
    void DeleteAtPos( const sal_uInt16 nPos );
    /// Delete the given Hint. The Hint must actually be in the array!
    void Delete( SwTxtAttr* pTxtHt );

    inline void SetInSplitNode(bool bInSplit) { m_bInSplitNode = bInSplit; }
    inline void SetCalcHiddenParaField() { m_bCalcHiddenParaField = true; }
    inline void SetHiddenParaField( const bool bNew )
        { m_bHasHiddenParaField = bNew; }
    inline bool HasHiddenParaField() const
    {
        if ( m_bCalcHiddenParaField )
        {
            (const_cast<SwpHints*>(this))->CalcHiddenParaField();
        }
        return m_bHasHiddenParaField;
    }

    void InsertNesting(SwTxtAttrNesting & rNewHint);
    bool TryInsertNesting(SwTxtNode & rNode, SwTxtAttrNesting & rNewHint);
    void BuildPortions( SwTxtNode& rNode, SwTxtAttr& rNewHint,
            const SetAttrMode nMode );
    bool MergePortions( SwTxtNode& rNode );

public:
    SwpHints();

    inline bool CanBeDeleted() const    { return !Count(); }

    /// register a History, which receives all attribute changes (for Undo)
    void Register( SwRegHistory* pHist ) { m_pHistory = pHist; }
    /// deregister the currently registered History
    void DeRegister() { Register(0); }
    SwRegHistory* GetHistory() const    { return m_pHistory; }

    /// try to insert the hint
    /// @return true iff hint successfully inserted
    bool TryInsertHint( SwTxtAttr * const pHint, SwTxtNode & rNode,
            const SetAttrMode nMode = nsSetAttrMode::SETATTR_DEFAULT );

    inline bool HasFtn() const          { return m_bFootnote; }
    inline bool IsInSplitNode() const   { return m_bInSplitNode; }

    /// calc current value of m_bHasHiddenParaField, returns true iff changed
    bool CalcHiddenParaField();

    DECL_FIXEDMEMPOOL_NEWDEL(SwpHints)
};

/// Inline Implementations

inline sal_uInt16 SwpHintsArray::GetStartOf( const SwTxtAttr *pHt ) const
{
    SwpHtStart::const_iterator const it =
        m_HintStarts.find(const_cast<SwTxtAttr*>(pHt));
    if ( it == m_HintStarts.end() )
    {
        return USHRT_MAX;
    }
    return it - m_HintStarts.begin();
}

inline SwTxtAttr *SwpHintsArray::Cut( const sal_uInt16 nPosInStart )
{
    SwTxtAttr *pHt = GetTextHint(nPosInStart);
    DeleteAtPos( nPosInStart );
    return pHt;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
