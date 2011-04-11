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

#ifndef _VALUESET_HXX
#define _VALUESET_HXX

#include "svtools/svtdllapi.h"

#include <vcl/ctrl.hxx>
#include <vcl/virdev.hxx>
#include <vcl/timer.hxx>
#include <vector>

class MouseEvent;
class TrackingEvent;
class HelpEvent;
class KeyEvent;
class DataChangedEvent;
class ScrollBar;
struct ValueSet_Impl;

struct ValueSetItem;
typedef ::std::vector< ValueSetItem* > ValueItemList;

#ifdef _SV_VALUESET_CXX
class ValueSetAcc;
class ValueItemAcc;
#endif

/*************************************************************************

Beschreibung
============

class ValueSet

Diese Klasse erlaubt die Auswahl eines Items. Dabei werden die Items
nebeneinander dargestellt. Die Auswahl der Items kann zum Beispiel bei
Farben oder Mustern uebersichtlicher sein, als in ListBox-Form. Es kann
angegeben werden, wieviele Spalten das Control anzeigen soll und ob die
Items umrandet werden sollen oder nicht. Optional kann auch ein
NoSelection-Feld oder ein Namenfeld angezeigt werden. An Items werden
standardmaessig Image, und Color unterstuetzt. Falls InsertItem()
nur mit einer ID aufgerufen wird, kann man die Items auch selber malen.
Dazu muss dann jedoch der UserDraw-Handler ueberlagert werden. Bei
UserDraw-Items, wie auch bei allen anderen Items kann der Beschreibungstext
auch hinterher gesetzt bzw. umgesetzt werden.

Querverweise

class ListBox

--------------------------------------------------------------------------

WinBits

WB_RADIOSEL         Wenn dieses Flag gesetzt wird, wird im ValueSet die
                    Selektion so gezeichnet, wie bei einem ImageRadioButton.
                    Dies ist jedoch nur sinnvoll, wenn die Images min. 8 Pixel
                    in horizontaler und vertikaler Richtung kleiner sind
                    als der Item-Bereich, WB_DOUBLEBORDER gesetzt ist und
                    als Color COL_WINDOWWORKSPACE gesetzt ist.
WB_FLATVALUESET     Flat Look (if you set WB_ITEMBORDER or WB_DOUBLEBORDER,
                    then you get extra border space, but the Borders
                    aren't painted),
WB_ITEMBORDER       Die Items werden umrandet
WB_DOUBLEBORDER     Die Items werden doppelt umrandet. Zusaetzlich muss
                    WB_ITEMBORDER gesetzt werden, ansonsten hat dieses
                    WinBit keine Auswirkung. Gebraucht wird dies, wenn man
                    Items mit weissem Hintergrund hat, da sonst der 3D Effekt
                    nicht mehr sichtbar ist.
WB_NAMEFIELD        Es gibt ein Namensfeld, wo der Name eines Items
                    mit angezeigt wird
WB_NONEFIELD        Es gibt ein NoSelection-Feld. Dieses kann selektiert
                    werden, wenn bei SelectItem 0 uebergeben wird. Bei
                    GetSelectItemId() wird entsprechend 0 zurueckgegeben
                    wenn dieses Feld selektiert ist, bzw. keines selektiert
                    wurde. Dieses Feld zeigt den Text an, der mit SetText()
                    gesetzt wurde, bzw. keinen, wenn kein Text gesetzt
                    wurde. Mit SetNoSelection() kann die Selektion ganz
                    ausgeschaltet werden.
WB_VSCROLL          Es wird immer ein ScrollBar angezeigt. Wenn dieses Flag
                    gesetzt wird, muss auch immer mit SetLineCount() die
                    sichtbare Anzahl der Zeilen eingestellt werden.
WB_BORDER           Um das Fenster wird ein Border gezeichnet.
WB_NOPOINTERFOCUS   Der Focus wird sich nicht geholt, wenn mit der Maus in
                    das Control geklickt wird.
WB_TABSTOP          Mit der TabTaste kann in das ValueSet gesprungen werden.
WB_NOTABSTOP        Mit der TabTaste kann nicht in das ValueSet gesprungen
                    werden.
WB_NO_DIRECTSELECT  Cursor travelling doesn't call select immediately. To
                    execute the selection <RETURN> has to be pressed.
--------------------------------------------------------------------------

Die Anzahl der Spalten muss entweder mit SetColCount() oder mit SetItemWidth()
eingestellt werden. Wenn mit SetColCount() die Anzahl der Spalten eingestellt
wird, wird die Breite der Items aus der sichtbaren Breite berechnet. Wenn
die Items eine feste Breite haben sollen, sollte die Breite der Items
mit SetItemWidth() eingestellt werden. Die Anzahl der Spalten wird
dann aus der sichtbaren Breite berechnet.

Die Anzahl der Zeilen ergibt sich durch Anzahl der Items / Anzahl der
Spalten. Die Anzahl der sichtbaren Zeilen muss entweder mit SetLineCount()
oder mit SetItemWidth() eingestellt werden. Wenn mit SetLineCount() die
Anzahl der sichtbaren Zeilen eingestellt wird, wird die Hoehe der Items
aus der sichtbaren Hoehe berechnet. Wenn die Items eine feste Hoehe haben
sollen, sollte die Hoehe der Items mit SetItemHeight() eingestellt werden.
Die Anzahl der sichtbaren Zeilen wird dann aus der sichtbaren Hoehe berechnet.
Wenn weder mit SetLineCount() noch mit SetItemHeight() die Anzahl der
sichtbaren Zeilen eingestellt wird, werden alle Zeilen dargestellt. Die
Hoehe der Items wird dann aus der sichtbaren Hoehe berechnet. Wenn mit
SetLineCount() oder mit SetItemHeight() die Anzahl der sichtbaren
Zeilen gesetzt wird, scrollt das ValueSet automatisch, wenn mehr Zeilen
vorhanden sind, als sichtbar sind. Wenn scrollen auch ueber einen
ScrollBar moeglich sein soll muss WB_VSCROLL gesetzt werden.
Mit SetFirstLine() kann die erste sichtbare Zeile eingestellt werden, wenn
mehr Zeilen vorhanden sind als sichtbar sind.

Mit SetExtraSpacing() kann der Abstand zwischen den Items vergroessert
werden. Der Abstand wird in Pixeln angegeben der zusaetzlich zwischen 2 Items
(sowohl in x wie auch in y) dargestellt werden soll.

Mit CalcWindowSizePixel() kann die genaue Fenstergroesse fuer eine bestimmte
Itemgroesse berechnet werden. Dazu muessen vorher aber alle relevanten
Daten (Spaltenanzahl/...) gesetzt werden und falls keine Zeilenanzahl
eingestellt wird, muessen auch alle Items eingefuegt werden. Falls das
Window mit WB_BORDER/Border=sal_True erzeugt wurde, muss die Groesse mit
SetOutputSizePixel() gesetzt werden, im anderen Fall koennen auch die
anderen Groessen-Methoden benutzt werden. Mit CalcItemSize() laesst sich
die innere und aeussere Groesse eines Items berechnen (dabei wird der
optional mit SetExtraSpacing() eingestellte Freiraum nicht mit eingerechnet).

Mit SetColor() kann die Hintergrundfarbe eingestellt werden, mit der Image
oder UserDraw-Items hinterlegt werden. Wenn keine Farbe eingestellt wird,
wird der Hintergrund in der gleichen Farbe hinterlegt wie andere
Fenster (WindowColor).

Mit InsertSpace() kann man ein leeres Feld einfuegen, welches weder
dargestellt noch selektierbar ist.

--------------------------------------------------------------------------

Da die Ausgabeflaeche vorberechnet wird, sollten erst alle Items eingefuegt
werden und dann erst Show() aufgerufen werden. Wenn dies nicht gemacht wird,
erscheint das erste Paint etwas langsamer. Deshalb sollte, wenn dieses
Control aus der Resource geladen wird und das Control erst im Programm
mit Items versorgt wird, mit Hide = sal_True geladen werden und im Programm
dann mit Show() angezeigt werden.

Bei einem sichbaren Control kann die Erzeugung der neuen Ausgabeflaeche
vor dem Paint aktiviert werden, indem Format() aufgerufen wird.

--------------------------------------------------------------------------

Wenn man ein Drag and Drop aus dem ValueSet heraus starten will, muss
der Command-Handler ueberlagert werden. Aus diesem muss dann StartDrag
aufgerufen werden. Wenn diese Methode sal_True zurueckliefert, kann mit
ExecuteDrag() der Drag-Vorgang gestartet werden, ansonsten sollte keine
Verarbeitung stattfinden. Diese Methode sorgt dafuer, das das ValueSet
seine Verarbeitung abbricht und gegebenenfalls den Eintrag selektiert. Es
muss daher damit gerechnet werden, das innerhalb dieser Funktion der
Select-Handler gerufen werden kann.

Fuer das Droppen muss man QueryDrop() und Drop() ueberlagern und sollte
in diesen Methoden ShowDropPos() und HideDropPos() aufrufen. Im QueryDrop-
Handler ruft man ShowDropPos() auf, um die Einfuegeposition anzuzeigen.
ShowDropPos() scollt auch gegebenenfalls das ValueSet, wenn die ueber-
gebene Position sich am Fensterrand befindet. Ausserdem liefert
ShowDropPos() die Position zurueck, an der das Item dann eingefuegt werden
soll, bzw. welche Einfuegeposition angezeigt wurde. Falls keine Einfuege-
Position ermittelt werden kann, wird VALUESET_ITEM_NOTFOUND zurueckgegeben.
Wenn beim Draggen das Fenster verlassen wird oder der Dragvorgang beendet
wird, sollte in jedem Fall HideDropPos() gerufen werden.

--------------------------------------------------------------------------

Mit SetItemBits() koennen Flags an einem Item gesetzt werden. Folgende
Flags sind zur Zeit vorgesehen:

VIB_NODOUBLECLICK   Jeder Klick fuehrt zu einer Select-Aktion. Der
                    Doppelklick-Handler wird in diesem Fall nie
                    gerufen.

--------------------------------------------------------------------------

Diese Klasse befindet sich zur Zeit noch in den SV-Tools. Deshalb muss das
ValueSet zur Zeit als Control aus der Resource geladen werden und die
gewuenschten WinBits (vor Show) mit SetStyle() gesetzt werden.

*************************************************************************/

