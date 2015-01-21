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
#ifndef INCLUDED_SW_INC_SWCRSR_HXX
#define INCLUDED_SW_INC_SWCRSR_HXX

#include <com/sun/star/i18n/WordType.hpp>

#include <pam.hxx>
#include <tblsel.hxx>
#include <cshtyp.hxx>

struct _SwCursor_SavePos;

namespace com { namespace sun { namespace star { namespace util {
    struct SearchOptions;
} } } }

// Base structure for parameters of the find-methods.
// Returns values of found-call.
const int FIND_NOT_FOUND    = 0;
const int FIND_FOUND        = 1;
const int FIND_NO_RING      = 2;

struct SwFindParas
{
    virtual int Find( SwPaM*, SwMoveFn, const SwPaM*, bool ) = 0;
    virtual bool IsReplaceMode() const = 0;

protected:
    ~SwFindParas() {}
};

typedef sal_uInt16 SwCursorSelOverFlags;
namespace nsSwCursorSelOverFlags
{
    const SwCursorSelOverFlags SELOVER_NONE                = 0x00;
    const SwCursorSelOverFlags SELOVER_CHECKNODESSECTION   = 0x01;
    const SwCursorSelOverFlags SELOVER_TOGGLE              = 0x02;
    const SwCursorSelOverFlags SELOVER_ENABLEREVDIREKTION  = 0x04;
    const SwCursorSelOverFlags SELOVER_CHANGEPOS           = 0x08;
}

class SW_DLLPUBLIC SwCursor : public SwPaM
{
    friend class SwCrsrSaveState;

    _SwCursor_SavePos* pSavePos;
    long mnRowSpanOffset;        // required for travelling in tabs with rowspans
    sal_uInt8 nCursorBidiLevel;       // bidi level of the cursor
    bool mbColumnSelection;      // true: cursor is aprt of a column selection

    sal_uLong FindAll( SwFindParas& , SwDocPositions, SwDocPositions, FindRanges, bool& bCancel );

    using SwPaM::Find;

protected:
    _SwCursor_SavePos* CreateNewSavePos() const;
    void SaveState();
    void RestoreState();

    const _SwCursor_SavePos* GetSavePos() const { return pSavePos; }

    virtual const SwCntntFrm* DoSetBidiLevelLeftRight(
        bool & io_rbLeft, bool bVisualAllowed, bool bInsertCrsr);
    virtual void DoSetBidiLevelUpDown();
    virtual bool IsSelOvrCheck(int eFlags);

public:
    // single argument ctors shall be explicit.
    SwCursor( const SwPosition &rPos, SwPaM* pRing, bool bColumnSel );
    virtual ~SwCursor();

    // @@@ semantic: no copy ctor.
    SwCursor( SwCursor& rCpy);

public:

    virtual SwCursor* Create( SwPaM* pRing = 0 ) const;

    virtual short MaxReplaceArived(); //returns RET_YES/RET_CANCEL/RET_NO
    virtual void SaveTblBoxCntnt( const SwPosition* pPos = 0 );

    void FillFindPos( SwDocPositions ePos, SwPosition& rPos ) const;
    SwMoveFnCollection* MakeFindRange( SwDocPositions, SwDocPositions,
                                        SwPaM* ) const;

    sal_uLong Find( const com::sun::star::util::SearchOptions& rSearchOpt,
                bool bSearchInNotes,
                SwDocPositions nStart, SwDocPositions nEnde,
                bool& bCancel,
                FindRanges = FND_IN_BODY,
                bool bReplace = false );
    sal_uLong Find( const SwTxtFmtColl& rFmtColl,
                SwDocPositions nStart, SwDocPositions nEnde,
                bool& bCancel,
                FindRanges = FND_IN_BODY,
                const SwTxtFmtColl* pReplFmt = 0 );
    sal_uLong Find( const SfxItemSet& rSet, bool bNoCollections,
                SwDocPositions nStart, SwDocPositions nEnde,
                bool& bCancel,
                FindRanges = FND_IN_BODY,
                const com::sun::star::util::SearchOptions* pSearchOpt = 0,
                const SfxItemSet* rReplSet = 0 );

