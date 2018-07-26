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
#ifndef INCLUDED_SC_SOURCE_UI_INC_VIEWDATA_HXX
#define INCLUDED_SC_SOURCE_UI_INC_VIEWDATA_HXX

#include <sfx2/zoomitem.hxx>
#include <rangelst.hxx>
#include <scdllapi.h>
#include <viewopti.hxx>
#include "docsh.hxx"

#include <memory>
#include <o3tl/typed_flags_set.hxx>

#define SC_SIZE_NONE        65535
const SCCOL SC_TABSTART_NONE = SCCOL_MAX;

enum class ScFillMode
{
    NONE        = 0,
    FILL        = 1,
    EMBED_LT    = 2,
    EMBED_RB    = 3,
    MATRIX      = 4,
};

enum ScSplitMode { SC_SPLIT_NONE = 0, SC_SPLIT_NORMAL, SC_SPLIT_FIX, SC_SPLIT_MODE_MAX_ENUM = SC_SPLIT_FIX };

enum ScSplitPos { SC_SPLIT_TOPLEFT, SC_SPLIT_TOPRIGHT, SC_SPLIT_BOTTOMLEFT, SC_SPLIT_BOTTOMRIGHT, SC_SPLIT_POS_MAX_ENUM = SC_SPLIT_BOTTOMRIGHT };
enum ScHSplitPos { SC_SPLIT_LEFT, SC_SPLIT_RIGHT };
enum ScVSplitPos { SC_SPLIT_TOP, SC_SPLIT_BOTTOM };

inline ScHSplitPos WhichH( ScSplitPos ePos );
inline ScVSplitPos WhichV( ScSplitPos ePos );

/**  Screen behavior related to cursor movements */
enum ScFollowMode { SC_FOLLOW_NONE, SC_FOLLOW_LINE, SC_FOLLOW_FIX, SC_FOLLOW_JUMP };

/** Mouse mode to select areas */
enum ScRefType { SC_REFTYPE_NONE, SC_REFTYPE_REF, SC_REFTYPE_FILL,
                    SC_REFTYPE_EMBED_LT, SC_REFTYPE_EMBED_RB };

/** States GetSimpleArea() returns for the underlying selection marks, so the
    caller can react if the result is not of type SC_MARK_SIMPLE. */
enum ScMarkType
{
    SC_MARK_NONE            = 0,    // Not returned by GetSimpleArea(), used internally.
                                    // Nothing marked always results in the
                                    // current cursor position being selected and a simple mark.
    SC_MARK_SIMPLE          = 1,    // Simple rectangular area marked, no filtered rows.
    SC_MARK_FILTERED        = 2,    // At least one mark contains filtered rows.
    SC_MARK_SIMPLE_FILTERED =       // Simple rectangular area marked containing filtered rows.
        SC_MARK_SIMPLE |
        SC_MARK_FILTERED,  // 3
    SC_MARK_MULTI           = 4     // Multiple selection marks.
    /* TODO: if filtered multi-selection was implemented, this would be the value to use. */
#if 0
        ,
    SC_MARK_MULTI_FILTERED  =       // Multiple selection marks containing filtered rows.
        SC_MARK_MULTI |
        SC_MARK_FILTERED   // 6
#endif
};

enum class ScPasteFlags
{
    NONE   = 0,    // No flags specified
    Mode   = 1,    // Enable paste-mode
    Border = 2,    // Show a border around the source cells
};
namespace o3tl {
    template<> struct typed_flags<ScPasteFlags> : is_typed_flags<ScPasteFlags, 0x03> {};
}

// for internal Drag&Drop:
enum class ScDragSrc{
    Undefined = 0,
    Navigator = 1,
    Table     = 2
};
namespace o3tl {
    template<> struct typed_flags<ScDragSrc> : is_typed_flags<ScDragSrc, 0x00000003> {};
}

class ScDocFunc;
class ScDocShell;
class ScDocument;
class ScDBFunc;
class ScTabViewShell;
class ScDrawView;
class ScEditEngineDefaulter;
class EditView;
class EditStatus;
class Outliner;
namespace vcl { class Window; }
class SfxObjectShell;
class SfxBindings;
class SfxDispatcher;
class ScPatternAttr;
class ScExtDocOptions;
class ScViewData;
class ScMarkData;
class ScGridWindow;

