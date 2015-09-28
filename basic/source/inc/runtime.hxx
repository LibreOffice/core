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

#ifndef INCLUDED_BASIC_SOURCE_INC_RUNTIME_HXX
#define INCLUDED_BASIC_SOURCE_INC_RUNTIME_HXX

#include <basic/sbx.hxx>

#include "sb.hxx"

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/file.hxx>
#include <rtl/math.hxx>
#include <i18nlangtag/lang.h>

#include <vector>
#include <memory>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <unotools/localedatawrapper.hxx>

class SbiInstance;                  // active StarBASIC process
class SbiRuntime;                   // active StarBASIC procedure instance

struct SbiArgvStack;                // Argv stack element
struct SbiGosubStack;               // GOSUB stack element
class  SbiImage;                    // Code-Image
class  SbiIoSystem;
class  SbiDdeControl;
class  SbiDllMgr;
class  SvNumberFormatter;           // time/date functions
enum class SbiImageFlags;

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
    sal_Int32           nCurCollectionIndex;
    sal_Int32*          pArrayCurIndices;
    sal_Int32*          pArrayLowerBounds;
    sal_Int32*          pArrayUpperBounds;
    css::uno::Reference< css::container::XEnumeration > xEnumeration;

    SbiForStack()
        : pNext(NULL)
        , eForType(FOR_TO)
        , nCurCollectionIndex(0)
        , pArrayCurIndices(NULL)
        , pArrayLowerBounds(NULL)
        , pArrayUpperBounds(NULL)
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
    const sal_uInt8* pCode;             // Return-Pointer
    sal_uInt16 nStartForLvl;            // #118235: For Level in moment of gosub
};

#define MAXRECURSION 500

#define Sb_ATTR_READONLY    0x0001
#define Sb_ATTR_HIDDEN      0x0002
#define Sb_ATTR_DIRECTORY   0x0010


class WildCard;

class SbiRTLData
{
public:

    ::osl::Directory* pDir;
    sal_Int16   nDirFlags;
    short   nCurDirPos;

    OUString sFullNameToBeChecked;
    WildCard* pWildCard;

    css::uno::Sequence< OUString > aDirSeq;

    SbiRTLData();
    ~SbiRTLData();
};

// The instance matches a running StarBASIC. Many basics running at the same
// time are managed by chained instances. There is all the data that only lives
// when the BASIC is living too, like the I/O-system.

typedef std::vector< css::uno::Reference< css::lang::XComponent > > ComponentVector_t;


class SbiInstance
{
    friend class SbiRuntime;

    SbiRTLData      aRTLData;

    // file system
    std::unique_ptr<SbiIoSystem>     pIosys;
    // DDE
    std::unique_ptr<SbiDdeControl>    pDdeCtrl;
    // DLL-Calls (DECLARE)
    std::unique_ptr<SbiDllMgr>        pDllMgr;
    std::unique_ptr<SvNumberFormatter> pNumberFormatter;
    StarBASIC*      pBasic;
    LanguageType    meFormatterLangType;
    DateFormat      meFormatterDateFormat;
    sal_uInt32      nStdDateIdx, nStdTimeIdx, nStdDateTimeIdx;

    SbError         nErr;
    OUString        aErrorMsg;      // last error message for $ARG
    sal_Int32       nErl;           // current error line
    bool        bReschedule;    // Flag: sal_True = Reschedule in main loop
    bool        bCompatibility; // Flag: sal_True = VBA runtime compatibility mode

    ComponentVector_t ComponentVector;
public:
    SbiRuntime*  pRun;              // Call-Stack
    SbiInstance* pNext;             // instances chain

    // #31460 new concept for StepInto/Over/Out,
    // explanation see runtime.cxx at SbiInstance::CalcBreakCallLevel()
    sal_uInt16  nCallLvl;
    sal_uInt16  nBreakCallLvl;
    void    CalcBreakCallLevel( sal_uInt16 nFlags );

    SbiInstance( StarBASIC* );
   ~SbiInstance();

    void Error( SbError );                      // trappable Error
    void Error( SbError, const OUString& rMsg );  // trappable Error with message
    void ErrorVB( sal_Int32 nVBNumber, const OUString& rMsg );
    void setErrorVB( sal_Int32 nVBNumber, const OUString& rMsg );
    void FatalError( SbError );                 // non-trappable Error
    void FatalError( SbError, const OUString& );  // non-trappable Error
    void Abort();                               // with current error code

