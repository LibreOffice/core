/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: baseprocessor3d.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-08-09 16:45:32 $
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
#define _DRAWINGLAYER_PROCESSOR3D_BASEPROCESSOR3D_HXX

#ifndef _DRAWINGLAYER_GEOMETRY_VIEWINFORMATION_HXX
#include <drawinglayer/geometry/viewinformation.hxx>
#endif

#ifndef _DRAWINGLAYER_GEOMETRY_TRANSFORMATION3D_HXX
#include <drawinglayer/geometry/transformation3d.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_PRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/primitive3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// predefines

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        class baseProcessor3D
        {
        protected:
            // the view information and the complete 3d stack
            const geometry::viewInformation         maViewInformation;
            const geometry::transformation3D        maTransformation3D;

        public:
            baseProcessor3D(
                const geometry::viewInformation& rViewInformation,
                const geometry::transformation3D& rTransformation3D);
            virtual ~baseProcessor3D();

            // the central processing method
            virtual void process(const primitive3d::primitiveVector3D& rSource) = 0;

            // data access
            const geometry::viewInformation& getViewInformation() const { return maViewInformation; }
            const geometry::transformation3D& getTransformation3D() const { return maTransformation3D; }
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        class collectingProcessor3D : public baseProcessor3D
        {
        protected:
            primitive3d::primitiveVector3D      maPrimitiveVector;

        public:
            collectingProcessor3D(
                const geometry::viewInformation& rViewInformation,
                const geometry::transformation3D& rTransformation3D);
            virtual ~collectingProcessor3D();

            // the central processing method
            virtual void process(const primitive3d::primitiveVector3D& rSource);

            // data access
            const primitive3d::primitiveVector3D& getPrimitives() const { return maPrimitiveVector; }
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_DRAWINGLAYER_PROCESSOR3D_BASEPROCESSOR3D_HXX

// eof
