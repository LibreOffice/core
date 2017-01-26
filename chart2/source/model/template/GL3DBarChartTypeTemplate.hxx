/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_GL3DBARCHARTTYPETEMPLATE_HXX
#define INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_GL3DBARCHARTTYPETEMPLATE_HXX

#include <MutexContainer.hxx>
#include <ChartTypeTemplate.hxx>
#include <OPropertySet.hxx>

#include <comphelper/uno3.hxx>

namespace chart {

class GL3DBarChartTypeTemplate :
    public MutexContainer, public property::OPropertySet, public ChartTypeTemplate
{
public:

    DECLARE_XINTERFACE()

    GL3DBarChartTypeTemplate(
        const css::uno::Reference<
            css::uno::XComponentContext>& xContext,
        const OUString& rServiceName );

    virtual ~GL3DBarChartTypeTemplate() override;

    virtual css::uno::Reference<css::chart2::XChartType> getChartTypeForIndex( sal_Int32 nChartTypeIndex ) override;

    // XChartTypeTemplate

    virtual sal_Bool SAL_CALL matchesTemplate(
        const css::uno::Reference<css::chart2::XDiagram>& xDiagram,
        sal_Bool bAdaptProperties ) override;

    virtual css::uno::Reference<css::chart2::XChartType> SAL_CALL
        getChartTypeForNewSeries( const css::uno::Sequence<css::uno::Reference<css::chart2::XChartType> >& xOldChartTypes ) override;

    virtual sal_Bool SAL_CALL supportsCategories() override;

    // OPropertySet
    virtual css::uno::Any GetDefaultValue( sal_Int32 nHandle ) const override;

    virtual cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

    // XPropertySet
    virtual css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL
        getPropertySetInfo() override;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
