/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: codegen.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:27:37 $
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

#include <sbx.hxx>
#include "sbcomp.hxx"
#pragma hdrstop
#include "image.hxx"

// nInc ist die Inkrementgroesse der Puffer

SbiCodeGen::SbiCodeGen( SbModule& r, SbiParser* p, short nInc )
         : rMod( r ), aCode( p, nInc )
{
    pParser = p;
    bStmnt = FALSE;
    nLine = 0;
    nCol = 0;
    nForLevel = 0;
}

USHORT SbiCodeGen::GetPC()
{
    return aCode.GetSize();
}

// Statement merken

void SbiCodeGen::Statement()
{
    bStmnt = TRUE;

    nLine = pParser->GetLine();
    nCol  = pParser->GetCol1();

    // #29955 Information der for-Schleifen-Ebene
    // in oberen Byte der Spalte speichern
    nCol = (nCol & 0xff) + 0x100 * nForLevel;
}

// Anfang eines Statements markieren

void SbiCodeGen::GenStmnt()
{
    if( bStmnt )
    {
        bStmnt = FALSE;
        Gen( _STMNT, nLine, nCol );
    }
}

// Die Gen-Routinen returnen den Offset des 1. Operanden,
// damit Jumps dort ihr Backchain versenken koennen

USHORT SbiCodeGen::Gen( SbiOpcode eOpcode )
{
#ifndef PRODUCT
    if( eOpcode < SbOP0_START || eOpcode > SbOP0_END )
        pParser->Error( SbERR_INTERNAL_ERROR, "OPCODE1" );
#endif
    GenStmnt();
    aCode += (UINT8) eOpcode;
    return GetPC();
}

USHORT SbiCodeGen::Gen( SbiOpcode eOpcode, UINT16 nOpnd )
{
#ifndef PRODUCT
    if( eOpcode < SbOP1_START || eOpcode > SbOP1_END )
        pParser->Error( SbERR_INTERNAL_ERROR, "OPCODE2" );
#endif
    GenStmnt();
    aCode += (UINT8) eOpcode;
    USHORT n = GetPC();
    aCode += nOpnd;
    return n;
}

USHORT SbiCodeGen::Gen( SbiOpcode eOpcode, UINT16 nOpnd1, UINT16 nOpnd2 )
{
#ifndef PRODUCT
    if( eOpcode < SbOP2_START || eOpcode > SbOP2_END )
        pParser->Error( SbERR_INTERNAL_ERROR, "OPCODE3" );
#endif
    GenStmnt();
    aCode += (UINT8) eOpcode;
    USHORT n = GetPC();
    aCode += nOpnd1;
    aCode += nOpnd2;
    return n;
}

// Abspeichern des erzeugten Images im Modul

