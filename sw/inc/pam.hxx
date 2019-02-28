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
#include "index.hxx"
#include "ndindex.hxx"
#include "swdllapi.h"

#include <iostream>

class SwDoc;
class SwPaM;
class Point;

/// Marks a position in the document model.
struct SAL_WARN_UNUSED SW_DLLPUBLIC SwPosition
{
    SwNodeIndex nNode;
    SwIndex nContent;

    SwPosition( const SwNodeIndex &rNode, const SwIndex &rContent );
    explicit SwPosition( const SwNodeIndex &rNode );
    explicit SwPosition( const SwNode& rNode );
    explicit SwPosition( SwContentNode& rNode, const sal_Int32 nOffset = 0 );

    /**
       Returns the document this position is in.

       @return the document this position is in.
    */
    SwDoc * GetDoc() const;

    bool operator < (const SwPosition &) const;
    bool operator > (const SwPosition &) const;
    bool operator <=(const SwPosition &) const;
    bool operator >=(const SwPosition &) const;
    bool operator ==(const SwPosition &) const;
    bool operator !=(const SwPosition &) const;
    void dumpAsXml(xmlTextWriterPtr pWriter) const;
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
SW_DLLPUBLIC bool GoInSection( SwPaM&, SwMoveFnCollection const &);
SW_DLLPUBLIC bool GoInNode( SwPaM&, SwMoveFnCollection const &);
SW_DLLPUBLIC bool GoInContent( SwPaM&, SwMoveFnCollection const &);
SW_DLLPUBLIC bool GoInContentCells( SwPaM&, SwMoveFnCollection const &);
SW_DLLPUBLIC bool GoInContentSkipHidden( SwPaM&, SwMoveFnCollection const &);
SW_DLLPUBLIC bool GoInContentCellsSkipHidden( SwPaM&, SwMoveFnCollection const &);

/// PaM is Point and Mark: a selection of the document model.
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
           long nMkOffset = 0, long nPtOffset = 0, SwPaM* pRing = nullptr );
    SwPaM( const SwNode& rMk, const SwNode& rPt,
           long nMkOffset = 0, long nPtOffset = 0, SwPaM* pRing = nullptr );
    SwPaM(  const SwNodeIndex& rMk, sal_Int32 nMkContent,
            const SwNodeIndex& rPt, sal_Int32 nPtContent, SwPaM* pRing = nullptr );
    SwPaM(  const SwNode& rMk, sal_Int32 nMkContent,
            const SwNode& rPt, sal_Int32 nPtContent, SwPaM* pRing = nullptr );
    SwPaM( const SwNode& rNd, sal_Int32 nContent = 0, SwPaM* pRing = nullptr );
    SwPaM( const SwNodeIndex& rNd, sal_Int32 nContent = 0, SwPaM* pRing = nullptr );
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
        if (m_pMark != m_pPoint)
        {
            /** clear the mark position; this helps if mark's SwIndex is
               registered at some node, and that node is then deleted */
            *m_pMark = SwPosition( SwNodeIndex( GetNode().GetNodes() ) );
            m_pMark = m_pPoint;
        }
    }
#ifdef DBG_UTIL
    void Exchange();

#else
    void Exchange()
    {
        if (m_pPoint != m_pMark)
        {
            SwPosition *pTmp = m_pPoint;
            m_pPoint = m_pMark;
            m_pMark = pTmp;
        }
    }
#endif

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

    /// @return current Node at Point/Mark
    SwNode    & GetNode      ( bool bPoint = true ) const
    {
        return ( bPoint ? m_pPoint->nNode : m_pMark->nNode ).GetNode();
    }

    /// @return current ContentNode at Point/Mark
    SwContentNode* GetContentNode( bool bPoint = true ) const
    {
        return GetNode(bPoint).GetContentNode();
    }

    /**
       Normalizes PaM, i.e. sort point and mark.

       @param bPointFirst true: If the point is behind the mark then swap.
                          false: If the mark is behind the point then swap.
    */
    void Normalize(bool bPointFirst = true);

    /// @return the document (SwDoc) at which the PaM is registered
    SwDoc* GetDoc() const   { return m_pPoint->nNode.GetNode().GetDoc(); }

          SwPosition& GetBound( bool bOne = true )
                            { return bOne ? m_Bound1 : m_Bound2; }
    const SwPosition& GetBound( bool bOne = true ) const
                            { return bOne ? m_Bound1 : m_Bound2; }

    /// Get number of page which contains cursor.
    sal_uInt16 GetPageNum( bool bAtPoint = true, const Point* pLayPos = nullptr );

    /** Is in something protected (readonly) or selection contains
       something protected. */
    bool HasReadonlySel( bool bFormView ) const;

    bool ContainsPosition(const SwPosition & rPos) const
    {
        return *Start() <= rPos && rPos <= *End();
    }

    OUString GetText() const;
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

bool CheckNodesRange( const SwNodeIndex&, const SwNodeIndex&, bool bChkSection );

#endif // INCLUDED_SW_INC_PAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
