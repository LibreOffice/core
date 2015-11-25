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
    friend class SwCursorSaveState;

    _SwCursor_SavePos* m_pSavePos;
    long m_nRowSpanOffset;        // required for travelling in tabs with rowspans
    sal_uInt8 m_nCursorBidiLevel; // bidi level of the cursor
    bool m_bColumnSelection;      // true: cursor is aprt of a column selection

    sal_uLong FindAll( SwFindParas& , SwDocPositions, SwDocPositions, FindRanges, bool& bCancel );

    using SwPaM::Find;

    SwCursor(SwCursor const& rPaM) = delete;

protected:
    _SwCursor_SavePos* CreateNewSavePos() const;
    void SaveState();
    void RestoreState();

    const _SwCursor_SavePos* GetSavePos() const { return m_pSavePos; }

    virtual const SwContentFrame* DoSetBidiLevelLeftRight(
        bool & io_rbLeft, bool bVisualAllowed, bool bInsertCursor);
    virtual void DoSetBidiLevelUpDown();
    virtual bool IsSelOvrCheck(int eFlags);

public:
    // single argument ctors shall be explicit.
    SwCursor( const SwPosition &rPos, SwPaM* pRing, bool bColumnSel );
    virtual ~SwCursor();

    /// this takes a second parameter, which indicates the Ring that
    /// the new cursor should be part of (may be null)
    SwCursor(SwCursor const& rCursor, SwPaM* pRing);

public:

    virtual SwCursor* Create( SwPaM* pRing = nullptr ) const;

    virtual short MaxReplaceArived(); //returns RET_YES/RET_CANCEL/RET_NO
    virtual void SaveTableBoxContent( const SwPosition* pPos = nullptr );

    void FillFindPos( SwDocPositions ePos, SwPosition& rPos ) const;
    SwMoveFnCollection* MakeFindRange( SwDocPositions, SwDocPositions,
                                        SwPaM* ) const;

    sal_uLong Find( const css::util::SearchOptions& rSearchOpt,
                bool bSearchInNotes,
                SwDocPositions nStart, SwDocPositions nEnde,
                bool& bCancel,
                FindRanges = FND_IN_BODY,
                bool bReplace = false );
    sal_uLong Find( const SwTextFormatColl& rFormatColl,
                SwDocPositions nStart, SwDocPositions nEnde,
                bool& bCancel,
                FindRanges = FND_IN_BODY,
                const SwTextFormatColl* pReplFormat = nullptr );
    sal_uLong Find( const SfxItemSet& rSet, bool bNoCollections,
                SwDocPositions nStart, SwDocPositions nEnde,
                bool& bCancel,
                FindRanges = FND_IN_BODY,
                const css::util::SearchOptions* pSearchOpt = nullptr,
                const SfxItemSet* rReplSet = nullptr );

    // UI versions
    bool IsStartWord( sal_Int16 nWordType = css::i18n::WordType::ANYWORD_IGNOREWHITESPACES ) const;
    bool IsEndWord( sal_Int16 nWordType = css::i18n::WordType::ANYWORD_IGNOREWHITESPACES ) const;
    bool IsInWord( sal_Int16 nWordType = css::i18n::WordType::ANYWORD_IGNOREWHITESPACES ) const;
    bool IsStartEndSentence( bool bEnd ) const;
    bool GoStartWord();
    bool GoEndWord();
    bool GoNextWord();
    bool GoPrevWord();
    bool SelectWord( SwViewShell* pViewShell, const Point* pPt = nullptr );

    // API versions of above functions (will be used with a different
    // WordType for the break iterator)
    bool IsStartWordWT( sal_Int16 nWordType ) const;
    bool IsEndWordWT( sal_Int16 nWordType ) const;
    bool IsInWordWT( sal_Int16 nWordType ) const;
    bool GoStartWordWT( sal_Int16 nWordType );
    bool GoEndWordWT( sal_Int16 nWordType );
    bool GoNextWordWT( sal_Int16 nWordType );
    bool GoPrevWordWT( sal_Int16 nWordType );
    bool SelectWordWT( SwViewShell* pViewShell, sal_Int16 nWordType, const Point* pPt = nullptr );

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
    bool GoStartSentence(){return GoSentence(START_SENT);}
    bool ExpandToSentenceBorders();

    virtual bool LeftRight( bool bLeft, sal_uInt16 nCnt, sal_uInt16 nMode,
        bool bAllowVisual, bool bSkipHidden, bool bInsertCursor );
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
    bool GotoTableBox( const OUString& rName );
    bool GotoRegion( const OUString& rName );
    bool GotoFootnoteAnchor();
    bool GotoFootnoteText();
    bool GotoNextFootnoteAnchor();
    bool GotoPrevFootnoteAnchor();

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
                                   bool bChgCursor = true );
    bool IsNoContent() const;

    /** Restore cursor state to the one saved by SwCursorSaveState **/
    void RestoreSavePos();

    // true: cursor can be set at this position.
    virtual bool IsAtValidPos( bool bPoint = true ) const;

    // Is cursor allowed in ready only ranges?
    virtual bool IsReadOnlyAvailable() const;

    virtual bool IsSkipOverProtectSections() const;
    virtual bool IsSkipOverHiddenSections() const;

    sal_uInt8 GetCursorBidiLevel() const { return m_nCursorBidiLevel; }
    void SetCursorBidiLevel( sal_uInt8 nNewLevel ) { m_nCursorBidiLevel = nNewLevel; }

    bool IsColumnSelection() const { return m_bColumnSelection; }
    void SetColumnSelection( bool bNew ) { m_bColumnSelection = bNew; }

    long GetCursorRowSpanOffset() const { return m_nRowSpanOffset; }

    DECL_FIXEDMEMPOOL_NEWDEL( SwCursor )
};

