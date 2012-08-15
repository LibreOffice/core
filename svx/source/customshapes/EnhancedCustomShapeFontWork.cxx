/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "EnhancedCustomShapeFontWork.hxx"
#include <tools/solar.h>               // UINTXX
#include <svx/svddef.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdopath.hxx>
#include <vcl/metric.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdasitm.hxx>
#include <svx/sdasaitm.hxx>
#include <svx/sdtfsitm.hxx>
#include <vcl/virdev.hxx>
#include <svx/svditer.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/charscaleitem.hxx>
#include "svx/EnhancedCustomShapeTypeNames.hxx"
#include <svx/svdorect.hxx>
#include <svx/svdoashp.hxx>
#include <editeng/outliner.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editeng.hxx>
#include <svx/svdmodel.hxx>
#include <vector>
#include <numeric>
#include <algorithm>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <basegfx/polygon/b2dpolygontools.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;

struct FWCharacterData                  // representing a single character
{
    std::vector< PolyPolygon >          vOutlines;
    Rectangle                           aBoundRect;
};
struct FWParagraphData                  // representing a single paragraph
{
    rtl::OUString                       aString;
    std::vector< FWCharacterData >      vCharacters;
    Rectangle                           aBoundRect;
    sal_Int16                           nFrameDirection;
};
struct FWTextArea                       // representing multiple concluding paragraphs
{
    std::vector< FWParagraphData >      vParagraphs;
    Rectangle                           aBoundRect;
};
struct FWData                           // representing the whole text
{
    std::vector< FWTextArea >           vTextAreas;
    double                              fHorizontalTextScaling;
    sal_uInt32                          nMaxParagraphsPerTextArea;
    sal_Int32                           nSingleLineHeight;
    sal_Bool                            bSingleLineMode;
};


sal_Bool InitializeFontWorkData( const SdrObject* pCustomShape, const sal_uInt16 nOutlinesCount2d, FWData& rFWData )
{
    sal_Bool bNoErr = sal_False;
    sal_Bool bSingleLineMode = sal_False;
    sal_uInt16 nTextAreaCount = nOutlinesCount2d;
    if ( nOutlinesCount2d & 1 )
        bSingleLineMode = sal_True;
    else
        nTextAreaCount >>= 1;

    if ( nTextAreaCount )
    {
        rFWData.bSingleLineMode = bSingleLineMode;

        // setting the strings
        OutlinerParaObject* pParaObj = ((SdrObjCustomShape*)pCustomShape)->GetOutlinerParaObject();
        if ( pParaObj )
        {
            const EditTextObject& rTextObj = pParaObj->GetTextObject();
            sal_Int32 nParagraphsLeft = rTextObj.GetParagraphCount();

            rFWData.nMaxParagraphsPerTextArea = ( ( nParagraphsLeft - 1 ) / nTextAreaCount ) + 1;
            sal_Int16 j = 0;
            while( nParagraphsLeft && nTextAreaCount )
            {
                FWTextArea aTextArea;
                sal_Int32 i, nParagraphs = ( ( nParagraphsLeft - 1 ) / nTextAreaCount ) + 1;
                for ( i = 0; i < nParagraphs; ++i, ++j )
                {
                    FWParagraphData aParagraphData;
                    aParagraphData.aString = rTextObj.GetText( j );

                    const SfxItemSet& rParaSet = rTextObj.GetParaAttribs( j );  // retrieving some paragraph attributes
                    aParagraphData.nFrameDirection = ((SvxFrameDirectionItem&)rParaSet.Get( EE_PARA_WRITINGDIR )).GetValue();
                    aTextArea.vParagraphs.push_back( aParagraphData );
                }
                rFWData.vTextAreas.push_back( aTextArea );
                nParagraphsLeft -= nParagraphs;
                nTextAreaCount--;
            }
            bNoErr = sal_True;
        }
    }
    return bNoErr;
}

double GetLength( const Polygon& rPolygon )
{
    double fLength = 0;
    if ( rPolygon.GetSize() > 1 )
    {
        sal_uInt16 nCount = rPolygon.GetSize();
        while( --nCount )
            fLength += ((Polygon&)rPolygon).CalcDistance( nCount, nCount - 1 );
    }
    return fLength;
}


