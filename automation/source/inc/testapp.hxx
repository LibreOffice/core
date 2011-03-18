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

#ifndef TESTAPP_HXX
#define TESTAPP_HXX

#include <basic/sbmod.hxx>
#include <basic/testtool.hxx>

class CommunicationLink;
class CommunicationManagerClientViaSocketTT;
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
    ErrorEntry(sal_uLong nNr, String aStr = String()) : nError(nNr),aText(aStr),nLine(0),nCol1(0),nCol2(0) {}
    ErrorEntry(sal_uLong nNr, String aStr, xub_StrLen l, xub_StrLen c1, xub_StrLen c2 )
        : nError(nNr),aText(aStr),nLine(l),nCol1(c1),nCol2(c2) {}
    sal_uLong nError;
    String aText;
    xub_StrLen nLine;
    xub_StrLen nCol1;
    xub_StrLen nCol2;
};

SV_DECL_PTRARR_DEL(CErrors, ErrorEntry*, 1, 1)

struct ControlDefLoad {
    const char* Kurzname;
    sal_uLong nUId;
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
    void DebugFindNoErrors( sal_Bool bDebugFindNoErrors );

private:
    sal_Bool bWasPrecompilerError;  // True wenn beim letzten Precompile ein Fehler auftrat
    sal_Bool CError( sal_uLong, const String&, xub_StrLen, xub_StrLen, xub_StrLen );
    void CalcPosition( String const &aSource, xub_StrLen nPos, xub_StrLen &l, xub_StrLen &c );
    xub_StrLen ImplSearch( const String &aSource, const xub_StrLen nStart, const xub_StrLen nEnd, const String &aSearch, const xub_StrLen nSearchStart = 0 );
    xub_StrLen PreCompilePart( String &aSource, xub_StrLen nStart, xub_StrLen nEnd, String aFinalErrorLabel, sal_uInt16 &nLabelCount );
    void PreCompileDispatchParts( String &aSource, String aStart, String aEnd, String aFinalLable );
public:
    String GetRevision(String const &aSourceIn);    // find Revision in the sourcecode
    String PreCompile(String const &aSourceIn); // try catch; testcase endcase ..
    sal_Bool WasPrecompilerError(); // True wenn beim letzten Precompile ein Fehler auftrat
    void            SFX_NOTIFY( SfxBroadcaster&, const TypeId&, const SfxHint& rHint, const TypeId& );
    virtual SbxVariable* Find( const String&, SbxClassType );
//  String aKeyPlusClasses;     // Pfad für keycodes & classes & res_type (Aus Configdatei)
    DECL_LINK( ReturnResultsLink, CommunicationLink* );
    sal_Bool            ReturnResults( SvStream *pIn ); // Rücklieferung des Antwortstreams über IPC oder TCP/IP oder direkt

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
    static const CErrors* GetFehlerListe() { return pFehlerListe; }
    sal_Bool bUseIPC;
    Link aLogHdl;               // Zum Logen der Fehlermeldungen im Testtool
    Link aWinInfoHdl;           // Anzeigen der Windows/Controls der zu testenden App
    Link aModuleWinExistsHdl;   // Prüft ob das Modul schon im Editor geladen ist
    Link aCErrorHdl;            // Melden von Compilererror
    Link aWriteStringHdl;       // Schreiben von text (e.g. MakroRecorder)
    sal_Bool bReturnOK;             // Bricht WaitForAnswer ab
    CRevNames *pShortNames;     // Aktuell verwendete Controls, zur gewinnung des Namens aus Fehlermeldung
    sal_uLong nSequence;            // Sequence um Antwort und Anfrage zu syncronisieren
    rtl::OString aNextReturnId; // Id des Returnwertes i.e. UId
    void ReplaceNumbers(String &aText); // Zahlen im String mit speziellem Format in Namen umwandeln

    String aLastRecordedKontext;//  Keeps the last kontext recorded by the Macro Recorder

#define FLAT sal_True
    String ProgPath;            // Dateiname der zu Testenden APP; Gesetzt über Start
    String aLogFileName;        // Momentaner Logfilename (Wie Programmdatei aber mit .res)
    sal_Bool IsBlock;               // Innerhalb Begin/EndBlock
    sal_Bool SingleCommandBlock;    // Implizit um jedes kommando ein Begin/EndBlock
    CmdStream *In;

    void AddName(String &aBisher, String &aNeu );   // Name eventuell mit / anhängen
    void AddToListByNr( CNames *&pControls, ControlItemUId *&pNewItem );    //
    CNames *m_pControls;
    CNames *m_pNameKontext;     // Zeigt auf den aktuellen Namenskontext, der über 'Kontext' gesetzt wurde
    CNames *m_pSIds;
    CNames *m_pReverseSlots;        // Slots mit Kurznamen nach Nummer
    CNames *m_pReverseControls; // Controls mit Kurznamen nach Nummer
    CNames *m_pReverseControlsSon;// Controls mit Kurznamen nach Nummer nach Fenstern (Son)
    CNames *m_pReverseUIds;     // Langnamen nach Nummer


    sal_uInt16 nMyVar;              // Wievielte Var aus Pool ist dran

    void InitTestToolObj();
    CommunicationManagerClientViaSocketTT *pCommunicationManager;
    void SendViaSocket();

    sal_Bool Load( String aFileName, SbModule *pMod );

    void ReadNames( String Filename, CNames *&pNames, CNames *&pUIds, sal_Bool bIsFlat = sal_False );
    void ReadFlat( String Filename, CNames *&pNames, sal_Bool bSortByName );
    sal_Bool ReadNamesBin( String Filename, CNames *&pSIds, CNames *&pControls );
    sal_Bool WriteNamesBin( String Filename, CNames *pSIds, CNames *pControls );
    void ReadHidLstByNumber();
    void SortControlsByNumber( sal_Bool bIncludeActive = sal_False );

    String GetMethodName( sal_uLong nMethodId );
    String GetKeyName( sal_uInt16 nKeyCode );

    void WaitForAnswer ();
    DECL_LINK( IdleHdl,   Application* );
    DECL_LINK( CallDialogHandler,   Application* );
    String aDialogHandlerName;
    sal_uInt16 nWindowHandlerCallLevel;

    sal_uInt16 nIdleCount;
    // wenn DialogHandler gesetzt wird er im IdleHandler inkrementiert und
    // in WaitForAnswer rückgesetzt. Übersteigt er einen gewissen wert, gehe ich davon aus,
    // daß WaitForAnswer still ligt und rufe die DialogHander Sub im BASIC auf.

    void BeginBlock();
    void EndBlock();

    SbTextType GetSymbolType( const String &rSymbol, sal_Bool bWasControl );
    static ControlDefLoad const arR_Cmds[];
    static CNames *pRCommands;

    static CErrors *pFehlerListe;       // Hier werden die Fehler des Testtools gespeichert

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
