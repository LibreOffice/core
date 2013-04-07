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
#include "ids.hxx"
#include <cppuhelper/component.hxx>
#include <cppuhelper/implbase2.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

//.........................................................................
namespace frm
{
//.........................................................................

//==================================================================
// OFormsCollection
// Implementiert den UNO-Container fuer Formulare
// enthaelt alle zugeordneten Forms
// dieses Container kann selbst den Context fuer Formulare darstellen
// oder aussen einen Context uebergeben bekommen
//==================================================================
typedef ::cppu::OComponentHelper FormsCollectionComponentBase;
typedef ::cppu::ImplHelper2<    ::com::sun::star::container::XChild
                                ,::com::sun::star::lang::XServiceInfo > OFormsCollection_BASE;

    // else MSVC kills itself on some statements
class OFormsCollection
        :public FormsCollectionComponentBase
        ,public OInterfaceContainer
        ,public OFormsCollection_BASE
{
    ::osl::Mutex                m_aMutex;
    OImplementationIdsRef       m_aHoldIdHelper;
    ::comphelper::InterfaceRef  m_xParent;          // Parent

public:
    OFormsCollection(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    OFormsCollection( const OFormsCollection& _cloneSource );
    virtual ~OFormsCollection();

public:
    DECLARE_UNO3_AGG_DEFAULTS(OFormsCollection, FormsCollectionComponentBase);

    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    // XPersistObject
    virtual OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual StringSequence SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // XCloneable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw (::com::sun::star::uno::RuntimeException);

    // OComponentHelper
    virtual void SAL_CALL disposing();

    // ::com::sun::star::container::XChild
    virtual ::comphelper::InterfaceRef SAL_CALL getParent() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setParent(const ::comphelper::InterfaceRef& Parent) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

    // prevent method hiding
    using OInterfaceContainer::disposing;
};

//.........................................................................
}   // namespace frm
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
