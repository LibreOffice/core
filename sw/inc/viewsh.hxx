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

#pragma once

#include "swdllapi.h"
#include "swtypes.hxx"
#include "ring.hxx"
#include "swrect.hxx"
#include <memory>
#include <stack>
#include <vcl/mapmod.hxx>
#include <vcl/vclptr.hxx>
#include <tools/lazydelete.hxx>
#include <vcl/window.hxx>

namespace comphelper { class OAccessible; }
class SwDoc;
class IDocumentSettingAccess;
class IDocumentDeviceAccess;
class IDocumentMarkAccess;
class IDocumentDrawModelAccess;
class IDocumentRedlineAccess;
class IDocumentLayoutAccess;
class IDocumentContentOperations;
class IDocumentStylePoolAccess;
class IDocumentStatistics;
class IDocumentUndoRedo;
class IDocumentListItems;
class IDocumentOutlineNodes;
class SfxPrinter;
class SwRootFrame;
class SwNodes;
class SdrView;
class SfxItemPool;
class SfxViewShell;
class SwViewOption;
class SwViewShellImp;
class SwPrintData;
struct ShellResource;
class SwRegionRects;
class SwPagePreviewLayout;
class SwTextFrame;

class SwAccessibilityOptions;
namespace vcl { class Region; }
class SwPostItMgr;
class SdrPaintWindow;
class SwAccessibleMap;
enum class Orientation;

enum class LockPaintReason
{
    ViewLayout = 1,
    OuterResize,
    Undo,
    Redo,
    OutlineFolding,
    EndSdrCreate,
    SwLayIdle,
    InvalidateLayout,
    StartDrag,
    DataChanged,
    InsertFrame,
    GotoPage,
    InsertGraphic,
    SetZoom,
    ExampleFrame
};

enum class ScrollSizeMode
{
    ScrollSizeDefault, //usually 30% of the visible area
    ScrollSizeMouseSelection, //make target rectangle visible
    ScrollSizeTimer, // increase of timer based scrolling
    ScrollSizeTimer2 // more increase of timer based scrolling
};
namespace vcl
{
    typedef OutputDevice RenderContext;
}

// Define for flags needed in ctor or layers below.
// Currently the Preview flag is needed for DrawPage.
#define VSHELLFLAG_ISPREVIEW            (tools::Long(0x1))
#define VSHELLFLAG_SHARELAYOUT          (tools::Long(0x2))
typedef std::shared_ptr<SwRootFrame> SwRootFramePtr;

typedef struct _xmlTextWriter* xmlTextWriterPtr;

struct SwVisiblePageNumbers
{
    sal_uInt16 nFirstPhy, nLastPhy;
    sal_uInt16 nFirstVirt, nLastVirt;
    OUString sFirstCustomPhy, sLastCustomPhy;
    OUString sFirstCustomVirt, sLastCustomVirt;
};

class SwView;

class SAL_DLLPUBLIC_RTTI SwViewShell : public sw::Ring<SwViewShell>
{
    friend void SetOutDev( SwViewShell *pSh, OutputDevice *pOut );
    friend void SetOutDevAndWin( SwViewShell *pSh, OutputDevice *pOut,
                                 vcl::Window *pWin, sal_uInt16 nZoom );

    friend class SwViewShellImp;
    friend class SwLayIdle;

    // For setting visible area for page preview paint.
    friend class SwPagePreviewLayout;

    // Set SwVisArea in order to enable clean formatting before printing.
    friend void SetSwVisArea( SwViewShell *pSh, const SwRect & );

    std::unique_ptr<BitmapEx> m_xReplaceBmp; ///< replaced display of still loaded images
    std::unique_ptr<BitmapEx> m_xErrorBmp;   ///< error display of missed images

    static bool sbLstAct;            // true if EndAction of last Shell
                                     // i.e. if the EndActions of the other
                                     // Shells on the document are through.

