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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/excel/XAxis.hpp>
#include <ooo/vba/excel/XChart.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <memory>
typedef InheritedHelperInterfaceWeakImpl< ov::excel::XAxis >  ScVbaAxis_BASE;
class ScVbaChart;
class ScVbaAxis : public ScVbaAxis_BASE
{
    css::uno::Reference< ov::excel::XChart > moChartParent;
    css::uno::Reference< css::beans::XPropertySet > mxPropertySet;
    sal_Int32 mnType;
    sal_Int32 mnGroup;
    bool bCrossesAreCustomized;
    /// @throws css::uno::RuntimeException
    ScVbaChart* getChartPtr();
    /// @throws css::script::BasicErrorException
    bool isValueAxis();
    std::unique_ptr<ov::ShapeHelper> oShapeHelper;

public:
    ScVbaAxis( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::beans::XPropertySet >& _xPropertySet, sal_Int32 _nType, sal_Int32 _nGroup );
    // Methods
    virtual void SAL_CALL Delete(  ) override;
    virtual css::uno::Reference< ::ooo::vba::excel::XAxisTitle > SAL_CALL getAxisTitle(  ) override;
    virtual void SAL_CALL setDisplayUnit( ::sal_Int32 DisplayUnit ) override;
    virtual ::sal_Int32 SAL_CALL getDisplayUnit(  ) override;
    virtual void SAL_CALL setCrosses( ::sal_Int32 Crosses ) override;
    virtual ::sal_Int32 SAL_CALL getCrosses(  ) override;
    virtual void SAL_CALL setCrossesAt( double CrossesAt ) override;
    virtual double SAL_CALL getCrossesAt(  ) override;
    virtual void SAL_CALL setType( ::sal_Int32 Type ) override;
    virtual ::sal_Int32 SAL_CALL getType(  ) override;
    virtual void SAL_CALL setHasTitle( sal_Bool HasTitle ) override;
    virtual sal_Bool SAL_CALL getHasTitle(  ) override;
    virtual void SAL_CALL setMinorUnit( double MinorUnit ) override;
    virtual double SAL_CALL getMinorUnit(  ) override;
    virtual void SAL_CALL setMinorUnitIsAuto( sal_Bool MinorUnitIsAuto ) override;
    virtual sal_Bool SAL_CALL getMinorUnitIsAuto(  ) override;
    virtual void SAL_CALL setReversePlotOrder( sal_Bool ReversePlotOrder ) override;
    virtual sal_Bool SAL_CALL getReversePlotOrder(  ) override;
    virtual void SAL_CALL setMajorUnit( double MajorUnit ) override;
    virtual double SAL_CALL getMajorUnit(  ) override;
    virtual void SAL_CALL setMajorUnitIsAuto( sal_Bool MajorUnitIsAuto ) override;
    virtual sal_Bool SAL_CALL getMajorUnitIsAuto(  ) override;
    virtual void SAL_CALL setMaximumScale( double MaximumScale ) override;
    virtual double SAL_CALL getMaximumScale(  ) override;
    virtual void SAL_CALL setMaximumScaleIsAuto( sal_Bool MaximumScaleIsAuto ) override;
    virtual sal_Bool SAL_CALL getMaximumScaleIsAuto(  ) override;
    virtual void SAL_CALL setMinimumScale( double MinimumScale ) override;
    virtual double SAL_CALL getMinimumScale(  ) override;
    virtual void SAL_CALL setMinimumScaleIsAuto( sal_Bool MinimumScaleIsAuto ) override;
    virtual sal_Bool SAL_CALL getMinimumScaleIsAuto(  ) override;
    virtual ::sal_Int32 SAL_CALL getAxisGroup(  ) override;
    virtual void SAL_CALL setScaleType( ::sal_Int32 ScaleType ) override;
    virtual ::sal_Int32 SAL_CALL getScaleType(  ) override;
    virtual double SAL_CALL getHeight(  ) override;
    virtual void SAL_CALL setHeight( double height ) override;
    virtual double SAL_CALL getWidth(  ) override;
    virtual void SAL_CALL setWidth( double width ) override;
    virtual double SAL_CALL getTop(  ) override;
    virtual void SAL_CALL setTop( double top ) override;
    virtual double SAL_CALL getLeft(  ) override;
    virtual void SAL_CALL setLeft( double left ) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
