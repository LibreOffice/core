/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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

#include "InterfaceContainer.hxx"

#include <cppuhelper/implbase2.hxx>

#include "ids.hxx"

#include <com/sun/star/lang/XServiceInfo.hpp>
namespace binfilter {

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
typedef ::cppu::ImplHelper2<	::com::sun::star::container::XChild
                                ,::com::sun::star::lang::XServiceInfo > OFormsCollection_BASE;

    // else MSVC kills itself on some statements
class OFormsCollection
        :public FormsCollectionComponentBase
        ,public OInterfaceContainer
        ,public OFormsCollection_BASE
{
    ::osl::Mutex				m_aMutex;
    OImplementationIdsRef		m_aHoldIdHelper;
    ::comphelper::InterfaceRef 	m_xParent;			// Parent

public:
    OFormsCollection(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    virtual ~OFormsCollection();

public:
    DECLARE_UNO3_AGG_DEFAULTS(OFormsCollection, FormsCollectionComponentBase);

    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);

// XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

// XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

// XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual StringSequence SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// ::com::sun::star::container::XChild
    virtual ::comphelper::InterfaceRef SAL_CALL getParent() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setParent(const ::comphelper::InterfaceRef& Parent) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
};

//.........................................................................
}	// namespace frm
//.........................................................................
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
