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

#include <sal/config.h>

#include <map>
#include <vector>

#include <config_feature_desktop.h>

#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>

#include <comphelper/propertysequence.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/SearchAlgorithms.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>

#include <vcl/InterimItemWindow.hxx>
#include <svl/ctloptions.hxx>
#include <svl/srchitem.hxx>
#include <svtools/acceleratorexecute.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/svapp.hxx>
#include <rtl/instance.hxx>
#include <svx/labelitemwindow.hxx>
#include <svx/srchdlg.hxx>
#include <vcl/event.hxx>

namespace
{
// protocol handler for "vnd.libreoffice.devtools:*" URLs
// The dispatch object will be used for shortcut commands for findbar
class DevToolsDispatcher : public css::lang::XServiceInfo,
                           public css::lang::XInitialization,
                           public css::frame::XDispatchProvider,
                           public css::frame::XDispatch,
                           public ::cppu::OWeakObject
{
public:
    DevToolsDispatcher();
    virtual ~DevToolsDispatcher() override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface(const css::uno::Type& aType) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& aArguments) override;

    // XDispatchProvider
    virtual css::uno::Reference<css::frame::XDispatch>
        SAL_CALL queryDispatch(const css::util::URL& aURL, const OUString& sTargetFrameName,
                               sal_Int32 nSearchFlags) override;
    virtual css::uno::Sequence<css::uno::Reference<css::frame::XDispatch>> SAL_CALL queryDispatches(
        const css::uno::Sequence<css::frame::DispatchDescriptor>& lDescriptions) override;

    // XDispatch
    virtual void SAL_CALL
    dispatch(const css::util::URL& aURL,
             const css::uno::Sequence<css::beans::PropertyValue>& lArguments) override;
    virtual void SAL_CALL
    addStatusListener(const css::uno::Reference<css::frame::XStatusListener>& xListener,
                      const css::util::URL& aURL) override;
    virtual void SAL_CALL
    removeStatusListener(const css::uno::Reference<css::frame::XStatusListener>& xListener,
                         const css::util::URL& aURL) override;

private:
    css::uno::Reference<css::frame::XFrame> m_xFrame;
};

DevToolsDispatcher::DevToolsDispatcher() {}

DevToolsDispatcher::~DevToolsDispatcher() { m_xFrame = nullptr; }

// XInterface
css::uno::Any SAL_CALL DevToolsDispatcher::queryInterface(const css::uno::Type& aType)
{
    css::uno::Any aReturn(::cppu::queryInterface(aType, static_cast<css::lang::XServiceInfo*>(this),
                                                 static_cast<css::lang::XInitialization*>(this),
                                                 static_cast<css::frame::XDispatchProvider*>(this),
                                                 static_cast<css::frame::XDispatch*>(this)));

    if (aReturn.hasValue())
        return aReturn;

    return OWeakObject::queryInterface(aType);
}

void SAL_CALL DevToolsDispatcher::acquire() throw() { OWeakObject::acquire(); }

void SAL_CALL DevToolsDispatcher::release() throw() { OWeakObject::release(); }

// XServiceInfo
OUString SAL_CALL DevToolsDispatcher::getImplementationName()
{
    return "org.libreoffice.Impl.DevToolsDispatcher";
}

sal_Bool SAL_CALL DevToolsDispatcher::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> SAL_CALL DevToolsDispatcher::getSupportedServiceNames()
{
    return { "org.libreoffice.DevToolsDispatcher", "com.sun.star.frame.ProtocolHandler" };
}

// XInitialization
void SAL_CALL DevToolsDispatcher::initialize(const css::uno::Sequence<css::uno::Any>& aArguments)
{
    if (aArguments.hasElements())
        aArguments[0] >>= m_xFrame;
}

// XDispatchProvider
css::uno::Reference<css::frame::XDispatch> SAL_CALL DevToolsDispatcher::queryDispatch(
    const css::util::URL& aURL, const OUString& /*sTargetFrameName*/, sal_Int32 /*nSearchFlags*/)
{
    css::uno::Reference<css::frame::XDispatch> xDispatch;

    if (aURL.Protocol == "vnd.libreoffice.devtools:")
        xDispatch = this;

    return xDispatch;
}

