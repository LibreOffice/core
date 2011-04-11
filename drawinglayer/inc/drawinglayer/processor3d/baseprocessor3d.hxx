/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
        /** BaseProcessor3D class

            Baseclass for all C++ implementations of instances which process
            primitives.

            Please have a look at baseprocessor2d.hxx for more comments.
         */
        class BaseProcessor3D
        {
        private:
            geometry::ViewInformation3D                     maViewInformation3D;

        protected:
            void updateViewInformation(const geometry::ViewInformation3D& rViewInformation3D)
            {
                maViewInformation3D = rViewInformation3D;
            }

            /*  as tooling, the process() implementation takes over API handling and calls this
                virtual render method when the primitive implementation is BasePrimitive3D-based.
                Default implementation does nothing
             */
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
        /** CollectingProcessor3D class

            A processor which just collects all primitives given to it in
            process(..) calls to maPrimitive3DSequence. This can e.g. be used to
            hand around as instance over various methods where every called
            method can add graphic content to it.
         */
        class CollectingProcessor3D : public BaseProcessor3D
        {
        private:
            primitive3d::Primitive3DSequence                        maPrimitive3DSequence;

        public:
            CollectingProcessor3D(const geometry::ViewInformation3D& rViewInformation);
            virtual ~CollectingProcessor3D();

            /// the central processing method
            virtual void process(const primitive3d::Primitive3DSequence& rSource);

            /// helpers for adding to local sequence
            void appendPrimitive3DReference(const primitive3d::Primitive3DReference& rSource)
            {
                primitive3d::appendPrimitive3DReferenceToPrimitive3DSequence(maPrimitive3DSequence, rSource);
            }

            /// data access and reset
            const primitive3d::Primitive3DSequence& getPrimitive3DSequence() const { return maPrimitive3DSequence; }
            void reset() { maPrimitive3DSequence = primitive3d::Primitive3DSequence(); }
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_DRAWINGLAYER_PROCESSOR3D_BASEPROCESSOR3D_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
