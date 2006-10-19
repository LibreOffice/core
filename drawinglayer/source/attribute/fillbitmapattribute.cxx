/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fillbitmapattribute.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2006-10-19 10:36:13 $
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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLBITMAPATTRIBUTE_HXX
#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        FillBitmapAttribute::FillBitmapAttribute(const Bitmap& rBitmap, const basegfx::B2DPoint& rTopLeft, const basegfx::B2DVector& rSize, bool bTiling)
        :   maBitmap(rBitmap),
            maTopLeft(rTopLeft),
            maSize(rSize),
            mbTiling(bTiling)
        {
        }

        bool FillBitmapAttribute::operator==(const FillBitmapAttribute& rCandidate) const
        {
            return (maBitmap == rCandidate.maBitmap
                && maTopLeft == rCandidate.maTopLeft
                && maSize == rCandidate.maSize
                && mbTiling == rCandidate.mbTiling);
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
