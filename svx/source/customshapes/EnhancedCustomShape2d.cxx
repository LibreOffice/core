/*************************************************************************
 *
 *  $RCSfile: EnhancedCustomShape2d.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-28 16:19:02 $
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

#ifndef _ENHANCEDCUSTOMSHAPE2D_HXX
#include "EnhancedCustomShape2d.hxx"
#endif
#ifndef _ENHANCEDCUSTOMSHAPEGEOMETRY_HXX
#include "EnhancedCustomShapeGeometry.hxx"
#endif
#ifndef _ENHANCED_CUSTOMSHAPE_TYPE_NAMES_HXX
#include "EnhancedCustomShapeTypeNames.hxx"
#endif
#ifndef _SVDOASHP_HXX
#include "svdoashp.hxx"
#endif
#ifndef _SVDTRANS_HXX
#include "svdtrans.hxx"
#endif
#ifndef _SVDOCIRC_HXX
#include <svdocirc.hxx>
#endif
#ifndef _SVDOGRP_HXX
#include <svdogrp.hxx>
#endif
#ifndef _SVDOPATH_HXX
#include <svdopath.hxx>
#endif
#ifndef _SVDOCAPT_HXX
#include "svdocapt.hxx"
#endif
#ifndef _SVDPAGE_HXX
#include <svdpage.hxx>
#endif
#ifndef _SVX_XFLCLIT_HXX
#include <xflclit.hxx>
#endif
#ifndef _SDASAITM_HXX
#include <sdasaitm.hxx>
#endif
#ifndef _SVDMODEL_HXX
#include <svdmodel.hxx>
#endif
#ifndef _XOUTX_HXX
#include <xoutx.hxx>
#endif
#ifndef _RTL_CRC_H_
#include <rtl/crc.h>
#endif
#ifndef SVX_XFILLIT0_HXX
#include <xfillit0.hxx>
#endif
#ifndef _SVX_XLNSTIT_HXX
#include <xlnstit.hxx>
#endif
#ifndef _SVX_XLNEDIT_HXX
#include <xlnedit.hxx>
#endif
#ifndef _SVX_XLNSTWIT_HXX
#include <xlnstwit.hxx>
#endif
#ifndef _SVX_XLNEDWIT_HXX
#include <xlnedwit.hxx>
#endif
#ifndef _SVX_XLNSTCIT_HXX
#include <xlnstcit.hxx>
#endif
#ifndef _SVX_XLNEDCIT_HXX
#include <xlnedcit.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEPARAMETERTYPE_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#endif
#ifndef __drafts_com_sun_star_drawing_EnhancedCustomShapeSegmentCommand_hpp__
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#endif
#ifndef __drafts_com_sun_star_drawing_EnhancedCustomShapeEquation_hpp__
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeEquation.hpp>
#endif
#ifndef __drafts_com_sun_star_drawing_EnhancedCustomShapeOperation_hpp__
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeOperation.hpp>
#endif

using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::drawing;
using namespace ::drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand;

void EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( EnhancedCustomShapeParameter& rParameter, const sal_Int32 nValue )
{
    sal_uInt32 nDat = (sal_uInt32)nValue;
    sal_Int32  nNewValue = nValue;

    // check if this is a special point
    if ( ( nDat >> 16 ) == 0x8000 )
    {
        nNewValue = (sal_uInt16)nDat;
        rParameter.Type = EnhancedCustomShapeParameterType::EQUATION;
    }
    else
        rParameter.Type = EnhancedCustomShapeParameterType::NORMAL;
    rParameter.Value <<= nNewValue;
}

void EnhancedCustomShape2d::SetEnhancedCustomShapeEquationParameter( EnhancedCustomShapeParameter& rParameter, const sal_Int16 nPara, const sal_Bool bIsSpecialValue )
{
    sal_Int32 nValue = 0;
    if ( bIsSpecialValue )
    {
        if ( nPara & 0x400 )
        {
            nValue = nPara & 0xff;
            rParameter.Type = EnhancedCustomShapeParameterType::EQUATION;
        }
        else
        {
            switch( nPara )
            {
                case DFF_Prop_adjustValue :
                case DFF_Prop_adjust2Value :
                case DFF_Prop_adjust3Value :
                case DFF_Prop_adjust4Value :
                case DFF_Prop_adjust5Value :
                case DFF_Prop_adjust6Value :
                case DFF_Prop_adjust7Value :
                case DFF_Prop_adjust8Value :
                case DFF_Prop_adjust9Value :
                case DFF_Prop_adjust10Value :
                {
                    nValue = nPara - DFF_Prop_adjustValue;
                    rParameter.Type = EnhancedCustomShapeParameterType::ADJUSTMENT;
                }
                break;
                case DFF_Prop_geoLeft :
                    rParameter.Type = EnhancedCustomShapeParameterType::LEFT;
                break;
                case DFF_Prop_geoTop :
                    rParameter.Type = EnhancedCustomShapeParameterType::TOP;
                break;
                case DFF_Prop_geoRight :
                    rParameter.Type = EnhancedCustomShapeParameterType::RIGHT;
                break;
                case DFF_Prop_geoBottom :
                    rParameter.Type = EnhancedCustomShapeParameterType::BOTTOM;
                break;
            }
        }
    }
    else
    {
        nValue = nPara;
        rParameter.Type = EnhancedCustomShapeParameterType::NORMAL;
    }
    rParameter.Value <<= nValue;
}

void EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( EnhancedCustomShapeParameter& rParameter, const sal_Int32 nPara, const sal_Bool bIsSpecialValue, sal_Bool bHorz )
{
    sal_Int32 nValue = 0;
    if ( bIsSpecialValue )
    {
        if ( ( nPara >= 0x100 ) && ( nPara <= 0x107 ) )
        {
            nValue = nPara & 0xff;
            rParameter.Type = EnhancedCustomShapeParameterType::ADJUSTMENT;
        }
        else if ( ( nPara >= 3 ) && ( nPara <= 0x82 ) )
        {
            nValue = nPara - 3;
            rParameter.Type = EnhancedCustomShapeParameterType::EQUATION;
        }
        else if ( nPara == 0 )
        {
            nValue = 0;
            if ( bHorz )
                rParameter.Type = EnhancedCustomShapeParameterType::LEFT;
            else
                rParameter.Type = EnhancedCustomShapeParameterType::TOP;
        }
        else if ( nPara == 1 )
        {
            nValue = 0;
            if ( bHorz )
                rParameter.Type = EnhancedCustomShapeParameterType::RIGHT;
            else
                rParameter.Type = EnhancedCustomShapeParameterType::BOTTOM;
        }
        else if ( nPara == 2 )
        {
            nValue = 0;
            rParameter.Type = EnhancedCustomShapeParameterType::CENTER;
        }
        else
        {
            nValue = nPara;
            rParameter.Type = EnhancedCustomShapeParameterType::NORMAL;
        }
    }
    else
    {
        nValue = nPara;
        rParameter.Type = EnhancedCustomShapeParameterType::NORMAL;
    }
    rParameter.Value <<= nValue;
}

sal_Bool EnhancedCustomShape2d::ConvertSequenceToEnhancedCustomShape2dHandle(
    const com::sun::star::beans::PropertyValues& rHandleProperties,
        EnhancedCustomShape2d::Handle& rDestinationHandle )
{
    sal_Bool bRetValue = sal_False;
    sal_uInt32 i, nProperties = rHandleProperties.getLength();
    if ( nProperties )
    {
        rDestinationHandle.nFlags = 0;
        for ( i = 0; i < nProperties; i++ )
        {
            const com::sun::star::beans::PropertyValue& rPropVal = rHandleProperties[ i ];

            const rtl::OUString sPosition           ( RTL_CONSTASCII_USTRINGPARAM( "Position" ) );
            const rtl::OUString sMirroredX          ( RTL_CONSTASCII_USTRINGPARAM( "MirroredX" ) );
            const rtl::OUString sMirroredY          ( RTL_CONSTASCII_USTRINGPARAM( "MirroredY" ) );
            const rtl::OUString sSwitched           ( RTL_CONSTASCII_USTRINGPARAM( "Switched" ) );
            const rtl::OUString sPolar              ( RTL_CONSTASCII_USTRINGPARAM( "Polar" ) );
//          const rtl::OUString sMap                ( RTL_CONSTASCII_USTRINGPARAM( "Map" ) );
            const rtl::OUString sRadiusRangeMinimum ( RTL_CONSTASCII_USTRINGPARAM( "RadiusRangeMinimum" ) );
            const rtl::OUString sRadiusRangeMaximum ( RTL_CONSTASCII_USTRINGPARAM( "RadiusRangeMaximum" ) );
            const rtl::OUString sRangeXMinimum      ( RTL_CONSTASCII_USTRINGPARAM( "RangeXMinimum" ) );
            const rtl::OUString sRangeXMaximum      ( RTL_CONSTASCII_USTRINGPARAM( "RangeXMaximum" ) );
            const rtl::OUString sRangeYMinimum      ( RTL_CONSTASCII_USTRINGPARAM( "RangeYMinimum" ) );
            const rtl::OUString sRangeYMaximum      ( RTL_CONSTASCII_USTRINGPARAM( "RangeYMaximum" ) );

            if ( rPropVal.Name.equals( sPosition ) )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aPosition )
                    bRetValue = sal_True;
            }
            else if ( rPropVal.Name.equals( sMirroredX ) )
            {
                sal_Bool bMirroredX;
                if ( rPropVal.Value >>= bMirroredX )
                {
                    if ( bMirroredX )
                        rDestinationHandle.nFlags |= HANDLE_FLAGS_MIRRORED_X;
                }
            }
            else if ( rPropVal.Name.equals( sMirroredY ) )
            {
                sal_Bool bMirroredY;
                if ( rPropVal.Value >>= bMirroredY )
                {
                    if ( bMirroredY )
                        rDestinationHandle.nFlags |= HANDLE_FLAGS_MIRRORED_Y;
                }
            }
            else if ( rPropVal.Name.equals( sSwitched ) )
            {
                sal_Bool bSwitched;
                if ( rPropVal.Value >>= bSwitched )
                {
                    if ( bSwitched )
                        rDestinationHandle.nFlags |= HANDLE_FLAGS_SWITCHED;
                }
            }
            else if ( rPropVal.Name.equals( sPolar ) )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aPolar )
                    rDestinationHandle.nFlags |= HANDLE_FLAGS_POLAR;
            }
/* seems not to be used.
            else if ( rPropVal.Name.equals( sMap ) )
            {
                drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair aMap;
                if ( rPropVal.Value >>= aMap )
                {
                    if ( GetValueForEnhancedCustomShapeHandleParameter( nXMap, aMap.First ) )
                        rDestinationHandle.Flags |= 0x800;
                    if ( GetValueForEnhancedCustomShapeHandleParameter( nYMap, aMap.Second ) )
                        rDestinationHandle.Flags |= 0x1000;
                    rDestinationHandle.Flags |= 0x10;
                }
            }
*/
            else if ( rPropVal.Name.equals( sRadiusRangeMinimum ) )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aRadiusRangeMinimum )
                    rDestinationHandle.nFlags |= HANDLE_FLAGS_RADIUS_RANGE_MINIMUM;
            }
            else if ( rPropVal.Name.equals( sRadiusRangeMaximum ) )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aRadiusRangeMaximum )
                    rDestinationHandle.nFlags |= HANDLE_FLAGS_RADIUS_RANGE_MAXIMUM;
            }
            else if ( rPropVal.Name.equals( sRangeXMinimum ) )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aXRangeMinimum )
                    rDestinationHandle.nFlags |= HANDLE_FLAGS_RANGE_X_MINIMUM;
            }
            else if ( rPropVal.Name.equals( sRangeXMaximum ) )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aXRangeMaximum )
                    rDestinationHandle.nFlags |= HANDLE_FLAGS_RANGE_X_MAXIMUM;
            }
            else if ( rPropVal.Name.equals( sRangeYMinimum ) )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aYRangeMinimum )
                    rDestinationHandle.nFlags |= HANDLE_FLAGS_RANGE_Y_MINIMUM;
            }
            else if ( rPropVal.Name.equals( sRangeYMaximum ) )
            {
                if ( rPropVal.Value >>= rDestinationHandle.aYRangeMaximum )
                    rDestinationHandle.nFlags |= HANDLE_FLAGS_RANGE_Y_MAXIMUM;
            }
        }
    }
    return bRetValue;
}

