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
#ifndef _EDTWIN_HXX
#define _EDTWIN_HXX

#include <tools/link.hxx>
#include <vcl/timer.hxx>
#include <vcl/window.hxx>
#include <swevent.hxx>

#define _SVSTDARR_STRINGSISORTDTOR
#include <bf_svtools/svstdarr.hxx>
namespace binfilter {

class	SwWrtShell;
class 	SwView;
class 	SwRect;
class 	SwDrawBase;
class	ViewShell;
class	SdrViewUserMarker;
class	SdrObject;
class	SwShadowCursor;
class	DataChangedEvent;
class   OfaAutoCorrCfg;
class   SvxAutoCorrect;
struct	SwApplyTemplate;
struct 	QuickHelpData;

/*--------------------------------------------------------------------
    Beschreibung:	Eingabe-Fenster
 --------------------------------------------------------------------*/

class SwEditWin: public Window
{
friend void 	ScrollMDI(ViewShell* pVwSh, const SwRect&,
                          USHORT nRangeX, USHORT nRangeY);
friend BOOL 	IsScrollMDI(ViewShell* pVwSh, const SwRect&);

friend void 	SizeNotify(ViewShell* pVwSh, const Size &);

friend void 	PageNumNotify( 	ViewShell* pVwSh,
                                USHORT nPhyNum,
                                USHORT nVirtNum,
                                const String& rPg );

    static  QuickHelpData* pQuickHlpData;

    static	BOOL	bReplaceQuote;
    static	long 	nDDStartPosX, nDDStartPosY;

    static 	Color	aTextColor; 	//Textfarbe, fuer die Giesskanne
    static 	BOOL	bTransparentBackColor; // Hintergrund transparent
    static 	Color	aTextBackColor; //Texthintergrundfarbe, fuer die Giesskanne

    /*
     * Timer und Handler fuer das Weiterscrollen, wenn der
     * Mauspointer innerhalb eines Drag-Vorgangs ausserhalb des
     * EditWin stehen bleibt.  In regelmaessigen Intervallen wird
     * die Selektion in Richtung der Mausposition vergroessert.
     */
    AutoTimer 		aTimer;
    // Timer fuer verschachtelte KeyInputs (z.B. fuer Tabellen)
    Timer 			aKeyInputTimer;
    // timer for ANY-KeyInut question without a following KeyInputEvent
    Timer 			aKeyInputFlushTimer;

    String			aInBuffer;
    Point			aStartPos;
    Point			aMovePos;
    Point			aRszMvHdlPt;
    Timer 			aTemplateTimer;

    // Type/Objecte ueber dem der MousePointer steht
    SwCallMouseEvent aSaveCallEvent;

    SwApplyTemplate 	*pApplyTempl;

    SdrViewUserMarker	*pUserMarker;
    SdrObject			*pUserMarkerObj;

    SwView		   &rView;

    int		 		aActHitType;	// aktueller Mauspointer

    ULONG 			nDropFormat;	//Format aus dem letzten QueryDrop
    USHORT			nDropAction;	//Action aus dem letzten QueryDrop
    USHORT			nDropDestination;	//Ziel aus dem letzten QueryDrop

    UINT16			eDrawMode;
    UINT16			eBezierMode;
    UINT16			nInsFrmColCount; //Spaltenzahl fuer interaktiven Rahmen
    BOOL			bLinkRemoved	: 1,
                    bMBPressed		: 1,
                    bInsDraw 		: 1,
                    bInsFrm 		: 1,
                    bIsInMove		: 1,
                    bIsInDrag		: 1, //StartExecuteDrag nich doppelt ausfuehren
                    bOldIdle		: 1, //Zum abschalten des Idle'ns
                    bOldIdleSet		: 1, //waehrend QeueryDrop
                    bTblInsDelMode	: 1, //
                    bTblIsInsMode	: 1, //
                    bTblIsColMode	: 1, //
                    bChainMode		: 1, //Rahmen verbinden
                    bWasShdwCrsr	: 1, //ShadowCrsr war im MouseButtonDown an
                    bLockInput		: 1; //Lock waehrend die Rechenleiste aktiv ist





