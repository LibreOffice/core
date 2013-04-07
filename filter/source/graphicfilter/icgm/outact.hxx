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

#ifndef CGM_OUTACT_HXX_
#define CGM_OUTACT_HXX_

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

typedef struct NodeFrameSet
{
    Point       nTopLeft;
    Point       nBottomRight;
    ::com::sun::star::awt::Size     nSize;
} NodeFrameSet;

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
        Point*                  mpPoints;
        PolyPolygon             maPolyPolygon;
        ::com::sun::star::awt::Gradient*            mpGradient;

        CGM*                    mpCGM;

    public:
                                CGMOutAct( CGM& rCGM );
    virtual                     ~CGMOutAct();
    virtual void                FirstOutPut() { mpCGM->mbFirstOutPut = sal_False; } ;
    virtual void                InsertPage() { mnCurrentPage++; } ;
    virtual void                BeginGroup() {} ;
    virtual void                EndGroup() {};
    virtual void                EndGrouping() {} ;
    void                        BeginFigure() ;
    void                        CloseRegion() ;
    void                        NewRegion() ;
    void                        EndFigure() ;
    void                        RegPolyLine( Polygon&, sal_Bool bReverse = sal_False ) ;
    void                        SetGradientOffset( long nHorzOfs, long nVertOfs, sal_uInt32 nType );
    void                        SetGradientAngle( long nAngle );
    void                        SetGradientDescriptor( sal_uInt32 nColorFrom, sal_uInt32 nColorTo );
    void                        SetGradientStyle( sal_uInt32 nStyle, double fRatio );
    virtual void                DrawRectangle( FloatRect& ) {} ;
    virtual void                DrawEllipse( FloatPoint&, FloatPoint&, double& ) {} ;
    virtual void                DrawEllipticalArc( FloatPoint&, FloatPoint&, double&,
                                    sal_uInt32, double&, double&) {} ;
    virtual void                DrawBitmap( CGMBitmapDescriptor* ) {} ;
    virtual void                DrawPolygon( Polygon& ) {} ;
    virtual void                DrawPolyLine( Polygon& ) {} ;
    virtual void                DrawPolybezier( Polygon& ) {} ;
    virtual void                DrawPolyPolygon( PolyPolygon& ) {} ;
    virtual void                DrawText( ::com::sun::star::awt::Point&, ::com::sun::star::awt::Size&, char*, sal_uInt32, FinalFlag ) {} ;
    virtual void                AppendText( char*, sal_uInt32, FinalFlag ) {} ;
    virtual sal_uInt32              DrawText( TextEntry*, NodeFrameSet&, sal_uInt32 ) { return 0; } ;
    virtual void                DrawChart(){} ;
};

class CGMImpressOutAct : public CGMOutAct
{
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPages >               maXDrawPages;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >                maXDrawPage;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >        maXMultiServiceFactory;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >                   maXShape;
    sal_Bool                        ImplCreateShape( const OUString& rType );

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >           maXPropSet;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >                  maXShapes;

    sal_uInt32                      nFinalTextCount;

    sal_Bool                        ImplInitPage();
    void                        ImplSetOrientation( FloatPoint& RefPoint, double& Orientation ) ;
    void                        ImplSetLineBundle() ;
    void                        ImplSetFillBundle() ;
    void                        ImplSetTextBundle( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & ) ;
public:
                                CGMImpressOutAct( CGM&, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & ) ;
                                ~CGMImpressOutAct() {} ;
    virtual void                InsertPage() ;
    virtual void                BeginGroup() ;
    virtual void                EndGroup() ;
    virtual void                EndGrouping() ;
    virtual void                DrawRectangle( FloatRect& ) ;
    virtual void                DrawEllipse( FloatPoint& center, FloatPoint&, double& Orientation ) ;
    virtual void                DrawEllipticalArc( FloatPoint& center, FloatPoint& size, double& orientation,
                                    sal_uInt32 etype, double& startangle, double& endangle ) ;
    virtual void                DrawBitmap( CGMBitmapDescriptor* ) ;
    virtual void                DrawPolygon( Polygon& ) ;
    virtual void                DrawPolyLine( Polygon& ) ;
    virtual void                DrawPolybezier( Polygon& ) ;
    virtual void                DrawPolyPolygon( PolyPolygon& ) ;
    virtual void                DrawText( ::com::sun::star::awt::Point& TextRectPos, ::com::sun::star::awt::Size& TextRectSize, char* String, sal_uInt32 StringSize, FinalFlag ) ;
    virtual void                AppendText( char* String, sal_uInt32 StringSize, FinalFlag ) ;
    virtual sal_uInt32              DrawText( TextEntry*, NodeFrameSet&, sal_uInt32 ) ;
    virtual void                DrawChart();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
