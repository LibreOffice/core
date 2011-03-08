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
#ifndef _PAM_HXX
#define _PAM_HXX

#include <stddef.h>         // For MemPool.
#include <tools/gen.hxx>
#include <tools/mempool.hxx>
#include <cshtyp.hxx>       // For function definitions.
#include <ring.hxx>         // Super class.
#include <index.hxx>        // For SwIndex.
#include <ndindex.hxx>      // For SwNodeIndex.
#include "swdllapi.h"

class SwFmt;
class SfxPoolItem;
class SfxItemSet;
class SwDoc;
class SwNode;
class SwCntntNode;
class SwPaM;

namespace com { namespace sun { namespace star { namespace util {
    struct SearchOptions;
} } } }

namespace utl {
    class TextSearch;
}

struct SW_DLLPUBLIC SwPosition
{
    SwNodeIndex nNode;
    SwIndex nContent;

    SwPosition( const SwNodeIndex &rNode, const SwIndex &rCntnt );
    explicit SwPosition( const SwNodeIndex &rNode );
    explicit SwPosition( const SwNode& rNode );
    explicit SwPosition( SwCntntNode& rNode, const xub_StrLen nOffset = 0 );

    SwPosition( const SwPosition & );
    SwPosition &operator=(const SwPosition &);

    /**
       Returns the document this position is in.

       @return the document this position is in.
    */
    SwDoc * GetDoc() const;

    BOOL operator < (const SwPosition &) const;
    BOOL operator > (const SwPosition &) const;
    BOOL operator <=(const SwPosition &) const;
    BOOL operator >=(const SwPosition &) const;
    BOOL operator ==(const SwPosition &) const;
    BOOL operator !=(const SwPosition &) const;
};


// Result of comparing positions.
enum SwComparePosition {
    POS_BEFORE,             // Pos1 before Pos2.
    POS_BEHIND,             // Pos1 behind Pos2.
    POS_INSIDE,             // Pos1 completely contained in Pos2.
    POS_OUTSIDE,            // Pos2 completely contained in Pos1.
    POS_EQUAL,              // Pos1 is as large as Pos2.
    POS_OVERLAP_BEFORE,     // Pos1 overlaps Pos2 at the beginning.
    POS_OVERLAP_BEHIND,     // Pos1 overlaps Pos2 at the end.
    POS_COLLIDE_START,      // Pos1 start touches at Pos2 end.
    POS_COLLIDE_END         // Pos1 end touches at Pos2 start.
};
SwComparePosition ComparePosition(
            const SwPosition& rStt1, const SwPosition& rEnd1,
            const SwPosition& rStt2, const SwPosition& rEnd2 );

SwComparePosition ComparePosition(
            const unsigned long nStt1, const unsigned long nEnd1,
            const unsigned long nStt2, const unsigned long nEnd2 );


// SwPointAndMark / SwPaM
struct SwMoveFnCollection;
typedef SwMoveFnCollection* SwMoveFn;
SW_DLLPUBLIC extern SwMoveFn fnMoveForward; // SwPam::Move()/Find() default argument.
SW_DLLPUBLIC extern SwMoveFn fnMoveBackward;

typedef BOOL (*SwGoInDoc)( SwPaM& rPam, SwMoveFn fnMove );
SW_DLLPUBLIC extern SwGoInDoc fnGoDoc;
extern SwGoInDoc fnGoSection;
SW_DLLPUBLIC extern SwGoInDoc fnGoNode;
SW_DLLPUBLIC extern SwGoInDoc fnGoCntnt; // SwPam::Move() default argument.
extern SwGoInDoc fnGoCntntCells;
extern SwGoInDoc fnGoCntntSkipHidden;
extern SwGoInDoc fnGoCntntCellsSkipHidden;

void _InitPam();

class SW_DLLPUBLIC SwPaM : public Ring
{
    SwPosition   m_Bound1;
    SwPosition   m_Bound2;
    SwPosition * m_pPoint; // points at either m_Bound1 or m_Bound2
    SwPosition * m_pMark;  // points at either m_Bound1 or m_Bound2
    bool m_bIsInFrontOfLabel;

    SwPaM* MakeRegion( SwMoveFn fnMove, const SwPaM * pOrigRg = 0 );

public:
    SwPaM( const SwPosition& rPos, SwPaM* pRing = 0 );
    SwPaM( const SwPosition& rMk, const SwPosition& rPt, SwPaM* pRing = 0 );
    SwPaM( const SwNodeIndex& rMk, const SwNodeIndex& rPt,
           long nMkOffset = 0, long nPtOffset = 0, SwPaM* pRing = 0 );
    SwPaM( const SwNode& rMk, const SwNode& rPt,
           long nMkOffset = 0, long nPtOffset = 0, SwPaM* pRing = 0 );
    SwPaM(  const SwNodeIndex& rMk, xub_StrLen nMkCntnt,
            const SwNodeIndex& rPt, xub_StrLen nPtCntnt, SwPaM* pRing = 0 );
    SwPaM(  const SwNode& rMk, xub_StrLen nMkCntnt,
            const SwNode& rPt, xub_StrLen nPtCntnt, SwPaM* pRing = 0 );
    SwPaM( const SwNode& rNd, xub_StrLen nCntnt = 0, SwPaM* pRing = 0 );
    SwPaM( const SwNodeIndex& rNd, xub_StrLen nCntnt = 0, SwPaM* pRing = 0 );
    virtual ~SwPaM();

