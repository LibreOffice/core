/*************************************************************************
 *
 *  $RCSfile: rsctop.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:56 $
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
/************************************************************************

    Source Code Control System - Header

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/source/res/rsctop.cxx,v 1.1.1.1 2000-09-18 16:42:56 hr Exp $

*************************************************************************/
/****************** I N C L U D E S **************************************/

// C and C++ Includes.
#include <stdio.h>
#include <string.h>

// Solar Definitionen
#include <tools/solar.h>

#ifndef _RSCTOP_HXX
#include <rsctop.hxx>
#endif
/****************** C O D E **********************************************/
/****************** R s c T o p ******************************************/
/*************************************************************************
|*
|*    RscTop::RscTop()
|*
|*    Beschreibung
|*    Ersterstellung    MM 03.06.91
|*    Letzte Aenderung  MM 03.06.91
|*
*************************************************************************/
RscTop::RscTop( HASHID nId, USHORT nTypIdent, RscTop * pSuperCl )
    : RefNode( nId )
    , nTypId( nTypIdent )
    , pSuperClass( pSuperCl )
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
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
|*    Beschreibung
|*    Ersterstellung    MM 19.06.91
|*    Letzte Aenderung  MM 19.06.91
|*
*************************************************************************/
void RscTop :: Pre_dtor(){
    if( aDfltInst.IsInst() ){
        aDfltInst.pClass->Destroy( aDfltInst );
        RscMem::Free( aDfltInst.pData );
        aDfltInst = RSCINST();
    };
}

/*************************************************************************
|*
|*    RscTop::GetConstant()
|*
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
HASHID  RscTop :: GetConstant( USHORT ){
    return HASH_NONAME;
}

/*************************************************************************
|*
|*    RscTop::GetIndexType()
|*
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
USHORT RscTop :: Size()
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
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
|*    Beschreibung
|*    Ersterstellung    MM 17.05.91
|*    Letzte Aenderung  MM 17.05.91
|*
*************************************************************************/
BOOL RscTop::InHierarchy( RscTop * pClass ){
    if( this == pClass )
        return( TRUE );
    if( pSuperClass )
        return( pSuperClass->InHierarchy( pClass ) );
    return( FALSE );
}

/*************************************************************************
|*
|*    RscTop::SetVariable()
|*
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
ERRTYPE RscTop::SetVariable( HASHID nVarName, RscTop * pClass,
                     RSCINST * pDflt, RSCVAR nVarType, USHORT nMask,
                     HASHID nDataBaseName )
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
|*    Beschreibung
|*    Ersterstellung    MM 03.02.93
|*    Letzte Aenderung  MM 03.02.93
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
RSCINST RscTop::GetVariable
(
    const RSCINST & rInst,
    HASHID nVarName,
    const RSCINST & rInitInst,
    BOOL bInitDflt,
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
RSCINST RscTop::GetCopyVar( const RSCINST & rInst, HASHID nVarName )
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
RSCINST RscTop::GetTupelVar( const RSCINST & rInst, USHORT nPos,
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
ERRTYPE RscTop::GetArrayEle
(
    const RSCINST & rInst,
    HASHID nId,
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
ERRTYPE RscTop::GetValueEle
(
    const RSCINST & rInst,
    long lValue,
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
RSCINST RscTop::GetPosEle( const RSCINST & rInst, USHORT nPos ){
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
ERRTYPE RscTop::MovePosEle( const RSCINST & rInst, USHORT nDestPos,
                    USHORT nSourcePos )
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
ERRTYPE RscTop::SetPosRscId( const RSCINST & rInst, USHORT nPos,
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
SUBINFO_STRUCT RscTop::GetInfoEle( const RSCINST & rInst, USHORT nPos ){
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
USHORT  RscTop::GetCount( const RSCINST & rInst ){
    if( pSuperClass )
        return pSuperClass->GetCount( rInst );
    else
        return 0;
}

/*************************************************************************
|*
|*    RscTop::SetNumber()
|*
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
ERRTYPE RscTop::SetNumber( const RSCINST & rInst, long lValue ){
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
ERRTYPE RscTop::SetBool( const RSCINST & rInst, BOOL bValue ){
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
ERRTYPE RscTop::SetConst( const RSCINST & rInst, HASHID nId, long nVal )
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
ERRTYPE RscTop::SetNotConst( const RSCINST & rInst, HASHID nId ){
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
ERRTYPE RscTop::SetString( const RSCINST & rInst, char * pStr ){
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
ERRTYPE RscTop::GetNumber( const RSCINST & rInst, long * pN ){
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
ERRTYPE RscTop::GetBool( const RSCINST & rInst, BOOL * pB ){
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
ERRTYPE RscTop::GetConst( const RSCINST & rInst, HASHID * pH ){
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
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
|*    Beschreibung
|*    Ersterstellung    MM 17.05.91
|*    Letzte Aenderung  MM 17.07.91
|*
*************************************************************************/
RSCINST RscTop::Create( RSCINST * pInst, const RSCINST & rDefInst, BOOL bOwnRange )
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
BOOL    RscTop::IsConsistent( const RSCINST & rInst,
                              RscInconsList * pList )
{
    if( pSuperClass )
        return pSuperClass->IsConsistent( rInst, pList );
    else
        return TRUE;
}