/* CalculateHorizontalScalingFactor returns the horizontal scaling factor for
the whole text object, so that each text will match its corresponding 2d Outline */
void CalculateHorizontalScalingFactor( const SdrObject* pCustomShape,
                                        FWData& rFWData, const PolyPolygon& rOutline2d )
{
    double fScalingFactor = 1.0;
    sal_Bool bScalingFactorDefined = sal_False;

    sal_uInt16 i = 0;
    sal_Bool bSingleLineMode = sal_False;
    sal_uInt16 nOutlinesCount2d = rOutline2d.Count();

    Font aFont;
    SvxFontItem& rFontItem = (SvxFontItem&)pCustomShape->GetMergedItem( EE_CHAR_FONTINFO );
    aFont.SetHeight( pCustomShape->GetLogicRect().GetHeight() / rFWData.nMaxParagraphsPerTextArea );
    aFont.SetAlign( ALIGN_TOP );
    aFont.SetName( rFontItem.GetFamilyName() );
    aFont.SetFamily( rFontItem.GetFamily() );
    aFont.SetStyleName( rFontItem.GetStyleName() );
    aFont.SetOrientation( 0 );
    // initializing virtual device

    VirtualDevice aVirDev( 1 );
    aVirDev.SetMapMode( MAP_100TH_MM );
    aVirDev.SetFont( aFont );

    if ( nOutlinesCount2d & 1 )
        bSingleLineMode = sal_True;

    std::vector< FWTextArea >::iterator aTextAreaIter = rFWData.vTextAreas.begin();
    std::vector< FWTextArea >::iterator aTextAreaIEnd = rFWData.vTextAreas.end();
    while( aTextAreaIter != aTextAreaIEnd )
    {
        // calculating the width of the corresponding 2d text area
        double fWidth = GetLength( rOutline2d.GetObject( i++ ) );
        if ( !bSingleLineMode )
        {
            fWidth += GetLength( rOutline2d.GetObject( i++ ) );
            fWidth /= 2.0;
        }
        std::vector< FWParagraphData >::const_iterator aParagraphIter( aTextAreaIter->vParagraphs.begin() );
        std::vector< FWParagraphData >::const_iterator aParagraphIEnd( aTextAreaIter->vParagraphs.end() );
        while( aParagraphIter != aParagraphIEnd )
        {
            double fTextWidth = aVirDev.GetTextWidth( aParagraphIter->aString );
            if ( fTextWidth > 0.0 )
            {
                double fScale = fWidth / fTextWidth;
                if ( !bScalingFactorDefined )
                {
                    fScalingFactor = fScale;
                    bScalingFactorDefined = sal_True;
                }
                else
                {
                    if ( fScale < fScalingFactor )
                        fScalingFactor = fScale;
                }
            }
            ++aParagraphIter;
        }
        ++aTextAreaIter;
    }
    rFWData.fHorizontalTextScaling = fScalingFactor;
}

