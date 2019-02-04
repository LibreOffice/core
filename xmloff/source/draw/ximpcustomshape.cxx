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

#include "ximpcustomshape.hxx"
#include "ximpshap.hxx"
#include <o3tl/any.hxx>
#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <xmloff/xmltoken.hxx>
#include <EnhancedCustomShapeToken.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlement.hxx>
#include <xexptran.hxx>
#include <xmloff/xmlerror.hxx>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextPathMode.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <sax/tools/converter.hxx>
#include <comphelper/sequence.hxx>
#include <memory>
#include <unordered_map>

using namespace ::com::sun::star;
using namespace ::xmloff::token;
using namespace ::xmloff::EnhancedCustomShapeToken;


XMLEnhancedCustomShapeContext::XMLEnhancedCustomShapeContext( SvXMLImport& rImport,
            css::uno::Reference< css::drawing::XShape >& rxShape,
            sal_uInt16 nPrefix, const OUString& rLocalName,
            std::vector< css::beans::PropertyValue >& rCustomShapeGeometry ) :
        SvXMLImportContext( rImport, nPrefix, rLocalName ),
        mrUnitConverter( rImport.GetMM100UnitConverter() ),
        mrxShape( rxShape ),
        mrCustomShapeGeometry( rCustomShapeGeometry )
{
}

const SvXMLEnumMapEntry<sal_uInt16> aXML_GluePointEnumMap[] =
{
    { XML_NONE,         0 },
    { XML_SEGMENTS,     1 },
    { XML_NONE,         2 },
    { XML_RECTANGLE,    3 },
    { XML_TOKEN_INVALID, 0 }
};
static void GetBool( std::vector< css::beans::PropertyValue >& rDest,
                        const OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    bool bAttrBool;
    if (::sax::Converter::convertBool( bAttrBool, rValue ))
    {
        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= bAttrBool;
        rDest.push_back( aProp );
    }
}

static void GetInt32( std::vector< css::beans::PropertyValue >& rDest,
                        const OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    sal_Int32 nAttrNumber;
    if (::sax::Converter::convertNumber( nAttrNumber, rValue ))
    {
        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= nAttrNumber;
        rDest.push_back( aProp );
    }
}

static void GetDouble( std::vector< css::beans::PropertyValue >& rDest,
                        const OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    double fAttrDouble;
    if (::sax::Converter::convertDouble( fAttrDouble, rValue ))
    {
        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= fAttrDouble;
        rDest.push_back( aProp );
    }
}

static void GetString( std::vector< css::beans::PropertyValue >& rDest,
                        const OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    beans::PropertyValue aProp;
    aProp.Name = EASGet( eDestProp );
    aProp.Value <<= rValue;
    rDest.push_back( aProp );
}

template<typename EnumT>
static void GetEnum( std::vector< css::beans::PropertyValue >& rDest,
                         const OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp,
                        const SvXMLEnumMapEntry<EnumT>& rMap )
{
    EnumT eKind;
    if( SvXMLUnitConverter::convertEnum( eKind, rValue, &rMap ) )
    {
        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= static_cast<sal_Int16>(eKind);
        rDest.push_back( aProp );
    }
}

static void GetDoublePercentage( std::vector< css::beans::PropertyValue >& rDest,
                         const OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    sal_Int16 const eSrcUnit = ::sax::Converter::GetUnitFromString(
            rValue, util::MeasureUnit::MM_100TH);
    if (util::MeasureUnit::PERCENT == eSrcUnit)
    {
        rtl_math_ConversionStatus eStatus;
        double fAttrDouble = ::rtl::math::stringToDouble( rValue,
            '.', ',', &eStatus );
        if ( eStatus == rtl_math_ConversionStatus_Ok )
        {
            beans::PropertyValue aProp;
            aProp.Name = EASGet( eDestProp );
            aProp.Value <<= fAttrDouble;
            rDest.push_back( aProp );
        }
    }
}

static void GetB3DVector( std::vector< css::beans::PropertyValue >& rDest,
                         const OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    ::basegfx::B3DVector aB3DVector;
    if ( SvXMLUnitConverter::convertB3DVector( aB3DVector, rValue ) )
    {
        drawing::Direction3D aDirection3D( aB3DVector.getX(), aB3DVector.getY(), aB3DVector.getZ() );
        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= aDirection3D;
        rDest.push_back( aProp );
    }
}

static bool GetEquationName( const OUString& rEquation, const sal_Int32 nStart, OUString& rEquationName )
{
    sal_Int32 nIndex = nStart;
    while( nIndex < rEquation.getLength() )
    {
        sal_Unicode nChar = rEquation[ nIndex ];
        if (
            ( ( nChar >= 'a' ) && ( nChar <= 'z' ) )
            || ( ( nChar >= 'A' ) && ( nChar <= 'Z' ) )
            || ( ( nChar >= '0' ) && ( nChar <= '9' ) )
            )
        {
            nIndex++;
        }
        else
            break;
    }
    bool bValid = ( nIndex - nStart ) != 0;
    if ( bValid )
        rEquationName = rEquation.copy( nStart, nIndex - nStart );
    return bValid;
}

