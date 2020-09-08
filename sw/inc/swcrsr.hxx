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

#include "pam.hxx"
#include "tblsel.hxx"
#include "cshtyp.hxx"

class SfxItemSet;
struct SwCursor_SavePos;
class SvxSearchItem;
namespace i18nutil {
    struct SearchOptions2;
}

// Base structure for parameters of the find-methods.
// Returns values of found-call.
const int FIND_NOT_FOUND    = 0;
const int FIND_FOUND        = 1;
const int FIND_NO_RING      = 2;

struct SwFindParas
{
    // @param xSearchItem allocate in parent so we can do so outside the calling loop
    virtual int DoFind(SwPaM &, SwMoveFnCollection const &, const SwPaM&, bool, std::unique_ptr<SvxSearchItem>& xSearchItem) = 0;
    virtual bool IsReplaceMode() const = 0;

protected:
    ~SwFindParas() {}
};

enum class SwCursorSelOverFlags : sal_uInt16
{
    NONE                = 0x00,
    CheckNodeSection    = 0x01,
    Toggle              = 0x02,
    EnableRevDirection  = 0x04,
    ChangePos           = 0x08
};
namespace o3tl {
    template<> struct typed_flags<SwCursorSelOverFlags> : is_typed_flags<SwCursorSelOverFlags, 0x0f> {};
}

// define for cursor travelling normally in western text cells and chars do
// the same, but in complex text cell skip over ligatures and char skip
// into it.
// These defines exist only to cut off the dependencies to I18N project.
const sal_uInt16 CRSR_SKIP_CHARS  = 0;
const sal_uInt16 CRSR_SKIP_CELLS  = 1;
const sal_uInt16 CRSR_SKIP_HIDDEN = 2;


class SW_DLLPUBLIC SwCursor : public SwPaM
{
    friend class SwCursorSaveState;

    std::vector<SwCursor_SavePos> m_vSavePos; // the current entry is the last element
    long m_nRowSpanOffset;        // required for travelling in tabs with rowspans
    sal_uInt8 m_nCursorBidiLevel; // bidi level of the cursor
    bool m_bColumnSelection;      // true: cursor is aprt of a column selection

    sal_uLong FindAll( SwFindParas& , SwDocPositions, SwDocPositions, FindRanges, bool& bCancel );

    SwCursor(SwCursor const& rPaM) = delete;

protected:
    void SaveState();
    void RestoreState();

    const SwCursor_SavePos* GetSavePos() const { return m_vSavePos.empty() ? nullptr : &m_vSavePos.back(); }

    virtual const SwContentFrame* DoSetBidiLevelLeftRight(
        bool & io_rbLeft, bool bVisualAllowed, bool bInsertCursor);
    virtual void DoSetBidiLevelUpDown();
    virtual bool IsSelOvrCheck(SwCursorSelOverFlags eFlags);

public:
    // single argument ctors shall be explicit.
    SwCursor( const SwPosition &rPos, SwPaM* pRing );
    virtual ~SwCursor() override;

    SwCursor & operator =(SwCursor const &) = default;

    /// this takes a second parameter, which indicates the Ring that
    /// the new cursor should be part of (may be null)
    SwCursor(SwCursor const& rCursor, SwPaM* pRing);

public:

    virtual SwCursor* Create( SwPaM* pRing = nullptr ) const;

    virtual short MaxReplaceArived(); //returns RET_YES/RET_CANCEL/RET_NO
    virtual void SaveTableBoxContent( const SwPosition* pPos );

    void FillFindPos( SwDocPositions ePos, SwPosition& rPos ) const;
    SwMoveFnCollection const & MakeFindRange( SwDocPositions, SwDocPositions,
                                        SwPaM* ) const;

    // note: DO NOT call it FindText because windows.h
    sal_uLong Find_Text( const i18nutil::SearchOptions2& rSearchOpt,
                bool bSearchInNotes,
                SwDocPositions nStart, SwDocPositions nEnd,
                bool& bCancel,
                FindRanges,
                bool bReplace = false,
                SwRootFrame const*const pLayout = nullptr);
    sal_uLong FindFormat( const SwTextFormatColl& rFormatColl,
                SwDocPositions nStart, SwDocPositions nEnd,
                bool& bCancel,
                FindRanges,
                const SwTextFormatColl* pReplFormat,
                SwRootFrame const*const pLayout = nullptr);
    sal_uLong FindAttrs( const SfxItemSet& rSet, bool bNoCollections,
                SwDocPositions nStart, SwDocPositions nEnd,
                bool& bCancel,
                FindRanges,
                const i18nutil::SearchOptions2* pSearchOpt,
                const SfxItemSet* rReplSet = nullptr,
                SwRootFrame const*const pLayout = nullptr);

    // UI versions
    bool IsStartEndSentence(bool bEnd, SwRootFrame const* pLayout) const;
    bool SelectWord( SwViewShell const * pViewShell, const Point* pPt );

    // API versions of above functions (will be used with a different
    // WordType for the break iterator)
    bool IsStartWordWT(sal_Int16 nWordType, SwRootFrame const* pLayout = nullptr) const;
    bool IsEndWordWT(sal_Int16 nWordType, SwRootFrame const* pLayout = nullptr) const;
    bool IsInWordWT(sal_Int16 nWordType, SwRootFrame const* pLayout = nullptr) const;
    bool GoStartWordWT(sal_Int16 nWordType, SwRootFrame const* pLayout = nullptr);
    bool GoEndWordWT(sal_Int16 nWordType, SwRootFrame const* pLayout = nullptr);
    bool GoNextWordWT(sal_Int16 nWordType, SwRootFrame const* pLayout = nullptr);
    bool GoPrevWordWT(sal_Int16 nWordType, SwRootFrame const* pLayout = nullptr);
    bool SelectWordWT( SwViewShell const * pViewShell, sal_Int16 nWordType, const Point* pPt );

