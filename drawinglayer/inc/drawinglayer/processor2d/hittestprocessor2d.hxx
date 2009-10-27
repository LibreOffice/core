/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contourextractor2d.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2008-06-24 15:30:17 $
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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_HITTESTPROCESSOR2D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_HITTESTPROCESSOR2D_HXX

#include <drawinglayer/processor2d/baseprocessor2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

namespace basegfx { class B2DPolygon; }
namespace basegfx { class B2DPolyPolygon; }

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        class HitTestProcessor2D : public BaseProcessor2D
        {
        private:
            // discrete HitTest position
            basegfx::B2DPoint           maDiscreteHitPosition;

            // discrete HitTolerance
            double                      mfDiscreteHitTolerance;

            // bitfield
            unsigned                    mbHit : 1;
            unsigned                    mbHitToleranceUsed : 1;

            // this flag decides if primitives of type HitTestPrimitive2D
            // will be taken into account for HitTesting or not. Those primitives
            // are created for objects which are else completely invisible and normally
            // their content exists of hairline primitives describing the object's contour
            unsigned                    mbUseHitTestPrimitiveContent : 1;

            // flag to concentraze on text hits only
            unsigned                    mbHitTextOnly : 1;

            // tooling methods
            void processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate);
            bool checkHairlineHitWithTolerance(
                const basegfx::B2DPolygon& rPolygon,
                double fDiscreteHitTolerance);
            bool checkFillHitWithTolerance(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                double fDiscreteHitTolerance);

        public:
            HitTestProcessor2D(
                const geometry::ViewInformation2D& rViewInformation,
                const basegfx::B2DPoint& rLogicHitPosition,
                double fLogicHitTolerance,
                bool bHitTextOnly);
            virtual ~HitTestProcessor2D();

            // data write access
            void setUseHitTestPrimitiveContent(bool bNew)
            {
                if((bool)mbUseHitTestPrimitiveContent != bNew) mbUseHitTestPrimitiveContent = bNew;
            }

            // data read access
            const basegfx::B2DPoint& getDiscreteHitPosition() const { return maDiscreteHitPosition; }
            double getDiscreteHitTolerance() const { return mfDiscreteHitTolerance; }
            bool getHit() const { return mbHit; }
            bool getHitToleranceUsed() const { return mbHitToleranceUsed; }
            bool getUseHitTestPrimitiveContent() const { return mbUseHitTestPrimitiveContent;}
            bool getHitTextOnly() const { return mbHitTextOnly; }
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_HITTESTPROCESSOR2D_HXX

// eof