static bool GetNextParameter( css::drawing::EnhancedCustomShapeParameter& rParameter, sal_Int32& nIndex, const OUString& rParaString )
{
    if ( nIndex >= rParaString.getLength() )
        return false;

    bool bValid = true;
    bool bNumberRequired = true;
    bool bMustBePositiveWholeNumbered = false;

    rParameter.Type = css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    if ( rParaString[ nIndex ] == '$' )
    {
        rParameter.Type = css::drawing::EnhancedCustomShapeParameterType::ADJUSTMENT;
        bMustBePositiveWholeNumbered = true;
        nIndex++;
    }
    else if ( rParaString[ nIndex ] == '?' )
    {
        nIndex++;
        bNumberRequired = false;
        OUString aEquationName;
        bValid = GetEquationName( rParaString, nIndex, aEquationName );
        if ( bValid )
        {
            rParameter.Type = css::drawing::EnhancedCustomShapeParameterType::EQUATION;
            rParameter.Value <<= aEquationName;
            nIndex += aEquationName.getLength();
        }
    }
    else if ( rParaString[ nIndex ] > '9' )
    {
        bNumberRequired = false;
        if ( rParaString.matchIgnoreAsciiCase( "left", nIndex ) )
        {
            rParameter.Type = css::drawing::EnhancedCustomShapeParameterType::LEFT;
            nIndex += 4;
        }
        else if ( rParaString.matchIgnoreAsciiCase( "top", nIndex ) )
        {
            rParameter.Type = css::drawing::EnhancedCustomShapeParameterType::TOP;
            nIndex += 3;
        }
        else if ( rParaString.matchIgnoreAsciiCase( "right", nIndex ) )
        {
            rParameter.Type = css::drawing::EnhancedCustomShapeParameterType::RIGHT;
            nIndex += 5;
        }
        else if ( rParaString.matchIgnoreAsciiCase( "bottom", nIndex ) )
        {
            rParameter.Type = css::drawing::EnhancedCustomShapeParameterType::BOTTOM;
            nIndex += 6;
        }
        else if ( rParaString.matchIgnoreAsciiCase( "xstretch", nIndex ) )
        {
            rParameter.Type = css::drawing::EnhancedCustomShapeParameterType::XSTRETCH;
            nIndex += 8;
        }
        else if ( rParaString.matchIgnoreAsciiCase( "ystretch", nIndex ) )
        {
            rParameter.Type = css::drawing::EnhancedCustomShapeParameterType::YSTRETCH;
            nIndex += 8;
        }
        else if ( rParaString.matchIgnoreAsciiCase( "hasstroke", nIndex ) )
        {
            rParameter.Type = css::drawing::EnhancedCustomShapeParameterType::HASSTROKE;
            nIndex += 9;
        }
        else if ( rParaString.matchIgnoreAsciiCase( "hasfill", nIndex ) )
        {
            rParameter.Type = css::drawing::EnhancedCustomShapeParameterType::HASFILL;
            nIndex += 7;
        }
        else if ( rParaString.matchIgnoreAsciiCase( "width", nIndex ) )
        {
            rParameter.Type = css::drawing::EnhancedCustomShapeParameterType::WIDTH;
            nIndex += 5;
        }
        else if ( rParaString.matchIgnoreAsciiCase( "height", nIndex ) )
        {
            rParameter.Type = css::drawing::EnhancedCustomShapeParameterType::HEIGHT;
            nIndex += 6;
        }
        else if ( rParaString.matchIgnoreAsciiCase( "logwidth", nIndex ) )
        {
            rParameter.Type = css::drawing::EnhancedCustomShapeParameterType::LOGWIDTH;
            nIndex += 8;
        }
        else if ( rParaString.matchIgnoreAsciiCase( "logheight", nIndex ) )
        {
            rParameter.Type = css::drawing::EnhancedCustomShapeParameterType::LOGHEIGHT;
            nIndex += 9;
        }
        else
            bValid = false;
    }
    if ( bValid )
    {
        if ( bNumberRequired )
        {
            sal_Int32 nStartIndex = nIndex;
            sal_Int32 nEIndex = 0;  // index of "E" in double

            bool bE = false;    // set if a double is including a "E" statement
            bool bENum = false; // there is at least one number after "E"
            bool bDot = false;  // set if there is a dot included
            bool bEnd = false;  // set for each value that can not be part of a double/integer

            while( ( nIndex < rParaString.getLength() ) && bValid )
            {
                switch( rParaString[ nIndex ] )
                {
                    case '.' :
                    {
                        if ( bMustBePositiveWholeNumbered )
                            bValid = false;
                        else
                        {
                            if ( bDot )
                                bValid = false;
                            else
                                bDot = true;
                        }
                    }
                    break;
                    case '-' :
                    {
                        if ( bMustBePositiveWholeNumbered )
                            bValid = false;
                        else
                        {
                            if ( nStartIndex == nIndex )
                               bValid = true;
                            else if ( bE )
                            {
                                if ( nEIndex + 1 == nIndex )
                                    bValid = true;
                                else if ( bENum )
                                    bEnd = true;
                                else
                                    bValid = false;
                            }
                        }
                    }
                    break;

                    case 'e' :
                    case 'E' :
                    {
                        if ( bMustBePositiveWholeNumbered )
                            bEnd = true;
                        else
                        {
                            if ( !bE )
                            {
                                bE = true;
                                nEIndex = nIndex;
                            }
                            else
                                bEnd = true;
                        }
                    }
                    break;
                    case '0' :
                    case '1' :
                    case '2' :
                    case '3' :
                    case '4' :
                    case '5' :
                    case '6' :
                    case '7' :
                    case '8' :
                    case '9' :
                    {
                        if ( bE && ! bENum )
                            bENum = true;
                    }
                    break;
                    default:
                        bEnd = true;
                }
                if ( !bEnd )
                    nIndex++;
                else
                    break;
            }
            if ( nIndex == nStartIndex )
                bValid = false;
            if ( bValid )
            {
                OUString aNumber( rParaString.copy( nStartIndex, nIndex - nStartIndex ) );
                if ( bE || bDot )
                {
                    double fAttrDouble;
                    if (::sax::Converter::convertDouble(fAttrDouble, aNumber))
                        rParameter.Value <<= fAttrDouble;
                    else
                        bValid = false;
                }
                else
                {
                    sal_Int32 nValue;
                    if (::sax::Converter::convertNumber(nValue, aNumber))
                        rParameter.Value <<= nValue;
                    else
                        bValid = false;
                }
            }
        }
    }
    if ( bValid )
    {
        // skipping white spaces and commas (#i121507#)
        const sal_Unicode aSpace(' ');
        const sal_Unicode aCommata(',');

        while(nIndex < rParaString.getLength())
        {
            const sal_Unicode aCandidate(rParaString[nIndex]);

            if(aSpace == aCandidate || aCommata == aCandidate)
            {
                nIndex++;
            }
            else
            {
                break;
            }
        }
    }
    return bValid;
}

