/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: linegeometryextractor2d.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2008-06-24 15:44:27 $
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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_TEXTASPOLYGONEXTRACTOR2D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_TEXTASPOLYGONEXTRACTOR2D_HXX

#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/color/bcolormodifier.hxx>
#include <vector>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        // helper data structure for returning the result
        struct TextAsPolygonDataNode
        {
        private:
            basegfx::B2DPolyPolygon                 maB2DPolyPolygon;
            basegfx::BColor                         maBColor;
            bool                                    mbIsFilled;

        public:
            TextAsPolygonDataNode(
                const basegfx::B2DPolyPolygon& rB2DPolyPolygon,
                const basegfx::BColor& rBColor,
                bool bIsFilled)
            :   maB2DPolyPolygon(rB2DPolyPolygon),
                maBColor(rBColor),
                mbIsFilled(bIsFilled)
            {
            }

            // data read access
            const basegfx::B2DPolyPolygon& getB2DPolyPolygon() const { return maB2DPolyPolygon; }
            const basegfx::BColor& getBColor() const { return maBColor; }
            bool getIsFilled() const { return mbIsFilled; }
        };

        // typedef for a vector of that helper data
        typedef ::std::vector< TextAsPolygonDataNode > TextAsPolygonDataNodeVector;

        class TextAsPolygonExtractor2D : public BaseProcessor2D
        {
        private:
            // extraction target
            TextAsPolygonDataNodeVector             maTarget;

            // the modifiedColorPrimitive stack
            basegfx::BColorModifierStack            maBColorModifierStack;

            // flag if we are in a decomposed text
            sal_uInt32                              mnInText;

            // tooling methods
            void processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate);

        public:
            TextAsPolygonExtractor2D(const geometry::ViewInformation2D& rViewInformation);
            virtual ~TextAsPolygonExtractor2D();

            // data read access
            const TextAsPolygonDataNodeVector& getTarget() const { return maTarget; }
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_TEXTASPOLYGONEXTRACTOR2D_HXX

// eof