class ScPositionHelper
{
public:
    typedef SCCOLROW index_type;
    typedef std::pair<index_type, long> value_type;
    static_assert(std::numeric_limits<index_type>::is_signed, "ScPositionCache: index type is not signed");

private:
    static const index_type null = std::numeric_limits<index_type>::min();

    class Comp
    {
    public:
        bool operator() (const value_type& rValue1, const value_type& rValue2) const;
    };

    const index_type MAX_INDEX;
    std::set<value_type, Comp> mData;

public:
    ScPositionHelper(bool bColumn);

    void insert(index_type nIndex, long nPos);
    void removeByIndex(index_type nIndex);
    void invalidateByIndex(index_type nIndex);
    void invalidateByPosition(long nPos);
    const value_type& getNearestByIndex(index_type nIndex) const;
    const value_type& getNearestByPosition(long nPos) const;
    long getPosition(index_type nIndex) const;
    long computePosition(index_type nIndex, const std::function<long (index_type)>& getSizePx);
};

class ScBoundsProvider
{
    typedef ScPositionHelper::value_type value_type;
    typedef SCCOLROW index_type;

    ScDocument* pDoc;
    const SCTAB nTab;
    const bool bColumnHeader;
    const index_type MAX_INDEX;

    index_type nFirstIndex;
    index_type nSecondIndex;
    long nFirstPositionPx;
    long nSecondPositionPx;

public:
    ScBoundsProvider(ScDocument* pD, SCTAB nT, bool bColumnHeader);

    void GetStartIndexAndPosition(SCCOL& nIndex, long& nPosition) const;
    void GetEndIndexAndPosition(SCCOL& nIndex, long& nPosition) const;
    void GetStartIndexAndPosition(SCROW& nIndex, long& nPosition) const;
    void GetEndIndexAndPosition(SCROW& nIndex, long& nPosition) const;

    void Compute(value_type aFirstNearest, value_type aSecondNearest,
                 long nFirstBound, long nSecondBound);

    void EnlargeStartBy(long nOffset);

    void EnlargeEndBy(long nOffset);

    void EnlargeBy(long nOffset)
    {
        EnlargeStartBy(nOffset);
        EnlargeEndBy(nOffset);
    }

private:
    long GetSize(index_type nIndex) const;

    void GetIndexAndPos(index_type nNearestIndex, long nNearestPosition,
                        long nBound, index_type& nFoundIndex, long& nPosition,
                        bool bTowards, long nDiff);

    void GeIndexBackwards(index_type nNearestIndex, long nNearestPosition,
                          long nBound, index_type& nFoundIndex, long& nPosition,
                          bool bTowards);

    void GetIndexTowards(index_type nNearestIndex, long nNearestPosition,
                         long nBound, index_type& nFoundIndex, long& nPosition,
                         bool bTowards);
};

class ScViewDataTable                           // per-sheet data
{
friend class ScViewData;
private:
    SvxZoomType     eZoomType;                  // selected zoom type (normal view)
    Fraction        aZoomX;                     // selected zoom X
    Fraction        aZoomY;                     // selected zoom Y (displayed)
    Fraction        aPageZoomX;                 // zoom in page break preview mode
    Fraction        aPageZoomY;

    long            nTPosX[2];                  // MapMode - Offset (Twips)
    long            nTPosY[2];
    long            nMPosX[2];                  // MapMode - Offset (1/100 mm)
    long            nMPosY[2];
    long            nPixPosX[2];                // Offset in Pixels
    long            nPixPosY[2];
    long            nHSplitPos;
    long            nVSplitPos;

    ScSplitMode     eHSplitMode;
    ScSplitMode     eVSplitMode;
    ScSplitPos      eWhichActive;

    SCCOL           nFixPosX;                   // Cell position of the splitter when freeze pane
    SCROW           nFixPosY;

    SCCOL           nCurX;
    SCROW           nCurY;
    SCCOL           nOldCurX;
    SCROW           nOldCurY;
    SCCOL           nLOKOldCurX;
    SCROW           nLOKOldCurY;

    ScPositionHelper aWidthHelper;
    ScPositionHelper aHeightHelper;