css::uno::Sequence<css::uno::Reference<css::frame::XDispatch>>
    SAL_CALL DevToolsDispatcher::queryDispatches(
        const css::uno::Sequence<css::frame::DispatchDescriptor>& seqDescripts)
{
    sal_Int32 nCount = seqDescripts.getLength();
    css::uno::Sequence<css::uno::Reference<XDispatch>> lDispatcher(nCount);

    std::transform(
        seqDescripts.begin(), seqDescripts.end(), lDispatcher.begin(),
        [this](const css::frame::DispatchDescriptor& rDescript) -> css::uno::Reference<XDispatch> {
            return queryDispatch(rDescript.FeatureURL, rDescript.FrameName, rDescript.SearchFlags);
        });

    return lDispatcher;
}

// XDispatch
void SAL_CALL DevToolsDispatcher::dispatch(
    const css::util::URL& aURL, const css::uno::Sequence<css::beans::PropertyValue>& /*lArgs*/)
{
    //vnd.libreoffice.devtools:FocusToDevTools  - set cursor to the XXX of the devtools
    if (aURL.Path != "FocusToDevTools")
        return;

    css::uno::Reference<css::beans::XPropertySet> xPropSet(m_xFrame, css::uno::UNO_QUERY);
    if (!xPropSet.is())
        return;

    css::uno::Reference<css::frame::XLayoutManager> xLayoutManager;
    css::uno::Any aValue = xPropSet->getPropertyValue("LayoutManager");
    aValue >>= xLayoutManager;
    if (!xLayoutManager.is())
        return;

    const OUString sResourceURL("private:resource/toolbar/devtools");
    css::uno::Reference<css::ui::XUIElement> xUIElement = xLayoutManager->getElement(sResourceURL);
    if (!xUIElement.is())
    {
        // show the devtools if necessary
        xLayoutManager->createElement(sResourceURL);
        xLayoutManager->showElement(sResourceURL);
        xUIElement = xLayoutManager->getElement(sResourceURL);
        if (!xUIElement.is())
            return;
    }

    css::uno::Reference<css::awt::XWindow> xWindow(xUIElement->getRealInterface(),
                                                   css::uno::UNO_QUERY);
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(xWindow);
    ToolBox* pToolBox = static_cast<ToolBox*>(pWindow.get());
    pToolBox->set_id("DevTools");
    if (!pToolBox)
        return;

    ToolBox::ImplToolItems::size_type nItemCount = pToolBox->GetItemCount();
    for (ToolBox::ImplToolItems::size_type i = 0; i < nItemCount; ++i)
    {
        sal_uInt16 id = pToolBox->GetItemId(i);
        OUString sItemCommand = pToolBox->GetItemCommand(id);
        if (sItemCommand == ".uno:DevTools")
        {
            vcl::Window* pItemWin = pToolBox->GetItemWindow(id);
            if (pItemWin)
            {
                SolarMutexGuard aSolarMutexGuard;
#if 0 // ???
                Control* pControl = dynamic_cast<Control*>(pItemWin);
                if ( pControl )
                    pControl->SetTextToSelected_Impl();
#endif
                pItemWin->GrabFocus();
                return;
            }
        }
    }
}

void SAL_CALL DevToolsDispatcher::addStatusListener(
    const css::uno::Reference<css::frame::XStatusListener>& /*xControl*/,
    const css::util::URL& /*aURL*/)
{
}

void SAL_CALL DevToolsDispatcher::removeStatusListener(
    const css::uno::Reference<css::frame::XStatusListener>& /*xControl*/,
    const css::util::URL& /*aURL*/)
{
}
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_libreoffice_Impl_DevToolsDispatcher_get_implementation(
    SAL_UNUSED_PARAMETER css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new DevToolsDispatcher);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
