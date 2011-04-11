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

#include <vcl/svapp.hxx>

// This works and was used before for standalone test, not sure why
// we'd want it.
#define LAYOUT_WEAK 1
#include "uno.hxx"

#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>

#include <com/sun/star/xml/sax/SAXException.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <rtl/ustring.hxx>
#include <ucbhelper/configurationkeys.hxx>
#include <ucbhelper/contentbroker.hxx>

#define SORT_DLG 1 /* requires sfx2, svx to be compiled */
#if SORT_DLG
#include "scitems.hxx"
#include "uiitems.hxx"
#endif /* SORT_DLG */

#include "editor.hxx"

#include "plugin.hxx"
#undef _LAYOUT_POST_HXX

#include "recover.hxx"
#undef _LAYOUT_POST_HXX

#if SORT_DLG
#include "sortdlg.hxx"
#undef _LAYOUT_POST_HXX
#endif /* SORT_DLG */

#include "wordcountdialog.hxx"
#undef _LAYOUT_POST_HXX

#include "zoom.hxx"
#undef _LAYOUT_POST_HXX

#include <layout/layout-pre.hxx>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class LayoutTest : public Application
{
    Reference< XComponentContext > mxContext;
    Reference< lang::XMultiServiceFactory > mxMSF;
    OUString mInstallDir;
    OUString mTestDialog;
    bool mEditMode;
    std::list< OUString > mFiles;

public:
    LayoutTest( char const* installDir );

    void RunEditor();
    void RunFiles();
    void ExceptionalMain();
    void Init();
    void InitUCB();
    void LoadFile( OUString const &aName );
    void Main();
    void ParseCommandLine();
};

static void usage()
{
    fprintf (stderr, "usage: test [--inst OOO_INSTALL_PREFIX] [DIALOG.XML]... | --test [DIALOG.XML]\n" );
    exit( 2 );
}

static uno::Reference< lang::XSingleServiceFactory > get_factory( char const *service )
{
    uno::Reference< lang::XSingleServiceFactory > xFactory(
        comphelper::createProcessComponent(
            rtl::OUString::createFromAscii( service ) ), uno::UNO_QUERY );

    if ( !xFactory.is() )
        fprintf( stderr, "error loading: %s\n", service );
    return xFactory;
}

#define GET_FACTORY(x) get_factory( #x )

void LayoutTest::LoadFile( const OUString &aTestFile )
{
    fprintf( stderr, "TEST: layout instance\n" );

    uno::Reference< lang::XSingleServiceFactory > xFactory
        = GET_FACTORY( com.sun.star.awt.Layout );
    if ( !xFactory.is() )
    {
        fprintf( stderr, "Layout engine not installed\n" );
        throw uno::RuntimeException(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Layout engine not installed" ) ),
            uno::Reference< uno::XInterface >() );
    }
    fprintf( stderr, "TEST: initing root\n" );

    uno::Sequence< uno::Any > aParams( 1 );
    aParams[0] <<= aTestFile;

    uno::Reference< awt::XLayoutRoot > xRoot (
        xFactory->createInstanceWithArguments( aParams ),
        uno::UNO_QUERY );

    fprintf( stderr, "TEST: file loaded\n" );
}

void LayoutTest::InitUCB()
{
    OUString aEmpty;
    Sequence< Any > aArgs( 6 );
    aArgs[0]
        <<= OUString(RTL_CONSTASCII_USTRINGPARAM( UCB_CONFIGURATION_KEY1_LOCAL ));
    aArgs[1]
        <<= OUString(RTL_CONSTASCII_USTRINGPARAM( UCB_CONFIGURATION_KEY2_OFFICE ));
    aArgs[2] <<= OUString(RTL_CONSTASCII_USTRINGPARAM("PIPE"));
    aArgs[3] <<= aEmpty;
    aArgs[4] <<= OUString(RTL_CONSTASCII_USTRINGPARAM("PORTAL"));
    aArgs[5] <<= aEmpty;

    if ( !::ucbhelper::ContentBroker::initialize( mxMSF, aArgs ) )
    {
        fprintf( stderr, "Failed to init content broker\n" );
        fprintf( stderr, "arg[0]: %s\n", UCB_CONFIGURATION_KEY1_LOCAL );
        fprintf( stderr, "arg[1]: %s\n", UCB_CONFIGURATION_KEY2_OFFICE );
    }
}

static void support_upstream_brand_prefix ()
{
    if ( char const* inst = getenv( "OOO_INSTALL_PREFIX" ) )
    {
        char const *brand_prefix = "/openoffice.org3";
        OUString brand_dir = OUString::createFromAscii( inst )
            + OUString::createFromAscii( brand_prefix );
        struct stat stat_info;
        if ( !stat ( OUSTRING_CSTR( brand_dir ), &stat_info ) )
        {
            OSL_TRACE( "Appending %s to OOO_INSTALL_PREFIX", brand_prefix );
            setenv( "OOO_INSTALL_PREFIX", OUSTRING_CSTR( brand_dir ), 1 );
        }
    }
}

void LayoutTest::Init()
{
    ParseCommandLine();
    setenv( "OOO_INSTALL_PREFIX", OUSTRING_CSTR( mInstallDir ), 0 );
    support_upstream_brand_prefix ();
    OSL_TRACE( "OOO_INSTALL_PREFIX=%s", getenv( "OOO_INSTALL_PREFIX" ) );

    mxContext = defaultBootstrap_InitialComponentContext();
    mxMSF = new UnoBootstrapLayout( Reference< lang::XMultiServiceFactory >( mxContext->getServiceManager(), UNO_QUERY ) );
    ::comphelper::setProcessServiceFactory( mxMSF );
    InitUCB();
}

