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

#ifndef _TABBAR_HXX
#define _TABBAR_HXX

#include "svtools/svtdllapi.h"
#include <tools/link.hxx>
#include <vcl/window.hxx>
#include <vector>

class MouseEvent;
class TrackingEvent;
class DataChangedEvent;
class ImplTabButton;
class ImplTabSizer;
class TabBarEdit;

struct ImplTabBarItem;
typedef ::std::vector< ImplTabBarItem* > ImplTabBarList;

// -----------------
// - Dokumentation -
// -----------------

/*

Erlaubte StyleBits
------------------

WB_SCROLL       - Die Tabs koennen ueber ein Extra-Feld gescrollt werden
WB_MINSCROLL    - Die Tabs koennen ueber 2 zusaetzliche Buttons gescrollt werden
WB_RANGESELECT  - Zusammenhaengende Bereiche koennen selektiert werden
WB_MULTISELECT  - Einzelne Tabs koennen selektiert werden
WB_BORDER       - Oben und unten wird ein Strich gezeichnet
WB_TOPBORDER    - Oben wird ein Border gezeichnet
WB_3DTAB        - Die Tabs und der Border werden in 3D gezeichnet
WB_DRAG         - Vom TabBar wird ein StartDrag-Handler gerufen, wenn
                  Drag and Drop gestartet werden soll. Es wird ausserdem
                  im TabBar mit EnableDrop() Drag and Drop eingeschaltet.
WB_SIZEABLE     - Vom TabBar wird ein Split-Handler gerufen, wenn der Anwender
                  den TabBar in der Breite aendern will
WB_STDTABBAR    - WB_BORDER

Wenn man den TabBar zum Beispiel als Property-Bar benutzen moechte, sollten
die WinBits WB_TOPBORDER und WB_3DTAB anstatt WB_BORDER gesetzt werden.


Erlaubte PageBits
-----------------

TPB_SPECIAL     - Andere Darstellung des TabTextes, zum Beispiel fuer
                  Szenario-Seiten.


Handler
-------

Select          - Wird gerufen, wenn eine Tab selektiert oder
                  deselektiert wird
DoubleClick     - Wird gerufen, wenn ein DoubleClick im TabBar ausgeloest
                  wurde. Innerhalb des Handlers liefert GetCurPageId() die
                  angeklickte Tab zurueck oder 0, wenn keine Tab angeklickt
                  wurde
ActivatePage    - Wird gerufen, wenn eine andere Seite aktiviert wird.
                  GetCurPageId() gibt die aktivierte Seite zurueck.
DeactivatePage  - Wird gerufen, wenn eine Seite deaktiviert wird. Wenn
                  eine andere Seite aktiviert werden darf, muss sal_True
                  zurueckgegeben werden, wenn eine andere Seite von
                  der Aktivierung ausgeschlossen werden soll, muss
                  sal_False zurueckgegeben werden. GetCurPageId() gibt die
                  zu deaktivierende Seite zurueck.



Drag and Drop
-------------

Fuer Drag and Drop muss das WinBit WB_DRAG gesetzt werden. Ausserdem
muss der Command-, QueryDrop-Handler und der Drop-Handler ueberlagert
werden. Dabei muss in den Handlern folgendes implementiert werden:

Command         - Wenn in diesem Handler das Dragging gestartet werden
                  soll, muss StartDrag() gerufen werden. Diese Methode
                  selektiert dann den entsprechenden Eintrag oder gibt
                  sal_False zurueck, wenn das Dragging nicht durchgefuhert
                  werden kann.

QueryDrop       - Dieser Handler wird von StarView immer dann gerufen, wenn
                  bei einem Drag-Vorgang die Maus ueber das Fenster gezogen
                  wird (siehe dazu auch SV-Doku). In diesem Handler muss
                  festgestellt werden, ob ein Drop moeglich ist. Die
                  Drop-Position kann im TabBar mit ShowDropPos() angezeigt
                  werden. Beim Aufruf muss die Position vom Event uebergeben
                  werden. Wenn sich die Position am linken oder rechten
                  Rand befindet, wird automatisch im TabBar gescrollt.
                  Diese Methode gibt auch die entsprechende Drop-Position
                  zurueck, die auch fuer ein Drop gebraucht wird. Wenn das
                  Fenster beim Drag verlassen wird, kann mit HideDropPos()
                  die DropPosition wieder weggenommen werden. Es ist dadurch
                  auch moeglich, ein von ausserhalb des TabBars ausgeloestes
                  Drag zu verarbeiten.

Drop            - Im Drop-Handler muessen dann die Pages verschoben werden,
                  oder die neuen Pages eingefuegt werden. Die entsprechende
                  Drop-Postion kann mit ShowDropPos() ermittelt werden.

Folgende Methoden werden fuer Drag and Drop gebraucht und muessen von
den Handlern gerufen werden:

StartDrag       - Muss aus dem Commnad-Handler gerufen werden. Als Parameter
                  muss der CommandEvent uebergeben werden und eine Referenz
                  auf eine Region. Diese Region muss dann bei ExecuteDrag()
                  uebergeben werden, wenn der Rueckgabewert sagt, das
                  ExecuteDrag durchgefuehrt werden soll. Falls der Eintrag
                  nicht selektiert ist, wird er vorher als aktueller
                  Eintrag gesetzt. Es ist daher darauf zu achten, das aus
                  dieser Methode heraus der Select-Handler gerufen werden
                  kann.

ShowDropPos     - Diese Methode muss vom QueryDrop-Handler gerufen werden,
                  damit der TabBar anzeigt, wo die Tabs eingefuegt werden.
                  Diese Methode kann auch im Drop-Handler benutzt werden,
                  um die Position zu ermitteln wo die Tabs eingefuegt werden
                  sollen. In der Methode muss die Position vom Event
                  uebergeben werden. Diese Methode gibt die Position zurueck,
                  wo die Tabs eingefuegt werden sollen.

HideDropPos     - Diese Methode nimmt die vorher mit ShowDropPos() angezeigte
                  DropPosition wieder zurueck. Diese Methode sollte dann
                  gerufen werden, wenn bei QueryDrop() das Fenster verlassen
                  wird oder der Dragvorgang beendet wurde.

Folgende Methoden koennen eingesetzt werden, wenn bei D&D die Seiten
umgeschaltet werden sollen:

SwitchPage      - Diese Methode muss vom QueryDrop-Handler gerufen werden,
                  wenn die Seite ueber der sich der Mousepointer befindet,
                  umgeschaltet werden soll. Diese Methode sollte jedesmal
                  gerufen werden, wenn der QueryDrop-Handler gerufen wird.
                  Das umschalten der Seite passiert zeitverzoegert (500 ms)
                  und wird automatisch von dieser Methode verwaltet.
                  In der Methode muss die Position vom Event uebergeben
                  werden. Diese Methode gibt sal_True zurueck, wenn die Page
                  umgeschaltet wurde.

EndSwitchPage   - Diese Methode setzt die Daten fuer das umschalten der
                  Seiten zurueck. Diese Methode sollte dann gerufen werden,
                  wenn bei QueryDrop() das Fenster verlassen wird oder
                  der Dragvorgang beendet wurde.

IsInSwitching   - Mit dieser Methode kann im ActivatePage()/DeactivatePage()
                  abgefragt werden, ob dies durch SwitchPage() veranlasst
                  wurde. So kann dann beispielsweise in DeactivatePage()
                  das Umschalten ohne eine Fehlerbox verhindert werden.


Fenster-Resize
--------------

Wenn das Fenster vom Anwender in der Breite geaendert werden kann, dann
muss das WinBit WB_SIZEABLE gesetzt werden. In diesem Fall muss noch
folgender Handler ueberlagert werden:

Split           - Wenn dieser Handler gerufen wird, sollte das Fenster
                  auf die Breite angepasst werden, die von GetSplitSize()
                  zurueckgegeben wird. Dabei wird keine minimale und
                  maximale Breite beruecksichtig. Eine minimale Breite
                  kann mit GetMinSize() abgefragt werden und die maximale
                  Breite muss von der Anwendung selber berechnet werden.
                  Da nur Online-Resize unterstuetzt wird, muss das Fenster
                  innerhalb dieses Handlers in der Breite geaendert
                  werden und eventuell abhaengige Fenster ebenfalls. Fuer
                  diesen Handler kann auch mit SetSplitHdl() ein
                  Link gesetzt werden.

Folgende Methoden liefern beim Splitten weitere Informationen:

GetSplitSize()  - Liefert die Breite des TabBars zurueck, auf die der
                  Anwender das Fenster resizen will. Dabei wird keine
                  minimale oder maximale Breite beruecksichtigt. Es wird
                  jedoch nie eine Breite < 5 zurueckgeliefert. Diese Methode
                  liefert nur solange richtige Werte, wie Splitten aktiv
                  ist.

GetMinSize()    - Mit dieser Methode kann eine minimale Fensterbreite
                  abgefragt werden, so das min. etwas eines Tabs sichtbar
                  ist. Jedoch kann der TabBar immer noch schmaler gesetzt
                  werden, als die Breite, die diese Methode zurueckliefert.
                  Diese Methode kann auch aufgerufen werden, wenn kein
                  Splitten aktiv ist.


Edit-Modus
----------

Der Tabbar bietet auch Moeglichkeiten, das der Anwender in den Tabreitern
die Namen aendern kann.

EnableEditMode  - Damit kann eingestellt werden, das bei Alt+LeftClick
                  StartEditMode() automatisch vom TabBar gerufen wird.
                  Im StartRenaming()-Handler kann dann das Umbenennen
                  noch abgelehnt werden.
StartEditMode   - Mit dieser Methode wird der EditModus auf einem
                  Tab gestartet. sal_False wird zurueckgegeben, wenn
                  der Editmodus schon aktiv ist, mit StartRenaming()
                  der Modus abgelehnt wurde oder kein Platz zum
                  Editieren vorhanden ist.
EndEditMode     - Mit dieser Methode wird der EditModus beendet.
SetEditText     - Mit dieser Methode kann der Text im AllowRenaming()-
                  Handler noch durch einen anderen Text ersetzt werden.
GetEditText     - Mit dieser Methode kann im AllowRenaming()-Handler
                  der Text abgefragt werden, den der Anwender eingegeben
                  hat.
IsInEditMode    - Mit dieser Methode kann abgefragt werden, ob der
                  Editmodus aktiv ist.
IsEditModeCanceled      - Mit dieser Methode kann im EndRenaming()-
                          Handler abgefragt werden, ob die Umbenenung
                          abgebrochen wurde.
GetEditPageId   - Mit dieser Methode wird in den Renaming-Handlern
                  abgefragt, welcher Tab umbenannt wird/wurde.

StartRenaming() - Dieser Handler wird gerufen, wenn ueber StartEditMode()
                  der Editmodus gestartet wurde. Mit GetEditPageId()
                  kann abgefragt werden, welcher Tab umbenannt werden
                  soll. sal_False sollte zurueckgegeben werden, wenn
                  der Editmodus nicht gestartet werden soll.
AllowRenaming() - Dieser Handler wird gerufen, wenn der Editmodus
                  beendet wird (nicht bei Cancel). In diesem Handler
                  kann dann getestet werden, ob der Text OK ist.
                  Mit GetEditPageId() kann abgefragt werden, welcher Tab
                  umbenannt wurde.
                  Es sollte einer der folgenden Werte zurueckgegeben
                  werden:
                  TAB_RENAMING_YES
                  Der Tab wird umbenannt.
                  TAB_RENAMING_NO
                  Der Tab wird nicht umbenannt, der Editmodus bleibt
                  jedoch aktiv, so das der Anwender den Namen
                  entsprechent anpassen kann.
                  TAB_RENAMING_CANCEL
                  Der Editmodus wird abgebrochen und der alte
                  Text wieder hergestellt.
EndRenaming()   - Dieser Handler wird gerufen, wenn der Editmodus
                  beendet wurde. Mit GetEditPageId() kann abgefragt
                  werden, welcher Tab umbenannt wurde. Mit
                  IsEditModeCanceled() kann abgefragt werden, ob der
                  Modus abgebrochen wurde und der Name dadurch nicht
                  geaendert wurde.


Maximale Pagebreite
-------------------

Die Pagebreite der Tabs kann begrenzt werden, damit ein einfacheres
Navigieren ueber diese moeglich ist. Wenn der Text dann nicht komplett
angezeigt werden kann, wird er mit ... abgekuerzt und in der Tip-
oder der aktiven Hilfe (wenn kein Hilfetext gesetzt ist) wird dann der
ganze Text angezeigt. Mit EnableAutoMaxPageWidth() kann eingestellt
werden, ob die maximale Pagebreite sich nach der gerade sichtbaren
Breite richten soll (ist der default). Ansonsten kann auch die
maximale Pagebreite mit SetMaxPageWidth() (in Pixeln) gesetzt werden
(die AutoMaxPageWidth wird dann ignoriert).


KontextMenu
-----------

Wenn ein kontextsensitives PopupMenu anzeigt werden soll, muss der
Command-Handler ueberlagert werden. Mit GetPageId() und bei
Uebergabe der Mausposition kann ermittelt werden, ob der Mausclick
ueber einem bzw. ueber welchem Item durchgefuehrt wurde.
*/