    // UI versions
    bool IsStartWord( sal_Int16 nWordType = com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES ) const;
    bool IsEndWord( sal_Int16 nWordType = com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES ) const;
    bool IsInWord( sal_Int16 nWordType = com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES ) const;
    bool IsStartEndSentence( bool bEnd ) const;
    bool GoStartWord();
    bool GoEndWord();
    bool GoNextWord();
    bool GoPrevWord();
    bool SelectWord( SwViewShell* pViewShell, const Point* pPt = 0 );

    // API versions of above functions (will be used with a different
    // WordType for the break iterator)
    bool IsStartWordWT( sal_Int16 nWordType ) const;
    bool IsEndWordWT( sal_Int16 nWordType ) const;
    bool IsInWordWT( sal_Int16 nWordType ) const;
    bool GoStartWordWT( sal_Int16 nWordType );
    bool GoEndWordWT( sal_Int16 nWordType );
    bool GoNextWordWT( sal_Int16 nWordType );
    bool GoPrevWordWT( sal_Int16 nWordType );
    bool SelectWordWT( SwViewShell* pViewShell, sal_Int16 nWordType, const Point* pPt = 0 );

    enum SentenceMoveType
    {
        NEXT_SENT,
        PREV_SENT,
        START_SENT,
        END_SENT
    };
    bool GoSentence(SentenceMoveType eMoveType);
    bool GoNextSentence(){return GoSentence(NEXT_SENT);}
    bool GoEndSentence(){return GoSentence(END_SENT);}
    bool GoPrevSentence(){return GoSentence(PREV_SENT);}
    bool GoStartSentence(){return GoSentence(START_SENT);}
    bool ExpandToSentenceBorders();

    virtual bool LeftRight( bool bLeft, sal_uInt16 nCnt, sal_uInt16 nMode,
        bool bAllowVisual, bool bSkipHidden, bool bInsertCrsr );
    bool UpDown( bool bUp, sal_uInt16 nCnt, Point* pPt, long nUpDownX );
    bool LeftRightMargin( bool bLeftMargin, bool bAPI = false );
    bool IsAtLeftRightMargin( bool bLeftMargin, bool bAPI = false ) const;
    bool SttEndDoc( bool bSttDoc );
    bool GoPrevNextCell( bool bNext, sal_uInt16 nCnt );

    bool Left( sal_uInt16 nCnt, sal_uInt16 nMode, bool bAllowVisual, bool bSkipHidden )
                                    { return LeftRight( true, nCnt, nMode, bAllowVisual, bSkipHidden, false ); }
    bool Right( sal_uInt16 nCnt, sal_uInt16 nMode, bool bAllowVisual, bool bSkipHidden )
                                    { return LeftRight( false, nCnt, nMode, bAllowVisual, bSkipHidden, false ); }
    bool GoNextCell( sal_uInt16 nCnt = 1 )  { return GoPrevNextCell( true, nCnt ); }
    bool GoPrevCell( sal_uInt16 nCnt = 1 )  { return GoPrevNextCell( false, nCnt ); }
    virtual bool GotoTable( const OUString& rName );
    bool GotoTblBox( const OUString& rName );
    bool GotoRegion( const OUString& rName );
    bool GotoFtnAnchor();
    bool GotoFtnTxt();
    bool GotoNextFtnAnchor();
    bool GotoPrevFtnAnchor();
    bool GotoNextFtnCntnt();
    bool GotoPrevFtnCntnt();

    bool MovePara( SwWhichPara, SwPosPara );
    bool MoveSection( SwWhichSection, SwPosSection );
    bool MoveTable( SwWhichTable, SwPosTable );
    bool MoveRegion( SwWhichRegion, SwPosRegion );

    // Is there a selection of content in table?
    // Return value indicates if cursor remains at its old position.
    virtual bool IsSelOvr( int eFlags =
                                ( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                  nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                                  nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ));
    bool IsInProtectTable( bool bMove = false,
                                   bool bChgCrsr = true );
    bool IsNoCntnt() const;

    /** Restore cursor state to the one saved by SwCrsrSaveState **/
    void RestoreSavePos();

    // true: cursor can be set at this position.
    virtual bool IsAtValidPos( bool bPoint = true ) const;

