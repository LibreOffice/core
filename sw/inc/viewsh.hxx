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
#ifndef SW_VIEWSH_HXX
#define SW_VIEWSH_HXX

#include <com/sun/star/embed/XClassifiedObject.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <tools/rtti.hxx>
#include "swdllapi.h"
#include <swtypes.hxx>
#include <ring.hxx>
#include <swrect.hxx>
#include <boost/shared_ptr.hpp> // swmod 080115
#include <stack>
#include <vcl/mapmod.hxx>
#include <vcl/print.hxx>

namespace com { namespace sun { namespace star { namespace accessibility {
           class XAccessible; } } } }

class SfxObjectShellRef;
class SwDoc;
class IDocumentSettingAccess;
class IDocumentDeviceAccess;
class IDocumentMarkAccess;
class IDocumentDrawModelAccess;
class IDocumentRedlineAccess;
class IDocumentLayoutAccess;
class IDocumentFieldsAccess;
class IDocumentContentOperations;
class IDocumentStylePoolAccess;
class IDocumentStatistics;
class IDocumentUndoRedo;
class IDocumentListItems;
class IDocumentOutlineNodes;
class SfxPrinter;
class SfxProgress;
class SwRootFrm;
class SwNodes;
class SdrView;
class SfxItemPool;
class SfxViewShell;
class SwViewOption;
class SwViewImp;
class SwPrintData;
class SwPagePreViewPrtData;
class Window;
class OutputDevice;
class SwLayIdle;
struct ShellResource;
class SwRegionRects;
class SwFrm;
class SvtAccessibilityOptions;
class SwPagePreviewLayout;
class SwTxtFrm;
class BitmapEx;

struct SwAccessibilityOptions;
class Region;
class SwPostItMgr;
class SdrPaintWindow;
class SwAccessibleMap;

namespace vcl
{
    class OldStylePrintAdaptor;
}

enum FrameControlType
{
    PageBreak,
    Header,
    Footer
};

// Define for flags needed in ctor or layers below.
// Currently the PreView flag is needed for DrawPage.
#define VSHELLFLAG_ISPREVIEW            ((long)0x1)
#define VSHELLFLAG_SHARELAYOUT          ((long)0x2)//swmod 080125 flag
typedef boost::shared_ptr<SwRootFrm> SwRootFrmPtr;

class SW_DLLPUBLIC ViewShell : public Ring
{
    friend void SetOutDev( ViewShell *pSh, OutputDevice *pOut );
    friend void SetOutDevAndWin( ViewShell *pSh, OutputDevice *pOut,
                                 Window *pWin, sal_uInt16 nZoom );

    friend class SwViewImp;
    friend class SwLayIdle;

    // For setting visible area for page preview paint.
    friend class SwPagePreviewLayout;


    // Set SwVisArea in order to enable clean formatting before printing.
    friend void SetSwVisArea( ViewShell *pSh, const SwRect &, sal_Bool bPDFExport = sal_False );

    static BitmapEx*    pReplaceBmp;    // replaced display of still loaded images
    static BitmapEx*    pErrorBmp;      // error display of missed images

    static sal_Bool bLstAct;        // sal_True if EndAction of last Shell
                                    // i.e. if the EndActions of the other
                                    // Shells on the document are through.

    Point         aPrtOffst;         // Ofst for Printer,
                                     // non-printable margin.
     Size         aBrowseBorder;     // Border for frame documents.
    SwRect        aInvalidRect;

    SfxViewShell *pSfxViewShell;
    SwViewImp    *pImp;             // Core-internals of ViewShell.
                                    // The pointer is never 0.

    Window       *pWin;              // = 0 during printing or pdf export
    OutputDevice *pOut;              // Window, Printer, VirtDev, ...
    OutputDevice* mpTmpRef;           // Temporariy reference device. Is used
                                     // during (printer depending) prospect
                                     // and page preview printing
                                     // (because a scaling has to be set at
                                     // the original printer)

    SwViewOption *pOpt;
    SwAccessibilityOptions* pAccOptions;