void GetTextAreaOutline( const FWData& rFWData, const SdrObject* pCustomShape, FWTextArea& rTextArea, sal_Bool bSameLetterHeights )
{
    sal_Bool bIsVertical = ((SdrObjCustomShape*)pCustomShape)->IsVerticalWriting();
    sal_Int32 nVerticalOffset = rFWData.nMaxParagraphsPerTextArea > rTextArea.vParagraphs.size()
                                    ? rFWData.nSingleLineHeight / 2 : 0;

    std::vector< FWParagraphData >::iterator aParagraphIter( rTextArea.vParagraphs.begin() );
    std::vector< FWParagraphData >::iterator aParagraphIEnd( rTextArea.vParagraphs.end() );
    while( aParagraphIter != aParagraphIEnd )
    {
        const rtl::OUString& rText = aParagraphIter->aString;
        if ( !rText.isEmpty() )
        {
            // generating vcl/font
            sal_uInt16 nScriptType = i18n::ScriptType::LATIN;
            Reference< i18n::XBreakIterator > xBI( EnhancedCustomShapeFontWork::GetBreakIterator() );
            if ( xBI.is() )
            {
                nScriptType = xBI->getScriptType( rText, 0 );
                if( i18n::ScriptType::WEAK == nScriptType )
                {
                    sal_uInt16 nChg = 0;
                    nChg = (xub_StrLen)xBI->endOfScript( rText, nChg, nScriptType );
                    if( nChg < rText.getLength() )
                        nScriptType = xBI->getScriptType( rText, nChg );
                    else
                        nScriptType = i18n::ScriptType::LATIN;
                }
            }
            sal_uInt16 nFntItm = EE_CHAR_FONTINFO;
            if ( nScriptType == i18n::ScriptType::COMPLEX )
                nFntItm = EE_CHAR_FONTINFO_CTL;
            else if ( nScriptType == i18n::ScriptType::ASIAN )
                nFntItm = EE_CHAR_FONTINFO_CJK;
            SvxFontItem& rFontItem = (SvxFontItem&)pCustomShape->GetMergedItem( nFntItm );
            Font aFont;
            aFont.SetHeight( rFWData.nSingleLineHeight );
            aFont.SetAlign( ALIGN_TOP );

            aFont.SetName( rFontItem.GetFamilyName() );
            aFont.SetFamily( rFontItem.GetFamily() );
            aFont.SetStyleName( rFontItem.GetStyleName() );
            aFont.SetOrientation( 0 );

            SvxPostureItem& rPostureItem = (SvxPostureItem&)pCustomShape->GetMergedItem( EE_CHAR_ITALIC );
            aFont.SetItalic( rPostureItem.GetPosture() );

            SvxWeightItem& rWeightItem = (SvxWeightItem&)pCustomShape->GetMergedItem( EE_CHAR_WEIGHT );
            aFont.SetWeight( rWeightItem.GetWeight() );

            // initializing virtual device
            VirtualDevice aVirDev( 1 );
            aVirDev.SetMapMode( MAP_100TH_MM );
            aVirDev.SetFont( aFont );
            aVirDev.EnableRTL( sal_True );
            if ( aParagraphIter->nFrameDirection == FRMDIR_HORI_RIGHT_TOP )
                aVirDev.SetLayoutMode( TEXT_LAYOUT_BIDI_RTL );

            SvxCharScaleWidthItem& rCharScaleWidthItem = (SvxCharScaleWidthItem&)pCustomShape->GetMergedItem( EE_CHAR_FONTWIDTH );
            sal_uInt16 nCharScaleWidth = rCharScaleWidthItem.GetValue();
            sal_Int32* pDXArry = NULL;
            sal_Int32 nWidth = 0;

            // VERTICAL
            if ( bIsVertical )
            {
                // vertical _> each single character needs to be rotated by 90
                sal_Int32 i;
                sal_Int32 nHeight = 0;
                Rectangle aSingleCharacterUnion;
                for ( i = 0; i < rText.getLength(); i++ )
                {
                    FWCharacterData aCharacterData;
                    rtl::OUString aCharText( (sal_Unicode)rText[ i ] );
                    if ( aVirDev.GetTextOutlines( aCharacterData.vOutlines, aCharText, 0, 0, STRING_LEN, sal_True, nWidth, pDXArry ) )
                    {
                        sal_Int32 nTextWidth = aVirDev.GetTextWidth( aCharText, 0, STRING_LEN );
                        std::vector< PolyPolygon >::iterator aOutlineIter = aCharacterData.vOutlines.begin();
                        std::vector< PolyPolygon >::iterator aOutlineIEnd = aCharacterData.vOutlines.end();
                        if ( aOutlineIter == aOutlineIEnd )
                        {
                            nHeight += rFWData.nSingleLineHeight;
                        }
                        else
                        {
                            while ( aOutlineIter != aOutlineIEnd )
                            {
                                // rotating
                                aOutlineIter->Rotate( Point( nTextWidth / 2, rFWData.nSingleLineHeight / 2 ), 900 );
                                aCharacterData.aBoundRect.Union( aOutlineIter->GetBoundRect() );
                                ++aOutlineIter;
                            }
                            aOutlineIter = aCharacterData.vOutlines.begin();
                            aOutlineIEnd = aCharacterData.vOutlines.end();
                            while ( aOutlineIter != aOutlineIEnd )
                            {
                                sal_Int32 nM = - aCharacterData.aBoundRect.Left() + nHeight;
                                aOutlineIter->Move( nM, 0 );
                                aCharacterData.aBoundRect.Move( nM, 0 );
                                ++aOutlineIter;
                            }
                            nHeight += aCharacterData.aBoundRect.GetWidth() + ( rFWData.nSingleLineHeight / 5 );
                            aSingleCharacterUnion.Union( aCharacterData.aBoundRect );
                        }
                    }
                    aParagraphIter->vCharacters.push_back( aCharacterData );
                }
                std::vector< FWCharacterData >::iterator aCharacterIter( aParagraphIter->vCharacters.begin() );
                std::vector< FWCharacterData >::iterator aCharacterIEnd( aParagraphIter->vCharacters.end() );
                while ( aCharacterIter != aCharacterIEnd )
                {
                    std::vector< PolyPolygon >::iterator aOutlineIter( aCharacterIter->vOutlines.begin() );
                    std::vector< PolyPolygon >::iterator aOutlineIEnd( aCharacterIter->vOutlines.end() );
                    while ( aOutlineIter != aOutlineIEnd )
                    {
                        aOutlineIter->Move( ( aSingleCharacterUnion.GetWidth() - aCharacterIter->aBoundRect.GetWidth() ) / 2, 0 );
                        ++aOutlineIter;
                    }
                    ++aCharacterIter;
                }
            }
            else
            {
                if ( ( nCharScaleWidth != 100 ) && nCharScaleWidth )
                {   // applying character spacing
                    pDXArry = new sal_Int32[ rText.getLength() ];
                    aVirDev.GetTextArray( rText, pDXArry, 0, STRING_LEN );
                    FontMetric aFontMetric( aVirDev.GetFontMetric() );
                    aFont.SetWidth( (sal_Int32)( (double)aFontMetric.GetWidth() * ( (double)100 / (double)nCharScaleWidth ) ) );
                    aVirDev.SetFont( aFont );
                }
                FWCharacterData aCharacterData;
                if ( aVirDev.GetTextOutlines( aCharacterData.vOutlines, rText, 0, 0, STRING_LEN, sal_True, nWidth, pDXArry ) )
                {
                    aParagraphIter->vCharacters.push_back( aCharacterData );
                }
            }
            delete[] pDXArry;

            // veritcal alignment
            std::vector< FWCharacterData >::iterator aCharacterIter( aParagraphIter->vCharacters.begin() );
            std::vector< FWCharacterData >::iterator aCharacterIEnd ( aParagraphIter->vCharacters.end() );
            while ( aCharacterIter != aCharacterIEnd )
            {
                std::vector< PolyPolygon >::iterator aOutlineIter( aCharacterIter->vOutlines.begin() );
                std::vector< PolyPolygon >::iterator aOutlineIEnd( aCharacterIter->vOutlines.end() );
                while( aOutlineIter != aOutlineIEnd )
                {

                    PolyPolygon& rPolyPoly = *aOutlineIter++;

                    if ( nVerticalOffset )
                        rPolyPoly.Move( 0, nVerticalOffset );

                    // retrieving the boundrect for the paragraph
                    Rectangle aBoundRect( rPolyPoly.GetBoundRect() );
                    aParagraphIter->aBoundRect.Union( aBoundRect );
                }
                ++aCharacterIter;
            }
        }
        // updating the boundrect for the text area by merging the current paragraph boundrect
        if ( aParagraphIter->aBoundRect.IsEmpty() )
        {
            if ( rTextArea.aBoundRect.IsEmpty() )
                rTextArea.aBoundRect = Rectangle( Point( 0, 0 ), Size( 1, rFWData.nSingleLineHeight ) );
            else
                rTextArea.aBoundRect.Bottom() += rFWData.nSingleLineHeight;
        }
        else
        {
            Rectangle& rParagraphBoundRect = aParagraphIter->aBoundRect;
            rTextArea.aBoundRect.Union( rParagraphBoundRect );

            if ( bSameLetterHeights )
            {
                std::vector< FWCharacterData >::iterator aCharacterIter( aParagraphIter->vCharacters.begin() );
                std::vector< FWCharacterData >::iterator aCharacterIEnd( aParagraphIter->vCharacters.end() );
                while ( aCharacterIter != aCharacterIEnd )
                {
                    std::vector< PolyPolygon >::iterator aOutlineIter( aCharacterIter->vOutlines.begin() );
                    std::vector< PolyPolygon >::iterator aOutlineIEnd( aCharacterIter->vOutlines.end() );
                    while( aOutlineIter != aOutlineIEnd )
                    {
                        Rectangle aPolyPolyBoundRect( aOutlineIter->GetBoundRect() );
                        if ( aPolyPolyBoundRect.GetHeight() != rParagraphBoundRect.GetHeight() )
                            aOutlineIter->Scale( 1.0, (double)rParagraphBoundRect.GetHeight() / aPolyPolyBoundRect.GetHeight() );
                        aPolyPolyBoundRect = aOutlineIter->GetBoundRect();
                        sal_Int32 nMove = aPolyPolyBoundRect.Top() - rParagraphBoundRect.Top();
                        if ( nMove )
                            aOutlineIter->Move( 0, -nMove );
                        ++aOutlineIter;
                    }
                    ++aCharacterIter;
                }
            }
        }
        if ( bIsVertical )
            nVerticalOffset -= rFWData.nSingleLineHeight;
        else
            nVerticalOffset += rFWData.nSingleLineHeight;
        ++aParagraphIter;
    }
}

