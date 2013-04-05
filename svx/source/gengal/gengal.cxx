/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <unistd.h>

#include <list>

#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>

#include <tools/urlobj.hxx>
#include <vcl/vclmain.hxx>

#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/bootstrap.hxx>
#include <vcl/svapp.hxx>

#include <svx/galtheme.hxx>
#include <svx/gallery1.hxx>

using namespace ::com::sun::star;

typedef ::std::list<OUString> FileNameList;

class GalApp : public Application
{
public:
    virtual int Main();

protected:
    uno::Reference<lang::XMultiServiceFactory> xMSF;
    void Init();
};

Gallery* createGallery( const OUString& rURL )
{
    return new Gallery( rURL );
}

void disposeGallery( Gallery* pGallery )
{
    delete pGallery;
}

static void createTheme( OUString aThemeName, OUString aGalleryURL,
                         OUString aDestDir, sal_uInt32 nNumFrom,
                         FileNameList &rFiles )
{
    Gallery* pGallery;

    pGallery = createGallery( aGalleryURL );
    if (!pGallery ) {
            fprintf( stderr, "Could't create '%s'\n",
                     OUStringToOString( aGalleryURL, RTL_TEXTENCODING_UTF8 ).getStr() );
            exit( 1 );
    }
    fprintf( stderr, "Work on gallery '%s'\n",
             OUStringToOString( aGalleryURL, RTL_TEXTENCODING_UTF8 ).getStr() );

    fprintf( stderr, "Existing themes: %d\n",
             pGallery->GetThemeCount() );

    GalleryTheme *pGalTheme;
    if( !pGallery->HasTheme( aThemeName) ) {
            if( !pGallery->CreateTheme( aThemeName, nNumFrom ) ) {
                    fprintf( stderr, "Failed to create theme\n" );
                    exit( 1 );
            }
    }

    fprintf( stderr, "Existing themes: %d\n",
             pGallery->GetThemeCount() );

    SfxListener aListener;

    if ( !( pGalTheme = pGallery->AcquireTheme( aThemeName, aListener ) ) ) {
            fprintf( stderr, "Failed to acquire theme\n" );
            exit( 1 );
    }

    fprintf( stderr, "Using DestDir: %s\n",
             OUStringToOString( aDestDir, RTL_TEXTENCODING_UTF8 ).getStr() );
    pGalTheme->SetDestDir(String(aDestDir));

    FileNameList::const_iterator aIter;

    for( aIter = rFiles.begin(); aIter != rFiles.end(); ++aIter )
    {
//  Should/could use:
//    if ( ! pGalTheme->InsertFileOrDirURL( aURL ) ) {
//    Requires a load more components ...

        Graphic aGraphic;

        if ( ! pGalTheme->InsertURL( *aIter ) )
            fprintf( stderr, "Failed to import '%s'\n",
                     OUStringToOString( *aIter, RTL_TEXTENCODING_UTF8 ).getStr() );
        else
            fprintf( stderr, "Imported file '%s' (%d)\n",
                     OUStringToOString( *aIter, RTL_TEXTENCODING_UTF8 ).getStr(),
                     pGalTheme->GetObjectCount() );
    }

    pGallery->ReleaseTheme( pGalTheme, aListener );

    disposeGallery( pGallery );
}

static int PrintHelp()
{
    fprintf( stdout, "Utility to generate LibreOffice gallery files\n\n" );

    fprintf( stdout, "using: gengal --name <name> --path <dir> [ --destdir <path> ]\n");
    fprintf( stdout, "              [ --number-from <num> ] [ files ... ]\n\n" );

    fprintf( stdout, "options:\n");
    fprintf( stdout, " --name <theme>\t\tdefines the user visible name of the created or updated theme.\n");
    fprintf( stdout, " --path <dir>\t\tdefines directory where the gallery files are created\n");
    fprintf( stdout, "\t\t\tor updated.\n");
    fprintf( stdout, " --destdir <dir>\tdefines a path prefix to be removed from the paths\n");
    fprintf( stdout, "\t\t\tstored in the gallery files. It is useful to create\n");
    fprintf( stdout, "\t\t\tRPM packages using the BuildRoot feature.\n");
    fprintf( stdout, " --number-from <num>\tdefines minimal number for the newly created gallery\n");
    fprintf( stdout, "\t\t\ttheme files.\n");
    fprintf( stdout, " files\t\t\tlists files to be added to the gallery. Absolute paths\n");
    fprintf( stdout, "\t\t\tare required.\n");

    return EXIT_SUCCESS;
}

static OUString Smartify( const OUString &rPath )
{
    INetURLObject aURL;
    aURL.SetSmartURL( rPath );
    return aURL.GetMainURL( INetURLObject::NO_DECODE );
}

void GalApp::Init()
{
    if( getenv( "OOO_INSTALL_PREFIX" ) == NULL ) {
        OUString fileName = GetAppFileName();
        int lastSlash = fileName.lastIndexOf( '/' );
#ifdef WNT
        // Don't know which directory separators GetAppFileName() returns on Windows.
        // Be safe and take into consideration they might be backslashes.
        if( fileName.lastIndexOf( '\\' ) > lastSlash )
            lastSlash = fileName.lastIndexOf( '\\' );
#endif
        OUString baseBinDir = fileName.copy( 0, lastSlash );
        OUString installPrefix = baseBinDir + OUString::createFromAscii( "/../.." );
        OUString assignment = OUString( "OOO_INSTALL_PREFIX=" ) + installPrefix;

        OUString envVar( "OOO_INSTALL_PREFIX");
        osl_setEnvironment(envVar.pData, installPrefix.pData);
    }
    OSL_TRACE( "OOO_INSTALL_PREFIX=%s", getenv( "OOO_INSTALL_PREFIX" ) );

    uno::Reference<uno::XComponentContext> xComponentContext
        = ::cppu::defaultBootstrap_InitialComponentContext();
    xMSF = uno::Reference<lang::XMultiServiceFactory>
        ( xComponentContext->getServiceManager(), uno::UNO_QUERY );
    if( !xMSF.is() )
    {
        fprintf( stderr, "Failed to bootstrap\n" );
        exit( 1 );
    }
    ::comphelper::setProcessServiceFactory( xMSF );

    // For backwards compatibility, in case some code still uses plain
    // createInstance w/o args directly to obtain an instance:
    com::sun::star::ucb::UniversalContentBroker::create(xComponentContext);
}

int GalApp::Main()
{
    OUString aPath, aDestDir;
    OUString aName( "Default name" );
    sal_uInt32 nNumFrom = 0;
    FileNameList aFiles;

    for( sal_uInt32 i = 0; i < GetCommandLineParamCount(); i++ )
    {
        OUString aParam = GetCommandLineParam( i );

        if ( aParam.startsWith( "-env:" ) )
            continue;
        else if ( aParam == "--help" || aParam == "-h"  )
            return PrintHelp();
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

    if( aFiles.size() < 1 )
        return PrintHelp();

    createTheme( aName, aPath, aDestDir, nNumFrom, aFiles );

    // Without this we get extraordinary crashes from the
    // drawinglayer VirtualDevice cache when importing svg
    uno::Reference< lang::XComponent >(
        comphelper::getProcessComponentContext(),
        uno::UNO_QUERY_THROW )-> dispose();
    ::comphelper::setProcessServiceFactory( NULL );

    return EXIT_SUCCESS;
}

void vclmain::createApplication()
{
    static GalApp aGalApp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
