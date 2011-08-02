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

#if ! defined INCLUDED_MONO_BRIDGE_H
#define INCLUDED_MONO_BRIDGE_H

#include "glib/gtypes.h"
#include "osl/interlck.h"
#include "rtl/ustring.hxx"
#include "typelib/typedescription.hxx"
#include "uno/dispatcher.h"
#include "uno/mapping.h"

extern "C" {
#include "mono/metadata/appdomain.h"
#include "mono/metadata/debug-helpers.h"
#include "mono/metadata/object.h"
#include "mono/metadata/threads.h"
}

namespace cssu = com::sun::star::uno;

typedef struct _uno_ExtEnvironment uno_ExtEnvironment;
typedef struct _uno_Environment uno_Environment;
typedef struct _typelib_TypeDescription typelib_TypeDescription;

namespace mono_uno
{

MonoObject* runtime_invoke (MonoMethod *method, void *obj, void **params,
                            MonoObject **exc, MonoDomain *domain);

//==== holds environments and mappings =========================================
struct Bridge;
struct Mapping : public uno_Mapping
{
    Bridge * m_bridge;
};

//==============================================================================
struct Bridge
{
    mutable oslInterlockedCount m_ref;
    MonoObject *                m_managedBridge;

    uno_ExtEnvironment *        m_uno_env;
    uno_Environment *           m_mono_env;

    Mapping                     m_mono2uno;
    Mapping                     m_uno2mono;
    bool                        m_registered_mono2uno;

    MonoMethod *                m_mapUnoToManagedMethod;
    MonoMethod *                m_mapManagedToUnoMethod;

    ~Bridge() SAL_THROW( () );
    explicit Bridge(
        uno_Environment * mono_env, uno_ExtEnvironment * uno_env,
        bool registered_mono2uno );

    void acquire() const;
    void release() const;

    void * map_to_mono(
        uno_Interface * pUnoI, typelib_TypeDescription * pTD ) const;

    uno_Interface * map_to_uno(
        void * pMonoI, typelib_TypeDescription * pTD ) const;
};

struct MonoProxy : public uno_Interface
{
    mutable oslInterlockedCount m_ref;
    guint32                     m_managedProxy;
    uno_ExtEnvironment *        m_unoEnv;
    const cssu::TypeDescription m_unoType;
    const rtl::OUString         m_Oid;
    MonoMethod *                m_managedDispatch;

    void acquire() const;
    void release() const;
    void dispatch(typelib_TypeDescription const * member_td, void * uno_ret,
                  void * uno_args [], uno_Any ** uno_exc);

    MonoProxy(uno_ExtEnvironment * pUnoEnv, guint32 managedProxy,
              rtl_uString * pOid, typelib_TypeDescription * pTD);
};

struct BridgeRuntimeError
{
    ::rtl::OUString m_message;

    inline BridgeRuntimeError( ::rtl::OUString const & message )
        : m_message( message )
        {}
};

}

#endif