static void GetPosition3D( std::vector< css::beans::PropertyValue >& rDest,                     // e.g. draw:extrusion-viewpoint
                        const OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp,
                        SvXMLUnitConverter& rUnitConverter )
{
    drawing::Position3D aPosition3D;
    if ( rUnitConverter.convertPosition3D( aPosition3D, rValue ) )
    {
        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= aPosition3D;
        rDest.push_back( aProp );
    }
}

static void GetDoubleSequence( std::vector< css::beans::PropertyValue >& rDest,                 // e.g. draw:glue-point-leaving-directions
                        const OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    std::vector< double > vDirection;
    sal_Int32 nIndex = 0;
    do
    {
        double fAttrDouble;
        OUString aToken( rValue.getToken( 0, ',', nIndex ) );
        if (!::sax::Converter::convertDouble( fAttrDouble, aToken ))
            break;
        else
            vDirection.push_back( fAttrDouble );
    }
    while ( nIndex >= 0 );

    if ( !vDirection.empty() )
    {
        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= comphelper::containerToSequence(vDirection);
        rDest.push_back( aProp );
    }
}

static void GetSizeSequence( std::vector< css::beans::PropertyValue >& rDest,
                      const OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    std::vector< sal_Int32 > vNum;
    sal_Int32 nIndex = 0;
    do
    {
        sal_Int32 n;
        OUString aToken( rValue.getToken( 0, ' ', nIndex ) );
        if (!::sax::Converter::convertNumber( n, aToken ))
            break;
        else
            vNum.push_back( n );
    }
    while ( nIndex >= 0 );

    if ( !vNum.empty() )
    {
        uno::Sequence< awt::Size > aSizeSeq((vNum.size() + 1) / 2);
        std::vector< sal_Int32 >::const_iterator aIter = vNum.begin();
        std::vector< sal_Int32 >::const_iterator aEnd = vNum.end();
        awt::Size* pValues = aSizeSeq.getArray();

        while ( aIter != aEnd ) {
            pValues->Width = *aIter++;
            if ( aIter != aEnd )
                pValues->Height = *aIter++;
            pValues ++;
        }

        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= aSizeSeq;
        rDest.push_back( aProp );
    }
}

static void GetEnhancedParameter( std::vector< css::beans::PropertyValue >& rDest,              // e.g. draw:handle-position
                        const OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    sal_Int32 nIndex = 0;
    css::drawing::EnhancedCustomShapeParameter aParameter;
    if ( GetNextParameter( aParameter, nIndex, rValue ) )
    {
        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= aParameter;
        rDest.push_back( aProp );
    }
}

static void GetEnhancedParameterPair( std::vector< css::beans::PropertyValue >& rDest,          // e.g. draw:handle-position
                        const OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    sal_Int32 nIndex = 0;
    css::drawing::EnhancedCustomShapeParameterPair aParameterPair;
    if ( GetNextParameter( aParameterPair.First, nIndex, rValue )
        && GetNextParameter( aParameterPair.Second, nIndex, rValue ) )
    {
        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= aParameterPair;
        rDest.push_back( aProp );
    }
}

static sal_Int32 GetEnhancedParameterPairSequence( std::vector< css::beans::PropertyValue >& rDest,     // e.g. draw:glue-points
                        const OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    std::vector< css::drawing::EnhancedCustomShapeParameterPair > vParameter;
    css::drawing::EnhancedCustomShapeParameterPair aParameter;

    sal_Int32 nIndex = 0;
    while ( GetNextParameter( aParameter.First, nIndex, rValue )
            && GetNextParameter( aParameter.Second, nIndex, rValue ) )
    {
        vParameter.push_back( aParameter );
    }
    if ( !vParameter.empty() )
    {
        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= comphelper::containerToSequence(vParameter);
        rDest.push_back( aProp );
    }
    return vParameter.size();
}

static void GetEnhancedRectangleSequence( std::vector< css::beans::PropertyValue >& rDest,      // e.g. draw:text-areas
                        const OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    std::vector< css::drawing::EnhancedCustomShapeTextFrame > vTextFrame;
    css::drawing::EnhancedCustomShapeTextFrame aParameter;

    sal_Int32 nIndex = 0;

    while ( GetNextParameter( aParameter.TopLeft.First, nIndex, rValue )
            && GetNextParameter( aParameter.TopLeft.Second, nIndex, rValue )
            && GetNextParameter( aParameter.BottomRight.First, nIndex, rValue )
            && GetNextParameter( aParameter.BottomRight.Second, nIndex, rValue ) )
    {
        vTextFrame.push_back( aParameter );
    }
    if ( !vTextFrame.empty() )
    {
        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= comphelper::containerToSequence(vTextFrame);
        rDest.push_back( aProp );
    }
}

