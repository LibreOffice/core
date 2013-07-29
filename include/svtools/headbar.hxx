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

#ifndef _HEADBAR_HXX
#define _HEADBAR_HXX

#include "svtools/svtdllapi.h"
#include <tools/link.hxx>
#include <vcl/window.hxx>

/*************************************************************************

Beschreibung
============

class HeaderBar

Diese Klasse dient zur Anzeige einer Ueberschiftszeile. Diese kann Texte,
Images oder beides anzeigen. Man kann die Items in der Groesse aendern,
verschieben oder anklicken. In vielen Faellen macht es zum Beispiel Sinn,
dieses Control mit einer SvTabListBox zu verbinden.

--------------------------------------------------------------------------

WinBits

WB_BORDER           Oben und unten wird ein Border gezeichnet
WB_BOTTOMBORDER     Unten wird ein Border gezeichnet
WB_BUTTONSTYLE      Die Items sehen aus wie Buttons, ansonsten sind sie flach
WB_3DLOOK           3D-Darstellung
WB_DRAG             Items koennen verschoben werden
WB_STDHEADERBAR     WB_BUTTONSTYLE | WB_BOTTOMBORDER

--------------------------------------------------------------------------

ItemBits

HIB_LEFT            Inhalt wird im Item linksbuendig ausgegeben
HIB_CENTER          Inhalt wird im Item zentriert ausgegeben
HIB_RIGHT           Inhalt wird im Item rechtsbuendig ausgegeben
HIB_TOP             Inhalt wird im Item an der oberen Kante ausgegeben
HIB_VCENTER         Inhalt wird im Item vertikal zentiert ausgegeben
HIB_BOTTOM          Inhalt wird im Item an der unteren Kante ausgegeben
HIB_LEFTIMAGE       Bei Text und Image, wird Image links vom Text ausgegeben
HIB_RIGHTIMAGE      Bei Text und Image, wird Image rechts vom Text ausgegeben
HIB_FIXED           Item laesst sich nicht in der Groesse aendern
HIB_FIXEDPOS        Item laesst sich nicht verschieben
HIB_CLICKABLE       Item laesst sich anklicken
                    (Select-Handler wird erst bei MouseButtonUp gerufen)
HIB_FLAT            Item wird flach dargestellt, auch wenn WB_BUTTONSTYLE gesetzt ist
HIB_DOWNARROW       Es wird ein Pfeil nach unter hinter dem Text ausgegeben,
                    welcher zum Beispiel angezeigt werden sollte, wenn nach
                    diesem Item eine dazugehoerende Liste absteigend sortiert
                    ist. Der Status des Pfeils kann mit SetItemBits()
                    gesetzt/zurueckgesetzt werden.
HIB_UPARROW         Es wird ein Pfeil nach oben hinter dem Text ausgegeben,
                    welcher zum Beispiel angezeigt werden sollte, wenn nach
                    diesem Item eine dazugehoerende Liste aufsteigend sortiert
                    ist.Der Status des Pfeils kann mit SetItemBits()
                    gesetzt/zurueckgesetzt werden.
HIB_USERDRAW        Zu diesem Item wird auch der UserDraw-Handler gerufen.
HIB_STDSTYLE        (HIB_LEFT | HIB_LEFTIMAGE | HIB_VCENTER | HIB_CLICKABLE)

--------------------------------------------------------------------------

Handler

Select()            Wird gerufen, wenn Item angeklickt wird. Wenn
                    HIB_CLICKABLE beim Item gesetzt ist und nicht HIB_FLAT,
                    wird der Handler erst im MouseButtonUp-Handler gerufen,
                    wenn die Maus ueber dem Item losgelassen wurde. Dann
                    verhaellt sich der Select-Handler wie bei einem
                    ToolBox-Button.
DoubleClick()       Dieser Handler wird gerufen, wenn ein Item
                    doppelt geklickt wird. Ob das Item oder der
                    Trenner angeklickt wurden, kann mit IsItemMode()
                    abgefragt werden. Wenn ein Trenner doppelt angeklickt
                    wird, sollte normalerweise die optimale Spaltenbreite
                    berechnet werden und diese gesetzt werden.
StartDrag()         Dieser Handler wird gerufen, wenn Draggen gestartet
                    wird, bzw. wenn ein Item angeklickt wurde.
                    In diesem Handler sollte spaetestens mit SetDragSize()
                    die Groesse der Size-Linie gesetzt werden, wenn
                    IsItemMode() sal_False zurueckliefert.
Drag()              Dieser Handler wird gerufen, wenn gedraggt wird. Wenn
                    mit SetDragSize() keine Groesse gesetzt wird, kann
                    dieser Handler dafuer benutzt werden, um die
                    Linie im angrenzenden Fenster selber zu zeichnen. Mit
                    GetDragPos() kann die aktuelle Drag-Position abgefragt
                    werden. Mit IsItemMode() sollte in diesem Fall
                    abgefragt werden, ob auch ein Trenner gedraggt wird.
EndDrag()           Dieser Handler wird gerufen, wenn ein Drag-Vorgang
                    beendet wurde. Wenn im EndDrag-Handler GetCurItemId()
                    0 zurueckliefert, wurde der Drag-Vorgang abgebrochen.
                    Wenn dies nicht der Fall ist und IsItemMode() sal_False
                    zurueckliefert, sollte von dem gedraggten Item
                    die neue Groesse mit GetItemSize() abgefragt werden
                    und entsprechend im dazugehoerigem Control uebernommen
                    werden. Wenn IsItemMode() sal_True, GetCurItemId() eine Id
                    und IsItemDrag() sal_True zurueckliefert, wurde dieses
                    Item verschoben. Es sollte dann mit GetItemPos() die
                    neue Position abgefragt werden und auch die Daten
                    im dazugehoerigem Control angepasst werden. Ansonsten
                    koennte auch mit GetItemDragPos() die Position abgefragt
                    werden, an welche Stelle das Item verschoben wurde.


Weitere Methoden, die fuer die Handler wichtig sind.

GetCurItemId()      Liefert die Id vom Item zurueck, fuer welches gerade
                    der Handler gerufen wurde. Liefert nur eine gueltige
                    Id in den Handlern Select(), DoubleClick(), StartDrag(),
                    Drag() und EndDrag(). Im EndDrag-Handler leifert
                    diese Methode die Id vom gedraggten Item zurueck oder
                    0, wenn der Drag-Vorgang abgebrochen wurde.
GetItemDragPos()    Liefert die Position zurueck, an der ein Item verschoben
                    wird bzw. wurde. HEADERBAR_ITEM_NOTFOUND wird
                    zurueckgeliefert, wenn der Vorgang abgebrochen wurde
                    oder wenn kein ItemDrag aktiv ist.
IsItemMode()        Mit dieser Methode kann abgefragt werden, ob fuer ein
                    Item oder einen Trenner der Handler gerufen wurde.
                    sal_True    - Handler wurde fuer das Item gerufen
                    sal_False   - Handler wurde fuer den Trenner gerufen
IsItemDrag()        Mit dieser Methode kann abgefragt werden, ob ein
                    Item gedragt oder selektiert wurde.
                    sal_True    - Item wird verschoben
                    sal_False   - Item wird selektiert
SetDragSize()       Mit dieser Methode wird gesetzt, wir gross der
                    Trennstrich sein soll, der vom Control gemalt wird.
                    Dies sollte so gross sein, wie das angrenzende Fenster
                    hoch ist. Die Hoehe vom HeaderBar wird automatisch
                    dazugerechnet.

--------------------------------------------------------------------------

Weitere Methoden

SetOffset()             Mit dieser Methode wird der Offset gesetzt, ab dem
                        die Items ausgegeben werden. Dies wird benoetigt,
                        wenn das dazugehoerige Fenster gescrollt wird.
CalcWindowSizePixel()   Mit dieser Methode kann man die Hoehe des Fensters
                        berechnen, damit der Inhalt der Items ausgegeben
                        werden kann.

--------------------------------------------------------------------------

Tips und Tricks:

1) KontextMenu
Wenn ein kontextsensitives PopupMenu anzeigt werden soll, muss der
Command-Handler ueberlagert werden. Mit GetItemId() und bei
Uebergabe der Mausposition kann ermittelt werden, ob der Mausclick
ueber einem bzw. ueber welchem Item durchgefuehrt wurde.

2) Letztes Item
Wenn man ButtonStyle gesetzt hat, sieht es besser aus, wenn man am
Ende noch ein leeres Item setzt, was den restlichen Platz einnimmt.
Dazu fuegt man ein Item mit einem leeren String ein und uebergibt als
Groesse HEADERBAR_FULLSIZE. Bei diesem Item sollte man dann auch
nicht HIB_CLICKABLE setzen und dafuer HIB_FIXEDPOS.

*************************************************************************/

