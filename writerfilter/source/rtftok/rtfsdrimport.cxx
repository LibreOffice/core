/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>

#include <ooxml/resourceids.hxx>
#include <filter/msfilter/escherex.hxx>
#include <filter/msfilter/util.hxx>
#include <svx/svdtrans.hxx>
#include <tools/mapunit.hxx>

#include <dmapper/DomainMapper.hxx>
#include "../dmapper/GraphicHelpers.hxx"
#include <rtfsdrimport.hxx>
#include <rtfreferenceproperties.hxx>

#include <oox/vml/vmlformatting.hxx>
#include <oox/helper/modelobjecthelper.hxx>
#include <oox/drawingml/shapepropertymap.hxx>
#include <oox/helper/propertyset.hxx>

using namespace com::sun::star;

namespace writerfilter
{
namespace rtftok
{

RTFSdrImport::RTFSdrImport(RTFDocumentImpl& rDocument,
                           uno::Reference<lang::XComponent> const& xDstDoc)
    : m_rImport(rDocument)
    , m_bTextFrame(false)
    , m_bFakePict(false)
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawings(xDstDoc, uno::UNO_QUERY);
    if (xDrawings.is())
        m_aParents.push(xDrawings->getDrawPage());
}

RTFSdrImport::~RTFSdrImport()
{
    if (m_aParents.size())
        m_aParents.pop();
}

void RTFSdrImport::createShape(const OUString& aStr, uno::Reference<drawing::XShape>& xShape, uno::Reference<beans::XPropertySet>& xPropertySet)
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
    aPropertyValue.Name = "HoriOrientRelation";
    aPropertyValue.Value <<= text::RelOrientation::FRAME;
    aRet.push_back(aPropertyValue);
    aPropertyValue.Name = "VertOrient";
    aPropertyValue.Value <<= text::VertOrientation::NONE;
    aRet.push_back(aPropertyValue);
    aPropertyValue.Name = "VertOrientRelation";
    aPropertyValue.Value <<= text::RelOrientation::FRAME;
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

void RTFSdrImport::pushParent(uno::Reference<drawing::XShapes> xParent)
{
    m_aParents.push(xParent);
}

void RTFSdrImport::popParent()
{
    m_aParents.pop();
}

void RTFSdrImport::resolveDhgt(uno::Reference<beans::XPropertySet> xPropertySet, sal_Int32 nZOrder, bool bOldStyle)
{
    writerfilter::dmapper::DomainMapper& rMapper =
        dynamic_cast<writerfilter::dmapper::DomainMapper&>(m_rImport.Mapper());
    writerfilter::dmapper::GraphicZOrderHelper* pHelper = rMapper.graphicZOrderHelper();
    xPropertySet->setPropertyValue("ZOrder", uno::makeAny(pHelper->findZOrder(nZOrder, bOldStyle)));
    pHelper->addItem(xPropertySet, nZOrder);
}

void RTFSdrImport::resolveFLine(uno::Reference<beans::XPropertySet> xPropertySet, sal_Int32 nFLine)
{
    if (nFLine == 0)
        xPropertySet->setPropertyValue("LineStyle", uno::makeAny(drawing::LineStyle_NONE));
}

void RTFSdrImport::applyProperty(uno::Reference<drawing::XShape> xShape, const OUString& aKey, const OUString& aValue)
{
    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
    sal_Int16 nHoriOrient = 0;
    sal_Int16 nVertOrient = 0;
    boost::optional<bool> obFitShapeToText;
    bool bFilled = true;

    if (aKey == "posh")
    {
        switch (aValue.toInt32())
        {
        case 1:
            nHoriOrient = text::HoriOrientation::LEFT;
            break;
        case 2:
            nHoriOrient = text::HoriOrientation::CENTER;
            break;
        case 3:
            nHoriOrient = text::HoriOrientation::RIGHT;
            break;
        case 4:
            nHoriOrient = text::HoriOrientation::INSIDE;
            break;
        case 5:
            nHoriOrient = text::HoriOrientation::OUTSIDE;
            break;
        default:
            break;
        }
    }
    else if (aKey == "posv")
    {
        switch (aValue.toInt32())
        {
        case 1:
            nVertOrient = text::VertOrientation::TOP;
            break;
        case 2:
            nVertOrient = text::VertOrientation::CENTER;
            break;
        case 3:
            nVertOrient = text::VertOrientation::BOTTOM;
            break;
        default:
            break;
        }
    }
    else if (aKey == "fFitShapeToText")
        obFitShapeToText.reset(aValue.toInt32() == 1);
    else if (aKey == "fFilled")
        bFilled = aValue.toInt32() == 1;
    else if (aKey == "rotation")
    {
        // See DffPropertyReader::Fix16ToAngle(): in RTF, positive rotation angles are clockwise, we have them as counter-clockwise.
        // Additionally, RTF type is 0..360*2^16, our is 0..360*100.
        sal_Int32 nRotation = aValue.toInt32()*100/65536;
        xPropertySet->setPropertyValue("RotateAngle", uno::makeAny(sal_Int32(NormAngle360(nRotation * -1))));
    }

    if (nHoriOrient != 0 && xPropertySet.is())
        xPropertySet->setPropertyValue("HoriOrient", uno::makeAny(nHoriOrient));
    if (nVertOrient != 0 && xPropertySet.is())
        xPropertySet->setPropertyValue("VertOrient", uno::makeAny(nVertOrient));
    if (obFitShapeToText && xPropertySet.is())
    {
        xPropertySet->setPropertyValue("SizeType", uno::makeAny(*obFitShapeToText ? text::SizeType::MIN : text::SizeType::FIX));
        xPropertySet->setPropertyValue("FrameIsAutomaticHeight", uno::makeAny(*obFitShapeToText));
    }
    if (!bFilled && xPropertySet.is())
    {
        if (m_bTextFrame)
            xPropertySet->setPropertyValue("BackColorTransparency", uno::makeAny(sal_Int32(100)));
        else
            xPropertySet->setPropertyValue("FillStyle", uno::makeAny(drawing::FillStyle_NONE));
    }
}

int RTFSdrImport::initShape(
    uno::Reference<drawing::XShape> & o_xShape,
    uno::Reference<beans::XPropertySet> & o_xPropSet,
    bool & o_rIsCustomShape,
    RTFShape const& rShape, bool const bClose, ShapeOrPict const shapeOrPict)
{
    assert(!o_xShape.is());
    assert(!o_xPropSet.is());
    o_rIsCustomShape = false;
    m_bFakePict = false;

    // first, find the shape type
    int nType = -1;
    std::vector< std::pair<OUString, OUString> >::const_iterator const iter(
        std::find_if(rShape.aProperties.begin(),
                     rShape.aProperties.end(),
                     boost::bind(&OUString::equals,
                         boost::bind(&std::pair<OUString, OUString>::first, _1),
                         OUString("shapeType"))));

    if (iter == rShape.aProperties.end())
    {
        if (SHAPE == shapeOrPict)
        {
            // The spec doesn't state what is the default for shapeType,
            // Word seems to implement it as a rectangle.
            nType = ESCHER_ShpInst_Rectangle;
        }
        else
        {   // pict is picture by default but can be a rectangle too fdo#79319
            nType = ESCHER_ShpInst_PictureFrame;
        }
    }
    else
    {
        nType = iter->second.toInt32();
        if (PICT == shapeOrPict && ESCHER_ShpInst_PictureFrame != nType)
        {
            m_bFakePict = true;
        }
    }

    switch (nType)
    {
    case ESCHER_ShpInst_PictureFrame:
        createShape("com.sun.star.drawing.GraphicObjectShape", o_xShape, o_xPropSet);
        break;
    case ESCHER_ShpInst_Line:
        createShape("com.sun.star.drawing.LineShape", o_xShape, o_xPropSet);
        break;
    case ESCHER_ShpInst_Rectangle:
    case ESCHER_ShpInst_TextBox:
        // If we're inside a groupshape, can't use text frames.
        if (!bClose && m_aParents.size() == 1)
        {
            createShape("com.sun.star.text.TextFrame", o_xShape, o_xPropSet);
            m_bTextFrame = true;
            std::vector<beans::PropertyValue> aDefaults = getTextFrameDefaults(true);
            for (size_t j = 0; j < aDefaults.size(); ++j)
                o_xPropSet->setPropertyValue(aDefaults[j].Name, aDefaults[j].Value);
            break;
        }
        // fall-through intended
    default:
        createShape("com.sun.star.drawing.CustomShape", o_xShape, o_xPropSet);
        o_rIsCustomShape = true;
        break;
    }

    // Defaults
    if (o_xPropSet.is() && !m_bTextFrame)
    {
        o_xPropSet->setPropertyValue("FillColor", uno::makeAny(sal_uInt32(
                        0xffffff))); // White in Word, kind of blue in Writer.
    }

    return nType;
}

void RTFSdrImport::resolve(RTFShape& rShape, bool bClose, ShapeOrPict const shapeOrPict)
{
    bool bPib = false;
    m_bTextFrame = false;

    uno::Reference<drawing::XShape> xShape;
    uno::Reference<beans::XPropertySet> xPropertySet;
    uno::Any aAny;
    beans::PropertyValue aPropertyValue;
    awt::Rectangle aViewBox;
    std::vector<beans::PropertyValue> aPathPropVec;
    // Default line color is black in Word, blue in Writer.
    uno::Any aLineColor = uno::makeAny(COL_BLACK);
    // Default line width is 0.75 pt (26 mm100) in Word, 0 in Writer.
    uno::Any aLineWidth = uno::makeAny(sal_Int32(26));
    text::WritingMode eWritingMode = text::WritingMode_LR_TB;
    // Groupshape support
    boost::optional<sal_Int32> oGroupLeft, oGroupTop, oGroupRight, oGroupBottom;
    boost::optional<sal_Int32> oRelLeft, oRelTop, oRelRight, oRelBottom;

    // Importing these are not trivial, let the VML import do the hard work.
    oox::vml::FillModel aFillModel; // Gradient.
    oox::vml::ShadowModel aShadowModel; // Shadow.

    bool bOpaque = true;

    boost::optional<sal_Int16> oRelativeWidth, oRelativeHeight;
    sal_Int16 nRelativeWidthRelation = text::RelOrientation::PAGE_FRAME;
    sal_Int16 nRelativeHeightRelation = text::RelOrientation::PAGE_FRAME;

    bool bCustom(false);
    int const nType =
        initShape(xShape, xPropertySet, bCustom, rShape, bClose, shapeOrPict);

    for (std::vector< std::pair<OUString, OUString> >::iterator i = rShape.aProperties.begin();
            i != rShape.aProperties.end(); ++i)
    {
        if (i->first == "shapeType")
        {
            continue; // ignore: already handled by initShape
        }
        else if (i->first == "wzName")
        {
            if (m_bTextFrame)
            {
                uno::Reference<container::XNamed> xNamed(xShape, uno::UNO_QUERY);
                xNamed->setName(i->second);
            }
            else
                xPropertySet->setPropertyValue("Name", uno::makeAny(i->second));
        }
        else if (i->first == "wzDescription")
            xPropertySet->setPropertyValue("Description", uno::makeAny(i->second));
        else if (i->first == "pib")
        {
            m_rImport.setDestinationText(i->second);
            bPib = true;
        }
        else if (i->first == "fillColor" && xPropertySet.is())
        {
            aAny <<= msfilter::util::BGRToRGB(i->second.toInt32());
            if (m_bTextFrame)
                xPropertySet->setPropertyValue("BackColor", aAny);
            else
                xPropertySet->setPropertyValue("FillColor", aAny);

            // fillType will decide, possible it'll be the start color of a gradient.
            aFillModel.moColor.set(OUString("#") + OStringToOUString(msfilter::util::ConvertColor(aAny.get<sal_Int32>()), RTL_TEXTENCODING_UTF8));
        }
        else if (i->first == "fillBackColor")
            // fillType will decide, possible it'll be the end color of a gradient.
            aFillModel.moColor2.set(OUString("#") + OStringToOUString(msfilter::util::ConvertColor(msfilter::util::BGRToRGB(i->second.toInt32())), RTL_TEXTENCODING_UTF8));
        else if (i->first == "lineColor")
            aLineColor <<= msfilter::util::BGRToRGB(i->second.toInt32());
        else if (i->first == "lineBackColor")
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
        else if (i->first == "lineWidth")
            aLineWidth <<= i->second.toInt32()/360;
        else if (i->first == "pVerticies")
        {
            uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aCoordinates;
            sal_Int32 nSize = 0; // Size of a token (its value is hardwired in the exporter)
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
                else if (aToken.getLength())
                {
                    // The coordinates are in an (x,y) form.
                    aToken = aToken.copy(1, aToken.getLength() - 2);
                    sal_Int32 nI = 0;
                    boost::optional<sal_Int32> oX;
                    boost::optional<sal_Int32> oY;
                    do
                    {
                        OUString aPoint = aToken.getToken(0, ',', nI);
                        if (!oX)
                            oX.reset(aPoint.toInt32());
                        else
                            oY.reset(aPoint.toInt32());
                    }
                    while (nI >= 0);
                    aCoordinates[nIndex].First.Value <<= *oX;
                    aCoordinates[nIndex].Second.Value <<= *oY;
                    nIndex++;
                }
            }
            while (nCharIndex >= 0);
            aPropertyValue.Name = "Coordinates";
            aPropertyValue.Value <<= aCoordinates;
            aPathPropVec.push_back(aPropertyValue);
        }
        else if (i->first == "pSegmentInfo")
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
        else if (i->first == "geoLeft")
            aViewBox.X = i->second.toInt32();
        else if (i->first == "geoTop")
            aViewBox.Y = i->second.toInt32();
        else if (i->first == "geoRight")
            aViewBox.Width = i->second.toInt32();
        else if (i->first == "geoBottom")
            aViewBox.Height = i->second.toInt32();
        else if (i->first == "dhgt")
        {
            // dhgt is Word 2007, \shpz is Word 97-2003, the later has priority.
            if (!rShape.oZ)
                resolveDhgt(xPropertySet, i->second.toInt32(), /*bOldStyle=*/false);
        }
        // These are in EMU, convert to mm100.
        else if (i->first == "dxTextLeft")
        {
            if (xPropertySet.is())
                xPropertySet->setPropertyValue("LeftBorderDistance", uno::makeAny(i->second.toInt32() / 360));
        }
        else if (i->first == "dyTextTop")
        {
            if (xPropertySet.is())
                xPropertySet->setPropertyValue("TopBorderDistance", uno::makeAny(i->second.toInt32() / 360));
        }
        else if (i->first == "dxTextRight")
        {
            if (xPropertySet.is())
                xPropertySet->setPropertyValue("RightBorderDistance", uno::makeAny(i->second.toInt32() / 360));
        }
        else if (i->first == "dyTextBottom")
        {
            if (xPropertySet.is())
                xPropertySet->setPropertyValue("BottomBorderDistance", uno::makeAny(i->second.toInt32() / 360));
        }
        else if (i->first == "dxWrapDistLeft")
        {
            if (xPropertySet.is())
                xPropertySet->setPropertyValue("LeftMargin", uno::makeAny(i->second.toInt32() / 360));
        }
        else if (i->first == "dyWrapDistTop")
        {
            if (xPropertySet.is())
                xPropertySet->setPropertyValue("TopMargin", uno::makeAny(i->second.toInt32() / 360));
        }
        else if (i->first == "dxWrapDistRight")
        {
            if (xPropertySet.is())
                xPropertySet->setPropertyValue("RightMargin", uno::makeAny(i->second.toInt32() / 360));
        }
        else if (i->first == "dyWrapDistBottom")
        {
            if (xPropertySet.is())
                xPropertySet->setPropertyValue("BottomMargin", uno::makeAny(i->second.toInt32() / 360));
        }
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
        else if (i->first == "fShadow" && xPropertySet.is())
        {
            if (i->second.toInt32() == 1)
                aShadowModel.mbHasShadow = true;
        }
        else if (i->first == "shadowColor")
            aShadowModel.moColor.set(OUString("#") + OStringToOUString(msfilter::util::ConvertColor(msfilter::util::BGRToRGB(i->second.toInt32())), RTL_TEXTENCODING_UTF8));
        else if (i->first == "shadowOffsetX")
            // EMUs to points
            aShadowModel.moOffset.set(OUString::number(i->second.toDouble() / 12700) + "pt");
        else if (i->first == "posh" || i->first == "posv" || i->first == "fFitShapeToText" || i->first == "fFilled" || i->first == "rotation")
            applyProperty(xShape, i->first, i->second);
        else if (i->first == "posrelh")
        {
            switch (i->second.toInt32())
            {
            case 1:
                rShape.nHoriOrientRelation = text::RelOrientation::PAGE_FRAME;
                break;
            default:
                break;
            }
        }
        else if (i->first == "posrelv")
        {
            switch (i->second.toInt32())
            {
            case 1:
                rShape.nVertOrientRelation = text::RelOrientation::PAGE_FRAME;
                break;
            default:
                break;
            }
        }
        else if (i->first == "groupLeft")
            oGroupLeft.reset(convertTwipToMm100(i->second.toInt32()));
        else if (i->first == "groupTop")
            oGroupTop.reset(convertTwipToMm100(i->second.toInt32()));
        else if (i->first == "groupRight")
            oGroupRight.reset(convertTwipToMm100(i->second.toInt32()));
        else if (i->first == "groupBottom")
            oGroupBottom.reset(convertTwipToMm100(i->second.toInt32()));
        else if (i->first == "relLeft")
            oRelLeft.reset(convertTwipToMm100(i->second.toInt32()));
        else if (i->first == "relTop")
            oRelTop.reset(convertTwipToMm100(i->second.toInt32()));
        else if (i->first == "relRight")
            oRelRight.reset(convertTwipToMm100(i->second.toInt32()));
        else if (i->first == "relBottom")
            oRelBottom.reset(convertTwipToMm100(i->second.toInt32()));
        else if (i->first == "fBehindDocument")
            bOpaque = !i->second.toInt32();
        else if (i->first == "pctHoriz" || i->first == "pctVert")
        {
            sal_Int16 nPercentage = rtl::math::round(i->second.toDouble() / 10);
            boost::optional<sal_Int16>& rPercentage = i->first == "pctHoriz" ? oRelativeWidth : oRelativeHeight;
            if (nPercentage)
                rPercentage = nPercentage;
        }
        else if (i->first == "sizerelh")
        {
            if (xPropertySet.is())
            {
                switch (i->second.toInt32())
                {
                case 0: // margin
                    nRelativeWidthRelation = text::RelOrientation::FRAME;
                    break;
                case 1: // page
                    nRelativeWidthRelation = text::RelOrientation::PAGE_FRAME;
                    break;
                default:
                    SAL_WARN("writerfilter", "RTFSdrImport::resolve: unhandled sizerelh value: " << i->second);
                    break;
                }
            }
        }
        else if (i->first == "sizerelv")
        {
            if (xPropertySet.is())
            {
                switch (i->second.toInt32())
                {
                case 0: // margin
                    nRelativeHeightRelation = text::RelOrientation::FRAME;
                    break;
                case 1: // page
                    nRelativeHeightRelation = text::RelOrientation::PAGE_FRAME;
                    break;
                default:
                    SAL_WARN("writerfilter", "RTFSdrImport::resolve: unhandled sizerelv value: " << i->second);
                    break;
                }
            }
        }
        else if (i->first == "fHorizRule") // TODO: what does "fStandardHR" do?
        {   // horizontal rule: relative width defaults to 100% of paragraph
            // TODO: does it have a default height?
            if (!oRelativeWidth)
            {
                oRelativeWidth = 100;
            }
            nRelativeWidthRelation = text::RelOrientation::FRAME;
            sal_Int16 const nVertOrient = text::VertOrientation::CENTER;
            if (xPropertySet.is())
            {
                xPropertySet->setPropertyValue("VertOrient", uno::makeAny(nVertOrient));
            }
        }
        else if (i->first == "pctHR")
        {   // horizontal rule relative width in permille
            oRelativeWidth = i->second.toInt32() / 10;
        }
        else if (i->first == "dxHeightHR")
        {   // horizontal rule height
            sal_uInt32 const nHeight(convertTwipToMm100(i->second.toInt32()));
            rShape.nBottom = rShape.nTop + nHeight;
        }
        else if (i->first == "dxWidthHR")
        {   // horizontal rule width
            sal_uInt32 const nWidth(convertTwipToMm100(i->second.toInt32()));
            rShape.nRight = rShape.nLeft + nWidth;
        }
        else if (i->first == "alignHR")
        {   // horizontal orientation *for horizontal rule*
            sal_Int16 nHoriOrient = text::HoriOrientation::NONE;
            switch (i->second.toInt32())
            {
                case 0:
                    nHoriOrient = text::HoriOrientation::LEFT;
                    break;
                case 1:
                    nHoriOrient = text::HoriOrientation::CENTER;
                    break;
                case 2:
                    nHoriOrient = text::HoriOrientation::RIGHT;
                    break;
            }
            if (xPropertySet.is() && text::HoriOrientation::NONE != nHoriOrient)
            {
                xPropertySet->setPropertyValue("HoriOrient", uno::makeAny(nHoriOrient));
            }
        }
        else
            SAL_INFO("writerfilter", "TODO handle shape property '" << i->first << "':'" << i->second << "'");
    }