    void    Stop();
    SbError GetErr()                { return nErr; }
    OUString  GetErrorMsg()           { return aErrorMsg; }
    sal_Int32 GetErl()             { return nErl; }
    void    EnableReschedule( bool bEnable ) { bReschedule = bEnable; }
    bool    IsReschedule() { return bReschedule; }
    void    EnableCompatibility( bool bEnable ) { bCompatibility = bEnable; }
    bool    IsCompatibility() { return bCompatibility; }

    ComponentVector_t& getComponentVector()  { return ComponentVector; }

    SbMethod* GetCaller( sal_uInt16 );
    SbModule* GetActiveModule();

    SbiIoSystem* GetIoSystem() { return pIosys.get(); }
    SbiDdeControl* GetDdeControl() { return pDdeCtrl.get(); }
    StarBASIC* GetBasic() { return pBasic; }
    SbiDllMgr* GetDllMgr();
    SbiRTLData* GetRTLData() const { return const_cast<SbiRTLData*>(&aRTLData); }

    SvNumberFormatter* GetNumberFormatter();
    sal_uInt32 GetStdDateIdx() const { return nStdDateIdx; }
    sal_uInt32 GetStdTimeIdx() const { return nStdTimeIdx; }
    sal_uInt32 GetStdDateTimeIdx() const { return nStdDateTimeIdx; }

    // offer NumberFormatter also static
    static SvNumberFormatter* PrepareNumberFormatter( sal_uInt32 &rnStdDateIdx,
        sal_uInt32 &rnStdTimeIdx, sal_uInt32 &rnStdDateTimeIdx,
        LanguageType* peFormatterLangType=NULL, DateFormat* peFormatterDateFormat=NULL );
};

// chainable items to keep references temporary
struct RefSaveItem
{
    SbxVariableRef xRef;
    RefSaveItem* pNext;

    RefSaveItem() { pNext = NULL; }
};


// There's one instance of this class for every executed sub-program.
// This instance is the heart of the BASIC-machine and contains only local data.

class SbiRuntime
{
    friend void SbRtl_CallByName( StarBASIC* pBasic, SbxArray& rPar, bool bWrite );

    typedef void( SbiRuntime::*pStep0 )();
    typedef void( SbiRuntime::*pStep1 )( sal_uInt32 nOp1 );
    typedef void( SbiRuntime::*pStep2 )( sal_uInt32 nOp1, sal_uInt32 nOp2 );
    static pStep0 aStep0[];         // opcode-table group 0
    static pStep1 aStep1[];
    static pStep2 aStep2[];

    StarBASIC&    rBasic;           // StarBASIC instance
    SbiInstance*   pInst;           // current thread
    SbModule*     pMod;             // current module
    SbMethod*     pMeth;            // method instance
    SbiIoSystem*   pIosys;          // I/O-System
    const SbiImage* pImg;           // Code-Image
    SbxArrayRef   refExprStk;       // expression stack
    SbxArrayRef   refCaseStk;       // CASE expression stack
    SbxArrayRef   refRedimpArray;   // Array saved to use for REDIM PRESERVE
    SbxVariableRef   refRedim;   // Array saved to use for REDIM
    SbxVariableRef xDummyVar;       // substitute for variables that weren't found
    SbxVariable* mpExtCaller;       // Caller ( external - e.g. button name, shape, range object etc. - only in vba mode )
    SbiArgvStack*  pArgvStk;        // ARGV-Stack
    SbiGosubStack* pGosubStk;       // GOSUB stack
    SbiForStack*   pForStk;         // FOR/NEXT-Stack
    sal_uInt16        nExprLvl;         // depth of the expr-stack
    sal_uInt16        nGosubLvl;        // to prevent dead-recursions
    sal_uInt16        nForLvl;          // #118235: Maintain for level
    const sal_uInt8*   pCode;            // current Code-Pointer
    const sal_uInt8*   pStmnt;           // beginning of the last statement
    const sal_uInt8*   pError;           // address of the current error handler
    const sal_uInt8*   pRestart;         // restart-address
    const sal_uInt8*   pErrCode;         // restart-adresse RESUME NEXT
    const sal_uInt8*   pErrStmnt;        // Restart-Adresse RESUME 0
    OUString        aLibName;         // Lib-name for declare-call
    SbxArrayRef   refParams;        // current procedure parameters
    SbxArrayRef   refLocals;        // local variable
    SbxArrayRef   refArgv;
    // #74254, one refSaveObj is not enough! new: pRefSaveList (see above)
    short         nArgc;
    bool          bRun;
    bool          bError;           // true: handle errors
    bool          bInError;         // true: in an error handler
    bool          bBlocked;         // true: blocked by next call level, #i48868
    bool          bVBAEnabled;
    sal_uInt16        nFlags;           // Debugging-Flags
    SbError       nError;
    sal_uInt16        nOps;             // opcode counter
    sal_uInt32    m_nLastTime;

