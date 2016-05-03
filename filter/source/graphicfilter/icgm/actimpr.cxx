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

#include <vcl/bitmapex.hxx>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/drawing/XShapeGrouper.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/FlagSequence.hpp>
#include <com/sun/star/drawing/ShapeCollection.hpp>
#include <com/sun/star/drawing/TextAdjust.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/style/HorizontalAlignment.hpp>

#include <comphelper/processfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include "main.hxx"
#include "outact.hxx"

using namespace ::com::sun::star;


CGMImpressOutAct::CGMImpressOutAct( CGM& rCGM, const uno::Reference< frame::XModel > & rModel ) :
        CGMOutAct       ( rCGM ),
        nFinalTextCount ( 0 )
{
    if ( mpCGM->mbStatus )
    {
        bool bStatRet = false;

        uno::Reference< drawing::XDrawPagesSupplier >  aDrawPageSup( rModel, uno::UNO_QUERY );
        if( aDrawPageSup.is() )
        {
            maXDrawPages = aDrawPageSup->getDrawPages();
            if ( maXDrawPages.is() )
            {
                maXMultiServiceFactory.set( rModel, uno::UNO_QUERY);
                if( maXMultiServiceFactory.is() )
                {
                    maXDrawPage = *static_cast<uno::Reference< drawing::XDrawPage > const *>(maXDrawPages->getByIndex( 0 ).getValue());
                    if ( ImplInitPage() )
                        bStatRet = true;
                }
            }
        }
        mpCGM->mbStatus = bStatRet;
    }
}

bool CGMImpressOutAct::ImplInitPage()
{
    bool    bStatRet = false;
    if( maXDrawPage.is() )
    {
        maXShapes = uno::Reference< drawing::XShapes >( maXDrawPage, uno::UNO_QUERY );
        if ( maXShapes.is() )
        {
            bStatRet = true;
        }
    }
    return bStatRet;
}


bool CGMImpressOutAct::ImplCreateShape( const OUString& rType )
{
    uno::Reference< uno::XInterface > xNewShape( maXMultiServiceFactory->createInstance( rType ) );
    maXShape.set( xNewShape, uno::UNO_QUERY );
    maXPropSet.set( xNewShape, uno::UNO_QUERY );
    if ( maXShape.is() && maXPropSet.is() )
    {
        maXShapes->add( maXShape );
        return true;
    }
    return false;
}


void CGMImpressOutAct::ImplSetOrientation( FloatPoint& rRefPoint, double& rOrientation )
{
    maXPropSet->setPropertyValue( "RotationPointX", uno::Any((sal_Int32)rRefPoint.X) );
    maXPropSet->setPropertyValue( "RotationPointY", uno::Any((sal_Int32)rRefPoint.Y) );
    maXPropSet->setPropertyValue( "RotateAngle", uno::Any((sal_Int32)( rOrientation * 100.0 )) );
}


void CGMImpressOutAct::ImplSetLineBundle()
{
    drawing::LineStyle  eLS;

    sal_uInt32          nLineColor;
    LineType            eLineType;
    double              fLineWidth;

    if ( mpCGM->pElement->nAspectSourceFlags & ASF_LINECOLOR )
        nLineColor = mpCGM->pElement->pLineBundle->GetColor();
    else
        nLineColor = mpCGM->pElement->aLineBundle.GetColor();
    if ( mpCGM->pElement->nAspectSourceFlags & ASF_LINETYPE )
        eLineType = mpCGM->pElement->pLineBundle->eLineType;
    else
        eLineType = mpCGM->pElement->aLineBundle.eLineType;
    if ( mpCGM->pElement->nAspectSourceFlags & ASF_LINEWIDTH )
        fLineWidth = mpCGM->pElement->pLineBundle->nLineWidth;
    else
        fLineWidth = mpCGM->pElement->aLineBundle.nLineWidth;

    maXPropSet->setPropertyValue( "LineColor", uno::Any((sal_Int32)nLineColor) );

    maXPropSet->setPropertyValue( "LineWidth", uno::Any((sal_Int32)fLineWidth) );

    switch( eLineType )
    {
        case LT_NONE :
            eLS = drawing::LineStyle_NONE;
        break;
        case LT_DASH :
        case LT_DOT :
        case LT_DASHDOT :
        case LT_DOTDOTSPACE :
        case LT_LONGDASH :
        case LT_DASHDASHDOT :
            eLS = drawing::LineStyle_DASH;
        break;
        case LT_SOLID :
        default:
            eLS = drawing::LineStyle_SOLID;
        break;
    }
    maXPropSet->setPropertyValue( "LineStyle", uno::Any(eLS) );
    if ( eLS == drawing::LineStyle_DASH )
    {
        drawing::LineDash aLineDash( drawing::DashStyle_RECTRELATIVE, 1, 50, 3, 33, 100 );
        maXPropSet->setPropertyValue( "LineDash", uno::Any(aLineDash) );
    }
}