void GetFontWorkOutline( FWData& rFWData, const SdrObject* pCustomShape )
{
    SdrTextHorzAdjust eHorzAdjust( ((SdrTextHorzAdjustItem&)pCustomShape->GetMergedItem( SDRATTR_TEXT_HORZADJUST )).GetValue() );
    SdrFitToSizeType  eFTS( ((SdrTextFitToSizeTypeItem&)pCustomShape->GetMergedItem( SDRATTR_TEXT_FITTOSIZE )).GetValue() );

    std::vector< FWTextArea >::iterator aTextAreaIter = rFWData.vTextAreas.begin();
    std::vector< FWTextArea >::iterator aTextAreaIEnd = rFWData.vTextAreas.end();

    rFWData.nSingleLineHeight = (sal_Int32)( ( (double)pCustomShape->GetLogicRect().GetHeight()
                                                / rFWData.nMaxParagraphsPerTextArea ) * rFWData.fHorizontalTextScaling );

    sal_Bool bSameLetterHeights = sal_False;
    SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)pCustomShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
    const rtl::OUString sTextPath( "TextPath"  );
    const rtl::OUString sSameLetterHeights( "SameLetterHeights"  );
    com::sun::star::uno::Any* pAny = rGeometryItem.GetPropertyValueByName( sTextPath, sSameLetterHeights );
    if ( pAny )
        *pAny >>= bSameLetterHeights;

    while ( aTextAreaIter != aTextAreaIEnd )
    {
        GetTextAreaOutline( rFWData, pCustomShape, *aTextAreaIter, bSameLetterHeights );
        if ( eFTS == SDRTEXTFIT_ALLLINES )
        {
            std::vector< FWParagraphData >::iterator aParagraphIter( aTextAreaIter->vParagraphs.begin() );
            std::vector< FWParagraphData >::iterator aParagraphIEnd( aTextAreaIter->vParagraphs.end() );
            while ( aParagraphIter != aParagraphIEnd )
            {
                sal_Int32 nParaWidth = aParagraphIter->aBoundRect.GetWidth();
                if ( nParaWidth )
                {
                    double fScale = (double)aTextAreaIter->aBoundRect.GetWidth() / nParaWidth;

                    std::vector< FWCharacterData >::iterator aCharacterIter( aParagraphIter->vCharacters.begin() );
                    std::vector< FWCharacterData >::iterator aCharacterIEnd( aParagraphIter->vCharacters.end() );
                    while ( aCharacterIter != aCharacterIEnd )
                    {
                        std::vector< PolyPolygon >::iterator aOutlineIter = aCharacterIter->vOutlines.begin();
                        std::vector< PolyPolygon >::iterator aOutlineIEnd = aCharacterIter->vOutlines.end();
                        while( aOutlineIter != aOutlineIEnd )
                        {
                            aOutlineIter->Scale( fScale, 1.0 );
                            ++aOutlineIter;
                        }
                        ++aCharacterIter;
                    }
                }
                ++aParagraphIter;
            }
        }
        else
        {
            switch( eHorzAdjust )
            {
                case SDRTEXTHORZADJUST_RIGHT :
                case SDRTEXTHORZADJUST_CENTER:
                {
                    std::vector< FWParagraphData >::iterator aParagraphIter( aTextAreaIter->vParagraphs.begin() );
                    std::vector< FWParagraphData >::iterator aParagraphIEnd( aTextAreaIter->vParagraphs.end() );
                    while ( aParagraphIter != aParagraphIEnd )
                    {
                        sal_Int32 nHorzDiff = 0;
                        if ( eHorzAdjust == SDRTEXTHORZADJUST_CENTER )
                            nHorzDiff = ( aTextAreaIter->aBoundRect.GetWidth() - aParagraphIter->aBoundRect.GetWidth() ) / 2;
                        else if ( eHorzAdjust == SDRTEXTHORZADJUST_RIGHT )
                            nHorzDiff = ( aTextAreaIter->aBoundRect.GetWidth() - aParagraphIter->aBoundRect.GetWidth() );
                        if ( nHorzDiff )
                        {
                            std::vector< FWCharacterData >::iterator aCharacterIter( aParagraphIter->vCharacters.begin() );
                            std::vector< FWCharacterData >::iterator aCharacterIEnd( aParagraphIter->vCharacters.end() );
                            while ( aCharacterIter != aCharacterIEnd )
                            {
                                std::vector< PolyPolygon >::iterator aOutlineIter = aCharacterIter->vOutlines.begin();
                                std::vector< PolyPolygon >::iterator aOutlineIEnd = aCharacterIter->vOutlines.end();
                                while( aOutlineIter != aOutlineIEnd )
                                {
                                    aOutlineIter->Move( nHorzDiff, 0 );
                                    ++aOutlineIter;
                                }
                                ++aCharacterIter;
                            }
                        }
                        ++aParagraphIter;
                    }
                }
                break;
                default:
                case SDRTEXTHORZADJUST_BLOCK : break;   // don't know
                case SDRTEXTHORZADJUST_LEFT : break;    // already left aligned -> nothing to do
            }
        }
        ++aTextAreaIter;
    }
}

