/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: runtime.hxx,v $
 *
 *  $Revision: 1.35 $
 *
 *  last change: $Author: vg $ $Date: 2007-08-30 10:01:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SBRUNTIME_HXX
#define _SBRUNTIME_HXX

#ifndef _SBX_HXX
#include <basic/sbx.hxx>
#endif

#include "sb.hxx"

// Define activates class UCBStream in iosys.cxx
#define _USE_UNO

#ifdef _USE_UNO
#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#include <rtl/math.hxx>
#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif

#include <vector>
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;


// Define activates old file implementation
// (only in non UCB case)
// #define _OLD_FILE_IMPL


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

enum ForType
{
    FOR_TO,
    FOR_EACH_ARRAY,
    FOR_EACH_COLLECTION,
    FOR_EACH_XENUMERATION
};

struct SbiForStack {                // for/next stack:
    SbiForStack*    pNext;          // Chain
    SbxVariableRef  refVar;         // loop variable
    SbxVariableRef  refEnd;         // end expression / for each: Array/BasicCollection object
    SbxVariableRef  refInc;         // increment expression

    // For each support
    ForType         eForType;
    INT32           nCurCollectionIndex;
    INT32*          pArrayCurIndices;
    INT32*          pArrayLowerBounds;
    INT32*          pArrayUpperBounds;
    Reference< XEnumeration > xEnumeration;

    SbiForStack( void )
        : pArrayCurIndices( NULL )
        , pArrayLowerBounds( NULL )
        , pArrayUpperBounds( NULL )
    {}
    ~SbiForStack()
    {
        delete[] pArrayCurIndices;
        delete[] pArrayLowerBounds;
        delete[] pArrayUpperBounds;
    }
};

struct SbiGosubStack {              // GOSUB-Stack:
    SbiGosubStack* pNext;           // Chain
    const BYTE* pCode;              // Return-Pointer
    USHORT nStartForLvl;            // #118235: For Level in moment of gosub
};

#define MAXRECURSION 500            // max. 500 Rekursionen

#define Sb_ATTR_NORMAL      0x0000
#define Sb_ATTR_HIDDEN      0x0002
#define Sb_ATTR_SYSTEM      0x0004
#define Sb_ATTR_VOLUME      0x0008
#define Sb_ATTR_DIRECTORY   0x0010
#define Sb_ATTR_ARCHIVE     0x0020


class Dir;
class WildCard;

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

    String sFullNameToBeChecked;
    WildCard* pWildCard;

#ifdef _USE_UNO
    Sequence< OUString > aDirSeq;
#endif /* _USE_UNO */

    SbiRTLData();
    ~SbiRTLData();
};

// Die Instanz entspricht einem laufenden StarBASIC. Mehrere gleichzeitig
// laufende BASICs werden ueber verkettete Instanzen verwaltet. Hier liegen
// alle Daten, die nur leben, wenn BASIC auch lebt, wie z.B. das I/O-System.

typedef ::std::vector
<
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
>
ComponentVector_t;


class SbiInstance
{
    friend class SbiRuntime;

    SbiRTLData      aRTLData;

    SbiIoSystem*    pIosys;         // Dateisystem
    SbiDdeControl*  pDdeCtrl;       // DDE
    SbiDllMgr*      pDllMgr;        // DLL-Calls (DECLARE)
    StarBASIC*      pBasic;
    SvNumberFormatter* pNumberFormatter;
    LanguageType    meFormatterLangType;
    DateFormat      meFormatterDateFormat;
    sal_uInt32      nStdDateIdx, nStdTimeIdx, nStdDateTimeIdx;

    SbError         nErr;           // aktueller Fehlercode
    String          aErrorMsg;      // letzte Error-Message fuer $ARG
    USHORT          nErl;           // aktuelle Fehlerzeile
    BOOL            bReschedule;    // Flag: TRUE = Reschedule in Hauptschleife
    BOOL            bCompatibility; // Flag: TRUE = VBA runtime compatibility mode

