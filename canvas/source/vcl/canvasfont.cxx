/*************************************************************************
 *
 *  $RCSfile: canvasfont.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2004-03-18 10:38:40 $
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

#include "canvasfont.hxx"
#include "spritecanvas.hxx"

#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XSPRITECANVAS_HPP_
#include <drafts/com/sun/star/rendering/XSpriteCanvas.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

namespace vclcanvas
{
    CanvasFont::CanvasFont( const rendering::FontRequest&   rFontRequest,
                            const OutDevProvider::ImplRef&  rCanvas ) :
        maFont( Font( rFontRequest.FamilyName, rFontRequest.StyleName, ::Size( 0,
                                                                               static_cast<long>(rFontRequest.CellSize + .5)) ) ),
        maFontRequest( rFontRequest ),
        mpCanvas( rCanvas )
    {
        maFont->SetAlign( ALIGN_BASELINE );
        maFont->SetHeight( static_cast<long>(rFontRequest.CellSize + .5) );
    }

    CanvasFont::~CanvasFont()
    {
    }

    uno::Sequence< uno::Reference< rendering::XPolyPolygon2D > > SAL_CALL CanvasFont::queryTextShapes( const rendering::StringContext&  text,
                                                                                                       const rendering::ViewState&      viewState,
                                                                                                       const rendering::RenderState&    renderState,
                                                                                                       sal_Int8                         direction ) throw (uno::RuntimeException)
    {
        // TODO
        return uno::Sequence< uno::Reference< rendering::XPolyPolygon2D > >();
    }

    uno::Sequence< geometry::RealRectangle2D > SAL_CALL CanvasFont::queryTightMeasures( const rendering::StringContext& text,
                                                                                     const rendering::ViewState&        viewState,
                                                                                     const rendering::RenderState&      renderState,
                                                                                     sal_Int8                           direction ) throw (uno::RuntimeException)
    {
        // TODO
        return uno::Sequence< geometry::RealRectangle2D >();
    }

    uno::Sequence< geometry::RealRectangle2D > SAL_CALL CanvasFont::queryTextMeasures( const rendering::StringContext&  text,
                                                                                    const rendering::ViewState&         viewState,
                                                                                    const rendering::RenderState&       renderState,
                                                                                    sal_Int8                            direction ) throw (uno::RuntimeException)
    {
        // TODO
        return uno::Sequence< geometry::RealRectangle2D >();
    }

    uno::Sequence< double > SAL_CALL CanvasFont::queryTextOffsets( const rendering::StringContext&  text,
                                                                   const rendering::ViewState&      viewState,
                                                                   const rendering::RenderState&    renderState,
                                                                   sal_Int8                         direction ) throw (uno::RuntimeException)
    {
        // TODO
        return uno::Sequence< double >();
    }

    geometry::RealRectangle2D SAL_CALL CanvasFont::queryTextBounds( const rendering::StringContext& text,
                                                                 const rendering::ViewState&        viewState,
                                                                 const rendering::RenderState&      renderState,
                                                                 sal_Int8                           direction ) throw (uno::RuntimeException)
    {
        // TODO
        return geometry::RealRectangle2D();
    }

    rendering::FontRequest SAL_CALL CanvasFont::getFontRequest(  ) throw (uno::RuntimeException)
    {
        return maFontRequest;
    }

    rendering::FontMetrics SAL_CALL CanvasFont::getFontMetrics(  ) throw (uno::RuntimeException)
    {
        return rendering::FontMetrics();
    }

    uno::Reference< rendering::XCanvas > SAL_CALL CanvasFont::getAssociatedCanvas(  ) throw (uno::RuntimeException)
    {
        return uno::Reference< rendering::XCanvas >( mpCanvas.getRef(),
                                                     uno::UNO_QUERY );
    }

#define SERVICE_NAME "drafts.com.sun.star.rendering.CanvasFont"

    ::rtl::OUString SAL_CALL CanvasFont::getImplementationName() throw( uno::RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( CANVASFONT_IMPLEMENTATION_NAME ) );
    }

    sal_Bool SAL_CALL CanvasFont::supportsService( const ::rtl::OUString& ServiceName ) throw( uno::RuntimeException )
    {
        return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL CanvasFont::getSupportedServiceNames()  throw( uno::RuntimeException )
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }

    ::Font CanvasFont::getVCLFont() const
    {
        return *maFont;
    }
}