basegfx::B2DPolyPolygon GetOutlinesFromShape2d( const SdrObject* pShape2d )
{
    basegfx::B2DPolyPolygon aOutlines2d;

    SdrObjListIter aObjListIter( *pShape2d, IM_DEEPWITHGROUPS );
    while( aObjListIter.IsMore() )
    {
        SdrObject* pPartObj = aObjListIter.Next();
        if ( pPartObj->ISA( SdrPathObj ) )
        {
            basegfx::B2DPolyPolygon aCandidate(((SdrPathObj*)pPartObj)->GetPathPoly());
            if(aCandidate.areControlPointsUsed())
            {
                aCandidate = basegfx::tools::adaptiveSubdivideByAngle(aCandidate);
            }
            aOutlines2d.append(aCandidate);
        }
    }

    return aOutlines2d;
}

void CalcDistances( const Polygon& rPoly, std::vector< double >& rDistances )
{
    sal_uInt16 i, nCount = rPoly.GetSize();
    if ( nCount > 1 )
    {
        for ( i = 0; i < nCount; i++ )
        {
            double fDistance = i ? ((Polygon&)rPoly).CalcDistance( i, i - 1 ) : 0.0;
            rDistances.push_back( fDistance );
        }
        std::partial_sum( rDistances.begin(), rDistances.end(), rDistances.begin() );
        double fLength = rDistances[ rDistances.size() - 1 ];
        if ( fLength > 0.0 )
        {
            std::vector< double >::iterator aIter = rDistances.begin();
            std::vector< double >::iterator aEnd = rDistances.end();
            while ( aIter != aEnd )
                *aIter++ /= fLength;
        }
    }
}

