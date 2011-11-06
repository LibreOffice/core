/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

    void AddNode( const SwTxtNode& rNd, sal_Bool bResetLRSpace );
    SwHistory* GetHistory() { return pHistory; }

};

class SwUndoMoveNum : public SwUndo, private SwUndRng
{
    sal_uLong nNewStt;
    long nOffset;
public:
    SwUndoMoveNum( const SwPaM& rPam, long nOffset, sal_Bool bIsOutlMv = sal_False );

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

