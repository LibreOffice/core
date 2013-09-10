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

#ifndef _RULER_HXX
#define _RULER_HXX

#include "svtools/svtdllapi.h"
#include <tools/link.hxx>
#include <vcl/window.hxx>
#include <vcl/virdev.hxx>
#include <vcl/field.hxx>

#include <boost/scoped_ptr.hpp>

class MouseEvent;
class TrackingEvent;
class DataChangedEvent;

/*************************************************************************

Description
============

class Ruler

This class is used for displaying a ruler, but it can also be used
for setting or moving tabs and margins.

--------------------------------------------------------------------------

WinBits

WB_HORZ             ruler is displayed horizontally
WB_VERT             ruler is displayed vertically
WB_3DLOOK           3D look
WB_BORDER           border at the bottom/right margin
WB_EXTRAFIELD       Field in the upper left corner for
                    displaying and selecting tabs, origin of coordinates, ...
WB_RIGHT_ALIGNED    Marks the vertical ruler as right aligned

--------------------------------------------------------------------------

All ruler parameters are set in pixel units. This way double conversions
and rounding errors are avoided and the ruler displays the margins
at their actual position in the document. Because of this, the application can,
for example in tables, do its own roundings and the positions on the ruler will
still match those in the document. However, for the ruler to know how the
document is displayed on the screen, some additional values have to be configured

SetWinPos() sets the offset of the ruler's edit window. In doing so,
the width of the window can also be configured. If there is a 0 among the
values passed to the function, the position/width is automatically set to
the width of the ruler.

SetPagePos() sets the offset of the page relative to the edit window and the
width of the page. If there is a 0 among the values passed to the function,
the position/width is automatically set as if the page filled the whole edit window.

SetBorderPos() sets the offset of the border. The position is relative to
the upper/left margin of the window. This is needed when there are a horizontal
and a vertical ruler visible at the same time. Example:
        aHRuler.SetBorderPos( aVRuler.GetSizePixel().Width()-1 );

SetNullOffset() sets the origin relative to the page.

All the other values (margins, indentation, tabs, ...) refer to the origin,
which is set with SetNullOffset().

The values are computed as described below:

- WinPos (if both windows have the same parent)

    Point aHRulerPos = aHRuler.GetPosPixel();
    Point aEditWinPos = aEditWin.GetPosPixel();
    aHRuler.SetWinPos( aEditWinPos().X() - aHRulerPos.X() );

- PagePos

    Point aPagePos = aEditWin.LogicToPixel( aEditWin.GetPagePos() );
    aHRuler.SetPagePos( aPagePos().X() );

- All other values

    Add the logical values, recompute as position and subtract the
    previously saved pixel positions (of PagePos and Null Offset).

--------------------------------------------------------------------------

SetUnit() and SetZoom() configure which unit is used to display
the values on the ruler. The following units are accepted:

    FUNIT_MM
    FUNIT_CM (Default)
    FUNIT_M
    FUNIT_KM
    FUNIT_INCH
    FUNIT_FOOT
    FUNIT_MILE
    FUNIT_POINT
    FUNIT_PICA

--------------------------------------------------------------------------

SetMargin1() sets the upper/left margin and SetMargin2() sets the
bottom/right margin. If these methods are called without arguments,
no margins are displayed. Otherwise, the following arguments can be passed:

    long    nPos            - offset in pixels relative to the origin
    sal_uInt16 nStyle       - bit style:
                                RULER_MARGIN_SIZEABLE
                                margin size can be changed

                                The following bits can be set in addition
                                to these styles:
                                RULER_STYLE_INVISIBLE


SetBorders() sets an array of margins. To do this, an array of type RulerBorder
has to be passed. In the array, the following values have to be initialized:

    long    nPos            - offset in pixels relative to the origin
    long    nWidth          - column spacing in pixels (can also be 0, for example,
                              for table columns)
    sal_uInt16 nStyle       - bit style:
                                RULER_BORDER_SIZEABLE
                                Column spacing can be changed. This flag should
                                only be set if the size of the spacing is changed,
                                not that of a cell.
                                RULER_BORDER_MOVEABLE
                                Column spacing/border can be moved. Whenever
                                table borders are to be moved, this flag should
                                be set instead of SIZEABLE (SIZEABLE indicates
                                that the size of a spacing, not that of a single
                                cell can be changed).
                                RULER_BORDER_VARIABLE
                                Not all of the column spacings are equal
                                RULER_BORDER_TABLE
                                Table border. Whenever this style ist set, the column
                                width must be 0.
                                RULER_BORDER_SNAP
                                Auxiliary line. Whenever this style is set, the
                                column width must be 0.
                                RULER_BORDER_MARGIN
                                Margin. Whenever this style is set, the column
                                width must be 0.

                                The following bits can be set in addition
                                to these styles:
                                RULER_STYLE_INVISIBLE

SetIndents() sets an array of indents. This method may only be used for horizontal
rulers. A Ruler Indent must be passed as an argument, with the following values
initialized:

    long    nPos            - offset relative to the origin in pixels
    sal_uInt16 nStyle       - bit style:
                                RULER_INDENT_TOP    (indent of the first line)
                                RULER_INDENT_BOTTOM (left/right indent)
                                RULER_INDENT_BORDER (Vertical line that shows the border distance)
                                The following bits can be set in addition
                                to these styles:
                                RULER_STYLE_DONTKNOW (for old position or for
                                                     ambiguity)
                                RULER_STYLE_INVISIBLE

SetTabs() sets an array of tabs. This method may only be used for horizontal rulers.
An array of type RulerTab must be passed as an argument, with the following values
initialized:

    long    nPos            - offset relative to the origin in pixels
    sal_uInt16 nStyle       - bit style:
                                RULER_TAB_DEFAULT (can't be selected)
                                RULER_TAB_LEFT
                                RULER_TAB_CENTER
                                RULER_TAB_RIGHT
                                RULER_TAB_DECIMAL
                                The following bits can be set in addition
                                to these styles:
                                RULER_STYLE_DONTKNOW (for old position of for
                                                     ambiguity)
                                RULER_STYLE_INVISIBLE

SetLines() displays position lines in the ruler. An array of type RulerLine must be passed, with
the following values initialized:

    long    nPos            - offset relative to the origin in pixels
    sal_uInt16 nStyle       - bit style (has to be 0 currently)

--------------------------------------------------------------------------

If the user should also be able to change the margins tabs, borders, ...
in the ruler, a bit more effort is necessary. In this case, the StartDrag(),
Drag() and EndDrag() methods have to be overridden. For the StartDrag() method
it is possible to prevent dragging by returning FALSE. In the drag handler,
the drag position must be queried and the values must be moved to the new
position. This is done by calling the particular Set methods. While in the
drag handler, the values are just cached and only afterward the ruler is redrawn.
All the handlers can also be set as links with the particular Set..Hdl() methods.

    - StartDrag()
        Is called when dragging is started. If FALSE is returned, the dragging.
        won't be executed. If TRUE is returned, the dragging will be permitted.
        If the handler isn't overridden, FALSE will be returned.

    - EndDrag()
        Is called at the end of dragging.

    - Drag()
        Is called when dragging takes place.

    - Click()
        This handler is called when no element has been clicked on.
        The position can be queried with GetClickPos(). This way it is possible
        to, for example, ser tabs in the ruler. After calling the click handler,
        the drag, if any, is immediately triggered. This makes it possible to
        set a new tab in the click handler and then immediately move it.

    - DoubleClick()
        This handler is called when a double-click has been performed outside
        the special panel. The methods GetClickType(), GetClickAryPos() and
        GetClickPos() can be used to query what has been clicked on.
        This way you can, for example, show the tab dialog when a double-click
        is performed on a tab.

In the drag handler it is possible to query what has been dragged and where
it has been dragged. There are the following query methods:

    - GetDragType()
        Returns what has been dragged.
            RULER_TYPE_MARGIN1
            RULER_TYPE_MARGIN2
            RULER_TYPE_BORDER
            RULER_TYPE_INDENT
            RULER_TYPE_TAB

    - GetDragPos()
        Returns the pixel position to which the user has moved the mouse
        relative to the set zero-offset.

    - GetDragAryPos()
        Liefert den Index im Array zurueck, wenn ein Border, Indent oder ein
        Tab gedragt wird. Achtung: Es wird die Array-Position waehrend des
        gesammten Drag-Vorgangs von dem Item im Array was vor dem Drag gesetzt
        war zurueckgeben. Dadurch ist es zum Beispiel auch moeglich, einen
        Tab nicht mehr anzuzeigen, wenn die Maus nach unten/rechts aus dem
        Lineal gezogen wird.

    - GetDragSize()
        Wenn Borders gedragt werden, kann hierueber abgefragt werden, ob
        die Groesse bzw. welche Seite oder die Position geaendert werden soll.
            RULER_DRAGSIZE_MOVE oder 0      - Move
            RULER_DRAGSIZE_1                - Linke/obere Kante
            RULER_DRAGSIZE_2                - Rechte/untere Kante

    - IsDragDelete()
        Mit dieser Methode kann abgefragt werden, ob beim Draggen die
        Maus unten/rechts aus dem Fenster gezogen wurde. Damit kann
        zum Beispiel festgestellt werden, ob der Benutzer einen Tab
        loeschen will.

    - IsDragCanceled()
        Mit dieser Methode kann im EndDrag-Handler abgefragt werden,
        ob die Aktion abgebrochen wurde, indem der Anwender die
        Maus oben/links vom Fenster losgelassen hat oder ESC gedrueckt
        hat. In diesem Fall werden die Werte nicht uebernommen. Wird
        waehrend des Draggings die Maus oben/links aus dem Fenster
        gezogen, werden automatisch die alten Werte dargestellt, ohne das
        der Drag-Handler gerufen wird.
        Falls der Benutzer jedoch den Wert auf die alte Position
        zurueckgeschoben hat, liefert die Methode trotzdem sal_False. Falls
        dies vermieden werden soll, muss sich die Applikation im StartDrag-
        Handler den alten Wert merken und im EndDrag-Handler den Wert
        vergleichen.

    - GetDragScroll()
        Mit dieser Methode kann abgefragt werden, ob gescrollt werden
        soll. Es wird einer der folgenden Werte zurueckgegeben:
            RULER_SCROLL_NO                 - Drag-Position befindet sich
                                              an keinem Rand und somit
                                              muss nicht gescrollt werden.
            RULER_SCROLL_1                  - Drag-Position befindet sich
                                              am linken/oberen Rand und
                                              somit sollte das Programm evt.
                                              ein Srcoll ausloesen.
            RULER_SCROLL_2                  - Drag-Position befindet sich
                                              am rechten/unteren Rand und
                                              somit sollte das Programm evt.
                                              ein Srcoll ausloesen.

    - GetDragModifier()
        Liefert die Modifier-Tasten zurueck, die beim Starten des Drag-
        Vorgangs gedrueckt waren. Siehe MouseEvent.

    - GetClickPos()
        Liefert die Pixel-Position bezogen auf den eingestellten Null-Offset
        zurueck, wo der Anwender die Maus gedrueckt hat.

    - GetClickType()
        Liefert zurueck, was per DoubleClick betaetigt wird:
            RULER_TYPE_DONTKNOW             (kein Element im Linealbereich)
            RULER_TYPE_OUTSIDE              (ausserhalb des Linealbereichs)
            RULER_TYPE_MARGIN1              (nur Margin1-Kante)
            RULER_TYPE_MARGIN2              (nur Margin2-Kante)
            RULER_TYPE_BORDER               (Border: GetClickAryPos())
            RULER_TYPE_INDENT               (Einzug: GetClickAryPos())
            RULER_TYPE_TAB                  (Tab: GetClickAryPos())

    - GetClickAryPos()
        Liefert den Index im Array zurueck, wenn ein Border, Indent oder ein
        Tab per DoubleClick betaetigt wird.

    - GetType()
        Mit dieser Methode kann man einen HitTest durchfuehren, um
        gegebenenfalls ueber das Abfangen des MouseButtonDown-Handlers
        auch ueber die rechte Maustaste etwas auf ein Item anzuwenden. Als
        Paramter ueber gibt man die Fensterposition und gegebenenfalls
        einen Pointer auf einen sal_uInt16, um die Array-Position eines
        Tabs, Indent oder Borders mitzubekommen. Als Type werden folgende
        Werte zurueckgegeben:
            RULER_TYPE_DONTKNOW             (kein Element im Linealbereich)
            RULER_TYPE_OUTSIDE              (ausserhalb des Linealbereichs)
            RULER_TYPE_MARGIN1              (nur Margin1-Kante)
            RULER_TYPE_MARGIN2              (nur Margin2-Kante)
            RULER_TYPE_BORDER               (Border: GetClickAryPos())
            RULER_TYPE_INDENT               (Einzug: GetClickAryPos())
            RULER_TYPE_TAB                  (Tab: GetClickAryPos())

Wenn der Drag-Vorgang abgebrochen werden soll, kann der Drag-Vorgang
mit CancelDrag() abgebrochen werden. Folgende Methoden gibt es fuer die
Drag-Steuerung:

    - IsDrag()
        Liefert sal_True zurueck, wenn sich das Lineal im Drag-Vorgang befindet.

    - CancelDrag()
        Bricht den Drag-Vorgang ab, falls einer durchgefuehrt wird. Dabei
        werden die alten Werte wieder hergestellt und der Drag und der
        EndDrag-Handler gerufen.

Um vom Dokument ein Drag auszuloesen, gibt es folgende Methoden:

    - StartDocDrag()
        Dieser Methode werden der MouseEvent vom Dokumentfenster und
        was gedragt werden soll uebergeben. Wenn als DragType
        RULER_TYPE_DONTKNOW uebergeben wird, bestimmt das Lineal, was
        verschoben werden soll. Bei den anderen, wird der Drag nur dann
        gestartet, wenn auch an der uebergebenen Position ein entsprechendes
        Element gefunden wurde. Dies ist zun Beispiel dann notwendig, wenn
        zum Beispiel Einzuege und Spalten an der gleichen X-Position liegen.
        Der Rueckgabewert gibt an, ob der Drag ausgeloest wurde. Wenn ein
        Drag ausgeloest wird, uebernimmt das Lineal die normale Drag-Steuerung
        und verhaelt sich dann so, wie als wenn direkt in das Lineal geklickt
        wurde. So captured das Lineal die Mouse und uebernimmt auch die
        Steuerung des Cancel (ueber Tastatur, oder wenn die Mouse ueber
        oder links vom Lineal ruasgeschoben wird). Auch alle Handler werden
        gerufen (inkl. des StartDrag-Handlers). Wenn ein MouseEvent mit
        Click-Count 2 uebergeben wird auch der DoubleClick-Handler
        entsprechend gerufen.

--------------------------------------------------------------------------

Fuer das Extra-Feld kann der Inhalt bestimmt werden und es gibt Handler,
womit man bestimmte Aktionen abfangen kann.

    - ExtraDown()
        Dieser Handler wird gerufen, wenn im Extra-Feld die Maus
        gedrueckt wird.

    - SetExtraType()
        Mit dieser Methode kann festgelegt werden, was im ExtraFeld
        dargestellt werden soll.
            - ExtraType         Was im Extrafeld dargestellt werden soll
                                RULER_EXTRA_DONTKNOW        (Nichts)
                                RULER_EXTRA_NULLOFFSET      (Koordinaaten-Kreuz)
                                RULER_EXTRA_TAB             (Tab)
            - sal_uInt16 nStyle     Bitfeld als Style:
                                    RULER_STYLE_HIGHLIGHT   (selektiert)
                                    RULER_TAB_...           (ein Tab-Style)

    - GetExtraClick()
        Liefert die Anzahl der Mausclicks zurueck. Dadurch ist es zum
        Beispiel auch moeglich, auch durch einen DoubleClick im Extrafeld
        eine Aktion auszuloesen.

    - GetExtraModifier()
        Liefert die Modifier-Tasten zurueck, die beim Klicken in das Extra-
        Feld gedrueckt waren. Siehe MouseEvent.

--------------------------------------------------------------------------

Weitere Hilfsfunktionen:

- static Ruler::DrawTab()
    Mit dieser Methode kann ein Tab auf einem OutputDevice ausgegeben
    werden. Dadurch ist es moeglich, auch in Dialogen die Tabs so
    anzuzeigen, wie Sie im Lineal gemalt werden.

    Diese Methode gibt den Tab zentriert an der uebergebenen Position
    aus. Die Groesse der Tabs kann ueber die Defines RULER_TAB_WIDTH und
    RULER_TAB_HEIGHT bestimmt werden.

--------------------------------------------------------------------------

Tips zur Benutzung des Lineals:

- Bei dem Lineal muss weder im Drag-Modus noch sonst das Setzen der Werte
  in SetUpdateMode() geklammert werden. Denn das Lineal sorgt von sich
  aus dafuer, das wenn mehrere Werte gesetzt werden, diese automatisch
  zusammengefast werden und flackerfrei ausgegeben werden.

- Initial sollten beim Lineal zuerst die Groessen, Positionen und Werte
  gesetzt werden, bevor es angezeigt wird. Dies ist deshalb wichtig, da
  ansonsten viele Werte unnoetig berechnet werden.

- Wenn das Dokumentfenster, in dem sich das Lineal befindet aktiv bzw.
  deaktiv wird, sollten die Methoden Activate() und Deactivate() vom
  Lineal gerufen werden. Denn je nach Einstellungen und System wird die
  Anzeige entsprechend umgeschaltet.

- Zum Beispiel sollte beim Drag von Tabs und Einzuegen nach Moeglichkeit die
  alten Positionen noch mit angezeigt werden. Dazu sollte zusaetzlich beim
  Setzen der Tabs und Einzuege als erstes im Array die alten Positionen
  eingetragen werden und mit dem Style RULER_STYLE_DONTKNOW verknuepft
  werden. Danach sollte im Array die restlichen Werte eingetragen werden.

- Bei mehreren markierten Absaetzen und Tabellen-Zellen, sollten die Tabs
  und Einzuege in grau von der ersten Zelle, bzw. vom ersten Absatz
  angezeigt werden. Dies kann man auch ueber den Style RULER_STYLE_DONTKNOW
  erreichen.

- Die Bemassungspfeile sollten immer dann angezeigt, wenn beim Drag die
  Alt-Taste (WW-Like) gedrueckt wird. Vielleicht sollte diese Einstellung
  auch immer vornehmbar sein und vielleicht beim Drag immer die
  Bemassungspfeile dargestellt werden. Bei allen Einstellung sollten die
  Werte immer auf ein vielfaches eines Wertes gerundet werden, da die
  Bildschirmausloesung sehr ungenau ist.

- DoppelKlicks sollten folgendermassen behandelt werden (GetClickType()):
    - RULER_TYPE_DONTKNOW
      RULER_TYPE_MARGIN1
      RULER_TYPE_MARGIN2
        Wenn die Bedingunden GetClickPos() <= GetMargin1() oder
        GetClickPos() >= GetMargin2() oder der Type gleich
        RULER_TYPE_MARGIN1 oder RULER_TYPE_MARGIN2 ist, sollte
        ein SeitenDialog angezeigt werden, wo der Focus auf dem
        entsprechenden Rand steht
    - RULER_TYPE_BORDER
        Es sollte ein Spalten- oder Tabellen-Dialog angezeigt werden,
        wo der Focus auf der entsprechenden Spalte steht, die mit
        GetClickAryPos() abgefragt werden kann.
    - RULER_TYPE_INDENT
        Es sollte der Dialog angezeigt werden, wo die Einzuege eingestellt
        werden koennen. Dabei sollte der Focus auf dem Einzug stehen, der
        mit GetClickAryPos() ermittelt werden kann.
    - RULER_TYPE_TAB
        Es sollte ein TabDialog angezeigt werden, wo der Tab selektiert
        sein sollte, der ueber GetClickAryPos() abgefragt werden kann.

*************************************************************************/

