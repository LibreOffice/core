/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <ServiceDocumenter.hxx>
#include <comphelper/servicedecl.hxx>
#include <com/sun/star/system/XSystemShellExecute.hpp>
using namespace com::sun::star;
using uno::Reference;
using lang::XServiceInfo;
using lang::XTypeProvider;

void unotools::misc::ServiceDocumenter::showCoreDocs(const Reference<XServiceInfo>& xService)
    throw (css::uno::RuntimeException, std::exception)
{
    if(!xService.is())
        return;
    auto xMSF(m_xContext->getServiceManager());
    Reference<system::XSystemShellExecute> xShell(xMSF->createInstanceWithContext("com.sun.star.system.SystemShellExecute", m_xContext), uno::UNO_QUERY);
    xShell->execute(m_sCoreBaseUrl + xService->getImplementationName() + ".html", "", 0);
}

void unotools::misc::ServiceDocumenter::showInterfaceDocs(const Reference<XTypeProvider>& xTypeProvider)
    throw (css::uno::RuntimeException, std::exception)
{
    if(!xTypeProvider.is())
        return;
    auto xMSF(m_xContext->getServiceManager());
    Reference<system::XSystemShellExecute> xShell(xMSF->createInstanceWithContext("com.sun.star.system.SystemShellExecute", m_xContext), uno::UNO_QUERY);
    for(auto aType : xTypeProvider->getTypes())
    {
        auto sUrl = aType.getTypeName();
        sal_Int32 nIdx = 0;
        while(nIdx != -1)
            sUrl = sUrl.replaceFirst(".", "_1_1", &nIdx);
        xShell->execute(m_sServiceBaseUrl + "/interface" + sUrl + ".html", "", 0);
    }
}

void unotools::misc::ServiceDocumenter::showServiceDocs(const Reference<XServiceInfo>& xService)
    throw (css::uno::RuntimeException, std::exception)
{
    if(!xService.is())
        return;
    auto xMSF(m_xContext->getServiceManager());
    Reference<system::XSystemShellExecute> xShell(xMSF->createInstanceWithContext("com.sun.star.system.SystemShellExecute", m_xContext), uno::UNO_QUERY);
    for(auto sService : xService->getSupportedServiceNames())
    {
        auto sUrl = sService;
        sal_Int32 nIdx = 0;
        while(nIdx != -1)
            sUrl = sUrl.replaceFirst(".", "_1_1", &nIdx);
        xShell->execute(m_sServiceBaseUrl + "/service" + sUrl + ".html", "", 0);
    }
}

namespace sdecl = ::comphelper::service_decl;
sdecl::class_< unotools::misc::ServiceDocumenter > ServiceDocumenterImpl;
extern const sdecl::ServiceDecl ServiceDocumenterDecl(
    ServiceDocumenterImpl,
    "com.sun.star.comp.unotools.misc.ServiceDocumenter",
    "");

