/*************************************************************************
 *
 *  $RCSfile: canvasbitmap.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 17:10:25 $
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

#include <canvas/debug.hxx>
#include "canvasbitmap.hxx"

#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;


namespace vclcanvas
{
    // Currently, the only way to generate an XBitmap is from
    // XGraphicDevice.getCompatibleBitmap(). Therefore, we don't even
    // take a bitmap here, but a VDev directly.
    CanvasBitmap::CanvasBitmap( const ::Size&                       rSize,
                                bool                                bAlphaBitmap,
                                const WindowGraphicDevice::ImplRef& rDevice )
    {
        ENSURE_AND_THROW( rDevice->getOutDev(),
                          "CanvasBitmap::CanvasBitmap(): Invalid reference outdev" );

        OutputDevice&            rOutDev( *rDevice->getOutDev() );
        tools::OutDevStateKeeper aStateKeeper( rOutDev );

        // create bitmap for given reference device
        // ========================================
        const USHORT nBitCount( (USHORT)24U );
        const BitmapPalette*    pPalette = NULL;

#if 0
        // TODO(P2): Seems like VCL doesn't handle palette bitmap with
        // alpha channel. Devise other ways to handle this, or fix VCL
        const USHORT nBitCount( ::std::min( (USHORT)24U,
                                            (USHORT)rOutDev.GetBitCount() ) );

        if( nBitCount <= 8 )
        {
            rOutDev.EnableMapMode( FALSE );

            // try to determine palette from output device (by
            // extracting a 1,1 bitmap, and querying it)
            const Point aEmptyPoint;
            const Size  aSize(1,1);
            Bitmap aTmpBitmap( rOutDev.GetBitmap( aEmptyPoint,
                                                  aSize ) );

            ScopedBitmapReadAccess pReadAccess( aTmpBitmap.AcquireReadAccess(),
                                                aTmpBitmap );

            pPalette = &pReadAccess->GetPalette();
        }
#endif

        Bitmap aBitmap( rSize, nBitCount, pPalette );

        // only create alpha channel bitmap, if factory requested
        // that. Providing alpha-channeled bitmaps by default has,
        // especially under VCL, a huge performance penalty (have to
        // use alpha VDev, then).
        if( bAlphaBitmap )
        {
            AlphaMask   aAlpha ( rSize );

            maCanvasHelper.setBitmap( BitmapEx( aBitmap, aAlpha ),
                                      rDevice );
        }
        else
        {
            maCanvasHelper.setBitmap( BitmapEx( aBitmap ),
                                      rDevice );
        }
    }

    CanvasBitmap::CanvasBitmap( const BitmapEx&                     rBitmap,
                                const WindowGraphicDevice::ImplRef& rDevice )
    {
        ENSURE_AND_THROW( rDevice->getOutDev(),
                          "CanvasBitmap::CanvasBitmap(): Invalid reference outdev" );

        maCanvasHelper.setBitmap( rBitmap,
                                  rDevice );
    }

    CanvasBitmap::~CanvasBitmap()
    {
    }

    void SAL_CALL CanvasBitmap::disposing()
    {
        tools::LocalGuard aGuard;

        // forward to parent
        CanvasBitmap_Base::disposing();
    }

#define SERVICE_NAME "drafts.com.sun.star.rendering.CanvasBitmap"

    ::rtl::OUString SAL_CALL CanvasBitmap::getImplementationName(  ) throw (uno::RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( CANVASBITMAP_IMPLEMENTATION_NAME ) );
    }

    sal_Bool SAL_CALL CanvasBitmap::supportsService( const ::rtl::OUString& ServiceName ) throw (uno::RuntimeException)
    {
        return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL CanvasBitmap::getSupportedServiceNames(  ) throw (uno::RuntimeException)
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }

    BitmapEx CanvasBitmap::getBitmap() const
    {
        return maCanvasHelper.getBitmap();
    }
}
