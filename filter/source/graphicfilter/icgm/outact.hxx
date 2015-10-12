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

struct NodeFrameSet
{
    Point          nTopLeft;
    Point          nBottomRight;
    css::awt::Size nSize;
};

class CGM;
class CGMBitmapDescriptor;
class CGMOutAct
{
protected:
        sal_uInt16                  mnCurrentPage;          // defaulted to zero

        sal_uInt32                  mnGroupActCount;        // grouping
        sal_uInt32                  mnGroupLevel;
        sal_uInt32*                 mpGroupLevel;

        sal_uInt16                  mnIndex;                // figure
        sal_uInt8*                  mpFlags;
        Point*                      mpPoints;
        tools::PolyPolygon          maPolyPolygon;
        css::awt::Gradient*         mpGradient;

        CGM*                        mpCGM;

public:
    explicit                    CGMOutAct( CGM& rCGM );
    virtual                     ~CGMOutAct();
    void                        FirstOutPut() { mpCGM->mbFirstOutPut = false; } ;
    virtual void                InsertPage() { mnCurrentPage++; } ;
    virtual void                BeginGroup() {} ;
    virtual void                EndGroup() {};
    virtual void                EndGrouping() {} ;
    void                        BeginFigure() ;
    void                        CloseRegion() ;
    void                        NewRegion() ;
    void                        EndFigure() ;
    void                        RegPolyLine( tools::Polygon&, bool bReverse = false ) ;
    void                        SetGradientOffset( long nHorzOfs, long nVertOfs, sal_uInt32 nType );
    void                        SetGradientAngle( long nAngle );
    void                        SetGradientDescriptor( sal_uInt32 nColorFrom, sal_uInt32 nColorTo );
    void                        SetGradientStyle( sal_uInt32 nStyle, double fRatio );
    virtual void                DrawRectangle( FloatRect& ) {} ;
    virtual void                DrawEllipse( FloatPoint&, FloatPoint&, double& ) {} ;
    virtual void                DrawEllipticalArc( FloatPoint&, FloatPoint&, double&,
                                    sal_uInt32, double&, double&) {} ;
    virtual void                DrawBitmap( CGMBitmapDescriptor* ) {} ;
    virtual void                DrawPolygon( tools::Polygon& ) {} ;
    virtual void                DrawPolyLine( tools::Polygon& ) {} ;
    virtual void                DrawPolybezier( tools::Polygon& ) {} ;
    virtual void                DrawPolyPolygon( tools::PolyPolygon& ) {} ;
    virtual void                DrawText( css::awt::Point&, css::awt::Size&, char*, sal_uInt32, FinalFlag ) {} ;
    virtual void                AppendText( char*, sal_uInt32, FinalFlag ) {} ;
    virtual void                DrawChart(){} ;
};

class CGMImpressOutAct : public CGMOutAct
{
    css::uno::Reference< css::drawing::XDrawPages >               maXDrawPages;
    css::uno::Reference< css::drawing::XDrawPage >                maXDrawPage;

    css::uno::Reference< css::lang::XMultiServiceFactory >        maXMultiServiceFactory;
    css::uno::Reference< css::drawing::XShape >                   maXShape;

    css::uno::Reference< css::beans::XPropertySet >               maXPropSet;
    css::uno::Reference< css::drawing::XShapes >                  maXShapes;

    sal_uInt32                      nFinalTextCount;

    bool                        ImplCreateShape( const OUString& rType );
    bool                        ImplInitPage();
    void                        ImplSetOrientation( FloatPoint& RefPoint, double& Orientation ) ;
    void                        ImplSetLineBundle() ;
    void                        ImplSetFillBundle() ;
    void                        ImplSetTextBundle( const css::uno::Reference< css::beans::XPropertySet > & ) ;
public:
                                CGMImpressOutAct( CGM&, const css::uno::Reference< css::frame::XModel > & ) ;
                                virtual ~CGMImpressOutAct() {} ;
    virtual void                InsertPage() override ;
    virtual void                BeginGroup() override ;
    virtual void                EndGroup() override ;
    virtual void                EndGrouping() override ;
    virtual void                DrawRectangle( FloatRect& ) override ;
    virtual void                DrawEllipse( FloatPoint& center, FloatPoint&, double& Orientation ) override ;
    virtual void                DrawEllipticalArc( FloatPoint& center, FloatPoint& size, double& orientation,
                                    sal_uInt32 etype, double& startangle, double& endangle ) override ;
    virtual void                DrawBitmap( CGMBitmapDescriptor* ) override ;
    virtual void                DrawPolygon( tools::Polygon& ) override ;
    virtual void                DrawPolyLine( tools::Polygon& ) override ;
    virtual void                DrawPolybezier( tools::Polygon& ) override ;
    virtual void                DrawPolyPolygon( tools::PolyPolygon& ) override ;
    virtual void                DrawText( css::awt::Point& TextRectPos, css::awt::Size& TextRectSize, char* String, sal_uInt32 StringSize, FinalFlag ) override ;
    virtual void                AppendText( char* String, sal_uInt32 StringSize, FinalFlag ) override ;
    virtual void                DrawChart() override;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
