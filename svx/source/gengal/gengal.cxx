/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include <vector>

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
#include <sal/types.h>
#include <vcl/svapp.hxx>

#include <svx/galtheme.hxx>
#include <svx/gallery1.hxx>

using namespace ::com::sun::star;

class GalApp : public Application
{
    bool mbInBuildTree;
    bool mbRelativeURLs;
public:
    GalApp() : mbInBuildTree( false ), mbRelativeURLs( false )
    {
    }
    virtual int Main() override;

protected:
    uno::Reference<lang::XMultiServiceFactory> xMSF;
    void Init() override;
    void DeInit() override;
};

Gallery* createGallery( const OUString& rURL )
{
    return new Gallery( rURL );
}

void disposeGallery( Gallery* pGallery )
{
    delete pGallery;
}

static void createTheme( const OUString& aThemeName, const OUString& aGalleryURL,
                         const OUString& aDestDir, std::vector<INetURLObject> &rFiles,
                         bool bRelativeURLs )
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

    fprintf( stderr, "Existing themes: %" SAL_PRI_SIZET "u\n",
             pGallery->GetThemeCount() );

    GalleryTheme *pGalTheme;
    if( !pGallery->HasTheme( aThemeName) ) {
            if( !pGallery->CreateTheme( aThemeName ) ) {
                    fprintf( stderr, "Failed to create theme\n" );
                    exit( 1 );
            }
    }

    fprintf( stderr, "Existing themes: %" SAL_PRI_SIZET "u\n",
             pGallery->GetThemeCount() );

    SfxListener aListener;

    pGalTheme = pGallery->AcquireTheme( aThemeName, aListener );
    if ( !pGalTheme ) {
            fprintf( stderr, "Failed to acquire theme\n" );
            exit( 1 );
    }

    fprintf( stderr, "Using DestDir: %s\n",
             OUStringToOString( aDestDir, RTL_TEXTENCODING_UTF8 ).getStr() );
    pGalTheme->SetDestDir( aDestDir, bRelativeURLs );

    std::vector<INetURLObject>::const_iterator aIter;

    for( aIter = rFiles.begin(); aIter != rFiles.end(); ++aIter )
    {
//  Should/could use:
//    if ( ! pGalTheme->InsertFileOrDirURL( aURL ) ) {
//    Requires a load more components ...

        Graphic aGraphic;

        if ( ! pGalTheme->InsertURL( *aIter ) )
            fprintf( stderr, "Failed to import '%s'\n",
                     OUStringToOString( aIter->GetMainURL(INetURLObject::DecodeMechanism::NONE), RTL_TEXTENCODING_UTF8 ).getStr() );
        else
            fprintf( stderr, "Imported file '%s' (%" SAL_PRI_SIZET "u)\n",
                     OUStringToOString( aIter->GetMainURL(INetURLObject::DecodeMechanism::NONE), RTL_TEXTENCODING_UTF8 ).getStr(),
                     pGalTheme->GetObjectCount() );
    }

    pGallery->ReleaseTheme( pGalTheme, aListener );

    disposeGallery( pGallery );
}

static int PrintHelp()
{
    fprintf( stdout, "Utility to generate LibreOffice gallery files\n\n" );

    fprintf( stdout, "using: gengal --name <name> --path <dir> [ --destdir <path> ]\n");
    fprintf( stdout, "              [ files ... ]\n\n" );

    fprintf( stdout, "options:\n");
    fprintf( stdout, " --name <theme>\t\tdefines the user visible name of the created or updated theme.\n");

    fprintf( stdout, " --path <dir>\t\tdefines directory where the gallery files are created\n");
    fprintf( stdout, "\t\t\tor updated.\n");

    fprintf( stdout, " --destdir <dir>\tdefines a path prefix to be removed from the paths\n");
    fprintf( stdout, "\t\t\tstored in the gallery files. It is useful to create\n");
    fprintf( stdout, "\t\t\tRPM packages using the BuildRoot feature.\n");

    fprintf( stdout, " --relative-urls\t\tflags that after removing the destdir, the URL should be a path relative to the gallery folder in the install\n");
    fprintf( stdout, "\t\t\tprimarily used for internal gallery generation at compile time.\n");
    fprintf( stdout, "\t\t\ttheme files.\n");
    fprintf( stdout, " files\t\t\tlists files to be added to the gallery. Absolute paths\n");
    fprintf( stdout, "\t\t\tare required.\n");
    // --build-tree not documented - only useful during the build ...

    return EXIT_SUCCESS;
}

static INetURLObject Smartify( const OUString &rPath )
{
    INetURLObject aURL;
    aURL.SetSmartURL( rPath );
    return aURL;
}

