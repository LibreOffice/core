/*************************************************************************
 *
 *  $RCSfile: outact.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CGM_OUTACT_HXX_
#define CGM_OUTACT_HXX_

#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGES_HPP_
#include <com/sun/star/drawing/XDrawPages.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif


#define CGM_OUTACT_MAX_GROUP_LEVEL 64

#include "cgm.hxx"
#include <chart.hxx>
#include <vcl/poly.hxx>

typedef struct NodeFrameSet
{
    Point       nTopLeft;
    Point       nBottomRight;
    ::com::sun::star::awt::Size     nSize;
} NodeFrameSet;

class CGM;
class VclGradient;
class CGMOutAct
{
    protected:
        sal_uInt16                  mnCurrentPage;          // defaulted to zero

        sal_uInt32                  mnGroupActCount;        // grouping
        sal_uInt32                  mnGroupLevel;
        sal_uInt32*                 mpGroupLevel;

        sal_uInt32                  mnIndex;                // figure
        BYTE*                   mpFlags;
        Point*                  mpPoints;
        PolyPolygon             maPolyPolygon;
        ::com::sun::star::awt::Gradient*            mpGradient;

        CGM*                    mpCGM;

    public:
                                CGMOutAct( CGM& rCGM );
                                ~CGMOutAct();
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
    void                        SetGradientEdge( long nEdge );
    void                        SetGradientAngle( long nAngle );
    void                        SetGradientDescriptor( sal_uInt32 nColorFrom, sal_uInt32 nColorTo );
    void                        SetGradientStyle( sal_uInt32 nStyle, double fRatio );
    virtual void                DrawRectangle( FloatRect& ) {} ;
    virtual void                DrawEllipse( FloatPoint& center, FloatPoint& size, double& orientation ) {} ;
    virtual void                DrawEllipticalArc( FloatPoint& center, FloatPoint& size, double& orientation,
                                    sal_uInt32 etype, double& startangle, double& endangle ) {} ;
    virtual void                DrawBitmap( CGMBitmapDescriptor* ) {} ;
    virtual void                DrawPolygon( Polygon& ) {} ;
    virtual void                DrawPolyLine( Polygon& ) {} ;
    virtual void                DrawPolybezier( Polygon& ) {} ;
    virtual void                DrawPolyPolygon( PolyPolygon& ) {} ;
    virtual void                DrawText( ::com::sun::star::awt::Point& TextRectPos, ::com::sun::star::awt::Size& TextRectSize, char* String, sal_uInt32 StringSize, FinalFlag ) {} ;
    virtual void                AppendText( char* String, sal_uInt32 StringSize, FinalFlag ) {} ;
    virtual sal_uInt32              DrawText( TextEntry*, NodeFrameSet&, sal_uInt32 ) { return 0; } ;
    virtual void                DrawChart(){} ;
};

class CGMMetaOutAct : public CGMOutAct
{
    public:
                                CGMMetaOutAct( CGM& rCGM ) ;
                                ~CGMMetaOutAct() {} ;
    virtual void                DrawRectangle( FloatRect& ) ;
    virtual void                DrawBitmap( CGMBitmapDescriptor* ) ;
    virtual void                DrawPolygon( Polygon& ) ;
    virtual void                DrawPolyLine( Polygon& ) ;
    virtual void                DrawPolybezier( Polygon& ) ;
};

class CGMImpressOutAct : public CGMOutAct
{
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPages >               maXDrawPages;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >                maXDrawPage;

//  ::com::sun::star::uno::Reference< XServiceRegistry >            maXServiceRegistry;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >        maXMultiServiceFactory;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >        maXServiceManagerSC;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >                   maXShape;
    sal_Bool                        ImplCreateShape( const ::rtl::OUString& rType );

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >           maXPropSet;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >                  maXShapes;

    sal_uInt32                      nFinalTextCount;

    sal_Bool                        ImplInitPage();
    void                        ImplSetOrientation( FloatPoint& RefPoint, double& Orientation ) ;
    void                        ImplSetLineBundle() ;
    void                        ImplSetFillBundle() ;
    void                        ImplSetTextBundle( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & ) ;
    void                        ImplGetFrameSet( int NodeNumber, NodeFrameSet& );
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

