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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNDONUMBERING_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNDONUMBERING_HXX

#include <vector>
#include <undobj.hxx>
#include <rtl/ustring.hxx>
#include <numrule.hxx>

class SwUndoInsNum : public SwUndo, private SwUndRng
{
    SwNumRule aNumRule;
    SwHistory* pHistory;
    sal_uLong nSttSet;
    SwNumRule* pOldNumRule;
    OUString sReplaceRule;
    sal_uInt16 nLRSavePos;

public:
    SwUndoInsNum( const SwPaM& rPam, const SwNumRule& rRule );
    SwUndoInsNum( const SwNumRule& rOldRule, const SwNumRule& rNewRule,
                  SwUndoId nUndoId = UNDO_INSFMTATTR );
    SwUndoInsNum( const SwPosition& rPos, const SwNumRule& rRule,
                            const OUString& rReplaceRule );

    virtual ~SwUndoInsNum();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    virtual SwRewriter GetRewriter() const override;

    SwHistory* GetHistory(); // will be created if necessary
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

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    void AddNode( const SwTextNode& rNd, bool bResetLRSpace );
    SwHistory* GetHistory() { return pHistory; }
};

class SwUndoMoveNum : public SwUndo, private SwUndRng
{
    sal_uLong nNewStt;
    long nOffset;

public:
    SwUndoMoveNum( const SwPaM& rPam, long nOffset, bool bIsOutlMv = false );

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    void SetStartNode( sal_uLong nValue ) { nNewStt = nValue; }
};

class SwUndoNumUpDown : public SwUndo, private SwUndRng
{
    short nOffset;

public:
    SwUndoNumUpDown( const SwPaM& rPam, short nOffset );

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;
};

class SwUndoNumOrNoNum : public SwUndo
{
    sal_uLong nIdx;
    bool mbNewNum, mbOldNum;

public:
    SwUndoNumOrNoNum( const SwNodeIndex& rIdx, bool mbOldNum,
                      bool mbNewNum );

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;
};

class SwUndoNumRuleStart : public SwUndo
{
    sal_uLong nIdx;
    sal_uInt16 nOldStt, nNewStt;
    bool bSetSttValue : 1;
    bool bFlag : 1;

public:
    SwUndoNumRuleStart( const SwPosition& rPos, bool bDelete );
    SwUndoNumRuleStart( const SwPosition& rPos, sal_uInt16 nStt );

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDONUMBERING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
