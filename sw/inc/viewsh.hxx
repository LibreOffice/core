/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewsh.hxx,v $
 *
 *  $Revision: 1.61 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-19 13:35:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _VIEWSH_HXX
#define _VIEWSH_HXX
#ifndef _COM_SUN_STAR_EMBED_XCLASSIFIEDOBJECT_HPP_
#include <com/sun/star/embed/XClassifiedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif

#ifndef _RTTI_HXX //autogen
#include <tools/rtti.hxx>
#endif
#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _RING_HXX
#include <ring.hxx>
#endif
#ifndef _SWRECT_HXX
#include <swrect.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

namespace com { namespace sun { namespace star { namespace accessibility {
           class XAccessible; } } } }

class SfxObjectShellRef;
class SwDoc;
class IDocumentSettingAccess;
class IDocumentDeviceAccess;
class IDocumentBookmarkAccess;
class IDocumentDrawModelAccess;
class IDocumentRedlineAccess;
class IDocumentLayoutAccess;
class IDocumentFieldsAccess;
class IDocumentContentOperations;
class IDocumentStylePoolAccess;
class IDocumentStatistics;
class IDocumentUndoRedo;
class SfxPrinter;
class SfxProgress;
class SwRootFrm;
class SwNodes;
class SdrView;
class SfxItemPool;
class SfxViewShell;
class SwViewOption;
class SwViewImp;
class SwPrtOptions;
class SwPagePreViewPrtData;
class Window;
class OutputDevice;
class SwLayIdle;
struct ShellResource;
class SwRegionRects;
class SwFrm;
class SvtAccessibilityOptions;
// OD 12.12.2002 #103492#
class SwPagePreviewLayout;
// --> OD 2005-12-01 #i27138#
class SwTxtFrm;
// <--
class BitmapEx;

struct SwAccessibilityOptions;
class Region;

// #i74769#
class SdrPaintWindow;

//JP 19.07.98: - Bug 52312
// define fuer Flags, die im CTOR oder den darunter liegenden Schichten
// benoetigt werden.
// Zur Zeit wird fuer die DrawPage das PreView Flag benoetigt
#define VSHELLFLAG_ISPREVIEW            ((long)0x1)

class SW_DLLPUBLIC ViewShell : public Ring
{
    friend void SetOutDev( ViewShell *pSh, OutputDevice *pOut );
    friend void SetOutDevAndWin( ViewShell *pSh, OutputDevice *pOut,
                                 Window *pWin, sal_uInt16 nZoom );

    friend class SwViewImp;
    friend class SwLayIdle;

    // OD 12.12.2002 #103492# - for setting visible area for page preview paint
    friend class SwPagePreviewLayout;

    //Umsetzen der SwVisArea, damit vor dem Drucken sauber formatiert
    //werden kann.
    friend void SetSwVisArea( ViewShell *pSh, const SwRect &, BOOL bPDFExport = FALSE );

    // --> PB 2007-05-30 #146850#
    static BitmapEx*    pReplaceBmp;    // replaced display of still loaded images
    static BitmapEx*    pErrorBmp;      // error display of missed images
    // <--

    static sal_Bool bLstAct;            // sal_True wenn Das EndAction der letzten Shell
                                    // laeuft; also die EndActions der
                                    // anderen Shells auf das Dokument
                                    // abgearbeitet sind.

    Point         aPrtOffst;         //Ofst fuer den Printer,
                                     //nicht bedruckbarer Rand.
     Size         aBrowseBorder;    //Rand fuer Framedokumente
    SwRect        aInvalidRect;

    SfxViewShell *pSfxViewShell;
    SwViewImp    *pImp;             //Core-Interna der ViewShell.
                                    //Der Pointer ist niemals 0.

    Window       *pWin;              // = 0 during printing or pdf export
    OutputDevice *pOut;              // Window, Printer, VirtDev, ...
    OutputDevice* mpTmpRef;           // Temporariy reference device. Is used
                                     // during (printer depending) prospect
                                     // and page preview printing
                                     // (because a scaling has to be set at
                                     // the original printer)

    SwViewOption *pOpt;
    SwAccessibilityOptions* pAccOptions;


