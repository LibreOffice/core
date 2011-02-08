/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2011 Oracle and/or its affiliates.
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

#include "precompiled_basic.hxx"

#include "rtlproto.hxx"
#include "sbdiagnose.hxx"

#include "basic/sbstar.hxx"

#include <tools/debug.hxx>
#include <comphelper/flagguard.hxx>

#ifdef DBG_UTIL

static DbgChannelId nRestoreChannelId = 0;
static DbgChannelId nAssertionChannelId = 0;
static StarBASICRef xAssertionChannelBasic;
static String sCaptureFunctionName;
static bool bReportingAssertion = false;

void ResetCapturedAssertions()
{
    DBG_INSTOUTERROR( nRestoreChannelId != 0 ? nRestoreChannelId : DBG_OUT_NULL );
    nRestoreChannelId = 0;
    xAssertionChannelBasic = NULL;
    sCaptureFunctionName = String();
    bReportingAssertion = false;
}

void DbgReportAssertion( const sal_Char* i_assertionMessage )
{
    if ( !xAssertionChannelBasic )
    {
        ResetCapturedAssertions();
        return;
    }

    // prevent infinite recursion
    if ( bReportingAssertion )
        return;
    ::comphelper::FlagRestorationGuard aGuard( bReportingAssertion, true );

    SbxArrayRef const xArguments( new SbxArray( SbxVARIANT ) );
    SbxVariableRef const xMessageText = new SbxVariable( SbxSTRING );
    xMessageText->PutString( String::CreateFromAscii( i_assertionMessage ) );
    xArguments->Put( xMessageText, 1 );

    ErrCode const nError = xAssertionChannelBasic->Call( sCaptureFunctionName, xArguments );
    if ( ( nError & SbERR_METHOD_NOT_FOUND ) != 0 )
        ResetCapturedAssertions();
}

#endif

/// capture assertions, route them to the given given Basic function
RTLFUNC(CaptureAssertions)
{
    (void)bWrite;

    // need exactly one argument
    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

#ifdef DBG_UTIL
    DBG_TESTSOLARMUTEX();

    String const sFunctionName = rPar.Get(1)->GetString();
    if ( sFunctionName.Len() == 0 )
    {
        ResetCapturedAssertions();
        return;
    }

    if ( nAssertionChannelId == 0 )
    {
        // TODO: should we register a named channel at the VCL API, instead of an unnamed channel at the tools API?
        // A named channel would mean it would appear in the nonpro-debug-options dialog
        nAssertionChannelId = DbgRegisterUserChannel( &DbgReportAssertion );
    }

    DbgChannelId const nCurrentChannelId = (DbgChannelId)DbgGetErrorOut();
    if ( nCurrentChannelId != nAssertionChannelId )
    {
        // remember the current channel
        nRestoreChannelId = nCurrentChannelId;

        // set the new channel
        DBG_INSTOUTERROR( nAssertionChannelId );

        // ensure OSL assertions are captured, too
        DbgData aData( *DbgGetData() );
        aData.bHookOSLAssert = TRUE;
        DbgUpdateOslHook( &aData );
    }

    xAssertionChannelBasic = pBasic;
    sCaptureFunctionName = sFunctionName;
#else
    (void)pBasic;
    (void)rPar;
    (void)bWrite;
#endif
}

