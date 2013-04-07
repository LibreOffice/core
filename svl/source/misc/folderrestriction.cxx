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


#include <svl/folderrestriction.hxx>
#include "osl/process.h"
#include "tools/urlobj.hxx"
#include "unotools/localfilehelper.hxx"

//-----------------------------------------------------------------------------

static void convertStringListToUrls (
    const OUString& _rColonSeparatedList, ::std::vector< String >& _rTokens, bool _bFinalSlash )
{
    const sal_Unicode cSeparator =
#if defined(WNT)
        ';'
#else
        ':'
#endif
            ;

    sal_Int32 nIndex = 0;
    do
    {
        // the current token in the list
        OUString sCurrentToken = _rColonSeparatedList.getToken( 0, cSeparator, nIndex );
        if (!sCurrentToken.isEmpty())
        {
            INetURLObject aCurrentURL;

            OUString sURL;
            if ( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( sCurrentToken, sURL ) )
                aCurrentURL = INetURLObject( sURL );
            else
            {
                // smart URL parsing, assuming FILE protocol
                aCurrentURL = INetURLObject( sCurrentToken, INET_PROT_FILE );
            }

            if ( _bFinalSlash )
                aCurrentURL.setFinalSlash( );
            else
                aCurrentURL.removeFinalSlash( );
            _rTokens.push_back( aCurrentURL.GetMainURL( INetURLObject::NO_DECODE ) );
        }
    }
    while ( nIndex >= 0 );
}

/** retrieves the value of an environment variable
    @return <TRUE/> if and only if the retrieved string value is not empty
*/
static bool getEnvironmentValue( const sal_Char* _pAsciiEnvName, OUString& _rValue )
{
    _rValue = OUString();
    OUString sEnvName = OUString::createFromAscii( _pAsciiEnvName );
    osl_getEnvironment( sEnvName.pData, &_rValue.pData );
    return !_rValue.isEmpty();
}

//-----------------------------------------------------------------------------

namespace svt
{

    void getUnrestrictedFolders( ::std::vector< String >& _rFolders )
    {
        _rFolders.resize( 0 );
        OUString sRestrictedPathList;
        if ( getEnvironmentValue( "RestrictedPath", sRestrictedPathList ) )
        {
            // append a final slash. This ensures that when we later on check
            // for unrestricted paths, we don't allow paths like "/home/user35" just because
            // "/home/user3" is allowed - with the final slash, we make it "/home/user3/".
            convertStringListToUrls( sRestrictedPathList, _rFolders, true );
        }
    }

} // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
