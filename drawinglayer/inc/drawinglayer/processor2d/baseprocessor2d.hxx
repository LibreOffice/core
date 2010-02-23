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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_BASEPROCESSOR2D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_BASEPROCESSOR2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        class BaseProcessor2D
        {
        private:
            geometry::ViewInformation2D                     maViewInformation2D;

        protected:
            void updateViewInformation(const geometry::ViewInformation2D& rViewInformation2D)
            {
                maViewInformation2D = rViewInformation2D;
            }

            // as tooling, the process() implementation takes over API handling and calls this
            // virtual render method when the primitive implementation is BasePrimitive2D-based.
            // Default implementation does nothing
            virtual void processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate);

        public:
            BaseProcessor2D(const geometry::ViewInformation2D& rViewInformation);
            virtual ~BaseProcessor2D();

            // the central processing method
            virtual void process(const primitive2d::Primitive2DSequence& rSource);

            // data access
            const geometry::ViewInformation2D& getViewInformation2D() const { return maViewInformation2D; }
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PROCESSOR2D_BASEPROCESSOR2D_HXX

// eof
