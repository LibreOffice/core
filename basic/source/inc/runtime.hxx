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

#pragma once

#include <basic/sberrors.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbx.hxx>

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/file.hxx>
#include <i18nlangtag/lang.h>

#include <cmath>
#include <vector>
#include <memory>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <unotools/localedatawrapper.hxx>
#include <o3tl/deleter.hxx>
#include <o3tl/typed_flags_set.hxx>

class SbiInstance;                  // active StarBASIC process
class SbiRuntime;                   // active StarBASIC procedure instance

struct SbiArgv;                     // Argv stack element
struct SbiGosub;                    // GOSUB stack element
class  SbiImage;                    // Code-Image
class  SbiIoSystem;
class  SbiDdeControl;
class  SbiDllMgr;
class  SvNumberFormatter;           // time/date functions
enum class SbiImageFlags;

enum class ForType {
    To,
    EachArray,
    EachCollection,
    EachXEnumeration,
    Error,
};

struct SbiForStack {                // for/next stack:
    SbiForStack*    pNext;          // Chain
    SbxVariableRef  refVar;         // loop variable
    SbxVariableRef  refEnd;         // end expression / for each: Array/BasicCollection object
    SbxVariableRef  refInc;         // increment expression

    // For each support
    ForType             eForType;
    sal_Int32           nCurCollectionIndex;
    std::unique_ptr<sal_Int32[]>
                        pArrayCurIndices;
    std::unique_ptr<sal_Int32[]>
                        pArrayLowerBounds;
    std::unique_ptr<sal_Int32[]>
                        pArrayUpperBounds;
    css::uno::Reference< css::container::XEnumeration > xEnumeration;

    SbiForStack()
        : pNext(nullptr)
        , eForType(ForType::To)
        , nCurCollectionIndex(0)
    {}
};

#define MAXRECURSION 500 //to prevent dead-recursions

enum class SbAttributes {
    NONE          = 0x0000,
    READONLY      = 0x0001,
    HIDDEN        = 0x0002,
    DIRECTORY     = 0x0010
};

namespace o3tl
{
    template<> struct typed_flags<SbAttributes> : is_typed_flags<SbAttributes, 0x13> {};
}

class WildCard;

class SbiRTLData
{
public:

    std::unique_ptr<osl::Directory> pDir;
    SbAttributes nDirFlags;
    short   nCurDirPos;

    OUString sFullNameToBeChecked;
    std::unique_ptr<WildCard> pWildCard;

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
    std::unique_ptr<SbiIoSystem, o3tl::default_delete<SbiIoSystem>> pIosys;
    // DDE
    std::unique_ptr<SbiDdeControl>    pDdeCtrl;
    // DLL-Calls (DECLARE)
    std::unique_ptr<SbiDllMgr>        pDllMgr;
    std::shared_ptr<SvNumberFormatter> pNumberFormatter;
    StarBASIC*      pBasic;
    LanguageType    meFormatterLangType;
    DateOrder       meFormatterDateOrder;
    sal_uInt32      nStdDateIdx, nStdTimeIdx, nStdDateTimeIdx;

    ErrCode         nErr;
    OUString        aErrorMsg;      // last error message for $ARG
    sal_Int32       nErl;           // current error line
    bool        bReschedule;    // Flag: sal_True = Reschedule in main loop
    bool        bCompatibility; // Flag: sal_True = VBA runtime compatibility mode

    ComponentVector_t ComponentVector;
public:
    SbiRuntime*  pRun;              // Call-Stack

    // #31460 new concept for StepInto/Over/Out,
    // explanation see runtime.cxx at SbiInstance::CalcBreakCallLevel()
    sal_uInt16  nCallLvl;
    sal_uInt16  nBreakCallLvl;
    void    CalcBreakCallLevel( BasicDebugFlags nFlags );

    SbiInstance( StarBASIC* );
   ~SbiInstance();

    void Error( ErrCode );                      // trappable Error
    void Error( ErrCode, const OUString& rMsg );  // trappable Error with message
    void ErrorVB( sal_Int32 nVBNumber, const OUString& rMsg );
    void setErrorVB( sal_Int32 nVBNumber );
    void FatalError( ErrCode );                 // non-trappable Error
    void FatalError( ErrCode, const OUString& );  // non-trappable Error
    void Abort();                               // with current error code

    void    Stop();
    ErrCode const & GetErr() const       { return nErr; }
    const OUString& GetErrorMsg() const  { return aErrorMsg; }
    sal_Int32 GetErl() const             { return nErl; }
    void    EnableReschedule( bool bEnable ) { bReschedule = bEnable; }
    bool    IsReschedule() const { return bReschedule; }
    void    EnableCompatibility( bool bEnable ) { bCompatibility = bEnable; }
    bool    IsCompatibility() const { return bCompatibility; }

