/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EnhancedCustomShapeFontWork.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 13:49:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _ENHANCEDCUSTOMSHAPEFONTWORK_HXX
#include "EnhancedCustomShapeFontWork.hxx"
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>               // UINTXX
#endif
#ifndef _XOUTX_HXX
#include "xoutx.hxx"
#endif
#ifndef _SVDDEF_HXX
#include "svddef.hxx"
#endif
#ifndef _SVDOGRP_HXX
#include <svdogrp.hxx>
#endif
#ifndef _SVDOPATH_HXX
#include <svdopath.hxx>
#endif
#ifndef _SV_METRIC_HXX
#include <vcl/metric.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svdpage.hxx>
#endif
#ifndef _SDASITM_HXX
#include <sdasitm.hxx>
#endif
#ifndef _SDASAITM_HXX
#include <sdasaitm.hxx>
#endif
#ifndef _SDTFSITM_HXX
#include <sdtfsitm.hxx>
#endif
#ifndef _SV_VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif
#ifndef _SVDITER_HXX
#include <svditer.hxx>
#endif
#ifndef _SV_METRIC_HXX
#include <vcl/metric.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <eeitem.hxx>
#endif
#define ITEMID_FONT         EE_CHAR_FONTINFO
#define ITEMID_CHARSCALE_W  EE_CHAR_FONTWIDTH
#define ITEMID_FRAMEDIR     EE_PARA_WRITINGDIR
#define ITEMID_POSTURE      EE_CHAR_ITALIC
#define ITEMID_WEIGHT       EE_CHAR_WEIGHT
#ifndef _SVX_FRMDIRITEM_HXX
#include <frmdiritem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <fontitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX
#include <postitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX
#include <wghtitem.hxx>
#endif
#ifndef _SVX_CHARSCALEITEM_HXX
#include <charscaleitem.hxx>
#endif
#ifndef _ENHANCED_CUSTOMSHAPE_TYPE_NAMES_HXX
#include "EnhancedCustomShapeTypeNames.hxx"
#endif
#include "svdorect.hxx"
#include "svdoashp.hxx"
#include "outliner.hxx"
#include "outlobj.hxx"
#include "editobj.hxx"
#include "editeng.hxx"
#include "svdmodel.hxx"
#include <vector>
#include <numeric>
#include <algorithm>

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#ifndef _COM_SUN_STAR_I18N_XBREAKITERATOR_HPP_
#include <com/sun/star/i18n/XBreakIterator.hpp>
#endif
#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_CHARACTERITERATORMODE_HDL_
#include <com/sun/star/i18n/CharacterIteratorMode.hdl>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

using namespace com::sun::star;
using namespace com::sun::star::uno;

