/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pam.hxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 15:53:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _PAM_HXX
#define _PAM_HXX

#include <stddef.h>         // fuer MemPool
#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _SVMEMPOOL_HXX //autogen
#include <tools/mempool.hxx>
#endif

#ifndef _CSHTYP_HXX
#include <cshtyp.hxx>       // fuer die Funktions-Definitionen
#endif
#ifndef _RING_HXX
#include <ring.hxx>         // Superklasse
#endif
#ifndef _INDEX_HXX
#include <index.hxx>        // fuer SwIndex
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>      // fuer SwNodeIndex
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

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

struct SwPosition
{
    SwNodeIndex nNode;
    SwIndex nContent;

    SwPosition( const SwNode& rNode );
    SwPosition( const SwNodeIndex &rNode );
    SwPosition( const SwNodeIndex &rNode, const SwIndex &rCntnt );
    /* @@@MAINTAINABILITY-HORROR@@@
       SwPosition desperately needs a constructor
       SwPosition( const SwNode& rNode, xub_StrLen nOffset );
    */

    SwPosition( const SwPosition & );
    SwPosition &operator=(const SwPosition &);

    // #111827#
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


// das Ergebnis eines Positions Vergleiches
enum SwComparePosition {
    POS_BEFORE,             // Pos1 liegt vor Pos2
    POS_BEHIND,             // Pos1 liegt hinter Pos2
    POS_INSIDE,             // Pos1 liegt vollstaendig in Pos2
    POS_OUTSIDE,            // Pos2 liegt vollstaendig in Pos1
    POS_EQUAL,              // Pos1 ist genauso gross wie Pos2
    POS_OVERLAP_BEFORE,     // Pos1 ueberlappt Pos2 am Anfang
    POS_OVERLAP_BEHIND,     // Pos1 ueberlappt Pos2 am Ende
    POS_COLLIDE_START,      // Pos1 Start stoesst an Pos2 Ende
    POS_COLLIDE_END         // Pos1 End stoesst an Pos2 Start
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
extern SwMoveFn fnMoveBackward;

typedef BOOL (*SwGoInDoc)( SwPaM& rPam, SwMoveFn fnMove );
extern SwGoInDoc fnGoDoc;
extern SwGoInDoc fnGoSection;
extern SwGoInDoc fnGoNode;
SW_DLLPUBLIC extern SwGoInDoc fnGoCntnt; // SwPam::Move() default argument.
extern SwGoInDoc fnGoCntntCells;
extern SwGoInDoc fnGoCntntSkipHidden;
extern SwGoInDoc fnGoCntntCellsSkipHidden;

void _InitPam();

class SwPaM : public Ring
{
    SwPosition aBound1;
    SwPosition aBound2;
    SwPosition *pPoint;
    SwPosition *pMark;
    BOOL bIsInFrontOfLabel;

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

    // Bewegen des Cursors
    BOOL Move( SwMoveFn fnMove = fnMoveForward,
                    SwGoInDoc fnGo = fnGoCntnt );

    // Suchen
    BYTE Find(  const com::sun::star::util::SearchOptions& rSearchOpt,
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


    inline BOOL IsInFrontOfLabel() const { return bIsInFrontOfLabel; }
    inline void _SetInFrontOfLabel( BOOL bNew ) { bIsInFrontOfLabel = bNew; }

    virtual void SetMark();
    void DeleteMark() { pMark = pPoint; }
#ifdef PRODUCT
    void Exchange()
    {
        if(pPoint != pMark)
        {
            SwPosition *pTmp = pPoint;
            pPoint = pMark;
            pMark = pTmp;
        }
    }
#else
    void Exchange();
#endif
    /*
     * Undokumented Feature: Liefert zurueck, ob das Pam ueber
     * eine Selektion verfuegt oder nicht. Definition einer
     * Selektion: Point und Mark zeigen auf unterschiedliche
     * Puffer.
     */
    BOOL HasMark() const { return pPoint == pMark? FALSE : TRUE; }

    const SwPosition *GetPoint() const { return pPoint; }
          SwPosition *GetPoint()       { return pPoint; }
    const SwPosition *GetMark()  const { return pMark; }
          SwPosition *GetMark()        { return pMark; }

    const SwPosition *Start() const
                    { return (*pPoint) <= (*pMark)? pPoint: pMark; }
          SwPosition *Start()
                    { return (*pPoint) <= (*pMark)? pPoint: pMark; }
    const SwPosition *End()   const
                    { return (*pPoint) > (*pMark)? pPoint: pMark; }
          SwPosition *End()
                    { return (*pPoint) > (*pMark)? pPoint: pMark; }

    // erfrage vom SwPaM den aktuellen Node/ContentNode am SPoint / Mark
    SwNode* GetNode( BOOL bPoint = TRUE ) const
    {
        return &( bPoint ? pPoint->nNode : pMark->nNode ).GetNode();
    }
    SwCntntNode* GetCntntNode( BOOL bPoint = TRUE ) const
    {
        return ( bPoint ? pPoint->nNode : pMark->nNode ).GetNode().GetCntntNode();
    }

    /**
       Normalizes PaM, i.e. sort point and mark.

       @param bPointFirst TRUE: If the point is behind the mark then swap.
                          FALSE: If the mark is behind the point then swap.
    */
    SwPaM & Normalize(BOOL bPointFirst = TRUE);

    // erfrage vom SwPaM das Dokument, in dem er angemeldet ist
    SwDoc* GetDoc() const { return pPoint->nNode.GetNode().GetDoc(); }
    SwPosition& GetBound( BOOL bOne = TRUE )
    {   return bOne ? aBound1 : aBound2; }
    const SwPosition& GetBound( BOOL bOne = TRUE ) const
    {   return bOne ? aBound1 : aBound2; }

    // erfrage die Seitennummer auf der der Cursor steht
    USHORT GetPageNum( BOOL bAtPoint = TRUE, const Point* pLayPos = 0 );

    // steht in etwas geschuetztem oder in die Selektion umspannt
    // etwas geschuetztes.
    BOOL HasReadonlySel( bool bFormView ) const;

    BOOL ContainsPosition(const SwPosition & rPos)
    { return *Start() <= rPos && rPos <= *End(); }

    DECL_FIXEDMEMPOOL_NEWDEL(SwPaM);

    String GetTxt() const;
};


BOOL CheckNodesRange( const SwNodeIndex&, const SwNodeIndex&, BOOL );
BOOL GoInCntnt( SwPaM & rPam, SwMoveFn fnMove );


#endif  // _PAM_HXX
