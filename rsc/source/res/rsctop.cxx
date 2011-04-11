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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_rsc.hxx"
/****************** I N C L U D E S **************************************/

// C and C++ Includes.
#include <stdio.h>
#include <string.h>

// Solar Definitionen
#include <tools/solar.h>
#include <rsctop.hxx>
/****************** C O D E **********************************************/
/****************** R s c T o p ******************************************/
/*************************************************************************
|*
|*    RscTop::RscTop()
|*
*************************************************************************/
RscTop::RscTop( Atom nId, sal_uInt32 nTypIdent, RscTop * pSuperCl )
    : RefNode( nId )
    , pSuperClass( pSuperCl )
    , nTypId( nTypIdent )
{
    pRefClass = this;
    if( pSuperClass )
        SetCallPar( pSuperClass->aCallPar1, pSuperClass->aCallPar2,
                    pSuperClass->aCallParType );
}

/*************************************************************************
|*
|*    RscTop::SetCallPar()
|*
*************************************************************************/
void RscTop::SetCallPar( const ByteString & rPar1, const ByteString & rPar2,
                         const ByteString & rParType )
{
    aCallPar1 = rPar1;
    aCallPar2 = rPar2;
    aCallParType = rParType;
}

/*************************************************************************
|*
|*    RscTop::GetDefault()
|*
*************************************************************************/
RSCINST RscTop::GetDefault()
{
    if( !aDfltInst.IsInst() )
        aDfltInst = this->Create( NULL, RSCINST() );
    return aDfltInst;
}

/*************************************************************************
|*
|*    RscTop::Pre_dtor()
|*
*************************************************************************/
void RscTop :: Pre_dtor(){
    if( aDfltInst.IsInst() ){
        aDfltInst.pClass->Destroy( aDfltInst );
        rtl_freeMemory( aDfltInst.pData );
        aDfltInst = RSCINST();
    };
}

/*************************************************************************
|*
|*    RscTop::GetConstant()
|*
*************************************************************************/
Atom    RscTop :: GetConstant( sal_uInt32 ){
    return InvalidAtom;
}

/*************************************************************************
|*
|*    RscTop::GetIndexType()
|*
*************************************************************************/
RscTop * RscTop::GetTypeClass() const
{
    if( pSuperClass )
        return pSuperClass->GetTypeClass();
    else
        return NULL;
}

/*************************************************************************
|*
|*    RscTop::Size()
|*
*************************************************************************/
sal_uInt32 RscTop :: Size()
{
    if( pSuperClass )
        return pSuperClass->Size();
    else
        return 0;
}

/*************************************************************************
|*
|*    RscTop::GetRef()
|*
*************************************************************************/
ERRTYPE RscTop :: GetRef( const RSCINST & rInst, RscId * pRscId ){
    if( pSuperClass )
        return pSuperClass->GetRef( rInst, pRscId );
    else
        return ERR_UNKNOWN_METHOD;
}

/*************************************************************************
|*
|*    RscTop::InHierarchy()
|*
*************************************************************************/
sal_Bool RscTop::InHierarchy( RscTop * pClass ){
    if( this == pClass )
        return( sal_True );
    if( pSuperClass )
        return( pSuperClass->InHierarchy( pClass ) );
    return( sal_False );
}

/*************************************************************************
|*
|*    RscTop::SetVariable()
|*
*************************************************************************/
ERRTYPE RscTop::SetVariable( Atom nVarName, RscTop * pClass,
                     RSCINST * pDflt, RSCVAR nVarType, sal_uInt32 nMask,
                     Atom nDataBaseName )
{
    if( pSuperClass )
        return pSuperClass->
               SetVariable( nVarName, pClass, pDflt,
                    nVarType, nMask, nDataBaseName );
    else
        return ERR_UNKNOWN_METHOD;
}

/*************************************************************************
|*
|*    RscTop::EnumVariable()
|*
*************************************************************************/
void RscTop::EnumVariables( void * pData, VarEnumCallbackProc pProc )
{
    if( pSuperClass )
        pSuperClass->EnumVariables( pData, pProc );
}

