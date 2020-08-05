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

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/chart/XDiagram.hpp>
#include <com/sun/star/chart/XAxisXSupplier.hpp>
#include <com/sun/star/chart/XAxisYSupplier.hpp>
#include <com/sun/star/chart/XStatisticDisplay.hpp>

#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/lang/XLocalizable.hpp>

#include <com/sun/star/chart/XChartDocument.hpp>

css::uno::Reference< css::uno::XInterface > SAL_CALL
    SampleAddIn_CreateInstance(
        const css::uno::Reference< css::lang::XMultiServiceFactory >& );

class SampleAddIn : public cppu::WeakImplHelper<
    css::lang::XInitialization,
    css::chart::XDiagram,
    css::chart::XAxisXSupplier,
    css::chart::XAxisYSupplier,
    css::chart::XStatisticDisplay,
    css::lang::XServiceName,
    css::lang::XServiceInfo,
    css::util::XRefreshable,
    css::lang::XLocalizable  >
{
private:
    css::uno::Reference< css::chart::XChartDocument > mxChartDoc;
    css::lang::Locale maLocale;

    css::uno::Reference< css::drawing::XShape > mxMyRedLine;
    css::uno::Reference< css::drawing::XShape > mxMyText;

public:
    SampleAddIn();
    virtual ~SampleAddIn();

    // class specific code
    static OUString  getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    sal_Bool getLogicalPosition( css::uno::Reference< css::drawing::XShape >& xAxis,
                                 double fValue,
                                 sal_Bool bVertical,
                                 css::awt::Point& aOutPosition );

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw( css::uno::Exception,
               css::uno::RuntimeException );

    // XDiagram
    virtual OUString SAL_CALL getDiagramType() throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getDataRowProperties( sal_Int32 nRow )
        throw( css::lang::IndexOutOfBoundsException,
               css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getDataPointProperties( sal_Int32 nCol, sal_Int32 nRow )
        throw( css::lang::IndexOutOfBoundsException,
               css::uno::RuntimeException );

    // XShape ( ::XDiagram )
    virtual css::awt::Size SAL_CALL getSize()
        throw( css::uno::RuntimeException );
    virtual void SAL_CALL setSize( const css::awt::Size& )
        throw( css::beans::PropertyVetoException,
               css::uno::RuntimeException );
    virtual css::awt::Point SAL_CALL getPosition()
        throw( css::uno::RuntimeException );
    virtual void SAL_CALL setPosition( const css::awt::Point& )
        throw( css::uno::RuntimeException );

    // XShapeDescriptor ( ::XShape ::XDiagram )
    virtual OUString SAL_CALL getShapeType() throw( css::uno::RuntimeException );

    // XAxisXSupplier
    virtual css::uno::Reference< css::drawing::XShape > SAL_CALL getXAxisTitle()
        throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getXAxis()
        throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getXMainGrid()
        throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getXHelpGrid()
        throw( css::uno::RuntimeException );

    // XAxisYSupplier
    virtual css::uno::Reference< css::drawing::XShape > SAL_CALL getYAxisTitle()
        throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getYAxis()
        throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getYHelpGrid()
        throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getYMainGrid()
        throw( css::uno::RuntimeException );

    // XStatisticDisplay
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getUpBar()
        throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getDownBar()
        throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getMinMaxLine()
        throw( css::uno::RuntimeException );

    // XServiceName
    virtual OUString SAL_CALL getServiceName() throw( css::uno::RuntimeException );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException );

    // XRefreshable
    virtual void SAL_CALL refresh() throw( css::uno::RuntimeException );
    virtual void SAL_CALL addRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l )
        throw( css::uno::RuntimeException );
    virtual void SAL_CALL removeRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l )
        throw( css::uno::RuntimeException );

    // XLocalizable
    virtual void SAL_CALL setLocale( const css::lang::Locale& eLocale )
        throw( css::uno::RuntimeException );
    virtual css::lang::Locale SAL_CALL getLocale()
        throw( css::uno::RuntimeException );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
