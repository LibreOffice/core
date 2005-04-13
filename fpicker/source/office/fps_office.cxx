
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include "cppuhelper/implementationentry.hxx"
#endif

#include "OfficeFilePicker.hxx"
#include "OfficeFolderPicker.hxx"

static cppu::ImplementationEntry g_entries[] =
{
    {
        SvtFilePicker::impl_createInstance,
        SvtFilePicker::impl_getStaticImplementationName,
        SvtFilePicker::impl_getStaticSupportedServiceNames,
        cppu::createSingleComponentFactory, 0, 0
    },
    {
        SvtFolderPicker::impl_createInstance,
        SvtFolderPicker::impl_getStaticImplementationName,
        SvtFolderPicker::impl_getStaticSupportedServiceNames,
        cppu::createSingleComponentFactory, 0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C"
{
SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment (
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo (
    void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_writeInfoHelper (
        pServiceManager, pRegistryKey, g_entries);
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory (
    const sal_Char * pImplementationName, void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_getFactoryHelper (
        pImplementationName, pServiceManager, pRegistryKey, g_entries);
}

} // extern "C"
