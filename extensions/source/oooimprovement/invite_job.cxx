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


#include "invite_job.hxx"
#include "config.hxx"
#include "logstorage.hxx"
#include <com/sun/star/oooimprovement/XCore.hpp>
#include <rtl/process.h>


using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::uno;
using ::com::sun::star::oooimprovement::XCore;
using ::rtl::OUString;

namespace
{
    // dont show Invitation, when:
    // -nofirststartwizard commandline switch is present
    // [add additional conditions here]
    static bool lcl_IsInvitationAllowed()
    {
        static OUString sNoFirstStartWizard(RTL_CONSTASCII_USTRINGPARAM("-nofirststartwizard"));
        sal_Int32 nCount = rtl_getAppCommandArgCount();
        for(sal_Int32 nCurrent=0; nCurrent<nCount; nCurrent++)
        {
            OUString sArg;
            rtl_getAppCommandArg(nCurrent, &sArg.pData);
            if(sNoFirstStartWizard == sArg)
                return false;
        }
        return true;
    }
}

namespace oooimprovement
{

    InviteJob::InviteJob(const Reference<XMultiServiceFactory>& sf)
        : m_ServiceFactory(sf)
    { }

    InviteJob::~InviteJob()
    { }

    void SAL_CALL InviteJob::executeAsync(const Sequence<NamedValue>&, const Reference<XJobListener>& listener) throw(RuntimeException)
    {
        Config config(m_ServiceFactory);
        {
            LogStorage log_storage(m_ServiceFactory);
            log_storage.assureExists();
        }
        if(config.getOfficeStartCounterdown() > 0)
            config.decrementOfficeStartCounterdown(1);
        else
        {
            if(lcl_IsInvitationAllowed() && !config.getShowedInvitation())
            {
                Reference<XCore> core(
                    m_ServiceFactory->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.oooimprovement.Core"))),
                    UNO_QUERY);
                if(core.is()) core->inviteUser();
            }
        }
        Any result;
        listener->jobFinished(Reference<XAsyncJob>(this), result);
    }

    sal_Bool SAL_CALL InviteJob::supportsService(const OUString& service_name) throw(RuntimeException)
    {
        const Sequence<OUString> service_names(getSupportedServiceNames());
        for (sal_Int32 idx = service_names.getLength()-1; idx>=0; --idx)
            if(service_name == service_names[idx]) return sal_True;
        return sal_False;
    }

    OUString SAL_CALL InviteJob::getImplementationName() throw(RuntimeException)
    { return getImplementationName_static(); }

    Sequence<OUString> SAL_CALL InviteJob::getSupportedServiceNames() throw(RuntimeException)
    { return getSupportedServiceNames_static(); }

    OUString SAL_CALL InviteJob::getImplementationName_static()
    { return OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.extensions.oooimprovement.InviteJob")); }

    Sequence<OUString> SAL_CALL InviteJob::getSupportedServiceNames_static()
    {
        Sequence<OUString> aServiceNames(1);
        aServiceNames[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.task.AsyncJob"));
        return aServiceNames;
    }

    Reference<XInterface> InviteJob::Create(const Reference<XMultiServiceFactory>& sm)
    { return *(new InviteJob(sm)); }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
