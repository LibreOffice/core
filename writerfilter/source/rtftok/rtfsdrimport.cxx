/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Miklos Vajna <vmiklos@frugalware.org>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/WritingMode.hpp>

#include <ooxml/resourceids.hxx> // NS_ooxml namespace
#include <filter/msfilter/escherex.hxx>
#include <filter/msfilter/util.hxx>

#include <dmapper/DomainMapper.hxx>
#include "../dmapper/GraphicHelpers.hxx"
#include <rtfsdrimport.hxx>

#include <oox/vml/vmlformatting.hxx>
#include <oox/helper/modelobjecthelper.hxx>
#include <oox/drawingml/shapepropertymap.hxx>
#include <oox/helper/propertyset.hxx>

namespace writerfilter {
namespace rtftok {

RTFSdrImport::RTFSdrImport(RTFDocumentImpl& rDocument,
        uno::Reference<lang::XComponent> const& xDstDoc)
    : m_rImport(rDocument)
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawings(xDstDoc, uno::UNO_QUERY);
    if (xDrawings.is())
        m_xDrawPage.set(xDrawings->getDrawPage(), uno::UNO_QUERY);
}

RTFSdrImport::~RTFSdrImport()
{
}

void RTFSdrImport::createShape(OUString aStr, uno::Reference<drawing::XShape>& xShape, uno::Reference<beans::XPropertySet>& xPropertySet)
{
    if (m_rImport.getModelFactory().is())
        xShape.set(m_rImport.getModelFactory()->createInstance(aStr), uno::UNO_QUERY);
    xPropertySet.set(xShape, uno::UNO_QUERY);
}

std::vector<beans::PropertyValue> RTFSdrImport::getTextFrameDefaults(bool bNew)
{
    std::vector<beans::PropertyValue> aRet;
    beans::PropertyValue aPropertyValue;

    aPropertyValue.Name = "HoriOrient";
    aPropertyValue.Value <<= text::HoriOrientation::NONE;
    aRet.push_back(aPropertyValue);
    aPropertyValue.Name = "VertOrient";
    aPropertyValue.Value <<= text::VertOrientation::NONE;
    aRet.push_back(aPropertyValue);
    if (!bNew)
    {
        aPropertyValue.Name = "BackColorTransparency";
        aPropertyValue.Value <<= sal_Int32(100);
        aRet.push_back(aPropertyValue);
    }
    // See the spec, new-style frame default margins are specified in EMUs.
    aPropertyValue.Name = "LeftBorderDistance";
    aPropertyValue.Value <<= sal_Int32(bNew ? (91440 / 360) : 0);
    aRet.push_back(aPropertyValue);
    aPropertyValue.Name = "RightBorderDistance";
    aPropertyValue.Value <<= sal_Int32(bNew ? (91440 / 360) : 0);
    aRet.push_back(aPropertyValue);
    aPropertyValue.Name = "TopBorderDistance";
    aPropertyValue.Value <<= sal_Int32(bNew ? (45720 / 360) : 0);
    aRet.push_back(aPropertyValue);
    aPropertyValue.Name = "BottomBorderDistance";
    aPropertyValue.Value <<= sal_Int32(bNew ? (45720 / 360) : 0);
    aRet.push_back(aPropertyValue);
    aPropertyValue.Name = "SizeType";
    aPropertyValue.Value <<= text::SizeType::FIX;
    aRet.push_back(aPropertyValue);
    return aRet;
}

void RTFSdrImport::resolveDhgt(uno::Reference<beans::XPropertySet> xPropertySet, sal_Int32 nZOrder)
{
    writerfilter::dmapper::DomainMapper& rMapper = (writerfilter::dmapper::DomainMapper&)m_rImport.Mapper();
    writerfilter::dmapper::GraphicZOrderHelper* pHelper = rMapper.graphicZOrderHelper();
    xPropertySet->setPropertyValue("ZOrder", uno::makeAny(pHelper->findZOrder(nZOrder)));
    pHelper->addItem(xPropertySet, nZOrder);
}

void RTFSdrImport::resolveFLine(uno::Reference<beans::XPropertySet> xPropertySet, sal_Int32 nFLine)
{
    if (nFLine == 0)
        xPropertySet->setPropertyValue("LineStyle", uno::makeAny(drawing::LineStyle_NONE));
}

void RTFSdrImport::resolve(RTFShape& rShape)
{
    int nType = -1;
    bool bPib = false;
    bool bCustom = false;
    bool bTextFrame = false;

    uno::Reference<drawing::XShape> xShape;
    uno::Reference<beans::XPropertySet> xPropertySet;
    // Create this early, as custom shapes may have properties before the type arrives.
    createShape("com.sun.star.drawing.CustomShape", xShape, xPropertySet);
    uno::Any aAny;
    beans::PropertyValue aPropertyValue;
    awt::Rectangle aViewBox;
    std::vector<beans::PropertyValue> aPathPropVec;
    // Default line color is black in Word, blue in Writer.
    uno::Any aLineColor = uno::makeAny(COL_BLACK);
    // Default line width is 0.75 pt (26 mm100) in Word, 0 in Writer.
    uno::Any aLineWidth = uno::makeAny(sal_Int32(26));
    text::WritingMode eWritingMode = text::WritingMode_LR_TB;
    // Used for gradients, let the VML import do the hard work.
    oox::vml::FillModel aFillModel;

    for (std::vector< std::pair<OUString, OUString> >::iterator i = rShape.aProperties.begin();
            i != rShape.aProperties.end(); ++i)
    {
        if ( i->first == "shapeType" )
        {
            nType = i->second.toInt32();
            switch (nType)
            {
                case ESCHER_ShpInst_Line:
                    createShape("com.sun.star.drawing.LineShape", xShape, xPropertySet);
                    break;
                case ESCHER_ShpInst_Rectangle:
                case ESCHER_ShpInst_TextBox:
                    if (!m_rImport.getShapetextBuffer().empty())
                    {
                        createShape("com.sun.star.text.TextFrame", xShape, xPropertySet);
                        bTextFrame = true;
                        std::vector<beans::PropertyValue> aDefaults = getTextFrameDefaults(true);
                        for (size_t j = 0; j < aDefaults.size(); ++j)
                            xPropertySet->setPropertyValue(aDefaults[j].Name, aDefaults[j].Value);
                    }
                    else
                        bCustom = true;
                    break;
                default:
                    bCustom = true;
                    break;
            }

            // Defaults
            aAny <<= (sal_uInt32)0xffffff; // White in Word, kind of blue in Writer.
            if (xPropertySet.is() && !bTextFrame)
                xPropertySet->setPropertyValue("FillColor", aAny);
        }
        else if ( i->first == "wzName" )
        {
            if (bTextFrame)
            {
                uno::Reference<container::XNamed> xNamed(xShape, uno::UNO_QUERY);
                xNamed->setName(i->second);
            }
            else
                xPropertySet->setPropertyValue("Name", uno::makeAny(i->second));
        }
        else if ( i->first == "wzDescription" )
            xPropertySet->setPropertyValue("Description", uno::makeAny(i->second));
        else if ( i->first == "pib" )
        {
            m_rImport.setDestinationText(i->second);
            bPib = true;
        }
        else if (i->first == "fillColor" && xPropertySet.is())
        {
            aAny <<= msfilter::util::BGRToRGB(i->second.toInt32());
            if (bTextFrame)
                xPropertySet->setPropertyValue("BackColor", aAny);
            else
                xPropertySet->setPropertyValue("FillColor", aAny);

            // fillType will decide, possible it'll be the start color of a gradient.
            aFillModel.moColor.set(OUString("#") + OStringToOUString(msfilter::util::ConvertColor(aAny.get<sal_Int32>()), RTL_TEXTENCODING_UTF8));
        }
        else if ( i->first == "fillBackColor" )
            // fillType will decide, possible it'll be the end color of a gradient.
            aFillModel.moColor2.set(OUString("#") + OStringToOUString(msfilter::util::ConvertColor(msfilter::util::BGRToRGB(i->second.toInt32())), RTL_TEXTENCODING_UTF8));
        else if (i->first == "lineColor")
            aLineColor <<= msfilter::util::BGRToRGB(i->second.toInt32());
        else if ( i->first == "lineBackColor" )
            ; // Ignore: complementer of lineColor
        else if (i->first == "txflTextFlow" && xPropertySet.is())
        {
            if (i->second.toInt32() == 1)
                eWritingMode = text::WritingMode_TB_RL;
        }
        else if (i->first == "fLine" && xPropertySet.is())
            resolveFLine(xPropertySet, i->second.toInt32());
        else if (i->first == "fillOpacity" && xPropertySet.is())
        {
           int opacity = 100 - (i->second.toInt32())*100/65536;
           aAny <<= uno::makeAny(sal_uInt32(opacity));
           xPropertySet->setPropertyValue("FillTransparence", aAny);
        }
        else if (i->first == "rotation" && xPropertySet.is())
        {
            aAny <<= i->second.toInt32()*100/65536;
            xPropertySet->setPropertyValue("RotateAngle", aAny);
        }
        else if (i->first == "lineWidth")
            aLineWidth <<= i->second.toInt32()/360;
        else if ( i->first == "pVerticies" )
        {
            uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aCoordinates;
            sal_Int32 nSize = 0; // Size of a token (it's value is hardwired in the exporter)
            sal_Int32 nCount = 0; // Number of tokens
            sal_Int32 nCharIndex = 0; // Character index
            sal_Int32 nIndex = 0; // Array index
            do
            {
                OUString aToken = i->second.getToken(0, ';', nCharIndex);
                if (!nSize)
                    nSize = aToken.toInt32();
                else if (!nCount)
                {
                    nCount = aToken.toInt32();
                    aCoordinates.realloc(nCount);
                }
                else
                {
                    // The coordinates are in an (x,y) form.
                    aToken = aToken.copy(1, aToken.getLength() - 2);
                    sal_Int32 nI = 0;
                    sal_Int32 nX = 0;
                    sal_Int32 nY = 0;
                    do
                    {
                        OUString aPoint = aToken.getToken(0, ',', nI);
                        if (!nX)
                            nX = aPoint.toInt32();
                        else
                            nY = aPoint.toInt32();
                    }
                    while (nI >= 0);
                    aCoordinates[nIndex].First.Value <<= nX;
                    aCoordinates[nIndex].Second.Value <<= nY;
                    nIndex++;
                }
            }
            while (nCharIndex >= 0);
            aPropertyValue.Name = "Coordinates";
            aPropertyValue.Value <<= aCoordinates;
            aPathPropVec.push_back(aPropertyValue);
        }
        else if ( i->first == "pSegmentInfo" )
        {
            uno::Sequence<drawing::EnhancedCustomShapeSegment> aSegments;
            sal_Int32 nSize = 0;
            sal_Int32 nCount = 0;
            sal_Int32 nCharIndex = 0;
            sal_Int32 nIndex = 0;
            do
            {
                sal_Int32 nSeg = i->second.getToken(0, ';', nCharIndex).toInt32();
                if (!nSize)
                    nSize = nSeg;
                else if (!nCount)
                {
                    nCount = nSeg;
                    aSegments.realloc(nCount);
                }
                else
                {
                    sal_Int32 nPoints = 1;
                    if (nSeg >= 0x2000 && nSeg < 0x20FF)
                    {
                        nPoints = nSeg & 0x0FFF;
                        nSeg &= 0xFF00;
                    }

                    switch (nSeg)
                    {
                        case 0x0001: // lineto
                            aSegments[nIndex].Command = drawing::EnhancedCustomShapeSegmentCommand::LINETO;
                            aSegments[nIndex].Count = sal_Int32(1);
                            break;
                        case 0x4000: // moveto
                            aSegments[nIndex].Command = drawing::EnhancedCustomShapeSegmentCommand::MOVETO;
                            aSegments[nIndex].Count = sal_Int32(1);
                            break;
                        case 0x2000: // curveto
                            aSegments[nIndex].Command = drawing::EnhancedCustomShapeSegmentCommand::CURVETO;
                            aSegments[nIndex].Count = sal_Int32(nPoints);
                            break;
                        case 0xb300: // arcto
                            aSegments[nIndex].Command = drawing::EnhancedCustomShapeSegmentCommand::ARCTO;
                            aSegments[nIndex].Count = sal_Int32(0);
                            break;
                        case 0xac00:
                        case 0xaa00: // nofill
                        case 0xab00: // nostroke
                        case 0x6001: // close
                            break;
                        case 0x8000: // end
                            aSegments[nIndex].Command = drawing::EnhancedCustomShapeSegmentCommand::ENDSUBPATH;
                            aSegments[nIndex].Count = sal_Int32(0);
                            break;
                        default: // given number of lineto elements
                            aSegments[nIndex].Command = drawing::EnhancedCustomShapeSegmentCommand::LINETO;
                            aSegments[nIndex].Count = nSeg;
                            break;
                    }
                    nIndex++;
                }
            }
            while (nCharIndex >= 0);
            aPropertyValue.Name = "Segments";
            aPropertyValue.Value <<= aSegments;
            aPathPropVec.push_back(aPropertyValue);
        }
        else if ( i->first == "geoLeft" )
            aViewBox.X = i->second.toInt32();
        else if ( i->first == "geoTop" )
            aViewBox.Y = i->second.toInt32();
        else if ( i->first == "geoRight" )
            aViewBox.Width = i->second.toInt32();
        else if ( i->first == "geoBottom" )
            aViewBox.Height = i->second.toInt32();
        else if ( i->first == "dhgt" )
        {
            resolveDhgt(xPropertySet, i->second.toInt32());
            // dhgt is Word 2007, \shpz is Word 97-2003, the previous has priority.
            rShape.oZ.reset();
        }
        // These are in EMU, convert to mm100.
        else if (i->first == "dxTextLeft")
            xPropertySet->setPropertyValue("LeftBorderDistance", uno::makeAny(i->second.toInt32() / 360));
        else if (i->first == "dyTextTop")
            xPropertySet->setPropertyValue("TopBorderDistance", uno::makeAny(i->second.toInt32() / 360));
        else if (i->first == "dxTextRight")
            xPropertySet->setPropertyValue("RightBorderDistance", uno::makeAny(i->second.toInt32() / 360));
        else if (i->first == "dyTextBottom")
            xPropertySet->setPropertyValue("BottomBorderDistance", uno::makeAny(i->second.toInt32() / 360));
        else if (i->first == "dxWrapDistLeft")
            xPropertySet->setPropertyValue("LeftMargin", uno::makeAny(i->second.toInt32() / 360));
        else if (i->first == "dyWrapDistTop")
            xPropertySet->setPropertyValue("TopMargin", uno::makeAny(i->second.toInt32() / 360));
        else if (i->first == "dxWrapDistRight")
            xPropertySet->setPropertyValue("RightMargin", uno::makeAny(i->second.toInt32() / 360));
        else if (i->first == "dyWrapDistBottom")
            xPropertySet->setPropertyValue("BottomMargin", uno::makeAny(i->second.toInt32() / 360));
        else if (i->first == "fillType")
        {
            switch (i->second.toInt32())
            {
                case 7: // Shade using the fillAngle
                    aFillModel.moType.set(oox::XML_gradient);
                break;
                default:
                    SAL_INFO("writerfilter", "TODO handle fillType value '" << i->second << "'");
                break;
            }
        }
        else if (i->first == "fillFocus")
            aFillModel.moFocus.set(i->second.toDouble() / 100); // percent
        else
            SAL_INFO("writerfilter", "TODO handle shape property '" << i->first << "':'" << i->second << "'");
    }

    if (xPropertySet.is())
    {
        if (!bTextFrame)
            xPropertySet->setPropertyValue("LineColor", aLineColor);
        xPropertySet->setPropertyValue("LineWidth", aLineWidth);
        if (rShape.oZ)
            resolveDhgt(xPropertySet, *rShape.oZ);
        if (bTextFrame)
            // Writer textframes implement text::WritingMode2, which is a different data type.
            xPropertySet->setPropertyValue("WritingMode", uno::makeAny(sal_Int16(eWritingMode)));
        else
            xPropertySet->setPropertyValue("TextWritingMode", uno::makeAny(eWritingMode));
    }

    if (nType == ESCHER_ShpInst_PictureFrame) // picture frame
    {
        if (bPib)
            m_rImport.resolvePict(false);
        return;
    }

    if (m_xDrawPage.is() && !bTextFrame)
        m_xDrawPage->add(xShape);
    if (bCustom && xShape.is())
    {
        uno::Reference<drawing::XEnhancedCustomShapeDefaulter> xDefaulter(xShape, uno::UNO_QUERY);
        xDefaulter->createCustomShapeDefaults(OUString::valueOf(sal_Int32(nType)));
    }

    // Creating Path property
    uno::Sequence<beans::PropertyValue> aPathPropSeq(aPathPropVec.size());
    beans::PropertyValue* pPathValues = aPathPropSeq.getArray();
    for (std::vector<beans::PropertyValue>::iterator i = aPathPropVec.begin(); i != aPathPropVec.end(); ++i)
        *pPathValues++ = *i;

    // Creating CustomShapeGeometry property
    std::vector<beans::PropertyValue> aGeomPropVec;
    if (aViewBox.X || aViewBox.Y || aViewBox.Width || aViewBox.Height)
    {
        aViewBox.Width -= aViewBox.X;
        aViewBox.Height -= aViewBox.Y;
        aPropertyValue.Name = "ViewBox";
        aPropertyValue.Value <<= aViewBox;
        aGeomPropVec.push_back(aPropertyValue);
    }
    if (aPathPropSeq.getLength())
    {
        aPropertyValue.Name = "Path";
        aPropertyValue.Value <<= aPathPropSeq;
        aGeomPropVec.push_back(aPropertyValue);
    }
    uno::Sequence<beans::PropertyValue> aGeomPropSeq(aGeomPropVec.size());
    beans::PropertyValue* pGeomValues = aGeomPropSeq.getArray();
    for (std::vector<beans::PropertyValue>::iterator i = aGeomPropVec.begin(); i != aGeomPropVec.end(); ++i)
        *pGeomValues++ = *i;
    if (aGeomPropSeq.getLength() && xPropertySet.is())
        xPropertySet->setPropertyValue("CustomShapeGeometry", uno::Any(aGeomPropSeq));

    // Set position and size
    if (xShape.is())
    {
        if (bTextFrame)
        {
            xPropertySet->setPropertyValue("HoriOrientPosition", uno::makeAny(rShape.nLeft));
            xPropertySet->setPropertyValue("VertOrientPosition", uno::makeAny(rShape.nTop));
        }
        else
            xShape->setPosition(awt::Point(rShape.nLeft, rShape.nTop));
        xShape->setSize(awt::Size(rShape.nRight - rShape.nLeft, rShape.nBottom - rShape.nTop));
        if (rShape.nHoriOrientRelation != 0)
            xPropertySet->setPropertyValue("HoriOrientRelation", uno::makeAny(rShape.nHoriOrientRelation));
        if (rShape.nVertOrientRelation != 0)
            xPropertySet->setPropertyValue("VertOrientRelation", uno::makeAny(rShape.nVertOrientRelation));
        if (rShape.nWrap != -1)
            xPropertySet->setPropertyValue("Surround", uno::makeAny(text::WrapTextMode(rShape.nWrap)));
        if (aFillModel.moType.has())
        {
            oox::ModelObjectHelper aModelObjectHelper(m_rImport.getModelFactory());
            oox::drawingml::ShapePropertyMap aPropMap(aModelObjectHelper);
            aFillModel.pushToPropMap(aPropMap, m_rImport.getGraphicHelper());
            // Sets the FillStyle and FillGradient UNO properties.
            oox::PropertySet(xShape).setProperties(aPropMap);
        }
    }

    // Send it to dmapper
    m_rImport.Mapper().startShape(xShape);
    m_rImport.replayShapetext();
    m_rImport.Mapper().endShape();
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
