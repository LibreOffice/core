/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: factreg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:46:19 $
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

namespace animcore
{
    rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;
}

using namespace animcore;

#define IMPLEMENTATION_ENTRY(N)\
{\
    createInstance_##N, getImplementationName_##N ,\
    getSupportedServiceNames_##N, createSingleComponentFactory ,\
    &g_moduleCount.modCnt , 0\
}\

static struct ImplementationEntry g_entries[] =
{
    IMPLEMENTATION_ENTRY( PAR ),
    IMPLEMENTATION_ENTRY( SEQ ),
    IMPLEMENTATION_ENTRY( ITERATE ),
    IMPLEMENTATION_ENTRY( ANIMATE ),
    IMPLEMENTATION_ENTRY( SET ),
    IMPLEMENTATION_ENTRY( ANIMATECOLOR ),
    IMPLEMENTATION_ENTRY( ANIMATEMOTION ),
    IMPLEMENTATION_ENTRY( ANIMATETRANSFORM ),
    IMPLEMENTATION_ENTRY( TRANSITIONFILTER ),
    IMPLEMENTATION_ENTRY( AUDIO ),
    IMPLEMENTATION_ENTRY( COMMAND ),
    IMPLEMENTATION_ENTRY( TargetPropertiesCreator ),
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
