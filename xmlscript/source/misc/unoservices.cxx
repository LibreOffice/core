/*************************************************************************
 *
 *  $RCSfile: unoservices.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-11-09 12:34:26 $
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
        const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
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
