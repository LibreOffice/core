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
#ifndef _NDHINTS_HXX
#define _NDHINTS_HXX


#include <svl/svarray.hxx>
#include <tools/mempool.hxx>

#include "swtypes.hxx"

class SwTxtNode;
class SwRegHistory;                 // Is in RolBck.hxx.
class SwTxtAttr;
class SwTxtAttrNesting;

class SfxPoolItem;
class SfxItemSet;
class SwDoc;

SW_DLLPRIVATE SwTxtAttr *
MakeTxtAttr( SwDoc & rDoc, SfxPoolItem & rNew,
        xub_StrLen nStt, xub_StrLen nEnd );
SW_DLLPRIVATE SwTxtAttr *
MakeTxtAttr( SwDoc & rDoc, const SfxItemSet & rSet,
        xub_StrLen nStt, xub_StrLen nEnd );

// create redline dummy text hint that must not be inserted into hints array
SW_DLLPRIVATE SwTxtAttr*
MakeRedlineTxtAttr( SwDoc & rDoc, SfxPoolItem& rAttr );


// Class SwpHints is derived indirectly via SwpHts, because only the
// class SwTxtNode should be allowed to insert and remove attributes.
// Other classes like the Frames are given only reading access via
// the index-operator.
// Size when created is 1 because an array is created only if
// also a hint is inserted.

 // Class SwpHtStart/End


SV_DECL_PTRARR_SORT(SwpHtStart,SwTxtAttr*,1,1)
SV_DECL_PTRARR_SORT(SwpHtEnd,SwTxtAttr*,1,1)

// Class SwpHintsArr


/// the Hints array
class SwpHintsArray
{

protected:
    SwpHtStart m_HintStarts;
    SwpHtEnd   m_HintEnds;

    //FIXME: why are the non-const methods public?
public:
    void Insert( const SwTxtAttr *pHt );
    void DeleteAtPos( const USHORT nPosInStart );
    bool Resort();
    SwTxtAttr * Cut( const USHORT nPosInStart );

    inline const SwTxtAttr * GetStart( const USHORT nPos ) const
        { return m_HintStarts[nPos]; }
    inline const SwTxtAttr * GetEnd  ( const USHORT nPos ) const
        { return m_HintEnds  [nPos]; }
    inline       SwTxtAttr * GetStart( const USHORT nPos )
        { return m_HintStarts[nPos]; }
    inline       SwTxtAttr * GetEnd  ( const USHORT nPos )
        { return m_HintEnds  [nPos]; }

    inline USHORT GetEndCount()   const { return m_HintEnds  .Count(); }
    inline USHORT GetStartCount() const { return m_HintStarts.Count(); }

    inline USHORT GetStartOf( const SwTxtAttr *pHt ) const;
    inline USHORT GetPos( const SwTxtAttr *pHt ) const
        { return m_HintStarts.GetPos( pHt ); }

    inline SwTxtAttr * GetTextHint( const USHORT nIdx )
        { return GetStart(nIdx); }
    inline const SwTxtAttr * operator[]( const USHORT nIdx ) const
        { return m_HintStarts[nIdx]; }
    inline USHORT Count() const { return m_HintStarts.Count(); }

#if OSL_DEBUG_LEVEL > 1
    bool Check() const;
#endif
};


// Class SwpHints


// public interface
class SwpHints : public SwpHintsArray
{
private:
    SwRegHistory* m_pHistory;   // for Undo

    bool m_bFontChange          : 1;  // font change
    // true: the Node is in Split and Frames are moved
    bool m_bInSplitNode         : 1;
    // m_bHasHiddenParaField is invalid, call CalcHiddenParaField()
    bool m_bCalcHiddenParaField : 1;
    bool m_bHasHiddenParaField  : 1;  // HiddenParaFld
    bool m_bFootnote            : 1;  // footnotes
    bool m_bDDEFields           : 1;  // the TextNode has DDE fields

    // records a new attibute in m_pHistory.
    void NoteInHistory( SwTxtAttr *pAttr, const bool bNew = false );

    void CalcFlags( );

    // Delete methods may only be called by the TextNode!
    // Because the TextNode also guarantees removal of the Character for
    // attributes without an end.
    friend class SwTxtNode;
    void DeleteAtPos( const USHORT nPos );
    // Delete the given Hint. The Hint must actually be in the array!
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

    // register a History, which receives all attribute changes (for Undo)
    void Register( SwRegHistory* pHist ) { m_pHistory = pHist; }
    // deregister the currently registered History
    void DeRegister() { Register(0); }
    SwRegHistory* GetHistory() const    { return m_pHistory; }

    /// try to insert the hint
    /// @return true iff hint successfully inserted
    bool TryInsertHint( SwTxtAttr * const pHint, SwTxtNode & rNode,
            const SetAttrMode nMode = nsSetAttrMode::SETATTR_DEFAULT );

    inline bool HasFtn() const          { return m_bFootnote; }
    inline bool IsInSplitNode() const   { return m_bInSplitNode; }

    // calc current value of m_bHasHiddenParaField, returns true iff changed
    bool CalcHiddenParaField();

    DECL_FIXEDMEMPOOL_NEWDEL(SwpHints)
};

// Output operator for text hints.
SvStream &operator<<(SvStream &aS, const SwpHints &rHints); //$ ostream


// Inline Implementations


inline USHORT SwpHintsArray::GetStartOf( const SwTxtAttr *pHt ) const
{
    USHORT nPos;
    if ( !m_HintStarts.Seek_Entry( pHt, &nPos ) )
    {
        nPos = USHRT_MAX;
    }
    return nPos;
}

inline SwTxtAttr *SwpHintsArray::Cut( const USHORT nPosInStart )
{
    SwTxtAttr *pHt = GetTextHint(nPosInStart);
    DeleteAtPos( nPosInStart );
    return pHt;
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
