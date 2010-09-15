/* libwpg
 * Copyright (C) 2006 Ariya Hidayat (ariya@kde.org)
 * Copyright (C) 2006-2007 Fridrich Strba (fridrich.strba@bluewin.ch)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02111-1301 USA
 *
 * For further information visit http://libwpg.sourceforge.net
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#include "OdgExporter.hxx"
#include "filter/DocumentElement.hxx"
#include "filter/DocumentHandler.hxx"
#include <rtl/math.hxx>


OdgExporter::OdgExporter(DocumentHandler *pHandler):
    mpHandler(pHandler),
    m_fillRule(AlternatingFill),
    m_gradientIndex(1),
    m_dashIndex(1),
    m_styleIndex(1)
{
}

OdgExporter::~OdgExporter()
{
}

void OdgExporter::startDocument(double width, double height)
{
    m_gradientIndex = 1;
    m_dashIndex = 1;
    m_styleIndex = 1;

    mpHandler->startDocument();
    TagOpenElement tmpOfficeDocumentContent("office:document");
    tmpOfficeDocumentContent.addAttribute("xmlns:office", "urn:oasis:names:tc:opendocument:xmlns:office:1.0");
    tmpOfficeDocumentContent.addAttribute("xmlns:style", "urn:oasis:names:tc:opendocument:xmlns:style:1.0");
    tmpOfficeDocumentContent.addAttribute("xmlns:text", "urn:oasis:names:tc:opendocument:xmlns:text:1.0");
    tmpOfficeDocumentContent.addAttribute("xmlns:draw", "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0");
    tmpOfficeDocumentContent.addAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
    tmpOfficeDocumentContent.addAttribute("xmlns:svg", "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0");
    tmpOfficeDocumentContent.addAttribute("xmlns:fo", "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0");
    tmpOfficeDocumentContent.addAttribute("office:version", "1.0");
    tmpOfficeDocumentContent.write(mpHandler);

    TagOpenElement("office:styles").write(mpHandler);
    TagCloseElement("office:styles").write(mpHandler);

    TagOpenElement("office:automatic-styles").write(mpHandler);

    TagOpenElement tmpStylePageLayoutOpenElement("style:page-layout");
    tmpStylePageLayoutOpenElement.addAttribute("style:name", "PM0");
    tmpStylePageLayoutOpenElement.write(mpHandler);

    TagOpenElement tmpStylePageLayoutPropertiesOpenElement("style:page-layout-properties");
    tmpStylePageLayoutPropertiesOpenElement.addAttribute("fo:margin-top", "0cm");
    tmpStylePageLayoutPropertiesOpenElement.addAttribute("fo:margin-bottom", "0cm");
    tmpStylePageLayoutPropertiesOpenElement.addAttribute("fo:margin-left", "0cm");
    tmpStylePageLayoutPropertiesOpenElement.addAttribute("fo:margin-right", "0cm");
    WPXString sValue;
    sValue = doubleToString(2.54 * width); sValue.append("cm");
    tmpStylePageLayoutPropertiesOpenElement.addAttribute("fo:page-width", sValue);
    sValue = doubleToString(2.54 * height); sValue.append("cm");
    tmpStylePageLayoutPropertiesOpenElement.addAttribute("fo:page-height", sValue);
    tmpStylePageLayoutPropertiesOpenElement.addAttribute("style:print-orientation", "portrait");
    tmpStylePageLayoutPropertiesOpenElement.write(mpHandler);

    TagCloseElement("style:page-layout-properties").write(mpHandler);

    TagCloseElement("style:page-layout").write(mpHandler);

    TagOpenElement tmpStyleStyleOpenElement("style:style");
    tmpStyleStyleOpenElement.addAttribute("style:name", "dp1");
    tmpStyleStyleOpenElement.addAttribute("style:family", "drawing-page");
    tmpStyleStyleOpenElement.write(mpHandler);

    TagOpenElement tmpStyleDrawingPagePropertiesOpenElement("style:drawing-page-properties");
    tmpStyleDrawingPagePropertiesOpenElement.addAttribute("draw:background-size", "border");
    tmpStyleDrawingPagePropertiesOpenElement.addAttribute("draw:fill", "none");
    tmpStyleDrawingPagePropertiesOpenElement.write(mpHandler);

    TagCloseElement("style:drawing-page-properties").write(mpHandler);

    TagCloseElement("style:style").write(mpHandler);
}

void OdgExporter::endDocument()
{
    TagCloseElement("office:automatic-styles").write(mpHandler);

    TagOpenElement("office:master-styles").write(mpHandler);

    TagOpenElement tmpStyleMasterPageOpenElement("style:master-page");
    tmpStyleMasterPageOpenElement.addAttribute("style:name", "Default");
    tmpStyleMasterPageOpenElement.addAttribute("style:page-layout-name", "PM0");
    tmpStyleMasterPageOpenElement.addAttribute("draw:style-name", "dp1");
    tmpStyleMasterPageOpenElement.write(mpHandler);

    TagCloseElement("style:master-page").write(mpHandler);

    TagCloseElement("office:master-styles").write(mpHandler);

    TagOpenElement("office:body").write(mpHandler);

    TagOpenElement("office:drawing").write(mpHandler);

    TagOpenElement tmpDrawPageOpenElement("draw:page");
    tmpDrawPageOpenElement.addAttribute("draw:name", "page1");
    tmpDrawPageOpenElement.addAttribute("draw:style-name", "dp1");
    tmpDrawPageOpenElement.addAttribute("draw:master-page-name", "Default");
    tmpDrawPageOpenElement.write(mpHandler);

    for (std::vector<DocumentElement *>::const_iterator bodyIter = mpBodyElements.begin();
        bodyIter != mpBodyElements.end(); bodyIter++)
    {
        (*bodyIter)->write(mpHandler);
    }

    TagCloseElement("draw:page").write(mpHandler);
    TagCloseElement("office:drawing").write(mpHandler);
    TagCloseElement("office:body").write(mpHandler);
    TagCloseElement("office:document").write(mpHandler);

    mpHandler->endDocument();
}

void OdgExporter::setPen(const libwpg::WPGPen& pen)
{
    m_pen = pen;
}

void OdgExporter::setBrush(const libwpg::WPGBrush& brush)
{
    m_brush = brush;
}

void OdgExporter::setFillRule(FillRule rule)
{
    m_fillRule = rule;
}

void OdgExporter::startLayer(unsigned int /* id */)
{
}