// - WinBits -

#define WB_EXTRAFIELD     ((WinBits)0x00004000)
#define WB_RIGHT_ALIGNED  ((WinBits)0x00008000)
#define WB_STDRULER       WB_HORZ

// - Ruler-Type -

enum RulerType { RULER_TYPE_DONTKNOW, RULER_TYPE_OUTSIDE,
                 RULER_TYPE_MARGIN1, RULER_TYPE_MARGIN2,
                 RULER_TYPE_BORDER, RULER_TYPE_INDENT, RULER_TYPE_TAB };

enum RulerExtra { RULER_EXTRA_DONTKNOW,
                  RULER_EXTRA_NULLOFFSET, RULER_EXTRA_TAB };

#define RULER_STYLE_HIGHLIGHT   ((sal_uInt16)0x8000)
#define RULER_STYLE_DONTKNOW    ((sal_uInt16)0x4000)
#define RULER_STYLE_INVISIBLE   ((sal_uInt16)0x2000)

#define RULER_DRAGSIZE_MOVE     0
#define RULER_DRAGSIZE_1        1
#define RULER_DRAGSIZE_2        2

#define RULER_MOUSE_BORDERMOVE  5
#define RULER_MOUSE_BORDERWIDTH 5
#define RULER_MOUSE_MARGINWIDTH 3