    // @@@ semantic: no copy ctor.
    SwPaM( SwPaM & );
    // @@@ semantic: no copy assignment for super class Ring.
    SwPaM& operator=( const SwPaM & );

    // Movement of cursor.
    BOOL Move( SwMoveFn fnMove = fnMoveForward,
                    SwGoInDoc fnGo = fnGoCntnt );

    // Search.
    BYTE Find(  const com::sun::star::util::SearchOptions& rSearchOpt,
                BOOL bSearchInNotes,
                utl::TextSearch& rSTxt,
                SwMoveFn fnMove = fnMoveForward,
                const SwPaM *pPam =0, BOOL bInReadOnly = FALSE);
    BOOL Find(  const SwFmt& rFmt,
                SwMoveFn fnMove = fnMoveForward,
                const SwPaM *pPam =0, BOOL bInReadOnly = FALSE);
    BOOL Find(  const SfxPoolItem& rAttr, BOOL bValue = TRUE,
                SwMoveFn fnMove = fnMoveForward,
                const SwPaM *pPam =0, BOOL bInReadOnly = FALSE );
    BOOL Find(  const SfxItemSet& rAttr, BOOL bNoColls,
                SwMoveFn fnMove,
                const SwPaM *pPam, BOOL bInReadOnly, BOOL bMoveFirst );

    bool DoSearch( const com::sun::star::util::SearchOptions& rSearchOpt, utl::TextSearch& rSTxt,
                    SwMoveFn fnMove, BOOL bSrchForward, BOOL bRegSearch, BOOL bChkEmptyPara, BOOL bChkParaEnd,
                    xub_StrLen &nStart, xub_StrLen &nEnde,xub_StrLen nTxtLen,SwNode* pNode, SwPaM* pPam);

    inline bool IsInFrontOfLabel() const        { return m_bIsInFrontOfLabel; }
    inline void _SetInFrontOfLabel( bool bNew ) { m_bIsInFrontOfLabel = bNew; }

    virtual void SetMark();

    void DeleteMark()
    {
        if (m_pMark != m_pPoint)
        {
            // clear the mark position; this helps if mark's SwIndex is
            // registered at some node, and that node is then deleted
            *m_pMark = SwPosition( SwNodeIndex( GetNode()->GetNodes() ) );
            m_pMark = m_pPoint;
        }
    }
#if OSL_DEBUG_LEVEL > 1
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
        @return     true iff the PaM spans a selection
     */
    bool HasMark() const { return m_pPoint == m_pMark ? false : true; }

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
    SwNode    * GetNode      ( bool bPoint = true ) const
    {
        return &( bPoint ? m_pPoint->nNode : m_pMark->nNode ).GetNode();
    }

    /// @return current ContentNode at Point/Mark
    SwCntntNode* GetCntntNode( bool bPoint = true ) const
    {
        return GetNode(bPoint)->GetCntntNode();
    }

    /**
       Normalizes PaM, i.e. sort point and mark.

       @param bPointFirst TRUE: If the point is behind the mark then swap.
                          FALSE: If the mark is behind the point then swap.
    */
    SwPaM & Normalize(BOOL bPointFirst = TRUE);

    /// @return the document (SwDoc) at which the PaM is registered
    SwDoc* GetDoc() const   { return m_pPoint->nNode.GetNode().GetDoc(); }

          SwPosition& GetBound( bool bOne = true )
                            { return bOne ? m_Bound1 : m_Bound2; }
    const SwPosition& GetBound( bool bOne = true ) const
                            { return bOne ? m_Bound1 : m_Bound2; }

    // Get number of page which contains cursor.
    USHORT GetPageNum( BOOL bAtPoint = TRUE, const Point* pLayPos = 0 );

    // Is in something protected (readonly) or selection contains
    // something protected.
    BOOL HasReadonlySel( bool bFormView ) const;

    BOOL ContainsPosition(const SwPosition & rPos)
    { return *Start() <= rPos && rPos <= *End(); }

    static BOOL Overlap(const SwPaM & a, const SwPaM & b);

    static BOOL LessThan(const SwPaM & a, const SwPaM & b);

    DECL_FIXEDMEMPOOL_NEWDEL(SwPaM);

    String GetTxt() const;
    void InvalidatePaM();
};


BOOL CheckNodesRange( const SwNodeIndex&, const SwNodeIndex&, BOOL );
BOOL GoInCntnt( SwPaM & rPam, SwMoveFn fnMove );


#endif  // _PAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
