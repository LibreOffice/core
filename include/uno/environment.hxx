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
#ifndef INCLUDED_UNO_ENVIRONMENT_HXX
#define INCLUDED_UNO_ENVIRONMENT_HXX

#include <rtl/alloc.h>
#include <rtl/ustring.hxx>
#include <uno/environment.h>

#include <uno/lbnames.h>

/** */ //for docpp
namespace com
{
/** */ //for docpp
namespace sun
{
/** */ //for docpp
namespace star
{
/** */ //for docpp
namespace uno
{

/** C++ wrapper for binary C uno_Environment.

    @see uno_Environment
*/
class Environment
{
    /** binary C uno_Environment
    */
    uno_Environment * _pEnv;

public:
    /** Returns the current Environment.

        @param typeName   the optional type of the Environment, falls back to "uno" in case being empty,
                          respectively to current C++ Environment.
        @since UDK 3.2.7
    */
    inline static Environment getCurrent(rtl::OUString const & typeName = rtl::OUString(CPPU_CURRENT_LANGUAGE_BINDING_NAME)) SAL_THROW(());

    /// @cond INTERNAL
    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new ( size_t nSize ) SAL_THROW(())
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete ( void * pMem ) SAL_THROW(())
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new ( size_t, void * pMem ) SAL_THROW(())
        { return pMem; }
    inline static void SAL_CALL operator delete ( void *, void * ) SAL_THROW(())
        {}
    /// @endcond

    /** Constructor: acquires given environment

        @param pEnv environment
    */
    inline Environment( uno_Environment * pEnv = 0 ) SAL_THROW(());

    /** Gets a specific environment. If the specified environment does not exist, then a default one
        is created and registered.

        @param envDcp           descriptor of the environment
        @param pContext         context pointer
    */
    inline explicit Environment( rtl::OUString const & envDcp, void * pContext = NULL ) SAL_THROW(());


    /** Copy constructor: acquires given environment

        @param rEnv another environment
    */
    inline Environment( const Environment & rEnv ) SAL_THROW(());

    /** Destructor: releases a set environment.
    */
    inline ~Environment() SAL_THROW(());

    /** Sets a given environment, i.e. acquires given one and releases a set one.

        @param pEnv another environment
        @return this environment
    */
    inline Environment & SAL_CALL operator = ( uno_Environment * pEnv ) SAL_THROW(());
    /** Sets a given environment, i.e. acquires given one and releases a set one.

        @param rEnv another environment
        @return this environment
    */
    inline Environment & SAL_CALL operator = ( const Environment & rEnv ) SAL_THROW(())
        { return operator = ( rEnv._pEnv ); }

    /** Provides UNacquired pointer to the set C environment.

        @return UNacquired pointer to the C environment struct
    */
    inline uno_Environment * SAL_CALL get() const SAL_THROW(())
        { return _pEnv; }

    /** Gets type name of set environment.

        @return type name of set environment
    */
    inline ::rtl::OUString SAL_CALL getTypeName() const SAL_THROW(())
        { return _pEnv->pTypeName; }

    /** Gets free context pointer of set environment.

        @return free context pointer of set environment
    */
    inline void * SAL_CALL getContext() const SAL_THROW(())
        { return _pEnv->pContext; }

    /** Tests if a environment is set.

        @return true, if a environment is set, false otherwise
    */
    inline bool SAL_CALL is() const SAL_THROW(())
        { return (_pEnv != 0); }

    /** Releases a set environment.
    */
    inline void SAL_CALL clear() SAL_THROW(());

    /** Invoke the passed function in this environment.

        @param pCallee  the function to call
        @param pParam   the parameter pointer to be passed to the function
        @since UDK 3.2.7
    */
    inline void SAL_CALL invoke_v(uno_EnvCallee * pCallee, va_list * pParam) const SAL_THROW(());

    /** Invoke the passed function in this environment.

        @param pCallee  the function to call
        @param ...      the parameters to be passed to the function
        @since UDK 3.2.7
    */
    inline void SAL_CALL invoke(uno_EnvCallee * pCallee, ...) const SAL_THROW(());

    /** Enter this environment explicitly.

        @since UDK 3.2.7
    */
    inline void SAL_CALL enter() const SAL_THROW(());

    /** Checks, if it is valid to currently call objects
        belonging to this environment.

        @since UDK 3.2.7
    */
    inline int  SAL_CALL isValid(rtl::OUString * pReason) const SAL_THROW(());
};

inline Environment::Environment( uno_Environment * pEnv ) SAL_THROW(())
    : _pEnv( pEnv )
{
    if (_pEnv)
        (*_pEnv->acquire)( _pEnv );
}

inline Environment::Environment( rtl::OUString const & rEnvDcp, void * pContext ) SAL_THROW(())
    : _pEnv(NULL)
{
    uno_getEnvironment(&_pEnv, rEnvDcp.pData, pContext);
}

inline Environment::Environment( const Environment & rEnv ) SAL_THROW(())
    : _pEnv( rEnv._pEnv )
{
    if (_pEnv)
        (*_pEnv->acquire)( _pEnv );
}

inline Environment::~Environment() SAL_THROW(())
{
    if (_pEnv)
        (*_pEnv->release)( _pEnv );
}

inline void Environment::clear() SAL_THROW(())
{
    if (_pEnv)
    {
        (*_pEnv->release)( _pEnv );
        _pEnv = 0;
    }
}

inline Environment & Environment::operator = ( uno_Environment * pEnv ) SAL_THROW(())
{
    if (pEnv != _pEnv)
    {
        if (pEnv)
            (*pEnv->acquire)( pEnv );
        if (_pEnv)
            (*_pEnv->release)( _pEnv );
        _pEnv = pEnv;
    }
    return *this;
}

inline void SAL_CALL Environment::invoke_v(uno_EnvCallee * pCallee, va_list * pParam) const SAL_THROW(())
{
    if (_pEnv)
        uno_Environment_invoke_v(_pEnv, pCallee, pParam);
}

inline void SAL_CALL Environment::invoke(uno_EnvCallee * pCallee, ...) const SAL_THROW(())
{
    if (_pEnv)
    {
        va_list param;

        va_start(param, pCallee);
        uno_Environment_invoke_v(_pEnv, pCallee, &param);
        va_end(param);
    }

}

inline void SAL_CALL Environment::enter() const SAL_THROW(())
{
    uno_Environment_enter(_pEnv);
}

inline int  SAL_CALL Environment::isValid(rtl::OUString * pReason) const SAL_THROW(())
{
    return uno_Environment_isValid(_pEnv, (rtl_uString **)pReason);
}

inline Environment Environment::getCurrent(rtl::OUString const & typeName) SAL_THROW(())
{
    Environment environment;

    uno_Environment * pEnv = NULL;
    uno_getCurrentEnvironment(&pEnv, typeName.pData);
    environment = pEnv;
    if (pEnv)
        pEnv->release(pEnv);

    return environment;
}

}
}
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