// -----------
// - WinBits -
// -----------

#define WB_RANGESELECT      ((WinBits)0x00200000)
#define WB_MULTISELECT      ((WinBits)0x00400000)
#define WB_TOPBORDER        ((WinBits)0x04000000)
#define WB_3DTAB            ((WinBits)0x08000000)
#define WB_MINSCROLL        ((WinBits)0x20000000)
#define WB_INSERTTAB        ((WinBits)0x40000000)
#define WB_STDTABBAR        WB_BORDER

// ------------------
// - TabBarPageBits -
// ------------------

typedef sal_uInt16 TabBarPageBits;

// -------------------------
// - Bits fuer TabBarPages -
// -------------------------

#define TPB_SPECIAL         ((TabBarPageBits)0x0001)

// ----------------
// - TabBar-Types -
// ----------------

#define TABBAR_RENAMING_YES    ((long)sal_True)
#define TABBAR_RENAMING_NO     ((long)sal_False)
#define TABBAR_RENAMING_CANCEL ((long)2)

// ----------
// - TabBar -
// ----------
struct TabBar_Impl;
struct ImplTabBarItem;

class SVT_DLLPUBLIC TabBar : public Window
{
    friend class    ImplTabButton;
    friend class    ImplTabSizer;

private:
    ImplTabBarList* mpItemList;
    ImplTabButton*  mpFirstBtn;
    ImplTabButton*  mpPrevBtn;
    ImplTabButton*  mpNextBtn;
    ImplTabButton*  mpLastBtn;
    TabBar_Impl*    mpImpl;
    TabBarEdit*     mpEdit;
    OUString        maEditText;
    Color           maSelColor;
    Color           maSelTextColor;
    Size            maWinSize;
    long            mnMaxPageWidth;
    long            mnCurMaxWidth;
    long            mnOffX;
    long            mnOffY;
    long            mnLastOffX;
    long            mnSplitSize;
    sal_uLong           mnSwitchTime;
    WinBits         mnWinStyle;
    sal_uInt16          mnCurPageId;
    sal_uInt16          mnFirstPos;
    sal_uInt16          mnDropPos;
    sal_uInt16          mnSwitchId;
    sal_uInt16          mnEditId;
    sal_Bool            mbFormat;
    sal_Bool            mbFirstFormat;
    sal_Bool            mbSizeFormat;
    sal_Bool            mbAutoMaxWidth;
    sal_Bool            mbInSwitching;
    sal_Bool            mbAutoEditMode;
    sal_Bool            mbEditCanceled;
    sal_Bool            mbDropPos;
    sal_Bool            mbInSelect;
    sal_Bool            mbSelColor;
    sal_Bool            mbSelTextColor;
    bool            mbMirrored;
    bool            mbHasInsertTab; // if true, the tab bar has an extra tab at the end.
    Link            maSelectHdl;
    Link            maDoubleClickHdl;
    Link            maSplitHdl;
    Link            maActivatePageHdl;
    Link            maDeactivatePageHdl;
    Link            maStartRenamingHdl;
    Link            maAllowRenamingHdl;
    Link            maEndRenamingHdl;
    size_t          maCurrentItemList;