    ComponentVector_t& getComponentVector()  { return ComponentVector; }

    SbMethod* GetCaller( sal_uInt16 );
    SbModule* GetActiveModule();

    SbiIoSystem* GetIoSystem() { return pIosys.get(); }
    SbiDdeControl* GetDdeControl() { return pDdeCtrl.get(); }
    StarBASIC* GetBasic() { return pBasic; }
    SbiDllMgr* GetDllMgr();
    SbiRTLData& GetRTLData() const { return const_cast<SbiRTLData&>(aRTLData); }

    std::shared_ptr<SvNumberFormatter> const & GetNumberFormatter();
    sal_uInt32 GetStdDateIdx() const { return nStdDateIdx; }
    sal_uInt32 GetStdTimeIdx() const { return nStdTimeIdx; }
    sal_uInt32 GetStdDateTimeIdx() const { return nStdDateTimeIdx; }

    // offer NumberFormatter also static
    static std::shared_ptr<SvNumberFormatter> PrepareNumberFormatter( sal_uInt32 &rnStdDateIdx,
        sal_uInt32 &rnStdTimeIdx, sal_uInt32 &rnStdDateTimeIdx,
        LanguageType const * peFormatterLangType=nullptr, DateOrder const * peFormatterDateOrder=nullptr );
};

// There's one instance of this class for every executed sub-program.
// This instance is the heart of the BASIC-machine and contains only local data.

class SbiRuntime
{
    friend void SbRtl_CallByName( StarBASIC* pBasic, SbxArray& rPar, bool bWrite );

    typedef void( SbiRuntime::*pStep0 )();
    typedef void( SbiRuntime::*pStep1 )( sal_uInt32 nOp1 );
    typedef void( SbiRuntime::*pStep2 )( sal_uInt32 nOp1, sal_uInt32 nOp2 );
    static const pStep0 aStep0[];         // opcode-table group 0
    static const pStep1 aStep1[];
    static const pStep2 aStep2[];

    StarBASIC&         rBasic;           // StarBASIC instance
    SbiInstance*       pInst;            // current thread
    SbModule*          pMod;             // current module
    SbMethod*          pMeth;            // method instance
    SbiIoSystem*       pIosys;           // I/O-System
    const SbiImage*    pImg;             // Code-Image
    SbxArrayRef        refExprStk;       // expression stack
    SbxArrayRef        refCaseStk;       // CASE expression stack
    SbxArrayRef        refRedimpArray;   // Array saved to use for REDIM PRESERVE
    SbxVariableRef     refRedim;         // Array saved to use for REDIM
    SbxVariableRef     xDummyVar;        // substitute for variables that weren't found
    SbxVariable*       mpExtCaller;      // Caller ( external - e.g. button name, shape, range object etc. - only in vba mode )
    SbiForStack*       pForStk;          // FOR/NEXT-Stack
    sal_uInt16         nExprLvl;         // depth of the expr-stack
    sal_uInt16         nForLvl;          // #118235: Maintain for level
    const sal_uInt8*   pCode;            // current Code-Pointer
    const sal_uInt8*   pStmnt;           // beginning of the last statement
    const sal_uInt8*   pError;           // address of the current error handler
    const sal_uInt8*   pRestart;         // restart-address
    const sal_uInt8*   pErrCode;         // restart-address RESUME NEXT
    const sal_uInt8*   pErrStmnt;        // restart-address RESUME 0
    OUString           aLibName;         // Lib-name for declare-call
    SbxArrayRef        refParams;        // current procedure parameters
    SbxArrayRef        refLocals;        // local variable
    SbxArrayRef        refArgv;
    // #74254, one refSaveObj is not enough! new: pRefSaveList (see above)
    short              nArgc;
    bool               bRun;
    bool               bError;           // true: handle errors
    bool               bInError;         // true: in an error handler
    bool               bBlocked;         // true: blocked by next call level, #i48868
    bool               bVBAEnabled;
    BasicDebugFlags    nFlags;           // Debugging-Flags
    ErrCode            nError;
    sal_uInt16         nOps;             // opcode counter
    sal_uInt32         m_nLastTime;

    std::vector<SbxVariableRef>  aRefSaved; // #74254 save temporary references
    std::vector<SbiGosub>   pGosubStk;      // GOSUB stack
    std::vector<SbiArgv>    pArgvStk;       // ARGV-Stack


