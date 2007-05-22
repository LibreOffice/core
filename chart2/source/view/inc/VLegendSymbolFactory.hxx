/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VLegendSymbolFactory.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:21:32 $
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
#ifndef CHART2_VLEGENDSYMBOLFACTORY_HXX
#define CHART2_VLEGENDSYMBOLFACTORY_HXX

#ifndef _COM_SUN_STAR_CHART2_LEGENDSYMBOLSTYLE_HPP_
#include <com/sun/star/chart2/LegendSymbolStyle.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif

namespace chart
{

class VLegendSymbolFactory
{
public:
    enum tPropertyType
    {
        PROP_TYPE_FILLED_SERIES,
        PROP_TYPE_LINE_SERIES,
        PROP_TYPE_FILL,
        PROP_TYPE_LINE,
        PROP_TYPE_FILL_AND_LINE
    };

    static ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShape >
        createSymbol(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes > xSymbolContainer,
            ::com::sun::star::chart2::LegendSymbolStyle eStyle,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory > & xShapeFactory,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > & xLegendEntryProperties,
            tPropertyType ePropertyType,
            const ::com::sun::star::uno::Any& rExplicitSymbol /*should contain a ::com::sun::star::chart2::Symbol without automatic symbol if the charttype does support symbols else empty*/);

    /// @param bWhiteDay: if <FALSE/> this symbol is for BlackDays
    static ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShape >
        createJapaneseCandleStickSymbol(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes > xSymbolContainer,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory > & xShapeFactory,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > & xLegendEntryProperties,
            bool bWhiteDay );

    enum tStockLineType
    {
        STOCK_LINE_TYPE_VERT,
        STOCK_LINE_TYPE_OPEN,
        STOCK_LINE_TYPE_CLOSE
    };

    static ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShape >
        createStockLineSymbol(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes > xSymbolContainer,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory > & xShapeFactory,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > & xLegendEntryProperties,
            tStockLineType eType );

private:
     VLegendSymbolFactory();
};

} //  namespace chart

// CHART2_VLEGENDSYMBOLFACTORY_HXX
#endif
