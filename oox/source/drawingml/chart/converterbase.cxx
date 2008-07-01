/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: converterbase.cxx,v $
 *
 * $Revision: 1.3 $
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

#include "oox/drawingml/chart/converterbase.hxx"
#include "oox/drawingml/theme.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <comphelper/processfactory.hxx>

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::chart2::XChartDocument;
using ::oox::core::XmlFilterBase;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

namespace {

/** Enumerates different sets of property names for chart object formatting. */
enum PropertyType
{
    PROPERTYTYPE_COMMON,                /// Common objects, no special handling.
    PROPERTYTYPE_LINEARSERIES,          /// Specific to linear data series.
    PROPERTYTYPE_FILLEDSERIES           /// Specific to filled data series.
};

// ============================================================================

enum AutoColorType
{
    AUTOCOLORTYPE_NONE,         /// No color.
    AUTOCOLORTYPE_RGB,          /// RGB color.
    AUTOCOLORTYPE_THEME         /// Theme color.
};

struct AutoFormatEntry
{
    sal_Int32           mnFirstStyleIdx;    /// First chart style index.
    sal_Int32           mnLastStyleIdx;     /// Last chart style index.
    sal_Int32           mnThemeIdx;         /// Themed style index.
    AutoColorType       meColorType;        /// Type of the color value.
    sal_Int32           mnColorValue;       /// Color value (dependent on type).
    sal_Int32           mnModToken;         /// Color modification token.
    sal_Int32           mnModValue;         /// Color modification value.
};

// ----------------------------------------------------------------------------

#define AUTOFORMAT_THEME_COL( first, last, theme_idx, color_token ) \
    { first, last, theme_idx, AUTOCOLORTYPE_THEME, color_token, XML_TOKEN_INVALID, 0 }

#define AUTOFORMAT_THEME_MOD( first, last, theme_idx, color_token, mod_token, mod_value ) \
    { first, last, theme_idx, AUTOCOLORTYPE_THEME, color_token, mod_token, mod_value }

#define AUTOFORMAT_NONE( first, last ) \
    { first, last, -1, AUTOCOLORTYPE_NONE, 0, XML_TOKEN_INVALID, 0 }

#define AUTOFORMAT_END() \
    AUTOFORMAT_NONE( -1, -1 )

static const AutoFormatEntry spNoFormats[] =
{
    AUTOFORMAT_NONE( 1, 48 ),
    AUTOFORMAT_END()
};

static const AutoFormatEntry spChartSpaceLines[] =
{
    AUTOFORMAT_THEME_MOD(  1, 32, THEMED_INDEX_SUBTLE, XML_tx1, XML_tint, 75000 ),
    AUTOFORMAT_THEME_MOD( 33, 40, THEMED_INDEX_SUBTLE, XML_dk1, XML_tint, 75000 ),
    // 41...48: no line, same as Chart2
    AUTOFORMAT_END()
};

static const AutoFormatEntry spChartSpaceFills[] =
{
    AUTOFORMAT_THEME_COL(  1, 32, THEMED_INDEX_SUBTLE, XML_bg1 ),
    AUTOFORMAT_THEME_COL( 33, 40, THEMED_INDEX_SUBTLE, XML_lt1 ),
    AUTOFORMAT_THEME_COL( 41, 48, THEMED_INDEX_SUBTLE, XML_dk1 ),
    AUTOFORMAT_END()
};

static const AutoFormatEntry spPlotArea2dFills[] =
{
    AUTOFORMAT_THEME_COL(  1, 32, THEMED_INDEX_SUBTLE, XML_bg1 ),
    AUTOFORMAT_THEME_MOD( 33, 34, THEMED_INDEX_SUBTLE, XML_dk1, XML_tint, 20000 ),
    AUTOFORMAT_THEME_COL( 35, 35, THEMED_INDEX_SUBTLE, XML_accent1 ),
    AUTOFORMAT_THEME_COL( 36, 36, THEMED_INDEX_SUBTLE, XML_accent2 ),
    AUTOFORMAT_THEME_COL( 37, 37, THEMED_INDEX_SUBTLE, XML_accent3 ),
    AUTOFORMAT_THEME_COL( 38, 38, THEMED_INDEX_SUBTLE, XML_accent4 ),
    AUTOFORMAT_THEME_COL( 39, 39, THEMED_INDEX_SUBTLE, XML_accent5 ),
    AUTOFORMAT_THEME_COL( 40, 40, THEMED_INDEX_SUBTLE, XML_accent6 ),
    AUTOFORMAT_THEME_MOD( 41, 48, THEMED_INDEX_SUBTLE, XML_dk1, XML_tint, 95000 ),
    AUTOFORMAT_END()
};

static const AutoFormatEntry spWallFloorFills[] =
{
    AUTOFORMAT_NONE(       1, 32 ),
    AUTOFORMAT_THEME_MOD( 33, 34, THEMED_INDEX_SUBTLE, XML_dk1, XML_tint, 20000 ),
    AUTOFORMAT_THEME_COL( 35, 35, THEMED_INDEX_SUBTLE, XML_accent1 ),
    AUTOFORMAT_THEME_COL( 36, 36, THEMED_INDEX_SUBTLE, XML_accent2 ),
    AUTOFORMAT_THEME_COL( 37, 37, THEMED_INDEX_SUBTLE, XML_accent3 ),
    AUTOFORMAT_THEME_COL( 38, 38, THEMED_INDEX_SUBTLE, XML_accent4 ),
    AUTOFORMAT_THEME_COL( 39, 39, THEMED_INDEX_SUBTLE, XML_accent5 ),
    AUTOFORMAT_THEME_COL( 40, 40, THEMED_INDEX_SUBTLE, XML_accent6 ),
    AUTOFORMAT_THEME_MOD( 41, 48, THEMED_INDEX_SUBTLE, XML_dk1, XML_tint, 95000 ),
    AUTOFORMAT_END()
};

#undef AUTOFORMAT_THEME_COL
#undef AUTOFORMAT_THEME_MOD
#undef AUTOFORMAT_NONE
#undef AUTOFORMAT_END

const AutoFormatEntry* lclGetEntry( const AutoFormatEntry* pEntries, sal_Int32 nStyle )
{
    for( ; pEntries && (pEntries->mnFirstStyleIdx >= 0); ++pEntries )
        if( (pEntries->mnFirstStyleIdx <= nStyle) && (nStyle <= pEntries->mnLastStyleIdx) )
            return pEntries;
    return 0;
}

// ----------------------------------------------------------------------------

/** Contains information about auto formatting of a specific chart object type. */
struct ObjectAutoFormatEntry
{
    ObjectType          meObjType;          /// Object type for automatic format.
    PropertyType        mePropType;         /// Property type for property names.
    const AutoFormatEntry* mpLines;          /// Automatic line formatting for all chart styles.
    const AutoFormatEntry* mpFills;          /// Automatic fill formatting for all chart styles.
    const AutoFormatEntry* mpEffects;        /// Automatic effects for all chart styles.
    bool                mbIsFrame;          /// True = object is a frame, false = object is a line.
};

// ----------------------------------------------------------------------------

static const ObjectAutoFormatEntry spObjAutoFormats[] =
{
    // object type               property type              auto line           auto fill           auto effect         isframe
    { OBJECTTYPE_CHARTSPACE,     PROPERTYTYPE_COMMON,       spChartSpaceLines,  spChartSpaceFills,  0 /* eq to Ch2 */,  true  },
    { OBJECTTYPE_CHARTTITLE,     PROPERTYTYPE_COMMON,       0 /* eq to Ch2 */,  0 /* eq to Ch2 */,  0 /* eq to Ch2 */,  true  },
    { OBJECTTYPE_LEGEND,         PROPERTYTYPE_COMMON,       spNoFormats,        spNoFormats,        0 /* eq to Ch2 */,  true  },
    { OBJECTTYPE_PLOTAREA2D,     PROPERTYTYPE_COMMON,       0 /* eq to Ch2 */,  spPlotArea2dFills,  0 /* eq to Ch2 */,  true  },
    { OBJECTTYPE_PLOTAREA3D,     PROPERTYTYPE_COMMON,       0 /* eq to Ch2 */,  0 /* eq to Ch2 */,  0 /* eq to Ch2 */,  true  },
    { OBJECTTYPE_WALL,           PROPERTYTYPE_COMMON,       0 /* eq to Ch2 */,  spWallFloorFills,   0 /* eq to Ch2 */,  true  },
    { OBJECTTYPE_FLOOR,          PROPERTYTYPE_COMMON,       0 /* eq to Ch2 */,  spWallFloorFills,   0 /* eq to Ch2 */,  true  },
    { OBJECTTYPE_AXIS,           PROPERTYTYPE_COMMON,       0,                  0,                  0,                  false },
    { OBJECTTYPE_AXISTITLE,      PROPERTYTYPE_COMMON,       0 /* eq to Ch2 */,  0 /* eq to Ch2 */,  0 /* eq to Ch2 */,  true  },
    { OBJECTTYPE_AXISUNIT,       PROPERTYTYPE_COMMON,       0,                  0,                  0,                  true  },
    { OBJECTTYPE_GRIDLINE,       PROPERTYTYPE_COMMON,       0,                  0,                  0,                  false },
    { OBJECTTYPE_LINEARSERIES2D, PROPERTYTYPE_LINEARSERIES, 0,                  0,                  0,                  false },
    { OBJECTTYPE_FILLEDSERIES2D, PROPERTYTYPE_FILLEDSERIES, 0,                  0,                  0,                  true  },
    { OBJECTTYPE_FILLEDSERIES3D, PROPERTYTYPE_FILLEDSERIES, 0,                  0,                  0,                  true  },
    { OBJECTTYPE_DATALABEL,      PROPERTYTYPE_COMMON,       0 /* eq to Ch2 */,  0 /* eq to Ch2 */,  0 /* eq to Ch2 */,  true  },
    { OBJECTTYPE_TRENDLINE,      PROPERTYTYPE_COMMON,       0,                  0,                  0,                  false },
    { OBJECTTYPE_TRENDLINELABEL, PROPERTYTYPE_COMMON,       0 /* eq to Ch2 */,  0 /* eq to Ch2 */,  0 /* eq to Ch2 */,  true  },
    { OBJECTTYPE_ERRORBAR,       PROPERTYTYPE_COMMON,       0,                  0,                  0,                  false },
    { OBJECTTYPE_SERLINE,        PROPERTYTYPE_COMMON,       0,                  0,                  0,                  false },
    { OBJECTTYPE_LEADERLINE,     PROPERTYTYPE_COMMON,       0,                  0,                  0,                  false },
    { OBJECTTYPE_DROPLINE,       PROPERTYTYPE_COMMON,       0,                  0,                  0,                  false },
    { OBJECTTYPE_HILOLINE,       PROPERTYTYPE_LINEARSERIES, 0,                  0,                  0,                  false },
    { OBJECTTYPE_UPBAR,          PROPERTYTYPE_COMMON,       0,                  0,                  0,                  true  },
    { OBJECTTYPE_DOWNBAR,        PROPERTYTYPE_COMMON,       0,                  0,                  0,                  true  },
    { OBJECTTYPE_DATATABLE,      PROPERTYTYPE_COMMON,       0,                  0,                  0,                  false },
};

// ============================================================================

/** Auto formatting for a specific object type. */
class ObjectAutoFormat
{
public:
    explicit            ObjectAutoFormat(
                            const XmlFilterBase& rFilter,
                            const ObjectAutoFormatEntry& rObjFmt,
                            sal_Int32 nStyle );