const sal_Int32* EnhancedCustomShape2d::ApplyShapeAttributes( const SdrCustomShapeGeometryItem& rGeometryItem )
{
    const sal_Int32* pDefData = NULL;
    const mso_CustomShape* pDefCustomShape = GetCustomShapeContent( eSpType );
    if ( pDefCustomShape )
        pDefData = pDefCustomShape->pDefData;

    //////////////////////
    // AdjustmentValues //
    //////////////////////
    const rtl::OUString sAdjustmentValues( RTL_CONSTASCII_USTRINGPARAM ( "AdjustmentValues" ) );
    const Any* pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sAdjustmentValues );
    if ( pAny )
        *pAny >>= seqAdjustmentValues;
    if ( pDefData ) // now check if we have to default some adjustment values
    {
        // first check if there are adjustment values are to be appended
        sal_Int32 i, nAdjustmentValues = seqAdjustmentValues.getLength();
        sal_Int32 nAdjustmentDefaults = *pDefData++;
        if ( nAdjustmentDefaults > nAdjustmentValues )
        {
            seqAdjustmentValues.realloc( nAdjustmentDefaults );
            for ( i = nAdjustmentValues; i < nAdjustmentDefaults; i++ )
            {
                seqAdjustmentValues[ i ].Value <<= pDefData[ i ];
                seqAdjustmentValues[ i ].State = com::sun::star::beans::PropertyState_DEFAULT_VALUE;
            }
        }
        // check if there are defaulted adjustment values that should be filled the hard coded defaults (pDefValue)
        sal_Int32 nCount = nAdjustmentValues > nAdjustmentDefaults ? nAdjustmentDefaults : nAdjustmentValues;
        for ( i = 0; i < nCount; i++ )
        {
            if ( seqAdjustmentValues[ i ].State != com::sun::star::beans::PropertyState_DIRECT_VALUE )
                seqAdjustmentValues[ i ].Value <<= pDefData[ i ];
        }
    }

    ///////////////
    // Coordsize //
    ///////////////
    const rtl::OUString sCoordinateOrigin( RTL_CONSTASCII_USTRINGPARAM ( "CoordinateOrigin" ) );
    const rtl::OUString sCoordinateSize( RTL_CONSTASCII_USTRINGPARAM ( "CoordinateSize" ) );
    const Any* pOrigin = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sCoordinateOrigin );
    const Any* pSize = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sCoordinateSize );
    if ( pOrigin || pSize )
    {
        sal_Int32 nGeoLeft = 0;
        sal_Int32 nGeoTop  = 0;
        sal_Int32 nGeoRight  = 21600;
        sal_Int32 nGeoBottom = 21600;
        if ( pOrigin )
        {
            com::sun::star::awt::Point aOrigin;
            if ( *pOrigin >>= aOrigin )
            {
                nGeoLeft = aOrigin.X;
                nGeoTop  = aOrigin.Y;
            }
        }
        if ( pSize )
        {
            com::sun::star::awt::Size aSize;
            if ( *pSize >>= aSize )
            {
                nGeoRight = aSize.Width + nGeoLeft;
                nGeoBottom= aSize.Height+ nGeoTop;

            }
        }
        nCoordWidth = labs( nGeoRight - nGeoLeft );
        nCoordHeight = labs( nGeoBottom - nGeoTop );
    }
    else if ( pDefCustomShape )
    {
        nCoordWidth = pDefCustomShape->nCoordWidth;
        nCoordHeight = pDefCustomShape->nCoordHeight;
    }

    const rtl::OUString sPath( RTL_CONSTASCII_USTRINGPARAM ( "Path" ) );

    //////////////////////
    // Path/Coordinates //
    //////////////////////
    const rtl::OUString sCoordinates( RTL_CONSTASCII_USTRINGPARAM ( "Coordinates" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, sCoordinates );
    if ( pAny )
        *pAny >>= seqCoordinates;
    else if ( pDefCustomShape && pDefCustomShape->nVertices && pDefCustomShape->pVertices )
    {
        sal_Int32 i, nCount = pDefCustomShape->nVertices;
        seqCoordinates.realloc( nCount );
        for ( i = 0; i < nCount; i++ )
        {
            SetEnhancedCustomShapeParameter( seqCoordinates[ i ].First, pDefCustomShape->pVertices[ i ].nValA );
            SetEnhancedCustomShapeParameter( seqCoordinates[ i ].Second, pDefCustomShape->pVertices[ i ].nValB );
        }
    }

    /////////////////////
    // Path/GluePoints //
    /////////////////////
    const rtl::OUString sGluePoints( RTL_CONSTASCII_USTRINGPARAM ( "GluePoints" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, sGluePoints );
    if ( pAny )
        *pAny >>= seqGluePoints;
    else if ( pDefCustomShape && pDefCustomShape->nGluePoints && pDefCustomShape->pGluePoints )
    {
        sal_Int32 i, nCount = pDefCustomShape->nGluePoints;
        seqGluePoints.realloc( nCount );
        for ( i = 0; i < nCount; i++ )
        {
            SetEnhancedCustomShapeParameter( seqGluePoints[ i ].First, pDefCustomShape->pGluePoints[ i ].nValA );
            SetEnhancedCustomShapeParameter( seqGluePoints[ i ].Second, pDefCustomShape->pGluePoints[ i ].nValB );
        }
    }

    ///////////////////
    // Path/Segments //
    ///////////////////
    const rtl::OUString sSegments( RTL_CONSTASCII_USTRINGPARAM ( "Segments" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, sSegments );
    if ( pAny )
        *pAny >>= seqSegments;
    else if ( pDefCustomShape && pDefCustomShape->nElements && pDefCustomShape->pElements )
    {
        sal_Int32 i, nCount = pDefCustomShape->nElements;
        seqSegments.realloc( nCount );
        for ( i = 0; i < nCount; i++ )
        {
            EnhancedCustomShapeSegment& rSegInfo = seqSegments[ i ];
            sal_uInt16 nSDat = pDefCustomShape->pElements[ i ];
            switch( nSDat >> 8 )
            {
                case 0x00 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::LINETO;
                    rSegInfo.Count   = nSDat & 0xff;
                }
                break;
                case 0x20 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CURVETO;
                    rSegInfo.Count   = nSDat & 0xff;
                }
                break;
                case 0x40 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::MOVETO;
                    rSegInfo.Count   = nSDat & 0xff;
                }
                break;
                case 0x60 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CLOSESUBPATH;
                    rSegInfo.Count   = nSDat & 0xff;
                }
                break;
                case 0x80 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ENDSUBPATH;
                    rSegInfo.Count   = nSDat & 0xff;
                }
                break;
                case 0xa1 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ANGLEELLIPSETO;
                    rSegInfo.Count   = ( nSDat & 0xff ) / 3;
                }
                break;
                case 0xa2 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ANGLEELLIPSE;
                    rSegInfo.Count   = ( nSDat & 0xff ) / 3;
                }
                break;
                case 0xa3 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ARCTO;
                    rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
                }
                break;
                case 0xa4 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ARC;
                    rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
                }
                break;
                case 0xa5 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CLOCKWISEARCTO;
                    rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
                }
                break;
                case 0xa6 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CLOCKWISEARC;
                    rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
                }
                break;
                case 0xa7 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTX;
                    rSegInfo.Count   = nSDat & 0xff;
                }
                break;
                case 0xa8 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTY;
                    rSegInfo.Count   = nSDat & 0xff;
                }
                break;
                case 0xaa :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::NOFILL;
                    rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
                }
                break;
                case 0xab :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::NOSTROKE;
                    rSegInfo.Count   = nSDat & 0xff;
                }
                break;
                default:
                case 0xf8 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::UNKNOWN;
                    rSegInfo.Count   = nSDat;
                }
                break;
            }
        }
    }

    ///////////////////////
    // Path/StretchPoint //
    ///////////////////////
    const rtl::OUString sStretchPoint( RTL_CONSTASCII_USTRINGPARAM ( "StretchPoint" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, sStretchPoint );
    if ( pAny )
    {
        com::sun::star::awt::Point aStretchPoint;
        if ( *pAny >>= aStretchPoint )
        {
            nXRef = aStretchPoint.X;
            nYRef = aStretchPoint.Y;
        }
    }
    else if ( pDefCustomShape )
    {
        nXRef = pDefCustomShape->nXRef;
        nYRef = pDefCustomShape->nYRef;
    }

    /////////////////////
    // Path/TextFrames //
    /////////////////////
    const rtl::OUString sTextFrames( RTL_CONSTASCII_USTRINGPARAM ( "TextFrames" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, sTextFrames );
    if ( pAny )
        *pAny >>= seqTextFrames;
    else if ( pDefCustomShape && pDefCustomShape->nTextRect && pDefCustomShape->pTextRect )
    {
        sal_Int32 i, nCount = pDefCustomShape->nTextRect;
        seqTextFrames.realloc( nCount );
        const SvxMSDffTextRectangles* pRectangles = pDefCustomShape->pTextRect;
        for ( i = 0; i < nCount; i++, pRectangles++ )
        {
            SetEnhancedCustomShapeParameter( seqTextFrames[ i ].TopLeft.First,    pRectangles->nPairA.nValA );
            SetEnhancedCustomShapeParameter( seqTextFrames[ i ].TopLeft.Second,   pRectangles->nPairA.nValB );
            SetEnhancedCustomShapeParameter( seqTextFrames[ i ].BottomRight.First,  pRectangles->nPairB.nValA );
            SetEnhancedCustomShapeParameter( seqTextFrames[ i ].BottomRight.Second, pRectangles->nPairB.nValB );
        }
    }

    ///////////////
    // Equations //
    ///////////////
    const rtl::OUString sEquations( RTL_CONSTASCII_USTRINGPARAM( "Equations" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sEquations );
    if ( pAny )
        *pAny >>= seqEquations;
    else if ( pDefCustomShape && pDefCustomShape->nCalculation && pDefCustomShape->pCalculation )
    {
        sal_Int32 i, nCount = pDefCustomShape->nCalculation;
        seqEquations.realloc( nCount );

        const SvxMSDffCalculationData* pData = pDefCustomShape->pCalculation;
        EnhancedCustomShapeEquation aEquation;
        aEquation.Parameters.realloc( 3 );
        for ( i = 0; i < nCount; i++, pData++ )
        {
            sal_uInt16 nFlags = pData->nFlags;
            aEquation.Operation = nFlags & 0xff;
            SetEnhancedCustomShapeEquationParameter( aEquation.Parameters[ 0 ], pData->nVal[ 0 ], ( nFlags & 0x2000 ) != 0 );
            SetEnhancedCustomShapeEquationParameter( aEquation.Parameters[ 1 ], pData->nVal[ 1 ], ( nFlags & 0x4000 ) != 0 );
            SetEnhancedCustomShapeEquationParameter( aEquation.Parameters[ 2 ], pData->nVal[ 2 ], ( nFlags & 0x8000 ) != 0 );
            seqEquations[ i ] = aEquation;
        }
    }

    /////////////
    // Handles //
    /////////////
    const rtl::OUString sHandles( RTL_CONSTASCII_USTRINGPARAM( "Handles" ) );
    pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sHandles );
    if ( pAny )
        *pAny >>= seqHandles;
    else if ( pDefCustomShape && pDefCustomShape->nHandles && pDefCustomShape->pHandles )
    {
        sal_Int32 i, n, nCount = pDefCustomShape->nHandles;
        const SvxMSDffHandle* pData = pDefCustomShape->pHandles;
        seqHandles.realloc( nCount );
        for ( i = 0; i < nCount; i++, pData++ )
        {
            sal_Int32 nPropertiesNeeded = 1;    // position is always needed
            sal_Int32 nFlags = pData->nFlags;
            if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_X )
                nPropertiesNeeded++;
            if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_Y )
                nPropertiesNeeded++;
            if ( nFlags & MSDFF_HANDLE_FLAGS_SWITCHED )
                nPropertiesNeeded++;
            if ( nFlags & MSDFF_HANDLE_FLAGS_POLAR )
            {
                nPropertiesNeeded++;
                if ( nFlags & MSDFF_HANDLE_FLAGS_RADIUS_RANGE )
                {
                    if ( pData->nRangeXMin != 0x80000000 )
                        nPropertiesNeeded++;
                    if ( pData->nRangeXMax != 0x7fffffff )
                        nPropertiesNeeded++;
                }
            }
            else if ( nFlags & MSDFF_HANDLE_FLAGS_RANGE )
            {
                if ( pData->nRangeXMin != 0x80000000 )
                    nPropertiesNeeded++;
                if ( pData->nRangeXMax != 0x7fffffff )
                    nPropertiesNeeded++;
                if ( pData->nRangeYMin != 0x80000000 )
                    nPropertiesNeeded++;
                if ( pData->nRangeYMax != 0x7fffffff )
                    nPropertiesNeeded++;
            }

            n = 0;
            com::sun::star::beans::PropertyValues& rPropValues = seqHandles[ i ];
            rPropValues.realloc( nPropertiesNeeded );

            // POSITION
            {
                const rtl::OUString sPosition( RTL_CONSTASCII_USTRINGPARAM ( "Position" ) );
                ::drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair aPosition;
                SetEnhancedCustomShapeHandleParameter( aPosition.First, pData->nPositionX, sal_True, sal_True );
                SetEnhancedCustomShapeHandleParameter( aPosition.Second, pData->nPositionY, sal_True, sal_False );
                rPropValues[ n ].Name = sPosition;
                rPropValues[ n++ ].Value <<= aPosition;
            }
            if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_X )
            {
                const rtl::OUString sMirroredX( RTL_CONSTASCII_USTRINGPARAM ( "MirroredX" ) );
                sal_Bool bMirroredX = sal_True;
                rPropValues[ n ].Name = sMirroredX;
                rPropValues[ n++ ].Value <<= bMirroredX;
            }
            if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_Y )
            {
                const rtl::OUString sMirroredY( RTL_CONSTASCII_USTRINGPARAM ( "MirroredY" ) );
                sal_Bool bMirroredY = sal_True;
                rPropValues[ n ].Name = sMirroredY;
                rPropValues[ n++ ].Value <<= bMirroredY;
            }
            if ( nFlags & MSDFF_HANDLE_FLAGS_SWITCHED )
            {
                const rtl::OUString sSwitched( RTL_CONSTASCII_USTRINGPARAM ( "Switched" ) );
                sal_Bool bSwitched = sal_True;
                rPropValues[ n ].Name = sSwitched;
                rPropValues[ n++ ].Value <<= bSwitched;
            }
            if ( nFlags & MSDFF_HANDLE_FLAGS_POLAR )
            {
                const rtl::OUString sPolar( RTL_CONSTASCII_USTRINGPARAM ( "Polar" ) );
                ::drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair aCenter;
                SetEnhancedCustomShapeHandleParameter( aCenter.First,  pData->nCenterX,
                    ( nFlags & MSDFF_HANDLE_FLAGS_CENTER_X_IS_SPECIAL ) != 0, sal_True  );
                SetEnhancedCustomShapeHandleParameter( aCenter.Second, pData->nCenterY,
                    ( nFlags & MSDFF_HANDLE_FLAGS_CENTER_Y_IS_SPECIAL ) != 0, sal_False );
                rPropValues[ n ].Name = sPolar;
                rPropValues[ n++ ].Value <<= aCenter;
                if ( nFlags & MSDFF_HANDLE_FLAGS_RADIUS_RANGE )
                {
                    if ( pData->nRangeXMin != 0x80000000 )
                    {
                        const rtl::OUString sRadiusRangeMinimum( RTL_CONSTASCII_USTRINGPARAM ( "RadiusRangeMinimum" ) );
                        ::drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aRadiusRangeMinimum;
                        SetEnhancedCustomShapeHandleParameter( aRadiusRangeMinimum, pData->nRangeXMin,
                            ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MIN_IS_SPECIAL ) != 0, sal_True  );
                        rPropValues[ n ].Name = sRadiusRangeMinimum;
                        rPropValues[ n++ ].Value <<= aRadiusRangeMinimum;
                    }
                    if ( pData->nRangeXMax != 0x7fffffff )
                    {
                        const rtl::OUString sRadiusRangeMaximum( RTL_CONSTASCII_USTRINGPARAM ( "RadiusRangeMaximum" ) );
                        ::drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aRadiusRangeMaximum;
                        SetEnhancedCustomShapeHandleParameter( aRadiusRangeMaximum, pData->nRangeXMax,
                            ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MAX_IS_SPECIAL ) != 0, sal_False );
                        rPropValues[ n ].Name = sRadiusRangeMaximum;
                        rPropValues[ n++ ].Value <<= aRadiusRangeMaximum;
                    }
                }
            }
            else if ( nFlags & MSDFF_HANDLE_FLAGS_RANGE )
            {
                if ( pData->nRangeXMin != 0x80000000 )
                {
                    const rtl::OUString sRangeXMinimum( RTL_CONSTASCII_USTRINGPARAM ( "RangeXMinimum" ) );
                    ::drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeXMinimum;
                    SetEnhancedCustomShapeHandleParameter( aRangeXMinimum, pData->nRangeXMin,
                        ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MIN_IS_SPECIAL ) != 0, sal_True  );
                    rPropValues[ n ].Name = sRangeXMinimum;
                    rPropValues[ n++ ].Value <<= aRangeXMinimum;
                }
                if ( pData->nRangeXMax != 0x7fffffff )
                {
                    const rtl::OUString sRangeXMaximum( RTL_CONSTASCII_USTRINGPARAM ( "RangeXMaximum" ) );
                    ::drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeXMaximum;
                    SetEnhancedCustomShapeHandleParameter( aRangeXMaximum, pData->nRangeXMax,
                        ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MAX_IS_SPECIAL ) != 0, sal_False );
                    rPropValues[ n ].Name = sRangeXMaximum;
                    rPropValues[ n++ ].Value <<= aRangeXMaximum;
                }
                if ( pData->nRangeYMin != 0x80000000 )
                {
                    const rtl::OUString sRangeYMinimum( RTL_CONSTASCII_USTRINGPARAM ( "RangeYMinimum" ) );
                    ::drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeYMinimum;
                    SetEnhancedCustomShapeHandleParameter( aRangeYMinimum, pData->nRangeYMin,
                        ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_Y_MIN_IS_SPECIAL ) != 0, sal_True );
                    rPropValues[ n ].Name = sRangeYMinimum;
                    rPropValues[ n++ ].Value <<= aRangeYMinimum;
                }
                if ( pData->nRangeYMax != 0x7fffffff )
                {
                    const rtl::OUString sRangeYMaximum( RTL_CONSTASCII_USTRINGPARAM ( "RangeYMaximum" ) );
                    ::drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeYMaximum;
                    SetEnhancedCustomShapeHandleParameter( aRangeYMaximum, pData->nRangeYMax,
                        ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_Y_MAX_IS_SPECIAL ) != 0, sal_False );
                    rPropValues[ n ].Name = sRangeYMaximum;
                    rPropValues[ n++ ].Value <<= aRangeYMaximum;
                }
            }
        }
    }
    return pDefData;
}

