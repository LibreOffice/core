/*************************************************************************
 *
 *  $RCSfile: runtime.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ab $ $Date: 2000-10-18 08:58:43 $
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

#ifndef _SBRUNTIME_HXX
#define _SBRUNTIME_HXX

#include "sb.hxx"

// Define activates class UCBStream in iosys.cxx
#define _USE_UNO

#ifdef _USE_UNO
#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

using namespace rtl;
using namespace com::sun::star::uno;


// Define activates old file implementation
// (only in non UCB case)
#define _OLD_FILE_IMPL


//#include <sal/types.h>
//#include <rtl/byteseq.hxx>
//#include <rtl/ustring>


namespace basicEncoder
{

// TODO: Use exported functionality (code is copied from deamons2/ucb)
class AsciiEncoder
{
public:
    static ::rtl::OUString decodeUnoUrlParamValue(const rtl::OUString & rSource);
    //static ::rtl::OUString encodeUnoUrlParamValue(const rtl::OUString & rSource);
    //static ::rtl::ByteSequence decode(const ::rtl::OUString & string);
    //static ::rtl::OUString encode(const ::rtl::ByteSequence & bytes);
    //static void test();
};

}

#endif /* _USE_UNO */

class SbiInstance;                  // aktiver StarBASIC-Prozess
class SbiRuntime;                   // aktive StarBASIC-Prozedur-Instanz

struct SbiArgvStack;                // Argv stack element
struct SbiGosubStack;               // GOSUB stack element
class  SbiImage;                    // Code-Image
class  SbiIoSystem;                 // Dateisystem
class  SbiDdeControl;               // DDE-Steuerung
class  SbiDllMgr;                   // Aufrufe in DLLs
class  SvNumberFormatter;           // Zeit/Datumsfunktionen

struct SbiForStack {                // for/next stack:
    SbiForStack*   pNext;           // Chain
    SbxVariableRef refVar;          // loop variable
    SbxVariableRef refEnd;          // end expression
    SbxVariableRef refInc;          // increment expression
};

#define MAXRECURSION 500            // max. 500 Rekursionen

#define Sb_ATTR_NORMAL      0x0000
#define Sb_ATTR_HIDDEN      0x0002
#define Sb_ATTR_SYSTEM      0x0004
#define Sb_ATTR_VOLUME      0x0008
#define Sb_ATTR_DIRECTORY   0x0010
#define Sb_ATTR_ARCHIVE     0x0020


class Dir;

class SbiRTLData
{
public:

#ifdef _OLD_FILE_IMPL
    Dir*    pDir;
#else
    ::osl::Directory* pDir;
#endif
    INT16   nDirFlags;
    short   nCurDirPos;

#ifdef _USE_UNO
    Sequence< OUString > aDirSeq;
#endif /* _USE_UNO */

    SbiRTLData();
    ~SbiRTLData();
};

// Die Instanz entspricht einem laufenden StarBASIC. Mehrere gleichzeitig
// laufende BASICs werden ueber verkettete Instanzen verwaltet. Hier liegen
// alle Daten, die nur leben, wenn BASIC auch lebt, wie z.B. das I/O-System.

class SbiInstance
{
    friend class SbiRuntime;

    SbiRTLData      aRTLData;

    SbiIoSystem*    pIosys;         // Dateisystem
    SbiDdeControl*  pDdeCtrl;       // DDE
    SbiDllMgr*      pDllMgr;        // DLL-Calls (DECLARE)
    StarBASIC*      pBasic;
    SvNumberFormatter* pNumberFormatter;
    ULONG           nStdDateIdx, nStdTimeIdx, nStdDateTimeIdx;

    SbError         nErr;           // aktueller Fehlercode
    String          aErrorMsg;      // letzte Error-Message fuer $ARG
    USHORT          nErl;           // aktuelle Fehlerzeile
    BOOL            bReschedule;    // Flag: TRUE = Reschedule in Hauptschleife

public:
    SbiRuntime*  pRun;              // Call-Stack
    SbiInstance* pNext;             // Instanzen-Chain

