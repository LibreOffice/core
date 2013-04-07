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

#include <com/sun/star/awt/grid/XGridColumnModel.hpp>
#include <com/sun/star/awt/grid/XGridColumn.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/util/Color.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase2.hxx>
#include <comphelper/componentcontext.hxx>
#include <vector>

namespace toolkit
{

typedef ::cppu::WeakComponentImplHelper2    <   ::com::sun::star::awt::grid::XGridColumnModel
                                            ,   ::com::sun::star::lang::XServiceInfo
                                            >   DefaultGridColumnModel_Base;

class DefaultGridColumnModel    :public ::cppu::BaseMutex
                                ,public DefaultGridColumnModel_Base
{
public:
    DefaultGridColumnModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext );
    DefaultGridColumnModel( DefaultGridColumnModel const & i_copySource );
    virtual ~DefaultGridColumnModel();

    // XGridColumnModel
    virtual ::sal_Int32 SAL_CALL getColumnCount() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > SAL_CALL createColumn(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL addColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > & column) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException);
    virtual void SAL_CALL removeColumn( ::sal_Int32 i_columnIndex )  throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > > SAL_CALL getColumns() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > SAL_CALL getColumn(::sal_Int32 index) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDefaultColumns(sal_Int32 rowElements) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

    // XContainer
    virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

    // XCloneable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw (::com::sun::star::uno::RuntimeException);

    // OComponentHelper
    virtual void SAL_CALL disposing();

private:
    typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > >   Columns;

    ::comphelper::ComponentContext      m_aContext;
    ::cppu::OInterfaceContainerHelper   m_aContainerListeners;
    Columns                             m_aColumns;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
