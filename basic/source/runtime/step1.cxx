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


#include <stdlib.h>
#include <rtl/math.hxx>
#include <basic/sbuno.hxx>
#include "runtime.hxx"
#include "sbintern.hxx"
#include "iosys.hxx"
#include "image.hxx"
#include "sbunoobj.hxx"
#include "errobject.hxx"

bool checkUnoObjectType( SbUnoObject* refVal, const ::rtl::OUString& aClass );

// loading a numeric constant (+ID)

void SbiRuntime::StepLOADNC( sal_uInt32 nOp1 )
{
    SbxVariable* p = new SbxVariable( SbxDOUBLE );

    // #57844 use localized function
    String aStr = pImg->GetString( static_cast<short>( nOp1 ) );
    // also allow , !!!
    sal_uInt16 iComma = aStr.Search( ',' );
    if( iComma != STRING_NOTFOUND )
    {
        String aStr1 = aStr.Copy( 0, iComma );
        String aStr2 = aStr.Copy( iComma + 1 );
        aStr = aStr1;
        aStr += '.';
        aStr += aStr2;
    }
    double n = ::rtl::math::stringToDouble( aStr, '.', ',', NULL, NULL );

    p->PutDouble( n );
    PushVar( p );
}

// loading a string constant (+ID)

void SbiRuntime::StepLOADSC( sal_uInt32 nOp1 )
{
    SbxVariable* p = new SbxVariable;
    p->PutString( pImg->GetString( static_cast<short>( nOp1 ) ) );
    PushVar( p );
}

// Immediate Load (+Wert)

void SbiRuntime::StepLOADI( sal_uInt32 nOp1 )
{
    SbxVariable* p = new SbxVariable;
    p->PutInteger( static_cast<sal_Int16>( nOp1 ) );
    PushVar( p );
}

// stora a named argument in Argv (+Arg-no. from 1!)

void SbiRuntime::StepARGN( sal_uInt32 nOp1 )
{
    if( !refArgv )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    else
    {
        String aAlias( pImg->GetString( static_cast<short>( nOp1 ) ) );
        SbxVariableRef pVal = PopVar();
        if( bVBAEnabled && ( pVal->ISA(SbxMethod) || pVal->ISA(SbUnoProperty) || pVal->ISA(SbProcedureProperty) ) )
        {
            // named variables ( that are Any especially properties ) can be empty at this point and need a broadcast
            if ( pVal->GetType() == SbxEMPTY )
                pVal->Broadcast( SBX_HINT_DATAWANTED );
            // evaluate methods and properties!
            SbxVariable* pRes = new SbxVariable( *pVal );
            pVal = pRes;
        }
        refArgv->Put( pVal, nArgc );
        refArgv->PutAlias( aAlias, nArgc++ );
    }
}

// converting the type of an argument in Argv for DECLARE-Fkt. (+type)

void SbiRuntime::StepARGTYP( sal_uInt32 nOp1 )
{
    if( !refArgv )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    else
    {
        bool bByVal = (nOp1 & 0x8000) != 0;         // Ist BYVAL requested?
        SbxDataType t = (SbxDataType) (nOp1 & 0x7FFF);
        SbxVariable* pVar = refArgv->Get( refArgv->Count() - 1 );   // last Arg

        // check BYVAL
        if( pVar->GetRefCount() > 2 )       // 2 is normal for BYVAL
        {
            // parameter is a reference
            if( bByVal )
            {
                // Call by Value is requested -> create a copy
                pVar = new SbxVariable( *pVar );
                pVar->SetFlag( SBX_READWRITE );
                refExprStk->Put( pVar, refArgv->Count() - 1 );
            }
            else
                pVar->SetFlag( SBX_REFERENCE );     // Ref-Flag for DllMgr
        }
        else
        {
            // parameter is NO reference
            if( bByVal )
                pVar->ResetFlag( SBX_REFERENCE );   // no reference -> OK
            else
                Error( SbERR_BAD_PARAMETERS );      // reference needed
        }

        if( pVar->GetType() != t )
        {
            // variant for correct conversion
            // besides error, if SbxBYREF
            pVar->Convert( SbxVARIANT );
            pVar->Convert( t );
        }
    }
}

// bring string to a definite length (+length)

