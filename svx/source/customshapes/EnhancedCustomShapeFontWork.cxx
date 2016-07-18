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

#include "EnhancedCustomShapeFontWork.hxx"
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
#include <o3tl/numeric.hxx>
#include <svx/svdmodel.hxx>
#include <vector>
#include <numeric>
#include <algorithm>
#include <memory>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <basegfx/polygon/b2dpolygontools.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;

struct FWCharacterData                  // representing a single character
{
    std::vector< tools::PolyPolygon >          vOutlines;
    Rectangle                           aBoundRect;
};
struct FWParagraphData                  // representing a single paragraph
{
    OUString                       aString;
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
    bool                            bSingleLineMode;
};


static bool InitializeFontWorkData( const SdrObject* pCustomShape, const sal_uInt16 nOutlinesCount2d, FWData& rFWData )
{
    bool bNoErr = false;
    bool bSingleLineMode = false;
    sal_uInt16 nTextAreaCount = nOutlinesCount2d;
    if ( nOutlinesCount2d & 1 )
        bSingleLineMode = true;
    else
        nTextAreaCount >>= 1;

    if ( nTextAreaCount )
    {
        rFWData.bSingleLineMode = bSingleLineMode;

        // setting the strings
        OutlinerParaObject* pParaObj = static_cast<const SdrObjCustomShape*>(pCustomShape)->GetOutlinerParaObject();
        if ( pParaObj )
        {
            const EditTextObject& rTextObj = pParaObj->GetTextObject();
            sal_Int32 nParagraphsLeft = rTextObj.GetParagraphCount();

            rFWData.nMaxParagraphsPerTextArea = ( ( nParagraphsLeft - 1 ) / nTextAreaCount ) + 1;
            sal_Int32 j = 0;
            while( nParagraphsLeft && nTextAreaCount )
            {
                FWTextArea aTextArea;
                sal_Int32 i, nParagraphs = ( ( nParagraphsLeft - 1 ) / nTextAreaCount ) + 1;
                for ( i = 0; i < nParagraphs; ++i, ++j )
                {
                    FWParagraphData aParagraphData;
                    aParagraphData.aString = rTextObj.GetText( j );

                    const SfxItemSet& rParaSet = rTextObj.GetParaAttribs( j );  // retrieving some paragraph attributes
                    aParagraphData.nFrameDirection = static_cast<const SvxFrameDirectionItem&>(rParaSet.Get( EE_PARA_WRITINGDIR )).GetValue();
                    aTextArea.vParagraphs.push_back( aParagraphData );
                }
                rFWData.vTextAreas.push_back( aTextArea );
                nParagraphsLeft -= nParagraphs;
                nTextAreaCount--;
            }
            bNoErr = true;
        }
    }
    return bNoErr;
}

double GetLength( const tools::Polygon& rPolygon )
{
    double fLength = 0;
    if ( rPolygon.GetSize() > 1 )
    {
        sal_uInt16 nCount = rPolygon.GetSize();
        while( --nCount )
            fLength += ((tools::Polygon&)rPolygon).CalcDistance( nCount, nCount - 1 );
    }
    return fLength;
}


