/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TitleHelper.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 16:00:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _CHART2_TOOLS_TITLEHELPER_HXX
#define _CHART2_TOOLS_TITLEHELPER_HXX

#include "ReferenceSizeProvider.hxx"

#ifndef _COM_SUN_STAR_CHART2_XTITLED_HPP_
#include <com/sun/star/chart2/XTitled.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <com/sun/star/chart2/XDiagram.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

class TitleHelper
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