/*************************************************************************
|*
|*    RscTop::GetVariable()
|*
*************************************************************************/
RSCINST RscTop::GetVariable
(
    const RSCINST & rInst,
    Atom nVarName,
    const RSCINST & rInitInst,
    sal_Bool bInitDflt,
    RscTop * pCreateClass
)
{
    if( pSuperClass )
        return pSuperClass->
            GetVariable( rInst, nVarName, rInitInst, bInitDflt, pCreateClass );
    else
        return RSCINST();
}

/*************************************************************************
|*
|*    RscTop::GetCopyVar()
|*
*************************************************************************/
RSCINST RscTop::GetCopyVar( const RSCINST & rInst, Atom nVarName )
{
    if( pSuperClass )
        return pSuperClass->
            GetCopyVar( rInst, nVarName );
    else
        return RSCINST();
}

/*************************************************************************
|*
|*    RscTop::GetTupelVar()
|*
*************************************************************************/
RSCINST RscTop::GetTupelVar( const RSCINST & rInst, sal_uInt32 nPos,
                            const RSCINST & rInitInst )
{
    if( pSuperClass )
        return pSuperClass->GetTupelVar( rInst, nPos, rInitInst );
    else
        return RSCINST();
}

/*************************************************************************
|*
|*    RscTop::GetElement()
|*
*************************************************************************/
ERRTYPE RscTop::GetElement( const RSCINST & rInst, const RscId & rEleName,
                    RscTop *pCreateClass, const RSCINST & rCreateInst,
                    RSCINST * pGetInst )
{
    if( pSuperClass )
        return pSuperClass->
            GetElement( rInst, rEleName,
                        pCreateClass, rCreateInst,
                        pGetInst );
    else
        return ERR_UNKNOWN_METHOD;
}

/*************************************************************************
|*
|*    RscTop::GetArrayEle()
|*
*************************************************************************/
ERRTYPE RscTop::GetArrayEle
(
    const RSCINST & rInst,
    Atom nId,
    RscTop * pCreateClass,
    RSCINST * pGetInst
)
{
    if( pSuperClass )
        return pSuperClass->GetArrayEle( rInst, nId, pCreateClass, pGetInst );
    else
        return ERR_UNKNOWN_METHOD;
}

/*************************************************************************
|*
|*    RscTop::GetValueEle()
|*
*************************************************************************/
ERRTYPE RscTop::GetValueEle
(
    const RSCINST & rInst,
    sal_Int32 lValue,
    RscTop * pCreateClass,
    RSCINST * pGetInst
)
{
    if( pSuperClass )
        return pSuperClass->GetValueEle( rInst, lValue, pCreateClass, pGetInst );
    else
        return ERR_UNKNOWN_METHOD;
}

/*************************************************************************
|*
|*    RscTop::SearchEle()
|*
*************************************************************************/
RSCINST RscTop::SearchEle( const RSCINST & rInst, const RscId & rEleName,
                   RscTop * pClass )
{
    if( pSuperClass )
        return pSuperClass->
            SearchEle( rInst, rEleName, pClass );
    else
        return RSCINST();
}

/*************************************************************************
|*
|*    RscTop::GetPosEle()
|*
*************************************************************************/
RSCINST RscTop::GetPosEle( const RSCINST & rInst, sal_uInt32 nPos ){
    if( pSuperClass )
        return pSuperClass->
            GetPosEle( rInst, nPos );
    else
        return RSCINST();
}

/*************************************************************************
|*
|*    RscTop::MovePosEle()
|*
*************************************************************************/
ERRTYPE RscTop::MovePosEle( const RSCINST & rInst, sal_uInt32 nDestPos,
                    sal_uInt32 nSourcePos )
{
    if( pSuperClass )
        return pSuperClass->
            MovePosEle( rInst, nDestPos, nSourcePos );
    else
        return ERR_UNKNOWN_METHOD;
}

/*************************************************************************
|*
|*    RscTop::SetPosRscId()
|*
*************************************************************************/
ERRTYPE RscTop::SetPosRscId( const RSCINST & rInst, sal_uInt32 nPos,
                     const RscId & rRscId )
{
    if( pSuperClass )
        return pSuperClass->
            SetPosRscId( rInst, nPos, rRscId );
    else
        return ERR_UNKNOWN_METHOD;
}