void OdgExporter::endLayer(unsigned int)
{
}

void OdgExporter::drawRectangle(const libwpg::WPGRect& rect, double rx, double /* ry */)
{
    writeStyle();
    TagOpenElement *pDrawRectElement = new TagOpenElement("draw:rect");
    WPXString sValue;
    sValue.sprintf("gr%i", m_styleIndex-1);
    pDrawRectElement->addAttribute("draw:style-name", sValue);
    sValue = doubleToString(2.54 * rect.x1); sValue.append("cm");
    pDrawRectElement->addAttribute("svg:x", sValue);
    sValue = doubleToString(2.54 * rect.y1); sValue.append("cm");
    pDrawRectElement->addAttribute("svg:y", sValue);
    sValue = doubleToString(2.54 * (rect.x2-rect.x1)); sValue.append("cm");
    pDrawRectElement->addAttribute("svg:width", sValue);
    sValue = doubleToString(2.54 * (rect.y2-rect.y1)); sValue.append("cm");
    pDrawRectElement->addAttribute("svg:height", sValue);
    sValue = doubleToString(2.54 * rx); sValue.append("cm");
    // FIXME: what to do when rx != ry ?
    pDrawRectElement->addAttribute("draw:corner-radius", sValue);
    mpBodyElements.push_back(static_cast<DocumentElement *>(pDrawRectElement));
    mpBodyElements.push_back(static_cast<DocumentElement *>(new TagCloseElement("draw:rect")));
}