    SCCOL           nPosX[2];                   ///< X position of the top left cell of the visible area.
    SCROW           nPosY[2];                   ///< Y position of the top left cell of the visible area.
    SCCOL           nMaxTiledCol;
    SCROW           nMaxTiledRow;

    bool            bShowGrid;                  // per sheet show grid lines option.
    bool            mbOldCursorValid;           // "virtual" Cursor position when combined
                    ScViewDataTable();

    void            WriteUserDataSequence(
                        css::uno::Sequence <css::beans::PropertyValue>& rSettings,
                        const ScViewData& rViewData ) const;

    void            ReadUserDataSequence(
                        const css::uno::Sequence <css::beans::PropertyValue>& rSettings,
                        ScViewData& rViewData, SCTAB nTab, bool& rHasZoom);

    /** Sanitize the active split range value to not point into a grid window
        that would never be initialized due to non-matching split modes.

        This is to be done when reading settings from file formats or
        configurations that could have arbitrary values. The caller is
        responsible for actually assigning the new value to eWhichActive because
        we want this function to be const to be able to call the check from
        anywhere.
     */
    SAL_WARN_UNUSED_RESULT ScSplitPos SanitizeWhichActive() const;
};

class SC_DLLPUBLIC ScViewData
{
private:
    double              nPPTX, nPPTY;               // Scaling factors

    ::std::vector<std::unique_ptr<ScViewDataTable>> maTabData;
    std::unique_ptr<ScMarkData> mpMarkData;
    ScViewDataTable*    pThisTab;                   // Data of the displayed sheet
    ScDocShell*         pDocShell;
    ScDocument*         pDoc;
    ScDBFunc*           pView;
    ScTabViewShell*     pViewShell;
    std::unique_ptr<EditView> pEditView[4];               // Belongs to the window
    std::unique_ptr<ScViewOptions> pOptions;
    EditView*           pSpellingView;

    Size                aScenButSize;

    Size                aScrSize;
    MapMode             aLogicMode;                 // skalierter 1/100mm-MapMode

    SvxZoomType         eDefZoomType;               // default zoom and type for missing TabData
    Fraction            aDefZoomX;
    Fraction            aDefZoomY;
    Fraction            aDefPageZoomX;              // zoom in page break preview mode
    Fraction            aDefPageZoomY;

    ScRefType           eRefType;

    SCTAB               nTabNo;                     // displayed sheet
    SCTAB               nRefTabNo;                  // sheet which contains RefInput
    SCCOL               nRefStartX;
    SCROW               nRefStartY;
    SCTAB               nRefStartZ;
    SCCOL               nRefEndX;
    SCROW               nRefEndY;
    SCTAB               nRefEndZ;
    SCCOL               nFillStartX;                // Fill Cursor
    SCROW               nFillStartY;
    SCCOL               nFillEndX;
    SCROW               nFillEndY;
    SCCOL               nEditCol;                   // Related position
    SCROW               nEditRow;
    SCCOL               nEditStartCol;
    SCCOL               nEditEndCol;                // End of Edit View
    SCROW               nEditEndRow;
    SCCOL               nTabStartCol;               // for Enter after Tab
    ScRange             aDelRange;                  // for delete AutoFill

    ScPasteFlags        nPasteFlags;

    ScSplitPos          eEditActivePart;            // the part that was active when edit mode was started
    ScFillMode          nFillMode;
    SvxAdjust           eEditAdjust;
    bool                bEditActive[4];             // Active?
    bool                bActive:1;                  // Active Window ?
    bool                bIsRefMode:1;               // Reference input
    bool                bDelMarkValid:1;            // Only valid at SC_REFTYPE_FILL
    bool                bPagebreak:1;               // Page break preview mode
    bool                bSelCtrlMouseClick:1;       // special selection handling for ctrl-mouse-click
    bool                bMoveArea:1;

    bool                bGrowing;

    long                m_nLOKPageUpDownOffset;

    DECL_DLLPRIVATE_LINK( EditEngineHdl, EditStatus&, void );


