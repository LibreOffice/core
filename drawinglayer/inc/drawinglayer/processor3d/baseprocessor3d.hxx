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

#include <drawinglayer/drawinglayerdllapi.h>

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
        class DRAWINGLAYER_DLLPUBLIC BaseProcessor3D
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
            explicit BaseProcessor3D(const geometry::ViewInformation3D& rViewInformation);
            virtual ~BaseProcessor3D();

            // the central processing method
            virtual void process(const primitive3d::Primitive3DSequence& rSource);

            // data access
            const geometry::ViewInformation3D& getViewInformation3D() const { return maViewInformation3D; }
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_DRAWINGLAYER_PROCESSOR3D_BASEPROCESSOR3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
