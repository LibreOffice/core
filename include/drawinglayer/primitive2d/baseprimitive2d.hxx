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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BASEPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BASEPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <cppuhelper/compbase.hxx>
#include <com/sun/star/graphic/XPrimitive2D.hpp>
#include <com/sun/star/util/XAccounting.hpp>
#include <cppuhelper/basemutex.hxx>
#include <basegfx/range/b2drange.hxx>
#include <deque>


/** defines for DeclPrimitive2DIDBlock and ImplPrimitive2DIDBlock
    Added to be able to simply change identification stuff later, e.g. add
    a identification string and/or ID to the interface and to the implementation
    ATM used to delclare implement getPrimitive2DID()
*/

#define DeclPrimitive2DIDBlock() \
    virtual sal_uInt32 getPrimitive2DID() const override;

#define ImplPrimitive2DIDBlock(TheClass, TheID) \
    sal_uInt32 TheClass::getPrimitive2DID() const { return TheID; }


// predefines

namespace drawinglayer { namespace geometry {
    class ViewInformation2D;
}}

namespace drawinglayer { namespace primitive2d {
    /// typedefs for basePrimitive2DImplBase, Primitive2DSequence and Primitive2DReference
    typedef cppu::WeakComponentImplHelper<
                css::graphic::XPrimitive2D,
                css::util::XAccounting
            > BasePrimitive2DImplBase;
    typedef css::uno::Reference< css::graphic::XPrimitive2D > Primitive2DReference;
    typedef css::uno::Sequence< Primitive2DReference > Primitive2DSequence;

    class Primitive2DContainer;
    // Visitor class for walking a tree of Primitive2DReference in BasePrimitive2D::get2DDecomposition
    class DRAWINGLAYER_DLLPUBLIC Primitive2DDecompositionVisitor {
    public:
        virtual void append(const Primitive2DReference&) = 0;
        virtual void append(const Primitive2DContainer&) = 0;
        virtual void append(Primitive2DContainer&&) = 0;
        virtual ~Primitive2DDecompositionVisitor();
    };

    class SAL_WARN_UNUSED DRAWINGLAYER_DLLPUBLIC Primitive2DContainer : public std::deque< Primitive2DReference >,
                                                                        public Primitive2DDecompositionVisitor
    {
    public:
        explicit Primitive2DContainer() {}
        explicit Primitive2DContainer( size_type count ) : deque(count) {}
        virtual ~Primitive2DContainer() override;
        Primitive2DContainer( const Primitive2DContainer& other ) : deque(other) {}
        Primitive2DContainer( const Primitive2DContainer&& other ) : deque(other) {}
        Primitive2DContainer( const std::deque< Primitive2DReference >& other ) : deque(other) {}
        Primitive2DContainer( std::initializer_list<Primitive2DReference> init ) : deque(init) {}

        virtual void append(const Primitive2DReference&) override;
        virtual void append(const Primitive2DContainer& rSource) override;
        virtual void append(Primitive2DContainer&& rSource) override;
        void append(const Primitive2DSequence& rSource);
        Primitive2DContainer& operator=(const Primitive2DContainer& r) { deque::operator=(r); return *this; }
        Primitive2DContainer& operator=(const Primitive2DContainer&& r) { deque::operator=(r); return *this; }
        bool operator==(const Primitive2DContainer& rB) const;
        bool operator!=(const Primitive2DContainer& rB) const { return !operator==(rB); }
        basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& aViewInformation) const;
        Primitive2DContainer maybeInvert(bool bInvert = false) const;
    };
}}


// basePrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** BasePrimitive2D class

            Baseclass for all C++ implementations of css::graphic::XPrimitive2D

            This class is strongly virtual due to the lack of getPrimitiveID() implementation.
            This is by purpose, this base class shall not be incarnated and be used directly as
            a XPrimitive2D.

            It is noncopyable to make clear that a primitive is a read-only
            instance and copying or changing values is not intended. The idea is to hold all data
            needed for visualisation of this primitive in unchangeable form.

            It is derived from cppu::BaseMutex to have a Mutex at hand; in a base
            implementation this may not be needed, but e.g. when buffering at last decomposition
            in a local member, multiple threads may try to decompose at the same time, so locking
            is needed to avoid race conditions seen from the UNO object implementation.

            A method to get a simplified representation is provided by get2DDecomposition. The
            default implementation returns an empty sequence. The idea is that processors
            using this primitive and do not know it, may get the decomposition and process
            these instead. An example is e.g. a fat line, who's decomposition may contain
            the geometric representation of that line using filled polygon primitives. When
            the renderer knows how to handle fat lines, he may process this primitive directly;
            if not he can use the decomposition. With this functionality, renderers may operate by
            knowing only a small set of primitives.

            When a primitive does not implement get2DDecomposition, it is called a 'Basic Primitive' and
            belongs to the set of primitives which a processor should be able to handle. Practice
            will define this minimal sets of primitives. When defined and the concept is proved,
            unique UNO APIs may be defined/implemented for these set to allow more intense work
            with primitives using UNO.

            Current Basic 2D Primitives are:

            - BitmapPrimitive2D (bitmap data, evtl. with transparence)
            - PointArrayPrimitive2D (single points)
            - PolygonHairlinePrimitive2D (hairline curves/polygons)
            - PolyPolygonColorPrimitive2D (colored polygons)

            UPDATE: MetafilePrimitive2D (VCL Metafile) is taken off this list since
            it is implemented with the integration of CWS aw078 into DV300m69.

            All other implemented primitives have a defined decomposition and can thus be
            decomposed down to this small set.

            A renderer implementing support for this minimal set of primitives can completely
            render primitive-based visualisations. Of course, he also has to take states into account
            which are represented by GroupPrimitive2D derivations, see groupprimitive2d.hxx

            To support getting the geometric BoundRect, getB2DRange is used. The default
            implementation will use the get2DDecomposition result and merge a range from the
            entries. Thus, an implementation is only necessary for the Basic Primitives, but
            of course speedups are possible (and are used) by implementing the method at higher-level
            primitives.

            For primitive identification, getPrimitiveID is used currently in this implementations
            to allow a fast switch/case processing. This needs a unique identifier mechanism which
            currently uses defines (see drawinglayer_primitivetypes2d.hxx). For UNO primitive API
            it will be needed to add a unique descriptor (Name?) later to the API.

            This base implementation provides mappings from the methods from XPrimitive2D
            (getDecomposition/getRange) to the appropriate methods in the C++ implementations
            (get2DDecomposition/getB2DRange). The PropertyValue ViewParameters is converted to
            the appropriate C++ implementation class ViewInformation2D.

            This base class does not implement any buffering; e.g. buffering the decomposition
            and/or the range. These may be buffered anytime since the definition is that the primitive
            is read-only and thus unchangeable. This implies that the decomposition and/or getting
            the range will lead to the same result as last time, under the precondition that
            the parameter ViewInformation2D is the same as the last one. This is usually the case
            for view-independent primitives which are defined by not using ViewInformation2D
            in their get2DDecomposition/getB2DRange implementations.
         */
        class DRAWINGLAYER_DLLPUBLIC BasePrimitive2D
        :   protected cppu::BaseMutex,
            public BasePrimitive2DImplBase
        {
            BasePrimitive2D(const BasePrimitive2D&) = delete;
            BasePrimitive2D& operator=( const BasePrimitive2D& ) = delete;
        public:
            // constructor/destructor
            BasePrimitive2D();
            virtual ~BasePrimitive2D() override;

            /** the ==operator is mainly needed to allow testing newly-created primitives against their last
                incarnation which buffers/holds the made decompositions. The default implementation
                uses getPrimitive2DID()-calls to test if it's the same ID at last.
                Overridden implementations are then based on this implementation
             */
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;
            bool operator!=( const BasePrimitive2D& rPrimitive ) const { return !operator==(rPrimitive); }

            /// The default implementation will use getDecomposition results to create the range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /** provide unique ID for fast identifying of known primitive implementations in renderers. These use
                the defines from drawinglayer_primitivetypes2d.hxx to define unique IDs.
                This method is normally defined using DeclPrimitive2DIDBlock()
             */
            virtual sal_uInt32 getPrimitive2DID() const = 0;

            /// The default implementation will return an empty sequence
            virtual void get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const;


            // Methods from XPrimitive2D


            /** The getDecomposition implementation for UNO API will use getDecomposition from this implementation. It
                will construct a ViewInformation2D from the ViewParameters for that purpose
             */
            virtual css::uno::Sequence< ::css::uno::Reference< ::css::graphic::XPrimitive2D > > SAL_CALL getDecomposition( const css::uno::Sequence< css::beans::PropertyValue >& rViewParameters ) override;

            /** The getRange implementation for UNO API will use getRange from this implementation. It
                will construct a ViewInformation2D from the ViewParameters for that purpose
             */
            virtual css::geometry::RealRectangle2D SAL_CALL getRange( const css::uno::Sequence< css::beans::PropertyValue >& rViewParameters ) override;

            // XAccounting
            virtual sal_Int64 SAL_CALL estimateUsage() override;

        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


// BufferedDecompositionPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** BufferedDecompositionPrimitive2D class

            Baseclass for all C++ implementations of css::graphic::XPrimitive2D
            which want to buffer the decomposition result

            Buffering the decomposition is the most-used buffering and is thus used my most
            primitive implementations which support a decomposition as base class.

            The buffering is done by holding the last decomposition in the local parameter
            maBuffered2DDecomposition. The default implementation of get2DDecomposition checks
            if maBuffered2DDecomposition is empty. If yes, it uses create2DDecomposition
            to create the content. In all cases, maBuffered2DDecomposition is returned.

            For view-dependent primitives derived from Primitive2DBufferDecomposition more needs
            to be done when the decomposition depends on parts of the parameter ViewInformation2D.
            This defines a standard method for processing these:

            Implement a view-dependent get2DDecomposition doing the following steps:
            (a) Locally extract needed parameters from ViewInformation2D to new, local parameters
                (this may be a complete local copy of ViewInformation2D)
            (b) If a buffered decomposition exists, ckeck if one of the new local parameters
                differs from the corresponding locally remembered (as member) ones. If yes,
                clear maBuffered2DDecomposition
            (d) call baseclass::get2DDecomposition which will use create2DDecomposition
                to fill maBuffered2DDecomposition if it's empty
            (e) copy the new local parameters to the corresponding locally remembered ones
                to identify if a new decomposition is needed at the next call
            (f) return maBuffered2DDecomposition
         */
        class DRAWINGLAYER_DLLPUBLIC BufferedDecompositionPrimitive2D
        :   public BasePrimitive2D
        {
        private:
            /// a sequence used for buffering the last create2DDecomposition() result
            Primitive2DContainer                             maBuffered2DDecomposition;

        protected:
            /** access methods to maBuffered2DDecomposition. The usage of this methods may allow
                later thread-safe stuff to be added if needed. Only to be used by getDecomposition()
                implementations for buffering the last decomposition.
             */
            const Primitive2DContainer& getBuffered2DDecomposition() const { return maBuffered2DDecomposition; }
            void setBuffered2DDecomposition(const Primitive2DContainer& rNew) { maBuffered2DDecomposition = rNew; }

            /** method which is to be used to implement the local decomposition of a 2D primitive. The default
                implementation will just return an empty decomposition
             */
            virtual void create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& rViewInformation) const;

        public:
            // constructor/destructor
            BufferedDecompositionPrimitive2D();

            /** The getDecomposition default implementation will on demand use create2DDecomposition() if
                maBuffered2DDecomposition is empty. It will set maBuffered2DDecomposition to this obtained decomposition
                to buffer it. If the decomposition is also ViewInformation2D-dependent, this method needs to be
                overridden and the ViewInformation2D for the last decomposition need to be remembered, too, and
                be used in the next call to decide if the buffered decomposition may be reused or not.
             */
            virtual void get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const override;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


// tooling

namespace drawinglayer
{
    namespace primitive2d
    {
        /// get B2DRange from a given Primitive2DReference
        basegfx::B2DRange DRAWINGLAYER_DLLPUBLIC getB2DRangeFromPrimitive2DReference(const Primitive2DReference& rCandidate, const geometry::ViewInformation2D& aViewInformation);

        /** compare two Primitive2DReferences for equality, including trying to get implementations (BasePrimitive2D)
            and using compare operator
         */
        bool DRAWINGLAYER_DLLPUBLIC arePrimitive2DReferencesEqual(const Primitive2DReference& rA, const Primitive2DReference& rB);

        OUString DRAWINGLAYER_DLLPUBLIC idToString(sal_uInt32 nId);

    } // end of namespace primitive2d
} // end of namespace drawinglayer


#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BASEPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
