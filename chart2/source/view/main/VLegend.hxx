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
#ifndef _VLEGEND_HXX
#define _VLEGEND_HXX

#include <com/sun/star/chart2/XLegend.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <vector>

//.............................................................................
namespace chart
{
//.............................................................................

class LegendEntryProvider;

//-----------------------------------------------------------------------------
/**
*/

class VLegend
{
public:
    VLegend( const ::com::sun::star::uno::Reference<
                 ::com::sun::star::chart2::XLegend > & xLegend,
             const ::com::sun::star::uno::Reference<
                 ::com::sun::star::uno::XComponentContext > & xContext,
             const std::vector< LegendEntryProvider* >& rLegendEntryProviderList );

    void init( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::drawing::XShapes >& xTargetPage,
                        const ::com::sun::star::uno::Reference<
                            ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
                        const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XModel > & xModel );

    void setDefaultWritingMode( sal_Int16 nDefaultWritingMode );

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
                    ::com::sun::star::chart2::XLegend >             m_xLegend;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShape >             m_xShape;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XModel >               m_xModel;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::uno::XComponentContext >      m_xContext;

    std::vector< LegendEntryProvider* >         m_aLegendEntryProviderList;

    sal_Int16 m_nDefaultWritingMode;//to be used when writing mode is set to page
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
