/*************************************************************************
 *
 *  $RCSfile: factreg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jbu $ $Date: 2001-06-22 16:32:57 $
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

#include <osl/diagnose.h>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

#include "factreg.hxx"

namespace io_stm
{
    rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;
}

using namespace io_stm;

static struct ImplementationEntry g_entries[] =
{
    {
        OPipeImpl_CreateInstance, OPipeImpl_getImplementationName ,
        OPipeImpl_getSupportedServiceNames, createSingleComponentFactory ,
        &g_moduleCount.modCnt , 0
    },
    {
        OPumpImpl_CreateInstance, OPumpImpl_getImplementationName ,
        OPumpImpl_getSupportedServiceNames, createSingleComponentFactory ,
        &g_moduleCount.modCnt , 0
    },
    {
        ODataInputStream_CreateInstance, ODataInputStream_getImplementationName,
        ODataInputStream_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt, 0
    },
    {
        ODataOutputStream_CreateInstance, ODataOutputStream_getImplementationName,
        ODataOutputStream_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt, 0
    },
    {
        OObjectInputStream_CreateInstance, OObjectInputStream_getImplementationName,
        OObjectInputStream_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt, 0
    },
    {
        OObjectOutputStream_CreateInstance, OObjectOutputStream_getImplementationName,
        OObjectOutputStream_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt, 0
    },
    {
        OMarkableInputStream_CreateInstance, OMarkableInputStream_getImplementationName,
        OMarkableInputStream_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt, 0
    },
    {
        OMarkableOutputStream_CreateInstance, OMarkableOutputStream_getImplementationName,
        OMarkableOutputStream_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt, 0
    },
    { 0, 0, 0, 0, 0, 0 }

};

extern "C"
{

sal_Bool SAL_CALL component_canUnload( TimeValue *pTime )
{
    return g_moduleCount.canUnload( &g_moduleCount , pTime );
}

//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    return component_writeInfoHelper( pServiceManager, pRegistryKey, g_entries );
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

}
