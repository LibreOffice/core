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
#ifndef CHART_DATASERIESTREE_HXX
#define CHART_DATASERIESTREE_HXX

#include <cppuhelper/implbase2.hxx>

#include "ServiceMacros.hxx"
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/chart2/XDataSeriesTreeParent.hpp>

#include <vector>

namespace chart
{

class DataSeriesTree : public
    ::cppu::WeakImplHelper2<
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::chart2::XDataSeriesTreeParent >
{
public:
    DataSeriesTree( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~DataSeriesTree();

    /// declare XServiceInfo methods
    APPHELPER_XSERVICEINFO_DECL()
    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( DataSeriesTree )

protected:

    // ____ XDataSeriesTreeParent ____
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeriesTreeNode > >
        SAL_CALL getChildren()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setChildren(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeriesTreeNode > >& aNewChildren )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addChild(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeriesTreeNode >& aNode )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeChild(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeriesTreeNode >& aNode )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XDataSeriesTreeNode ____

private:
    typedef ::std::vector< ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDataSeriesTreeNode > >
        m_tChildType;

    m_tChildType   m_aChildren;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        m_xContext;
};

} //  namespace chart

// CHART_DATASERIESTREE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