#define RULER_SCROLL_NO         0
#define RULER_SCROLL_1          1
#define RULER_SCROLL_2          2

// - RulerMargin -

#define RULER_MARGIN_SIZEABLE   ((sal_uInt16)0x0001)

// - RulerBorder -

#define RULER_BORDER_SIZEABLE   ((sal_uInt16)0x0001)
#define RULER_BORDER_MOVEABLE   ((sal_uInt16)0x0002)
#define RULER_BORDER_VARIABLE   ((sal_uInt16)0x0004)
#define RULER_BORDER_TABLE      ((sal_uInt16)0x0008)
#define RULER_BORDER_SNAP       ((sal_uInt16)0x0010)
#define RULER_BORDER_MARGIN     ((sal_uInt16)0x0020)

struct RulerBorder
{
    long        nPos;
    long        nWidth;
    sal_uInt16  nStyle;
    long        nMinPos; //minimum/maximum position, supported for table borders/rows
    long        nMaxPos;
};

// - RulerIndent -

#define RULER_INDENT_TOP        ((sal_uInt16)0x0000)
#define RULER_INDENT_BOTTOM     ((sal_uInt16)0x0001)
#define RULER_INDENT_BORDER     ((sal_uInt16)0x0002)
#define RULER_INDENT_STYLE      ((sal_uInt16)0x000F)

