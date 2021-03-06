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

#pragma once

#include <InterfaceContainer.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/implbase2.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/form/XForms.hpp>


namespace frm
{

// OFormsCollection
// Implements the UNO Container for Forms and contains all assigned Forms.
// It can either represent the Context for Forms or be passed a Context.

typedef ::cppu::ImplHelper2< css::form::XForms
                             ,css::lang::XServiceInfo > OFormsCollection_BASE;
class OFormsCollection
        :public ::cppu::OComponentHelper
        ,public OInterfaceContainer
        ,public OFormsCollection_BASE
{
    ::osl::Mutex                               m_aMutex;
    css::uno::Reference<css::uno::XInterface>  m_xParent; // Parent

public:
    explicit OFormsCollection(const css::uno::Reference< css::uno::XComponentContext>& _rxFactory);
    OFormsCollection( const OFormsCollection& _cloneSource );
    virtual ~OFormsCollection() override;

public:
    DECLARE_UNO3_AGG_DEFAULTS(OFormsCollection, ::cppu::OComponentHelper)

    virtual css::uno::Any SAL_CALL queryAggregation(const css::uno::Type& _rType) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    // XPersistObject
    virtual OUString SAL_CALL getServiceName() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames(  ) override;

    // XCloneable
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) override;

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

    // css::container::XChild
    virtual css::uno::Reference<css::uno::XInterface> SAL_CALL getParent() override;
    virtual void SAL_CALL setParent(const css::uno::Reference<css::uno::XInterface>& Parent) override;

    // prevent method hiding
    using OInterfaceContainer::disposing;

    // inheritance ambiguity
    virtual css::uno::Type SAL_CALL getElementType() override
        { return OInterfaceContainer::getElementType(); }
    virtual sal_Bool SAL_CALL hasElements() override
        { return OInterfaceContainer::hasElements(); }
    virtual css::uno::Any SAL_CALL getByName(const OUString& p1) override
        { return OInterfaceContainer::getByName(p1); }
    virtual css::uno::Sequence<OUString> SAL_CALL getElementNames() override
        { return OInterfaceContainer::getElementNames(); }
    virtual sal_Bool SAL_CALL hasByName(const OUString& p1) override
        { return OInterfaceContainer::hasByName(p1); }
    virtual void SAL_CALL replaceByName(const OUString& p1, const css::uno::Any& p2) override
        { OInterfaceContainer::replaceByName(p1, p2); }
    virtual void SAL_CALL insertByName(const OUString& p1, const css::uno::Any& p2) override
        { OInterfaceContainer::insertByName(p1, p2); }
    virtual void SAL_CALL removeByName(const OUString& p1) override
        { OInterfaceContainer::removeByName(p1); }
    virtual sal_Int32 SAL_CALL getCount() override
        { return OInterfaceContainer::getCount(); }
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 p1) override
        { return OInterfaceContainer::getByIndex(p1); }
    virtual void SAL_CALL replaceByIndex(sal_Int32 p1, const css::uno::Any& p2) override
        { return OInterfaceContainer::replaceByIndex(p1, p2); }
    virtual void SAL_CALL insertByIndex(sal_Int32 p1, const css::uno::Any& p2) override
        { return OInterfaceContainer::insertByIndex(p1, p2); }
    virtual void SAL_CALL removeByIndex(sal_Int32 p1) override
        { return OInterfaceContainer::removeByIndex(p1); }
    virtual css::uno::Reference<css::container::XEnumeration> SAL_CALL createEnumeration() override
        { return OInterfaceContainer::createEnumeration(); }
    virtual void SAL_CALL registerScriptEvent(sal_Int32 p1, const css::script::ScriptEventDescriptor& p2) override
        { OInterfaceContainer::registerScriptEvent(p1, p2); }
    virtual void SAL_CALL registerScriptEvents(sal_Int32 p1, const css::uno::Sequence<css::script::ScriptEventDescriptor>& p2) override
        { OInterfaceContainer::registerScriptEvents(p1, p2); }
    virtual void SAL_CALL revokeScriptEvent(sal_Int32 p1, const OUString& p2, const OUString& p3, const OUString& p4) override
        { OInterfaceContainer::revokeScriptEvent(p1, p2, p3, p4); }
    virtual void SAL_CALL revokeScriptEvents(sal_Int32 p1) override
        { OInterfaceContainer::revokeScriptEvents(p1); }
    virtual void SAL_CALL insertEntry(sal_Int32 p1) override
        { OInterfaceContainer::insertEntry(p1); }
    virtual void SAL_CALL removeEntry(sal_Int32 p1) override
        { OInterfaceContainer::removeEntry(p1); }
    virtual css::uno::Sequence<css::script::ScriptEventDescriptor> SAL_CALL getScriptEvents(sal_Int32 p1) override
        { return OInterfaceContainer::getScriptEvents(p1); }
    virtual void SAL_CALL attach(sal_Int32 p1, const css::uno::Reference<css::uno::XInterface>& p2, const css::uno::Any& p3) override
        { OInterfaceContainer::attach(p1, p2, p3); }
    virtual void SAL_CALL detach(sal_Int32 p1, const css::uno::Reference<css::uno::XInterface>& p2) override
        { OInterfaceContainer::detach(p1, p2); }
    virtual void SAL_CALL addScriptListener(const css::uno::Reference<css::script::XScriptListener>& p1) override
        { OInterfaceContainer::addScriptListener(p1); }
    virtual void SAL_CALL removeScriptListener(const css::uno::Reference<css::script::XScriptListener>& p1) override
        { OInterfaceContainer::removeScriptListener(p1); }
    virtual void SAL_CALL dispose() override
        { ::cppu::OComponentHelper::dispose(); }
    virtual void SAL_CALL addEventListener(const css::uno::Reference<css::lang::XEventListener>& p1) override
        { ::cppu::OComponentHelper::addEventListener(p1); }
    virtual void SAL_CALL removeEventListener(const css::uno::Reference<css::lang::XEventListener>& p1) override
        { ::cppu::OComponentHelper::removeEventListener(p1); }
    virtual void SAL_CALL addContainerListener(const css::uno::Reference<css::container::XContainerListener>& p1) override
        { OInterfaceContainer::addContainerListener(p1); }
    virtual void SAL_CALL removeContainerListener(const css::uno::Reference<css::container::XContainerListener>& p1) override
        { OInterfaceContainer::removeContainerListener(p1); }
};


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
