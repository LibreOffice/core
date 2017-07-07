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

#ifndef INCLUDED_FILTER_SOURCE_GRAPHICFILTER_ICGM_OUTACT_HXX
#define INCLUDED_FILTER_SOURCE_GRAPHICFILTER_ICGM_OUTACT_HXX

#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>


#define CGM_OUTACT_MAX_GROUP_LEVEL 64

#include "cgm.hxx"
#include <chart.hxx>
#include <tools/poly.hxx>
#include <array>
#include <memory>

class CGM;
class CGMBitmapDescriptor;

class CGMImpressOutAct
{
    sal_uInt16                  mnCurrentPage;          // defaulted to zero

    sal_uInt32                  mnGroupActCount;        // grouping
    sal_uInt32                  mnGroupLevel;
    std::array<sal_uInt32, CGM_OUTACT_MAX_GROUP_LEVEL>
                                mpGroupLevel;

    std::vector<PolyFlags>      maFlags;
    std::vector<Point>          maPoints;
    tools::PolyPolygon          maPolyPolygon;
    std::unique_ptr<css::awt::Gradient>
                                mpGradient;

    CGM*                        mpCGM;

    css::uno::Reference< css::drawing::XDrawPages >               maXDrawPages;
    css::uno::Reference< css::drawing::XDrawPage >                maXDrawPage;

    css::uno::Reference< css::lang::XMultiServiceFactory >        maXMultiServiceFactory;
    css::uno::Reference< css::drawing::XShape >                   maXShape;

    css::uno::Reference< css::beans::XPropertySet >               maXPropSet;
    css::uno::Reference< css::drawing::XShapes >                  maXShapes;

    sal_uInt32                      nFinalTextCount;

    bool                        ImplCreateShape( const OUString& rType );
    bool                        ImplInitPage();
    void                        ImplSetOrientation( FloatPoint& RefPoint, double& Orientation );
    void                        ImplSetLineBundle();
    void                        ImplSetFillBundle();
    void                        ImplSetTextBundle( const css::uno::Reference< css::beans::XPropertySet > & );
public:
                                CGMImpressOutAct( CGM&, const css::uno::Reference< css::frame::XModel > & );
                                ~CGMImpressOutAct();
    void                        InsertPage();
    void                        BeginGroup();
    void                        EndGroup();
    void                        EndGrouping();
    void                        DrawRectangle( FloatRect& );
    void                        DrawEllipse( FloatPoint& center, FloatPoint&, double& Orientation );
    void                        DrawEllipticalArc( FloatPoint& center, FloatPoint& size, double& orientation,
                                    sal_uInt32 etype, double& startangle, double& endangle );
    void                        DrawBitmap( CGMBitmapDescriptor* );
    void                        DrawPolygon( tools::Polygon& );
    void                        DrawPolyLine( tools::Polygon& );
    void                        DrawPolybezier( tools::Polygon& );
    void                        DrawPolyPolygon( tools::PolyPolygon& );
    void                        DrawText( css::awt::Point& TextRectPos, css::awt::Size& TextRectSize, char* String, FinalFlag );
    void                        AppendText( const char* String );

    void                        FirstOutPut() { mpCGM->mbFirstOutPut = false; } ;
    void                        BeginFigure();
    void                        CloseRegion();
    void                        NewRegion();
    void                        EndFigure();
    void                        RegPolyLine( tools::Polygon&, bool bReverse = false );
    void                        SetGradientOffset( long nHorzOfs, long nVertOfs );
    void                        SetGradientAngle( long nAngle );
    void                        SetGradientDescriptor( sal_uInt32 nColorFrom, sal_uInt32 nColorTo );
    void                        SetGradientStyle( sal_uInt32 nStyle );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
