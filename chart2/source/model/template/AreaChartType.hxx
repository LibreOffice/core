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
#ifndef INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_AREACHARTTYPE_HXX
#define INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_AREACHARTTYPE_HXX

#include "ChartType.hxx"

namespace chart
{

class AreaChartType : public ChartType
{
public:
    explicit AreaChartType(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > const & xContext );
    virtual ~AreaChartType();

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
    explicit AreaChartType( const AreaChartType & rOther );

    // ____ XChartType ____
    virtual OUString SAL_CALL getChartType()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_AREACHARTTYPE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