    // #31460 Neues Konzept fuer StepInto/Over/Out,
    // Erklaerung siehe runtime.cxx bei SbiInstance::CalcBreakCallLevel()
    USHORT  nCallLvl;               // Call-Level (wg. Rekursion)
    USHORT  nBreakCallLvl;          // Call-Level zum Anhalten
    void    CalcBreakCallLevel( USHORT nFlags );    // Gemaess Flags setzen

    SbiInstance( StarBASIC* );
   ~SbiInstance();

    void Error( SbError );          // trappable Error
    void Error( SbError, const String& rMsg );  // trappable Error mit Message
    void FatalError( SbError );     // non-trappable Error
    void Abort();                   // Abbruch mit aktuellem Fehlercode

    void    Stop();
    SbError GetErr()                { return nErr; }
    String  GetErrorMsg()           { return aErrorMsg; }
    xub_StrLen GetErl()             { return nErl; }
    void    EnableReschedule( BOOL bEnable ) { bReschedule = bEnable; }
    BOOL    IsReschedule( void ) { return bReschedule; }

    SbMethod* GetCaller( USHORT );
    SbModule* GetActiveModule();
    SbxArray* GetLocals( SbMethod* );

    SbiIoSystem* GetIoSystem() { return pIosys; }
    SbiDdeControl* GetDdeControl() { return pDdeCtrl; }
    SbiDllMgr* GetDllMgr();
    SbiRTLData* GetRTLData() const { return (SbiRTLData*)&aRTLData; }

    SvNumberFormatter* GetNumberFormatter();
    ULONG GetStdDateIdx() const { return nStdDateIdx; }
    ULONG GetStdTimeIdx() const { return nStdTimeIdx; }
    ULONG GetStdDateTimeIdx() const { return nStdDateTimeIdx; }

    // #39629# NumberFormatter auch statisch anbieten
    static void PrepareNumberFormatter( SvNumberFormatter*& rpNumberFormatter,
        ULONG &rnStdDateIdx, ULONG &rnStdTimeIdx, ULONG &rnStdDateTimeIdx );
};

SbiIoSystem* SbGetIoSystem();       // das aktuelle I/O-System


// Verkettbare Items, um Referenzen temporaer zu halten
struct RefSaveItem
{
    SbxVariableRef xRef;
    RefSaveItem* pNext;

    RefSaveItem() { pNext = NULL; }
};

// #72488 Spezielle SbxVariable, die beim get das Verhalten
// einer nicht initialisierten Variable simuliert. Wenn als
// Typ SbxOBJECT verlangt wird, geht das jedoch nicht.
class UnoClassSbxVariable : public SbxVariable
{
    SbxDataType meOrgType;
    BOOL mbOverWritten;
    const SbiImage* mpImg;
    SbiRuntime* mpRuntime;

public:
    UnoClassSbxVariable( SbxDataType eType, const SbiImage* pImg_, SbiRuntime* pRuntime_ )
        : SbxVariable( SbxVARIANT ), mpImg( pImg_ ), mpRuntime( pRuntime_ )
    {
        meOrgType = eType;
        mbOverWritten = FALSE;
    }
    UnoClassSbxVariable( const UnoClassSbxVariable& r )
        : SbxVariable( r ), meOrgType( r.meOrgType), mbOverWritten( r.mbOverWritten),
          mpImg( r.mpImg ), mpRuntime( r.mpRuntime )
    {}

    virtual BOOL Get( SbxValues& ) const;
    virtual BOOL Put( const SbxValues& );

    TYPEINFO();
};

