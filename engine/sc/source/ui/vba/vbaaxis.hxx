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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/excel/XAxis.hpp>
#include <ooo/vba/excel/XChart.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <rtl/ref.hxx>
#include <memory>

class ScVbaChart;

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XAxis >  ScVbaAxis_BASE;
class ScVbaAxis : public ScVbaAxis_BASE
{
    rtl::Reference< ScVbaChart > moChartParent;
    cpo::uno::Reference< css::beans::XPropertySet > mxPropertySet;
    sal_Int32 mnType;
    sal_Int32 mnGroup;
    ov::ShapeHelper maShapeHelper;

    bool bCrossesAreCustomized;
    /// @throws cpo::uno::RuntimeException
    ScVbaChart* getChartPtr();
    /// @throws css::script::BasicErrorException
    bool isValueAxis();

public:
    ScVbaAxis( const rtl::Reference< ScVbaChart >& xParent, const cpo::uno::Reference< cpo::uno::XComponentContext > & xContext, cpo::uno::Reference< css::beans::XPropertySet >  _xPropertySet, sal_Int32 _nType, sal_Int32 _nGroup );
    // Methods
    virtual void Delete(  ) override;
    virtual cpo::uno::Reference< ::ooo::vba::excel::XAxisTitle > getAxisTitle(  ) override;
    virtual void setDisplayUnit( ::sal_Int32 DisplayUnit ) override;
    virtual ::sal_Int32 getDisplayUnit(  ) override;
    virtual void setCrosses( ::sal_Int32 Crosses ) override;
    virtual ::sal_Int32 getCrosses(  ) override;
    virtual void setCrossesAt( double CrossesAt ) override;
    virtual double getCrossesAt(  ) override;
    virtual void setType( ::sal_Int32 Type ) override;
    virtual ::sal_Int32 getType(  ) override;
    virtual void setHasTitle( bool HasTitle ) override;
    virtual bool getHasTitle(  ) override;
    virtual void setMinorUnit( double MinorUnit ) override;
    virtual double getMinorUnit(  ) override;
    virtual void setMinorUnitIsAuto( bool MinorUnitIsAuto ) override;
    virtual bool getMinorUnitIsAuto(  ) override;
    virtual void setReversePlotOrder( bool ReversePlotOrder ) override;
    virtual bool getReversePlotOrder(  ) override;
    virtual void setMajorUnit( double MajorUnit ) override;
    virtual double getMajorUnit(  ) override;
    virtual void setMajorUnitIsAuto( bool MajorUnitIsAuto ) override;
    virtual bool getMajorUnitIsAuto(  ) override;
    virtual void setMaximumScale( double MaximumScale ) override;
    virtual double getMaximumScale(  ) override;
    virtual void setMaximumScaleIsAuto( bool MaximumScaleIsAuto ) override;
    virtual bool getMaximumScaleIsAuto(  ) override;
    virtual void setMinimumScale( double MinimumScale ) override;
    virtual double getMinimumScale(  ) override;
    virtual void setMinimumScaleIsAuto( bool MinimumScaleIsAuto ) override;
    virtual bool getMinimumScaleIsAuto(  ) override;
    virtual ::sal_Int32 getAxisGroup(  ) override;
    virtual void setScaleType( ::sal_Int32 ScaleType ) override;
    virtual ::sal_Int32 getScaleType(  ) override;
    virtual double getHeight(  ) override;
    virtual void setHeight( double height ) override;
    virtual double getWidth(  ) override;
    virtual void setWidth( double width ) override;
    virtual double getTop(  ) override;
    virtual void setTop( double top ) override;
    virtual double getLeft(  ) override;
    virtual void setLeft( double left ) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
