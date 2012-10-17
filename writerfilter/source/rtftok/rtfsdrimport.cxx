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

#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/WritingMode.hpp>

#include <ooxml/resourceids.hxx> // NS_ooxml namespace
#include <filter/msfilter/escherex.hxx>
#include <filter/msfilter/util.hxx>

#include <dmapper/DomainMapper.hxx>
#include "../dmapper/GraphicHelpers.hxx"
#include <rtfsdrimport.hxx>

using rtl::OString;
using rtl::OStringBuffer;
using rtl::OUString;
using rtl::OUStringBuffer;
using rtl::OUStringToOString;

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

void RTFSdrImport::resolveDhgt(uno::Reference<beans::XPropertySet> xPropertySet, sal_Int32 nZOrder)
{
    writerfilter::dmapper::DomainMapper& rMapper = (writerfilter::dmapper::DomainMapper&)m_rImport.Mapper();
    writerfilter::dmapper::GraphicZOrderHelper* pHelper = rMapper.graphicZOrderHelper();
    xPropertySet->setPropertyValue("ZOrder", uno::makeAny(pHelper->findZOrder(nZOrder)));
    pHelper->addItem(xPropertySet, nZOrder);
}

void RTFSdrImport::resolve(RTFShape& rShape)
{
    int nType = -1;
    bool bPib = false;
    bool bCustom = false;

    uno::Reference<drawing::XShape> xShape;
    uno::Reference<beans::XPropertySet> xPropertySet;
    // Create this early, as custom shapes may have properties before the type arrives.
    createShape("com.sun.star.drawing.CustomShape", xShape, xPropertySet);
    uno::Any aAny;
    beans::PropertyValue aPropertyValue;
    awt::Rectangle aViewBox;
    std::vector<beans::PropertyValue> aPathPropVec;

    for (std::vector< std::pair<rtl::OUString, rtl::OUString> >::iterator i = rShape.aProperties.begin();
            i != rShape.aProperties.end(); ++i)
    {
        if ( i->first == "shapeType" )
        {
            nType = i->second.toInt32();
            switch (nType)
            {
                case 20: // Line
                    createShape("com.sun.star.drawing.LineShape", xShape, xPropertySet);
                    break;
                default:
                    bCustom = true;
                    break;
            }

            // Defaults
            aAny <<= (sal_uInt32)0xffffff; // White in Word, kind of blue in Writer.
            if (xPropertySet.is())
                xPropertySet->setPropertyValue("FillColor", aAny);
        }
        else if ( i->first == "wzName" )
            xPropertySet->setPropertyValue("Name", uno::makeAny(i->second));
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
            xPropertySet->setPropertyValue("FillColor", aAny);
        }
        else if ( i->first == "fillBackColor" )
            ; // Ignore: complementer of fillColor
        else if (i->first == "lineColor" && xPropertySet.is())
        {
            aAny <<= msfilter::util::BGRToRGB(i->second.toInt32());
            xPropertySet->setPropertyValue("LineColor", aAny);
        }
        else if ( i->first == "lineBackColor" )
            ; // Ignore: complementer of lineColor
        else if (i->first == "txflTextFlow" && xPropertySet.is())
        {
            if (i->second.toInt32() == 1)
            {
                aAny <<= text::WritingMode_TB_RL;
                xPropertySet->setPropertyValue("TextWritingMode", aAny);
            }
        }
        else if (i->first == "fLine" && xPropertySet.is())
        {
            if (i->second.toInt32() == 0)
            {
                aAny <<= drawing::LineStyle_NONE;
                xPropertySet->setPropertyValue("LineStyle", aAny);
            }
        }
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
        else if (i->first == "lineWidth" && xPropertySet.is())
        {

            aAny <<= i->second.toInt32()/360;
            xPropertySet->setPropertyValue("LineWidth", aAny);
        }
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
            resolveDhgt(xPropertySet, i->second.toInt32());
        else
            SAL_INFO("writerfilter", OSL_THIS_FUNC << ": TODO handle shape property '" <<
                    OUStringToOString( i->first, RTL_TEXTENCODING_UTF8 ).getStr() << "':'" <<
                    OUStringToOString( i->second, RTL_TEXTENCODING_UTF8 ).getStr() << "'");
    }

    if (nType == ESCHER_ShpInst_PictureFrame) // picture frame
    {
        if (bPib)
            m_rImport.resolvePict(false);
        return;
    }

    if (m_xDrawPage.is())
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
        xShape->setPosition(awt::Point(rShape.nLeft, rShape.nTop));
        xShape->setSize(awt::Size(rShape.nRight - rShape.nLeft, rShape.nBottom - rShape.nTop));
        if (rShape.nHoriOrientRelation != 0)
            xPropertySet->setPropertyValue("HoriOrientRelation", uno::makeAny(rShape.nHoriOrientRelation));
        if (rShape.nVertOrientRelation != 0)
            xPropertySet->setPropertyValue("VertOrientRelation", uno::makeAny(rShape.nVertOrientRelation));
        if (rShape.nWrap != -1)
            xPropertySet->setPropertyValue("Surround", uno::makeAny(text::WrapTextMode(rShape.nWrap)));
    }

    // Send it to dmapper
    m_rImport.Mapper().startShape(xShape);
    m_rImport.replayShapetext();
    m_rImport.Mapper().endShape();
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