    /** Sets auto formatting properties to the passed property set. */
    void                convertAutoFormats( PropertySet& rPropSet ) const;

private:
    struct AutoFormatInfo
    {
        enum AutoFormatType { AUTOFORMAT_SKIP, AUTOFORMAT_INVISIBLE, AUTOFORMAT_EXPLICIT };

        AutoFormatType      meType;             /// Type of the auto formatting.
        sal_Int32           mnThemeIdx;         /// Themed style index.
        sal_Int32           mnColor;            /// RGB color used for themed style.

        explicit            AutoFormatInfo( const XmlFilterBase& rFilter, const AutoFormatEntry* pEntry );
    };

    AutoFormatInfo      maLineFmt;          /// Automatic line formatting.
    AutoFormatInfo      maFillFmt;          /// Automatic fill formatting.
    AutoFormatInfo      maEffectFmt;        /// Automatic effect formatting.
    PropertyType        mePropType;         /// Property type for property names.
    bool                mbIsFrame;          /// True = object is a frame, false = object is a line.
};

ObjectAutoFormat::AutoFormatInfo::AutoFormatInfo( const XmlFilterBase& rFilter, const AutoFormatEntry* pEntry ) :
    meType( AUTOFORMAT_SKIP ),
    mnThemeIdx( -1 ),
    mnColor( -1 )
{
    if( pEntry )
    {
        meType = (pEntry->meColorType == AUTOCOLORTYPE_NONE) ? AUTOFORMAT_INVISIBLE : AUTOFORMAT_EXPLICIT;
        mnThemeIdx = pEntry->mnThemeIdx;

        Color aColor;
        switch( pEntry->meColorType )
        {
        case AUTOCOLORTYPE_NONE:
            break;
            case AUTOCOLORTYPE_RGB:
                aColor.setSrgbClr( pEntry->mnColorValue );
            break;
            case AUTOCOLORTYPE_THEME:
                aColor.setSchemeClr( pEntry->mnColorValue );
            break;
        }
        if( pEntry->mnModToken != XML_TOKEN_INVALID )
            aColor.addTransformation( pEntry->mnModToken, pEntry->mnModValue );
        mnColor = aColor.getColor( rFilter );
    }
}

ObjectAutoFormat::ObjectAutoFormat( const XmlFilterBase& rFilter, const ObjectAutoFormatEntry& rObjFmt, sal_Int32 nStyle ) :
    maLineFmt( rFilter, lclGetEntry( rObjFmt.mpLines, nStyle ) ),
    maFillFmt( rFilter, lclGetEntry( rObjFmt.mpFills, nStyle ) ),
    maEffectFmt( rFilter, lclGetEntry( rObjFmt.mpEffects, nStyle ) ),
    mePropType( rObjFmt.mePropType ),
    mbIsFrame( rObjFmt.mbIsFrame )
{
}

void ObjectAutoFormat::convertAutoFormats( PropertySet& rPropSet ) const
{
    namespace cssd = ::com::sun::star::drawing;

    switch( maLineFmt.meType )
    {
        case AutoFormatInfo::AUTOFORMAT_SKIP:
        break;
        case AutoFormatInfo::AUTOFORMAT_INVISIBLE:
            rPropSet.setProperty( CREATE_OUSTRING( "LineStyle" ), cssd::LineStyle_NONE );
        break;
        case AutoFormatInfo::AUTOFORMAT_EXPLICIT:
            rPropSet.setProperty( CREATE_OUSTRING( "LineStyle" ), cssd::LineStyle_SOLID );
            rPropSet.setProperty( CREATE_OUSTRING( "LineWidth" ), sal_Int32( 0 ) ); // hairline
            rPropSet.setProperty( CREATE_OUSTRING( "LineColor" ), maLineFmt.mnColor );
        break;
    }

    switch( maFillFmt.meType )
    {
        case AutoFormatInfo::AUTOFORMAT_SKIP:
        break;
        case AutoFormatInfo::AUTOFORMAT_INVISIBLE:
            rPropSet.setProperty( CREATE_OUSTRING( "FillStyle" ), cssd::FillStyle_NONE );
        break;
        case AutoFormatInfo::AUTOFORMAT_EXPLICIT:
            rPropSet.setProperty( CREATE_OUSTRING( "FillStyle" ), cssd::FillStyle_SOLID );
            rPropSet.setProperty( CREATE_OUSTRING( "FillColor" ), maFillFmt.mnColor );
        break;
    }
}

} // namespace

