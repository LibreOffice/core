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

#ifndef INCLUDED_SALHELPER_DYNLOAD_HXX
#define INCLUDED_SALHELPER_DYNLOAD_HXX

#include "sal/types.h"
#include "rtl/ustring.hxx"
#include "osl/module.h"
#include "salhelper/salhelperdllapi.h"

namespace salhelper
{

/** The ORealDynamicLoader is an implementation helper class for the template loader ODynamicLoader.
 */
class SALHELPER_DLLPUBLIC ORealDynamicLoader
{
public:
    /** initializes the loader, loads the library and call the initialization function.

        @param ppSetToZeroInDestructor points to the loader instance which must be set to NULL
                                       if the loader will be destroyed.
        @param strModuleName specifies the library name.
        @param strInitFunction specifies the name of the initialization function.
     */
    static ORealDynamicLoader* SAL_CALL newInstance(
            ORealDynamicLoader ** ppSetToZeroInDestructor,
            const ::rtl::OUString& strModuleName,
            const ::rtl::OUString& strInitFunction );

    /// increase the reference count.
    sal_uInt32 SAL_CALL acquire();
    /// decrease the reference count and delete the last instance.
    sal_uInt32 SAL_CALL release();

    /// returns a pointer to the initialized API function structure.
    void* SAL_CALL getApi() const;

protected:
    /** Constructor.

        @param ppSetToZeroInDestructor points to the loader instance which must be set to NULL
                                       if the loader will be destroyed.
        @param strModuleName specifies the library name.
        @param strInitFunction specifies the name of the initialization function.
        @param pApi points to a structure with the initialized API function pointers.
        @param pModule points to the loaded library handle.
     */
    ORealDynamicLoader( ORealDynamicLoader ** ppSetToZeroInDestructor,
                        const ::rtl::OUString& strModuleName,
                        const ::rtl::OUString& strInitFunction,
                        void* pApi,
                        oslModule pModule );

    /// Destructor, try to unload the library.
    virtual ~ORealDynamicLoader();

    /// points to  the structure with the initialized API function pointers.
    void*                   m_pApi;
    /// stores the reference count.
    sal_uInt32              m_refCount;
    /// stores the library handle.
    oslModule               m_pModule;
    /// stores the library name.
    ::rtl::OUString         m_strModuleName;
    /// stores the name of the initialization function.
    ::rtl::OUString         m_strInitFunction;
    /** stores a pointer to itself, which must be reset in the destructor to signal
        that the loader is invalid.
    */
    ORealDynamicLoader **   ppSetToZeroInDestructor;
};


/** The ODynamicLoader provides a special load on call mechanism for dynamic libraries
    which support a C-API.

    The libraries must provide a struct with function pointers for all supported C functions.
    The loader loads the specified library and call the specified initialization function
    to initialize the function pointers with the real functions. Furthermore provides the
    loader a reference counter for the library. When the last instance of the laoder will
    be destroyed the loader will unload the library.

    @deprecated
    Do not use.
 */
template<class API>
class ODynamicLoader
{
public:
    /// Default constructor
    ODynamicLoader()
    {
        m_pLoader = NULL;
    }

    /** Constructor, loads the library if necessary otherwise the reference count will
        be increased.

        @param strModuleName specifies the library name.
        @param strInitFunction specifies the name of the initialization function.
     */
    ODynamicLoader( const ::rtl::OUString& strModuleName,
                       const ::rtl::OUString& strInitFunction )
    {
        if (!m_pStaticLoader)
        {
            m_pStaticLoader = ORealDynamicLoader::newInstance(
               &m_pStaticLoader,
               strModuleName,
               strInitFunction);
        }
        else
        {
            m_pStaticLoader->acquire();
        }

        m_pLoader = m_pStaticLoader;
    }

    /// Copy constructor
    ODynamicLoader(const ODynamicLoader<API>& toCopy)
    {
        m_pLoader = toCopy.m_pLoader;
        if( m_pLoader )
            m_pLoader->acquire();
    }

    /// Destructor, decrease the reference count and unload the library if it is the last instance.
    ~ODynamicLoader()
    {
        if( m_pLoader )
            if (m_pLoader->release()==0)
                m_pStaticLoader = NULL;
    }

    /// Assign operator
    ODynamicLoader<API>& SAL_CALL operator = (const ODynamicLoader<API>& toAssign)
    {
        if( m_pLoader != toAssign.m_pLoader )
        {
            if( toAssign.m_pLoader )
            {
                toAssign.m_pLoader->acquire();
            }
            if( m_pLoader )
            {
                m_pLoader->release();
            }
            m_pLoader = toAssign.m_pLoader;
        }

        return (*this);
    }

    /// returns a pointer to the initialized API function structure.
    API* SAL_CALL getApi() const
    {
        return static_cast<API*>(m_pLoader->getApi());
    }

    /// cast operator, which cast to a pointer with the initialized API function structure.
    API* SAL_CALL operator->() const
    {
        return static_cast<API*>(m_pLoader->getApi());
    }

    /// checks if the loader works on a loaded and initialized library.
    bool SAL_CALL isLoaded() const
    {
        return (m_pLoader != NULL);
    }

protected:
    /// stores the real loader helper instance
    static ORealDynamicLoader*  m_pStaticLoader;
    ORealDynamicLoader*         m_pLoader;
};


template<class API>
ORealDynamicLoader* ODynamicLoader<API>::m_pStaticLoader = NULL;

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
