/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "rtlproto.hxx"
#include "sbdiagnose.hxx"

#include <basic/sbstar.hxx>

#include <tools/debug.hxx>
#include <comphelper/flagguard.hxx>

#ifdef DBG_UTIL

static DbgChannelId nRestoreChannelId = 0;
static DbgChannelId nAssertionChannelId = 0;
static StarBASICRef xAssertionChannelBasic;
static OUString sCaptureFunctionName;
static bool bReportingAssertion = false;

void ResetCapturedAssertions()
{
    if ( nRestoreChannelId != 0 )
    {
        DBG_INSTOUTERROR( nRestoreChannelId );
    }
    nRestoreChannelId = 0;
    xAssertionChannelBasic = NULL;
    sCaptureFunctionName = OUString();
    bReportingAssertion = false;
}

void DbgReportAssertion( const sal_Char* i_assertionMessage )
{
    if ( !xAssertionChannelBasic )
    {
        ResetCapturedAssertions();
        return;
    }

    
    if ( bReportingAssertion )
    {
        return;
    }
    ::comphelper::FlagRestorationGuard aGuard( bReportingAssertion, true );

    SbxArrayRef const xArguments( new SbxArray( SbxVARIANT ) );
    SbxVariableRef const xMessageText = new SbxVariable( SbxSTRING );
    xMessageText->PutString( OUString::createFromAscii(i_assertionMessage) );
    xArguments->Put( xMessageText, 1 );

    ErrCode const nError = xAssertionChannelBasic->Call( sCaptureFunctionName, xArguments );
    if ( ( nError & SbERR_METHOD_NOT_FOUND ) != 0 )
    {
        ResetCapturedAssertions();
    }
}

#endif


RTLFUNC(CaptureAssertions)
{
    (void)bWrite;

    
    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

#ifdef DBG_UTIL
    DBG_TESTSOLARMUTEX();

    OUString const sFunctionName = rPar.Get(1)->GetOUString();
    if ( sFunctionName.isEmpty() )
    {
        ResetCapturedAssertions();
        return;
    }

    if ( nAssertionChannelId == 0 )
    {
        
        
        nAssertionChannelId = DbgRegisterUserChannel( &DbgReportAssertion );
    }

    DbgChannelId const nCurrentChannelId = (DbgChannelId)DbgGetErrorOut();
    if ( nCurrentChannelId != nAssertionChannelId )
    {
        
        nRestoreChannelId = nCurrentChannelId;

        
        DBG_INSTOUTERROR( nAssertionChannelId );

        
        DbgData aData( *DbgGetData() );
        aData.bHookOSLAssert = true;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
