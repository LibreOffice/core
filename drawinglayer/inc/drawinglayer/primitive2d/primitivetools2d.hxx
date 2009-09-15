/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: baseprimitive2d.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:16 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVE2DTOOLS_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVE2DTOOLS_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// tooling class for BufDecPrimitive2D baseed classes which are view-dependent
// regarding the size of a discrete unit. The implementation of get2DDecomposition
// guards the buffered local decomposition and ensures that a createLocal2DDecomposition
// implementation may use an up-to-date DiscreteUnit accessible using getDiscreteUnit()

namespace drawinglayer
{
    namespace primitive2d
    {
        class DiscreteMetricDependentPrimitive2D : public BufDecPrimitive2D
        {
        private:
            // the last used fDiscreteUnit definitions for decomposition. Since this
            // is checked and updated from get2DDecomposition() it will be current and
            // usable in createLocal2DDecomposition()
            double                                  mfDiscreteUnit;

        public:
            DiscreteMetricDependentPrimitive2D()
            :   BufDecPrimitive2D(),
                mfDiscreteUnit(0.0)
            {
            }

            // data access
            double getDiscreteUnit() const { return mfDiscreteUnit; }

            // get local decomposition. Overloaded since this decomposition is view-dependent
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// tooling class for BufDecPrimitive2D baseed classes which are view-dependent
// regarding the viewport. The implementation of get2DDecomposition
// guards the buffered local decomposition and ensures that a createLocal2DDecomposition
// implementation may use an up-to-date Viewport accessible using getViewport()

namespace drawinglayer
{
    namespace primitive2d
    {
        class ViewportDependentPrimitive2D : public BufDecPrimitive2D
        {
        private:
            // the last used Viewport definition for decomposition. Since this
            // is checked and updated from get2DDecomposition() it will be current and
            // usable in createLocal2DDecomposition()
            basegfx::B2DRange                       maViewport;

        public:
            ViewportDependentPrimitive2D()
            :   BufDecPrimitive2D(),
                maViewport()
            {
            }

            // data access
            const basegfx::B2DRange& getViewport() const { return maViewport; }

            // get local decomposition. Overloaded since this decomposition is view-dependent
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVE2DTOOLS_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