    RefSaveItem*  pRefSaveList;     // #74254 save temporary references
    RefSaveItem*  pItemStoreList;   // keep unused items
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
    void ClearRefs()
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
    ( SbxObject* pObj, sal_uInt32 nOp1, sal_uInt32 nOp2, SbError, bool bLocal, bool bStatic = false );
    void SetupArgs( SbxVariable*, sal_uInt32 );
    SbxVariable* CheckArray( SbxVariable* );

    void PushVar( SbxVariable* );
    SbxVariableRef PopVar();
    SbxVariable* GetTOS( short=0 );
    void TOSMakeTemp();
    void ClearExprStack();

    void PushGosub( const sal_uInt8* );
    void PopGosub();
    void ClearGosubStack();

    void PushArgv();
    void PopArgv();
    void ClearArgvStack();

    void PushFor();
    void PushForEach();
    void PopFor();
    void ClearForStack();

    void StepArith( SbxOperator );
    void StepUnary( SbxOperator );
    void StepCompare( SbxOperator );

    void SetParameters( SbxArray* );

    // HAS TO BE IMPLEMENTED SOME TIME
    void DllCall( const OUString&, const OUString&, SbxArray*, SbxDataType, bool );

    // #56204 swap out DIM-functionality into help method (step0.cxx)
    void DimImpl( SbxVariableRef refVar );

    static bool implIsClass( SbxObject* pObj, const OUString& aClass );

    void StepSETCLASS_impl( sal_uInt32 nOp1, bool bHandleDflt = false );

    // the following routines are called by the single
    // stepper and implement the single opcodes
    void StepNOP(),     StepEXP(),      StepMUL(),      StepDIV();
    void StepMOD(),     StepPLUS(),     StepMINUS(),    StepNEG();
    void StepEQ(),      StepNE(),       StepLT(),       StepGT();
    void StepLE(),      StepGE(),       StepIDIV(),     StepAND();
    void StepOR(),      StepXOR(),      StepEQV(),      StepIMP();
    void StepNOT(),     StepCAT(),      StepLIKE(),     StepIS();
    void StepARGC();
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
    void StepARRAYACCESS(), StepBYVAL();
    // all opcodes with one operand
    void StepLOADNC( sal_uInt32 ),  StepLOADSC( sal_uInt32 ),   StepLOADI( sal_uInt32 );
    void StepARGN( sal_uInt32 ),    StepBASED( sal_uInt32 ),    StepPAD( sal_uInt32 );
    void StepJUMP( sal_uInt32 ),    StepJUMPT( sal_uInt32 );
    void StepJUMPF( sal_uInt32 ),   StepONJUMP( sal_uInt32 );
    void StepGOSUB( sal_uInt32 ),   StepRETURN( sal_uInt32 );
    void StepTESTFOR( sal_uInt32 ), StepCASETO( sal_uInt32 ),   StepERRHDL( sal_uInt32 );
    void StepRESUME( sal_uInt32 ),  StepSETCLASS( sal_uInt32 ), StepVBASETCLASS( sal_uInt32 ),  StepTESTCLASS( sal_uInt32 ), StepLIB( sal_uInt32 );
    bool checkClass_Impl( const SbxVariableRef& refVal, const OUString& aClass, bool bRaiseErrors, bool bDefault = true );
    void StepCLOSE( sal_uInt32 ),   StepPRCHAR( sal_uInt32 ),   StepARGTYP( sal_uInt32 );
    // all opcodes with two operands
    void StepRTL( sal_uInt32, sal_uInt32 ),     StepPUBLIC( sal_uInt32, sal_uInt32 ),   StepPUBLIC_P( sal_uInt32, sal_uInt32 );
    void StepPUBLIC_Impl( sal_uInt32, sal_uInt32, bool bUsedForClassModule );
    void StepFIND_Impl( SbxObject* pObj, sal_uInt32 nOp1, sal_uInt32 nOp2, SbError, bool bLocal, bool bStatic = false );
    void StepFIND( sal_uInt32, sal_uInt32 ),    StepELEM( sal_uInt32, sal_uInt32 );
    void StepGLOBAL( sal_uInt32, sal_uInt32 ),  StepLOCAL( sal_uInt32, sal_uInt32 );
    void StepPARAM( sal_uInt32, sal_uInt32),    StepCREATE( sal_uInt32, sal_uInt32 );
    void StepCALL( sal_uInt32, sal_uInt32 ),    StepCALLC( sal_uInt32, sal_uInt32 );
    void StepCASEIS( sal_uInt32, sal_uInt32 ),  StepSTMNT( sal_uInt32, sal_uInt32 );
    SbxVariable* StepSTATIC_Impl( OUString& aName, SbxDataType& t );
    void StepOPEN( sal_uInt32, sal_uInt32 ),    StepSTATIC( sal_uInt32, sal_uInt32 );
    void StepTCREATE(sal_uInt32,sal_uInt32),    StepDCREATE(sal_uInt32,sal_uInt32);
    void StepGLOBAL_P( sal_uInt32, sal_uInt32 ),StepFIND_G( sal_uInt32, sal_uInt32 );
    void StepDCREATE_REDIMP(sal_uInt32,sal_uInt32), StepDCREATE_IMPL(sal_uInt32,sal_uInt32);
    void StepFIND_CM( sal_uInt32, sal_uInt32 );
    void StepFIND_STATIC( sal_uInt32, sal_uInt32 );
    static void implHandleSbxFlags( SbxVariable* pVar, SbxDataType t, sal_uInt32 nOp2 );
public:
    void          SetVBAEnabled( bool bEnabled );
    bool          IsImageFlag( SbiImageFlags n ) const;
    sal_uInt16      GetBase();
    sal_Int32  nLine,nCol1,nCol2;
    SbiRuntime* pNext;               // Stack-Chain