class Accelerator;
class ImplHeadItem;

typedef ::std::vector< ImplHeadItem* > ImplHeadItemList;

// -----------
// - WinBits -
// -----------

#define WB_BOTTOMBORDER         ((WinBits)0x0400)
#define WB_BUTTONSTYLE          ((WinBits)0x0800)
#define WB_STDHEADERBAR         (WB_BUTTONSTYLE | WB_BOTTOMBORDER)

// ---------------------
// - HeaderBarItemBits -
// ---------------------

typedef sal_uInt16 HeaderBarItemBits;

// ----------------------------
// - Bits fuer HeaderBarItems -
// ----------------------------

#define HIB_LEFT                ((HeaderBarItemBits)0x0001)
#define HIB_CENTER              ((HeaderBarItemBits)0x0002)
#define HIB_RIGHT               ((HeaderBarItemBits)0x0004)
#define HIB_TOP                 ((HeaderBarItemBits)0x0008)
#define HIB_VCENTER             ((HeaderBarItemBits)0x0010)
#define HIB_BOTTOM              ((HeaderBarItemBits)0x0020)
#define HIB_LEFTIMAGE           ((HeaderBarItemBits)0x0040)
#define HIB_RIGHTIMAGE          ((HeaderBarItemBits)0x0080)
#define HIB_FIXED               ((HeaderBarItemBits)0x0100)
#define HIB_FIXEDPOS            ((HeaderBarItemBits)0x0200)
#define HIB_CLICKABLE           ((HeaderBarItemBits)0x0400)
#define HIB_FLAT                ((HeaderBarItemBits)0x0800)
#define HIB_DOWNARROW           ((HeaderBarItemBits)0x1000)
#define HIB_UPARROW             ((HeaderBarItemBits)0x2000)
#define HIB_USERDRAW            ((HeaderBarItemBits)0x4000)
#define HIB_STDSTYLE            (HIB_LEFT | HIB_LEFTIMAGE | HIB_VCENTER | HIB_CLICKABLE)

