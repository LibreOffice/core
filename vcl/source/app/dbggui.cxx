/*************************************************************************
 *
 *  $RCSfile: dbggui.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cd $ $Date: 2000-10-23 06:08:19 $
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

#pragma hdrstop

#ifdef DBG_UTIL

#define _SV_DBGGUI_CXX

#include "svdata.hxx"
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <svsys.h>

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <lstbox.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <button.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <edit.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <fixed.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <group.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <field.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <msgbox.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <wrkwin.hxx>
#endif
#ifndef _SV_SYSTEM_HXX
#include <system.hxx>
#endif

#ifndef _SV_DBGGUI_HXX
#include <dbggui.hxx>
#endif

// =======================================================================

static sal_Char* pDbgHelpText[] =
{
"Object Test\n",
"------------------------------------------\n",
"\n",
"--- Macros ---\n",
"DBG_NAME( aName )\n",
"Definiert die Verwaltungsdaten fuer eine Klasse. Dieses Makro darf nur in "
"einem Source-File mit dem gleichen Namen benutzt werden.\n",
"\n",
"DBG_NAMEEX( aName )\n",
"Wie DBG_NAME, nur fuer weitere Source-Files.\n",
"\n",
"DBG_CTOR( aName, fTest )\n",
"Muss in allen Konstruktoren einer Klasse benutzt werden (auch beim "
"CopyCtor). Als erster Paramter muss der registrierte Name (am besten der "
"Klassenname) uebergeben werden und als zweiter Parameter die Testfunktion "
"oder 0.\n",
"\n",
"DBG_DTOR( aName, fTest )\n",
"Muss im Destruktor der Klasse benutzt werden. Als erster Paramter muss "
"der registrierte Name uebergeben werden und als zweiter Parameter die "
"Testfunktion oder 0.\n",
"\n",
"DBG_CHKTHIS( aName, fTest )\n",
"Kann in Methoden einer Klasse benutzt werden, wo die Konstruktoren und "
"der Destruktor der Klasse mit den entsprechenden Makros ausgestattet sind. "
"Als erster Paramter muss der registrierte Name uebergeben werden und als "
"zweiter Parameter die Testfunktion oder 0.\n",
"\n",
"DBG_CHKOBJ( pObj, aName, fTest )\n",
"Kann auf Instanzen einer Klasse angewendet werden, wo die Konstruktoren und "
"der Destruktor der Klasse mit den entsprechenden Makros ausgestattet sind. "
"Als erster Paramter muss die Adresse des zu testenden Objects uebergeben "
"werden, als zweiter Parameter der registrierte Name und als dritter "
"Parameter die Testfunktion oder 0.\n",
"\n",
"Damit die Makros Wirkung haben, muss DBG_UTIL defniert sein.\n",
"\n",
#ifndef CFRONT
"--- Optionen ---\n",
"This\n",
"Es wird auf gueltigen This-Pointer getestet. Dadurch kann man erreichen, "
"das bei allen Objekten die damit ausgestattet sind, geprueft wird, ob "
"mit einem existierenden Objekt gearbeitet wird. Dadurch findet man zum "
"Beispiel schneller Fehler durch falsche Mehrfachvererbung, Alignment oder "
"Compilerfehler. Da fast alle Standard-Klassen von SV (String, List, Pen, "
"Brush, Polygon, ...) mit DBG_CHKTHIS() ausgestattet sind, werden viele "
"Fehler gefunden, jedoch kostet dadurch dieser Test auch entsprechend viel "
"Performence.\n",
"\n",
"Function\n",
"Wenn eine Funktion bei den Macros uebergeben wird, wird sie gerufen.\n",
"\n",
"Exit\n",
"This- und Func-Test wird auch beim Funktionsaustritt durchgefuehrt.\n",
"\n",
"Report\n",
"Am Programmende wird die Anzahl der angelegten Objekte ausgegeben. "
"Da alle wichtigen SV-Klassen zumindest mit DBG_CTOR()/DBG_DTOR() "
"ausgestattet sind, kann man damit feststellen, ob man die sogenannten "
"Resource-Leaks hat (Systemobjekte, die nicht freigegeben werden). Dazu "
"gehoehren OutputDevice, Window, VirtualDevice, Printer und Menu. Achtung: "
"Dtor-Aufrufe von statischen Objekten werden nicht beruecksichtigt. Deshalb "
"bleiben bei jedem SV-Programm auch 2 Strings und eine Bitmap nach.\n",
"\n",
"Trace\n",
"Erzeugung, Zerstoerung und Benutzung der mit DBG_XTOR ausgestatteten "
"Objekte wird protokoliert.\n",
"\n",
"\n",
"Memory Test\n",
"------------------------------------------\n",
"\n",
"--- Macros ---\n",
"DBG_MEMTEST()\n",
"Fuehrt die eingestellten Memory Tests durch.\n",
"\n",
"DBG_MEMTEST_PTR( p )\n",
"Fuehrt die eingestellten Memory Tests durch und zusaetzlich wird der "
"uebergebene Pointer auf Gueltigkeit geprueft, wenn Pointer Test an ist.\n",
"\n",
"--- Optionen ---\n",
"Initilize\n",
"Allokierter Speicher wird mit 0x77 und freier oder freigegebener Speicher "
"wird mit 0x33 initialisiert. Diese Option kostet (fast) keine Performence "
"und sollte deshalb waehrend der Entwicklung fast immer an sein. Denn "
"dadurch erreicht man auch, das Abstuerze oefters reproduzierbarer "
"auftreten.\n",
"\n",
"Overwrite\n",
"Es wird getestet, ob vor/hinter die Bloecke geschrieben wird. Vor und "
"hinter dem Block wird der Speicher mit 0x55 initialisiert. Diese Option "
"kostet Performence, sollte jedoch auch mal eingesetzt werden um die "
"haeufigsten Speicherueberschreiber (+-1-Fehler) zu testen. Diese Option "
"sollte auch eingeschaltet werden, wenn das Programm im new oder "
"delete-Operator abstuerzt.\n",
"\n",
"Free\n",
"Es wird getestet, ob freier Speicher ueberschrieben wird. Diese Option "
"kostet eine ganze Menge Performence und sollte deshalb nur gelegentlich "
"eingesetzt werden um Speicherueberschreiber zu testen. Diese Option "
"sollte evt. auch eingeschaltet werden, wenn das Programm im new oder "
"delete-Operator abstuerzt.\n",
"\n",
"Pointer\n",
"Bei delete und DBG_MEMTEST_PTR() wird der Zeiger getestet, ob er auch mit "
"new oder SvMemAlloc() angelegt wurde. Wenn diese Option eingeschaltet ist, "
"werden Fehler wie doppeltes delete und delete auf Stack-Objekte oder "
"ungueltige Zeiger gefunden. Diese Option kostet Performence und sollte "
"deshalb nicht immer eingeschaltet sein. Jedoch sollte auch ab und zu mit "
"dieser Option getestet werden, da der Memory-Manager nicht immer bei delete "
"und ungueltigem Zeiger abstuerzt. Diese Option sollte auch eingeschaltet "
"werden, wenn das Programm im new oder delete-Operator abstuerzt.\n",
"\n",
"Report\n",
"Am Programmende wird eine kleine Statistik und der nicht freigegebene "
"Speicher ausgegeben. Achtung: Speicher der von globalen Objekten noch "
"freigegeben wird, taucht auch in der Leak-Liste auf.\n",
"\n",
"Trace\n",
"Allokation und Freigeben von Speicher wird protokoliert.\n",
"\n",
"Leak-Report\n",
"Gibt unter WNT beim Programmende eine Liste der Memory-Leaks mit "
"Stack-Trace aus. Dabei werden nur Bloecke beruecksichtigt, die innerhalb "
"von Application::Execute() angelegt und freigegeben werden. Wenn diese "
"Option und Overwrite gesetzt ist, wird bei einem Speicherueberschreiber "
"auch noch versucht den Stack auszugeben, wo der Block angelegt wurde. "
"Diese Ausgabe erfolgt erst nach Ausgabe der Fehlermeldung in die "
"Log-Datei.\n"
"\n",
"New/Delete\n",
"Memory-Tests werden auf den gesammten Speicher bei jedem new/delete "
"durchgefuhert. Achtung: Diese Option macht die Programme sehr langsam "
"und sollte nur eingeschaltet werden, wenn ein Speicherueberschreiber "
"eingegrenzt werden soll. Ansonsten reicht es, die einzelnen Optionen "
"einzuschalten, da (kein Leak vorrausgesetzt) jeder zu erkennende "
"Speicherueberschreiber waehrend der Laufzeit eines Programms gefunden "
"werden sollte.\n",
"\n",
"Object Test\n",
"Memory-Tests werden auf den gesammten Speicher bei jedem Object-Test "
"durchgefuhert. Achtung: Diese Option macht die Programme sehr langsam "
"und sollte nur eingeschaltet werden, wenn ein Speicherueberschreiber "
"eingegrenzt werden soll. Ansonsten reicht es, die einzelnen Optionen "
"einzuschalten, da (kein Leak vorrausgesetzt) jeder zu erkennende "
"Speicherueberschreiber waehrend der Laufzeit eines Programms gefunden "
"werden sollte.\n",
"\n",
"SysAlloc\n",
"Wenn dieses Flag gesetzt ist, wird Speicher mit direkten Systemfunktionen"
"angefordert und der Memory-Manager von SV umgangen. Dadurch ist es moeglich"
"leistungsfaehigere Memory-Test-Tools einzusetzen. Es ist jedoch darauf zu"
"achten, das nicht auf jedem System die Memory-Funktionen vom Compiler"
"gerufen werden, sondern die Systemfunktionen. Dadurch ist man unter"
"Windows 16-Bit auf ca. 4000 news begrenzt und unter OS2 werden immer"
"4096 KB Bloecke angefordert.\n",
"\n",
"Windows 16-Bit und Debug-Tests\n",
"Achtung: Wenn Memory-Tests an sind (ausser Initilize) wird niemals "
"(auch nicht bei >= 64 KB) Speicher mit Offset 0 zurueckgeben. Falls man "
"darauf angewiesen ist, muessen die Tests mit 32-Bit-Versionen der "
"Programme durchgefuehrt werden. Teilweise reicht es aber auch schon aus, "
"wenn man statt 64 KB nur 64 KB - 64-Bytes anlegt, da es dann nicht zu "
"einem Segmentueberlauf kommt.\n",
"Ausserdem sollten die Memory- genauso wie die Object-Tests nur dann "
"eingesetzt werden, wenn nur eine SV-Applikation gleichzeitig laeuft. "
"Ansonsten kann es zu unkontrolierten Fehlern kommen. Hier hilft dann auch "
"nur ein ausweichen auf 32-Bit-Programme."
"\n",
"\n",
"\nWeitere Test's und Makros\n",
"------------------------------------------\n",
"\n",
"Profiling\n",
"DBG_PROFSTART() / DBG_PROFSTOP() / DBG_PROFCONTINUE() / DBG_PROFPAUSE() "
"werden ausgewertet und beim Programmende wird die Anzahl der Durchlaeufe "
"und die dazu benoetigte Zeit (inklusive der Childaufrufe) in "
"Millisekunden ausgegeben. Diese Macros koennen dann eingesetzt werden, wenn "
"die gleichen Funktionsablaeufe ueber die gesammte Entwicklungszeit "
"beobachtet werden sollen, wie zum Beispiel die Startup-Zeiten. Bei den "
"Makros muss der registrierte Name uebergeben werden, der mit DBG_NAME() "
"registriert wurde.\n",
"\n",
"Resourcen\n",
"Bei Resource-Fehlern gibt es einen Fehler-Dialog, bevor der "
"Exception-Handler gerufen wird.\n",
"\n",
"Dialog\n",
"Es werden FixedTexte, CheckBoxen, TriStateBoxen und RadioButtons mit "
"einer anderen Hintergrundfarbe versehen, damit man feststellen kann, wie "
"Gross die Controls sind. Ausserdem wird getestet, ob sich Controls "
"ueberschneiden, die Tabreihenfolge in Ordnung ist und die Mnemonischen "
"Zeichen ordentlich vergeben wurden. Bei Dialogen wird auch angemahnt, "
"wenn kein DefPushButton oder kein OK-/CancelButton vorhanden ist. "
"Diese Tests sind nicht 100% (es wird evt. zuviel angemahnt) und "
"erfuellen auch keine Garantie, das alle Problemfaelle festgestellt "
"werden, da zum Beispiel nur initial und dann nur die sichtbaren Controls "
"getestet werden. Es werden somit keine Fehler gefunden, die waehrend der "
"Benutzung des Dialoges auftreten.\n",
"\n",
"Bold AppFont\n",
"Es wird der Applikationsfont auf Fett gesetzt, damit man feststellen kann, "
"ob der Platz fuer die Texte auf anderen Systemen oder bei anderer "
"Systemeinstellung ausreicht. Denn bei schmalen Fonts werden die Dialoge "
"kuenstlich breiter gemacht, da diese ansonsten zu schmal aussehen.\n",
"\n",
"Trace-Ausgaben\n",
"DBG_TRACE() kann verwendet werden, wenn man TRACE-Ausgaben haben moechte. "
"DBG_TRACEFILE() gibt zu der Meldung auch noch die Datei und die Zeilennummer "
"aus, an der das Makro steht. DBG_TRACE1() bis DBG_TRACE5() koennen "
"verwendet werden, wenn man eine formatierte Ausgabe (printf-Formatstring) "
"haben moechte. Die Trace-Ausgaben werden aktiviert, in dem man in der "
"DropDown-ListBox eine entsprechende Ausgabe waehlt.\n"
"\n",
"Warnings\n",
"DBG_WARNING() kann verwendet werden, wenn man Warnungen ausgeben moechte. "
"DBG_WARNINGFILE() gibt zu der Warnung auch noch die Datei und die "
"Zeilennummer aus, an der das Makro steht. DBG_WARNING1() bis DBG_WARNING5() "
"koennen verwendet werden, wenn man eine formatierte Ausgabe "
"(printf-Formatstring) haben moechte. Wenn man die Warnung von einer "
"Bedingung abhaengig machen moechte, kann man DBG_ASSERTWARNING() "
"benutzen. Die Warning wird ausgegeben, wenn die Bedingung nicht erfuellt "
"wurde. Als erster Parameter muss die zu testende Bedingung und als zweiter "
"Parameter die auszugebene Meldung uebergeben werden. Die Warnungen werden "
"aktiviert, in dem man in der DropDown-ListBox eine entsprechende Ausgabe "
"waehlt. Wenn None gewaehlt ist, wird auch die Bedingung bei "
"DBG_ASSERTWARNING() nicht ausgewertet.\n",
"\n",
"Errors\n",
"DBG_ERROR() kann verwendet werden, wenn man Fehlermeldungen ausgeben "
"moechte. DBG_ERRORFILE() gibt zu dem Fehler auch noch die Datei und die "
"Zeilennummer aus, an der das Makro steht. DBG_ERROR1() bis DBG_ERROR5() "
"koennen verwendet werden, wenn man eine formatierte Ausgabe "
"(printf-Formatstring) haben moechte. Wenn man die Fehlerausgabe von einer "
"Bedingung abhaengig machen moechte, kann man DBG_ASSERT() benutzen. Der "
"Fehler wird ausgegeben, wenn die Bedingung nicht erfuellt wurde. Als erster "
"Parameter muss die zu testende Bedingung und als zweiter Parameter die "
"auszugebene Meldung uebergeben werden. Die Fehlermeldungen werden "
"aktiviert, in dem man in der DropDown-ListBox eine entsprechende Ausgabe "
"waehlt. Wenn None gewaehlt ist, wird auch die Bedingung bei "
"DBG_ASSERT() nicht ausgewertet.\n",
"\n",
"\n",
"Output\n",
"------------------------------------------\n",
"\n",
"Overwrite - CheckBox\n",
"Bei jedem neuen Programmstart wird das Log-File ueberschrieben, wenn "
"eine Ausgabe stattgefunden hat.\n",
"\n",
"Include-ObjectTest-Filter\n",
"Es werden nur die Klassen bei Object-Test ausgewertet, die einen der "
"angegebenen Filter enthalten. Die Filter werden mit ';' getrennt und "
"sind casesensitiv. Wildcards werden nicht unterstuetzt. Wenn kein Text "
"angegeben wird, ist der Filter nicht aktiv.\n",
"\n",
"Exclude-ObjectTest-Filter\n",
"Es werden die Klassen bei Object-Test ausgewertet, die einen der "
"angegebenen Filter nicht enthalten. Die Filter werden mit ';' getrennt "
"und sind casesensitiv. Wildcards werden nicht unterstuetzt. Wenn kein "
"Text angegeben wird, ist der Filter nicht aktiv.\n",
"\n",
"Include-Filter\n",
"Es werden nur die Texte ausgegeben, die einen der angegebenen Filter "
"enthalten. Die Filter werden mit ';' getrennt und sind casesensitiv. "
"Wildcards werden nicht unterstuetzt. Der Filter gilt fuer alle Ausgaben "
"(jedoch nicht fuer Errors). Wenn kein Text angegeben wird, ist der Filter "
"nicht aktiv.\n",
"\n",
"Exclude-Filter\n",
"Es werden nur die Texte ausgegeben, die einen der angegebenen Filter "
"nicht enthalten. Die Filter werden mit ';' getrennt und sind casesensitiv. "
"Wildcards werden nicht unterstuetzt. Der Filter gilt fuer alle Ausgaben "
"(jedoch nicht fuer Errors). Wenn kein Text angegeben wird, ist der Filter "
"nicht aktiv.\n",
"\n",
"Ausserdem kann eingestellt werden, wohin die Daten ausgegeben werden "
"sollen:\n",
"\n",
"None\n",
"Ausgabe wird unterdrueckt.\n",
"\n",
"File\n",
"Ausgabe ins Debug-File. Dateiname kann im Editfeld eingegeben werden.\n",
"\n",
"Window\n",
"Ausgabe in ein kleines Debug-Window. Die Fenstergroesse wird gespeichert, "
"wenn man den Debug-Dialog mit OK beendet und das Fenster sichtbar ist.\n",
"\n",
"Shell\n",
"Ausgabe in ein Debug-System (Windows Debug-Window) wenn vorhanden oder "
"unter Unix ins Shell-Fenster. Ansonsten das gleiche wie Window.\n",
"\n",
"MessageBox\n",
"Ausgabe in MessageBox. In dieser hat man dann die Auswahl, ob das Programm "
"fortgesetzt, beendet (Application::Abort) oder mit CoreDump abgebrochen "
"werden soll. Da eine MessageBox weitere Event-Verarbeitung zulaest koennen "
"jeweils weitere Fehler zum Beispiel durch Paints, Activate/Deactivate, "
"GetFocus/LoseFocus die Ausgabe der Meldung oder weitere und falsche "
"Fehler und Meldungen ausloesen. Deshalb sollte bei Problemen die Meldungen "
"evt. auch in ein File/Debugger geleitet werden um die (richtigen) Fehlermeldungen "
"zu erhalten.\n",
"\n",
"TestTool\n",
"Wenn das TestTool laeuft, werden die Meldungen in das TestTool umgeleitet.\n",
"\n",
"Debugger\n",
"Versucht den Debugger zu aktivieren und dort die Meldung auszugeben, "
"so das man im Debugger dann immer auch den dazugehoerenden Stacktrace "
"erhaellt.\n",
"\n",
"CoreDump\n",
"Erzeugt einen Absturz\n",
"\n",
"\n",
"Einstellungen\n",
"------------------------------------------\n",
"\n",
"Wo standardmaessig das INI-File und LOG-File gelesen und geschrieben "
"wird, kann folgendermassen eingestellt werden:\n",
"\n",
"WIN/WNT (WIN.INI, Gruppe SV, Default: dbgsv.ini und dbgsv.log):\n",
"INI: dbgsv\n",
"LOG: dbgsvlog\n",
"\n",
"OS2 (OS2.INI, Application SV, Default: dbgsv.ini und dbgsv.log):\n",
"INI: DBGSV\n",
"LOG: DBGSVLOG\n",
"\n",
"UNIX (Environment-Variable, Default: .dbgsv.init und dbgsv.log):\n",
"INI: DBGSV_INIT\n",
"LOG: DBGSV_LOG\n",
"\n",
"MAC (Default: dbgsv.ini und dbgsv.log):\n",
"INI: keine Moeglichkeit\n",
"LOG: nur Debug-Dialogeinstellung\n",
"\n",
"Es muss jeweils Pfad und Dateiname angegeben werden. Der Name der "
"Log-Datei, der im Debug-Dialog eintragen wurde, hat immer vorrang.\n",
"\n",
"\n",
"Beispiel\n",
"------------------------------------------\n",
"\n",
"DBG_NAME( String );\n",
"\n",
"#ifdef DBG_UTIL\n",
"const sal_Char* DbgCheckString( const void* pString )\n",
"{\n",
"    String* p = (String*)pString;\n",
"\n",
"    if ( p->mpData->maStr[p->mpData->mnLen] != 0 )\n",
"        return \"String damaged: aStr[nLen] != 0\";\n",
"\n",
"    return NULL;\n",
"}\n",
"#endif\n",
"\n",
"String::String()\n",
"{\n",
"    DBG_CTOR( String, DbgCheckString );\n",
"    // ...\n",
"}\n",
"\n",
"String::~String()\n",
"{\n",
"    DBG_DTOR( String, DbgCheckString );\n",
"    //...\n",
"}\n",
"\n",
"char& String::operator [] ( USHORT nIndex )\n",
"{\n",
"    DBG_CHKTHIS( String, DbgCheckString );\n",
"    DBG_ASSERT( nIndex <= pData->nLen, \"String::[] : nIndex > Len\" );\n",
"\n",
"    //...\n",
"}\n",
"\n",
"USHORT String::Search( const String& rStr, USHORT nIndex ) const\n",
"{\n",
"    DBG_CHKTHIS( String, DbgCheckString );\n",
"    DBG_CHKOBJ( &rStr, String, DbgCheckString );\n",
"\n",
"    //...\n",
"}",
"\n",
#endif
NULL
};

// =======================================================================

// -------------
// - DbgWindow -
// -------------

#define DBGWIN_MAXLINES     100

class DbgWindow : public WorkWindow
{
private:
    ListBox         maLstBox;

public:
                    DbgWindow();

    virtual BOOL    Close();
    virtual void    Resize();
    void            InsertLine( const XubString& rLine );
    void            Update() { WorkWindow::Update(); maLstBox.Update(); }
};

// -----------------
// - DbgInfoDialog -
// -----------------

class DbgInfoDialog : public ModalDialog
{
private:
    ListBox         maListBox;
    OKButton        maOKButton;
    BOOL            mbHelpText;

public:
                    DbgInfoDialog( Window* pParent, BOOL bHelpText = FALSE );

    void            SetInfoText( const XubString& rStr );
};

// -------------
// - DbgDialog -
// -------------

class DbgDialog : public ModalDialog
{
private:
    CheckBox        maXtorThis;
    CheckBox        maXtorFunc;
    CheckBox        maXtorExit;
    CheckBox        maXtorReport;
    CheckBox        maXtorTrace;
    GroupBox        maBox1;

    CheckBox        maMemInit;
    CheckBox        maMemOverwrite;
    CheckBox        maMemOverwriteFree;
    CheckBox        maMemPtr;
    CheckBox        maMemReport;
    CheckBox        maMemTrace;
    CheckBox        maMemLeakReport;
    CheckBox        maMemNewDel;
    CheckBox        maMemXtor;
    CheckBox        maMemSysAlloc;
    GroupBox        maBox2;

    CheckBox        maProf;
    CheckBox        maRes;
    CheckBox        maDialog;
    CheckBox        maBoldAppFont;
    GroupBox        maBox3;

    Edit            maDebugName;
    CheckBox        maOverwrite;
    FixedText       maInclClassText;
    Edit            maInclClassFilter;
    FixedText       maExclClassText;
    Edit            maExclClassFilter;
    FixedText       maInclText;
    Edit            maInclFilter;
    FixedText       maExclText;
    Edit            maExclFilter;
    FixedText       maTraceText;
    ListBox         maTraceBox;
    FixedText       maWarningText;
    ListBox         maWarningBox;
    FixedText       maErrorText;
    ListBox         maErrorBox;
    GroupBox        maBox4;

    OKButton        maOKButton;
    CancelButton    maCancelButton;
    PushButton      maInfoButton;
    HelpButton      maHelpButton;
    USHORT          mnErrorOff;

public:
                    DbgDialog();

                    DECL_LINK( ClickHdl, Button* );
    void            RequestHelp( const HelpEvent& rHEvt );
};

// =======================================================================

static sal_Char aDbgInfoBuf[12288];
static sal_Char aDbgOutBuf[DBG_BUF_MAXLEN];

// =======================================================================

DbgWindow::DbgWindow() :
    WorkWindow( NULL, WB_STDWORK ),
    maLstBox( this, WB_AUTOHSCROLL )
{
    DbgData* pData = DbgGetData();

    maLstBox.Show();
    maLstBox.SetPosPixel( Point( 0, 0 ) );

    SetOutputSizePixel( Size( 250, 400 ) );
    if ( pData->aDbgWinState )
    {
        ByteString aState( pData->aDbgWinState );
        SetWindowState( aState );
    }

    SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "StarView Debug Window" ) ) );
    Show();
    Update();
}

// -----------------------------------------------------------------------

BOOL DbgWindow::Close()
{
    delete this;
    ImplGetSVData()->maWinData.mpDbgWin = NULL;
    return TRUE;
}

// -----------------------------------------------------------------------

void DbgWindow::Resize()
{
    maLstBox.SetSizePixel( GetOutputSizePixel() );
}

// -----------------------------------------------------------------------

void DbgWindow::InsertLine( const XubString& rLine )
{
    XubString aStr = rLine;
    aStr.ConvertLineEnd( LINEEND_LF );
    xub_StrLen  nPos = aStr.Search( _LF );
    while ( nPos != STRING_NOTFOUND )
    {
        if ( maLstBox.GetEntryCount() >= DBGWIN_MAXLINES )
            maLstBox.RemoveEntry( 0 );
        maLstBox.InsertEntry( aStr.Copy( 0, nPos ) );
        aStr.Erase( 0, nPos+1 );
        nPos = aStr.Search( _LF );
    }
    if ( maLstBox.GetEntryCount() >= DBGWIN_MAXLINES )
        maLstBox.RemoveEntry( 0 );
    maLstBox.InsertEntry( aStr );
    maLstBox.SetTopEntry( DBGWIN_MAXLINES-1 );
    maLstBox.Update();
}

// =======================================================================

DbgDialog::DbgDialog() :
    ModalDialog( Application::GetAppWindow(), WB_STDMODAL | WB_SYSTEMWINDOW ),
    maXtorThis( this ),
    maXtorFunc( this ),
    maXtorExit( this ),
    maXtorReport( this ),
    maXtorTrace( this ),
    maBox1( this ),
    maMemInit( this ),
    maMemOverwrite( this ),
    maMemOverwriteFree( this ),
    maMemPtr( this ),
    maMemReport( this ),
    maMemTrace( this ),
    maMemLeakReport( this ),
    maMemNewDel( this ),
    maMemXtor( this ),
    maMemSysAlloc( this ),
    maBox2( this ),
    maProf( this ),
    maRes( this ),
    maDialog( this ),
    maBoldAppFont( this ),
    maBox3( this ),
    maDebugName( this ),
    maOverwrite( this ),
    maInclClassText( this ),
    maInclClassFilter( this ),
    maExclClassText( this ),
    maExclClassFilter( this ),
    maInclText( this ),
    maInclFilter( this ),
    maExclText( this ),
    maExclFilter( this ),
    maTraceText( this ),
    maTraceBox( this, WB_DROPDOWN ),
    maWarningText( this ),
    maWarningBox( this, WB_DROPDOWN ),
    maErrorText( this ),
    maErrorBox( this, WB_DROPDOWN ),
    maBox4( this ),
    maOKButton( this, WB_DEFBUTTON ),
    maCancelButton( this ),
    maInfoButton( this ),
    maHelpButton( this )
{
    DbgData*    pData = DbgGetData();
    MapMode     aAppMap( MAP_APPFONT );
    Size        aButtonSize = LogicToPixel( Size( 60, 12 ), aAppMap );

    {
    maXtorThis.Show();
    maXtorThis.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "T~his" ) ) );
    if ( pData->nTestFlags & DBG_TEST_XTOR_THIS )
        maXtorThis.Check( TRUE );
    maXtorThis.SetPosSizePixel( LogicToPixel( Point( 10, 15 ), aAppMap ),
                                aButtonSize );
    }

    {
    maXtorFunc.Show();
    maXtorFunc.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Function" ) ) );
    if ( pData->nTestFlags & DBG_TEST_XTOR_FUNC )
        maXtorFunc.Check( TRUE );
    maXtorFunc.SetPosSizePixel( LogicToPixel( Point( 75, 15 ), aAppMap ),
                                aButtonSize );
    }

    {
    maXtorExit.Show();
    maXtorExit.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "E~xit" ) ) );
    if ( pData->nTestFlags & DBG_TEST_XTOR_EXIT )
        maXtorExit.Check( TRUE );
    maXtorExit.SetPosSizePixel( LogicToPixel( Point( 140, 15 ), aAppMap ),
                                aButtonSize );
    }

    {
    maXtorReport.Show();
    maXtorReport.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Report" ) ) );
    if ( pData->nTestFlags & DBG_TEST_XTOR_REPORT )
        maXtorReport.Check( TRUE );
    maXtorReport.SetPosSizePixel( LogicToPixel( Point( 205, 15 ), aAppMap ),
                                  aButtonSize );
    }

    {
    maXtorTrace.Show();
    maXtorTrace.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Trace" ) ) );
    if ( pData->nTestFlags & DBG_TEST_XTOR_TRACE )
        maXtorTrace.Check( TRUE );
    maXtorTrace.SetPosSizePixel( LogicToPixel( Point( 270, 15 ), aAppMap ),
                                 aButtonSize );
    }

    {
    maBox1.Show();
    maBox1.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Object Tests" ) ) );
    maBox1.SetPosSizePixel( LogicToPixel( Point( 5, 5 ), aAppMap ),
                            LogicToPixel( Size( 330, 30 ), aAppMap ) );
    }

    {
    maMemInit.Show();
    maMemInit.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Initilize" ) ) );
    if ( pData->nTestFlags & DBG_TEST_MEM_INIT )
        maMemInit.Check( TRUE );
    maMemInit.SetPosSizePixel( LogicToPixel( Point( 10, 50 ), aAppMap ),
                               aButtonSize );
    }

    {
    maMemOverwrite.Show();
    maMemOverwrite.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Overwrite" )) );
    if ( pData->nTestFlags & DBG_TEST_MEM_OVERWRITE )
        maMemOverwrite.Check( TRUE );
    maMemOverwrite.SetPosSizePixel( LogicToPixel( Point( 75, 50 ), aAppMap ),
                                    aButtonSize );
    }

    {
    maMemOverwriteFree.Show();
    maMemOverwriteFree.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Free" ) ) );
    if ( pData->nTestFlags & DBG_TEST_MEM_OVERWRITEFREE )
        maMemOverwriteFree.Check( TRUE );
    maMemOverwriteFree.SetPosSizePixel( LogicToPixel( Point( 140, 50 ), aAppMap ),
                                        aButtonSize );
    }

    {
    maMemPtr.Show();
    maMemPtr.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Pointer" ) ) );
    if ( pData->nTestFlags & DBG_TEST_MEM_POINTER )
        maMemPtr.Check( TRUE );
    maMemPtr.SetPosSizePixel( LogicToPixel( Point( 205, 50 ), aAppMap ),
                              aButtonSize );
    }

    {
    maMemReport.Show();
    maMemReport.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Report" ) ) );
    if ( pData->nTestFlags & DBG_TEST_MEM_REPORT )
        maMemReport.Check( TRUE );
    maMemReport.SetPosSizePixel( LogicToPixel( Point( 270, 50 ), aAppMap ),
                                 aButtonSize );
    }

    {
    maMemTrace.Show();
    maMemTrace.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Trace" ) ) );
    if ( pData->nTestFlags & DBG_TEST_MEM_TRACE )
        maMemTrace.Check( TRUE );
    maMemTrace.SetPosSizePixel( LogicToPixel( Point( 10, 65 ), aAppMap ),
                                aButtonSize );
    }

    {
    maMemLeakReport.Show();
    maMemLeakReport.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Leak-Report" ) ) );
    if ( pData->nTestFlags & DBG_TEST_MEM_LEAKREPORT )
        maMemLeakReport.Check( TRUE );
    maMemLeakReport.SetPosSizePixel( LogicToPixel( Point( 75, 65 ), aAppMap ),
                                     aButtonSize );
    }

    {
    maMemNewDel.Show();
    maMemNewDel.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~New/Delete" ) ) );
    if ( pData->nTestFlags & DBG_TEST_MEM_NEWDEL )
        maMemNewDel.Check( TRUE );
    maMemNewDel.SetPosSizePixel( LogicToPixel( Point( 140, 65 ), aAppMap ),
                                 aButtonSize );
    }

    {
    maMemXtor.Show();
    maMemXtor.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Ob~ject Test" ) ) );
    if ( pData->nTestFlags & DBG_TEST_MEM_XTOR )
        maMemXtor.Check( TRUE );
    maMemXtor.SetPosSizePixel( LogicToPixel( Point( 205, 65 ), aAppMap ),
                               aButtonSize );
    }

    {
    maMemSysAlloc.Show();
    maMemSysAlloc.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "System ~Alloc" ) ) );
    if ( pData->nTestFlags & DBG_TEST_MEM_SYSALLOC )
    {
        maMemSysAlloc.Check( TRUE );
        ClickHdl( &maMemSysAlloc );
    }
    maMemSysAlloc.SetClickHdl( LINK( this, DbgDialog, ClickHdl ) );
    maMemSysAlloc.SetPosSizePixel( LogicToPixel( Point( 270, 65 ), aAppMap ),
                                   aButtonSize );
    }

    {
    maBox2.Show();
    maBox2.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Memory Tests" ) ) );
    maBox2.SetPosSizePixel( LogicToPixel( Point( 5, 40 ), aAppMap ),
                            LogicToPixel( Size( 330, 40 ), aAppMap ) );
    }

    {
    maProf.Show();
    maProf.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Profiling" ) ) );
    if ( pData->nTestFlags & DBG_TEST_PROFILING )
        maProf.Check( TRUE );
    maProf.SetPosSizePixel( LogicToPixel( Point( 10, 95 ), aAppMap ),
                            aButtonSize );
    }

    {
    maRes.Show();
    maRes.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Resourcen" ) ) );
    if ( pData->nTestFlags & DBG_TEST_RESOURCE )
        maRes.Check( TRUE );
    maRes.SetPosSizePixel( LogicToPixel( Point( 75, 95 ), aAppMap ),
                           aButtonSize );
    }

    {
    maDialog.Show();
    maDialog.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Dialog" ) ) );
    if ( pData->nTestFlags & DBG_TEST_DIALOG )
        maDialog.Check( TRUE );
    maDialog.SetPosSizePixel( LogicToPixel( Point( 140, 95 ), aAppMap ),
                              aButtonSize );
    }

    {
    maBoldAppFont.Show();
    maBoldAppFont.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Bold AppFont" ) ) );
    if ( pData->nTestFlags & DBG_TEST_BOLDAPPFONT )
        maBoldAppFont.Check( TRUE );
    maBoldAppFont.SetPosSizePixel( LogicToPixel( Point( 205, 95 ), aAppMap ),
                                   aButtonSize );
    maBoldAppFont.SaveValue();
    }

    {
    maBox3.Show();
    maBox3.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Test Options" ) ) );
    maBox3.SetPosSizePixel( LogicToPixel( Point( 5, 85 ), aAppMap ),
                            LogicToPixel( Size( 330, 30 ), aAppMap ) );
    }

    {
    maDebugName.Show();
    maDebugName.SetText( XubString( pData->aDebugName, RTL_TEXTENCODING_UTF8 ) );
    maDebugName.SetMaxTextLen( sizeof( pData->aDebugName ) );
    maDebugName.SetPosSizePixel( LogicToPixel( Point( 10, 130 ), aAppMap ),
                                 LogicToPixel( Size( 185, 14 ), aAppMap ) );
    }

    {
    maOverwrite.Show();
    maOverwrite.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Overwrite ~File" ) ) );
    if ( pData->bOverwrite )
        maOverwrite.Check( TRUE );
    maOverwrite.SetPosSizePixel( LogicToPixel( Point( 205, 130 ), aAppMap ),
                                 aButtonSize );
    }

    {
    maInclClassText.Show();
    maInclClassText.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Include-ObjectTest-Filter" ) ) );
    maInclClassText.SetPosSizePixel( LogicToPixel( Point( 10, 150 ), aAppMap ),
                                     LogicToPixel( Size( 95, 9 ), aAppMap ) );
    }

    {
    maInclClassFilter.Show();
    maInclClassFilter.SetText( XubString( pData->aInclClassFilter, RTL_TEXTENCODING_UTF8 ) );
    maInclClassFilter.SetMaxTextLen( sizeof( pData->aInclClassFilter ) );
    maInclClassFilter.SetPosSizePixel( LogicToPixel( Point( 10, 160 ), aAppMap ),
                                       LogicToPixel( Size( 95, 14 ), aAppMap ) );
    }

    {
    maExclClassText.Show();
    maExclClassText.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Exclude-ObjectTest-Filter" ) ) );
    maExclClassText.SetPosSizePixel( LogicToPixel( Point( 115, 150 ), aAppMap ),
                                     LogicToPixel( Size( 95, 9 ), aAppMap ) );
    }

    {
    maExclClassFilter.Show();
    maExclClassFilter.SetText( XubString( pData->aExclClassFilter, RTL_TEXTENCODING_UTF8 ) );
    maExclClassFilter.SetMaxTextLen( sizeof( pData->aExclClassFilter ) );
    maExclClassFilter.SetPosSizePixel( LogicToPixel( Point( 115, 160 ), aAppMap ),
                                       LogicToPixel( Size( 95, 14 ), aAppMap ) );
    }

    {
    maInclText.Show();
    maInclText.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Include-Filter" ) ) );
    maInclText.SetPosSizePixel( LogicToPixel( Point( 10, 180 ), aAppMap ),
                                LogicToPixel( Size( 95, 9 ), aAppMap ) );
    }

    {
    maInclFilter.Show();
    maInclFilter.SetText( XubString( pData->aInclFilter, RTL_TEXTENCODING_UTF8 ) );
    maInclFilter.SetMaxTextLen( sizeof( pData->aInclFilter ) );
    maInclFilter.SetPosSizePixel( LogicToPixel( Point( 10, 190 ), aAppMap ),
                                  LogicToPixel( Size( 95, 14 ), aAppMap ) );
    }

    {
    maExclText.Show();
    maExclText.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Exclude-Filter" ) ) );
    maExclText.SetPosSizePixel( LogicToPixel( Point( 115, 180 ), aAppMap ),
                                LogicToPixel( Size( 95, 9 ), aAppMap ) );
    }

    {
    maExclFilter.Show();
    maExclFilter.SetText( XubString( pData->aExclFilter, RTL_TEXTENCODING_UTF8 ) );
    maExclFilter.SetMaxTextLen( sizeof( pData->aExclFilter ) );
    maExclFilter.SetPosSizePixel( LogicToPixel( Point( 115, 190 ), aAppMap ),
                                  LogicToPixel( Size( 95, 14 ), aAppMap ) );
    }

    {
    maTraceText.Show();
    maTraceText.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Trace" ) ) );
    maTraceText.SetPosSizePixel( LogicToPixel( Point( 10, 210 ), aAppMap ),
                                 LogicToPixel( Size( 95, 9 ), aAppMap ) );
    }

    {
    maTraceBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "None" ) ) );
    maTraceBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "File" ) ) );
    maTraceBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Window" ) ) );
    maTraceBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Shell" ) ) );
    maTraceBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "MessageBox" ) ) );
    maTraceBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "TestTool" ) ) );
    maTraceBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Debugger" ) ) );
    maTraceBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "CoreDump" ) ) );
    maTraceBox.SelectEntryPos( (USHORT)pData->nTraceOut );
    maTraceBox.Show();
    maTraceBox.SetPosSizePixel( LogicToPixel( Point( 10, 220 ), aAppMap ),
                                LogicToPixel( Size( 95, 80 ), aAppMap ) );
    }

    {
    maWarningText.Show();
    maWarningText.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Warning" ) ) );
    maWarningText.SetPosSizePixel( LogicToPixel( Point( 115, 210 ), aAppMap ),
                                   LogicToPixel( Size( 95, 9 ), aAppMap ) );
    }

    {
    maWarningBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "None" ) ) );
    maWarningBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "File" ) ) );
    maWarningBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Window" ) ) );
    maWarningBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Shell" ) ) );
    maWarningBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "MessageBox" ) ) );
    maWarningBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "TestTool" ) ) );
    maWarningBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Debugger" ) ) );
    maWarningBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "CoreDump" ) ) );
    maWarningBox.SelectEntryPos( (USHORT)pData->nWarningOut );
    maWarningBox.Show();
    maWarningBox.SetPosSizePixel( LogicToPixel( Point( 115, 220 ), aAppMap ),
                                  LogicToPixel( Size( 95, 80 ), aAppMap ) );
    }

    {
    maErrorText.Show();
    maErrorText.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Error" ) ) );
    maErrorText.SetPosSizePixel( LogicToPixel( Point( 220, 210 ), aAppMap ),
                                 LogicToPixel( Size( 95, 9 ), aAppMap ) );
    }

    {
    if ( DbgIsAllErrorOut() )
    {
        maErrorBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "None" ) ) );
        maErrorBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "File" ) ) );
        maErrorBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Window" ) ) );
        maErrorBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Shell" ) ) );
        mnErrorOff = 0;
    }
    else
        mnErrorOff = 4;
    maErrorBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "MessageBox" ) ) );
    maErrorBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "TestTool" ) ) );
    maErrorBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Debugger" ) ) );
    maErrorBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "CoreDump" ) ) );
    maErrorBox.SelectEntryPos( (USHORT)pData->nErrorOut-mnErrorOff );
    maErrorBox.Show();
    maErrorBox.SetPosSizePixel( LogicToPixel( Point( 220, 220 ), aAppMap ),
                                LogicToPixel( Size( 95, 80 ), aAppMap ) );
    }

    {
    maBox4.Show();
    maBox4.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Output" ) ) );
    maBox4.SetPosSizePixel( LogicToPixel( Point( 5, 120 ), aAppMap ),
                            LogicToPixel( Size( 330, 120 ), aAppMap ) );
    }

    {
    maOKButton.Show();
    maOKButton.SetClickHdl( LINK( this, DbgDialog, ClickHdl ) );
    maOKButton.SetPosSizePixel( LogicToPixel( Point( 10, 245 ), aAppMap ),
                                LogicToPixel( Size( 50, 15 ), aAppMap ) );
    }
    {
    maCancelButton.Show();
    maCancelButton.SetPosSizePixel( LogicToPixel( Point( 70, 245 ), aAppMap ),
                                    LogicToPixel( Size( 50, 15 ), aAppMap ) );
    }
    {
    maInfoButton.Show();
    maInfoButton.SetClickHdl( LINK( this, DbgDialog, ClickHdl ) );
    maInfoButton.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Info..." ) ) );
    maInfoButton.SetPosSizePixel( LogicToPixel( Point( 130, 245 ), aAppMap ),
                                  LogicToPixel( Size( 50, 15 ), aAppMap ) );
    }
    {
    maHelpButton.Show();
    maHelpButton.SetPosSizePixel( LogicToPixel( Point( 190, 245 ), aAppMap ),
                                  LogicToPixel( Size( 50, 15 ), aAppMap ) );
    }

    {
#ifdef REMOTE_APPSERVER
    SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "VCL Debug Options (Server)" ) ) );
#else
    SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "VCL Debug Options" ) ) );
#endif
    SetOutputSizePixel( LogicToPixel( Size( 340, 265 ), aAppMap ) );
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( DbgDialog, ClickHdl, Button*, pButton )
{
    if ( pButton == &maOKButton )
    {
        DbgData aData;

        memcpy( &aData, DbgGetData(), sizeof( DbgData ) );
        aData.nTestFlags = 0;

        aData.nTraceOut   = maTraceBox.GetSelectEntryPos();
        aData.nWarningOut = maWarningBox.GetSelectEntryPos();
        aData.nErrorOut   = maErrorBox.GetSelectEntryPos()+mnErrorOff;

        strncpy( aData.aDebugName, ByteString( maDebugName.GetText(), RTL_TEXTENCODING_UTF8 ).GetBuffer(), sizeof( aData.aDebugName ) );
        strncpy( aData.aInclClassFilter, ByteString( maInclClassFilter.GetText(), RTL_TEXTENCODING_UTF8 ).GetBuffer(), sizeof( aData.aInclClassFilter ) );
        strncpy( aData.aExclClassFilter, ByteString( maExclClassFilter.GetText(), RTL_TEXTENCODING_UTF8 ).GetBuffer(), sizeof( aData.aExclClassFilter ) );
        strncpy( aData.aInclFilter, ByteString( maInclFilter.GetText(), RTL_TEXTENCODING_UTF8 ).GetBuffer(), sizeof( aData.aInclFilter ) );
        strncpy( aData.aExclFilter, ByteString( maExclFilter.GetText(), RTL_TEXTENCODING_UTF8 ).GetBuffer(), sizeof( aData.aExclFilter ) );
        aData.aDebugName[sizeof( aData.aDebugName )-1] = '\0';
        aData.aInclClassFilter[sizeof( aData.aInclClassFilter )-1] = '\0';
        aData.aExclClassFilter[sizeof( aData.aExclClassFilter )-1] = '\0';
        aData.aInclFilter[sizeof( aData.aInclFilter )-1] = '\0';
        aData.aExclFilter[sizeof( aData.aExclFilter )-1] = '\0';

        if ( maOverwrite.IsChecked() )
            aData.bOverwrite = TRUE;
        else
            aData.bOverwrite = FALSE;

        if ( maXtorThis.IsChecked() )
            aData.nTestFlags |= DBG_TEST_XTOR_THIS;

        if ( maXtorFunc.IsChecked() )
            aData.nTestFlags |= DBG_TEST_XTOR_FUNC;

        if ( maXtorExit.IsChecked() )
            aData.nTestFlags |= DBG_TEST_XTOR_EXIT;

        if ( maXtorReport.IsChecked() )
            aData.nTestFlags |= DBG_TEST_XTOR_REPORT;

        if ( maXtorTrace.IsChecked() )
            aData.nTestFlags |= DBG_TEST_XTOR_TRACE;

        if ( maMemInit.IsChecked() )
            aData.nTestFlags |= DBG_TEST_MEM_INIT;

        if ( maMemOverwrite.IsChecked() )
            aData.nTestFlags |= DBG_TEST_MEM_OVERWRITE;

        if ( maMemOverwriteFree.IsChecked() )
            aData.nTestFlags |= DBG_TEST_MEM_OVERWRITEFREE;

        if ( maMemPtr.IsChecked() )
            aData.nTestFlags |= DBG_TEST_MEM_POINTER;

        if ( maMemReport.IsChecked() )
            aData.nTestFlags |= DBG_TEST_MEM_REPORT;

        if ( maMemTrace.IsChecked() )
            aData.nTestFlags |= DBG_TEST_MEM_TRACE;

        if ( maMemLeakReport.IsChecked() )
            aData.nTestFlags |= DBG_TEST_MEM_LEAKREPORT;

        if ( maMemNewDel.IsChecked() )
            aData.nTestFlags |= DBG_TEST_MEM_NEWDEL;

        if ( maMemXtor.IsChecked() )
            aData.nTestFlags |= DBG_TEST_MEM_XTOR;

        if ( maMemSysAlloc.IsChecked() )
            aData.nTestFlags |= DBG_TEST_MEM_SYSALLOC;

        if ( maProf.IsChecked() )
            aData.nTestFlags |= DBG_TEST_PROFILING;

        if ( maRes.IsChecked() )
            aData.nTestFlags |= DBG_TEST_RESOURCE;

        if ( maDialog.IsChecked() )
            aData.nTestFlags |= DBG_TEST_DIALOG;

        if ( maBoldAppFont.IsChecked() )
            aData.nTestFlags |= DBG_TEST_BOLDAPPFONT;

        // Fensterposition mit abspeichern
        DbgWindow* pDbgWindow = ImplGetSVData()->maWinData.mpDbgWin;
        if ( pDbgWindow )
        {
            ByteString aState = pDbgWindow->GetWindowState();
            if ( aState.Len() < sizeof( aData.aDbgWinState ) )
                memcpy( aData.aDbgWinState, aState.GetBuffer(), aState.Len() );
        }

        // Daten speichern
        DbgSaveData( aData );

        // Umschalten der Laufzeitwerte
        DBG_INSTOUTTRACE( aData.nTraceOut );
        DBG_INSTOUTWARNING( aData.nWarningOut );
        DBG_INSTOUTERROR( aData.nErrorOut );

        DbgData* pData = DbgGetData();
        pData->nTestFlags &= ~(DBG_TEST_XTOR_TRACE | DBG_TEST_MEM_INIT | DBG_TEST_RESOURCE | DBG_TEST_DIALOG | DBG_TEST_BOLDAPPFONT);
        pData->nTestFlags |= aData.nTestFlags & (DBG_TEST_XTOR_TRACE | DBG_TEST_MEM_INIT | DBG_TEST_RESOURCE | DBG_TEST_DIALOG | DBG_TEST_BOLDAPPFONT);
        strcpy( pData->aInclClassFilter, aData.aInclClassFilter );
        strcpy( pData->aExclClassFilter, aData.aExclClassFilter );
        strcpy( pData->aInclFilter, aData.aInclFilter );
        strcpy( pData->aExclFilter, aData.aExclFilter );
        if ( maBoldAppFont.GetSavedValue() != maBoldAppFont.IsChecked() )
        {
            AllSettings aSettings = Application::GetSettings();
            StyleSettings aStyleSettings = aSettings.GetStyleSettings();
            Font aFont = aStyleSettings.GetAppFont();
            if ( maBoldAppFont.IsChecked() )
                aFont.SetWeight( WEIGHT_BOLD );
            else
                aFont.SetWeight( WEIGHT_NORMAL );
            aStyleSettings.SetAppFont( aFont );
            aSettings.SetStyleSettings( aStyleSettings );
            Application::SetSettings( aSettings );
        }
        EndDialog( TRUE );
    }
    else if ( pButton == &maInfoButton )
    {
        DbgInfoDialog aInfoDialog( this );
        aDbgInfoBuf[0] = '\0';
        DbgMemInfo( aDbgInfoBuf );
        DbgXtorInfo( aDbgInfoBuf );
        XubString aInfoText( aDbgInfoBuf, RTL_TEXTENCODING_UTF8 );
        aInfoDialog.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Debug InfoReport" ) ) );
        aInfoDialog.SetInfoText( aInfoText );
        aInfoDialog.Execute();
    }
    else if ( pButton == &maMemSysAlloc )
    {
        BOOL bEnable = maMemSysAlloc.IsChecked() == 0;
        maMemInit.Enable( bEnable );
        maMemOverwrite.Enable( bEnable );
        maMemOverwriteFree.Enable( bEnable );
        maMemPtr.Enable( bEnable );
        maMemReport.Enable( bEnable );
        maMemTrace.Enable( bEnable );
        maMemLeakReport.Enable( bEnable );
        maMemNewDel.Enable( bEnable );
        maMemXtor.Enable( bEnable );
    }

    return 0;
}

// -----------------------------------------------------------------------

void DbgDialog::RequestHelp( const HelpEvent& rHEvt )
{
    if ( rHEvt.GetMode() & HELPMODE_CONTEXT )
    {
        DbgInfoDialog aInfoDialog( this, TRUE );
        XubString aHelpText;
        sal_Char** pHelpStrs = pDbgHelpText;
        while ( *pHelpStrs )
        {
            aHelpText.AppendAscii( *pHelpStrs );
            pHelpStrs++;
        }
        aInfoDialog.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Debug Hilfe" ) ) );
        aInfoDialog.SetInfoText( aHelpText );
        aInfoDialog.Execute();
    }
}

// =======================================================================

DbgInfoDialog::DbgInfoDialog( Window* pParent, BOOL bHelpText ) :
    ModalDialog( pParent, WB_STDMODAL ),
    maListBox( this, WB_BORDER | WB_AUTOHSCROLL ),
    maOKButton( this, WB_DEFBUTTON )
{
    mbHelpText = bHelpText;

    if ( !bHelpText )
    {
        Font aFont = System::GetStandardFont( STDFONT_FIXED );
        aFont.SetHeight( 8 );
        aFont.SetPitch( PITCH_FIXED );
        maListBox.SetControlFont( aFont );
    }
    maListBox.SetPosSizePixel( Point( 5, 5 ), Size( 630, 380 ) );
    maListBox.Show();

    maOKButton.SetPosSizePixel( Point( 290, 390 ), Size( 60, 25 ) );
    maOKButton.Show();

    SetOutputSizePixel( Size( 640, 420 ) );
}

// -----------------------------------------------------------------------

void DbgInfoDialog::SetInfoText( const XubString& rStr )
{
    maListBox.SetUpdateMode( FALSE );
    maListBox.Clear();
    XubString aStr = rStr;
    aStr.ConvertLineEnd( LINEEND_LF );
    USHORT nIndex = 0;
    USHORT nFoundIndex;
    do
    {
        nFoundIndex = aStr.Search( _LF, nIndex );
        XubString aTextParagraph = aStr.Copy( nIndex, nFoundIndex-nIndex );
        if ( mbHelpText )
        {
            long    nMaxWidth = maListBox.GetOutputSizePixel().Width()-30;
            USHORT  nLastIndex = 0;
            USHORT  nIndex = aTextParagraph.Search( ' ' );
            while ( nIndex != STRING_NOTFOUND )
            {
                if ( maListBox.GetTextWidth( aTextParagraph, 0, nIndex ) > nMaxWidth )
                {
                    if ( !nLastIndex )
                        nLastIndex = nIndex+1;
                    XubString aTempStr = aTextParagraph.Copy( 0, nLastIndex );
                    aTextParagraph.Erase( 0, nLastIndex );
                    maListBox.InsertEntry( aTempStr );
                    nLastIndex = 0;
                }
                else
                    nLastIndex = nIndex+1;
                nIndex = aTextParagraph.Search( ' ', nLastIndex );
            }

            if ( maListBox.GetTextWidth( aTextParagraph, 0, nIndex ) > nMaxWidth )
            {
                if ( !nLastIndex )
                    nLastIndex = nIndex+1;
                XubString aTempStr = aTextParagraph.Copy( 0, nLastIndex );
                aTextParagraph.Erase( 0, nLastIndex );
                maListBox.InsertEntry( aTempStr );
            }
        }
        maListBox.InsertEntry( aTextParagraph );
        nIndex = nFoundIndex+1;
    }
    while ( nFoundIndex != STRING_NOTFOUND );
    maListBox.SetUpdateMode( TRUE );
}

// =======================================================================

void DbgDialogTest( Window* pWindow )
{
    BOOL        aAccelBuf[65536];
    USHORT      nChildCount = pWindow->GetChildCount();
    Window*     pGetChild = pWindow->GetWindow( WINDOW_FIRSTCHILD );
    Window*     pChild;
    Point       aTabPos;

    if ( !pGetChild )
        return;

    Rectangle*  pRectAry = (Rectangle*)new long[(sizeof(Rectangle)*nChildCount)/sizeof(long)];
    memset( aAccelBuf, 0, sizeof( aAccelBuf ) );
    memset( pRectAry, 0, sizeof(Rectangle)*nChildCount );

    if ( pWindow->IsDialog() )
    {
        BOOL    bOKCancelButton = FALSE;
        BOOL    bDefPushButton = FALSE;
        BOOL    bButton = FALSE;
        pGetChild = pWindow->GetWindow( WINDOW_FIRSTCHILD );
        while ( pGetChild )
        {
            pChild = pGetChild->ImplGetWindow();

            if ( pChild->ImplIsPushButton() )
            {
                bButton = TRUE;
                if ( (pChild->GetType() == WINDOW_OKBUTTON) || (pChild->GetType() == WINDOW_CANCELBUTTON) )
                    bOKCancelButton = TRUE;
                if ( pChild->GetStyle() & WB_DEFBUTTON )
                    bDefPushButton = TRUE;
            }

            pGetChild = pGetChild->GetWindow( WINDOW_NEXT );
        }

        if ( bButton )
        {
            if ( !bOKCancelButton )
                DbgError( "Dialogs should have a OK- or CancelButton" );
            if ( !bDefPushButton )
                DbgError( "Dialogs should have a Button with WB_DEFBUTTON" );
        }
    }

    USHORT i = 0;
    pGetChild = pWindow->GetWindow( WINDOW_FIRSTCHILD );
    while ( pGetChild )
    {
        pChild = pGetChild->ImplGetWindow();

        if ( (pChild->GetType() != WINDOW_TABCONTROL) &&
             (pChild->GetType() != WINDOW_TABPAGE) &&
             (pChild->GetType() != WINDOW_GROUPBOX) )
        {
            XubString       aText = pChild->GetText();
            USHORT          nAccelPos;
            xub_Unicode     cAccel = 0;
            if ( aText.Len() )
            {
                nAccelPos = aText.Search( '~' );
                if ( nAccelPos != STRING_NOTFOUND )
                {
                    const International& rIntn = Application::GetSettings().GetInternational();
                    XubString aUpperText = rIntn.Upper( aText );
                    cAccel = aUpperText.GetChar( nAccelPos+1 );
                    if ( pChild->IsVisible() )
                    {
                        if ( aAccelBuf[cAccel] )
                            DbgOutTypef( DBG_OUT_ERROR, "Double mnemonic char: %c", cAccel );
                        else
                            aAccelBuf[cAccel] = TRUE;
                    }
                }
            }

            if ( (pChild->GetType() == WINDOW_RADIOBUTTON) ||
                 (pChild->GetType() == WINDOW_IMAGERADIOBUTTON) ||
                 (pChild->GetType() == WINDOW_CHECKBOX) ||
                 (pChild->GetType() == WINDOW_TRISTATEBOX) ||
                 (pChild->GetType() == WINDOW_PUSHBUTTON) )
            {
                if ( !cAccel && aText.Len() && !aText.EqualsAscii( "..." ) )
                {
                    const char* pClass;
                    if ( pChild->GetType() == WINDOW_RADIOBUTTON )
                        pClass = "RadioButton";
                    else if ( pChild->GetType() == WINDOW_IMAGERADIOBUTTON )
                        pClass = "ImageRadioButton";
                    else if ( pChild->GetType() == WINDOW_CHECKBOX )
                        pClass = "CheckBox";
                    else if ( pChild->GetType() == WINDOW_TRISTATEBOX )
                        pClass = "TriStateBox";
                    else if ( pChild->GetType() == WINDOW_PUSHBUTTON )
                        pClass = "PushButton";
                    else
                        pClass = "Dontknow";
                    DbgOutTypef( DBG_OUT_ERROR,
                                 "%s should have a mnemonic char (~): %s",
                                 pClass,
                                 ByteString( aText, RTL_TEXTENCODING_UTF8 ).GetBuffer() );
                }
            }

            if ( pChild->GetType() == WINDOW_FIXEDTEXT )
            {
                if ( (pChild->GetSizePixel().Height() >= pChild->GetTextHeight()*2) &&
                     !(pChild->GetStyle() & WB_WORDBREAK) )
                {
                    DbgOutTypef( DBG_OUT_ERROR,
                                 "FixedText greater than one line, but WordBreak is not set: %s",
                                 ByteString( aText, RTL_TEXTENCODING_UTF8 ).GetBuffer() );
                }

                if ( (i+1 < nChildCount) && aText.Len() )
                {
                    Window* pTempChild = pGetChild->GetWindow( WINDOW_NEXT )->ImplGetWindow();
                    if ( (pTempChild->GetType() == WINDOW_EDIT) ||
                         (pTempChild->GetType() == WINDOW_MULTILINEEDIT) ||
                         (pTempChild->GetType() == WINDOW_SPINFIELD) ||
                         (pTempChild->GetType() == WINDOW_PATTERNFIELD) ||
                         (pTempChild->GetType() == WINDOW_NUMERICFIELD) ||
                         (pTempChild->GetType() == WINDOW_METRICFIELD) ||
                         (pTempChild->GetType() == WINDOW_CURRENCYFIELD) ||
                         (pTempChild->GetType() == WINDOW_DATEFIELD) ||
                         (pTempChild->GetType() == WINDOW_TIMEFIELD) ||
                         (pTempChild->GetType() == WINDOW_LISTBOX) ||
                         (pTempChild->GetType() == WINDOW_MULTILISTBOX) ||
                         (pTempChild->GetType() == WINDOW_COMBOBOX) ||
                         (pTempChild->GetType() == WINDOW_PATTERNBOX) ||
                         (pTempChild->GetType() == WINDOW_NUMERICBOX) ||
                         (pTempChild->GetType() == WINDOW_METRICBOX) ||
                         (pTempChild->GetType() == WINDOW_CURRENCYBOX) ||
                         (pTempChild->GetType() == WINDOW_DATEBOX) ||
                         (pTempChild->GetType() == WINDOW_TIMEBOX) )
                    {
                        if ( !cAccel )
                        {
                            DbgOutTypef( DBG_OUT_ERROR,
                                         "Labels befor Fields (Edit,ListBox,...) should have a mnemonic char (~): %s",
                                         ByteString( aText, RTL_TEXTENCODING_UTF8 ).GetBuffer() );
                        }
                        if ( !pTempChild->IsEnabled() && pChild->IsEnabled() )
                        {
                            DbgOutTypef( DBG_OUT_ERROR,
                                         "Labels befor Fields (Edit,ListBox,...) should be disabled, when the field is disabled: %s",
                                         ByteString( aText, RTL_TEXTENCODING_UTF8 ).GetBuffer() );
                        }
                    }
                }
            }

            if ( pChild->GetType() == WINDOW_MULTILINEEDIT )
            {
                if ( !(pChild->GetStyle() & WB_IGNORETAB) )
                {
                    DbgOutTypef( DBG_OUT_ERROR,
                                 "MultiLineEdits in Dialogs should have the Style WB_IGNORETAB: %s",
                                 ByteString( aText, RTL_TEXTENCODING_UTF8 ).GetBuffer() );
                }
            }

            if ( (pChild->GetType() == WINDOW_RADIOBUTTON) ||
                 (pChild->GetType() == WINDOW_IMAGERADIOBUTTON) ||
                 (pChild->GetType() == WINDOW_CHECKBOX) ||
                 (pChild->GetType() == WINDOW_TRISTATEBOX) ||
                 (pChild->GetType() == WINDOW_FIXEDTEXT) )
            {
                pChild->SetBackground( Wallpaper( Color( COL_LIGHTGREEN ) ) );
            }

            if ( pChild->IsVisible() )
            {
                BOOL bMaxWarning = FALSE;
                if ( pChild->GetType() == WINDOW_NUMERICFIELD )
                {
                    NumericField* pField = (NumericField*)pChild;
                    if ( pField->GetMax() == LONG_MAX )
                        bMaxWarning = TRUE;
                }
                else if ( pChild->GetType() == WINDOW_METRICFIELD )
                {
                    MetricField* pField = (MetricField*)pChild;
                    if ( pField->GetMax() == LONG_MAX )
                        bMaxWarning = TRUE;
                }
                else if ( pChild->GetType() == WINDOW_CURRENCYFIELD )
                {
                    CurrencyField* pField = (CurrencyField*)pChild;
                    if ( pField->GetMax() == LONG_MAX )
                        bMaxWarning = TRUE;
                }
                else if ( pChild->GetType() == WINDOW_TIMEFIELD )
                {
                    TimeField* pField = (TimeField*)pChild;
                    if ( pField->GetMax() == Time( 23, 59, 59, 99 ) )
                        bMaxWarning = TRUE;
                }
                else if ( pChild->GetType() == WINDOW_DATEFIELD )
                {
                    DateField* pField = (DateField*)pChild;
                    if ( pField->GetMax() == Date( 31, 12, 9999 ) )
                        bMaxWarning = TRUE;
                }
                else if ( pChild->GetType() == WINDOW_NUMERICBOX )
                {
                    NumericBox* pBox = (NumericBox*)pChild;
                    if ( pBox->GetMax() == LONG_MAX )
                        bMaxWarning = TRUE;
                }
                else if ( pChild->GetType() == WINDOW_METRICBOX )
                {
                    MetricBox* pBox = (MetricBox*)pChild;
                    if ( pBox->GetMax() == LONG_MAX )
                        bMaxWarning = TRUE;
                }
                else if ( pChild->GetType() == WINDOW_CURRENCYBOX )
                {
                    CurrencyBox* pBox = (CurrencyBox*)pChild;
                    if ( pBox->GetMax() == LONG_MAX )
                        bMaxWarning = TRUE;
                }
                else if ( pChild->GetType() == WINDOW_TIMEBOX )
                {
                    TimeBox* pBox = (TimeBox*)pChild;
                    if ( pBox->GetMax() == Time( 23, 59, 59, 99 ) )
                        bMaxWarning = TRUE;
                }
                else if ( pChild->GetType() == WINDOW_DATEBOX )
                {
                    DateBox* pBox = (DateBox*)pChild;
                    if ( pBox->GetMax() == Date( 31, 12, 9999 ) )
                        bMaxWarning = TRUE;
                }
                if ( bMaxWarning )
                {
                    DbgOutTypef( DBG_OUT_ERROR,
                                 "No Max-Value is set: %s",
                                 ByteString( aText, RTL_TEXTENCODING_UTF8 ).GetBuffer() );
                }

                if ( (pChild->GetType() == WINDOW_RADIOBUTTON) ||
                     (pChild->GetType() == WINDOW_IMAGERADIOBUTTON) ||
                     (pChild->GetType() == WINDOW_CHECKBOX) ||
                     (pChild->GetType() == WINDOW_TRISTATEBOX) ||
                     (pChild->GetType() == WINDOW_PUSHBUTTON) ||
                     (pChild->GetType() == WINDOW_OKBUTTON) ||
                     (pChild->GetType() == WINDOW_CANCELBUTTON) ||
                     (pChild->GetType() == WINDOW_HELPBUTTON) ||
                     (pChild->GetType() == WINDOW_IMAGEBUTTON) ||
                     (pChild->GetType() == WINDOW_FIXEDTEXT) ||
                     (pChild->GetType() == WINDOW_EDIT) ||
                     (pChild->GetType() == WINDOW_MULTILINEEDIT) ||
                     (pChild->GetType() == WINDOW_SPINFIELD) ||
                     (pChild->GetType() == WINDOW_PATTERNFIELD) ||
                     (pChild->GetType() == WINDOW_NUMERICFIELD) ||
                     (pChild->GetType() == WINDOW_METRICFIELD) ||
                     (pChild->GetType() == WINDOW_CURRENCYFIELD) ||
                     (pChild->GetType() == WINDOW_DATEFIELD) ||
                     (pChild->GetType() == WINDOW_TIMEFIELD) ||
                     (pChild->GetType() == WINDOW_LISTBOX) ||
                     (pChild->GetType() == WINDOW_MULTILISTBOX) ||
                     (pChild->GetType() == WINDOW_COMBOBOX) ||
                     (pChild->GetType() == WINDOW_PATTERNBOX) ||
                     (pChild->GetType() == WINDOW_NUMERICBOX) ||
                     (pChild->GetType() == WINDOW_METRICBOX) ||
                     (pChild->GetType() == WINDOW_CURRENCYBOX) ||
                     (pChild->GetType() == WINDOW_DATEBOX) ||
                     (pChild->GetType() == WINDOW_TIMEBOX) )
                {
                    Point       aNewPos = pChild->GetPosPixel();
                    Rectangle   aChildRect( aNewPos, pChild->GetSizePixel() );

                    if ( cAccel || (pChild->GetStyle() & WB_TABSTOP) ||
                         (pChild->GetType() == WINDOW_RADIOBUTTON) ||
                         (pChild->GetType() == WINDOW_IMAGERADIOBUTTON) )
                    {
                        if ( (aNewPos.X() <= aTabPos.X()) && (aNewPos.Y() <= aTabPos.Y()) )
                        {
                            DbgOutTypef( DBG_OUT_ERROR,
                                         "Possible wrong childorder for dialogcontrol: %s",
                                         ByteString( aText, RTL_TEXTENCODING_UTF8 ).GetBuffer() );
                        }
                        aTabPos = aNewPos;
                    }

                    for ( USHORT j = 0; j < i; j++ )
                    {
                        if ( ((pRectAry[j].Right() != 0) || (pRectAry[j].Bottom() != 0)) &&
                             aChildRect.IsOver( pRectAry[j] ) )
                        {
                            DbgOutTypef( DBG_OUT_ERROR,
                                         "Window overlaps with sibling window: %s",
                                         ByteString( aText, RTL_TEXTENCODING_UTF8 ).GetBuffer() );
                        }
                    }
                    pRectAry[i] = aChildRect;
                }
            }
        }

        pGetChild = pGetChild->GetWindow( WINDOW_NEXT );
        i++;
    }

    delete pRectAry;
}

// =======================================================================

void DbgPrintMsgBox( const char* pLine )
{
    if ( Application::IsDialogCancelEnabled() )
    {
#if defined( WNT )
        if ( GetSystemMetrics( SM_DEBUG ) )
        {
            strcpy( aDbgOutBuf, pLine );
            strcat( aDbgOutBuf, "\r\n" );
            OutputDebugString( aDbgOutBuf );
            return;
        }
#endif
#ifdef UNX
        fprintf( stderr, "%s\n", pLine );
        return;
#else
        DbgPrintFile( pLine );
        return;
#endif
    }

    strcpy( aDbgOutBuf, pLine );
    strcat( aDbgOutBuf, "\nAbort ? (Yes=abort / No=ignore / Cancel=core dump)" );

    // Tracking beenden und Mouse freigeben, damit die Boxen nicht haengen
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maWinData.mpTrackWin )
        pSVData->maWinData.mpTrackWin->EndTracking( ENDTRACK_CANCEL );
    if ( pSVData->maWinData.mpCaptureWin )
        pSVData->maWinData.mpCaptureWin->ReleaseMouse();

#ifndef REMOTE_APPSERVER
#if defined( WNT )
    BOOL bOldCallTimer = pSVData->mbNoCallTimer;
    pSVData->mbNoCallTimer = TRUE;
    short nRet = (short)MessageBox( 0, (LPSTR)aDbgOutBuf, "Debug Output",
                                    MB_TASKMODAL | MB_YESNOCANCEL |
                                    MB_DEFBUTTON2 | MB_ICONSTOP );
    MessageBeep( MB_ICONHAND );
    pSVData->mbNoCallTimer = bOldCallTimer;
    switch ( nRet )
    {
        case IDYES:
            nRet = RET_YES;
            break;
        case IDNO:
            nRet = RET_NO;
            break;
        case IDCANCEL:
            nRet = RET_CANCEL;
            break;
    }
#elif defined( OS2 )
    BOOL bOldCallTimer = pSVData->mbNoCallTimer;
    pSVData->mbNoCallTimer = TRUE;
    PM_ULONG nRet = WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
                                   (PSZ)aDbgOutBuf, (PSZ)"Debug Output", 0,
                                   MB_APPLMODAL | MB_MOVEABLE |
                                   MB_YESNOCANCEL | MB_DEFBUTTON2 |
                                   MB_ERROR );
    pSVData->mbNoCallTimer = bOldCallTimer;
    switch ( nRet )
    {
        case MBID_YES:
            nRet = RET_YES;
            break;
        case MBID_NO:
            nRet = RET_NO;
            break;
        case MBID_CANCEL:
            nRet = RET_CANCEL;
            break;
    }
#else
    USHORT nOldMode = Application::GetSystemWindowMode();
    Application::SetSystemWindowMode( nOldMode & ~SYSTEMWINDOW_MODE_NOAUTOMODE );
    ErrorBox aBox( Application::GetAppWindow(), WB_YES_NO_CANCEL | WB_DEF_NO,
                   UniString( aDbgOutBuf, RTL_TEXTENCODING_UTF8 ) );
    aBox.SetText( String( RTL_CONSTASCII_USTRINGPARAM("Debug Output") ) );
    Application::SetSystemWindowMode( nOldMode );
    short nRet = aBox.Execute();
#endif
#else
#if defined( WNT )
    if ( GetSystemMetrics( SM_DEBUG ) )
    {
        strcpy( aDbgOutBuf, pLine );
        strcat( aDbgOutBuf, "\r\n" );
        OutputDebugString( aDbgOutBuf );
        return;
    }
#endif
#ifdef UNX
    fprintf( stderr, "%s\n", pLine );
    return;
#else
    DbgPrintFile( pLine );
    return;
#endif
/*
    USHORT nOldMode = Application::GetSystemWindowMode();
    Application::SetSystemWindowMode( nOldMode & ~SYSTEMWINDOW_MODE_NOAUTOMODE );
    ErrorBox aBox( Application::GetAppWindow(), WB_YES_NO_CANCEL | WB_DEF_NO,
                   UniString( aDbgOutBuf, RTL_TEXTENCODING_UTF8 ) );
    aBox.SetText( String( RTL_CONSTASCII_USTRINGPARAM("Debug Output (Server)") ) );
    Application::SetSystemWindowMode( nOldMode );
    short nRet = aBox.Execute();
*/
#endif
#ifndef REMOTE_APPSERVER
    if ( nRet == RET_YES )
        GetpApp()->Abort( XubString( RTL_CONSTASCII_USTRINGPARAM( "Debug-Utilities-Error" ) ) );
    else if ( nRet == RET_CANCEL )
        DbgCoreDump();