    sal_Bool  bDocSizeChgd     :1;  // For DocChgNotify(): Announce new DocSize
                                    // at EndAction to DocMDI.
    sal_Bool  bPaintWorks      :1;  // Normal Painting if sal_True,
                                    // remember Paint if sal_False.
    sal_Bool  bPaintInProgress :1;  // Block any double paint.
    sal_Bool  bViewLocked      :1;  // Lock visible range;
                                    // in this case MakeVisible is ineffectual.
    sal_Bool  bInEndAction     :1;  // Avoid problems, cf. viewsh.cxx.
    sal_Bool  bPreView         :1;  // If sal_True it is a PreView-ViewShell.
    sal_Bool  bFrameView       :1;  // If sal_True it is a  (HTML-)Frame.
    sal_Bool  bEnableSmooth    :1;  // Disable SmoothScroll, e.g. for drag
                                    // of scrollbars.
    sal_Bool  bEndActionByVirDev:1; // Paints from EndAction always via virtual device
                                    // (e.g. when browsing).
    bool      bShowHeaderSeparator:1; ///< Flag to say that we are showing the header control
    bool      bShowFooterSeparator:1; ///< Flag to say that we are showing the footer control
    bool      bHeaderFooterEdit:1;  ///< Flag to say that we are editing header or footer (according to the bShow(Header|Footer)Separator above)

    // boolean, indicating that class in in constructor.
    bool mbInConstructor:1;

    SdrPaintWindow*         mpTargetPaintWindow;
    OutputDevice*           mpBufferedOut;

    SwRootFrmPtr            pLayout;            //swmod 080116

    // Initialization; called by the diverse constructors.
    SW_DLLPRIVATE void Init( const SwViewOption *pNewOpt );

    inline void ResetInvalidRect();

    SW_DLLPRIVATE void Reformat();          // Invalidates complete Layout (ApplyViewOption).

    SW_DLLPRIVATE void PaintDesktop( const SwRect & );  // Collect values for painting of desktop
                                                        // and calling.
    // PaintDesktop split. This pars is also used by PreViewPage.
    SW_DLLPRIVATE void _PaintDesktop( const SwRegionRects &rRegion );

    SW_DLLPRIVATE sal_Bool CheckInvalidForPaint( const SwRect & );  // Direct Paint or rather
                                                                    // trigger an action.

    SW_DLLPRIVATE void PrepareForPrint( const SwPrintData &rOptions );

    SW_DLLPRIVATE void ImplApplyViewOptions( const SwViewOption &rOpt );

protected:
    static ShellResource*   pShellRes;      // Resources for the Shell.
    static Window*          pCareWindow;    // Avoid this window.

    SwRect                  aVisArea;       // The modern version of VisArea.
    SwDoc                   *pDoc;          // The document; never 0.

    sal_uInt16 nStartAction; // != 0 if at least one ::com::sun::star::chaos::Action is active.
    sal_uInt16 nLockPaint;   // != 0 if Paint is locked.

public:
    TYPEINFO();

          SwViewImp *Imp() { return pImp; }
    const SwViewImp *Imp() const { return pImp; }

    const SwNodes& GetNodes() const;

    // After change of printer; by Doc.
    void            InitPrt( OutputDevice *pOutDev );

    // Bracketing of actions belonging together.
    inline void StartAction();
           void ImplStartAction();
    inline void EndAction( const sal_Bool bIdleEnd = sal_False );
           void ImplEndAction( const sal_Bool bIdleEnd = sal_False );
    sal_uInt16 ActionCount() const { return nStartAction; }
    sal_Bool ActionPend() const { return nStartAction != 0; }
    sal_Bool IsInEndAction() const { return bInEndAction; }

    void SetEndActionByVirDev( sal_Bool b ) { bEndActionByVirDev = b; }
    sal_Bool IsEndActionByVirDev()          { return bEndActionByVirDev; }

    // The ActionCount for all Shells is temporarily set to zero and then
    // restored at the RootFrame via UNO.
    void    SetRestoreActions(sal_uInt16 nSet);
    sal_uInt16  GetRestoreActions() const;

    inline sal_Bool HasInvalidRect() const { return aInvalidRect.HasArea(); }
    void ChgHyphenation() { Reformat(); }
    void ChgNumberDigits();

    sal_Bool AddPaintRect( const SwRect &rRect );

    void InvalidateWindows( const SwRect &rRect );

    //////////////////////////////////////////////////////////////////////////////
    // #i72754# set of Pre/PostPaints with lock counter and initial target OutDev
protected:
    std::stack<Region>          mPrePostPaintRegions; // acts also as a lock counter (empty == not locked)
    OutputDevice*           mpPrePostOutDev;
    MapMode                 maPrePostMapMode;
public:
    void PrePaint();
    void DLPrePaint2(const Region& rRegion);
    void DLPostPaint2(bool bPaintFormLayer);
    const MapMode& getPrePostMapMode() const { return maPrePostMapMode; }
    //////////////////////////////////////////////////////////////////////////////

    virtual void Paint(const Rectangle &rRect);
    sal_Bool IsPaintInProgress() const { return bPaintInProgress; }
    bool IsDrawingLayerPaintInProgress() const { return !mPrePostPaintRegions.empty(); }

