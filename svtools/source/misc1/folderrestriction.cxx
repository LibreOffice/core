/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: folderrestriction.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:15:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include "folderrestriction.hxx"

#ifndef _OSL_PROCESS_H_
#include "osl/process.h"
#endif

#ifndef _URLOBJ_HXX
#include "tools/urlobj.hxx"
#endif

#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include "unotools/localfilehelper.hxx"
#endif

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

