/*************************************************************************
 *
 *  $RCSfile: shapeexport4.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-04-02 13:53:10 $
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

#ifndef _B3D_HMATRIX_HXX
#include <goodies/hmatrix.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX_HPP_
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONSHAPE3D_HPP_
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_PROJECTIONMODE_HPP_
#include <com/sun/star/drawing/ProjectionMode.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_SHADEMODE_HPP_
#include <com/sun/star/drawing/ShadeMode.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_DIRECTION3D_HPP_
#include <com/sun/star/drawing/Direction3D.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_POSITION3D_HPP_
#include <com/sun/star/drawing/Position3D.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_CAMERAGEOMETRY_HPP_
#include <com/sun/star/drawing/CameraGeometry.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_DOUBLESEQUENCE_HPP_
#include <com/sun/star/drawing/DoubleSequence.hpp>
#endif

#ifndef _XMLOFF_SHAPEEXPORT_HXX
#include "shapeexport.hxx"
#endif

#ifndef _SDPROPLS_HXX
#include "sdpropls.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XEXPTRANSFORM_HXX
#include "xexptran.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _ENHANCED_CUSTOMSHAPE_TOKEN_HXX
#include "EnhancedCustomShapeToken.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XIDENTIFIERCONTAINER_HPP_
#include <com/sun/star/container/XIdentifierContainer.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPECALLOUTDROP_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeCalloutDrop.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPECALLOUTTYPE_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeCalloutType.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEEXTRUSIONPLANE_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeExtrusionPlane.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEEXTRUSIONRENDERMODE_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeExtrusionRenderMode.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEEQUATION_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeEquation.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEPARAMETERTYPE_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEOPERATION_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeOperation.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEPARAMETERPARI_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEGLUEPOINTTYPE_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeGluePointType.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPETEXTFRAME_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPESEGMENT_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPESEGMENTCOMMAND_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEADJUSTMENTVALUE_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#include "xmlnmspe.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::xmloff::token;
using namespace ::xmloff::EnhancedCustomShapeToken;

//////////////////////////////////////////////////////////////////////////////

void ExportParameter( rtl::OUStringBuffer& rStrBuffer, const drafts::com::sun::star::drawing::EnhancedCustomShapeParameter& rParameter )
{
    if ( rStrBuffer.getLength() )
        rStrBuffer.append( (sal_Unicode)' ' );
    if ( rParameter.Value.getValueTypeClass() == uno::TypeClass_DOUBLE )
    {
        double fNumber = 0.0;
        rParameter.Value >>= fNumber;
        ::rtl::math::doubleToUStringBuffer( rStrBuffer, fNumber, rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max, '.', sal_True );
    }
    else
    {
        sal_Int32 nValue = 0;
        rParameter.Value >>= nValue;

        switch( rParameter.Type )
        {
            case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::EQUATION :
            {
                rStrBuffer.append( (sal_Unicode)'@' );
                rStrBuffer.append( rtl::OUString::valueOf( nValue ) );
            }
            break;

            case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::ADJUSTMENT :
            {
                rStrBuffer.append( (sal_Unicode)'#' );
                rStrBuffer.append( rtl::OUString::valueOf( nValue ) );
            }
            break;

            case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::BOTTOM :
                rStrBuffer.append( GetXMLToken( XML_BOTTOM ) ); break;
            case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::RIGHT :
                rStrBuffer.append( GetXMLToken( XML_RIGHT ) ); break;
            case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::TOP :
                rStrBuffer.append( GetXMLToken( XML_TOP ) ); break;
            case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::LEFT :
                rStrBuffer.append( GetXMLToken( XML_LEFT ) ); break;
            default :
                rStrBuffer.append( rtl::OUString::valueOf( nValue ) );
        }
    }
}

void ImpExportEquations( SvXMLExport& rExport, const uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeEquation >& rEquations )
{
    sal_Int32 i, j;
    sal_uInt16 nElements = (sal_uInt16)rEquations.getLength();
    if ( nElements )
    {
        rtl::OUString       aStr;
        rtl::OUStringBuffer aStrBuffer;

        for ( i = 0; i < nElements; i++ )
        {
            switch( rEquations[ i ].Operation )
            {
                case drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::SUM :
                    aStrBuffer.append( GetXMLToken( XML_SUM ) ); break;
                case drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::PROD :
                    aStrBuffer.append( GetXMLToken( XML_PRODUCT ) ); break;
                case drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::MID :
                    aStrBuffer.append( GetXMLToken( XML_MID ) ); break;
                case drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::ABS :
                    aStrBuffer.append( GetXMLToken( XML_ABS ) ); break;
                case drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::MIN :
                    aStrBuffer.append( GetXMLToken( XML_MIN ) ); break;
                case drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::MAX :
                    aStrBuffer.append( GetXMLToken( XML_MAX ) ); break;
                case drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::IF :
                    aStrBuffer.append( GetXMLToken( XML_IF ) ); break;
                case drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::MOD :
                    aStrBuffer.append( GetXMLToken( XML_MOD ) ); break;
                case drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::ATAN2 :
                    aStrBuffer.append( GetXMLToken( XML_ATAN2 ) ); break;
                case drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::SIN :
                    aStrBuffer.append( GetXMLToken( XML_SIN ) ); break;
                case drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::COS :
                    aStrBuffer.append( GetXMLToken( XML_COS ) ); break;
                case drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::COSATAN2 :
                    aStrBuffer.append( GetXMLToken( XML_COSATAN2 ) ); break;
                case drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::SINATAN2 :
                    aStrBuffer.append( GetXMLToken( XML_SINATAN2 ) ); break;
                case drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::SQRT :
                    aStrBuffer.append( GetXMLToken( XML_SQRT ) ); break;
                case drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::SUMANGLE :
                    aStrBuffer.append( GetXMLToken( XML_SUMANGLE ) ); break;
                case drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::ELLIPSE :
                    aStrBuffer.append( GetXMLToken( XML_ELLIPSE ) ); break;
                case drafts::com::sun::star::drawing::EnhancedCustomShapeOperation::TAN :
                    aStrBuffer.append( GetXMLToken( XML_TAN ) ); break;
            }
            sal_Int32 nParameters = rEquations[ i ].Parameters.getLength();
            for ( j = 0; j < nParameters; j++ )
                ExportParameter( aStrBuffer, rEquations[ i ].Parameters[ j ] );
            aStr = aStrBuffer.makeStringAndClear();
            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_FORMULA, aStr );
            SvXMLElementExport aOBJ( rExport, XML_NAMESPACE_DRAW, XML_EQUATION, sal_True, sal_True );
        }
    }
    else
    {
        SvXMLElementExport aOBJ( rExport, XML_NAMESPACE_DRAW, XML_EQUATION, sal_True, sal_True );
    }
}

void ImpExportHandles( SvXMLExport& rExport, const uno::Sequence< beans::PropertyValues >& rHandles )
{
    sal_uInt32 i, j, nElements = rHandles.getLength();
    if ( nElements )
    {
        rtl::OUString       aStr;
        rtl::OUStringBuffer aStrBuffer;

        for ( i = 0; i < nElements; i++ )
        {
            const uno::Sequence< beans::PropertyValue >& rPropSeq = rHandles[ i ];
            for ( j = 0; j < (sal_uInt32)rPropSeq.getLength(); j++ )
            {
                const beans::PropertyValue& rPropVal = rPropSeq[ j ];
                switch( EASGet( rPropVal.Name ) )
                {
                    case EAS_Position :
                    {
                        drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair aPosition;
                        if ( rPropVal.Value >>= aPosition )
                        {
                            ExportParameter( aStrBuffer, aPosition.First );
                            ExportParameter( aStrBuffer, aPosition.Second );
                            aStr = aStrBuffer.makeStringAndClear();
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_POSITION, aStr );
                        }
                    }
                    break;
                    case EAS_MirroredX :
                    {
                        sal_Bool bMirroredX;
                        if ( rPropVal.Value >>= bMirroredX )
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_MIRROR_HORIZONTAL,
                                bMirroredX ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                    }
                    break;
                    case EAS_MirroredY :
                    {
                        sal_Bool bMirroredY;
                        if ( rPropVal.Value >>= bMirroredY )
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_MIRROR_VERTICAL,
                                bMirroredY ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                    }
                    break;
                    case EAS_Switched :
                    {
                        sal_Bool bSwitched;
                        if ( rPropVal.Value >>= bSwitched )
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_SWITCHED,
                                bSwitched ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                    }
                    break;
                    case EAS_Polar :
                    {
                        drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair aPolar;
                        if ( rPropVal.Value >>= aPolar )
                        {
                            ExportParameter( aStrBuffer, aPolar.First );
                            ExportParameter( aStrBuffer, aPolar.Second );
                            aStr = aStrBuffer.makeStringAndClear();
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_POLAR, aStr );
                        }
                    }
                    break;
                    case EAS_RadiusRangeMinimum :
                    {
                        drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aRadiusRangeMinimum;
                        if ( rPropVal.Value >>= aRadiusRangeMinimum )
                        {
                            ExportParameter( aStrBuffer, aRadiusRangeMinimum );
                            aStr = aStrBuffer.makeStringAndClear();
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_RADIUS_RANGE_MINIMUM, aStr );
                        }
                    }
                    break;
                    case EAS_RadiusRangeMaximum :
                    {
                        drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aRadiusRangeMaximum;
                        if ( rPropVal.Value >>= aRadiusRangeMaximum )
                        {
                            ExportParameter( aStrBuffer, aRadiusRangeMaximum );
                            aStr = aStrBuffer.makeStringAndClear();
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_RADIUS_RANGE_MAXIMUM, aStr );
                        }
                    }
                    break;
                    case EAS_RangeXMinimum :
                    {
                        drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aXRangeMinimum;
                        if ( rPropVal.Value >>= aXRangeMinimum )
                        {
                            ExportParameter( aStrBuffer, aXRangeMinimum );
                            aStr = aStrBuffer.makeStringAndClear();
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_RANGE_X_MINIMUM, aStr );
                        }
                    }
                    break;
                    case EAS_RangeXMaximum :
                    {
                        drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aXRangeMaximum;
                        if ( rPropVal.Value >>= aXRangeMaximum )
                        {
                            ExportParameter( aStrBuffer, aXRangeMaximum );
                            aStr = aStrBuffer.makeStringAndClear();
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_RANGE_X_MAXIMUM, aStr );
                        }
                    }
                    break;
                    case EAS_RangeYMinimum :
                    {
                        drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aYRangeMinimum;
                        if ( rPropVal.Value >>= aYRangeMinimum )
                        {
                            ExportParameter( aStrBuffer, aYRangeMinimum );
                            aStr = aStrBuffer.makeStringAndClear();
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_RANGE_Y_MINIMUM, aStr );
                        }
                    }
                    break;
                    case EAS_RangeYMaximum :
                    {
                        drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aYRangeMaximum;
                        if ( rPropVal.Value >>= aYRangeMaximum )
                        {
                            ExportParameter( aStrBuffer, aYRangeMaximum );
                            aStr = aStrBuffer.makeStringAndClear();
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HANDLE_RANGE_Y_MAXIMUM, aStr );
                        }
                    }
                    break;
                }
            }
            SvXMLElementExport aOBJ( rExport, XML_NAMESPACE_DRAW, XML_HANDLE, sal_True, sal_True );
        }
    }
    else
    {
        SvXMLElementExport aOBJ( rExport, XML_NAMESPACE_DRAW, XML_HANDLE, sal_True, sal_True );
    }
}

void ImpExportEnhancedPath( SvXMLExport& rExport,
    const uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair >& rCoordinates,
        const uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeSegment >& rSegments )
{

    rtl::OUString       aStr;
    rtl::OUStringBuffer aStrBuffer;

    sal_Int32 i, j, k, l;

    sal_Int32 nCoords = rCoordinates.getLength();
    sal_Int32 nSegments = rSegments.getLength();
    sal_Bool bSimpleSegments = nSegments == 0;
    if ( bSimpleSegments )
        nSegments = nCoords + 2;
    for ( j = i = 0; j < nSegments; j++ )
    {
        drafts::com::sun::star::drawing::EnhancedCustomShapeSegment aSegment;
        if ( bSimpleSegments )
        {
            // if there are not enough segments we will default them
            if ( !j )
            {
                aSegment.Count = 1;
                aSegment.Command = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::MOVETO;
            }
            else if ( j == ( nSegments - 2 ) )
            {
                aSegment.Count = 1;
                aSegment.Command = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::CLOSESUBPATH;
            }
            else if ( j == ( nSegments - 1 ) )
            {
                aSegment.Count = 1;
                aSegment.Command = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ENDSUBPATH;
            }
            else
            {
                aSegment.Count = (sal_Int16)Min( nCoords - i, (sal_Int32)32767 );
                aSegment.Command = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::LINETO;
            }
        }
        else
            aSegment = rSegments[ j ];

        if ( aStrBuffer.getLength() )
            aStrBuffer.append( (sal_Unicode)' ' );

        sal_Int32 nParameter = 0;
        switch( aSegment.Command )
        {
            case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::CLOSESUBPATH :
                aStrBuffer.append( (sal_Unicode)'Z' ); break;
            case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ENDSUBPATH :
                aStrBuffer.append( (sal_Unicode)'N' ); break;
            case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::NOFILL :
                aStrBuffer.append( (sal_Unicode)'F' ); break;
            case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::NOSTROKE :
                aStrBuffer.append( (sal_Unicode)'S' ); break;

            case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::MOVETO :
                aStrBuffer.append( (sal_Unicode)'M' ); nParameter = 1; break;
            case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::LINETO :
                aStrBuffer.append( (sal_Unicode)'L' ); nParameter = 1; break;
            case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::CURVETO :
                aStrBuffer.append( (sal_Unicode)'C' ); nParameter = 3; break;
            case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ANGLEELLIPSETO :
                aStrBuffer.append( (sal_Unicode)'T' ); nParameter = 3; break;
            case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ANGLEELLIPSE :
                aStrBuffer.append( (sal_Unicode)'U' ); nParameter = 3; break;
            case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ARCTO :
                aStrBuffer.append( (sal_Unicode)'A' ); nParameter = 4; break;
            case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ARC :
                aStrBuffer.append( (sal_Unicode)'B' ); nParameter = 4; break;
            case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARCTO :
                aStrBuffer.append( (sal_Unicode)'W' ); nParameter = 4; break;
            case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARC :
                aStrBuffer.append( (sal_Unicode)'V' ); nParameter = 4; break;
            case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTX :
                aStrBuffer.append( (sal_Unicode)'X' ); nParameter = 1; break;
            case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTY :
                aStrBuffer.append( (sal_Unicode)'Y' ); nParameter = 1; break;
            case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::QUADRATICCURVETO :
                aStrBuffer.append( (sal_Unicode)'Q' ); nParameter = 2; break;

            default : // ups, seems to be something wrong
            {
                aSegment.Count = 1;
                aSegment.Command = drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::LINETO;
            }
            break;
        }
        if ( nParameter )
        {
            for ( k = 0; k < aSegment.Count; k++ )
            {
                if ( ( i + nParameter ) <= nCoords )
                {
                    for ( l = 0; l < nParameter; l++ )
                    {
                        ExportParameter( aStrBuffer, rCoordinates[ i ].First );
                        ExportParameter( aStrBuffer, rCoordinates[ i++ ].Second );
                    }
                }
                else
                {
                    j = nSegments;  // error -> exiting
                    break;
                }
            }
        }
    }
    aStr = aStrBuffer.makeStringAndClear();
    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_ENHANCED_PATH, aStr );
}

void ImpExportEnhancedGeometry( SvXMLExport& rExport, const uno::Reference< beans::XPropertySet >& xPropSet )
{
    sal_Bool bEquations = sal_False;
    uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeEquation > aEquations;

    sal_Bool bHandles = sal_False;
    uno::Sequence< beans::PropertyValues > aHandles;

    sal_Bool bCoordinates = sal_False;
    uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeSegment > aSegments;
    uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair > aCoordinates;

    uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue > aAdjustmentValues;

    rtl::OUString       aStr;
    rtl::OUStringBuffer aStrBuffer;
    SvXMLUnitConverter& rUnitConverter = rExport.GetMM100UnitConverter();

    uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

    // geometry
    const rtl::OUString sCustomShapeGeometry( RTL_CONSTASCII_USTRINGPARAM( "CustomShapeGeometry" ) );
    if ( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName( sCustomShapeGeometry ) )
    {
        uno::Any aGeoPropSet( xPropSet->getPropertyValue( sCustomShapeGeometry ) );
        uno::Sequence< beans::PropertyValue > aGeoPropSeq;

        if ( aGeoPropSet >>= aGeoPropSeq )
        {
            const rtl::OUString sCustomShapeType( RTL_CONSTASCII_USTRINGPARAM( "NonPrimitive" ) );
            rtl::OUString aCustomShapeType( sCustomShapeType );

            sal_Int32 i, nCount = aGeoPropSeq.getLength();
            for ( i = 0; i < nCount; i++ )
            {
                const beans::PropertyValue& rProp = aGeoPropSeq[ i ];
                switch( EASGet( rProp.Name ) )
                {
                    case EAS_PredefinedType :
                    {
                        rProp.Value >>= aCustomShapeType;
                    }
                    break;
                    case EAS_MirroredX :
                    {
                        sal_Bool bMirroredX;
                        if ( rProp.Value >>= bMirroredX )
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_MIRROR_HORIZONTAL,
                                bMirroredX ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                    }
                    break;
                    case EAS_MirroredY :
                    {
                        sal_Bool bMirroredY;
                        if ( rProp.Value >>= bMirroredY )
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_MIRROR_VERTICAL,
                                bMirroredY ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                    }
                    break;
                    case EAS_CoordinateOrigin :
                    {
                        awt::Point aCoordinateOrigin;
                        if ( rProp.Value >>= aCoordinateOrigin )
                        {
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_COORDINATE_ORIGIN_X, rtl::OUString::valueOf( aCoordinateOrigin.X ) );
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_COORDINATE_ORIGIN_Y, rtl::OUString::valueOf( aCoordinateOrigin.Y ) );
                        }
                    }
                    break;
                    case EAS_CoordinateSize :
                    {
                        awt::Size aCoordinateSize;
                        if ( rProp.Value >>= aCoordinateSize )
                        {
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_COORDINATE_WIDTH, rtl::OUString::valueOf( aCoordinateSize.Width ) );
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_COORDINATE_HEIGHT, rtl::OUString::valueOf( aCoordinateSize.Height ) );
                        }
                    }
                    break;
                    case EAS_TextRotateAngle :
                    {
                        double fTextRotateAngle;
                        if ( rProp.Value >>= fTextRotateAngle )
                        {
                            rUnitConverter.convertDouble( aStrBuffer, fTextRotateAngle );
                            aStr = aStrBuffer.makeStringAndClear();
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_TEXT_ROTATE_ANGLE, aStr );
                        }
                    }
                    break;
                    case EAS_Callout :
                    {
                        uno::Sequence< beans::PropertyValue > aCalloutPropSeq;
                        if ( rProp.Value >>= aCalloutPropSeq )
                        {
                            sal_Int32 i, nCount = aCalloutPropSeq.getLength();
                            for ( i = 0; i < nCount; i++ )
                            {
                                const beans::PropertyValue& rProp = aCalloutPropSeq[ i ];
                                switch( EASGet( rProp.Name ) )
                                {
                                    case EAS_On :
                                    {
                                        sal_Bool bCalloutOn;
                                        if ( rProp.Value >>= bCalloutOn )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CALLOUT,
                                                bCalloutOn ? GetXMLToken( XML_ON ) : GetXMLToken( XML_OFF ) );
                                    }
                                    break;
                                    case EAS_AccentBar :
                                    {
                                        sal_Bool bAccentBar;
                                        if ( rProp.Value >>= bAccentBar )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CALLOUT_ACCENT_BAR,
                                                bAccentBar ? GetXMLToken( XML_ON ) : GetXMLToken( XML_OFF ) );
                                    }
                                    break;
                                    case EAS_Angle :
                                    {
                                        double fAngle;
                                        if ( rProp.Value >>= fAngle )
                                        {
                                            rUnitConverter.convertDouble( aStrBuffer, fAngle );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CALLOUT_ANGLE, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_Distance :
                                    {
                                        sal_Int32 nDistance;
                                        if ( rProp.Value >>= nDistance )
                                        {
                                            rUnitConverter.convertMeasure( aStrBuffer, nDistance );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CALLOUT_DROP_DISTANCE, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_Drop :
                                    {
                                        sal_Int16 nDrop;
                                        if ( rProp.Value >>= nDrop )
                                        {
                                            switch ( nDrop )
                                            {
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeCalloutDrop::TOP :    aStr = GetXMLToken( XML_TOP );    break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeCalloutDrop::CENTER : aStr = GetXMLToken( XML_CENTER ); break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeCalloutDrop::BOTTOM : aStr = GetXMLToken( XML_BOTTOM ); break;
                                            }
                                            if ( aStr.getLength() )
                                                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CALLOUT_DROP, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_DropAuto :
                                    {
                                        sal_Bool bDropAuto;
                                        if ( rProp.Value >>= bDropAuto )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CALLOUT_DROP_AUTOMATIC,
                                                bDropAuto ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_Gap :
                                    {
                                        sal_Int32 nGap;
                                        if ( rProp.Value >>= nGap )
                                        {
                                            rUnitConverter.convertMeasure( aStrBuffer, nGap );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CALLOUT_GAP, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_Length :
                                    {
                                        sal_Int32 nLength;
                                        if ( rProp.Value >>= nLength )
                                        {
                                            rUnitConverter.convertMeasure( aStrBuffer, nLength );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CALLOUT_LENGTH, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_LengthSpecified :
                                    {
                                        sal_Bool bLengthSpecified;
                                        if ( rProp.Value >>= bLengthSpecified )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CALLOUT_LENGTH_SPECIFIED,
                                                bLengthSpecified ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_FlipX :
                                    {
                                        sal_Bool bFlipX;
                                        if ( rProp.Value >>= bFlipX )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CALLOUT_FLIP_X,
                                                bFlipX ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_FlipY :
                                    {
                                        sal_Bool bFlipY;
                                        if ( rProp.Value >>= bFlipY )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CALLOUT_FLIP_Y,
                                                bFlipY ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_TextBorder :
                                    {
                                        sal_Bool bTextBorder;
                                        if ( rProp.Value >>= bTextBorder )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CALLOUT_TEXT_BORDER,
                                                bTextBorder ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_Type :
                                    {
                                        sal_Int16 nType;
                                        if ( rProp.Value >>= nType )
                                        {
                                            switch ( nType )
                                            {
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeCalloutType::RECTANGLE      : aStr = GetXMLToken( XML_RECTANGLE ); break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeCalloutType::ROUNDRECTANGLE : aStr = GetXMLToken( XML_ROUNDRECTANGLE ); break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeCalloutType::OVAL         : aStr = GetXMLToken( XML_OVAL ); break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeCalloutType::CLOUD          : aStr = GetXMLToken( XML_CLOUD ); break;
                                            }
                                            if ( aStr.getLength() )
                                                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CALLOUT_TYPE, aStr );
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    break;
                    case EAS_Extrusion :
                    {
                        uno::Sequence< beans::PropertyValue > aExtrusionPropSeq;
                        if ( rProp.Value >>= aExtrusionPropSeq )
                        {
                            sal_Int32 i, nCount = aExtrusionPropSeq.getLength();
                            for ( i = 0; i < nCount; i++ )
                            {
                                const beans::PropertyValue& rProp = aExtrusionPropSeq[ i ];
                                switch( EASGet( rProp.Name ) )
                                {
                                    case EAS_On :
                                    {
                                        sal_Bool bExtrusionOn;
                                        if ( rProp.Value >>= bExtrusionOn )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION,
                                                bExtrusionOn ? GetXMLToken( XML_ON ) : GetXMLToken( XML_OFF ) );
                                    }
                                    break;
                                    case EAS_AutoRotationCenter :
                                    {
                                        sal_Bool bExtusionAutoRotationCenter;
                                        if ( rProp.Value >>= bExtusionAutoRotationCenter )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_AUTO_ROTATION_CENTER,
                                                bExtusionAutoRotationCenter ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_BackwardDepth :
                                    {
                                        double fExtrusionBackwardDepth;
                                        if ( rProp.Value >>= fExtrusionBackwardDepth )
                                        {
                                            rUnitConverter.convertDouble( aStrBuffer, fExtrusionBackwardDepth, sal_True );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_BACKWARD_DEPTH, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_Brightness :
                                    {
                                        double fExtrusionBrightness;
                                        if ( rProp.Value >>= fExtrusionBrightness )
                                        {
                                            rUnitConverter.convertDouble( aStrBuffer, fExtrusionBrightness, sal_False, MAP_RELATIVE, MAP_RELATIVE );
                                            aStrBuffer.append( (sal_Unicode)'%' );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_BRIGHTNESS, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_Diffusity :
                                    {
                                        double fExtrusionDiffusity;
                                        if ( rProp.Value >>= fExtrusionDiffusity )
                                        {
                                            rUnitConverter.convertDouble( aStrBuffer, fExtrusionDiffusity, sal_False, MAP_RELATIVE, MAP_RELATIVE );
                                            aStrBuffer.append( (sal_Unicode)'%' );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_DIFFUSITY, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_Edge :
                                    {
                                        sal_Int32 nExtrusionEdge;
                                        if ( rProp.Value >>= nExtrusionEdge )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_EDGE, rtl::OUString::valueOf( nExtrusionEdge ) );
                                    }
                                    break;
                                    case EAS_Facet :
                                    {
                                        sal_Int32 nExtrusionFacet;
                                        if ( rProp.Value >>= nExtrusionFacet )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_FACET, rtl::OUString::valueOf( nExtrusionFacet ) );
                                    }
                                    break;
                                    case EAS_ForewardDepth :
                                    {
                                        double fExtrusionForewardDepth;
                                        if ( rProp.Value >>= fExtrusionForewardDepth )
                                        {
                                            rUnitConverter.convertDouble( aStrBuffer, fExtrusionForewardDepth, sal_True );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_FOREWARD_DEPTH, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_LightFace :
                                    {
                                        sal_Bool bExtrusionLightFace;
                                        if ( rProp.Value >>= bExtrusionLightFace )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_LIGHT_FACE,
                                                bExtrusionLightFace ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_LightHarsh1 :
                                    {
                                        sal_Bool bExtrusionLightHarsh1;
                                        if ( rProp.Value >>= bExtrusionLightHarsh1 )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_LIGHT_HARSH1,
                                                bExtrusionLightHarsh1 ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_LightHarsh2 :
                                    {
                                        sal_Bool bExtrusionLightHarsh2;
                                        if ( rProp.Value >>= bExtrusionLightHarsh2 )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_LIGHT_HARSH2,
                                                bExtrusionLightHarsh2 ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_LightLevel1 :
                                    {
                                        double fExtrusionLightLevel1;
                                        if ( rProp.Value >>= fExtrusionLightLevel1 )
                                        {
                                            rUnitConverter.convertDouble( aStrBuffer, fExtrusionLightLevel1, sal_False, MAP_RELATIVE, MAP_RELATIVE );
                                            aStrBuffer.append( (sal_Unicode)'%' );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_LIGHT_LEVEL1, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_LightLevel2 :
                                    {
                                        double fExtrusionLightLevel2;
                                        if ( rProp.Value >>= fExtrusionLightLevel2 )
                                        {
                                            rUnitConverter.convertDouble( aStrBuffer, fExtrusionLightLevel2, sal_False, MAP_RELATIVE, MAP_RELATIVE );
                                            aStrBuffer.append( (sal_Unicode)'%' );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_LIGHT_LEVEL2, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_LightDirection1 :
                                    {
                                        drawing::Direction3D aExtrusionLightDirection1;
                                        if ( rProp.Value >>= aExtrusionLightDirection1 )
                                        {
                                            Vector3D aVec3D( aExtrusionLightDirection1.DirectionX, aExtrusionLightDirection1.DirectionY,
                                                aExtrusionLightDirection1.DirectionZ );
                                            rUnitConverter.convertVector3D( aStrBuffer, aVec3D );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_LIGHT_DIRECTION1, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_LightDirection2 :
                                    {
                                        drawing::Direction3D aExtrusionLightDirection2;
                                        if ( rProp.Value >>= aExtrusionLightDirection2 )
                                        {
                                            Vector3D aVec3D( aExtrusionLightDirection2.DirectionX, aExtrusionLightDirection2.DirectionY,
                                                aExtrusionLightDirection2.DirectionZ );
                                            rUnitConverter.convertVector3D( aStrBuffer, aVec3D );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_LIGHT_DIRECTION2, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_Metal :
                                    {
                                        sal_Bool bExtrusionMetal;
                                        if ( rProp.Value >>= bExtrusionMetal )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_METAL,
                                                bExtrusionMetal ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_Plane :
                                    {
                                        sal_Int16 nExtrusionPlane;
                                        if ( rProp.Value >>= nExtrusionPlane )
                                        {
                                            switch ( nExtrusionPlane )
                                            {
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeExtrusionPlane::XY : aStr = GetXMLToken( XML_XY ); break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeExtrusionPlane::ZX : aStr = GetXMLToken( XML_ZX ); break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeExtrusionPlane::YZ : aStr = GetXMLToken( XML_YZ ); break;
                                            }
                                            if ( aStr.getLength() )
                                                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_PLANE, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_RenderMode :
                                    {
                                        sal_Int16 nExtrusionRenderMode;
                                        if ( rProp.Value >>= nExtrusionRenderMode )
                                        {
                                            switch ( nExtrusionRenderMode )
                                            {
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeExtrusionRenderMode::SOLID : aStr = GetXMLToken( XML_SOLID ); break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeExtrusionRenderMode::WIREFRAME : aStr = GetXMLToken( XML_WIREFRAME ); break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeExtrusionRenderMode::BOUNDINGCUBE : aStr = GetXMLToken( XML_BOUNDINGCUBE ); break;
                                            }
                                            if ( aStr.getLength() )
                                                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_RENDER_MODE, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_AngleX :
                                    {
                                        double fExtrusionAngleX;
                                        if ( rProp.Value >>= fExtrusionAngleX )
                                        {
                                            rUnitConverter.convertDouble( aStrBuffer, fExtrusionAngleX );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_ROTATION_ANGLE_X, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_AngleY :
                                    {
                                        double fExtrusionAngleY;
                                        if ( rProp.Value >>= fExtrusionAngleY )
                                        {
                                            rUnitConverter.convertDouble( aStrBuffer, fExtrusionAngleY );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_ROTATION_ANGLE_Y, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_RotationCenterX :
                                    {
                                        double fExtrusionRotationCenterX;
                                        if ( rProp.Value >>= fExtrusionRotationCenterX )
                                        {
                                            rUnitConverter.convertDouble( aStrBuffer, fExtrusionRotationCenterX, sal_False );
                                            aStrBuffer.append( (sal_Unicode)'%' );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_ROTATION_CENTER_X, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_RotationCenterY :
                                    {
                                        double fExtrusionRotationCenterY;
                                        if ( rProp.Value >>= fExtrusionRotationCenterY )
                                        {
                                            rUnitConverter.convertDouble( aStrBuffer, fExtrusionRotationCenterY, sal_False );
                                            aStrBuffer.append( (sal_Unicode)'%' );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_ROTATION_CENTER_Y, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_RotationCenterZ :
                                    {
                                        double fExtrusionRotationCenterZ;
                                        if ( rProp.Value >>= fExtrusionRotationCenterZ )
                                        {
                                            rUnitConverter.convertDouble( aStrBuffer, fExtrusionRotationCenterZ, sal_False );
                                            aStrBuffer.append( (sal_Unicode)'%' );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_ROTATION_CENTER_Z, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_Shininess :
                                    {
                                        double fExtrusionShininess;
                                        if ( rProp.Value >>= fExtrusionShininess )
                                        {
                                            rUnitConverter.convertDouble( aStrBuffer, fExtrusionShininess, sal_False, MAP_RELATIVE, MAP_RELATIVE );
                                            aStrBuffer.append( (sal_Unicode)'%' );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_SHININESS, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_Skew :
                                    {
                                        double fExtrusionSkew;
                                        if ( rProp.Value >>= fExtrusionSkew )
                                        {
                                            rUnitConverter.convertDouble( aStrBuffer, fExtrusionSkew, sal_False, MAP_RELATIVE, MAP_RELATIVE );
                                            aStrBuffer.append( (sal_Unicode)'%' );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_SKEW, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_SkewAngle :
                                    {
                                        double fExtrusionSkewAngle;
                                        if ( rProp.Value >>= fExtrusionSkewAngle )
                                        {
                                            rUnitConverter.convertDouble( aStrBuffer, fExtrusionSkewAngle );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_SKEW_ANGLE, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_Specularity :
                                    {
                                        double fExtrusionSpecularity;
                                        if ( rProp.Value >>= fExtrusionSpecularity )
                                        {
                                            rUnitConverter.convertDouble( aStrBuffer, fExtrusionSpecularity, sal_False, MAP_RELATIVE, MAP_RELATIVE );
                                            aStrBuffer.append( (sal_Unicode)'%' );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_SPECULARITY, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_Parallel :
                                    {
                                        sal_Bool bExtrusionParallel;
                                        if ( rProp.Value >>= bExtrusionParallel )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_PARALLEL,
                                                bExtrusionParallel ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_ViewPoint :
                                    {
                                        drawing::Position3D aExtrusionViewPoint;
                                        if ( rProp.Value >>= aExtrusionViewPoint )
                                        {
                                            rUnitConverter.convertPosition3D( aStrBuffer, aExtrusionViewPoint );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_VIEWPOINT, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_OriginX :
                                    {
                                        double fOriginX;
                                        if ( rProp.Value >>= fOriginX )
                                        {
                                            rUnitConverter.convertDouble( aStrBuffer, fOriginX, sal_False );
                                            aStrBuffer.append( (sal_Unicode)'%' );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_ORIGIN_X, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_OriginY :
                                    {
                                        double fOriginY;
                                        if ( rProp.Value >>= fOriginY )
                                        {
                                            rUnitConverter.convertDouble( aStrBuffer, fOriginY, sal_False );
                                            aStrBuffer.append( (sal_Unicode)'%' );
                                            aStr = aStrBuffer.makeStringAndClear();
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_ORIGIN_Y, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_Color :
                                    {
                                        sal_Bool bExtrusionColor;
                                        if ( rProp.Value >>= bExtrusionColor )
                                        {
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_COLOR,
                                                bExtrusionColor ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    break;
                    case EAS_TextPath :
                    {
                        uno::Sequence< beans::PropertyValue > aTextPathPropSeq;
                        if ( rProp.Value >>= aTextPathPropSeq )
                        {
                            sal_Int32 i, nCount = aTextPathPropSeq.getLength();
                            for ( i = 0; i < nCount; i++ )
                            {
                                const beans::PropertyValue& rProp = aTextPathPropSeq[ i ];
                                switch( EASGet( rProp.Name ) )
                                {
                                    case EAS_On :
                                    {
                                        sal_Bool bTextPathOn;
                                        if ( rProp.Value >>= bTextPathOn )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_TEXT_PATH,
                                                bTextPathOn ? GetXMLToken( XML_ON ) : GetXMLToken( XML_OFF ) );
                                    }
                                    break;
                                    case EAS_FitPath :
                                    {
                                        sal_Bool bTextPathFitPath;
                                        if ( rProp.Value >>= bTextPathFitPath )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_TEXT_PATH_FIT_TEXT,
                                                bTextPathFitPath ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_FitShape :
                                    {
                                        sal_Bool bTextPathFitShape;
                                        if ( rProp.Value >>= bTextPathFitShape )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_TEXT_PATH_FIT_SHAPE,
                                                bTextPathFitShape ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_ScaleX :
                                    {
                                        sal_Bool bTextPathScaleX;
                                        if ( rProp.Value >>= bTextPathScaleX )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_TEXT_PATH_SCALE_X,
                                                bTextPathScaleX ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_SameLetterHeights :
                                    {
                                        sal_Bool bSameLetterHeights;
                                        if ( rProp.Value >>= bSameLetterHeights )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_TEXT_PATH_SAME_LETTER_HEIGHTS,
                                                bSameLetterHeights ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    break;
                    case EAS_Path :
                    {
                        uno::Sequence< beans::PropertyValue > aPathPropSeq;
                        if ( rProp.Value >>= aPathPropSeq )
                        {
                            sal_Int32 i, nCount = aPathPropSeq.getLength();
                            for ( i = 0; i < nCount; i++ )
                            {
                                const beans::PropertyValue& rProp = aPathPropSeq[ i ];
                                switch( EASGet( rProp.Name ) )
                                {
                                    case EAS_ExtrusionAllowed :
                                    {
                                        sal_Bool bExtrusionAllowed;
                                        if ( rProp.Value >>= bExtrusionAllowed )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_EXTRUSION_ALLOWED,
                                                bExtrusionAllowed ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_ConcentricGradientFillAllowed :
                                    {
                                        sal_Bool bConcentricGradientFillAllowed;
                                        if ( rProp.Value >>= bConcentricGradientFillAllowed )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CONCENTRIC_GRADIENT_FILL_ALLOWED,
                                                bConcentricGradientFillAllowed ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_TextPathAllowed  :
                                    {
                                        sal_Bool bTextPathAllowed;
                                        if ( rProp.Value >>= bTextPathAllowed )
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_TEXT_PATH_ALLOWED,
                                                bTextPathAllowed ? GetXMLToken( XML_TRUE ) : GetXMLToken( XML_FALSE ) );
                                    }
                                    break;
                                    case EAS_GluePoints :
                                    {
                                        com::sun::star::uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair> aGluePoints;
                                        if ( rProp.Value >>= aGluePoints )
                                        {
                                            sal_Int32 j, nElements = aGluePoints.getLength();
                                            if ( nElements )
                                            {
                                                for( j = 0; j < nElements; j++ )
                                                {
                                                    ExportParameter( aStrBuffer, aGluePoints[ j ].First );
                                                    ExportParameter( aStrBuffer, aGluePoints[ j ].Second );
                                                }
                                                aStr = aStrBuffer.makeStringAndClear();
                                            }
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_GLUE_POINTS, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_GluePointType :
                                    {
                                        sal_Int16 nGluePointType;
                                        if ( rProp.Value >>= nGluePointType )
                                        {
                                            switch ( nGluePointType )
                                            {
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeGluePointType::NONE     : aStr = GetXMLToken( XML_NONE );    break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeGluePointType::SEGMENTS : aStr = GetXMLToken( XML_SEGMENTS ); break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeGluePointType::RECT     : aStr = GetXMLToken( XML_RECTANGLE ); break;
                                            }
                                            if ( aStr.getLength() )
                                                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_GLUE_POINT_TYPE, aStr );
                                        }
                                    }
                                    break;
                                    case EAS_Coordinates :
                                    {
                                        bCoordinates = ( rProp.Value >>= aCoordinates );
                                    }
                                    break;
                                    case EAS_Segments :
                                    {
                                        rProp.Value >>= aSegments;
                                    }
                                    case EAS_StretchPoint :
                                    {
                                        awt::Point aPathStretchPoint;
                                        if ( rProp.Value >>= aPathStretchPoint )
                                        {
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_PATH_STRETCHPOINT_X, rtl::OUString::valueOf( aPathStretchPoint.X ) );
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_PATH_STRETCHPOINT_Y, rtl::OUString::valueOf( aPathStretchPoint.Y ) );
                                        }
                                    }
                                    break;
                                    case EAS_TextFrames :
                                    {
                                        com::sun::star::uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeTextFrame > aPathTextFrames;
                                        if ( rProp.Value >>= aPathTextFrames )
                                        {
                                            if ( (sal_uInt16)aPathTextFrames.getLength() )
                                            {
                                                sal_uInt16 j, nElements = (sal_uInt16)aPathTextFrames.getLength();
                                                for ( j = 0; j < nElements; j++ )
                                                {
                                                    ExportParameter( aStrBuffer, aPathTextFrames[ j ].TopLeft.First );
                                                    ExportParameter( aStrBuffer, aPathTextFrames[ j ].TopLeft.Second );
                                                    ExportParameter( aStrBuffer, aPathTextFrames[ j ].BottomRight.First );
                                                    ExportParameter( aStrBuffer, aPathTextFrames[ j ].BottomRight.Second );
                                                }
                                                aStr = aStrBuffer.makeStringAndClear();
                                            }
                                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_TEXT_FRAMES, aStr );
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    break;
                    case EAS_Equations :
                    {
                        bEquations = ( rProp.Value >>= aEquations );
                    }
                    break;
                    case EAS_Handles :
                    {
                        bHandles = ( rProp.Value >>= aHandles );
                    }
                    break;
                    case EAS_AdjustmentValues :
                    {
                        rProp.Value >>= aAdjustmentValues;
                    }
                    break;
                }
            }   // for
            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_PREDEFINED_TYPE, aCustomShapeType );

            // adjustments
            sal_Int32 nAdjustmentValues = aAdjustmentValues.getLength();
            if ( nAdjustmentValues )
            {
                sal_Int32 i, nValue;
                for ( i = 0; i < nAdjustmentValues; i++ )
                {
                    if ( i )
                        aStrBuffer.append( (sal_Unicode)',' );

                    const drafts::com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue& rAdj = aAdjustmentValues[ i ];
                    if ( rAdj.State == beans::PropertyState_DIRECT_VALUE )
                    {
                        if ( rAdj.Value.getValueTypeClass() == uno::TypeClass_DOUBLE )
                        {
                            double fValue;
                            rAdj.Value >>= fValue;
                            rUnitConverter.convertDouble( aStrBuffer, fValue );
                        }
                        else
                        {
                            rAdj.Value >>= nValue;
                            rUnitConverter.convertNumber( aStrBuffer, nValue );
                        }
                    }
                }
                aStr = aStrBuffer.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_ADJUSTMENTS, aStr );
            }
            if ( bCoordinates )
                ImpExportEnhancedPath( rExport, aCoordinates, aSegments );
        }
    }
    SvXMLElementExport aOBJ( rExport, XML_NAMESPACE_DRAW, XML_ENHANCED_GEOMETRY, sal_True, sal_True );
    if ( bEquations )
        ImpExportEquations( rExport, aEquations );
    if ( bHandles )
        ImpExportHandles( rExport, aHandles );
}

void XMLShapeExport::ImpExportCustomShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures, com::sun::star::awt::Point* pRefPoint )
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if ( xPropSet.is() )
    {
        rtl::OUString aStr;
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

        // Transformation
        ImpExportNewTrans( xPropSet, nFeatures, pRefPoint );

        if ( xPropSetInfo.is() )
        {
            const rtl::OUString sCustomShapeEngine( RTL_CONSTASCII_USTRINGPARAM( "CustomShapeEngine" ) );
            if ( xPropSetInfo->hasPropertyByName( sCustomShapeEngine ) )
            {
                uno::Any aEngine( xPropSet->getPropertyValue( sCustomShapeEngine ) );
                if ( ( aEngine >>= aStr ) && aStr.getLength() )
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_ENGINE, aStr );
            }
            const rtl::OUString sCustomShapeData( RTL_CONSTASCII_USTRINGPARAM( "CustomShapeData" ) );
            if ( xPropSetInfo->hasPropertyByName( sCustomShapeData ) )
            {
                uno::Any aData( xPropSet->getPropertyValue( sCustomShapeData ) );
                if ( ( aData >>= aStr ) && aStr.getLength() )
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DATA, aStr );
            }
        }
        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aOBJ( rExport, XML_NAMESPACE_DRAW, XML_CUSTOM_SHAPE, bCreateNewline, sal_True );
        ImpExportEvents( xShape );
        ImpExportGluePoints( xShape );
        ImpExportText( xShape );
        ImpExportEnhancedGeometry( rExport, xPropSet );
    }
}

