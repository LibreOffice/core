/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: service.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 16:00:29 $
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

// =============================================================================
// component exports
// =============================================================================
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace vbaobj
{
    // =============================================================================
    // component operations
    // =============================================================================

    uno::Reference< XInterface > SAL_CALL create(
        Reference< XComponentContext > const & xContext )
        SAL_THROW( () );

    // -----------------------------------------------------------------------------

    ::rtl::OUString SAL_CALL getImplementationName();

    Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames();

//  Reference<XInterface> SAL_CALL create(
//      Sequence<Any> const &, Reference<XComponentContext> const & );
} // end namespace vbaobj


    // =============================================================================

    const ::cppu::ImplementationEntry s_component_entries [] =
    {
        {
            ::vbaobj::create, ::vbaobj::getImplementationName,
            ::vbaobj::getSupportedServiceNames,
        ::cppu::createSingleComponentFactory,
            0, 0
        },
        { 0, 0, 0, 0, 0, 0 }
    };

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
        if ( ::cppu::component_writeInfoHelper(
            pServiceManager, pRegistryKey, s_component_entries ) )
        {
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
        OSL_TRACE("In component_getFactory");
        return ::cppu::component_getFactoryHelper(
            pImplName, pServiceManager, pRegistryKey, s_component_entries );
    }
}
