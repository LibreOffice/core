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
    std::unique_ptr<SwHistory> m_pHistory;
    std::unique_ptr<SwRedlineData> m_pRedlineData;
    sal_uLong m_nNode;
    sal_Int32 m_nContent;
    bool m_bTableFlag : 1;
    bool m_bCheckTableStart : 1;
    sal_uInt32 m_nParRsid;

public:
    SwUndoSplitNode( SwDoc* pDoc, const SwPosition& rPos, bool bChkTable );

    virtual ~SwUndoSplitNode() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    void SetTableFlag()       { m_bTableFlag = true; }
};

class SwUndoMove : public SwUndo, private SwUndRng, private SwUndoSaveContent
{
    // nDest... - destination range of move (after move!)
    // nIns...  - source Position of move (after move!)
    // nMv...   - destination position of move (before move!); for REDO
    sal_uLong m_nDestStartNode, m_nDestEndNode, m_nInsPosNode, m_nMoveDestNode;
    sal_Int32 m_nDestStartContent, m_nDestEndContent, m_nInsPosContent, m_nMoveDestContent;

    sal_uInt16 m_nFootnoteStart; // StartPos of Footnotes in History

    bool m_bJoinNext : 1,
         m_bMoveRange : 1;

    bool m_bMoveRedlines; // use DOC_MOVEREDLINES when calling SwDoc::Move

    void DelFootnote( const SwPaM& );

public:
    SwUndoMove( const SwPaM&, const SwPosition& );
    SwUndoMove( SwDoc& rDoc, const SwNodeRange&, const SwNodeIndex& );

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    /// set the destination range after the move
    void SetDestRange( const SwNodeIndex& rStt, const SwNodeIndex& rEnd,
                        const SwNodeIndex& rInsPos );

    bool IsMoveRange() const        { return m_bMoveRange; }
    sal_uLong GetEndNode() const        { return m_nEndNode; }
    sal_uLong GetDestSttNode() const    { return m_nDestStartNode; }
    sal_Int32 GetDestSttContent() const  { return m_nDestStartContent; }

    void SetMoveRedlines( bool b )       { m_bMoveRedlines = b; }
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDOSPLITMOVE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