void LayoutTest::ParseCommandLine()
{
    printf ("%s\n", __PRETTY_FUNCTION__);
    for ( sal_uInt16 i = 0; i < GetCommandLineParamCount(); i++ )
    {
        OUString aParam = OUString( GetCommandLineParam( i ) );
        if ( aParam.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-h" ) ) || aParam.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "--help" ) ) )
            usage();
        if ( aParam.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "--inst" ) ) )
        {
            if ( i >= GetCommandLineParamCount() - 1)
                usage();
            mInstallDir = GetCommandLineParam( ++i );
            setenv( "OOO_INSTALL_PREFIX", OUSTRING_CSTR( mInstallDir ), 1 );
        }
        else if ( aParam.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "--test" ) ) )
        {
            mTestDialog = OUString(RTL_CONSTASCII_USTRINGPARAM("zoom"));
            if (i + 1 < GetCommandLineParamCount())
                mTestDialog = GetCommandLineParam( ++i );
        }
        else if ( aParam.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "--editor" ) ) )
            mEditMode = true;
        else
            mFiles.push_back( aParam );
    }

    if ( mFiles.size() <= 0 )
        mFiles.push_back( OUString(RTL_CONSTASCII_USTRINGPARAM("layout.xml")) );
}

void LayoutTest::RunEditor()
{
    OUString aFile;
    if ( !mFiles.empty()
        && mFiles.front().compareToAscii( "layout.xml" ) != 0 )
        aFile = mFiles.front();
    Editor editor( mxMSF, aFile );
    editor.Show();
    editor.Execute();
}

short RunDialog( Dialog& dialog )
{
    dialog.Show();
    short result = dialog.Execute();
    fprintf( stderr, "Done: dialog execute exited:%d\n", result);
    return result;
}

#undef Dialog
short RunDialog( ::Dialog& dialog )
{
    dialog.Show();
    short result = dialog.Execute();
    fprintf( stderr, "Done: dialog execute exited:%d\n", result);
    return result;
}

#if SORT_DLG
static void LoadSC()
{
    get_factory( "com.sun.star.comp.sfx2.DocumentTemplates" );
    get_factory( "com.sun.star.comp.Calc.SpreadsheetDocument" );
    GET_FACTORY( com.sun.star.i18n.Transliteration.l10n );
}
#endif /* SORT_DLG */

void TestDialog( OUString const& name )
{
    if ( 0 )
        ;
    else if ( name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "plugin" ) ) )
    {
        PluginDialog plugin ( 0 );
        RunDialog( plugin );
    }
    else if ( name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "query" ) ) )
    {
        QueryBox query ( 0, "Do you want to do?", "do");
        RunDialog( query );
    }
    else if ( name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "query-compat" ) ) )
    {
        QueryBox query ( 0,
                         WinBits( WB_YES_NO | WB_DEF_YES ),
//                         WinBits( WB_ABORT_RETRY_IGNORE ),
                         OUString(RTL_CONSTASCII_USTRINGPARAM ("Do you want to do?")));
        RunDialog( query );
    }
    else if ( name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "recover" ) ) )
    {
        SvxRecoverDialog recover ( 0 );
        RunDialog( recover );
    }
#if SORT_DLG
    else if ( name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "sort" ) ) )
    {
        LoadSC();
        ScSortDlg sort (0, 0);
        RunDialog( sort );
    }
#endif /* SORT_DLG */
    else if ( name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "wordcount" ) ) )
    {
        SwWordCountDialog words ( 0 );
        RunDialog( words );
    }
    else if ( name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "zoom" ) ) )
    {
        SvxZoomDialog zoom( 0 );
        RunDialog( zoom );
    }
}

void LayoutTest::RunFiles()
{
    fprintf( stderr, "TEST: loading files\n" );
    for ( std::list< OUString >::iterator  i = mFiles.begin(); i != mFiles.end(); ++i )
        LoadFile( *i );
    fprintf( stderr, "TEST: executing\n" );
    Execute();
    fprintf( stderr, "TEST: done executing\n" );
}

void LayoutTest::ExceptionalMain()
{
    if ( mTestDialog.getLength() )
        TestDialog( mTestDialog );
    else if ( mEditMode )
        RunEditor();
    else
        RunFiles();
}

void LayoutTest::Main()
{
    try
    {
        ExceptionalMain();
    }
    catch (xml::sax::SAXException & rExc)
    {
        OString aStr( OUStringToOString( rExc.Message,
                                         RTL_TEXTENCODING_ASCII_US ) );
        uno::Exception exc;
        if (rExc.WrappedException >>= exc)
        {
            aStr += OString( " >>> " );
            aStr += OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US );
        }
        fprintf (stderr, "Parsing error: '%s'\n", aStr.getStr());
        OSL_FAIL( aStr.getStr() );
    }
    catch ( uno::Exception & rExc )
    {
        OString aStr( OUStringToOString( rExc.Message,
                                         RTL_TEXTENCODING_ASCII_US ) );
        fprintf (stderr, "UNO error: '%s'\n", aStr.getStr());
        OSL_FAIL( aStr.getStr() );
    }

    Reference< lang::XComponent > xComp( mxContext, UNO_QUERY );
    if ( xComp.is() )
        xComp->dispose();
}

LayoutTest::LayoutTest( char const* installDir )
    : mInstallDir( OUString::createFromAscii ( installDir ) )
{
}

LayoutTest layout_test( "/usr/local/lib/ooo" );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
