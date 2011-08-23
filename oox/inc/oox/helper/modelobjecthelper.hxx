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

#ifndef OOX_HELPER_MODELOBJECTHELPER_HXX
#define OOX_HELPER_MODELOBJECTHELPER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include "oox/helper/containerhelper.hxx"

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
    namespace awt { struct Gradient; }
    namespace drawing { struct LineDash; }
    namespace drawing { struct PolyPolygonBezierCoords; }
} } }

namespace oox {

// ============================================================================

/** Contains tables for named drawing objects for a document model.

    Contains tables for named line markers, line dashes, fill gradients, and
    fill bitmaps. The class is needed to handle different document models in
    the same filter (e.g. embedded charts) which carry their own drawing object
    tables.
 */
class ModelObjectHelper
{
public:
    explicit            ModelObjectHelper(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxModelFactory );

    /** Returns true, if the model contains a line marker with the passed name. */
    bool                hasLineMarker( const ::rtl::OUString& rMarkerName ) const;

    /** Inserts a new named line marker, overwrites an existing line marker
        with the same name. Returns true, if the marker could be inserted. */
    bool                insertLineMarker(
                            const ::rtl::OUString& rMarkerName,
                            const ::com::sun::star::drawing::PolyPolygonBezierCoords& rMarker );

    /** Inserts a new named line dash, returns the line dash name, based on an
        internal constant name with a new unused index appended. */
    ::rtl::OUString     insertLineDash( const ::com::sun::star::drawing::LineDash& rDash );

    /** Inserts a new named fill gradient, returns the gradient name, based on
        an internal constant name with a new unused index appended. */
    ::rtl::OUString     insertFillGradient( const ::com::sun::star::awt::Gradient& rGradient );

    /** Inserts a new named fill bitmap, returns the bitmap name, based on an
        internal constant name with a new unused index appended. */
    ::rtl::OUString     insertFillBitmap( const ::rtl::OUString& rGraphicUrl );

private:
    ObjectContainer     maMarkerContainer;
    ObjectContainer     maDashContainer;
    ObjectContainer     maGradientContainer;
    ObjectContainer     maBitmapContainer;
    const ::rtl::OUString maDashNameBase;
    const ::rtl::OUString maGradientNameBase;
    const ::rtl::OUString maBitmapNameBase;
};

// ============================================================================

} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
