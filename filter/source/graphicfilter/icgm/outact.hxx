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
    Point		nTopLeft;
    Point		nBottomRight;
    ::com::sun::star::awt::Size		nSize;
} NodeFrameSet;

class CGM;
class VclGradient;
class CGMOutAct
{
    protected:
        sal_uInt16					mnCurrentPage;			// defaulted to zero

        sal_uInt32					mnGroupActCount;		// grouping
        sal_uInt32					mnGroupLevel;
        sal_uInt32* 				mpGroupLevel;

        USHORT					mnIndex;				// figure
        BYTE*					mpFlags;
        Point*					mpPoints;
        PolyPolygon				maPolyPolygon;
        ::com::sun::star::awt::Gradient*			mpGradient;

        CGM*					mpCGM;

    public:
                                CGMOutAct( CGM& rCGM );
    virtual						~CGMOutAct();
    virtual void				FirstOutPut() { mpCGM->mbFirstOutPut = sal_False; } ;
    virtual void				InsertPage() { mnCurrentPage++; } ;
    virtual void				BeginGroup() {} ;
    virtual void				EndGroup() {};
    virtual void				EndGrouping() {} ;
    void						BeginFigure() ;
    void						CloseRegion() ;
    void						NewRegion() ;
    void						EndFigure() ;
    void						RegPolyLine( Polygon&, sal_Bool bReverse = sal_False ) ;
    void						SetGradientOffset( long nHorzOfs, long nVertOfs, sal_uInt32 nType );
    void						SetGradientAngle( long nAngle );
    void						SetGradientDescriptor( sal_uInt32 nColorFrom, sal_uInt32 nColorTo );
    void						SetGradientStyle( sal_uInt32 nStyle, double fRatio );
    virtual void				DrawRectangle( FloatRect& ) {} ;
    virtual void				DrawEllipse( FloatPoint&, FloatPoint&, double& ) {} ;
    virtual void				DrawEllipticalArc( FloatPoint&, FloatPoint&, double&,
                                    sal_uInt32, double&, double&) {} ;
    virtual void				DrawBitmap( CGMBitmapDescriptor* ) {} ;
    virtual void				DrawPolygon( Polygon& ) {} ;
    virtual void				DrawPolyLine( Polygon& ) {} ;
    virtual void				DrawPolybezier( Polygon& ) {} ;
    virtual void				DrawPolyPolygon( PolyPolygon& ) {} ;
    virtual void				DrawText( ::com::sun::star::awt::Point&, ::com::sun::star::awt::Size&, char*, sal_uInt32, FinalFlag ) {} ;
    virtual void				AppendText( char*, sal_uInt32, FinalFlag ) {} ;
    virtual sal_uInt32				DrawText( TextEntry*, NodeFrameSet&, sal_uInt32 ) { return 0; } ;
    virtual void				DrawChart(){} ;
};

class CGMImpressOutAct : public CGMOutAct
{
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPages > 				maXDrawPages;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > 				maXDrawPage;

//	::com::sun::star::uno::Reference< XServiceRegistry > 			maXServiceRegistry;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > 		maXMultiServiceFactory;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > 		maXServiceManagerSC;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > 					maXShape;
    sal_Bool						ImplCreateShape( const ::rtl::OUString& rType );

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  			maXPropSet;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > 					maXShapes;

    sal_uInt32						nFinalTextCount;

    sal_Bool						ImplInitPage();
    void						ImplSetOrientation( FloatPoint& RefPoint, double& Orientation ) ;
    void						ImplSetLineBundle() ;
    void						ImplSetFillBundle() ;
    void						ImplSetTextBundle( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & ) ;
public:
                                CGMImpressOutAct( CGM&, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & ) ;
                                ~CGMImpressOutAct() {} ;
    virtual void				InsertPage() ;
    virtual void				BeginGroup() ;
    virtual void				EndGroup() ;
    virtual void				EndGrouping() ;
    virtual void				DrawRectangle( FloatRect& ) ;
    virtual void				DrawEllipse( FloatPoint& center, FloatPoint&, double& Orientation ) ;
    virtual void				DrawEllipticalArc( FloatPoint& center, FloatPoint& size, double& orientation,
                                    sal_uInt32 etype, double& startangle, double& endangle ) ;
    virtual void				DrawBitmap( CGMBitmapDescriptor* ) ;
    virtual void				DrawPolygon( Polygon& ) ;
    virtual void				DrawPolyLine( Polygon& ) ;
    virtual void				DrawPolybezier( Polygon& ) ;
    virtual void				DrawPolyPolygon( PolyPolygon& ) ;
    virtual void				DrawText( ::com::sun::star::awt::Point& TextRectPos, ::com::sun::star::awt::Size& TextRectSize, char* String, sal_uInt32 StringSize, FinalFlag ) ;
    virtual void				AppendText( char* String, sal_uInt32 StringSize, FinalFlag ) ;
    virtual sal_uInt32				DrawText( TextEntry*, NodeFrameSet&, sal_uInt32 ) ;
    virtual void				DrawChart();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
