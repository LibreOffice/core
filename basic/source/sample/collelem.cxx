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
    const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
