/*************************************************************************
 *
 *  $RCSfile: dp_resource.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: sb $ $Date: 2004-07-30 09:59:55 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "dp_misc.h"
#include "dp_resource.h"
#include "osl/module.hxx"
#include "unotools/configmgr.hxx"
#include "tools/resmgr.hxx"
#include "tools/isolang.hxx"


using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace dp_misc
{

static ::vos::OMutex s_mutex;
::vos::IMutex * g_pResMgrMmutex = &s_mutex;

//==============================================================================
static ResMgr * getResMgr()
{
    static ResMgr * s_pResMgr = 0;
    if (s_pResMgr == 0)
    {
        OUString slang;
        if (! (::utl::ConfigManager::GetDirectConfigProperty(
                   ::utl::ConfigManager::LOCALE ) >>= slang))
            throw RuntimeException( OUSTR("Cannot determine language!"),
                                    Reference<XInterface>() );

        sal_Int32 nIndex = 0;
        ::com::sun::star::lang::Locale aLocale;
        aLocale.Language = slang.getToken( 0, '-', nIndex );
        aLocale.Country = slang.getToken( 1, '-', nIndex );
        aLocale.Variant = slang.getToken( 2, '-', nIndex );

        OUString path;
        ::osl::Module::getUrlFromAddress( (void *) getResMgr, path );
        path = path.copy( 0, path.lastIndexOf( '/' ) + 1 ) + OUSTR("resource");
        String resourcePath( path );

        s_pResMgr = ResMgr::CreateResMgr(
            "deployment" LIBRARY_SOLARUPD(),
            aLocale, NULL, &resourcePath );
        OSL_ASSERT( s_pResMgr != 0 );
    }
    return s_pResMgr;
}

//==============================================================================
ResId getResId( USHORT id )
{
    ::vos::OGuard guard( g_pResMgrMmutex );
    return ResId( id, getResMgr() );
}

//==============================================================================
String getResourceString( USHORT id )
{
    ::vos::OGuard guard( g_pResMgrMmutex );
    String ret( ResId( id, getResMgr() ) );
    if (ret.SearchAscii( "%PRODUCTNAME" ) != STRING_NOTFOUND)
    {
        static String s_brandName;
        if (s_brandName.Len() == 0)
        {
            OUString brandName(
                extract_throw<OUString>(
                    ::utl::ConfigManager::GetDirectConfigProperty(
                        ::utl::ConfigManager::PRODUCTNAME ) ) );
            s_brandName = brandName;
        }
        ret.SearchAndReplaceAllAscii( "%PRODUCTNAME", s_brandName );
    }
    return ret;
}

}