    ComponentVector_t ComponentVector;

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
    void    EnableCompatibility( BOOL bEnable ) { bCompatibility = bEnable; }
    BOOL    IsCompatibility( void ) { return bCompatibility; }

    ComponentVector_t& getComponentVector( void )  { return ComponentVector; }

    SbMethod* GetCaller( USHORT );
    SbModule* GetActiveModule();
    SbxArray* GetLocals( SbMethod* );

    SbiIoSystem* GetIoSystem() { return pIosys; }
    SbiDdeControl* GetDdeControl() { return pDdeCtrl; }
    StarBASIC* GetBasic( void ) { return pBasic; }
    SbiDllMgr* GetDllMgr();
    SbiRTLData* GetRTLData() const { return (SbiRTLData*)&aRTLData; }

    SvNumberFormatter* GetNumberFormatter();
    sal_uInt32 GetStdDateIdx() const { return nStdDateIdx; }
    sal_uInt32 GetStdTimeIdx() const { return nStdTimeIdx; }
    sal_uInt32 GetStdDateTimeIdx() const { return nStdDateTimeIdx; }

    // #39629# NumberFormatter auch statisch anbieten
    static void PrepareNumberFormatter( SvNumberFormatter*& rpNumberFormatter,
        sal_uInt32 &rnStdDateIdx, sal_uInt32 &rnStdTimeIdx, sal_uInt32 &rnStdDateTimeIdx,
        LanguageType* peFormatterLangType=NULL, DateFormat* peFormatterDateFormat=NULL );
};

SbiIoSystem* SbGetIoSystem();       // das aktuelle I/O-System


// Verkettbare Items, um Referenzen temporaer zu halten
struct RefSaveItem
{
    SbxVariableRef xRef;
    RefSaveItem* pNext;

    RefSaveItem() { pNext = NULL; }
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
    typedef void( SbiRuntime::*pStep1 )( UINT32 nOp1 );
    typedef void( SbiRuntime::*pStep2 )( UINT32 nOp1, UINT32 nOp2 );
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
    USHORT        nForLvl;          // #118235: Maintain for level
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
    BOOL          bBlocked;         // TRUE: blocked by next call level, #i48868
    BOOL          bVBAEnabled;
    USHORT        nFlags;           // Debugging-Flags
    SbError       nError;           // letzter Fehler
    USHORT        nOps;             // Opcode-Zaehler
    sal_uInt32    m_nLastTime;

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
    ( SbxObject* pObj, UINT32 nOp1, UINT32 nOp2, SbError, BOOL );
    void SetupArgs( SbxVariable*, UINT32 );
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
    void PushForEach();             // For-Each-Element push
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

    // #115829
    bool implIsClass( SbxObject* pObj, const String& aClass );

