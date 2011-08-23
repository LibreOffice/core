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
#ifndef _VIEWSH_HXX
#define _VIEWSH_HXX

#include <tools/rtti.hxx>

#include <ring.hxx>
#include <swrect.hxx>
/*N*/ #include <tools/debug.hxx> //for stripping
#ifdef ACCESSIBLE_LAYOUT
namespace com { namespace sun { namespace star { namespace accessibility {
           class XAccessible; } } } }
#endif
class VirtualDevice;
class Window;
class OutputDevice;
class SvtAccessibilityOptions;
class Fraction;
class SvEmbeddedObjectRef;

namespace binfilter {

class SfxItemPool;
class SwDoc;
class SfxPrinter;

class SfxProgress;
class SwRootFrm;
class SwNodes;
class SdrView;

class SfxViewShell;
class SwViewOption;
class SwViewImp;
class SwPrtOptions;
class SwPagePreViewPrtData;
class SwLayIdle;
struct ShellResource;
class SwRegionRects;
class SwFrm;
struct SwPrintData;
// OD 12.12.2002 #103492#
class SwPagePreviewLayout;

struct SwAccessibilityOptions;


//JP 19.07.98: - Bug 52312
// define fuer Flags, die im CTOR oder den darunter liegenden Schichten
// benoetigt werden.
// Zur Zeit wird fuer die DrawPage das PreView Flag benoetigt
#define VSHELLFLAG_ISPREVIEW 			((long)0x1)

class ViewShell : public Ring
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

    static sal_Bool bLstAct;			// sal_True wenn Das EndAction der letzten Shell
                                    // laeuft; also die EndActions der
                                    // anderen Shells auf das Dokument
                                    // abgearbeitet sind.

    Point		  aPrtOffst;		 //Ofst fuer den Printer,
                                     //nicht bedruckbarer Rand.
     Size		  aBrowseBorder;	//Rand fuer Framedokumente
    SwRect 		  aInvalidRect;

    SfxViewShell *pSfxViewShell;
    SwViewImp 	 *pImp;				//Core-Interna der ViewShell.
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


    sal_Bool  bDocSizeChgd	   :1;	//Fuer DocChgNotify(): Neue DocGroesse bei
                                //EndAction an das DocMDI melden.
    sal_Bool  bPaintWorks	   :1;	//Normal Painten wenn sal_True,
                                //Paint merken wenn sal_False
    sal_Bool  bPaintInProgress :1;	//Kein zweifaches Paint durchlassen.
    sal_Bool  bViewLocked	   :1;	//Lockt den sichtbaren Bereich,
                                //MakeVisible laeuft dann in's leere.
    sal_Bool  bInEndAction	   :1;  //Fiese unstaende vermeiden, siehe viewsh.cxx
    sal_Bool  bPreView		   :1;	//Ist sal_True wenns eine PreView-ViewShell ist.
    sal_Bool  bFrameView	   :1;  //sal_True wenn es ein (HTML-)Frame ist.
    sal_Bool  bEnableSmooth    :1;	//Disable des SmoothScroll z.B. fuer
                                //Drag der Scrollbars.
    sal_Bool  bEndActionByVirDev:1;	//Paints aus der EndAction immer ueber virtuelles

                                //Device (etwa beim Browsen)

    //Initialisierung, wird von den verschiedenen Konstruktoren gerufen.
    void Init( const SwViewOption *pNewOpt );

    inline void ResetInvalidRect();


    void Reformat();			//Invalidert das ges. Layout (ApplyViewOption)

                                                // Malen der Wiese und rufen
    // PaintDesktop gesplittet, dieser Teil wird auch von PreViewPage benutzt

    sal_Bool CheckInvalidForPaint( const SwRect & );//Direkt Paint oder lieber
                                                //eine Aktion ausloesen.

    void Scroll();	//Scrollen wenn sich aus der LayAction Scrollmoeglichkeiten
                    //ergaben.



protected:
    static ShellResource*	pShellRes;		// Resourcen fuer die Shell
    static Window*			pCareWindow;	// diesem Fenster ausweichen

    SwRect					aVisArea;		//Die moderne Ausfuerung der VisArea
    SwDoc					*pDoc;			//Das Dokument, niemals 0

    sal_uInt16 nStartAction; //ist != 0 wenn mindestens eine ::com::sun::star::chaos::Action laeuft
    sal_uInt16 nLockPaint;	 //ist != 0 wenn das Paint gelocked ist.

public:
    TYPEINFO();

          SwViewImp *Imp() { return pImp; }
    const SwViewImp *Imp() const { return pImp; }

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

    void SetEndActionByVirDev( sal_Bool b )	{ bEndActionByVirDev = b; }
    sal_Bool IsEndActionByVirDev()			{ return bEndActionByVirDev; }

    // 	Per UNO wird am RootFrame fuer alle shells der ActionCount kurzfristig
    //  auf Null gesetzt und wieder restauriert

    inline sal_Bool HasInvalidRect() const { return aInvalidRect.HasArea(); }
    void ChgHyphenation() {	Reformat(); }
    void ChgNumberDigits() { Reformat(); }

    //Methoden fuer Paint- und Scrollrects, die auf allen Shells im
    //Ring arbeiten.
    sal_Bool AddPaintRect( const SwRect &rRect );
    void AddScrollRect( const SwFrm *pFrm, const SwRect &rRect, long nOffs );
    void SetNoNextScroll();

