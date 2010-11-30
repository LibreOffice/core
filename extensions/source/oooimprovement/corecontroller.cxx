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
#include "config.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;


namespace oooimprovement
{
    CoreController::CoreController(const Reference<XMultiServiceFactory>& sf)
        : m_ServiceFactory(sf)
    { }

    CoreController::~CoreController()
    { }

    sal_Bool SAL_CALL CoreController::enablingUiEventsLoggerAllowed(sal_Int16 version) throw(RuntimeException)
    {
        Config config(m_ServiceFactory);
        if(version==1 && config.getEnablingAllowed() && config.getShowedInvitation() && config.getInvitationAccepted())
            return true;
        return false;
    }

    sal_Bool SAL_CALL CoreController::showBuiltinOptionsPage(sal_Int16 version) throw(RuntimeException)
    {
        Config config(m_ServiceFactory);
        if(version==1 && config.getEnablingAllowed())
            return true;
        return false;
    }

    sal_Bool SAL_CALL CoreController::supportsService(const OUString& service_name) throw(RuntimeException)
    {
        const Sequence<OUString> service_names(getSupportedServiceNames());
        for (sal_Int32 idx = service_names.getLength()-1; idx>=0; --idx)
            if(service_name == service_names[idx]) return sal_True;
        return sal_False;
    }

    OUString SAL_CALL CoreController::getImplementationName() throw(RuntimeException)
    { return getImplementationName_static(); }

    Sequence<OUString> SAL_CALL CoreController::getSupportedServiceNames() throw(RuntimeException)
    { return getSupportedServiceNames_static(); }

    OUString SAL_CALL CoreController::getImplementationName_static()
    { return OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.extensions.oooimprovement.CoreController")); }

    Sequence<OUString> SAL_CALL CoreController::getSupportedServiceNames_static()
    {
        Sequence<OUString> aServiceNames(1);
        aServiceNames[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.oooimprovement.CoreController"));
        return aServiceNames;
    }

    Reference<XInterface> SAL_CALL CoreController::Create(const Reference<XMultiServiceFactory>& sm)
    { return *(new CoreController(sm)); }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
