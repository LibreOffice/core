/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: baseprocessor3d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2006-10-19 10:33:20 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_BASEPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        class BaseProcessor3D
        {
        private:
            double                                              mfTime;

        public:
            BaseProcessor3D(double fTime);
            virtual ~BaseProcessor3D();

            // the central processing method
            virtual void process(const primitive3d::Primitive3DSequence& rSource) = 0;

            // data access
            double getTime() const { return mfTime; }
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
            primitive3d::Primitive3DSequence                        maPrimitiveSequence;

        public:
            CollectingProcessor3D(double fTime);

            // the central processing method
            virtual void process(const primitive3d::Primitive3DSequence& rSource);

            // helpers for adding to local sequence
            void appendPrimitive3DSequence(const primitive3d::Primitive3DSequence& rSource)
            {
                primitive3d::appendPrimitive3DSequenceToPrimitive3DSequence(maPrimitiveSequence, rSource);
            }

            void appendPrimitive3DReference(const primitive3d::Primitive3DReference& rSource)
            {
                primitive3d::appendPrimitive3DReferenceToPrimitive3DSequence(maPrimitiveSequence, rSource);
            }

            // data access
            const primitive3d::Primitive3DSequence& getPrimitive3DSequence() const { return maPrimitiveSequence; }
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_DRAWINGLAYER_PROCESSOR3D_BASEPROCESSOR3D_HXX

// eof
