/*************************************************************************
 *
 *  $RCSfile: implbitmap.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 20:58:48 $
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

#include <implbitmap.hxx>
#include <implbitmapcanvas.hxx>

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif


using namespace ::drafts::com::sun::star;
using namespace ::com::sun::star;

namespace cppcanvas
{

    namespace internal
    {

        ImplBitmap::ImplBitmap( const CanvasSharedPtr&                      rParentCanvas,
                                const uno::Reference< rendering::XBitmap >& rBitmap ) :
            CanvasGraphicHelper( rParentCanvas ),
            mxBitmap( rBitmap ),
            mpBitmapCanvas(
                new ImplBitmapCanvas( uno::Reference< rendering::XBitmapCanvas >(rBitmap,
                                                                                 uno::UNO_QUERY) ) )
        {
            OSL_ENSURE( mxBitmap.is(), "ImplBitmap::ImplBitmap: no valid bitmap" );
        }

        ImplBitmap::~ImplBitmap()
        {
        }

        bool ImplBitmap::draw() const
        {
            CanvasSharedPtr pCanvas( getCanvas() );

            OSL_ENSURE( pCanvas.get() != NULL &&
                        pCanvas->getUNOCanvas().is(),
                        "ImplBitmap::draw: invalid canvas" );

            if( pCanvas.get() == NULL ||
                !pCanvas->getUNOCanvas().is() )
            {
                return false;
            }

            // TODO(P1): implement caching
            pCanvas->getUNOCanvas()->drawBitmap( mxBitmap,
                                                 pCanvas->getViewState(),
                                                 maRenderState );

            return true;
        }

        bool ImplBitmap::drawAlphaModulated( double nAlphaModulation ) const
        {
            CanvasSharedPtr pCanvas( getCanvas() );

            OSL_ENSURE( pCanvas.get() != NULL &&
                        pCanvas->getUNOCanvas().is(),
                        "ImplBitmap::drawAlphaModulated(): invalid canvas" );

            if( pCanvas.get() == NULL ||
                !pCanvas->getUNOCanvas().is() )
            {
                return false;
            }

            rendering::RenderState aLocalState( maRenderState );
            aLocalState.DeviceColor.realloc( 4 );
            aLocalState.DeviceColor[0] = 1.0;
            aLocalState.DeviceColor[1] = 1.0;
            aLocalState.DeviceColor[2] = 1.0;
            aLocalState.DeviceColor[3] = nAlphaModulation;

            // TODO(P1): implement caching
            pCanvas->getUNOCanvas()->drawBitmapModulated( mxBitmap,
                                                          pCanvas->getViewState(),
                                                          aLocalState );

            return true;
        }

        BitmapCanvasSharedPtr ImplBitmap::getBitmapCanvas() const
        {
            return mpBitmapCanvas;
        }

        uno::Reference< rendering::XBitmap > ImplBitmap::getUNOBitmap() const
        {
            return mxBitmap;
        }
    }
}