void OdgExporter::drawEllipse(const libwpg::WPGPoint& center, double rx, double ry)
{
    writeStyle();
    TagOpenElement *pDrawEllipseElement = new TagOpenElement("draw:ellipse");
    WPXString sValue;
    sValue.sprintf("gr%i", m_styleIndex-1);
    pDrawEllipseElement->addAttribute("draw:style-name", sValue);
    sValue = doubleToString(2.54 * (center.x-rx)); sValue.append("cm");
    pDrawEllipseElement->addAttribute("svg:x", sValue);
    sValue = doubleToString(2.54 * (center.y-ry)); sValue.append("cm");
    pDrawEllipseElement->addAttribute("svg:y", sValue);
    sValue = doubleToString(2 * 2.54 * rx); sValue.append("cm");
    pDrawEllipseElement->addAttribute("svg:width", sValue);
    sValue = doubleToString(2 * 2.54 * ry); sValue.append("cm");
    pDrawEllipseElement->addAttribute("svg:height", sValue);
    mpBodyElements.push_back(static_cast<DocumentElement *>(pDrawEllipseElement));
    mpBodyElements.push_back(static_cast<DocumentElement *>(new TagCloseElement("draw:ellipse")));
}

void OdgExporter::drawPolygon(const libwpg::WPGPointArray& vertices)
{
    if(vertices.count() < 2)
        return;

    if(vertices.count() == 2)
    {
        const libwpg::WPGPoint& p1 = vertices[0];
        const libwpg::WPGPoint& p2 = vertices[1];

        writeStyle();
        TagOpenElement *pDrawLineElement = new TagOpenElement("draw:line");
        WPXString sValue;
        sValue.sprintf("gr%i", m_styleIndex-1);
        pDrawLineElement->addAttribute("draw:style-name", sValue);
        pDrawLineElement->addAttribute("draw:text-style-name", "P1");
        pDrawLineElement->addAttribute("draw:layer", "layout");
        sValue = doubleToString(2.54 * p1.x); sValue.append("cm");
        pDrawLineElement->addAttribute("svg:x1", sValue);
        sValue = doubleToString(2.54 * p1.y); sValue.append("cm");
        pDrawLineElement->addAttribute("svg:y1", sValue);
        sValue = doubleToString(2.54 * p2.x); sValue.append("cm");
        pDrawLineElement->addAttribute("svg:x2", sValue);
        sValue = doubleToString(2.54 * p2.y); sValue.append("cm");
        pDrawLineElement->addAttribute("svg:y2", sValue);
        mpBodyElements.push_back(static_cast<DocumentElement *>(pDrawLineElement));
        mpBodyElements.push_back(static_cast<DocumentElement *>(new TagCloseElement("draw:line")));
    }
    else
    {
        // draw as path
        libwpg::WPGPath path;
        path.moveTo(vertices[0]);
        for(unsigned long ii = 1; ii < vertices.count(); ii++)
            path.lineTo(vertices[ii]);
        path.closed = true;
        drawPath(path);
    }
}