static void GetEnhancedPath( std::vector< css::beans::PropertyValue >& rDest,                   // e.g. draw:enhanced-path
                        const OUString& rValue )
{
    std::vector< css::drawing::EnhancedCustomShapeParameterPair >    vCoordinates;
    std::vector< css::drawing::EnhancedCustomShapeSegment >      vSegments;

    sal_Int32 nIndex = 0;
    sal_Int32 nParameterCount = 0;

    sal_Int32 nParametersNeeded = 1;
    sal_Int16 nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::MOVETO;

    bool bValid = true;

    while( bValid && ( nIndex < rValue.getLength() ) )
    {
        switch( rValue[ nIndex ] )
        {
            case 'M' :
            {
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::MOVETO;
                nParametersNeeded = 1;
                nIndex++;
            }
            break;
            case 'L' :
            {
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::LINETO;
                nParametersNeeded = 1;
                nIndex++;
            }
            break;
            case 'C' :
            {
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::CURVETO;
                nParametersNeeded = 3;
                nIndex++;
            }
            break;
            case 'Z' :
            {
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::CLOSESUBPATH;
                nParametersNeeded = 0;
                nIndex++;
            }
            break;
            case 'N' :
            {
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::ENDSUBPATH;
                nParametersNeeded = 0;
                nIndex++;
            }
            break;
            case 'F' :
            {
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::NOFILL;
                nParametersNeeded = 0;
                nIndex++;
            }
            break;
            case 'S' :
            {
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::NOSTROKE;
                nParametersNeeded = 0;
                nIndex++;
            }
            break;
            case 'T' :
            {
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::ANGLEELLIPSETO;
                nParametersNeeded = 3;
                nIndex++;
            }
            break;
            case 'U' :
            {
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::ANGLEELLIPSE;
                nParametersNeeded = 3;
                nIndex++;
            }
            break;
            case 'A' :
            {
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::ARCTO;
                nParametersNeeded = 4;
                nIndex++;
            }
            break;
            case 'B' :
            {
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::ARC;
                nParametersNeeded = 4;
                nIndex++;
            }
            break;
            case 'G' :
            {
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::ARCANGLETO;
                nParametersNeeded = 2;
                nIndex++;
            }
            break;
            case 'H' :
            {
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::DARKEN;
                nParametersNeeded = 0;
                nIndex++;
            }
            break;
            case 'I' :
            {
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::DARKENLESS;
                nParametersNeeded = 0;
                nIndex++;
            }
            break;
            case 'J' :
            {
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::LIGHTEN;
                nParametersNeeded = 0;
                nIndex++;
            }
            break;
            case 'K' :
            {
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::LIGHTENLESS;
                nParametersNeeded = 0;
                nIndex++;
            }
            break;
            case 'W' :
            {
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARCTO;
                nParametersNeeded = 4;
                nIndex++;
            }
            break;
            case 'V' :
            {
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARC;
                nParametersNeeded = 4;
                nIndex++;
            }
            break;
            case 'X' :
            {
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTX;
                nParametersNeeded = 1;
                nIndex++;
            }
            break;
            case 'Y' :
            {
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTY;
                nParametersNeeded = 1;
                nIndex++;
            }
            break;
            case 'Q' :
            {
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::QUADRATICCURVETO;
                nParametersNeeded = 2;
                nIndex++;
            }
            break;
            case ' ' :
            {
                nIndex++;
            }
            break;

            case '$' :
            case '?' :
            case '0' :
            case '1' :
            case '2' :
            case '3' :
            case '4' :
            case '5' :
            case '6' :
            case '7' :
            case '8' :
            case '9' :
            case '.' :
            case '-' :
            {
                css::drawing::EnhancedCustomShapeParameterPair aPair;
                if ( GetNextParameter( aPair.First, nIndex, rValue ) &&
                        GetNextParameter( aPair.Second, nIndex, rValue ) )
                {
                    vCoordinates.push_back( aPair );
                    nParameterCount++;
                }
                else
                    bValid = false;
            }
            break;
            default:
                nIndex++;
            break;
        }
        if ( !nParameterCount && !nParametersNeeded )
        {
            css::drawing::EnhancedCustomShapeSegment aSegment;
            aSegment.Command = nLatestSegmentCommand;
            aSegment.Count = 0;
            vSegments.push_back( aSegment );
            nParametersNeeded = 0x7fffffff;
        }
        else if ( nParameterCount >= nParametersNeeded )
        {
            // Special rule for moveto in ODF 1.2 section 19.145
            // "If a moveto is followed by multiple pairs of coordinates, they are treated as lineto."
            if ( nLatestSegmentCommand == css::drawing::EnhancedCustomShapeSegmentCommand::MOVETO )
            {
                css::drawing::EnhancedCustomShapeSegment aSegment;
                aSegment.Command = css::drawing::EnhancedCustomShapeSegmentCommand::MOVETO;
                aSegment.Count = 1;
                vSegments.push_back( aSegment );
                nIndex--;
                nLatestSegmentCommand = css::drawing::EnhancedCustomShapeSegmentCommand::LINETO;
                nParametersNeeded = 1;
            }
            else
            {
                // General rule in ODF 1.2. section 19.145
                // "If a command is repeated multiple times, all repeated command characters
                // except the first one may be omitted." Thus check if the last command is identical,
                // if so, we just need to increment the count
                if ( !vSegments.empty() && ( vSegments[ vSegments.size() - 1 ].Command == nLatestSegmentCommand ) )
                    vSegments[ vSegments.size() -1 ].Count++;
                else
                {
                    css::drawing::EnhancedCustomShapeSegment aSegment;
                    aSegment.Command = nLatestSegmentCommand;
                    aSegment.Count = 1;
                    vSegments.push_back( aSegment );
                }
            }
            nParameterCount = 0;
        }
    }

    // adding the Coordinates property
    beans::PropertyValue aProp;
    aProp.Name = EASGet( EAS_Coordinates );
    aProp.Value <<= comphelper::containerToSequence(vCoordinates);
    rDest.push_back( aProp );

    // adding the Segments property
    aProp.Name = EASGet( EAS_Segments );
    aProp.Value <<= comphelper::containerToSequence(vSegments);
    rDest.push_back( aProp );
}

