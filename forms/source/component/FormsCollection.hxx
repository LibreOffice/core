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

#include "InterfaceContainer.hxx"
#include <cppuhelper/component.hxx>
#include <cppuhelper/implbase2.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/form/XForms.hpp>


namespace frm
{



// OFormsCollection
// Implementiert den UNO-Container fuer Formulare
// enthaelt alle zugeordneten Forms
// dieses Container kann selbst den Context fuer Formulare darstellen
// oder aussen einen Context uebergeben bekommen

typedef ::cppu::OComponentHelper FormsCollectionComponentBase;
typedef ::cppu::ImplHelper2<    ::com::sun::star::form::XForms
                                ,::com::sun::star::lang::XServiceInfo > OFormsCollection_BASE;

    // else MSVC kills itself on some statements
class OFormsCollection
        :public FormsCollectionComponentBase
        ,public OInterfaceContainer
        ,public OFormsCollection_BASE
{
    ::osl::Mutex                m_aMutex;
    ::comphelper::InterfaceRef  m_xParent;          // Parent

public:
    OFormsCollection(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxFactory);
    OFormsCollection( const OFormsCollection& _cloneSource );
    virtual ~OFormsCollection();

public:
    DECLARE_UNO3_AGG_DEFAULTS(OFormsCollection, FormsCollectionComponentBase);

    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XPersistObject
    virtual OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual StringSequence SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XCloneable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    // OComponentHelper
    virtual void SAL_CALL disposing();

    // ::com::sun::star::container::XChild
    virtual ::comphelper::InterfaceRef SAL_CALL getParent() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setParent(const ::comphelper::InterfaceRef& Parent) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception);

    // prevent method hiding
    using OInterfaceContainer::disposing;

