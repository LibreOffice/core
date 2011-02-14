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

#ifndef SW_UNDO_MOVE_HXX
#define SW_UNDO_MOVE_HXX

#include <undobj.hxx>


class SwUndoSplitNode: public SwUndo
{
    SwHistory* pHistory;
    SwRedlineData* pRedlData;
    sal_uLong nNode;
    xub_StrLen nCntnt;
    sal_Bool bTblFlag : 1;
    sal_Bool bChkTblStt : 1;

public:
    SwUndoSplitNode( SwDoc* pDoc, const SwPosition& rPos, sal_Bool bChkTbl );

    virtual ~SwUndoSplitNode();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    void SetTblFlag()       { bTblFlag = sal_True; }
};


class SwUndoMove : public SwUndo, private SwUndRng, private SwUndoSaveCntnt
{
    // nDest.. - destination range of move (after move!)
    // nIns..  - source Position of move (after move!)
    // nMv..   - destination position of move (before move!); for REDO
    sal_uLong nDestSttNode, nDestEndNode, nInsPosNode, nMvDestNode;
    xub_StrLen nDestSttCntnt, nDestEndCntnt, nInsPosCntnt, nMvDestCntnt;

    sal_uInt16 nFtnStt; // StartPos of Footnotes in History

    sal_Bool bJoinNext : 1,
         bJoinPrev : 1,
         bMoveRange : 1;

    bool bMoveRedlines; // use DOC_MOVEREDLINES when calling SwDoc::Move

    void DelFtn( const SwPaM& );

public:
    SwUndoMove( const SwPaM&, const SwPosition& );
    SwUndoMove( SwDoc* pDoc, const SwNodeRange&, const SwNodeIndex& );

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    /// set the destination range after the move
    void SetDestRange( const SwPaM&, const SwPosition&, sal_Bool, sal_Bool );
    void SetDestRange( const SwNodeIndex& rStt, const SwNodeIndex& rEnd,
                        const SwNodeIndex& rInsPos );

    sal_Bool IsMoveRange() const        { return bMoveRange; }
    sal_uLong GetEndNode() const        { return nEndNode; }
    sal_uLong GetDestSttNode() const    { return nDestSttNode; }
    xub_StrLen GetDestSttCntnt() const  { return nDestSttCntnt; }

    void SetMoveRedlines( bool b )       { bMoveRedlines = b; }

};

#endif // SW_UNDO_MOVE_HXX

