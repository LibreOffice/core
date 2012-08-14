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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BASEPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BASEPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <cppuhelper/compbase1.hxx>
#include <boost/utility.hpp>
#include <com/sun/star/graphic/XPrimitive2D.hpp>
#include <comphelper/broadcasthelper.hxx>
#include <basegfx/range/b2drange.hxx>

//////////////////////////////////////////////////////////////////////////////
/** defines for DeclPrimitrive2DIDBlock and ImplPrimitrive2DIDBlock
    Added to be able to simply change identification stuff later, e.g. add
    a identification string and/or ID to the interface and to the implementation
    ATM used to delclare implement getPrimitive2DID()
*/

#define DeclPrimitrive2DIDBlock() \
    virtual sal_uInt32 getPrimitive2DID() const;

#define ImplPrimitrive2DIDBlock(TheClass, TheID) \
    sal_uInt32 TheClass::getPrimitive2DID() const { return TheID; }

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace geometry {
    class ViewInformation2D;
}}

namespace drawinglayer { namespace primitive2d {
    /// typedefs for basePrimitive2DImplBase, Primitive2DSequence and Primitive2DReference
    typedef cppu::WeakComponentImplHelper1< ::com::sun::star::graphic::XPrimitive2D > BasePrimitive2DImplBase;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XPrimitive2D > Primitive2DReference;
    typedef ::com::sun::star::uno::Sequence< Primitive2DReference > Primitive2DSequence;
}}

