/*************************************************************************
 *
 *  $RCSfile: actimpr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:13 $
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

#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEDASH_HPP_
#include <com/sun/star/drawing/LineDash.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_HATCH_HPP_
#include <com/sun/star/drawing/Hatch.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTWEIGHT_HPP_
#include <com/sun/star/awt/FontWeight.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTUNDERLINE_HPP_
#include <com/sun/star/awt/FontUnderline.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPEGROUPER_HPP_
#include <com/sun/star/drawing/XShapeGrouper.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_CIRCLEKIND_HPP_
#include <com/sun/star/drawing/CircleKind.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XBITMAP_HPP_
#include <com/sun/star/awt/XBitmap.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCESEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONBEZIERCOORDS_HPP_
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FLAGSEQUENCE_HPP_
#include <com/sun/star/drawing/FlagSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTADJUST_HPP_
#include <com/sun/star/drawing/TextAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGE_HPP_
#include <com/sun/star/text/XTextRange.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_HORIZONTALALIGNMENT_HPP_
#include <com/sun/star/style/HorizontalAlignment.hpp>
#endif

#include <unotools/processfactory.hxx>

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif

//#include <toolkit/unohlp.hxx>

#include <main.hxx>
#include <outact.hxx>

using namespace ::com::sun::star;

// ---------------------------------------------------------------
// ---------------------------------------------------------------
// ---------------------------------------------------------------
// ---------------------------------------------------------------

CGMImpressOutAct::CGMImpressOutAct( CGM& rCGM, const uno::Reference< frame::XModel > & rModel ) :
        CGMOutAct       ( rCGM ),
        nFinalTextCount ( 0 )
{
    sal_Bool bStatRet = sal_False;

    if ( mpCGM->mbStatus )
    {
        uno::Reference< drawing::XDrawPagesSupplier >  aDrawPageSup( rModel, uno::UNO_QUERY );
        if( aDrawPageSup.is() )
        {
            maXDrawPages = aDrawPageSup->getDrawPages();
            if ( maXDrawPages.is() )
            {
                maXServiceManagerSC = utl::getProcessServiceFactory();
                if ( maXServiceManagerSC.is() )
                {

                    maXMultiServiceFactory = ::com::sun::star::uno::Reference< lang::XMultiServiceFactory >
                            ( maXServiceManagerSC, ::com::sun::star::uno::UNO_QUERY );
                    if ( maXMultiServiceFactory.is() )
                    {
                        maXDrawPage = *(uno::Reference< drawing::XDrawPage > *)maXDrawPages->getByIndex( 0 ).getValue();
                        if ( ImplInitPage() )
                            bStatRet = sal_True;
                    }
/*
                    uno::Any aAny( rModel->queryInterface( ::getCppuType((const uno::Reference< lang::XMultiServiceFactory >*)0) ));
                    if( aAny >>= maXMultiServiceFactory )
                    {
                        maXDrawPage = *(uno::Reference< drawing::XDrawPage > *)maXDrawPages->getByIndex( 0 ).getValue();
                        if ( ImplInitPage() )
                            bStatRet = sal_True;
                    }
*/
                }
            }
        }
        mpCGM->mbStatus = bStatRet;
    }
};

// ---------------------------------------------------------------

sal_Bool CGMImpressOutAct::ImplInitPage()
{
    sal_Bool    bStatRet = sal_False;
    if( maXDrawPage.is() )
    {
        maXShapes = uno::Reference< drawing::XShapes >( maXDrawPage, uno::UNO_QUERY );
        if ( maXShapes.is() )
        {
            bStatRet = sal_True;
        }
    }
    return bStatRet;
}

// ---------------------------------------------------------------

sal_Bool CGMImpressOutAct::ImplCreateShape( const ::rtl::OUString& rType )
{
    uno::Reference< uno::XInterface > xNewShape( maXMultiServiceFactory->createInstance( rType ) );
    maXShape = uno::Reference< drawing::XShape >( xNewShape, uno::UNO_QUERY );
    maXPropSet = uno::Reference< beans::XPropertySet >( xNewShape, uno::UNO_QUERY );
    if ( maXShape.is() && maXPropSet.is() )
    {
        maXShapes->add( maXShape );
        return sal_True;
    }
    return sal_False;
}

// ---------------------------------------------------------------

void CGMImpressOutAct::ImplSetOrientation( FloatPoint& rRefPoint, double& rOrientation )
{
    uno::Any aAny;
    aAny <<= (sal_Int32)rRefPoint.X;
    maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("RotationPointX"), aAny );
    aAny <<= (sal_Int32)rRefPoint.Y;
    maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("RotationPointY"), aAny );
    aAny <<= (sal_Int32)( rOrientation * 100.0 );
    maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("RotateAngle"), aAny );
}

// ---------------------------------------------------------------

