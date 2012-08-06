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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_BASEPROCESSOR2D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_BASEPROCESSOR2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        /** BaseProcessor2D class

            Baseclass for all C++ implementations of instances which process
            primitives.

            Instances which process primitives can be renderers, but also stuff
            for HitTests, BoundRect calculations and/or animation processing. The
            main usage are renderers, but they are supposed to handle any primitive
            processing.

            The base implementation is constructed with a ViewInformation2D which
            is accessible throughout the processor implementations. The idea is
            to construct any processor with a given ViewInformation2D. To be able
            to handle primitives which need to change the current transformation
            (as e.g. TransformPrimitive2D) it is allowed for the processor implementation
            to change tis local value using updateViewInformation.

            The basic processing method is process(..) which gets handed over the
            sequence of primitives to process. For convenience of the C++ implementations,
            the default implementation of process(..) maps all accesses to primitives to
            single calls to processBasePrimitive2D(..) where the primitive in question is
            already casted to the C++ implementation class.

            The process(..) implementation makes a complete iteration over the given
            sequence of primitives. If the Primitive is not derived from BasePrimitive2D
            and thus not part of the C++ implementations, it converts ViewInformation2D
            to the corresponding API implementation (an uno::Sequence< beans::PropertyValue >)
            and recursively calls the method process(..) at the primitive with the decomposition
            derived from that primitive. This is the preparation to handle unknown implementations
            of the com::sun::star::graphic::XPrimitive2D interface in the future.

            So, to implement a basic processor, it is necessary to overload and implement the
            processBasePrimitive2D(..) method. A minimal renderer has to support the
            Basic Primitives (see baseprimitive2d.hxx) and the Grouping Primitives (see
            groupprimitive2d.hxx). These are (currently):

            Basic Primitives:

            - BitmapPrimitive2D (bitmap data, evtl. with transparence)
            - PointArrayPrimitive2D (single points)
            - PolygonHairlinePrimitive2D (hairline curves/polygons)
            - PolyPolygonColorPrimitive2D (colored polygons)

            Grouping Primitives:

            - TransparencePrimitive2D (objects with freely defined transparence)
            - InvertPrimitive2D (for XOR)
            - MaskPrimitive2D (for masking)
            - ModifiedColorPrimitive2D (for a stack of color modifications)
            - TransformPrimitive2D (for a transformation stack)

            A processor doing so is a minimal processor. Of course a processor may
            handle any higher-level prmitive (that has a decomposition implementation)
            for more direct data access or performance reasons, too.

            The main part of a processBasePrimitive2D implementation is a switch..case
            construct, looking like the following:

            void foo::processBasePrimitive2D(const BasePrimitive2D& rCandidate)
            {
                switch(rCandidate.getPrimitive2DID())
                {
                    case PRIMITIVE2D_ID_??? :
                    {
                        // process PRIMITIVE2D_ID_??? here...

                        ...

                        break;
                    }

                    ...

                    default :
                    {
                        // process recursively
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                        break;
                    }
                }
            }

            The default case makes the processor work with all complex primitives
            by recursively using their decomposition.

            You can also add a case for ignoring primitives by using:

                    case PRIMITIVE2D_ID_...IGNORE.A.. :
                    case PRIMITIVE2D_ID_...IGNORE.B.. :
                    case PRIMITIVE2D_ID_...IGNORE.C.. :
                    {
                        // ignore these primitives by neither processing nor
                        // recursively processing their decomposition
                        break;
                    }

            Another useful case is embedding the processing of a complex primitive by
            bracketing it with some actions:

                    case PRIMITIVE2D_ID_SOME_TEXT :
                    {
                        // encapsulate e.g. with changing local varibles, e.g.
                        // sometimes it's good to know if a basic primitive is
                        // part of a text, especially when not handling the text
                        // self but by purpose want to handle the decomposed
                        // geometries in the processor
                        startText();
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                        endText();
                        break;
                    }

            As an example a processor collecting the outlines of a sequence of primitives
            only needs to handle some Basic Primitives and create outline and collect
            outline polygons e.g. for primitives with area like BitmapPrimitive2D (a
            rectangle) and PolyPolygonColorPrimitive2D. When also handling the Grouping
            Primitives MaskPrimitive2D (e.g. ignoring it's content, using the mask polyPolygon)
            and TransformPrimitive2D (to have the correct local transformation), a processor
            creating the outline can be written using just four (4) primitives. As a tipp, it can
            be helpful to add many for the purpose not interesting higher level primitives
            to not force their decomposition to be created and/or parsed.
         */
        class DRAWINGLAYER_DLLPUBLIC BaseProcessor2D
        {
        private:
            /// The ViewInformation2D itself. It's private to isolate accesses to it
            geometry::ViewInformation2D                     maViewInformation2D;

        protected:
            /*  access method to allow the implementations to change the current
                ViewInformation2D if needed. This allows isolating these accesses
                later if needed
             */
            void updateViewInformation(const geometry::ViewInformation2D& rViewInformation2D)
            {
                maViewInformation2D = rViewInformation2D;
            }

            /*  as tooling, the process() implementation takes over API handling and calls this
                virtual render method when the primitive implementation is BasePrimitive2D-based.
                Default implementation does nothing
             */
            virtual void processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate);

        public:
            /// constructor/destructor
            explicit BaseProcessor2D(const geometry::ViewInformation2D& rViewInformation);
            virtual ~BaseProcessor2D();

            /// the central processing method
            virtual void process(const primitive2d::Primitive2DSequence& rSource);

            /// data read access
            const geometry::ViewInformation2D& getViewInformation2D() const { return maViewInformation2D; }
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PROCESSOR2D_BASEPROCESSOR2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