void InsertMissingOutlinePoints( const Polygon& /*rOutlinePoly*/, const std::vector< double >& rDistances, const Rectangle& rTextAreaBoundRect, Polygon& rPoly )
{
    sal_uInt16 i = 0;
    double fLastDistance = 0.0;
    for ( i = 0; i < rPoly.GetSize(); i++ )
    {
        Point& rPoint = rPoly[ i ];
        double fDistance = (double)( rPoint.X() - rTextAreaBoundRect.Left() ) / (double)rTextAreaBoundRect.GetWidth();
        if ( i )
        {
            if ( fDistance > fLastDistance )
            {
                std::vector< double >::const_iterator aIter = std::upper_bound( rDistances.begin(), rDistances.end(), fLastDistance );
                if  ( aIter != rDistances.end() && ( *aIter > fLastDistance ) && ( *aIter < fDistance ) )
                {
                    Point& rPt0 = rPoly[ i - 1 ];
                    sal_Int32 fX = rPoint.X() - rPt0.X();
                    sal_Int32 fY = rPoint.Y() - rPt0.Y();
                    double fd = ( 1.0 / ( fDistance - fLastDistance ) ) * ( *aIter - fLastDistance );
                    rPoly.Insert( i, Point( (sal_Int32)( rPt0.X() + fX * fd ), (sal_Int32)( rPt0.Y() + fY * fd ) ) );
                    fDistance = *aIter;
                }
            }
            else if ( fDistance < fLastDistance )
            {
                std::vector< double >::const_iterator aIter = std::lower_bound( rDistances.begin(), rDistances.end(), fLastDistance );
                if  ( aIter-- != rDistances.begin() )
                {
                    if ( ( *aIter > fDistance ) && ( *aIter < fLastDistance ) )
                    {
                        Point& rPt0 = rPoly[ i - 1 ];
                        sal_Int32 fX = rPoint.X() - rPt0.X();
                        sal_Int32 fY = rPoint.Y() - rPt0.Y();
                        double fd = ( 1.0 / ( fDistance - fLastDistance ) ) * ( *aIter - fLastDistance );
                        rPoly.Insert( i, Point( (sal_Int32)( rPt0.X() + fX * fd ), (sal_Int32)( rPt0.Y() + fY * fd ) ) );
                        fDistance = *aIter;
                    }
                }
            }
        }
        fLastDistance = fDistance;
    }
}

void GetPoint( const Polygon& rPoly, const std::vector< double >& rDistances, const double& fX, double& fx1, double& fy1 )
{
    fy1 = fx1 = 0.0;
    if ( rPoly.GetSize() > 1 )
    {
        std::vector< double >::const_iterator aIter = std::lower_bound( rDistances.begin(), rDistances.end(), fX );
        sal_uInt16 nIdx = sal::static_int_cast<sal_uInt16>( std::distance( rDistances.begin(), aIter ) );
        if ( aIter == rDistances.end() )
            nIdx--;
        const Point& rPt = rPoly[ nIdx ];
        fx1 = rPt.X();
        fy1 = rPt.Y();
        if ( nIdx && ( aIter != rDistances.end() ) && ( *aIter != fX ) )
        {
            nIdx = sal::static_int_cast<sal_uInt16>( std::distance( rDistances.begin(), aIter ) );
            double fDist0 = *( aIter - 1 );
            double fd = ( 1.0 / ( *aIter - fDist0 ) ) * ( fX - fDist0 );
            const Point& rPt2 = rPoly[ nIdx - 1 ];
            double fWidth = rPt.X() - rPt2.X();
            double fHeight= rPt.Y() - rPt2.Y();
            fWidth *= fd;
            fHeight*= fd;
            fx1 = rPt2.X() + fWidth;
            fy1 = rPt2.Y() + fHeight;
        }
    }
}

