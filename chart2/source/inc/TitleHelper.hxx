/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
