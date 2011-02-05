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
#ifndef SC_VBA_CHARTS_HXX
#define SC_VBA_CHARTS_HXX
#include <ooo/vba/excel/XCharts.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <vbahelper/vbacollectionimpl.hxx>
#include "excelvbahelper.hxx"
#include <boost/unordered_map.hpp>

typedef CollTestImplHelper< ov::excel::XCharts > Charts_BASE;

class ScVbaCharts : public Charts_BASE
{
    css::uno::Reference< ov::excel::XChart > xActiveChart;
    css::uno::Reference< css::sheet::XSpreadsheetDocument > xSpreadsheetDocument;
    css::uno::Reference< css::lang::XComponent > xComponent;
public:
    ScVbaCharts( const css::uno::Reference< ov::XHelperInterface >& _xParent, const css::uno::Reference< css::uno::XComponentContext >& _xContext, const css::uno::Reference< css::frame::XModel >& xModel );
    // XCharts
    virtual css::uno::Any SAL_CALL Add() throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XChart > SAL_CALL getActiveChart(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XCollection
    ::sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException);
    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) { return ov::excel::XChart::static_type(0); }
    // ScVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );
    // Charts_BASE or HelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