    SbiRuntime( SbModule*, SbMethod*, sal_uInt32 );
   ~SbiRuntime();
    void Error( SbError, bool bVBATranslationAlreadyDone = false );     // set error if != 0
    void Error( SbError, const OUString& );       // set error if != 0
    void FatalError( SbError );                 // error handling = standard, set error
    void FatalError( SbError, const OUString& );  // error handling = standard, set error
    static sal_Int32 translateErrorToVba( SbError nError, OUString& rMsg );
    bool Step();                    // single step (one opcode)
    void Stop()            { bRun = false;   }
    void block()     { bBlocked = true; }
    void unblock()   { bBlocked = false; }
    SbModule* GetModule()  { return pMod;    }
    sal_uInt16 GetDebugFlags() { return nFlags;  }
    void SetDebugFlags( sal_uInt16 nFl ) { nFlags = nFl;  }
    SbMethod* GetCaller() { return pMeth;}
    SbxVariable* GetExternalCaller(){ return mpExtCaller; }

    SbiForStack* FindForStackItemForCollection( class BasicCollection* pCollection );

    SbxBase* FindElementExtern( const OUString& rName );
    static bool isVBAEnabled();

};

inline void checkArithmeticOverflow( double d )
{
    if( !::rtl::math::isFinite( d ) )
        StarBASIC::Error( ERRCODE_BASIC_MATH_OVERFLOW );
}

inline void checkArithmeticOverflow( SbxVariable* pVar )
{
    if( pVar->GetType() == SbxDOUBLE )
    {
        double d = pVar->GetDouble();
        checkArithmeticOverflow( d );
    }
}


StarBASIC* GetCurrentBasic( StarBASIC* pRTBasic );

// Get information if security restrictions should be
// used (File IO based on UCB, no RTL function SHELL
// no DDE functionality, no DLLCALL) in basic because
// of portal "virtual" users (portal user != UNIX user)
// (Implemented in iosys.cxx)
bool needSecurityRestrictions();

// Returns true if UNO is available, otherwise the old
// file system implementation has to be used
// (Implemented in iosys.cxx)
bool hasUno();

// Converts possibly relative paths to absolute paths
// according to the setting done by ChDir/ChDrive
// (Implemented in methods.cxx)
OUString getFullPath( const OUString& aRelPath );

// Implementation of StepRENAME with UCB
// (Implemented in methods.cxx, so step0.cxx
// has not to be infected with UNO)
void implStepRenameUCB( const OUString& aSource, const OUString& aDest );

void implStepRenameOSL( const OUString& aSource, const OUString& aDest );
bool IsBaseIndexOne();

void removeDimAsNewRecoverItem( SbxVariable* pVar );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