    void StepSETCLASS_impl( UINT32 nOp1, bool bHandleDflt = false );

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
    void StepGET(),     StepSET(),  StepVBASET(),   StepPUT(),      StepPUTC();
    void StepSET_Impl( SbxVariableRef& refVal, SbxVariableRef& refVar, bool bDefaultHandling = false );
    void StepDIM(),     StepREDIM(),    StepREDIMP(),   StepERASE();
    void StepINITFOR(), StepNEXT(),     StepERROR(),    StepINITFOREACH();
    void StepCASE(),    StepENDCASE(),  StepSTDERROR();
    void StepNOERROR(), StepCHANNEL(),  StepCHANNEL0(), StepPRINT();
    void StepPRINTF(),  StepWRITE(),    StepRENAME(),   StepPROMPT();
    void StepRESTART(), StepEMPTY(),    StepLEAVE();
    void StepLSET(),    StepRSET(),     StepREDIMP_ERASE(),     StepERASE_CLEAR();
    // Alle Opcodes mit einem Operanden
    void StepLOADNC( UINT32 ),  StepLOADSC( UINT32 ),   StepLOADI( UINT32 );
    void StepARGN( UINT32 ),    StepBASED( UINT32 ),    StepPAD( UINT32 );
    void StepJUMP( UINT32 ),    StepJUMPT( UINT32 );
    void StepJUMPF( UINT32 ),   StepONJUMP( UINT32 );
    void StepGOSUB( UINT32 ),   StepRETURN( UINT32 );
    void StepTESTFOR( UINT32 ), StepCASETO( UINT32 ),   StepERRHDL( UINT32 );
    void StepRESUME( UINT32 ),  StepSETCLASS( UINT32 ), StepVBASETCLASS( UINT32 ),  StepTESTCLASS( UINT32 ), StepLIB( UINT32 );
    bool checkClass_Impl( const SbxVariableRef& refVal, const String& aClass, bool bRaiseErrors );
    void StepCLOSE( UINT32 ),   StepPRCHAR( UINT32 ),   StepARGTYP( UINT32 );
    // Alle Opcodes mit zwei Operanden
    void StepRTL( UINT32, UINT32 ),     StepPUBLIC( UINT32, UINT32 ),   StepPUBLIC_P( UINT32, UINT32 );
    void StepPUBLIC_Impl( UINT32, UINT32, bool bUsedForClassModule );
    void StepFIND( UINT32, UINT32 ),    StepELEM( UINT32, UINT32 );
    void StepGLOBAL( UINT32, UINT32 ),  StepLOCAL( UINT32, UINT32 );
    void StepPARAM( UINT32, UINT32),    StepCREATE( UINT32, UINT32 );
    void StepCALL( UINT32, UINT32 ),    StepCALLC( UINT32, UINT32 );
    void StepCASEIS( UINT32, UINT32 ),  StepSTMNT( UINT32, UINT32 );
    void StepOPEN( UINT32, UINT32 ),    StepSTATIC( UINT32, UINT32 );
    void StepTCREATE(UINT32,UINT32),    StepDCREATE(UINT32,UINT32);
    void StepGLOBAL_P( UINT32, UINT32 ),StepFIND_G( UINT32, UINT32 );
    void StepDCREATE_REDIMP(UINT32,UINT32), StepDCREATE_IMPL(UINT32,UINT32);
    void StepFIND_CM( UINT32, UINT32 );
public:
    void          SetVBAEnabled( bool bEnabled ) { bVBAEnabled = bEnabled; };
    USHORT      GetImageFlag( USHORT n ) const;
    USHORT      GetBase();
    xub_StrLen  nLine,nCol1,nCol2;  // aktuelle Zeile, Spaltenbereich
    SbiRuntime* pNext;               // Stack-Chain

    SbiRuntime( SbModule*, SbMethod*, UINT32 );
   ~SbiRuntime();
    void Error( SbError );          // Fehler setzen, falls != 0
    void FatalError( SbError );     // Fehlerbehandlung=Standard, Fehler setzen
    void DumpPCode();
    BOOL Step();                    // Einzelschritt (ein Opcode)
    void Stop()            { bRun = FALSE;   }
    BOOL IsRun()           { return bRun;    }
    void block( void )     { bBlocked = TRUE; }
    void unblock( void )   { bBlocked = FALSE; }
    SbMethod* GetMethod()  { return pMeth;   }
    SbModule* GetModule()  { return pMod;    }
    USHORT GetDebugFlags() { return nFlags;  }
    void SetDebugFlags( USHORT nFl ) { nFlags = nFl;  }
    SbMethod* GetCaller();
    SbxArray* GetLocals();
    SbxArray* GetParams();

    SbxBase* FindElementExtern( const String& rName );
    static bool isVBAEnabled();

};

inline void checkArithmeticOverflow( double d )
{
    if( !::rtl::math::isFinite( d ) )
        StarBASIC::Error( SbERR_MATH_OVERFLOW );
}

inline void checkArithmeticOverflow( SbxVariable* pVar )
{
    if( pVar->GetType() == SbxDOUBLE )
    {
        double d = pVar->GetDouble();
        checkArithmeticOverflow( d );
    }
}

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
// #87427 OSL need File URLs, so map to getFullPath
inline String getFullPathUNC( const String& aRelPath )
{
    return getFullPath( aRelPath );
}
void implStepRenameOSL( const String& aSource, const String& aDest );

#endif
