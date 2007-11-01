/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dx_vcltools.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 18:00:10 $
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

#ifndef _DXCANVAS_VCLTOOLS_HXX
#define _DXCANVAS_VCLTOOLS_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_TRISTATE_HPP_
#include <com/sun/star/util/TriState.hpp>
#endif

#include <boost/shared_ptr.hpp>


namespace com { namespace sun { namespace star { namespace lang
{
    class XUnoTunnel;
} } } }


namespace dxcanvas
{
    class SurfaceGraphics;

    namespace tools
    {
        /** Raw RGBA bitmap data,
            contiguous in memory
         */
        struct RawRGBABitmap
        {
            sal_Int32                           mnWidth;
            sal_Int32                           mnHeight;
            ::boost::shared_ptr< sal_uInt8 >    mpBitmapData;
        };

        bool drawVCLBitmapFromUnoTunnel( const ::boost::shared_ptr< SurfaceGraphics >&  rGraphics,
                                         const ::com::sun::star::uno::Reference<
                                             ::com::sun::star::lang::XUnoTunnel >&      xTunnel );

        ::com::sun::star::util::TriState isAlphaVCLBitmapFromUnoTunnel( const ::com::sun::star::uno::Reference<
                                                                            ::com::sun::star::lang::XUnoTunnel >& xTunnel );
    }
}

#endif /* _DXCANVAS_VCLTOOLS_HXX */
