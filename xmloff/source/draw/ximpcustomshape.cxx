/*************************************************************************
 *
 *  $RCSfile: ximpcustomshape.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-15 12:36:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_XIMPCUSTOMSHAPE_HXX_
#include "ximpcustomshape.hxx"
#endif

#ifndef _XIMPSHAPE_HXX
#include "ximpshap.hxx"
#endif

#ifndef _XMLOFF_XMLEHELP_HXX
#include "xmlehelp.hxx"
#endif

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _ENHANCED_CUSTOMSHAPE_TOKEN_HXX
#include "EnhancedCustomShapeToken.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_XMLTKMAP_HXX
#include "xmltkmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XEXPTRANSFORM_HXX
#include "xexptran.hxx"
#endif

#ifndef _XMLOFF_XMLERROR_HXX
#include "xmlerror.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_DIRECTION3D_HPP_
#include <com/sun/star/drawing/Direction3D.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEOPERATION_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeOperation.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEPARAMETER_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeParameter.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEPARAMETERPAIR_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEPARAMETERTYPE_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPETEXTFRAME_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEADJUSTMENTVALUE_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPESEGMENT_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPESEGMENTCOMMAND_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPETEXTPATHMODE_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeTextPathMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_PROJECTIONMODE_HPP_
#include <com/sun/star/drawing/ProjectionMode.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::xmloff::token;
using namespace ::xmloff::EnhancedCustomShapeToken;

TYPEINIT1( XMLEnhancedCustomShapeContext, SvXMLImportContext );

XMLEnhancedCustomShapeContext::XMLEnhancedCustomShapeContext( SvXMLImport& rImport, sal_uInt16 nPrefix, const rtl::OUString& rLocalName,
                                                            std::vector< com::sun::star::beans::PropertyValue >& rCustomShapeGeometry ) :
        mrCustomShapeGeometry( rCustomShapeGeometry ),
        mrUnitConverter( rImport.GetMM100UnitConverter() ),
        SvXMLImportContext( rImport, nPrefix, rLocalName )
{
}

const SvXMLEnumMapEntry aXML_GluePointEnumMap[] =
{
    { XML_NONE,         0 },
    { XML_SEGMENTS,     1 },
    { XML_NONE,         2 },
    { XML_RECTANGLE,    3 },
    { XML_TOKEN_INVALID, 0 }
};
void GetBool( std::vector< com::sun::star::beans::PropertyValue >& rDest,
                        const rtl::OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    sal_Bool bAttrBool;
    if ( SvXMLUnitConverter::convertBool( bAttrBool, rValue ) )
    {
        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= bAttrBool;
        rDest.push_back( aProp );
    }
}

void GetInt32( std::vector< com::sun::star::beans::PropertyValue >& rDest,
                        const rtl::OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    sal_Int32 nAttrNumber;
    if ( SvXMLUnitConverter::convertNumber( nAttrNumber, rValue ) )
    {
        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= nAttrNumber;
        rDest.push_back( aProp );
    }
}

void GetDouble( std::vector< com::sun::star::beans::PropertyValue >& rDest,
                        const rtl::OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    double fAttrDouble;
    if ( SvXMLUnitConverter::convertDouble( fAttrDouble, rValue ) )
    {
        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= fAttrDouble;
        rDest.push_back( aProp );
    }
}

void GetDistance( std::vector< com::sun::star::beans::PropertyValue >& rDest,
                        const rtl::OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    double fAttrDouble;
    MapUnit eSrcUnit( SvXMLExportHelper::GetUnitFromString( rValue, MAP_100TH_MM ) );
    if ( SvXMLUnitConverter::convertDouble( fAttrDouble, rValue, eSrcUnit, MAP_100TH_MM ) )
    {
        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= fAttrDouble;
        rDest.push_back( aProp );
    }
}

void GetString( std::vector< com::sun::star::beans::PropertyValue >& rDest,
                        const rtl::OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    beans::PropertyValue aProp;
    aProp.Name = EASGet( eDestProp );
    aProp.Value <<= rValue;
    rDest.push_back( aProp );
}

void GetEnum( std::vector< com::sun::star::beans::PropertyValue >& rDest,
                         const rtl::OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp,
                        const SvXMLEnumMapEntry& rMap )
{
    USHORT eKind;
    if( SvXMLUnitConverter::convertEnum( eKind, rValue, &rMap ) )
    {
        sal_Int16 nEnum = (sal_Int16)eKind;
        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= nEnum;
        rDest.push_back( aProp );
    }
}

void GetDoublePercentage( std::vector< com::sun::star::beans::PropertyValue >& rDest,
                         const rtl::OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    MapUnit eSrcUnit = SvXMLExportHelper::GetUnitFromString( rValue, MAP_100TH_MM );
    if ( eSrcUnit == MAP_RELATIVE )
    {
        rtl_math_ConversionStatus eStatus;
        double fAttrDouble = ::rtl::math::stringToDouble( rValue,
            (sal_Unicode)('.'), (sal_Unicode)(','), &eStatus, NULL );
        if ( eStatus == rtl_math_ConversionStatus_Ok )
        {
            beans::PropertyValue aProp;
            aProp.Name = EASGet( eDestProp );
            aProp.Value <<= fAttrDouble;
            rDest.push_back( aProp );
        }
    }
}

void GetVector3D( std::vector< com::sun::star::beans::PropertyValue >& rDest,
                         const rtl::OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    Vector3D aVector3D;
    if ( SvXMLUnitConverter::convertVector3D( aVector3D, rValue ) )
    {
        drawing::Direction3D aDirection3D( aVector3D.X(), aVector3D.Y(), aVector3D.Z() );
        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= aDirection3D;
        rDest.push_back( aProp );
    }
}

sal_Bool GetNextParameter( drafts::com::sun::star::drawing::EnhancedCustomShapeParameter& rParameter, sal_Int32& nIndex, const rtl::OUString& rParaString )
{
    if ( nIndex >= rParaString.getLength() )
        return sal_False;

    sal_Bool bValid = sal_True;
    sal_Bool bNumberRequired = sal_True;
    sal_Bool bMustBePositiveWholeNumbered = sal_False;

    rParameter.Type = drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::NORMAL;
    if ( rParaString[ nIndex ] == (sal_Unicode)'#' )
    {
        rParameter.Type = drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::ADJUSTMENT;
        bMustBePositiveWholeNumbered = sal_True;
        nIndex++;
    }
    else if ( rParaString[ nIndex ] == (sal_Unicode)'@' )
    {
        rParameter.Type = drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::EQUATION;
        bMustBePositiveWholeNumbered = sal_True;
        nIndex++;
    }
    else if ( rParaString[ nIndex ] > (sal_Unicode)'9' )
    {
        bNumberRequired = sal_False;
        if ( rParaString.matchIgnoreAsciiCaseAsciiL( "left", 4, nIndex ) )
        {
            rParameter.Type = drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::LEFT;
            nIndex += 4;
        }
        else if ( rParaString.matchIgnoreAsciiCaseAsciiL( "top", 3, nIndex ) )
        {
            rParameter.Type = drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::TOP;
            nIndex += 3;
        }
        else if ( rParaString.matchIgnoreAsciiCaseAsciiL( "right", 5, nIndex ) )
        {
            rParameter.Type = drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::RIGHT;
            nIndex += 5;
        }
        else if ( rParaString.matchIgnoreAsciiCaseAsciiL( "bottom", 6, nIndex ) )
        {
            rParameter.Type = drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::BOTTOM;
            nIndex += 6;
        }
        else if ( rParaString.matchIgnoreAsciiCaseAsciiL( "center", 6, nIndex ) )
        {
            rParameter.Type = drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::CENTER;
            nIndex += 6;
        }
        else
            bValid = sal_False;
    }
    if ( bValid )
    {
        if ( bNumberRequired )
        {
            sal_Int32 nStartIndex = nIndex;

            sal_Bool bM = sal_False;    // set if the value is negative
            sal_Bool bE = sal_False;    // set if a double is including a "E" statement
            sal_Bool bEM = sal_False;   // set if a double is including a "E-"statement
            sal_Bool bDot = sal_False;  // set if there is a dot included
            sal_Bool bEnd = sal_False;  // set for each value that can not be part of a double/integer

            while( ( nIndex < rParaString.getLength() ) && bValid )
            {
                switch( rParaString[ nIndex ] )
                {
                    case '.' :
                    {
                        if ( bMustBePositiveWholeNumbered )
                            bValid = sal_False;
                        else
                        {
                            if ( bDot )
                                bValid = sal_False;
                            else
                                bDot = sal_True;
                        }
                    }
                    break;
                    case '-' :
                    {
                        if ( bMustBePositiveWholeNumbered )
                            bValid = sal_False;
                        else
                        {
                            if ( nStartIndex == nIndex )
                                bM = sal_True;
                            else if ( bE )
                                bEM = sal_True;
                            else
                                bValid = sal_False;
                        }
                    }
                    break;

                    case 'e' :
                    case 'E' :
                    {
                        if ( bMustBePositiveWholeNumbered )
                            bEnd = sal_True;
                        else
                        {
                            if ( !bE )
                                bE = sal_True;
                            else
                                bEnd = sal_True;
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
                    break;
                    default:
                        bEnd = sal_True;
                }
                if ( !bEnd )
                    nIndex++;
                else
                    break;
            }
            if ( nIndex == nStartIndex )
                bValid = sal_False;
            if ( bValid )
            {
                rtl::OUString aNumber( rParaString.copy( nStartIndex, nIndex - nStartIndex ) );
                if ( bE || bDot )
                {
                    double fAttrDouble;
                    if ( SvXMLUnitConverter::convertDouble( fAttrDouble, aNumber ) )
                        rParameter.Value <<= fAttrDouble;
                    else
                        bValid = sal_False;
                }
                else
                {
                    sal_Int32 nValue;
                    if ( SvXMLUnitConverter::convertNumber( nValue, aNumber ) )
                        rParameter.Value <<= nValue;
                    else
                        bValid = sal_False;
                }
            }
        }
    }
    if ( bValid )
    {   // skipping white spaces
        while( ( nIndex < rParaString.getLength() ) && rParaString[ nIndex ] == (sal_Unicode)' ' )
            nIndex++;
    }
    return bValid;
}

void GetPosition3D( std::vector< com::sun::star::beans::PropertyValue >& rDest,                     // e.g. draw:extrusion-viewpoint
                        const rtl::OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp,
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

void GetDoubleSequence( std::vector< com::sun::star::beans::PropertyValue >& rDest,                 // e.g. draw:glue-point-leaving-directions
                        const rtl::OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    std::vector< double > vDirection;
    sal_Int32 nIndex = 0;
    do
    {
        double fAttrDouble;
        rtl::OUString aToken( rValue.getToken( 0, ',', nIndex ) );
        if ( !SvXMLUnitConverter::convertDouble( fAttrDouble, aToken ) )
            break;
        else
            vDirection.push_back( fAttrDouble );
    }
    while ( nIndex >= 0 );

    if ( vDirection.size() )
    {
        uno::Sequence< double > aDirectionsSeq( vDirection.size() );
        std::vector< double >::const_iterator aIter = vDirection.begin();
        std::vector< double >::const_iterator aEnd = vDirection.end();
        double* pValues = aDirectionsSeq.getArray();

        while ( aIter != aEnd )
            *pValues++ = *aIter++;

        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= aDirectionsSeq;
        rDest.push_back( aProp );
    }
}

void GetEnhancedParameter( std::vector< com::sun::star::beans::PropertyValue >& rDest,              // e.g. draw:handle-position
                        const rtl::OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    sal_Int32 nIndex = 0;
    drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aParameter;
    if ( GetNextParameter( aParameter, nIndex, rValue ) )
    {
        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= aParameter;
        rDest.push_back( aProp );
    }
}

void GetEnhancedParameterPair( std::vector< com::sun::star::beans::PropertyValue >& rDest,          // e.g. draw:handle-position
                        const rtl::OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    sal_Int32 nIndex = 0;
    drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair aParameterPair;
    if ( GetNextParameter( aParameterPair.First, nIndex, rValue )
        && GetNextParameter( aParameterPair.Second, nIndex, rValue ) )
    {
        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= aParameterPair;
        rDest.push_back( aProp );
    }
}

void GetEnhancedParameterPairSequence( std::vector< com::sun::star::beans::PropertyValue >& rDest,      // e.g. draw:glue-points
                        const rtl::OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair > vParameter;
    drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair aParameter;

    sal_Int32 nIndex = 0;

    while ( GetNextParameter( aParameter.First, nIndex, rValue )
            && GetNextParameter( aParameter.Second, nIndex, rValue ) )
    {
        vParameter.push_back( aParameter );
    }
    if ( vParameter.size() )
    {
        uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair > aParameterSeq( vParameter.size() );
        std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair >::const_iterator aIter = vParameter.begin();
        std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair >::const_iterator aEnd = vParameter.end();
        drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair* pValues = aParameterSeq.getArray();

        while ( aIter != aEnd )
            *pValues++ = *aIter++;

        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= aParameterSeq;
        rDest.push_back( aProp );
    }
}

void GetEnhancedRectangleSequence( std::vector< com::sun::star::beans::PropertyValue >& rDest,      // e.g. draw:text-frames
                        const rtl::OUString& rValue, const EnhancedCustomShapeTokenEnum eDestProp )
{
    std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeTextFrame > vTextFrame;
    drafts::com::sun::star::drawing::EnhancedCustomShapeTextFrame aParameter;

    sal_Int32 nIndex = 0;

    while ( GetNextParameter( aParameter.TopLeft.First, nIndex, rValue )
            && GetNextParameter( aParameter.TopLeft.Second, nIndex, rValue )
            && GetNextParameter( aParameter.BottomRight.First, nIndex, rValue )
            && GetNextParameter( aParameter.BottomRight.Second, nIndex, rValue ) )
    {
        vTextFrame.push_back( aParameter );
    }
    if ( vTextFrame.size() )
    {
        uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeTextFrame > aTextFrameSeq( vTextFrame.size() );
        std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeTextFrame >::const_iterator aIter = vTextFrame.begin();
        std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeTextFrame >::const_iterator aEnd = vTextFrame.end();
        drafts::com::sun::star::drawing::EnhancedCustomShapeTextFrame* pValues = aTextFrameSeq.getArray();

        while ( aIter != aEnd )
            *pValues++ = *aIter++;

        beans::PropertyValue aProp;
        aProp.Name = EASGet( eDestProp );
        aProp.Value <<= aTextFrameSeq;
        rDest.push_back( aProp );
    }
}

void GetEnhancedPath( std::vector< com::sun::star::beans::PropertyValue >& rDest,                   // e.g. draw:enhanced-path
                        const rtl::OUString& rValue )
{
    std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair >    vCoordinates;
    std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeSegment >      vSegments;

    sal_Int32 nIndex = 0;
    sal_Int32 nParameterCount = 0;

    sal_Int32 nParametersNeeded = 1;
    sal_Int16 nLatestSegmentCommand = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::MOVETO;

    sal_Bool bValid = sal_True;

    while( bValid && ( nIndex < rValue.getLength() ) )
    {
        switch( rValue[ nIndex ] )
        {
            case 'M' :
            {
                nLatestSegmentCommand = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::MOVETO;
                nParametersNeeded = 1;
                nIndex++;
            }
            break;
            case 'L' :
            {
                nLatestSegmentCommand = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::LINETO;
                nParametersNeeded = 1;
                nIndex++;
            }
            break;
            case 'C' :
            {
                nLatestSegmentCommand = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::CURVETO;
                nParametersNeeded = 3;
                nIndex++;
            }
            break;
            case 'Z' :
            {
                nLatestSegmentCommand = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::CLOSESUBPATH;
                nParametersNeeded = 0;
                nIndex++;
            }
            break;
            case 'N' :
            {
                nLatestSegmentCommand = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ENDSUBPATH;
                nParametersNeeded = 0;
                nIndex++;
            }
            break;
            case 'F' :
            {
                nLatestSegmentCommand = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::NOFILL;
                nParametersNeeded = 0;
                nIndex++;
            }
            break;
            case 'S' :
            {
                nLatestSegmentCommand = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::NOSTROKE;
                nParametersNeeded = 0;
                nIndex++;
            }
            break;
            case 'T' :
            {
                nLatestSegmentCommand = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ANGLEELLIPSETO;
                nParametersNeeded = 3;
                nIndex++;
            }
            break;
            case 'U' :
            {
                nLatestSegmentCommand = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ANGLEELLIPSE;
                nParametersNeeded = 3;
                nIndex++;
            }
            break;
            case 'A' :
            {
                nLatestSegmentCommand = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ARCTO;
                nParametersNeeded = 4;
                nIndex++;
            }
            break;
            case 'B' :
            {
                nLatestSegmentCommand = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ARC;
                nParametersNeeded = 4;
                nIndex++;
            }
            break;
            case 'W' :
            {
                nLatestSegmentCommand = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARCTO;
                nParametersNeeded = 4;
                nIndex++;
            }
            break;
            case 'V' :
            {
                nLatestSegmentCommand = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARC;
                nParametersNeeded = 4;
                nIndex++;
            }
            break;
            case 'X' :
            {
                nLatestSegmentCommand = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTX;
                nParametersNeeded = 1;
                nIndex++;
            }
            break;
            case 'Y' :
            {
                nLatestSegmentCommand = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTY;
                nParametersNeeded = 1;
                nIndex++;
            }
            break;
            case 'Q' :
            {
                nLatestSegmentCommand = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::QUADRATICCURVETO;
                nParametersNeeded = 2;
                nIndex++;
            }
            break;
            case ' ' :
            {
                nIndex++;
            }
            break;

            case '#' :
            case '@' :
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
            {
                drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair aPair;
                if ( GetNextParameter( aPair.First, nIndex, rValue ) &&
                        GetNextParameter( aPair.Second, nIndex, rValue ) )
                {
                    vCoordinates.push_back( aPair );
                    nParameterCount++;
                }
                else
                    bValid = sal_False;
            }
            break;
            default:
                nIndex++;
            break;
        }
        if ( nParameterCount >= nParametersNeeded )
        {
            // check if the last command is identical,
            // if so, we just need to increment the count
            if ( vSegments.size() && ( vSegments[ vSegments.size() - 1 ].Command == nLatestSegmentCommand ) )
                vSegments[ vSegments.size() -1 ].Count++;
            else
            {
                drafts::com::sun::star::drawing::EnhancedCustomShapeSegment aSegment;
                aSegment.Command = nLatestSegmentCommand;
                aSegment.Count = 1;
                vSegments.push_back( aSegment );
            }
            nParameterCount = 0;
        }
    }
    // adding the Coordinates property
    uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair > seqCoordinates( vCoordinates.size() );
    std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair >::const_iterator aCoordinatesIter = vCoordinates.begin();
    std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair >::const_iterator aCoordinatesEnd = vCoordinates.end();
    drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair* pCoordinateValues = seqCoordinates.getArray();

    while ( aCoordinatesIter != aCoordinatesEnd )
        *pCoordinateValues++ = *aCoordinatesIter++;

    beans::PropertyValue aProp;
    aProp.Name = EASGet( EAS_Coordinates );
    aProp.Value <<= seqCoordinates;
    rDest.push_back( aProp );


    // adding the Segments property
    uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeSegment > seqSegments( vSegments.size() );
    std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeSegment >::const_iterator aSegmentsIter = vSegments.begin();
    std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeSegment >::const_iterator aSegmentsEnd = vSegments.end();
    drafts::com::sun::star::drawing::EnhancedCustomShapeSegment* pSegmentValues = seqSegments.getArray();

    while ( aSegmentsIter != aSegmentsEnd )
        *pSegmentValues++ = *aSegmentsIter++;

    aProp.Name = EASGet( EAS_Segments );
    aProp.Value <<= seqSegments;
    rDest.push_back( aProp );
}

void GetEnhancedCustomShapeEquation( std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeEquation >& rDest,        // draw:formula
                        const rtl::OUString& rValue )
{
    drafts::com::sun::star::drawing::EnhancedCustomShapeEquation aEquation;

    rtl::OUString aString( rValue.trim() );     // removing white spaces
    sal_Int32 i = aString.indexOf( (sal_Char)' ', 0 );
    if ( ( !i ) || ( i == -1 ) )
        aEquation.Operation = drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::UNKNOWN;
    else
    {
        rtl::OUString aOperation( aString.copy( 0, i ) );
        rtl::OUString aParaString( aString.copy( i + 1 ).trim() );

        aEquation.Operation = drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::UNKNOWN;
        switch( aOperation.getLength() )
        {
            case 2 :
            {
                if ( aOperation.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii( "if" ) ) )
                    aEquation.Operation = drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::IF;
            }
            break;
            case 3 :
            {
                if ( aOperation.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii( "sum" ) ) )
                    aEquation.Operation = drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::SUM;
                else if ( aOperation.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii( "mid" ) ) )
                    aEquation.Operation = drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::MID;
                else if ( aOperation.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii( "abs" ) ) )
                    aEquation.Operation = drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::ABS;
                else if ( aOperation.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii( "min" ) ) )
                    aEquation.Operation = drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::MIN;
                else if ( aOperation.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii( "max" ) ) )
                    aEquation.Operation = drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::MAX;
                else if ( aOperation.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii( "mod" ) ) )
                    aEquation.Operation = drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::MOD;
                else if ( aOperation.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii( "sin" ) ) )
                    aEquation.Operation = drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::SIN;
                else if ( aOperation.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii( "cos" ) ) )
                    aEquation.Operation = drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::COS;
                else if ( aOperation.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii( "tan" ) ) )
                    aEquation.Operation = drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::TAN;
            }
            break;
            case 4 :
            {
                if ( aOperation.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii( "sqrt" ) ) )
                    aEquation.Operation = drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::SQRT;
            }
            break;
            case 5 :
            {
                if ( aOperation.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii( "atan2" ) ) )
                    aEquation.Operation = drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::ATAN2;

            }
            break;
            case 7 :
            {
                if ( aOperation.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii( "product" ) ) )
                    aEquation.Operation = drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::PROD;
                else if ( aOperation.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii( "ellipse" ) ) )
                    aEquation.Operation = drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::ELLIPSE;
            }
            break;
            case 8 :
            {
                if ( aOperation.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii( "cosatan2" ) ) )
                    aEquation.Operation = drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::COSATAN2;
                else if ( aOperation.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii( "sinatan2" ) ) )
                    aEquation.Operation = drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::SINATAN2;
                else if ( aOperation.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii( "sumangle" ) ) )
                    aEquation.Operation = drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::SUMANGLE;
            }
            break;
        }
        i = 0;
        std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeParameter > aParameters;
        drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aParameter;

        while( GetNextParameter( aParameter, i, aParaString ) )
        {
            aParameters.push_back( aParameter );
        }
        if ( aParameters.size() )
        {
            uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeParameter > aParaPropSeq( aParameters.size() );
            std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeParameter >::const_iterator aIter = aParameters.begin();
            std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeParameter >::const_iterator aEnd = aParameters.end();
            drafts::com::sun::star::drawing::EnhancedCustomShapeParameter* pValues = aParaPropSeq.getArray();

            while ( aIter != aEnd )
                *pValues++ = *aIter++;

            aEquation.Parameters = aParaPropSeq;
        }
    }
    rDest.push_back( aEquation );
}

void GetAdjustmentValues( std::vector< com::sun::star::beans::PropertyValue >& rDest,               // draw:adjustments
                        const rtl::OUString& rValue )
{
    std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue > vAdjustmentValue;
    sal_Int32 nIndex = 0;
    do
    {
        drafts::com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue aAdj;
        rtl::OUString aToken( rValue.getToken( 0, ',', nIndex ) );

        // the adjustment value can be double or integer, we use GetNextParameter
        // to determine the correct type
        sal_Int32 nIndex = 0;
        drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aParameter;
        if ( GetNextParameter( aParameter, nIndex, aToken )
            && ( aParameter.Type == drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::NORMAL ) )
        {
            aAdj.Value <<= aParameter.Value;
            aAdj.State = beans::PropertyState_DIRECT_VALUE;
        }
        else
            aAdj.State = beans::PropertyState_DEFAULT_VALUE;

        vAdjustmentValue.push_back( aAdj );
    }
    while ( nIndex >= 0 );

    sal_Int32 nAdjustmentValues = vAdjustmentValue.size();
    if ( nAdjustmentValues )
    {
        uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue > aAdjustmentValues( nAdjustmentValues );
        std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue >::const_iterator aIter = vAdjustmentValue.begin();
        std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue >::const_iterator aEnd = vAdjustmentValue.end();
        drafts::com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue* pValues = aAdjustmentValues.getArray();

        while ( aIter != aEnd )
            *pValues++ = *aIter++;

        beans::PropertyValue aProp;
        aProp.Name = EASGet( EAS_AdjustmentValues );
        aProp.Value <<= aAdjustmentValues;
        rDest.push_back( aProp );
    }
}

void XMLEnhancedCustomShapeContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    sal_Int16 nLength = xAttrList->getLength();
    if ( nLength )
    {
        sal_Int32               nAttrNumber;

        sal_Bool    bStretchPointUsed = sal_False;
        awt::Point  aStretchPoint( 0, 0 );

        for( sal_Int16 nAttr = 0; nAttr < nLength; nAttr++ )
        {
            rtl::OUString aLocalName;
            const rtl::OUString& rValue = xAttrList->getValueByIndex( nAttr );
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( xAttrList->getNameByIndex( nAttr ), &aLocalName );

            switch( EASGet( aLocalName ) )
            {
                case EAS_predefined_type :
                    GetString( mrCustomShapeGeometry, rValue, EAS_PredefinedType );
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
                    drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair aParameterPair;
                    drafts::com::sun::star::drawing::EnhancedCustomShapeParameter& rDepth = aParameterPair.First;
                    drafts::com::sun::star::drawing::EnhancedCustomShapeParameter& rFraction = aParameterPair.Second;
                    if ( GetNextParameter( rDepth, nIndex, rValue ) )
                    {
                        // try to catch the unit for the depth
                        MapUnit eSrcUnit( SvXMLExportHelper::GetUnitFromString( rValue, MAP_100TH_MM ) );

                        rtl::OUStringBuffer aUnitStr;
                        double fFactor = SvXMLExportHelper::GetConversionFactor( aUnitStr, MAP_100TH_MM, eSrcUnit );
                        if ( ( fFactor != 1.0 ) && ( fFactor != 0.0 ) )
                        {
                            double fDepth;
                            if ( rDepth.Value >>= fDepth )
                            {
                                fDepth /= fFactor;
                                rDepth.Value <<= fDepth;
                            }
                        }
                        if ( rValue.matchIgnoreAsciiCase( rtl::OUString( aUnitStr ), nIndex ) )
                            nIndex += aUnitStr.getLength();

                        // skipping white spaces
                        while( ( nIndex < rValue.getLength() ) && rValue[ nIndex ] == (sal_Unicode)' ' )
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
                    GetVector3D( maExtrusion, rValue, EAS_FirstLightDirection );
                break;
                case EAS_extrusion_second_light_direction :
                    GetVector3D( maExtrusion, rValue, EAS_SecondLightDirection );
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
                    GetVector3D( maExtrusion, rValue, EAS_RotationCenter );
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
                    if ( SvXMLUnitConverter::convertNumber( nAttrNumber, rValue ) )
                    {
                        bStretchPointUsed = sal_True;
                        aStretchPoint.X = nAttrNumber;
                    }
                }
                break;
                case EAS_path_stretchpoint_y :
                {
                    if ( SvXMLUnitConverter::convertNumber( nAttrNumber, rValue ) )
                    {
                        bStretchPointUsed = sal_True;
                        aStretchPoint.Y = nAttrNumber;
                    }
                }
                break;
                case EAS_text_frames :
                    GetEnhancedRectangleSequence( maPath, rValue, EAS_TextFrames );
                break;
                case EAS_glue_points :
                    GetEnhancedParameterPairSequence( maPath, rValue, EAS_GluePoints );
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
                    drafts::com::sun::star::drawing::EnhancedCustomShapeTextPathMode eTextPathMode( drafts::com::sun::star::drawing::EnhancedCustomShapeTextPathMode_NORMAL );
                    if( IsXMLToken( rValue, XML_PATH ) )
                        eTextPathMode = drafts::com::sun::star::drawing::EnhancedCustomShapeTextPathMode_PATH;
                    else if ( IsXMLToken( rValue, XML_SHAPE ) )
                        eTextPathMode = drafts::com::sun::star::drawing::EnhancedCustomShapeTextPathMode_SHAPE;

                    beans::PropertyValue aProp;
                    aProp.Name = EASGet( EAS_TextPathMode );
                    aProp.Value <<= eTextPathMode;
                    maTextPath.push_back( aProp );
                }
                break;
                case EAS_text_path_scale_x :
                    GetBool( maTextPath, rValue, EAS_ScaleX );
                break;
                case EAS_text_path_same_letter_heights :
                    GetBool( maTextPath, rValue, EAS_SameLetterHeights );
                break;
                case EAS_adjustments :
                    GetAdjustmentValues( mrCustomShapeGeometry, rValue );
                break;
            }
        }
        if ( bStretchPointUsed )
        {
            beans::PropertyValue aProp;
            aProp.Name = EASGet( EAS_StretchPoint );
            aProp.Value <<= aStretchPoint;
            maPath.push_back( aProp );
        }
    }
}

void SdXMLCustomShapePropertyMerge( std::vector< com::sun::star::beans::PropertyValue >& rPropVec,
                                    const std::vector< beans::PropertyValues >& rElement,
                                        const rtl::OUString& rElementName )
{
    if ( rElement.size() )
    {
        uno::Sequence< beans::PropertyValues > aPropSeq( rElement.size() );
        std::vector< beans::PropertyValues >::const_iterator aIter = rElement.begin();
        std::vector< beans::PropertyValues >::const_iterator aEnd = rElement.end();
        beans::PropertyValues* pValues = aPropSeq.getArray();

        while ( aIter != aEnd )
            *pValues++ = *aIter++;

        beans::PropertyValue aProp;
        aProp.Name = rElementName;
        aProp.Value <<= aPropSeq;
        rPropVec.push_back( aProp );
    }
}

void SdXMLCustomShapePropertyMerge( std::vector< com::sun::star::beans::PropertyValue >& rPropVec,
                                    const std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeEquation >& rElement,
                                        const rtl::OUString& rElementName )
{
    if ( rElement.size() )
    {
        uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeEquation > aPropSeq( rElement.size() );
        std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeEquation >::const_iterator aIter = rElement.begin();
        std::vector< drafts::com::sun::star::drawing::EnhancedCustomShapeEquation >::const_iterator aEnd = rElement.end();
        drafts::com::sun::star::drawing::EnhancedCustomShapeEquation* pValues = aPropSeq.getArray();

        while ( aIter != aEnd )
            *pValues++ = *aIter++;

        beans::PropertyValue aProp;
        aProp.Name = rElementName;
        aProp.Value <<= aPropSeq;
        rPropVec.push_back( aProp );
    }
}

void SdXMLCustomShapePropertyMerge( std::vector< com::sun::star::beans::PropertyValue >& rPropVec,
                                    const std::vector< com::sun::star::beans::PropertyValue >& rElement,
                                        const rtl::OUString& rElementName )
{
    if ( rElement.size() )
    {
        uno::Sequence< beans::PropertyValue > aPropSeq( rElement.size() );
        std::vector< beans::PropertyValue >::const_iterator aIter = rElement.begin();
        std::vector< beans::PropertyValue >::const_iterator aEnd = rElement.end();
        beans::PropertyValue* pValues = aPropSeq.getArray();

        while ( aIter != aEnd )
            *pValues++ = *aIter++;

        beans::PropertyValue aProp;
        aProp.Name = rElementName;
        aProp.Value <<= aPropSeq;
        rPropVec.push_back( aProp );
    }
}

void XMLEnhancedCustomShapeContext::EndElement()
{
    SdXMLCustomShapePropertyMerge( mrCustomShapeGeometry, maExtrusion, EASGet( EAS_Extrusion ) );
    SdXMLCustomShapePropertyMerge( mrCustomShapeGeometry, maPath,      EASGet( EAS_Path ) );
    SdXMLCustomShapePropertyMerge( mrCustomShapeGeometry, maTextPath,  EASGet( EAS_TextPath ) );
    SdXMLCustomShapePropertyMerge( mrCustomShapeGeometry, maEquations, EASGet( EAS_Equations ) );
    if  ( maHandles.size() )
        SdXMLCustomShapePropertyMerge( mrCustomShapeGeometry, maHandles, EASGet( EAS_Handles ) );
}

SvXMLImportContext* XMLEnhancedCustomShapeContext::CreateChildContext( USHORT nPrefix,const rtl::OUString& rLocalName,
                                                                    const uno::Reference< xml::sax::XAttributeList> & xAttrList )
{
    EnhancedCustomShapeTokenEnum aTokenEnum = EASGet( rLocalName );
    if ( aTokenEnum == EAS_equation )
    {
        sal_Int16 nLength = xAttrList->getLength();
        if ( nLength )
        {
            for( sal_Int16 nAttr = 0; nAttr < nLength; nAttr++ )
            {
                rtl::OUString aLocalName;
                const rtl::OUString& rValue = xAttrList->getValueByIndex( nAttr );
                sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( xAttrList->getNameByIndex( nAttr ), &aLocalName );
                if ( EASGet( aLocalName ) == EAS_formula )
                    GetEnhancedCustomShapeEquation( maEquations, rValue );
            }
        }
    }
    else if ( aTokenEnum == EAS_handle )
    {
        std::vector< com::sun::star::beans::PropertyValue > aHandle;
        for( sal_Int16 nAttr = 0; nAttr < xAttrList->getLength(); nAttr++ )
        {
            rtl::OUString aLocalName;
            const rtl::OUString& rValue = xAttrList->getValueByIndex( nAttr );
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( xAttrList->getNameByIndex( nAttr ), &aLocalName );
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
            }
        }
        beans::PropertyValues aPropSeq( aHandle.size() );
        std::vector< beans::PropertyValue >::const_iterator aIter = aHandle.begin();
        std::vector< beans::PropertyValue >::const_iterator aEnd = aHandle.end();
        beans::PropertyValue* pValues = aPropSeq.getArray();

        while ( aIter != aEnd )
            *pValues++ = *aIter++;

        maHandles.push_back( aPropSeq );
    }
    return SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );
}