void CGMImpressOutAct::ImplSetFillBundle()
{
    drawing::LineStyle      eLS;
    drawing::FillStyle      eFS;

    sal_uInt32              nEdgeColor = 0;
    EdgeType                eEdgeType;
    double                  fEdgeWidth = 0;

    sal_uInt32              nFillColor;
    FillInteriorStyle       eFillStyle;
    sal_uInt32              nHatchIndex;

    if ( mpCGM->pElement->eEdgeVisibility == EV_ON )
    {
        if ( mpCGM->pElement->nAspectSourceFlags & ASF_EDGETYPE )
            eEdgeType = mpCGM->pElement->pEdgeBundle->eEdgeType;
        else
            eEdgeType = mpCGM->pElement->aEdgeBundle.eEdgeType;
        if ( mpCGM->pElement->nAspectSourceFlags & ASF_EDGEWIDTH )
            fEdgeWidth = mpCGM->pElement->pEdgeBundle->nEdgeWidth;
        else
            fEdgeWidth = mpCGM->pElement->aEdgeBundle.nEdgeWidth;
        if ( mpCGM->pElement->nAspectSourceFlags & ASF_EDGECOLOR )
            nEdgeColor = mpCGM->pElement->pEdgeBundle->GetColor();
        else
            nEdgeColor = mpCGM->pElement->aEdgeBundle.GetColor();
    }
    else
        eEdgeType = ET_NONE;

    if ( mpCGM->pElement->nAspectSourceFlags & ASF_FILLINTERIORSTYLE )
        eFillStyle = mpCGM->pElement->pFillBundle->eFillInteriorStyle;
    else
        eFillStyle = mpCGM->pElement->aFillBundle.eFillInteriorStyle;
    if ( mpCGM->pElement->nAspectSourceFlags & ASF_FILLCOLOR )
        nFillColor = mpCGM->pElement->pFillBundle->GetColor();
    else
        nFillColor = mpCGM->pElement->aFillBundle.GetColor();
    if ( mpCGM->pElement->nAspectSourceFlags & ASF_HATCHINDEX )
        nHatchIndex = (sal_uInt32)mpCGM->pElement->pFillBundle->nFillHatchIndex;
    else
        nHatchIndex = (sal_uInt32)mpCGM->pElement->aFillBundle.nFillHatchIndex;

    maXPropSet->setPropertyValue( "FillColor", uno::Any((sal_Int32)nFillColor) );

    switch ( eFillStyle )
    {
        case FIS_HATCH   :
        {
            if ( nHatchIndex == 0 )
                eFS = drawing::FillStyle_NONE;
            else
                eFS = drawing::FillStyle_HATCH;
        }
        break;
        case FIS_PATTERN :
        case FIS_SOLID :
        {
            eFS = drawing::FillStyle_SOLID;
        }
        break;

        case FIS_GEOPATTERN :
        {
            if ( mpCGM->pElement->eTransparency == T_ON )
                nFillColor = mpCGM->pElement->nAuxiliaryColor;
            eFS = drawing::FillStyle_NONE;
        }
        break;

        case FIS_INTERPOLATED :
        case FIS_GRADIENT :
        {
            eFS = drawing::FillStyle_GRADIENT;
        }
        break;

        case FIS_HOLLOW :
        case FIS_EMPTY :
        default:
        {
            eFS = drawing::FillStyle_NONE;
        }
    }

    if ( mpCGM->mnAct4PostReset & ACT4_GRADIENT_ACTION )
        eFS = drawing::FillStyle_GRADIENT;

    if ( eFS == drawing::FillStyle_GRADIENT )
    {
        maXPropSet->setPropertyValue( "FillGradient", uno::Any(*mpGradient) );
    }
    maXPropSet->setPropertyValue( "FillStyle", uno::Any(eFS) );

    eLS = drawing::LineStyle_NONE;
    if ( eFillStyle == FIS_HOLLOW )
    {
        eLS = drawing::LineStyle_SOLID;
        maXPropSet->setPropertyValue( "LineColor", uno::Any((sal_Int32)nFillColor) );
        maXPropSet->setPropertyValue( "LineWidth", uno::Any((sal_Int32)0) );
    }
    else if ( eEdgeType != ET_NONE )
    {
        maXPropSet->setPropertyValue( "LineColor", uno::Any((sal_Int32)nEdgeColor) );

        maXPropSet->setPropertyValue( "LineWidth", uno::Any((sal_Int32)fEdgeWidth) );

        switch( eEdgeType )
        {
            case ET_DASH :
            case ET_DOT :
            case ET_DASHDOT :
            case ET_DASHDOTDOT :
            case ET_DOTDOTSPACE :
            case ET_LONGDASH :
            case ET_DASHDASHDOT :
            default:            // case ET_SOLID :
            {
                eLS = drawing::LineStyle_SOLID;
            }
            break;
        }
    }

    maXPropSet->setPropertyValue( "LineStyle", uno::Any(eLS) );

    if ( eFS == drawing::FillStyle_HATCH )
    {
        drawing::Hatch aHatch;

        aHatch.Color = nFillColor;
        if ( mpCGM->pElement->maHatchMap.find( nHatchIndex ) !=  mpCGM->pElement->maHatchMap.end() )
        {
            HatchEntry& rHatchEntry = mpCGM->pElement->maHatchMap[ nHatchIndex ];
            switch ( rHatchEntry.HatchStyle )
            {
            case 0 : aHatch.Style = drawing::HatchStyle_SINGLE; break;
            case 1 : aHatch.Style = drawing::HatchStyle_DOUBLE; break;
            case 2 : aHatch.Style = drawing::HatchStyle_TRIPLE; break;
            }
            aHatch.Distance = rHatchEntry.HatchDistance;
            aHatch.Angle = rHatchEntry.HatchAngle;
        }
        else
        {
            aHatch.Style = drawing::HatchStyle_TRIPLE;
            aHatch.Distance = 10 * ( nHatchIndex & 0x1f ) | 100;
            aHatch.Angle = 15 * ( ( nHatchIndex & 0x1f ) - 5 );
        }
        maXPropSet->setPropertyValue( "FillHatch", uno::Any(aHatch) );
    }
}

