/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclunohelper.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:00:25 $
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

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#define _TOOLKIT_HELPER_VCLUNOHELPER_HXX_

#ifndef TOOLKIT_DLLAPI_H
#include <toolkit/dllapi.h>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

namespace com { namespace sun { namespace star { namespace uno {
    class XInterface;
}}}}

namespace com { namespace sun { namespace star { namespace awt {
    class XBitmap;
    class XWindow;
    class XWindow2;
    class XWindowPeer;
    class XGraphics;
    class XRegion;
    class XDevice;
    class XPointer;
    class XToolkit;
    class XFont;
    class XControlContainer;
    struct SimpleFontMetric;
    struct FontDescriptor;
    struct Rectangle;
}}}}


#include <vcl/bitmapex.hxx>
#include <vcl/region.hxx>
#include <vcl/metric.hxx>
#include <vcl/mapunit.hxx>
#include <tools/poly.hxx>

class Window;
class OutputDevice;

//  ----------------------------------------------------
//  class VclUnoHelper
//  ----------------------------------------------------
class TOOLKIT_DLLPUBLIC VCLUnoHelper
{
public:
    // Toolkit
    static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit>   CreateToolkit();

    // Bitmap
    static BitmapEx                                                         GetBitmap( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap>& rxBitmap );
    static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap>    CreateBitmap( const BitmapEx& rBitmap );

    // Window
    static Window*                                                          GetWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>& rxWindow );
    static Window*                                                          GetWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow2>& rxWindow2 );
    static Window*                                                          GetWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer>& rxWindowPeer );
    static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>    GetInterface( Window* pWindow );

    // OutputDevice
    static OutputDevice*                            GetOutputDevice( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice>& rxDevice );
    static OutputDevice*                            GetOutputDevice( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics>& rxGraphics );

    // Region
    static Region                                   GetRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion );


    // Pointer
    static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPointer> CreatePointer();

    // Polygon
    static Polygon                                  CreatePolygon( const ::com::sun::star::uno::Sequence< sal_Int32 >& DataX, const ::com::sun::star::uno::Sequence< sal_Int32 >& DataY );

    // Font
    static ::com::sun::star::awt::FontDescriptor    CreateFontDescriptor( const Font& rFont );
    static Font                                     CreateFont( const ::com::sun::star::awt::FontDescriptor& rDescr, const Font& rInitFont );
    static Font                                     CreateFont( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont >& rxFont );
    static ::com::sun::star::awt::SimpleFontMetric  CreateFontMetric( const FontMetric& rFontMetric );
    static float                                    ConvertFontWidth( FontWidth eWidth );
    static FontWidth                                ConvertFontWidth( float f );
    static float                                    ConvertFontWeight( FontWeight eWeight );
    static FontWeight                               ConvertFontWeight( float f );

    // Rectangle
    static sal_Bool                                 IsZero( ::com::sun::star::awt::Rectangle rRect );

    static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer>  CreateControlContainer( Window* pWindow );

    // MapUnits
    static MapUnit                                  UnoEmbed2VCLMapUnit( sal_Int32 nUnoEmbedMapUnit );
    static sal_Int32                                VCL2UnoEmbedMapUnit( MapUnit nVCLMapUnit );

};


#endif  // _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