// -----------------------
// - ValueSet-Item-Typen -
// -----------------------

#define VIB_NODOUBLECLICK       ((sal_uInt16)0x0080)

// ------------------
// - ValueSet-Typen -
// ------------------

#define WB_RADIOSEL             ((WinBits)0x00008000)
#define WB_ITEMBORDER           ((WinBits)0x00010000)
#define WB_DOUBLEBORDER         ((WinBits)0x00020000)
#define WB_NAMEFIELD            ((WinBits)0x00040000)
#define WB_NONEFIELD            ((WinBits)0x00080000)
#define WB_FLATVALUESET         ((WinBits)0x02000000)
#define WB_NO_DIRECTSELECT      ((WinBits)0x04000000)
#define WB_MENUSTYLEVALUESET    ((WinBits)0x08000000)

// ------------
// - ValueSet -
// ------------

#define VALUESET_APPEND         ((sal_uInt16)-1)
#define VALUESET_ITEM_NOTFOUND  ((sal_uInt16)-1)

class SVT_DLLPUBLIC ValueSet : public Control
{
private:

    VirtualDevice   maVirDev;
    Timer           maTimer;
    ValueSet_Impl*  mpImpl;
    ValueSetItem*   mpNoneItem;
    ScrollBar*      mpScrBar;
    long            mnTextOffset;
    long            mnVisLines;
    long            mnLines;
    long            mnUserItemWidth;
    long            mnUserItemHeight;
    sal_uInt16          mnOldItemId;
    sal_uInt16          mnSelItemId;
    sal_uInt16          mnHighItemId;
    size_t          mnDropPos;
    sal_uInt16          mnCols;
    sal_uInt16          mnCurCol;
    sal_uInt16          mnUserCols;
    sal_uInt16          mnUserVisLines;
    sal_uInt16          mnFirstLine;
    sal_uInt16          mnSpacing;
    sal_uInt16          mnFrameStyle;
    sal_Bool            mbFormat;
    sal_Bool            mbHighlight;
    sal_Bool            mbSelection;
    sal_Bool            mbNoSelection;
    sal_Bool            mbDrawSelection;
    sal_Bool            mbBlackSel;
    sal_Bool            mbDoubleSel;
    sal_Bool            mbScroll;
    sal_Bool            mbDropPos;
    sal_Bool            mbFullMode;
    Color           maColor;
    Link            maDoubleClickHdl;
    Link            maSelectHdl;

#ifdef _SV_VALUESET_CXX
    friend class ValueSetAcc;
    friend class ValueItemAcc;
    using Control::ImplInitSettings;
    using Window::ImplInit;
    SVT_DLLPRIVATE void         ImplInit();
    SVT_DLLPRIVATE void         ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
    SVT_DLLPRIVATE void         ImplInitScrollBar();
    SVT_DLLPRIVATE void         ImplDeleteItems();
    SVT_DLLPRIVATE void         ImplFormatItem( ValueSetItem* pItem );
    SVT_DLLPRIVATE void         ImplDrawItemText( const XubString& rStr );
    SVT_DLLPRIVATE void         ImplDrawSelect();
    SVT_DLLPRIVATE void         ImplHideSelect( sal_uInt16 nItemId );
    SVT_DLLPRIVATE void         ImplHighlightItem( sal_uInt16 nItemId, sal_Bool bIsSelection = sal_True );
    SVT_DLLPRIVATE void         ImplDrawDropPos( sal_Bool bShow );
    SVT_DLLPRIVATE void         ImplDraw();
    using Window::ImplScroll;
    SVT_DLLPRIVATE sal_Bool         ImplScroll( const Point& rPos );
    SVT_DLLPRIVATE size_t       ImplGetItem( const Point& rPoint, sal_Bool bMove = sal_False ) const;
    SVT_DLLPRIVATE ValueSetItem*    ImplGetItem( size_t nPos );
    SVT_DLLPRIVATE ValueSetItem*    ImplGetFirstItem();
    SVT_DLLPRIVATE sal_uInt16          ImplGetVisibleItemCount() const;
    SVT_DLLPRIVATE ValueSetItem*    ImplGetVisibleItem( sal_uInt16 nVisiblePos );
    SVT_DLLPRIVATE void            ImplFireAccessibleEvent( short nEventId, const ::com::sun::star::uno::Any& rOldValue, const ::com::sun::star::uno::Any& rNewValue );
    SVT_DLLPRIVATE sal_Bool            ImplHasAccessibleListeners();
    SVT_DLLPRIVATE void         ImplTracking( const Point& rPos, sal_Bool bRepeat );
    SVT_DLLPRIVATE void         ImplEndTracking( const Point& rPos, sal_Bool bCancel );
    DECL_DLLPRIVATE_LINK( ImplScrollHdl, ScrollBar* );
    DECL_DLLPRIVATE_LINK( ImplTimerHdl, Timer* );
#endif

