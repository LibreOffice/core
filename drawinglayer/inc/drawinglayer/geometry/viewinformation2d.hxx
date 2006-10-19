/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewinformation2d.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-10-19 10:31:22 $
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

#ifndef INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX
#define INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace {
    class ImpViewInformation2D;
}}

namespace basegfx {
    class B2DHomMatrix;
    class B2DRange;
}

namespace com { namespace sun { namespace star { namespace graphic {
    struct Primitive2DParameters;
}}}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace geometry
    {
        class ViewInformation2D
        {
        private:
            ImpViewInformation2D*                   mpViewInformation2D;

        public:
            // constructors/destructor
            ViewInformation2D(
                const ::basegfx::B2DHomMatrix& rViewTransformation,
                const ::basegfx::B2DRange& rViewport,
                double fViewTime);
            ViewInformation2D(
                const ::com::sun::star::graphic::Primitive2DParameters& rParams);
            ViewInformation2D(
                const ViewInformation2D& rCandidate);
            ~ViewInformation2D();

            // assignment operator
            ViewInformation2D& operator=(const ViewInformation2D& rCandidate);

            // compare operator
            bool operator==(const ViewInformation2D& rCandidate) const;
            bool operator!=(const ViewInformation2D& rCandidate) const { return !operator==(rCandidate); }

            // data access
            const ::basegfx::B2DHomMatrix& getViewTransformation() const;
            const ::basegfx::B2DRange& getViewport() const;
            double getViewTime() const;

            // data access with on-demand preparations
            const ::basegfx::B2DHomMatrix& getInverseViewTransformation() const;
            const ::basegfx::B2DRange& getDiscreteViewport() const;
        };
    } // end of namespace geometry
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX

// eof