// -------------------
// - HeaderBar-Types -
// -------------------

#define HEADERBAR_APPEND            ((sal_uInt16)0xFFFF)
#define HEADERBAR_ITEM_NOTFOUND     ((sal_uInt16)0xFFFF)
#define HEADERBAR_FULLSIZE          ((long)1000000000)

#define HEADERBAR_TEXTOFF           2

// -------------
// - HeaderBar -
// -------------

class SVT_DLLPUBLIC HeaderBar : public Window
{
private:
    ImplHeadItemList*   mpItemList;
    long                mnBorderOff1;
    long                mnBorderOff2;
    long                mnOffset;
    long                mnDX;
    long                mnDY;
    long                mnDragSize;
    long                mnStartPos;
    long                mnDragPos;
    long                mnMouseOff;
    sal_uInt16              mnCurItemId;
    sal_uInt16              mnItemDragPos;
    sal_Bool                mbDragable;
    sal_Bool                mbDrag;
    sal_Bool                mbItemDrag;
    sal_Bool                mbOutDrag;
    sal_Bool                mbButtonStyle;
    sal_Bool                mbItemMode;
    Link                maStartDragHdl;
    Link                maDragHdl;
    Link                maEndDragHdl;
    Link                maSelectHdl;
    Link                maDoubleClickHdl;
    Link                maCreateAccessibleHdl;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >
                        mxAccessible;

    using Window::ImplInit;
    SVT_DLLPRIVATE void             ImplInit( WinBits nWinStyle );
    SVT_DLLPRIVATE void             ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
    SVT_DLLPRIVATE long             ImplGetItemPos( sal_uInt16 nPos ) const;
    SVT_DLLPRIVATE Rectangle            ImplGetItemRect( sal_uInt16 nPos ) const;
    using Window::ImplHitTest;
    SVT_DLLPRIVATE sal_uInt16               ImplHitTest( const Point& rPos, long& nMouseOff, sal_uInt16& nPos ) const;
    SVT_DLLPRIVATE void             ImplInvertDrag( sal_uInt16 nStartPos, sal_uInt16 nEndPos );
    SVT_DLLPRIVATE void             ImplDrawItem( OutputDevice* pDev,
                                      sal_uInt16 nPos, sal_Bool bHigh, sal_Bool bDrag,
                                      const Rectangle& rItemRect,
                                      const Rectangle* pRect,
                                      sal_uLong nFlags );
    SVT_DLLPRIVATE void             ImplDrawItem( sal_uInt16 nPos, sal_Bool bHigh = sal_False,
                                      sal_Bool bDrag = sal_False,
                                      const Rectangle* pRect = NULL );
    SVT_DLLPRIVATE void             ImplUpdate( sal_uInt16 nPos,
                                    sal_Bool bEnd = sal_False, sal_Bool bDirect = sal_False );
    SVT_DLLPRIVATE void             ImplStartDrag( const Point& rPos, sal_Bool bCommand );
    SVT_DLLPRIVATE void             ImplDrag( const Point& rPos );
    SVT_DLLPRIVATE void             ImplEndDrag( sal_Bool bCancel );

public:
                        HeaderBar( Window* pParent, WinBits nWinBits = WB_STDHEADERBAR );
                        HeaderBar( Window* pParent, const ResId& rResId );
                        ~HeaderBar();

    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        MouseMove( const MouseEvent& rMEvt );
    virtual void        Tracking( const TrackingEvent& rTEvt );
    virtual void        Paint( const Rectangle& rRect );
    virtual void        Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void        Resize();
    virtual void        Command( const CommandEvent& rCEvt );
    virtual void        RequestHelp( const HelpEvent& rHEvt );
    virtual void        StateChanged( StateChangedType nStateChange );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