void SbiRuntime::StepPAD( sal_uInt32 nOp1 )
{
    SbxVariable* p = GetTOS();
    String& s = (String&)(const String&) *p;
    if( s.Len() > nOp1 )
        s.Erase( static_cast<xub_StrLen>( nOp1 ) );
    else
        s.Expand( static_cast<xub_StrLen>( nOp1 ), ' ' );
}

// jump (+target)

void SbiRuntime::StepJUMP( sal_uInt32 nOp1 )
{
#ifdef DBG_UTIL
    // #QUESTION shouln't this be
    // if( (sal_uInt8*)( nOp1+pImagGetCode() ) >= pImg->GetCodeSize() )
    if( nOp1 >= pImg->GetCodeSize() )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
#endif
    pCode = (const sal_uInt8*) pImg->GetCode() + nOp1;
}

// evaluate TOS, conditional jump (+target)

void SbiRuntime::StepJUMPT( sal_uInt32 nOp1 )
{
    SbxVariableRef p = PopVar();
    if( p->GetBool() )
        StepJUMP( nOp1 );
}

// evaluate TOS, conditional jump (+target)

void SbiRuntime::StepJUMPF( sal_uInt32 nOp1 )
{
    SbxVariableRef p = PopVar();
    // In a test e.g. If Null then
        // will evaluate Null will act as if False
    if( ( bVBAEnabled && p->IsNull() ) || !p->GetBool() )
        StepJUMP( nOp1 );
}

// evaluate TOS, jump into JUMP-table (+MaxVal)
// looks like this:
// ONJUMP 2
// JUMP target1
// JUMP target2
// ...
// if 0x8000 is set in the operand, push the return address (ON..GOSUB)

void SbiRuntime::StepONJUMP( sal_uInt32 nOp1 )
{
    SbxVariableRef p = PopVar();
    sal_Int16 n = p->GetInteger();
    if( nOp1 & 0x8000 )
    {
        nOp1 &= 0x7FFF;
        PushGosub( pCode + 5 * nOp1 );
    }
    if( n < 1 || static_cast<sal_uInt32>(n) > nOp1 )
        n = static_cast<sal_Int16>( nOp1 + 1 );
    nOp1 = (sal_uInt32) ( (const char*) pCode - pImg->GetCode() ) + 5 * --n;
    StepJUMP( nOp1 );
}

// UP-call (+target)

void SbiRuntime::StepGOSUB( sal_uInt32 nOp1 )
{
    PushGosub( pCode );
    if( nOp1 >= pImg->GetCodeSize() )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    pCode = (const sal_uInt8*) pImg->GetCode() + nOp1;
}

// UP-return (+0 or target)

void SbiRuntime::StepRETURN( sal_uInt32 nOp1 )
{
    PopGosub();
    if( nOp1 )
        StepJUMP( nOp1 );
}

// check FOR-variable (+Endlabel)

void SbiRuntime::StepTESTFOR( sal_uInt32 nOp1 )
{
    if( !pForStk )
    {
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
        return;
    }

    bool bEndLoop = false;
    switch( pForStk->eForType )
    {
        case FOR_TO:
        {
            SbxOperator eOp = ( pForStk->refInc->GetDouble() < 0 ) ? SbxLT : SbxGT;
            if( pForStk->refVar->Compare( eOp, *pForStk->refEnd ) )
                bEndLoop = true;
            break;
        }
        case FOR_EACH_ARRAY:
        {
            SbiForStack* p = pForStk;
            if( p->pArrayCurIndices == NULL )
            {
                bEndLoop = true;
            }
            else
            {
                SbxDimArray* pArray = (SbxDimArray*)(SbxVariable*)p->refEnd;
                short nDims = pArray->GetDims();

                // Empty array?
                if( nDims == 1 && p->pArrayLowerBounds[0] > p->pArrayUpperBounds[0] )
                {
                    bEndLoop = true;
                    break;
                }
                SbxVariable* pVal = pArray->Get32( p->pArrayCurIndices );
                *(p->refVar) = *pVal;

                bool bFoundNext = false;
                for( short i = 0 ; i < nDims ; i++ )
                {
                    if( p->pArrayCurIndices[i] < p->pArrayUpperBounds[i] )
                    {
                        bFoundNext = true;
                        p->pArrayCurIndices[i]++;
                        for( short j = i - 1 ; j >= 0 ; j-- )
                            p->pArrayCurIndices[j] = p->pArrayLowerBounds[j];
                        break;
                    }
                }
                if( !bFoundNext )
                {
                    delete[] p->pArrayCurIndices;
                    p->pArrayCurIndices = NULL;
                }
            }
            break;
        }
        case FOR_EACH_COLLECTION:
        {
            BasicCollection* pCollection = (BasicCollection*)(SbxVariable*)pForStk->refEnd;
            SbxArrayRef xItemArray = pCollection->xItemArray;
            sal_Int32 nCount = xItemArray->Count32();
            if( pForStk->nCurCollectionIndex < nCount )
            {
                SbxVariable* pRes = xItemArray->Get32( pForStk->nCurCollectionIndex );
                pForStk->nCurCollectionIndex++;
                (*pForStk->refVar) = *pRes;
            }
            else
            {
                bEndLoop = true;
            }
            break;
        }
        case FOR_EACH_XENUMERATION:
        {
            SbiForStack* p = pForStk;
            if( p->xEnumeration->hasMoreElements() )
            {
                Any aElem = p->xEnumeration->nextElement();
                SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
                unoToSbxValue( (SbxVariable*)xVar, aElem );
                (*pForStk->refVar) = *xVar;
            }
            else
            {
                bEndLoop = true;
            }
            break;
        }
    }
    if( bEndLoop )
    {
        PopFor();
        StepJUMP( nOp1 );
    }
}