    Point         maPrtOffset;       // Offset for Printer,
                                     // non-printable margin.
    Size          maBrowseBorder;    // Border for frame documents.
    SwRect        maInvalidRect;

    SfxViewShell *mpSfxViewShell;
    std::unique_ptr<SwViewShellImp>
                  mpImp;             // Core-internals of SwViewShell.
                                     // The pointer is never 0.

    VclPtr<vcl::Window>   mpWin;     ///< = 0 during printing or pdf export
    VclPtr<OutputDevice>  mpOut;     ///< Window, Printer, VirtDev, ...

    std::unique_ptr<SwViewOption> mpOpt;
    std::unique_ptr<SwAccessibilityOptions> mpAccOptions;

    bool  mbDocSizeChgd     :1;  // For DocChgNotify(): Announce new DocSize
                                    // at EndAction to DocMDI.
    bool  mbPaintWorks      :1;  // Normal Painting if true,
                                    // remember Paint if false.
    bool  mbPaintInProgress :1;  // Block any double paint.
    bool  mbViewLocked      :1;  // Lock visible range;
                                    // in this case MakeVisible is ineffectual.
    bool  mbInEndAction     :1;  // Avoid problems, cf. viewsh.cxx.
    bool  mbPreview         :1;  // If true it is a Preview-SwViewShell.
    bool  mbEnableSmooth    :1;  // Disable SmoothScroll, e.g. for drag
                                    // of scrollbars.
    bool      mbShowHeaderSeparator:1; ///< Flag to say that we are showing the header control
    bool      mbShowFooterSeparator:1; ///< Flag to say that we are showing the footer control
    bool      mbHeaderFooterEdit:1;  ///< Flag to say that we are editing header or footer (according to the bShow(Header|Footer)Separator above)

    // boolean, indicating that class in constructor.
    bool mbInConstructor:1;

    SdrPaintWindow*         mpTargetPaintWindow;
    VclPtr<OutputDevice>    mpBufferedOut;

    SwRootFramePtr            mpLayout;

    // Initialization; called by the diverse constructors.
    void Init( const SwViewOption *pNewOpt, tools::Long nFlags);

    inline void ResetInvalidRect();



    void PaintDesktop(const vcl::RenderContext& rRenderContext, const SwRect&);  // Collect values for painting of desktop
                                                        // and calling.
    // PaintDesktop split. This pars is also used by PreviewPage.
    void PaintDesktop_(const SwRegionRects &rRegion);

    static bool DrawAppBackgroundBitmap(vcl::RenderContext* rRenderContext, const SwRect& rRect);

    bool CheckInvalidForPaint( const SwRect & );  // Direct Paint or rather
                                                                    // trigger an action.

    void PrepareForPrint( const SwPrintData &rOptions, bool bIsPDFExport = false );

    void ImplApplyViewOptions( const SwViewOption &rOpt );

    void InvalidateAll(std::vector<LockPaintReason>& rReasons);

    void InvalidatePageAndHFSubsidiaryLines();

protected:
    static ShellResource*      spShellRes;      ///< Resources for the Shell.

    SwRect                  maVisArea;       ///< The modern version of VisArea.
    tools::Rectangle        maLOKVisibleArea;///< The visible area in the LibreOfficeKit client.
    rtl::Reference<SwDoc>   mxDoc;          ///< The document; never 0.

    sal_uInt16 mnStartAction; ///< != 0 if at least one Action is active.
    sal_uInt16 mnLockPaint;   ///< != 0 if Paint is locked.
    std::vector<LockPaintReason> maLockPaintReasons;
    bool      mbSelectAll; ///< Special select all mode: whole document selected, even if doc starts with table.

    /// The virtual device we paint to will end up on the screen.
    bool mbOutputToWindow;

public:

          SwViewShellImp *Imp() { return mpImp.get(); }
    const SwViewShellImp *Imp() const { return mpImp.get(); }

    const SwNodes& GetNodes() const;

    // After change of printer; by Doc.
    void            InitPrt( OutputDevice *pOutDev );