void CGMImpressOutAct::ImplSetTextBundle( const uno::Reference< beans::XPropertySet > & rProperty )
{
    sal_uInt32      nTextFontIndex;
    sal_uInt32      nTextColor;

    if ( mpCGM->pElement->nAspectSourceFlags & ASF_TEXTFONTINDEX )
        nTextFontIndex = mpCGM->pElement->pTextBundle->nTextFontIndex;
    else
        nTextFontIndex = mpCGM->pElement->aTextBundle.nTextFontIndex;
    if ( mpCGM->pElement->nAspectSourceFlags & ASF_TEXTCOLOR )
        nTextColor = mpCGM->pElement->pTextBundle->GetColor();
    else
        nTextColor = mpCGM->pElement->aTextBundle.GetColor();

    rProperty->setPropertyValue( "CharColor", uno::Any((sal_Int32)nTextColor) );

    sal_uInt32 nFontType = 0;
    awt::FontDescriptor aFontDescriptor;
    FontEntry* pFontEntry = mpCGM->pElement->aFontList.GetFontEntry( nTextFontIndex );
    if ( pFontEntry )
    {
        nFontType = pFontEntry->nFontType;
        aFontDescriptor.Name = OUString::createFromAscii( reinterpret_cast<char*>(pFontEntry->pFontName) );
    }
    aFontDescriptor.Height = ( sal_Int16 )( ( mpCGM->pElement->nCharacterHeight * (double)1.50 ) );
    if ( nFontType & 1 )
        aFontDescriptor.Slant = awt::FontSlant_ITALIC;
    if ( nFontType & 2 )
        aFontDescriptor.Weight = awt::FontWeight::BOLD;
    else
        aFontDescriptor.Weight = awt::FontWeight::NORMAL;

    if ( mpCGM->pElement->eUnderlineMode != UM_OFF )
    {
        aFontDescriptor.Underline = awt::FontUnderline::SINGLE;
    }
    rProperty->setPropertyValue( "FontDescriptor", uno::Any(aFontDescriptor) );
}