struct RulerIndent
{
    long        nPos;
    sal_uInt16  nStyle;
};

// - RulerTab -

#define RULER_TAB_LEFT          ((sal_uInt16)0x0000)
#define RULER_TAB_RIGHT         ((sal_uInt16)0x0001)
#define RULER_TAB_CENTER        ((sal_uInt16)0x0002)
#define RULER_TAB_DECIMAL       ((sal_uInt16)0x0003)
#define RULER_TAB_DEFAULT       ((sal_uInt16)0x0004)
#define RULER_TAB_STYLE         ((sal_uInt16)0x000F)
#define RULER_TAB_RTL           ((sal_uInt16)0x0010)

struct RulerTab
{
    long        nPos;
    sal_uInt16  nStyle;
};

#define RULER_TAB_WIDTH         7
#define RULER_TAB_HEIGHT        6

// - RulerLine -

struct RulerLine
{
    long    nPos;
    sal_uInt16  nStyle;
};

struct RulerSelection
{
    long        nPos;
    RulerType   eType;
    sal_uInt16  nAryPos;
    sal_uInt16  mnDragSize;
    bool        bSize;
    bool        bSizeBar;
    bool        bExpandTest;

    RulerSelection() :
        bExpandTest( false )
    {}
};

struct RulerUnitData
{
    MapUnit         eMapUnit;           // MAP_UNIT for calculaion
    long            nTickUnit;          // Unit divider
    double          nTick1;             // Minimal step
    double          nTick2;             // Tick quarter unit
    double          nTick3;             // Tick half unit
    double          nTick4;             // Tick whole unit
    long            n100THMM;           // 100mm Unit divider
    sal_uInt16      nUnitDigits;        // Number of digits
    sal_Char        aUnitStr[8];        // Unit string
};

