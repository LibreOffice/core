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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_FORMSCOLLECTION_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_FORMSCOLLECTION_HXX

#include "InterfaceContainer.hxx"
#include <cppuhelper/component.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/form/XForms.hpp>


namespace frm
{

// OFormsCollection
// Implements the UNO Container for Forms and contains all assigned Forms.
// It can either represent the Context for Forms or be passed a Context.

typedef ::cppu::OComponentHelper FormsCollectionComponentBase;
typedef ::cppu::ImplHelper <    css::form::XForms
                                ,css::lang::XServiceInfo > OFormsCollection_BASE;

// else MSVC kills itself on some statements
class OFormsCollection
        :public FormsCollectionComponentBase
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
    DECLARE_UNO3_AGG_DEFAULTS(OFormsCollection, FormsCollectionComponentBase)

    virtual css::uno::Any SAL_CALL queryAggregation(const css::uno::Type& _rType) throw(css::uno::RuntimeException, std::exception) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XPersistObject
    virtual OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XCloneable
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) override;

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

    // css::container::XChild
    virtual css::uno::Reference<css::uno::XInterface> SAL_CALL getParent() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setParent(const css::uno::Reference<css::uno::XInterface>& Parent) throw(css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

    // prevent method hiding
    using OInterfaceContainer::disposing;

    // inheritance ambiguity
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException, std::exception) override
        { return OInterfaceContainer::getElementType(); }
    virtual sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException, std::exception) override
        { return OInterfaceContainer::hasElements(); }
    virtual css::uno::Any SAL_CALL getByName(const rtl::OUString& p1)
        throw (css::container::NoSuchElementException,
               css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
        { return OInterfaceContainer::getByName(p1); }
    virtual css::uno::Sequence<rtl::OUString> SAL_CALL getElementNames() throw (css::uno::RuntimeException, std::exception) override
        { return OInterfaceContainer::getElementNames(); }
    virtual sal_Bool SAL_CALL hasByName(const rtl::OUString& p1) throw (css::uno::RuntimeException, std::exception) override
        { return OInterfaceContainer::hasByName(p1); }
    virtual void SAL_CALL replaceByName(const rtl::OUString& p1, const css::uno::Any& p2)
        throw (css::lang::IllegalArgumentException,
               css::container::NoSuchElementException,
               css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
        { OInterfaceContainer::replaceByName(p1, p2); }
    virtual void SAL_CALL insertByName(const rtl::OUString& p1, const css::uno::Any& p2)
        throw (css::lang::IllegalArgumentException,
               css::container::ElementExistException,
               css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
        { OInterfaceContainer::insertByName(p1, p2); }
    virtual void SAL_CALL removeByName(const rtl::OUString& p1)
        throw (css::container::NoSuchElementException,
               css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
        { OInterfaceContainer::removeByName(p1); }
    virtual sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException, std::exception) override
        { return OInterfaceContainer::getCount(); }
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 p1)
        throw (css::lang::IndexOutOfBoundsException,
               css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
        { return OInterfaceContainer::getByIndex(p1); }
    virtual void SAL_CALL replaceByIndex(sal_Int32 p1, const css::uno::Any& p2)
        throw (css::lang::IllegalArgumentException,
               css::lang::IndexOutOfBoundsException,
               css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
        { return OInterfaceContainer::replaceByIndex(p1, p2); }
    virtual void SAL_CALL insertByIndex(sal_Int32 p1, const css::uno::Any& p2)
        throw (css::lang::IllegalArgumentException,
               css::lang::IndexOutOfBoundsException,
               css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
        { return OInterfaceContainer::insertByIndex(p1, p2); }
    virtual void SAL_CALL removeByIndex(sal_Int32 p1)
        throw (css::lang::IndexOutOfBoundsException,
               css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
        { return OInterfaceContainer::removeByIndex(p1); }
    virtual css::uno::Reference<css::container::XEnumeration> SAL_CALL createEnumeration() throw (css::uno::RuntimeException, std::exception) override
        { return OInterfaceContainer::createEnumeration(); }
    virtual void SAL_CALL registerScriptEvent(sal_Int32 p1, const css::script::ScriptEventDescriptor& p2)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override
        { OInterfaceContainer::registerScriptEvent(p1, p2); }
    virtual void SAL_CALL registerScriptEvents(sal_Int32 p1, const css::uno::Sequence<css::script::ScriptEventDescriptor>& p2)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override
        { OInterfaceContainer::registerScriptEvents(p1, p2); }
    virtual void SAL_CALL revokeScriptEvent(sal_Int32 p1, const rtl::OUString& p2, const rtl::OUString& p3, const rtl::OUString& p4)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override
        { OInterfaceContainer::revokeScriptEvent(p1, p2, p3, p4); }
    virtual void SAL_CALL revokeScriptEvents(sal_Int32 p1)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override
        { OInterfaceContainer::revokeScriptEvents(p1); }
    virtual void SAL_CALL insertEntry(sal_Int32 p1)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override
        { OInterfaceContainer::insertEntry(p1); }
    virtual void SAL_CALL removeEntry(sal_Int32 p1)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override
        { OInterfaceContainer::removeEntry(p1); }
    virtual css::uno::Sequence<css::script::ScriptEventDescriptor> SAL_CALL getScriptEvents(sal_Int32 p1)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override
        { return OInterfaceContainer::getScriptEvents(p1); }
    virtual void SAL_CALL attach(sal_Int32 p1, const css::uno::Reference<css::uno::XInterface>& p2, const css::uno::Any& p3)
        throw (css::lang::IllegalArgumentException,
               css::lang::ServiceNotRegisteredException,
               css::uno::RuntimeException, std::exception) override
        { OInterfaceContainer::attach(p1, p2, p3); }
    virtual void SAL_CALL detach(sal_Int32 p1, const css::uno::Reference<css::uno::XInterface>& p2)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override
        { OInterfaceContainer::detach(p1, p2); }
    virtual void SAL_CALL addScriptListener(const css::uno::Reference<css::script::XScriptListener>& p1)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override
        { OInterfaceContainer::addScriptListener(p1); }
    virtual void SAL_CALL removeScriptListener(const css::uno::Reference<css::script::XScriptListener>& p1)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override
        { OInterfaceContainer::removeScriptListener(p1); }
    virtual void SAL_CALL dispose() throw (css::uno::RuntimeException, std::exception) override
        { FormsCollectionComponentBase::dispose(); }
    virtual void SAL_CALL addEventListener(const css::uno::Reference<css::lang::XEventListener>& p1) throw (css::uno::RuntimeException, std::exception) override
        { FormsCollectionComponentBase::addEventListener(p1); }
    virtual void SAL_CALL removeEventListener(const css::uno::Reference<css::lang::XEventListener>& p1) throw (css::uno::RuntimeException, std::exception) override
        { FormsCollectionComponentBase::removeEventListener(p1); }
    virtual void SAL_CALL addContainerListener(const css::uno::Reference<css::container::XContainerListener>& p1) throw (css::uno::RuntimeException, std::exception) override
        { OInterfaceContainer::addContainerListener(p1); }
    virtual void SAL_CALL removeContainerListener(const css::uno::Reference<css::container::XContainerListener>& p1) throw (css::uno::RuntimeException, std::exception) override
        { OInterfaceContainer::removeContainerListener(p1); }
};


}   // namespace frm


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_FORMSCOLLECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