/*************************************************************************
|*
|*    RscTop::GetInfoEle()
|*
*************************************************************************/
SUBINFO_STRUCT RscTop::GetInfoEle( const RSCINST & rInst, sal_uInt32 nPos ){
    if( pSuperClass )
        return pSuperClass->
            GetInfoEle( rInst, nPos );
    else
        return SUBINFO_STRUCT();
}

/*************************************************************************
|*
|*    RscTop::GetCount()
|*
*************************************************************************/
sal_uInt32  RscTop::GetCount( const RSCINST & rInst ){
    if( pSuperClass )
        return pSuperClass->GetCount( rInst );
    else
        return 0;
}

/*************************************************************************
|*
|*    RscTop::SetNumber()
|*
*************************************************************************/
ERRTYPE RscTop::SetNumber( const RSCINST & rInst, sal_Int32 lValue ){
    if( pSuperClass )
        return pSuperClass->
                    SetNumber( rInst, lValue );
    else
        return ERR_UNKNOWN_METHOD;
}

/*************************************************************************
|*
|*    RscTop::SetBool()
|*
*************************************************************************/
ERRTYPE RscTop::SetBool( const RSCINST & rInst, sal_Bool bValue ){
    if( pSuperClass )
        return pSuperClass->
                    SetBool( rInst, bValue );
    else
        return ERR_UNKNOWN_METHOD;
}

/*************************************************************************
|*
|*    RscTop::SetConst()
|*
*************************************************************************/
ERRTYPE RscTop::SetConst( const RSCINST & rInst, Atom nId, sal_Int32 nVal )
{
    if( pSuperClass )
        return pSuperClass->SetConst( rInst, nId, nVal );
    else
        return ERR_UNKNOWN_METHOD;
}

/*************************************************************************
|*
|*    RscTop::SetNotConst()
|*
*************************************************************************/
ERRTYPE RscTop::SetNotConst( const RSCINST & rInst, Atom nId ){
    if( pSuperClass )
        return pSuperClass->
                    SetNotConst( rInst, nId );
    else
        return ERR_UNKNOWN_METHOD;
}

/*************************************************************************
|*
|*    RscTop::SetString()
|*
*************************************************************************/
ERRTYPE RscTop::SetString( const RSCINST & rInst, const char * pStr ){
    if( pSuperClass )
        return pSuperClass->
                    SetString( rInst, pStr );
    else
        return ERR_UNKNOWN_METHOD;
}

/*************************************************************************
|*
|*    RscTop::GetNumber()
|*
*************************************************************************/
ERRTYPE RscTop::GetNumber( const RSCINST & rInst, sal_Int32 * pN ){
    if( pSuperClass )
        return pSuperClass->
                    GetNumber( rInst, pN );
    else
        return ERR_UNKNOWN_METHOD;
}

/*************************************************************************
|*
|*    RscTop::GetBool()
|*
*************************************************************************/
ERRTYPE RscTop::GetBool( const RSCINST & rInst, sal_Bool * pB ){
 if( pSuperClass )
        return pSuperClass->
                    GetBool( rInst, pB );
    else
        return ERR_UNKNOWN_METHOD;
}

/*************************************************************************
|*
|*    RscTop::GetCont()
|*
*************************************************************************/
ERRTYPE RscTop::GetConst( const RSCINST & rInst, Atom * pH ){
    if( pSuperClass )
        return pSuperClass->
                    GetConst( rInst, pH );
    else
        return ERR_UNKNOWN_METHOD;
}

/*************************************************************************
|*
|*    RscTop::GetString()
|*
*************************************************************************/
ERRTYPE RscTop::GetString( const RSCINST & rInst, char ** ppStr ){
    if( pSuperClass )
        return pSuperClass->
                    GetString( rInst, ppStr );
    else
        return ERR_UNKNOWN_METHOD;
}