void CGMImpressOutAct::InsertPage()
{
    if ( mnCurrentPage )    // one side is always existing, therefore the first side will be left out
    {
        uno::Reference< drawing::XDrawPage > xPage( maXDrawPages->insertNewByIndex( 0xffff ), uno::UNO_QUERY );
        maXDrawPage = xPage;
        if ( !ImplInitPage() )
            mpCGM->mbStatus = false;
    }
    mnCurrentPage++;
}

void CGMImpressOutAct::BeginGroup()
{
    if ( mnGroupLevel < CGM_OUTACT_MAX_GROUP_LEVEL )
    {
        mpGroupLevel[ mnGroupLevel ] = maXShapes->getCount();
    }
    mnGroupLevel++;
    mnGroupActCount = mpCGM->mnActCount;
}

void CGMImpressOutAct::EndGroup()
{
    if ( mnGroupLevel )     // preserve overflow
        mnGroupLevel--;
    if ( mnGroupLevel < CGM_OUTACT_MAX_GROUP_LEVEL )
    {
        sal_uInt32 mnFirstIndex = mpGroupLevel[ mnGroupLevel ];
        if ( mnFirstIndex == 0xffffffff )
            mnFirstIndex = 0;
        sal_uInt32 mnCurrentCount = maXShapes->getCount();
        if ( ( mnCurrentCount - mnFirstIndex ) > 1 )
        {
            uno::Reference< drawing::XShapeGrouper > aXShapeGrouper;
            aXShapeGrouper.set( maXDrawPage, uno::UNO_QUERY );
            if( aXShapeGrouper.is() )
            {
                uno::Reference< drawing::XShapes >  aXShapes = drawing::ShapeCollection::create(comphelper::getProcessComponentContext());
                for ( sal_uInt32 i = mnFirstIndex; i < mnCurrentCount; i++ )
                {
                    uno::Reference< drawing::XShape >  aXShape = *static_cast<uno::Reference< drawing::XShape > const *>(maXShapes->getByIndex( i ).getValue());
                    if (aXShape.is() )
                    {
                        aXShapes->add( aXShape );
                    }
                }
                uno::Reference< drawing::XShapeGroup >  aXShapeGroup = aXShapeGrouper->group( aXShapes );
            }
        }
    }
}

void CGMImpressOutAct::EndGrouping()
{
    while ( mnGroupLevel )
    {
        EndGroup();
    }
}

void CGMImpressOutAct::DrawRectangle( FloatRect& rFloatRect )
{
    if ( mnGroupActCount != ( mpCGM->mnActCount - 1 ) )         // POWERPOINT HACK !!!
    {
        if ( ImplCreateShape( "com.sun.star.drawing.RectangleShape" ) )
        {
            awt::Size aSize( (long)(rFloatRect.Right - rFloatRect.Left ), (long)(rFloatRect.Bottom-rFloatRect.Top ) );
            maXShape->setSize( aSize );
            maXShape->setPosition( awt::Point( (long)rFloatRect.Left, (long)rFloatRect.Top ) );
            ImplSetFillBundle();
        }
    }
}

void CGMImpressOutAct::DrawEllipse( FloatPoint& rCenter, FloatPoint& rSize, double& rOrientation )
{
    if ( ImplCreateShape( "com.sun.star.drawing.EllipseShape" ) )
    {
        drawing::CircleKind eCircleKind = drawing::CircleKind_FULL;
        uno::Any aAny( &eCircleKind, ::cppu::UnoType<drawing::CircleKind>::get() );
        maXPropSet->setPropertyValue( "CircleKind", aAny );

        long nXSize = (long)( rSize.X * 2.0 );      // strange behaviour with a awt::Size of 0
        long nYSize = (long)( rSize.Y * 2.0 );
        if ( nXSize < 1 )
            nXSize = 1;
        if ( nYSize < 1 )
            nYSize = 1;
        maXShape->setSize( awt::Size( nXSize, nYSize ) );
        maXShape->setPosition( awt::Point( (long)( rCenter.X - rSize.X ), (long)( rCenter.Y - rSize.Y ) ) );

        if ( rOrientation != 0 )
        {
            ImplSetOrientation( rCenter, rOrientation );
        }
        ImplSetFillBundle();
    }
}

