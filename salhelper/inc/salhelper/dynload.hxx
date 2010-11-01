/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SALHELPER_DYNLOAD_HXX_
#define _SALHELPER_DYNLOAD_HXX_

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <osl/module.h>

namespace salhelper
{

/** The ORealDynamicLoader is an implementation helper class for the template loader ODynamicLoader.
 */
class ORealDynamicLoader
{
public:
    /** initializes the loader, loads the library and call the initialization fucntion.

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

    /// returns a poiner to the initialized API function structure.
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

    /// points to  the structure with the initialzed API function pointers.
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


/** The ODynmaicLoader provides a special load on call mechanism for dynamic libraries
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
    ODynamicLoader() SAL_THROW(())
    {
        m_pLoader = 0;
    }

    /** Constructor, loads the library if necessary otherwise the refernece count will
        be increased.

        @param strModuleName specifies the library name.
        @param strInitFunction specifies the name of the initialization function.
     */
    ODynamicLoader( const ::rtl::OUString& strModuleName,
                       const ::rtl::OUString& strInitFunction ) SAL_THROW(())
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
    ODynamicLoader(const ODynamicLoader<API>& toCopy) SAL_THROW(())
    {
        m_pLoader = toCopy.m_pLoader;
        if( m_pLoader )
            m_pLoader->acquire();
    }

    /// Destructor, decrease the reference count and unload the library if it is tha last instance.
    ~ODynamicLoader() SAL_THROW(())
    {
        if( m_pLoader )
            m_pLoader->release();
    }

    /// Assign operator
    ODynamicLoader<API>& SAL_CALL operator = (const ODynamicLoader<API>& toAssign) SAL_THROW(())
    {
        if( m_pLoader != toAssign.m_pLoader )
        {
            if( toAssign.m_pLoader )
            toAssign.m_pLoader->acquire();
            if( m_pLoader )
            m_pLoader->release();
            m_pLoader = toAssign.m_pLoader;
        }

        return (*this);
    }

    /// returns a poiner to the initialized API function structure.
    API* SAL_CALL getApi() const SAL_THROW(())
    {
        return (API*)m_pLoader->getApi();
    }

    /// cast operator, which cast to a poiner with the initialized API function structure.
    API* SAL_CALL operator->() const SAL_THROW(())
    {
        return (API*)m_pLoader->getApi();
    }

    /// checks if the loader works on a loaded and initialized library.
    sal_Bool SAL_CALL isLoaded() const SAL_THROW(())
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