static void GetAdjustmentValues( std::vector< css::beans::PropertyValue >& rDest,               // draw:adjustments
                        const OUString& rValue )
{
    std::vector< css::drawing::EnhancedCustomShapeAdjustmentValue > vAdjustmentValue;
    css::drawing::EnhancedCustomShapeParameter aParameter;
    sal_Int32 nIndex = 0;
    while ( GetNextParameter( aParameter, nIndex, rValue ) )
    {
        css::drawing::EnhancedCustomShapeAdjustmentValue aAdj;
        if ( aParameter.Type == css::drawing::EnhancedCustomShapeParameterType::NORMAL )
        {
            aAdj.Value = aParameter.Value;
            aAdj.State = beans::PropertyState_DIRECT_VALUE;
        }
        else
            aAdj.State = beans::PropertyState_DEFAULT_VALUE;    // this should not be, but better than setting nothing

        vAdjustmentValue.push_back( aAdj );
    }

    sal_Int32 nAdjustmentValues = vAdjustmentValue.size();
    if ( nAdjustmentValues )
    {
        beans::PropertyValue aProp;
        aProp.Name = EASGet( EAS_AdjustmentValues );
        aProp.Value <<= comphelper::containerToSequence(vAdjustmentValue);
        rDest.push_back( aProp );
    }
}

void XMLEnhancedCustomShapeContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    sal_Int16 nLength = xAttrList->getLength();
    if ( nLength )
    {
        sal_Int32               nAttrNumber;
        for( sal_Int16 nAttr = 0; nAttr < nLength; nAttr++ )
        {
            OUString aLocalName;
            const OUString& rValue = xAttrList->getValueByIndex( nAttr );
            /* sven fixme, this must be checked! sal_uInt16 nPrefix = */ GetImport().GetNamespaceMap().GetKeyByAttrName( xAttrList->getNameByIndex( nAttr ), &aLocalName );

            switch( EASGet( aLocalName ) )
            {
                case EAS_type :
                    GetString( mrCustomShapeGeometry, rValue, EAS_Type );
                break;
                case EAS_mirror_horizontal :
                    GetBool( mrCustomShapeGeometry, rValue, EAS_MirroredX );
                break;
                case EAS_mirror_vertical :
                    GetBool( mrCustomShapeGeometry, rValue, EAS_MirroredY );
                break;
                case EAS_viewBox :
                {
                    SdXMLImExViewBox aViewBox( rValue, GetImport().GetMM100UnitConverter() );
                    awt::Rectangle aRect( aViewBox.GetX(), aViewBox.GetY(), aViewBox.GetWidth(), aViewBox.GetHeight() );
                    beans::PropertyValue aProp;
                    aProp.Name = EASGet( EAS_ViewBox );
                    aProp.Value <<= aRect;
                    mrCustomShapeGeometry.push_back( aProp );
                }
                break;
                case EAS_sub_view_size:
                    GetSizeSequence( maPath, rValue, EAS_SubViewSize );
                break;
                case EAS_text_rotate_angle :
                    GetDouble( mrCustomShapeGeometry, rValue, EAS_TextRotateAngle );
                break;
                case EAS_extrusion_allowed :
                    GetBool( maPath, rValue, EAS_ExtrusionAllowed );
                break;
                case EAS_text_path_allowed :
                    GetBool( maPath, rValue, EAS_TextPathAllowed );
                break;
                case EAS_concentric_gradient_fill_allowed :
                    GetBool( maPath, rValue, EAS_ConcentricGradientFillAllowed );
                break;
                case EAS_extrusion :
                    GetBool( maExtrusion, rValue, EAS_Extrusion );
                break;
                case EAS_extrusion_brightness :
                    GetDoublePercentage( maExtrusion, rValue, EAS_Brightness );
                break;
                case EAS_extrusion_depth :
                {
                    sal_Int32 nIndex = 0;
                    css::drawing::EnhancedCustomShapeParameterPair aParameterPair;
                    css::drawing::EnhancedCustomShapeParameter& rDepth = aParameterPair.First;
                    if ( GetNextParameter( rDepth, nIndex, rValue ) )
                    {
                        css::drawing::EnhancedCustomShapeParameter& rFraction = aParameterPair.Second;
                        // try to catch the unit for the depth
                        sal_Int16 const eSrcUnit(
                            ::sax::Converter::GetUnitFromString(
                                rValue, util::MeasureUnit::MM_100TH));

                        OUStringBuffer aUnitStr;
                        double fFactor = ::sax::Converter::GetConversionFactor(
                            aUnitStr, util::MeasureUnit::MM_100TH, eSrcUnit);
                        if ( ( fFactor != 1.0 ) && ( fFactor != 0.0 ) )
                        {
                            double fDepth(0.0);
                            if ( rDepth.Value >>= fDepth )
                            {
                                fDepth /= fFactor;
                                rDepth.Value <<= fDepth;
                            }
                        }
                        if ( rValue.matchIgnoreAsciiCase( aUnitStr.toString(), nIndex ) )
                            nIndex += aUnitStr.getLength();

                        // skipping white spaces
                        while( ( nIndex < rValue.getLength() ) && rValue[ nIndex ] == ' ' )
                            nIndex++;

                        if ( GetNextParameter( rFraction, nIndex, rValue ) )
                        {
                            beans::PropertyValue aProp;
                            aProp.Name = EASGet( EAS_Depth );
                            aProp.Value <<= aParameterPair;
                            maExtrusion.push_back( aProp );
                        }
                    }
                }
                break;
                case EAS_extrusion_diffusion :
                    GetDoublePercentage( maExtrusion, rValue, EAS_Diffusion );
                break;
                case EAS_extrusion_number_of_line_segments :
                    GetInt32( maExtrusion, rValue, EAS_NumberOfLineSegments );
                break;
                case EAS_extrusion_light_face :
                    GetBool( maExtrusion, rValue, EAS_LightFace );
                break;
                case EAS_extrusion_first_light_harsh :
                    GetBool( maExtrusion, rValue, EAS_FirstLightHarsh );
                break;
                case EAS_extrusion_second_light_harsh :
                    GetBool( maExtrusion, rValue, EAS_SecondLightHarsh );
                break;
                case EAS_extrusion_first_light_level :
                    GetDoublePercentage( maExtrusion, rValue, EAS_FirstLightLevel );
                break;
                case EAS_extrusion_second_light_level :
                    GetDoublePercentage( maExtrusion, rValue, EAS_SecondLightLevel );
                break;
                case EAS_extrusion_first_light_direction :
                    GetB3DVector( maExtrusion, rValue, EAS_FirstLightDirection );
                break;
                case EAS_extrusion_second_light_direction :
                    GetB3DVector( maExtrusion, rValue, EAS_SecondLightDirection );
                break;
                case EAS_extrusion_metal :
                    GetBool( maExtrusion, rValue, EAS_Metal );
                break;
                case EAS_shade_mode :
                {
                    drawing::ShadeMode eShadeMode( drawing::ShadeMode_FLAT );
                    if( IsXMLToken( rValue, XML_PHONG ) )
                        eShadeMode = drawing::ShadeMode_PHONG;
                    else if ( IsXMLToken( rValue, XML_GOURAUD ) )
                        eShadeMode = drawing::ShadeMode_SMOOTH;
                    else if ( IsXMLToken( rValue, XML_DRAFT ) )
                        eShadeMode = drawing::ShadeMode_DRAFT;

                    beans::PropertyValue aProp;
                    aProp.Name = EASGet( EAS_ShadeMode );
                    aProp.Value <<= eShadeMode;
                    maExtrusion.push_back( aProp );
                }
                break;
                case EAS_extrusion_rotation_angle :
                    GetEnhancedParameterPair( maExtrusion, rValue, EAS_RotateAngle );
                break;
                case EAS_extrusion_rotation_center :
                    GetB3DVector( maExtrusion, rValue, EAS_RotationCenter );
                break;
                case EAS_extrusion_shininess :
                    GetDoublePercentage( maExtrusion, rValue, EAS_Shininess );
                break;
                case EAS_extrusion_skew :
                    GetEnhancedParameterPair( maExtrusion, rValue, EAS_Skew );
                break;
                case EAS_extrusion_specularity :
                    GetDoublePercentage( maExtrusion, rValue, EAS_Specularity );
                break;
                case EAS_projection :
                {
                    drawing::ProjectionMode eProjectionMode( drawing::ProjectionMode_PERSPECTIVE );
                    if( IsXMLToken( rValue, XML_PARALLEL ) )
                        eProjectionMode = drawing::ProjectionMode_PARALLEL;

                    beans::PropertyValue aProp;
                    aProp.Name = EASGet( EAS_ProjectionMode );
                    aProp.Value <<= eProjectionMode;
                    maExtrusion.push_back( aProp );
                }
                break;
                case EAS_extrusion_viewpoint :
                    GetPosition3D( maExtrusion, rValue, EAS_ViewPoint, mrUnitConverter );
                break;
                case EAS_extrusion_origin :
                    GetEnhancedParameterPair( maExtrusion, rValue, EAS_Origin );
                break;
                case EAS_extrusion_color :
                    GetBool( maExtrusion, rValue, EAS_Color );
                break;
                case EAS_enhanced_path :
                    GetEnhancedPath( maPath, rValue );
                break;
                case EAS_path_stretchpoint_x :
                {
                    if (::sax::Converter::convertNumber(nAttrNumber, rValue))
                    {
                        beans::PropertyValue aProp;
                        aProp.Name = EASGet( EAS_StretchX );
                        aProp.Value <<= nAttrNumber;
                        maPath.push_back( aProp );
                    }
                }
                break;
                case EAS_path_stretchpoint_y :
                {
                    if (::sax::Converter::convertNumber(nAttrNumber, rValue))
                    {
                        beans::PropertyValue aProp;
                        aProp.Name = EASGet( EAS_StretchY );
                        aProp.Value <<= nAttrNumber;
                        maPath.push_back( aProp );
                    }
                }
                break;
                case EAS_text_areas :
                    GetEnhancedRectangleSequence( maPath, rValue, EAS_TextFrames );
                break;
                case EAS_glue_points :
                {
                    sal_Int32 i, nPairs = GetEnhancedParameterPairSequence( maPath, rValue, EAS_GluePoints );
                    GetImport().GetShapeImport()->moveGluePointMapping( mrxShape, nPairs );
                    for ( i = 0; i < nPairs; i++ )
                        GetImport().GetShapeImport()->addGluePointMapping( mrxShape, i + 4, i + 4 );
                }
                break;
                case EAS_glue_point_type :
                    GetEnum( maPath, rValue, EAS_GluePointType, *aXML_GluePointEnumMap );
                break;
                case EAS_glue_point_leaving_directions :
                    GetDoubleSequence( maPath, rValue, EAS_GluePointLeavingDirections );
                break;
                case EAS_text_path :
                    GetBool( maTextPath, rValue, EAS_TextPath );
                break;
                case EAS_text_path_mode :
                {
                    css::drawing::EnhancedCustomShapeTextPathMode eTextPathMode( css::drawing::EnhancedCustomShapeTextPathMode_NORMAL );
                    if( IsXMLToken( rValue, XML_PATH ) )
                        eTextPathMode = css::drawing::EnhancedCustomShapeTextPathMode_PATH;
                    else if ( IsXMLToken( rValue, XML_SHAPE ) )
                        eTextPathMode = css::drawing::EnhancedCustomShapeTextPathMode_SHAPE;

                    beans::PropertyValue aProp;
                    aProp.Name = EASGet( EAS_TextPathMode );
                    aProp.Value <<= eTextPathMode;
                    maTextPath.push_back( aProp );
                }
                break;
                case EAS_text_path_scale :
                {
                    bool bScaleX = IsXMLToken( rValue, XML_SHAPE );
                    beans::PropertyValue aProp;
                    aProp.Name = EASGet( EAS_ScaleX );
                    aProp.Value <<= bScaleX;
                    maTextPath.push_back( aProp );
                }
                break;
                case EAS_text_path_same_letter_heights :
                    GetBool( maTextPath, rValue, EAS_SameLetterHeights );
                break;
                case EAS_modifiers :
                    GetAdjustmentValues( mrCustomShapeGeometry, rValue );
                break;
                default:
                    break;
            }
        }
    }
}

