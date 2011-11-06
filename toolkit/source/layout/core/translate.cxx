/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "translate.hxx"

#include <list>
#if TEST_LAYOUT
#include <cstdio>
#include "tools/getprocessworkingdir.hxx"
#endif

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
    aSubdirs.push_front( OUString::createFromAscii( "en-US" ) );
    if ( rLocale.Language.getLength() )
        aSubdirs.push_front( rLocale.Language );
    if ( rLocale.Country.getLength() )
    {
        OUString aLocaleCountry = rLocale.Language
            + OUString::createFromAscii( "-" )
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
        OSL_TRACE( "testing: %s", OUSTRING_CSTR( aFile ) );
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
    tools::getProcessWorkingDir( &aCurrentWorkingUrl );
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

    OSL_TRACE( "FOUND:%s", OUSTRING_CSTR ( OUString (aXMLFile) ) );
    return aXMLFile;
}

} // namespace layoutimpl