    if (xPropertySet.is())
    {
        if (!m_bTextFrame)
        {
            xPropertySet->setPropertyValue("LineColor", aLineColor);
            xPropertySet->setPropertyValue("LineWidth", aLineWidth);
        }
        else
        {
            static const OUString aBorders[] =
            {
                OUString("TopBorder"), OUString("LeftBorder"), OUString("BottomBorder"), OUString("RightBorder")
            };
            for (unsigned int i = 0; i < SAL_N_ELEMENTS(aBorders); ++i)
            {
                table::BorderLine2 aBorderLine = xPropertySet->getPropertyValue(aBorders[i]).get<table::BorderLine2>();
                aBorderLine.Color = aLineColor.get<sal_Int32>();
                aBorderLine.LineWidth = aLineWidth.get<sal_Int32>();
                xPropertySet->setPropertyValue(aBorders[i], uno::makeAny(aBorderLine));
            }
        }
        if (rShape.oZ)
            resolveDhgt(xPropertySet, *rShape.oZ, /*bOldStyle=*/false);
        if (m_bTextFrame)
            // Writer textframes implement text::WritingMode2, which is a different data type.
            xPropertySet->setPropertyValue("WritingMode", uno::makeAny(sal_Int16(eWritingMode)));
        else
            xPropertySet->setPropertyValue("TextWritingMode", uno::makeAny(eWritingMode));
    }