/* CalculateHorizontalScalingFactor returns the horizontal scaling factor for
the whole text object, so that each text will match its corresponding 2d Outline */
void CalculateHorizontalScalingFactor( const SdrObject* pCustomShape,
                                        FWData& rFWData, const tools::PolyPolygon& rOutline2d )
{
    double fScalingFactor = 1.0;
    bool bScalingFactorDefined = false;

    sal_uInt16 i = 0;
    bool bSingleLineMode = false;
    sal_uInt16 nOutlinesCount2d = rOutline2d.Count();

    vcl::Font aFont;
    const SvxFontItem& rFontItem = static_cast<const SvxFontItem&>(pCustomShape->GetMergedItem( EE_CHAR_FONTINFO ));
    aFont.SetFontHeight( pCustomShape->GetLogicRect().GetHeight() / rFWData.nMaxParagraphsPerTextArea );
    aFont.SetAlignment( ALIGN_TOP );
    aFont.SetFamilyName( rFontItem.GetFamilyName() );
    aFont.SetFamily( rFontItem.GetFamily() );
    aFont.SetStyleName( rFontItem.GetStyleName() );
    aFont.SetOrientation( 0 );
    // initializing virtual device

    ScopedVclPtrInstance< VirtualDevice > pVirDev(DeviceFormat::BITMASK);
    pVirDev->SetMapMode( MAP_100TH_MM );
    pVirDev->SetFont( aFont );

    if ( nOutlinesCount2d & 1 )
        bSingleLineMode = true;

    std::vector< FWTextArea >::iterator aTextAreaIter = rFWData.vTextAreas.begin();
    std::vector< FWTextArea >::const_iterator aTextAreaIEnd = rFWData.vTextAreas.end();
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
            double fTextWidth = pVirDev->GetTextWidth( aParagraphIter->aString );
            if ( fTextWidth > 0.0 )
            {
                double fScale = fWidth / fTextWidth;
                if ( !bScalingFactorDefined )
                {
                    fScalingFactor = fScale;
                    bScalingFactorDefined = true;
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

void GetTextAreaOutline( const FWData& rFWData, const SdrObject* pCustomShape, FWTextArea& rTextArea, bool bSameLetterHeights )
{
    bool bIsVertical = static_cast<const SdrObjCustomShape*>(pCustomShape)->IsVerticalWriting();
    sal_Int32 nVerticalOffset = rFWData.nMaxParagraphsPerTextArea > rTextArea.vParagraphs.size()
                                    ? rFWData.nSingleLineHeight / 2 : 0;

    std::vector< FWParagraphData >::iterator aParagraphIter( rTextArea.vParagraphs.begin() );
    std::vector< FWParagraphData >::const_iterator aParagraphIEnd( rTextArea.vParagraphs.end() );
    while( aParagraphIter != aParagraphIEnd )
    {
        const OUString& rText = aParagraphIter->aString;
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
                    sal_Int32 nChg = xBI->endOfScript( rText, 0, nScriptType );
                    if (nChg < rText.getLength() && nChg >= 0)
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
            const SvxFontItem& rFontItem = static_cast<const SvxFontItem&>(pCustomShape->GetMergedItem( nFntItm ));
            vcl::Font aFont;
            aFont.SetFontHeight( rFWData.nSingleLineHeight );
            aFont.SetAlignment( ALIGN_TOP );

            aFont.SetFamilyName( rFontItem.GetFamilyName() );
            aFont.SetFamily( rFontItem.GetFamily() );
            aFont.SetStyleName( rFontItem.GetStyleName() );
            aFont.SetOrientation( 0 );

            const SvxPostureItem& rPostureItem = static_cast<const SvxPostureItem&>(pCustomShape->GetMergedItem( EE_CHAR_ITALIC ));
            aFont.SetItalic( rPostureItem.GetPosture() );

            const SvxWeightItem& rWeightItem = static_cast<const SvxWeightItem&>(pCustomShape->GetMergedItem( EE_CHAR_WEIGHT ));
            aFont.SetWeight( rWeightItem.GetWeight() );

            // initializing virtual device
            ScopedVclPtrInstance< VirtualDevice > pVirDev(DeviceFormat::BITMASK);
            pVirDev->SetMapMode( MAP_100TH_MM );
            pVirDev->SetFont( aFont );
            pVirDev->EnableRTL();
            if ( aParagraphIter->nFrameDirection == FRMDIR_HORI_RIGHT_TOP )
                pVirDev->SetLayoutMode( ComplexTextLayoutFlags::BiDiRtl );

            const SvxCharScaleWidthItem& rCharScaleWidthItem = static_cast<const SvxCharScaleWidthItem&>(pCustomShape->GetMergedItem( EE_CHAR_FONTWIDTH ));
            sal_uInt16 nCharScaleWidth = rCharScaleWidthItem.GetValue();
            std::unique_ptr<long[]> pDXArry;
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
                    OUString aCharText( (sal_Unicode)rText[ i ] );
                    if ( pVirDev->GetTextOutlines( aCharacterData.vOutlines, aCharText, 0, 0, -1, nWidth, pDXArry.get() ) )
                    {
                        sal_Int32 nTextWidth = pVirDev->GetTextWidth( aCharText);
                        std::vector< tools::PolyPolygon >::iterator aOutlineIter = aCharacterData.vOutlines.begin();
                        std::vector< tools::PolyPolygon >::const_iterator aOutlineIEnd = aCharacterData.vOutlines.end();
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
                std::vector< FWCharacterData >::const_iterator aCharacterIEnd( aParagraphIter->vCharacters.end() );
                while ( aCharacterIter != aCharacterIEnd )
                {
                    std::vector< tools::PolyPolygon >::iterator aOutlineIter( aCharacterIter->vOutlines.begin() );
                    std::vector< tools::PolyPolygon >::const_iterator aOutlineIEnd( aCharacterIter->vOutlines.end() );
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
                    pDXArry.reset(new long[ rText.getLength() ]);
                    pVirDev->GetTextArray( rText, pDXArry.get());
                    FontMetric aFontMetric( pVirDev->GetFontMetric() );
                    aFont.SetAverageFontWidth( (sal_Int32)( (double)aFontMetric.GetAverageFontWidth() * ( (double)100 / (double)nCharScaleWidth ) ) );
                    pVirDev->SetFont( aFont );
                }
                FWCharacterData aCharacterData;
                if ( pVirDev->GetTextOutlines( aCharacterData.vOutlines, rText, 0, 0, -1, nWidth, pDXArry.get() ) )
                {
                    aParagraphIter->vCharacters.push_back( aCharacterData );
                }
            }

            // vertical alignment
            std::vector< FWCharacterData >::iterator aCharacterIter( aParagraphIter->vCharacters.begin() );
            std::vector< FWCharacterData >::const_iterator aCharacterIEnd ( aParagraphIter->vCharacters.end() );
            while ( aCharacterIter != aCharacterIEnd )
            {
                std::vector< tools::PolyPolygon >::iterator aOutlineIter( aCharacterIter->vOutlines.begin() );
                std::vector< tools::PolyPolygon >::const_iterator aOutlineIEnd( aCharacterIter->vOutlines.end() );
                while( aOutlineIter != aOutlineIEnd )
                {

                    tools::PolyPolygon& rPolyPoly = *aOutlineIter++;

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
                std::vector< FWCharacterData >::const_iterator aCharacterIEnd( aParagraphIter->vCharacters.end() );
                while ( aCharacterIter != aCharacterIEnd )
                {
                    std::vector< tools::PolyPolygon >::iterator aOutlineIter( aCharacterIter->vOutlines.begin() );
                    std::vector< tools::PolyPolygon >::const_iterator aOutlineIEnd( aCharacterIter->vOutlines.end() );
                    while( aOutlineIter != aOutlineIEnd )
                    {
                        Rectangle aPolyPolyBoundRect( aOutlineIter->GetBoundRect() );
                        if (aPolyPolyBoundRect.GetHeight() != rParagraphBoundRect.GetHeight() && aPolyPolyBoundRect.GetHeight())
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
    SdrTextHorzAdjust eHorzAdjust( static_cast<const SdrTextHorzAdjustItem&>(pCustomShape->GetMergedItem( SDRATTR_TEXT_HORZADJUST )).GetValue() );
    SdrFitToSizeType  eFTS( static_cast<const SdrTextFitToSizeTypeItem&>(pCustomShape->GetMergedItem( SDRATTR_TEXT_FITTOSIZE )).GetValue() );

    std::vector< FWTextArea >::iterator aTextAreaIter = rFWData.vTextAreas.begin();
    std::vector< FWTextArea >::const_iterator aTextAreaIEnd = rFWData.vTextAreas.end();

    rFWData.nSingleLineHeight = (sal_Int32)( ( (double)pCustomShape->GetLogicRect().GetHeight()
                                                / rFWData.nMaxParagraphsPerTextArea ) * rFWData.fHorizontalTextScaling );

    bool bSameLetterHeights = false;
    const SdrCustomShapeGeometryItem& rGeometryItem = static_cast<const SdrCustomShapeGeometryItem&>(pCustomShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ));
    const css::uno::Any* pAny = rGeometryItem.GetPropertyValueByName( "TextPath", "SameLetterHeights" );
    if ( pAny )
        *pAny >>= bSameLetterHeights;

    while ( aTextAreaIter != aTextAreaIEnd )
    {
        GetTextAreaOutline( rFWData, pCustomShape, *aTextAreaIter, bSameLetterHeights );
        if ( eFTS == SDRTEXTFIT_ALLLINES )
        {
            std::vector< FWParagraphData >::iterator aParagraphIter( aTextAreaIter->vParagraphs.begin() );
            std::vector< FWParagraphData >::const_iterator aParagraphIEnd( aTextAreaIter->vParagraphs.end() );
            while ( aParagraphIter != aParagraphIEnd )
            {
                sal_Int32 nParaWidth = aParagraphIter->aBoundRect.GetWidth();
                if ( nParaWidth )
                {
                    double fScale = (double)aTextAreaIter->aBoundRect.GetWidth() / nParaWidth;

                    std::vector< FWCharacterData >::iterator aCharacterIter( aParagraphIter->vCharacters.begin() );
                    std::vector< FWCharacterData >::const_iterator aCharacterIEnd( aParagraphIter->vCharacters.end() );
                    while ( aCharacterIter != aCharacterIEnd )
                    {
                        std::vector< tools::PolyPolygon >::iterator aOutlineIter = aCharacterIter->vOutlines.begin();
                        std::vector< tools::PolyPolygon >::const_iterator aOutlineIEnd = aCharacterIter->vOutlines.end();
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
                    std::vector< FWParagraphData >::const_iterator aParagraphIEnd( aTextAreaIter->vParagraphs.end() );
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
                            std::vector< FWCharacterData >::const_iterator aCharacterIEnd( aParagraphIter->vCharacters.end() );
                            while ( aCharacterIter != aCharacterIEnd )
                            {
                                std::vector< tools::PolyPolygon >::iterator aOutlineIter = aCharacterIter->vOutlines.begin();
                                std::vector< tools::PolyPolygon >::const_iterator aOutlineIEnd = aCharacterIter->vOutlines.end();
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
        if ( dynamic_cast<const SdrPathObj*>( pPartObj) !=  nullptr )
        {
            basegfx::B2DPolyPolygon aCandidate(static_cast<SdrPathObj*>(pPartObj)->GetPathPoly());
            if(aCandidate.areControlPointsUsed())
            {
                aCandidate = basegfx::tools::adaptiveSubdivideByAngle(aCandidate);
            }
            aOutlines2d.append(aCandidate);
        }
    }

    return aOutlines2d;
}

void CalcDistances( const tools::Polygon& rPoly, std::vector< double >& rDistances )
{
    sal_uInt16 i, nCount = rPoly.GetSize();
    if ( nCount > 1 )
    {
        for ( i = 0; i < nCount; i++ )
        {
            double fDistance = i ? ((tools::Polygon&)rPoly).CalcDistance( i, i - 1 ) : 0.0;
            rDistances.push_back( fDistance );
        }
        std::partial_sum( rDistances.begin(), rDistances.end(), rDistances.begin() );
        double fLength = rDistances[ rDistances.size() - 1 ];
        if ( fLength > 0.0 )
        {
            std::vector< double >::iterator aIter = rDistances.begin();
            std::vector< double >::const_iterator aEnd = rDistances.end();
            while ( aIter != aEnd )
                *aIter++ /= fLength;
        }
    }
}

void InsertMissingOutlinePoints( const tools::Polygon& /*rOutlinePoly*/, const std::vector< double >& rDistances,
                                 const Rectangle& rTextAreaBoundRect, tools::Polygon& rPoly )
{
    sal_uInt16 nSize = rPoly.GetSize();
    if (nSize == 0)
        return;

    long nTextWidth = rTextAreaBoundRect.GetWidth();

    if (nTextWidth == 0)
        throw o3tl::divide_by_zero();

    double fLastDistance = 0.0;
    for (sal_uInt16 i = 0; i < nSize; ++i)
    {
        Point& rPoint = rPoly[ i ];
        double fDistance = (double)( rPoint.X() - rTextAreaBoundRect.Left() ) / (double)nTextWidth;
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
                if  ( aIter != rDistances.begin() )
                {
                    --aIter;
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

void GetPoint( const tools::Polygon& rPoly, const std::vector< double >& rDistances, const double& fX, double& fx1, double& fy1 )
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
        if ( nIdx && ( aIter != rDistances.end() ) && !rtl::math::approxEqual( *aIter, fX ) )
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

void FitTextOutlinesToShapeOutlines( const tools::PolyPolygon& aOutlines2d, FWData& rFWData )
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
            const tools::Polygon& rOutlinePoly( aOutlines2d[ nOutline2dIdx++ ] );
            const sal_uInt16 nPointCount = rOutlinePoly.GetSize();
            if ( nPointCount > 1 )
            {
                std::vector< double > vDistances;
                vDistances.reserve( nPointCount );
                CalcDistances( rOutlinePoly, vDistances );
                if ( !vDistances.empty() )
                {
                    std::vector< FWParagraphData >::iterator aParagraphIter( aTextAreaIter->vParagraphs.begin() );
                    std::vector< FWParagraphData >::const_iterator aParagraphIEnd( aTextAreaIter->vParagraphs.end() );
                    while( aParagraphIter != aParagraphIEnd )
                    {
                        std::vector< FWCharacterData >::iterator aCharacterIter( aParagraphIter->vCharacters.begin() );
                        std::vector< FWCharacterData >::const_iterator aCharacterIEnd( aParagraphIter->vCharacters.end() );
                        while ( aCharacterIter != aCharacterIEnd )
                        {
                            std::vector< tools::PolyPolygon >::iterator aOutlineIter = aCharacterIter->vOutlines.begin();
                            std::vector< tools::PolyPolygon >::const_iterator aOutlineIEnd = aCharacterIter->vOutlines.end();
                            while( aOutlineIter != aOutlineIEnd )
                            {
                                tools::PolyPolygon& rPolyPoly = *aOutlineIter;
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
            const tools::Polygon& rOutlinePoly( aOutlines2d[ nOutline2dIdx++ ] );
            const tools::Polygon& rOutlinePoly2( aOutlines2d[ nOutline2dIdx++ ] );
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
                std::vector< FWParagraphData >::const_iterator aParagraphIEnd = aTextAreaIter->vParagraphs.end();
                while( aParagraphIter != aParagraphIEnd )
                {
                    std::vector< FWCharacterData >::iterator aCharacterIter( aParagraphIter->vCharacters.begin() );
                    std::vector< FWCharacterData >::const_iterator aCharacterIEnd( aParagraphIter->vCharacters.end() );
                    while ( aCharacterIter != aCharacterIEnd )
                    {
                        std::vector< tools::PolyPolygon >::iterator aOutlineIter = aCharacterIter->vOutlines.begin();
                        std::vector< tools::PolyPolygon >::const_iterator aOutlineIEnd = aCharacterIter->vOutlines.end();
                        while( aOutlineIter != aOutlineIEnd )
                        {
                            tools::PolyPolygon& rPolyPoly = *aOutlineIter;
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
                                tools::Polygon aLocalPoly(aCandidate);

                                InsertMissingOutlinePoints( rOutlinePoly, vDistances, rTextAreaBoundRect, aLocalPoly );
                                InsertMissingOutlinePoints( rOutlinePoly2, vDistances2, rTextAreaBoundRect, aLocalPoly );

                                sal_uInt16 _nPointCount = aLocalPoly.GetSize();
                                if (_nPointCount)
                                {
                                    if (!nWidth || !nHeight)
                                        throw o3tl::divide_by_zero();
                                    for (sal_uInt16 j = 0; j < _nPointCount; ++j)
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
    SdrObject* pRet = nullptr;
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
                    std::vector< tools::PolyPolygon >::const_iterator aOutlineIter = aCharacterIter->vOutlines.begin();
                    std::vector< tools::PolyPolygon >::const_iterator aOutlineIEnd = aCharacterIter->vOutlines.end();
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

        SfxItemSet aSet( pCustomShape->GetMergedItemSet() );
        aSet.ClearItem( SDRATTR_TEXTDIRECTION );    //SJ: vertical writing is not required, by removing this item no outliner is created
        aSet.Put(makeSdrShadowItem(false)); // #i37011# NO shadow for FontWork geometry
        pRet->SetMergedItemSet( aSet );             // * otherwise we would crash, because the outliner tries to create a Paraobject, but there is no model
    }
    return pRet;
}

Reference < i18n::XBreakIterator > EnhancedCustomShapeFontWork::mxBreakIterator = nullptr;

Reference < i18n::XBreakIterator > const & EnhancedCustomShapeFontWork::GetBreakIterator()
{
    if ( !mxBreakIterator.is() )
    {
        Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        mxBreakIterator = i18n::BreakIterator::create(xContext);
    }
    return mxBreakIterator;
}

SdrObject* EnhancedCustomShapeFontWork::CreateFontWork( const SdrObject* pShape2d, const SdrObject* pCustomShape )
{
    SdrObject* pRet = nullptr;

    tools::PolyPolygon aOutlines2d( GetOutlinesFromShape2d( pShape2d ) );
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
