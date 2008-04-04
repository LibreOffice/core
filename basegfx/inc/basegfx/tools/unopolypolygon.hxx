/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unopolypolygon.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 16:03:13 $
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

#ifndef INCLUDED_BASEGFX_UNOPOLYPOLYGON_HXX
#define INCLUDED_BASEGFX_UNOPOLYPOLYGON_HXX

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase3.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/FillRule.hpp>
#include <com/sun/star/rendering/XLinePolyPolygon2D.hpp>
#include <com/sun/star/rendering/XBezierPolyPolygon2D.hpp>

#include <basegfx/polygon/b2dpolypolygon.hxx>


namespace basegfx
{
namespace unotools
{
    typedef ::cppu::WeakComponentImplHelper3<
             ::com::sun::star::rendering::XLinePolyPolygon2D,
          ::com::sun::star::rendering::XBezierPolyPolygon2D,
          ::com::sun::star::lang::XServiceInfo > UnoPolyPolygonBase;

    class UnoPolyPolygon : private cppu::BaseMutex,
                           public UnoPolyPolygonBase
    {
    public:
        explicit UnoPolyPolygon( const B2DPolyPolygon& );

        // XPolyPolygon2D
        virtual void SAL_CALL addPolyPolygon( const ::com::sun::star::geometry::RealPoint2D& position, const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >& polyPolygon ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getNumberOfPolygons(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getNumberOfPolygonPoints( ::sal_Int32 polygon ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::rendering::FillRule SAL_CALL getFillRule(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setFillRule( ::com::sun::star::rendering::FillRule fillRule ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isClosed( ::sal_Int32 index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setClosed( ::sal_Int32 index, ::sal_Bool closedState ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

        // XLinePolyPolygon2D
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealPoint2D > > SAL_CALL getPoints( ::sal_Int32 nPolygonIndex, ::sal_Int32 nNumberOfPolygons, ::sal_Int32 nPointIndex, ::sal_Int32 nNumberOfPoints ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPoints( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealPoint2D > >& points, ::sal_Int32 nPolygonIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::geometry::RealPoint2D SAL_CALL getPoint( ::sal_Int32 nPolygonIndex, ::sal_Int32 nPointIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPoint( const ::com::sun::star::geometry::RealPoint2D& point, ::sal_Int32 nPolygonIndex, ::sal_Int32 nPointIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

        // XBezierPolyPolygon2D
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealBezierSegment2D > > SAL_CALL getBezierSegments( ::sal_Int32 nPolygonIndex, ::sal_Int32 nNumberOfPolygons, ::sal_Int32 nPointIndex, ::sal_Int32 nNumberOfPoints ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setBezierSegments( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealBezierSegment2D > >& points, ::sal_Int32 nPolygonIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::geometry::RealBezierSegment2D SAL_CALL getBezierSegment( ::sal_Int32 nPolygonIndex, ::sal_Int32 nPointIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setBezierSegment( const ::com::sun::star::geometry::RealBezierSegment2D& point, ::sal_Int32 nPolygonIndex, ::sal_Int32 nPointIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()  throw( ::com::sun::star::uno::RuntimeException );

        B2DPolyPolygon getPolyPolygon() const;

    protected:
        /// Check whether index is a valid polygon index
        void checkIndex( sal_Int32 nIndex ) const // throw (::com::sun::star::lang::IndexOutOfBoundsException);
        {
            if( nIndex < 0 || nIndex >= static_cast<sal_Int32>(maPolyPoly.count()) )
                throw ::com::sun::star::lang::IndexOutOfBoundsException();
        }

        B2DPolyPolygon getSubsetPolyPolygon( sal_Int32 nPolygonIndex,
                                             sal_Int32 nNumberOfPolygons,
                                             sal_Int32 nPointIndex,
                                             sal_Int32 nNumberOfPoints ) const;

        /// Get cow copy of internal polygon. not thread-safe outside this object.
        B2DPolyPolygon getPolyPolygonUnsafe() const;

        /// Called whenever internal polypolygon gets modified
        virtual void modifying() const {}

    private:
        UnoPolyPolygon( const UnoPolyPolygon& );
        UnoPolyPolygon& operator=( const UnoPolyPolygon& );

        B2DPolyPolygon                        maPolyPoly;
        ::com::sun::star::rendering::FillRule meFillRule;
    };
}
}

#endif /* INCLUDED_BASEGFX_UNOPOLYPOLYGON_HXX */