    // Notification that visible area has been changed.
    // VisArea is reset, after that scrolling takes place.
    // The passed rect is situated on pixel borders
    // in order to avoid pixel errors when scrolling.
    virtual void VisPortChgd( const SwRect & );
    sal_Bool SmoothScroll( long lXDiff, long lYDiff, const Rectangle* );//Browser
    void EnableSmooth( sal_Bool b ) { bEnableSmooth = b; }

    const SwRect& VisArea() const { return aVisArea; }

    // If necessary scroll until passed Rect is situated in visible sector.
    void MakeVisible( const SwRect & );

    // At nearest occasion pass new document size to UI.
    void SizeChgNotify();
    void UISizeNotify();            // Passing of current size.

    Point GetPagePos( sal_uInt16 nPageNum ) const;

    sal_uInt16 GetNumPages();   // Ask count of current pages from layout.
    sal_Bool   IsDummyPage( sal_uInt16 nPageNum ) const;  // An empty page?

    // Invalidate first visible page for all Shells in ring.
    void SetFirstVisPageInvalid();

    SwRootFrm   *GetLayout() const;// swmod 080116
    sal_Bool         IsNewLayout() const; // Has Layout been loaded or created?

     Size GetDocSize() const;   // Get document size.

    void CalcLayout();  // Force complete formatting of layout.

    sal_uInt16 GetPageCount() const;

    const Size GetPageSize( sal_uInt16 nPageNum, bool bSkipEmptyPages ) const;

    inline SwDoc *GetDoc()  const { return pDoc; }  //Never 0.

    /** Provides access to the document setting interface
     */
    const IDocumentSettingAccess* getIDocumentSettingAccess() const;
          IDocumentSettingAccess* getIDocumentSettingAccess();

    /** Provides access to the document device interface
     */
    const IDocumentDeviceAccess* getIDocumentDeviceAccess() const;
          IDocumentDeviceAccess* getIDocumentDeviceAccess();

    /** Provides access to the document bookmark interface
     */
    const IDocumentMarkAccess* getIDocumentMarkAccess() const;
          IDocumentMarkAccess* getIDocumentMarkAccess();

    /** Provides access to the document draw model interface
     */
    const IDocumentDrawModelAccess* getIDocumentDrawModelAccess() const;
          IDocumentDrawModelAccess* getIDocumentDrawModelAccess();

    /** Provides access to the document redline interface
     */
    const IDocumentRedlineAccess* getIDocumentRedlineAccess() const;
          IDocumentRedlineAccess* getIDocumentRedlineAccess();

    /** Provides access to the document layout interface
     */
    const IDocumentLayoutAccess* getIDocumentLayoutAccess() const;
          IDocumentLayoutAccess* getIDocumentLayoutAccess();

    /** Provides access to the content operations interface
     */
    IDocumentContentOperations* getIDocumentContentOperations();

    /** Provides access to the document style pool interface
     */
    IDocumentStylePoolAccess* getIDocumentStylePoolAccess();

    /** Provides access to the document statistics interface
     */
    const IDocumentStatistics* getIDocumentStatistics() const;

    /** Provides access to the document undo/redo interface
     */
    IDocumentUndoRedo const& GetIDocumentUndoRedo() const;
    IDocumentUndoRedo      & GetIDocumentUndoRedo();

    const IDocumentListItems* getIDocumentListItemsAccess() const;
    const IDocumentOutlineNodes* getIDocumentOutlineNodesAccess() const;

    // 1. GetRefDev:   Either the printer or the virtual device from the doc
    // 2. GetWin:      Available if we not printing
    // 3. GetOut:      Printer, Window or Virtual device
    OutputDevice& GetRefDev() const;
    inline Window* GetWin()    const { return pWin; }
    inline OutputDevice* GetOut()     const { return pOut; }

    static inline sal_Bool IsLstEndAction() { return ViewShell::bLstAct; }

    // Change of all page descriptors.
    void   ChgAllPageOrientation( sal_uInt16 eOri );
    void   ChgAllPageSize( Size &rSz );

    // Printing of one page.
    // bIsPDFExport == true is: do PDF Export (no printing!)
    sal_Bool PrintOrPDFExport( OutputDevice *pOutDev,
            SwPrintData const& rPrintData,
            sal_Int32 nRenderer /* offset in vector of pages to print */ );

    // Printing of one brochure page.
    void PrintProspect( OutputDevice *pOutDev, const SwPrintData &rPrintData,
            sal_Int32 nRenderer /* offset in vector of page pairs for prospect printing */ );