    sal_Bool  bDocSizeChgd     :1;  //Fuer DocChgNotify(): Neue DocGroesse bei
                                //EndAction an das DocMDI melden.
    sal_Bool  bPaintWorks      :1;  //Normal Painten wenn sal_True,
                                //Paint merken wenn sal_False
    sal_Bool  bPaintInProgress :1;  //Kein zweifaches Paint durchlassen.
    sal_Bool  bViewLocked      :1;  //Lockt den sichtbaren Bereich,
                                //MakeVisible laeuft dann in's leere.
    sal_Bool  bInEndAction     :1;  //Fiese unstaende vermeiden, siehe viewsh.cxx
    sal_Bool  bPreView         :1;  //Ist sal_True wenns eine PreView-ViewShell ist.
    sal_Bool  bFrameView       :1;  //sal_True wenn es ein (HTML-)Frame ist.
    sal_Bool  bEnableSmooth    :1;  //Disable des SmoothScroll z.B. fuer
                                //Drag der Scrollbars.
    sal_Bool  bEndActionByVirDev:1; //Paints aus der EndAction immer ueber virtuelles

                                //Device (etwa beim Browsen)

    // OD 2004-06-01 #i26791# - boolean, indicating that class in in constructor
    bool mbInConstructor:1;

    // #i74769#
    SdrPaintWindow*         mpTargetPaintWindow;
    OutputDevice*           mpBufferedOut;

    //Initialisierung, wird von den verschiedenen Konstruktoren gerufen.
    SW_DLLPRIVATE void Init( const SwViewOption *pNewOpt );

    inline void ResetInvalidRect();

    SW_DLLPRIVATE void Reformat();          //Invalidert das ges. Layout (ApplyViewOption)

    SW_DLLPRIVATE void PaintDesktop( const SwRect & );      // sammeln der Werte fuers
                                                // Malen der Wiese und rufen
    // PaintDesktop gesplittet, dieser Teil wird auch von PreViewPage benutzt
    SW_DLLPRIVATE void _PaintDesktop( const SwRegionRects &rRegion );

    SW_DLLPRIVATE sal_Bool CheckInvalidForPaint( const SwRect & );//Direkt Paint oder lieber
                                                //eine Aktion ausloesen.

    SW_DLLPRIVATE void Scroll();    //Scrollen wenn sich aus der LayAction Scrollmoeglichkeiten
                    //ergaben.

    SW_DLLPRIVATE void PrepareForPrint( const SwPrtOptions &rOptions );

    SW_DLLPRIVATE void ImplApplyViewOptions( const SwViewOption &rOpt );

protected:
    static ShellResource*   pShellRes;      // Resourcen fuer die Shell
    static Window*          pCareWindow;    // diesem Fenster ausweichen

    SwRect                  aVisArea;       //Die moderne Ausfuerung der VisArea
    SwDoc                   *pDoc;          //Das Dokument, niemals 0

    sal_uInt16 nStartAction; //ist != 0 wenn mindestens eine ::com::sun::star::chaos::Action laeuft
    sal_uInt16 nLockPaint;   //ist != 0 wenn das Paint gelocked ist.

public:
    TYPEINFO();

          SwViewImp *Imp() { return pImp; }
    const SwViewImp *Imp() const { return pImp; }

    const SwNodes& GetNodes() const;

    //Nach Druckerwechsel, vom Doc
    //pPDFOut != NULL is used for PDF export.
    void            InitPrt( SfxPrinter * , OutputDevice *pPDFOut = NULL );

    //Klammerung von zusammengehoerenden Aktionen.
    inline void StartAction();
           void ImplStartAction();
    inline void EndAction( const sal_Bool bIdleEnd = sal_False );
           void ImplEndAction( const sal_Bool bIdleEnd = sal_False );
    sal_uInt16 ActionCount() const { return nStartAction; }
    sal_Bool ActionPend() const { return nStartAction != 0; }
    sal_Bool IsInEndAction() const { return bInEndAction; }

    void SetEndActionByVirDev( sal_Bool b ) { bEndActionByVirDev = b; }
    sal_Bool IsEndActionByVirDev()          { return bEndActionByVirDev; }

    //  Per UNO wird am RootFrame fuer alle shells der ActionCount kurzfristig
    //  auf Null gesetzt und wieder restauriert
    void    SetRestoreActions(sal_uInt16 nSet);
    sal_uInt16  GetRestoreActions() const;