/*************************************************************************
|*
|*    RscTop::SetToDefault()
|*
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
BOOL    RscTop::IsDefault( const RSCINST & rInst ){
    if( pSuperClass )
        return pSuperClass->IsDefault( rInst );
    else
        return TRUE;
}

/*************************************************************************
|*
|*    RscTop::IsValueDefault()
|*
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
BOOL    RscTop::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ){
    if( pSuperClass )
        return pSuperClass->IsValueDefault( rInst, pDef );
    else
        return TRUE;
}

/*************************************************************************
|*
|*    RscTop::SetDefault()
|*
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
void    RscTop::SetDefault( const RSCINST & rInst, HASHID nVarId ){
    if( pSuperClass )
        pSuperClass->SetDefault( rInst, nVarId );
}

/*************************************************************************
|*
|*    RscTop::GetDefault()
|*
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
RSCINST RscTop::GetDefault( HASHID nVarId ){
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
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
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
void    RscTop::DeletePos( const RSCINST & rInst, USHORT nPos )
{
    if( pSuperClass )
        pSuperClass->DeletePos( rInst, nPos );
}

/*************************************************************************
|*
|*    RscTop::SetRef()
|*
|*    Beschreibung
|*    Ersterstellung    MM 18.09.91
|*    Letzte Aenderung  MM 18.09.91
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
|*    Beschreibung
|*    Ersterstellung    MM 29.05.91
|*    Letzte Aenderung  MM 29.05.91
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
|*    Beschreibung
|*    Ersterstellung    MM 29.05.91
|*    Letzte Aenderung  MM 29.05.91
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
|*    Beschreibung
|*    Ersterstellung    MM 29.05.91
|*    Letzte Aenderung  MM 29.05.91
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
|*    Beschreibung
|*    Ersterstellung    MM 29.05.91
|*    Letzte Aenderung  MM 29.05.91
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
|*    Beschreibung
|*    Ersterstellung    MM 08.04.91
|*    Letzte Aenderung  MM 08.04.91
|*
*************************************************************************/
void RscTop::WriteSrcHeader( const RSCINST & rInst, FILE * fOutput,
                             RscTypCont * pTC, USHORT nTab,
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
|*    Beschreibung
|*    Ersterstellung    MM 08.04.91
|*    Letzte Aenderung  MM 08.04.91
|*
*************************************************************************/
void RscTop::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                       RscTypCont * pTC, USHORT nTab, const char * pVarName )
{
    if( pSuperClass )
        pSuperClass->WriteSrc( rInst, fOutput, pTC, nTab, pVarName );
}

/*************************************************************************
|*
|*    RscTop::WriteRcHeader()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.04.91
|*    Letzte Aenderung  MM 12.04.91
|*
*************************************************************************/
ERRTYPE RscTop::WriteRcHeader( const RSCINST & rInst, RscWriteRc & rMem,
                               RscTypCont * pTC, const RscId & rId,
                               USHORT nDeep, BOOL bExtra )
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
|*    Beschreibung
|*    Ersterstellung    MM 08.04.91
|*    Letzte Aenderung  MM 08.04.91
|*
*************************************************************************/
ERRTYPE RscTop::WriteRc( const RSCINST & rInst, RscWriteRc & rMem,
                         RscTypCont * pTC, USHORT nDeep, BOOL bExtra )
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
|*    Beschreibung
|*    Ersterstellung    MM 29.05.91
|*    Letzte Aenderung  MM 29.05.91
|*
*************************************************************************/
void RscTop::WriteSyntaxHeader( FILE * fOutput, RscTypCont * pTC )
{
    if( GetId() != HASH_NONAME )
    {
        fprintf( fOutput, "class %s \n{\n", pHS->Get( GetId() ) );
        WriteSyntax( fOutput, pTC );
        fprintf( fOutput, "};\n\n" );
    }
}

/*************************************************************************
|*
|*    RscTop::WriteSyntax()
|*
|*    Beschreibung
|*    Ersterstellung    MM 29.05.91
|*    Letzte Aenderung  MM 29.05.91
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
    RscTypCont * pTC,
    const char * pName
)
{
    if( GetId() != HASH_NONAME )
    {
        fprintf( fOutput, "\t\t//%s %s\n",
                pHS->Get( GetId() ), pName );
    }
}

//========================================================================
void RscTop::WriteRcCtor( FILE * fOutput, RscTypCont * pTC )
{
    if( pSuperClass )
        pSuperClass->WriteRcCtor( fOutput, pTC );
}