void CGMImpressOutAct::ImplSetLineBundle()
{
    uno::Any            aAny;
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

    aAny <<= (sal_Int32)nLineColor;
    maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("LineColor"), aAny );

    aAny <<= (sal_Int32)fLineWidth;
    maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("LineWidth"), aAny );

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
    aAny <<= eLS;
    maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("LineStyle"), aAny );
    if ( eLS == drawing::LineStyle_DASH )
    {
        drawing::LineDash aLineDash( drawing::DashStyle_RECTRELATIVE, 1, 50, 3, 33, 100 );
        aAny <<= aLineDash;
        maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("LineDash"), aAny );
    }
};

// ---------------------------------------------------------------

void CGMImpressOutAct::ImplSetFillBundle()
{

    uno::Any                aAny;
    drawing::LineStyle      eLS;
    drawing::FillStyle      eFS;

    sal_uInt32              nEdgeColor;
    EdgeType                eEdgeType;
    double                  fEdgeWidth;

    sal_uInt32              nFillColor;
    FillInteriorStyle       eFillStyle;
    long                    nPatternIndex;
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
    if ( mpCGM->pElement->nAspectSourceFlags & ASF_PATTERNINDEX )
        nPatternIndex = mpCGM->pElement->pFillBundle->nFillPatternIndex;
    else
        nPatternIndex = mpCGM->pElement->aFillBundle.nFillPatternIndex;

    aAny <<= (sal_Int32)nFillColor;
    maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("FillColor"), aAny );

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
            if ( mpCGM->pElement->eTransparency = T_ON )
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
        aAny <<= *mpGradient;
        maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("FillGradient"), aAny );
    }
    aAny <<= eFS;
    maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("FillStyle"), aAny );

    eLS = drawing::LineStyle_NONE;
    if ( eFillStyle == FIS_HOLLOW )
    {
        eLS = drawing::LineStyle_SOLID;
        aAny <<= (sal_Int32)nFillColor;
        maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("LineColor"), aAny );
        aAny <<= (sal_Int32)0;
        maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("LineWidth"), aAny );
    }
    else if ( eEdgeType != ET_NONE )
    {
        aAny <<= (sal_Int32)nEdgeColor;
        maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("LineColor"), aAny );

        aAny <<= (sal_Int32)fEdgeWidth;
        maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("LineWidth"), aAny );

        switch( eEdgeType )
        {
            case ET_DASH :
            case ET_DOT :
            case ET_DASHDOT :
            case ET_DASHDOTDOT :
            case ET_DOTDOTSPACE :
            case ET_LONGDASH :
            case ET_DASHDASHDOT :
//          {
//              eLS = LineStyle_DASH;
//              aAny.setValue( &eLS, ::getCppuType((const drawing::LineStyle*)0) );
//              maXPropSet->setPropertyValue( L"LineStyle", aAny );
//              drawing::LineDash   aLineDash( DashStyle_RECTRELATIVE, 1, 160, 1, 160, 190 );
//              aAny.setValue( &aLineDash, ::getCppuType((const drawing::LineDash*)0) );
//              maXPropSet->setPropertyValue( L"DashStyle", aAny );
//          }
//          break;
            default:            // case ET_SOLID :
            {
                eLS = drawing::LineStyle_SOLID;
            }
            break;
        }
    }

    aAny <<= eLS;
    maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("LineStyle"), aAny );

    if ( eFS == drawing::FillStyle_HATCH )
    {
        drawing::Hatch aHatch;

        if ( mpCGM->pElement->nAspectSourceFlags & ASF_LINECOLOR )
            aHatch.Color = nFillColor;
        else
            aHatch.Color = nFillColor;
        HatchEntry*     pHatchEntry = (HatchEntry*)mpCGM->pElement->aHatchTable.Get( nHatchIndex );
        if ( pHatchEntry )
        {
            switch ( pHatchEntry->HatchStyle )
            {
            case 0 : aHatch.Style = drawing::HatchStyle_SINGLE; break;
            case 1 : aHatch.Style = drawing::HatchStyle_DOUBLE; break;
            case 2 : aHatch.Style = drawing::HatchStyle_TRIPLE; break;
            }
            aHatch.Distance = pHatchEntry->HatchDistance;
            aHatch.Angle = pHatchEntry->HatchAngle;
        }
        else
        {
            aHatch.Style = drawing::HatchStyle_TRIPLE;
            aHatch.Distance = 10 * ( nHatchIndex & 0x1f ) | 100;
            aHatch.Angle = 15 * ( ( nHatchIndex & 0x1f ) - 5 );
        }
        aAny <<= aHatch;
        maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("FillHatch"), aAny );
    }
};

// ---------------------------------------------------------------