static void SdXMLCustomShapePropertyMerge( std::vector< css::beans::PropertyValue >& rPropVec,
                                    const std::vector< beans::PropertyValues >& rElement,
                                        const OUString& rElementName )
{
    if ( !rElement.empty() )
    {
        beans::PropertyValue aProp;
        aProp.Name = rElementName;
        aProp.Value <<= comphelper::containerToSequence(rElement);
        rPropVec.push_back( aProp );
    }
}

static void SdXMLCustomShapePropertyMerge( std::vector< css::beans::PropertyValue >& rPropVec,
                                    const std::vector< OUString >& rElement,
                                        const OUString& rElementName )
{
    if ( !rElement.empty() )
    {
        beans::PropertyValue aProp;
        aProp.Name = rElementName;
        aProp.Value <<= comphelper::containerToSequence(rElement);
        rPropVec.push_back( aProp );
    }
}

static void SdXMLCustomShapePropertyMerge( std::vector< css::beans::PropertyValue >& rPropVec,
                                    const std::vector< css::beans::PropertyValue >& rElement,
                                        const OUString& rElementName )
{
    if ( !rElement.empty() )
    {
        beans::PropertyValue aProp;
        aProp.Name = rElementName;
        aProp.Value <<= comphelper::containerToSequence(rElement);
        rPropVec.push_back( aProp );
    }
}

typedef std::unordered_map< OUString, sal_Int32 > EquationHashMap;

/* if rPara.Type is from type EnhancedCustomShapeParameterType::EQUATION, the name of the equation
   will be converted from OUString to index */
static void CheckAndResolveEquationParameter( css::drawing::EnhancedCustomShapeParameter& rPara, EquationHashMap* pH )
{
    if ( rPara.Type == css::drawing::EnhancedCustomShapeParameterType::EQUATION )
    {
        OUString aEquationName;
        if ( rPara.Value >>= aEquationName )
        {
            sal_Int32 nIndex = 0;
            EquationHashMap::iterator aHashIter( pH->find( aEquationName ) );
            if ( aHashIter != pH->end() )
                nIndex = (*aHashIter).second;
            rPara.Value <<= nIndex;
        }
    }
}

void XMLEnhancedCustomShapeContext::EndElement()
{
    // resolve properties that are indexing a Equation
    if ( !maEquations.empty() )
    {
        // creating hash map containing the name and index of each equation
        std::unique_ptr<EquationHashMap> pH = std::make_unique<EquationHashMap>();
        std::vector< OUString >::iterator aEquationNameIter = maEquationNames.begin();
        std::vector< OUString >::iterator aEquationNameEnd  = maEquationNames.end();
        while( aEquationNameIter != aEquationNameEnd )
        {
            (*pH)[ *aEquationNameIter ] = static_cast<sal_Int32>( aEquationNameIter - maEquationNames.begin() );
            ++aEquationNameIter;
        }

        // resolve equation
        for( auto& rEquation : maEquations )
        {
            sal_Int32 nIndexOf = 0;
            do
            {
                nIndexOf = rEquation.indexOf( '?', nIndexOf );
                if ( nIndexOf != -1 )
                {
                    OUString aEquationName;
                    if ( GetEquationName( rEquation, nIndexOf + 1, aEquationName ) )
                    {
                        // copying first characters inclusive '?'
                        OUString aNew( rEquation.copy( 0, nIndexOf + 1 ) );
                        sal_Int32 nIndex = 0;
                        EquationHashMap::iterator aHashIter( pH->find( aEquationName ) );
                        if ( aHashIter != pH->end() )
                            nIndex = (*aHashIter).second;
                        aNew += OUString::number( nIndex );
                        aNew += rEquation.copy( nIndexOf + aEquationName.getLength() + 1 );
                        rEquation = aNew;
                    }
                    nIndexOf++;
                }
            }
            while( nIndexOf != -1 );
        }

        // Path
        sal_Int32 i;
        for ( beans::PropertyValue& rPathItem : maPath )
        {
            switch( EASGet( rPathItem.Name ) )
            {
                case EAS_Coordinates :
                case EAS_GluePoints :
                {
                    uno::Sequence< css::drawing::EnhancedCustomShapeParameterPair > const & rSeq =
                        *o3tl::doAccess<uno::Sequence< css::drawing::EnhancedCustomShapeParameterPair > >(
                            rPathItem.Value);
                    for ( i = 0; i < rSeq.getLength(); i++ )
                    {
                        CheckAndResolveEquationParameter( const_cast<css::drawing::EnhancedCustomShapeParameter &>(rSeq[ i ].First), pH.get() );
                        CheckAndResolveEquationParameter( const_cast<css::drawing::EnhancedCustomShapeParameter &>(rSeq[ i ].Second), pH.get() );
                    }
                }
                break;
                case EAS_TextFrames :
                {
                    uno::Sequence< css::drawing::EnhancedCustomShapeTextFrame > const & rSeq =
                        *o3tl::doAccess<uno::Sequence< css::drawing::EnhancedCustomShapeTextFrame > >(
                            rPathItem.Value);
                    for ( i = 0; i < rSeq.getLength(); i++ )
                    {
                        CheckAndResolveEquationParameter( const_cast<css::drawing::EnhancedCustomShapeParameter &>(rSeq[ i ].TopLeft.First), pH.get() );
                        CheckAndResolveEquationParameter( const_cast<css::drawing::EnhancedCustomShapeParameter &>(rSeq[ i ].TopLeft.Second), pH.get() );
                        CheckAndResolveEquationParameter( const_cast<css::drawing::EnhancedCustomShapeParameter &>(rSeq[ i ].BottomRight.First), pH.get() );
                        CheckAndResolveEquationParameter( const_cast<css::drawing::EnhancedCustomShapeParameter &>(rSeq[ i ].BottomRight.Second), pH.get() );
                    }
                }
                break;
                default:
                    break;
            }
        }
        for ( css::beans::PropertyValues const & aHandle : maHandles )
        {
            for ( beans::PropertyValue const & propValue : aHandle )
            {
                switch( EASGet( propValue.Name ) )
                {
                    case EAS_RangeYMinimum :
                    case EAS_RangeYMaximum :
                    case EAS_RangeXMinimum :
                    case EAS_RangeXMaximum :
                    case EAS_RadiusRangeMinimum :
                    case EAS_RadiusRangeMaximum :
                    {
                        CheckAndResolveEquationParameter( const_cast<css::drawing::EnhancedCustomShapeParameter &>(*o3tl::doAccess<css::drawing::EnhancedCustomShapeParameter>(
                            propValue.Value)), pH.get() );
                    }
                    break;

                    case EAS_Position :
                    case EAS_Polar :
                    {
                        CheckAndResolveEquationParameter( const_cast<css::drawing::EnhancedCustomShapeParameter &>((*o3tl::doAccess<css::drawing::EnhancedCustomShapeParameterPair>(
                            propValue.Value)).First), pH.get() );
                        CheckAndResolveEquationParameter( const_cast<css::drawing::EnhancedCustomShapeParameter &>((*o3tl::doAccess<css::drawing::EnhancedCustomShapeParameterPair>(
                            propValue.Value)).Second), pH.get() );
                    }
                    break;
                    default:
                        break;
                }
            }
        }
    }

    SdXMLCustomShapePropertyMerge( mrCustomShapeGeometry, maExtrusion, EASGet( EAS_Extrusion ) );
    SdXMLCustomShapePropertyMerge( mrCustomShapeGeometry, maPath,      EASGet( EAS_Path ) );
    SdXMLCustomShapePropertyMerge( mrCustomShapeGeometry, maTextPath,  EASGet( EAS_TextPath ) );
    SdXMLCustomShapePropertyMerge( mrCustomShapeGeometry, maEquations, EASGet( EAS_Equations ) );
    if  ( !maHandles.empty() )
        SdXMLCustomShapePropertyMerge( mrCustomShapeGeometry, maHandles, EASGet( EAS_Handles ) );
}