/*************************************************************************
|*
|*    RscTop::Create()
|*
*************************************************************************/
RSCINST RscTop::Create( RSCINST * pInst, const RSCINST & rDefInst, sal_Bool bOwnRange )
{
    if( pSuperClass )
        return pSuperClass->
                    Create( pInst, rDefInst, bOwnRange );
    else{
        if( pInst )
            return *pInst;
        return RSCINST();
    }
}

/*************************************************************************
|*
|*    RscTop::Destroy()
|*
*************************************************************************/
void    RscTop::Destroy( const RSCINST & rInst ){
    if( pSuperClass )
        pSuperClass->Destroy( rInst );
}

/*************************************************************************
|*
|*    RscTop::IsConsistent()
|*
*************************************************************************/
sal_Bool RscTop::IsConsistent( const RSCINST & rInst )
{
    if( pSuperClass )
        return pSuperClass->IsConsistent( rInst );
    else
        return sal_True;
}

/*************************************************************************
|*
|*    RscTop::SetToDefault()
|*
*************************************************************************/
void    RscTop::SetToDefault( const RSCINST & rInst )
{
    if( pSuperClass )
        pSuperClass->SetToDefault( rInst );
}

/*************************************************************************
|*
|*    RscTop::IsDefault()
|*
*************************************************************************/
sal_Bool    RscTop::IsDefault( const RSCINST & rInst ){
    if( pSuperClass )
        return pSuperClass->IsDefault( rInst );
    else
        return sal_True;
}

/*************************************************************************
|*
|*    RscTop::IsValueDefault()
|*
*************************************************************************/
sal_Bool    RscTop::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ){
    if( pSuperClass )
        return pSuperClass->IsValueDefault( rInst, pDef );
    else
        return sal_True;
}

/*************************************************************************
|*
|*    RscTop::SetDefault()
|*
*************************************************************************/
void    RscTop::SetDefault( const RSCINST & rInst, Atom nVarId ){
    if( pSuperClass )
        pSuperClass->SetDefault( rInst, nVarId );
}

/*************************************************************************
|*
|*    RscTop::GetDefault()
|*
*************************************************************************/
RSCINST RscTop::GetDefault( Atom nVarId ){
    if( pSuperClass )
        return pSuperClass->
                    GetDefault( nVarId );
    else
        return RSCINST();
}

/*************************************************************************
|*
|*    RscTop::Delete()
|*
*************************************************************************/
void    RscTop::Delete( const RSCINST & rInst, RscTop * pClass,
                const RscId & rId )
{
    if( pSuperClass )
        pSuperClass->Delete( rInst, pClass, rId );
}

/*************************************************************************
|*
|*    RscTop::DeletePos()
|*
*************************************************************************/
void    RscTop::DeletePos( const RSCINST & rInst, sal_uInt32 nPos )
{
    if( pSuperClass )
        pSuperClass->DeletePos( rInst, nPos );
}

/*************************************************************************
|*
|*    RscTop::SetRef()
|*
*************************************************************************/
ERRTYPE RscTop::SetRef( const RSCINST & rInst, const RscId & rRefId ){
    if( pSuperClass )
        return pSuperClass->SetRef( rInst, rRefId );
    else
        return ERR_UNKNOWN_METHOD;
}

/*************************************************************************
|*
|*    RscTop::WriteHxxHeader()
|*
*************************************************************************/
ERRTYPE RscTop::WriteHxxHeader( const RSCINST & rInst, FILE * fOutput,
                                RscTypCont * pTC, const RscId & rId )
{
    if( pSuperClass )
        return pSuperClass->WriteHxxHeader( rInst, fOutput, pTC, rId );
    else
        return rInst.pClass->WriteHxx( rInst, fOutput, pTC, rId );
}

/*************************************************************************
|*
|*    RscTop::WriteHxx()
|*
*************************************************************************/
ERRTYPE RscTop::WriteHxx( const RSCINST & rInst, FILE * fOutput,
                          RscTypCont * pTC, const RscId & rId ){
    if( pSuperClass )
        return pSuperClass->WriteHxx( rInst, fOutput, pTC, rId );
    else
        return( ERR_OK );
}