typedef std::vector< std::vector< double > > PolyPolygonDistances;

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
            sal_Int16 nParagraphsLeft = rTextObj.GetParagraphCount();

            rFWData.nMaxParagraphsPerTextArea = ( ( nParagraphsLeft - 1 ) / nTextAreaCount ) + 1;
            sal_Int16 j = 0;
            while( nParagraphsLeft && nTextAreaCount )
            {
                FWTextArea aTextArea;
                sal_Int16 i, nParagraphs = ( ( nParagraphsLeft - 1 ) / nTextAreaCount ) + 1;
                for ( i = 0; i < nParagraphs; i++, j++ )
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
            aParagraphIter++;
        }
        aTextAreaIter++;
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
        if ( rText.getLength() )
        {
            // generating vcl/font
            SvxFontItem& rFontItem = (SvxFontItem&)pCustomShape->GetMergedItem( EE_CHAR_FONTINFO );
            Font aFont;
            aFont.SetHeight( rFWData.nSingleLineHeight );
            aFont.SetAlign( ALIGN_TOP );
    //      aFont.SetAlign( )

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
            long* pDXArry = NULL;
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
                    if ( aVirDev.GetTextOutlines( aCharacterData.vOutlines, aCharText, 0, 0, STRING_LEN, TRUE, nWidth, pDXArry ) )
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
                                aOutlineIter++;
                            }
                            aOutlineIter = aCharacterData.vOutlines.begin();
                            aOutlineIEnd = aCharacterData.vOutlines.end();
                            while ( aOutlineIter != aOutlineIEnd )
                            {
                                sal_Int32 nM = - aCharacterData.aBoundRect.Left() + nHeight;
                                aOutlineIter->Move( nM, 0 );
                                aCharacterData.aBoundRect.Move( nM, 0 );
                                aOutlineIter++;
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
                        aOutlineIter++;
                    }
                    aCharacterIter++;
                }
            }
            else
            {
                if ( ( nCharScaleWidth != 100 ) && nCharScaleWidth )
                {   // applying character spacing
                    pDXArry = new long[ rText.getLength() ];
                    aVirDev.GetTextArray( rText, pDXArry, 0, STRING_LEN );
                    FontMetric aFontMetric( aVirDev.GetFontMetric() );
                    aFont.SetWidth( (sal_Int32)( (double)aFontMetric.GetWidth() * ( (double)100 / (double)nCharScaleWidth ) ) );
                    aVirDev.SetFont( aFont );
                }
                FWCharacterData aCharacterData;
                if ( aVirDev.GetTextOutlines( aCharacterData.vOutlines, rText, 0, 0, STRING_LEN, TRUE, nWidth, pDXArry ) )
                {
                    aParagraphIter->vCharacters.push_back( aCharacterData );
                }

/* trying to retrieve each single character _> is not working well
                sal_Int32 i;
                for ( i = 0; i < rText.getLength(); i++ )
                {
                    FWCharacterData aCharacterData;
                    if ( aVirDev.GetTextOutlines( aCharacterData.vOutlines, rText, 0, i, 1, TRUE, nWidth, pDXArry ) )
                    {
                        std::vector< PolyPolygon >::iterator aOutlineIter = aCharacterData.vOutlines.begin();
                        std::vector< PolyPolygon >::iterator aOutlineIEnd  = aCharacterData.vOutlines.end();
                        while ( aOutlineIter != aOutlineIEnd )
                        {
                            aCharacterData.aBoundRect.Union( aOutlineIter->GetBoundRect() );
                            aOutlineIter++;
                        }
                    }
                    aParagraphIter->vCharacters.push_back( aCharacterData );
                }
*/
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
                aCharacterIter++;
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
                        aOutlineIter++;
                    }
                    aCharacterIter++;
                }
            }
        }
        if ( bIsVertical )
            nVerticalOffset -= rFWData.nSingleLineHeight;
        else
            nVerticalOffset += rFWData.nSingleLineHeight;
        aParagraphIter++;
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
    const rtl::OUString sTextPath( RTL_CONSTASCII_USTRINGPARAM ( "TextPath" ) );
    const rtl::OUString sSameLetterHeights( RTL_CONSTASCII_USTRINGPARAM ( "SameLetterHeights" ) );
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
                            aOutlineIter++;
                        }
                        aCharacterIter++;
                    }
                }
                aParagraphIter++;
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
                                    aOutlineIter++;
                                }
                                aCharacterIter++;
                            }
                        }
                        aParagraphIter++;
                    }
                }
                break;
                default:
                case SDRTEXTHORZADJUST_BLOCK : break;   // don't know
                case SDRTEXTHORZADJUST_LEFT : break;    // already left aligned -> nothing to do
            }
        }
        aTextAreaIter++;
    }
}

