/*************************************************************************
 *
 *  $RCSfile: vclxgraphics.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:07 $
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

#ifndef _TOOLKIT_AWT_VCLXGRAPHICS_HXX_
#define _TOOLKIT_AWT_VCLXGRAPHICS_HXX_


#ifndef _COM_SUN_STAR_AWT_XGRAPHICS_HPP_
#include <com/sun/star/awt/XGraphics.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#include <vcl/font.hxx>
#include <vcl/color.hxx>
#include <vcl/vclenum.hxx>

class OutputDevice;
class Region;


#define INITOUTDEV_FONT         0x0001
#define INITOUTDEV_COLORS       0x0002
#define INITOUTDEV_RASTEROP     0x0004
#define INITOUTDEV_CLIPREGION   0x0008
#define INITOUTDEV_ALL          0xFFFF


//  ----------------------------------------------------
//  class VCLXGraphics
//  ----------------------------------------------------

class VCLXGraphics :    public ::com::sun::star::awt::XGraphics,
                        public ::com::sun::star::lang::XTypeProvider,
                        public ::com::sun::star::lang::XUnoTunnel,
                        public ::cppu::OWeakObject
{
private:
    NAMESPACE_VOS(IMutex)&  mrMutex;    // Reference to SolarMutex
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice> mxDevice; // nur um bei getDevice() immer das gleiche zurueckzugeben

    OutputDevice*   mpOutputDevice;
    Font            maFont;
    Color           maTextColor;
    Color           maTextFillColor;
    Color           maLineColor;
    Color           maFillColor;
    RasterOp        meRasterOp;
    Region*         mpClipRegion;

protected:
    NAMESPACE_VOS(IMutex)&  GetMutex() { return mrMutex; }

public:
                    VCLXGraphics();
                    ~VCLXGraphics();

    void            Init( OutputDevice* pOutDev );
    void            InitOutputDevice( sal_uInt16 nFlags );

    void            SetOutputDevice( OutputDevice* pOutDev );
    OutputDevice*   GetOutputDevice() const { return mpOutputDevice; }

    const Font&     GetFont() const { return maFont; }

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

    // ::com::sun::star::lang::XUnoTunnel
    static const ::com::sun::star::uno::Sequence< sal_Int8 >&   GetUnoTunnelId() throw();
    static VCLXGraphics*                                        GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) throw();
    sal_Int64                                                   SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XGraphics
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice >  SAL_CALL getDevice(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::SimpleFontMetric                             SAL_CALL getFontMetric() throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL setFont( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont >& xNewFont ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL selectFont( const ::com::sun::star::awt::FontDescriptor& aDescription ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL setTextColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL setTextFillColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL setLineColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL setFillColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL setRasterOp( ::com::sun::star::awt::RasterOperation ROP ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL setClipRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& Clipping ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL intersectClipRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& xClipping ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL push(  ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL pop(  ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL copy( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice >& xSource, sal_Int32 nSourceX, sal_Int32 nSourceY, sal_Int32 nSourceWidth, sal_Int32 nSourceHeight, sal_Int32 nDestX, sal_Int32 nDestY, sal_Int32 nDestWidth, sal_Int32 nDestHeight ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL draw( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDisplayBitmap >& xBitmapHandle, sal_Int32 SourceX, sal_Int32 SourceY, sal_Int32 SourceWidth, sal_Int32 SourceHeight, sal_Int32 DestX, sal_Int32 DestY, sal_Int32 DestWidth, sal_Int32 DestHeight ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawPixel( sal_Int32 X, sal_Int32 Y ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawLine( sal_Int32 X1, sal_Int32 Y1, sal_Int32 X2, sal_Int32 Y2 ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawRect( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawRoundedRect( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int32 nHorzRound, sal_Int32 nVertRound ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawPolyLine( const ::com::sun::star::uno::Sequence< sal_Int32 >& DataX, const ::com::sun::star::uno::Sequence< sal_Int32 >& DataY ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawPolygon( const ::com::sun::star::uno::Sequence< sal_Int32 >& DataX, const ::com::sun::star::uno::Sequence< sal_Int32 >& DataY ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawPolyPolygon( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< sal_Int32 > >& DataX, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< sal_Int32 > >& DataY ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawEllipse( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawArc( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int32 X1, sal_Int32 Y1, sal_Int32 X2, sal_Int32 Y2 ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawPie( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int32 X1, sal_Int32 Y1, sal_Int32 X2, sal_Int32 Y2 ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawChord( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int32 nX1, sal_Int32 nY1, sal_Int32 nX2, sal_Int32 nY2 ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawGradient( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 Height, const ::com::sun::star::awt::Gradient& aGradient ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawText( sal_Int32 X, sal_Int32 Y, const ::rtl::OUString& Text ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawTextArray( sal_Int32 X, sal_Int32 Y, const ::rtl::OUString& Text, const ::com::sun::star::uno::Sequence< sal_Int32 >& Longs ) throw(::com::sun::star::uno::RuntimeException);
};




#endif // _TOOLKIT_AWT_VCLXGRAPHICS_HXX_