#endif
}

// -----------------------------------------------------------------------

void DbgPrintWindow( const char* pLine )
{
    static BOOL bIn = FALSE;

    // keine rekursiven Traces
    if ( bIn )
        return;
    bIn = TRUE;

    DbgWindow* pDbgWindow = ImplGetSVData()->maWinData.mpDbgWin;
    if ( !pDbgWindow )
    {
        pDbgWindow = new DbgWindow;
        ImplGetSVData()->maWinData.mpDbgWin = pDbgWindow;
    }

    pDbgWindow->InsertLine( XubString( pLine, RTL_TEXTENCODING_UTF8 ) );

    bIn = FALSE;
}

// -----------------------------------------------------------------------

void DbgPrintShell( const char* pLine )
{
#if defined( WNT )
    if ( GetSystemMetrics( SM_DEBUG ) )
    {
        strcpy( aDbgOutBuf, pLine );
        strcat( aDbgOutBuf, "\r\n" );
        OutputDebugString( aDbgOutBuf );
        return;
    }
#endif
#ifdef UNX
    fprintf( stderr, "%s\n", pLine );
    return;
#endif

    DbgPrintWindow( pLine );
}

// =======================================================================

#if defined( REMOTE_APPSERVER ) || defined( WNT ) || defined( OS2 )
void ImplDbgTestSolarMutex();
#endif

