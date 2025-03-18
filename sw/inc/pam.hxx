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
#ifndef INCLUDED_SW_INC_PAM_HXX
#define INCLUDED_SW_INC_PAM_HXX

#include <sal/types.h>
#include "ring.hxx"
#include "contentindex.hxx"
#include "ndindex.hxx"
#include "swdllapi.h"
#include "nodeoffset.hxx"

#include <iostream>
#include <utility>

class SwDoc;
class SwPaM;
class Point;

/// Marks a position in the document model.
struct SAL_WARN_UNUSED SW_DLLPUBLIC SwPosition
{
    SwNodeIndex nNode;
    SwContentIndex nContent;

    SwPosition( const SwNodeIndex &rNode, const SwContentIndex &rContent );
    SwPosition( const SwNode &rNode, const SwContentIndex &rContent );
    explicit SwPosition( SwNodes& rNodes, SwNodeOffset nIndex = SwNodeOffset(0) );
    explicit SwPosition( const SwNodeIndex &rNode, SwNodeOffset nDiff = SwNodeOffset(0) );
    explicit SwPosition( const SwNode& rNode, SwNodeOffset nDiff = SwNodeOffset(0) );
    explicit SwPosition( const SwContentNode& rNode, sal_Int32 nContentOffset = 0 );
    SwPosition( const SwNodeIndex &rNode, const SwContentNode*, sal_Int32 nContentOffset );
    SwPosition( const SwNode &rNode, const SwContentNode*, sal_Int32 nContentOffset );
    SwPosition( const SwNodeIndex &rNode, SwNodeOffset nDiff, const SwContentNode*, sal_Int32 nContentOffset );
    SwPosition( const SwNode &rNode, SwNodeOffset nDiff, const SwContentNode*, sal_Int32 nContentOffset );
    SwPosition( const SwContentIndex &, short nDiff );

    // callers should be using one of the other constructors to avoid creating a temporary
    SwPosition( SwNodeIndex && ) = delete;
    SwPosition( const SwNodeIndex &, SwContentIndex && ) = delete;
    SwPosition( SwNodeIndex &&, SwContentIndex && ) = delete;
    SwPosition( SwNodeIndex &&, const SwContentNode*, sal_Int32 ) = delete;
    SwPosition( SwNodeIndex &&, SwNodeOffset ) = delete;
    SwPosition( SwContentIndex &&, short ) = delete;

    /**
       Returns the document this position is in.

       @return the document this position is in.
    */
    SwDoc& GetDoc() const;

    bool operator < (const SwPosition &) const;
    bool operator > (const SwPosition &) const;
    bool operator <=(const SwPosition &) const;
    bool operator >=(const SwPosition &) const;
    bool operator ==(const SwPosition &) const;
    bool operator !=(const SwPosition &) const;
    void dumpAsXml(xmlTextWriterPtr pWriter) const;


    SwNodeOffset GetNodeIndex() const { return nNode.GetIndex(); }
    const SwNodes& GetNodes() const { return nNode.GetNodes(); }
    SwNodes& GetNodes() { return nNode.GetNodes(); }
    SwNode& GetNode() const { return nNode.GetNode(); }


    const SwContentNode* GetContentNode() const { return nContent.GetContentNode(); }
    sal_Int32 GetContentIndex() const { return nContent.GetIndex(); }
    void SetOwner(ISwContentIndexOwner* pOwner) { nContent.SetOwner(pOwner); }

    /// These all set both nNode and nContent
    void Assign( const SwNode& rNd, SwNodeOffset nDelta, sal_Int32 nContentOffset = 0 );
    void Assign( SwNodeOffset nNodeOffset, sal_Int32 nContentOffset = 0 );
    void Assign( const SwContentNode& rNode, sal_Int32 nContentOffset = 0 );
    void Assign( const SwNode& rNd, sal_Int32 nContentOffset = 0 );
    void Assign( const SwNodeIndex& rNdIdx, sal_Int32 nContentOffset = 0 );
    /// Set nNode to rNd, and nContent to the beginning of rNd
    void AssignStartIndex( const SwContentNode& rNd );
    /// Set nNode to rNd, and nContent to the end of rNd
    void AssignEndIndex( const SwContentNode& rNd );
    /// Adjust node position, and resets content position to zero
    void Adjust( SwNodeOffset nDelta );
    /// Adjust content index, only valid to call this if the position points to a SwContentNode subclass
    void AdjustContent( sal_Int32 nDelta );
    /// Set content index, only valid to call this if the position points to a SwContentNode subclass
    void SetContent( sal_Int32 nContentIndex );
};

SW_DLLPUBLIC std::ostream &operator <<(std::ostream& s, const SwPosition& position);

// Result of comparing positions.
enum class SwComparePosition {
    Before,             ///< Pos1 before Pos2.
    Behind,             ///< Pos1 behind Pos2.
    Inside,             ///< Pos1 completely contained in Pos2.
    Outside,            ///< Pos2 completely contained in Pos1.
    Equal,              ///< Pos1 is as large as Pos2.
    OverlapBefore,      ///< Pos1 overlaps Pos2 at the beginning.
    OverlapBehind,      ///< Pos1 overlaps Pos2 at the end.
    CollideStart,       ///< Pos1 start touches at Pos2 end.
    CollideEnd          ///< Pos1 end touches at Pos2 start.
};