    using Window::ImplInit;
    SVT_DLLPRIVATE void            ImplInit( WinBits nWinStyle );
    SVT_DLLPRIVATE void            ImplInitSettings( sal_Bool bFont, sal_Bool bBackground );
    SVT_DLLPRIVATE void            ImplGetColors( Color& rFaceColor, Color& rFaceTextColor,
                                   Color& rSelectColor, Color& rSelectTextColor );
    SVT_DLLPRIVATE void            ImplShowPage( sal_uInt16 nPos );
    SVT_DLLPRIVATE sal_Bool            ImplCalcWidth();
    SVT_DLLPRIVATE void            ImplFormat();
    SVT_DLLPRIVATE sal_uInt16          ImplGetLastFirstPos();
    SVT_DLLPRIVATE void            ImplInitControls();
    SVT_DLLPRIVATE void            ImplEnableControls();
    SVT_DLLPRIVATE void         ImplSelect();
    SVT_DLLPRIVATE void         ImplActivatePage();
    SVT_DLLPRIVATE long         ImplDeactivatePage();
    SVT_DLLPRIVATE void            ImplPrePaint();
    SVT_DLLPRIVATE ImplTabBarItem* ImplGetLastTabBarItem( sal_uInt16 nItemCount );
    SVT_DLLPRIVATE Rectangle       ImplGetInsertTabRect(ImplTabBarItem* pItem) const;
                    DECL_DLLPRIVATE_LINK( ImplClickHdl, ImplTabButton* );

