#include <stdio.h>
#include <unistd.h>

#include <list>

#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/regpathhelper.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>

#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/configurationkeys.hxx>

#include <tools/urlobj.hxx>
#include <tools/fsys.hxx>
#include <tools/intn.hxx>
#include <svtools/filedlg.hxx>

#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <vcl/font.hxx>
#include <vcl/sound.hxx>
#include <vcl/print.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/help.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>

#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/bootstrap.hxx>

#include <galtheme.hxx>
#include <gallery1.hxx>

using namespace ::vos;
using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::lang;

typedef ::std::list<rtl::OUString> FileNameList;

class GalApp : public Application
{
public:
    virtual void Main();
};

Reference< XMultiServiceFactory > createApplicationServiceManager()
{
    Reference< XMultiServiceFactory > xMS;
    try
    {
        Reference< XComponentContext > xComponentContext = ::cppu::defaultBootstrap_InitialComponentContext();
        if ( xComponentContext.is() )
            xMS = xMS.query( xComponentContext->getServiceManager() );
    }
    catch( ::com::sun::star::uno::Exception& )
    {
    }

    return xMS;
}

static void createTheme( rtl::OUString aThemeName,
                         rtl::OUString aGalleryURL,
                         rtl::OUString aDestDir,
                         UINT32 nNumFrom,
                         FileNameList &rFiles )
{
    Gallery* pGallery;

    pGallery = Gallery::AcquireGallery( aGalleryURL );
    if (!pGallery ) {
            fprintf( stderr, "Could't acquire '%s'\n",
                     (const sal_Char *) rtl::OUStringToOString( aGalleryURL,
                                                                RTL_TEXTENCODING_UTF8 ) );
            exit( 1 );
    }
    fprintf( stderr, "Work on gallery '%s'\n",
                     (const sal_Char *) rtl::OUStringToOString( aGalleryURL, RTL_TEXTENCODING_UTF8 ) );

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
             (const sal_Char *) rtl::OUStringToOString( aDestDir, RTL_TEXTENCODING_UTF8 ) );
    pGalTheme->SetDestDir(String(aDestDir));

    FileNameList::const_iterator aIter;

    for( aIter = rFiles.begin(); aIter != rFiles.end(); aIter++ )
    {
//  Should/could use:
//  if ( ! pGalTheme->InsertFileOrDirURL( aURL ) ) {
//  Requires a load more components ...

        Graphic aGraphic;
        String aFormat;

#if 1
        if ( ! pGalTheme->InsertURL( *aIter ) )
            fprintf( stderr, "Failed to import '%s'\n",
                     (const sal_Char *) rtl::OUStringToOString( *aIter, RTL_TEXTENCODING_UTF8 ) );
        else
            fprintf( stderr, "Imported file '%s' (%d)\n",
                     (const sal_Char *) rtl::OUStringToOString( *aIter, RTL_TEXTENCODING_UTF8 ),
                     pGalTheme->GetObjectCount() );

#else // only loads BMPs
        SvStream *pStream = ::utl::UcbStreamHelper::CreateStream( *aIter, STREAM_READ );
        if (!pStream) {
            fprintf( stderr, "Can't find image to import\n" );
            exit (1);
        }
        *pStream >> aGraphic;
        delete pStream;
        if( aGraphic.GetType() == GRAPHIC_NONE )
        {
            fprintf( stderr, "Failed to load '%s'\n",
                     (const sal_Char *) rtl::OUStringToOString( *aIter, RTL_TEXTENCODING_UTF8 ) );
            continue;
        }

        SgaObjectBmp aObject( aGraphic, *aIter, aFormat );
        if ( ! aObject.IsValid() ) {
            fprintf( stderr, "Failed to create thumbnail for image\n" );
            continue;
        }

        if ( ! pGalTheme->InsertObject( aObject ) ) {
            fprintf( stderr, "Failed to insert file or URL\n" );
            continue;
        }
#endif
    }

    pGallery->ReleaseTheme( pGalTheme, aListener );

    Gallery::ReleaseGallery( pGallery );
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