    // Bracketing of actions belonging together.
    virtual void StartAction();
    SW_DLLPUBLIC void ImplStartAction();
    virtual void EndAction(const bool bIdleEnd = false);
    SW_DLLPUBLIC void ImplEndAction( const bool bIdleEnd );
    sal_uInt16 ActionCount() const { return mnStartAction; }
    bool ActionPend() const { return mnStartAction != 0; }
    bool IsInEndAction() const { return mbInEndAction; }

    /// For all views of this document.
    SW_DLLPUBLIC void StartAllAction();
    SW_DLLPUBLIC void EndAllAction();

    // The ActionCount for all Shells is temporarily set to zero and then
    // restored at the RootFrame via UNO.
    void    SetRestoreActions(sal_uInt16 nSet);
    sal_uInt16  GetRestoreActions() const;

    bool HasInvalidRect() const { return maInvalidRect.HasArea(); }
    void ChgHyphenation() { Reformat(); }
    void ChgNumberDigits();

    bool AddPaintRect( const SwRect &rRect );

    void InvalidateWindows( const SwRect &rRect );
    void FlushPendingLOKInvalidateTiles();

    /// Invalidates complete Layout (ApplyViewOption).
    SW_DLLPUBLIC void Reformat();

    // #i72754# set of Pre/PostPaints with lock counter and initial target OutDev
protected:
    std::stack<vcl::Region> mPrePostPaintRegions; // acts also as a lock counter (empty == not locked)
    VclPtr<OutputDevice>    mpPrePostOutDev;
    MapMode                 maPrePostMapMode;
public:
    void PrePaint();
    void DLPrePaint2(const vcl::Region& rRegion);
    void DLPostPaint2(bool bPaintFormLayer);
    const MapMode& getPrePostMapMode() const { return maPrePostMapMode; }

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle &rRect);

    /** Paint tile.

        Sets the pOut so that the rRect is always painted over the entire
        pOut, ie. starts in 0,0 and ends in width/height.
    */
    void PaintTile(VirtualDevice &rDevice, int contextWidth, int contextHeight, int tilePosX, int tilePosY, tools::Long tileWidth, tools::Long tileHeight);

    bool IsPaintInProgress() const { return mbPaintInProgress; }
    bool IsDrawingLayerPaintInProgress() const { return !mPrePostPaintRegions.empty(); }

    // Notification that visible area has been changed.
    // VisArea is reset, after that scrolling takes place.
    // The passed rect is situated on pixel borders
    // in order to avoid pixel errors when scrolling.
    virtual void VisPortChgd( const SwRect & );
    bool SmoothScroll( tools::Long lXDiff, tools::Long lYDiff, const tools::Rectangle* );//Browser
    void EnableSmooth( bool b ) { mbEnableSmooth = b; }

    const SwRect& VisArea() const;

    /// The visible area in the client (set by setClientVisibleArea).
    const tools::Rectangle & getLOKVisibleArea() const { return maLOKVisibleArea; }
    void setLOKVisibleArea(const tools::Rectangle& rArea) { maLOKVisibleArea = rArea; }

    // If necessary scroll until passed Rect is situated in visible sector.
    void MakeVisible( const SwRect &, ScrollSizeMode eScrollSizeMode = ScrollSizeMode::ScrollSizeDefault );

    // At nearest occasion pass new document size to UI.
    void SizeChgNotify();
    void UISizeNotify();            // Passing of current size.

    Point GetPagePos( sal_uInt16 nPageNum ) const;

    sal_uInt16 GetNumPages() const;   // Ask count of current pages from layout.
    bool   IsDummyPage( sal_uInt16 nPageNum ) const;  // An empty page?

    // Invalidate first visible page for all Shells in ring.
    void SetFirstVisPageInvalid();

    SW_DLLPUBLIC SwRootFrame* GetLayout() const;
    bool         IsNewLayout() const; // Has Layout been loaded or created?

     Size GetDocSize() const;   // Get document size.

    virtual void CalcLayout();  // Force complete formatting of layout.

    SW_DLLPUBLIC sal_uInt16 GetPageCount() const;

    SW_DLLPUBLIC Size GetPageSize( sal_uInt16 nPageNum, bool bSkipEmptyPages ) const;

    SwDoc *GetDoc()  const { return mxDoc.get(); }  //Never 0.

    /** Provides access to the document setting interface
     */
    const IDocumentSettingAccess& getIDocumentSettingAccess() const;
    SW_DLLPUBLIC IDocumentSettingAccess& getIDocumentSettingAccess();

    /** Provides access to the document device interface
     */
    const IDocumentDeviceAccess& getIDocumentDeviceAccess() const;
    SW_DLLPUBLIC IDocumentDeviceAccess& getIDocumentDeviceAccess();

    /** Provides access to the document bookmark interface
     */
    const IDocumentMarkAccess* getIDocumentMarkAccess() const;
    SW_DLLPUBLIC IDocumentMarkAccess* getIDocumentMarkAccess();

    /** Provides access to the document draw model interface
     */
    const IDocumentDrawModelAccess& getIDocumentDrawModelAccess() const;
          IDocumentDrawModelAccess& getIDocumentDrawModelAccess();

    /** Provides access to the document redline interface
     */
    const IDocumentRedlineAccess& getIDocumentRedlineAccess() const;
          IDocumentRedlineAccess& getIDocumentRedlineAccess();

    /** Provides access to the document layout interface
     */
    const IDocumentLayoutAccess& getIDocumentLayoutAccess() const;
    SW_DLLPUBLIC IDocumentLayoutAccess& getIDocumentLayoutAccess();

    /** Provides access to the content operations interface
     */
    IDocumentContentOperations& getIDocumentContentOperations();

    /** Provides access to the document style pool interface
     */
    SW_DLLPUBLIC IDocumentStylePoolAccess& getIDocumentStylePoolAccess();

    /** Provides access to the document statistics interface
     */
    const IDocumentStatistics& getIDocumentStatistics() const;

    /** Provides access to the document undo/redo interface
     */
    IDocumentUndoRedo const& GetIDocumentUndoRedo() const;
    IDocumentUndoRedo      & GetIDocumentUndoRedo();

    SW_DLLPUBLIC const IDocumentListItems* getIDocumentListItemsAccess() const;
    SW_DLLPUBLIC const IDocumentOutlineNodes* getIDocumentOutlineNodesAccess() const;

    // 1. GetRefDev:   Either the printer or the virtual device from the doc
    // 2. GetWin:      Available if we not printing
    // 3. GetOut:      Printer, Window or Virtual device
    vcl::RenderContext& GetRefDev() const;
    vcl::Window* GetWin()    const { return mpWin; }
    vcl::RenderContext* GetOut()     const { return mpOut; }

    void SetWin(vcl::Window* win) { mpWin = win; }
    void SetOut(vcl::RenderContext* pOut) { mpOut = pOut; }
    static bool IsLstEndAction() { return SwViewShell::sbLstAct; }

    // Change of all page descriptors.
    void   ChgAllPageOrientation( Orientation eOri );
    void   ChgAllPageSize( Size const &rSz );

    // Printing of one page.
    // bIsPDFExport == true is: do PDF Export (no printing!)
    bool PrintOrPDFExport( OutputDevice *pOutDev,
            SwPrintData const& rPrintData,
            sal_Int32 nRenderer, /* offset in vector of pages to print */
            bool bIsPDFExport );

    // Printing of one brochure page.
    void PrintProspect( OutputDevice *pOutDev, const SwPrintData &rPrintData,
            sal_Int32 nRenderer /* offset in vector of page pairs for prospect printing */ );

    // Printing for OLE 2.0.
    static void PrtOle2( SwDoc *pDoc, const SwViewOption *pOpt, const SwPrintData& rOptions,
                         vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect,
                         bool bOutputForScreen );

    // Fill temporary doc with selected text for Print or PDF export.
    void FillPrtDoc( SwDoc& rPrtDoc, const SfxPrinter* pPrt );

    // Called internally for Shell. Formats pages.
    void CalcPagesForPrint( sal_uInt16 nMax );

    // All about fields.
    SW_DLLPUBLIC void UpdateFields(bool bCloseDB = false, bool bSetModified = true);
    bool IsAnyFieldInDoc() const;

    /// Update the previews of all OLE objects.
    SW_DLLPUBLIC void UpdateOleObjectPreviews();

    // Update all charts, for that exists any table.
    void UpdateAllCharts();
    bool HasCharts() const;

    // DOCUMENT COMPATIBILITY FLAGS START

    // Add or maximize paragraph spacing?
    SW_DLLPUBLIC void SetParaSpaceMax( bool bNew );

    // Add or maximize paragraph spacing?
    SW_DLLPUBLIC void SetParaSpaceMaxAtPages( bool bNew );

    // Compatible behaviour of tabs.
    SW_DLLPUBLIC void SetTabCompat( bool bNew );

    // Font metric attribute "External Leading" should be considered.
    SW_DLLPUBLIC void SetAddExtLeading( bool bNew );

    // Adding paragraph and table spacing at bottom
    // of table cells.
    SW_DLLPUBLIC void SetAddParaSpacingToTableCells( bool _bAddParaSpacingToTableCells );

    // Former formatting of text lines with
    // proportional line spacing or not.
    SW_DLLPUBLIC void SetUseFormerLineSpacing( bool _bUseFormerLineSpacing );

    // Former object positioning.
    SW_DLLPUBLIC void SetUseFormerObjectPositioning( bool _bUseFormerObjPos );

    SW_DLLPUBLIC void SetConsiderWrapOnObjPos( bool _bConsiderWrapOnObjPos );

    SW_DLLPUBLIC void SetUseFormerTextWrapping( bool _bUseFormerTextWrapping );

    SW_DLLPUBLIC void SetDoNotJustifyLinesWithManualBreak( bool _bDoNotJustifyLinesWithManualBreak );

    SW_DLLPUBLIC void SetProtectForm( bool _bProtectForm );

    SW_DLLPUBLIC void SetMsWordCompTrailingBlanks( bool _bMsWordCompTrailingBlanks );

    SW_DLLPUBLIC void SetSubtractFlysAnchoredAtFlys(bool bSubtractFlysAnchoredAtFlys);

    SW_DLLPUBLIC void SetEmptyDbFieldHidesPara(bool bEmptyDbFieldHidesPara);

    SW_DLLPUBLIC void SetNoGapAfterNoteNumber(bool bNew);

    SW_DLLPUBLIC void SetTabsRelativeToIndent(bool bNew);

    SW_DLLPUBLIC void SetTabOverMargin(bool bNew);

    SW_DLLPUBLIC void SetDoNotMirrorRtlDrawObjs(bool bDoNotMirrorRtlDrawObjs);

    SW_DLLPUBLIC void SetContinuousEndnotes(bool bContinuousEndnotes);

    SW_DLLPUBLIC void SetMsWordCompGridMetrics(bool bMsWordCompGridMetrics);

    SW_DLLPUBLIC void SetIgnoreTabsAndBlanksForLineCalculation(bool bIgnoreTabsAndBlanksForLineCalculation);

    SW_DLLPUBLIC void SetMsWordUlTrailSpace(bool val);

    SW_DLLPUBLIC void SetBalanceSpacesAndIdeographicSpaces(bool bValue);

    SW_DLLPUBLIC void SetAdjustTableLineHeightsToGridHeight(bool bValue);

    // DOCUMENT COMPATIBILITY FLAGS END

    // Calls Idle-formatter of Layout.
    SW_DLLPUBLIC void LayoutIdle();

    const SwViewOption *GetViewOptions() const { return mpOpt.get(); }
    virtual void  ApplyViewOptions( const SwViewOption &rOpt );
           void  SetUIOptions( const SwViewOption &rOpt );
    virtual void  SetReadonlyOption(bool bSet);          // Set readonly-bit of ViewOptions.
           void  SetPDFExportOption(bool bSet);         // Set/reset PDF export mode.
           void  SetPrtFormatOption(bool bSet);         // Set PrtFormat-Bit of ViewOptions.
           void  SetReadonlySelectionOption(bool bSet); // Change the selection mode in readonly docs.

    const SwAccessibilityOptions* GetAccessibilityOptions() const { return mpAccOptions.get();}

    static void           SetShellRes( ShellResource* pRes ) { spShellRes = pRes; }
    SW_DLLPUBLIC static ShellResource* GetShellRes();

    static weld::Window*   CareChildWin(SwViewShell const & rVSh);
    SW_DLLPUBLIC static void SetCareDialog(const std::shared_ptr<weld::Window>& rNew);
    static weld::Window*  GetCareDialog(SwViewShell const & rVSh);

    SAL_RET_MAYBENULL SfxViewShell   *GetSfxViewShell() const { return mpSfxViewShell; }
    void           SetSfxViewShell(SfxViewShell *pNew) { mpSfxViewShell = pNew; }

    // Selection of  Draw Engine has been changed.
    virtual void DrawSelChanged();

    SwPagePreviewLayout* PagePreviewLayout();

    /** adjust view options for page preview

        Because page preview should show the document as it is printed -
        page preview is print preview -, the view options are adjusted to the
        same as for printing.

        @param _rPrintOptions
        input parameter - constant reference to print options, to which the
        view option will be adjusted.
    */
    void AdjustOptionsForPagePreview( SwPrintData const& rPrintOptions );

    bool IsViewLocked() const { return mbViewLocked; }
    void LockView( bool b )   { mbViewLocked = b;    }

    inline void LockPaint(LockPaintReason eReason);
           void ImplLockPaint();
    inline void UnlockPaint(bool bVirDev = false );
           void ImplUnlockPaint( std::vector<LockPaintReason>& rReasons, bool bVirDev );
           bool IsPaintLocked() const { return mnLockPaint != 0; }

    // Get/set DrawView and PageView.
    bool HasDrawView() const;
    void MakeDrawView();

    // Are we dragging draw shapes around.
    bool HasDrawViewDrag() const;

    // DrawView may be used at UI.
    SW_DLLPUBLIC SdrView *GetDrawView();
    const SdrView *GetDrawView() const { return const_cast<SwViewShell*>(this)->GetDrawView(); }

    // Take care that MarkList is up-to-date in any case (Bug 57153).
    SdrView *GetDrawViewWithValidMarkList();

    // Query attribute pool.
    inline const SfxItemPool& GetAttrPool() const;
    SW_DLLPUBLIC SfxItemPool& GetAttrPool();

    bool IsPreview() const { return mbPreview; }

    // Invalidates pages and contents.
    // When bSizeChanged==true, adds/removes
    // headers and footers as necessary.
    void InvalidateLayout(bool bSizeChanged);

    const Size& GetBrowseBorder() const;
    sal_Int32 GetBrowseWidth() const;
    void SetBrowseBorder( const Size& rNew );

    rtl::Reference<comphelper::OAccessible> CreateAccessible();

    rtl::Reference<comphelper::OAccessible> CreateAccessiblePreview();

    void ShowPreviewSelection( sal_uInt16 nSelPage );
    void InvalidateAccessibleFocus();

    // Apply Accessibility options.
    void ApplyAccessibilityOptions();

    /** invalidate CONTENT_FLOWS_FROM/_TO relation for paragraphs

        @param _pFromTextFrame
        input parameter - paragraph frame, for which the relation CONTENT_FLOWS_FROM
        has to be invalidated.
        If NULL, no CONTENT_FLOWS_FROM relation has to be invalidated

        @param _pToTextFrame
        input parameter - paragraph frame, for which the relation CONTENT_FLOWS_TO
        has to be invalidated.
        If NULL, no CONTENT_FLOWS_TO relation has to be invalidated
    */
    void InvalidateAccessibleParaFlowRelation( const SwTextFrame* _pFromTextFrame,
                                               const SwTextFrame* _pToTextFrame );

    /** invalidate text selection for paragraphs
    */
    void InvalidateAccessibleParaTextSelection();

    /** invalidate attributes for paragraphs and paragraph's characters

        usage also for changes of the attributes of
        paragraph's characters.

        @param rTextFrame
        input parameter - paragraph frame, whose attributes have changed
    */
    void InvalidateAccessibleParaAttrs( const SwTextFrame& rTextFrame );

    SwAccessibleMap* GetAccessibleMap();

    SwViewShell( SwViewShell&, vcl::Window *pWin, OutputDevice *pOut = nullptr,
                tools::Long nFlags = 0 );
    SwViewShell( SwDoc& rDoc, vcl::Window *pWin,
               const SwViewOption *pOpt, OutputDevice *pOut = nullptr,
               tools::Long nFlags = 0 );
    virtual ~SwViewShell() override;

    sal_Int32 GetPageNumAndSetOffsetForPDF( OutputDevice& rOut, const SwRect& rRect ) const;

    bool IsInConstructor() const { return mbInConstructor; }

    const BitmapEx& GetReplacementBitmap(bool bIsErrorState);
    void DeleteReplacementBitmaps();

    const SwPostItMgr* GetPostItMgr() const { return const_cast<SwViewShell*>(this)->GetPostItMgr(); }
    SW_DLLPUBLIC virtual SwPostItMgr* GetPostItMgr();

    /// Acts both for headers / footers, depending on the bShow(Header|Footer)Separator flags
    void ToggleHeaderFooterEdit();
    /// Acts both for headers / footers, depending on the bShow(Header|Footer)Separator flags
    bool IsHeaderFooterEdit() const { return mbHeaderFooterEdit; }
    bool IsShowHeaderFooterSeparator( FrameControlType eControl ) { return (eControl == FrameControlType::Header)? mbShowHeaderSeparator: mbShowFooterSeparator; }
    virtual void SetShowHeaderFooterSeparator( FrameControlType eControl, bool bShow );
    bool IsSelectAll() const { return mbSelectAll; }

    void setOutputToWindow(bool bOutputToWindow);
    bool isOutputToWindow() const;
    void OnGraphicArrived(const SwRect&);

    void GetFirstLastVisPageNumbers(SwVisiblePageNumbers& rVisiblePageNumbers, const SwView& rView);

    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const;

    // SwTextFrame::AutoSpell_ calls this, to notify the shell, that a word has a spelling error,
    // but that couldn't be drawn, because the cursor was in that word (so that the user is not
    // annoyed while typing). The shell's task is to re-run the spell check (i.e., call LayoutIdle,
    // which internally does the spell check), when the cursor leaves that word (implemented in
    // SwCursorShell).
    virtual void OnSpellWrongStatePending() {}
};

// manages global ShellPointer
class CurrShell
{
public:
    SwViewShell *pPrev;
    SwRootFrame *pRoot;

    CurrShell( SwViewShell *pNew );
    ~CurrShell();
};

inline void SwViewShell::ResetInvalidRect()
{
    maInvalidRect.Clear();
}

inline void SwViewShell::LockPaint(LockPaintReason eReason)
{
    maLockPaintReasons.push_back(eReason);
    if ( !mnLockPaint++ )
        ImplLockPaint();
}

inline void SwViewShell::UnlockPaint(bool bVirDev )
{
    if ( 0 == --mnLockPaint )
    {
        ImplUnlockPaint(maLockPaintReasons, bVirDev);
        maLockPaintReasons.clear();
    }
}

inline const SfxItemPool& SwViewShell::GetAttrPool() const
{
    return const_cast<SwViewShell*>(this)->GetAttrPool();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
