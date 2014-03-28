/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHART2_GL3DBARCHARTTYPETEMPLATE_HXX
#define CHART2_GL3DBARCHARTTYPETEMPLATE_HXX

#include <MutexContainer.hxx>
#include <ChartTypeTemplate.hxx>
#include <OPropertySet.hxx>
#include <ServiceMacros.hxx>

namespace chart {

class GL3DBarChartTypeTemplate : public ChartTypeTemplate
{
public:
    GL3DBarChartTypeTemplate(
        const css::uno::Reference<
            css::uno::XComponentContext>& xContext,
        const OUString& rServiceName );

    virtual ~GL3DBarChartTypeTemplate();

    virtual css::uno::Reference<css::chart2::XChartType> getChartTypeForIndex( sal_Int32 nChartTypeIndex ) SAL_OVERRIDE;

    // XChartTypeTemplate

    virtual css::uno::Reference<css::chart2::XChartType> SAL_CALL
        getChartTypeForNewSeries( const css::uno::Sequence<css::uno::Reference<css::chart2::XChartType> >& xOldChartTypes )
            throw (::css::uno::RuntimeException, ::std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL supportsCategories()
        throw (::css::uno::RuntimeException, ::std::exception) SAL_OVERRIDE;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