    ImplTabBarItem* seek( size_t i );
    ImplTabBarItem* prev();
    ImplTabBarItem* next();

public:
    static const sal_uInt16 APPEND;
    static const sal_uInt16 PAGE_NOT_FOUND;
    static const sal_uInt16 INSERT_TAB_POS;

                    TabBar( Window* pParent, WinBits nWinStyle = WB_STDTABBAR );
    virtual         ~TabBar();

    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Resize();
    virtual void    RequestHelp( const HelpEvent& rHEvt );
    virtual void    StateChanged( StateChangedType nStateChange );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    virtual void    Select();
    virtual void    DoubleClick();
    virtual void    Split();
    virtual void    ActivatePage();
    virtual long    DeactivatePage();
    virtual long    StartRenaming();
    virtual long    AllowRenaming();
    virtual void    EndRenaming();
    virtual void    Mirror();

    void            InsertPage( sal_uInt16 nPageId, const OUString& rText,
                                TabBarPageBits nBits = 0,
                                sal_uInt16 nPos = TabBar::APPEND );
    void            RemovePage( sal_uInt16 nPageId );
    void            MovePage( sal_uInt16 nPageId, sal_uInt16 nNewPos );

    Color           GetTabBgColor( sal_uInt16 nPageId ) const;
    void            SetTabBgColor( sal_uInt16 nPageId, const Color& aTabBgColor );
    sal_Bool        IsDefaultTabBgColor( sal_uInt16 nPageId );