void CGMImpressOutAct::ImplSetTextBundle( const uno::Reference< beans::XPropertySet > & rProperty )
{
    uno::Any        aAny;
    TextPrecision   eTextPrecision;
    sal_uInt32      nTextFontIndex;
    sal_uInt32      nTextColor;
    double          fCharacterExpansion;
    double          fCharacterSpacing;

    if ( mpCGM->pElement->nAspectSourceFlags & ASF_TEXTFONTINDEX )
        nTextFontIndex = mpCGM->pElement->pTextBundle->nTextFontIndex;
    else
        nTextFontIndex = mpCGM->pElement->aTextBundle.nTextFontIndex;
    if ( mpCGM->pElement->nAspectSourceFlags & ASF_TEXTPRECISION )
        eTextPrecision = mpCGM->pElement->pTextBundle->eTextPrecision;
    else
        eTextPrecision = mpCGM->pElement->aTextBundle.eTextPrecision;
    if ( mpCGM->pElement->nAspectSourceFlags & ASF_CHARACTEREXPANSION )
        fCharacterExpansion = mpCGM->pElement->pTextBundle->nCharacterExpansion;
    else
        fCharacterExpansion = mpCGM->pElement->aTextBundle.nCharacterExpansion;
    if ( mpCGM->pElement->nAspectSourceFlags & ASF_CHARACTERSPACING )
        fCharacterSpacing = mpCGM->pElement->pTextBundle->nCharacterSpacing;
    else
        fCharacterSpacing = mpCGM->pElement->aTextBundle.nCharacterSpacing;
    if ( mpCGM->pElement->nAspectSourceFlags & ASF_TEXTCOLOR )
        nTextColor = mpCGM->pElement->pTextBundle->GetColor();
    else
        nTextColor = mpCGM->pElement->aTextBundle.GetColor();

    aAny <<= (sal_Int32)nTextColor;
    rProperty->setPropertyValue( rtl::OUString::createFromAscii("CharColor"), aAny );

    sal_uInt32 nFontType = 0;
    awt::FontDescriptor aFontDescriptor;
    FontEntry* pFontEntry = mpCGM->pElement->aFontList.GetFontEntry( nTextFontIndex );
    if ( pFontEntry )
    {
        nFontType = pFontEntry->nFontType;
        aFontDescriptor.Name = String::CreateFromAscii( (char*)pFontEntry->pFontName );
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
    aAny <<= aFontDescriptor;
    rProperty->setPropertyValue( rtl::OUString::createFromAscii("FontDescriptor"), aAny );
};

// ---------------------------------------------------------------

void CGMImpressOutAct::InsertPage()
{
    if ( mnCurrentPage )    // eine seite ist immer vorhanden, deshalb wird die erste Seite ausgelassen
    {
        uno::Reference< drawing::XDrawPage > xPage( maXDrawPages->insertNewByIndex( 0xffff ), uno::UNO_QUERY );
        maXDrawPage = xPage;
        if ( ImplInitPage() == sal_False )
            mpCGM->mbStatus = sal_False;
    }
    mnCurrentPage++;
};

// ---------------------------------------------------------------

void CGMImpressOutAct::BeginGroup()
{
    if ( mnGroupLevel < CGM_OUTACT_MAX_GROUP_LEVEL )
    {
        mpGroupLevel[ mnGroupLevel ] = maXShapes->getCount();
    }
    mnGroupLevel++;
    mnGroupActCount = mpCGM->mnActCount;
};

// ---------------------------------------------------------------

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
            uno::Any aAny( maXDrawPage->queryInterface( ::getCppuType(((const uno::Reference< drawing::XShapeGrouper >*)0) )));
            if( aAny >>= aXShapeGrouper )
            {
                uno::Reference< drawing::XShapes >  aXShapes;
//              if ( maXServiceManagerSC->createInstance( L"stardiv.one.drawing.ShapeCollection" )->queryInterface( ::getCppuType((const Reference< drawing::XShapes >*)0), aXShapes ) )

                uno::Reference< drawing::XShape >  aXShapeCollection( maXServiceManagerSC->createInstance( rtl::OUString::createFromAscii("com.sun.star.drawing.ShapeCollection") ), uno::UNO_QUERY );
                if ( aXShapeCollection.is() )
                {
                    aXShapes = uno::Reference< drawing::XShapes >( aXShapeCollection, uno::UNO_QUERY );
                    if( aXShapes.is() )
                    {
                        for ( sal_uInt32 i = mnFirstIndex; i < mnCurrentCount; i++ )
                        {
                            uno::Reference< drawing::XShape >  aXShape = *(uno::Reference< drawing::XShape > *)maXShapes->getByIndex( i ).getValue();
                            if (aXShape.is() )
                            {
                                aXShapes->add( aXShape );
                            }
                        }
                    }
                }
                uno::Reference< drawing::XShapeGroup >  aXShapeGroup = aXShapeGrouper->group( aXShapes );
            }
        }
    }
};

// ---------------------------------------------------------------

void CGMImpressOutAct::EndGrouping()
{
    while ( mnGroupLevel )
    {
        EndGroup();
    }
}

// ---------------------------------------------------------------