//////////////////////////////////////////////////////////////////////////////
// basePrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** BasePrimitive2D class

            Baseclass for all C++ implementations of com::sun::star::graphic::XPrimitive2D

            This class is strongly virtual due to the lack of getPrimitiveID() implementation.
            This is by purpose, this base class shall not be incarnated and be used directly as
            a XPrimitive2D.

            Is is derived from boost::noncopyable to make clear that a primitive is a read-only
            instance and copying or changing values is not intended. The idea is to hold all data
            needed for visualisation of this primitive in unchangeable form.

            It is derived from comphelper::OBaseMutex to have a Mutex at hand; in a base
            implementation this may not be needed, but e.g. when buffering at last decomposition
            in a local member, multiple threads may try to decompose at the same time, so locking
            is needed to avoid race conditions seen from the UNO object implementation.

            A method to get a simplified representation is provided by get2DDecomposition. The
            default implementation returns an empty sequence. The idea is that processors
            using this primitive and do not know it, may get the decomposition and process
            these instead. An example is e.g. a fat line, who's decomposition may contain
            the geometric representation of that line using filled polygon prmimitives. When
            the renderer knows how to handle fat lines, he may proccess this primitive directly;
            if not he can use the decomposition. With this functionality, renderers may operate by
            knowing only a small set of primitives.

            When a primitive does not implement get2DDecomposition, it is called a 'Basic Primitive' and
            belongs to the set of primitives which a processor should be able to handle. Practice
            will define this minimal sets of primitives. When defined and the concept is prooved,
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
            which are representated by GroupPrimitive2D derivations, see groupprimitive2d.hxx

            To support getting the geometric BoundRect, getB2DRange is used. The default
            implementation will use the get2DDecomposition result and merge a range from the
            entries. Thus, an implementation is only necessary for the Basic Primitives, but
            of course speedups are possible (and are used) by implementing the method at higher-level
            primitives.

            For primitive identification, getPrimitiveID is used currently in this implementations
            to allow a fast switch/case processing. This needs a unique identifier mechanism which
            currently uses defines (see drawinglayer_primitivetypes2d.hxx). For UNO prmitive API
            it will be needed to add a unique descriptor (Name?) later to the API.

            This base implementation provides mappings from the methods from XPrimitive2D
            (getDecomposition/getRange) to the appropriate methods in the C++ implementations
            (get2DDecomposition/getB2DRange). The PropertyValue ViewParameters is converted to
            the appropriate C++ implementation class ViewInformation2D.

            This base class does not implement any buffering; e.g. buffering the decompositon
            and/or the range. These may be buffered anytime since the definition is that the primitive
            is read-only and thus unchangeable. This implies that the decomposition and/or getting
            the range will lead to the same result as last time, under the precondition that
            the parameter ViewInformation2D is the same as the last one. This is usually the case
            for view-independent primitives which are defined by not using ViewInformation2D
            in their get2DDecomposition/getB2DRange implementations.
         */
        class DRAWINGLAYER_DLLPUBLIC BasePrimitive2D
        :   private boost::noncopyable,
            protected comphelper::OBaseMutex,
            public BasePrimitive2DImplBase
        {
        private:
        protected:
        public:
            // constructor/destructor
            BasePrimitive2D();
            virtual ~BasePrimitive2D();

            /** the ==operator is mainly needed to allow testing newly-created primitives against their last
                incarnation which buffers/holds the made decompositions. The default implementation
                uses getPrimitive2DID()-calls to test if it's the same ID at last. Overloaded implementation are then
                based on this implementation
             */
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;
            bool operator!=( const BasePrimitive2D& rPrimitive ) const { return !operator==(rPrimitive); }

            /// The default implementation will use getDecomposition results to create the range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /** provide unique ID for fast identifying of known primitive implementations in renderers. These use
                the the defines from drawinglayer_primitivetypes2d.hxx to define unique IDs.
                This method is normally defined using DeclPrimitrive2DIDBlock()
             */
            virtual sal_uInt32 getPrimitive2DID() const = 0;

            /// The default implementation will return an empty sequence
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

            //
            // Methods from XPrimitive2D
            //

            /** The getDecomposition implementation for UNO API will use getDecomposition from this implementation. It
                will construct a ViewInformation2D from the ViewParameters for that purpose
             */
            virtual Primitive2DSequence SAL_CALL getDecomposition( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rViewParameters ) throw ( ::com::sun::star::uno::RuntimeException );

            /** The getRange implementation for UNO API will use getRange from this implementation. It
                will construct a ViewInformation2D from the ViewParameters for that purpose
             */
            virtual ::com::sun::star::geometry::RealRectangle2D SAL_CALL getRange( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rViewParameters ) throw ( ::com::sun::star::uno::RuntimeException );
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// BufferedDecompositionPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** BufferedDecompositionPrimitive2D class

            Baseclass for all C++ implementations of com::sun::star::graphic::XPrimitive2D
            which want to buffer the decomoposition result

            Buffering the decomposition is the most-used buffering and is thus used my most
            primitive implementations which support a decomposition as base class.

            The buffering is done by holding the last decomposition in the local parameter
            maBuffered2DDecomposition. The default implementation of get2DDecomposition checks
            if maBuffered2DDecomposition is empty. If yes, it uses create2DDecomposition
            to create the content. In all cases, maBuffered2DDecomposition is returned.

            For view-dependent primitives derived from Primitive2DBufferDecomposition more needs
            to be done when the decomposition depends on parts of the parameter ViewInformation2D.
            This defines a standard method for processing these:

            Implement a view-dependent get2DDecomposition doing te following steps:
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
            Primitive2DSequence                             maBuffered2DDecomposition;

        protected:
            /** access methods to maBuffered2DDecomposition. The usage of this methods may allow
                later thread-safe stuff to be added if needed. Only to be used by getDecomposition()
                implementations for buffering the last decomposition.
             */
            const Primitive2DSequence& getBuffered2DDecomposition() const { return maBuffered2DDecomposition; }
            void setBuffered2DDecomposition(const Primitive2DSequence& rNew) { maBuffered2DDecomposition = rNew; }

            /** method which is to be used to implement the local decomposition of a 2D primitive. The default
                implementation will just return an empty decomposition
             */
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            // constructor/destructor
            BufferedDecompositionPrimitive2D();

            /** The getDecomposition default implementation will on demand use create2DDecomposition() if
                maBuffered2DDecomposition is empty. It will set maBuffered2DDecomposition to this obtained decomposition
                to buffer it. If the decomposition is also ViewInformation2D-dependent, this method needs to be
                overloaded and the ViewInformation2D for the last decomposition need to be remembered, too, and
                be used in the next call to decide if the buffered decomposition may be reused or not.
             */
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// tooling

namespace drawinglayer
{
    namespace primitive2d
    {
        /// get B2DRange from a given Primitive2DReference
        basegfx::B2DRange DRAWINGLAYER_DLLPUBLIC getB2DRangeFromPrimitive2DReference(const Primitive2DReference& rCandidate, const geometry::ViewInformation2D& aViewInformation);

        /// get B2DRange from a given Primitive2DSequence
        basegfx::B2DRange DRAWINGLAYER_DLLPUBLIC getB2DRangeFromPrimitive2DSequence(const Primitive2DSequence& rCandidate, const geometry::ViewInformation2D& aViewInformation);

        /** compare two Primitive2DReferences for equality, including trying to get implementations (BasePrimitive2D)
            and using compare operator
         */
        bool DRAWINGLAYER_DLLPUBLIC arePrimitive2DReferencesEqual(const Primitive2DReference& rA, const Primitive2DReference& rB);

        /// compare two Primitive2DReferences for equality, uses arePrimitive2DReferencesEqual internally
        bool DRAWINGLAYER_DLLPUBLIC arePrimitive2DSequencesEqual(const Primitive2DSequence& rA, const Primitive2DSequence& rB);

        /// concatenate sequence
        void DRAWINGLAYER_DLLPUBLIC appendPrimitive2DSequenceToPrimitive2DSequence(Primitive2DSequence& rDest, const Primitive2DSequence& rSource);

        /// concatenate single Primitive2D
        void DRAWINGLAYER_DLLPUBLIC appendPrimitive2DReferenceToPrimitive2DSequence(Primitive2DSequence& rDest, const Primitive2DReference& rSource);

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BASEPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