    enum SentenceMoveType
    {
        NEXT_SENT,
        PREV_SENT,
        START_SENT,
        END_SENT
    };
    bool GoSentence(SentenceMoveType eMoveType, SwRootFrame const*pLayout = nullptr);
    bool ExpandToSentenceBorders(SwRootFrame const* pLayout);

    virtual bool LeftRight( bool bLeft, sal_uInt16 nCnt, sal_uInt16 nMode,
        bool bAllowVisual, bool bSkipHidden, bool bInsertCursor,
        SwRootFrame const* pLayout);
    bool UpDown(bool bUp, sal_uInt16 nCnt, Point const * pPt, long nUpDownX, SwRootFrame & rLayout);
    bool LeftRightMargin(SwRootFrame const& rLayout, bool bLeftMargin, bool bAPI);
    bool IsAtLeftRightMargin(SwRootFrame const& rLayout, bool bLeftMargin, bool bAPI) const;
    bool SttEndDoc( bool bSttDoc );
    bool GoPrevNextCell( bool bNext, sal_uInt16 nCnt );

    bool Left( sal_uInt16 nCnt )   { return LeftRight( true, nCnt, CRSR_SKIP_CHARS, false/*bAllowVisual*/, false/*bSkipHidden*/, false, nullptr ); }
    bool Right( sal_uInt16 nCnt )  { return LeftRight( false, nCnt, CRSR_SKIP_CHARS, false/*bAllowVisual*/, false/*bSkipHidden*/, false, nullptr ); }
    bool GoNextCell( sal_uInt16 nCnt = 1 )  { return GoPrevNextCell( true, nCnt ); }
    bool GoPrevCell( sal_uInt16 nCnt = 1 )  { return GoPrevNextCell( false, nCnt ); }
    virtual bool GotoTable( const OUString& rName );
    bool GotoTableBox( const OUString& rName );
    bool GotoRegion( const OUString& rName );
    bool GotoFootnoteAnchor();
    bool GotoFootnoteText();
    bool GotoNextFootnoteAnchor();
    bool GotoPrevFootnoteAnchor();

    bool MovePara( SwWhichPara, SwMoveFnCollection const & );
    bool MoveSection( SwWhichSection, SwMoveFnCollection const & );
    bool MoveTable( SwWhichTable, SwMoveFnCollection const & );
    bool MoveRegion( SwWhichRegion, SwMoveFnCollection const & );

    // Is there a selection of content in table?
    // Return value indicates if cursor remains at its old position.
    virtual bool IsSelOvr( SwCursorSelOverFlags eFlags =
                                SwCursorSelOverFlags::CheckNodeSection |
                                SwCursorSelOverFlags::Toggle |
                                SwCursorSelOverFlags::ChangePos );
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

    SwCursor* GetNext()             { return dynamic_cast<SwCursor *>(GetNextInRing()); }
    const SwCursor* GetNext() const { return dynamic_cast<SwCursor const *>(GetNextInRing()); }
    SwCursor* GetPrev()             { return dynamic_cast<SwCursor *>(GetPrevInRing()); }
    const SwCursor* GetPrev() const { return dynamic_cast<SwCursor const *>(GetPrevInRing()); }
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
struct SwCursor_SavePos final
{
    sal_uLong nNode;
    sal_Int32 nContent;

    SwCursor_SavePos( const SwCursor& rCursor )
        : nNode( rCursor.GetPoint()->nNode.GetIndex() ),
          nContent( rCursor.GetPoint()->nContent.GetIndex() )
    {}
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

    virtual bool IsSelOvrCheck(SwCursorSelOverFlags eFlags) override;

public:
    SwTableCursor( const SwPosition &rPos );
    SwTableCursor( SwTableCursor& );
    virtual ~SwTableCursor() override;

    virtual bool LeftRight( bool bLeft, sal_uInt16 nCnt, sal_uInt16 nMode,
        bool bAllowVisual, bool bSkipHidden, bool bInsertCursor,
        SwRootFrame const*) override;
    virtual bool GotoTable( const OUString& rName ) override;

    void InsertBox( const SwTableBox& rTableBox );
    void DeleteBox(size_t nPos);
    size_t GetSelectedBoxesCount() const { return m_SelectedBoxes.size(); }
    const SwSelBoxes& GetSelectedBoxes() const { return m_SelectedBoxes; }

    // Creates cursor for all boxes.
    SwCursor* MakeBoxSels( SwCursor* pCurrentCursor );
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
    void SetChgd() { m_bChanged = true; }

    // Park table cursor at start node of boxes.
    void ParkCursor();

    bool NewTableSelection();
    void ActualizeSelection( const SwSelBoxes &rBoxes );

    SwTableCursor* GetNext()             { return dynamic_cast<SwTableCursor *>(GetNextInRing()); }
    const SwTableCursor* GetNext() const { return dynamic_cast<SwTableCursor const *>(GetNextInRing()); }
    SwTableCursor* GetPrev()             { return dynamic_cast<SwTableCursor *>(GetPrevInRing()); }
    const SwTableCursor* GetPrev() const { return dynamic_cast<SwTableCursor const *>(GetPrevInRing()); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