    // Printing for OLE 2.0.
    static void PrtOle2( SwDoc *pDoc, const SwViewOption *pOpt, const SwPrintData& rOptions,
                         OutputDevice* pOleOut, const Rectangle& rRect );

    // Fill temporary doc with selected text for Print or PDF export.
    SwDoc * FillPrtDoc( SwDoc* pPrtDoc, const SfxPrinter* pPrt );

    // Called internally for Shell. Formats pages.
    void CalcPagesForPrint( sal_uInt16 nMax );

    // All about fields.
    void UpdateFlds(sal_Bool bCloseDB = sal_False);
    sal_Bool IsAnyFieldInDoc() const;
    // Update all charts, for that exists any table.
    void UpdateAllCharts();
    sal_Bool HasCharts() const;

    // DOCUMENT COMPATIBILITY FLAGS START

    // Add or maximize paragraph spacing?
    void SetParaSpaceMax( bool bNew );

    // Add or maximize paragraph spacing?
    void SetParaSpaceMaxAtPages( bool bNew );

    // Compatible behaviour of tabs.
    void SetTabCompat( bool bNew );

    // Font metric attribute "External Leading" should be considered.
    void SetAddExtLeading( bool bNew );

    // Formatting by virtual device or printer.
    void SetUseVirDev( bool nNew );

    // Adding paragraph and table spacing at bottom
    // of table cells.
    void SetAddParaSpacingToTableCells( bool _bAddParaSpacingToTableCells );

    // Former formatting of text lines with
    // proportional line spacing or not.
    void SetUseFormerLineSpacing( bool _bUseFormerLineSpacing );

    // Former object positioning.
    void SetUseFormerObjectPositioning( bool _bUseFormerObjPos );

    void SetConsiderWrapOnObjPos( bool _bConsiderWrapOnObjPos );

    void SetUseFormerTextWrapping( bool _bUseFormerTextWrapping );

    void SetDoNotJustifyLinesWithManualBreak( bool _bDoNotJustifyLinesWithManualBreak );

    // DOCUMENT COMPATIBILITY FLAGS END

    // Calls Idle-formatter of Layout.
    void LayoutIdle();

    inline const SwViewOption *GetViewOptions() const { return pOpt; }
    virtual void  ApplyViewOptions( const SwViewOption &rOpt );
           void  SetUIOptions( const SwViewOption &rOpt );
    virtual void  SetReadonlyOption(sal_Bool bSet);          // Set readonly-bit of ViewOptions.
           void  SetPDFExportOption(sal_Bool bSet);         // Set/reset PDF export mode.
           void  SetPrtFormatOption(sal_Bool bSet);         // Set PrtFormat-Bit of ViewOptions.
           void  SetReadonlySelectionOption(sal_Bool bSet); // Change the selection mode in readonly docs.

    const SwAccessibilityOptions* GetAccessibilityOptions() const { return pAccOptions;}

    static void           SetShellRes( ShellResource* pRes ) { pShellRes = pRes; }
    static ShellResource* GetShellRes();

    static void           SetCareWin( Window* pNew );
    static Window*        GetCareWin(ViewShell& rVSh)
                          { return pCareWindow ? pCareWindow : CareChildWin(rVSh); }
    static Window*        CareChildWin(ViewShell& rVSh);

    inline SfxViewShell   *GetSfxViewShell() { return pSfxViewShell; }
    inline void           SetSfxViewShell(SfxViewShell *pNew) { pSfxViewShell = pNew; }

    // Selection of  Draw ::com::sun::star::script::Engine has been changed.
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

    sal_Bool IsViewLocked() const { return bViewLocked; }
    void LockView( sal_Bool b )   { bViewLocked = b;    }

    inline void LockPaint();
           void ImplLockPaint();
    inline void UnlockPaint( sal_Bool bVirDev = sal_False );
           void ImplUnlockPaint( sal_Bool bVirDev );
           sal_Bool IsPaintLocked() const { return nLockPaint != 0; }

    // Get/set DrawView and PageView.
    sal_Bool HasDrawView() const;
    void MakeDrawView();

    // DrawView may be used at UI.
          SdrView *GetDrawView();
    const SdrView *GetDrawView() const { return ((ViewShell*)this)->GetDrawView(); }

    // Take care that MarkList is up-to-date in any case (Bug 57153).
    SdrView *GetDrawViewWithValidMarkList();

    // Query attribute pool.
    inline const SfxItemPool& GetAttrPool() const;
                 SfxItemPool& GetAttrPool();

    sal_Bool IsPreView() const { return bPreView; }

    sal_Bool IsFrameView()  const { return bFrameView; }
    void SetFrameView( const Size& rBrowseBorder )
           { bFrameView = sal_True; aBrowseBorder = rBrowseBorder; }

