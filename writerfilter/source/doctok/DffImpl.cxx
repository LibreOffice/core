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

#include "resources.hxx"
#include "WW8DocumentImpl.hxx"

namespace writerfilter {
namespace doctok 
{

class ShapeTypeToString
{
    typedef boost::shared_ptr<ShapeTypeToString> Pointer_t;

    static Pointer_t pInstance;

    typedef map<sal_uInt32, string> Map_t;
    Map_t mMap;

protected:
    ShapeTypeToString();

public:
    static Pointer_t Instance() 
    {
        if (pInstance == NULL)
            pInstance = Pointer_t(new ShapeTypeToString());

        return pInstance;
    }

    string operator()(sal_uInt32 nShapeType)
    {
        return mMap[nShapeType];
    }
};

ShapeTypeToString::Pointer_t ShapeTypeToString::pInstance;

ShapeTypeToString::ShapeTypeToString()
{
    mMap[0]="NotPrimitive"; 
    mMap[1]="Rectangle"; 
    mMap[2]="RoundRectangle"; 
    mMap[3]="Ellipse"; 
    mMap[4]="Diamond"; 
    mMap[5]="IsocelesTriangle"; 
    mMap[6]="RightTriangle"; 
    mMap[7]="Parallelogram"; 
    mMap[8]="Trapezoid"; 
    mMap[9]="Hexagon"; 
    mMap[10]="Octagon"; 
    mMap[11]="Plus"; 
    mMap[12]="Star"; 
    mMap[13]="Arrow"; 
    mMap[14]="ThickArrow"; 
    mMap[15]="HomePlate"; 
    mMap[16]="Cube"; 
    mMap[17]="Balloon"; 
    mMap[18]="Seal"; 
    mMap[19]="Arc"; 
    mMap[20]="Line"; 
    mMap[21]="Plaque"; 
    mMap[22]="Can"; 
    mMap[23]="Donut"; 
    mMap[24]="TextSimple"; 
    mMap[25]="TextOctagon"; 
    mMap[26]="TextHexagon"; 
    mMap[27]="TextCurve"; 
    mMap[28]="TextWave"; 
    mMap[29]="TextRing"; 
    mMap[30]="TextOnCurve"; 
    mMap[31]="TextOnRing"; 
    mMap[32]="StraightConnector1"; 
    mMap[33]="BentConnector2"; 
    mMap[34]="BentConnector3"; 
    mMap[35]="BentConnector4"; 
    mMap[36]="BentConnector5"; 
    mMap[37]="CurvedConnector2"; 
    mMap[38]="CurvedConnector3"; 
    mMap[39]="CurvedConnector4"; 
    mMap[40]="CurvedConnector5"; 
    mMap[41]="Callout1"; 
    mMap[42]="Callout2"; 
    mMap[43]="Callout3"; 
    mMap[44]="AccentCallout1"; 
    mMap[45]="AccentCallout2"; 
    mMap[46]="AccentCallout3"; 
    mMap[47]="BorderCallout1"; 
    mMap[48]="BorderCallout2"; 
    mMap[49]="BorderCallout3"; 
    mMap[50]="AccentBorderCallout1"; 
    mMap[51]="AccentBorderCallout2"; 
    mMap[52]="AccentBorderCallout3"; 
    mMap[53]="Ribbon"; 
    mMap[54]="Ribbon2"; 
    mMap[55]="Chevron"; 
    mMap[56]="Pentagon"; 
    mMap[57]="NoSmoking"; 
    mMap[58]="Seal8"; 
    mMap[59]="Seal16"; 
    mMap[60]="Seal32"; 
    mMap[61]="WedgeRectCallout"; 
    mMap[62]="WedgeRRectCallout"; 
    mMap[63]="WedgeEllipseCallout"; 
    mMap[64]="Wave"; 
    mMap[65]="FoldedCorner"; 
    mMap[66]="LeftArrow"; 
    mMap[67]="DownArrow"; 
    mMap[68]="UpArrow"; 
    mMap[69]="LeftRightArrow"; 
    mMap[70]="UpDownArrow"; 
    mMap[71]="IrregularSeal1"; 
    mMap[72]="IrregularSeal2"; 
    mMap[73]="LightningBolt"; 
    mMap[74]="Heart"; 
    mMap[75]="PictureFrame"; 
    mMap[76]="QuadArrow"; 
    mMap[77]="LeftArrowCallout"; 
    mMap[78]="RightArrowCallout"; 
    mMap[79]="UpArrowCallout"; 
    mMap[80]="DownArrowCallout"; 
    mMap[81]="LeftRightArrowCallout"; 
    mMap[82]="UpDownArrowCallout"; 
    mMap[83]="QuadArrowCallout"; 
    mMap[84]="Bevel"; 
    mMap[85]="LeftBracket"; 
    mMap[86]="RightBracket"; 
    mMap[87]="LeftBrace"; 
    mMap[88]="RightBrace"; 
    mMap[89]="LeftUpArrow"; 
    mMap[90]="BentUpArrow"; 
    mMap[91]="BentArrow"; 
    mMap[92]="Seal24"; 
    mMap[93]="StripedRightArrow"; 
    mMap[94]="NotchedRightArrow"; 
    mMap[95]="BlockArc"; 
    mMap[96]="SmileyFace"; 
    mMap[97]="VerticalScroll"; 
    mMap[98]="HorizontalScroll"; 
    mMap[99]="CircularArrow"; 
    mMap[100]="NotchedCircularArrow"; 
    mMap[101]="UturnArrow"; 
    mMap[102]="CurvedRightArrow"; 
    mMap[103]="CurvedLeftArrow"; 
    mMap[104]="CurvedUpArrow"; 
    mMap[105]="CurvedDownArrow"; 
    mMap[106]="CloudCallout"; 
    mMap[107]="EllipseRibbon"; 
    mMap[108]="EllipseRibbon2"; 
    mMap[109]="FlowChartProcess"; 
    mMap[110]="FlowChartDecision"; 
    mMap[111]="FlowChartInputOutput"; 
    mMap[112]="FlowChartPredefinedProcess"; 
    mMap[113]="FlowChartInternalStorage"; 
    mMap[114]="FlowChartDocument"; 
    mMap[115]="FlowChartMultidocument"; 
    mMap[116]="FlowChartTerminator"; 
    mMap[117]="FlowChartPreparation"; 
    mMap[118]="FlowChartManualInput"; 
    mMap[119]="FlowChartManualOperation"; 
    mMap[120]="FlowChartConnector"; 
    mMap[121]="FlowChartPunchedCard"; 
    mMap[122]="FlowChartPunchedTape"; 
    mMap[123]="FlowChartSummingJunction"; 
    mMap[124]="FlowChartOr"; 
    mMap[125]="FlowChartCollate"; 
    mMap[126]="FlowChartSort"; 
    mMap[127]="FlowChartExtract"; 
    mMap[128]="FlowChartMerge"; 
    mMap[129]="FlowChartOfflineStorage"; 
    mMap[130]="FlowChartOnlineStorage"; 
    mMap[131]="FlowChartMagneticTape"; 
    mMap[132]="FlowChartMagneticDisk"; 
    mMap[133]="FlowChartMagneticDrum"; 
    mMap[134]="FlowChartDisplay"; 
    mMap[135]="FlowChartDelay"; 
    mMap[136]="TextPlainText"; 
    mMap[137]="TextStop"; 
    mMap[138]="TextTriangle"; 
    mMap[139]="TextTriangleInverted"; 
    mMap[140]="TextChevron"; 
    mMap[141]="TextChevronInverted"; 
    mMap[142]="TextRingInside"; 
    mMap[143]="TextRingOutside"; 
    mMap[144]="TextArchUpCurve"; 
    mMap[145]="TextArchDownCurve"; 
    mMap[146]="TextCircleCurve"; 
    mMap[147]="TextButtonCurve"; 
    mMap[148]="TextArchUpPour"; 
    mMap[149]="TextArchDownPour"; 
    mMap[150]="TextCirclePour"; 
    mMap[151]="TextButtonPour"; 
    mMap[152]="TextCurveUp"; 
    mMap[153]="TextCurveDown"; 
    mMap[154]="TextCascadeUp"; 
    mMap[155]="TextCascadeDown"; 
    mMap[156]="TextWave1"; 
    mMap[157]="TextWave2"; 
    mMap[158]="TextWave3"; 
    mMap[159]="TextWave4"; 
    mMap[160]="TextInflate"; 
    mMap[161]="TextDeflate"; 
    mMap[162]="TextInflateBottom"; 
    mMap[163]="TextDeflateBottom"; 
    mMap[164]="TextInflateTop"; 
    mMap[165]="TextDeflateTop"; 
    mMap[166]="TextDeflateInflate"; 
    mMap[167]="TextDeflateInflateDeflate"; 
    mMap[168]="TextFadeRight"; 
    mMap[169]="TextFadeLeft"; 
    mMap[170]="TextFadeUp"; 
    mMap[171]="TextFadeDown"; 
    mMap[172]="TextSlantUp"; 
    mMap[173]="TextSlantDown"; 
    mMap[174]="TextCanUp"; 
    mMap[175]="TextCanDown"; 
    mMap[176]="FlowChartAlternateProcess"; 
    mMap[177]="FlowChartOffpageConnector"; 
    mMap[178]="Callout90"; 
    mMap[179]="AccentCallout90"; 
    mMap[180]="BorderCallout90"; 
    mMap[181]="AccentBorderCallout90"; 
    mMap[182]="LeftRightUpArrow"; 
    mMap[183]="Sun"; 
    mMap[184]="Moon"; 
    mMap[185]="BracketPair"; 
    mMap[186]="BracePair"; 
    mMap[187]="Seal4"; 
    mMap[188]="DoubleWave"; 
    mMap[189]="ActionButtonBlank"; 
    mMap[190]="ActionButtonHome"; 
    mMap[191]="ActionButtonHelp"; 
    mMap[192]="ActionButtonInformation"; 
    mMap[193]="ActionButtonForwardNext"; 
    mMap[194]="ActionButtonBackPrevious"; 
    mMap[195]="ActionButtonEnd"; 
    mMap[196]="ActionButtonBeginning"; 
    mMap[197]="ActionButtonReturn"; 
    mMap[198]="ActionButtonDocument"; 
    mMap[199]="ActionButtonSound"; 
    mMap[200]="ActionButtonMovie"; 
    mMap[201]="HostControl"; 
    mMap[202]="TextBox"; 
}

// DffOPT

sal_uInt32 DffOPT::get_property_count()
{
    return getInstance();
}

writerfilter::Reference<Properties>::Pointer_t 
DffOPT::get_property(sal_uInt32 nPos)
{
    WW8FOPTE * pTmp = new WW8FOPTE(this, 0x8 + nPos * WW8FOPTE::getSize());
    pTmp->setIndex(nPos);

    return writerfilter::Reference<Properties>::Pointer_t(pTmp);
}

sal_uInt32 DffOPT::get_extraoffset_count()
{
    return get_property_count();
}

sal_uInt32 DffOPT::get_extraoffset(sal_uInt32 pos)
{
    sal_uInt32 nResult;
    sal_uInt32 nCount = get_property_count();

    if (pos < nCount)
    {
        nResult = 0x8 + nCount * WW8FOPTE::getSize();
        
        for (sal_uInt32 n = 0; n < pos; ++n)
        {
            WW8FOPTE aFOPTE(this, 0x8 + n * WW8FOPTE::getSize());
            
            if (aFOPTE.get_fComplex())
            {
                sal_uInt32 nValue = aFOPTE.get_op();
                nResult += nValue;
            }
        }
    }
    else
        nResult = getCount();
    
    return nResult;
}

//DffDGG

sal_uInt32 DffDGG::get_fidcl_count()
{
    return (getCount() - 0x18) / WW8FIDCL::getSize();
}

writerfilter::Reference<Properties>::Pointer_t
DffDGG::get_fidcl(sal_uInt32 pos)
{
    return writerfilter::Reference<Properties>::Pointer_t
        (new WW8FIDCL(this, 0x18 + pos * WW8FIDCL::getSize()));
}


// DffBSE

rtl::OUString DffBSE::get_blipname()
{
    rtl::OUString sResult;
    WW8FBSE aFBSE(this, 0x8);
 
    if (aFBSE.get_cbName() > 0)
        sResult = getString(0x24, aFBSE.get_cbName());

    return sResult;
}

writerfilter::Reference<Properties>::Pointer_t
DffBSE::get_blip()
{
    writerfilter::Reference<Properties>::Pointer_t pResult;

    WW8FBSE aFBSE(this, 8);
    sal_uInt32 nOffset = 8 + WW8FBSE::getSize() + aFBSE.get_cbName();

    if (nOffset + 8 < getCount())
    {
        WW8StructBase aTmp(this, nOffset, 0x8);
        
        sal_uInt32 nCount = getCount() - 8;
        
        if (aTmp.getU32(0x4) - 8 < nCount)
            nCount = aTmp.getU32(0x4) - 8;
        
        if (nCount)
        {
            DffRecord * pRecord = createDffRecord(this, nOffset);
            
            pResult = writerfilter::Reference<Properties>::Pointer_t(pRecord);
        }
    }
    else
    {
        nOffset = sal::static_int_cast<sal_Int32>(aFBSE.get_foDelay());
        if (! (nOffset & 1 << 31) && nOffset > 0 && getDocument() != NULL)
        {
            WW8StructBase aStructBase(*getDocument()->getDocStream(), 
                                      nOffset, 0x8);

            DffRecord * pRecord = 
                createDffRecord(*getDocument()->getDocStream(), 
                                aFBSE.get_foDelay());

            pResult = writerfilter::Reference<Properties>::Pointer_t(pRecord);
        }
    }

    return pResult;
}

#if 0
WW8BinaryObjReference::Pointer_t DffBSE::get_binary()
{
    WW8BinaryObjReference::Pointer_t pResult;

    if (getCount() > 0x45)
        pResult = WW8BinaryObjReference::Pointer_t
            (new WW8BinaryObjReference(this, 0x45,
                                       getCount() - 0x45));
    else
    {
        WW8FBSE aFBSE(this, 0x8);

        sal_Int32 nOffset = sal::static_int_cast<sal_Int32>(aFBSE.get_foDelay());
        if (nOffset > 0 && getDocument() != NULL)
        {
            WW8StructBase aStructBase(*getDocument()->getDocStream(), 
                                      nOffset, 0x8);
            
            sal_uInt32 nCount = aStructBase.getU32(0x4) - 0x11;

            pResult = WW8BinaryObjReference::Pointer_t
                (new WW8BinaryObjReference(*getDocument()->getDocStream(), 
                                           aFBSE.get_foDelay() + 0x19, nCount));
        }
    }

    return pResult;
}
#endif

// WW8FOPTE
void WW8FOPTE::resolveNoAuto(Properties & rHandler)
{
    sal_uInt16 nId = get_pid() + 1;
    sal_uInt32 nOp = get_op();
    sal_uInt32 nMask = 2;

    while (isBooleanDffOpt(nId))
    {
        WW8Value::Pointer_t pVal = createValue(getDffOptName(nId));
        rHandler.attribute(NS_rtf::LN_shpname, *pVal);

        pVal = createValue((nOp & nMask) != 0);
        rHandler.attribute(NS_rtf::LN_shpvalue, *pVal);

        --nId;
        nMask = nMask << 1;
    }
}

// DffFSP

rtl::OUString DffFSP::get_shptypename()
{
    string aName = (*ShapeTypeToString::Instance())(get_shptype());

    return rtl::OUString::createFromAscii(aName.c_str());
}

// DffSpContainer

writerfilter::Reference<Properties>::Pointer_t
DffSpContainer::get_blip()
{
    writerfilter::Reference<Properties>::Pointer_t pResult;
    
    if (getShapeType() == 75)
    {
        sal_uInt32 nBid = getShapeBid();

        if (getDocument() != NULL && nBid > 0)
            pResult = getDocument()->getBlip(nBid);
    }

    return pResult;
}

writerfilter::Reference<Stream>::Pointer_t
DffSpContainer::get_shptxt()
{
    writerfilter::Reference<Stream>::Pointer_t pResult;

    if (getShapeType() == 202)
    {
        sal_uInt32 nShpId = getShapeId();

        if (getDocument() != NULL)
            pResult = getDocument()->getTextboxText(nShpId);
    }
    
    return pResult;
}

// DffUDefProp

void DffUDefProp::resolveNoAuto(Properties & rHandler)
{
    sal_uInt32 nOffset = 0x8;
    sal_uInt32 nCount = getCount();

    while (nOffset + 6 <= nCount)
    {
        sal_uInt16 nPid = getU16(nOffset);
        sal_uInt32 nValue = getU32(nOffset + 2);

        sal_uInt32 nAttrid = 0;
        switch (nPid)
        {
        case 0x38f: nAttrid = NS_rtf::LN_XAlign; break;
        case 0x390: nAttrid = NS_rtf::LN_XRelTo; break;
        case 0x391: nAttrid = NS_rtf::LN_YAlign; break;
        case 0x392: nAttrid = NS_rtf::LN_YRelTo; break;
        case 0x3bf: nAttrid = NS_rtf::LN_LayoutInTableCell; break;
        case 0x53f: nAttrid = NS_rtf::LN_Inline; break;
        default:
            break;
        }

        if (nAttrid != 0)
        {
            WW8Value::Pointer_t pVal = createValue(nValue);
            rHandler.attribute(nAttrid, *pVal);
        }

        nOffset += 6;
    }
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
