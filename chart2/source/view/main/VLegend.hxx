/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VLegend.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:51:19 $
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
#ifndef _VLEGEND_HXX
#define _VLEGEND_HXX

#ifndef _COM_SUN_STAR_CHART2_XLEGEND_HPP_
#include <com/sun/star/chart2/XLegend.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASERIESTREEPARENT_HPP_
#include <com/sun/star/chart2/XDataSeriesTreeParent.hpp>
#endif
#ifndef _COM_SUN_STAR_LAYOUT_ALIGNMENT_HPP_
#include <com/sun/star/layout/Alignment.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class VLegend
{
public:
    VLegend( const ::com::sun::star::uno::Reference<
                 ::com::sun::star::chart2::XLegend > & xLegend );

    void SAL_CALL init( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::drawing::XShapes >& xTargetPage,
                        const ::com::sun::star::uno::Reference<
                            ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
                        const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XModel > & xModel );

    void createShapes( const ::com::sun::star::awt::Size & rAvailableSpace,
                       const ::com::sun::star::awt::Size & rPageSize );

    /** Sets the position according to its internal anchor.

        @param rOutAvailableSpace
            is modified by the method, if the legend is in a standard position,
            such that the space allocated by the legend is removed from it.

        @param rReferenceSize
            is used to calculate the offset (default 2%) from the edge.
     */
    void changePosition(
        ::com::sun::star::awt::Rectangle & rOutAvailableSpace,
        const ::com::sun::star::awt::Size & rReferenceSize );

    static bool isVisible(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XLegend > & xLegend );

private:
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes >            m_xTarget;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory>   m_xShapeFactory;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XLegend >     m_xLegend;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShape >             m_xShape;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XModel >               m_xModel;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