void OdgExporter::drawPath(const libwpg::WPGPath& path)
{
    if(path.count() == 0)
        return;

    // try to find the bounding box
    // this is simple convex hull technique, the bounding box might not be
    // accurate but that should be enough for this purpose
    libwpg::WPGPoint p = path.element(0).point;
    libwpg::WPGPoint q = path.element(0).point;
    for(unsigned k = 0; k < path.count(); k++)
    {
        libwpg::WPGPathElement element = path.element(k);
        p.x = (p.x > element.point.x) ? element.point.x : p.x;
        p.y = (p.y > element.point.y) ? element.point.y : p.y;
        q.x = (q.x < element.point.x) ? element.point.x : q.x;
        q.y = (q.y < element.point.y) ? element.point.y : q.y;
        if(element.type == libwpg::WPGPathElement::CurveToElement)
        {
            p.x = (p.x > element.extra1.x) ? element.extra1.x : p.x;
            p.y = (p.y > element.extra1.y) ? element.extra1.y : p.y;
            q.x = (q.x < element.extra1.x) ? element.extra1.x : q.x;
            q.y = (q.y < element.extra1.y) ? element.extra1.y : q.y;
            p.x = (p.x > element.extra2.x) ? element.extra2.x : p.x;
            p.y = (p.y > element.extra2.y) ? element.extra2.y : p.y;
            q.x = (q.x < element.extra2.x) ? element.extra2.x : q.x;
            q.y = (q.y < element.extra2.y) ? element.extra2.y : q.y;
        }
    }
    double vw = q.x - p.x;
    double vh = q.y - p.y;

    writeStyle();

    TagOpenElement *pDrawPathElement = new TagOpenElement("draw:path");
    WPXString sValue;
    sValue.sprintf("gr%i", m_styleIndex-1);
    pDrawPathElement->addAttribute("draw:style-name", sValue);
    pDrawPathElement->addAttribute("draw:text-style-name", "P1");
    pDrawPathElement->addAttribute("draw:layer", "layout");
    sValue = doubleToString(2.54 * p.x); sValue.append("cm");
    pDrawPathElement->addAttribute("svg:x", sValue);
    sValue = doubleToString(2.54 * p.y); sValue.append("cm");
    pDrawPathElement->addAttribute("svg:y", sValue);
    sValue = doubleToString(2.54 * vw); sValue.append("cm");
    pDrawPathElement->addAttribute("svg:width", sValue);
    sValue = doubleToString(2.54 * vh); sValue.append("cm");
    pDrawPathElement->addAttribute("svg:height", sValue);
    sValue.sprintf("%i %i %i %i", 0, 0, (int)(vw*2540), (int)(vh*2540));
    pDrawPathElement->addAttribute("svg:viewBox", sValue);

    sValue.clear();
    for(unsigned i = 0; i < path.count(); i++)
    {
        libwpg::WPGPathElement element = path.element(i);
        libwpg::WPGPoint point = element.point;
        WPXString sElement;
        switch(element.type)
        {
            // 2540 is 2.54*1000, 2.54 cm = 1 inch
            case libwpg::WPGPathElement::MoveToElement:
                sElement.sprintf("M%i %i", (int)((point.x-p.x)*2540), (int)((point.y-p.y)*2540));
                break;

            case libwpg::WPGPathElement::LineToElement:
                sElement.sprintf("L%i %i", (int)((point.x-p.x)*2540), (int)((point.y-p.y)*2540));
                break;

            case libwpg::WPGPathElement::CurveToElement:
                sElement.sprintf("C%i %i %i %i %i %i", (int)((element.extra1.x-p.x)*2540),
                (int)((element.extra1.y-p.y)*2540), (int)((element.extra2.x-p.x)*2540),
                (int)((element.extra2.y-p.y)*2540), (int)((point.x-p.x)*2540), (int)((point.y-p.y)*2540));
                break;

            default:
                break;
        }
        sValue.append(sElement);
    }
    if(path.closed)
        sValue.append(" Z");
    pDrawPathElement->addAttribute("svg:d", sValue);
    mpBodyElements.push_back(static_cast<DocumentElement *>(pDrawPathElement));
    mpBodyElements.push_back(static_cast<DocumentElement *>(new TagCloseElement("draw:path")));
}

void OdgExporter::drawBitmap(const libwpg::WPGBitmap& bitmap)
{
    TagOpenElement *pDrawFrameElement = new TagOpenElement("draw:frame");
    WPXString sValue;
    sValue = doubleToString(2.54 * bitmap.rect.x1); sValue.append("cm");
    pDrawFrameElement->addAttribute("svg:x", sValue);
    sValue = doubleToString(2.54 * bitmap.rect.y1); sValue.append("cm");
    pDrawFrameElement->addAttribute("svg:y", sValue);
    sValue = doubleToString(2.54 * bitmap.rect.height()); sValue.append("cm");
    pDrawFrameElement->addAttribute("svg:height", sValue);
    sValue = doubleToString(2.54 * bitmap.rect.width()); sValue.append("cm");
    pDrawFrameElement->addAttribute("svg:width", sValue);
    mpBodyElements.push_back(static_cast<DocumentElement *>(pDrawFrameElement));

    mpBodyElements.push_back(static_cast<DocumentElement *>(new TagOpenElement("draw:image")));

    mpBodyElements.push_back(static_cast<DocumentElement *>(new TagOpenElement("office:binary-data")));

    libwpg::WPGString base64Binary;
    bitmap.generateBase64DIB(base64Binary);
    mpBodyElements.push_back(static_cast<DocumentElement *>(new CharDataElement(base64Binary.cstr())));

    mpBodyElements.push_back(static_cast<DocumentElement *>(new TagCloseElement("office:binary-data")));

    mpBodyElements.push_back(static_cast<DocumentElement *>(new TagCloseElement("draw:image")));

    mpBodyElements.push_back(static_cast<DocumentElement *>(new TagCloseElement("draw:frame")));
}

