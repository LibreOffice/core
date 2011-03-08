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
    aSubdirs.push_front( OUString(RTL_CONSTASCII_USTRINGPARAM(".")) );
    aSubdirs.push_front( OUString(RTL_CONSTASCII_USTRINGPARAM("en-US")) );
    if ( rLocale.Language.getLength() )
        aSubdirs.push_front( rLocale.Language );
    if ( rLocale.Country.getLength() )
    {
        OUString aLocaleCountry = rLocale.Language
            + OUString(RTL_CONSTASCII_USTRINGPARAM("-"))
            + rLocale.Country;
        aSubdirs.push_front( aLocaleCountry );
        if ( rLocale.Variant.getLength() )
            aSubdirs.push_front( aLocaleCountry
                                 + OUString(RTL_CONSTASCII_USTRINGPARAM("."))
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
    static OUString const aSlash(RTL_CONSTASCII_USTRINGPARAM("/"));
    String aResult;
    for ( std::list<OUString>::const_iterator i = aSubDirs.begin();
          i != aSubDirs.end(); ++i )
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
    tools::getProcessWorkingDir( aCurrentWorkingUrl );
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
        OUString aXMLUrl = aShareUrl + OUString(RTL_CONSTASCII_USTRINGPARAM("/layout"));
        String aXMLDir;
        LocalFileHelper::ConvertURLToPhysicalName( aXMLUrl, aXMLDir );
        aXMLFile = getFirstExisting( aXMLDir, aSubdirs, aXMLName );
    }

    OSL_TRACE( "FOUND:%s", OUSTRING_CSTR ( OUString (aXMLFile) ) );
    return aXMLFile;
}

} // namespace layoutimpl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
