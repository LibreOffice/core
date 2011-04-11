/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include "corecontroller.hxx"
#include "invite_job.hxx"
#include "onlogrotate_job.hxx"
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <cppuhelper/factory.hxx>
#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>


using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::uno;
using namespace ::oooimprovement;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;


namespace
{
    void writeInfo(const Reference<XRegistryKey>& reg_key,
        const OUString& implementation_name,
        const OUString& service_name)
    {
        OUStringBuffer buf(256);
        buf.append(implementation_name);
        buf.appendAscii("/UNO/SERVICES/");
        buf.append(service_name);
        reg_key->createKey(buf.makeStringAndClear());
    }
}

extern "C"
{
    void SAL_CALL component_getImplementationEnvironment(const sal_Char** env_type_name, uno_Environment**)
    { *env_type_name = CPPU_CURRENT_LANGUAGE_BINDING_NAME; }

    void* SAL_CALL component_getFactory(const sal_Char* pImplName, void* pServiceManager, void*)
    {
        if ( !pServiceManager || !pImplName ) return 0;

        Reference<XSingleServiceFactory> factory;
        Reference<XMultiServiceFactory>  sm(reinterpret_cast<XMultiServiceFactory*>(pServiceManager), UNO_QUERY);
        OUString impl_name = OUString::createFromAscii(pImplName);
        Sequence<OUString> names(1);
        names[0] = impl_name;

        if (impl_name.equals(CoreController::getImplementationName_static()))
            factory = ::cppu::createSingleFactory(sm, impl_name, CoreController::Create, names);
        if (impl_name.equals(OnLogRotateJob::getImplementationName_static()))
            factory = ::cppu::createSingleFactory(sm, impl_name, OnLogRotateJob::Create, names);
        if (impl_name.equals(InviteJob::getImplementationName_static()))
            factory = ::cppu::createSingleFactory(sm, impl_name, InviteJob::Create, names);
        if (!factory.is()) return 0;
        factory->acquire();
        return factory.get();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