/*************************************************************************
|*
|*    RscTop::WriteCxxHeader()
|*
*************************************************************************/
ERRTYPE RscTop::WriteCxxHeader( const RSCINST & rInst, FILE * fOutput,
                                RscTypCont * pTC, const RscId & rId )
{
    if( pSuperClass )
        return pSuperClass->WriteCxxHeader( rInst, fOutput, pTC, rId );
    else
        return rInst.pClass->WriteCxx( rInst, fOutput, pTC, rId );
}

/*************************************************************************
|*
|*    RscTop::WriteCxx()
|*
*************************************************************************/
ERRTYPE RscTop::WriteCxx( const RSCINST & rInst, FILE * fOutput,
                          RscTypCont * pTC, const RscId & rId ){
    if( pSuperClass )
        return pSuperClass->WriteCxx( rInst, fOutput, pTC, rId );
    else
        return ERR_OK;
}

/*************************************************************************
|*
|*    RscTop::WriteSrcHeader()
|*
*************************************************************************/
void RscTop::WriteSrcHeader( const RSCINST & rInst, FILE * fOutput,
                             RscTypCont * pTC, sal_uInt32 nTab,
                             const RscId & rId, const char * pVarName )
{
    if( pSuperClass )
        pSuperClass->WriteSrcHeader( rInst, fOutput, pTC, nTab, rId, pVarName );
    else
        rInst.pClass->WriteSrc( rInst, fOutput, pTC, nTab, pVarName );
}

/*************************************************************************
|*
|*    RscTop::WriteSrc()
|*
*************************************************************************/
void RscTop::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                       RscTypCont * pTC, sal_uInt32 nTab, const char * pVarName )
{
    if( pSuperClass )
        pSuperClass->WriteSrc( rInst, fOutput, pTC, nTab, pVarName );
}

/*************************************************************************
|*
|*    RscTop::WriteRcHeader()
|*
*************************************************************************/
ERRTYPE RscTop::WriteRcHeader( const RSCINST & rInst, RscWriteRc & rMem,
                               RscTypCont * pTC, const RscId & rId,
                               sal_uInt32 nDeep, sal_Bool bExtra )
{
    if( pSuperClass )
        return( pSuperClass->
            WriteRcHeader( rInst, rMem, pTC, rId, nDeep, bExtra ) );
    else
        return( rInst.pClass->WriteRc( rInst, rMem, pTC, nDeep, bExtra ) );
}

/*************************************************************************
|*
|*    RscTop::WriteRc()
|*
*************************************************************************/
ERRTYPE RscTop::WriteRc( const RSCINST & rInst, RscWriteRc & rMem,
                         RscTypCont * pTC, sal_uInt32 nDeep, sal_Bool bExtra )
{
    if( pSuperClass )
        return( pSuperClass->
            WriteRc( rInst, rMem, pTC, nDeep, bExtra ) );
    else
        return( ERR_OK );
}

/*************************************************************************
|*
|*    RscTop::WriteSyntaxHeader()
|*
*************************************************************************/
void RscTop::WriteSyntaxHeader( FILE * fOutput, RscTypCont * pTC )
{
    if( GetId() != InvalidAtom )
    {
        fprintf( fOutput, "class %s \n{\n", pHS->getString( GetId() ).getStr() );
        WriteSyntax( fOutput, pTC );
        fprintf( fOutput, "};\n\n" );
    }
}

/*************************************************************************
|*
|*    RscTop::WriteSyntax()
|*
*************************************************************************/
void RscTop::WriteSyntax( FILE * fOutput, RscTypCont * pTC )
{
    if( pSuperClass )
        pSuperClass->WriteSyntax( fOutput, pTC );
}

//========================================================================
void RscTop::WriteRcAccess
(
    FILE * fOutput,
    RscTypCont * /*pTC*/,
    const char * pName
)
{
    if( GetId() != InvalidAtom )
    {
        fprintf( fOutput, "\t\t//%s %s\n",
                pHS->getString( GetId() ).getStr(), pName );
    }
}

//========================================================================
void RscTop::WriteRcCtor( FILE * fOutput, RscTypCont * pTC )
{
    if( pSuperClass )
        pSuperClass->WriteRcCtor( fOutput, pTC );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
