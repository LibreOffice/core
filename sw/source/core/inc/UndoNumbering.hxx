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

#include <undobj.hxx>

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#define _SVSTDARR_ULONGS
#define _SVSTDARR_BOOLS
#define _SVSTDARR_BYTES
#define _SVSTDARR_USHORTSSORT
#include <svl/svstdarr.hxx>
#endif

#include <numrule.hxx>


//--------------------------------------------------------------------
//----------- Undo for Numbering -------------------------------------

class SwUndoInsNum : public SwUndo, private SwUndRng
{
    SwNumRule aNumRule;
    SwHistory* pHistory;
    ULONG nSttSet;
    SwNumRule* pOldNumRule;
    String sReplaceRule;
    USHORT nLRSavePos;
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
    void SetSttNum( ULONG nNdIdx ) { nSttSet = nNdIdx; }
    void SaveOldNumRule( const SwNumRule& rOld );

    void SetLRSpaceEndPos();

};

class SwUndoDelNum : public SwUndo, private SwUndRng
{
    SvULongs aNodeIdx;
    SvBytes aLevels;
    SvBools aRstLRSpaces;
    SwHistory* pHistory;
public:
    SwUndoDelNum( const SwPaM& rPam );

    virtual ~SwUndoDelNum();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    void AddNode( const SwTxtNode& rNd, BOOL bResetLRSpace );
    SwHistory* GetHistory() { return pHistory; }

};

class SwUndoMoveNum : public SwUndo, private SwUndRng
{
    ULONG nNewStt;
    long nOffset;
public:
    SwUndoMoveNum( const SwPaM& rPam, long nOffset, BOOL bIsOutlMv = FALSE );

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    void SetStartNode( ULONG nValue ) { nNewStt = nValue; }
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
    ULONG nIdx;
    BOOL mbNewNum, mbOldNum;

public:
    SwUndoNumOrNoNum( const SwNodeIndex& rIdx, BOOL mbOldNum,
                      BOOL mbNewNum );

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );
};

class SwUndoNumRuleStart : public SwUndo
{
    ULONG nIdx;
    USHORT nOldStt, nNewStt;
    BOOL bSetSttValue : 1;
    BOOL bFlag : 1;
public:
    SwUndoNumRuleStart( const SwPosition& rPos, BOOL bDelete );
    SwUndoNumRuleStart( const SwPosition& rPos, USHORT nStt );

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );
};

#endif // SW_UNDO_NUMBERING_HXX