void FitTextOutlinesToShapeOutlines( const PolyPolygon& aOutlines2d, FWData& rFWData )
{
    std::vector< FWTextArea >::iterator aTextAreaIter = rFWData.vTextAreas.begin();
    std::vector< FWTextArea >::iterator aTextAreaIEnd = rFWData.vTextAreas.end();

    sal_uInt16 nOutline2dIdx = 0;
    while( aTextAreaIter != aTextAreaIEnd )
    {
        Rectangle rTextAreaBoundRect = aTextAreaIter->aBoundRect;
        sal_Int32 nLeft = rTextAreaBoundRect.Left();
        sal_Int32 nTop = rTextAreaBoundRect.Top();
        sal_Int32 nWidth = rTextAreaBoundRect.GetWidth();
        sal_Int32 nHeight= rTextAreaBoundRect.GetHeight();
        if ( rFWData.bSingleLineMode && nHeight && nWidth )
        {
            if ( nOutline2dIdx >= aOutlines2d.Count() )
                break;
            const Polygon& rOutlinePoly( aOutlines2d[ nOutline2dIdx++ ] );
            const sal_uInt16 nPointCount = rOutlinePoly.GetSize();
            if ( nPointCount > 1 )
            {
                std::vector< double > vDistances;
                vDistances.reserve( nPointCount );
                CalcDistances( rOutlinePoly, vDistances );
                if ( !vDistances.empty() )
                {
                    std::vector< FWParagraphData >::iterator aParagraphIter( aTextAreaIter->vParagraphs.begin() );
                    std::vector< FWParagraphData >::iterator aParagraphIEnd( aTextAreaIter->vParagraphs.end() );
                    while( aParagraphIter != aParagraphIEnd )
                    {
                        std::vector< FWCharacterData >::iterator aCharacterIter( aParagraphIter->vCharacters.begin() );
                        std::vector< FWCharacterData >::iterator aCharacterIEnd( aParagraphIter->vCharacters.end() );
                        while ( aCharacterIter != aCharacterIEnd )
                        {
                            std::vector< PolyPolygon >::iterator aOutlineIter = aCharacterIter->vOutlines.begin();
                            std::vector< PolyPolygon >::iterator aOutlineIEnd = aCharacterIter->vOutlines.end();
                            while( aOutlineIter != aOutlineIEnd )
                            {
                                PolyPolygon& rPolyPoly = *aOutlineIter;
                                Rectangle aBoundRect( rPolyPoly.GetBoundRect() );
                                double fx1 = aBoundRect.Left() - nLeft;
                                double fx2 = aBoundRect.Right() - nLeft;
                                double fy1, fy2;
                                double fM1 = fx1 / (double)nWidth;
                                double fM2 = fx2 / (double)nWidth;

                                GetPoint( rOutlinePoly, vDistances, fM1, fx1, fy1 );
                                GetPoint( rOutlinePoly, vDistances, fM2, fx2, fy2 );

                                double fvx = ( fy2 - fy1 );
                                double fvy = - ( fx2 - fx1 );
                                fx1 = fx1 + ( ( fx2 - fx1 ) * 0.5 );
                                fy1 = fy1 + ( ( fy2 - fy1 ) * 0.5 );

                                double fAngle = atan2( -fvx, -fvy );
                                double fL = hypot( fvx, fvy );
                                fvx = fvx / fL;
                                fvy = fvy / fL;
                                fL = (double)( aTextAreaIter->aBoundRect.GetHeight() / 2.0 + aTextAreaIter->aBoundRect.Top() ) - aParagraphIter->aBoundRect.Center().Y();
                                fvx *= fL;
                                fvy *= fL;
                                rPolyPoly.Rotate( Point( aBoundRect.Center().X(), aParagraphIter->aBoundRect.Center().Y() ), sin( fAngle ), cos( fAngle ) );
                                rPolyPoly.Move( (sal_Int32)( ( fx1 + fvx )- aBoundRect.Center().X() ), (sal_Int32)( ( fy1 + fvy ) - aParagraphIter->aBoundRect.Center().Y() ) );

                                ++aOutlineIter;
                            }
                            ++aCharacterIter;
                        }
                        ++aParagraphIter;
                    }
                }
            }
        }
        else
        {
            if ( ( nOutline2dIdx + 1 ) >= aOutlines2d.Count() )
                break;
            const Polygon& rOutlinePoly( aOutlines2d[ nOutline2dIdx++ ] );
            const Polygon& rOutlinePoly2( aOutlines2d[ nOutline2dIdx++ ] );
            const sal_uInt16 nPointCount = rOutlinePoly.GetSize();
            const sal_uInt16 nPointCount2 = rOutlinePoly2.GetSize();
            if ( ( nPointCount > 1 ) && ( nPointCount2 > 1 ) )
            {
                std::vector< double > vDistances;
                vDistances.reserve( nPointCount );
                std::vector< double > vDistances2;
                vDistances2.reserve( nPointCount2 );
                CalcDistances( rOutlinePoly, vDistances );
                CalcDistances( rOutlinePoly2, vDistances2 );
                std::vector< FWParagraphData >::iterator aParagraphIter = aTextAreaIter->vParagraphs.begin();
                std::vector< FWParagraphData >::iterator aParagraphIEnd = aTextAreaIter->vParagraphs.end();
                while( aParagraphIter != aParagraphIEnd )
                {
                    std::vector< FWCharacterData >::iterator aCharacterIter( aParagraphIter->vCharacters.begin() );
                    std::vector< FWCharacterData >::iterator aCharacterIEnd( aParagraphIter->vCharacters.end() );
                    while ( aCharacterIter != aCharacterIEnd )
                    {
                        std::vector< PolyPolygon >::iterator aOutlineIter = aCharacterIter->vOutlines.begin();
                        std::vector< PolyPolygon >::iterator aOutlineIEnd = aCharacterIter->vOutlines.end();
                        while( aOutlineIter != aOutlineIEnd )
                        {
                            PolyPolygon& rPolyPoly = *aOutlineIter;
                            sal_uInt16 i, nPolyCount = rPolyPoly.Count();
                            for ( i = 0; i < nPolyCount; i++ )
                            {
                                // #i35928#
                                basegfx::B2DPolygon aCandidate(rPolyPoly[ i ].getB2DPolygon());

                                if(aCandidate.areControlPointsUsed())
                                {
                                    aCandidate = basegfx::tools::adaptiveSubdivideByAngle(aCandidate);
                                }

                                // create local polygon copy to work on
                                 Polygon aLocalPoly(aCandidate);

                                InsertMissingOutlinePoints( rOutlinePoly, vDistances, rTextAreaBoundRect, aLocalPoly );
                                InsertMissingOutlinePoints( rOutlinePoly2, vDistances2, rTextAreaBoundRect, aLocalPoly );

                                sal_uInt16 j, _nPointCount = aLocalPoly.GetSize();
                                for ( j = 0; j < _nPointCount; j++ )
                                {
                                    Point& rPoint = aLocalPoly[ j ];
                                    rPoint.X() -= nLeft;
                                    rPoint.Y() -= nTop;
                                    double fX = (double)rPoint.X() / (double)nWidth;
                                    double fY = (double)rPoint.Y() / (double)nHeight;

                                    double fx1, fy1, fx2, fy2;
                                    GetPoint( rOutlinePoly, vDistances, fX, fx1, fy1 );
                                    GetPoint( rOutlinePoly2, vDistances2, fX, fx2, fy2 );
                                    double fWidth = fx2 - fx1;
                                    double fHeight= fy2 - fy1;
                                    rPoint.X() = (sal_Int32)( fx1 + fWidth * fY );
                                    rPoint.Y() = (sal_Int32)( fy1 + fHeight* fY );
                                }

                                // write back polygon
                                rPolyPoly[ i ] = aLocalPoly;
                            }
                            ++aOutlineIter;
                        }
                        ++aCharacterIter;
                    }
                    ++aParagraphIter;
                }
            }
        }
        ++aTextAreaIter;
    }
}

