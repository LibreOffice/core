/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "rtfsdrimport.hxx"
#include <cmath>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <ooxml/resourceids.hxx>
#include <filter/msfilter/escherex.hxx>
#include <filter/msfilter/util.hxx>
#include <filter/msfilter/rtfutil.hxx>
#include <sal/log.hxx>
#include <svx/svdtrans.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/propertysequence.hxx>
#include "rtfreferenceproperties.hxx"
#include <oox/vml/vmlformatting.hxx>
#include <oox/helper/modelobjecthelper.hxx>
#include <oox/drawingml/shapepropertymap.hxx>
#include <oox/helper/propertyset.hxx>
#include <boost/logic/tribool.hpp>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <dmapper/GraphicZOrderHelper.hxx>
#include "rtfdocumentimpl.hxx"

using namespace com::sun::star;

namespace writerfilter
{
namespace rtftok
{
RTFSdrImport::RTFSdrImport(RTFDocumentImpl& rDocument,
                           uno::Reference<lang::XComponent> const& xDstDoc)
    : m_rImport(rDocument)
    , m_bTextFrame(false)
    , m_bTextGraphicObject(false)
    , m_bFakePict(false)
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawings(xDstDoc, uno::UNO_QUERY);
    if (xDrawings.is())
        m_aParents.push(xDrawings->getDrawPage());
    m_aGraphicZOrderHelpers.push(writerfilter::dmapper::GraphicZOrderHelper());
}

RTFSdrImport::~RTFSdrImport()
{
    if (!m_aGraphicZOrderHelpers.empty())
        m_aGraphicZOrderHelpers.pop();
    if (!m_aParents.empty())
        m_aParents.pop();
}

void RTFSdrImport::createShape(const OUString& rService, uno::Reference<drawing::XShape>& xShape,
                               uno::Reference<beans::XPropertySet>& xPropertySet)
{
    if (m_rImport.getModelFactory().is())
        xShape.set(m_rImport.getModelFactory()->createInstance(rService), uno::UNO_QUERY);
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

void RTFSdrImport::pushParent(uno::Reference<drawing::XShapes> const& xParent)
{
    m_aParents.push(xParent);
    m_aGraphicZOrderHelpers.push(writerfilter::dmapper::GraphicZOrderHelper());
}

void RTFSdrImport::popParent()
{
    if (!m_aGraphicZOrderHelpers.empty())
        m_aGraphicZOrderHelpers.pop();
    if (!m_aParents.empty())
        m_aParents.pop();
}

void RTFSdrImport::resolveDhgt(uno::Reference<beans::XPropertySet> const& xPropertySet,
                               sal_Int32 const nZOrder, bool const bOldStyle)
{
    if (!m_aGraphicZOrderHelpers.empty())
    {
        writerfilter::dmapper::GraphicZOrderHelper& rHelper = m_aGraphicZOrderHelpers.top();
        xPropertySet->setPropertyValue("ZOrder",
                                       uno::makeAny(rHelper.findZOrder(nZOrder, bOldStyle)));
        rHelper.addItem(xPropertySet, nZOrder);
    }
}

void RTFSdrImport::resolveLineColorAndWidth(bool bTextFrame,
                                            const uno::Reference<beans::XPropertySet>& xPropertySet,
                                            uno::Any const& rLineColor, uno::Any const& rLineWidth)
{
    if (!bTextFrame)
    {
        xPropertySet->setPropertyValue("LineColor", rLineColor);
        xPropertySet->setPropertyValue("LineWidth", rLineWidth);
    }
    else
    {
        static const char* aBorders[]
            = { "TopBorder", "LeftBorder", "BottomBorder", "RightBorder" };
        for (const char* pBorder : aBorders)
        {
            auto aBorderLine = xPropertySet->getPropertyValue(OUString::createFromAscii(pBorder))
                                   .get<table::BorderLine2>();
            if (rLineColor.hasValue())
                aBorderLine.Color = rLineColor.get<sal_Int32>();
            if (rLineWidth.hasValue())
                aBorderLine.LineWidth = rLineWidth.get<sal_Int32>();
            xPropertySet->setPropertyValue(OUString::createFromAscii(pBorder),
                                           uno::makeAny(aBorderLine));
        }
    }
}

void RTFSdrImport::resolveFLine(uno::Reference<beans::XPropertySet> const& xPropertySet,
                                sal_Int32 const nFLine)
{
    if (nFLine == 0)
        xPropertySet->setPropertyValue("LineStyle", uno::makeAny(drawing::LineStyle_NONE));
    else
        xPropertySet->setPropertyValue("LineStyle", uno::makeAny(drawing::LineStyle_SOLID));
}

void RTFSdrImport::applyProperty(uno::Reference<drawing::XShape> const& xShape,
                                 const OUString& aKey, const OUString& aValue)
{
    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
    sal_Int16 nHoriOrient = 0;
    sal_Int16 nVertOrient = 0;
    boost::logic::tribool obFitShapeToText(boost::logic::indeterminate);
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
        obFitShapeToText = aValue.toInt32() == 1;
    else if (aKey == "fFilled")
        bFilled = aValue.toInt32() == 1;
    else if (aKey == "rotation")
    {
        // See DffPropertyReader::Fix16ToAngle(): in RTF, positive rotation angles are clockwise, we have them as counter-clockwise.
        // Additionally, RTF type is 0..360*2^16, our is 0..360*100.
        sal_Int32 nRotation = aValue.toInt32() * 100 / RTF_MULTIPLIER;
        uno::Reference<lang::XServiceInfo> xServiceInfo(xShape, uno::UNO_QUERY);
        if (!xServiceInfo->supportsService("com.sun.star.text.TextFrame"))
            xPropertySet->setPropertyValue(
                "RotateAngle",
                uno::makeAny(sal_Int32(NormAngle36000(static_cast<long>(nRotation) * -1))));
    }

    if (nHoriOrient != 0 && xPropertySet.is())
        xPropertySet->setPropertyValue("HoriOrient", uno::makeAny(nHoriOrient));
    if (nVertOrient != 0 && xPropertySet.is())
        xPropertySet->setPropertyValue("VertOrient", uno::makeAny(nVertOrient));
    if (!boost::logic::indeterminate(obFitShapeToText) && xPropertySet.is())
    {
        xPropertySet->setPropertyValue(
            "SizeType", uno::makeAny(obFitShapeToText ? text::SizeType::MIN : text::SizeType::FIX));
        xPropertySet->setPropertyValue("FrameIsAutomaticHeight",
                                       uno::makeAny(static_cast<bool>(obFitShapeToText)));
    }
    if (!bFilled && xPropertySet.is())
    {
        if (m_bTextFrame)
            xPropertySet->setPropertyValue("BackColorTransparency", uno::makeAny(sal_Int32(100)));
        else
            xPropertySet->setPropertyValue("FillStyle", uno::makeAny(drawing::FillStyle_NONE));
    }
}

int RTFSdrImport::initShape(uno::Reference<drawing::XShape>& o_xShape,
                            uno::Reference<beans::XPropertySet>& o_xPropSet, bool& o_rIsCustomShape,
                            RTFShape const& rShape, bool const bClose,
                            ShapeOrPict const shapeOrPict)
{
    assert(!o_xShape.is());
    assert(!o_xPropSet.is());
    o_rIsCustomShape = false;
    m_bFakePict = false;

    // first, find the shape type
    int nType = -1;
    auto iter = std::find_if(
        rShape.getProperties().begin(), rShape.getProperties().end(),
        [](std::pair<OUString, OUString> aProperty) { return aProperty.first == "shapeType"; });

    if (iter == rShape.getProperties().end())
    {
        if (SHAPE == shapeOrPict)
        {
            // The spec doesn't state what is the default for shapeType,
            // Word seems to implement it as a rectangle.
            nType = ESCHER_ShpInst_Rectangle;
        }
        else
        {
            // pict is picture by default but can be a rectangle too fdo#79319
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
            m_bTextGraphicObject = true;
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
                for (beans::PropertyValue& i : aDefaults)
                    o_xPropSet->setPropertyValue(i.Name, i.Value);
                break;
            }
            [[fallthrough]];
        default:
            createShape("com.sun.star.drawing.CustomShape", o_xShape, o_xPropSet);
            o_rIsCustomShape = true;
            break;
    }

    // Defaults
    if (o_xPropSet.is() && !m_bTextFrame)
    {
        o_xPropSet->setPropertyValue(
            "FillColor",
            uno::makeAny(sal_uInt32(0xffffff))); // White in Word, kind of blue in Writer.
        o_xPropSet->setPropertyValue("VertOrient", uno::makeAny(text::VertOrientation::NONE));
    }

    return nType;
}

void RTFSdrImport::resolve(RTFShape& rShape, bool bClose, ShapeOrPict const shapeOrPict)
{
    bool bPib = false;
    m_bTextFrame = false;
    m_bTextGraphicObject = false;

    uno::Reference<drawing::XShape> xShape;
    uno::Reference<beans::XPropertySet> xPropertySet;
    uno::Any aAny;
    beans::PropertyValue aPropertyValue;
    awt::Rectangle aViewBox;
    std::vector<beans::PropertyValue> aPath;
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
    boost::logic::tribool obRelFlipV(boost::logic::indeterminate);
    boost::logic::tribool obFlipH(boost::logic::indeterminate);
    boost::logic::tribool obFlipV(boost::logic::indeterminate);

    OUString aShapeText = "";
    OUString aFontFamily = "";
    float nFontSize = 1.0;

    bool bCustom(false);
    int const nType = initShape(xShape, xPropertySet, bCustom, rShape, bClose, shapeOrPict);

    for (auto& rProperty : rShape.getProperties())
    {
        if (rProperty.first == "shapeType")
        {
            continue; // ignore: already handled by initShape
        }
        if (rProperty.first == "wzName")
        {
            if (m_bTextFrame)
            {
                uno::Reference<container::XNamed> xNamed(xShape, uno::UNO_QUERY);
                xNamed->setName(rProperty.second);
            }
            else
                xPropertySet->setPropertyValue("Name", uno::makeAny(rProperty.second));
        }
        else if (rProperty.first == "wzDescription")
            xPropertySet->setPropertyValue("Description", uno::makeAny(rProperty.second));
        else if (rProperty.first == "gtextUNICODE")
            aShapeText = rProperty.second;
        else if (rProperty.first == "gtextFont")
            aFontFamily = rProperty.second;
        else if (rProperty.first == "gtextSize")
        {
            // RTF size is multiplied by 2^16
            nFontSize = static_cast<float>(rProperty.second.toUInt32()) / RTF_MULTIPLIER;
        }
        else if (rProperty.first == "pib")
        {
            m_rImport.setDestinationText(rProperty.second);
            bPib = true;
        }
        else if (rProperty.first == "fillColor" && xPropertySet.is())
        {
            aAny <<= msfilter::util::BGRToRGB(rProperty.second.toUInt32());
            if (m_bTextFrame)
                xPropertySet->setPropertyValue("BackColor", aAny);
            else
                xPropertySet->setPropertyValue("FillColor", aAny);

            // fillType will decide, possible it'll be the start color of a gradient.
            aFillModel.moColor.set(
                "#" + OUString::fromUtf8(msfilter::util::ConvertColor(aAny.get<sal_Int32>())));
        }
        else if (rProperty.first == "fillBackColor")
            // fillType will decide, possible it'll be the end color of a gradient.
            aFillModel.moColor2.set("#"
                                    + OUString::fromUtf8(msfilter::util::ConvertColor(
                                          msfilter::util::BGRToRGB(rProperty.second.toInt32()))));
        else if (rProperty.first == "lineColor")
            aLineColor <<= msfilter::util::BGRToRGB(rProperty.second.toInt32());
        else if (rProperty.first == "lineBackColor")
            ; // Ignore: complementer of lineColor
        else if (rProperty.first == "txflTextFlow" && xPropertySet.is())
        {
            if (rProperty.second.toInt32() == 1)
                eWritingMode = text::WritingMode_TB_RL;
        }
        else if (rProperty.first == "fLine" && xPropertySet.is())
            resolveFLine(xPropertySet, rProperty.second.toInt32());
        else if (rProperty.first == "fillOpacity" && xPropertySet.is())
        {
            int opacity = 100 - (rProperty.second.toInt32()) * 100 / RTF_MULTIPLIER;
            xPropertySet->setPropertyValue("FillTransparence", uno::Any(sal_uInt32(opacity)));
        }
        else if (rProperty.first == "lineWidth")
            aLineWidth <<= rProperty.second.toInt32() / 360;
        else if (rProperty.first == "pVerticies")
        {
            std::vector<drawing::EnhancedCustomShapeParameterPair> aCoordinates;
            sal_Int32 nSize = 0; // Size of a token (its value is hardwired in the exporter)
            sal_Int32 nCount = 0; // Number of tokens
            sal_Int32 nCharIndex = 0; // Character index
            do
            {
                OUString aToken = rProperty.second.getToken(0, ';', nCharIndex);
                if (!nSize)
                    nSize = aToken.toInt32();
                else if (!nCount)
                    nCount = aToken.toInt32();
                else if (aToken.getLength())
                {
                    // The coordinates are in an (x,y) form.
                    aToken = aToken.copy(1, aToken.getLength() - 2);
                    sal_Int32 nI = 0;
                    sal_Int32 nX = aToken.getToken(0, ',', nI).toInt32();
                    sal_Int32 nY = (nI >= 0) ? aToken.getToken(0, ',', nI).toInt32() : 0;
                    drawing::EnhancedCustomShapeParameterPair aPair;
                    aPair.First.Value <<= nX;
                    aPair.Second.Value <<= nY;
                    aCoordinates.push_back(aPair);
                }
            } while (nCharIndex >= 0);
            aPropertyValue.Name = "Coordinates";
            aPropertyValue.Value <<= comphelper::containerToSequence(aCoordinates);
            aPath.push_back(aPropertyValue);
        }
        else if (rProperty.first == "pSegmentInfo")
        {
            std::vector<drawing::EnhancedCustomShapeSegment> aSegments;
            sal_Int32 nSize = 0;
            sal_Int32 nCount = 0;
            sal_Int32 nCharIndex = 0;
            do
            {
                sal_Int32 nSeg = rProperty.second.getToken(0, ';', nCharIndex).toInt32();
                if (!nSize)
                    nSize = nSeg;
                else if (!nCount)
                    nCount = nSeg;
                else
                {
                    sal_Int32 nPoints = 1;
                    if (nSeg >= 0x2000 && nSeg < 0x20FF)
                    {
                        nPoints = nSeg & 0x0FFF;
                        nSeg &= 0xFF00;
                    }

                    drawing::EnhancedCustomShapeSegment aSegment;
                    switch (nSeg)
                    {
                        case 0x0001: // lineto
                            aSegment.Command = drawing::EnhancedCustomShapeSegmentCommand::LINETO;
                            aSegment.Count = sal_Int32(1);
                            aSegments.push_back(aSegment);
                            break;
                        case 0x4000: // moveto
                            aSegment.Command = drawing::EnhancedCustomShapeSegmentCommand::MOVETO;
                            aSegment.Count = sal_Int32(1);
                            aSegments.push_back(aSegment);
                            break;
                        case 0x2000: // curveto
                            aSegment.Command = drawing::EnhancedCustomShapeSegmentCommand::CURVETO;
                            aSegment.Count = nPoints;
                            aSegments.push_back(aSegment);
                            break;
                        case 0xb300: // arcto
                            aSegment.Command = drawing::EnhancedCustomShapeSegmentCommand::ARCTO;
                            aSegment.Count = sal_Int32(0);
                            aSegments.push_back(aSegment);
                            break;
                        case 0xac00:
                        case 0xaa00: // nofill
                        case 0xab00: // nostroke
                        case 0x6001: // close
                            break;
                        case 0x8000: // end
                            aSegment.Command
                                = drawing::EnhancedCustomShapeSegmentCommand::ENDSUBPATH;
                            aSegment.Count = sal_Int32(0);
                            aSegments.push_back(aSegment);
                            break;
                        default: // given number of lineto elements
                            aSegment.Command = drawing::EnhancedCustomShapeSegmentCommand::LINETO;
                            aSegment.Count = nSeg;
                            aSegments.push_back(aSegment);
                            break;
                    }
                }
            } while (nCharIndex >= 0);
            aPropertyValue.Name = "Segments";
            aPropertyValue.Value <<= comphelper::containerToSequence(aSegments);
            aPath.push_back(aPropertyValue);
        }
        else if (rProperty.first == "geoLeft")
            aViewBox.X = rProperty.second.toInt32();
        else if (rProperty.first == "geoTop")
            aViewBox.Y = rProperty.second.toInt32();
        else if (rProperty.first == "geoRight")
            aViewBox.Width = rProperty.second.toInt32();
        else if (rProperty.first == "geoBottom")
            aViewBox.Height = rProperty.second.toInt32();
        else if (rProperty.first == "dhgt")
        {
            // dhgt is Word 2007, \shpz is Word 97-2003, the later has priority.
            if (!rShape.hasZ())
                resolveDhgt(xPropertySet, rProperty.second.toInt32(), /*bOldStyle=*/false);
        }
        // These are in EMU, convert to mm100.
        else if (rProperty.first == "dxTextLeft")
        {
            if (xPropertySet.is())
                xPropertySet->setPropertyValue("LeftBorderDistance",
                                               uno::makeAny(rProperty.second.toInt32() / 360));
        }
        else if (rProperty.first == "dyTextTop")
        {
            if (xPropertySet.is())
                xPropertySet->setPropertyValue("TopBorderDistance",
                                               uno::makeAny(rProperty.second.toInt32() / 360));
        }
        else if (rProperty.first == "dxTextRight")
        {
            if (xPropertySet.is())
                xPropertySet->setPropertyValue("RightBorderDistance",
                                               uno::makeAny(rProperty.second.toInt32() / 360));
        }
        else if (rProperty.first == "dyTextBottom")
        {
            if (xPropertySet.is())
                xPropertySet->setPropertyValue("BottomBorderDistance",
                                               uno::makeAny(rProperty.second.toInt32() / 360));
        }
        else if (rProperty.first == "dxWrapDistLeft")
        {
            if (m_bTextGraphicObject)
                rShape.getAnchorAttributes().set(NS_ooxml::LN_CT_Anchor_distL,
                                                 new RTFValue(rProperty.second.toInt32()));
            else if (xPropertySet.is())
                xPropertySet->setPropertyValue("LeftMargin",
                                               uno::makeAny(rProperty.second.toInt32() / 360));
        }
        else if (rProperty.first == "dyWrapDistTop")
        {
            if (m_bTextGraphicObject)
                rShape.getAnchorAttributes().set(NS_ooxml::LN_CT_Anchor_distT,
                                                 new RTFValue(rProperty.second.toInt32()));
            else if (xPropertySet.is())
                xPropertySet->setPropertyValue("TopMargin",
                                               uno::makeAny(rProperty.second.toInt32() / 360));
        }
        else if (rProperty.first == "dxWrapDistRight")
        {
            if (m_bTextGraphicObject)
                rShape.getAnchorAttributes().set(NS_ooxml::LN_CT_Anchor_distR,
                                                 new RTFValue(rProperty.second.toInt32()));
            else if (xPropertySet.is())
                xPropertySet->setPropertyValue("RightMargin",
                                               uno::makeAny(rProperty.second.toInt32() / 360));
        }
        else if (rProperty.first == "dyWrapDistBottom")
        {
            if (m_bTextGraphicObject)
                rShape.getAnchorAttributes().set(NS_ooxml::LN_CT_Anchor_distB,
                                                 new RTFValue(rProperty.second.toInt32()));
            else if (xPropertySet.is())
                xPropertySet->setPropertyValue("BottomMargin",
                                               uno::makeAny(rProperty.second.toInt32() / 360));
        }
        else if (rProperty.first == "fillType")
        {
            switch (rProperty.second.toInt32())
            {
                case 7: // Shade using the fillAngle
                    aFillModel.moType.set(oox::XML_gradient);
                    break;
                default:
                    SAL_INFO("writerfilter",
                             "TODO handle fillType value '" << rProperty.second << "'");
                    break;
            }
        }
        else if (rProperty.first == "fillFocus")
            aFillModel.moFocus.set(rProperty.second.toDouble() / 100); // percent
        else if (rProperty.first == "fShadow" && xPropertySet.is())
        {
            if (rProperty.second.toInt32() == 1)
                aShadowModel.mbHasShadow = true;
        }
        else if (rProperty.first == "shadowColor")
            aShadowModel.moColor.set("#"
                                     + OUString::fromUtf8(msfilter::util::ConvertColor(
                                           msfilter::util::BGRToRGB(rProperty.second.toInt32()))));
        else if (rProperty.first == "shadowOffsetX")
            // EMUs to points
            aShadowModel.moOffset.set(OUString::number(rProperty.second.toDouble() / 12700) + "pt");
        else if (rProperty.first == "posh" || rProperty.first == "posv"
                 || rProperty.first == "fFitShapeToText" || rProperty.first == "fFilled"
                 || rProperty.first == "rotation")
            applyProperty(xShape, rProperty.first, rProperty.second);
        else if (rProperty.first == "posrelh")
        {
            switch (rProperty.second.toInt32())
            {
                case 1:
                    rShape.setHoriOrientRelation(text::RelOrientation::PAGE_FRAME);
                    break;
                default:
                    break;
            }
        }
        else if (rProperty.first == "posrelv")
        {
            switch (rProperty.second.toInt32())
            {
                case 1:
                    rShape.setVertOrientRelation(text::RelOrientation::PAGE_FRAME);
                    break;
                default:
                    break;
            }
        }
        else if (rProperty.first == "groupLeft")
            oGroupLeft = convertTwipToMm100(rProperty.second.toInt32());
        else if (rProperty.first == "groupTop")
            oGroupTop = convertTwipToMm100(rProperty.second.toInt32());
        else if (rProperty.first == "groupRight")
            oGroupRight = convertTwipToMm100(rProperty.second.toInt32());
        else if (rProperty.first == "groupBottom")
            oGroupBottom = convertTwipToMm100(rProperty.second.toInt32());
        else if (rProperty.first == "relLeft")
            oRelLeft = convertTwipToMm100(rProperty.second.toInt32());
        else if (rProperty.first == "relTop")
            oRelTop = convertTwipToMm100(rProperty.second.toInt32());
        else if (rProperty.first == "relRight")
            oRelRight = convertTwipToMm100(rProperty.second.toInt32());
        else if (rProperty.first == "relBottom")
            oRelBottom = convertTwipToMm100(rProperty.second.toInt32());
        else if (rProperty.first == "fBehindDocument")
            bOpaque = !rProperty.second.toInt32();
        else if (rProperty.first == "pctHoriz" || rProperty.first == "pctVert")
        {
            sal_Int16 nPercentage = rtl::math::round(rProperty.second.toDouble() / 10);
            if (nPercentage)
            {
                boost::optional<sal_Int16>& rPercentage
                    = rProperty.first == "pctHoriz" ? oRelativeWidth : oRelativeHeight;
                rPercentage = nPercentage;
            }
        }
        else if (rProperty.first == "sizerelh")
        {
            if (xPropertySet.is())
            {
                switch (rProperty.second.toInt32())
                {
                    case 0: // margin
                        nRelativeWidthRelation = text::RelOrientation::FRAME;
                        break;
                    case 1: // page
                        nRelativeWidthRelation = text::RelOrientation::PAGE_FRAME;
                        break;
                    default:
                        SAL_WARN("writerfilter", "RTFSdrImport::resolve: unhandled sizerelh value: "
                                                     << rProperty.second);
                        break;
                }
            }
        }
        else if (rProperty.first == "sizerelv")
        {
            if (xPropertySet.is())
            {
                switch (rProperty.second.toInt32())
                {
                    case 0: // margin
                        nRelativeHeightRelation = text::RelOrientation::FRAME;
                        break;
                    case 1: // page
                        nRelativeHeightRelation = text::RelOrientation::PAGE_FRAME;
                        break;
                    default:
                        SAL_WARN("writerfilter", "RTFSdrImport::resolve: unhandled sizerelv value: "
                                                     << rProperty.second);
                        break;
                }
            }
        }
        else if (rProperty.first == "fHorizRule") // TODO: what does "fStandardHR" do?
        {
            // horizontal rule: relative width defaults to 100% of paragraph
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
        else if (rProperty.first == "pctHR")
        {
            // horizontal rule relative width in permille
            oRelativeWidth = rProperty.second.toInt32() / 10;
        }
        else if (rProperty.first == "dxHeightHR")
        {
            // horizontal rule height
            sal_uInt32 const nHeight(convertTwipToMm100(rProperty.second.toInt32()));
            rShape.setBottom(rShape.getTop() + nHeight);
        }
        else if (rProperty.first == "dxWidthHR")
        {
            // horizontal rule width
            sal_uInt32 const nWidth(convertTwipToMm100(rProperty.second.toInt32()));
            rShape.setRight(rShape.getLeft() + nWidth);
        }
        else if (rProperty.first == "alignHR")
        {
            // horizontal orientation *for horizontal rule*
            sal_Int16 nHoriOrient = text::HoriOrientation::NONE;
            switch (rProperty.second.toInt32())
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
        else if (rProperty.first == "pWrapPolygonVertices")
        {
            RTFSprms aPolygonSprms;
            sal_Int32 nSize = 0; // Size of a token
            sal_Int32 nCount = 0; // Number of tokens
            sal_Int32 nCharIndex = 0; // Character index
            do
            {
                OUString aToken = rProperty.second.getToken(0, ';', nCharIndex);
                if (!nSize)
                    nSize = aToken.toInt32();
                else if (!nCount)
                    nCount = aToken.toInt32();
                else if (aToken.getLength())
                {
                    // The coordinates are in an (x,y) form.
                    aToken = aToken.copy(1, aToken.getLength() - 2);
                    sal_Int32 nI = 0;
                    sal_Int32 nX = aToken.getToken(0, ',', nI).toInt32();
                    sal_Int32 nY = (nI >= 0) ? aToken.getToken(0, ',', nI).toInt32() : 0;
                    RTFSprms aPathAttributes;
                    aPathAttributes.set(NS_ooxml::LN_CT_Point2D_x, new RTFValue(nX));
                    aPathAttributes.set(NS_ooxml::LN_CT_Point2D_y, new RTFValue(nY));
                    aPolygonSprms.set(NS_ooxml::LN_CT_WrapPath_lineTo,
                                      new RTFValue(aPathAttributes), RTFOverwrite::NO_APPEND);
                }
            } while (nCharIndex >= 0);
            rShape.getWrapPolygonSprms() = aPolygonSprms;
        }
        else if (rProperty.first == "fRelFlipV")
            obRelFlipV = rProperty.second.toInt32() == 1;
        else if (rProperty.first == "fFlipH")
            obFlipH = rProperty.second.toInt32() == 1;
        else if (rProperty.first == "fFlipV")
            obFlipV = rProperty.second.toInt32() == 1;
        else
            SAL_INFO("writerfilter", "TODO handle shape property '" << rProperty.first << "':'"
                                                                    << rProperty.second << "'");
    }

    if (xPropertySet.is())
    {
        resolveLineColorAndWidth(m_bTextFrame, xPropertySet, aLineColor, aLineWidth);
        if (rShape.hasZ())
        {
            bool bOldStyle = m_aParents.size() > 1;
            resolveDhgt(xPropertySet, rShape.getZ(), bOldStyle);
        }
        if (m_bTextFrame)
            // Writer textframes implement text::WritingMode2, which is a different data type.
            xPropertySet->setPropertyValue("WritingMode", uno::makeAny(sal_Int16(eWritingMode)));
        else
            xPropertySet->setPropertyValue("TextWritingMode", uno::makeAny(eWritingMode));
    }

    if (!m_aParents.empty() && m_aParents.top().is() && !m_bTextFrame)
        m_aParents.top()->add(xShape);

    if (bPib)
    {
        m_rImport.resolvePict(false, xShape);
    }

    if (nType == ESCHER_ShpInst_PictureFrame) // picture frame
    {
        assert(!m_bTextFrame);
        if (!bPib) // ??? not sure if the early return should be removed on else?
        {
            m_xShape = xShape; // store it for later resolvePict call
        }

        // Handle horizontal flip.
        if (obFlipH == true && xPropertySet.is())
            xPropertySet->setPropertyValue("IsMirrored", uno::makeAny(true));
        return;
    }

    if (bCustom && xShape.is() && !bPib)
    {
        uno::Reference<drawing::XEnhancedCustomShapeDefaulter> xDefaulter(xShape, uno::UNO_QUERY);
        xDefaulter->createCustomShapeDefaults(OUString::number(nType));
    }

    // Set shape text
    if (bCustom && !aShapeText.isEmpty())
    {
        uno::Reference<text::XTextRange> xTextRange(xShape, uno::UNO_QUERY);
        if (xTextRange.is())
            xTextRange->setString(aShapeText);

        xPropertySet->setPropertyValue("CharFontName", uno::makeAny(aFontFamily));
        xPropertySet->setPropertyValue("CharHeight", uno::makeAny(nFontSize));
    }

    // Creating CustomShapeGeometry property
    std::vector<beans::PropertyValue> aGeometry;
    if (aViewBox.X || aViewBox.Y || aViewBox.Width || aViewBox.Height)
    {
        aViewBox.Width -= aViewBox.X;
        aViewBox.Height -= aViewBox.Y;
        aPropertyValue.Name = "ViewBox";
        aPropertyValue.Value <<= aViewBox;
        aGeometry.push_back(aPropertyValue);
    }
    if (!aPath.empty())
    {
        aPropertyValue.Name = "Path";
        aPropertyValue.Value <<= comphelper::containerToSequence(aPath);
        aGeometry.push_back(aPropertyValue);
    }
    if (!aGeometry.empty() && xPropertySet.is() && !m_bTextFrame)
        xPropertySet->setPropertyValue("CustomShapeGeometry",
                                       uno::Any(comphelper::containerToSequence(aGeometry)));

    if (!aShapeText.isEmpty())
    {
        auto aGeomPropSeq = xPropertySet->getPropertyValue("CustomShapeGeometry")
                                .get<uno::Sequence<beans::PropertyValue>>();
        auto aGeomPropVec
            = comphelper::sequenceToContainer<std::vector<beans::PropertyValue>>(aGeomPropSeq);
        uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence({
            { "TextPath", uno::makeAny(true) },
        }));
        auto it = std::find_if(
            aGeomPropVec.begin(), aGeomPropVec.end(),
            [](const beans::PropertyValue& rValue) { return rValue.Name == "TextPath"; });
        if (it == aGeomPropVec.end())
            aGeomPropVec.push_back(comphelper::makePropertyValue("TextPath", aPropertyValues));
        else
            it->Value <<= aPropertyValues;

        xPropertySet->setPropertyValue("CustomShapeGeometry",
                                       uno::makeAny(comphelper::containerToSequence(aGeomPropVec)));
        xPropertySet->setPropertyValue("TextAutoGrowHeight", uno::makeAny(false));
        xPropertySet->setPropertyValue("TextAutoGrowWidth", uno::makeAny(false));
    }

    if (!boost::logic::indeterminate(obRelFlipV) && xPropertySet.is())
    {
        if (nType == ESCHER_ShpInst_Line)
        {
            // Line shape inside group shape: get the polygon sequence and transform it.
            uno::Sequence<uno::Sequence<awt::Point>> aPolyPolySequence;
            if ((xPropertySet->getPropertyValue("PolyPolygon") >>= aPolyPolySequence)
                && aPolyPolySequence.hasElements())
            {
                uno::Sequence<awt::Point>& rPolygon = aPolyPolySequence[0];
                basegfx::B2DPolygon aPoly;
                for (sal_Int32 i = 0; i < rPolygon.getLength(); ++i)
                {
                    const awt::Point& rPoint = rPolygon[i];
                    aPoly.insert(i, basegfx::B2DPoint(rPoint.X, rPoint.Y));
                }
                basegfx::B2DHomMatrix aTransformation;
                aTransformation.scale(1.0, obRelFlipV ? -1.0 : 1.0);
                aPoly.transform(aTransformation);
                for (sal_Int32 i = 0; i < rPolygon.getLength(); ++i)
                {
                    basegfx::B2DPoint aPoint(aPoly.getB2DPoint(i));
                    rPolygon[i]
                        = awt::Point(static_cast<sal_Int32>(convertMm100ToTwip(aPoint.getX())),
                                     static_cast<sal_Int32>(convertMm100ToTwip(aPoint.getY())));
                }
                xPropertySet->setPropertyValue("PolyPolygon", uno::makeAny(aPolyPolySequence));
            }
        }
    }

    // Set position and size
    if (xShape.is())
    {
        sal_Int32 nLeft = rShape.getLeft();
        sal_Int32 nTop = rShape.getTop();

        bool bInShapeGroup = oGroupLeft && oGroupTop && oGroupRight && oGroupBottom && oRelLeft
                             && oRelTop && oRelRight && oRelBottom;
        awt::Size aSize;
        if (bInShapeGroup)
        {
            // See lclGetAbsPoint() in the VML import: rShape is the group shape, oGroup is its coordinate system, oRel is the relative child shape.
            sal_Int32 nShapeWidth = rShape.getRight() - rShape.getLeft();
            sal_Int32 nShapeHeight = rShape.getBottom() - rShape.getTop();
            sal_Int32 nCoordSysWidth = *oGroupRight - *oGroupLeft;
            sal_Int32 nCoordSysHeight = *oGroupBottom - *oGroupTop;
            double fWidthRatio = static_cast<double>(nShapeWidth) / nCoordSysWidth;
            double fHeightRatio = static_cast<double>(nShapeHeight) / nCoordSysHeight;
            nLeft = static_cast<sal_Int32>(rShape.getLeft()
                                           + fWidthRatio * (*oRelLeft - *oGroupLeft));
            nTop = static_cast<sal_Int32>(rShape.getTop() + fHeightRatio * (*oRelTop - *oGroupTop));

            // See lclGetAbsRect() in the VML import.
            aSize.Width = std::lround(fWidthRatio * (*oRelRight - *oRelLeft));
            aSize.Height = std::lround(fHeightRatio * (*oRelBottom - *oRelTop));
        }

        if (m_bTextFrame)
        {
            xPropertySet->setPropertyValue("HoriOrientPosition", uno::makeAny(nLeft));
            xPropertySet->setPropertyValue("VertOrientPosition", uno::makeAny(nTop));
        }
        else
            xShape->setPosition(awt::Point(nLeft, nTop));

        if (bInShapeGroup)
            xShape->setSize(aSize);
        else
            xShape->setSize(awt::Size(rShape.getRight() - rShape.getLeft(),
                                      rShape.getBottom() - rShape.getTop()));

        if (obFlipH == true || obFlipV == true)
        {
            // Line shapes have no CustomShapeGeometry.
            if (nType != ESCHER_ShpInst_Line)
            {
                // This has to be set after position and size is set, otherwise flip will affect the position.
                comphelper::SequenceAsHashMap aCustomShapeGeometry(
                    xPropertySet->getPropertyValue("CustomShapeGeometry"));
                if (obFlipH == true)
                    aCustomShapeGeometry["MirroredX"] <<= true;
                if (obFlipV == true)
                    aCustomShapeGeometry["MirroredY"] <<= true;
                xPropertySet->setPropertyValue(
                    "CustomShapeGeometry",
                    uno::makeAny(aCustomShapeGeometry.getAsConstPropertyValueList()));
            }
        }

        if (rShape.getHoriOrientRelation() != 0)
            xPropertySet->setPropertyValue("HoriOrientRelation",
                                           uno::makeAny(rShape.getHoriOrientRelation()));
        if (rShape.getVertOrientRelation() != 0)
            xPropertySet->setPropertyValue("VertOrientRelation",
                                           uno::makeAny(rShape.getVertOrientRelation()));
        if (rShape.getWrap() != text::WrapTextMode::WrapTextMode_MAKE_FIXED_SIZE)
            xPropertySet->setPropertyValue("Surround", uno::makeAny(rShape.getWrap()));
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
        xPropertySet->setPropertyValue("AnchorType",
                                       uno::makeAny(text::TextContentAnchorType_AT_CHARACTER));
        xPropertySet->setPropertyValue("Opaque", uno::makeAny(bOpaque));
        if (oRelativeWidth)
        {
            xPropertySet->setPropertyValue("RelativeWidth", uno::makeAny(*oRelativeWidth));
            xPropertySet->setPropertyValue("RelativeWidthRelation",
                                           uno::makeAny(nRelativeWidthRelation));
        }
        if (oRelativeHeight)
        {
            xPropertySet->setPropertyValue("RelativeHeight", uno::makeAny(*oRelativeHeight));
            xPropertySet->setPropertyValue("RelativeHeightRelation",
                                           uno::makeAny(nRelativeHeightRelation));
        }
    }

    if (m_rImport.isInBackground())
    {
        RTFSprms aAttributes;
        aAttributes.set(NS_ooxml::LN_CT_Background_color,
                        new RTFValue(xPropertySet->getPropertyValue("FillColor").get<sal_Int32>()));
        m_rImport.Mapper().props(new RTFReferenceProperties(aAttributes));

        uno::Reference<lang::XComponent> xComponent(xShape, uno::UNO_QUERY);
        xComponent->dispose();
        return;
    }

    // Send it to dmapper
    if (xShape.is())
    {
        m_rImport.Mapper().startShape(xShape);
        if (bClose)
        {
            m_rImport.Mapper().endShape();
        }
    }
    m_xShape = xShape;
}

void RTFSdrImport::close() { m_rImport.Mapper().endShape(); }

void RTFSdrImport::append(const OUString& aKey, const OUString& aValue)
{
    applyProperty(m_xShape, aKey, aValue);
}

void RTFSdrImport::appendGroupProperty(const OUString& aKey, const OUString& aValue)
{
    if (m_aParents.empty())
        return;
    uno::Reference<drawing::XShape> xShape(m_aParents.top(), uno::UNO_QUERY);
    if (xShape.is())
        applyProperty(xShape, aKey, aValue);
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
