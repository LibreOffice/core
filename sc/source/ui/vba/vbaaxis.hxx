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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAAXIS_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAAXIS_HXX
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
    ScVbaChart* getChartPtr() throw( css::uno::RuntimeException );
    bool isValueAxis() throw( css::script::BasicErrorException );
    std::unique_ptr<ov::ShapeHelper> oShapeHelper;

public:
    ScVbaAxis( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::beans::XPropertySet >& _xPropertySet, sal_Int32 _nType, sal_Int32 _nGroup );
    // Methods
    virtual void SAL_CALL Delete(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ::ooo::vba::excel::XAxisTitle > SAL_CALL getAxisTitle(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDisplayUnit( ::sal_Int32 DisplayUnit ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getDisplayUnit(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCrosses( ::sal_Int32 Crosses ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getCrosses(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCrossesAt( double CrossesAt ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getCrossesAt(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setType( ::sal_Int32 Type ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getType(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setHasTitle( sal_Bool HasTitle ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getHasTitle(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMinorUnit( double MinorUnit ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getMinorUnit(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMinorUnitIsAuto( sal_Bool MinorUnitIsAuto ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getMinorUnitIsAuto(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setReversePlotOrder( sal_Bool ReversePlotOrder ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getReversePlotOrder(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMajorUnit( double MajorUnit ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getMajorUnit(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMajorUnitIsAuto( sal_Bool MajorUnitIsAuto ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getMajorUnitIsAuto(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMaximumScale( double MaximumScale ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getMaximumScale(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMaximumScaleIsAuto( sal_Bool MaximumScaleIsAuto ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getMaximumScaleIsAuto(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMinimumScale( double MinimumScale ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getMinimumScale(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMinimumScaleIsAuto( sal_Bool MinimumScaleIsAuto ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getMinimumScaleIsAuto(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getAxisGroup(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setScaleType( ::sal_Int32 ScaleType ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getScaleType(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getHeight(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setHeight( double height ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getWidth(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setWidth( double width ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getTop(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTop( double top ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getLeft(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLeft( double left ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif // INCLUDED_SC_SOURCE_UI_VBA_VBAAXIS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
