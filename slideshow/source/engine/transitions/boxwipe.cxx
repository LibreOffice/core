/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: boxwipe.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 08:37:38 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

#include "canvas/debug.hxx"
#include "boxwipe.hxx"
#include "basegfx/matrix/b2dhommatrix.hxx"


namespace presentation {
namespace internal {

::basegfx::B2DPolyPolygon BoxWipe::operator () ( double t )
{
    ::basegfx::B2DHomMatrix aTransform;
    double d = ::basegfx::pruneScaleValue(t);
    if (m_topCentered) {
        aTransform.translate( -0.5, 0.0 );
        aTransform.scale( d, d );
        aTransform.translate( 0.5, 0.0 );
    } else {
        aTransform.scale( d, d );
    }

    ::basegfx::B2DPolyPolygon res( m_unitRect );
    res.transform( aTransform );
    return res;
}

}
}