void CGMImpressOutAct::DrawEllipticalArc( FloatPoint& rCenter, FloatPoint& rSize, double& rOrientation,
            sal_uInt32 nType, double& fStartAngle, double& fEndAngle )
{
    if ( ImplCreateShape( "com.sun.star.drawing.EllipseShape" ) )
    {
        uno::Any aAny;
        drawing::CircleKind eCircleKind;


        long nXSize = (long)( rSize.X * 2.0 );      // strange behaviour with a awt::Size of 0
        long nYSize = (long)( rSize.Y * 2.0 );
        if ( nXSize < 1 )
            nXSize = 1;
        if ( nYSize < 1 )
            nYSize = 1;

        maXShape->setSize( awt::Size ( nXSize, nYSize ) );

        if ( rOrientation != 0 )
        {
            fStartAngle += rOrientation;
            if ( fStartAngle >= 360 )
                fStartAngle -= 360;
            fEndAngle += rOrientation;
            if ( fEndAngle >= 360 )
                fEndAngle -= 360;
        }
        switch( nType )
        {
            case 0 : eCircleKind = drawing::CircleKind_SECTION; break;
            case 1 : eCircleKind = drawing::CircleKind_CUT; break;
            case 2 : eCircleKind = drawing::CircleKind_ARC; break;
            default : eCircleKind = drawing::CircleKind_FULL; break;
        }
        if ( (long)fStartAngle == (long)fEndAngle )
        {
            eCircleKind = drawing::CircleKind_FULL;
            maXPropSet->setPropertyValue( "CircleKind", uno::Any(eCircleKind) );
        }
        else
        {
            maXPropSet->setPropertyValue( "CircleKind", uno::Any(eCircleKind) );
            maXPropSet->setPropertyValue( "CircleStartAngle", uno::Any((sal_Int32)( fStartAngle * 100 )) );
            maXPropSet->setPropertyValue( "CircleEndAngle", uno::Any((sal_Int32)( fEndAngle * 100 )) );
        }
        maXShape->setPosition( awt::Point( (long)( rCenter.X - rSize.X ), (long)( rCenter.Y - rSize.Y ) ) );
        if ( rOrientation != 0 )
        {
            ImplSetOrientation( rCenter, rOrientation );
        }
        if ( eCircleKind == drawing::CircleKind_ARC )
        {
            ImplSetLineBundle();
        }
        else
        {
            ImplSetFillBundle();
            if ( nType == 2 )
            {
                ImplSetLineBundle();
                drawing::FillStyle eFillStyle = drawing::FillStyle_NONE;
                aAny.setValue( &eFillStyle, cppu::UnoType<drawing::FillStyle>::get());
                maXPropSet->setPropertyValue( "FillStyle", aAny );
            }
        }
    }
}

void CGMImpressOutAct::DrawBitmap( CGMBitmapDescriptor* pBmpDesc )
{
    if ( pBmpDesc->mbStatus && pBmpDesc->mpBitmap )
    {
        FloatPoint aOrigin = pBmpDesc->mnOrigin;
        double fdx = pBmpDesc->mndx;
        double fdy = pBmpDesc->mndy;

        BmpMirrorFlags nMirr = BmpMirrorFlags::NONE;
        if ( pBmpDesc->mbVMirror )
            nMirr |= BmpMirrorFlags::Vertical;
        if ( pBmpDesc->mbHMirror )
            nMirr |= BmpMirrorFlags::Horizontal;
        if ( nMirr != BmpMirrorFlags::NONE )
            pBmpDesc->mpBitmap->Mirror( nMirr );

        mpCGM->ImplMapPoint( aOrigin );
        mpCGM->ImplMapX( fdx );
        mpCGM->ImplMapY( fdy );

        if ( ImplCreateShape( "com.sun.star.drawing.GraphicObjectShape" ) )
        {
            maXShape->setSize( awt::Size( (long)fdx, (long)fdy ) );
            maXShape->setPosition( awt::Point( (long)aOrigin.X, (long)aOrigin.Y ) );

            if ( pBmpDesc->mnOrientation != 0 )
            {
                ImplSetOrientation( aOrigin, pBmpDesc->mnOrientation );
            }

            uno::Reference< awt::XBitmap > xBitmap( VCLUnoHelper::CreateBitmap( BitmapEx( *( pBmpDesc->mpBitmap ) ) ) );
            maXPropSet->setPropertyValue( "GraphicObjectFillBitmap", uno::Any(xBitmap) );

        }
    }
}

