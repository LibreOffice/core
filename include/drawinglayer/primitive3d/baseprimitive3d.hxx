/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_BASEPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_BASEPRIMITIVE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/graphic/XPrimitive3D.hpp>
#include <basegfx/range/b3drange.hxx>
#include <deque>


/** defines for DeclPrimitive3DIDBlock and ImplPrimitive3DIDBlock
    Added to be able to simply change identification stuff later, e.g. add
    a identification string and/or ID to the interface and to the implementation
    ATM used to delclare implement getPrimitive3DID()
 */

#define DeclPrimitive3DIDBlock() \
    virtual sal_uInt32 getPrimitive3DID() const override;

#define ImplPrimitive3DIDBlock(TheClass, TheID) \
    sal_uInt32 TheClass::getPrimitive3DID() const { return TheID; }


// predefines

namespace drawinglayer { namespace geometry {
    class ViewInformation3D;
}}

namespace drawinglayer { namespace primitive3d {
    /// typedefs for basePrimitive3DImplBase, Primitive3DContainer and Primitive3DReference
    typedef cppu::WeakComponentImplHelper< css::graphic::XPrimitive3D > BasePrimitive3DImplBase;
    typedef css::uno::Reference< css::graphic::XPrimitive3D > Primitive3DReference;
    typedef css::uno::Sequence< Primitive3DReference > Primitive3DSequence;

    class SAL_WARN_UNUSED DRAWINGLAYER_DLLPUBLIC Primitive3DContainer : public std::deque< Primitive3DReference >
    {
    public:
        explicit Primitive3DContainer() {}
        explicit Primitive3DContainer( size_type count ) : deque(count) {}
        Primitive3DContainer( const Primitive3DContainer& other ) : deque(other) {}
        Primitive3DContainer( const Primitive3DContainer&& other ) : deque(other) {}
        Primitive3DContainer( std::initializer_list<Primitive3DReference> init ) : deque(init) {}

        void append(const Primitive3DContainer& rSource);
        Primitive3DContainer& operator=(const Primitive3DContainer& r) { deque::operator=(r); return *this; }
        Primitive3DContainer& operator=(const Primitive3DContainer&& r) { deque::operator=(r); return *this; }
        bool operator==(const Primitive3DContainer& rB) const;
        bool operator!=(const Primitive3DContainer& rB) const { return !operator==(rB); }
        basegfx::B3DRange getB3DRange(const geometry::ViewInformation3D& aViewInformation) const;
    };
}}


// basePrimitive3D class

