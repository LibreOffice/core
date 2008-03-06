#include <vcl/svapp.hxx>
//#include <transex3/vosapp.hxx>

// This works and was used before for standalone test, not sure why
// we'd want it.
#define LAYOUT_WEAK 1
#include "uno.hxx"

#include <cstdio>
#include <osl/file.h>

#include <rtl/ustring.hxx>
#include <cppuhelper/bootstrap.hxx>

#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/configurationkeys.hxx>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/xml/sax/SAXException.hpp>

#include "editor.hxx"

#include "wordcountdialog.hxx"
#undef _LAYOUT_POST_HXX
#include "zoom.hxx"
#undef _LAYOUT_POST_HXX

//#undef SW_WORDCOUNTDIALOG_HXX
#include <layout/layout-pre.hxx>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

Reference< XComponentContext > createInitialComponentContext(
    OUString const & inst_dir )
{
    Reference< XComponentContext > xContext;

    try
    {
        OUString file_url;
        oslFileError rc = osl_getFileURLFromSystemPath(
            inst_dir.pData, &file_url.pData );
        OSL_ASSERT( osl_File_E_None == rc );
        (void) rc;  // quiet warning

        OUString unorc = file_url + OUString(
            OUString::createFromAscii( "/program/" SAL_CONFIGFILE( "uno" )) );

        return defaultBootstrap_InitialComponentContext( unorc );
    }

    catch( Exception& rExc )
    {
        OString aStr( OUStringToOString( rExc.Message,
                                         RTL_TEXTENCODING_ASCII_US ) );
        OSL_ENSURE( 0, aStr.getStr() );
    }

    return xContext;
}


// -----------------------------------------------------------------------

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
    void IInit();
    void InitUCB();
    void LoadFile( OUString const &aName );
    void Main();
    void ParseCommandLine();
};

static void usage()
{
    fprintf (stderr, "usage: test [--inst inst_dir] [DIALOG.XML]... | --test [DIALOG.XML]\n" );
    exit( 2 );
}

void LayoutTest::LoadFile( const OUString &aTestFile )
{
    fprintf( stderr, "TEST: layout instance\n" );

    uno::Reference< lang::XSingleServiceFactory > xFactory(
        comphelper::createProcessComponent(
            rtl::OUString::createFromAscii( "com.sun.star.awt.Layout" ) ),
        uno::UNO_QUERY );
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

// Nurgh ...
void LayoutTest::InitUCB()
{
    OUString aEmpty;
    Sequence< Any > aArgs(6);
    aArgs[0]
        <<= OUString::createFromAscii(UCB_CONFIGURATION_KEY1_LOCAL);
    aArgs[1]
        <<= OUString::createFromAscii(UCB_CONFIGURATION_KEY2_OFFICE);
    aArgs[2] <<= OUString::createFromAscii( "PIPE" );
    aArgs[3] <<= aEmpty;
    aArgs[4] <<= OUString::createFromAscii( "PORTAL" );
    aArgs[5] <<= aEmpty;

    if (! ::ucbhelper::ContentBroker::initialize( mxMSF, aArgs ) )
        fprintf( stderr, "Failed to init content broker\n" );
}

#define INIT_ENV_VOODOO( installDir )                                   \
    /* Unless you do this, obviously you don't deserve to have anything work */ \
    OString aCfg = "CFG_INIFILE=file://"                                \
                 + OUStringToOString( installDir, RTL_TEXTENCODING_UTF8 ) \
                 + "/program/configmgrrc";                              \
    putenv( (char *)aCfg.getStr() );                                    \
    /* Unless you do this, obviously you don't want your paths to be correct & \
       hence no images.zip location is possible. */                     \
    OString aBaseInst = "BaseInstallation=file://"                      \
                 + OUStringToOString( installDir, RTL_TEXTENCODING_UTF8 ); \
    putenv( (char *)aBaseInst.getStr() );                               \

void LayoutTest::IInit()
{
    INIT_ENV_VOODOO( mInstallDir );
    mxContext = createInitialComponentContext( mInstallDir );
    mxMSF = new UnoBootstrapLayout( Reference< lang::XMultiServiceFactory >( mxContext->getServiceManager(), UNO_QUERY ) );
    ::comphelper::setProcessServiceFactory( mxMSF );
    InitUCB();
}

void LayoutTest::ParseCommandLine()
{
    for ( int i = 0; i < GetCommandLineParamCount(); i++ )
    {
        OUString aParam = OUString( GetCommandLineParam( i ) );
        if ( aParam.equalsAscii( "-h" ) || aParam.equalsAscii( "--help" ) )
            usage();
        if ( aParam.equalsAscii( "--inst" ) )
        {
            if ( i >= GetCommandLineParamCount() - 1)
                usage();
            mInstallDir = GetCommandLineParam( ++i );
        }
        else if ( aParam.equalsAscii( "--test" ) )
        {
            mTestDialog = OUString::createFromAscii( "zoom" );
            if (i + 1 < GetCommandLineParamCount())
                mTestDialog = GetCommandLineParam( ++i );
        }
        else if ( aParam.equalsAscii( "--editor" ) )
            mEditMode = true;
        else
            mFiles.push_back( aParam );
    }

    if ( mFiles.size() <= 0 )
        mFiles.push_back( OUString::createFromAscii( "layout.xml" ) );
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

void RunDialog( layout::Dialog& dialog )
{
    dialog.Show();
    dialog.Execute();
    fprintf( stderr, "1st execute exited" );
    dialog.Execute();
    fprintf( stderr, "2nd execute exited" );
}

void TestDialog( OUString const& name )
{
    if ( name.equalsAscii( "zoom" ) )
    {
        SvxZoomDialog zoom( 0 );
        RunDialog( zoom );
    }
    else if ( name.equalsAscii( "wordcount" ) )
    {
        SwWordCountDialog words ( 0 );
        RunDialog( words );
    }
}

void LayoutTest::RunFiles()
{
    fprintf( stderr, "TEST: loading files\n" );
    for ( std::list< OUString >::iterator  i = mFiles.begin(); i != mFiles.end(); i++ )
        LoadFile( *i );
    fprintf( stderr, "TEST: executing\n" );
    Execute();
    fprintf( stderr, "TEST: done executing\n" );
}

void LayoutTest::ExceptionalMain()
{
    ParseCommandLine();
    IInit();
    INIT_ENV_VOODOO( mInstallDir );

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
        OSL_ENSURE( 0, aStr.getStr() );
    }
    catch ( uno::Exception & rExc )
    {
        OString aStr( OUStringToOString( rExc.Message,
                                         RTL_TEXTENCODING_ASCII_US ) );
        fprintf (stderr, "UNO error: '%s'\n", aStr.getStr());
        OSL_ENSURE( 0, aStr.getStr() );
    }

    Reference< lang::XComponent > xComp( mxContext, UNO_QUERY );
    if ( xComp.is() )
        xComp->dispose();
}

LayoutTest::LayoutTest( char const* installDir )
    : mInstallDir( OUString::createFromAscii ( installDir ) )
{
}

LayoutTest lt( "/usr/local/lib/ooo" );