    //Hilfsfunktionen fuer D&D

    /*
     * Handler fuer das Weiterscrollen, wenn der Mauspointer innerhalb eines
     * Drag-Vorgangs ausserhalb des EditWin stehen bleibt. In regelmaessigen
     * Intervallen wird die Selektion in Richtung der Mausposition
     * vergroessert.
     */
    DECL_LINK( TimerHandler, Timer * );

    // timer for ANY-KeyInut question without a following KeyInputEvent
    DECL_LINK( KeyInputFlushHandler, Timer * );

    // Timer fuer verschachtelte KeyInputs (z.B. fuer Tabellen)
    DECL_LINK( KeyInputTimerHandler, Timer * );

    // Timer fuer das ApplyTemplates per Maus (verkapptes Drag&Drop)
    DECL_LINK( TemplateTimerHdl, Timer* );

protected:






                                // Drag & Drop Interface

    void    ShowAutoTextCorrectQuickHelp( const String& rWord, OfaAutoCorrCfg* pACfg, SvxAutoCorrect* pACorr );
public:

    void			UpdatePointer(const Point &, USHORT nButtons = 0);

    BOOL			IsDrawAction() 					{ return (bInsDraw); }
    void			SetDrawAction(BOOL bFlag) 		{ bInsDraw = bFlag; }
    inline UINT16	GetDrawMode(BOOL bBuf = FALSE) const { return eDrawMode; }
    inline void		SetDrawMode(UINT16 eDrwMode)	{ eDrawMode = eDrwMode; }
    BOOL			IsFrmAction() 					{ return (bInsFrm); }
    inline UINT16	GetBezierMode() 				{ return eBezierMode; }
    void			SetBezierMode(UINT16 eBezMode)	{ eBezierMode = eBezMode; }
    UINT16			GetFrmColCount() const {return nInsFrmColCount;} //Spaltenzahl fuer interaktiven Rahmen


    BOOL			IsChainMode() const				{ return bChainMode; }


    static	void 	SetReplaceQuote(BOOL bOn = TRUE) { bReplaceQuote = bOn; }
    static	BOOL 	IsReplaceQuote() { return bReplaceQuote; }

    SwApplyTemplate* GetApplyTemplate() const { return pApplyTempl; }

    USHORT			GetDropAction() const { return nDropAction; }
    ULONG			GetDropFormat() const { return nDropFormat; }

    Color 			GetTextColor() { return aTextColor; }
    void 			SetTextColor(const Color& rCol ) { aTextColor = rCol; }

    Color 			GetTextBackColor()
                                            { return aTextBackColor; }
    void 			SetTextBackColor(const Color& rCol )
                                            { aTextBackColor = rCol; }
    void			SetTextBackColorTransparent(BOOL bSet)
                                    { bTransparentBackColor = bSet; }
    BOOL			IsTextBackColorTransparent()
                                    { return bTransparentBackColor; }
    void			LockKeyInput(BOOL bSet){bLockInput = bSet;}

    const SwView &GetView() const { return rView; }
          SwView &GetView() 	  { return rView; }

#ifdef ACCESSIBLE_LAYOUT
#endif

    // Tipfenster loeschen

    static inline long GetDDStartPosX() { return nDDStartPosX; }
    static inline long GetDDStartPosY() { return nDDStartPosY; }

    static void	_InitStaticData();
    static void	_FinitStaticData();
    
    //#i3370# remove quick help to prevent saving of autocorrection suggestions
    void StopQuickHelp();

    SwEditWin(Window *pParent, SwView &);
    virtual ~SwEditWin();
};


} //namespace binfilter
#endif