    inline sal_Bool HasInvalidRect() const { return aInvalidRect.HasArea(); }
    void ChgHyphenation() { Reformat(); }
    void ChgNumberDigits() { Reformat(); }

    //Methoden fuer Paint- und Scrollrects, die auf allen Shells im
    //Ring arbeiten.
    sal_Bool AddPaintRect( const SwRect &rRect );
    void AddScrollRect( const SwFrm *pFrm, const SwRect &rRect, long nOffs );
    void SetNoNextScroll();

    void InvalidateWindows( const SwRect &rRect );

    //////////////////////////////////////////////////////////////////////////////
    // #i72754# set of Pre/PostPaints with lock counter and initial target OutDev
protected:
    sal_uInt32              mnPrePostPaintCount;
    OutputDevice*           mpPrePostOutDev;
public:
    void DLPrePaint2(const Region& rRegion);
    void DLPostPaint2();
    //////////////////////////////////////////////////////////////////////////////

    virtual void Paint(const Rectangle &rRect);
    sal_Bool IsPaintInProgress() const { return bPaintInProgress; }

    //Benachrichtung, dass sich der sichtbare Bereich geaendert hat.
    //VisArea wird neu gesetzt, anschliessend wird gescrollt.
    //Das uebergebene Rect liegt auf Pixelgrenzen,
    //um Pixelfehler beim Scrollen zu vermeiden.
    virtual void VisPortChgd( const SwRect & );
    sal_Bool SmoothScroll( long lXDiff, long lYDiff, const Rectangle* );//Browser
    void EnableSmooth( sal_Bool b ) { bEnableSmooth = b; }

    const SwRect &VisArea() const { return aVisArea; }
        //Es wird, wenn notwendig, soweit gescrollt, dass das
        //uebergebene Rect im sichtbaren Ausschnitt liegt.
    void MakeVisible( const SwRect & );

    //Bei naechster Gelegenheit die neue Dokuemntgroesse an das UI weiterreichen.
    void SizeChgNotify();
    void UISizeNotify();            //Das weiterreichen der aktuellen groesse.

    Point GetPagePos( sal_uInt16 nPageNum ) const;

    sal_uInt16 GetNumPages();   //Anzahl der aktuellen Seiten Layout erfragen.
    sal_Bool   IsDummyPage( USHORT nPageNum ) const;  // An empty page?

    //Invalidierung der ersten Sichtbaren Seite fuer alle Shells im Ring.
    void SetFirstVisPageInvalid();

    SwRootFrm   *GetLayout() const;
    sal_Bool         IsNewLayout() const; //Wurde das Layout geladen oder neu
                                      //erzeugt?

     Size GetDocSize() const;// erfrage die Groesse des Dokuments

    void CalcLayout();  //Durchformatierung des Layouts erzwingen.

    inline SwDoc *GetDoc()  const { return pDoc; }  //niemals 0.

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
    const IDocumentBookmarkAccess* getIDocumentBookmarkAccess() const;
          IDocumentBookmarkAccess* getIDocumentBookmarkAccess();

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

    /** Provides access to the document fields administration interface
     */
    const IDocumentFieldsAccess* getIDocumentFieldsAccess() const;

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
    IDocumentUndoRedo* getIDocumentUndoRedoAccess();

    // 1. GetRefDev:   Either the printer or the virtual device from the doc
    // 2. GetWin:      Available if we not printing
    // 3. GetOut:      Printer, Window or Virtual device
    OutputDevice& GetRefDev() const;
    inline Window* GetWin()    const { return pWin; }
    inline OutputDevice* GetOut()     const { return pOut; }

    static inline sal_Bool IsLstEndAction() { return ViewShell::bLstAct; }

    //Andern alle PageDescriptoren
    void   ChgAllPageOrientation( sal_uInt16 eOri );
    void   ChgAllPageSize( Size &rSz );

    //Druckauftrag abwickeln.
    // pPDFOut != Null is: do PDF Export (no printing!)
    sal_Bool Prt( SwPrtOptions& rOptions, SfxProgress* pProgress,
                  OutputDevice* pPDFOut = NULL );