    void            Clear();

    sal_Bool            IsPageEnabled( sal_uInt16 nPageId ) const;

    void            SetPageBits( sal_uInt16 nPageId, TabBarPageBits nBits = 0 );
    TabBarPageBits  GetPageBits( sal_uInt16 nPageId ) const;

    sal_uInt16          GetPageCount() const;
    sal_uInt16          GetPageId( sal_uInt16 nPos ) const;
    sal_uInt16          GetPagePos( sal_uInt16 nPageId ) const;
    sal_uInt16          GetPageId( const Point& rPos, bool bCheckInsTab = false ) const;
    Rectangle       GetPageRect( sal_uInt16 nPageId ) const;
    // returns the rectangle in which page tabs are drawn
    Rectangle       GetPageArea() const;

    void            SetCurPageId( sal_uInt16 nPageId );
    sal_uInt16          GetCurPageId() const { return mnCurPageId; }

    void            SetFirstPageId( sal_uInt16 nPageId );
    sal_uInt16          GetFirstPageId() const { return GetPageId( mnFirstPos ); }
    void            MakeVisible( sal_uInt16 nPageId );

    void            SelectPage( sal_uInt16 nPageId, sal_Bool bSelect = sal_True );
    sal_uInt16          GetSelectPageCount() const;
    sal_Bool            IsPageSelected( sal_uInt16 nPageId ) const;

    void            EnableAutoMaxPageWidth( sal_Bool bEnable = sal_True ) { mbAutoMaxWidth = bEnable; }
    sal_Bool            IsAutoMaxPageWidthEnabled() const { return mbAutoMaxWidth; }
    void            SetMaxPageWidth( long nMaxWidth );
    long            GetMaxPageWidth() const { return mnMaxPageWidth; }
    void            ResetMaxPageWidth() { SetMaxPageWidth( 0 ); }
    sal_Bool            IsMaxPageWidth() const { return mnMaxPageWidth != 0; }