//BFS09PolyPolygon GetOutlinesFromShape2d( const SdrObject* pShape2d )
::basegfx::B2DPolyPolygon GetOutlinesFromShape2d( const SdrObject* pShape2d )
{
//BFS09 PolyPolygon aOutlines2d;
    ::basegfx::B2DPolyPolygon aOutlines2d;

    SdrObjListIter aObjListIter( *pShape2d, IM_DEEPWITHGROUPS );
    while( aObjListIter.IsMore() )
    {
        SdrObject* pPartObj = aObjListIter.Next();
        if ( pPartObj->ISA( SdrPathObj ) )
        {
            const XPolyPolygon& rXPolyPoly = ((SdrPathObj*)pPartObj)->GetPathPoly();
            ::basegfx::B2DPolyPolygon aCandidate(rXPolyPoly.getB2DPolyPolygon());
            if(aCandidate.areControlPointsUsed())
            {
                aCandidate = ::basegfx::tools::adaptiveSubdivideByAngle(aCandidate);
            }
            aOutlines2d.append(aCandidate);

//BFS09         sal_uInt16 i, nCount = rXPolyPoly.Count();
//BFS09         for ( i = 0; i < nCount; i++ )
//BFS09         {
//BFS09//BFS09      Polygon aPoly( XOutCreatePolygonBezier( rXPolyPoly.GetObject( i ), NULL ) );
//BFS09             Polygon aPoly( XOutCreatePolygonBezier( rXPolyPoly.GetObject( i )) );
//BFS09             Polygon aSimplePoly;
//BFS09             aPoly.GetSimple( aSimplePoly );
//BFS09             aOutlines2d.Insert( aSimplePoly, POLYPOLY_APPEND );
//BFS09         }
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

void InsertMissingOutlinePoints( const Polygon& rOutlinePoly, const std::vector< double >& rDistances, const Rectangle& rTextAreaBoundRect, Polygon& rPoly )
{
    sal_uInt16 i = 0;
    double fLastDistance;
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
    if ( rPoly.GetSize() )
    {
        std::vector< double >::const_iterator aIter = std::lower_bound( rDistances.begin(), rDistances.end(), fX );
        sal_uInt16 nIdx = std::distance( rDistances.begin(), aIter );
        if ( aIter == rDistances.end() )
            nIdx--;
        const Point& rPt = rPoly[ nIdx ];
        fx1 = rPt.X();
        fy1 = rPt.Y();
        if ( nIdx && ( aIter != rDistances.end() ) && ( *aIter != fX ) )
        {
            nIdx = std::distance( rDistances.begin(), aIter );
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
            if ( nPointCount )
            {
                std::vector< double > vDistances;
                vDistances.reserve( nPointCount );
                CalcDistances( rOutlinePoly, vDistances );
                if ( vDistances.size() )
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

                                aOutlineIter++;
                            }
                            aCharacterIter++;
                        }
                        aParagraphIter++;
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
            if ( nPointCount && nPointCount2 )
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
                                ::basegfx::B2DPolygon aCandidate(rPolyPoly[ i ].getB2DPolygon());

                                if(aCandidate.areControlVectorsUsed())
                                {
                                    aCandidate = ::basegfx::tools::adaptiveSubdivideByAngle(aCandidate);
                                }

                                // create local polygon copy to work on
                                 Polygon aLocalPoly(aCandidate);

                                InsertMissingOutlinePoints( rOutlinePoly, vDistances, rTextAreaBoundRect, aLocalPoly );
                                InsertMissingOutlinePoints( rOutlinePoly2, vDistances2, rTextAreaBoundRect, aLocalPoly );

//BFS09                                 Polygon& rPoly = rPolyPoly[ i ];
//BFS09                             InsertMissingOutlinePoints( rOutlinePoly, vDistances, rTextAreaBoundRect, rPoly );
//BFS09                             InsertMissingOutlinePoints( rOutlinePoly2, vDistances2, rTextAreaBoundRect, rPoly );

                                sal_uInt16 j, nPointCount = aLocalPoly.GetSize();
                                for ( j = 0; j < nPointCount; j++ )
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
                            aOutlineIter++;
                        }
                        aCharacterIter++;
                    }
                    aParagraphIter++;
                }
            }
        }
        aTextAreaIter++;
    }
}

SdrObject* CreateSdrObjectFromParagraphOutlines( const FWData& rFWData, const SdrObject* pCustomShape )
{
    SdrObject* pRet = NULL;
    if ( rFWData.vTextAreas.size() )
    {
        pRet = new SdrObjGroup();
// SJ: not setting model, so we save a lot of broadcasting and the model is not modified any longer
//      pRet->SetModel( pCustomShape->GetModel() );
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
                        SdrObject* pPathObj = new SdrPathObj( OBJ_POLY, *aOutlineIter );
    // SJ: not setting model, so we save a lot of broadcasting and the model is not modified any longer
    //                  pPathObj->SetModel( pCustomShape->GetModel() );
                        ((SdrObjGroup*)pRet)->GetSubList()->NbcInsertObject( pPathObj );
                        aOutlineIter++;
                    }
                    aCharacterIter++;
                }
                aParagraphIter++;
            }
            aTextAreaIter++;
        }

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

SdrObject* EnhancedCustomShapeFontWork::CreateFontWork( const SdrObject* pShape2d, const SdrObject* pCustomShape )
{
    SdrObject* pRet = NULL;

    Rectangle aLogicRect( pCustomShape->GetLogicRect() );
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