    // Is cursor allowed in ready only ranges?
    virtual bool IsReadOnlyAvailable() const;

    virtual bool IsSkipOverProtectSections() const;
    virtual bool IsSkipOverHiddenSections() const;

    sal_uInt8 GetCrsrBidiLevel() const { return nCursorBidiLevel; }
    void SetCrsrBidiLevel( sal_uInt8 nNewLevel ) { nCursorBidiLevel = nNewLevel; }

    bool IsColumnSelection() const { return mbColumnSelection; }
    void SetColumnSelection( bool bNew ) { mbColumnSelection = bNew; }

    long GetCrsrRowSpanOffset() const { return mnRowSpanOffset; }
    void SetCrsrRowSpanOffset( long nNew ) { mnRowSpanOffset = nNew; }

    DECL_FIXEDMEMPOOL_NEWDEL( SwCursor )
};

/**
 A helper class to save cursor state (position). Create SwCrsrSaveState
 object to save current state, use SwCursor::RestoreSavePos() to actually
 restore cursor state to the saved state (SwCrsrSaveState destructor only
 removes the saved state from an internal stack). It is possible to stack
 several SwCrsrSaveState objects.
**/
class SwCrsrSaveState
{
    SwCursor& rCrsr;
public:
    SwCrsrSaveState( SwCursor& rC ) : rCrsr( rC ) { rC.SaveState(); }
    ~SwCrsrSaveState() { rCrsr.RestoreState(); }
};

// internal, used by SwCursor::SaveState() etc.
struct _SwCursor_SavePos
{
    sal_uLong nNode;
    sal_Int32 nCntnt;
    _SwCursor_SavePos* pNext;

    _SwCursor_SavePos( const SwCursor& rCrsr )
        : nNode( rCrsr.GetPoint()->nNode.GetIndex() ),
        nCntnt( rCrsr.GetPoint()->nContent.GetIndex() ),
        pNext( 0 )
    {}
    virtual ~_SwCursor_SavePos() {}

    DECL_FIXEDMEMPOOL_NEWDEL( _SwCursor_SavePos )
};

class SwTableCursor : public virtual SwCursor
{

protected:
    sal_uLong nTblPtNd, nTblMkNd;
    sal_Int32 nTblPtCnt;
    sal_Int32 nTblMkCnt;
    SwSelBoxes m_SelectedBoxes;
    bool bChg : 1;
    bool bParked : 1;       // Table-cursor was parked.

    virtual bool IsSelOvrCheck(int eFlags) SAL_OVERRIDE;

public:
    SwTableCursor( const SwPosition &rPos, SwPaM* pRing = 0 );
    SwTableCursor( SwTableCursor& );
    virtual ~SwTableCursor();

    virtual bool LeftRight( bool bLeft, sal_uInt16 nCnt, sal_uInt16 nMode,
        bool bAllowVisual, bool bSkipHidden, bool bInsertCrsr ) SAL_OVERRIDE;
    virtual bool GotoTable( const OUString& rName ) SAL_OVERRIDE;

    void InsertBox( const SwTableBox& rTblBox );
    void DeleteBox(size_t nPos);
    size_t GetSelectedBoxesCount() const { return m_SelectedBoxes.size(); }
    const SwSelBoxes& GetSelectedBoxes() const { return m_SelectedBoxes; }

    // Creates cursor for all boxes.
    SwCursor* MakeBoxSels( SwCursor* pAktCrsr );
    // Any boxes protected?
    bool HasReadOnlyBoxSel() const;

    // Has table cursor been changed? If so, save new values immediately.
    bool IsCrsrMovedUpdt();
    // Has table cursor been changed?
    bool IsCrsrMoved() const
    {
        return  nTblMkNd != GetMark()->nNode.GetIndex() ||
                nTblPtNd != GetPoint()->nNode.GetIndex() ||
                nTblMkCnt != GetMark()->nContent.GetIndex() ||
                nTblPtCnt != GetPoint()->nContent.GetIndex();
    }

    bool IsChgd() const { return bChg; }

    // Park table cursor at start node of boxes.
    void ParkCrsr();

    bool NewTableSelection();
    void ActualizeSelection( const SwSelBoxes &rBoxes );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