    SAL_DLLPRIVATE void          CalcPPT();
    SAL_DLLPRIVATE void          CreateTabData( SCTAB nNewTab );
    SAL_DLLPRIVATE void          CreateTabData( std::vector< SCTAB >& rvTabs );
    SAL_DLLPRIVATE void          CreateSelectedTabData();
    SAL_DLLPRIVATE void          EnsureTabDataSize(size_t nSize);
    SAL_DLLPRIVATE void          UpdateCurrentTab();

public:
                    ScViewData( ScDocShell* pDocSh, ScTabViewShell* pViewSh );
                    ~ScViewData() COVERITY_NOEXCEPT_FALSE;

    void            InitData( ScDocument* pDocument );

    ScDocShell*     GetDocShell() const     { return pDocShell; }
    ScDocFunc&      GetDocFunc() const;
    ScDBFunc*       GetView() const         { return pView; }
    ScTabViewShell* GetViewShell() const    { return pViewShell; }
    SfxObjectShell* GetSfxDocShell() const  { return pDocShell; }
    SfxBindings&    GetBindings();          // from ViewShell's ViewFrame
    SfxDispatcher&  GetDispatcher();        // from ViewShell's ViewFrame

    ScMarkData&     GetMarkData();
    const ScMarkData& GetMarkData() const;

    vcl::Window*         GetDialogParent();          // forwarded from tabvwsh
    ScGridWindow*   GetActiveWin();             // from View
    const ScGridWindow* GetActiveWin() const;
    ScDrawView*     GetScDrawView();            // from View
    bool            IsMinimized();              // from View

    void            UpdateInputHandler( bool bForce = false );

    void            WriteUserData(OUString& rData);
    void            ReadUserData(const OUString& rData);
    void            WriteExtOptions( ScExtDocOptions& rOpt ) const;
    void            ReadExtOptions( const ScExtDocOptions& rOpt );
    void            WriteUserDataSequence(css::uno::Sequence <css::beans::PropertyValue>& rSettings) const;
    void            ReadUserDataSequence(const css::uno::Sequence <css::beans::PropertyValue>& rSettings);

    ScDocument*     GetDocument() const;

    bool            IsActive() const            { return bActive; }
    void            Activate(bool bActivate)    { bActive = bActivate; }

    void            InsertTab( SCTAB nTab );
    void            InsertTabs( SCTAB nTab, SCTAB nNewSheets );
    void            DeleteTab( SCTAB nTab );
    void            DeleteTabs( SCTAB nTab, SCTAB nSheets );
    void            CopyTab( SCTAB nSrcTab, SCTAB nDestTab );
    void            MoveTab( SCTAB nSrcTab, SCTAB nDestTab );

    SCTAB           GetRefTabNo() const                     { return nRefTabNo; }
    void            SetRefTabNo( SCTAB nNewTab )            { nRefTabNo = nNewTab; }

    SCTAB           GetTabNo() const                        { return nTabNo; }
    ScSplitPos      GetActivePart() const                   { return pThisTab->eWhichActive; }
    SCCOL           GetPosX( ScHSplitPos eWhich ) const     { return pThisTab->nPosX[eWhich]; }
    SCROW           GetPosY( ScVSplitPos eWhich ) const     { return pThisTab->nPosY[eWhich]; }
    SCCOL           GetCurX() const                         { return pThisTab->nCurX; }
    SCROW           GetCurY() const                         { return pThisTab->nCurY; }
    SCCOL           GetCurXForTab( SCTAB nTabIndex ) const;
    SCROW           GetCurYForTab( SCTAB nTabIndex ) const;
    SCCOL           GetOldCurX() const;
    SCROW           GetOldCurY() const;
    SCCOL           GetLOKOldCurX() const                   { return pThisTab->nLOKOldCurX; }
    SCROW           GetLOKOldCurY() const                   { return pThisTab->nLOKOldCurY; }
    long            GetLOKDocWidthPixel() const             { return pThisTab->aWidthHelper.getPosition(pThisTab->nMaxTiledCol); }
    long            GetLOKDocHeightPixel() const            { return pThisTab->aHeightHelper.getPosition(pThisTab->nMaxTiledRow); }

    ScPositionHelper& GetLOKWidthHelper()                   { return pThisTab->aWidthHelper; }
    ScPositionHelper& GetLOKHeightHelper()                  { return pThisTab->aHeightHelper; }