// #72732 Spezielle SbxVariable, die beim put/get prueft,
// ob der Kontext fuer eine UnoClass sinnvoll ist. Sonst
// liegt eventuell ein Schreibfehler im Basic-Source vor.
class UnoClassMemberVariable : public SbxVariable
{
    SbiRuntime* mpRuntime;
    BOOL bInternalUse;

public:
    UnoClassMemberVariable( SbiRuntime* pRuntime_, const SbxObjectRef& xWrapper )
        : SbxVariable( SbxVARIANT ), mpRuntime( pRuntime_ )
    {
        bInternalUse = TRUE;
        PutObject( xWrapper );
        bInternalUse = FALSE;
    }

    virtual BOOL Get( SbxValues& ) const;
    virtual BOOL Put( const SbxValues& );

    TYPEINFO();
};


// Eine Instanz dieser Klasse wird fuer jedes ausgefuehrte Unterprogramm
// aufgesetzt. Diese Instanz ist das Herz der BASIC-Maschine und enthaelt
// nur lokale Daten.

class SbiRuntime
{
    typedef void( SbiRuntime::*pStep0 )();
    typedef void( SbiRuntime::*pStep1 )( USHORT nOp1 );
    typedef void( SbiRuntime::*pStep2 )( USHORT nOp1, USHORT nOp2 );
    static pStep0 aStep0[];         // Opcode-Tabelle Gruppe 0
    static pStep1 aStep1[];         // Opcode-Tabelle Gruppe 1
    static pStep2 aStep2[];         // Opcode-Tabelle Gruppe 2

    StarBASIC&    rBasic;           // StarBASIC-Instanz
    SbiInstance*   pInst;           // aktiver Thread
    SbModule*     pMod;             // aktuelles Modul
    SbMethod*     pMeth;            // Methoden-Instanz
    SbiIoSystem*   pIosys;          // I/O-System
    const SbiImage* pImg;           // Code-Image
    SbxArrayRef   refExprStk;       // expression stack
    SbxArrayRef   refCaseStk;       // CASE expression stack
    SbxArrayRef   refRedimpArray;   // Array saved to use for REDIM PRESERVE
    SbxVariableRef xDummyVar;       // Ersatz fuer nicht gefundene Variablen
    SbiArgvStack*  pArgvStk;        // ARGV-Stack
    SbiGosubStack* pGosubStk;       // GOSUB stack
    SbiForStack*   pForStk;         // FOR/NEXT-Stack
    USHORT        nExprLvl;         // Tiefe des Expr-Stacks
    USHORT        nGosubLvl;        // Zum Vermeiden von Tot-Rekursionen
    const BYTE*   pCode;            // aktueller Code-Pointer
    const BYTE*   pStmnt;           // Beginn des lezten Statements
    const BYTE*   pError;           // Adresse des aktuellen Error-Handlers
    const BYTE*   pRestart;         // Restart-Adresse
    const BYTE*   pErrCode;         // Restart-Adresse RESUME NEXT
    const BYTE*   pErrStmnt;        // Restart-Adresse RESUMT 0
    String        aLibName;         // Lib-Name fuer Declare-Call
    SbxArrayRef   refParams;        // aktuelle Prozedur-Parameter
    SbxArrayRef   refLocals;        // lokale Variable
    SbxArrayRef   refArgv;          // aktueller Argv
    // AB, 28.3.2000 #74254, Ein refSaveObj reicht nicht! Neu: pRefSaveList (s.u.)
    //SbxVariableRef refSaveObj;      // #56368 Bei StepElem Referenz sichern
    short         nArgc;            // aktueller Argc
    BOOL          bRun;             // TRUE: Programm ist aktiv
    BOOL          bError;           // TRUE: Fehler behandeln
    BOOL          bInError;         // TRUE: in einem Fehler-Handler
    USHORT        nFlags;           // Debugging-Flags
    SbError       nError;           // letzter Fehler
    USHORT        nOps;             // Opcode-Zaehler