// - Ruler -

class ImplRulerData;

class SVT_DLLPUBLIC Ruler : public Window
{
private:
    VirtualDevice   maVirDev;
    MapMode         maMapMode;
    long            mnBorderOff;
    long            mnWinOff;
    long            mnWinWidth;
    long            mnWidth;
    long            mnHeight;
    long            mnVirOff;
    long            mnVirWidth;
    long            mnVirHeight;
    long            mnBorderWidth;
    long            mnStartDragPos;
    long            mnDragPos;
    sal_uLong       mnUpdateEvtId;
    ImplRulerData*  mpSaveData;
    ImplRulerData*  mpData;
    ImplRulerData*  mpDragData;
    Rectangle       maExtraRect;
    WinBits         mnWinStyle;
    sal_uInt16      mnUnitIndex;
    sal_uInt16      mnDragAryPos;
    sal_uInt16      mnDragSize;
    sal_uInt16      mnDragScroll;
    sal_uInt16      mnDragModifier;
    sal_uInt16      mnExtraStyle;
    sal_uInt16      mnExtraClicks;
    sal_uInt16      mnExtraModifier;
    long            mnCharWidth;
    long            mnLineHeight;

    RulerExtra      meExtraType;
    RulerType       meDragType;
    MapUnit         meSourceUnit;
    FieldUnit       meUnit;
    Fraction        maZoom;
    sal_Bool        mbCalc;
    sal_Bool        mbFormat;
    sal_Bool        mbDrag;
    sal_Bool        mbDragDelete;
    sal_Bool        mbDragCanceled;
    sal_Bool        mbAutoWinWidth;
    sal_Bool        mbActive;
    sal_uInt8       mnUpdateFlags;

