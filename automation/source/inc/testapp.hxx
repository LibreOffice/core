/*************************************************************************
 *
 *  $RCSfile: testapp.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-18 16:03:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2002
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2002 by Sun Microsystems, Inc.
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef TESTAPP_HXX
#define TESTAPP_HXX

#include <basic/sbmod.hxx>
#ifndef _BASIC_TESTTOOL_HXX_
#include <basic/testtool.hxx>
#endif

class CommunicationLink;
class CommunicationManager;
class CNames;
class ControlItemUId;
class CRevNames;
//class SbxTransportVariableRef;
class ControlsRef;
class CmdStream;
class FloatingLoadConf;
class TestToolObj;
class ControlDef;

class SbxTransportMethod;
class Application;

class SotStorage;

class ImplTestToolObj;
class MyBasic;

class ErrorEntry
{
public:
    ErrorEntry(ULONG nNr, String aStr = String()) : nError(nNr),aText(aStr),nLine(0),nCol1(0),nCol2(0) {}
    ErrorEntry(ULONG nNr, String aStr, xub_StrLen l, xub_StrLen c1, xub_StrLen c2 )
        : nError(nNr),aText(aStr),nLine(l),nCol1(c1),nCol2(c2) {}
    String aText;
    ULONG nError;
    xub_StrLen nLine;
    xub_StrLen nCol1;
    xub_StrLen nCol2;
};

SV_DECL_PTRARR_DEL(CErrors, ErrorEntry*, 1, 1)

struct ControlDefLoad {
    char* Kurzname;
    ULONG nUId;
};

class TestToolObj: public SbxObject
{
    friend class TTBasic;
    friend class Controls;
public:
    TestToolObj( String aName, String aFilePath );      // Alle Dateien in FilePath, Kein IPC
    TestToolObj( String aName, MyBasic* pBas ); // Pfade aus INI, IPC benutzen
    ~TestToolObj();
    void LoadIniFile();             // Laden der IniEinstellungen, die durch den ConfigDialog geändert werden können
    void DebugFindNoErrors( BOOL bDebugFindNoErrors );

private:
    BOOL bWasPrecompilerError;  // True wenn beim letzten Precompile ein Fehler auftrat
    BOOL CError( ULONG, const String&, xub_StrLen, xub_StrLen, xub_StrLen );
    void CalcPosition( String const &aSource, xub_StrLen nPos, xub_StrLen &l, xub_StrLen &c );
    xub_StrLen ImplSearch( const String &aSource, const xub_StrLen nStart, const xub_StrLen nEnd, const String &aSearch, const xub_StrLen nSearchStart = 0 );
    xub_StrLen PreCompilePart( String &aSource, xub_StrLen nStart, xub_StrLen nEnd, String aFinalErrorLabel, USHORT &nLabelCount );
    void PreCompileDispatchParts( String &aSource, String aStart, String aEnd, String aFinalLable );
public:
    String PreCompile(String const &aSourceIn); // try catch; testcase endcase ..
    BOOL WasPrecompilerError(); // True wenn beim letzten Precompile ein Fehler auftrat
    void            SFX_NOTIFY( SfxBroadcaster&, const TypeId&, const SfxHint& rHint, const TypeId& );
    virtual SbxVariable* Find( const String&, SbxClassType );
//  String aKeyPlusClasses;     // Pfad für keycodes & classes & res_type (Aus Configdatei)
    DECL_LINK( ReturnResultsLink, CommunicationLink* );
    BOOL            ReturnResults( SvStream *pIn ); // Rücklieferung des Antwortstreams über IPC oder TCP/IP oder direkt

    void            SetLogHdl( const Link& rLink ) { aLogHdl = rLink; }
    const Link&     GetLogHdl() const { return aLogHdl; }

    void            SetWinInfoHdl( const Link& rLink ) { aWinInfoHdl = rLink; }
    const Link&     GetWinInfoHdl() const { return aWinInfoHdl; }

    void            SetModuleWinExistsHdl( const Link& rLink ) { aModuleWinExistsHdl = rLink; }
    const Link&     GetModuleWinExistsHdl() const { return aModuleWinExistsHdl; }

    void            SetCErrorHdl( const Link& rLink ) { aCErrorHdl = rLink; }
    const Link&     GetCErrorHdl() const { return aCErrorHdl; }

    void            SetWriteStringHdl( const Link& rLink ) { aWriteStringHdl = rLink; }
    const Link&     GetWriteStringHdl() const { return aWriteStringHdl; }

    SfxBroadcaster& GetTTBroadcaster();

private:
    ImplTestToolObj *pImpl;     // Alles was von der Implementation abhängt
    CErrors* const GetFehlerListe() { return pFehlerListe; }
    BOOL bQuietErrors;          // Wenn Control in pControls nicht gefunden kein Programmabbruch
    BOOL bUseIPC;
    Link aLogHdl;               // Zum Logen der Fehlermeldungen im Testtool
    Link aWinInfoHdl;           // Anzeigen der Windows/Controls der zu testenden App
    Link aModuleWinExistsHdl;   // Prüft ob das Modul schon im Editor geladen ist
    Link aCErrorHdl;            // Melden von Compilererror
    Link aWriteStringHdl;       // Schreiben von text (e.g. MakroRecorder)
    CErrors *pFehlerListe;      // Hier werden die Fehler des Testtools gespeichert
    BOOL bReturnOK;             // Bricht WaitForAnswer ab
    CRevNames *pShortNames;     // Aktuell verwendete Controls, zur gewinnung des Namens aus Fehlermeldung
    ULONG nSequence;            // Sequence um Antwort und Anfrage zu syncronisieren
    TTUniqueId aNextReturnId;   // Id des Returnwertes i.e. UId
    void ReplaceNumbers(String &aText); // Zahlen im String mit speziellem Format in Namen umwandeln

    String aLastRecordedKontext;//  Keeps the last kontext recorded by the Macro Recorder

#define FLAT TRUE
    String ProgPath;            // Dateiname der zu Testenden APP; Gesetzt über Start
    String aLogFileName;        // Momentaner Logfilename (Wie Programmdatei aber mit .res)
    BOOL IsBlock;               // Innerhalb Begin/EndBlock
    BOOL SingleCommandBlock;    // Implizit um jedes kommando ein Begin/EndBlock
    CmdStream *In;

    void AddName(String &aBisher, String &aNeu );   // Name eventuell mit / anhängen
    void AddToListByNr( CNames *&pControls, ControlItemUId *&pNewItem );    //
    CNames *pControls;
    CNames *pNameKontext;       // Zeigt auf den aktuellen Namenskontext, der über 'Kontext' gesetzt wurde
    CNames *pSIds;
    CNames *pReverseSlots;      // Slots mit Kurznamen nach Nummer
    CNames *pReverseControls;   // Controls mit Kurznamen nach Nummer
    CNames *pReverseControlsSon;// Controls mit Kurznamen nach Nummer nach Fenstern (Son)
    CNames *pReverseUIds;       // Langnamen nach Nummer


    USHORT nMyVar;              // Wievielte Var aus Pool ist dran
    USHORT nControlsObj;

    void InitTestToolObj();
    CommunicationManager *pCommunicationManager;
    void SendViaSocket();

    BOOL Load( String aName, SbModule *pMod );

    void ReadNames( String Filename, CNames *&pNames, CNames *&pUIds, BOOL bIsFlat = FALSE );
    void ReadFlat( String Filename, CNames *&pNames, BOOL bSortByName );
    BOOL ReadNamesBin( String Filename, CNames *&pSIds, CNames *&pControls );
    BOOL WriteNamesBin( String Filename, CNames *pSIds, CNames *pControls );
    void ReadHidLstByNumber();
    void SortControlsByNumber( BOOL bIncludeActive = FALSE );

    String GetMethodName( ULONG nMethodId );
    String GetKeyName( USHORT nKeyCode );

    void WaitForAnswer ();
    DECL_LINK( IdleHdl,   Application* );
    DECL_LINK( CallDialogHandler,   Application* );
    String aDialogHandlerName;
    USHORT nWindowHandlerCallLevel;

    USHORT nIdleCount;
    // wenn DialogHandler gesetzt wird er im IdleHandler inkrementiert und
    // in WaitForAnswer rückgesetzt. Übersteigt er einen gewissen wert, gehe ich davon aus,
    // daß WaitForAnswer still ligt und rufe die DialogHander Sub im BASIC auf.

    void BeginBlock();
    void EndBlock();

    SbTextType GetSymbolType( const String &rSymbol, BOOL bWasControl );
    static ControlDefLoad const arR_Cmds[];
    static CNames *pRCommands;

};

#endif
