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


#include <stdio.h>
#include <unistd.h>
#include <memory>
#include <list>

#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>

#include <tools/urlobj.hxx>
#include <tools/fsys.hxx>

#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <vcl/font.hxx>
#include <vcl/print.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/help.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>

#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/bootstrap.hxx>

#include <svx/galtheme.hxx>
#include <svx/gallery1.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::lang;

typedef ::std::list<rtl::OUString> FileNameList;

class GalApp : public Application
{
public:
    virtual int Main();

protected:
    Reference<XMultiServiceFactory> xMSF;
    void Init();
};

Gallery* createGallery( const rtl::OUString& aGalleryURL )
{
    return new Gallery( aGalleryURL );
}

void disposeGallery( Gallery* pGallery )
{
    delete pGallery;
}

static void createTheme( rtl::OUString aThemeName,
                         rtl::OUString aGalleryURL,
                         rtl::OUString aDestDir,
                         sal_uInt32 nNumFrom,
                         FileNameList &rFiles )
{
    Gallery * pGallery( createGallery( aGalleryURL ) );

    if (!pGallery ) {
            fprintf( stderr, "Could't acquire '%s'\n",
                     rtl::OUStringToOString(aGalleryURL, RTL_TEXTENCODING_UTF8).getStr() );
            exit( 1 );
    }
    fprintf( stderr, "Work on gallery '%s'\n",
                     rtl::OUStringToOString(aGalleryURL, RTL_TEXTENCODING_UTF8).getStr() );

    fprintf( stderr, "Existing themes: %lu\n",
             sal::static_int_cast< unsigned long >(
                 pGallery->GetThemeCount() ) );

    if( !pGallery->HasTheme( aThemeName) ) {
            if( !pGallery->CreateTheme( aThemeName, nNumFrom ) ) {
                    fprintf( stderr, "Failed to create theme\n" );
                    disposeGallery( pGallery );
                    exit( 1 );
            }
    }

    fprintf( stderr, "Existing themes: %lu\n",
             sal::static_int_cast< unsigned long >(
                 pGallery->GetThemeCount() ) );

    SfxListener aListener;

    GalleryTheme *pGalTheme = pGallery->AcquireTheme( aThemeName, aListener );
    if ( pGalTheme == NULL  ) {
            fprintf( stderr, "Failed to acquire theme\n" );
            disposeGallery( pGallery );
            exit( 1 );
    }

    fprintf( stderr, "Using DestDir: %s\n",
             rtl::OUStringToOString(aDestDir, RTL_TEXTENCODING_UTF8).getStr() );
    pGalTheme->SetDestDir(String(aDestDir));

    FileNameList::const_iterator aIter;

    for( aIter = rFiles.begin(); aIter != rFiles.end(); ++aIter )
    {
//  Should/could use:
//  if ( ! pGalTheme->InsertFileOrDirURL( aURL ) ) {
//  Requires a load more components ...

        Graphic aGraphic;
        String aFormat;

        if ( ! pGalTheme->InsertURL( *aIter ) )
            fprintf( stderr, "Failed to import '%s'\n",
                     rtl::OUStringToOString(*aIter, RTL_TEXTENCODING_UTF8).getStr() );
        else
            fprintf( stderr, "Imported file '%s' (%lu)\n",
                     rtl::OUStringToOString(*aIter, RTL_TEXTENCODING_UTF8).getStr(),
                     sal::static_int_cast< unsigned long >(
                         pGalTheme->GetObjectCount() ) );
    }

    pGallery->ReleaseTheme( pGalTheme, aListener );
    disposeGallery( pGallery );
}

