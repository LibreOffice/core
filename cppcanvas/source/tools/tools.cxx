/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tools.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:24:08 $
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

#include <tools.hxx>


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace tools
    {
        uno::Sequence< double > intSRGBAToDoubleSequence( const uno::Reference< rendering::XGraphicDevice >&    rDevice,
                                                          Color::IntSRGBA                                       aColor  )
        {
            uno::Sequence< double > aRes( 4 );

            aRes[0] = getRed(aColor) / 255.0;
            aRes[1] = getGreen(aColor) / 255.0;
            aRes[2] = getBlue(aColor) / 255.0;
            aRes[3] = getAlpha(aColor) / 255.0;

            return aRes;
        }

        Color::IntSRGBA doubleSequenceToIntSRGBA( const uno::Reference< rendering::XGraphicDevice >&    rDevice,
                                                  const uno::Sequence< double >&                        rColor  )
        {
            return makeColor( static_cast<sal_uInt8>( 255*rColor[0] + .5 ),
                              static_cast<sal_uInt8>( 255*rColor[1] + .5 ),
                              static_cast<sal_uInt8>( 255*rColor[2] + .5 ),
                              static_cast<sal_uInt8>( 255*rColor[3] + .5 ) );
        }
    }
}
