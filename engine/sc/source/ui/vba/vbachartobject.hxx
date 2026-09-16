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

#include <cpo/uno/XComponentContext.hpp>
#include <com/sun/star/table/XTableChart.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <ooo/vba/excel/XChartObject.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <optional>

typedef InheritedHelperInterfaceWeakImpl<ov::excel::XChartObject > ChartObjectImpl_BASE;

class ScVbaChartObject : public ChartObjectImpl_BASE
{

    cpo::uno::Reference< css::table::XTableChart  > xTableChart;
    cpo::uno::Reference< css::document::XEmbeddedObjectSupplier > xEmbeddedObjectSupplier;
    cpo::uno::Reference< css::drawing::XDrawPageSupplier > xDrawPageSupplier;
    cpo::uno::Reference< css::drawing::XDrawPage > xDrawPage;
    cpo::uno::Reference< css::drawing::XShape > xShape;
    cpo::uno::Reference< css::container::XNamed > xNamed;
    OUString sPersistName;
    std::optional<ov::ShapeHelper> oShapeHelper;
    cpo::uno::Reference< css::container::XNamed > xNamedShape;
    OUString const & getPersistName();
    /// @throws css::script::BasicErrorException
    cpo::uno::Reference< css::drawing::XShape > setShape();
public:
    ScVbaChartObject( const cpo::uno::Reference< ov::XHelperInterface >& _xParent, const cpo::uno::Reference< cpo::uno::XComponentContext >& _xContext, cpo::uno::Reference< css::table::XTableChart >  _xTableChart, cpo::uno::Reference< css::drawing::XDrawPageSupplier >  _xDrawPageSupplier );
    virtual OUString getName() override;
    virtual void setName( const OUString& sName ) override;
    virtual cpo::uno::Reference< ov::excel::XChart > getChart() override;
    virtual void Delete() override;
    /// @throws css::script::BasicErrorException
    void Activate();
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
