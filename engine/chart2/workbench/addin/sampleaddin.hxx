/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

css::uno::Reference< css::uno::XInterface >
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
    static cpo::uno::Sequence< OUString > getSupportedServiceNames_Static();

    bool getLogicalPosition( css::uno::Reference< css::drawing::XShape >& xAxis,
                                 double fValue,
                                 bool bVertical,
                                 css::awt::Point& aOutPosition );

    // XInitialization
    virtual void initialize( const cpo::uno::Sequence< cpo::uno::Any >& aArguments )
        throw( css::uno::Exception,
               css::uno::RuntimeException );

    // XDiagram
    virtual OUString getDiagramType() throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > getDataRowProperties( sal_Int32 nRow )
        throw( css::lang::IndexOutOfBoundsException,
               css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > getDataPointProperties( sal_Int32 nCol, sal_Int32 nRow )
        throw( css::lang::IndexOutOfBoundsException,
               css::uno::RuntimeException );

    // XShape ( ::XDiagram )
    virtual css::awt::Size getSize()
        throw( css::uno::RuntimeException );
    virtual void setSize( const css::awt::Size& )
        throw( css::beans::PropertyVetoException,
               css::uno::RuntimeException );
    virtual css::awt::Point getPosition()
        throw( css::uno::RuntimeException );
    virtual void setPosition( const css::awt::Point& )
        throw( css::uno::RuntimeException );

    // XShapeDescriptor ( ::XShape ::XDiagram )
    virtual OUString getShapeType() throw( css::uno::RuntimeException );

    // XAxisXSupplier
    virtual css::uno::Reference< css::drawing::XShape > getXAxisTitle()
        throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > getXAxis()
        throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > getXMainGrid()
        throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > getXHelpGrid()
        throw( css::uno::RuntimeException );

    // XAxisYSupplier
    virtual css::uno::Reference< css::drawing::XShape > getYAxisTitle()
        throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > getYAxis()
        throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > getYHelpGrid()
        throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > getYMainGrid()
        throw( css::uno::RuntimeException );

    // XStatisticDisplay
    virtual css::uno::Reference< css::beans::XPropertySet > getUpBar()
        throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > getDownBar()
        throw( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > getMinMaxLine()
        throw( css::uno::RuntimeException );

    // XServiceName
    virtual OUString getServiceName() throw( css::uno::RuntimeException );

    // XServiceInfo
    virtual OUString getImplementationName() throw( css::uno::RuntimeException );
    virtual bool supportsService( const OUString& ServiceName )
        throw( css::uno::RuntimeException );
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames()
        throw( css::uno::RuntimeException );

    // XRefreshable
    virtual void refresh() throw( css::uno::RuntimeException );
    virtual void addRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l )
        throw( css::uno::RuntimeException );
    virtual void removeRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l )
        throw( css::uno::RuntimeException );

    // XLocalizable
    virtual void setLocale( const css::lang::Locale& eLocale )
        throw( css::uno::RuntimeException );
    virtual css::lang::Locale getLocale()
        throw( css::uno::RuntimeException );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