    virtual void        UserDraw( const UserDrawEvent& rUDEvt );
    virtual void        StartDrag();
    virtual void        Drag();
    virtual void        EndDrag();
    virtual void        Select();
    virtual void        DoubleClick();

    void                InsertItem( sal_uInt16 nItemId, const XubString& rText,
                                    long nSize, HeaderBarItemBits nBits = HIB_STDSTYLE,
                                    sal_uInt16 nPos = HEADERBAR_APPEND );
    void                RemoveItem( sal_uInt16 nItemId );
    void                MoveItem( sal_uInt16 nItemId, sal_uInt16 nNewPos );
    void                Clear();

    void                SetOffset( long nNewOffset = 0 );
    long                GetOffset() const { return mnOffset; }
    inline void         SetDragSize( long nNewSize = 0 ) { mnDragSize = nNewSize; }
    long                GetDragSize() const { return mnDragSize; }

    sal_uInt16              GetItemCount() const;
    sal_uInt16              GetItemPos( sal_uInt16 nItemId ) const;
    sal_uInt16              GetItemId( sal_uInt16 nPos ) const;
    sal_uInt16              GetItemId( const Point& rPos ) const;
    Rectangle           GetItemRect( sal_uInt16 nItemId ) const;
    sal_uInt16              GetCurItemId() const { return mnCurItemId; }
    long                GetDragPos() const { return mnDragPos; }
    sal_uInt16              GetItemDragPos() const { return mnItemDragPos; }
    sal_Bool                IsItemMode() const { return mbItemMode; }
    sal_Bool                IsItemDrag() const { return mbItemDrag; }

    void                SetItemSize( sal_uInt16 nItemId, long nNewSize );
    long                GetItemSize( sal_uInt16 nItemId ) const;
    void                SetItemBits( sal_uInt16 nItemId, HeaderBarItemBits nNewBits );
    HeaderBarItemBits   GetItemBits( sal_uInt16 nItemId ) const;

    void                SetItemText( sal_uInt16 nItemId, const XubString& rText );
    XubString           GetItemText( sal_uInt16 nItemId ) const;

    XubString           GetHelpText( sal_uInt16 nItemId ) const;
    OString        GetHelpId( sal_uInt16 nItemId ) const;

    Size                CalcWindowSizePixel() const;

    inline void             SetHelpText( const OUString& rText )    { Window::SetHelpText( rText ); }
    inline const OUString&  GetHelpText() const                     { return Window::GetHelpText(); }
    inline void             SetHelpId( const OString& rId )    { Window::SetHelpId( rId ); }
    inline const OString& GetHelpId() const                    { return Window::GetHelpId(); }

    inline void         SetStartDragHdl( const Link& rLink )        { maStartDragHdl = rLink; }
    inline const Link&  GetStartDragHdl() const                     { return maStartDragHdl; }
    inline void         SetDragHdl( const Link& rLink )             { maDragHdl = rLink; }
    inline const Link&  GetDragHdl() const                          { return maDragHdl; }
    inline void         SetEndDragHdl( const Link& rLink )          { maEndDragHdl = rLink; }
    inline const Link&  GetEndDragHdl() const                       { return maEndDragHdl; }
    inline void         SetSelectHdl( const Link& rLink )           { maSelectHdl = rLink; }
    inline const Link&  GetSelectHdl() const                        { return maSelectHdl; }
    inline void         SetDoubleClickHdl( const Link& rLink )      { maDoubleClickHdl = rLink; }
    inline const Link&  GetDoubleClickHdl() const                   { return maDoubleClickHdl; }
    inline void         SetCreateAccessibleHdl( const Link& rLink ) { maCreateAccessibleHdl = rLink; }
    inline const Link&  GetCreateAccessibleHdl() const              { return maCreateAccessibleHdl; }

    inline sal_Bool         IsDragable() const                          { return mbDragable; }

    /** Creates and returns the accessible object of the header bar. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >  CreateAccessible();
    void SetAccessible( ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > );
};

#endif  // _HEADBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
