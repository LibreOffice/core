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

#ifndef INCLUDED_XMLOFF_SCHXMLSERIESHELPER_HXX
#define INCLUDED_XMLOFF_SCHXMLSERIESHELPER_HXX

#include <com/sun/star/uno/Reference.hxx>

#include <xmloff/dllapi.h>

#include <vector>
#include <map>

namespace com { namespace sun { namespace star { namespace beans { class XPropertySet; } } } }
namespace com { namespace sun { namespace star { namespace chart2 { class XDataSeries; } } } }
namespace com { namespace sun { namespace star { namespace chart2 { class XDiagram; } } } }
namespace com { namespace sun { namespace star { namespace frame { class XModel; } } } }

class XMLOFF_DLLPUBLIC SchXMLSeriesHelper
{
public:
    static ::std::vector< css::uno::Reference< css::chart2::XDataSeries > >
            getDataSeriesFromDiagram(
                const css::uno::Reference< css::chart2::XDiagram > & xDiagram );
    static ::std::map< css::uno::Reference< css::chart2::XDataSeries >, sal_Int32 >
            getDataSeriesIndexMapFromDiagram(
                const css::uno::Reference< css::chart2::XDiagram > & xDiagram );

    static bool isCandleStickSeries(
                  const css::uno::Reference< css::chart2::XDataSeries >& xSeries
                , const css::uno::Reference< css::frame::XModel >& xChartModel  );

    static css::uno::Reference< css::beans::XPropertySet > createOldAPISeriesPropertySet(
                    const css::uno::Reference< css::chart2::XDataSeries >& xSeries
                    , const css::uno::Reference< css::frame::XModel >& xChartModel );

    static css::uno::Reference< css::beans::XPropertySet > createOldAPIDataPointPropertySet(
                    const css::uno::Reference< css::chart2::XDataSeries >& xSeries
                    , sal_Int32 nPointIndex
                    , const css::uno::Reference< css::frame::XModel >& xChartModel );
};

// INCLUDED_XMLOFF_SCHXMLSERIESHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
