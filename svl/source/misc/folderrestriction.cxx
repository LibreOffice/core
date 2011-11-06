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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"

#include <svl/folderrestriction.hxx>
#include "osl/process.h"
#include "tools/urlobj.hxx"
#include "unotools/localfilehelper.hxx"

//-----------------------------------------------------------------------------

static void convertStringListToUrls (
    const String& _rColonSeparatedList, ::std::vector< String >& _rTokens, bool _bFinalSlash )
{
    const sal_Unicode s_cSeparator =
#if defined(WNT)
        ';'
#else
        ':'
#endif
            ;
    xub_StrLen nTokens = _rColonSeparatedList.GetTokenCount( s_cSeparator );
    _rTokens.resize( 0 ); _rTokens.reserve( nTokens );
    for ( xub_StrLen i=0; i<nTokens; ++i )
    {
        // the current token in the list
        String sCurrentToken = _rColonSeparatedList.GetToken( i, s_cSeparator );
        if ( !sCurrentToken.Len() )
            continue;

        INetURLObject aCurrentURL;

        String sURL;
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

/** retrieves the value of an environment variable
    @return <TRUE/> if and only if the retrieved string value is not empty
*/
static bool getEnvironmentValue( const sal_Char* _pAsciiEnvName, ::rtl::OUString& _rValue )
{
    _rValue = ::rtl::OUString();
    ::rtl::OUString sEnvName = ::rtl::OUString::createFromAscii( _pAsciiEnvName );
    osl_getEnvironment( sEnvName.pData, &_rValue.pData );
    return _rValue.getLength() != 0;
}

//-----------------------------------------------------------------------------

namespace svt
{

    void getUnrestrictedFolders( ::std::vector< String >& _rFolders )
    {
        _rFolders.resize( 0 );
        ::rtl::OUString sRestrictedPathList;
        if ( getEnvironmentValue( "RestrictedPath", sRestrictedPathList ) )
        {
            // append a final slash. This ensures that when we later on check
            // for unrestricted paths, we don't allow paths like "/home/user35" just because
            // "/home/user3" is allowed - with the final slash, we make it "/home/user3/".
            convertStringListToUrls( sRestrictedPathList, _rFolders, true );
        }
    }

} // namespace svt

