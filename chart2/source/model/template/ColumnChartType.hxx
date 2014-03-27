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
#ifndef INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_COLUMNCHARTTYPE_HXX
#define INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_COLUMNCHARTTYPE_HXX

#include "ChartType.hxx"
#include "ServiceMacros.hxx"

namespace chart
{

class ColumnChartType : public ChartType
{
public:
    ColumnChartType(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > const & xContext );
    virtual ~ColumnChartType();

    //TODO: are these actually used (given they are not SAL_OVERRIDE)?
    virtual OUString SAL_CALL
        getImplementationName()
            throw( ::com::sun::star::uno::RuntimeException, std::exception );
    virtual sal_Bool SAL_CALL
        supportsService( const OUString& ServiceName )
            throw( ::com::sun::star::uno::RuntimeException, std::exception );
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
            throw( ::com::sun::star::uno::RuntimeException, std::exception );
    static OUString getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< OUString >
        getSupportedServiceNames_Static();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( ColumnChartType )

protected:
    explicit ColumnChartType( const ColumnChartType & rOther );

    // ____ XChartType ____
    virtual OUString SAL_CALL getChartType()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedPropertyRoles()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ OPropertySet ____
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException) SAL_OVERRIDE;
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() SAL_OVERRIDE;

    // ____ XPropertySet ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_COLUMNCHARTTYPE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