    // inheritance ambiguity
    virtual com::sun::star::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException, std::exception)
        { return OInterfaceContainer::getElementType(); }
    virtual sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException, std::exception)
        { return OInterfaceContainer::hasElements(); }
    virtual com::sun::star::uno::Any SAL_CALL getByName(const rtl::OUString& p1)
        throw (css::container::NoSuchElementException,
               css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception)
        { return OInterfaceContainer::getByName(p1); }
    virtual com::sun::star::uno::Sequence<rtl::OUString> SAL_CALL getElementNames() throw (css::uno::RuntimeException, std::exception)
        { return OInterfaceContainer::getElementNames(); }
    virtual sal_Bool SAL_CALL hasByName(const rtl::OUString& p1) throw (css::uno::RuntimeException, std::exception)
        { return OInterfaceContainer::hasByName(p1); }
    virtual void SAL_CALL replaceByName(const rtl::OUString& p1, const com::sun::star::uno::Any& p2)
        throw (css::lang::IllegalArgumentException,
               css::container::NoSuchElementException,
               css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception)
        { OInterfaceContainer::replaceByName(p1, p2); }
    virtual void SAL_CALL insertByName(const rtl::OUString& p1, const com::sun::star::uno::Any& p2)
        throw (css::lang::IllegalArgumentException,
               css::container::ElementExistException,
               css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception)
        { OInterfaceContainer::insertByName(p1, p2); }
    virtual void SAL_CALL removeByName(const rtl::OUString& p1)
        throw (css::container::NoSuchElementException,
               css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception)
        { OInterfaceContainer::removeByName(p1); }
    virtual sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException, std::exception)
        { return OInterfaceContainer::getCount(); }
    virtual com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 p1)
        throw (css::lang::IndexOutOfBoundsException,
               css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception)
        { return OInterfaceContainer::getByIndex(p1); }
    virtual void SAL_CALL replaceByIndex(sal_Int32 p1, const com::sun::star::uno::Any& p2)
        throw (css::lang::IllegalArgumentException,
               css::lang::IndexOutOfBoundsException,
               css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception)
        { return OInterfaceContainer::replaceByIndex(p1, p2); }
    virtual void SAL_CALL insertByIndex(sal_Int32 p1, const com::sun::star::uno::Any& p2)
        throw (css::lang::IllegalArgumentException,
               css::lang::IndexOutOfBoundsException,
               css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception)
        { return OInterfaceContainer::insertByIndex(p1, p2); }
    virtual void SAL_CALL removeByIndex(sal_Int32 p1)
        throw (css::lang::IndexOutOfBoundsException,
               css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception)
        { return OInterfaceContainer::removeByIndex(p1); }
    virtual com::sun::star::uno::Reference<com::sun::star::container::XEnumeration> SAL_CALL createEnumeration() throw (css::uno::RuntimeException, std::exception)
        { return OInterfaceContainer::createEnumeration(); }
    virtual void SAL_CALL registerScriptEvent(sal_Int32 p1, const com::sun::star::script::ScriptEventDescriptor& p2)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
        { OInterfaceContainer::registerScriptEvent(p1, p2); }
    virtual void SAL_CALL registerScriptEvents(sal_Int32 p1, const com::sun::star::uno::Sequence<com::sun::star::script::ScriptEventDescriptor>& p2)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
        { OInterfaceContainer::registerScriptEvents(p1, p2); }
    virtual void SAL_CALL revokeScriptEvent(sal_Int32 p1, const rtl::OUString& p2, const rtl::OUString& p3, const rtl::OUString& p4)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
        { OInterfaceContainer::revokeScriptEvent(p1, p2, p3, p4); }
    virtual void SAL_CALL revokeScriptEvents(sal_Int32 p1)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
        { OInterfaceContainer::revokeScriptEvents(p1); }
    virtual void SAL_CALL insertEntry(sal_Int32 p1)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
        { OInterfaceContainer::insertEntry(p1); }
    virtual void SAL_CALL removeEntry(sal_Int32 p1)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
        { OInterfaceContainer::removeEntry(p1); }
    virtual com::sun::star::uno::Sequence<com::sun::star::script::ScriptEventDescriptor> SAL_CALL getScriptEvents(sal_Int32 p1)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
        { return OInterfaceContainer::getScriptEvents(p1); }
    virtual void SAL_CALL attach(sal_Int32 p1, const com::sun::star::uno::Reference<com::sun::star::uno::XInterface>& p2, const com::sun::star::uno::Any& p3)
        throw (css::lang::IllegalArgumentException,
               css::lang::ServiceNotRegisteredException,
               css::uno::RuntimeException, std::exception)
        { OInterfaceContainer::attach(p1, p2, p3); }
    virtual void SAL_CALL detach(sal_Int32 p1, const com::sun::star::uno::Reference<com::sun::star::uno::XInterface>& p2)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
        { OInterfaceContainer::detach(p1, p2); }
    virtual void SAL_CALL addScriptListener(const com::sun::star::uno::Reference<com::sun::star::script::XScriptListener>& p1)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
        { OInterfaceContainer::addScriptListener(p1); }
    virtual void SAL_CALL removeScriptListener(const com::sun::star::uno::Reference<com::sun::star::script::XScriptListener>& p1)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
        { OInterfaceContainer::removeScriptListener(p1); }
    virtual void SAL_CALL dispose() throw (css::uno::RuntimeException, std::exception)
        { FormsCollectionComponentBase::dispose(); }
    virtual void SAL_CALL addEventListener(const com::sun::star::uno::Reference<com::sun::star::lang::XEventListener>& p1) throw (css::uno::RuntimeException, std::exception)
        { FormsCollectionComponentBase::addEventListener(p1); }
    virtual void SAL_CALL removeEventListener(const com::sun::star::uno::Reference<com::sun::star::lang::XEventListener>& p1) throw (css::uno::RuntimeException, std::exception)
        { FormsCollectionComponentBase::removeEventListener(p1); }
    virtual void SAL_CALL addContainerListener(const com::sun::star::uno::Reference<com::sun::star::container::XContainerListener>& p1) throw (css::uno::RuntimeException, std::exception)
        { OInterfaceContainer::addContainerListener(p1); }
    virtual void SAL_CALL removeContainerListener(const com::sun::star::uno::Reference<com::sun::star::container::XContainerListener>& p1) throw (css::uno::RuntimeException, std::exception)
        { OInterfaceContainer::removeContainerListener(p1); }
};


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