static OUString Smartify( const OUString &rPath )
{
    INetURLObject aURL;
    aURL.SetSmartURL( rPath );
    return aURL.GetMainURL( INetURLObject::NO_DECODE );
}

// Cut/paste [ evilness ] from Desktop::CreateApplicationServiceManager()
Reference< XMultiServiceFactory > CreateApplicationServiceManager()
{
    try
    {
        Reference<XComponentContext> xComponentContext = ::cppu::defaultBootstrap_InitialComponentContext();
        Reference<XMultiServiceFactory> xMS(xComponentContext->getServiceManager(), UNO_QUERY);

        return xMS;
    }
    catch( ::com::sun::star::uno::Exception& )
    {
    }

    return Reference< XMultiServiceFactory >();
}

void GalApp::Main()
{
#ifdef TOO_SIMPLE_BY_HALF
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
        xMSF = cppu::createRegistryServiceFactory(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "gengal.rdb" ) ), sal_True );
    ::comphelper::setProcessServiceFactory( xMSF );

    // Without this no file access works ...
    ucb::ContentProviderDataList aData;
    ucb::ContentProviderData aFileProvider(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.FileContentProvider" ) ),
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "file" ) ),
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "" ) ) );
    aData.push_back( aFileProvider );
    ucb::ContentBroker::initialize( xMSF, aData );
#else
    Reference<XComponentContext> xComponentContext = ::cppu::defaultBootstrap_InitialComponentContext();
    Reference<XMultiServiceFactory> xMSF(xComponentContext->getServiceManager(), UNO_QUERY);
    if( !xMSF.is() )
        fprintf( stderr, "Failed to bootstrap\n" );
    ::comphelper::setProcessServiceFactory( xMSF );

    OUString aEmpty;
    Sequence< Any > aArgs(6);
    aArgs[0]
        <<= rtl::OUString::createFromAscii(UCB_CONFIGURATION_KEY1_LOCAL);
    aArgs[1]
        <<= rtl::OUString::createFromAscii(UCB_CONFIGURATION_KEY2_OFFICE);
    aArgs[2] <<= rtl::OUString::createFromAscii("PIPE");
    aArgs[3] <<= aEmpty;
    aArgs[4] <<= rtl::OUString::createFromAscii("PORTAL");
    aArgs[5] <<= aEmpty;

    if (! ::ucb::ContentBroker::initialize( xMSF, aArgs ) )
            fprintf( stderr, "Failed to init content broker\n" );
#endif


    bool bHelp = false;
    rtl::OUString aPath, aDestDir;
    rtl::OUString aName = rtl::OUString::createFromAscii( "Default name" );
    UINT32 nNumFrom = 0;
    FileNameList aFiles;

    for( USHORT i = 0; i < GetCommandLineParamCount(); i++ )
    {
        OUString aParam = GetCommandLineParam( i );

        if( aParam.equalsAscii( "--help" ) ||
            aParam.equalsAscii( "-h" ) )
                bHelp = true;

        else if ( aParam.equalsAscii( "--name" ) )
            aName = GetCommandLineParam( ++i );

        else if ( aParam.equalsAscii( "--path" ) )
            aPath = Smartify( GetCommandLineParam( ++i ) );

        else if ( aParam.equalsAscii( "--destdir" ) )
            aDestDir = GetCommandLineParam( ++i );

        else if ( aParam.equalsAscii( "--number-from" ) )
             nNumFrom = GetCommandLineParam( ++i ).ToInt32();

        else
            aFiles.push_back( Smartify( aParam ) );
    }

    if( bHelp )
    {
        PrintHelp();
        return;
    }

    createTheme( aName, aPath, aDestDir, nNumFrom, aFiles );
}

GalApp aGalApp;
