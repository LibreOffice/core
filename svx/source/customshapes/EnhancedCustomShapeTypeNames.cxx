/*************************************************************************
 *
 *  $RCSfile: EnhancedCustomShapeTypeNames.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-04-02 14:05:15 $
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

#ifndef _ENHANCED_CUSTOMSHAPE_TYPE_NAMES_HXX
#include "EnhancedCustomShapeTypeNames.hxx"
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#include <hash_map>

using namespace ::std;

struct TCheck
{
    bool operator()( const char* s1, const char* s2 ) const
    {
        return strcmp( s1, s2 ) == 0;
    }
};
typedef hash_map< const char*, MSO_SPT, std::hash<const char*>, TCheck> TypeNameHashMap;
static TypeNameHashMap* pHashMap = NULL;
static ::osl::Mutex& getHashMapMutex()
{
    static osl::Mutex s_aHashMapProtection;
    return s_aHashMapProtection;
}

struct NameTypeTable
{
    char*       pS;
    MSO_SPT     pE;
};
static const NameTypeTable pNameTypeTableArray[] =
{
    { "NonPrimitive", mso_sptMin },
    { "Rectangle", mso_sptRectangle },
    { "RoundRectangle", mso_sptRoundRectangle },
    { "Ellipse", mso_sptEllipse },
    { "Diamond", mso_sptDiamond },
    { "IsocelesTriangle", mso_sptIsocelesTriangle },
    { "RightTriangle", mso_sptRightTriangle },
    { "Parallelogram", mso_sptParallelogram },
    { "Trapezoid", mso_sptTrapezoid },
    { "Hexagon", mso_sptHexagon },
    { "Octagon", mso_sptOctagon },
    { "Plus", mso_sptPlus },
    { "Star", mso_sptStar },
    { "Arrow", mso_sptArrow },
    { "ThickArrow", mso_sptThickArrow },
    { "HomePlate", mso_sptHomePlate },
    { "Cube", mso_sptCube },
    { "Balloon", mso_sptBalloon },
    { "Seal", mso_sptSeal },
    { "Arc", mso_sptArc },
    { "Line", mso_sptLine },
    { "Plaque", mso_sptPlaque },
    { "Can", mso_sptCan },
    { "Donut", mso_sptDonut },
    { "TextSimple", mso_sptTextSimple },
    { "TextOctagon", mso_sptTextOctagon },
    { "TextHexagon", mso_sptTextHexagon },
    { "TextCurve", mso_sptTextCurve },
    { "TextWave", mso_sptTextWave },
    { "TextRing", mso_sptTextRing },
    { "TextOnCurve", mso_sptTextOnCurve },
    { "TextOnRing", mso_sptTextOnRing },
    { "StraightConnector1", mso_sptStraightConnector1 },
    { "BentConnector2", mso_sptBentConnector2 },
    { "BentConnector3", mso_sptBentConnector3 },
    { "BentConnector4", mso_sptBentConnector4 },
    { "BentConnector5", mso_sptBentConnector5 },
    { "CurvedConnector2", mso_sptCurvedConnector2 },
    { "CurvedConnector3", mso_sptCurvedConnector3 },
    { "CurvedConnector4", mso_sptCurvedConnector4 },
    { "CurvedConnector5", mso_sptCurvedConnector5 },
    { "Callout1", mso_sptCallout1 },
    { "Callout2", mso_sptCallout2 },
    { "Callout3", mso_sptCallout3 },
    { "AccentCallout1", mso_sptAccentCallout1 },
    { "AccentCallout2", mso_sptAccentCallout2 },
    { "AccentCallout3", mso_sptAccentCallout3 },
    { "BorderCallout1", mso_sptBorderCallout1 },
    { "BorderCallout2", mso_sptBorderCallout2 },
    { "BorderCallout3", mso_sptBorderCallout3 },
    { "AccentBorderCallout1", mso_sptAccentBorderCallout1 },
    { "AccentBorderCallout2", mso_sptAccentBorderCallout2 },
    { "AccentBorderCallout3", mso_sptAccentBorderCallout3 },
    { "Ribbon", mso_sptRibbon },
    { "Ribbon2", mso_sptRibbon2 },
    { "Chevron", mso_sptChevron },
    { "Pentagon", mso_sptPentagon },
    { "NoSmoking", mso_sptNoSmoking },
    { "Seal8", mso_sptSeal8 },
    { "Seal16", mso_sptSeal16 },
    { "Seal32", mso_sptSeal32 },
    { "WedgeRectCallout", mso_sptWedgeRectCallout },
    { "WedgeRRectCallout", mso_sptWedgeRRectCallout },
    { "WedgeEllipseCallout", mso_sptWedgeEllipseCallout },
    { "Wave", mso_sptWave },
    { "FoldedCorner", mso_sptFoldedCorner },
    { "LeftArrow", mso_sptLeftArrow },
    { "DownArrow", mso_sptDownArrow },
    { "UpArrow", mso_sptUpArrow },
    { "LeftRightArrow", mso_sptLeftRightArrow },
    { "UpDownArrow", mso_sptUpDownArrow },
    { "IrregularSeal1", mso_sptIrregularSeal1 },
    { "IrregularSeal2", mso_sptIrregularSeal2 },
    { "LightningBolt", mso_sptLightningBolt },
    { "Heart", mso_sptHeart },
    { "PictureFrame", mso_sptPictureFrame },
    { "QuadArrow", mso_sptQuadArrow },
    { "LeftArrowCallout", mso_sptLeftArrowCallout },
    { "RightArrowCallout", mso_sptRightArrowCallout },
    { "UpArrowCallout", mso_sptUpArrowCallout },
    { "DownArrowCallout", mso_sptDownArrowCallout },
    { "LeftRightArrowCallout", mso_sptLeftRightArrowCallout },
    { "UpDownArrowCallout", mso_sptUpDownArrowCallout },
    { "QuadArrowCallout", mso_sptQuadArrowCallout },
    { "Bevel", mso_sptBevel },
    { "LeftBracket", mso_sptLeftBracket },
    { "RightBracket", mso_sptRightBracket },
    { "LeftBrace", mso_sptLeftBrace },
    { "RightBrace", mso_sptRightBrace },
    { "LeftUpArrow", mso_sptLeftUpArrow },
    { "BentUpArrow", mso_sptBentUpArrow },
    { "BentArrow", mso_sptBentArrow },
    { "Seal24", mso_sptSeal24 },
    { "StripedRightArrow", mso_sptStripedRightArrow },
    { "NotchedRightArrow", mso_sptNotchedRightArrow },
    { "BlockArc", mso_sptBlockArc },
    { "SmileyFace", mso_sptSmileyFace },
    { "VerticalScroll", mso_sptVerticalScroll },
    { "HorizontalScroll", mso_sptHorizontalScroll },
    { "CircularArrow", mso_sptCircularArrow },
    { "NotchedCircularArrow", mso_sptNotchedCircularArrow },
    { "UturnArrow", mso_sptUturnArrow },
    { "CurvedRightArrow", mso_sptCurvedRightArrow },
    { "CurvedLeftArrow", mso_sptCurvedLeftArrow },
    { "CurvedUpArrow", mso_sptCurvedUpArrow },
    { "CurvedDownArrow", mso_sptCurvedDownArrow },
    { "CloudCallout", mso_sptCloudCallout },
    { "EllipseRibbon", mso_sptEllipseRibbon },
    { "EllipseRibbon2", mso_sptEllipseRibbon2 },
    { "FlowChartProcess", mso_sptFlowChartProcess },
    { "FlowChartDecision", mso_sptFlowChartDecision },
    { "FlowChartInputOutput", mso_sptFlowChartInputOutput },
    { "FlowChartPredefinedProcess", mso_sptFlowChartPredefinedProcess },
    { "FlowChartInternalStorage", mso_sptFlowChartInternalStorage },
    { "FlowChartDocument", mso_sptFlowChartDocument },
    { "FlowChartMultidocument", mso_sptFlowChartMultidocument },
    { "FlowChartTerminator", mso_sptFlowChartTerminator },
    { "FlowChartPreparation", mso_sptFlowChartPreparation },
    { "FlowChartManualInput", mso_sptFlowChartManualInput },
    { "FlowChartManualOperation", mso_sptFlowChartManualOperation },
    { "FlowChartConnector", mso_sptFlowChartConnector },
    { "FlowChartPunchedCard", mso_sptFlowChartPunchedCard },
    { "FlowChartPunchedTape", mso_sptFlowChartPunchedTape },
    { "FlowChartSummingJunction", mso_sptFlowChartSummingJunction },
    { "FlowChartOr", mso_sptFlowChartOr },
    { "FlowChartCollate", mso_sptFlowChartCollate },
    { "FlowChartSort", mso_sptFlowChartSort },
    { "FlowChartExtract", mso_sptFlowChartExtract },
    { "FlowChartMerge", mso_sptFlowChartMerge },
    { "FlowChartOfflineStorage", mso_sptFlowChartOfflineStorage },
    { "FlowChartOnlineStorage", mso_sptFlowChartOnlineStorage },
    { "FlowChartMagneticTape", mso_sptFlowChartMagneticTape },
    { "FlowChartMagneticDisk", mso_sptFlowChartMagneticDisk },
    { "FlowChartMagneticDrum", mso_sptFlowChartMagneticDrum },
    { "FlowChartDisplay", mso_sptFlowChartDisplay },
    { "FlowChartDelay", mso_sptFlowChartDelay },
    { "TextPlainText", mso_sptTextPlainText },
    { "TextStop", mso_sptTextStop },
    { "TextTriangle", mso_sptTextTriangle },
    { "TextTriangleInverted", mso_sptTextTriangleInverted },
    { "TextChevron", mso_sptTextChevron },
    { "TextChevronInverted", mso_sptTextChevronInverted },
    { "TextRingInside", mso_sptTextRingInside },
    { "TextRingOutside", mso_sptTextRingOutside },
    { "TextArchUpCurve", mso_sptTextArchUpCurve },
    { "TextArchDownCurve", mso_sptTextArchDownCurve },
    { "TextCircleCurve", mso_sptTextCircleCurve },
    { "TextButtonCurve", mso_sptTextButtonCurve },
    { "TextArchUpPour", mso_sptTextArchUpPour },
    { "TextArchDownPour", mso_sptTextArchDownPour },
    { "TextCirclePour", mso_sptTextCirclePour },
    { "TextButtonPour", mso_sptTextButtonPour },
    { "TextCurveUp", mso_sptTextCurveUp },
    { "TextCurveDown", mso_sptTextCurveDown },
    { "TextCascadeUp", mso_sptTextCascadeUp },
    { "TextCascadeDown", mso_sptTextCascadeDown },
    { "TextWave1", mso_sptTextWave1 },
    { "TextWave2", mso_sptTextWave2 },
    { "TextWave3", mso_sptTextWave3 },
    { "TextWave4", mso_sptTextWave4 },
    { "TextInflate", mso_sptTextInflate },
    { "TextDeflate", mso_sptTextDeflate },
    { "TextInflateBottom", mso_sptTextInflateBottom },
    { "TextDeflateBottom", mso_sptTextDeflateBottom },
    { "TextInflateTop", mso_sptTextInflateTop },
    { "TextDeflateTop", mso_sptTextDeflateTop },
    { "TextDeflateInflate", mso_sptTextDeflateInflate },
    { "TextDeflateInflateDeflate", mso_sptTextDeflateInflateDeflate },
    { "TextFadeRight", mso_sptTextFadeRight },
    { "TextFadeLeft", mso_sptTextFadeLeft },
    { "TextFadeUp", mso_sptTextFadeUp },
    { "TextFadeDown", mso_sptTextFadeDown },
    { "TextSlantUp", mso_sptTextSlantUp },
    { "TextSlantDown", mso_sptTextSlantDown },
    { "TextCanUp", mso_sptTextCanUp },
    { "TextCanDown", mso_sptTextCanDown },
    { "FlowChartAlternateProcess", mso_sptFlowChartAlternateProcess },
    { "FlowChartOffpageConnector", mso_sptFlowChartOffpageConnector },
    { "Callout90", mso_sptCallout90 },
    { "AccentCallout90", mso_sptAccentCallout90 },
    { "BorderCallout90", mso_sptBorderCallout90 },
    { "AccentBorderCallout90", mso_sptAccentBorderCallout90 },
    { "LeftRightUpArrow", mso_sptLeftRightUpArrow },
    { "Sun", mso_sptSun },
    { "Moon", mso_sptMoon },
    { "BracketPair", mso_sptBracketPair },
    { "BracePair", mso_sptBracePair },
    { "Seal4", mso_sptSeal4 },
    { "DoubleWave", mso_sptDoubleWave },
    { "ActionButtonBlank", mso_sptActionButtonBlank },
    { "ActionButtonHome", mso_sptActionButtonHome },
    { "ActionButtonHelp", mso_sptActionButtonHelp },
    { "ActionButtonInformation", mso_sptActionButtonInformation },
    { "ActionButtonForwardNext", mso_sptActionButtonForwardNext },
    { "ActionButtonBackPrevious", mso_sptActionButtonBackPrevious },
    { "ActionButtonEnd", mso_sptActionButtonEnd },
    { "ActionButtonBeginning", mso_sptActionButtonBeginning },
    { "ActionButtonReturn", mso_sptActionButtonReturn },
    { "ActionButtonDocument", mso_sptActionButtonDocument },
    { "ActionButtonSound", mso_sptActionButtonSound },
    { "ActionButtonMovie", mso_sptActionButtonMovie },
    { "HostControl", mso_sptHostControl },
    { "TextBox", mso_sptTextBox }
};

MSO_SPT EnhancedCustomShapeTypeNames::Get( const rtl::OUString& rShapeType )
{
    if ( !pHashMap )
    {   // init hash map
        ::osl::MutexGuard aGuard( getHashMapMutex() );
        if ( !pHashMap )
        {
            TypeNameHashMap* pH = new TypeNameHashMap;
            const NameTypeTable* pPtr = pNameTypeTableArray;
            const NameTypeTable* pEnd = pPtr + ( sizeof( pNameTypeTableArray ) / sizeof( NameTypeTable ) );
            for ( ; pPtr < pEnd; pPtr++ )
                (*pH)[ pPtr->pS ] = pPtr->pE;
            pHashMap = pH;
        }
    }
    MSO_SPT eRetValue = mso_sptNil;
    int i, nLen = rShapeType.getLength();
    char* pBuf = new char[ nLen + 1 ];
    for ( i = 0; i < nLen; i++ )
        pBuf[ i ] = (char)rShapeType[ i ];
    pBuf[ i ] = 0;
    TypeNameHashMap::iterator aHashIter( pHashMap->find( pBuf ) );
    delete[] pBuf;
    if ( aHashIter != pHashMap->end() )
        eRetValue = (*aHashIter).second;
    return eRetValue;
}

rtl::OUString EnhancedCustomShapeTypeNames::Get( const MSO_SPT eShapeType )
{
    sal_uInt32 i = eShapeType > mso_sptTextBox
        ? (sal_uInt32)mso_sptTextBox + 1
        : (sal_uInt32)eShapeType;
    return rtl::OUString::createFromAscii( pNameTypeTableArray[ i ].pS );
}
