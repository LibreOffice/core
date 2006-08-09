/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: baseprocessor3d.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-08-09 16:57:47 $
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

#ifndef _DRAWINGLAYER_PROCESSOR3D_BASEPROCESSOR3D_HXX
#include <drawinglayer/processor3d/baseprocessor3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        baseProcessor3D::baseProcessor3D(
            const geometry::viewInformation& rViewInformation,
            const geometry::transformation3D& rTransformation3D)
        :   maViewInformation(rViewInformation),
            maTransformation3D(rTransformation3D)
        {
        }

        baseProcessor3D::~baseProcessor3D()
        {
        }
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        collectingProcessor3D::collectingProcessor3D(
            const geometry::viewInformation& rViewInformation,
            const geometry::transformation3D& rTransformation3D)
        :   baseProcessor3D(rViewInformation, rTransformation3D)
        {
        }

        collectingProcessor3D::~collectingProcessor3D()
        {
        }

        void collectingProcessor3D::process(const primitive3d::primitiveVector3D& rSource)
        {
            // accept everything
            maPrimitiveVector.insert(maPrimitiveVector.end(), rSource.begin(), rSource.end());
        }
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