    RulerSelection  maHoverSelection;

    Link            maStartDragHdl;
    Link            maDragHdl;
    Link            maEndDragHdl;
    Link            maClickHdl;
    Link            maDoubleClickHdl;
    Link            maExtraDownHdl;

    boost::scoped_ptr<RulerSelection> mpCurrentHitTest;
    boost::scoped_ptr<RulerSelection> mpPreviousHitTest;

    SVT_DLLPRIVATE void ImplVDrawLine( long nX1, long nY1, long nX2, long nY2 );
    SVT_DLLPRIVATE void ImplVDrawRect( long nX1, long nY1, long nX2, long nY2 );
    SVT_DLLPRIVATE void ImplVDrawText( long nX, long nY, const OUString& rText, long nMin = LONG_MIN, long nMax = LONG_MAX );

    SVT_DLLPRIVATE void ImplDrawTicks( long nMin, long nMax, long nStart, long nVirTop, long nVirBottom );
    SVT_DLLPRIVATE void ImplDrawBorders( long nMin, long nMax, long nVirTop, long nVirBottom );
    SVT_DLLPRIVATE void ImplDrawIndent( const Polygon& rPoly, sal_uInt16 nStyle, bool bIsHit = false );
    SVT_DLLPRIVATE void ImplDrawIndents( long nMin, long nMax, long nVirTop, long nVirBottom );
    SVT_DLLPRIVATE void ImplDrawTab( OutputDevice* pDevice, const Point& rPos, sal_uInt16 nStyle );
    SVT_DLLPRIVATE void ImplDrawTabs( long nMin, long nMax, long nVirTop, long nVirBottom );