    //"Drucken" fuer OLE 2.0
    static void PrtOle2( SwDoc *pDoc, const SwViewOption *pOpt, SwPrtOptions& rOptions,
                         OutputDevice* pOleOut, const Rectangle& rRect );

    // creates temporary doc with selected text for PDF export
    SwDoc * CreatePrtDoc( SfxPrinter* pPrt, SfxObjectShellRef& );
    SwDoc * FillPrtDoc( SwDoc* pPrtDoc, const SfxPrinter* pPrt );

    //Wird intern fuer die Shell gerufen die Druckt. Formatiert die Seiten.
    void CalcPagesForPrint( sal_uInt16 nMax, SfxProgress* pProgress = 0,
        const String* pStr = NULL, ULONG nMergeAct = 0, ULONG nMergeCnt = 0 );

    //All about fields.
    void UpdateFlds(sal_Bool bCloseDB = sal_False);
    sal_Bool IsAnyFieldInDoc() const;
    // update all charts, for that exists any table
    void UpdateAllCharts();
    sal_Bool HasCharts() const;

    //
    // DOCUMENT COMPATIBILITY FLAGS START
    //

    // Sollen Absatzabstaende addiert oder maximiert werden?
    void SetParaSpaceMax( bool bNew );

    // Sollen Absatzabstaende addiert oder maximiert werden?
    void SetParaSpaceMaxAtPages( bool bNew );

    // compatible behaviour of tabs
    void SetTabCompat( bool bNew );

    //#i24363# tab stops relative to indent
    void SetTabsRelativeToIndent( bool bNew );

    // font metric attribute "External Leading" should be considered
    void SetAddExtLeading( bool bNew );

    // formatting by virtual device or printer
    void SetUseVirDev( bool nNew );

    // OD 2004-02-16 #106629# - adding paragraph and table spacing at bottom
    // of table cells
    void SetAddParaSpacingToTableCells( bool _bAddParaSpacingToTableCells );

    // OD 06.01.2004 #i11859# - former formatting of text lines with
    // proportional line spacing or not
    void SetUseFormerLineSpacing( bool _bUseFormerLineSpacing );

    // OD 2004-03-12 #i11860# - former object positioning
    void SetUseFormerObjectPositioning( bool _bUseFormerObjPos );

    // OD 2004-05-05 #i28701#
    void SetConsiderWrapOnObjPos( bool _bConsiderWrapOnObjPos );

    // --> FME #108724#
    void SetUseFormerTextWrapping( bool _bUseFormerTextWrapping );

    // -> PB 2007-06-11 #i45491#
    void SetDoNotJustifyLinesWithManualBreak( bool _bDoNotJustifyLinesWithManualBreak );
    // <--

    //
    // DOCUMENT COMPATIBILITY FLAGS END
    //

    //Ruft den Idle-Formatierer des Layouts
    void LayoutIdle();

    inline const SwViewOption *GetViewOptions() const { return pOpt; }
           void  ApplyViewOptions( const SwViewOption &rOpt );
           void  SetUIOptions( const SwViewOption &rOpt );
           void  SetReadonlyOption(sal_Bool bSet);   // Readonly-Bit d. ViewOptions setzen
           void  SetPDFExportOption(sal_Bool bSet);   // set/reset PDF export mode
           void  SetPrtFormatOption(sal_Bool bSet);  // PrtFormat-Bit d. ViewOptions setzen
           void  SetReadonlySelectionOption(sal_Bool bSet);//change the selection mode in readonly docs

    const SwAccessibilityOptions* GetAccessibilityOptions() const { return pAccOptions;}

    static void           SetShellRes( ShellResource* pRes ) { pShellRes = pRes; }
    static ShellResource* GetShellRes();

    static void           SetCareWin( Window* pNew );
    static Window*        GetCareWin(ViewShell& rVSh)
                          { return pCareWindow ? pCareWindow : CareChildWin(rVSh); }
    static Window*        CareChildWin(ViewShell& rVSh);

    inline SfxViewShell   *GetSfxViewShell() { return pSfxViewShell; }
    inline void           SetSfxViewShell(SfxViewShell *pNew) { pSfxViewShell = pNew; }

    // Selektion der Draw ::com::sun::star::script::Engine geaendert
    virtual void DrawSelChanged();

