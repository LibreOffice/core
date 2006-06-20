/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoservices.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:10:54 $
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

#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star::uno;


// =============================================================================

namespace xmlscript
{
    Sequence< OUString > SAL_CALL getSupportedServiceNames_DocumentHandlerImpl();
    OUString SAL_CALL getImplementationName_DocumentHandlerImpl();
    Reference< XInterface > SAL_CALL create_DocumentHandlerImpl(
        Reference< XComponentContext > const & xContext )
            SAL_THROW( (Exception) );

    Sequence< OUString > SAL_CALL getSupportedServiceNames_XMLBasicExporter();
    OUString SAL_CALL getImplementationName_XMLBasicExporter();
    Reference< XInterface > SAL_CALL create_XMLBasicExporter(
        Reference< XComponentContext > const & xContext )
            SAL_THROW( (Exception) );

    Sequence< OUString > SAL_CALL getSupportedServiceNames_XMLOasisBasicExporter();
    OUString SAL_CALL getImplementationName_XMLOasisBasicExporter();
    Reference< XInterface > SAL_CALL create_XMLOasisBasicExporter(
        Reference< XComponentContext > const & xContext )
            SAL_THROW( (Exception) );

    Sequence< OUString > SAL_CALL getSupportedServiceNames_XMLBasicImporter();
    OUString SAL_CALL getImplementationName_XMLBasicImporter();
    Reference< XInterface > SAL_CALL create_XMLBasicImporter(
        Reference< XComponentContext > const & xContext )
            SAL_THROW( (Exception) );

    Sequence< OUString > SAL_CALL getSupportedServiceNames_XMLOasisBasicImporter();
    OUString SAL_CALL getImplementationName_XMLOasisBasicImporter();
    Reference< XInterface > SAL_CALL create_XMLOasisBasicImporter(
        Reference< XComponentContext > const & xContext )
            SAL_THROW( (Exception) );

    // -----------------------------------------------------------------------------

    static struct ::cppu::ImplementationEntry s_entries [] =
    {
        {
            create_DocumentHandlerImpl, getImplementationName_DocumentHandlerImpl,
            getSupportedServiceNames_DocumentHandlerImpl, ::cppu::createSingleComponentFactory,
            0, 0
        },
        {
            create_XMLBasicExporter, getImplementationName_XMLBasicExporter,
            getSupportedServiceNames_XMLBasicExporter, ::cppu::createSingleComponentFactory,
            0, 0
        },
        {
            create_XMLOasisBasicExporter, getImplementationName_XMLOasisBasicExporter,
            getSupportedServiceNames_XMLOasisBasicExporter, ::cppu::createSingleComponentFactory,
            0, 0
        },
        {
            create_XMLBasicImporter, getImplementationName_XMLBasicImporter,
            getSupportedServiceNames_XMLBasicImporter, ::cppu::createSingleComponentFactory,
            0, 0
        },
        {
            create_XMLOasisBasicImporter, getImplementationName_XMLOasisBasicImporter,
            getSupportedServiceNames_XMLOasisBasicImporter, ::cppu::createSingleComponentFactory,
            0, 0
        },
        { 0, 0, 0, 0, 0, 0 }
    };
}

// =============================================================================

extern "C"
{
    void SAL_CALL component_getImplementationEnvironment(
        const sal_Char ** ppEnvTypeName, uno_Environment ** )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    // -----------------------------------------------------------------------------

    sal_Bool SAL_CALL component_writeInfo(
        void * pServiceManager, void * pRegistryKey )
    {
        return ::cppu::component_writeInfoHelper(
            pServiceManager, pRegistryKey, ::xmlscript::s_entries );
    }

    // -----------------------------------------------------------------------------

    void * SAL_CALL component_getFactory(
        const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
    {
        return ::cppu::component_getFactoryHelper(
            pImplName, pServiceManager, pRegistryKey, ::xmlscript::s_entries );
    }
}