/**
 A helper class to save cursor state (position). Create SwCursorSaveState
 object to save current state, use SwCursor::RestoreSavePos() to actually
 restore cursor state to the saved state (SwCursorSaveState destructor only
 removes the saved state from an internal stack). It is possible to stack
 several SwCursorSaveState objects.
**/
class SwCursorSaveState
{
private:
    SwCursor& m_rCursor;
public:
    SwCursorSaveState( SwCursor& rC ) : m_rCursor( rC ) { rC.SaveState(); }
    ~SwCursorSaveState() { m_rCursor.RestoreState(); }
};

// internal, used by SwCursor::SaveState() etc.
struct _SwCursor_SavePos
{
    sal_uLong nNode;
    sal_Int32 nContent;
    _SwCursor_SavePos* pNext;

    _SwCursor_SavePos( const SwCursor& rCursor )
        : nNode( rCursor.GetPoint()->nNode.GetIndex() ),
        nContent( rCursor.GetPoint()->nContent.GetIndex() ),
        pNext( nullptr )
    {}
    virtual ~_SwCursor_SavePos() {}

    DECL_FIXEDMEMPOOL_NEWDEL( _SwCursor_SavePos )
};

class SwTableCursor : public virtual SwCursor
{

protected:
    sal_uLong m_nTablePtNd;
    sal_uLong m_nTableMkNd;
    sal_Int32 m_nTablePtCnt;
    sal_Int32 m_nTableMkCnt;
    SwSelBoxes m_SelectedBoxes;
    bool m_bChanged : 1;
    bool m_bParked : 1;       // Table-cursor was parked.

    virtual bool IsSelOvrCheck(int eFlags) override;

public:
    SwTableCursor( const SwPosition &rPos, SwPaM* pRing = nullptr );
    SwTableCursor( SwTableCursor& );
    virtual ~SwTableCursor();

    virtual bool LeftRight( bool bLeft, sal_uInt16 nCnt, sal_uInt16 nMode,
        bool bAllowVisual, bool bSkipHidden, bool bInsertCursor ) override;
    virtual bool GotoTable( const OUString& rName ) override;

    void InsertBox( const SwTableBox& rTableBox );
    void DeleteBox(size_t nPos);
    size_t GetSelectedBoxesCount() const { return m_SelectedBoxes.size(); }
    const SwSelBoxes& GetSelectedBoxes() const { return m_SelectedBoxes; }

    // Creates cursor for all boxes.
    SwCursor* MakeBoxSels( SwCursor* pAktCursor );
    // Any boxes protected?
    bool HasReadOnlyBoxSel() const;

    // Has table cursor been changed? If so, save new values immediately.
    bool IsCursorMovedUpdate();
    // Has table cursor been changed?
    bool IsCursorMoved() const
    {
        return  m_nTableMkNd != GetMark()->nNode.GetIndex() ||
                m_nTablePtNd != GetPoint()->nNode.GetIndex() ||
                m_nTableMkCnt != GetMark()->nContent.GetIndex() ||
                m_nTablePtCnt != GetPoint()->nContent.GetIndex();
    }

    bool IsChgd() const { return m_bChanged; }

    // Park table cursor at start node of boxes.
    void ParkCursor();

    bool NewTableSelection();
    void ActualizeSelection( const SwSelBoxes &rBoxes );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