    // Forbidden and not implemented.
    ValueSet (const ValueSet &);
    ValueSet & operator= (const ValueSet &);

protected:

    sal_Bool            StartDrag( const CommandEvent& rCEvt, Region& rRegion );
    sal_uInt16          ShowDropPos( const Point& rPos );
    void            HideDropPos();

protected:

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

public:
                    ValueSet( Window* pParent, WinBits nWinStyle = WB_ITEMBORDER, bool bDisableTransientChildren = false );
                    ValueSet( Window* pParent, const ResId& rResId, bool bDisableTransientChildren = false );
                    ~ValueSet();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    Tracking( const TrackingEvent& rMEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    GetFocus();
    virtual void    LoseFocus();
    virtual void    Resize();
    virtual void    RequestHelp( const HelpEvent& rHEvt );
    virtual void    StateChanged( StateChangedType nStateChange );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    virtual void    Select();
    virtual void    DoubleClick();
    virtual void    UserDraw( const UserDrawEvent& rUDEvt );

    void            InsertItem( sal_uInt16 nItemId, const Image& rImage,
                                size_t nPos = VALUESET_APPEND );
    void            InsertItem( sal_uInt16 nItemId, const Color& rColor,
                                size_t nPos = VALUESET_APPEND );
    void            InsertItem( sal_uInt16 nItemId,
                                const Image& rImage, const XubString& rStr,
                                size_t nPos = VALUESET_APPEND );
    void            InsertItem( sal_uInt16 nItemId,
                                const Color& rColor, const XubString& rStr,
                                size_t nPos = VALUESET_APPEND );
    void            InsertItem( sal_uInt16 nItemId,
                                size_t nPos = VALUESET_APPEND );
    void            InsertSpace( sal_uInt16 nItemId,
                                 size_t nPos = VALUESET_APPEND );
    void            RemoveItem( sal_uInt16 nItemId );

    void            CopyItems( const ValueSet& rValueSet );
    void            Clear();

    size_t          GetItemCount() const;
    size_t          GetItemPos( sal_uInt16 nItemId ) const;
    sal_uInt16          GetItemId( size_t nPos ) const;
    sal_uInt16          GetItemId( const Point& rPos ) const;
    Rectangle       GetItemRect( sal_uInt16 nItemId ) const;

    void            EnableFullItemMode( sal_Bool bFullMode = sal_True );
    sal_Bool            IsFullItemModeEnabled() const { return mbFullMode; }
    void            SetColCount( sal_uInt16 nNewCols = 1 );
    sal_uInt16          GetColCount() const { return mnUserCols; }
    void            SetLineCount( sal_uInt16 nNewLines = 0 );
    sal_uInt16          GetLineCount() const { return mnUserVisLines; }
    void            SetItemWidth( long nItemWidth = 0 );
    long            GetItemWidth() const { return mnUserItemWidth; }
    void            SetItemHeight( long nLineHeight = 0 );
    long            GetItemHeight() const { return mnUserItemHeight; }
    void            SetFirstLine( sal_uInt16 nNewLine = 0 );
    sal_uInt16          GetFirstLine() const { return mnFirstLine; }

    void            SelectItem( sal_uInt16 nItemId );
    sal_uInt16          GetSelectItemId() const { return mnSelItemId; }
    sal_Bool            IsItemSelected( sal_uInt16 nItemId ) const
                        { return ((!mbNoSelection && (nItemId == mnSelItemId)) ? sal_True : sal_False); }
    void            SetNoSelection();
    sal_Bool            IsNoSelection() const { return mbNoSelection; }

    void            SetItemBits( sal_uInt16 nItemId, sal_uInt16 nBits );
    sal_uInt16          GetItemBits( sal_uInt16 nItemId ) const;
    void            SetItemImage( sal_uInt16 nItemId, const Image& rImage );
    Image           GetItemImage( sal_uInt16 nItemId ) const;
    void            SetItemColor( sal_uInt16 nItemId, const Color& rColor );
    Color           GetItemColor( sal_uInt16 nItemId ) const;
    void            SetItemData( sal_uInt16 nItemId, void* pData );
    void*           GetItemData( sal_uInt16 nItemId ) const;
    void            SetItemText( sal_uInt16 nItemId, const XubString& rStr );
    XubString       GetItemText( sal_uInt16 nItemId ) const;
    void            SetColor( const Color& rColor );
    void            SetColor() { SetColor( Color( COL_TRANSPARENT ) ); }
    Color           GetColor() const { return maColor; }
    sal_Bool            IsColor() const { return maColor.GetTransparency() == 0; }

    void            SetExtraSpacing( sal_uInt16 nNewSpacing );
    sal_uInt16          GetExtraSpacing() { return mnSpacing; }

    void            Format();

    void            StartSelection();
    void            EndSelection();

    Size            CalcWindowSizePixel( const Size& rItemSize,
                                         sal_uInt16 nCalcCols = 0,
                                         sal_uInt16 nCalcLines = 0 );
    Size            CalcItemSizePixel( const Size& rSize, sal_Bool bOut = sal_True ) const;
    long            GetScrollWidth() const;

    void            SetSelectHdl( const Link& rLink ) { maSelectHdl = rLink; }
    const Link&     GetSelectHdl() const { return maSelectHdl; }
    void            SetDoubleClickHdl( const Link& rLink ) { maDoubleClickHdl = rLink; }
    const Link&     GetDoubleClickHdl() const { return maDoubleClickHdl; }

    void            SetHighlightHdl( const Link& rLink );
    const Link&     GetHighlightHdl() const;

private:
    /** Determine whether RTL (right to left writing) is active.  For this
        method to return <true/> two conditions have to be fullfilled:
        <ol><li>RTL is active for the whole application.</li>
        <li>RTL is enabled for the control.</li></ol>
        @return
             The method returns <true/> when RTL is active for the control
             and <false/> when RTL is not active.  Note then just enabling
             RTL for the control is not enough.
    */
    SVT_DLLPRIVATE bool IsRTLActive (void);
};

#endif  // _VALUESET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
