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

#include "sal/config.h"

#include <cstddef>

#include "rtl/alloc.h"
#include "rtl/ustring.hxx"
#include "uno/environment.h"

#include "uno/lbnames.h"

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
    inline static Environment getCurrent(rtl::OUString const & typeName = rtl::OUString(CPPU_CURRENT_LANGUAGE_BINDING_NAME));

    /// @cond INTERNAL
    // these are here to force memory de/allocation to sal lib.
    static void * SAL_CALL operator new ( size_t nSize )
        { return ::rtl_allocateMemory( nSize ); }
    static void SAL_CALL operator delete ( void * pMem )
        { ::rtl_freeMemory( pMem ); }
    static void * SAL_CALL operator new ( size_t, void * pMem )
        { return pMem; }
    static void SAL_CALL operator delete ( void *, void * )
        {}
    /// @endcond

    /** Constructor: acquires given environment

        @param pEnv environment
    */
    inline Environment( uno_Environment * pEnv = NULL );

    /** Gets a specific environment. If the specified environment does not exist, then a default one
        is created and registered.

        @param envDcp           descriptor of the environment
        @param pContext         context pointer
    */
    inline explicit Environment( rtl::OUString const & envDcp, void * pContext = NULL );


    /** Copy constructor: acquires given environment

        @param rEnv another environment
    */
    inline Environment( const Environment & rEnv );

#if defined LIBO_INTERNAL_ONLY
    Environment(Environment && other): _pEnv(other._pEnv)
    { other._pEnv = nullptr; }
#endif

    /** Destructor: releases a set environment.
    */
    inline ~Environment();

    /** Sets a given environment, i.e. acquires given one and releases a set one.

        @param pEnv another environment
        @return this environment
    */
    inline Environment & SAL_CALL operator = ( uno_Environment * pEnv );
    /** Sets a given environment, i.e. acquires given one and releases a set one.

        @param rEnv another environment
        @return this environment
    */
    Environment & SAL_CALL operator = ( const Environment & rEnv )
        { return operator = ( rEnv._pEnv ); }

#if defined LIBO_INTERNAL_ONLY
    Environment & operator =(Environment && other) {
        if (_pEnv != nullptr) {
            (*_pEnv->release)(_pEnv);
        }
        _pEnv = other._pEnv;
        other._pEnv = nullptr;
        return *this;
    }
#endif

    /** Provides UNacquired pointer to the set C environment.

        @return UNacquired pointer to the C environment struct
    */
    uno_Environment * SAL_CALL get() const
        { return _pEnv; }

    /** Gets type name of set environment.

        @return type name of set environment
    */
    ::rtl::OUString SAL_CALL getTypeName() const
        { return _pEnv->pTypeName; }

    /** Gets free context pointer of set environment.

        @return free context pointer of set environment
    */
    void * SAL_CALL getContext() const
        { return _pEnv->pContext; }

    /** Tests if a environment is set.

        @return true, if a environment is set, false otherwise
    */
    bool SAL_CALL is() const
        { return (_pEnv != NULL); }

    /** Releases a set environment.
    */
    inline void SAL_CALL clear();

    /** Invoke the passed function in this environment.

        @param pCallee  the function to call
        @param pParam   the parameter pointer to be passed to the function
        @since UDK 3.2.7
    */
    inline void SAL_CALL invoke_v(uno_EnvCallee * pCallee, va_list * pParam) const;

    /** Invoke the passed function in this environment.

        @param pCallee  the function to call
        @param ...      the parameters to be passed to the function
        @since UDK 3.2.7
    */
    inline void SAL_CALL invoke(uno_EnvCallee * pCallee, ...) const;

    /** Enter this environment explicitly.

        @since UDK 3.2.7
    */
    inline void SAL_CALL enter() const;

    /** Checks, if it is valid to currently call objects
        belonging to this environment.

        @since UDK 3.2.7
    */
    inline int  SAL_CALL isValid(rtl::OUString * pReason) const;
};

inline Environment::Environment( uno_Environment * pEnv )
    : _pEnv( pEnv )
{
    if (_pEnv)
        (*_pEnv->acquire)( _pEnv );
}

inline Environment::Environment( rtl::OUString const & rEnvDcp, void * pContext )
    : _pEnv(NULL)
{
    uno_getEnvironment(&_pEnv, rEnvDcp.pData, pContext);
}

inline Environment::Environment( const Environment & rEnv )
    : _pEnv( rEnv._pEnv )
{
    if (_pEnv)
        (*_pEnv->acquire)( _pEnv );
}

inline Environment::~Environment()
{
    if (_pEnv)
        (*_pEnv->release)( _pEnv );
}

inline void Environment::clear()
{
    if (_pEnv)
    {
        (*_pEnv->release)( _pEnv );
        _pEnv = NULL;
    }
}

inline Environment & Environment::operator = ( uno_Environment * pEnv )
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

inline void SAL_CALL Environment::invoke_v(uno_EnvCallee * pCallee, va_list * pParam) const
{
    if (_pEnv)
        uno_Environment_invoke_v(_pEnv, pCallee, pParam);
}

inline void SAL_CALL Environment::invoke(uno_EnvCallee * pCallee, ...) const
{
    if (_pEnv)
    {
        va_list param;

        va_start(param, pCallee);
        uno_Environment_invoke_v(_pEnv, pCallee, &param);
        va_end(param);
    }

}

inline void SAL_CALL Environment::enter() const
{
    uno_Environment_enter(_pEnv);
}

inline int  SAL_CALL Environment::isValid(rtl::OUString * pReason) const
{
    return uno_Environment_isValid(_pEnv, &pReason->pData);
}

inline Environment Environment::getCurrent(rtl::OUString const & typeName)
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
