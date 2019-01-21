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

#include <sal/config.h>

#include <o3tl/any.hxx>
#include <o3tl/safeint.hxx>
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
#include <tools/helpers.hxx>
#include <vcl/gradient.hxx>

#include "main.hxx"
#include "outact.hxx"

using namespace ::com::sun::star;

CGMImpressOutAct::CGMImpressOutAct(CGM& rCGM, const uno::Reference< frame::XModel > & rModel)
    : mnCurrentPage(0)
    , mnGroupActCount(0)
    , mnGroupLevel(0)
    , maGroupLevel()
    , mpCGM(&rCGM)
    , nFinalTextCount(0)
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
                    maXDrawPage = *o3tl::doAccess<uno::Reference<drawing::XDrawPage>>(maXDrawPages->getByIndex( 0 ));
                    if ( ImplInitPage() )
                        bStatRet = true;
                }
            }
        }
        mpCGM->mbStatus = bStatRet;
    }
}

CGMImpressOutAct::~CGMImpressOutAct()
{
    for (auto &a : maLockedNewXShapes)
        a->removeActionLock();
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
        uno::Reference<document::XActionLockable> xLockable(maXShape, uno::UNO_QUERY);
        if (xLockable)
        {
            xLockable->addActionLock();
            maLockedNewXShapes.push_back(xLockable);
        }
        return true;
    }
    return false;
}


void CGMImpressOutAct::ImplSetOrientation( FloatPoint const & rRefPoint, double rOrientation )
{
    maXPropSet->setPropertyValue( "RotationPointX", uno::Any(static_cast<sal_Int32>(rRefPoint.X)) );
    maXPropSet->setPropertyValue( "RotationPointY", uno::Any(static_cast<sal_Int32>(rRefPoint.Y)) );
    maXPropSet->setPropertyValue( "RotateAngle", uno::Any(static_cast<sal_Int32>( rOrientation * 100.0 )) );
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

    maXPropSet->setPropertyValue( "LineColor", uno::Any(static_cast<sal_Int32>(nLineColor)) );

    maXPropSet->setPropertyValue( "LineWidth", uno::Any(static_cast<sal_Int32>(fLineWidth)) );

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
        nHatchIndex = static_cast<sal_uInt32>(mpCGM->pElement->pFillBundle->nFillHatchIndex);
    else
        nHatchIndex = static_cast<sal_uInt32>(mpCGM->pElement->aFillBundle.nFillHatchIndex);

    maXPropSet->setPropertyValue( "FillColor", uno::Any(static_cast<sal_Int32>(nFillColor)) );

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
        maXPropSet->setPropertyValue( "LineColor", uno::Any(static_cast<sal_Int32>(nFillColor)) );
        maXPropSet->setPropertyValue( "LineWidth", uno::Any(sal_Int32(0)) );
    }
    else if ( eEdgeType != ET_NONE )
    {
        maXPropSet->setPropertyValue( "LineColor", uno::Any(static_cast<sal_Int32>(nEdgeColor)) );

        maXPropSet->setPropertyValue( "LineWidth", uno::Any(static_cast<sal_Int32>(fEdgeWidth)) );

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

    rProperty->setPropertyValue( "CharColor", uno::Any(static_cast<sal_Int32>(nTextColor)) );

    sal_uInt32 nFontType = 0;
    awt::FontDescriptor aFontDescriptor;
    FontEntry* pFontEntry = mpCGM->pElement->aFontList.GetFontEntry( nTextFontIndex );
    if ( pFontEntry )
    {
        nFontType = pFontEntry->nFontType;
        aFontDescriptor.Name = OUString::createFromAscii( reinterpret_cast<char*>(pFontEntry->pFontName.get()) );
    }
    aFontDescriptor.Height = sal_Int16( mpCGM->pElement->nCharacterHeight * 1.50 );
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
        maGroupLevel[mnGroupLevel] = maXShapes->getCount();
    }
    ++mnGroupLevel;
    mnGroupActCount = mpCGM->mnActCount;
}

