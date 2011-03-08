/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _CHART2_TOOLS_LEGENDHELPER_HXX
#define _CHART2_TOOLS_LEGENDHELPER_HXX

// header for class OUString
#include <rtl/ustring.hxx>
#include <com/sun/star/chart2/XLegend.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include "charttoolsdllapi.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
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

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
