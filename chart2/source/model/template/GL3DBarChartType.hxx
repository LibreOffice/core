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
class GL3DBarChartType : public ChartType
{
public:
    explicit GL3DBarChartType( const css::uno::Reference<css::uno::XComponentContext>& xContext );
    virtual ~GL3DBarChartType();

    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedPropertyRoles()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL
        getImplementationName()
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
        supportsService( const OUString& ServiceName )
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    static OUString getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< OUString >
        getSupportedServiceNames_Static();

protected:
    GL3DBarChartType( const GL3DBarChartType& rOther );

    virtual OUString SAL_CALL getChartType()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<css::util::XCloneable> SAL_CALL
        createClone()
            throw (css::uno::RuntimeException, std::exception) override;

    // OPropertySet
    virtual css::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw (css::beans::UnknownPropertyException) override;

    virtual cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

    // XPropertySet
    virtual css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL
        getPropertySetInfo()
            throw (css::uno::RuntimeException, std::exception) override;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
