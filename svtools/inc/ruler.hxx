/*************************************************************************
 *
 *  $RCSfile: ruler.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:52 $
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

#ifndef _RULER_HXX
#define _RULER_HXX

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

#ifndef _WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif

class MouseEvent;
class TrackingEvent;
class DataChangedEvent;

/*************************************************************************

Beschreibung
============

class Ruler

Diese Klasse dient zur Anzeige eines Lineals. Dabei kann diese Klasse nicht
nur als Anzeige-Control verwendet werden, sondern auch als aktives Control
zum Setzen/Verschieben von Tabulatoren und Raendern.

--------------------------------------------------------------------------

WinBits

WB_HORZ             Lineal wird horizontal dargestellt
WB_VERT             Lineal wird vertikal dargestellt
WB_3DLOOK           3D-Darstellung
WB_BORDER           Border am unteren/rechten Rand
WB_EXTRAFIELD       Feld in der linken/oberen Ecke zur Anzeige und
                    Auswahl von Tabs, Null-Punkt, ...

--------------------------------------------------------------------------

Beim Lineal werden alle Werte als Pixel-Werte eingestellt. Dadurch werden
doppelte Umrechnungen und Rundungsfehler vermieden und die Raender werden
im Lineal auch an der Position angezeigt, den Sie auch im Dokument haben.
Dadurch kann die Applikation zum Beispiel bei Tabellendarstellung auch
eigene Rundungen vornehmen und die Positionen im Lineal passen trotzdem noch
zu denen im Dokument. Damit aber das Lineal weiss, wie das Dokument auf dem
Bildschirm dargestellt wird, muessen noch ein paar zusaetzliche Werte
eingestellt werden.

Mit SetWinPos() wird der Offset des Edit-Fenster zum Lineal eingestellt.
Dabei kann auch die Breite des Fensters eingestellt werden. Wenn bei den
Werten 0 uebergeben wird, wird die Position/Breite vom Fenster automatisch
so breit gesetzt, wie das Lineal breit ist.

Mit SetPagePos() wird der Offset der Seite zum Edit-Fenster eingestellt und
die Breite der Seite eingestellt. Wenn bei den Werten 0 uebergeben wird,
wird die Position/Breite automatisch so gesetzt, als ob die Seite das ganze
Editfenster ausfuellen wuerde.

Mit SetBorderPos() kann der Offset eingestellt werden, ab dem der
Border ausgegeben wird. Die Position bezieht sich auf die linke bzw. obere
Fensterkante. Dies wird gebraucht, wenn ein horizontales und vertikales
Lineal gleichzeitig sichtbar sind. Beispiel:
        aHRuler.SetBorderPos( aVRuler.GetSizePixel().Width()-1 );

Mit SetNullOffset() wird der Null-Punkt bezogen auf die Seite gesetzt.

Alle anderen Werte (Raender, Einzug, Tabs, ...) beziehen sich auf den 0 Punkt,
der mit SetNullOffset() eingestellt wird.

Die Werte werden zum Beispiel folgendermassen berechnet:

- WinPos (wenn beide Fenster den gleichen Parent haben)

    Point aHRulerPos = aHRuler.GetPosPixel();
    Point aEditWinPos = aEditWin.GetPosPixel();
    aHRuler.SetWinPos( aEditWinPos().X() - aHRulerPos.X() );

- PagePos

    Point aPagePos = aEditWin.LogicToPixel( aEditWin.GetPagePos() );
    aHRuler.SetPagePos( aPagePos().X() );

- Alle anderen Werte

    Die logischen Werte zusammenaddieren, als Position umrechnen und
    die vorher gemerkten Pixel-Positionen (von PagePos und NullOffset)
    entsprechend abziehen.

--------------------------------------------------------------------------

Mit SetUnit() und SetZoom() wird eingestellt, in welcher Einheit das Lineal
die Werte anzeigt. Folgende Einheiten werden akzeptiert:

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

Mit SetMargin1() kann der linke/obere Rand und mit SetMargin2() kann
der rechte/untere Rand gesetzt werden. Falls diese Methoden ohne Parameter
aufgerufen werden, werden keine Raender angezeigt. Wenn SetMargin1() bzw.
SetMargin2() mit Parametern aufgerufen werden, kann bei diesen
folgendes angegeben werden:

    long    nPos            - Offset zum NullPunkt in Pixel
    USHORT  nStyle          - Bit-Style:
                                RULER_MARGIN_SIZEABLE
                                Rand kann in der Groesse veraendert werden.

                                Zu diesen Style's koennen folgende Style-
                                Bits dazugeodert werden:
                                RULER_STYLE_INVISIBLE (fuer nicht sichtbar)


Mit SetBorders() kann ein Array von Raendern gesetzt werden. Dabei muss
ein Array vom Typ RulerBorder uebergeben werden, wobei folgende Werte
initialisiert werden muessen:

    long    nPos            - Offset zum NullPunkt in Pixel
    long    nWidth          - Breite des Spaltenabstands in Pixel (kann zum
                              Beispiel fuer Tabellenspalten auch 0 sein)
    USHORT  nStyle          - Bit-Style:
                                RULER_BORDER_SIZEABLE
                                Spaltenabstand kann in der Groesse veraendert
                                werden. Dieses Flag sollte nur gesetzt werden,
                                wenn ein Abstand in der Groesse geaendert wird
                                und nicht die Groesse einer Zelle.
                                RULER_BORDER_MOVEABLE
                                Spaltenabstand/Begrenzung kann verschoben
                                werden. Wenn Tabellenbegrenzungen verschoben
                                werden, sollte dieses Flag gesetzt werden und
                                nicht Sizeable. Denn Sizeable gibt an, das
                                ein Abstand vergroessert werden kann und nicht
                                eine einzelne Zelle in der Groesse geaendert
                                werden kann.
                                RULER_BORDER_VARIABLE
                                Nicht alle Spaltenabstande sind gleich
                                RULER_BORDER_TABLE
                                Tabellenrahmen. Wenn dieser Style gesetzt
                                wird, muss die Spaltenbreite 0 sein.
                                RULER_BORDER_SNAP
                                Hilfslinie / Fanglinie. Wenn dieser Style
                                gesetzt wird, muss die Spaltenbreite 0 sein.

                                Zu diesen Style's koennen folgende Style-
                                Bits dazugeodert werden:
                                RULER_STYLE_INVISIBLE (fuer nicht sichtbar)

Mit SetIndents() kann ein Array von Indents gesetzt werden. Diese Methode darf
nur angewendet werden, wenn es sich um ein horizontales Lineal handelt. Als
Parameter muss ein Array vom Typ RulerIndent uebergeben werden, wobei folgende
Werte initialisiert werden muessen:

    long    nPos            - Offset zum NullPunkt in Pixel
    USHORT  nStyle          - Bit-Style:
                                RULER_INDENT_TOP    (Erstzeileneinzug)
                                RULER_INDENT_BOTTOM (Linker/Rechter Einzug)
                                Zu diesen Style's koennen folgende Style-
                                Bits dazugeodert werden:
                                RULER_STYLE_DONTKNOW (fuer alte Position oder
                                                     fuer Uneindeutigkeit)
                                RULER_STYLE_INVISIBLE (fuer nicht sichtbar)

Mit SetTabs() kann ein Array von Tabs gesetzt werden. Diese Methode darf nur
angewendet werden, wenn es sich um ein horizontales Lineal handelt. Als
Parameter muss ein Array vom Typ RulerTab uebergeben werden, wobei folgende
Werte initialisiert werden muessen:

    long    nPos            - Offset zum NullPunkt in Pixel
    USHORT  nStyle          - Bit-Style:
                                RULER_TAB_DEFAULT (kann nicht selektiert werden)
                                RULER_TAB_LEFT
                                RULER_TAB_CENTER
                                RULER_TAB_RIGHT
                                RULER_TAB_DECIMAL
                                Zu diesen Style's koennen folgende Style-
                                Bits dazugeodert werden:
                                RULER_STYLE_DONTKNOW (fuer alte Position oder
                                                     fuer Uneindeutigkeit)
                                RULER_STYLE_INVISIBLE (fuer nicht sichtbar)

Mit SetLines() koennen Positionslinien im Lineal angezeigt werden. Dabei
muss ein Array vom Typ RulerLine uebergeben werden, wobei folgende Werte
initialisiert werden muessen:

    long    nPos            - Offset zum NullPunkt in Pixel
    USHORT  nStyle          - Bit-Style (muss zur Zeit immer 0 sein)

Mit SetArrows() koennen Bemassungspfeile im Lineal angezeigt werden. Wenn
Bemassungspfeile gesetzt werden, werden im Lineal auch keine Unterteilungen
mehr angezeigt. Deshalb sollten die Bemassungspfeile immer ueber die ganze
Linealbreite gesetzt werden. Dabei muss ein Array vom Typ RulerArrow
uebergeben werden, wobei folgende Werte initialisiert werden muessen:

    long    nPos            - Offset zum NullPunkt in Pixel
    long    nWidth          - Breite des Pfeils
    long    nLogWidth       - Breite des Pfeils in logischer Einheit
    USHORT  nStyle          - Bit-Style (muss zur Zeit immer 0 sein)

Mit SetSourceUnit() wird die Einheit eingestellt, in welcher die logischen
Werte vorliegen, die bei SetArrows() uebergeben werden. Dabei werden nur die
Einheiten MAP_TWIP und MAP_100TH_MM (default) akzeptiert.

--------------------------------------------------------------------------

Wenn auch vom Benutzer die Raender, Tabs, Border, ... ueber das Lineal
geaendert werden koennen, muss etwas mehr Aufwand getrieben werden. Dazu
muessen die Methoden StartDrag(), Drag() und EndDrag() ueberlagert werden.
Bei der Methode StartDrag() besteht die Moeglichkeit durch das zurueckgeben
von FALSE das Draggen zu verhindern. Im Drag-Handler muss die Drag-Position
abgefragt werden und die Werte muessen an die neue Position verschoben werden.
Dazu ruft man einfach die einzelnen Set-Methoden auf. Solange man sich
im Drag-Handler befindet, werden sich die Werte nur gemerkt und erst
danach das Lineal neu ausgegeben. Alle Handler koennen auch als Links ueber
entsprechende Set..Hdl()-Methoden gesetzt werden.

    - StartDrag()
        Wird gerufen, wenn das Draggen gestartet wird. Wenn FALSE
        zurueckgegeben wird, wird das Draggen nicht ausgefuehrt. Bei TRUE
        wird das Draggen zugelassen. Wenn der Handler nicht ueberlagert
        wird, wird FALSE zurueckgegeben.

    - EndDrag()
        Wird gerufen, wenn das Draggen beendet wird.

    - Drag()
        Wird gerufen, wenn gedragt wird.

    - Click()
        Dieser Handler wird gerufen, wenn kein Element angeklickt wurde.
        Die Position kann mit GetClickPos() abgefragt werden. Dadurch
        kann man zum Beispiel Tabs in das Lineal setzen. Nach Aufruf des
        Click-Handlers wird gegebenenfalls das Drag sofort ausgeloest. Dadurch
        ist es moeglich, einen neuen Tab im Click-Handler zu setzen und
        danach gleich zu verschieben.

    - DoubleClick()
        Dieser Handler wird gerufen, wenn ein DoubleClick ausserhalb des
        Extrafeldes gemacht wurde. Was angeklickt wurde, kann mit
        GetClickType(), GetClickAryPos() und GetClickPos() abgefragt werden.
        Somit kann man zum Beispiel den Tab-Dialog anzeigen, wenn ein
        Tab mit einem DoubleClick betaetigt wurde.

Im Drag-Handler kann man abfragen, was und wohin gedragt wurde. Dazu gibt
es folgende Abfrage-Methoden.

    - GetDragType()
        Liefert zurueck, was gedragt wird:
            RULER_TYPE_MARGIN1
            RULER_TYPE_MARGIN2
            RULER_TYPE_BORDER
            RULER_TYPE_INDENT
            RULER_TYPE_TAB

    - GetDragPos()
        Liefert die Pixel-Position bezogen auf den eingestellten Null-Offset
        zurueck, wohin der Anwender die Maus bewegt hat.

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
        zurueckgeschoben hat, liefert die Methode trotzdem FALSE. Falls
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
        einen Pointer auf einen USHORT, um die Array-Position eines
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
        Liefert TRUE zurueck, wenn sich das Lineal im Drag-Vorgang befindet.

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

    - GetDocType()
        Dieser Methode wird die Position vom Dokumentfenster uebergeben und
        testet, was sich unter der Position befindet. Dabei kann wie bei
        StartDocDrag() der entsprechende Test auf ein bestimmtes Element
        eingeschraenkt werden. Im Gegensatz zu GetType() liefert diese
        Methode immer DontKnow zurueck, falls kein Element getroffen wurde.
        Falls man den HitTest selber durchfuehren moechte, kann man
        folgende Defines fuer die Toleranz benutzen (Werte gelten fuer
        eine Richtung):
            RULER_MOUSE_TABLEWIDTH      - fuer Tabellenspalten
            RULER_MOUSE_MARGINWIDTH     - fuer Margins

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
            - USHORT nStyle     Bitfeld als Style:
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

// -----------
// - WinBits -
// -----------

#define WB_EXTRAFIELD       ((WinBits)0x00004000)
#define WB_STDRULER         WB_HORZ

// ---------------
// - Ruler-Types -
// ---------------

struct ImplRulerHitTest;

// --------------
// - Ruler-Type -
// --------------

enum RulerType { RULER_TYPE_DONTKNOW, RULER_TYPE_OUTSIDE,
                 RULER_TYPE_MARGIN1, RULER_TYPE_MARGIN2,
                 RULER_TYPE_BORDER, RULER_TYPE_INDENT, RULER_TYPE_TAB };

enum RulerExtra { RULER_EXTRA_DONTKNOW,
                  RULER_EXTRA_NULLOFFSET, RULER_EXTRA_TAB };

#define RULER_STYLE_HIGHLIGHT   ((USHORT)0x8000)
#define RULER_STYLE_DONTKNOW    ((USHORT)0x4000)
#define RULER_STYLE_INVISIBLE   ((USHORT)0x2000)

#define RULER_DRAGSIZE_MOVE     0
#define RULER_DRAGSIZE_1        1
#define RULER_DRAGSIZE_2        2

#define RULER_MOUSE_BORDERMOVE  5
#define RULER_MOUSE_BORDERWIDTH 5
#define RULER_MOUSE_TABLEWIDTH  1
#define RULER_MOUSE_MARGINWIDTH 3

#define RULER_SCROLL_NO         0
#define RULER_SCROLL_1          1
#define RULER_SCROLL_2          2

// ---------------
// - RulerMargin -
// ---------------

#define RULER_MARGIN_SIZEABLE   ((USHORT)0x0001)

// ---------------
// - RulerBorder -
// ---------------

#define RULER_BORDER_SIZEABLE   ((USHORT)0x0001)
#define RULER_BORDER_MOVEABLE   ((USHORT)0x0002)
#define RULER_BORDER_VARIABLE   ((USHORT)0x0004)
#define RULER_BORDER_TABLE      ((USHORT)0x0008)
#define RULER_BORDER_SNAP       ((USHORT)0x0010)

struct RulerBorder
{
    long    nPos;
    long    nWidth;
    USHORT  nStyle;
};

// ---------------
// - RulerIndent -
// ---------------

#define RULER_INDENT_TOP        ((USHORT)0x0000)
#define RULER_INDENT_BOTTOM     ((USHORT)0x0001)
#define RULER_INDENT_STYLE      ((USHORT)0x000F)

struct RulerIndent
{
    long    nPos;
    USHORT  nStyle;
};

// ------------
// - RulerTab -
// ------------

#define RULER_TAB_LEFT          ((USHORT)0x0000)
#define RULER_TAB_RIGHT         ((USHORT)0x0001)
#define RULER_TAB_DECIMAL       ((USHORT)0x0002)
#define RULER_TAB_CENTER        ((USHORT)0x0003)
#define RULER_TAB_DEFAULT       ((USHORT)0x0004)
#define RULER_TAB_STYLE         ((USHORT)0x000F)

struct RulerTab
{
    long    nPos;
    USHORT  nStyle;
};

#define RULER_TAB_WIDTH         7
#define RULER_TAB_HEIGHT        6

// -------------
// - RulerLine -
// -------------

struct RulerLine
{
    long    nPos;
    USHORT  nStyle;
};

// --------------
// - RulerArrow -
// --------------

struct RulerArrow
{
    long    nPos;
    long    nWidth;
    long    nLogWidth;
    USHORT  nStyle;
};

// -----------------
// - ImplRulerData -
// -----------------

class ImplRulerData
{
    friend              class Ruler;

private:
    RulerLine*          pLines;
    RulerArrow*         pArrows;
    RulerBorder*        pBorders;
    RulerIndent*        pIndents;
    RulerTab*           pTabs;
    long                nNullVirOff;
    long                nRulVirOff;
    long                nRulWidth;
    long                nPageOff;
    long                nPageWidth;
    long                nNullOff;
    long                nMargin1;
    long                nMargin2;
    USHORT              nLines;
    USHORT              nArrows;
    USHORT              nBorders;
    USHORT              nIndents;
    USHORT              nTabs;
    USHORT              nMargin1Style;
    USHORT              nMargin2Style;
    BOOL                bAutoPageWidth;
    BOOL                bDummyAlign;

#ifdef _SV_RULER_CXX
public:
                        ImplRulerData();
                        ~ImplRulerData();
    ImplRulerData&      operator=( const ImplRulerData& rData );
#endif
};

// ---------
// - Ruler -
// ---------

class Ruler : public Window
{
private:
    VirtualDevice       maVirDev;
    MapMode             maMapMode;
    long                mnBorderOff;
    long                mnWinOff;
    long                mnWinWidth;
    long                mnWidth;
    long                mnHeight;
    long                mnVirOff;
    long                mnVirWidth;
    long                mnVirHeight;
    long                mnBorderWidth;
    long                mnStartDragPos;
    long                mnDragPos;
    ULONG               mnUpdateEvtId;
    ImplRulerData*      mpData;
    ImplRulerData       maData;
    ImplRulerData       maDragData;
    Rectangle           maExtraRect;
    WinBits             mnWinStyle;
    USHORT              mnUnitIndex;
    USHORT              mnDragAryPos;
    USHORT              mnDragSize;
    USHORT              mnDragScroll;
    USHORT              mnDragModifier;
    USHORT              mnExtraStyle;
    USHORT              mnExtraClicks;
    USHORT              mnExtraModifier;
    RulerExtra          meExtraType;
    RulerType           meDragType;
    MapUnit             meSourceUnit;
    FieldUnit           meUnit;
    Fraction            maZoom;
    BOOL                mbCalc;
    BOOL                mbFormat;
    BOOL                mbDrag;
    BOOL                mbDragDelete;
    BOOL                mbDragCanceled;
    BOOL                mbAutoWinWidth;
    BOOL                mbActive;
    BYTE                mnUpdateFlags;
    Link                maStartDragHdl;
    Link                maDragHdl;
    Link                maEndDragHdl;
    Link                maClickHdl;
    Link                maDoubleClickHdl;
    Link                maExtraDownHdl;

#ifdef _SV_RULER_CXX
    void                ImplVDrawLine( long nX1, long nY1, long nX2, long nY2 );
    void                ImplVDrawRect( long nX1, long nY1, long nX2, long nY2 );
    void                ImplVDrawText( long nX, long nY, const String& rText );

    void                ImplDrawTicks( long nMin, long nMax, long nStart, long nCenter );
    void                ImplDrawArrows( long nCenter );
    void                ImplDrawBorders( long nMin, long nMax, long nVirTop, long nVirBottom );
    void                ImplDrawIndent( const Polygon& rPoly, USHORT nStyle );
    void                ImplDrawIndents( long nMin, long nMax, long nVirTop, long nVirBottom );
    void                ImplDrawTab( OutputDevice* pDevice, const Point& rPos, USHORT nStyle );
    void                ImplDrawTabs( long nMin, long nMax, long nVirBottom );
    void                ImplInit( WinBits nWinBits );
    void                ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground );
    void                ImplCalc();
    void                ImplFormat();
    void                ImplInitExtraField( BOOL bUpdate );
    void                ImplInvertLines( BOOL bErase = FALSE );
    void                ImplDraw();
    void                ImplDrawExtra( BOOL bPaint = FALSE );
    void                ImplUpdate( BOOL bMustCalc = FALSE );
    BOOL                ImplHitTest( const Point& rPos, ImplRulerHitTest* pHitTest ) const;
    BOOL                ImplDocHitTest( const Point& rPos, RulerType eDragType, ImplRulerHitTest* pHitTest ) const;
    BOOL                ImplStartDrag( ImplRulerHitTest* pHitTest, USHORT nModifier );
    void                ImplDrag( const Point& rPos );
    void                ImplEndDrag();
                        DECL_LINK( ImplUpdateHdl, void* );
#endif

public:
                        Ruler( Window* pParent, WinBits nWinStyle = WB_STDRULER );
    virtual             ~Ruler();

    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        MouseMove( const MouseEvent& rMEvt );
    virtual void        Tracking( const TrackingEvent& rTEvt );
    virtual void        Paint( const Rectangle& rRect );
    virtual void        Resize();
    virtual void        StateChanged( StateChangedType nStateChange );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

    virtual long        StartDrag();
    virtual void        Drag();
    virtual void        EndDrag();
    virtual void        Click();
    virtual void        DoubleClick();
    virtual void        ExtraDown();

    void                Activate();
    void                Deactivate();
    BOOL                IsActive() const { return mbActive; }

    void                SetWinPos( long nOff = 0, long nWidth = 0 );
    long                GetWinOffset() const { return mnWinOff; }
    long                GetWinWidth() const { return mnWinWidth; }
    void                SetPagePos( long nOff = 0, long nWidth = 0 );
    long                GetPageOffset() const { return mpData->nPageOff; }
    long                GetPageWidth() const { return mpData->nPageWidth; }
    void                SetBorderPos( long nOff = 0 );
    long                GetBorderOffset() const { return mnBorderOff; }
    Rectangle           GetExtraRect() const { return maExtraRect; }

    void                SetUnit( FieldUnit eNewUnit );
    FieldUnit           GetUnit() const { return meUnit; }
    void                SetZoom( const Fraction& rNewZoom );
    Fraction            GetZoom() const { return maZoom; }

    void                SetSourceUnit( MapUnit eNewUnit ) { meSourceUnit = eNewUnit; }
    MapUnit             GetSourceUnit() const { return meSourceUnit; }

    void                SetExtraType( RulerExtra eNewExtraType, USHORT nStyle = 0 );
    RulerExtra          GetExtraType() const { return meExtraType; }
    USHORT              GetExtraStyle()  const { return mnExtraStyle; }
    USHORT              GetExtraClicks() const { return mnExtraClicks; }
    USHORT              GetExtraModifier() const { return mnExtraModifier; }

    BOOL                StartDocDrag( const MouseEvent& rMEvt,
                                      RulerType eDragType = RULER_TYPE_DONTKNOW );
    RulerType           GetDocType( const Point& rPos,
                                    RulerType eDragType = RULER_TYPE_DONTKNOW,
                                    USHORT* pAryPos = NULL ) const;
    RulerType           GetDragType() const { return meDragType; }
    long                GetDragPos() const { return mnDragPos; }
    USHORT              GetDragAryPos() const { return mnDragAryPos; }
    USHORT              GetDragSize() const { return mnDragSize; }
    BOOL                IsDragDelete() const { return mbDragDelete; }
    BOOL                IsDragCanceled() const { return mbDragCanceled; }
    USHORT              GetDragScroll() const { return mnDragScroll; }
    USHORT              GetDragModifier() const { return mnDragModifier; }
    BOOL                IsDrag() const { return mbDrag; }
    void                CancelDrag();
    long                GetClickPos() const { return mnDragPos; }
    RulerType           GetClickType() const { return meDragType; }
    USHORT              GetClickAryPos() const { return mnDragAryPos; }
    RulerType           GetType( const Point& rPos,
                                 USHORT* pAryPos = NULL ) const;

    void                SetNullOffset( long nPos );
    long                GetNullOffset() const { return mpData->nNullOff; }
    void                SetMargin1() { SetMargin1( 0, RULER_STYLE_INVISIBLE ); }
    void                SetMargin1( long nPos, USHORT nMarginStyle = RULER_MARGIN_SIZEABLE );
    long                GetMargin1() const { return mpData->nMargin1; }
    USHORT              GetMargin1Style() const { return mpData->nMargin1Style; }
    void                SetMargin2() { SetMargin2( 0, RULER_STYLE_INVISIBLE ); }
    void                SetMargin2( long nPos, USHORT nMarginStyle = RULER_MARGIN_SIZEABLE );
    long                GetMargin2() const { return mpData->nMargin2; }
    USHORT              GetMargin2Style() const { return mpData->nMargin2Style; }

    void                SetLines( USHORT n = 0, const RulerLine* pLineAry = NULL );
    USHORT              GetLineCount() const { return mpData->nLines; }
    const RulerLine*    GetLines() const { return mpData->pLines; }

    void                SetArrows( USHORT n = 0, const RulerArrow* pArrowAry = NULL );
    USHORT              GetArrowCount() const { return mpData->nArrows; }
    const RulerArrow*   GetArrows() const { return mpData->pArrows; }

    void                SetBorders( USHORT n = 0, const RulerBorder* pBrdAry = NULL );
    USHORT              GetBorderCount() const { return mpData->nBorders; }
    const RulerBorder*  GetBorders() const { return mpData->pBorders; }

    void                SetIndents( USHORT n = 0, const RulerIndent* pIndentAry = NULL );
    USHORT              GetIndentCount() const { return mpData->nIndents; }
    const RulerIndent*  GetIndents() const { return mpData->pIndents; }

    void                SetTabs( USHORT n = 0, const RulerTab* pTabAry = NULL );
    USHORT              GetTabCount() const { return mpData->nTabs; }
    const RulerTab*     GetTabs() const { return mpData->pTabs; }

    static void         DrawTab( OutputDevice* pDevice,
                                 const Point& rPos, USHORT nStyle );

    void                SetStyle( WinBits nStyle );
    WinBits             GetStyle() const { return mnWinStyle; }

    void                SetStartDragHdl( const Link& rLink ) { maStartDragHdl = rLink; }
    const Link&         GetStartDragHdl() const { return maStartDragHdl; }
    void                SetDragHdl( const Link& rLink ) { maDragHdl = rLink; }
    const Link&         GetDragHdl() const { return maDragHdl; }
    void                SetEndDragHdl( const Link& rLink ) { maEndDragHdl = rLink; }
    const Link&         GetEndDragHdl() const { return maEndDragHdl; }
    void                SetClickHdl( const Link& rLink ) { maClickHdl = rLink; }
    const Link&         GetClickHdl() const { return maClickHdl; }
    void                SetDoubleClickHdl( const Link& rLink ) { maDoubleClickHdl = rLink; }
    const Link&         GetDoubleClickHdl() const { return maDoubleClickHdl; }
    void                SetExtraDownHdl( const Link& rLink ) { maExtraDownHdl = rLink; }
    const Link&         GetExtraDownHdl() const { return maExtraDownHdl; }
};

#endif  // _RULER_HXX
