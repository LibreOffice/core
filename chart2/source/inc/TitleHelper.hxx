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
#ifndef _CHART2_TOOLS_TITLEHELPER_HXX
#define _CHART2_TOOLS_TITLEHELPER_HXX

#include "ReferenceSizeProvider.hxx"
#include "charttoolsdllapi.hxx"
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

class OOO_DLLPUBLIC_CHARTTOOLS TitleHelper
{
public:
    enum eTitleType
    {
        TITLE_BEGIN = 0,
        MAIN_TITLE = 0,
        SUB_TITLE,
        X_AXIS_TITLE,
        Y_AXIS_TITLE,
        Z_AXIS_TITLE,
        SECONDARY_X_AXIS_TITLE,
        SECONDARY_Y_AXIS_TITLE,
        NORMAL_TITLE_END,

        //it is intended that this both types are after NORMAL_TITLE_END
        TITLE_AT_STANDARD_X_AXIS_POSITION, //equals the Y_AXIS_TITLE for barchart
        TITLE_AT_STANDARD_Y_AXIS_POSITION  //equals the X_AXIS_TITLE for barchart
    };

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XTitle >
        getTitle( eTitleType nTitleIndex
                    , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XModel >& xModel );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XTitle >
        createTitle(  eTitleType nTitleIndex
                    , const rtl::OUString& rTitleText
                    , const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XModel >& xModel
                    , const ::com::sun::star::uno::Reference<
                            ::com::sun::star::uno::XComponentContext > & xContext
                    , ReferenceSizeProvider * pRefSizeProvider = 0 );

    static void removeTitle( eTitleType nTitleIndex
                    , const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XModel >& xModel );

    static rtl::OUString getCompleteString( const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XTitle >& xTitle );
    static void setCompleteString( const rtl::OUString& rNewText
        , const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XTitle >& xTitle
        , const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext
        , float * pDefaultCharHeight = 0 );

    static bool getTitleType( eTitleType& rType
                    , const ::com::sun::star::uno::Reference<
                        ::com::sun::star::chart2::XTitle >& xTitle
                    , const ::com::sun::star::uno::Reference<
                        ::com::sun::star::frame::XModel >& xModel );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