    void InvalidateWindows( const SwRect &rRect );
    sal_Bool IsPaintInProgress() const { return bPaintInProgress; }

    //Benachrichtung, dass sich der sichtbare Bereich geaendert hat.
    //VisArea wird neu gesetzt, anschliessend wird gescrollt.
    //Das uebergebene Rect liegt auf Pixelgrenzen,
    //um Pixelfehler beim Scrollen zu vermeiden.
    void EnableSmooth( sal_Bool b ) { bEnableSmooth = b; }

    const SwRect &VisArea() const { return aVisArea; }
        //Es wird, wenn notwendig, soweit gescrollt, dass das
        //uebergebene Rect im sichtbaren Ausschnitt liegt.
    void MakeVisible( const SwRect & );

    //Bei naechster Gelegenheit die neue Dokuemntgroesse an das UI weiterreichen.
    void SizeChgNotify(const Size &);
    void UISizeNotify();			//Das weiterreichen der aktuellen groesse.



    //Invalidierung der ersten Sichtbaren Seite fuer alle Shells im Ring.
    void SetFirstVisPageInvalid();

    SwRootFrm	*GetLayout() const;
                                      //erzeugt?

    void CalcLayout();	//Durchformatierung des Layouts erzwingen.

    inline SwDoc *GetDoc()	const { return pDoc; }	//niemals 0.

    // 1. GetPrt:      The printer at the document
    // 3. GetRefDev:   Either the printer or the virtual device from the doc
    // 2. GetWin:      Available if we not printing
    // 4. GetOut:      Printer, Window or Virtual device
    SfxPrinter* GetPrt( sal_Bool bCreate = sal_False ) const;
    VirtualDevice* GetVirDev( sal_Bool bCreate = sal_False ) const;
    OutputDevice& GetRefDev() const;
    inline Window* GetWin()    const { return pWin; }
    inline OutputDevice* GetOut()     const { return pOut; }

    static inline sal_Bool IsLstEndAction() { return ViewShell::bLstAct; }

    // Setzt Drucker fuer ALLE Sichten im Ring; einschl. Invalidierungen
    void SetVirDev( VirtualDevice* );

    //Andern alle PageDescriptoren

    // creates temporary doc with selected text for PDF export
    SwDoc * CreatePrtDoc( SfxPrinter* pPrt, SvEmbeddedObjectRef &rDocShellRef );

    //Wird intern fuer die Shell gerufen die Druckt. Formatiert die Seiten.

    //All about fields.
    // update all charts, for that exists any table

    // Sollen Absatzabstaende addiert oder maximiert werden?

    // compatible behaviour of tabs

    //formatting by virtual device or printer
    sal_Bool IsUseVirtualDevice()const;
    void SetUseVirtualDevice(sal_Bool bSet);

    //Ruft den Idle-Formatierer des Layouts
    void LayoutIdle();

    inline const SwViewOption *GetViewOptions() const { return pOpt; }
    void  ApplyViewOptions( const SwViewOption &rOpt ){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 		   void  ApplyViewOptions( const SwViewOption &rOpt );

    //static void           SetShellRes( ShellResource* pRes ) { pShellRes = pRes; }
    static ShellResource* GetShellRes();

    static void 		  SetCareWin( Window* pNew ) { pCareWindow = pNew; }
    static Window* 		  GetCareWin(ViewShell& rVSh)
                        { return pCareWindow ? pCareWindow : CareChildWin(rVSh); }
    static Window* 		  CareChildWin(ViewShell& rVSh){DBG_BF_ASSERT(0, "STRIP"); return NULL;} ;//STRIP001 	static Window* 		  CareChildWin(ViewShell& rVSh);

    inline SfxViewShell   *GetSfxViewShell() { return pSfxViewShell; }
    inline void 		  SetSfxViewShell(SfxViewShell *pNew) { pSfxViewShell = pNew; }

    // Selektion der Draw ::com::sun::star::script::Engine geaendert
    virtual void DrawSelChanged(SdrView*);

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

    sal_Bool IsViewLocked() const { return bViewLocked; }
    void LockView( sal_Bool b )	  { bViewLocked = b;	}

           sal_Bool IsPaintLocked() const { return nLockPaint != 0; }

    // Abfragen/Erzeugen DrawView + PageView
    sal_Bool HasDrawView() const;
    void MakeDrawView();

    //DrawView darf u.U. am UI benutzt werden.
          SdrView *GetDrawView();
    const SdrView *GetDrawView() const { return ((ViewShell*)this)->GetDrawView(); }

    //sorge dafuer, das auf jedenfall die MarkListe aktuell ist (Bug 57153)

    sal_Bool IsPreView() const { return bPreView; }

    sal_Bool IsFrameView()  const { return bFrameView; }
    void SetFrameView( const Size& rBrowseBorder )
        { bFrameView = sal_True; aBrowseBorder = rBrowseBorder; }

    //Damit in der UI nicht ueberall das dochxx includet werden muss
    sal_Bool IsBrowseMode() const;


    const Size& GetBrowseBorder() const{ return aBrowseBorder; }

    virtual ~ViewShell();

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

} //namespace binfilter
#endif //_VIEWSH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