static void PrintHelp()
{
    fprintf( stdout, "Utility to generate OO.o gallery files\n\n" );

    fprintf( stdout, "using: gengal --name <name> --path <dir> [ --destdir <path> ]\n");
    fprintf( stdout, "              [ --number-from <num> ] [ files ... ]\n\n" );

    fprintf( stdout, "options:\n");
    fprintf( stdout, " --name <theme>\t\tdefines a name of the created or updated theme.\n");
    fprintf( stdout, " --path <dir>\t\tdefines directory where the gallery files are created\n");
    fprintf( stdout, "\t\t\tor updated.\n");
    fprintf( stdout, " --destdir <dir>\tdefines a path prefix to be removed from the paths\n");
    fprintf( stdout, "\t\t\tstored in the gallery files. It is useful to create\n");
    fprintf( stdout, "\t\t\tRPM packages using the BuildRoot feature.\n");
    fprintf( stdout, " --number-from <num>\tdefines minimal number for the newly created gallery\n");
    fprintf( stdout, "\t\t\ttheme files.\n");
    fprintf( stdout, " files\t\t\tlists files to be added to the gallery. Absolute paths\n");
    fprintf( stdout, "\t\t\tare required.\n");
}

static rtl::OUString Smartify( const rtl::OUString &rPath )
{
    INetURLObject aURL;
    aURL.SetSmartURL( rPath );
    return aURL.GetMainURL( INetURLObject::NO_DECODE );
}

void GalApp::Init()
{
    if( getenv( "OOO_INSTALL_PREFIX" ) == NULL ) {
        rtl::OUString fileName = GetAppFileName();
        int lastSlash = fileName.lastIndexOf( '/' );
#ifdef WNT
        // Don't know which directory separators GetAppFileName() returns on Windows.
        // Be safe and take into consideration they might be backslashes.
        if( fileName.lastIndexOf( '\\' ) > lastSlash )
            lastSlash = fileName.lastIndexOf( '\\' );
#endif
        rtl::OUString baseBinDir = fileName.copy( 0, lastSlash );
        rtl::OUString installPrefix = baseBinDir + rtl::OUString("/../..");

        rtl::OUString envVar("OOO_INSTALL_PREFIX");
        osl_setEnvironment(envVar.pData, installPrefix.pData);
    }
    OSL_TRACE( "OOO_INSTALL_PREFIX=%s", getenv( "OOO_INSTALL_PREFIX" ) );

    Reference<XComponentContext> xComponentContext
        = ::cppu::defaultBootstrap_InitialComponentContext();
    xMSF = Reference<XMultiServiceFactory>
        ( xComponentContext->getServiceManager(), UNO_QUERY );
    if( !xMSF.is() )
        fprintf( stderr, "Failed to bootstrap\n" );
    ::comphelper::setProcessServiceFactory( xMSF );

    // For backwards compatibility, in case some code still uses plain
    // createInstance w/o args directly to obtain an instance:
    com::sun::star::ucb::UniversalContentBroker::create(xComponentContext);
}

int GalApp::Main()
{
    bool bHelp = false;
    rtl::OUString aPath, aDestDir;
    rtl::OUString aName("Default name");
    sal_uInt32 nNumFrom = 0;
    FileNameList aFiles;

    for( sal_uInt16 i = 0; i < GetCommandLineParamCount(); i++ )
    {
        rtl::OUString aParam = GetCommandLineParam( i );

        if( aParam.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "--help" ) ) ||
            aParam.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-h" ) ) )
                bHelp = true;

        else if ( aParam == "--name" )
            aName = GetCommandLineParam( ++i );

        else if ( aParam == "--path" )
            aPath = Smartify( GetCommandLineParam( ++i ) );

        else if ( aParam == "--destdir" )
            aDestDir = GetCommandLineParam( ++i );

        else if ( aParam == "--number-from" )
             nNumFrom = GetCommandLineParam( ++i ).ToInt32();

        else
            aFiles.push_back( Smartify( aParam ) );
    }

    if( bHelp )
    {
        PrintHelp();
        return EXIT_SUCCESS;
    }

    createTheme( aName, aPath, aDestDir, nNumFrom, aFiles );
    return EXIT_SUCCESS;
}

GalApp aGalApp;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