void CGMImpressOutAct::DrawPolygon( tools::Polygon& rPoly )
{
    sal_uInt16 nPoints = rPoly.GetSize();

    if ( ( nPoints > 1 ) && ImplCreateShape( "com.sun.star.drawing.PolyPolygonShape" ) )
    {
        drawing::PointSequenceSequence aRetval;

        // prepare inside polygons
        aRetval.realloc( 1 );

        // get pointer to outside arrays
        drawing::PointSequence* pOuterSequence = aRetval.getArray();

        // make room in arrays
        pOuterSequence->realloc((sal_Int32)nPoints);

        // get pointer to arrays
        awt::Point* pInnerSequence = pOuterSequence->getArray();

        for( sal_uInt16 n = 0; n < nPoints; n++ )
            *pInnerSequence++ = awt::Point( rPoly[ n ].X(), rPoly[n].Y() );

        uno::Any aParam;
        aParam <<= aRetval;
        maXPropSet->setPropertyValue( "PolyPolygon", aParam );
        ImplSetFillBundle();
    }
}

void CGMImpressOutAct::DrawPolyLine( tools::Polygon& rPoly )
{
    sal_uInt16 nPoints = rPoly.GetSize();

    if ( ( nPoints > 1 ) && ImplCreateShape( "com.sun.star.drawing.PolyLineShape" ) )
    {
        drawing::PointSequenceSequence aRetval;

        // prepare inside polygons
        aRetval.realloc( 1 );

        // get pointer to outside arrays
        drawing::PointSequence* pOuterSequence = aRetval.getArray();

        // make room in arrays
        pOuterSequence->realloc((sal_Int32)nPoints);

        // get pointer to arrays
        awt::Point* pInnerSequence = pOuterSequence->getArray();

        for( sal_uInt16 n = 0; n < nPoints; n++ )
            *pInnerSequence++ = awt::Point( rPoly[ n ].X(), rPoly[n].Y() );

        uno::Any aParam;
        aParam <<= aRetval;
        maXPropSet->setPropertyValue( "PolyPolygon", aParam );
        ImplSetLineBundle();
    }
}

void CGMImpressOutAct::DrawPolybezier( tools::Polygon& rPolygon )
{
    sal_uInt16 nPoints = rPolygon.GetSize();
    if ( ( nPoints > 1 ) && ImplCreateShape( "com.sun.star.drawing.OpenBezierShape" ) )
    {
        drawing::PolyPolygonBezierCoords aRetval;

        aRetval.Coordinates.realloc( 1 );
        aRetval.Flags.realloc( 1 );

        // get pointer to outside arrays
        drawing::PointSequence* pOuterSequence = aRetval.Coordinates.getArray();
        drawing::FlagSequence* pOuterFlags = aRetval.Flags.getArray();

        // make room in arrays
        pOuterSequence->realloc( nPoints );
        pOuterFlags->realloc( nPoints );

        awt::Point* pInnerSequence = pOuterSequence->getArray();
        drawing::PolygonFlags* pInnerFlags = pOuterFlags->getArray();

        for( sal_uInt16 i = 0; i < nPoints; i++ )
        {
            *pInnerSequence++ = awt::Point( rPolygon[ i ].X(), rPolygon[ i ].Y() );
            *pInnerFlags++ = (drawing::PolygonFlags)rPolygon.GetFlags( i );
        }
        uno::Any aParam;
        aParam <<= aRetval;
        maXPropSet->setPropertyValue( "PolyPolygonBezier", aParam );
        ImplSetLineBundle();
    }
}