void OdgExporter::drawImageObject(const libwpg::WPGBinaryData& binaryData)
{
    if (binaryData.mimeType.length() <= 0)
        return;

    TagOpenElement *pDrawFrameElement = new TagOpenElement("draw:frame");
    WPXString sValue;
    sValue = doubleToString(2.54 * binaryData.rect.x1); sValue.append("cm");
    pDrawFrameElement->addAttribute("svg:x", sValue);
    sValue = doubleToString(2.54 * binaryData.rect.y1); sValue.append("cm");
    pDrawFrameElement->addAttribute("svg:y", sValue);
    sValue = doubleToString(2.54 * binaryData.rect.height()); sValue.append("cm");
    pDrawFrameElement->addAttribute("svg:height", sValue);
    sValue = doubleToString(2.54 * binaryData.rect.width()); sValue.append("cm");
    pDrawFrameElement->addAttribute("svg:width", sValue);
    mpBodyElements.push_back(static_cast<DocumentElement *>(pDrawFrameElement));

    mpBodyElements.push_back(static_cast<DocumentElement *>(new TagOpenElement("draw:image")));

    mpBodyElements.push_back(static_cast<DocumentElement *>(new TagOpenElement("office:binary-data")));

    libwpg::WPGString base64Binary = binaryData.getBase64Data();;
    mpBodyElements.push_back(static_cast<DocumentElement *>(new CharDataElement(base64Binary.cstr())));

    mpBodyElements.push_back(static_cast<DocumentElement *>(new TagCloseElement("office:binary-data")));

    mpBodyElements.push_back(static_cast<DocumentElement *>(new TagCloseElement("draw:image")));

    mpBodyElements.push_back(static_cast<DocumentElement *>(new TagCloseElement("draw:frame")));
}

