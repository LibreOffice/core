/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_resource.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: vg $ $Date: 2006-04-07 14:46:03 $
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

#include "dp_misc.h"
#include "dp_resource.h"
#include "osl/module.hxx"
#include "osl/mutex.hxx"
#include "cppuhelper/implbase1.hxx"
#include "unotools/configmgr.hxx"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace dp_misc {
namespace {

struct OfficeLocale :
        public rtl::StaticWithInit<const lang::Locale, OfficeLocale> {
    const lang::Locale operator () () {
        OUString slang;
        if (! (::utl::ConfigManager::GetDirectConfigProperty(
                   ::utl::ConfigManager::LOCALE ) >>= slang))
            throw RuntimeException( OUSTR("Cannot determine language!"), 0 );
        return toLocale(slang);
    }
};

void dummy() {}
struct DeploymentResMgr : public rtl::StaticWithInit<
    ResMgr *, DeploymentResMgr> {
    ResMgr * operator () () {
        return ResMgr::CreateResMgr( "deployment" LIBRARY_SOLARUPD(),
                                     OfficeLocale::get() );
    }
};

osl::Mutex s_mutex;

} // anon namespace

//==============================================================================
ResId getResId( USHORT id )
{
    const osl::MutexGuard guard( s_mutex );
    return ResId( id, DeploymentResMgr::get() );
}

//==============================================================================
String getResourceString( USHORT id )
{
    const osl::MutexGuard guard( s_mutex );
    String ret( ResId( id, DeploymentResMgr::get() ) );
    if (ret.SearchAscii( "%PRODUCTNAME" ) != STRING_NOTFOUND) {
        static String s_brandName;
        if (s_brandName.Len() == 0) {
            OUString brandName(
                ::utl::ConfigManager::GetDirectConfigProperty(
                    ::utl::ConfigManager::PRODUCTNAME ).get<OUString>() );
            s_brandName = brandName;
        }
        ret.SearchAndReplaceAllAscii( "%PRODUCTNAME", s_brandName );
    }
    return ret;
}

//==============================================================================
lang::Locale const & getOfficeLocale()
{
    return OfficeLocale::get();
}

}

