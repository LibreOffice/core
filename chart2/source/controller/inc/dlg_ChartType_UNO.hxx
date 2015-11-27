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

#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_INC_DLG_CHARTTYPE_UNO_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_INC_DLG_CHARTTYPE_UNO_HXX

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <vcl/vclevent.hxx>

#include <svtools/genericunodialog.hxx>

namespace chart
{
typedef ::svt::OGenericUnoDialog ChartTypeUnoDlg_BASE;
class ChartTypeUnoDlg : public ChartTypeUnoDlg_BASE
                        ,public ::comphelper::OPropertyArrayUsageHelper< ChartTypeUnoDlg >
{
public:
    ChartTypeUnoDlg( const css::uno::Reference< css::uno::XComponentContext >& xContext );

    // XServiceInfo - static methods
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();
    static OUString getImplementationName_Static() throw( css::uno::RuntimeException );
private:
    virtual ~ChartTypeUnoDlg();

    // OGenericUnoDialog overridables
    virtual void implInitialize(const css::uno::Any& _rValue) override;
    virtual VclPtr<Dialog> createDialog(vcl::Window* _pParent) override;

    // XTypeProvider
    virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(css::uno::RuntimeException, std::exception) override;
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

    ChartTypeUnoDlg(const ChartTypeUnoDlg&) = delete;
    void operator =(const ChartTypeUnoDlg&) = delete;

    css::uno::Reference< css::frame::XModel >           m_xChartModel;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
