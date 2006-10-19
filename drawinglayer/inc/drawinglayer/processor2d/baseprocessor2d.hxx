/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: baseprocessor2d.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-10-19 10:28:25 $
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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_BASEPROCESSOR2D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_BASEPROCESSOR2D_HXX

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BASEPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX
#include <drawinglayer/geometry/viewinformation2d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        class BaseProcessor2D
        {
        protected:
            geometry::ViewInformation2D                     maViewInformation2D;

        public:
            BaseProcessor2D(const geometry::ViewInformation2D& rViewInformation);
            virtual ~BaseProcessor2D();

            // the central processing method
            virtual void process(const primitive2d::Primitive2DSequence& rSource) = 0;

            // data access
            const geometry::ViewInformation2D& getViewInformation2D() const { return maViewInformation2D; }
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        class CollectingProcessor2D : public BaseProcessor2D
        {
        private:
            primitive2d::Primitive2DSequence                maPrimitiveSequence;

        public:
            CollectingProcessor2D(const geometry::ViewInformation2D& rViewInformation);

            // the central processing method
            virtual void process(const primitive2d::Primitive2DSequence& rSource);

            // helpers for adding to local sequence
            void appendPrimitive2DSequence(const primitive2d::Primitive2DSequence& rSource)
            {
                primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(maPrimitiveSequence, rSource);
            }

            void appendPrimitive2DReference(const primitive2d::Primitive2DReference& rSource)
            {
                primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(maPrimitiveSequence, rSource);
            }

            // data access
            const primitive2d::Primitive2DSequence& getPrimitive2DSequence() const { return maPrimitiveSequence; }
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PROCESSOR2D_BASEPROCESSOR2D_HXX

// eof
