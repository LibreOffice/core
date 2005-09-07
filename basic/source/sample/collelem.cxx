/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: collelem.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:43:36 $
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

#ifndef _ERRCODE_HXX //autogen
#include <tools/errcode.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <sbx.hxx>
#endif
#pragma hdrstop
#include "collelem.hxx"

// Das Sample-Element ist ein kleines Objekt, das die Properties
// Name und Value enth„lt sowie die Methode Say, die den bergebenen
// Text mit dem eigenen Namen verkoppelt und ausgibt.

SampleElement::SampleElement( const String& r ) : SbxObject( r )
{
    // Methode Say mit einem String-Parameter
    SbxVariable* pMeth = Make( String( RTL_CONSTASCII_USTRINGPARAM("Say") ), SbxCLASS_METHOD, SbxEMPTY );
    pMeth->SetUserData( 0x12345678 );
    pMeth->ResetFlag( SBX_FIXED );
    SbxInfo* pInfo = new SbxInfo;
    pInfo->AddParam( String( RTL_CONSTASCII_USTRINGPARAM("text") ), SbxSTRING, SBX_READ );
    pMeth->SetInfo( pInfo );
}

void SampleElement::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                const SfxHint& rHint, const TypeId& rHintType )
{
    const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbxArray* pPar = pVar->GetParameters();
        ULONG t = pHint->GetId();
        if( t == SBX_HINT_DATAWANTED && pVar->GetUserData() == 0x12345678 )
        {
            // Die Say-Methode:
            // 1 Parameter + Returnwert
            if( !pPar || pPar->Count() != 2 )
                SetError( SbxERR_WRONG_ARGS );
            else
            {
                String s( GetName() );
                s.AppendAscii( " says: " );
                s += pPar->Get( 1 )->GetString();
                // Aus Gag: den String zurueckliefern
                SbxVariable *pRet = pPar->Get( 0 );
                pPar->Get( 0 )->SetType(SbxSTRING);
                pPar->Get( 0 )->PutString( s );
                InfoBox( NULL, s ).Execute();
            }
            return;
        }
        SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
    }
}

