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

#include <memory>
#include <vector>
#include <undobj.hxx>
#include <rtl/ustring.hxx>
#include <numrule.hxx>

class SwUndoInsNum : public SwUndo, private SwUndRng
{
    SwNumRule m_aNumRule;
    std::unique_ptr<SwHistory> m_pHistory;
    std::unique_ptr<SwNumRule> m_pOldNumRule;
    OUString m_sReplaceRule;
    sal_uInt16 m_nLRSavePos;

public:
    SwUndoInsNum( const SwPaM& rPam, const SwNumRule& rRule );
    SwUndoInsNum( const SwNumRule& rOldRule, const SwNumRule& rNewRule,
                  const SwDoc& rDoc, SwUndoId nUndoId = SwUndoId::INSFMTATTR );
    SwUndoInsNum( const SwPosition& rPos, const SwNumRule& rRule,
                            const OUString& rReplaceRule );

    virtual ~SwUndoInsNum() override;

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
        NodeLevel(sal_uLong idx, int lvl) : index(idx), level(lvl) {};
    };
    std::vector<NodeLevel>     m_aNodes;
    std::unique_ptr<SwHistory> m_pHistory;

public:
    SwUndoDelNum( const SwPaM& rPam );

    virtual ~SwUndoDelNum() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    void AddNode( const SwTextNode& rNd );
    SwHistory* GetHistory() { return m_pHistory.get(); }
};

class SwUndoMoveNum : public SwUndo, private SwUndRng
{
    sal_uLong m_nNewStart;
    long m_nOffset;

public:
    SwUndoMoveNum( const SwPaM& rPam, long nOffset, bool bIsOutlMv );

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    void SetStartNode( sal_uLong nValue ) { m_nNewStart = nValue; }
};

class SwUndoNumUpDown : public SwUndo, private SwUndRng
{
    short m_nOffset;

public:
    SwUndoNumUpDown( const SwPaM& rPam, short nOffset );

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;
};

class SwUndoNumOrNoNum : public SwUndo
{
    sal_uLong m_nIndex;
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
    sal_uLong m_nIndex;
    sal_uInt16 m_nOldStart, m_nNewStart;
    bool m_bSetStartValue : 1;
    bool m_bFlag : 1;

public:
    SwUndoNumRuleStart( const SwPosition& rPos, bool bDelete );
    SwUndoNumRuleStart( const SwPosition& rPos, sal_uInt16 nStt );

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDONUMBERING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
