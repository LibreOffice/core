/*************************************************************************
 *
 *  $RCSfile: vclunohelper.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:08 $
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

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#define _TOOLKIT_HELPER_VCLUNOHELPER_HXX_

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
#include <vcl/poly.hxx>

class Window;
class OutputDevice;

//  ----------------------------------------------------
//  class VclUnoHelper
//  ----------------------------------------------------
class VCLUnoHelper
{
public:
    // Toolkit
    static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit>   CreateToolkit();

    // Bitmap
    static BitmapEx                                                         GetBitmap( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap>& rxBitmap );
    static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap>    CreateBitmap( const BitmapEx& rBitmap );

    // Window
    static Window*                                                          GetWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>& rxWindow );
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

};


#endif  // _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