void CGMImpressOutAct::DrawRectangle( FloatRect& rFloatRect )
{
    if ( mnGroupActCount != ( mpCGM->mnActCount - 1 ) )         // POWERPOINT HACK !!!
    {
        if ( ImplCreateShape( rtl::OUString::createFromAscii("com.sun.star.drawing.RectangleShape") ) )
        {
            awt::Size aSize( (long)(rFloatRect.Right - rFloatRect.Left ), (long)(rFloatRect.Bottom-rFloatRect.Top ) );
            maXShape->setSize( aSize );
            maXShape->setPosition( awt::Point( (long)rFloatRect.Left, (long)rFloatRect.Top ) );
            ImplSetFillBundle();
        }
    }
};

// ---------------------------------------------------------------

void CGMImpressOutAct::DrawEllipse( FloatPoint& rCenter, FloatPoint& rSize, double& rOrientation )
{
    if ( ImplCreateShape( rtl::OUString::createFromAscii("com.sun.star.drawing.EllipseShape") ) )
    {
        drawing::CircleKind eCircleKind = drawing::CircleKind_FULL;
        uno::Any aAny( &eCircleKind, ::getCppuType((const drawing::CircleKind*)0) );
        maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("CircleKind"), aAny );

        long nXSize = (long)( rSize.X * 2.0 );      // Merkwuerdigkes Verhalten bei einer awt::Size von 0
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
};

// ---------------------------------------------------------------

void CGMImpressOutAct::DrawEllipticalArc( FloatPoint& rCenter, FloatPoint& rSize, double& rOrientation,
            sal_uInt32 nType, double& fStartAngle, double& fEndAngle )
{
    if ( ImplCreateShape( rtl::OUString::createFromAscii("com.sun.star.drawing.EllipseShape") ) )
    {
        uno::Any aAny;
        drawing::CircleKind eCircleKind;


        long nXSize = (long)( rSize.X * 2.0 );      // Merkwuerdigkes Verhalten bei einer awt::Size von 0
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
            aAny.setValue( &eCircleKind, ::getCppuType((const drawing::CircleKind*)0) );
        }
        else
        {
            aAny.setValue( &eCircleKind, ::getCppuType((const drawing::CircleKind*)0) );
            maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("CircleKind"), aAny );
            aAny <<= (sal_Int32)( (long)( fStartAngle * 100 ) );
            maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("CircleStartAngle"), aAny );
            aAny <<= (sal_Int32)( (long)( fEndAngle * 100 ) );
            maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("CircleEndAngle"), aAny );
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
                aAny.setValue( &eFillStyle, ::getCppuType((const drawing::FillStyle*)0) );
                maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("FillStyle"), aAny );
            }
        }
    }
};

// ---------------------------------------------------------------

void CGMImpressOutAct::DrawBitmap( CGMBitmapDescriptor* pBmpDesc )
{
    if ( pBmpDesc->mbStatus && pBmpDesc->mpBitmap )
    {
        FloatPoint aOrigin = pBmpDesc->mnOrigin;
        double fdx = pBmpDesc->mndx;
        double fdy = pBmpDesc->mndy;

        sal_uInt32  nMirr = BMP_MIRROR_NONE;
        if ( pBmpDesc->mbVMirror )
            nMirr |= BMP_MIRROR_VERT;
        if ( pBmpDesc->mbHMirror )
            nMirr |= BMP_MIRROR_HORZ;
        if ( nMirr != BMP_MIRROR_NONE )
            pBmpDesc->mpBitmap->Mirror( nMirr );

        mpCGM->ImplMapPoint( aOrigin );
        mpCGM->ImplMapX( fdx );
        mpCGM->ImplMapY( fdy );

        if ( ImplCreateShape( rtl::OUString::createFromAscii("com.sun.star.drawing.GraphicObjectShape") ) )
        {
            maXShape->setSize( awt::Size( (long)fdx, (long)fdy ) );
            maXShape->setPosition( awt::Point( (long)aOrigin.X, (long)aOrigin.Y ) );

            if ( pBmpDesc->mnOrientation != 0 )
            {
                ImplSetOrientation( aOrigin, pBmpDesc->mnOrientation );
            }

            uno::Reference< awt::XBitmap > xBitmap( VCLUnoHelper::CreateBitmap( BitmapEx( *( pBmpDesc->mpBitmap ) ) ) );
            uno::Any aAny;
            aAny <<= xBitmap;
            maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("GraphicObjectFillBitmap"), aAny );

        }
    }
};

// ---------------------------------------------------------------