    ScPositionHelper* GetLOKWidthHelper(SCTAB nTabIndex);
    ScPositionHelper* GetLOKHeightHelper(SCTAB nTabIndex);

    ScSplitMode     GetHSplitMode() const                   { return pThisTab->eHSplitMode; }
    ScSplitMode     GetVSplitMode() const                   { return pThisTab->eVSplitMode; }
    long            GetHSplitPos() const                    { return pThisTab->nHSplitPos; }
    long            GetVSplitPos() const                    { return pThisTab->nVSplitPos; }
    SCCOL           GetFixPosX() const                      { return pThisTab->nFixPosX; }
    SCROW           GetFixPosY() const                      { return pThisTab->nFixPosY; }
    SCCOL           GetMaxTiledCol() const                  { return pThisTab->nMaxTiledCol; }
    SCROW           GetMaxTiledRow() const                  { return pThisTab->nMaxTiledRow; }

    bool            IsPagebreakMode() const                 { return bPagebreak; }
    bool            IsPasteMode() const                     { return bool(nPasteFlags & ScPasteFlags::Mode); }
    bool            ShowPasteSource() const                 { return bool(nPasteFlags & ScPasteFlags::Border); }

    void            SetPosX( ScHSplitPos eWhich, SCCOL nNewPosX );
    void            SetPosY( ScVSplitPos eWhich, SCROW nNewPosY );
    void            SetCurX( SCCOL nNewCurX )                       { pThisTab->nCurX = nNewCurX; }
    void            SetCurY( SCROW nNewCurY )                       { pThisTab->nCurY = nNewCurY; }
    void            SetCurXForTab( SCCOL nNewCurX, SCTAB nTabIndex );
    void            SetCurYForTab( SCCOL nNewCurY, SCTAB nTabIndex );
    void            SetOldCursor( SCCOL nNewX, SCROW nNewY );
    void            ResetOldCursor();
    void            SetLOKOldCurX( SCCOL nCurX )                    { pThisTab->nLOKOldCurX = nCurX; }
    void            SetLOKOldCurY( SCROW nCurY )                    { pThisTab->nLOKOldCurY = nCurY; }

    void            SetHSplitMode( ScSplitMode eMode )              { pThisTab->eHSplitMode = eMode; }
    void            SetVSplitMode( ScSplitMode eMode )              { pThisTab->eVSplitMode = eMode; }
    void            SetHSplitPos( long nPos )                       { pThisTab->nHSplitPos = nPos; }
    void            SetVSplitPos( long nPos )                       { pThisTab->nVSplitPos = nPos; }
    void            SetFixPosX( SCCOL nPos )                        { pThisTab->nFixPosX = nPos; }
    void            SetFixPosY( SCROW nPos )                        { pThisTab->nFixPosY = nPos; }
    void            SetMaxTiledCol( SCCOL nCol );
    void            SetMaxTiledRow( SCROW nRow );

    void            SetPagebreakMode( bool bSet );
    void            SetPasteMode ( ScPasteFlags nFlags )            { nPasteFlags = nFlags; }

    void            SetZoomType( SvxZoomType eNew, bool bAll );
    void            SetZoomType( SvxZoomType eNew, std::vector< SCTAB >& tabs );
    void            SetZoom( const Fraction& rNewX, const Fraction& rNewY, std::vector< SCTAB >& tabs );
    void            SetZoom( const Fraction& rNewX, const Fraction& rNewY, bool bAll );
    void            RefreshZoom();

    void            SetSelCtrlMouseClick( bool bTmp ) { bSelCtrlMouseClick = bTmp; }

    SvxZoomType     GetZoomType() const     { return pThisTab->eZoomType; }
    const Fraction& GetZoomX() const        { return bPagebreak ? pThisTab->aPageZoomX : pThisTab->aZoomX; }
    const Fraction& GetZoomY() const        { return bPagebreak ? pThisTab->aPageZoomY : pThisTab->aZoomY; }

    void            SetShowGrid( bool bShow );
    bool            GetShowGrid() const { return pThisTab->bShowGrid; }

    const MapMode&  GetLogicMode( ScSplitPos eWhich );
    const MapMode&  GetLogicMode();                     // Offset 0

