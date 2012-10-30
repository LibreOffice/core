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

#ifndef SW_UNDO_NUMBERING_HXX
#define SW_UNDO_NUMBERING_HXX

#include <vector>
#include <undobj.hxx>
#include <boost/shared_ptr.hpp>
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