template<typename T>
SwComparePosition ComparePosition(
            const T& rStt1, const T& rEnd1,
            const T& rStt2, const T& rEnd2 )
{
    SwComparePosition nRet;
    if( rStt1 < rStt2 )
    {
        if( rEnd1 > rStt2 )
        {
            if( rEnd1 >= rEnd2 )
                nRet = SwComparePosition::Outside;
            else
                nRet = SwComparePosition::OverlapBefore;

        }
        else if( rEnd1 == rStt2 )
            nRet = SwComparePosition::CollideEnd;
        else
            nRet = SwComparePosition::Before;
    }
    else if( rEnd2 > rStt1 )
    {
        if( rEnd2 >= rEnd1 )
        {
            if( rEnd2 == rEnd1 && rStt2 == rStt1 )
                nRet = SwComparePosition::Equal;
            else
                nRet = SwComparePosition::Inside;
        }
        else
        {
            if (rStt1 == rStt2)
                nRet = SwComparePosition::Outside;
            else
                nRet = SwComparePosition::OverlapBehind;
        }
    }
    else if( rEnd2 == rStt1 )
        nRet = SwComparePosition::CollideStart;
    else
        nRet = SwComparePosition::Behind;
    return nRet;
}

/// SwPointAndMark / SwPaM
struct SwMoveFnCollection;
SW_DLLPUBLIC extern SwMoveFnCollection const & fnMoveForward; ///< SwPam::Move()/Find() default argument.
SW_DLLPUBLIC extern SwMoveFnCollection const & fnMoveBackward;

using SwGoInDoc = auto (*)(SwPaM& rPam, SwMoveFnCollection const & fnMove) -> bool;
SW_DLLPUBLIC bool GoInDoc( SwPaM&, SwMoveFnCollection const &);
bool GoInSection( SwPaM&, SwMoveFnCollection const &);
SW_DLLPUBLIC bool GoInNode( SwPaM&, SwMoveFnCollection const &);
SW_DLLPUBLIC bool GoInContent( SwPaM&, SwMoveFnCollection const &);
bool GoInContentCells( SwPaM&, SwMoveFnCollection const &);
bool GoInContentSkipHidden( SwPaM&, SwMoveFnCollection const &);
bool GoInContentCellsSkipHidden( SwPaM&, SwMoveFnCollection const &);

/**
 * PaM is Point and Mark: a selection of the document model.
 *
 * The reason for the distinction is that the point moves around during adjusting the selection with
 * shift-arrow keys, while the mark remains where it is.
 */
class SAL_WARN_UNUSED SW_DLLPUBLIC SwPaM : public sw::Ring<SwPaM>
{
    SwPosition   m_Bound1;
    SwPosition   m_Bound2;
    SwPosition * m_pPoint; ///< points at either m_Bound1 or m_Bound2
    SwPosition * m_pMark;  ///< points at either m_Bound1 or m_Bound2
    bool m_bIsInFrontOfLabel;

    SwPaM(SwPaM const& rPaM) = delete;

public:
    explicit SwPaM( const SwPosition& rPos, SwPaM* pRing = nullptr );
    SwPaM( const SwPosition& rMk, const SwPosition& rPt, SwPaM* pRing = nullptr );
    SwPaM( const SwNodeIndex& rMk, const SwNodeIndex& rPt,
           SwNodeOffset nMkOffset = SwNodeOffset(0), SwNodeOffset nPtOffset = SwNodeOffset(0), SwPaM* pRing = nullptr );
    SwPaM( const SwNode& rMk, const SwNode& rPt,
           SwNodeOffset nMkOffset = SwNodeOffset(0), SwNodeOffset nPtOffset = SwNodeOffset(0), SwPaM* pRing = nullptr );
    SwPaM(  const SwNodeIndex& rMk, sal_Int32 nMkContent,
            const SwNodeIndex& rPt, sal_Int32 nPtContent, SwPaM* pRing = nullptr );
    SwPaM(  const SwNode& rMk, sal_Int32 nMkContent,
            const SwNode& rPt, sal_Int32 nPtContent, SwPaM* pRing = nullptr );
    SwPaM(  const SwNode& rMk, SwNodeOffset nMkOffset, sal_Int32 nMkContent,
            const SwNode& rPt, SwNodeOffset nPtOffset, sal_Int32 nPtContent, SwPaM* pRing = nullptr );
    SwPaM( const SwNode& rNd, SwNodeOffset nNdOffset, sal_Int32 nContent = 0, SwPaM* pRing = nullptr );
    explicit SwPaM( const SwNode& rNd, sal_Int32 nContent = 0, SwPaM* pRing = nullptr );
    explicit SwPaM( const SwNodeIndex& rNd, sal_Int32 nContent = 0, SwPaM* pRing = nullptr );
    explicit SwPaM( SwNodes& rNds, SwNodeOffset nMkOffset = SwNodeOffset(0), SwPaM* pRing = nullptr );
    virtual ~SwPaM() override;

