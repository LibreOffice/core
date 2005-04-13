/*************************************************************************
 *
 *  $RCSfile: folderrestriction.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-04-13 11:27:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