EnhancedCustomShape2d::~EnhancedCustomShape2d()
{
}

EnhancedCustomShape2d::EnhancedCustomShape2d( SdrObject* pAObj ) :
    pCustomShapeObj     ( pAObj ),
    SfxItemSet          ( pAObj->GetMergedItemSet() ),
    eSpType             ( mso_sptNil ),
    nXRef               ( 0x80000000 ),
    nYRef               ( 0x80000000 ),
    nCoordWidth         ( 21600 ),
    nCoordHeight        ( 21600 ),
    nFlags              ( 0 ),
    nColorData          ( 0 ),
    bTextFlow           ( sal_False ),
    bFilled             ( ((const XFillStyleItem&)pAObj->GetMergedItem( XATTR_FILLSTYLE )).GetValue() != XFILL_NONE ),
    bFlipH              ( sal_False ),
    bFlipV              ( sal_False )
{
    Point aP( pCustomShapeObj->GetSnapRect().Center() );
    Size aS( pCustomShapeObj->GetLogicRect().GetSize() );
    aP.X() -= aS.Width() / 2;
    aP.Y() -= aS.Height() / 2;
    aLogicRect = Rectangle( aP, aS );

    const rtl::OUString sPredefinedType( RTL_CONSTASCII_USTRINGPARAM ( "PredefinedType" ) );
    const rtl::OUString sMirroredX( RTL_CONSTASCII_USTRINGPARAM ( "MirroredX" ) );
    const rtl::OUString sMirroredY( RTL_CONSTASCII_USTRINGPARAM ( "MirroredY" ) );

    rtl::OUString sShapeType;
    SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)(const SdrCustomShapeGeometryItem&)pCustomShapeObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
    Any* pAny = rGeometryItem.GetPropertyValueByName( sPredefinedType );
    if ( pAny )
        *pAny >>= sShapeType;
    eSpType = EnhancedCustomShapeTypeNames::Get( sShapeType );

    pAny = rGeometryItem.GetPropertyValueByName( sMirroredX );
    if ( pAny )
        *pAny >>= bFlipH;
    pAny = rGeometryItem.GetPropertyValueByName( sMirroredY );
    if ( pAny )
        *pAny >>= bFlipV;

    if ( pCustomShapeObj->ISA( SdrObjCustomShape ) )    // should always be a SdrObjCustomShape, but you don't know
        nRotateAngle = (sal_Int32)(((SdrObjCustomShape*)pCustomShapeObj)->GetObjectRotation() * 100.0);
    else
         nRotateAngle = pCustomShapeObj->GetRotateAngle();

    const sal_Int32* pDefData = ApplyShapeAttributes( rGeometryItem );
    switch( eSpType )
    {
        case mso_sptCan :                       nColorData = 0x20200000; break;
        case mso_sptCube :                      nColorData = 0x302d0000; break;
        case mso_sptActionButtonBlank :         nColorData = 0x502ad400; break;
        case mso_sptActionButtonHome :          nColorData = 0x702ad4ad; break;
        case mso_sptActionButtonHelp :          nColorData = 0x602ad4a0; break;
        case mso_sptActionButtonInformation :   nColorData = 0x702ad4a5; break;
        case mso_sptActionButtonBackPrevious :  nColorData = 0x602ad4a0; break;
        case mso_sptActionButtonForwardNext :   nColorData = 0x602ad4a0; break;
        case mso_sptActionButtonBeginning :     nColorData = 0x602ad4a0; break;
        case mso_sptActionButtonEnd :           nColorData = 0x602ad4a0; break;
        case mso_sptActionButtonReturn :        nColorData = 0x602ad4a0; break;
        case mso_sptActionButtonDocument :      nColorData = 0x702ad4da; break;
        case mso_sptActionButtonSound :         nColorData = 0x602ad4a0; break;
        case mso_sptActionButtonMovie :         nColorData = 0x602ad4a0; break;
        case mso_sptBevel :                     nColorData = 0x502ad400; break;
        case mso_sptFoldedCorner :              nColorData = 0x20d00000; break;
        case mso_sptSmileyFace :                nColorData = 0x20d00000; break;
        case mso_sptCurvedLeftArrow :
        {
            if ( ( seqAdjustmentValues.getLength() > 2 ) && ( seqAdjustmentValues[ 2 ].State == com::sun::star::beans::PropertyState_DIRECT_VALUE ) )
            {
                double fValue;
                seqAdjustmentValues[ 2 ].Value >>= fValue;
                fValue = 21600 - fValue;
                seqAdjustmentValues[ 2 ].Value <<= fValue;
            }
            nFlags |= DFF_CUSTOMSHAPE_FLIP_H;
        }
        break;
        case mso_sptCurvedUpArrow :
        {
            if ( ( seqAdjustmentValues.getLength() > 2 ) && ( seqAdjustmentValues[ 2 ].State == com::sun::star::beans::PropertyState_DIRECT_VALUE ) )
            {
                double fValue;
                seqAdjustmentValues[ 2 ].Value >>= fValue;
                fValue = 21600 - fValue;
                seqAdjustmentValues[ 2 ].Value <<= fValue;
            }
            nFlags |= DFF_CUSTOMSHAPE_FLIP_V | DFF_CUSTOMSHAPE_EXCH;
        }
        break;
        case mso_sptCurvedDownArrow :           nFlags |= DFF_CUSTOMSHAPE_EXCH; break;
        case mso_sptRibbon2 :                   nColorData = 0x30dd0000; break;
        case mso_sptRibbon :
        {
            if ( ( seqAdjustmentValues.getLength() > 1 ) && ( seqAdjustmentValues[ 1 ].State == com::sun::star::beans::PropertyState_DIRECT_VALUE ) )
            {
                double fValue;
                seqAdjustmentValues[ 1 ].Value >>= fValue;
                fValue = 21600 - fValue;
                seqAdjustmentValues[ 1 ].Value <<= fValue;
            }
            nFlags |= DFF_CUSTOMSHAPE_FLIP_V;
            nColorData = 0x30dd0000;
        }
        break;
        case mso_sptEllipseRibbon2 :            nColorData = 0x30dd0000; break;
        case mso_sptEllipseRibbon :             // !!!!!!!!!!
        {
            if ( ( seqAdjustmentValues.getLength() > 1 ) && ( seqAdjustmentValues[ 1 ].State == com::sun::star::beans::PropertyState_DIRECT_VALUE ) )
            {
                double fValue;
                seqAdjustmentValues[ 1 ].Value >>= fValue;
                fValue = 21600 - fValue;
                seqAdjustmentValues[ 1 ].Value <<= fValue;
            }
            nFlags |= DFF_CUSTOMSHAPE_FLIP_V;
            nColorData = 0x30dd0000;
        }
        break;
        case mso_sptVerticalScroll :            nColorData = 0x30dd0000; break;
        case mso_sptHorizontalScroll :          nColorData = 0x30dd0000; break;
    }
    fXScale = (double)aLogicRect.GetWidth() / (double)nCoordWidth;
    fYScale = (double)aLogicRect.GetHeight() / (double)nCoordHeight;
}

