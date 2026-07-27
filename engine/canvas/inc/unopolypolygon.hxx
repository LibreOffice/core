/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <com/sun/star/rendering/XLinePolyPolygon2D.hpp>
#include <com/sun/star/rendering/XBezierPolyPolygon2D.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <canvas/canvastoolsdllapi.h>
#include <o3tl/safeint.hxx>

namespace com::sun::star::rendering { enum class FillRule; }

namespace canvastools
{
    typedef comphelper::WeakComponentImplHelper<
            css::rendering::XLinePolyPolygon2D,
            css::rendering::XBezierPolyPolygon2D,
            css::lang::XServiceInfo > UnoPolyPolygonBase;

    class CANVASTOOLS_DLLPUBLIC UnoPolyPolygon
        : public UnoPolyPolygonBase
    {
    public:
        explicit UnoPolyPolygon( basegfx::B2DPolyPolygon );
        UnoPolyPolygon( basegfx::B2DPolyPolygon, css::rendering::FillRule fillRule);

        // XPolyPolygon2D
        SAL_DLLPRIVATE virtual ::sal_Int32 getNumberOfPolygons(  ) override final;
        SAL_DLLPRIVATE virtual css::rendering::FillRule getFillRule(  ) override final;

        // XLinePolyPolygon2D
        SAL_DLLPRIVATE virtual cpo::uno::Sequence< cpo::uno::Sequence< css::geometry::RealPoint2D > > getPoints( ::sal_Int32 nPolygonIndex, ::sal_Int32 nNumberOfPolygons, ::sal_Int32 nPointIndex, ::sal_Int32 nNumberOfPoints ) override final;

        // XBezierPolyPolygon2D
        SAL_DLLPRIVATE virtual cpo::uno::Sequence< cpo::uno::Sequence< css::geometry::RealBezierSegment2D > > getBezierSegments( ::sal_Int32 nPolygonIndex, ::sal_Int32 nNumberOfPolygons, ::sal_Int32 nPointIndex, ::sal_Int32 nNumberOfPoints ) override final;

        // XServiceInfo
        SAL_DLLPRIVATE virtual OUString getImplementationName() override final;
        SAL_DLLPRIVATE virtual bool supportsService( const OUString& ServiceName ) override final;
        SAL_DLLPRIVATE virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override final;

        SAL_DLLPRIVATE basegfx::B2DPolyPolygon getPolyPolygon() const;

    protected:
        /// Check whether index is a valid polygon index
        void checkIndex( sal_Int32 nIndex ) const // throw (css::lang::IndexOutOfBoundsException);
        {
            if( nIndex < 0 || o3tl::make_unsigned(nIndex) >= maPolyPoly.count() )
                throw css::lang::IndexOutOfBoundsException();
        }

        SAL_DLLPRIVATE basegfx::B2DPolyPolygon getSubsetPolyPolygon( sal_Int32 nPolygonIndex,
                                             sal_Int32 nNumberOfPolygons,
                                             sal_Int32 nPointIndex,
                                             sal_Int32 nNumberOfPoints ) const;

        /// Get cow copy of internal polygon. not thread-safe outside this object.
        const basegfx::B2DPolyPolygon& getPolyPolygonUnsafe() const
        {
            return maPolyPoly;
        }

    private:
        UnoPolyPolygon(const UnoPolyPolygon&) = delete;
        UnoPolyPolygon& operator=(const UnoPolyPolygon&) = delete;

        basegfx::B2DPolyPolygon               maPolyPoly;
        css::rendering::FillRule              meFillRule;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