namespace drawinglayer
{
    namespace primitive3d
    {
        /** BasePrimitive3D class

            Baseclass for all C++ implementations of css::graphic::XPrimitive2D

            The description/functionality is identical with the 2D case in baseprimitive2d.hxx,
            please see there for detailed information.

            Current Basic 3D Primitives are:

            - PolygonHairlinePrimitive3D (for 3D hairlines)
            - PolyPolygonMaterialPrimitive3D (for 3D filled plane polygons)

            That's all for 3D!
         */
        class DRAWINGLAYER_DLLPUBLIC BasePrimitive3D
        :   protected cppu::BaseMutex,
            public BasePrimitive3DImplBase
        {
            BasePrimitive3D(const BasePrimitive3D&) = delete;
            BasePrimitive3D& operator=( const BasePrimitive3D& ) = delete;
        public:
            // constructor/destructor
            BasePrimitive3D();
            virtual ~BasePrimitive3D() override;

            /** the ==operator is mainly needed to allow testing newly-created high level primitives against their last
                incarnation which buffers/holds the decompositions. The default implementation
                uses getPrimitive3DID()-calls to test if it's the same ID at last.
                Overridden implementation are then based on this implementation.
             */
            virtual bool operator==( const BasePrimitive3D& rPrimitive ) const;
            bool operator!=( const BasePrimitive3D& rPrimitive ) const { return !operator==(rPrimitive); }

            /** This method is for places where using the C++ implementation directly is possible. The subprocessing
                and range merging is more efficient when working directly on basegfx::B3DRange. The default implementation
                will use getDecomposition results to create the range
             */
            virtual basegfx::B3DRange getB3DRange(const geometry::ViewInformation3D& rViewInformation) const;

            /** provide unique ID for fast identifying of known primitive implementations in renderers. These use
                the defines from primitivetypes3d.hxx to define unique IDs.
             */
            virtual sal_uInt32 getPrimitive3DID() const = 0;

            /// The default implementation returns an empty sequence
            virtual Primitive3DContainer get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const;


            // Methods from XPrimitive3D


            /** The getDecomposition implementation for UNO API will use getDecomposition from this implementation. It
                will get the ViewInformation from the ViewParameters for that purpose
             */
            virtual css::uno::Sequence< ::css::uno::Reference< ::css::graphic::XPrimitive3D > > SAL_CALL getDecomposition( const css::uno::Sequence< css::beans::PropertyValue >& rViewParameters ) override;

            /** the getRange default implementation will use getDecomposition to create the range information from merging
                getRange results from the single local decomposition primitives.
             */
            virtual css::geometry::RealRectangle3D SAL_CALL getRange( const css::uno::Sequence< css::beans::PropertyValue >& rViewParameters ) override;
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer


// BufferedDecompositionPrimitive3D class

namespace drawinglayer
{
    namespace primitive3d
    {
        /** BufferedDecompositionPrimitive3D class

            Baseclass for all C++ implementations of css::graphic::XPrimitive2D

            The description/functionality is identical with the 2D case in baseprimitive2d.hxx,
            please see there for detailed information
         */
        class DRAWINGLAYER_DLLPUBLIC BufferedDecompositionPrimitive3D
        :   public BasePrimitive3D
        {
        private:
            /// a sequence used for buffering the last create3DDecomposition() result
            Primitive3DContainer                             maBuffered3DDecomposition;

        protected:
            /** access methods to maBuffered3DDecomposition. The usage of this methods may allow
                later thread-safe stuff to be added if needed. Only to be used by getDecomposition()
                implementations for buffering the last decomposition.
             */
            const Primitive3DContainer& getBuffered3DDecomposition() const { return maBuffered3DDecomposition; }
            void setBuffered3DDecomposition(const Primitive3DContainer& rNew) { maBuffered3DDecomposition = rNew; }

            /** method which is to be used to implement the local decomposition of a 2D primitive. The default
                implementation will just return an empty decomposition
             */
            virtual Primitive3DContainer create3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const;

        public:
            // constructor
            BufferedDecompositionPrimitive3D();

            /** The getDecomposition default implementation will on demand use create3DDecomposition() if
                maBuffered3DDecomposition is empty. It will set maBuffered3DDecomposition to this obtained decomposition
                to buffer it. If the decomposition is also ViewInformation-dependent, this method needs to be
                overridden and the ViewInformation for the last decomposition needs to be remembered, too, and
                be used in the next call to decide if the buffered decomposition may be reused or not.
             */
            virtual Primitive3DContainer get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const override;
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer


// tooling

namespace drawinglayer
{
    namespace primitive3d
    {
        /// get B3DRange from a given Primitive3DReference
        basegfx::B3DRange DRAWINGLAYER_DLLPUBLIC getB3DRangeFromPrimitive3DReference(const Primitive3DReference& rCandidate, const geometry::ViewInformation3D& aViewInformation);

        /** compare two Primitive2DReferences for equality, including trying to get implementations (BasePrimitive2D)
            and using compare operator
         */
        bool DRAWINGLAYER_DLLPUBLIC arePrimitive3DReferencesEqual(const Primitive3DReference& rA, const Primitive3DReference& rB);

    } // end of namespace primitive3d
} // end of namespace drawinglayer


#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_BASEPRIMITIVE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
