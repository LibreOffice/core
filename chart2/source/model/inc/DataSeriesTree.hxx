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
