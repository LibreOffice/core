/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_OSL_MODULE_HXX
#define INCLUDED_OSL_MODULE_HXX

#include "sal/config.h"

#include <cstddef>

#include "rtl/ustring.hxx"
#include "osl/module.h"

namespace osl
{

class Module
{
    Module( const Module&) SAL_DELETED_FUNCTION;
    Module& operator = ( const Module&) SAL_DELETED_FUNCTION;

public:
    static bool getUrlFromAddress(void * addr, ::rtl::OUString & libraryUrl) {
        return osl_getModuleURLFromAddress(addr, &libraryUrl.pData);
    }

    /** Get module URL from the specified function address in the module.

        Similar to getUrlFromAddress, but use a function address to get URL of the Module.
        Use Function pointer as symbol address to conceal type conversion.

        @param[in] addr            function address in oslGenericFunction format.
        @param[in,out] libraryUrl  receives the URL of the module.

        @retval true on success
        @retval false can not get the URL from the specified function address or the parameter is invalid.

        @see getUrlFromAddress
    */
    static bool getUrlFromAddress( oslGenericFunction addr, ::rtl::OUString & libraryUrl){
        return osl_getModuleURLFromFunctionAddress( addr, &libraryUrl.pData );
    }

    Module(): m_Module(NULL){}

#ifndef DISABLE_DYNLOADING

    Module( const ::rtl::OUString& strModuleName, sal_Int32 nRtldMode = SAL_LOADMODULE_DEFAULT) : m_Module(NULL)
    {
        load( strModuleName, nRtldMode);
    }

#endif

    ~Module()
    {
#ifndef DISABLE_DYNLOADING
        osl_unloadModule(m_Module);
#endif
    }

#ifndef DISABLE_DYNLOADING

    bool SAL_CALL load( const ::rtl::OUString& strModuleName,
        sal_Int32 nRtldMode = SAL_LOADMODULE_DEFAULT)
    {
        unload();
        m_Module= osl_loadModule( strModuleName.pData, nRtldMode );
        return is();
    }

    /// @since UDK 3.2.8
    bool SAL_CALL loadRelative(
        ::oslGenericFunction baseModule, ::rtl::OUString const & relativePath,
        ::sal_Int32 mode = SAL_LOADMODULE_DEFAULT)
    {
        unload();
        m_Module = osl_loadModuleRelative(baseModule, relativePath.pData, mode);
        return is();
    }

    /// @since LibreOffice 3.5
    bool SAL_CALL loadRelative(
        oslGenericFunction baseModule, char const * relativePath,
        sal_Int32 mode = SAL_LOADMODULE_DEFAULT)
    {
        unload();
        m_Module = osl_loadModuleRelativeAscii(baseModule, relativePath, mode);
        return is();
    }

    void SAL_CALL unload()
    {
        if (m_Module)
        {
            osl_unloadModule(m_Module);
            m_Module = NULL;
        }
    }

#endif

    bool SAL_CALL is() const
    {
           return m_Module != NULL;
    }

    void* SAL_CALL getSymbol( const ::rtl::OUString& strSymbolName)
    {
        return osl_getSymbol( m_Module, strSymbolName.pData );
    }

    /** Get function address by the function name in the module.

        getFunctionSymbol is an alternative function for getSymbol.
        Use Function pointer as symbol address to conceal type conversion.

        @param[in] ustrFunctionSymbolName  Function name to be looked up.

        @retval oslGenericFunction format function address on success
        @retval NULL lookup failed or parameter is somewhat invalid

        @see getSymbol
    */
    oslGenericFunction SAL_CALL getFunctionSymbol( const ::rtl::OUString& ustrFunctionSymbolName ) const
    {
        return osl_getFunctionSymbol( m_Module, ustrFunctionSymbolName.pData );
    }

    /// @since LibreOffice 3.5
    oslGenericFunction SAL_CALL getFunctionSymbol(char const * name) const {
        return osl_getAsciiFunctionSymbol(m_Module, name);
    }

    operator oslModule() const
    {
        return m_Module;
    }

    /** Release the module so that it will not be unloaded from the destructor.

        This instance returns to the state of a default-constructed instance
        again.

        @since LibreOffice 4.3
    */
    void release() { m_Module = NULL; }

private:
    oslModule m_Module;

};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