// Tos+1 <= Tos+2 <= Tos, 2xremove (+Target)

void SbiRuntime::StepCASETO( sal_uInt32 nOp1 )
{
    if( !refCaseStk || !refCaseStk->Count() )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    else
    {
        SbxVariableRef xTo   = PopVar();
        SbxVariableRef xFrom = PopVar();
        SbxVariableRef xCase = refCaseStk->Get( refCaseStk->Count() - 1 );
        if( *xCase >= *xFrom && *xCase <= *xTo )
            StepJUMP( nOp1 );
    }
}


void SbiRuntime::StepERRHDL( sal_uInt32 nOp1 )
{
    const sal_uInt8* p = pCode;
    StepJUMP( nOp1 );
    pError = pCode;
    pCode = p;
    pInst->aErrorMsg = String();
    pInst->nErr = 0;
    pInst->nErl = 0;
    nError = 0;
    SbxErrObject::getUnoErrObject()->Clear();
}

// Resume after errors (+0=statement, 1=next or Label)

void SbiRuntime::StepRESUME( sal_uInt32 nOp1 )
{
    // #32714 Resume without error? -> error
    if( !bInError )
    {
        Error( SbERR_BAD_RESUME );
        return;
    }
    if( nOp1 )
    {
        // set Code-pointer to the next statement
        sal_uInt16 n1, n2;
        pCode = pMod->FindNextStmnt( pErrCode, n1, n2, sal_True, pImg );
    }
    else
        pCode = pErrStmnt;
    if ( pError ) // current in error handler ( and got a Resume Next statment )
        SbxErrObject::getUnoErrObject()->Clear();

    if( nOp1 > 1 )
        StepJUMP( nOp1 );
    pInst->aErrorMsg = String();
    pInst->nErr = 0;
    pInst->nErl = 0;
    nError = 0;
    bInError = sal_False;
}

// close channel (+channel, 0=all)
void SbiRuntime::StepCLOSE( sal_uInt32 nOp1 )
{
    SbError err;
    if( !nOp1 )
        pIosys->Shutdown();
    else
    {
        err = pIosys->GetError();
        if( !err )
        {
            pIosys->Close();
        }
    }
    err = pIosys->GetError();
    Error( err );
}

// output character (+char)

void SbiRuntime::StepPRCHAR( sal_uInt32 nOp1 )
{
    rtl::OString s(static_cast<sal_Char>(nOp1));
    pIosys->Write( s );
    Error( pIosys->GetError() );
}

// check whether TOS is a certain object class (+StringID)

bool SbiRuntime::implIsClass( SbxObject* pObj, const ::rtl::OUString& aClass )
{
    bool bRet = true;

    if( !aClass.isEmpty() )
    {
        bRet = pObj->IsClass( aClass );
        if( !bRet )
            bRet = aClass.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM("object") );
        if( !bRet )
        {
            String aObjClass = pObj->GetClassName();
            SbModule* pClassMod = GetSbData()->pClassFac->FindClass( aObjClass );
            SbClassData* pClassData;
            if( pClassMod && (pClassData=pClassMod->pClassData) != NULL )
            {
                SbxVariable* pClassVar =
                    pClassData->mxIfaces->Find( aClass, SbxCLASS_DONTCARE );
                bRet = (pClassVar != NULL);
            }
        }
    }
    return bRet;
}

