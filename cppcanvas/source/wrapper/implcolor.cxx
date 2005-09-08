/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: implcolor.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:27:03 $
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

#include <implcolor.hxx>
#include <tools.hxx>

using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {

        ImplColor::ImplColor( const uno::Reference< rendering::XGraphicDevice >& rDevice ) :
            mxDevice( rDevice )
        {
            OSL_ENSURE( mxDevice.is(), "ImplColor::ImplColor(): Invalid graphic device" );
        }

        ImplColor::~ImplColor()
        {
        }

        Color::IntSRGBA ImplColor::getIntSRGBA( uno::Sequence< double >& rDeviceColor ) const
        {
            OSL_ENSURE( mxDevice.is(), "ImplColor::getIntSRGBA(): Invalid graphic device" );

            // TODO(F1): Color space handling
            return tools::doubleSequenceToIntSRGBA( mxDevice, rDeviceColor );
        }

        uno::Sequence< double > ImplColor::getDeviceColor( Color::IntSRGBA aSRGBA ) const
        {
            OSL_ENSURE( mxDevice.is(), "ImplColor::getDeviceColor(): Invalid graphic device" );

            // TODO(F1): Color space handling
            return tools::intSRGBAToDoubleSequence( mxDevice, aSRGBA );
        }

    }
}
