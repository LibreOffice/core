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
#ifndef _VLEGEND_HXX
#define _VLEGEND_HXX

#include <com/sun/star/chart2/XLegend.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <vector>

namespace chart
{

class LegendEntryProvider;

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

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
