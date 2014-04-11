/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <GL3DHelper.hxx>

#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XChartType.hpp>

using namespace com::sun::star;

namespace chart {

bool GL3DHelper::isGL3DDiagram( const css::uno::Reference<css::chart2::XDiagram>& xDiagram )
{
    uno::Reference<chart2::XCoordinateSystemContainer> xCooSysContainer(xDiagram, uno::UNO_QUERY);

    if (!xCooSysContainer.is())
        return false;

    uno::Sequence< uno::Reference<chart2::XCoordinateSystem> > aCooSysList = xCooSysContainer->getCoordinateSystems();
    for (sal_Int32 nCS = 0; nCS < aCooSysList.getLength(); ++nCS)
    {
        uno::Reference<chart2::XCoordinateSystem> xCooSys = aCooSysList[nCS];

        //iterate through all chart types in the current coordinate system
        uno::Reference<chart2::XChartTypeContainer> xChartTypeContainer(xCooSys, uno::UNO_QUERY);
        OSL_ASSERT( xChartTypeContainer.is());
        if( !xChartTypeContainer.is() )
            continue;

        uno::Sequence< uno::Reference<chart2::XChartType> > aChartTypeList = xChartTypeContainer->getChartTypes();
        for( sal_Int32 nT = 0; nT < aChartTypeList.getLength(); ++nT )
        {
            uno::Reference<chart2::XChartType> xChartType = aChartTypeList[nT];
            OUString aChartType = xChartType->getChartType();
            if( aChartType == "com.sun.star.chart2.GL3DBarChartType" )
                return true;
        }
    }

    return false;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
