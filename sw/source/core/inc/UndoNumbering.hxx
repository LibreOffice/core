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

#ifndef SW_UNDO_NUMBERING_HXX
#define SW_UNDO_NUMBERING_HXX

#include <vector>
#include <undobj.hxx>
#include <boost/shared_ptr.hpp>
#include <tools/string.hxx>
#include <numrule.hxx>

//----------- Undo for Numbering -------------------------------------
class SwUndoInsNum : public SwUndo, private SwUndRng
{
    SwNumRule aNumRule;
    SwHistory* pHistory;
    sal_uLong nSttSet;
    SwNumRule* pOldNumRule;
    String sReplaceRule;
    sal_uInt16 nLRSavePos;

public:
    SwUndoInsNum( const SwPaM& rPam, const SwNumRule& rRule );
    SwUndoInsNum( const SwNumRule& rOldRule, const SwNumRule& rNewRule,
                  SwUndoId nUndoId = UNDO_INSFMTATTR );
    SwUndoInsNum( const SwPosition& rPos, const SwNumRule& rRule,
                            const String& rReplaceRule );

    virtual ~SwUndoInsNum();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    virtual SwRewriter GetRewriter() const;

    SwHistory* GetHistory(); // will be created if necessary
    void SetSttNum( sal_uLong nNdIdx ) { nSttSet = nNdIdx; }
    void SaveOldNumRule( const SwNumRule& rOld );

    void SetLRSpaceEndPos();

};

class SwUndoDelNum : public SwUndo, private SwUndRng
{
    struct NodeLevel
    {
        sal_uLong index;
        int level;
        inline NodeLevel(sal_uLong idx, int lvl) : index(idx), level(lvl) {};
    };
    std::vector<NodeLevel> aNodes;
    SwHistory* pHistory;

public:
    SwUndoDelNum( const SwPaM& rPam );

    virtual ~SwUndoDelNum();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    void AddNode( const SwTxtNode& rNd, sal_Bool bResetLRSpace );
    SwHistory* GetHistory() { return pHistory; }
};

class SwUndoMoveNum : public SwUndo, private SwUndRng
{
    sal_uLong nNewStt;
    long nOffset;

public:
    SwUndoMoveNum( const SwPaM& rPam, long nOffset, bool bIsOutlMv = false );

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    void SetStartNode( sal_uLong nValue ) { nNewStt = nValue; }
};

class SwUndoNumUpDown : public SwUndo, private SwUndRng
{
    short nOffset;

public:
    SwUndoNumUpDown( const SwPaM& rPam, short nOffset );

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );
};

class SwUndoNumOrNoNum : public SwUndo
{
    sal_uLong nIdx;
    sal_Bool mbNewNum, mbOldNum;

public:
    SwUndoNumOrNoNum( const SwNodeIndex& rIdx, sal_Bool mbOldNum,
                      sal_Bool mbNewNum );

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );
};

class SwUndoNumRuleStart : public SwUndo
{
    sal_uLong nIdx;
    sal_uInt16 nOldStt, nNewStt;
    sal_Bool bSetSttValue : 1;
    sal_Bool bFlag : 1;

public:
    SwUndoNumRuleStart( const SwPosition& rPos, sal_Bool bDelete );
    SwUndoNumRuleStart( const SwPosition& rPos, sal_uInt16 nStt );

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );
};

#endif // SW_UNDO_NUMBERING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