SdrObject* CreateSdrObjectFromParagraphOutlines( const FWData& rFWData, const SdrObject* pCustomShape )
{
    SdrObject* pRet = NULL;
    basegfx::B2DPolyPolygon aPolyPoly;
    if ( !rFWData.vTextAreas.empty() )
    {
        std::vector< FWTextArea >::const_iterator aTextAreaIter = rFWData.vTextAreas.begin();
        std::vector< FWTextArea >::const_iterator aTextAreaIEnd = rFWData.vTextAreas.end();
        while ( aTextAreaIter != aTextAreaIEnd )
        {
            std::vector< FWParagraphData >::const_iterator aParagraphIter = aTextAreaIter->vParagraphs.begin();
            std::vector< FWParagraphData >::const_iterator aParagraphIEnd = aTextAreaIter->vParagraphs.end();
            while ( aParagraphIter != aParagraphIEnd )
            {
                std::vector< FWCharacterData >::const_iterator aCharacterIter( aParagraphIter->vCharacters.begin() );
                std::vector< FWCharacterData >::const_iterator aCharacterIEnd( aParagraphIter->vCharacters.end() );
                while ( aCharacterIter != aCharacterIEnd )
                {
                    std::vector< PolyPolygon >::const_iterator aOutlineIter = aCharacterIter->vOutlines.begin();
                    std::vector< PolyPolygon >::const_iterator aOutlineIEnd = aCharacterIter->vOutlines.end();
                    while( aOutlineIter != aOutlineIEnd )
                    {
                        aPolyPoly.append( aOutlineIter->getB2DPolyPolygon() );
                        ++aOutlineIter;
                    }
                    ++aCharacterIter;
                }
                ++aParagraphIter;
            }
            ++aTextAreaIter;
        }

        pRet = new SdrPathObj( OBJ_POLY, aPolyPoly );

        Point aP( pCustomShape->GetSnapRect().Center() );
        Size aS( pCustomShape->GetLogicRect().GetSize() );
        aP.X() -= aS.Width() / 2;
        aP.Y() -= aS.Height() / 2;
        Rectangle aLogicRect( aP, aS );

        SfxItemSet aSet( pCustomShape->GetMergedItemSet() );
        aSet.ClearItem( SDRATTR_TEXTDIRECTION );    //SJ: vertical writing is not required, by removing this item no outliner is created
        aSet.Put(SdrShadowItem(sal_False)); // #i37011# NO shadow for FontWork geometry
        pRet->SetMergedItemSet( aSet );             // * otherwise we would crash, because the outliner tries to create a Paraobject, but there is no model
    }
    return pRet;
}

::com::sun::star::uno::Reference < ::com::sun::star::i18n::XBreakIterator > EnhancedCustomShapeFontWork::mxBreakIterator = 0;

Reference < i18n::XBreakIterator > EnhancedCustomShapeFontWork::GetBreakIterator()
{
    if ( !mxBreakIterator.is() )
    {
        Reference< lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
        Reference < XInterface > xI = xMSF->createInstance( rtl::OUString("com.sun.star.i18n.BreakIterator") );
        if ( xI.is() )
        {
            Any x = xI->queryInterface( ::getCppuType((const Reference< i18n::XBreakIterator >*)0) );
            x >>= mxBreakIterator;
        }
    }
    return mxBreakIterator;
}

SdrObject* EnhancedCustomShapeFontWork::CreateFontWork( const SdrObject* pShape2d, const SdrObject* pCustomShape )
{
    SdrObject* pRet = NULL;

    PolyPolygon aOutlines2d( GetOutlinesFromShape2d( pShape2d ) );
    sal_uInt16 nOutlinesCount2d = aOutlines2d.Count();
    if ( nOutlinesCount2d )
    {
        FWData aFWData;
        if ( InitializeFontWorkData( pCustomShape, nOutlinesCount2d, aFWData ) )
        {
            /* retrieves the horizontal scaling factor that has to be used
            to fit each paragraph text into its corresponding 2d outline */
            CalculateHorizontalScalingFactor( pCustomShape, aFWData, aOutlines2d );

            /* retrieving the Outlines for the each Paragraph. */

            GetFontWorkOutline( aFWData, pCustomShape );

            FitTextOutlinesToShapeOutlines( aOutlines2d, aFWData );

            pRet = CreateSdrObjectFromParagraphOutlines( aFWData, pCustomShape );
        }
    }
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