double EnhancedCustomShape2d::GetAdjustValueAsDouble( const sal_Int32 nIndex ) const
{
    double fNumber = 0.0;
    if ( nIndex < seqAdjustmentValues.getLength() )
    {
        if ( seqAdjustmentValues[ nIndex ].Value.getValueTypeClass() == TypeClass_DOUBLE )
            seqAdjustmentValues[ nIndex ].Value >>= fNumber;
        else
        {
            sal_Int32 nNumber;
            seqAdjustmentValues[ nIndex ].Value >>= nNumber;
            fNumber = (double)nNumber;
        }
    }
    return fNumber;
}
sal_Int32 EnhancedCustomShape2d::GetAdjustValueAsInteger( const sal_Int32 nIndex, const sal_Int32 nDefault ) const
{
    sal_Int32 nNumber = nDefault;
    if ( nIndex < seqAdjustmentValues.getLength() )
    {
        if ( seqAdjustmentValues[ nIndex ].Value.getValueTypeClass() == TypeClass_DOUBLE )
        {
            double fNumber;
            seqAdjustmentValues[ nIndex ].Value >>= fNumber;
            nNumber = (sal_Int32)fNumber;
        }
        else
            seqAdjustmentValues[ nIndex ].Value >>= nNumber;
    }
    return nNumber;
}
sal_Bool EnhancedCustomShape2d::SetAdjustValueAsDouble( const double& rValue, const sal_Int32 nIndex )
{
    sal_Bool bRetValue = sal_False;
    if ( nIndex < seqAdjustmentValues.getLength() )
    {
        // updating our local adjustment sequence
        seqAdjustmentValues[ nIndex ].Value <<= rValue;
        seqAdjustmentValues[ nIndex ].State = com::sun::star::beans::PropertyState_DIRECT_VALUE;
/*
        Any* pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sAdjustmentValues );
        if ( pAny )
            *pAny <<= seqAdjustmentValues;
*/
        bRetValue = sal_True;
    }
    return bRetValue;
}

Point EnhancedCustomShape2d::GetPoint( const drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair& rPair, sal_Bool bScale ) const
{
    Point       aRetValue;
    sal_Bool    bExchange = ( nFlags & DFF_CUSTOMSHAPE_EXCH ) != 0; // x <-> y
    sal_uInt32  nPass = 0;
    do
    {
        sal_uInt32  nIndex = nPass;

        if ( bExchange )
            nIndex ^= 1;

        double      fVal;
        sal_Bool    bScaleWidth = nPass == 0;
        sal_uInt32  nGeometryFlags = 0;
        const EnhancedCustomShapeParameter& rParameter = nIndex ? rPair.Second : rPair.First;
        GetParameter( fVal, nGeometryFlags, rParameter );
        if ( bScale )
        {
            if ( nGeometryFlags & bExchange )   // left <-> top, right <-> bottom
            {
                nGeometryFlags = ( ( nGeometryFlags & 1 ) << 1 ) | ( ( nGeometryFlags & 2 ) >> 1 ) |
                                    ( ( nGeometryFlags & 4 ) << 1 ) | ( ( nGeometryFlags & 8 ) >> 1 );
            }
            if ( bScaleWidth )
            {
                if ( ( aLogicRect.GetWidth() > aLogicRect.GetHeight() ) && ( ( nXRef != 0x80000000 ) || nGeometryFlags ) )
                {
                    sal_Bool bGeo = ( ( ( nGeometryFlags & GEOMETRY_USED_LEFT ) == 0 ) && ( fVal > nXRef ) )
                                        || ( ( nGeometryFlags & GEOMETRY_USED_RIGHT ) != 0 );
                    if ( ( nGeometryFlags & ( GEOMETRY_USED_LEFT | GEOMETRY_USED_RIGHT ) ) == ( GEOMETRY_USED_LEFT | GEOMETRY_USED_RIGHT ) )
                    {
                        fVal -= (double)nCoordWidth * 0.5;
                        fVal *= fYScale;
                        fVal += (double)aLogicRect.GetWidth() * 0.5;
                    }
                    else
                    {
                        fVal *= fYScale;
                        if ( bGeo )
                            fVal += (double)nCoordWidth * fXScale - (double)nCoordWidth * fYScale;
                    }
                }
                else
                    fVal *= fXScale;
                if ( nFlags & DFF_CUSTOMSHAPE_FLIP_H )
                    fVal = aLogicRect.GetWidth() - fVal;
            }
            else
            {
                if ( ( aLogicRect.GetHeight() > aLogicRect.GetWidth() ) && ( ( nYRef != 0x80000000 ) || nGeometryFlags ) )
                {
                    sal_Bool bGeo = ( ( ( nGeometryFlags & GEOMETRY_USED_TOP ) == 0 ) && ( fVal > nYRef ) )
                                        || ( ( nGeometryFlags & GEOMETRY_USED_BOTTOM ) != 0 );
                    if ( ( nGeometryFlags & ( GEOMETRY_USED_TOP | GEOMETRY_USED_BOTTOM ) ) == ( GEOMETRY_USED_TOP | GEOMETRY_USED_BOTTOM ) )
                    {
                        fVal -= (double)nCoordHeight * 0.5;
                        fVal *= fXScale;
                        fVal += (double)aLogicRect.GetHeight() * 0.5;
                    }
                    else
                    {
                        fVal *= fXScale;
                        if ( bGeo )
                            fVal += (double)nCoordHeight * fYScale - (double)nCoordHeight * fXScale;
                    }
                }
                else
                    fVal *= fYScale;
                if ( nFlags & DFF_CUSTOMSHAPE_FLIP_V )
                    fVal = aLogicRect.GetHeight() - fVal;
            }
        }
        if ( nPass )
            aRetValue.Y() = (sal_Int32)fVal;
        else
            aRetValue.X() = (sal_Int32)fVal;
    }
    while ( ++nPass < 2 );
    return aRetValue;
}

sal_Bool EnhancedCustomShape2d::GetParameter( double& rRetValue, sal_uInt32& nGeometryFlags,
                        const EnhancedCustomShapeParameter& rParameter ) const
{
    rRetValue = 0.0;
    sal_Bool bRetValue = sal_False;
    switch ( rParameter.Type )
    {
        case EnhancedCustomShapeParameterType::ADJUSTMENT :
        {
            sal_Int32 nAdjustmentIndex;
            if ( rParameter.Value >>= nAdjustmentIndex )
            {
                rRetValue = GetAdjustValueAsDouble( nAdjustmentIndex );
                bRetValue = sal_True;
            }
        }
        break;
        case EnhancedCustomShapeParameterType::EQUATION :
        {
            sal_Int32 nEquationIndex;
            if ( rParameter.Value >>= nEquationIndex )
            {
                rRetValue = GetEquationValue( nEquationIndex, nGeometryFlags );
                bRetValue = sal_True;
            }
        }
        break;
        case EnhancedCustomShapeParameterType::NORMAL :
        {
            if ( rParameter.Value.getValueTypeClass() == TypeClass_DOUBLE )
            {
                double fValue;
                if ( rParameter.Value >>= fValue )
                {
                    rRetValue = fValue;
                    bRetValue = sal_True;
                }
            }
            else
            {
                sal_Int32 nValue;
                if ( rParameter.Value >>= nValue )
                {
                    rRetValue = nValue;
                    bRetValue = sal_True;
                }
            }
        }
        break;
        case EnhancedCustomShapeParameterType::LEFT :
        {
            nGeometryFlags |= GEOMETRY_USED_LEFT;
            rRetValue  = 0.0;
            bRetValue = sal_True;
        }
        break;
        case EnhancedCustomShapeParameterType::TOP :
        {
            nGeometryFlags |= GEOMETRY_USED_TOP;
            rRetValue  = 0.0;
            bRetValue = sal_True;
        }
        break;
        case EnhancedCustomShapeParameterType::RIGHT :
        {
            nGeometryFlags |= GEOMETRY_USED_RIGHT;
            rRetValue = nCoordWidth;
            bRetValue = sal_True;
        }
        break;
        case EnhancedCustomShapeParameterType::BOTTOM :
        {
            nGeometryFlags |= GEOMETRY_USED_BOTTOM;
            rRetValue = nCoordHeight;
            bRetValue = sal_True;
        }
        break;
        case EnhancedCustomShapeParameterType::CENTER :
        {
            rRetValue = nCoordWidth / 2;
            bRetValue = sal_True;
        }
        break;
    }
    return bRetValue;
}

sal_Bool EnhancedCustomShape2d::GetEquationParameter( double& rParameterReturnValue, sal_uInt32& nGeometryFlags,
                                const drafts::com::sun::star::drawing::EnhancedCustomShapeEquation& rEquation, const sal_Int32 nIndex ) const
{
    rParameterReturnValue = 0.0;
    sal_Bool bRetValue = rEquation.Parameters.getLength() > nIndex;
    if ( bRetValue )
        bRetValue = GetParameter( rParameterReturnValue, nGeometryFlags, rEquation.Parameters[ nIndex ] );
    return bRetValue;
}