// =======================================================================

void DbgGUIInit()
{
    DbgSetPrintMsgBox( DbgPrintMsgBox );
    DbgSetPrintWindow( DbgPrintWindow );
    DbgSetPrintShell( DbgPrintShell );
#if defined( REMOTE_APPSERVER ) || defined( WNT ) || defined( OS2 )
    DbgSetTestSolarMutex( ImplDbgTestSolarMutex );
#endif
}

// -----------------------------------------------------------------------

void DbgGUIDeInit()
{
    DbgSetPrintMsgBox( NULL );
    DbgSetPrintWindow( NULL );
    DbgSetPrintShell( NULL );
#if defined( REMOTE_APPSERVER ) || defined( WNT ) || defined( OS2 )
    DbgSetTestSolarMutex( NULL );
#endif

    DbgWindow* pDbgWindow = ImplGetSVData()->maWinData.mpDbgWin;
    if ( pDbgWindow )
        delete pDbgWindow;
}

// -----------------------------------------------------------------------

void DbgGUIStart()
{
    DbgData* pData = DbgGetData();

    if ( pData )
    {
        DbgDialog* pDialog = new DbgDialog;
        // Fuer den Debug-Dialog schalten wir Dialogtests aus
        ULONG nOldFlags = pData->nTestFlags;
        pData->nTestFlags &= ~DBG_TEST_DIALOG;
        if ( !pDialog->Execute() )
            pData->nTestFlags |= (nOldFlags & DBG_TEST_DIALOG);
        delete pDialog;
    }
    else
    {
        ErrorBox( 0, WB_OK,
                  XubString( RTL_CONSTASCII_USTRINGPARAM( "TOOLS Library has no Debug-Routines" ) ) ).Execute();
    }
}

#endif // DBG_UTIL
