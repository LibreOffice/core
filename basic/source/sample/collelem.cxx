/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"
#include <tools/errcode.hxx>
#include <vcl/msgbox.hxx>
#include <basic/sbx.hxx>
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
    SbxInfo* pInfo_ = new SbxInfo;
    pInfo_->AddParam( String( RTL_CONSTASCII_USTRINGPARAM("text") ), SbxSTRING, SBX_READ );
    pMeth->SetInfo( pInfo_ );
}

void SampleElement::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                const SfxHint& rHint, const TypeId& rHintType )
{
    const SbxHint* pHint = dynamic_cast< const SbxHint* >( &rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbxArray* pPar_ = pVar->GetParameters();
        sal_uIntPtr t = pHint->GetId();
        if( t == SBX_HINT_DATAWANTED && pVar->GetUserData() == 0x12345678 )
        {
            // Die Say-Methode:
            // 1 Parameter + Returnwert
            if( !pPar_ || pPar_->Count() != 2 )
                SetError( SbxERR_WRONG_ARGS );
            else
            {
                String s( GetName() );
                s.AppendAscii( " says: " );
                s += pPar_->Get( 1 )->GetString();
                pPar_->Get( 0 )->SetType(SbxSTRING);
                pPar_->Get( 0 )->PutString( s );
                InfoBox( NULL, s ).Execute();
            }
            return;
        }
        SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
    }
}