void CGMImpressOutAct::DrawPolygon( Polygon& rPoly )
{
    sal_uInt16 nPoints = rPoly.GetSize();

    if ( ( nPoints > 1 ) && ImplCreateShape( rtl::OUString::createFromAscii("com.sun.star.drawing.PolyPolygonShape") ) )
    {
        drawing::PointSequenceSequence aRetval;

        // Polygone innerhalb vrobereiten
        aRetval.realloc( 1 );

        // Zeiger auf aeussere Arrays holen
        drawing::PointSequence* pOuterSequence = aRetval.getArray();

        // Platz in Arrays schaffen
        pOuterSequence->realloc((sal_Int32)nPoints);

        // Pointer auf arrays holen
        awt::Point* pInnerSequence = pOuterSequence->getArray();

        for( sal_uInt16 n = 0; n < nPoints; n++ )
            *pInnerSequence++ = awt::Point( rPoly[ n ].X(), rPoly[n].Y() );

        uno::Any aParam;
        aParam <<= aRetval;
        maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("PolyPolygon"), aParam );
        ImplSetFillBundle();
    }
};


// ---------------------------------------------------------------

void CGMImpressOutAct::DrawPolyLine( Polygon& rPoly )
{
    sal_uInt16 nPoints = rPoly.GetSize();

    if ( ( nPoints > 1 ) && ImplCreateShape( rtl::OUString::createFromAscii("com.sun.star.drawing.PolyLineShape") ) )
    {
        drawing::PointSequenceSequence aRetval;

        // Polygone innerhalb vrobereiten
        aRetval.realloc( 1 );

        // Zeiger auf aeussere Arrays holen
        drawing::PointSequence* pOuterSequence = aRetval.getArray();

        // Platz in Arrays schaffen
        pOuterSequence->realloc((sal_Int32)nPoints);

        // Pointer auf arrays holen
        awt::Point* pInnerSequence = pOuterSequence->getArray();

        for( sal_uInt16 n = 0; n < nPoints; n++ )
            *pInnerSequence++ = awt::Point( rPoly[ n ].X(), rPoly[n].Y() );

        uno::Any aParam;
        aParam <<= aRetval;
        maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("PolyPolygon"), aParam );
        ImplSetLineBundle();
    }
};

// ---------------------------------------------------------------

void CGMImpressOutAct::DrawPolybezier( Polygon& rPolygon )
{
    sal_uInt32 nPoints = rPolygon.GetSize();
    if ( ( nPoints > 1 ) && ImplCreateShape( rtl::OUString::createFromAscii("com.sun.star.drawing.OpenBezierShape") ) )
    {
        drawing::PolyPolygonBezierCoords aRetval;

        aRetval.Coordinates.realloc( 1 );
        aRetval.Flags.realloc( 1 );

        // Zeiger auf aeussere Arrays holen
        drawing::PointSequence* pOuterSequence = aRetval.Coordinates.getArray();
        drawing::FlagSequence* pOuterFlags = aRetval.Flags.getArray();

        // Platz in Arrays schaffen
        pOuterSequence->realloc( nPoints );
        pOuterFlags->realloc( nPoints );

        awt::Point* pInnerSequence = pOuterSequence->getArray();
        drawing::PolygonFlags* pInnerFlags = pOuterFlags->getArray();

        for( sal_uInt32 i = 0; i < nPoints; i++ )
        {
            *pInnerSequence++ = awt::Point( rPolygon[ i ].X(), rPolygon[ i ].Y() );
            *pInnerFlags++ = (drawing::PolygonFlags)rPolygon.GetFlags( i );
        }
        uno::Any aParam;
        aParam <<= aRetval;
        maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("PolyPolygonBezier"), aParam );
        ImplSetLineBundle();
    }
};

// ---------------------------------------------------------------

void CGMImpressOutAct::DrawPolyPolygon( PolyPolygon& rPolyPolygon )
{
    sal_uInt32 nNumPolys = rPolyPolygon.Count();
    if ( nNumPolys && ImplCreateShape( rtl::OUString::createFromAscii("com.sun.star.drawing.ClosedBezierShape") ) )
    {
        drawing::PolyPolygonBezierCoords aRetval;

        // Polygone innerhalb vrobereiten
        aRetval.Coordinates.realloc((sal_Int32)nNumPolys);
        aRetval.Flags.realloc((sal_Int32)nNumPolys);

        // Zeiger auf aeussere Arrays holen
        drawing::PointSequence* pOuterSequence = aRetval.Coordinates.getArray();
        drawing::FlagSequence* pOuterFlags = aRetval.Flags.getArray();

        for( sal_uInt16 a = 0; a < nNumPolys; a++ )
        {
            Polygon aPolygon( rPolyPolygon.GetObject( a ) );
            sal_uInt32 nNumPoints = aPolygon.GetSize();

            // Platz in Arrays schaffen
            pOuterSequence->realloc((sal_Int32)nNumPoints);
            pOuterFlags->realloc((sal_Int32)nNumPoints);

            // Pointer auf arrays holen
            awt::Point* pInnerSequence = pOuterSequence->getArray();
            drawing::PolygonFlags* pInnerFlags = pOuterFlags->getArray();

            for( sal_uInt16 b = 0; b < nNumPoints; b++ )
            {
                *pInnerSequence++ = awt::Point( aPolygon.GetPoint( b ).X(), aPolygon.GetPoint( b ).Y() ) ;
                *pInnerFlags++ = (drawing::PolygonFlags)aPolygon.GetFlags( b );
            }
            pOuterSequence++;
            pOuterFlags++;
        }
        uno::Any aParam;
        aParam <<= aRetval;
        maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("PolyPolygonBezier"), aParam);
        ImplSetFillBundle();
    }
};