void SbiCodeGen::Save()
{
    SbiImage* p = new SbiImage;
    if( !p )
    {
        SbERR_NO_MEMORY; return;
    }
    rMod.StartDefinitions();
    // OPTION BASE-Wert:
    p->nDimBase = pParser->nBase;
    // OPTION EXPLICIT-Flag uebernehmen
    if( pParser->bExplicit )
        p->SetFlag( SBIMG_EXPLICIT );

    int nIfaceCount = 0;
    if( pParser->bClassModule )
    {
        p->SetFlag( SBIMG_CLASSMODULE );
        pCLASSFAC->AddClassModule( &rMod );

        nIfaceCount = pParser->aIfaceVector.size();
        if( nIfaceCount )
        {
            if( !rMod.pClassData )
                rMod.pClassData = new SbClassData;

            for( int i = 0 ; i < nIfaceCount ; i++ )
            {
                const String& rIfaceName = pParser->aIfaceVector[i];
                SbxVariable* pIfaceVar = new SbxVariable( SbxVARIANT );
                pIfaceVar->SetName( rIfaceName );
                SbxArray* pIfaces = rMod.pClassData->mxIfaces;
                pIfaces->Insert( pIfaceVar, pIfaces->Count() );
            }
        }
    }
    else
    {
        pCLASSFAC->RemoveClassModule( &rMod );
    }
    if( pParser->bText )
        p->SetFlag( SBIMG_COMPARETEXT );
    // GlobalCode-Flag
    if( pParser->HasGlobalCode() )
        p->SetFlag( SBIMG_INITCODE );
    // Die Entrypoints:
    for( SbiSymDef* pDef = pParser->aPublics.First(); pDef;
                   pDef = pParser->aPublics.Next() )
    {
        SbiProcDef* pProc = pDef->GetProcDef();
        if( pProc && pProc->IsDefined() )
        {
            String aProcName = pProc->GetName();
            String aIfaceProcName;
            String aIfaceName;
            USHORT nPassCount = 1;
            if( nIfaceCount )
            {
                int nPropPrefixFound =
                    aProcName.Search( String( RTL_CONSTASCII_USTRINGPARAM("Property ") ) );
                String aPureProcName = aProcName;
                String aPropPrefix;
                if( nPropPrefixFound == 0 )
                {
                    aPropPrefix = aProcName.Copy( 0, 13 );      // 13 == Len( "Property ?et " )
                    aPureProcName = aProcName.Copy( 13 );
                }
                for( int i = 0 ; i < nIfaceCount ; i++ )
                {
                    const String& rIfaceName = pParser->aIfaceVector[i];
                    int nFound = aPureProcName.Search( rIfaceName );
                    if( nFound == 0 && '_' == aPureProcName.GetChar( rIfaceName.Len() ) )
                    {
                        if( nPropPrefixFound == 0 )
                            aIfaceProcName += aPropPrefix;
                        aIfaceProcName += aPureProcName.Copy( rIfaceName.Len() + 1 );
                        aIfaceName = rIfaceName;
                        nPassCount = 2;
                        break;
                    }
                }
            }
            SbMethod* pMeth;
            for( USHORT nPass = 0 ; nPass < nPassCount ; nPass++ )
            {
                if( nPass == 1 )
                    aProcName = aIfaceProcName;

                PropertyMode ePropMode = pProc->getPropertyMode();
                if( ePropMode != PROPERTY_MODE_NONE )
                {
                    SbxDataType ePropType;
                    switch( ePropMode )
                    {
                        case PROPERTY_MODE_GET:
                            ePropType = pProc->GetType();
                            break;
                        case PROPERTY_MODE_LET:
                        {
                            // type == type of first parameter
                            ePropType = SbxVARIANT;     // Default
                            SbiSymPool* pPool = &pProc->GetParams();
                            if( pPool->GetSize() > 1 )
                            {
                                SbiSymDef* pPar = pPool->Get( 1 );
                                if( pPar )
                                    ePropType = pPar->GetType();
                            }
                            break;
                        }
                        case PROPERTY_MODE_SET:
                            ePropType = SbxOBJECT;
                            break;
                    }
                    String aPropName = pProc->GetPropName();
                    if( nPass == 1 )
                        aPropName = aPropName.Copy( aIfaceName.Len() + 1 );
                    SbProcedureProperty* pProcedureProperty =
                        rMod.GetProcedureProperty( aPropName, ePropType );
                        // rMod.GetProcedureProperty( pProc->GetPropName(), ePropType );
                }
                if( nPass == 1 )
                {
                    SbIfaceMapperMethod* pMapperMeth =
                        rMod.GetIfaceMapperMethod( aProcName, pMeth );
                }
                else
                {
                    pMeth = rMod.GetMethod( aProcName, pProc->GetType() );

                    // #110004
                    if( !pProc->IsPublic() )
                        pMeth->SetFlag( SBX_PRIVATE );

                    pMeth->nStart = pProc->GetAddr();
                    pMeth->nLine1 = pProc->GetLine1();
                    pMeth->nLine2 = pProc->GetLine2();
                    // Die Parameter:
                    SbxInfo* pInfo = pMeth->GetInfo();
                    String aHelpFile, aComment;
                    ULONG nHelpId = 0;
                    if( pInfo )
                    {
                        // Die Zusatzdaten retten
                        aHelpFile = pInfo->GetHelpFile();
                        aComment  = pInfo->GetComment();
                        nHelpId   = pInfo->GetHelpId();
                    }
                    // Und die Parameterliste neu aufbauen
                    pInfo = new SbxInfo( aHelpFile, nHelpId );
                    pInfo->SetComment( aComment );
                    SbiSymPool* pPool = &pProc->GetParams();
                    // Das erste Element ist immer der Funktionswert!
                    for( USHORT i = 1; i < pPool->GetSize(); i++ )
                    {
                        SbiSymDef* pPar = pPool->Get( i );
                        SbxDataType t = pPar->GetType();
                        if( !pPar->IsByVal() )
                            t = (SbxDataType) ( t | SbxBYREF );
                        if( pPar->GetDims() )
                            t = (SbxDataType) ( t | SbxARRAY );
                        // #33677 Optional-Info durchreichen
                        USHORT nFlags = SBX_READ;
                        if( pPar->IsOptional() )
                            nFlags |= SBX_OPTIONAL;

                        pInfo->AddParam( pPar->GetName(), t, nFlags );

                        UINT32 nUserData = 0;
                        USHORT nDefaultId = pPar->GetDefaultId();
                        if( nDefaultId )
                            nUserData |= nDefaultId;
                        if( pPar->IsParamArray() )
                            nUserData |= PARAM_INFO_PARAMARRAY;
                        if( nUserData )
                        {
                            SbxParamInfo* pParam = (SbxParamInfo*)pInfo->GetParam( i );
                            pParam->nUserData = nUserData;
                        }
                    }
                    pMeth->SetInfo( pInfo );
                }

            }   // for( iPass...
        }
    }
    // Der Code
    p->AddCode( aCode.GetBuffer(), aCode.GetSize() );

    // Der globale StringPool. 0 ist nicht belegt.
    SbiStringPool* pPool = &pParser->aGblStrings;
    USHORT nSize = pPool->GetSize();
    p->MakeStrings( nSize );
    USHORT i;
    for( i = 1; i <= nSize; i++ )
        p->AddString( pPool->Find( i ) );

    // Typen einfuegen
    USHORT nCount = pParser->rTypeArray->Count();
    for (i = 0; i < nCount; i++)
         p->AddType((SbxObject *)pParser->rTypeArray->Get(i));

    // Insert enum objects
    nCount = pParser->rEnumArray->Count();
    for (i = 0; i < nCount; i++)
         p->AddEnum((SbxObject *)pParser->rEnumArray->Get(i));

    if( !p->IsError() )
        rMod.pImage = p;
    else
        delete p;

    rMod.EndDefinitions();
}