    using Window::ImplInit;
    SVT_DLLPRIVATE void ImplInit( WinBits nWinBits );
    SVT_DLLPRIVATE void ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
    SVT_DLLPRIVATE void ImplCalc();
    SVT_DLLPRIVATE void ImplFormat();
    SVT_DLLPRIVATE void ImplInitExtraField( sal_Bool bUpdate );
    SVT_DLLPRIVATE void ImplInvertLines( sal_Bool bErase = sal_False );
    SVT_DLLPRIVATE void ImplDraw();
    SVT_DLLPRIVATE void ImplDrawExtra( sal_Bool bPaint = sal_False );
    SVT_DLLPRIVATE void ImplUpdate( sal_Bool bMustCalc = sal_False );

    using Window::ImplHitTest;
    SVT_DLLPRIVATE sal_Bool ImplHitTest( const Point& rPosition,
                                         RulerSelection* pHitTest,
                                         sal_Bool bRequiredStyle = sal_False,
                                         sal_uInt16 nRequiredStyle = 0 ) const;
    SVT_DLLPRIVATE sal_Bool ImplDocHitTest( const Point& rPos, RulerType eDragType, RulerSelection* pHitTest ) const;
    SVT_DLLPRIVATE sal_Bool ImplStartDrag( RulerSelection* pHitTest, sal_uInt16 nModifier );
    SVT_DLLPRIVATE void     ImplDrag( const Point& rPos );
    SVT_DLLPRIVATE void     ImplEndDrag();

    DECL_DLLPRIVATE_LINK( ImplUpdateHdl, void* );