    SbxVariable* FindElement
    ( SbxObject* pObj, sal_uInt32 nOp1, sal_uInt32 nOp2, ErrCode, bool bLocal, bool bStatic = false );
    void SetupArgs( SbxVariable*, sal_uInt32 );
    SbxVariable* CheckArray( SbxVariable* );

    void PushVar( SbxVariable* );
    SbxVariableRef PopVar();
    SbxVariable* GetTOS();
    void TOSMakeTemp();
    void ClearExprStack();

    void PushGosub( const sal_uInt8* );
    void PopGosub();

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
    void DllCall( std::u16string_view, std::u16string_view, SbxArray*, SbxDataType, bool );

    // #56204 swap out DIM-functionality into help method (step0.cxx)
    void DimImpl(const SbxVariableRef& refVar);
    bool EvaluateTopOfStackAsBool();

    static bool implIsClass( SbxObject const * pObj, const OUString& aClass );

    void StepSETCLASS_impl( sal_uInt32 nOp1, bool bHandleDflt );

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
    bool checkClass_Impl( const SbxVariableRef& refVal, const OUString& aClass, bool bRaiseErrors, bool bDefault );
    void StepCLOSE( sal_uInt32 ),   StepPRCHAR( sal_uInt32 ),   StepARGTYP( sal_uInt32 );
    // all opcodes with two operands
    void StepRTL( sal_uInt32, sal_uInt32 ),     StepPUBLIC( sal_uInt32, sal_uInt32 ),   StepPUBLIC_P( sal_uInt32, sal_uInt32 );
    void StepPUBLIC_Impl( sal_uInt32, sal_uInt32, bool bUsedForClassModule );
    void StepFIND_Impl( SbxObject* pObj, sal_uInt32 nOp1, sal_uInt32 nOp2, ErrCode, bool bStatic = false );
    void StepFIND( sal_uInt32, sal_uInt32 ),    StepELEM( sal_uInt32, sal_uInt32 );
    void StepGLOBAL( sal_uInt32, sal_uInt32 ),  StepLOCAL( sal_uInt32, sal_uInt32 );
    void StepPARAM( sal_uInt32, sal_uInt32),    StepCREATE( sal_uInt32, sal_uInt32 );
    void StepCALL( sal_uInt32, sal_uInt32 ),    StepCALLC( sal_uInt32, sal_uInt32 );
    void StepCASEIS( sal_uInt32, sal_uInt32 ),  StepSTMNT( sal_uInt32, sal_uInt32 );
    SbxVariable* StepSTATIC_Impl(
        OUString const & aName, SbxDataType t, sal_uInt32 nOp2 );
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
    sal_uInt16      GetBase() const;
    sal_Int32  nLine,nCol1,nCol2;
    SbiRuntime* pNext;               // Stack-Chain

    // tdf#79426, tdf#125180 - adds the information about a missing parameter
    static void SetIsMissing( SbxVariable* );
    // tdf#79426, tdf#125180 - checks if a variable contains the information about a missing parameter
    static bool IsMissing( SbxVariable*, sal_uInt16 );

    SbiRuntime( SbModule*, SbMethod*, sal_uInt32 );
   ~SbiRuntime();
    void Error( ErrCode, bool bVBATranslationAlreadyDone = false );     // set error if != 0
    void Error( ErrCode, const OUString& );       // set error if != 0
    void FatalError( ErrCode );                 // error handling = standard, set error
    void FatalError( ErrCode, const OUString& );  // error handling = standard, set error
    static sal_Int32 translateErrorToVba( ErrCode nError, OUString& rMsg );
    bool Step();                    // single step (one opcode)
    void Stop()            { bRun = false;   }
    void block()     { bBlocked = true; }
    void unblock()   { bBlocked = false; }
    SbModule* GetModule()  { return pMod;    }
    BasicDebugFlags GetDebugFlags() const { return nFlags;  }
    void SetDebugFlags( BasicDebugFlags nFl ) { nFlags = nFl;  }
    SbMethod* GetCaller() { return pMeth;}
    SbxVariable* GetExternalCaller(){ return mpExtCaller; }

    SbiForStack* FindForStackItemForCollection( class BasicCollection const * pCollection );

    SbxBase* FindElementExtern( const OUString& rName );
    static bool isVBAEnabled();

};

inline void checkArithmeticOverflow( double d )
{
    if( !std::isfinite( d ) )
        StarBASIC::Error( ERRCODE_BASIC_MATH_OVERFLOW );
}

inline void checkArithmeticOverflow( SbxVariable const * pVar )
{
    if( pVar->GetType() == SbxDOUBLE )
    {
        double d = pVar->GetDouble();
        checkArithmeticOverflow( d );
    }
}


StarBASIC* GetCurrentBasic( StarBASIC* pRTBasic );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
