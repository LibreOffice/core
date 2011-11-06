/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
        static OUString sNoFirstStartWizard = OUString::createFromAscii("-nofirststartwizard");
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
//    InviteJob::InviteJob(const Reference<XComponentContext>& context)
//        : m_ServiceFactory(Reference<XMultiServiceFactory>(
//            context->getServiceManager()->createInstanceWithContext(
//                OUString::createFromAscii("com.sun.star.lang.XMultiServiceFactory"), context),
//            UNO_QUERY))
//    { }

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
                    m_ServiceFactory->createInstance(OUString::createFromAscii("com.sun.star.oooimprovement.Core")),
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
    { return OUString::createFromAscii("com.sun.star.comp.extensions.oooimprovement.InviteJob"); }

    Sequence<OUString> SAL_CALL InviteJob::getSupportedServiceNames_static()
    {
        Sequence<OUString> aServiceNames(1);
        aServiceNames[0] = OUString::createFromAscii("com.sun.star.task.AsyncJob");
        return aServiceNames;
    }

//    Reference<XInterface> InviteJob::Create(const Reference<XComponentContext>& context)
//    { return *(new InviteJob(context)); }

    Reference<XInterface> InviteJob::Create(const Reference<XMultiServiceFactory>& sm)
    { return *(new InviteJob(sm)); }
}