    double          GetPPTX() const { return nPPTX; }
    double          GetPPTY() const { return nPPTY; }

    ScMarkType      GetSimpleArea( SCCOL& rStartCol, SCROW& rStartRow, SCTAB& rStartTab,
                                    SCCOL& rEndCol, SCROW& rEndRow, SCTAB& rEndTab ) const;
    ScMarkType      GetSimpleArea( ScRange& rRange ) const;
                    /// May modify rNewMark using MarkToSimple().
    ScMarkType      GetSimpleArea( ScRange & rRange, ScMarkData & rNewMark ) const;
    void            GetMultiArea( ScRangeListRef& rRange ) const;

    bool            SimpleColMarked();
    bool            SimpleRowMarked();

    bool            IsMultiMarked();

                    /** Disallow cell fill (Paste,Fill,...) on Ctrl+A all
                        selected or another high amount of selected cells.
                        We'd go DOOM.
                     */
    bool            SelectionForbidsCellFill();
                    /// Determine DOOM condition, i.e. from selected range.
    static bool     SelectionFillDOOM( const ScRange& rRange );

    void            SetFillMode( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow );
    void            SetDragMode( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    ScFillMode nMode );
    void            GetFillData( SCCOL& rStartCol, SCROW& rStartRow,
                                 SCCOL& rEndCol, SCROW& rEndRow );
    void            ResetFillMode();
    bool            IsAnyFillMode()             { return nFillMode != ScFillMode::NONE; }
    bool            IsFillMode()                { return nFillMode == ScFillMode::FILL; }
    ScFillMode      GetFillMode()               { return nFillMode; }

    SvxAdjust       GetEditAdjust() const {return eEditAdjust; }
    void            SetEditAdjust( SvxAdjust eNewEditAdjust ) { eEditAdjust = eNewEditAdjust; }

                    // TRUE: Cell is merged
    bool            GetMergeSizePixel( SCCOL nX, SCROW nY, long& rSizeXPix, long& rSizeYPix ) const;
    void            GetPosFromPixel( long nClickX, long nClickY, ScSplitPos eWhich,
                                        SCCOL& rPosX, SCROW& rPosY,
                                        bool bTestMerge = true, bool bRepair = false );
    void            GetMouseQuadrant( const Point& rClickPos, ScSplitPos eWhich,
                                        SCCOL nPosX, SCROW nPosY, bool& rLeft, bool& rTop );

    bool            IsRefMode() const                       { return bIsRefMode; }
    ScRefType       GetRefType() const                      { return eRefType; }
    SCCOL           GetRefStartX() const                    { return nRefStartX; }
    SCROW           GetRefStartY() const                    { return nRefStartY; }
    SCTAB           GetRefStartZ() const                    { return nRefStartZ; }
    SCCOL           GetRefEndX() const                      { return nRefEndX; }
    SCROW           GetRefEndY() const                      { return nRefEndY; }
    SCTAB           GetRefEndZ() const                      { return nRefEndZ; }

    void            SetRefMode( bool bNewMode, ScRefType eNewType )
                                    { bIsRefMode = bNewMode; eRefType = eNewType; }

    void            SetRefStart( SCCOL nNewX, SCROW nNewY, SCTAB nNewZ );
    void            SetRefEnd( SCCOL nNewX, SCROW nNewY, SCTAB nNewZ );

    void            ResetDelMark()                          { bDelMarkValid = false; }
    void            SetDelMark( const ScRange& rRange )
                            { aDelRange = rRange; bDelMarkValid = true; }

    bool            GetDelMark( ScRange& rRange ) const
                            { rRange = aDelRange; return bDelMarkValid; }

    inline void     GetMoveCursor( SCCOL& rCurX, SCROW& rCurY );

    const ScViewOptions&    GetOptions() const { return *pOptions; }
    void                    SetOptions( const ScViewOptions& rOpt );