void CGMImpressOutAct::EndGroup()
{
    if (!mnGroupLevel)
        return;
    --mnGroupLevel;
    if ( mnGroupLevel < CGM_OUTACT_MAX_GROUP_LEVEL )
    {
        sal_uInt32 nFirstIndex = maGroupLevel[mnGroupLevel];
        if ( nFirstIndex == 0xffffffff )
            nFirstIndex = 0;
        sal_uInt32 nCurrentCount = maXShapes->getCount();
        if ( ( nCurrentCount - nFirstIndex ) > 1 )
        {
            uno::Reference< drawing::XShapeGrouper > aXShapeGrouper;
            aXShapeGrouper.set( maXDrawPage, uno::UNO_QUERY );
            if( aXShapeGrouper.is() )
            {
                uno::Reference< drawing::XShapes >  aXShapes = drawing::ShapeCollection::create(comphelper::getProcessComponentContext());
                for ( sal_uInt32 i = nFirstIndex; i < nCurrentCount; i++ )
                {
                    uno::Reference< drawing::XShape >  aXShape = *o3tl::doAccess<uno::Reference<drawing::XShape>>(maXShapes->getByIndex( i ));
                    if (aXShape.is() )
                    {
                        aXShapes->add( aXShape );
                    }
                }
                aXShapeGrouper->group( aXShapes );
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

void CGMImpressOutAct::DrawRectangle( FloatRect const & rFloatRect )
{
    if ( mnGroupActCount != ( mpCGM->mnActCount - 1 ) )         // POWERPOINT HACK !!!
    {
        if ( ImplCreateShape( "com.sun.star.drawing.RectangleShape" ) )
        {
            awt::Size aSize( static_cast<long>(rFloatRect.Right - rFloatRect.Left ), static_cast<long>(rFloatRect.Bottom-rFloatRect.Top ) );
            maXShape->setSize( aSize );
            maXShape->setPosition( awt::Point( static_cast<long>(rFloatRect.Left), static_cast<long>(rFloatRect.Top) ) );
            ImplSetFillBundle();
        }
    }
}

void CGMImpressOutAct::DrawEllipse( FloatPoint const & rCenter, FloatPoint const & rSize, double& rOrientation )
{
    if ( ImplCreateShape( "com.sun.star.drawing.EllipseShape" ) )
    {
        drawing::CircleKind eCircleKind = drawing::CircleKind_FULL;
        uno::Any aAny( &eCircleKind, ::cppu::UnoType<drawing::CircleKind>::get() );
        maXPropSet->setPropertyValue( "CircleKind", aAny );

        long nXSize = static_cast<long>( rSize.X * 2.0 );      // strange behaviour with a awt::Size of 0
        long nYSize = static_cast<long>( rSize.Y * 2.0 );
        if ( nXSize < 1 )
            nXSize = 1;
        if ( nYSize < 1 )
            nYSize = 1;
        maXShape->setSize( awt::Size( nXSize, nYSize ) );
        maXShape->setPosition( awt::Point( static_cast<long>( rCenter.X - rSize.X ), static_cast<long>( rCenter.Y - rSize.Y ) ) );

        if ( rOrientation != 0 )
        {
            ImplSetOrientation( rCenter, rOrientation );
        }
        ImplSetFillBundle();
    }
}

void CGMImpressOutAct::DrawEllipticalArc( FloatPoint const & rCenter, FloatPoint const & rSize, double& rOrientation,
            sal_uInt32 nType, double& fStartAngle, double& fEndAngle )
{
    if ( ImplCreateShape( "com.sun.star.drawing.EllipseShape" ) )
    {
        uno::Any aAny;
        drawing::CircleKind eCircleKind;


        long nXSize = static_cast<long>( rSize.X * 2.0 );      // strange behaviour with a awt::Size of 0
        long nYSize = static_cast<long>( rSize.Y * 2.0 );
        if ( nXSize < 1 )
            nXSize = 1;
        if ( nYSize < 1 )
            nYSize = 1;

        maXShape->setSize( awt::Size ( nXSize, nYSize ) );

        if ( rOrientation != 0 )
        {
            fStartAngle = NormAngle360(fStartAngle + rOrientation);
            fEndAngle = NormAngle360(fEndAngle + rOrientation);
        }
        switch( nType )
        {
            case 0 : eCircleKind = drawing::CircleKind_SECTION; break;
            case 1 : eCircleKind = drawing::CircleKind_CUT; break;
            case 2 : eCircleKind = drawing::CircleKind_ARC; break;
            default : eCircleKind = drawing::CircleKind_FULL; break;
        }
        if ( static_cast<long>(fStartAngle) == static_cast<long>(fEndAngle) )
        {
            eCircleKind = drawing::CircleKind_FULL;
            maXPropSet->setPropertyValue( "CircleKind", uno::Any(eCircleKind) );
        }
        else
        {
            maXPropSet->setPropertyValue( "CircleKind", uno::Any(eCircleKind) );
            maXPropSet->setPropertyValue( "CircleStartAngle", uno::Any(static_cast<sal_Int32>( fStartAngle * 100 )) );
            maXPropSet->setPropertyValue( "CircleEndAngle", uno::Any(static_cast<sal_Int32>( fEndAngle * 100 )) );
        }
        maXShape->setPosition( awt::Point( static_cast<long>( rCenter.X - rSize.X ), static_cast<long>( rCenter.Y - rSize.Y ) ) );
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
                aAny <<= drawing::FillStyle_NONE;
                maXPropSet->setPropertyValue( "FillStyle", aAny );
            }
        }
    }
}

void CGMImpressOutAct::DrawBitmap( CGMBitmapDescriptor* pBmpDesc )
{
    if ( pBmpDesc->mbStatus && !!pBmpDesc->mxBitmap )
    {
        FloatPoint aOrigin = pBmpDesc->mnOrigin;
        double fdx = pBmpDesc->mndx;
        double fdy = pBmpDesc->mndy;

        BmpMirrorFlags nMirr = BmpMirrorFlags::NONE;
        if ( pBmpDesc->mbVMirror )
            nMirr |= BmpMirrorFlags::Vertical;
        if ( nMirr != BmpMirrorFlags::NONE )
            pBmpDesc->mxBitmap.Mirror( nMirr );

        mpCGM->ImplMapPoint( aOrigin );
        mpCGM->ImplMapX( fdx );
        mpCGM->ImplMapY( fdy );

        if ( ImplCreateShape( "com.sun.star.drawing.GraphicObjectShape" ) )
        {
            maXShape->setSize( awt::Size( static_cast<long>(fdx), static_cast<long>(fdy) ) );
            maXShape->setPosition( awt::Point( static_cast<long>(aOrigin.X), static_cast<long>(aOrigin.Y) ) );

            if ( pBmpDesc->mnOrientation != 0 )
            {
                ImplSetOrientation( aOrigin, pBmpDesc->mnOrientation );
            }

            uno::Reference< awt::XBitmap > xBitmap( VCLUnoHelper::CreateBitmap( pBmpDesc->mxBitmap ) );
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
        pOuterSequence->realloc(static_cast<sal_Int32>(nPoints));

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
        pOuterSequence->realloc(static_cast<sal_Int32>(nPoints));

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
            *pInnerFlags++ = static_cast<drawing::PolygonFlags>(rPolygon.GetFlags( i ));
        }
        uno::Any aParam;
        aParam <<= aRetval;
        maXPropSet->setPropertyValue( "PolyPolygonBezier", aParam );
        ImplSetLineBundle();
    }
}

void CGMImpressOutAct::DrawPolyPolygon( tools::PolyPolygon const & rPolyPolygon )
{
    sal_uInt32 nNumPolys = rPolyPolygon.Count();
    if ( nNumPolys && ImplCreateShape( "com.sun.star.drawing.ClosedBezierShape" ) )
    {
        drawing::PolyPolygonBezierCoords aRetval;

        // prepare inside polygons
        aRetval.Coordinates.realloc(static_cast<sal_Int32>(nNumPolys));
        aRetval.Flags.realloc(static_cast<sal_Int32>(nNumPolys));

        // get pointer to outside arrays
        drawing::PointSequence* pOuterSequence = aRetval.Coordinates.getArray();
        drawing::FlagSequence* pOuterFlags = aRetval.Flags.getArray();

        for( sal_uInt32 a = 0; a < nNumPolys; a++ )
        {
            const tools::Polygon& aPolygon( rPolyPolygon.GetObject( a ) );
            sal_uInt32 nNumPoints = aPolygon.GetSize();

            // make room in arrays
            pOuterSequence->realloc(static_cast<sal_Int32>(nNumPoints));
            pOuterFlags->realloc(static_cast<sal_Int32>(nNumPoints));

            // get pointer to arrays
            awt::Point* pInnerSequence = pOuterSequence->getArray();
            drawing::PolygonFlags* pInnerFlags = pOuterFlags->getArray();

            for( sal_uInt32 b = 0; b < nNumPoints; b++ )
            {
                *pInnerSequence++ = awt::Point( aPolygon.GetPoint( b ).X(), aPolygon.GetPoint( b ).Y() ) ;
                *pInnerFlags++ = static_cast<drawing::PolygonFlags>(aPolygon.GetFlags( b ));
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

void CGMImpressOutAct::DrawText(awt::Point const & rTextPos, awt::Size const & rTextSize, const OUString& rString, FinalFlag eFlag)
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
                aTextPos.Y = o3tl::saturating_add(aTextPos.X, static_cast<sal_Int32>((mpCGM->pElement->nCharacterHeight * -1.5) / 2));
            }
            break;

            case TAV_BASE :
            case TAV_BOTTOM :
            case TAV_NORMAL :
                aTextPos.Y = o3tl::saturating_add(aTextPos.Y, static_cast<sal_Int32>(mpCGM->pElement->nCharacterHeight * -1.5));
                break;
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
        double fSqrt = sqrt(nX * nX + nY * nY);
        double nOrientation = fSqrt != 0.0 ? (acos(nX / fSqrt) * 57.29577951308) : 0.0;
        if ( nY < 0 )
            nOrientation = 360 - nOrientation;

        if ( nOrientation )
        {
            maXPropSet->setPropertyValue( "RotationPointX", uno::Any(static_cast<sal_Int32>( aTextPos.X )) );
            maXPropSet->setPropertyValue( "RotationPointY", uno::Any(static_cast<sal_Int32>( aTextPos.Y + nHeight )) );
            maXPropSet->setPropertyValue( "RotateAngle", uno::Any(static_cast<sal_Int32>( nOrientation * 100 )) );
        }
        if ( nWidth == -1 )
        {
            aAny <<= true;
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
            maXPropSet->setPropertyValue( "TextAutoGrowHeight", uno::Any(true) );
        }
        uno::Reference< text::XText >  xText;
        uno::Any aFirstQuery( maXShape->queryInterface( cppu::UnoType<text::XText>::get()));
        if( aFirstQuery >>= xText )
        {
            uno::Reference< text::XTextCursor >  aXTextCursor( xText->createTextCursor() );
            {
                aXTextCursor->gotoEnd( false );
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
                                    aAny <<= sal_Int16(style::HorizontalAlignment_RIGHT);
                                break;
                                case TAH_LEFT :
                                case TAH_CONT :
                                case TAH_NORMAL :
                                    aAny <<= sal_Int16(style::HorizontalAlignment_LEFT);
                                break;
                                case TAH_CENTER :
                                    aAny <<= sal_Int16(style::HorizontalAlignment_CENTER);
                                break;
                            }
                            aCursorPropSet->setPropertyValue( "ParaAdjust", aAny );
                        }
                        if ( nWidth > 0 && nHeight > 0 )    // restricted text
                        {
                            aAny <<= true;
                            maXPropSet->setPropertyValue( "TextFitToSize", aAny );
                        }
                        aCursorText->setString(rString);
                        aXTextCursor->gotoEnd( true );
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

void CGMImpressOutAct::AppendText( const char* pString )
{
    if ( nFinalTextCount )
    {
        uno::Reference< drawing::XShape >  aShape = *o3tl::doAccess<uno::Reference<drawing::XShape>>(maXShapes->getByIndex( nFinalTextCount - 1 ));
        if ( aShape.is() )
        {
            uno::Reference< text::XText >  xText;
            uno::Any aFirstQuery(  aShape->queryInterface( cppu::UnoType<text::XText>::get()) );
            if( aFirstQuery >>= xText )
            {
                OUString aStr(pString, strlen(pString), RTL_TEXTENCODING_ASCII_US);

                uno::Reference< text::XTextCursor >  aXTextCursor( xText->createTextCursor() );
                if ( aXTextCursor.is() )
                {
                    aXTextCursor->gotoEnd( false );
                    uno::Reference< text::XTextRange >  aCursorText;
                    uno::Any aSecondQuery(aXTextCursor->queryInterface( cppu::UnoType<text::XTextRange>::get()));
                    if ( aSecondQuery >>= aCursorText )
                    {
                        uno::Reference< beans::XPropertySet >  aPropSet;
                        uno::Any aQuery(aCursorText->queryInterface( cppu::UnoType<beans::XPropertySet>::get()));
                        if( aQuery >>= aPropSet )
                        {
                            aCursorText->setString( aStr );
                            aXTextCursor->gotoEnd( true );
                            ImplSetTextBundle( aPropSet );
                        }
                    }
                }
            }
        }
    }
}


void CGMImpressOutAct::BeginFigure()
{
    if (!maPoints.empty())
        EndFigure();

    BeginGroup();
    maPoints.clear();
    maFlags.clear();
}

void CGMImpressOutAct::CloseRegion()
{
    if (maPoints.size() > 2)
    {
        NewRegion();
        DrawPolyPolygon( maPolyPolygon );
        maPolyPolygon.Clear();
    }
}

void CGMImpressOutAct::NewRegion()
{
    if (maPoints.size() > 2)
    {
        tools::Polygon aPolygon(maPoints.size(), maPoints.data(), maFlags.data());
        maPolyPolygon.Insert( aPolygon );
    }
    maPoints.clear();
    maFlags.clear();
}

void CGMImpressOutAct::EndFigure()
{
    NewRegion();
    DrawPolyPolygon( maPolyPolygon );
    maPolyPolygon.Clear();
    EndGroup();
    maPoints.clear();
    maFlags.clear();
}

void CGMImpressOutAct::RegPolyLine( tools::Polygon const & rPolygon, bool bReverse )
{
    sal_uInt16 nPoints = rPolygon.GetSize();
    if ( nPoints )
    {
        if ( bReverse )
        {
            for ( sal_uInt16 i = 0; i <  nPoints; i++ )
            {
                maPoints.push_back(rPolygon.GetPoint(nPoints - i - 1));
                maFlags.push_back(rPolygon.GetFlags(nPoints - i - 1));
            }
        }
        else
        {
            for ( sal_uInt16 i = 0; i <  nPoints; i++ )
            {
                maPoints.push_back(rPolygon.GetPoint(i));
                maFlags.push_back(rPolygon.GetFlags(i));
            }
        }
    }
}

void CGMImpressOutAct::SetGradientOffset( long nHorzOfs, long nVertOfs )
{
    if ( !mpGradient )
        mpGradient.reset( new awt::Gradient );
    mpGradient->XOffset = ( static_cast<sal_uInt16>(nHorzOfs) & 0x7f );
    mpGradient->YOffset = ( static_cast<sal_uInt16>(nVertOfs) & 0x7f );
}

void CGMImpressOutAct::SetGradientAngle( long nAngle )
{
    if ( !mpGradient )
        mpGradient.reset( new awt::Gradient );
    mpGradient->Angle = sal::static_int_cast< sal_Int16 >(nAngle);
}

void CGMImpressOutAct::SetGradientDescriptor( sal_uInt32 nColorFrom, sal_uInt32 nColorTo )
{
    if ( !mpGradient )
        mpGradient.reset( new awt::Gradient );
    mpGradient->StartColor = nColorFrom;
    mpGradient->EndColor = nColorTo;
}

void CGMImpressOutAct::SetGradientStyle( sal_uInt32 nStyle )
{
    if ( !mpGradient )
        mpGradient.reset( new awt::Gradient );
    switch ( nStyle )
    {
        case 0xff :
        {
            mpGradient->Style = awt::GradientStyle_AXIAL;
        }
        break;
        case 4 :
        {
            mpGradient->Style = awt::GradientStyle_RADIAL;          // CONICAL
        }
        break;
        case 3 :
        {
            mpGradient->Style = awt::GradientStyle_RECT;
        }
        break;
        case 2 :
        {
            mpGradient->Style = awt::GradientStyle_ELLIPTICAL;
        }
        break;
        default :
        {
            mpGradient->Style = awt::GradientStyle_LINEAR;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