// ============================================================================
// ============================================================================

struct ConverterData
{
    typedef RefMap< ObjectType, ObjectAutoFormat > ObjectAutoFormatMap;

    XmlFilterBase&      mrFilter;
    ChartConverter&     mrConverter;
    Reference< XChartDocument > mxDoc;
    ObjectAutoFormatMap maAutoFormats;

    explicit            ConverterData(
                            XmlFilterBase& rFilter,
                            ChartConverter& rChartConverter,
                            const Reference< XChartDocument >& rxChartDoc );
                        ~ConverterData();
};

// ----------------------------------------------------------------------------

ConverterData::ConverterData(
        XmlFilterBase& rFilter,
        ChartConverter& rChartConverter,
        const Reference< XChartDocument >& rxChartDoc ) :
    mrFilter( rFilter ),
    mrConverter( rChartConverter ),
    mxDoc( rxChartDoc )
{
    OSL_ENSURE( mxDoc.is(), "ConverterData::ConverterData - missing chart document" );
    // lock the model to suppress internal updates during conversion
    try
    {
        Reference< XModel > xModel( mxDoc, UNO_QUERY_THROW );
        xModel->lockControllers();
    }
    catch( Exception& )
    {
    }
}

ConverterData::~ConverterData()
{
    // unlock the model
    try
    {
        Reference< XModel > xModel( mxDoc, UNO_QUERY_THROW );
        xModel->unlockControllers();
    }
    catch( Exception& )
    {
    }
}