// ---------------------------------------------------------------

void CGMImpressOutAct::DrawText( awt::Point& rTextPos, awt::Size& rTextSize, char* pString, sal_uInt32 nSize, FinalFlag eFlag )
{
    if ( ImplCreateShape( rtl::OUString::createFromAscii("com.sun.star.drawing.TextShape") ) )
    {
        uno::Any    aAny;
        long    nWidth = rTextSize.Width;
        long    nHeight = rTextSize.Height;

        awt::Point aTextPos( rTextPos );
        switch ( mpCGM->pElement->eTextAlignmentV )
        {
            case TAV_HALF :
            {
                aTextPos.Y -= ( ( mpCGM->pElement->nCharacterHeight * 1.5 ) / 2 );
            }
            break;

            case TAV_BASE :
            case TAV_BOTTOM :
            case TAV_NORMAL :
                aTextPos.Y -= ( mpCGM->pElement->nCharacterHeight * 1.5 );
            case TAV_TOP :
            break;
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
            aAny <<= (sal_Int32)( aTextPos.X );
            maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("RotationPointX"), aAny );
            aAny <<= (sal_Int32)( aTextPos.Y + nHeight );
            maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("RotationPointY"), aAny );
            aAny <<= (sal_Int32)( (sal_Int32)( nOrientation * 100 ) );
            maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("RotateAngle"), aAny );
        }
        if ( nWidth == -1 )
        {
            sal_Bool bTrue( sal_True );
            aAny.setValue( &bTrue, ::getCppuType((const sal_Bool*)0 ));
            maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("TextAutoGrowWidth"), aAny );

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
            aAny <<= eTextAdjust;
            maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("TextHorizontalAdjust"), aAny );
        }
        if ( nHeight == -1 )
        {
            sal_Bool bTrue = sal_True;
            aAny.setValue( &bTrue, ::getCppuType((const sal_Bool*)0) );
            maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("TextAutoGrowHeight"), aAny );
        }
        uno::Reference< text::XText >  xText;
        uno::Any aQuery( maXShape->queryInterface( ::getCppuType((const uno::Reference< text::XText >*)0) ));
        if( aQuery >>= xText )
        {
            String aStr( String::CreateFromAscii( pString ) );

            uno::Reference< text::XTextCursor >  aXTextCursor( xText->createTextCursor() );
            {
                aXTextCursor->gotoEnd( sal_False );
                uno::Reference< text::XTextRange >  aCursorText;
                uno::Any aQuery( aXTextCursor->queryInterface( ::getCppuType((const uno::Reference< text::XTextRange >*)0) ));
                if ( aQuery >>= aCursorText )
                {
                    uno::Reference< beans::XPropertySet >  aCursorPropSet;

                    uno::Any aQuery( aCursorText->queryInterface( ::getCppuType((const uno::Reference< beans::XPropertySet >*)0) ));
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
                            aCursorPropSet->setPropertyValue( rtl::OUString::createFromAscii("ParaAdjust"), aAny );
                        }
                        if ( nWidth > 0 && nHeight > 0 )    // restricted text
                        {
                            sal_Bool bTrue = sal_True;
                            aAny.setValue( &bTrue, ::getCppuType((const sal_Bool*)0));
                            maXPropSet->setPropertyValue( rtl::OUString::createFromAscii("TextFitToSize"), aAny );
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
};

// ---------------------------------------------------------------

void CGMImpressOutAct::AppendText( char* pString, sal_uInt32 nSize, FinalFlag eFlag )
{
    if ( nFinalTextCount )
    {
        uno::Reference< drawing::XShape >  aShape = *(uno::Reference< drawing::XShape > *)maXShapes->getByIndex( nFinalTextCount - 1 ).getValue();
        if ( aShape.is() )
        {
            uno::Reference< text::XText >  xText;
            uno::Any aQuery(  aShape->queryInterface( ::getCppuType((const uno::Reference< text::XText >*)0)) );
            if( aQuery >>= xText )
            {
                String aStr( String::CreateFromAscii( pString ) );

                uno::Reference< text::XTextCursor >  aXTextCursor( xText->createTextCursor() );
                if ( aXTextCursor.is() )
                {
                    aXTextCursor->gotoEnd( sal_False );
                    uno::Reference< text::XTextRange >  aCursorText;
                    uno::Any aQuery(aXTextCursor->queryInterface( ::getCppuType((const uno::Reference< text::XTextRange >*)0) ));
                    if ( aQuery >>= aCursorText )
                    {
                        uno::Reference< beans::XPropertySet >  aPropSet;
                        uno::Any aQuery(aCursorText->queryInterface( ::getCppuType((const uno::Reference< beans::XPropertySet >*)0) ));
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

// ---------------------------------------------------------------
// nCount != 0 -> Append Text
sal_uInt32 CGMImpressOutAct::DrawText( TextEntry* pTextEntry, NodeFrameSet& rNodeFrameSet, sal_uInt32 nObjCount )
{

return 0;

/*
    uno::Reference< drawing::XShape >  aShape;

    if ( nObjCount )
    {
         aShape = (drawing::XShape*) maXShapes->getElementByIndex( nObjCount - 1 )->queryInterface( ::getCppuType((const Reference< drawing::XShape >*)0) );
    }
    else
    {
        aShape = maXShapes->insertShape( maXShapeFactory->createShape( L"ShapeText", rNodeFrameSet.nSize ), rNodeFrameSet.nTopLeft );
    }
    if ( aShape.is() )
    {
        uno::Reference< text::XText >  xText = (text::XText*)aShape->queryInterface( ::getCppuType((const Reference< text::XText >*)0) );
        if ( xText.is() )
        {
            uno::Reference< text::XTextCursor >  aXTextCursor = (text::XTextCursor*)xText->createTextCursor()->queryInterface( ::getCppuType((const Reference< text::XTextCursor >*)0) );
            if ( aXTextCursor.is() )
            {
                uno::Any    aAny;
                sal_uInt32  nTextOfs = 0;
                TextAttribute* pTAttr = pTextEntry->pAttribute;
                do
                {
                    if ( pTAttr->nTextAttribSize > 0.3 )    // is text readable
                    {
                        aXTextCursor->gotoEnd( sal_False );
                        char nPushedChar = pTextEntry->pText[ nTextOfs + pTAttr->nTextAttribCount ];
                        pTextEntry->pText[ nTextOfs + pTAttr->nTextAttribCount ] = 0;
                        UString aStr( StringToOUString( pTextEntry->pText +  nTextOfs, CHARSET_SYSTEM ) );

                        uno::Reference< text::XText >  aCursorText = (text::XText*)aXTextCursor->queryInterface( ::getCppuType((const Reference< text::XText >*)0) );
                        if ( aCursorText.is() )
                        {
                            uno::Reference< beans::XPropertySet >  aPropSet = (beans::XPropertySet*)aCursorText->queryInterface( ::getCppuType((const Reference< beans::XPropertySet >*)0) );
                            if ( aPropSet.is() )
                            {
                                if ( pTextEntry->nRowOrLineNum )
                                {
                                    uno::Reference< XControlCharacterInsertable >  aCRef = (XControlCharacterInsertable*)aXTextCursor->queryInterface( ::getCppuType((const Reference< XControlCharacterInsertable >*)0) );
                                    if ( aCRef.is() )
                                    {
                                        aCRef->insertControlCharacter( TEXTCONTROLCHAR_PARAGRAPH_BREAK );
                                    }
                                }
                                aCursorText->setText( aStr );
                                aXTextCursor->gotoEnd( sal_True );
                                double nSize = mpCGM->mnOutdx;
                                if ( mpCGM->mnOutdx < mpCGM->mnOutdy )
                                    nSize = mpCGM->mnOutdy;
                                nSize = ( nSize * (double)pTAttr->nTextAttribSize * (double)1.5 ) / 100;

                                aAny <<= (sal_Int32)( (sal_Int32)nSize );
                                aPropSet->setPropertyValue( L"CharHeight", aAny );

                                sal_uInt32 nTextColor = pTAttr->nTextColorIndex;
                                if ( nTextColor == 0xff )
                                {
                                    nTextColor = ( pTAttr->nTextColorBlue << 16 ) + ( pTAttr->nTextColorGreen << 8 ) + pTAttr->nTextColorRed;
                                }
                                else
                                {
                                    nTextColor = mpCGM->pElement->aColorTable[ nTextColor ];
                                }

                                sal_uInt32 nFontType = 0;

                                if (  pTAttr->nTextFontType == 0xff )
                                {
                                    FontEntry* pFontEntry = mpCGM->pElement->aFontList.GetFontEntry( pTAttr->nTextFontFamily );
                                    if ( pFontEntry )
                                    {
                                        nFontType = pFontEntry->nFontType;
                                        if ( mpCGM->pElement->nAspectSourceFlags & ASF_TEXTCOLOR )
                                            nTextColor = mpCGM->pElement->pTextBundle->GetColor();
                                        else
                                            nTextColor = mpCGM->pElement->aTextBundle.GetColor();
                                    }
                                    FontItalic eFontItalic = ITALIC_NONE;
                                    if ( nFontType & 1 )
                                        eFontItalic = ITALIC_NORMAL;
                                    aAny.setValue( &eFontItalic, ::getCppuType((const FontItalic*)0) );
                                    aPropSet->setPropertyValue( L"CharPosture", aAny );
                                }
                                aAny <<= (sal_Int32)( (sal_Int32)nTextColor );
                                aPropSet->setPropertyValue( L"CharColor", aAny );

                                awt::FontWeight eFontWeight = WEIGHT_NORMAL;
                                if ( nFontType & 2 )
                                    eFontWeight = WEIGHT_BOLD;
                                aAny.setValue( &eFontWeight, ::getCppuType((const awt::FontWeight*)0) );
                                aPropSet->setPropertyValue( L"CharWeight", aAny );

                                if ( pTAttr->nTextAttribBits & 0x4000 )
                                {
                                    awt::FontUnderline eUnderline = UNDERLINE_SINGLE;
                                    aAny.setValue( &eUnderline, ::getCppuType((const awt::FontUnderline*)0) );
                                    aPropSet->setPropertyValue( L"CharUnderline", aAny );
                                }
                            }
                        }
                        pTextEntry->pText[ nTextOfs + pTAttr->nTextAttribCount ] = nPushedChar;
                    }
                    nTextOfs += pTAttr->nTextAttribCount;
                }
                while ( ( ( pTAttr = pTAttr->pNextAttribute ) != NULL ) );
            }
        }
    }
    return ( nObjCount ) ? nObjCount : maXShapes->getCount();
*/
}


// ---------------------------------------------------------------

void CGMImpressOutAct::ImplGetFrameSet( int nNodeNumber, NodeFrameSet& rFrameSet )
{
/*
    DataNode* pDataNode = &mpCGM->mpChart->mDataNode[ nNodeNumber ];
    FloatPoint aFloatPoint0, aFloatPoint1;

    if ( pDataNode->nBoxX1 < pDataNode->nBoxX2 )
    {
        aFloatPoint0.X = (double)pDataNode->nBoxX1;
        aFloatPoint1.X = (double)pDataNode->nBoxX2;
    }
    else
    {
        aFloatPoint0.X = (double)pDataNode->nBoxX2;
        aFloatPoint1.X = (double)pDataNode->nBoxX1;
    }

    if ( pDataNode->nBoxY1 < pDataNode->nBoxY2 )
    {
        aFloatPoint0.Y = (double)( pDataNode->nBoxY1 );
        aFloatPoint1.Y = (double)( pDataNode->nBoxY2 );
    }
    else
    {
        aFloatPoint0.Y = (double)pDataNode->nBoxY2;
        aFloatPoint1.Y = (double)pDataNode->nBoxY1;
    }
    mpCGM->ImplMapPoint( aFloatPoint0 );
    mpCGM->ImplMapPoint( aFloatPoint1 );
    rFrameSet.nTopLeft.X() = (long)aFloatPoint0.X;
    rFrameSet.nTopLeft.Y() = (long)aFloatPoint0.Y;
    rFrameSet.nBottomRight.X() = (long)aFloatPoint1.X;
    rFrameSet.nBottomRight.Y() = (long)aFloatPoint1.Y;
    rFrameSet.nSize.Width() = ( (long)( aFloatPoint1.X - aFloatPoint0.X ) );
    rFrameSet.nSize.Height() = ( (long)( aFloatPoint1.Y - aFloatPoint0.Y ) );
*/
}

// ---------------------------------------------------------------

void CGMImpressOutAct::DrawChart()
{
/*
    NodeFrameSet    aNodeFrameSet;
    CGMChart*       mpChart = mpCGM->mpChart;
    switch ( mpChart->mnCurrentFileType )
    {
        case BULCHART :
        {
            sal_Bool    bBulletBody = sal_False;
            sal_uInt32  nObjCount = 0;

            TextEntry*  pTextEntry;
            while( ( pTextEntry = mpChart->GetFirstTextEntry() ) )
            {
                switch( pTextEntry->nTypeOfText )
                {
                    case IOC_CHTTITLE :
                    {
                        switch( pTextEntry->nRowOrLineNum )
                        {
                            case 0 :        // MainTitle
                            {
                                ImplGetFrameSet( 2, aNodeFrameSet );
                                nObjCount = DrawText( pTextEntry, aNodeFrameSet, 0 );
                            }
                            break;
                        };
                    }
                    break;

                    case IOC_BULLETBODY :
                    {
                        if ( bBulletBody )
                        {
                            DrawText( pTextEntry, aNodeFrameSet, nObjCount );
                        }
                        else
                        {
                            bBulletBody = sal_True; // this is the first text entry for the bullet zone
                            ImplGetFrameSet( 3, aNodeFrameSet );
                            nObjCount = DrawText( pTextEntry, aNodeFrameSet, 0 );
                        }
                    }
                    break;

                    default :
                    break;
                };
                mpChart->DeleteTextEntry( pTextEntry );
            }
        }
        break;

        default :
        {
            TextEntry*  pTextEntry;
            while( ( pTextEntry = mpChart->GetFirstTextEntry() ) )
            {
                mpChart->DeleteTextEntry( pTextEntry );
            }
        }
        break;
    }
*/
}