void CGMImpressOutAct::DrawPolyPolygon( tools::PolyPolygon& rPolyPolygon )
{
    sal_uInt32 nNumPolys = rPolyPolygon.Count();
    if ( nNumPolys && ImplCreateShape( "com.sun.star.drawing.ClosedBezierShape" ) )
    {
        drawing::PolyPolygonBezierCoords aRetval;

        // prepare inside polygons
        aRetval.Coordinates.realloc((sal_Int32)nNumPolys);
        aRetval.Flags.realloc((sal_Int32)nNumPolys);

        // get pointer to outside arrays
        drawing::PointSequence* pOuterSequence = aRetval.Coordinates.getArray();
        drawing::FlagSequence* pOuterFlags = aRetval.Flags.getArray();

        for( sal_uInt32 a = 0; a < nNumPolys; a++ )
        {
            tools::Polygon aPolygon( rPolyPolygon.GetObject( a ) );
            sal_uInt32 nNumPoints = aPolygon.GetSize();

            // make room in arrays
            pOuterSequence->realloc((sal_Int32)nNumPoints);
            pOuterFlags->realloc((sal_Int32)nNumPoints);

            // get pointer to arrays
            awt::Point* pInnerSequence = pOuterSequence->getArray();
            drawing::PolygonFlags* pInnerFlags = pOuterFlags->getArray();

            for( sal_uInt32 b = 0; b < nNumPoints; b++ )
            {
                *pInnerSequence++ = awt::Point( aPolygon.GetPoint( b ).X(), aPolygon.GetPoint( b ).Y() ) ;
                *pInnerFlags++ = (drawing::PolygonFlags)aPolygon.GetFlags( b );
            }
            pOuterSequence++;
            pOuterFlags++;
        }
        uno::Any aParam;
        aParam <<= aRetval;
        maXPropSet->setPropertyValue( "PolyPolygonBezier", aParam);
        ImplSetFillBundle();
    }
}

void CGMImpressOutAct::DrawText( awt::Point& rTextPos, awt::Size& rTextSize, char* pString, sal_uInt32 /*nSize*/, FinalFlag eFlag )
{
    if ( ImplCreateShape( "com.sun.star.drawing.TextShape" ) )
    {
        uno::Any    aAny;
        long    nWidth = rTextSize.Width;
        long    nHeight = rTextSize.Height;

        awt::Point aTextPos( rTextPos );
        switch ( mpCGM->pElement->eTextAlignmentV )
        {
            case TAV_HALF :
            {
                aTextPos.Y -= static_cast<sal_Int32>( ( mpCGM->pElement->nCharacterHeight * 1.5 ) / 2 );
            }
            break;

            case TAV_BASE :
            case TAV_BOTTOM :
            case TAV_NORMAL :
                aTextPos.Y -= static_cast<sal_Int32>( mpCGM->pElement->nCharacterHeight * 1.5 );
            case TAV_TOP :
                break;
            case TAV_CAP:
            case TAV_CONT:
                break;  // -Wall these two were not here.
        }

        if ( nWidth < 0 )
        {
            nWidth = -nWidth;
        }
        else if ( nWidth == 0 )
        {
            nWidth = -1;
        }
        if ( nHeight < 0 )
        {
            nHeight = -nHeight;
        }
        else if ( nHeight == 0 )
        {
            nHeight = -1;
        }
        maXShape->setPosition( aTextPos );
        maXShape->setSize( awt::Size( nWidth, nHeight ) );
        double nX = mpCGM->pElement->nCharacterOrientation[ 2 ];
        double nY = mpCGM->pElement->nCharacterOrientation[ 3 ];
        double nOrientation = acos( nX / sqrt( nX * nX + nY * nY ) ) * 57.29577951308;
        if ( nY < 0 )
            nOrientation = 360 - nOrientation;

        if ( nOrientation )
        {
            maXPropSet->setPropertyValue( "RotationPointX", uno::Any((sal_Int32)( aTextPos.X )) );
            maXPropSet->setPropertyValue( "RotationPointY", uno::Any((sal_Int32)( aTextPos.Y + nHeight )) );
            maXPropSet->setPropertyValue( "RotateAngle", uno::Any((sal_Int32)( nOrientation * 100 )) );
        }
        if ( nWidth == -1 )
        {
            sal_Bool bTrue( sal_True );
            aAny.setValue( &bTrue, cppu::UnoType<sal_Bool>::get());
            maXPropSet->setPropertyValue( "TextAutoGrowWidth", aAny );

            drawing::TextAdjust eTextAdjust;
            switch ( mpCGM->pElement->eTextAlignmentH )
            {
                case TAH_RIGHT :
                    eTextAdjust = drawing::TextAdjust_RIGHT;
                break;
                case TAH_LEFT :
                case TAH_CONT :
                case TAH_NORMAL :
                    eTextAdjust = drawing::TextAdjust_LEFT;
                break;
                case TAH_CENTER :
                    eTextAdjust = drawing::TextAdjust_CENTER;
                break;
            }
            maXPropSet->setPropertyValue( "TextHorizontalAdjust", uno::Any(eTextAdjust) );
        }
        if ( nHeight == -1 )
        {
            sal_Bool bTrue = sal_True;
            aAny.setValue( &bTrue, cppu::UnoType<sal_Bool>::get());
            maXPropSet->setPropertyValue( "TextAutoGrowHeight", aAny );
        }
        uno::Reference< text::XText >  xText;
        uno::Any aFirstQuery( maXShape->queryInterface( cppu::UnoType<text::XText>::get()));
        if( aFirstQuery >>= xText )
        {
            OUString aStr( OUString::createFromAscii( pString ) );

            uno::Reference< text::XTextCursor >  aXTextCursor( xText->createTextCursor() );
            {
                aXTextCursor->gotoEnd( sal_False );
                uno::Reference< text::XTextRange >  aCursorText;
                uno::Any aSecondQuery( aXTextCursor->queryInterface( cppu::UnoType<text::XTextRange>::get()));
                if ( aSecondQuery >>= aCursorText )
                {
                    uno::Reference< beans::XPropertySet >  aCursorPropSet;

                    uno::Any aQuery( aCursorText->queryInterface( cppu::UnoType<beans::XPropertySet>::get()));
                    if( aQuery >>= aCursorPropSet )
                    {
                        if ( nWidth != -1 )     // paragraph adjusting in a valid textbox ?
                        {
                            switch ( mpCGM->pElement->eTextAlignmentH )
                            {
                                case TAH_RIGHT :
                                    aAny <<= (sal_Int16)style::HorizontalAlignment_RIGHT;
                                break;
                                case TAH_LEFT :
                                case TAH_CONT :
                                case TAH_NORMAL :
                                    aAny <<= (sal_Int16)style::HorizontalAlignment_LEFT;
                                break;
                                case TAH_CENTER :
                                    aAny <<= (sal_Int16)style::HorizontalAlignment_CENTER;
                                break;
                            }
                            aCursorPropSet->setPropertyValue( "ParaAdjust", aAny );
                        }
                        if ( nWidth > 0 && nHeight > 0 )    // restricted text
                        {
                            sal_Bool bTrue = sal_True;
                            aAny.setValue( &bTrue, cppu::UnoType<sal_Bool>::get());
                            maXPropSet->setPropertyValue( "TextFitToSize", aAny );
                        }
                        aCursorText->setString( aStr );
                        aXTextCursor->gotoEnd( sal_True );
                        ImplSetTextBundle( aCursorPropSet );
                    }
                }
            }
        }
        if ( eFlag == FF_NOT_FINAL )
        {
            nFinalTextCount = maXShapes->getCount();
        }
    }
}