    RefSaveItem*  pRefSaveList;     // #74254 Temporaere Referenzen sichern
    RefSaveItem*  pItemStoreList;   // Unbenutzte Items aufbewahren
    void SaveRef( SbxVariable* pVar )
    {
        RefSaveItem* pItem = pItemStoreList;
        if( pItem )
            pItemStoreList = pItem->pNext;
        else
            pItem = new RefSaveItem();
        pItem->pNext = pRefSaveList;
        pItem->xRef = pVar;
        pRefSaveList = pItem;
    }
    void ClearRefs( void )
    {
        while( pRefSaveList )
        {
            RefSaveItem* pToClearItem = pRefSaveList;
            pRefSaveList = pToClearItem->pNext;
            pToClearItem->xRef = NULL;
            pToClearItem->pNext = pItemStoreList;
            pItemStoreList = pToClearItem;
        }
    }

    SbxVariable* FindElement
    ( SbxObject* pObj, USHORT nOp1, USHORT nOp2, SbError, BOOL );
    void SetupArgs( SbxVariable*, USHORT );
    SbxVariable* CheckArray( SbxVariable* );

    void PushVar( SbxVariable* );   // Variable push
    SbxVariableRef PopVar();          // Variable pop
    SbxVariable* GetTOS( short=0 ); // Variable vom TOS holen
    void TOSMakeTemp();             // TOS in temp. Variable wandeln
    BOOL ClearExprStack();          // Expr-Stack freigeben

    void PushGosub( const BYTE* );  // GOSUB-Element push
    void PopGosub();                // GOSUB-Element pop
    void ClearGosubStack();         // GOSUB-Stack freigeben

    void PushArgv();                // Argv-Element push
    void PopArgv();                 // Argv-Element pop
    void ClearArgvStack();          // Argv-Stack freigeben

    void PushFor();                 // For-Element push
    void PopFor();                  // For-Element pop
    void ClearForStack();           // For-Stack freigeben

    void StepArith( SbxOperator );  // arithmetische Verknuepfungen
    void StepUnary( SbxOperator );  // unaere Verknuepfungen
    void StepCompare( SbxOperator );// Vergleiche

    void SetParameters( SbxArray* );// Parameter uebernehmen

    // MUSS NOCH IMPLEMENTIERT WERDEN
    void DllCall( const String&, const String&, SbxArray*, SbxDataType, BOOL );

    // #56204 DIM-Funktionalitaet in Hilfsmethode auslagern (step0.cxx)
    void DimImpl( SbxVariableRef refVar );

