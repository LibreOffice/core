/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hatchtextureprimitive3d.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: aw $ $Date: 2008-06-10 09:29:21 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_HATCHTEXTUREPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_HATCHTEXTUREPRIMITIVE3D_HXX

#include <drawinglayer/primitive3d/textureprimitive3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        /** HatchTexturePrimitive3D is derived from GroupPrimitive3D, but implements
            a decomposition which is complicated enough for buffering. Since the group
            primitive has no default buffering, it is necessary here to add a local
            buffering mechanism for the decomposition
         */
        class HatchTexturePrimitive3D : public TexturePrimitive3D
        {
        private:
            attribute::FillHatchAttribute                   maHatch;
            Primitive3DSequence                             maBuffered3DDecomposition;

        protected:
            /// helper: local decomposition
            Primitive3DSequence impCreate3DDecomposition() const;

            /// local access methods to maBufferedDecomposition
            const Primitive3DSequence& getBuffered3DDecomposition() const { return maBuffered3DDecomposition; }
            void setBuffered3DDecomposition(const Primitive3DSequence& rNew) { maBuffered3DDecomposition = rNew; }

        public:
            HatchTexturePrimitive3D(
                const attribute::FillHatchAttribute& rHatch,
                const Primitive3DSequence& rChildren,
                const basegfx::B2DVector& rTextureSize,
                bool bModulate,
                bool bFilter);

            // get data
            const attribute::FillHatchAttribute& getHatch() const { return maHatch; }

            // compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const;

            // local decomposition.
            virtual Primitive3DSequence get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const;

            // provide unique ID
            DeclPrimitrive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_HATCHTEXTUREPRIMITIVE3D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