// ============================================================================

ConverterRoot::ConverterRoot(
        XmlFilterBase& rFilter,
        ChartConverter& rChartConverter,
        const Reference< XChartDocument >& rxChartDoc ) :
    mxData( new ConverterData( rFilter, rChartConverter, rxChartDoc ) )
{
}

ConverterRoot::~ConverterRoot()
{
}

Reference< XInterface > ConverterRoot::createInstance(
        const Reference< XMultiServiceFactory >& rxFactory, const OUString& rServiceName )
{
    Reference< XInterface > xInt;
    if( rxFactory.is() ) try
    {
        xInt = rxFactory->createInstance( rServiceName );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( xInt.is(), "ConverterRoot::createInstance - cannot create instance" );
    return xInt;
}

Reference< XInterface > ConverterRoot::createInstance( const OUString& rServiceName )
{
    return createInstance( ::comphelper::getProcessServiceFactory(), rServiceName );
}

XmlFilterBase& ConverterRoot::getFilter() const
{
    return mxData->mrFilter;
}

ChartConverter& ConverterRoot::getChartConverter() const
{
    return mxData->mrConverter;
}

Reference< XChartDocument > ConverterRoot::getChartDocument() const
{
    return mxData->mxDoc;
}

void ConverterRoot::initAutoFormats( sal_Int32 nStyle )
{
    const ObjectAutoFormatEntry* pObjFmtEnd = STATIC_ARRAY_END( spObjAutoFormats );
    for( const ObjectAutoFormatEntry* pObjFmt = spObjAutoFormats; pObjFmt != pObjFmtEnd; ++pObjFmt )
        mxData->maAutoFormats[ pObjFmt->meObjType ].reset( new ObjectAutoFormat( mxData->mrFilter, *pObjFmt, nStyle ) );
}

void ConverterRoot::convertAutoFormats( PropertySet& rPropSet, ObjectType eObjType ) const
{
    if( const ObjectAutoFormat* pAutoFormat = mxData->maAutoFormats.get( eObjType ).get() )
        pAutoFormat->convertAutoFormats( rPropSet );
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

