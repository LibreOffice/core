/*************************************************************************
 *
 *  $RCSfile: codegen.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:10 $
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

#include <svtools/sbx.hxx>
#include "sbcomp.hxx"
#pragma hdrstop
#include "image.hxx"

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBCOMP, SBCOMP_CODE )

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
        if( pProc && pProc->IsPublic() && pProc->IsDefined() )
        {
            SbMethod* pMeth = rMod.GetMethod( pProc->GetName(), pProc->GetType() );
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
            }
            pMeth->SetInfo( pInfo );
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

    if( !p->IsError() )
        rMod.pImage = p;
    else
        delete p;

    rMod.EndDefinitions();
}