    // Makes the necessary invalidations:
    // If BrowseMode has changed bBrowseChgd == sal_True
    // or bBrowseChgd == sal_False in BrowseMode
    // if proportions have changed.
    void CheckBrowseView( sal_Bool bBrowseChgd );

    const Size& GetBrowseBorder() const;
    sal_Int32 GetBrowseWidth() const;
    void SetBrowseBorder( const Size& rNew );

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >
            CreateAccessiblePreview();

    void ShowPreViewSelection( sal_uInt16 nSelPage );
    void InvalidateAccessibleFocus();

    // Apply Accessiblity options.
    void ApplyAccessiblityOptions(SvtAccessibilityOptions& rAccessibilityOptions);

    /** invalidate CONTENT_FLOWS_FROM/_TO relation for paragraphs

        @author OD

        @param _pFromTxtFrm
        input parameter - paragraph frame, for which the relation CONTENT_FLOWS_FROM
        has to be invalidated.
        If NULL, no CONTENT_FLOWS_FROM relation has to be invalidated

        @param _pToTxtFrm
        input parameter - paragraph frame, for which the relation CONTENT_FLOWS_TO
        has to be invalidated.
        If NULL, no CONTENT_FLOWS_TO relation has to be invalidated
    */
    void InvalidateAccessibleParaFlowRelation( const SwTxtFrm* _pFromTxtFrm,
                                               const SwTxtFrm* _pToTxtFrm );

    /** invalidate text selection for paragraphs

        @author OD
    */
    void InvalidateAccessibleParaTextSelection();

    /** invalidate attributes for paragraphs and paragraph's characters

        usage also for changes of the attributes of
        paragraph's characters.

        @author OD

        @param rTxtFrm
        input parameter - paragraph frame, whose attributes have changed
    */
    void InvalidateAccessibleParaAttrs( const SwTxtFrm& rTxtFrm );

    SwAccessibleMap* GetAccessibleMap();

    ViewShell( ViewShell&, Window *pWin = 0, OutputDevice *pOut = 0,
                long nFlags = 0 );
    ViewShell( SwDoc& rDoc, Window *pWin,
               const SwViewOption *pOpt = 0, OutputDevice *pOut = 0,
               long nFlags = 0 );
    virtual ~ViewShell();

    sal_Int32 GetPageNumAndSetOffsetForPDF( OutputDevice& rOut, const SwRect& rRect ) const;

    inline bool IsInConstructor() const { return mbInConstructor; }

    static const BitmapEx& GetReplacementBitmap( bool bIsErrorState );
    static void DeleteReplacementBitmaps();

    const SwPostItMgr* GetPostItMgr() const { return (const_cast<ViewShell*>(this))->GetPostItMgr(); }
    SwPostItMgr* GetPostItMgr();

    /// Acts both for headers / footers, depending on the bShow(Header|Footer)Separator flags
    void ToggleHeaderFooterEdit();
    /// Acts both for headers / footers, depending on the bShow(Header|Footer)Separator flags
    bool IsHeaderFooterEdit() const { return bHeaderFooterEdit; }
    bool IsShowHeaderFooterSeparator( FrameControlType eControl ) { return (eControl == Header)? bShowHeaderSeparator: bShowFooterSeparator; }
    virtual void SetShowHeaderFooterSeparator( FrameControlType eControl, bool bShow ) { if ( eControl == Header ) bShowHeaderSeparator = bShow; else bShowFooterSeparator = bShow; }
};

//---- class CurrShell manages global ShellPointer -------------------

class CurrShell
{
public:
    ViewShell *pPrev;
    SwRootFrm *pRoot;

    CurrShell( ViewShell *pNew );
    ~CurrShell();
};

inline void ViewShell::ResetInvalidRect()
{
   aInvalidRect.Clear();
}

inline void ViewShell::StartAction()
{
    if ( !nStartAction++ )
        ImplStartAction();
}
inline void ViewShell::EndAction( const sal_Bool bIdleEnd )
{
    if( 0 == (nStartAction - 1) )
        ImplEndAction( bIdleEnd );
    --nStartAction;
}

inline void ViewShell::LockPaint()
{
    if ( !nLockPaint++ )
        ImplLockPaint();
}
inline void ViewShell::UnlockPaint( sal_Bool bVirDev )
{
    if ( 0 == --nLockPaint )
        ImplUnlockPaint( bVirDev );
}
inline const SfxItemPool& ViewShell::GetAttrPool() const
{
    return ((ViewShell*)this)->GetAttrPool();
}



#endif // SW_VIEWSH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
