/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: baseprimitive2d.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: aw $ $Date: 2008-02-07 13:41:57 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BASEPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BASEPRIMITIVE2D_HXX

#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif

#include <boost/utility.hpp>

#ifndef _COM_SUN_STAR_GRAPHIC_XPRIMITIVE2D_HPP_
#include <com/sun/star/graphic/XPrimitive2D.hpp>
#endif

#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// defines for DeclPrimitrive2DIDBlock and ImplPrimitrive2DIDBlock
// Added to be able to simply change identification stuff later, e.g. add
// a identification string and/or ID to the interface and to the implementation
// ATM used to delclare implement getPrimitiveID()

#define DeclPrimitrive2DIDBlock() \
    virtual sal_uInt32 getPrimitiveID() const;

#define ImplPrimitrive2DIDBlock(TheClass, TheID) \
    sal_uInt32 TheClass::getPrimitiveID() const { return TheID; }

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace geometry {
    class ViewInformation2D;
}}

//////////////////////////////////////////////////////////////////////////////
// basePrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        // typedefs for basePrimitive2DImplBase, Primitive2DSequence and Primitive2DReference
        typedef cppu::WeakComponentImplHelper1< ::com::sun::star::graphic::XPrimitive2D > BasePrimitive2DImplBase;
        typedef ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XPrimitive2D > Primitive2DReference;
        typedef ::com::sun::star::uno::Sequence< Primitive2DReference > Primitive2DSequence;

        // base class for all C++ implementations of com::sun::star::graphic::XPrimitive2D. This class
        // is strongly virtual due to the lack of getPrimitiveID() implementation. This is by purpose, this
        // base class shall not be incarnated and be used directly as a XPrimitive2D.
        class BasePrimitive2D
        :   private boost::noncopyable,
            protected comphelper::OBaseMutex,
            public BasePrimitive2DImplBase
        {
        private:
            // a sequence used for buffering the last createLocalDecomposition() result. Use
            // the protected access methods to change. Only implementations of getDecomposition()
            // should make use.
            Primitive2DSequence                             maLocalDecomposition;

        protected:
            // access methods to maLocalDecomposition. The usage of this methods may allow
            // later thread-safe stuff to be added if needed. Only to be used by getDecomposition()
            // implementations for buffering the last decomposition.
            const Primitive2DSequence& getLocalDecomposition() const { return maLocalDecomposition; }
            void setLocalDecomposition(const Primitive2DSequence& rNew) { maLocalDecomposition = rNew; }

            // method which is to be used to implement the local decomposition of a 2D primitive. The default
            // implementation will just return an empty decomposition
            virtual Primitive2DSequence createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            // constructor
            BasePrimitive2D();

            // the ==operator is mainly needed to allow testing newly-created primitives against their last
            // incarnation which buffers/holds the made decompositions. The default implementation
            // uses getPrimitiveID()-calls to test if it's the same ID at last. Overloaded implementation are then
            // based on this implementation
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;
            bool operator!=( const BasePrimitive2D& rPrimitive ) const { return !operator==(rPrimitive); }

            // The default implementation will use getDecomposition results to create the range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            // provide unique ID for fast identifying of known primitive implementations in renderers. These use
            // the the defines from primitivetypes2d.hxx to define unique IDs.
            // This method is normally defined using DeclPrimitrive2DIDBlock()
            virtual sal_uInt32 getPrimitiveID() const = 0;

            // The getDecomposition default implementation will on demand use createLocalDecomposition() if maLocalDecomposition is empty.
            // It will set maLocalDecomposition to this obtained decomposition to buffer it.
            // If the decomposition is also ViewInformation2D-dependent, this method needs to be overloaded and the
            // ViewInformation2D for the last decomposition need to be remembered, too, and be used in the next call to decide if
            // the buffered decomposition may be reused or not.
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

            //
            // Methods from XPrimitive2D
            //

            // The getDecomposition implementation for UNO API will use getDecomposition from this implementation. It
            // will construct a ViewInformation2D from the ViewParameters for that purpose
            virtual Primitive2DSequence SAL_CALL getDecomposition( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rViewParameters ) throw ( ::com::sun::star::uno::RuntimeException );

            // The getRange implementation for UNO API will use getRange from this implementation. It
            // will construct a ViewInformation2D from the ViewParameters for that purpose
            virtual ::com::sun::star::geometry::RealRectangle2D SAL_CALL getRange( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rViewParameters ) throw ( ::com::sun::star::uno::RuntimeException );
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// tooling

namespace drawinglayer
{
    namespace primitive2d
    {
        // get B2DRange from a given Primitive2DReference
        basegfx::B2DRange getB2DRangeFromPrimitive2DReference(const Primitive2DReference& rCandidate, const geometry::ViewInformation2D& aViewInformation);

        // get B2DRange from a given Primitive2DSequence
        basegfx::B2DRange getB2DRangeFromPrimitive2DSequence(const Primitive2DSequence& rCandidate, const geometry::ViewInformation2D& aViewInformation);

        // compare two Primitive2DReferences for equality, including trying to get implementations (BasePrimitive2D)
        // and using compare operator
        bool arePrimitive2DReferencesEqual(const Primitive2DReference& rA, const Primitive2DReference& rB);

        // compare two Primitive2DReferences for equality, uses arePrimitive2DReferencesEqual internally
        bool arePrimitive2DSequencesEqual(const Primitive2DSequence& rA, const Primitive2DSequence& rB);

        // concatenate sequence
        void appendPrimitive2DSequenceToPrimitive2DSequence(Primitive2DSequence& rDest, const Primitive2DSequence& rSource);

        // concatenate single Primitive2D
        void appendPrimitive2DReferenceToPrimitive2DSequence(Primitive2DSequence& rDest, const Primitive2DReference& rSource);

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BASEPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
