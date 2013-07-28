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
#ifndef _CHART2_TOOLS_LEGENDHELPER_HXX
#define _CHART2_TOOLS_LEGENDHELPER_HXX

// header for class OUString
#include <rtl/ustring.hxx>
#include <com/sun/star/chart2/XLegend.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include "charttoolsdllapi.hxx"

namespace chart
{

/**
*/

class OOO_DLLPUBLIC_CHARTTOOLS LegendHelper
{
public:
    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XLegend >
        showLegend( const ::com::sun::star::uno::Reference<
                       ::com::sun::star::frame::XModel >& xModel
                  , const ::com::sun::star::uno::Reference<
                       ::com::sun::star::uno::XComponentContext >& xContext );

    static  void hideLegend( const ::com::sun::star::uno::Reference<
                       ::com::sun::star::frame::XModel >& xModel );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XLegend >
        getLegend( const ::com::sun::star::uno::Reference<
                       ::com::sun::star::frame::XModel >& xModel
                 , const ::com::sun::star::uno::Reference<
                       ::com::sun::star::uno::XComponentContext >& xContext = 0
                 , bool bCreate = false );

    /** returns <FALSE/>, if either there is no legend at the diagram, or there
        is a legend which has a "Show" property of value <FALSE/>. Otherwise,
        <TRUE/> is returned.
     */
    static bool hasLegend( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram > & xDiagram );
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