SvXMLImportContextRef XMLEnhancedCustomShapeContext::CreateChildContext( sal_uInt16 nPrefix,const OUString& rLocalName,
                                                                    const uno::Reference< xml::sax::XAttributeList> & xAttrList )
{
    EnhancedCustomShapeTokenEnum aTokenEnum = EASGet( rLocalName );
    if ( aTokenEnum == EAS_equation )
    {
        sal_Int16 nLength = xAttrList->getLength();
        if ( nLength )
        {
            OUString aFormula;
            OUString aFormulaName;
            for( sal_Int16 nAttr = 0; nAttr < nLength; nAttr++ )
            {
                OUString aLocalName;
                const OUString& rValue = xAttrList->getValueByIndex( nAttr );
                /* fixme sven, this needs to be checked! sal_uInt16 nPrefix = */ GetImport().GetNamespaceMap().GetKeyByAttrName( xAttrList->getNameByIndex( nAttr ), &aLocalName );

                switch( EASGet( aLocalName ) )
                {
                    case EAS_formula :
                        aFormula = rValue;
                    break;
                    case EAS_name :
                        aFormulaName = rValue;
                    break;
                    default:
                        break;
                }
            }
            if ( !aFormulaName.isEmpty() || !aFormula.isEmpty() )
            {
                maEquations.push_back( aFormula );
                maEquationNames.push_back( aFormulaName );
            }
        }
    }
    else if ( aTokenEnum == EAS_handle )
    {
        std::vector< css::beans::PropertyValue > aHandle;
        const sal_Int16 nLength = xAttrList->getLength();
        for( sal_Int16 nAttr = 0; nAttr < nLength; nAttr++ )
        {
            OUString aLocalName;
            const OUString& rValue = xAttrList->getValueByIndex( nAttr );
            /* fixme sven, this needs to be checked! sal_uInt16 nPrefix = */ GetImport().GetNamespaceMap().GetKeyByAttrName( xAttrList->getNameByIndex( nAttr ), &aLocalName );
            switch( EASGet( aLocalName ) )
            {
                case EAS_handle_mirror_vertical :
                    GetBool( aHandle, rValue, EAS_MirroredY );
                break;
                case EAS_handle_mirror_horizontal :
                    GetBool( aHandle, rValue, EAS_MirroredX );
                break;
                case EAS_handle_switched :
                    GetBool( aHandle, rValue, EAS_Switched );
                break;
                case EAS_handle_position :
                    GetEnhancedParameterPair( aHandle, rValue, EAS_Position );
                break;
                case EAS_handle_range_x_minimum :
                    GetEnhancedParameter( aHandle, rValue, EAS_RangeXMinimum );
                break;
                case EAS_handle_range_x_maximum :
                    GetEnhancedParameter( aHandle, rValue, EAS_RangeXMaximum );
                break;
                case EAS_handle_range_y_minimum :
                    GetEnhancedParameter( aHandle, rValue, EAS_RangeYMinimum );
                break;
                case EAS_handle_range_y_maximum :
                    GetEnhancedParameter( aHandle, rValue, EAS_RangeYMaximum );
                break;
                case EAS_handle_polar :
                    GetEnhancedParameterPair( aHandle, rValue, EAS_Polar );
                break;
                case EAS_handle_radius_range_minimum :
                    GetEnhancedParameter( aHandle, rValue, EAS_RadiusRangeMinimum );
                break;
                case EAS_handle_radius_range_maximum :
                    GetEnhancedParameter( aHandle, rValue, EAS_RadiusRangeMaximum );
                break;
                default:
                    break;
            }
        }
        maHandles.push_back( comphelper::containerToSequence(aHandle) );
    }
    return SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