void GalApp::Init()
{
    try {
        if( !mbInBuildTree && getenv( "OOO_INSTALL_PREFIX" ) == nullptr ) {
            OUString fileName = GetAppFileName();
            int lastSlash = fileName.lastIndexOf( '/' );
#ifdef _WIN32
        // Don't know which directory separators GetAppFileName() returns on Windows.
        // Be safe and take into consideration they might be backslashes.
            if( fileName.lastIndexOf( '\\' ) > lastSlash )
                lastSlash = fileName.lastIndexOf( '\\' );
#endif
            OUString baseBinDir = fileName.copy( 0, lastSlash );
            OUString installPrefix = baseBinDir + "/../..";

            OUString envVar( "OOO_INSTALL_PREFIX");
            osl_setEnvironment(envVar.pData, installPrefix.pData);
        }
        SAL_INFO("svx", "OOO_INSTALL_PREFIX=" << getenv( "OOO_INSTALL_PREFIX" ) );

        uno::Reference<uno::XComponentContext> xComponentContext
            = ::cppu::defaultBootstrap_InitialComponentContext();
        xMSF.set( xComponentContext->getServiceManager(), uno::UNO_QUERY );
        if( !xMSF.is() )
        {
            fprintf( stderr, "Failed to bootstrap\n" );
            exit( 1 );
        }
        ::comphelper::setProcessServiceFactory( xMSF );

        // For backwards compatibility, in case some code still uses plain
        // createInstance w/o args directly to obtain an instance:
        css::ucb::UniversalContentBroker::create(xComponentContext);
    } catch (const uno::Exception &e) {
        fprintf( stderr, "Bootstrap exception '%s'\n",
                 rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
        exit( 1 );
    }
}

std::vector<OUString> ReadResponseFile_Impl(OUString const& rInput)
{
    osl::File file(rInput);
    osl::FileBase::RC rc = file.open(osl_File_OpenFlag_Read);
    OString const uInput(rtl::OUStringToOString(rInput, RTL_TEXTENCODING_UTF8));
    if (osl::FileBase::E_None != rc)
    {
        fprintf(stderr, "error while opening response file: %s (%d)\n",
            uInput.getStr(), rc);
        exit(1);
    }

    std::vector<OUString> ret;
    OUStringBuffer b;
    char buf[1<<16];
    while (true)
    {
        sal_uInt64 size(0);
        rc = file.read(buf, sizeof(buf), size);
        if (osl::FileBase::E_None != rc)
        {
            fprintf(stderr, "error while reading response file: %s (%d)\n",
                uInput.getStr(), rc);
            exit(1);
        }
        if (!size)
            break;
        for (sal_uInt64 i = 0; i < size; ++i)
        {
            if (static_cast<unsigned char>(buf[i]) >= 128)
            {
                fprintf(stderr, "non-ASCII character in response file: %s\n",
                    uInput.getStr());
                exit(1);
            }
            switch (buf[i])
            {
                case ' ' :
                case '\t':
                case '\r':
                case '\n':
                    if (!b.isEmpty())
                        ret.push_back(b.makeStringAndClear());
                    break;
                default:
                    b.append(buf[i]);
                    break;
            }
        }
    }
    if (!b.isEmpty())
        ret.push_back(b.makeStringAndClear());
    return ret;
}

void
ReadResponseFile(std::vector<INetURLObject> & rFiles, OUString const& rInput)
{
    std::vector<OUString> files(ReadResponseFile_Impl(rInput));
    for (size_t i = 0; i < files.size(); ++i)
    {
        rFiles.push_back(Smartify(files[i]));
    }
}

int GalApp::Main()
{
    try
    {
        OUString aPath, aDestDir;
        OUString aName( "Default name" );
        std::vector<INetURLObject> aFiles;

        for( sal_uInt16 i = 0; i < GetCommandLineParamCount(); ++i )
        {
            OUString aParam = GetCommandLineParam( i );

            if ( aParam.startsWith( "-env:" ) )
                continue;
            else if ( aParam == "--help" || aParam == "-h"  )
                return PrintHelp();
            else if ( aParam == "--build-tree" )
            {
                mbRelativeURLs = true;
                mbInBuildTree = true;
            }
            else if ( aParam == "--name" )
                aName = GetCommandLineParam( ++i );
            else if ( aParam == "--path" )
                aPath = Smartify( GetCommandLineParam( ++i ) ).
                    GetMainURL(INetURLObject::DecodeMechanism::NONE);
            else if ( aParam == "--destdir" )
                aDestDir = GetCommandLineParam( ++i );
            else if ( aParam == "--relative-urls" )
                mbRelativeURLs = true;
            else if ( aParam == "--number-from" )
                fprintf ( stderr, "--number-from is deprecated, themes now "
                          "have filenames based on their names\n" );
            else if ( aParam == "--filenames" )
                ReadResponseFile(aFiles, GetCommandLineParam(++i));
            else
                aFiles.push_back( Smartify( aParam ) );
        }

        if( aFiles.empty() )
            return PrintHelp();

        createTheme( aName, aPath, aDestDir, aFiles, mbRelativeURLs );
    }
    catch (const uno::Exception& e)
    {
        SAL_WARN("svx", "Fatal exception: " << e.Message);
        return EXIT_FAILURE;
    }
    catch (const std::exception &e)
    {
        SAL_WARN("svx", "Fatal exception: " << e.what());
        return 1;
    }

    return EXIT_SUCCESS;
}

void GalApp::DeInit()
{
    uno::Reference< lang::XComponent >(
        comphelper::getProcessComponentContext(),
        uno::UNO_QUERY_THROW )-> dispose();
    ::comphelper::setProcessServiceFactory( nullptr );
}

void vclmain::createApplication()
{
    Application::EnableConsoleOnly();
    static GalApp aGalApp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
