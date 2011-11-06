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
    { return OUString::createFromAscii("com.sun.star.comp.extensions.oooimprovement.CoreController"); }

    Sequence<OUString> SAL_CALL CoreController::getSupportedServiceNames_static()
    {
        Sequence<OUString> aServiceNames(1);
        aServiceNames[0] = OUString::createFromAscii("com.sun.star.oooimprovement.CoreController");
        return aServiceNames;
    }

    Reference<XInterface> SAL_CALL CoreController::Create(const Reference<XMultiServiceFactory>& sm)
    { return *(new CoreController(sm)); }
}