double EnhancedCustomShape2d::GetEquationValue( sal_Int32 nIndex, sal_uInt32& nGeometryFlags ) const
{
    double fP1, fP2, fRetValue = 0;
    if ( seqEquations.getLength() > nIndex )
    {
        const EnhancedCustomShapeEquation& rEquation = seqEquations[ nIndex ];
        GetEquationParameter( fRetValue, nGeometryFlags, rEquation, 0 );
        switch( rEquation.Operation )
        {
            case EnhancedCustomShapeOperation::SUM :
            {
                if ( GetEquationParameter( fP1, nGeometryFlags, rEquation, 1 ) )
                    fRetValue += fP1;
                if ( GetEquationParameter( fP2, nGeometryFlags, rEquation, 2 ) )
                    fRetValue -= fP2;
            }
            break;
            case EnhancedCustomShapeOperation::PROD :
            {
                if ( GetEquationParameter( fP1, nGeometryFlags, rEquation, 1 ) && ( fP1 != 0.0 ) )
                    fRetValue *= fP1;
                if ( GetEquationParameter( fP2, nGeometryFlags, rEquation, 2 ) && ( fP2 != 0.0 ) )
                    fRetValue /= fP2;
            }
            break;
            case EnhancedCustomShapeOperation::MID :
            {
                if ( GetEquationParameter( fP1, nGeometryFlags, rEquation, 1 ) )
                {
                    fRetValue += fP1;
                    fRetValue /= 2.0;
                }
            }
            break;
            case EnhancedCustomShapeOperation::ABS :
            {
                fRetValue = fabs( fRetValue );
            }
            break;
            case EnhancedCustomShapeOperation::MIN :
            {
                if ( GetEquationParameter( fP1, nGeometryFlags, rEquation, 1 ) )
                {
                    if ( fP1 < fRetValue )
                        fRetValue = fP1;
                }
            }
            break;
            case EnhancedCustomShapeOperation::MAX :
            {
                if ( GetEquationParameter( fP1, nGeometryFlags, rEquation, 1 ) )
                {
                    if ( fP1 > fRetValue )
                        fRetValue = fP1;
                }
            }
            break;
            case EnhancedCustomShapeOperation::IF :
            {
                if ( GetEquationParameter( fP1, nGeometryFlags, rEquation, 1 ) && GetEquationParameter( fP2, nGeometryFlags, rEquation, 2 ) )
                {
                    if ( fRetValue > 0.0 )
                        fRetValue = fP1;
                    else
                        fRetValue = fP2;
                }
            }
            break;
            case EnhancedCustomShapeOperation::MOD :
            {
                GetEquationParameter( fP1, nGeometryFlags, rEquation, 1 );
                GetEquationParameter( fP2, nGeometryFlags, rEquation, 2 );
                fRetValue = sqrt( fRetValue * fRetValue + fP1 * fP1 + fP2 * fP2 );
            }
            break;
            case EnhancedCustomShapeOperation::ATAN2 :
            {
                if ( GetEquationParameter( fP1, nGeometryFlags, rEquation, 1 ) )
                {
                    fRetValue = atan2( fP1, fRetValue ) / F_PI180;
//                  fRetValue *= 65536.0;
                }
            }
            break;
            case EnhancedCustomShapeOperation::SIN :
            {
                if ( GetEquationParameter( fP1, nGeometryFlags, rEquation, 1 ) )
                    fRetValue *= sin( ( fP1 /* / 65536 */ ) * F_PI180 );
            }
            break;
            case EnhancedCustomShapeOperation::COS :
            {
                if ( GetEquationParameter( fP1, nGeometryFlags, rEquation, 1 ) )
                    fRetValue *= cos( ( fP1 /* / 65536 */ ) * F_PI180 );
            }
            break;
            case EnhancedCustomShapeOperation::COSATAN2 :
            {
                if ( GetEquationParameter( fP1, nGeometryFlags, rEquation, 1 ) && GetEquationParameter( fP2, nGeometryFlags, rEquation, 2 ) )
                    fRetValue *= cos( atan2( fP2, fP1 ) );
            }
            break;
            case EnhancedCustomShapeOperation::SINATAN2 :
            {
                if ( GetEquationParameter( fP1, nGeometryFlags, rEquation, 1 ) && GetEquationParameter( fP2, nGeometryFlags, rEquation, 2 ) )
                    fRetValue *= sin( atan2( fP2, fP1 ) );
            }
            break;
            case EnhancedCustomShapeOperation::SQRT :
            {
                fRetValue = sqrt( fRetValue );
            }
            break;
            case EnhancedCustomShapeOperation::SUMANGLE :
            {
                if ( GetEquationParameter( fP1, nGeometryFlags, rEquation, 1 ) )
                    fRetValue += fP1; //* 65536.0;
                if ( GetEquationParameter( fP2, nGeometryFlags, rEquation, 2 ) )
                    fRetValue -= fP2; //* 65536.0;
            }
            break;
            case EnhancedCustomShapeOperation::ELLIPSE :
            {
                if ( GetEquationParameter( fP1, nGeometryFlags, rEquation, 1 ) && GetEquationParameter( fP2, nGeometryFlags, rEquation, 2 ) && ( fP1 != 0.0 ) )
                {
                    fRetValue /= fP1;
                    fRetValue = fP2 * sqrt( 1 - fRetValue * fRetValue );
                }
            }
            break;
            case EnhancedCustomShapeOperation::TAN :
            {
                if ( GetEquationParameter( fP1, nGeometryFlags, rEquation, 1 ) )
                    fRetValue *= tan( fP1 );
            }
            break;

            case 0x80 :
            {
                // fVal[0]^2 + fVal[1]^2 = fVal[2]^2
                if ( GetEquationParameter( fP1, nGeometryFlags, rEquation, 1 ) && GetEquationParameter( fP2, nGeometryFlags, rEquation, 2 ) )
                {
                    if ( fP2 == 0.0 )
                        fRetValue = sqrt( fRetValue * fRetValue + fP1 * fP1 );
                    else
                    {
                        double fA = fRetValue != 0.0 ? fRetValue : fP1;
                        fRetValue = sqrt( fP2 * fP2 - fA * fA );
                    }
                }
            }
            break;
            case 0x81 :
            {
                if ( GetEquationParameter( fP1, nGeometryFlags, rEquation, 1 ) && GetEquationParameter( fP2, nGeometryFlags, rEquation, 2 ) )
                {
                    double fAngle = F_PI1800 * fP2;
                    fRetValue = ( cos( fAngle ) * ( fRetValue - 10800 )
                                    + sin( fAngle ) * ( fP1 - 10800 ) ) + 10800;
                }
            }
            break;
            case 0x82 :
            {
                if ( GetEquationParameter( fP1, nGeometryFlags, rEquation, 1 ) && GetEquationParameter( fP2, nGeometryFlags, rEquation, 2 ) )
                {
                    double fAngle = F_PI1800 * fP2;
                    fRetValue = - ( sin( fAngle ) * ( fRetValue - 10800 )
                                    - cos( fAngle ) * ( fP1 - 10800 ) ) + 10800;
                }
            }
            break;
        }
    }
    return fRetValue;
}

// nLumDat 28-31 = number of luminance entries in nLumDat
// nLumDat 27-24 = nLumDatEntry 0
// nLumDat 23-20 = nLumDatEntry 1 ...
// each 4bit entry is to be interpreted as a 10 percent signed luminance changing
Color EnhancedCustomShape2d::GetColorData( const Color& rFillColor, sal_uInt32 nIndex )
{
    Color aRetColor;

    sal_uInt32 i, nColor, nTmp, nCount = nColorData >> 28;

    if ( nCount )
    {
        if ( nIndex >= nCount )
            nIndex = nCount - 1;

        sal_uInt32 nFillColor = (sal_uInt32)rFillColor.GetRed() |
                                    ((sal_uInt32)rFillColor.GetGreen() << 8 ) |
                                        ((sal_uInt32)rFillColor.GetBlue() << 16 );

        sal_Int32 nLumDat = nColorData << ( ( 1 + nIndex ) << 2 );
        sal_Int32 nLuminance = ( nLumDat >> 28 ) * 12;

        nTmp = nFillColor;
        nColor = 0;
        for ( i = 0; i < 3; i++ )
        {
            sal_Int32 nC = (sal_uInt8)nTmp;
            nTmp >>= 8;
            nC += ( ( nLuminance * nC ) >> 8 );
            if ( nC < 0 )
                nC = 0;
            else if ( nC &~ 0xff )
                nC = 0xff;
            nColor >>= 8;
            nColor |= nC << 16;
        }
        aRetColor = Color( (sal_uInt8)nColor, (sal_uInt8)( nColor >> 8 ), (sal_uInt8)( nColor >> 16 ) );
    }
    return aRetColor;
}

Rectangle EnhancedCustomShape2d::GetTextRect() const
{
    sal_Int32 nIndex, nSize = seqTextFrames.getLength();
    if ( !nSize )
        return aLogicRect;
    nIndex = 0;
    if ( bTextFlow && ( nSize > 1 ) )
        nIndex++;
    Point aTopLeft( GetPoint( seqTextFrames[ nIndex ].TopLeft, sal_True ) );
    Point aBottomRight( GetPoint( seqTextFrames[ nIndex ].BottomRight, sal_True ) );
    Rectangle aRect( aTopLeft, aBottomRight );
    aRect.Move( aLogicRect.Left(), aLogicRect.Top() );
    return aRect;
}

sal_uInt32 EnhancedCustomShape2d::GetHdlCount() const
{
    return seqHandles.getLength();
}

sal_Bool EnhancedCustomShape2d::GetHandlePosition( const sal_uInt32 nIndex, Point& rReturnPosition ) const
{
    sal_Bool bRetValue = sal_False;
    if ( nIndex < GetHdlCount() )
    {
        Handle aHandle;
        if ( ConvertSequenceToEnhancedCustomShape2dHandle( seqHandles[ nIndex ], aHandle ) )
        {
            if ( aHandle.nFlags & HANDLE_FLAGS_POLAR )
            {
                Point aReferencePoint( GetPoint( aHandle.aPolar, sal_True ) );

                double      fAngle;
                double      fRadius;
                sal_uInt32  nGeometryFlags = 0;
                GetParameter( fRadius, nGeometryFlags, aHandle.aPosition.First );
                GetParameter( fAngle,  nGeometryFlags, aHandle.aPosition.Second );

                double a = ( 360.0 - fAngle ) * F_PI180;
                double dx = fRadius * fXScale;
                double fX = dx * cos( a );
                double fY =-dx * sin( a );
                rReturnPosition = Point( Round( fX + aReferencePoint.X() ), Round( ( fY * fYScale ) / fXScale + aReferencePoint.Y() ) );
            }
            else
            {
                if ( aHandle.nFlags & HANDLE_FLAGS_SWITCHED )
                {
                    if ( aLogicRect.GetHeight() > aLogicRect.GetWidth() )
                    {
                        drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aFirst = aHandle.aPosition.First;
                        drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aSecond = aHandle.aPosition.Second;
                        aHandle.aPosition.First = aSecond;
                        aHandle.aPosition.Second = aFirst;
                    }
                }
                rReturnPosition = GetPoint( aHandle.aPosition, sal_True );
            }
            if ( nRotateAngle )
            {
                double a = nRotateAngle * F_PI18000;
                RotatePoint( rReturnPosition, Point( aLogicRect.GetWidth() / 2, aLogicRect.GetHeight() / 2 ), sin( a ), cos( a ) );
            }
            if ( bFlipH )
                rReturnPosition.X() = aLogicRect.GetWidth() - rReturnPosition.X();
            if ( bFlipV )
                rReturnPosition.Y() = aLogicRect.GetHeight() - rReturnPosition.Y();
            rReturnPosition.Move( aLogicRect.Left(), aLogicRect.Top() );
            bRetValue = sal_True;
        }
    }
    return bRetValue;
}