    void            EnableEditMode( sal_Bool bEnable = sal_True ) { mbAutoEditMode = bEnable; }
    sal_Bool            IsEditModeEnabled() const { return mbAutoEditMode; }
    sal_Bool            StartEditMode( sal_uInt16 nPageId );
    void            EndEditMode( sal_Bool bCancel = sal_False );
    void            SetEditText( const OUString& rText ) { maEditText = rText; }
    const OUString& GetEditText() const { return maEditText; }
    sal_Bool            IsInEditMode() const { return (mpEdit != NULL); }
    sal_Bool            IsEditModeCanceled() const { return mbEditCanceled; }
    sal_uInt16          GetEditPageId() const { return mnEditId; }

    /** Mirrors the entire control including position of buttons and splitter.
        Mirroring is done relative to the current direction of the GUI.
        @param bMirrored  sal_True = the control will draw itself RTL in LTR GUI,
            and vice versa; sal_False = the control behaves according to the
            current direction of the GUI. */
    void            SetMirrored(bool bMirrored = true);
    /** Returns sal_True, if the control is set to mirrored mode (see SetMirrored()). */
    bool            IsMirrored() const { return mbMirrored; }

    /** Sets the control to LTR or RTL mode regardless of the GUI direction.
        @param bRTL  sal_False = the control will draw from left to right;
            sal_True = the control will draw from right to left. */
    void            SetEffectiveRTL( bool bRTL );
    /** Returns true, if the control draws from right to left (see SetEffectiveRTL()). */
    bool            IsEffectiveRTL() const;

    sal_Bool            StartDrag( const CommandEvent& rCEvt, Region& rRegion );
    sal_uInt16          ShowDropPos( const Point& rPos );
    void            HideDropPos();
    sal_Bool            SwitchPage( const Point& rPos );
    void            EndSwitchPage();
    sal_Bool            IsInSwitching() { return mbInSwitching; }

    const Color&    GetSelectColor() const { return maSelColor; }
    sal_Bool            IsSelectColor() const { return mbSelColor; }
    const Color&    GetSelectTextColor() const { return maSelTextColor; }
    sal_Bool            IsSelectTextColor() const { return mbSelTextColor; }

    void            SetPageText( sal_uInt16 nPageId, const OUString& rText );
    OUString        GetPageText( sal_uInt16 nPageId ) const;
    OUString        GetHelpText( sal_uInt16 nPageId ) const;
    OString         GetHelpId( sal_uInt16 nPageId ) const;

    long            GetSplitSize() const { return mnSplitSize; }

    void            SetHelpText( const OUString& rText )
                        { Window::SetHelpText( rText ); }
    OUString        GetHelpText() const
                        { return Window::GetHelpText(); };
    void            SetHelpId( const OString& rId )
                        { Window::SetHelpId( rId ); }
    const OString& GetHelpId() const
                        { return Window::GetHelpId(); }

    void            SetStyle( WinBits nStyle );
    WinBits         GetStyle() const { return mnWinStyle; }

    Size            CalcWindowSizePixel() const;

    void            SetSelectHdl( const Link& rLink ) { maSelectHdl = rLink; }
    const Link&     GetSelectHdl() const { return maSelectHdl; }
    void            SetDoubleClickHdl( const Link& rLink ) { maDoubleClickHdl = rLink; }
    const Link&     GetDoubleClickHdl() const { return maDoubleClickHdl; }
    void            SetSplitHdl( const Link& rLink ) { maSplitHdl = rLink; }
    const Link&     GetSplitHdl() const { return maSplitHdl; }
    void            SetActivatePageHdl( const Link& rLink ) { maActivatePageHdl = rLink; }
    const Link&     GetActivatePageHdl() const { return maActivatePageHdl; }
    void            SetDeactivatePageHdl( const Link& rLink ) { maDeactivatePageHdl = rLink; }
    const Link&     GetDeactivatePageHdl() const { return maDeactivatePageHdl; }
    void            SetStartRenamingHdl( const Link& rLink ) { maStartRenamingHdl = rLink; }
    const Link&     GetStartRenamingHdl() const { return maStartRenamingHdl; }
    void            SetAllowRenamingHdl( const Link& rLink ) { maAllowRenamingHdl = rLink; }
    const Link&     GetAllowRenamingHdl() const { return maAllowRenamingHdl; }
    void            SetEndRenamingHdl( const Link& rLink ) { maEndRenamingHdl = rLink; }
    const Link&     GetEndRenamingHdl() const { return maEndRenamingHdl; }

    // accessibility
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();
};

#endif  // _TABBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