    bool    IsGridMode      () const            { return pOptions->GetOption( VOPT_GRID ); }
    void    SetGridMode     ( bool bNewMode )   { pOptions->SetOption( VOPT_GRID, bNewMode ); }
    bool    IsSyntaxMode    () const            { return pOptions->GetOption( VOPT_SYNTAX ); }
    void    SetSyntaxMode   ( bool bNewMode )   { pOptions->SetOption( VOPT_SYNTAX, bNewMode ); }
    bool    IsHeaderMode    () const            { return pOptions->GetOption( VOPT_HEADER ); }
    void    SetHeaderMode   ( bool bNewMode )   { pOptions->SetOption( VOPT_HEADER, bNewMode ); }
    bool    IsTabMode       () const            { return pOptions->GetOption( VOPT_TABCONTROLS ); }
    void    SetTabMode      ( bool bNewMode )   { pOptions->SetOption( VOPT_TABCONTROLS, bNewMode ); }
    bool    IsVScrollMode   () const            { return pOptions->GetOption( VOPT_VSCROLL ); }
    void    SetVScrollMode  ( bool bNewMode )   { pOptions->SetOption( VOPT_VSCROLL, bNewMode ); }
    bool    IsHScrollMode   () const            { return pOptions->GetOption( VOPT_HSCROLL ); }
    void    SetHScrollMode  ( bool bNewMode )   { pOptions->SetOption( VOPT_HSCROLL, bNewMode ); }
    bool    IsOutlineMode   () const            { return pOptions->GetOption( VOPT_OUTLINER ); }
    void    SetOutlineMode  ( bool bNewMode )   { pOptions->SetOption( VOPT_OUTLINER, bNewMode ); }

    /// Force page size for PgUp/PgDown to overwrite the computation based on m_aVisArea.
    void ForcePageUpDownOffset(long nTwips) { m_nLOKPageUpDownOffset = nTwips; }
    long GetPageUpDownOffset() { return m_nLOKPageUpDownOffset; }

    void            KillEditView();
    void            ResetEditView();
    void            SetEditEngine( ScSplitPos eWhich,
                                    ScEditEngineDefaulter* pNewEngine,
                                    vcl::Window* pWin, SCCOL nNewX, SCROW nNewY );
    void            GetEditView( ScSplitPos eWhich, EditView*& rViewPtr, SCCOL& rCol, SCROW& rRow );
    bool            HasEditView( ScSplitPos eWhich ) const
                                        { return pEditView[eWhich] && bEditActive[eWhich]; }
    EditView*       GetEditView( ScSplitPos eWhich ) const
                                        { return pEditView[eWhich].get(); }

    /**
     * Extend the output area for the edit engine view in a horizontal
     * direction as needed.
     */
    void            EditGrowX();

    /**
     * Extend the output area for the edit engine view in a vertical direction
     * as needed.
     *
     * @param bInitial when true, then the call originates from a brand-new
     *                 edit engine instance.
     */
    void            EditGrowY( bool bInitial = false );

    ScSplitPos      GetEditActivePart() const       { return eEditActivePart; }
    SCCOL           GetEditViewCol() const          { return nEditCol; }
    SCROW           GetEditViewRow() const          { return nEditRow; }
    SCCOL           GetEditStartCol() const         { return nEditStartCol; }
    SCROW           GetEditStartRow() const         { return nEditRow; }        // never editing above the cell
    SCCOL           GetEditEndCol() const           { return nEditEndCol; }
    SCROW           GetEditEndRow() const           { return nEditEndRow; }

    tools::Rectangle       GetEditArea( ScSplitPos eWhich, SCCOL nPosX, SCROW nPosY, vcl::Window* pWin,
                                    const ScPatternAttr* pPattern, bool bForceToTop );

    void            SetTabNo( SCTAB nNewTab );
    void            SetActivePart( ScSplitPos eNewActive );

    Point           GetScrPos( SCCOL nWhereX, SCROW nWhereY, ScSplitPos eWhich,
                                bool bAllowNeg = false ) const;
    Point           GetScrPos( SCCOL nWhereX, SCROW nWhereY, ScHSplitPos eWhich ) const;
    Point           GetScrPos( SCCOL nWhereX, SCROW nWhereY, ScVSplitPos eWhich ) const;

    SCCOL           CellsAtX( SCCOL nPosX, SCCOL nDir, ScHSplitPos eWhichX, sal_uInt16 nScrSizeY = SC_SIZE_NONE ) const;
    SCROW           CellsAtY( SCROW nPosY, SCROW nDir, ScVSplitPos eWhichY, sal_uInt16 nScrSizeX = SC_SIZE_NONE ) const;

