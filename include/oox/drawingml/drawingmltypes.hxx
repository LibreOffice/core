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

#ifndef OOX_DRAWINGML_TYPES_HXX
#define OOX_DRAWINGML_TYPES_HXX

#include <boost/shared_ptr.hpp>
#include <com/sun/star/style/TabAlign.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/xml/sax/XFastAttributeList.hpp>
#include "oox/helper/helper.hxx"

namespace oox {
namespace drawingml {

// ============================================================================

const sal_Int32 PER_PERCENT     = 1000;
const sal_Int32 MAX_PERCENT     = 100 * PER_PERCENT;

const sal_Int32 PER_DEGREE      = 60000;
const sal_Int32 MAX_DEGREE      = 360 * PER_DEGREE;

// ============================================================================

struct LineProperties;
typedef ::boost::shared_ptr< LineProperties > LinePropertiesPtr;

struct FillProperties;
typedef ::boost::shared_ptr< FillProperties > FillPropertiesPtr;

struct GraphicProperties;
typedef ::boost::shared_ptr< GraphicProperties > GraphicPropertiesPtr;

struct Shape3DProperties;
typedef ::boost::shared_ptr< Shape3DProperties > Shape3DPropertiesPtr;

struct TextCharacterProperties;
typedef ::boost::shared_ptr< TextCharacterProperties > TextCharacterPropertiesPtr;

struct TextBodyProperties;
typedef ::boost::shared_ptr< TextBodyProperties > TextBodyPropertiesPtr;

struct EffectProperties;
typedef ::boost::shared_ptr< EffectProperties > EffectPropertiesPtr;

class TextBody;
typedef ::boost::shared_ptr< TextBody > TextBodyPtr;

class Shape;
typedef ::boost::shared_ptr< Shape > ShapePtr;

class Theme;
typedef ::boost::shared_ptr< Theme > ThemePtr;

// ---------------------------------------------------------------------------

namespace table {

class TableProperties;
typedef ::boost::shared_ptr< TableProperties > TablePropertiesPtr;

} // namespace table

// ============================================================================

/** converts the attributes from an CT_TLPoint into an awt Point with 1/1000% */
com::sun::star::awt::Point GetPointPercent( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs );


/** converts the attributes from an CT_Size2D into an awt Size with 1/100th mm */
com::sun::star::awt::Size GetSize2D( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes );

/** converts the attributes from a CT_RelativeRect to an IntegerRectangle2D */
com::sun::star::geometry::IntegerRectangle2D GetRelativeRect( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes );

/** converts EMUs into 1/100th mmm */
sal_Int32 GetCoordinate( sal_Int32 nValue );

/** converts an emu string into 1/100th mmm */
sal_Int32 GetCoordinate( const OUString& sValue );

/** converts a ST_Percentage % string into 1/1000th of % */
sal_Int32 GetPercent( const OUString& sValue );

/** Converts a ST_PositiveFixedPercentage to a float. 1.0 == 100% */
double GetPositiveFixedPercentage( const OUString& sValue );

/** converts the ST_TextFontSize to point */
float GetTextSize( const OUString& rValue );

/** converts the ST_TextSpacingPoint to 1/100mm */
sal_Int32 GetTextSpacingPoint(  const OUString& sValue );
sal_Int32 GetTextSpacingPoint(  const sal_Int32 nValue );

/** */
::com::sun::star::style::TabAlign GetTabAlign( ::sal_Int32 aToken );

float GetFontHeight( sal_Int32 nHeight );

sal_Int16 GetFontUnderline( sal_Int32 nToken );

sal_Int16 GetFontStrikeout( sal_Int32 nToken );

sal_Int16 GetCaseMap( sal_Int32 nToken );

/** converts a paragraph align to a ParaAdjust */
sal_Int16 GetParaAdjust( sal_Int32 nAlign );

// ============================================================================

// CT_IndexRange
struct IndexRange {
    sal_Int32 start;
    sal_Int32 end;
};

/** retrieve the content of CT_IndexRange */
IndexRange GetIndexRange( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes );

// ============================================================================

const sal_Int32 EMU_PER_HMM = 360;      /// 360 EMUs per 1/100 mm.

/** Converts the passed 32-bit integer value from 1/100 mm to EMUs. */
inline sal_Int64 convertHmmToEmu( sal_Int32 nValue )
{
    return static_cast< sal_Int64 >( nValue ) * EMU_PER_HMM;
}

/** Converts the passed 64-bit integer value from EMUs to 1/100 mm. */
inline sal_Int32 convertEmuToHmm( sal_Int64 nValue )
{
    sal_Int32 nCorrection = (nValue > 0 ? 1 : -1) * EMU_PER_HMM / 2; // So that the implicit floor will round.
    return getLimitedValue< sal_Int32, sal_Int64 >( (nValue + nCorrection) / EMU_PER_HMM, SAL_MIN_INT32, SAL_MAX_INT32 );
}

// ============================================================================

/** A structure for a point with 64-bit interger components. */
struct EmuPoint
{
    sal_Int64           X;
    sal_Int64           Y;

    inline explicit     EmuPoint() : X( 0 ), Y( 0 ) {}
    inline explicit     EmuPoint( sal_Int64 nX, sal_Int64 nY ) : X( nX ), Y( nY ) {}
};

// ============================================================================

/** A structure for a size with 64-bit interger components. */
struct EmuSize
{
    sal_Int64           Width;
    sal_Int64           Height;

    inline explicit     EmuSize() : Width( 0 ), Height( 0 ) {}
    inline explicit     EmuSize( sal_Int64 nWidth, sal_Int64 nHeight ) : Width( nWidth ), Height( nHeight ) {}
};

// ============================================================================

/** A structure for a rectangle with 64-bit interger components. */
struct EmuRectangle : public EmuPoint, public EmuSize
{
    inline explicit     EmuRectangle() {}
    inline explicit     EmuRectangle( const EmuPoint& rPos, const EmuSize& rSize ) : EmuPoint( rPos ), EmuSize( rSize ) {}
    inline explicit     EmuRectangle( sal_Int64 nX, sal_Int64 nY, sal_Int64 nWidth, sal_Int64 nHeight ) : EmuPoint( nX, nY ), EmuSize( nWidth, nHeight ) {}

    inline void         setPos( const EmuPoint& rPos ) { static_cast< EmuPoint& >( *this ) = rPos; }
    inline void         setSize( const EmuSize& rSize ) { static_cast< EmuSize& >( *this ) = rSize; }
};

// ============================================================================

} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