    // Die nachfolgenden Routinen werden vom Single Stepper
    // gerufen und implementieren die einzelnen Opcodes
    void StepNOP(),     StepEXP(),      StepMUL(),      StepDIV();
    void StepMOD(),     StepPLUS(),     StepMINUS(),    StepNEG();
    void StepEQ(),      StepNE(),       StepLT(),       StepGT();
    void StepLE(),      StepGE(),       StepIDIV(),     StepAND();
    void StepOR(),      StepXOR(),      StepEQV(),      StepIMP();
    void StepNOT(),     StepCAT(),      StepLIKE(),     StepIS();
    void StepCLONE(),   StepOLDBASED(), StepARGC();
    void StepARGV(),    StepINPUT(),    StepLINPUT(),   StepSTOP();
    void StepGET(),     StepSET(),      StepPUT(),      StepPUTC();
    void StepDIM(),     StepREDIM(),    StepREDIMP(),   StepERASE();
    void StepINITFOR(), StepNEXT(),     StepERROR();
    void StepCASE(),    StepENDCASE(),  StepSTDERROR();
    void StepNOERROR(), StepCHANNEL(),  StepCHANNEL0(), StepPRINT();
    void StepPRINTF(),  StepWRITE(),    StepRENAME(),   StepPROMPT();
    void StepRESTART(), StepEMPTY(),    StepLEAVE();
    void StepLSET(),    StepRSET(),     StepREDIMP_ERASE();
    // Alle Opcodes mit einem Operanden
    void StepLOADNC( USHORT ),  StepLOADSC( USHORT ),   StepLOADI( USHORT );
    void StepARGN( USHORT ),    StepBASED( USHORT ),    StepPAD( USHORT );
    void StepJUMP( USHORT ),    StepJUMPT( USHORT );
    void StepJUMPF( USHORT ),   StepONJUMP( USHORT );
    void StepGOSUB( USHORT ),   StepRETURN( USHORT );
    void StepTESTFOR( USHORT ), StepCASETO( USHORT ),   StepERRHDL( USHORT );
    void StepRESUME( USHORT ),  StepCLASS( USHORT ),    StepLIB( USHORT );
    void StepCLOSE( USHORT ),   StepPRCHAR( USHORT ),   StepARGTYP( USHORT );
    // Alle Opcodes mit zwei Operanden
    void StepRTL( USHORT, USHORT ),     StepPUBLIC( USHORT, USHORT );
    void StepFIND( USHORT, USHORT ),    StepELEM( USHORT, USHORT );
    void StepGLOBAL( USHORT, USHORT ),  StepLOCAL( USHORT, USHORT );
    void StepPARAM( USHORT, USHORT),    StepCREATE( USHORT, USHORT );
    void StepCALL( USHORT, USHORT ),    StepCALLC( USHORT, USHORT );
    void StepCASEIS( USHORT, USHORT ),  StepSTMNT( USHORT, USHORT );
    void StepOPEN( USHORT, USHORT ),    StepSTATIC( USHORT, USHORT );
    void StepTCREATE(USHORT,USHORT),    StepDCREATE(USHORT,USHORT);
public:
    xub_StrLen  nLine,nCol1,nCol2;  // aktuelle Zeile, Spaltenbereich
    SbiRuntime* pNext;               // Stack-Chain

    SbiRuntime( SbModule*, SbMethod*, USHORT );
   ~SbiRuntime();
    void Error( SbError );          // Fehler setzen, falls != 0
    void FatalError( SbError );     // Fehlerbehandlung=Standard, Fehler setzen
    BOOL Step();                    // Einzelschritt (ein Opcode)
    void Stop()            { bRun = FALSE;   }
    SbMethod* GetMethod()  { return pMeth;   }
    SbModule* GetModule()  { return pMod;    }
    USHORT GetDebugFlags() { return nFlags;  }
    void SetDebugFlags( USHORT nFl ) { nFlags = nFl;  }
    SbMethod* GetCaller();
    SbxArray* GetLocals();
    SbxArray* GetParams();

    SbxBase* FindElementExtern( const String& rName );
};

// Hilfsfunktion, um aktives Basic zu finden
StarBASIC* GetCurrentBasic( StarBASIC* pRTBasic );

// Get information if security restrictions should be
// used (File IO based on UCB, no RTL function SHELL
// no DDE functionality, no DLLCALL) in basic because
// of portal "virtual" users (portal user != UNIX user)
// (Implemented in iosys.cxx)
BOOL needSecurityRestrictions( void );

// Returns TRUE if UNO is available, otherwise the old
// file system implementation has to be used
// (Implemented in iosys.cxx)
BOOL hasUno( void );

// Converts possibly relative paths to absolute paths
// according to the setting done by ChDir/ChDrive
// (Implemented in methods.cxx)
String getFullPath( const String& aRelPath );

// Sets (virtual) current path for UCB file access
void implChDir( const String& aDir );

// Sets (virtual) current drive for UCB file access
void implChDrive( const String& aDrive );

// Returns (virtual) current path for UCB file access
String implGetCurDir( void );

// Implementation of StepRENAME with UCB
// (Implemented in methods.cxx, so step0.cxx
// has not to be infected with UNO)
void implStepRenameUCB( const String& aSource, const String& aDest );

//*** OSL file access ***
String getFullPathUNC( const String& aRelPath );
void implStepRenameOSL( const String& aSource, const String& aDest );

#endif
