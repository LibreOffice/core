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

#ifndef INCLUDED_BASEGFX_TOOLS_UNOPOLYPOLYGON_HXX
#define INCLUDED_BASEGFX_TOOLS_UNOPOLYPOLYGON_HXX

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase3.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/FillRule.hpp>
#include <com/sun/star/rendering/XLinePolyPolygon2D.hpp>
#include <com/sun/star/rendering/XBezierPolyPolygon2D.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
namespace unotools
{
    typedef ::cppu::WeakComponentImplHelper3<
            ::com::sun::star::rendering::XLinePolyPolygon2D,
            ::com::sun::star::rendering::XBezierPolyPolygon2D,
            ::com::sun::star::lang::XServiceInfo > UnoPolyPolygonBase;

    class BASEGFX_DLLPUBLIC UnoPolyPolygon
        : private cppu::BaseMutex
        , public UnoPolyPolygonBase
    {
    public:
        explicit UnoPolyPolygon( const B2DPolyPolygon& );

        // XPolyPolygon2D
        virtual void SAL_CALL addPolyPolygon( const ::com::sun::star::geometry::RealPoint2D& position, const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >& polyPolygon ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getNumberOfPolygons(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getNumberOfPolygonPoints( ::sal_Int32 polygon ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::rendering::FillRule SAL_CALL getFillRule(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setFillRule( ::com::sun::star::rendering::FillRule fillRule ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isClosed( ::sal_Int32 index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setClosed( ::sal_Int32 index, sal_Bool closedState ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XLinePolyPolygon2D
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealPoint2D > > SAL_CALL getPoints( ::sal_Int32 nPolygonIndex, ::sal_Int32 nNumberOfPolygons, ::sal_Int32 nPointIndex, ::sal_Int32 nNumberOfPoints ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPoints( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealPoint2D > >& points, ::sal_Int32 nPolygonIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::geometry::RealPoint2D SAL_CALL getPoint( ::sal_Int32 nPolygonIndex, ::sal_Int32 nPointIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPoint( const ::com::sun::star::geometry::RealPoint2D& point, ::sal_Int32 nPolygonIndex, ::sal_Int32 nPointIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XBezierPolyPolygon2D
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealBezierSegment2D > > SAL_CALL getBezierSegments( ::sal_Int32 nPolygonIndex, ::sal_Int32 nNumberOfPolygons, ::sal_Int32 nPointIndex, ::sal_Int32 nNumberOfPoints ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setBezierSegments( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealBezierSegment2D > >& points, ::sal_Int32 nPolygonIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::geometry::RealBezierSegment2D SAL_CALL getBezierSegment( ::sal_Int32 nPolygonIndex, ::sal_Int32 nPointIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setBezierSegment( const ::com::sun::star::geometry::RealBezierSegment2D& point, ::sal_Int32 nPolygonIndex, ::sal_Int32 nPointIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()  throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

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
        B2DPolyPolygon getPolyPolygonUnsafe() const
        {
            return maPolyPoly;
        }

        /// Called whenever internal polypolygon gets modified
        virtual void modifying() const {}

    private:
        UnoPolyPolygon(const UnoPolyPolygon&) = delete;
        UnoPolyPolygon& operator=(const UnoPolyPolygon&) = delete;

        B2DPolyPolygon                        maPolyPoly;
        ::com::sun::star::rendering::FillRule meFillRule;
    };
}
}

#endif // INCLUDED_BASEGFX_TOOLS_UNOPOLYPOLYGON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