    if (m_aParents.size() && m_aParents.top().is() && !m_bTextFrame)
        m_aParents.top()->add(xShape);

    if (nType == ESCHER_ShpInst_PictureFrame) // picture frame
    {
        assert(!m_bTextFrame);
        if (bPib)
        {
            m_rImport.resolvePict(false, xShape);
        }
        else // ??? not sure if the early return should be removed on else?
        {
            m_xShape = xShape; // store it for later resolvePict call
        }
        return;
    }

    if (bCustom && xShape.is())
    {
        uno::Reference<drawing::XEnhancedCustomShapeDefaulter> xDefaulter(xShape, uno::UNO_QUERY);
        xDefaulter->createCustomShapeDefaults(OUString::number(nType));
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
        sal_Int32 nLeft = rShape.nLeft;
        sal_Int32 nTop = rShape.nTop;

        bool bInShapeGroup = oGroupLeft && oGroupTop && oGroupRight && oGroupBottom
                             && oRelLeft && oRelTop && oRelRight && oRelBottom;
        if (bInShapeGroup)
        {
            // See lclGetAbsPoint() in the VML import: rShape is the group shape, oGroup is its coordinate system, oRel is the relative child shape.
            sal_Int32 nShapeWidth = rShape.nRight - rShape.nLeft;
            sal_Int32 nShapeHeight = rShape.nBottom - rShape.nTop;
            sal_Int32 nCoordSysWidth = *oGroupRight - *oGroupLeft;
            sal_Int32 nCoordSysHeight = *oGroupBottom - *oGroupTop;
            double fWidthRatio = static_cast< double >(nShapeWidth) / nCoordSysWidth;
            double fHeightRatio = static_cast< double >(nShapeHeight) / nCoordSysHeight;
            nLeft = static_cast< sal_Int32 >(rShape.nLeft + fWidthRatio * (*oRelLeft - *oGroupLeft));
            nTop = static_cast< sal_Int32 >(rShape.nTop + fHeightRatio * (*oRelTop - *oGroupTop));
        }

        if (m_bTextFrame)
        {
            xPropertySet->setPropertyValue("HoriOrientPosition", uno::makeAny(nLeft));
            xPropertySet->setPropertyValue("VertOrientPosition", uno::makeAny(nTop));
        }
        else
            xShape->setPosition(awt::Point(nLeft, nTop));

        if (bInShapeGroup)
            xShape->setSize(awt::Size(*oRelRight - *oRelLeft, *oRelBottom - *oRelTop));
        else
            xShape->setSize(awt::Size(rShape.nRight - rShape.nLeft, rShape.nBottom - rShape.nTop));

        if (rShape.nHoriOrientRelation != 0)
            xPropertySet->setPropertyValue("HoriOrientRelation", uno::makeAny(rShape.nHoriOrientRelation));
        if (rShape.nVertOrientRelation != 0)
            xPropertySet->setPropertyValue("VertOrientRelation", uno::makeAny(rShape.nVertOrientRelation));
        if (rShape.nWrap != -1)
            xPropertySet->setPropertyValue("Surround", uno::makeAny(text::WrapTextMode(rShape.nWrap)));
        oox::ModelObjectHelper aModelObjectHelper(m_rImport.getModelFactory());
        if (aFillModel.moType.has())
        {
            oox::drawingml::ShapePropertyMap aPropMap(aModelObjectHelper);
            aFillModel.pushToPropMap(aPropMap, m_rImport.getGraphicHelper());
            // Sets the FillStyle and FillGradient UNO properties.
            oox::PropertySet(xShape).setProperties(aPropMap);
        }

        if (aShadowModel.mbHasShadow)
        {
            oox::drawingml::ShapePropertyMap aPropMap(aModelObjectHelper);
            aShadowModel.pushToPropMap(aPropMap, m_rImport.getGraphicHelper());
            // Sets the ShadowFormat UNO property.
            oox::PropertySet(xShape).setProperties(aPropMap);
        }
        xPropertySet->setPropertyValue("AnchorType", uno::makeAny(text::TextContentAnchorType_AT_CHARACTER));
        xPropertySet->setPropertyValue("Opaque", uno::makeAny(bOpaque));
        if (oRelativeWidth)
        {
            xPropertySet->setPropertyValue("RelativeWidth", uno::makeAny(*oRelativeWidth));
            xPropertySet->setPropertyValue("RelativeWidthRelation", uno::makeAny(nRelativeWidthRelation));
        }
        if (oRelativeHeight)
        {
            xPropertySet->setPropertyValue("RelativeHeight", uno::makeAny(*oRelativeHeight));
            xPropertySet->setPropertyValue("RelativeHeightRelation", uno::makeAny(nRelativeHeightRelation));
        }
    }