sal_Bool EnhancedCustomShape2d::SetHandleControllerPosition( const sal_uInt32 nIndex, const com::sun::star::awt::Point& rPosition )
{
    sal_Bool bRetValue = sal_False;
    if ( nIndex < GetHdlCount() )
    {
        Handle aHandle;
        if ( ConvertSequenceToEnhancedCustomShape2dHandle( seqHandles[ nIndex ], aHandle ) )
        {
            sal_Bool bAdjFirst = aHandle.aPosition.First.Type == EnhancedCustomShapeParameterType::ADJUSTMENT;
            sal_Bool bAdjSecond= aHandle.aPosition.Second.Type == EnhancedCustomShapeParameterType::ADJUSTMENT;
            if ( bAdjFirst || bAdjSecond )
            {
                Point aP( rPosition.X, rPosition.Y );
                // apply the negative object rotation to the controller position

                aP.Move( -aLogicRect.Left(), -aLogicRect.Top() );
                if ( bFlipH )
                    aP.X() = aLogicRect.GetWidth() - aP.X();
                if ( bFlipV )
                    aP.Y() = aLogicRect.GetHeight() - aP.Y();
                if ( nRotateAngle )
                {
                    double a = -nRotateAngle * F_PI18000;
                    RotatePoint( aP, Point( aLogicRect.GetWidth() / 2, aLogicRect.GetHeight() / 2 ), sin( a ), cos( a ) );
                }
                double fPos1 = aP.X();  //( bFlipH ) ? aLogicRect.GetWidth() - aP.X() : aP.X();
                double fPos2 = aP.Y();  //( bFlipV ) ? aLogicRect.GetHeight() -aP.Y() : aP.Y();
                fPos1 /= fXScale;
                fPos2 /= fYScale;

                if ( aHandle.nFlags & HANDLE_FLAGS_SWITCHED )
                {
                    if ( aLogicRect.GetHeight() > aLogicRect.GetWidth() )
                    {
                        double fX = fPos1;
                        double fY = fPos2;
                        fPos1 = fY;
                        fPos2 = fX;
                    }
                }

                sal_Int32 nFirstAdjustmentValue, nSecondAdjustmentValue;
                aHandle.aPosition.First.Value >>= nFirstAdjustmentValue;
                aHandle.aPosition.Second.Value>>= nSecondAdjustmentValue;

                if ( aHandle.nFlags & HANDLE_FLAGS_POLAR )
                {
                    sal_uInt32 nDummy;
                    double fXRef, fYRef, fAngle;
                    GetParameter( fXRef, nDummy, aHandle.aPolar.First );
                    GetParameter( fYRef, nDummy, aHandle.aPolar.Second );
                    const double fDX = fPos1 - fXRef;
                    fAngle = -( atan2( -fPos2 + fYRef, ( ( fDX == 0.0L ) ? 0.000000001 : fDX ) ) / F_PI180 );
                    double fX = ( fPos1 - fXRef );
                    double fY = ( fPos2 - fYRef );
                    double fRadius = sqrt( fX * fX + fY * fY );
                    if ( aHandle.nFlags & HANDLE_FLAGS_RADIUS_RANGE_MINIMUM )
                    {
                        double fMin;
                        GetParameter( fMin, nDummy, aHandle.aRadiusRangeMinimum );
                        if ( fRadius < fMin )
                            fRadius = fMin;
                    }
                    if ( aHandle.nFlags & HANDLE_FLAGS_RADIUS_RANGE_MAXIMUM )
                    {
                        double fMax;
                        GetParameter( fMax, nDummy, aHandle.aRadiusRangeMaximum );
                        if ( fRadius > fMax )
                            fRadius = fMax;
                    }
                    if ( bAdjFirst )
                        SetAdjustValueAsDouble( fRadius, nFirstAdjustmentValue );
                    if ( bAdjSecond )
                        SetAdjustValueAsDouble( fAngle,  nSecondAdjustmentValue );
                }
                else
                {
                    if ( bAdjFirst )
                    {
                        if ( aHandle.nFlags & HANDLE_FLAGS_RANGE_X_MINIMUM )        // check if horizontal handle needs to be within a range
                        {
                            sal_uInt32 nDummy;
                            double fXMin;
                            GetParameter( fXMin, nDummy, aHandle.aXRangeMinimum );
                            if ( fPos1 < fXMin )
                                fPos1 = fXMin;
                        }
                        if ( aHandle.nFlags & HANDLE_FLAGS_RANGE_X_MAXIMUM )        // check if horizontal handle needs to be within a range
                        {
                            sal_uInt32 nDummy;
                            double fXMax;
                            GetParameter( fXMax, nDummy, aHandle.aXRangeMaximum );
                            if ( fPos1 > fXMax )
                                fPos1 = fXMax;
                        }
                        SetAdjustValueAsDouble( fPos1, nFirstAdjustmentValue );
                    }
                    if ( bAdjSecond )
                    {
                        if ( aHandle.nFlags & HANDLE_FLAGS_RANGE_Y_MINIMUM )        // check if vertical handle needs to be within a range
                        {
                            sal_uInt32 nDummy;
                            double fYMin;
                            GetParameter( fYMin, nDummy, aHandle.aYRangeMinimum );
                            if ( fPos2 < fYMin )
                                fPos2 = fYMin;
                        }
                        if ( aHandle.nFlags & HANDLE_FLAGS_RANGE_Y_MAXIMUM )        // check if vertical handle needs to be within a range
                        {
                            sal_uInt32 nDummy;
                            double fYMax;
                            GetParameter( fYMax, nDummy, aHandle.aYRangeMaximum );
                            if ( fPos2 > fYMax )
                                fPos2 = fYMax;
                        }
                        SetAdjustValueAsDouble( fPos2, nSecondAdjustmentValue );
                    }
                }
                // and writing them back into the GeometryItem
                SdrCustomShapeGeometryItem aGeometryItem((SdrCustomShapeGeometryItem&)
                    (const SdrCustomShapeGeometryItem&)pCustomShapeObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ));
                const rtl::OUString sAdjustmentValues( RTL_CONSTASCII_USTRINGPARAM ( "AdjustmentValues" ) );
                com::sun::star::beans::PropertyValue aPropVal;
                aPropVal.Name = sAdjustmentValues;
                aPropVal.Value <<= seqAdjustmentValues;
                aGeometryItem.SetPropertyValue( aPropVal );
                pCustomShapeObj->SetMergedItem( aGeometryItem );
                bRetValue = sal_True;
            }
        }
    }
    return bRetValue;
}

void EnhancedCustomShape2d::SwapStartAndEndArrow( SdrObject* pObj ) //#108274
{
    XLineStartItem       aLineStart;
    aLineStart.SetValue(((XLineStartItem&)pObj->GetMergedItem( XATTR_LINEEND )).GetValue());
    XLineStartWidthItem  aLineStartWidth(((XLineStartWidthItem&)pObj->GetMergedItem( XATTR_LINEENDWIDTH )).GetValue());
    XLineStartCenterItem aLineStartCenter(((XLineStartCenterItem&)pObj->GetMergedItem( XATTR_LINEENDCENTER )).GetValue());

    XLineEndItem         aLineEnd;
    aLineEnd.SetValue(((XLineEndItem&)pObj->GetMergedItem( XATTR_LINESTART )).GetValue());
    XLineEndWidthItem    aLineEndWidth(((XLineEndWidthItem&)pObj->GetMergedItem( XATTR_LINESTARTWIDTH )).GetValue());
    XLineEndCenterItem   aLineEndCenter(((XLineEndCenterItem&)pObj->GetMergedItem( XATTR_LINESTARTCENTER )).GetValue());

    pObj->SetMergedItem( aLineStart );
    pObj->SetMergedItem( aLineStartWidth );
    pObj->SetMergedItem( aLineStartCenter );
    pObj->SetMergedItem( aLineEnd );
    pObj->SetMergedItem( aLineEndWidth );
    pObj->SetMergedItem( aLineEndCenter );
}

void AppendArc( const Rectangle& rRect, const Point& rStart, const Point& rEnd, const sal_Bool bClockwise, XPolygon& rPoly )
{
    Rectangle aRect( rRect );
    Point aStart( rStart );
    Point aEnd( rEnd );

    sal_Int32 bSwapStartEndAngle = 0;

    if ( aRect.Left() > aRect.Right() )
        bSwapStartEndAngle ^= 0x01;
    if ( aRect.Top() > aRect.Bottom() )
        bSwapStartEndAngle ^= 0x11;
    if ( bSwapStartEndAngle )
    {
        aRect.Justify();
        if ( bSwapStartEndAngle & 1 )
        {
            Point aTmp( aStart );
            aStart = aEnd;
            aEnd = aTmp;
        }
    }
    Polygon aTempPoly( aRect, aStart, aEnd, POLY_ARC );

    sal_uInt16 j, nDstPt = rPoly.GetPointCount();
    if ( bClockwise )
    {
        for ( j = aTempPoly.GetSize(); j--; )
            rPoly[ nDstPt++ ] = aTempPoly[ j ];
    }
    else
    {
        for ( j = 0; j < aTempPoly.GetSize(); j++ )
            rPoly[ nDstPt++ ] = aTempPoly[ j ];
    }
}

