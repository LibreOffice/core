/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef _LEGACYBINFILTERSMGR_HXX
#define _LEGACYBINFILTERSMGR_HXX

#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include "cppuhelper/factory.hxx"
namespace legacy_binfilters
{

::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > const & SAL_CALL getLegacyProcessServiceFactory();

} //namespace legacy_binfilters

extern "C"
{
//==================================================================================================
void SAL_CALL legacy_component_getImplementationEnvironment(
    sal_Char const ** ppEnvTypeName, uno_Environment ** ppEnv );
//==================================================================================================
sal_Bool SAL_CALL legacysmgr_component_writeInfo(
    ::com::sun::star::lang::XMultiServiceFactory * smgr, ::com::sun::star::registry::XRegistryKey * key );
void * SAL_CALL legacysmgr_component_getFactory(
    sal_Char const * implName, ::com::sun::star::lang::XMultiServiceFactory * smgr, ::com::sun::star::registry::XRegistryKey * key );
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
