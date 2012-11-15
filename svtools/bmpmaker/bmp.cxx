/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <rtl/math.hxx>
#include <rtl/strbuf.hxx>

#include <math.h>
#include <stdio.h>
#include <signal.h>
#include <vector>
using namespace std;

#include <vcl/svapp.hxx>

#include "svl/solar.hrc"
#include "bmpcore.hxx"
#include "bmp.hrc"

// ----------
// - BmpApp -
// ----------

class BmpApp : public BmpCreator
{
private:

    String          aOutputFileName;
    sal_uInt8           cExitCode;

    sal_Bool            GetCommandOption( const ::std::vector< String >& rArgs, const String& rSwitch, String& rSwitchParam );
    sal_Bool            GetCommandOptions( const ::std::vector< String >& rArgs, const String& rSwitch, ::std::vector< String >& rSwitchParams );

    void            SetExitCode( sal_uInt8 cExit )
                    {
                        if( ( EXIT_NOERROR == cExitCode ) || ( cExit != EXIT_NOERROR ) )
                            cExitCode = cExit;
                    }
    void            ShowUsage();

    virtual void    Message( const String& rText, sal_uInt8 cExitCode );

public:

                    BmpApp();
                    ~BmpApp();

    int             Start( const ::std::vector< String >& rArgs );
};

// -----------------------------------------------------------------------------

BmpApp::BmpApp()
{
}

// -----------------------------------------------------------------------------

BmpApp::~BmpApp()
{
}

// -----------------------------------------------------------------------

sal_Bool BmpApp::GetCommandOption( const ::std::vector< String >& rArgs, const String& rSwitch, String& rParam )
{
    sal_Bool bRet = sal_False;

    for( int i = 0, nCount = rArgs.size(); ( i < nCount ) && !bRet; i++ )
    {
        rtl::OUString  aTestStr( '-' );

        for( int n = 0; ( n < 2 ) && !bRet; n++ )
        {
            aTestStr += rSwitch;

            if( aTestStr.equalsIgnoreAsciiCase( rArgs[ i ] ) )
            {
                bRet = sal_True;

                if( i < ( nCount - 1 ) )
                    rParam = rArgs[ i + 1 ];
                else
                    rParam = String();
            }

            if( 0 == n )
                aTestStr = rtl::OUString('/');
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------

sal_Bool BmpApp::GetCommandOptions( const ::std::vector< String >& rArgs, const String& rSwitch, ::std::vector< String >& rParams )
{
    sal_Bool bRet = sal_False;

    for( int i = 0, nCount = rArgs.size(); ( i < nCount ); i++ )
    {
        rtl::OUString  aTestStr( '-' );

        for( int n = 0; ( n < 2 ) && !bRet; n++ )
        {
            aTestStr += rSwitch;

            if( aTestStr.equalsIgnoreAsciiCase( rArgs[ i ] ) )
            {
                if( i < ( nCount - 1 ) )
                    rParams.push_back( rArgs[ i + 1 ] );
                else
                    rParams.push_back( String() );

                break;
            }

            if( 0 == n )
                aTestStr = rtl::OUString('/');
        }
    }

    return( rParams.size() > 0 );
}

// -----------------------------------------------------------------------

void BmpApp::Message( const String& rText, sal_uInt8 cExit )
{
    if( EXIT_NOERROR != cExit )
        SetExitCode( cExit );

    rtl::OStringBuffer aText(rtl::OUStringToOString(rText, RTL_TEXTENCODING_UTF8));
    aText.append(RTL_CONSTASCII_STRINGPARAM("\r\n"));
    fprintf(stderr, "%s", aText.getStr());
}

// -----------------------------------------------------------------------------

void BmpApp::ShowUsage()
{
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "Usage:" ) ), EXIT_NOERROR );
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "    bmp srs_inputfile output_dir lang_dir lang_num -i input_dir [-i input_dir ][-f err_file]" ) ), EXIT_NOERROR );
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "Options:" ) ), EXIT_NOERROR );
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "   -i ...        name of directory to be searched for input files [multiple occurrence is possible]" ) ), EXIT_NOERROR );
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "   -f            name of file, output should be written to" ) ), EXIT_NOERROR );
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "Examples:" ) ), EXIT_NOERROR );
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "    bmp /home/test.srs /home/out enus 01 -i /home/res -f /home/out/bmp.err" ) ), EXIT_NOERROR );
}

// -----------------------------------------------------------------------------

int BmpApp::Start( const ::std::vector< String >& rArgs )
{
    String aOutName;

    cExitCode = EXIT_NOERROR;

    if( rArgs.size() >= 6 )
    {
        LangInfo                aLangInfo;
        sal_uInt16                  nCurCmd = 0;
        const String            aSrsName( rArgs[ nCurCmd++ ] );
        ::std::vector< String > aInDirVector;

        aOutName = rArgs[ nCurCmd++ ];

        rtl::OString aLangDir(rtl::OUStringToOString(rArgs[nCurCmd++], RTL_TEXTENCODING_ASCII_US));
        aLangInfo.mnLangNum = static_cast< sal_uInt16 >( rArgs[ nCurCmd++ ].ToInt32() );

        memcpy( aLangInfo.maLangDir, aLangDir.getStr(), aLangDir.getLength() + 1 );

        GetCommandOption( rArgs, rtl::OUString('f'), aOutputFileName );
        GetCommandOptions( rArgs, rtl::OUString('i'), aInDirVector );

        Create( aSrsName, aInDirVector, aOutName, aLangInfo );
    }
    else
    {
        ShowUsage();
        cExitCode = EXIT_COMMONERROR;
    }

    if( ( EXIT_NOERROR == cExitCode ) && aOutputFileName.Len() && aOutName.Len() )
    {
        SvFileStream    aOStm( aOutputFileName, STREAM_WRITE | STREAM_TRUNC );
        rtl::OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM("Successfully generated ImageList(s) in: "));
        aStr.append(rtl::OUStringToOString(aOutName, RTL_TEXTENCODING_UTF8));
        aOStm.WriteLine(aStr.makeStringAndClear());
        aOStm.Close();
    }

    return cExitCode;
}

// --------
// - Main -
// --------

int main( int nArgCount, char* ppArgs[] )
{
#ifdef UNX
    static char aDisplayVar[ 1024 ];

    strcpy( aDisplayVar, "DISPLAY=" );
    putenv( aDisplayVar );
#endif

    ::std::vector< String > aArgs;
    BmpApp                  aBmpApp;

    InitVCL();

    for( int i = 1; i < nArgCount; i++ )
        aArgs.push_back( String( ppArgs[ i ], RTL_TEXTENCODING_ASCII_US ) );

    return aBmpApp.Start( aArgs );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
