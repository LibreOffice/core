/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: service.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:42:52 $
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
#include "cppuhelper/implementationentry.hxx"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"
#include "comphelper/servicedecl.hxx"

// =============================================================================
// component exports
// =============================================================================
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sdecl = comphelper::service_decl;

// reference service helper(s)
namespace  range
{
extern sdecl::ServiceDecl const serviceDecl;
}
namespace  workbook
{
extern sdecl::ServiceDecl const serviceDecl;
}
namespace  worksheet
{
extern sdecl::ServiceDecl const serviceDecl;
}
namespace globals
{
extern sdecl::ServiceDecl const serviceDecl;
}



extern "C"
{
    void SAL_CALL component_getImplementationEnvironment(
        const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
    {
        OSL_TRACE("In component_getImplementationEnv");
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    sal_Bool SAL_CALL component_writeInfo(
        lang::XMultiServiceFactory * pServiceManager, registry::XRegistryKey * pRegistryKey )
    {
        OSL_TRACE("In component_writeInfo");

    // Component registration
        if ( component_writeInfoHelper( pServiceManager, pRegistryKey,
        range::serviceDecl, workbook::serviceDecl, worksheet::serviceDecl, globals::serviceDecl ) )
        {
            // Singleton registration
            try
            {
                registry::XRegistryKey * pKey =
                    reinterpret_cast< registry::XRegistryKey * >(pRegistryKey);

                Reference< registry::XRegistryKey >xKey = pKey->createKey(
                    rtl::OUString::createFromAscii( ("org.openoffice.vba.Globals/UNO/SINGLETONS/org.openoffice.vba.theGlobals") ) );
                xKey->setStringValue( ::rtl::OUString::createFromAscii(
                    ("org.openoffice.vba.Globals") ) );
                return sal_True;
            }
            catch( uno::Exception& /*e*/ )
            {
                //recomp & friends will detect false returned and fail
            }
        }
        return sal_False;
    }

    void * SAL_CALL component_getFactory(
        const sal_Char * pImplName, lang::XMultiServiceFactory * pServiceManager,
        registry::XRegistryKey * pRegistryKey )
    {
        OSL_TRACE("In component_getFactory for %s", pImplName );
    void* pRet =  component_getFactoryHelper(
            pImplName, pServiceManager, pRegistryKey, range::serviceDecl, workbook::serviceDecl, worksheet::serviceDecl, globals::serviceDecl );
    OSL_TRACE("Ret is 0x%x", pRet);
    return pRet;
    }
}
