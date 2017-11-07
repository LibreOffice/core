/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_GL3DBARCHARTTYPE_HXX
#define INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_GL3DBARCHARTTYPE_HXX

#include "ChartType.hxx"

namespace chart {

/**
 * Chart type that represents 3 dimensional data content in 3D space using
 * OpenGL.
 */
class GL3DBarChartType final : public ChartType
{
public:
    explicit GL3DBarChartType( const css::uno::Reference<css::uno::XComponentContext>& xContext );
    virtual ~GL3DBarChartType() override;

    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedPropertyRoles() override;
    virtual OUString SAL_CALL
        getImplementationName() override;
    virtual sal_Bool SAL_CALL
        supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames() override;

private:
    GL3DBarChartType( const GL3DBarChartType& rOther );

    virtual OUString SAL_CALL getChartType() override;

    virtual css::uno::Reference<css::util::XCloneable> SAL_CALL
        createClone() override;

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
