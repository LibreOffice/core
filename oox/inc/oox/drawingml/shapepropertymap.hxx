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

#ifndef OOX_DRAWINGML_SHAPEPROPERTYMAP_HXX
#define OOX_DRAWINGML_SHAPEPROPERTYMAP_HXX

#include "oox/helper/propertymap.hxx"
#include "oox/dllapi.h"

namespace oox { class ModelObjectHelper; }

namespace oox {
namespace drawingml {

// ============================================================================

/** Enumeration for various properties related to drawing shape formatting.

    This is an abstraction for shape formatting properties that have different
    names in various implementations, e.g. drawing shapes vs. chart objects.
 */
enum ShapePropertyId
{
    SHAPEPROP_LineStyle,
    SHAPEPROP_LineWidth,
    SHAPEPROP_LineColor,
    SHAPEPROP_LineTransparency,
    SHAPEPROP_LineDash,                     /// Explicit line dash or name of a line dash stored in a global container.
    SHAPEPROP_LineJoint,
    SHAPEPROP_LineStart,                    /// Explicit line start marker or name of a line marker stored in a global container.
    SHAPEPROP_LineStartWidth,
    SHAPEPROP_LineStartCenter,
    SHAPEPROP_LineEnd,                      /// Explicit line end marker or name of a line marker stored in a global container.
    SHAPEPROP_LineEndWidth,
    SHAPEPROP_LineEndCenter,
    SHAPEPROP_FillStyle,
    SHAPEPROP_FillColor,
    SHAPEPROP_FillTransparency,
    SHAPEPROP_GradientTransparency,
    SHAPEPROP_FillGradient,                 /// Explicit fill gradient or name of a fill gradient stored in a global container.
    SHAPEPROP_FillBitmapUrl,                /// Explicit fill bitmap URL or name of a fill bitmap URL stored in a global container.
    SHAPEPROP_FillBitmapMode,
    SHAPEPROP_FillBitmapSizeX,
    SHAPEPROP_FillBitmapSizeY,
    SHAPEPROP_FillBitmapOffsetX,
    SHAPEPROP_FillBitmapOffsetY,
    SHAPEPROP_FillBitmapRectanglePoint,
    SHAPEPROP_ShadowXDistance,
    SHAPEPROP_END
};

// ============================================================================

struct OOX_DLLPUBLIC ShapePropertyInfo
{
    const sal_Int32*    mpnPropertyIds;         /// Pointer to array of property identifiers for all SHAPEPROP properties.
    bool                mbNamedLineMarker;      /// True = use named line marker instead of explicit line marker.
    bool                mbNamedLineDash;        /// True = use named line dash instead of explicit line dash.
    bool                mbNamedFillGradient;    /// True = use named fill gradient instead of explicit fill gradient.
    bool                mbNamedFillBitmapUrl;   /// True = use named fill bitmap URL instead of explicit fill bitmap URL.

    static ShapePropertyInfo DEFAULT;           /// Default property info (used as default parameter of other methods).

    explicit            ShapePropertyInfo(
                            const sal_Int32* pnPropertyIds,
                            bool bNamedLineMarker,
                            bool bNamedLineDash,
                            bool bNamedFillGradient,
                            bool bNamedFillBitmapUrl );

    inline bool         has( ShapePropertyId ePropId ) const { return mpnPropertyIds[ ePropId ] >= 0; }
    inline sal_Int32    operator[]( ShapePropertyId ePropId ) const { return mpnPropertyIds[ ePropId ]; }
};

// ============================================================================

class OOX_DLLPUBLIC ShapePropertyMap : public PropertyMap
{
public:
    explicit            ShapePropertyMap(
                            ModelObjectHelper& rModelObjHelper,
                            const ShapePropertyInfo& rShapePropInfo = ShapePropertyInfo::DEFAULT );

    /** Returns true, if the specified property is supported. */
    bool                supportsProperty( ShapePropertyId ePropId ) const;

    /** Returns true, if named line markers are supported, and the specified
        line marker has already been inserted into the marker table. */
    bool                hasNamedLineMarkerInTable( const ::rtl::OUString& rMarkerName ) const;

    /** Sets the specified shape property to the passed value. */
    bool                setAnyProperty( ShapePropertyId ePropId, const ::com::sun::star::uno::Any& rValue );

    /** Sets the specified shape property to the passed value. */
    template< typename Type >
    inline bool         setProperty( ShapePropertyId ePropId, const Type& rValue )
                            { return setAnyProperty( ePropId, ::com::sun::star::uno::Any( rValue ) ); }

    using PropertyMap::setAnyProperty;
    using PropertyMap::setProperty;
    using PropertyMap::operator[];

private:
    /** Sets an explicit line marker, or creates a named line marker. */
    bool                setLineMarker( sal_Int32 nPropId, const ::com::sun::star::uno::Any& rValue );
    /** Sets an explicit line dash, or creates a named line dash. */
    bool                setLineDash( sal_Int32 nPropId, const ::com::sun::star::uno::Any& rValue );
    /** Sets an explicit fill gradient, or creates a named fill gradient. */
    bool                setFillGradient( sal_Int32 nPropId, const ::com::sun::star::uno::Any& rValue );
    /** Creates a named transparency gradient. */
    bool                setGradientTrans( sal_Int32 nPropId, const ::com::sun::star::uno::Any& rValue );
    /** Sets an explicit fill bitmap URL, or creates a named fill bitmap URL. */
    bool                setFillBitmapUrl( sal_Int32 nPropId, const ::com::sun::star::uno::Any& rValue );

    // not implemented, to prevent implicit conversion from enum to int
    ::com::sun::star::uno::Any& operator[]( ShapePropertyId ePropId );
    const ::com::sun::star::uno::Any& operator[]( ShapePropertyId ePropId ) const;

private:
    ModelObjectHelper&  mrModelObjHelper;
    ShapePropertyInfo   maShapePropInfo;
};

// ============================================================================

} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
