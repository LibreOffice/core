/*************************************************************************
 *
 *  $RCSfile: collelem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:11 $
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

#ifndef _ERRCODE_HXX //autogen
#include <tools/errcode.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
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

