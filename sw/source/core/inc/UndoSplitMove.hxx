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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNDOSPLITMOVE_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNDOSPLITMOVE_HXX

#include <undobj.hxx>

class SwUndoSplitNode: public SwUndo
{
    SwHistory* pHistory;
    SwRedlineData* pRedlData;
    sal_uLong nNode;
    sal_Int32 nCntnt;
    sal_Bool bTblFlag : 1;
    sal_Bool bChkTblStt : 1;
    sal_uInt32 nParRsid;

public:
    SwUndoSplitNode( SwDoc* pDoc, const SwPosition& rPos, sal_Bool bChkTbl );

    virtual ~SwUndoSplitNode();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;
    virtual void RepeatImpl( ::sw::RepeatContext & ) SAL_OVERRIDE;

    void SetTblFlag()       { bTblFlag = sal_True; }
};

class SwUndoMove : public SwUndo, private SwUndRng, private SwUndoSaveCntnt
{
    // nDest.. - destination range of move (after move!)
    // nIns..  - source Position of move (after move!)
    // nMv..   - destination position of move (before move!); for REDO
    sal_uLong nDestSttNode, nDestEndNode, nInsPosNode, nMvDestNode;
    sal_Int32 nDestSttCntnt, nDestEndCntnt, nInsPosCntnt, nMvDestCntnt;

    sal_uInt16 nFtnStt; // StartPos of Footnotes in History

    sal_Bool bJoinNext : 1,
         bJoinPrev : 1,
         bMoveRange : 1;

    bool bMoveRedlines; // use DOC_MOVEREDLINES when calling SwDoc::Move

    void DelFtn( const SwPaM& );

public:
    SwUndoMove( const SwPaM&, const SwPosition& );
    SwUndoMove( SwDoc* pDoc, const SwNodeRange&, const SwNodeIndex& );

    virtual void UndoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;

    /// set the destination range after the move
    void SetDestRange( const SwPaM&, const SwPosition&, sal_Bool, sal_Bool );
    void SetDestRange( const SwNodeIndex& rStt, const SwNodeIndex& rEnd,
                        const SwNodeIndex& rInsPos );

    sal_Bool IsMoveRange() const        { return bMoveRange; }
    sal_uLong GetEndNode() const        { return nEndNode; }
    sal_uLong GetDestSttNode() const    { return nDestSttNode; }
    sal_Int32 GetDestSttCntnt() const  { return nDestSttCntnt; }

    void SetMoveRedlines( bool b )       { bMoveRedlines = b; }
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDOSPLITMOVE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