    if (m_rImport.isInBackground())
    {
        RTFSprms aAttributes;
        aAttributes.set(NS_ooxml::LN_CT_Background_color, RTFValue::Pointer_t(new RTFValue(xPropertySet->getPropertyValue("FillColor").get<sal_Int32>())));
        writerfilter::Reference<Properties>::Pointer_t const pProperties(new RTFReferenceProperties(aAttributes));
        m_rImport.Mapper().props(pProperties);

        uno::Reference<lang::XComponent> xComponent(xShape, uno::UNO_QUERY);
        xComponent->dispose();
        return;
    }

    // Send it to dmapper
    m_rImport.Mapper().startShape(xShape);
    if (bClose)
    {
        m_rImport.Mapper().endShape();
    }
    m_xShape = xShape;
}

void RTFSdrImport::close()
{
    m_rImport.Mapper().endShape();
}

void RTFSdrImport::append(const OUString& aKey, const OUString& aValue)
{
    applyProperty(m_xShape, aKey, aValue);
}

void RTFSdrImport::appendGroupProperty(const OUString& aKey, const OUString& aValue)
{
    uno::Reference<drawing::XShape> xShape(m_aParents.top(), uno::UNO_QUERY);
    if (xShape.is())
        applyProperty(xShape, aKey, aValue);
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