void OdgExporter::writeStyle()
{
    if(!m_pen.solid && (m_pen.dashArray.count() >=2 ) )
    {
        // ODG only supports dashes with the same length of spaces inbetween
        // here we take the first space and assume everything else the same
        // note that dash length is written in percentage
        double distance = m_pen.dashArray.at(1);
        TagOpenElement tmpDrawStrokeDashElement("draw:stroke-dash");
        tmpDrawStrokeDashElement.addAttribute("draw:style", "rect");
        WPXString sValue;
        sValue.sprintf("Dash_%i", m_dashIndex++);
        tmpDrawStrokeDashElement.addAttribute("draw:name", sValue);
        sValue.sprintf("%i \%", distance*100);
        tmpDrawStrokeDashElement.addAttribute("draw:distance", sValue);
        WPXString sName;
        for(unsigned i = 0; i < m_pen.dashArray.count()/2; i++)
        {
            sName.sprintf("draw:dots%i", i+1);
            tmpDrawStrokeDashElement.addAttribute(sName.cstr(), "1");
            sName.sprintf("draw:dots%i-length", i+1);
            sValue.sprintf("%i\%", 100*m_pen.dashArray.at(i*2));
            tmpDrawStrokeDashElement.addAttribute(sName.cstr(), sValue);
        }
        tmpDrawStrokeDashElement.write(mpHandler);
        TagCloseElement("draw:stroke-dash").write(mpHandler);
    }

    if(m_brush.style == libwpg::WPGBrush::Gradient)
    {
        TagOpenElement tmpDrawGradientElement("draw:gradient");
        tmpDrawGradientElement.addAttribute("draw:style", "linear");
        WPXString sValue;
        sValue.sprintf("Gradient_%i", m_gradientIndex++);
        tmpDrawGradientElement.addAttribute("draw:name", sValue);

        // ODG angle unit is 0.1 degree
        double angle = -m_brush.gradient.angle();
        while(angle < 0)
            angle += 360;
        while(angle > 360)
            angle -= 360;

        sValue.sprintf("%i", angle*10);
        tmpDrawGradientElement.addAttribute("draw:angle", sValue);

        libwpg::WPGColor startColor = m_brush.gradient.stopColor(0);
        libwpg::WPGColor stopColor = m_brush.gradient.stopColor(1);
        sValue.sprintf("#%.2x%.2x%.2x", (startColor.red & 0xff), (startColor.green & 0xff), (startColor.blue & 0xff));
        tmpDrawGradientElement.addAttribute("draw:start-color", sValue);
        sValue.sprintf("#%.2x%.2x%.2x", (stopColor.red & 0xff), (stopColor.green & 0xff), (stopColor.blue & 0xff));
        tmpDrawGradientElement.addAttribute("draw:end-color", sValue);
        tmpDrawGradientElement.addAttribute("draw:start-intensity", "100%");
        tmpDrawGradientElement.addAttribute("draw:end-intensity", "100%");
        tmpDrawGradientElement.addAttribute("draw:border", "0%");
        tmpDrawGradientElement.write(mpHandler);
        TagCloseElement("draw:gradient").write(mpHandler);
    }

    TagOpenElement tmpStyleStyleElement("style:style");
    WPXString sValue;
    sValue.sprintf("gr%i",  m_styleIndex);
    tmpStyleStyleElement.addAttribute("style:name", sValue);
    tmpStyleStyleElement.addAttribute("style:family", "graphic");
    tmpStyleStyleElement.addAttribute("style:parent-style-name", "standard");
    tmpStyleStyleElement.write(mpHandler);

    TagOpenElement tmpStyleGraphicPropertiesElement("style:graphic-properties");

    if(m_pen.width > 0.0)
    {
        sValue = doubleToString(2.54 * m_pen.width); sValue.append("cm");
        tmpStyleGraphicPropertiesElement.addAttribute("svg:stroke-width", sValue);
        sValue.sprintf("#%.2x%.2x%.2x", (m_pen.foreColor.red & 0xff),
                      (m_pen.foreColor.green & 0xff), (m_pen.foreColor.blue & 0xff));
        tmpStyleGraphicPropertiesElement.addAttribute("svg:stroke-color", sValue);

        if(!m_pen.solid)
        {
            tmpStyleGraphicPropertiesElement.addAttribute("draw:stroke", "dash");
            sValue.sprintf("Dash_%i", m_dashIndex-1);
            tmpStyleGraphicPropertiesElement.addAttribute("draw:stroke-dash", sValue);
            }
    }
    else
        tmpStyleGraphicPropertiesElement.addAttribute("draw:stroke", "none");

    if(m_brush.style == libwpg::WPGBrush::NoBrush)
        tmpStyleGraphicPropertiesElement.addAttribute("draw:fill", "none");

    if(m_brush.style == libwpg::WPGBrush::Solid)
    {
        tmpStyleGraphicPropertiesElement.addAttribute("draw:fill", "solid");
        sValue.sprintf("#%.2x%.2x%.2x", (m_brush.foreColor.red & 0xff),
                      (m_brush.foreColor.green & 0xff), (m_brush.foreColor.blue & 0xff));
        tmpStyleGraphicPropertiesElement.addAttribute("draw:fill-color", sValue);
    }

    if(m_brush.style == libwpg::WPGBrush::Gradient)
    {
        tmpStyleGraphicPropertiesElement.addAttribute("draw:fill", "gradient");
        sValue.sprintf("Gradient_%i", m_gradientIndex-1);
        tmpStyleGraphicPropertiesElement.addAttribute("draw:fill-gradient-name", sValue);
    }

    tmpStyleGraphicPropertiesElement.write(mpHandler);
    TagCloseElement("style:graphic-properties").write(mpHandler);

    TagCloseElement("style:style").write(mpHandler);
    m_styleIndex++;
}

WPXString OdgExporter::doubleToString(const double value)
{
    return WPXString((char *)::rtl::math::doubleToString(value, rtl_math_StringFormat_F, 4, '.').getStr());
}