void EnhancedCustomShape2d::CreateSubPath( sal_uInt16& rSrcPt, sal_uInt16& rSegmentInd, std::vector< SdrPathObj* >& rObjectList,
                                                                                        const sal_Bool bLineGeometryNeededOnly,
                                                                                        const sal_Bool bSortFilledObjectsToBack )
{
    SdrObject* pRet = NULL;

    sal_Bool bNoFill = sal_False;
    sal_Bool bNoStroke = sal_False;

    XPolyPolygon    aPolyPoly;
    XPolygon        aPoly;
    XPolygon        aEmptyPoly;

    sal_Int32 nCoordSize = seqCoordinates.getLength();
    sal_Int32 nSegInfoSize = seqSegments.getLength();
    if ( !nSegInfoSize )
    {
        sal_Int32 nPtNum;
        aPoly = XPolygon( (sal_uInt16)( nCoordSize + 1 ) );
        const EnhancedCustomShapeParameterPair* pTmp = seqCoordinates.getArray();
        for ( nPtNum = 0; nPtNum < nCoordSize; nPtNum++ )
            aPoly[ (sal_uInt16)nPtNum ] = GetPoint( *pTmp++, sal_True );
        if ( aPoly[ 0 ] != aPoly[ (sal_uInt16)( nPtNum - 1 ) ] )
            aPoly[ (sal_uInt16)nPtNum ] = aPoly[ 0 ];
    }
    else
    {
        for ( ;rSegmentInd < nSegInfoSize; )
        {
            sal_Int16 nCommand = seqSegments[ rSegmentInd ].Command;
            sal_Int16 nPntCount= seqSegments[ rSegmentInd++ ].Count;

            switch ( nCommand )
            {
                case NOFILL :
                    bNoFill = sal_True;
                break;
                case NOSTROKE :
                    bNoStroke = sal_True;
                break;
                case MOVETO :
                {
                    if ( aPoly.GetPointCount() > 1 )
                        aPolyPoly.Insert( aPoly );
                    aPoly = aEmptyPoly;
                    if ( rSrcPt < nCoordSize )
                        aPoly[ 0 ] = GetPoint( seqCoordinates[ rSrcPt++ ] );
                }
                break;
                case ENDSUBPATH :
                break;
                case CLOSESUBPATH :
                {
                    sal_uInt16 nDstPt = aPoly.GetPointCount();
                    if ( nDstPt )
                    {
                        aPoly[ nDstPt ] = aPoly[ 0 ];
                        if ( aPoly.GetPointCount() > 1 )
                            aPolyPoly.Insert( aPoly );
                        aPoly = aEmptyPoly;
                    }
                }
                break;
                case CURVETO :
                {
                    sal_uInt16 nDstPt = aPoly.GetPointCount();
                    for ( sal_uInt16 i = 0; ( i < nPntCount ) && ( ( rSrcPt + 2 ) < nCoordSize ); i++ )
                    {
                        aPoly[ nDstPt ] = GetPoint( seqCoordinates[ rSrcPt++ ] );
                        aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                        aPoly[ nDstPt ] = GetPoint( seqCoordinates[ rSrcPt++ ] );
                        aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                        aPoly[ nDstPt++ ] = GetPoint( seqCoordinates[ rSrcPt++ ] );
                    }
                }
                break;

                case ANGLEELLIPSE :
                {
                    if ( aPoly.GetPointCount() > 1 )
                        aPolyPoly.Insert( aPoly );
                    aPoly = aEmptyPoly;
                }
                case ANGLEELLIPSETO :
                {
                    sal_uInt16 nDstPt = aPoly.GetPointCount();
                    for ( sal_uInt16 i = 0; ( i < nPntCount ) && ( ( rSrcPt + 2 ) < nCoordSize ); i++ )
                    {    // create a circle

                        sal_uInt32 nFlags = 0;
                        Point aCenter( GetPoint( seqCoordinates[ rSrcPt ] ) );
                        double fWidth, fHeight;
                        GetParameter( fWidth, nFlags, seqCoordinates[ rSrcPt + 1 ].First  );
                        GetParameter( fHeight, nFlags, seqCoordinates[ rSrcPt + 1 ].Second );
                        fWidth *= fXScale;
                        fHeight*= fYScale;
                        Point aP( (sal_Int32)( aCenter.X() - fWidth ), (sal_Int32)( aCenter.Y() - fHeight ) );
                        Size  aS( (sal_Int32)( fWidth * 2.0 ), (sal_Int32)( fHeight * 2.0 ) );
                        Rectangle aRect( aP, aS );
                        if ( aRect.GetWidth() && aRect.GetHeight() )
                        {
                            double fStartAngle, fEndAngle;
                            GetParameter( fStartAngle, nFlags, seqCoordinates[ rSrcPt + 2 ].First  );
                            GetParameter( fEndAngle  , nFlags, seqCoordinates[ rSrcPt + 2 ].Second );

                            if ( ((sal_Int32)fStartAngle % 360) != ((sal_Int32)fEndAngle % 360) )
                            {
                                if ( (sal_Int32)fStartAngle & 0x7fff0000 )  // SJ: if the angle was imported from our escher import, then the
                                    fStartAngle /= 65536.0;                 // value is shifted by 16. TODO: already change the fixed float to a
                                if ( (sal_Int32)fEndAngle & 0x7fff0000 )    // double in the import filter
                                {
                                    fEndAngle /= 65536.0;
                                    fEndAngle = fEndAngle + fStartAngle;
                                    if ( fEndAngle < 0 )
                                    {   // in the binary filter the endangle is the amount
                                        double fTemp = fStartAngle;
                                        fStartAngle = fEndAngle;
                                        fEndAngle = fStartAngle;
                                    }
                                }
                                double fCenterX = aRect.Center().X();
                                double fCenterY = aRect.Center().Y();
                                double fx1 = ( cos( fStartAngle * F_PI180 ) * 65536.0 * fXScale ) + fCenterX;
                                double fy1 = ( -sin( fStartAngle * F_PI180 ) * 65536.0 * fYScale ) + fCenterY;
                                double fx2 = ( cos( fEndAngle * F_PI180 ) * 65536.0 * fXScale ) + fCenterX;
                                double fy2 = ( -sin( fEndAngle * F_PI180 ) * 65536.0 * fYScale ) + fCenterY;
                                AppendArc( aRect, Point( (sal_Int32)fx1, (sal_Int32)fy1 ), Point( (sal_Int32)fx2, (sal_Int32)fy2 ), sal_False, aPoly );
                            }
                            else
                            {   /* SJ: TODO: this block should be replaced sometimes, because the current point
                                   is not set correct, it also does not use the correct moveto
                                   point if ANGLEELLIPSETO was used, but the method AppendArc
                                   is at the moment not able to draw full circles (if startangle is 0
                                   and endangle 360 nothing is painted :-( */
                                sal_Int32 nXControl = (sal_Int32)((double)aRect.GetWidth() * 0.2835 );
                                sal_Int32 nYControl = (sal_Int32)((double)aRect.GetHeight() * 0.2835 );
                                Point     aCenter( aRect.Center() );
                                aPoly[ nDstPt++ ] = Point( aCenter.X(), aRect.Top() );
                                aPoly[ nDstPt ] = Point( aCenter.X() + nXControl, aRect.Top() );
                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                aPoly[ nDstPt ] = Point( aRect.Right(), aCenter.Y() - nYControl );
                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                aPoly[ nDstPt++ ] = Point( aRect.Right(), aCenter.Y() );
                                aPoly[ nDstPt ] = Point( aRect.Right(), aCenter.Y() + nYControl );
                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                aPoly[ nDstPt ] = Point( aCenter.X() + nXControl, aRect.Bottom() );
                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                aPoly[ nDstPt++ ] = Point( aCenter.X(), aRect.Bottom() );
                                aPoly[ nDstPt ] = Point( aCenter.X() - nXControl, aRect.Bottom() );
                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                aPoly[ nDstPt ] = Point( aRect.Left(), aCenter.Y() + nYControl );
                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                aPoly[ nDstPt++ ] = Point( aRect.Left(), aCenter.Y() );
                                aPoly[ nDstPt ] = Point( aRect.Left(), aCenter.Y() - nYControl );
                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                aPoly[ nDstPt ] = Point( aCenter.X() - nXControl, aRect.Top() );
                                aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                                aPoly[ nDstPt++ ] = Point( aCenter.X(), aRect.Top() );
                            }
                        }
                        rSrcPt += 3;
                    }
                }
                break;

                case LINETO :
                {
                    sal_uInt16 nDstPt = aPoly.GetPointCount();
                    for ( sal_uInt16 i = 0; ( i < nPntCount ) && ( rSrcPt < nCoordSize ); i++ )
                        aPoly[ nDstPt++ ] = GetPoint( seqCoordinates[ rSrcPt++ ] );
                }
                break;

                case ARC :
                case CLOCKWISEARC :
                {
                    if ( aPoly.GetPointCount() > 1 )
                        aPolyPoly.Insert( aPoly );
                    aPoly = aEmptyPoly;
                }
                case ARCTO :
                case CLOCKWISEARCTO :
                {
                    sal_Bool bClockwise = ( nCommand == CLOCKWISEARC ) || ( nCommand == CLOCKWISEARCTO );
                    sal_uInt32 nXor = bClockwise ? 3 : 2;
                    for ( sal_uInt16 i = 0; ( i < nPntCount ) && ( ( rSrcPt + 3 ) < nCoordSize ); i++ )
                    {
                        Rectangle aRect( GetPoint( seqCoordinates[ rSrcPt ] ), GetPoint( seqCoordinates[ rSrcPt + 1 ] ) );
                        Point aCenter( aRect.Center() );
                        Point aStart( GetPoint( seqCoordinates[ (sal_uInt16)( rSrcPt + nXor ) ] ) );
                        Point aEnd( GetPoint( seqCoordinates[ (sal_uInt16)( rSrcPt + ( nXor ^ 1 ) ) ] ) );
                        aStart.X() = (sal_Int32)( ( (double)( aStart.X() - aCenter.X() ) / fXScale ) ) + aCenter.X();
                        aStart.Y() = (sal_Int32)( ( (double)( aStart.Y() - aCenter.Y() ) / fYScale ) ) + aCenter.Y();
                        aEnd.X() = (sal_Int32)( ( (double)( aEnd.X() - aCenter.X() ) / fXScale ) ) + aCenter.X();
                        aEnd.Y() = (sal_Int32)( ( (double)( aEnd.Y() - aCenter.Y() ) / fYScale ) ) + aCenter.Y();
                        AppendArc( aRect, aStart, aEnd, bClockwise, aPoly );
                        rSrcPt += 4;
                    }
                }
                break;

                case ELLIPTICALQUADRANTX :
                case ELLIPTICALQUADRANTY :
                {
                    BOOL    bFirstDirection;
                    sal_uInt16  nDstPt = aPoly.GetPointCount();
                    for ( sal_uInt16 i = 0; ( i < nPntCount ) && ( rSrcPt < nCoordSize ); i++ )
                    {
                        sal_uInt32 nModT = ( nCommand == ELLIPTICALQUADRANTX ) ? 1 : 0;
                        Point aCurrent( GetPoint( seqCoordinates[ rSrcPt ] ) );
                        if ( rSrcPt )   // we need a previous point
                        {
                            Point aPrev( GetPoint( seqCoordinates[ rSrcPt - 1 ] ) );
                            sal_Int32 nX, nY;
                            nX = aCurrent.X() - aPrev.X();
                            nY = aCurrent.Y() - aPrev.Y();
                            if ( ( nY ^ nX ) & 0x80000000 )
                            {
                                if ( !i )
                                    bFirstDirection = TRUE;
                                else if ( !bFirstDirection )
                                    nModT ^= 1;
                            }
                            else
                            {
                                if ( !i )
                                    bFirstDirection = FALSE;
                                else if ( bFirstDirection )
                                    nModT ^= 1;
                            }
                            if ( nModT )            // get the right corner
                            {
                                nX = aCurrent.X();
                                nY = aPrev.Y();
                            }
                            else
                            {
                                nX = aPrev.X();
                                nY = aCurrent.Y();
                            }
                            sal_Int32 nXVec = ( nX - aPrev.X() ) >> 1;
                            sal_Int32 nYVec = ( nY - aPrev.Y() ) >> 1;
                            Point aControl1( aPrev.X() + nXVec, aPrev.Y() + nYVec );
                            aPoly[ nDstPt ] = aControl1;
                            aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                            nXVec = ( nX - aCurrent.X() ) >> 1;
                            nYVec = ( nY - aCurrent.Y() ) >> 1;
                            Point aControl2( aCurrent.X() + nXVec, aCurrent.Y() + nYVec );
                            aPoly[ nDstPt ] = aControl2;
                            aPoly.SetFlags( nDstPt++, XPOLY_CONTROL );
                        }
                        aPoly[ nDstPt ] = aCurrent;
                        rSrcPt++;
                        nDstPt++;
                    }
                }
                break;

    #ifdef DBG_CUSTOMSHAPE
                case UNKNOWN :
                default :
                {
                    ByteString aString( "CustomShapes::unknown PolyFlagValue :" );
                    aString.Append( ByteString::CreateFromInt32( nPolyFlags ) );
                    DBG_ERROR( aString.GetBuffer() );
                }
                break;
    #endif
            }
            if ( nCommand == ENDSUBPATH )
                break;
        }
    }
    if ( rSegmentInd == nSegInfoSize )
        rSegmentInd++;

    if ( aPoly.GetPointCount() > 1 )
        aPolyPoly.Insert( aPoly );
    aPoly = aEmptyPoly;
    if ( aPolyPoly.Count() )
    {
        if ( bLineGeometryNeededOnly )
        {
            SdrPathObj* pStroke = new SdrPathObj( OBJ_PLIN, aPolyPoly );
            pStroke->SetModel( pCustomShapeObj->GetModel() );
            pStroke->SetMergedItemSet( *this );
            pStroke->SetMergedItem( SdrShadowItem( FALSE ) );
            pStroke->SetMergedItem( XFillStyleItem( XFILL_NONE ) );
            rObjectList.push_back( pStroke );
        }
        else
        {
            if ( !bSortFilledObjectsToBack )
            {
                SdrObjKind eObjKind( bNoFill ? OBJ_PLIN : OBJ_POLY );
                SdrPathObj* pObj = new SdrPathObj( eObjKind, aPolyPoly );
                pObj->SetModel( pCustomShapeObj->GetModel() );
                pObj->SetMergedItemSet( *this );
                pObj->SetMergedItem( SdrShadowItem( FALSE ) );
                if ( !bNoFill )
                {   // close object
                    sal_Int16 i, nCnt = aPolyPoly.Count();
                    for ( i = 0; i < nCnt; i++ )
                    {
                        XPolygon& rPoly = aPolyPoly[ i ];
                        Point aPt0( rPoly[ 0 ] );
                        if ( aPt0 != rPoly[ rPoly.GetPointCount() - 1 ] )
                            rPoly[ rPoly.GetPointCount() ] = aPt0;
                    }
                }
                else
                    pObj->SetMergedItem( XFillStyleItem( XFILL_NONE ) );

                if ( bNoStroke )
                    pObj->SetMergedItem( XLineStyleItem( XLINE_NONE ) );
                rObjectList.push_back( pObj );
            }
            else
            {
                if ( !bNoStroke )
                {
                    SdrPathObj* pStroke = new SdrPathObj( OBJ_PLIN, aPolyPoly );
                    pStroke->SetModel( pCustomShapeObj->GetModel() );
                    pStroke->SetMergedItemSet( *this );
                    pStroke->SetMergedItem( SdrShadowItem( FALSE ) );
                    pStroke->SetMergedItem( XFillStyleItem( XFILL_NONE ) );
                    rObjectList.push_back( pStroke );
                }
                if ( bFilled && !bNoFill )
                {
                    sal_Int16 i, nCnt = aPolyPoly.Count();
                    for ( i = 0; i < nCnt; i++ )
                    {
                        XPolygon& rPoly = aPolyPoly[ i ];
                        Point aPt0( rPoly[ 0 ] );
                        if ( aPt0 != rPoly[ rPoly.GetPointCount() - 1 ] )
                            rPoly[ rPoly.GetPointCount() ] = aPt0;
                    }
                    SdrPathObj* pFill = new SdrPathObj( OBJ_POLY, aPolyPoly );
                    pFill->SetModel( pCustomShapeObj->GetModel() );
                    pFill->SetMergedItemSet( *this );
                    pFill->SetMergedItem( SdrShadowItem( FALSE ) );
                    pFill->SetMergedItem( XLineStyleItem( XLINE_NONE ) );
                    rObjectList.push_back( pFill );
                }
            }
        }
    }
}