    /// this takes a second parameter, which indicates the Ring that
    /// the new PaM should be part of (may be null)
    SwPaM(SwPaM const& rPaM, SwPaM * pRing);
    /// @@@ semantic: no copy assignment for super class Ring.
    SwPaM& operator=( const SwPaM & );

    /// Movement of cursor.
    bool Move( SwMoveFnCollection const & fnMove = fnMoveForward,
                SwGoInDoc fnGo = GoInContent );

    bool IsInFrontOfLabel() const        { return m_bIsInFrontOfLabel; }
    void SetInFrontOfLabel_( bool bNew ) { m_bIsInFrontOfLabel = bNew; }

    /// Unless this is called, the getter method of Mark will return Point.
    virtual void SetMark();

    void DeleteMark()
    {
        if (HasMark())
        {
            /** clear the mark position; this helps if mark's SwContentIndex is
               registered at some node, and that node is then deleted */
            m_pMark->Assign( *GetPointNode().GetNodes()[SwNodeOffset(0)] );
            m_pMark = m_pPoint;
        }
    }
    void Exchange()
    {
        if (HasMark())
            std::swap(m_pPoint, m_pMark);
    }

    /** A PaM marks a selection if Point and Mark are distinct positions.
        @return     true if the PaM spans a selection
     */
    bool HasMark() const { return m_pPoint != m_pMark; }

    const SwPosition *GetPoint() const { return m_pPoint; }
          SwPosition *GetPoint()       { return m_pPoint; }
    const SwPosition *GetMark()  const { return m_pMark; }
          SwPosition *GetMark()        { return m_pMark; }

    const SwPosition *Start() const
                { return (*m_pPoint) <= (*m_pMark) ? m_pPoint : m_pMark; }
          SwPosition *Start()
                { return (*m_pPoint) <= (*m_pMark) ? m_pPoint : m_pMark; }

    const SwPosition *End()   const
                { return (*m_pPoint) >  (*m_pMark) ? m_pPoint : m_pMark; }
          SwPosition *End()
                { return (*m_pPoint) >  (*m_pMark) ? m_pPoint : m_pMark; }

    /// Because sometimes the cost of the operator<= can add up
    std::pair<const SwPosition *, const SwPosition *> StartEnd() const
                { if ((*m_pPoint) <= (*m_pMark)) return { m_pPoint, m_pMark }; else return { m_pMark, m_pPoint }; }
    std::pair<SwPosition *, SwPosition *> StartEnd()
                { if ((*m_pPoint) <= (*m_pMark)) return { m_pPoint, m_pMark }; else return { m_pMark, m_pPoint }; }

    /// @return current Node at Point/Mark
    SwNode& GetPointNode() const { return m_pPoint->nNode.GetNode(); }
    SwNode& GetMarkNode() const { return m_pMark->nNode.GetNode(); }

    /// @return current ContentNode at Point/Mark
    SwContentNode* GetPointContentNode() const { return m_pPoint->nNode.GetNode().GetContentNode(); }
    SwContentNode* GetMarkContentNode() const { return m_pMark->nNode.GetNode().GetContentNode(); }

    /**
       Normalizes PaM, i.e. sort point and mark.

       @param bPointFirst true: If the point is behind the mark then swap.
                          false: If the mark is behind the point then swap.
    */
    void Normalize(bool bPointFirst = true);

    /// @return the document (SwDoc) at which the PaM is registered
    SwDoc& GetDoc() const   { return m_pPoint->nNode.GetNode().GetDoc(); }

          SwPosition& GetBound( bool bOne = true )
                            { return bOne ? m_Bound1 : m_Bound2; }
    const SwPosition& GetBound( bool bOne = true ) const
                            { return bOne ? m_Bound1 : m_Bound2; }

    /// Get number of page which contains cursor.
    sal_uInt16 GetPageNum( bool bAtPoint = true, const Point* pLayPos = nullptr );

    /** Is in something protected (readonly) or selection contains
       something protected. */
    bool HasReadonlySel(bool bFormView, bool isReplace) const;
    /** Is there hidden sections in the selected area. */
    bool HasHiddenSections() const;

    bool ContainsPosition(const SwPosition & rPos) const
    {
        return *Start() <= rPos && rPos <= *End();
    }

    OUString GetText() const;
    // copy text into buffer
    void AppendTextTo(rtl::OUStringBuffer& rBuffer) const;
    void InvalidatePaM();
    SwPaM* GetNext()
        { return GetNextInRing(); }
    const SwPaM* GetNext() const
        { return GetNextInRing(); }
    SwPaM* GetPrev()
        { return GetPrevInRing(); }
    const SwPaM* GetPrev() const
        { return GetPrevInRing(); }
    bool IsMultiSelection() const
        { return !unique(); }

    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

SW_DLLPUBLIC std::ostream &operator <<(std::ostream& s, const SwPaM& pam);

bool CheckNodesRange(const SwNode&, const SwNode&, bool bChkSection);

#endif // INCLUDED_SW_INC_PAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
