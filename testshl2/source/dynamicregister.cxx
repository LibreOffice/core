#include "dynamicregister.hxx"
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
// #include <osl/mutex.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include "filehelper.hxx"

// -----------------------------------------------------------------------------

DynamicLibraryHelper::DynamicLibraryHelper(rtl::OUString const& _sDLLName, GetOpt & _aOptions)
        :m_pModule(new ::osl::Module()),
         m_suDLLName(_sDLLName),
         m_aOptions(_aOptions)
{
    // create and load the module (shared library)
    if (! m_pModule->load( FileHelper::convertPath( _sDLLName ) ))
    {
        rtl::OString sDLLName = rtl::OUStringToOString(_sDLLName, RTL_TEXTENCODING_ASCII_US);
        fprintf(stderr, "warning: Can't load module '%s'.\n", sDLLName.getStr());
    }
}

DynamicLibraryHelper::~DynamicLibraryHelper()
{
    delete m_pModule;
}