SdrObject* EnhancedCustomShape2d::CreatePathObj( sal_Bool bLineGeometryNeededOnly )
{
    sal_Int32 nCoordSize = seqCoordinates.getLength();
    if ( !nCoordSize )
        return NULL;

    sal_uInt16 nSrcPt = 0;
    sal_uInt16 nSegmentInd = 0;

    std::vector< SdrPathObj* > vObjectList;
    sal_Bool bSortFilledObjectsToBack = SortFilledObjectsToBackByDefault( eSpType );

    while( nSegmentInd <= seqSegments.getLength() )
        CreateSubPath( nSrcPt, nSegmentInd, vObjectList, bLineGeometryNeededOnly, bSortFilledObjectsToBack );

    SdrObject* pRet = NULL;
    sal_uInt32 i;
    if ( vObjectList.size() )
    {
        sal_Bool    bShadow = (((SdrShadowItem&)pCustomShapeObj->GetMergedItem( SDRATTR_SHADOW )).GetValue());
        PolyPolygon aShadowUnion;

        // inserting all filled objects
        Color           aBasicColor( COL_WHITE );
        Color           aFillColor;
        sal_uInt32      nColorCount = nColorData >> 28;
        sal_uInt32      nColorIndex = 0;
        if ( nColorCount )
        {
            const SfxPoolItem* pPoolItem = NULL;
            SfxItemState eState( GetItemState( XATTR_FILLCOLOR, FALSE, &pPoolItem ) );
            if( ( SFX_ITEM_SET == eState ) && pPoolItem )
                aBasicColor = ((XFillColorItem*)pPoolItem)->GetValue();
        }
        pRet = new SdrObjGroup;
        pRet->SetModel( pCustomShapeObj->GetModel() );

        if ( bSortFilledObjectsToBack )
        {
            // taking care of filled objects -> first
            for ( i = 0; i < vObjectList.size(); i++ )
            {
                SdrPathObj* pObj( vObjectList[ i ] );
                if ( !pObj->IsLine() )
                {
                    if ( nColorIndex < nColorCount )
                        aFillColor = GetColorData( aBasicColor, nColorIndex++ );
                    if ( nColorCount )
                        pObj->SetMergedItem( XFillColorItem( String(), aFillColor ) );
                    pRet->GetSubList()->NbcInsertObject( pObj );

                    // we will create the union of all polygon objects if shadow is used
                    if ( bShadow )
                    {
                        const XPolyPolygon& rPathPoly = pObj->GetPathPoly();
                        PolyPolygon aVCLPolyPoly( XOutCreatePolyPolygonBezier( rPathPoly, NULL ) );
                        PolyPolygon aSource( aShadowUnion );
                        aVCLPolyPoly.GetUnion( aSource, aShadowUnion );
                    }
                }
            }

            // inserting stroked objects
            for ( i = 0; i < vObjectList.size(); i++ )
            {
                SdrPathObj* pObj( vObjectList[ i ] );
                if ( pObj->IsLine() )
                    pRet->GetSubList()->NbcInsertObject( pObj );
            }
        }
        else
        {
            for ( i = 0; i < vObjectList.size(); i++ )
            {
                SdrPathObj* pObj( vObjectList[ i ] );
                if ( pObj->IsLine() )
                    pRet->GetSubList()->NbcInsertObject( pObj );
                else
                {
                    if ( nColorIndex < nColorCount )
                        aFillColor = GetColorData( aBasicColor, nColorIndex++ );
                    if ( nColorCount )
                        pObj->SetMergedItem( XFillColorItem( String(), aFillColor ) );
                    pRet->GetSubList()->NbcInsertObject( pObj );

                    // we will create the union of all polygon objects if shadow is used
                    if ( bShadow )
                    {
                        const XPolyPolygon& rPathPoly = pObj->GetPathPoly();
                        PolyPolygon aVCLPolyPoly( XOutCreatePolyPolygonBezier( rPathPoly, NULL ) );
                        PolyPolygon aSource( aShadowUnion );
                        aVCLPolyPoly.GetUnion( aSource, aShadowUnion );
                    }
                }
            }
        }

        // set the shadow object
        if ( !bLineGeometryNeededOnly && bShadow && aShadowUnion.Count() )
        {
            XPolyPolygon aXShadowPolyPoly( aShadowUnion );
            SdrPathObj* pShadow = new SdrPathObj( OBJ_POLY, aXShadowPolyPoly );
            pShadow->SetModel( pCustomShapeObj->GetModel() );
            pShadow->SetMergedItemSet( *this );
            pShadow->SetMergedItem( XLineStyleItem( XLINE_NONE ) );
            pRet->GetSubList()->NbcInsertObject( pShadow, 0 );
        }
        Rectangle aCurRect( pRet->GetSnapRect() );
        aCurRect.Move( aLogicRect.Left(), aLogicRect.Top() );
        pRet->NbcSetSnapRect( aCurRect );
    }
    return pRet;
}

SdrObject* EnhancedCustomShape2d::CreateObject( sal_Bool bLineGeometryNeededOnly )
{
    SdrObject* pRet = NULL;

    if ( eSpType == mso_sptRectangle )
    {
        pRet = new SdrRectObj( aLogicRect );
        pRet->SetModel( pCustomShapeObj->GetModel() );
        pRet->SetMergedItemSet( *this );
    }
    else if ( eSpType == mso_sptRoundRectangle )
    {
        sal_Int32 nW = aLogicRect.Right() - aLogicRect.Left();
        sal_Int32 nH = aLogicRect.Bottom() - aLogicRect.Top();
        if ( nH < nW )
            nW = nH;
        double fAdjust = GetAdjustValueAsDouble( 0 ) / 21600.0;
        nW = (sal_Int32)( (double)nW * fAdjust );
        pRet = new SdrRectObj( aLogicRect );
        pRet->SetModel( pCustomShapeObj->GetModel() );
        pRet->SetMergedItemSet( *this );
        pRet->SetMergedItem( SdrEckenradiusItem( nW ) );
    }
    else if ( eSpType == mso_sptEllipse )
    {
        pRet = new SdrCircObj( OBJ_CIRC, aLogicRect );
        pRet->SetModel( pCustomShapeObj->GetModel() );
        pRet->SetMergedItemSet( *this );
    }
    else if ( eSpType == mso_sptArc )
    {   // the arc is something special, because sometimes the snaprect does not match
        Rectangle aPolyBoundRect;
        sal_Int32 nPtNum, nNumElemVert = seqCoordinates.getLength();
        if ( nNumElemVert )
        {
            XPolygon aXP( (sal_uInt16)nNumElemVert );
            const EnhancedCustomShapeParameterPair* pTmp = seqCoordinates.getArray();
            for ( nPtNum = 0; nPtNum < nNumElemVert; nPtNum++ )
                aXP[ (sal_uInt16)nPtNum ] = GetPoint( *pTmp++, sal_False );
            aPolyBoundRect = Rectangle( aXP.GetBoundRect() );
        }
        else
            aPolyBoundRect = aLogicRect;
        sal_Int32   nEndAngle = NormAngle360( - GetAdjustValueAsInteger( 0 ) * 100 );
        sal_Int32   nStartAngle = NormAngle360( - GetAdjustValueAsInteger( 1 ) * 100 );

        if ( nStartAngle == nEndAngle )
            return NULL;

        if ( bFilled )      // ( filled ) ? we have to import an pie : we have to construct an arc
        {
            pRet = new SdrCircObj( OBJ_SECT, aPolyBoundRect, nStartAngle, nEndAngle );
            pRet->NbcSetSnapRect( aLogicRect );
            pRet->SetModel( pCustomShapeObj->GetModel() );
            pRet->SetMergedItemSet( *this );
        }
        else
        {
            Point aStart, aEnd, aCenter( aPolyBoundRect.Center() );
            aStart.X() = (sal_Int32)( ( cos( ( (double)nStartAngle * F_PI18000 ) ) * 1000.0 ) );
            aStart.Y() = - (sal_Int32)( ( sin( ( (double)nStartAngle * F_PI18000 ) ) * 1000.0 ) );
            aEnd.X() = (sal_Int32)( ( cos( ( (double)nEndAngle * F_PI18000 ) ) * 1000.0 ) );
            aEnd.Y() = - (sal_Int32)( ( sin( ( (double)nEndAngle * F_PI18000 ) ) * 1000.0 ) );
            aStart.X() += aCenter.X();
            aStart.Y() += aCenter.Y();
            aEnd.X() += aCenter.X();
            aEnd.Y() += aCenter.Y();

            Polygon aPolygon( aPolyBoundRect, aStart, aEnd, POLY_PIE );
            Rectangle aPolyPieRect( aPolygon.GetBoundRect() );

            USHORT nPt = aPolygon.GetSize();

            if ( nPt < 4 )
                return NULL;

            aPolygon[ 0 ] = aPolygon[ 1 ];                              // try to get the arc boundrect
            aPolygon[ nPt - 1 ] = aPolygon[ nPt - 2 ];
            Rectangle aPolyArcRect( aPolygon.GetBoundRect() );

            double  fYScale, fXScale;
            double  fYOfs, fXOfs;
            int     nCond;

            fYOfs = fXOfs = 0.0;
            if ( aPolyPieRect.GetWidth() != aPolyArcRect.GetWidth() )
            {
                nCond = ( (sal_uInt32)( nStartAngle - 9000 ) > 18000 ) && ( (sal_uInt32)( nEndAngle - 9000 ) > 18000 ) ? 1 : 0;
                nCond ^= bFlipH ? 1 : 0;
                if ( nCond )
                {
                    fXScale = (double)aLogicRect.GetWidth() / (double)aPolyPieRect.GetWidth();
                    fXOfs = ( (double)aPolyPieRect.GetWidth() - (double)aPolyArcRect.GetWidth() ) * fXScale;
                }
            }
            if ( aPolyPieRect.GetHeight() != aPolyArcRect.GetHeight() )
            {
                nCond = ( ( nStartAngle > 18000 ) && ( nEndAngle > 18000 ) ) ? 1 : 0;
                nCond ^= bFlipV ? 1 : 0;
                if ( nCond )
                {
                    fYScale = (double)aLogicRect.GetHeight() / (double)aPolyPieRect.GetHeight();
                    fYOfs = ( (double)aPolyPieRect.GetHeight() - (double)aPolyArcRect.GetHeight() ) * fYScale;
                }
            }
            fXScale = (double)aPolyArcRect.GetWidth() / (double)aPolyPieRect.GetWidth();
            fYScale = (double)aPolyArcRect.GetHeight() / (double)aPolyPieRect.GetHeight();

            aPolyArcRect = Rectangle( Point( aLogicRect.Left() + (sal_Int32)fXOfs, aLogicRect.Top() + (sal_Int32)fYOfs ),
                Size( (sal_Int32)( aLogicRect.GetWidth() * fXScale ), (sal_Int32)( aLogicRect.GetHeight() * fYScale ) ) );

            SdrCircObj* pObjCirc = new SdrCircObj( OBJ_CARC, aPolyBoundRect, nStartAngle, nEndAngle );
            pObjCirc->SetSnapRect( aPolyArcRect );
            pObjCirc->SetModel( pCustomShapeObj->GetModel() );
            pObjCirc->SetMergedItemSet( *this );

            int nSwap = bFlipH ? 1 : 0;
            nSwap ^= bFlipV ? 1 : 0;
            if ( nSwap )
                SwapStartAndEndArrow( pObjCirc );

            SdrRectObj* pRect = new SdrRectObj( aPolyArcRect );
            pRect->SetSnapRect( aPolyArcRect );
            pRect->SetModel( pCustomShapeObj->GetModel() );
            pRect->SetMergedItemSet( *this );
            pRect->SetMergedItem( XLineStyleItem( XLINE_NONE ) );
            pRect->SetMergedItem( XFillStyleItem( XFILL_NONE ) );

            pRet = new SdrObjGroup();
            pRet->SetModel( pCustomShapeObj->GetModel() );
            ((SdrObjGroup*)pRet)->GetSubList()->NbcInsertObject( pRect );
            ((SdrObjGroup*)pRet)->GetSubList()->NbcInsertObject( pObjCirc );
        }
    }
    if ( !pRet )
        pRet = CreatePathObj( bLineGeometryNeededOnly );

    if ( pRet && seqGluePoints.getLength() )
    {
        sal_uInt32 i, nCount = seqGluePoints.getLength();
        for ( i = 0; i < nCount; i++ )
        {
            SdrGluePoint aGluePoint;
            const Point& rPoint = GetPoint( seqGluePoints[ i ], sal_True );
            double fXRel = rPoint.X();
            double fYRel = rPoint.Y();
            fXRel = fXRel / aLogicRect.GetWidth() * 10000;
            fYRel = fYRel / aLogicRect.GetHeight() * 10000;
            aGluePoint.SetPos( Point( (sal_Int32)fXRel, (sal_Int32)fYRel ) );
            aGluePoint.SetPercent( sal_True );
            aGluePoint.SetAlign( SDRVERTALIGN_TOP | SDRHORZALIGN_LEFT );
            aGluePoint.SetEscDir( SDRESC_SMART );
            SdrGluePointList* pList = pRet->ForceGluePointList();
            if( pList )
                sal_uInt16 nId = pList->Insert( aGluePoint );
        }
    }
    return pRet;
}

SdrObject* EnhancedCustomShape2d::CreateLineGeometry()
{
    return CreateObject( sal_True );
}


