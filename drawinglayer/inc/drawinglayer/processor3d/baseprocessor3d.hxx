/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: baseprocessor3d.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: aw $ $Date: 2008-06-24 15:30:18 $
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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_BASEPROCESSOR3D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR3D_BASEPROCESSOR3D_HXX

#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        class BaseProcessor3D
        {
        private:
            geometry::ViewInformation3D                     maViewInformation3D;

        protected:
            void updateViewInformation(const geometry::ViewInformation3D& rViewInformation3D)
            {
                maViewInformation3D = rViewInformation3D;
            }

            // as tooling, the process() implementation takes over API handling and calls this
            // virtual render method when the primitive implementation is BufDecPrimitive3D-based.
            // Default implementation does nothing
            virtual void processBasePrimitive3D(const primitive3d::BasePrimitive3D& rCandidate);

        public:
            BaseProcessor3D(const geometry::ViewInformation3D& rViewInformation);
            virtual ~BaseProcessor3D();

            // the central processing method
            virtual void process(const primitive3d::Primitive3DSequence& rSource);

            // data access
            const geometry::ViewInformation3D& getViewInformation3D() const { return maViewInformation3D; }
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        class CollectingProcessor3D : public BaseProcessor3D
        {
        private:
            primitive3d::Primitive3DSequence                        maPrimitive3DSequence;

        public:
            CollectingProcessor3D(const geometry::ViewInformation3D& rViewInformation);
            virtual ~CollectingProcessor3D();

            // the central processing method
            virtual void process(const primitive3d::Primitive3DSequence& rSource);

            // helpers for adding to local sequence
            void appendPrimitive3DReference(const primitive3d::Primitive3DReference& rSource)
            {
                primitive3d::appendPrimitive3DReferenceToPrimitive3DSequence(maPrimitive3DSequence, rSource);
            }

            // data access and reset
            const primitive3d::Primitive3DSequence& getPrimitive3DSequence() const { return maPrimitive3DSequence; }
            void reset() { maPrimitive3DSequence = primitive3d::Primitive3DSequence(); }
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_DRAWINGLAYER_PROCESSOR3D_BASEPROCESSOR3D_HXX

// eof