    // Forbidden and not implemented.
    Ruler (const Ruler &);
    Ruler& operator= (const Ruler &);

protected:
    long            GetRulerVirHeight() const;
    MapMode         GetCurrentMapMode() const { return maMapMode; }
    RulerUnitData   GetCurrentRulerUnit() const;

public:
            Ruler( Window* pParent, WinBits nWinStyle = WB_STDRULER );
    virtual ~Ruler();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    Tracking( const TrackingEvent& rTEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Resize();
    virtual void    StateChanged( StateChangedType nStateChange );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    virtual long    StartDrag();
    virtual void    Drag();
    virtual void    EndDrag();
    virtual void    Click();
    virtual void    DoubleClick();
    virtual void    ExtraDown();

    void            Activate();
    void            Deactivate();
    sal_Bool        IsActive() const { return mbActive; }

    void            SetWinPos( long nOff = 0, long nWidth = 0 );
    long            GetWinOffset() const { return mnWinOff; }
    long            GetWinWidth() const { return mnWinWidth; }
    void            SetPagePos( long nOff = 0, long nWidth = 0 );
    long            GetPageOffset() const;
    long            GetPageWidth() const;
    void            SetBorderPos( long nOff = 0 );
    long            GetBorderOffset() const { return mnBorderOff; }
    Rectangle       GetExtraRect() const { return maExtraRect; }

    void            SetUnit( FieldUnit eNewUnit );
    FieldUnit       GetUnit() const { return meUnit; }
    void            SetZoom( const Fraction& rNewZoom );
    Fraction        GetZoom() const { return maZoom; }

    void            SetSourceUnit( MapUnit eNewUnit ) { meSourceUnit = eNewUnit; }
    MapUnit         GetSourceUnit() const { return meSourceUnit; }

    void            SetExtraType( RulerExtra eNewExtraType, sal_uInt16 nStyle = 0 );
    RulerExtra      GetExtraType() const { return meExtraType; }
    sal_uInt16      GetExtraStyle()  const { return mnExtraStyle; }
    sal_uInt16      GetExtraClicks() const { return mnExtraClicks; }
    sal_uInt16      GetExtraModifier() const { return mnExtraModifier; }

    sal_Bool        StartDocDrag( const MouseEvent& rMEvt,
                                  RulerType eDragType = RULER_TYPE_DONTKNOW );
    RulerType       GetDragType() const { return meDragType; }
    long            GetDragPos() const { return mnDragPos; }
    sal_uInt16      GetDragAryPos() const { return mnDragAryPos; }
    sal_uInt16      GetDragSize() const { return mnDragSize; }
    sal_Bool        IsDragDelete() const { return mbDragDelete; }
    sal_Bool        IsDragCanceled() const { return mbDragCanceled; }
    sal_uInt16      GetDragScroll() const { return mnDragScroll; }
    sal_uInt16      GetDragModifier() const { return mnDragModifier; }
    sal_Bool        IsDrag() const { return mbDrag; }
    void            CancelDrag();
    long            GetClickPos() const { return mnDragPos; }
    RulerType       GetClickType() const { return meDragType; }
    sal_uInt16      GetClickAryPos() const { return mnDragAryPos; }

    RulerSelection  GetHoverSelection() const { return maHoverSelection; }

    using Window::GetType;
    RulerType       GetType( const Point& rPos, sal_uInt16* pAryPos = NULL ) const;

    void            SetNullOffset( long nPos );
    long            GetNullOffset() const;
    void            SetMargin1() { SetMargin1( 0, RULER_STYLE_INVISIBLE ); }
    void            SetMargin1( long nPos, sal_uInt16 nMarginStyle = RULER_MARGIN_SIZEABLE );
    long            GetMargin1() const;
    void            SetMargin2() { SetMargin2( 0, RULER_STYLE_INVISIBLE ); }
    void            SetMargin2( long nPos, sal_uInt16 nMarginStyle = RULER_MARGIN_SIZEABLE );
    long            GetMargin2() const;

    void            SetLeftFrameMargin( long nPos );
    void            SetRightFrameMargin( long nPos );
    void            SetLines( sal_uInt32 n = 0, const RulerLine* pLineAry = NULL );
    void            SetBorders( sal_uInt32 n = 0, const RulerBorder* pBrdAry = NULL );
    void            SetIndents( sal_uInt32 n = 0, const RulerIndent* pIndentAry = NULL );

    void            SetTabs( sal_uInt32 n = 0, const RulerTab* pTabAry = NULL );
    sal_uInt32      GetTabCount() const;
    const RulerTab* GetTabs() const;

    static void     DrawTab( OutputDevice* pDevice, const Color &rFillColor,
                             const Point& rPos, sal_uInt16 nStyle );

    void            SetStyle( WinBits nStyle );
    WinBits         GetStyle() const { return mnWinStyle; }

    void            SetStartDragHdl( const Link& rLink ) { maStartDragHdl = rLink; }
    const Link&     GetStartDragHdl() const { return maStartDragHdl; }
    void            SetDragHdl( const Link& rLink ) { maDragHdl = rLink; }
    const Link&     GetDragHdl() const { return maDragHdl; }
    void            SetEndDragHdl( const Link& rLink ) { maEndDragHdl = rLink; }
    const Link&     GetEndDragHdl() const { return maEndDragHdl; }
    void            SetClickHdl( const Link& rLink ) { maClickHdl = rLink; }
    const Link&     GetClickHdl() const { return maClickHdl; }
    void            SetDoubleClickHdl( const Link& rLink ) { maDoubleClickHdl = rLink; }
    const Link&     GetDoubleClickHdl() const { return maDoubleClickHdl; }
    void            SetExtraDownHdl( const Link& rLink ) { maExtraDownHdl = rLink; }
    const Link&     GetExtraDownHdl() const { return maExtraDownHdl; }

    void            SetTextRTL(sal_Bool bRTL);

    void            SetCharWidth( long nWidth ) { mnCharWidth = nWidth ; }
    void            SetLineHeight( long nHeight ) { mnLineHeight = nHeight ; }

    void            DrawTicks();
};

#endif  // _RULER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