    SCCOL           VisibleCellsX( ScHSplitPos eWhichX ) const;     // Completely visible cell
    SCROW           VisibleCellsY( ScVSplitPos eWhichY ) const;
    SCCOL           PrevCellsX( ScHSplitPos eWhichX ) const;        // Cells on the preceding page
    SCROW           PrevCellsY( ScVSplitPos eWhichY ) const;

    bool            IsOle();
    void            SetScreen( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
    void            SetScreen( const tools::Rectangle& rVisArea );
    void            SetScreenPos( const Point& rVisAreaStart );

    void            UpdateScreenZoom( const Fraction& rNewX, const Fraction& rNewY );

    const Size&     GetScrSize() const              { return aScrSize; }

    void            RecalcPixPos();
    Point           GetPixPos( ScSplitPos eWhich ) const
                    { return Point( pThisTab->nPixPosX[WhichH(eWhich)],
                                    pThisTab->nPixPosY[WhichV(eWhich)] ); }
    void            SetSpellingView( EditView* pSpView) { pSpellingView = pSpView; }
    EditView*       GetSpellingView() const { return pSpellingView; }

    void            UpdateOutlinerFlags( Outliner& rOutl ) const;

    Point           GetMousePosPixel();

    bool            UpdateFixX(SCTAB nTab = MAXTAB+1);
    bool            UpdateFixY(SCTAB nTab = MAXTAB+1);

    SCCOL           GetTabStartCol() const          { return nTabStartCol; }
    void            SetTabStartCol(SCCOL nNew)      { nTabStartCol = nNew; }

    ScAddress       GetCurPos() const;

    const Size&     GetScenButSize() const              { return aScenButSize; }
    void            SetScenButSize(const Size& rNew)    { aScenButSize = rNew; }

    bool            IsSelCtrlMouseClick() { return bSelCtrlMouseClick; }

    static inline long ToPixel( sal_uInt16 nTwips, double nFactor );

    /** while (rScrY <= nEndPixels && rPosY <= nEndRow) add pixels of row
        heights converted with nPPTY to rScrY, optimized for row height
        segments. Upon return rPosY is the last row evaluated <= nEndRow, rScrY
        may be > nEndPixels!
     */
    static void     AddPixelsWhile( long & rScrY, long nEndPixels,
                                    SCROW & rPosY, SCROW nEndRow, double nPPTY,
                                    const ScDocument * pDoc, SCTAB nTabNo );

    /** while (rScrY <= nEndPixels && rPosY >= nStartRow) add pixels of row
        heights converted with nPPTY to rScrY, optimized for row height
        segments. Upon return rPosY is the last row evaluated >= nStartRow,
        rScrY may be > nEndPixels!
     */
    static void     AddPixelsWhileBackward( long & rScrY, long nEndPixels,
                                    SCROW & rPosY, SCROW nStartRow, double nPPTY,
                                    const ScDocument * pDoc, SCTAB nTabNo );
};

inline long ScViewData::ToPixel( sal_uInt16 nTwips, double nFactor )
{
    long nRet = static_cast<long>( nTwips * nFactor );
    if ( !nRet && nTwips )
        nRet = 1;
    return nRet;
}

inline void ScViewData::GetMoveCursor( SCCOL& rCurX, SCROW& rCurY )
{
    if ( bIsRefMode )
    {
        rCurX = nRefEndX;
        rCurY = nRefEndY;
    }
    else
    {
        rCurX = GetCurX();
        rCurY = GetCurY();
    }
}

inline ScHSplitPos WhichH( ScSplitPos ePos )
{
    return (ePos==SC_SPLIT_TOPLEFT || ePos==SC_SPLIT_BOTTOMLEFT) ?
                SC_SPLIT_LEFT : SC_SPLIT_RIGHT;
}

inline ScVSplitPos WhichV( ScSplitPos ePos )
{
    return (ePos==SC_SPLIT_TOPLEFT || ePos==SC_SPLIT_TOPRIGHT) ?
                SC_SPLIT_TOP : SC_SPLIT_BOTTOM;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
