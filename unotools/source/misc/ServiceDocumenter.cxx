/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "ServiceDocumenter.hxx"
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>

using namespace com::sun::star;
using uno::Reference;
using lang::XServiceInfo;
using lang::XTypeProvider;

void unotools::misc::ServiceDocumenter::showCoreDocs(const Reference<XServiceInfo>& xService)
{
    if(!xService.is())
        return;
    auto xMSF(m_xContext->getServiceManager());
    Reference<system::XSystemShellExecute> xShell(xMSF->createInstanceWithContext("com.sun.star.system.SystemShellExecute", m_xContext), uno::UNO_QUERY);
    xShell->execute(
        m_sCoreBaseUrl + xService->getImplementationName() + ".html", "",
        css::system::SystemShellExecuteFlags::URIS_ONLY);
}

void unotools::misc::ServiceDocumenter::showInterfaceDocs(const Reference<XTypeProvider>& xTypeProvider)
{
    if(!xTypeProvider.is())
        return;
    auto xMSF(m_xContext->getServiceManager());
    Reference<system::XSystemShellExecute> xShell(xMSF->createInstanceWithContext("com.sun.star.system.SystemShellExecute", m_xContext), uno::UNO_QUERY);
    const css::uno::Sequence<css::uno::Type> aTypes = xTypeProvider->getTypes();
    for(const auto& aType : aTypes)
    {
        auto sUrl = aType.getTypeName();
        sal_Int32 nIdx = 0;
        while(nIdx != -1)
            sUrl = sUrl.replaceFirst(".", "_1_1", &nIdx);
        xShell->execute(
            m_sServiceBaseUrl + "/interface" + sUrl + ".html", "",
            css::system::SystemShellExecuteFlags::URIS_ONLY);
    }
}

void unotools::misc::ServiceDocumenter::showServiceDocs(const Reference<XServiceInfo>& xService)
{
    if(!xService.is())
        return;
    auto xMSF(m_xContext->getServiceManager());
    Reference<system::XSystemShellExecute> xShell(xMSF->createInstanceWithContext("com.sun.star.system.SystemShellExecute", m_xContext), uno::UNO_QUERY);
    const css::uno::Sequence<OUString> aServiceNames = xService->getSupportedServiceNames();
    for(const auto& sService : aServiceNames)
    {
        auto sUrl = sService;
        sal_Int32 nIdx = 0;
        while(nIdx != -1)
            sUrl = sUrl.replaceFirst(".", "_1_1", &nIdx);
        xShell->execute(
            m_sServiceBaseUrl + "/service" + sUrl + ".html", "",
            css::system::SystemShellExecuteFlags::URIS_ONLY);
    }
}

//  XServiceInfo
sal_Bool unotools::misc::ServiceDocumenter::supportsService(const OUString& sServiceName)
{
    return cppu::supportsService(this, sServiceName);
}
OUString unotools::misc::ServiceDocumenter::getImplementationName()
{
    return "com.sun.star.comp.unotools.misc.ServiceDocumenter";
}
css::uno::Sequence< OUString > unotools::misc::ServiceDocumenter::getSupportedServiceNames()
{
    return { "com.sun.star.script.ServiceDocumenter" };
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
unotools_ServiceDocument_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new unotools::misc::ServiceDocumenter(context));
}
