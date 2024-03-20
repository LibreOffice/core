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

#pragma once

#include <comphelper/compbase.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/FillRule.hpp>
#include <com/sun/star/rendering/XLinePolyPolygon2D.hpp>
#include <com/sun/star/rendering/XBezierPolyPolygon2D.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/basegfxdllapi.h>
#include <o3tl/safeint.hxx>

namespace basegfx::unotools
{
    typedef comphelper::WeakComponentImplHelper<
            css::rendering::XLinePolyPolygon2D,
            css::rendering::XBezierPolyPolygon2D,
            css::lang::XServiceInfo > UnoPolyPolygonBase;

    class BASEGFX_DLLPUBLIC UnoPolyPolygon
        : public UnoPolyPolygonBase
    {
    public:
        explicit UnoPolyPolygon( B2DPolyPolygon );

        // XPolyPolygon2D
        SAL_DLLPRIVATE virtual void SAL_CALL addPolyPolygon( const css::geometry::RealPoint2D& position, const css::uno::Reference< css::rendering::XPolyPolygon2D >& polyPolygon ) override final;
        SAL_DLLPRIVATE virtual ::sal_Int32 SAL_CALL getNumberOfPolygons(  ) override final;
        SAL_DLLPRIVATE virtual ::sal_Int32 SAL_CALL getNumberOfPolygonPoints( ::sal_Int32 polygon ) override final;
        SAL_DLLPRIVATE virtual css::rendering::FillRule SAL_CALL getFillRule(  ) override final;
        SAL_DLLPRIVATE virtual void SAL_CALL setFillRule( css::rendering::FillRule fillRule ) override final;
        SAL_DLLPRIVATE virtual sal_Bool SAL_CALL isClosed( ::sal_Int32 index ) override final;
        SAL_DLLPRIVATE virtual void SAL_CALL setClosed( ::sal_Int32 index, sal_Bool closedState ) override final;

        // XLinePolyPolygon2D
        SAL_DLLPRIVATE virtual css::uno::Sequence< css::uno::Sequence< css::geometry::RealPoint2D > > SAL_CALL getPoints( ::sal_Int32 nPolygonIndex, ::sal_Int32 nNumberOfPolygons, ::sal_Int32 nPointIndex, ::sal_Int32 nNumberOfPoints ) override final;
        SAL_DLLPRIVATE virtual void SAL_CALL setPoints( const css::uno::Sequence< css::uno::Sequence< css::geometry::RealPoint2D > >& points, ::sal_Int32 nPolygonIndex ) override final;
        SAL_DLLPRIVATE virtual css::geometry::RealPoint2D SAL_CALL getPoint( ::sal_Int32 nPolygonIndex, ::sal_Int32 nPointIndex ) override final;
        SAL_DLLPRIVATE virtual void SAL_CALL setPoint( const css::geometry::RealPoint2D& point, ::sal_Int32 nPolygonIndex, ::sal_Int32 nPointIndex ) override final;

        // XBezierPolyPolygon2D
        SAL_DLLPRIVATE virtual css::uno::Sequence< css::uno::Sequence< css::geometry::RealBezierSegment2D > > SAL_CALL getBezierSegments( ::sal_Int32 nPolygonIndex, ::sal_Int32 nNumberOfPolygons, ::sal_Int32 nPointIndex, ::sal_Int32 nNumberOfPoints ) override final;
        SAL_DLLPRIVATE virtual void SAL_CALL setBezierSegments( const css::uno::Sequence< css::uno::Sequence< css::geometry::RealBezierSegment2D > >& points, ::sal_Int32 nPolygonIndex ) override final;
        SAL_DLLPRIVATE virtual css::geometry::RealBezierSegment2D SAL_CALL getBezierSegment( ::sal_Int32 nPolygonIndex, ::sal_Int32 nPointIndex ) override final;
        SAL_DLLPRIVATE virtual void SAL_CALL setBezierSegment( const css::geometry::RealBezierSegment2D& point, ::sal_Int32 nPolygonIndex, ::sal_Int32 nPointIndex ) override final;

        // XServiceInfo
        SAL_DLLPRIVATE virtual OUString SAL_CALL getImplementationName() override final;
        SAL_DLLPRIVATE virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override final;
        SAL_DLLPRIVATE virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override final;

        SAL_DLLPRIVATE B2DPolyPolygon getPolyPolygon() const;

    protected:
        /// Check whether index is a valid polygon index
        void checkIndex( sal_Int32 nIndex ) const // throw (css::lang::IndexOutOfBoundsException);
        {
            if( nIndex < 0 || o3tl::make_unsigned(nIndex) >= maPolyPoly.count() )
                throw css::lang::IndexOutOfBoundsException();
        }

        SAL_DLLPRIVATE B2DPolyPolygon getSubsetPolyPolygon( sal_Int32 nPolygonIndex,
                                             sal_Int32 nNumberOfPolygons,
                                             sal_Int32 nPointIndex,
                                             sal_Int32 nNumberOfPoints ) const;

        /// Get cow copy of internal polygon. not thread-safe outside this object.
        const B2DPolyPolygon& getPolyPolygonUnsafe() const
        {
            return maPolyPoly;
        }

        /// Called whenever internal polypolygon gets modified
        virtual void modifying() const {}

    private:
        UnoPolyPolygon(const UnoPolyPolygon&) = delete;
        UnoPolyPolygon& operator=(const UnoPolyPolygon&) = delete;

        B2DPolyPolygon                        maPolyPoly;
        css::rendering::FillRule              meFillRule;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
