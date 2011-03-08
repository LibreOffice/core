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
#ifndef CHART_LAYOUTCONTAINER_HXX
#define CHART_LAYOUTCONTAINER_HXX

#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/layout/XLayoutContainer.hpp>

#include "ServiceMacros.hxx"

#include <vector>
#include <map>

namespace chart
{

class LayoutContainer : public
    ::cppu::WeakImplHelper2<
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::layout::XLayoutContainer >
{
public:
    LayoutContainer();
    virtual ~LayoutContainer();

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

protected:
    // ____ XLayoutContainer ____
    virtual void SAL_CALL addConstrainedElementByIdentifier( const ::rtl::OUString& aIdentifier, const ::com::sun::star::layout::Constraint& Constraint )
        throw (::com::sun::star::layout::IllegalConstraintException,
               ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addElementByIdentifier( const ::rtl::OUString& aIdentifier )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeElementByIdentifier( const ::rtl::OUString& aIdentifier )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setConstraintByIdentifier( const ::rtl::OUString& aIdentifier, const ::com::sun::star::layout::Constraint& Constraint )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::layout::Constraint SAL_CALL getConstraintByIdentifier( const ::rtl::OUString& aIdentifier )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
        ::rtl::OUString > SAL_CALL getElementIdentifiers()
        throw (::com::sun::star::uno::RuntimeException);

private:
    typedef ::std::vector< ::rtl::OUString > tLayoutElements;

    typedef ::std::map<
        ::rtl::OUString,
        ::com::sun::star::layout::Constraint > tConstraintsMap;

    tLayoutElements           m_aLayoutElements;
    tConstraintsMap           m_aConstraints;
};

} //  namespace chart

// CHART_LAYOUTCONTAINER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