bool SbiRuntime::checkClass_Impl( const SbxVariableRef& refVal,
    const ::rtl::OUString& aClass, bool bRaiseErrors, bool bDefault )
{
    bool bOk = bDefault;

    SbxDataType t = refVal->GetType();
    SbxVariable* pVal = (SbxVariable*)refVal;
    // we don't know the type of uno properties that are (maybevoid)
    if ( t == SbxEMPTY && refVal->ISA(SbUnoProperty) )
    {
        SbUnoProperty* pProp = (SbUnoProperty*)pVal;
        t = pProp->getRealType();
    }
    if( t == SbxOBJECT )
    {
        SbxObject* pObj;
        if( pVal->IsA( TYPE(SbxObject) ) )
            pObj = (SbxObject*) pVal;
        else
        {
            pObj = (SbxObject*) refVal->GetObject();
            if( pObj && !pObj->IsA( TYPE(SbxObject) ) )
                pObj = NULL;
        }
        if( pObj )
        {
            if( !implIsClass( pObj, aClass ) )
            {
                if ( bVBAEnabled && pObj->IsA( TYPE(SbUnoObject) ) )
                {
                    SbUnoObject* pUnoObj = PTR_CAST(SbUnoObject,pObj);
                    bOk = checkUnoObjectType( pUnoObj, aClass );
                }
                else
                    bOk = false;
                if ( !bOk )
                {
                    if( bRaiseErrors )
                        Error( SbERR_INVALID_USAGE_OBJECT );
                }
            }
            else
            {
                bOk = true;

                SbClassModuleObject* pClassModuleObject = PTR_CAST(SbClassModuleObject,pObj);
                if( pClassModuleObject != NULL )
                    pClassModuleObject->triggerInitializeEvent();
            }
        }
    }
    else
    {
        if ( !bVBAEnabled )
        {
            if( bRaiseErrors )
                Error( SbERR_NEEDS_OBJECT );
            bOk = false;
        }
    }
    return bOk;
}

void SbiRuntime::StepSETCLASS_impl( sal_uInt32 nOp1, bool bHandleDflt )
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    String aClass( pImg->GetString( static_cast<short>( nOp1 ) ) );

    bool bOk = checkClass_Impl( refVal, aClass, true );
    if( bOk )
        StepSET_Impl( refVal, refVar, bHandleDflt ); // don't do handle dflt prop for a "proper" set
}

void SbiRuntime::StepVBASETCLASS( sal_uInt32 nOp1 )
{
    StepSETCLASS_impl( nOp1, false );
}

void SbiRuntime::StepSETCLASS( sal_uInt32 nOp1 )
{
    StepSETCLASS_impl( nOp1, true );
}

void SbiRuntime::StepTESTCLASS( sal_uInt32 nOp1 )
{
    SbxVariableRef xObjVal = PopVar();
    String aClass( pImg->GetString( static_cast<short>( nOp1 ) ) );
    bool bDefault = !bVBAEnabled;
    bool bOk = checkClass_Impl( xObjVal, aClass, false, bDefault );

    SbxVariable* pRet = new SbxVariable;
    pRet->PutBool( bOk );
    PushVar( pRet );
}

// define library for following declare-call

void SbiRuntime::StepLIB( sal_uInt32 nOp1 )
{
    aLibName = pImg->GetString( static_cast<short>( nOp1 ) );
}

// TOS is incremented by BASE, BASE is pushed before (+BASE)
// This opcode is pushed before DIM/REDIM-commands,
// if there's been only one index named.

void SbiRuntime::StepBASED( sal_uInt32 nOp1 )
{
    SbxVariable* p1 = new SbxVariable;
    SbxVariableRef x2 = PopVar();

    // #109275 Check compatiblity mode
    bool bCompatible = ((nOp1 & 0x8000) != 0);
    sal_uInt16 uBase = static_cast<sal_uInt16>(nOp1 & 1);       // Can only be 0 or 1
    p1->PutInteger( uBase );
    if( !bCompatible )
        x2->Compute( SbxPLUS, *p1 );
    PushVar( x2 );  // first the Expr
    PushVar( p1 );  // then the Base
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
