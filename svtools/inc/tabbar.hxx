/*************************************************************************
 *
 *  $RCSfile: tabbar.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _TABBAR_HXX
#define _TABBAR_HXX

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _WINDOW_HXX
#include <vcl/window.hxx>
#endif

class MouseEvent;
class TrackingEvent;
class DataChangedEvent;
class ImplTabBarList;
class ImplTabButton;
class ImplTabSizer;
class TabBarEdit;

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
                  eine andere Seite aktiviert werden darf, muss TRUE
                  zurueckgegeben werden, wenn eine andere Seite von
                  der Aktivierung ausgeschlossen werden soll, muss
                  FALSE zurueckgegeben werden. GetCurPageId() gibt die
                  zu deaktivierende Seite zurueck.



Drag and Drop
-------------

Fuer Drag and Drop muss das WinBit WB_DRAG gesetzt werden. Ausserdem
muss der Command-, QueryDrop-Handler und der Drop-Handler ueberlagert
werden. Dabei muss in den Handlern folgendes implementiert werden:

Command         - Wenn in diesem Handler das Dragging gestartet werden
                  soll, muss StartDrag() gerufen werden. Diese Methode
                  selektiert dann den entsprechenden Eintrag oder gibt
                  FALSE zurueck, wenn das Dragging nicht durchgefuhert
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
                  werden. Diese Methode gibt TRUE zurueck, wenn die Page
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
                  Tab gestartet. FALSE wird zurueckgegeben, wenn
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
                  soll. FALSE sollte zurueckgegeben werden, wenn
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
#define WB_STDTABBAR        WB_BORDER

// ------------------
// - TabBarPageBits -
// ------------------

typedef USHORT TabBarPageBits;

// -------------------------
// - Bits fuer TabBarPages -
// -------------------------

#define TPB_SPECIAL         ((TabBarPageBits)0x0001)

// ----------------
// - TabBar-Types -
// ----------------

#define TAB_APPEND          ((USHORT)0xFFFF)
#define TAB_PAGE_NOTFOUND   ((USHORT)0xFFFF)

#define TAB_RENAMING_YES    ((long)TRUE)
#define TAB_RENAMING_NO     ((long)FALSE)
#define TAB_RENAMING_CANCEL ((long)2)

// ----------
// - TabBar -
// ----------

class TabBar : public Window
{
    friend class    ImplTabButton;
    friend class    ImplTabSizer;

private:
    ImplTabBarList* mpItemList;
    ImplTabButton*  mpFirstBtn;
    ImplTabButton*  mpPrevBtn;
    ImplTabButton*  mpNextBtn;
    ImplTabButton*  mpLastBtn;
    ImplTabSizer*   mpSizer;
    TabBarEdit*     mpEdit;
    XubString       maEditText;
    Color           maSelColor;
    Color           maSelTextColor;
    Size            maWinSize;
    long            mnMaxPageWidth;
    long            mnCurMaxWidth;
    long            mnOffX;
    long            mnOffY;
    long            mnOutWidth;
    long            mnSplitSize;
    ULONG           mnSwitchTime;
    WinBits         mnWinStyle;
    USHORT          mnCurPageId;
    USHORT          mnFirstPos;
    USHORT          mnDropPos;
    USHORT          mnSwitchId;
    USHORT          mnEditId;
    BOOL            mbFormat;
    BOOL            mbFirstFormat;
    BOOL            mbSizeFormat;
    BOOL            mbAutoMaxWidth;
    BOOL            mbInSwitching;
    BOOL            mbAutoEditMode;
    BOOL            mbEditCanceled;
    BOOL            mbDropPos;
    BOOL            mbInSelect;
    BOOL            mbSelColor;
    BOOL            mbSelTextColor;
    Link            maSelectHdl;
    Link            maDoubleClickHdl;
    Link            maSplitHdl;
    Link            maActivatePageHdl;
    Link            maDeactivatePageHdl;
    Link            maStartRenamingHdl;
    Link            maAllowRenamingHdl;
    Link            maEndRenamingHdl;

#ifdef _SV_TABBAR_CXX
    void            ImplInit( WinBits nWinStyle );
    void            ImplInitSettings( BOOL bFont, BOOL bBackground );
    void            ImplGetColors( Color& rFaceColor, Color& rFaceTextColor,
                                   Color& rSelectColor, Color& rSelectTextColor );
    void            ImplShowPage( USHORT nPos );
    BOOL            ImplCalcWidth();
    void            ImplFormat();
    USHORT          ImplGetLastFirstPos();
    void            ImplInitControls();
    void            ImplEnableControls();
                    DECL_LINK( ImplClickHdl, ImplTabButton* );
#endif

public:
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

    void            InsertPage( USHORT nPageId, const XubString& rText,
                                TabBarPageBits nBits = 0,
                                USHORT nPos = TAB_APPEND );
    void            RemovePage( USHORT nPageId );
    void            MovePage( USHORT nPageId, USHORT nNewPos );
    void            Clear();

    void            EnablePage( USHORT nPageId, BOOL bEnable = TRUE );
    BOOL            IsPageEnabled( USHORT nPageId ) const;

    void            SetPageBits( USHORT nPageId, TabBarPageBits nBits = 0 );
    TabBarPageBits  GetPageBits( USHORT nPageId ) const;

    USHORT          GetPageCount() const;
    USHORT          GetPageId( USHORT nPos ) const;
    USHORT          GetPagePos( USHORT nPageId ) const;
    USHORT          GetPageId( const Point& rPos ) const;
    Rectangle       GetPageRect( USHORT nPageId ) const;

    void            SetCurPageId( USHORT nPageId );
    USHORT          GetCurPageId() const { return mnCurPageId; }

    void            SetFirstPageId( USHORT nPageId );
    USHORT          GetFirstPageId() const { return GetPageId( mnFirstPos ); }

    void            SelectPage( USHORT nPageId, BOOL bSelect = TRUE );
    void            SelectPageRange( BOOL bSelect = FALSE,
                                     USHORT nStartPos = 0,
                                     USHORT nEndPos = TAB_APPEND );
    USHORT          GetSelectPage( USHORT nSelIndex = 0 ) const;
    USHORT          GetSelectPageCount() const;
    BOOL            IsPageSelected( USHORT nPageId ) const;

    void            EnableAutoMaxPageWidth( BOOL bEnable = TRUE ) { mbAutoMaxWidth = bEnable; }
    BOOL            IsAutoMaxPageWidthEnabled() const { return mbAutoMaxWidth; }
    void            SetMaxPageWidth( long nMaxWidth );
    long            GetMaxPageWidth() const { return mnMaxPageWidth; }
    void            ResetMaxPageWidth() { SetMaxPageWidth( 0 ); }
    BOOL            IsMaxPageWidth() const { return mnMaxPageWidth != 0; }

    void            EnableEditMode( BOOL bEnable = TRUE ) { mbAutoEditMode = bEnable; }
    BOOL            IsEditModeEnabled() const { return mbAutoEditMode; }
    BOOL            StartEditMode( USHORT nPageId );
    void            EndEditMode( BOOL bCancel = FALSE );
    void            SetEditText( const XubString& rText ) { maEditText = rText; }
    const XubString& GetEditText() const { return maEditText; }
    BOOL            IsInEditMode() const { return (mpEdit != NULL); }
    BOOL            IsEditModeCanceled() const { return mbEditCanceled; }
    USHORT          GetEditPageId() const { return mnEditId; }

    BOOL            StartDrag( const CommandEvent& rCEvt, Region& rRegion );
    USHORT          ShowDropPos( const Point& rPos );
    void            HideDropPos();
    BOOL            SwitchPage( const Point& rPos );
    void            EndSwitchPage();
    BOOL            IsInSwitching() { return mbInSwitching; }

    void            SetSelectColor();
    void            SetSelectColor( const Color& rColor );
    const Color&    GetSelectColor() const { return maSelColor; }
    BOOL            IsSelectColor() const { return mbSelColor; }
    void            SetSelectTextColor();
    void            SetSelectTextColor( const Color& rColor );
    const Color&    GetSelectTextColor() const { return maSelTextColor; }
    BOOL            IsSelectTextColor() const { return mbSelTextColor; }

    void            SetPageText( USHORT nPageId, const XubString& rText );
    XubString       GetPageText( USHORT nPageId ) const;
    void            SetHelpText( USHORT nPageId, const XubString& rText );
    XubString       GetHelpText( USHORT nPageId ) const;
    void            SetHelpId( USHORT nPageId, ULONG nHelpId );
    ULONG           GetHelpId( USHORT nPageId ) const;

    long            GetSplitSize() const { return mnSplitSize; }
    long            GetMinSize() const;

    void            SetHelpText( const XubString& rText )
                        { Window::SetHelpText( rText ); }
    XubString       GetHelpText() const
                        { return Window::GetHelpText(); };
    void            SetHelpId( ULONG nId )
                        { Window::SetHelpId( nId ); }
    ULONG           GetHelpId() const
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
};

#endif  // _TABBAR_HXX
