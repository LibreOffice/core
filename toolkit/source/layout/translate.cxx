#include "translate.hxx"

#include <list>
#if TEST_LAYOUT
#include <stdio.h>
#endif

#include <osl/process.h>
#include <unotools/bootstrap.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <vcl/svapp.hxx>

#include "proplist.hxx"

namespace layoutimpl
{
namespace css = ::com::sun::star;
using namespace css;
using ::rtl::OUString;
using ::utl::LocalFileHelper;
using ::utl::UCBContentHelper;
using ::utl::Bootstrap;

static std::list<OUString>
getLocaleSubdirList( lang::Locale const& rLocale )
{
    std::list<OUString> aSubdirs;
    aSubdirs.push_front( OUString::createFromAscii( "." ) );
    aSubdirs.push_front( OUString::createFromAscii( "en_US" ) );
    if ( rLocale.Language.getLength() )
        aSubdirs.push_front( rLocale.Language );
    if ( rLocale.Country.getLength() )
    {
        OUString aLocaleCountry = rLocale.Language
            + OUString::createFromAscii( "_" )
            + rLocale.Country;
        aSubdirs.push_front( aLocaleCountry );
        if ( rLocale.Variant.getLength() )
            aSubdirs.push_front( aLocaleCountry
                                 + OUString::createFromAscii( "." )
                                 + rLocale.Variant );
    }
    return aSubdirs;
}

static bool
fileExists( String const& aFile )
{
    String aUrl;
    LocalFileHelper::ConvertPhysicalNameToURL( aFile, aUrl );
    return UCBContentHelper::Exists( aUrl );
}

static OUString
getFirstExisting( OUString const& aDir, std::list<OUString> const& aSubDirs,
                  OUString const& aXMLName )
{
    static OUString const aSlash = OUString::createFromAscii( "/" );
    String aResult;
    for ( std::list<OUString>::const_iterator i = aSubDirs.begin();
          i != aSubDirs.end(); i++ )
    {
        String aFile = aDir + aSlash + *i + aSlash + aXMLName;
#if TEST_LAYOUT
        printf( "testing: %s\n", OUSTRING_CSTR( aFile ) );
#endif
        if ( fileExists( aFile ) )
            return aFile;
    }
    return OUString();
}

/*  FIXME: IWBN to share code with impimagetree.cxx, also for reading
  from zip files.  */
OUString
readRightTranslation( OUString const& aXMLName )
{
    String aXMLFile;
    std::list<OUString> aSubdirs
        = getLocaleSubdirList( Application::GetSettings().GetUILocale() );
#if TEST_LAYOUT // read from cwd first
    OUString aCurrentWorkingUrl;
    osl_getProcessWorkingDir( &aCurrentWorkingUrl.pData );
    String aCurrentWorkingDir;
    LocalFileHelper::ConvertURLToPhysicalName( aCurrentWorkingUrl, aCurrentWorkingDir );
    aXMLFile = getFirstExisting( aCurrentWorkingDir, aSubdirs, aXMLName );
    if ( aXMLFile.Len() )
        ;
    else
#endif /* TEST_LAYOUT */
    {
        OUString aShareUrl;
        Bootstrap::locateSharedData( aShareUrl );
        OUString aXMLUrl = aShareUrl + OUString::createFromAscii( "/layout" );
        String aXMLDir;
        LocalFileHelper::ConvertURLToPhysicalName( aXMLUrl, aXMLDir );
        aXMLFile = getFirstExisting( aXMLDir, aSubdirs, aXMLName );
    }

#if TEST_LAYOUT
    printf( "FOUND:%s\n", OUSTRING_CSTR ( OUString (aXMLFile) ) );
#endif /* TEST_LAYOUT */
    return aXMLFile;
}

} // namespace layoutimpl