void CGMImpressOutAct::AppendText( char* pString, sal_uInt32 /*nSize*/, FinalFlag /*eFlag*/ )
{
    if ( nFinalTextCount )
    {
        uno::Reference< drawing::XShape >  aShape = *static_cast<uno::Reference< drawing::XShape > const *>(maXShapes->getByIndex( nFinalTextCount - 1 ).getValue());
        if ( aShape.is() )
        {
            uno::Reference< text::XText >  xText;
            uno::Any aFirstQuery(  aShape->queryInterface( cppu::UnoType<text::XText>::get()) );
            if( aFirstQuery >>= xText )
            {
                OUString aStr( OUString::createFromAscii( pString ) );

                uno::Reference< text::XTextCursor >  aXTextCursor( xText->createTextCursor() );
                if ( aXTextCursor.is() )
                {
                    aXTextCursor->gotoEnd( sal_False );
                    uno::Reference< text::XTextRange >  aCursorText;
                    uno::Any aSecondQuery(aXTextCursor->queryInterface( cppu::UnoType<text::XTextRange>::get()));
                    if ( aSecondQuery >>= aCursorText )
                    {
                        uno::Reference< beans::XPropertySet >  aPropSet;
                        uno::Any aQuery(aCursorText->queryInterface( cppu::UnoType<beans::XPropertySet>::get()));
                        if( aQuery >>= aPropSet )
                        {
                            aCursorText->setString( aStr );
                            aXTextCursor->gotoEnd( sal_True );
                            ImplSetTextBundle( aPropSet );
                        }
                    }
                }
            }
        }
    }
}


void CGMImpressOutAct::DrawChart()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