    // OD 12.12.2002 #103492#
    SwPagePreviewLayout* PagePreviewLayout();

    /** adjust view options for page preview

        OD 09.01.2003 #i6467#
        Because page preview should show the document as it is printed -
        page preview is print preview -, the view options are adjusted to the
        same as for printing.

        @param _rPrintOptions
        input parameter - constant reference to print options, to which the
        view option will be adjusted.
    */
    void AdjustOptionsForPagePreview( const SwPrtOptions &_rPrintOptions );

    // print page/print preview
    void PrintPreViewPage( SwPrtOptions& rOptions, sal_uInt16 nRowCol,
                           SfxProgress& rProgress,
                           const SwPagePreViewPrtData* = 0 );

    // Prospekt-Format drucken
    void PrintProspect( SwPrtOptions&, SfxProgress& , BOOL bRTL);

    sal_Bool IsViewLocked() const { return bViewLocked; }
    void LockView( sal_Bool b )   { bViewLocked = b;    }

    inline void LockPaint();
           void ImplLockPaint();
    inline void UnlockPaint( sal_Bool bVirDev = sal_False );
           void ImplUnlockPaint( sal_Bool bVirDev );
           sal_Bool IsPaintLocked() const { return nLockPaint != 0; }

    // Abfragen/Erzeugen DrawView + PageView
    const sal_Bool HasDrawView() const;
    void MakeDrawView();

    //DrawView darf u.U. am UI benutzt werden.
          SdrView *GetDrawView();
    const SdrView *GetDrawView() const { return ((ViewShell*)this)->GetDrawView(); }

    //sorge dafuer, das auf jedenfall die MarkListe aktuell ist (Bug 57153)
    SdrView *GetDrawViewWithValidMarkList();

    // erfrage den Attribut Pool
    inline const SfxItemPool& GetAttrPool() const;
                 SfxItemPool& GetAttrPool();

    sal_Bool IsPreView() const { return bPreView; }

    sal_Bool IsFrameView()  const { return bFrameView; }
    void SetFrameView( const Size& rBrowseBorder )
           { bFrameView = sal_True; aBrowseBorder = rBrowseBorder; }

    //Nimmt die notwendigen Invalidierungen vor,
    //wenn sich der BrowdseModus aendert, bBrowseChgd == sal_True
    //oder, im BrowseModus, wenn sich die Groessenverhaeltnisse
    //aendern (bBrowseChgd == sal_False)
    void CheckBrowseView( BOOL bBrowseChgd );

    const Size& GetBrowseBorder() const;
    sal_Int32 GetBrowseWidth() const;
    void SetBrowseBorder( const Size& rNew );

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    // OD 15.01.2003 #103492# - change method signature due to new page preview
    // functionality.
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >
            CreateAccessiblePreview();

    void ShowPreViewSelection( sal_uInt16 nSelPage );
    void InvalidateAccessibleFocus();

    //apply Accessiblity options
    void ApplyAccessiblityOptions(SvtAccessibilityOptions& rAccessibilityOptions);

    /** invalidate CONTENT_FLOWS_FROM/_TO relation for paragraphs

        OD 2005-12-01 #i27138#

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

        OD 2005-12-12 #i27301#

        @author OD
    */
    void InvalidateAccessibleParaTextSelection();

    ViewShell( ViewShell&, Window *pWin = 0, OutputDevice *pOut = 0,
                long nFlags = 0 );
    ViewShell( SwDoc& rDoc, Window *pWin,
               const SwViewOption *pOpt = 0, OutputDevice *pOut = 0,
               long nFlags = 0 );
    virtual ~ViewShell();

    // --> FME 2004-06-15 #i12836# enhanced pdf export
    sal_Int32 GetPageNumAndSetOffsetForPDF( OutputDevice& rOut, const SwRect& rRect ) const;
    // <--

    inline bool IsInConstructor() const { return mbInConstructor; }

    // --> PB 2007-05-30 #146850#
    static const BitmapEx& GetReplacementBitmap( bool bIsErrorState );
    static void DeleteReplacementBitmaps();
    // <--
};

//---- class CurrShell verwaltet den globalen ShellPointer -------------------

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



#endif //_VIEWSH_HXX
