/* libwpg
 * Copyright (C) 2006 Ariya Hidayat (ariya@kde.org)
 * Copyright (C) 2006 Fridrich Strba (fridrich.strba@bluewin.ch)
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
#include "DocumentElement.hxx"
#include "DocumentHandler.hxx"
#include <locale.h>
#include <math.h>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

OdgExporter::OdgExporter(DocumentHandlerInterface *pHandler):
    mpHandler(pHandler),
    miGradientIndex(1),
    miDashIndex(1),
    miGraphicsStyleIndex(1),
    mfWidth(0.0),
    mfHeight(0.0)
{
}

OdgExporter::~OdgExporter()
{

    for (std::vector<DocumentElement *>::iterator iterBody = mBodyElements.begin(); iterBody != mBodyElements.end(); ++iterBody)
    {
        delete (*iterBody);
        (*iterBody) = NULL;
    }

    for (std::vector<DocumentElement *>::iterator iterGraphicsAutomaticStyles = mGraphicsAutomaticStyles.begin();
        iterGraphicsAutomaticStyles != mGraphicsAutomaticStyles.end(); ++iterGraphicsAutomaticStyles)
    {
        delete((*iterGraphicsAutomaticStyles));
    }

    for (std::vector<DocumentElement *>::iterator iterGraphicsStrokeDashStyles = mGraphicsStrokeDashStyles.begin();
        iterGraphicsStrokeDashStyles != mGraphicsStrokeDashStyles.end(); ++iterGraphicsStrokeDashStyles)
    {
        delete((*iterGraphicsStrokeDashStyles));
    }

    for (std::vector<DocumentElement *>::iterator iterGraphicsGradientStyles = mGraphicsGradientStyles.begin();
        iterGraphicsGradientStyles != mGraphicsGradientStyles.end(); ++iterGraphicsGradientStyles)
    {
        delete((*iterGraphicsGradientStyles));
    }
}

void OdgExporter::startGraphics(const ::WPXPropertyList &propList)
{
    miGradientIndex = 1;
    miDashIndex = 1;
    miGraphicsStyleIndex = 1;
    mfWidth = 0.0;
    mfHeight = 0.0;

    if (propList["svg:width"])
        mfWidth = propList["svg:width"]->getDouble();

    if (propList["svg:height"])
        mfHeight = propList["svg:height"]->getDouble();

    mpHandler->startDocument();
    TagOpenElement tmpOfficeDocumentContent("office:document");
    tmpOfficeDocumentContent.addAttribute("xmlns:office", "urn:oasis:names:tc:opendocument:xmlns:office:1.0");
    tmpOfficeDocumentContent.addAttribute("xmlns:style", "urn:oasis:names:tc:opendocument:xmlns:style:1.0");
    tmpOfficeDocumentContent.addAttribute("xmlns:text", "urn:oasis:names:tc:opendocument:xmlns:text:1.0");
    tmpOfficeDocumentContent.addAttribute("xmlns:draw", "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0");
    tmpOfficeDocumentContent.addAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
    tmpOfficeDocumentContent.addAttribute("xmlns:svg", "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0");
    tmpOfficeDocumentContent.addAttribute("xmlns:fo", "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0");
    tmpOfficeDocumentContent.addAttribute("xmlns:config", "urn:oasis:names:tc:opendocument:xmlns:config:1.0");
    tmpOfficeDocumentContent.addAttribute("xmlns:ooo", "http://openoffice.org/2004/office");
    tmpOfficeDocumentContent.addAttribute("office:version", "1.0");
    tmpOfficeDocumentContent.addAttribute("office:mimetype", "application/vnd.oasis.opendocument.graphics");
    tmpOfficeDocumentContent.write(mpHandler);

    TagOpenElement("office:settings").write(mpHandler);

    TagOpenElement configItemSetOpenElement("config:config-item-set");
    configItemSetOpenElement.addAttribute("config:name", "ooo:view-settings");
    configItemSetOpenElement.write(mpHandler);

    TagOpenElement configItemOpenElement("config:config-item");

    configItemOpenElement.addAttribute("config:name", "VisibleAreaTop");
    configItemOpenElement.addAttribute("config:type", "int");
    configItemOpenElement.write(mpHandler);
    mpHandler->characters("0");
    mpHandler->endElement("config:config-item");

    configItemOpenElement.addAttribute("config:name", "VisibleAreaLeft");
    configItemOpenElement.addAttribute("config:type", "int");
    configItemOpenElement.write(mpHandler);
    mpHandler->characters("0");
    mpHandler->endElement("config:config-item");

    configItemOpenElement.addAttribute("config:name", "VisibleAreaWidth");
    configItemOpenElement.addAttribute("config:type", "int");
    configItemOpenElement.write(mpHandler);
    WPXString sWidth; sWidth.sprintf("%li", (unsigned long)(2540 * mfWidth));
    mpHandler->characters(sWidth);
    mpHandler->endElement("config:config-item");

    configItemOpenElement.addAttribute("config:name", "VisibleAreaHeight");
    configItemOpenElement.addAttribute("config:type", "int");
    configItemOpenElement.write(mpHandler);
    WPXString sHeight; sHeight.sprintf("%li", (unsigned long)(2540 * mfHeight));
    mpHandler->characters(sHeight);
    mpHandler->endElement("config:config-item");

    mpHandler->endElement("config:config-item-set");

    mpHandler->endElement("office:settings");
}

void OdgExporter::endGraphics()
{
    TagOpenElement("office:styles").write(mpHandler);

    for (std::vector<DocumentElement *>::const_iterator iterGraphicsStrokeDashStyles = mGraphicsStrokeDashStyles.begin();
        iterGraphicsStrokeDashStyles != mGraphicsStrokeDashStyles.end(); ++iterGraphicsStrokeDashStyles)
    {
        (*iterGraphicsStrokeDashStyles)->write(mpHandler);
    }

    for (std::vector<DocumentElement *>::const_iterator iterGraphicsGradientStyles = mGraphicsGradientStyles.begin();
        iterGraphicsGradientStyles != mGraphicsGradientStyles.end(); ++iterGraphicsGradientStyles)
    {
        (*iterGraphicsGradientStyles)->write(mpHandler);
    }

    mpHandler->endElement("office:styles");

    TagOpenElement("office:automatic-styles").write(mpHandler);

    // writing out the graphics automatic styles
    for (std::vector<DocumentElement *>::iterator iterGraphicsAutomaticStyles = mGraphicsAutomaticStyles.begin();
        iterGraphicsAutomaticStyles != mGraphicsAutomaticStyles.end(); ++iterGraphicsAutomaticStyles)
    {
        (*iterGraphicsAutomaticStyles)->write(mpHandler);
    }

    TagOpenElement tmpStylePageLayoutOpenElement("style:page-layout");
    tmpStylePageLayoutOpenElement.addAttribute("style:name", "PM0");
    tmpStylePageLayoutOpenElement.write(mpHandler);

    TagOpenElement tmpStylePageLayoutPropertiesOpenElement("style:page-layout-properties");
    tmpStylePageLayoutPropertiesOpenElement.addAttribute("fo:margin-top", "0in");
    tmpStylePageLayoutPropertiesOpenElement.addAttribute("fo:margin-bottom", "0in");
    tmpStylePageLayoutPropertiesOpenElement.addAttribute("fo:margin-left", "0in");
    tmpStylePageLayoutPropertiesOpenElement.addAttribute("fo:margin-right", "0in");
    WPXString sValue;
    sValue = doubleToString(mfWidth); sValue.append("in");
    tmpStylePageLayoutPropertiesOpenElement.addAttribute("fo:page-width", sValue);
    sValue = doubleToString(mfHeight); sValue.append("in");
    tmpStylePageLayoutPropertiesOpenElement.addAttribute("fo:page-height", sValue);
    tmpStylePageLayoutPropertiesOpenElement.addAttribute("style:print-orientation", "portrait");
    tmpStylePageLayoutPropertiesOpenElement.write(mpHandler);

    mpHandler->endElement("style:page-layout-properties");

    mpHandler->endElement("style:page-layout");

    TagOpenElement tmpStyleStyleOpenElement("style:style");
    tmpStyleStyleOpenElement.addAttribute("style:name", "dp1");
    tmpStyleStyleOpenElement.addAttribute("style:family", "drawing-page");
    tmpStyleStyleOpenElement.write(mpHandler);

    TagOpenElement tmpStyleDrawingPagePropertiesOpenElement("style:drawing-page-properties");
    // tmpStyleDrawingPagePropertiesOpenElement.addAttribute("draw:background-size", "border");
    tmpStyleDrawingPagePropertiesOpenElement.addAttribute("draw:fill", "none");
    tmpStyleDrawingPagePropertiesOpenElement.write(mpHandler);

    mpHandler->endElement("style:drawing-page-properties");

    mpHandler->endElement("style:style");

    mpHandler->endElement("office:automatic-styles");

    TagOpenElement("office:master-styles").write(mpHandler);

    TagOpenElement tmpStyleMasterPageOpenElement("style:master-page");
    tmpStyleMasterPageOpenElement.addAttribute("style:name", "Default");
    tmpStyleMasterPageOpenElement.addAttribute("style:page-layout-name", "PM0");
    tmpStyleMasterPageOpenElement.addAttribute("draw:style-name", "dp1");
    tmpStyleMasterPageOpenElement.write(mpHandler);

    mpHandler->endElement("style:master-page");

    mpHandler->endElement("office:master-styles");

    TagOpenElement("office:body").write(mpHandler);

    TagOpenElement("office:drawing").write(mpHandler);

    TagOpenElement tmpDrawPageOpenElement("draw:page");
    tmpDrawPageOpenElement.addAttribute("draw:name", "page1");
    tmpDrawPageOpenElement.addAttribute("draw:style-name", "dp1");
    tmpDrawPageOpenElement.addAttribute("draw:master-page-name", "Default");
    tmpDrawPageOpenElement.write(mpHandler);

    for (std::vector<DocumentElement *>::const_iterator bodyIter = mBodyElements.begin();
        bodyIter != mBodyElements.end(); ++bodyIter)
    {
        (*bodyIter)->write(mpHandler);
    }

    mpHandler->endElement("draw:page");
    mpHandler->endElement("office:drawing");
    mpHandler->endElement("office:body");

    mpHandler->endElement("office:document");

    mpHandler->endDocument();
}

void OdgExporter::setStyle(const ::WPXPropertyList & propList, const ::WPXPropertyListVector& gradient)
{
    mxStyle = propList;
    mxGradient = gradient;
}

void OdgExporter::startLayer(const ::WPXPropertyList & /* propList */)
{
}

void OdgExporter::endLayer()
{
}

void OdgExporter::drawRectangle(const ::WPXPropertyList &propList)
{
    writeGraphicsStyle();
    TagOpenElement *pDrawRectElement = new TagOpenElement("draw:rect");
    WPXString sValue;
    sValue.sprintf("gr%i", miGraphicsStyleIndex-1);
    pDrawRectElement->addAttribute("draw:style-name", sValue);
    pDrawRectElement->addAttribute("svg:x", propList["svg:x"]->getStr());
    pDrawRectElement->addAttribute("svg:y", propList["svg:y"]->getStr());
    pDrawRectElement->addAttribute("svg:width", propList["svg:width"]->getStr());
    pDrawRectElement->addAttribute("svg:height", propList["svg:height"]->getStr());
    // FIXME: what to do when rx != ry ?
    if (propList["svg:rx"])
        pDrawRectElement->addAttribute("draw:corner-radius", propList["svg:rx"]->getStr());
    else
        pDrawRectElement->addAttribute("draw:corner-radius", "0.0000in");
    mBodyElements.push_back(pDrawRectElement);
    mBodyElements.push_back(new TagCloseElement("draw:rect"));
}

void OdgExporter::drawEllipse(const ::WPXPropertyList &propList)
{
    writeGraphicsStyle();
    TagOpenElement *pDrawEllipseElement = new TagOpenElement("draw:ellipse");
    WPXString sValue;
    sValue.sprintf("gr%i", miGraphicsStyleIndex-1);
    pDrawEllipseElement->addAttribute("draw:style-name", sValue);
    sValue = doubleToString(2 * propList["svg:rx"]->getDouble()); sValue.append("in");
    pDrawEllipseElement->addAttribute("svg:width", sValue);
    sValue = doubleToString(2 * propList["svg:ry"]->getDouble()); sValue.append("in");
    pDrawEllipseElement->addAttribute("svg:height", sValue);
    if (propList["libwpg:rotate"] && propList["libwpg:rotate"]->getDouble() != 0.0)
    {
        double rotation = propList["libwpg:rotate"]->getDouble();
        while(rotation < -180)
            rotation += 360;
        while(rotation > 180)
            rotation -= 360;
        double radrotation = rotation*M_PI/180.0;
        double deltax = sqrt(pow(propList["svg:rx"]->getDouble(), 2.0)
            + pow(propList["svg:ry"]->getDouble(), 2.0))*cos(atan(propList["svg:ry"]->getDouble()/propList["svg:rx"]->getDouble())
            - radrotation ) - propList["svg:rx"]->getDouble();
        double deltay = sqrt(pow(propList["svg:rx"]->getDouble(), 2.0)
            + pow(propList["svg:ry"]->getDouble(), 2.0))*sin(atan(propList["svg:ry"]->getDouble()/propList["svg:rx"]->getDouble())
            - radrotation ) - propList["svg:ry"]->getDouble();
        sValue = "rotate("; sValue.append(doubleToString(radrotation)); sValue.append(") ");
        sValue.append("translate("); sValue.append(doubleToString(propList["svg:cx"]->getDouble() - propList["svg:rx"]->getDouble() - deltax));
        sValue.append("in, ");
        sValue.append(doubleToString(propList["svg:cy"]->getDouble() - propList["svg:ry"]->getDouble() - deltay)); sValue.append("in)");
        pDrawEllipseElement->addAttribute("draw:transform", sValue);
    }
    else
    {
        sValue = doubleToString(propList["svg:cx"]->getDouble()-propList["svg:rx"]->getDouble()); sValue.append("in");
        pDrawEllipseElement->addAttribute("svg:x", sValue);
        sValue = doubleToString(propList["svg:cy"]->getDouble()-propList["svg:ry"]->getDouble()); sValue.append("in");
        pDrawEllipseElement->addAttribute("svg:y", sValue);
    }
    mBodyElements.push_back(pDrawEllipseElement);
    mBodyElements.push_back(new TagCloseElement("draw:ellipse"));
}

void OdgExporter::drawPolyline(const ::WPXPropertyListVector& vertices)
{
    drawPolySomething(vertices, false);
}

void OdgExporter::drawPolygon(const ::WPXPropertyListVector& vertices)
{
    drawPolySomething(vertices, true);
}

void OdgExporter::drawPolySomething(const ::WPXPropertyListVector& vertices, bool isClosed)
{
    if(vertices.count() < 2)
        return;

    if(vertices.count() == 2)
    {
        writeGraphicsStyle();
        TagOpenElement *pDrawLineElement = new TagOpenElement("draw:line");
        WPXString sValue;
        sValue.sprintf("gr%i", miGraphicsStyleIndex-1);
        pDrawLineElement->addAttribute("draw:style-name", sValue);
        pDrawLineElement->addAttribute("draw:text-style-name", "P1");
        pDrawLineElement->addAttribute("draw:layer", "layout");
        pDrawLineElement->addAttribute("svg:x1", vertices[0]["svg:x"]->getStr());
        pDrawLineElement->addAttribute("svg:y1", vertices[0]["svg:y"]->getStr());
        pDrawLineElement->addAttribute("svg:x2", vertices[1]["svg:x"]->getStr());
        pDrawLineElement->addAttribute("svg:y2", vertices[1]["svg:y"]->getStr());
        mBodyElements.push_back(pDrawLineElement);
        mBodyElements.push_back(new TagCloseElement("draw:line"));
    }
    else
    {
        ::WPXPropertyListVector path;
        ::WPXPropertyList element;

        for (unsigned long ii = 0; ii < vertices.count(); ii++)
        {
            element = vertices[ii];
            if (ii == 0)
                element.insert("libwpg:path-action", "M");
            else
                element.insert("libwpg:path-action", "L");
            path.append(element);
            element.clear();
        }
        if (isClosed)
        {
            element.insert("libwpg:path-action", "Z");
            path.append(element);
        }
        drawPath(path);
    }
}

void OdgExporter::drawPath(const WPXPropertyListVector& path)
{
    if(path.count() == 0)
        return;

    // try to find the bounding box
    // this is simple convex hull technique, the bounding box might not be
    // accurate but that should be enough for this purpose
    double px = path[0]["svg:x"]->getDouble();
    double py = path[0]["svg:y"]->getDouble();
    double qx = path[0]["svg:x"]->getDouble();
    double qy = path[0]["svg:y"]->getDouble();
    for(unsigned k = 0; k < path.count(); k++)
    {
        if (!path[k]["svg:x"] || !path[k]["svg:y"])
            continue;
        px = (px > path[k]["svg:x"]->getDouble()) ? path[k]["svg:x"]->getDouble() : px;
        py = (py > path[k]["svg:y"]->getDouble()) ? path[k]["svg:y"]->getDouble() : py;
        qx = (qx < path[k]["svg:x"]->getDouble()) ? path[k]["svg:x"]->getDouble() : qx;
        qy = (qy < path[k]["svg:y"]->getDouble()) ? path[k]["svg:y"]->getDouble() : qy;
        if(path[k]["libwpg:path-action"]->getStr() == "C")
        {
            px = (px > path[k]["svg:x1"]->getDouble()) ? path[k]["svg:x1"]->getDouble() : px;
            py = (py > path[k]["svg:y1"]->getDouble()) ? path[k]["svg:y1"]->getDouble() : py;
            qx = (qx < path[k]["svg:x1"]->getDouble()) ? path[k]["svg:x1"]->getDouble() : qx;
            qy = (qy < path[k]["svg:y1"]->getDouble()) ? path[k]["svg:y1"]->getDouble() : qy;
            px = (px > path[k]["svg:x2"]->getDouble()) ? path[k]["svg:x2"]->getDouble() : px;
            py = (py > path[k]["svg:y2"]->getDouble()) ? path[k]["svg:y2"]->getDouble() : py;
            qx = (qx < path[k]["svg:x2"]->getDouble()) ? path[k]["svg:x2"]->getDouble() : qx;
            qy = (qy < path[k]["svg:y2"]->getDouble()) ? path[k]["svg:y2"]->getDouble() : qy;
        }
        if(path[k]["libwpg:path-action"]->getStr() == "A")
        {
            px = (px > path[k]["svg:x"]->getDouble()-2*path[k]["svg:rx"]->getDouble()) ? path[k]["svg:x"]->getDouble()-2*path[k]["svg:rx"]->getDouble() : px;
            py = (py > path[k]["svg:y"]->getDouble()-2*path[k]["svg:ry"]->getDouble()) ? path[k]["svg:y"]->getDouble()-2*path[k]["svg:ry"]->getDouble() : py;
            qx = (qx < path[k]["svg:x"]->getDouble()+2*path[k]["svg:rx"]->getDouble()) ? path[k]["svg:x"]->getDouble()+2*path[k]["svg:rx"]->getDouble() : qx;
            qy = (qy < path[k]["svg:y"]->getDouble()+2*path[k]["svg:ry"]->getDouble()) ? path[k]["svg:y"]->getDouble()+2*path[k]["svg:ry"]->getDouble() : qy;
        }
    }
    double vw = qx - px;
    double vh = qy - py;

    writeGraphicsStyle();

    TagOpenElement *pDrawPathElement = new TagOpenElement("draw:path");
    WPXString sValue;
    sValue.sprintf("gr%i", miGraphicsStyleIndex-1);
    pDrawPathElement->addAttribute("draw:style-name", sValue);
    pDrawPathElement->addAttribute("draw:text-style-name", "P1");
    pDrawPathElement->addAttribute("draw:layer", "layout");
    sValue = doubleToString(px); sValue.append("in");
    pDrawPathElement->addAttribute("svg:x", sValue);
    sValue = doubleToString(py); sValue.append("in");
    pDrawPathElement->addAttribute("svg:y", sValue);
    sValue = doubleToString(vw); sValue.append("in");
    pDrawPathElement->addAttribute("svg:width", sValue);
    sValue = doubleToString(vh); sValue.append("in");
    pDrawPathElement->addAttribute("svg:height", sValue);
    sValue.sprintf("%i %i %i %i", 0, 0, (unsigned)(vw*2540), (unsigned)(vh*2540));
    pDrawPathElement->addAttribute("svg:viewBox", sValue);

    sValue.clear();
    for(unsigned i = 0; i < path.count(); i++)
    {
        WPXString sElement;
        if (path[i]["libwpg:path-action"]->getStr() == "M")
        {
            // 2540 is 2.54*1000, 2.54 in = 1 inch
            sElement.sprintf("M%i %i", (unsigned)((path[i]["svg:x"]->getDouble()-px)*2540),
                (unsigned)((path[i]["svg:y"]->getDouble()-py)*2540));
            sValue.append(sElement);
        }
        else if (path[i]["libwpg:path-action"]->getStr() == "L")
        {
            sElement.sprintf("L%i %i", (unsigned)((path[i]["svg:x"]->getDouble()-px)*2540),
                (unsigned)((path[i]["svg:y"]->getDouble()-py)*2540));
            sValue.append(sElement);
        }
        else if (path[i]["libwpg:path-action"]->getStr() == "C")
        {
            sElement.sprintf("C%i %i %i %i %i %i", (unsigned)((path[i]["svg:x1"]->getDouble()-px)*2540),
                (int)((path[i]["svg:y1"]->getDouble()-py)*2540), (unsigned)((path[i]["svg:x2"]->getDouble()-px)*2540),
                (int)((path[i]["svg:y2"]->getDouble()-py)*2540), (unsigned)((path[i]["svg:x"]->getDouble()-px)*2540),
                (unsigned)((path[i]["svg:y"]->getDouble()-py)*2540));
            sValue.append(sElement);
        }
        else if (path[i]["libwpg:path-action"]->getStr() == "A")
        {
            sElement.sprintf("A%i %i %i %i %i %i %i", (unsigned)((path[i]["svg:rx"]->getDouble())*2540),
                (int)((path[i]["svg:ry"]->getDouble())*2540), (path[i]["libwpg:rotate"] ? path[i]["libwpg:rotate"]->getInt() : 0),
                0, 0, (unsigned)((path[i]["svg:x"]->getDouble()-px)*2540), (unsigned)((path[i]["svg:y"]->getDouble()-py)*2540));
            sValue.append(sElement);
        }
        else if (path[i]["libwpg:path-action"]->getStr() == "Z" && i >= (path.count() - 1))
            sValue.append(" Z");
    }
    pDrawPathElement->addAttribute("svg:d", sValue);
    mBodyElements.push_back(pDrawPathElement);
    mBodyElements.push_back(new TagCloseElement("draw:path"));
}

void OdgExporter::drawGraphicObject(const ::WPXPropertyList &propList, const ::WPXBinaryData& binaryData)
{
    if (!propList["libwpg:mime-type"] && propList["libwpg:mime-type"]->getStr().len() <= 0)
        return;
    TagOpenElement *pDrawFrameElement = new TagOpenElement("draw:frame");


    WPXString sValue;
    if (propList["svg:x"])
        pDrawFrameElement->addAttribute("svg:x", propList["svg:x"]->getStr());
    if (propList["svg:y"])
        pDrawFrameElement->addAttribute("svg:y", propList["svg:y"]->getStr());
    if (propList["svg:height"])
        pDrawFrameElement->addAttribute("svg:height", propList["svg:height"]->getStr());
    if (propList["svg:width"])
        pDrawFrameElement->addAttribute("svg:width", propList["svg:width"]->getStr());
    mBodyElements.push_back(pDrawFrameElement);

    mBodyElements.push_back(new TagOpenElement("draw:image"));

    mBodyElements.push_back(new TagOpenElement("office:binary-data"));

    ::WPXString base64Binary = binaryData.getBase64Data();
    mBodyElements.push_back(new CharDataElement(base64Binary.cstr()));

    mBodyElements.push_back(new TagCloseElement("office:binary-data"));

    mBodyElements.push_back(new TagCloseElement("draw:image"));

    mBodyElements.push_back(new TagCloseElement("draw:frame"));
}

void OdgExporter::writeGraphicsStyle()
{
#if 0
    if(mxStyle["libwpg:stroke-solid"] && !mxStyle["libwpg:stroke-solid"]->getInt() && (mxDashArray.count() >=2 ) )
    {
        // ODG only supports dashes with the same length of spaces inbetween
        // here we take the first space and assume everything else the same
        // note that dash length is written in percentage ?????????????????
        double distance = mxDashArray.at(1);
        TagOpenElement *pDrawStrokeDashElement = new TagOpenElement("draw:stroke-dash");
        pDrawStrokeDashElement->addAttribute("draw:style", "rect");
        WPXString sValue;
        sValue.sprintf("Dash_%i", miDashIndex++);
        pDrawStrokeDashElement->addAttribute("draw:name", sValue);
        sValue = doubleToString(distance); sValue.append("in");
        pDrawStrokeDashElement->addAttribute("draw:distance", sValue);
        WPXString sName;
        // We have to find out how to do this intelligently, since the ODF is allowing only
        // two pairs draw:dots1 draw:dots1-length and draw:dots2 draw:dots2-length
        for(unsigned i = 0; i < mxDashArray.count()/2 && i < 2; i++)
        {
            sName.sprintf("draw:dots%i", i+1);
            pDrawStrokeDashElement->addAttribute(sName.cstr(), "1");
            sName.sprintf("draw:dots%i-length", i+1);
            sValue = doubleToString(mxDashArray.at(i*2)); sValue.append("in");
            pDrawStrokeDashElement->addAttribute(sName.cstr(), sValue);
        }
        mGraphicsStrokeDashStyles.push_back(pDrawStrokeDashElement);
        mGraphicsStrokeDashStyles.push_back(new TagCloseElement("draw:stroke-dash"));
    }
#endif
    if(mxStyle["draw:fill"] && mxStyle["draw:fill"]->getStr() == "gradient" && mxGradient.count() >= 2)
    {
        TagOpenElement *pDrawGradientElement = new TagOpenElement("draw:gradient");
        pDrawGradientElement->addAttribute("draw:style", "linear");
        WPXString sValue;
        sValue.sprintf("Gradient_%i", miGradientIndex++);
        pDrawGradientElement->addAttribute("draw:name", sValue);

        // ODG angle unit is 0.1 degree
        double angle = mxStyle["draw:angle"] ? -mxStyle["draw:angle"]->getDouble() : 0.0;
        while(angle < 0)
            angle += 360;
        while(angle > 360)
            angle -= 360;

        sValue.sprintf("%i", (unsigned)(angle*10));
        pDrawGradientElement->addAttribute("draw:angle", sValue);

        pDrawGradientElement->addAttribute("draw:start-color", mxGradient[0]["svg:stop-color"]->getStr().cstr());
        pDrawGradientElement->addAttribute("draw:end-color", mxGradient[1]["svg:stop-color"]->getStr().cstr());
        pDrawGradientElement->addAttribute("draw:start-intensity", "100%");
        pDrawGradientElement->addAttribute("draw:end-intensity", "100%");
        pDrawGradientElement->addAttribute("draw:border", "0%");
        mGraphicsGradientStyles.push_back(pDrawGradientElement);
        mGraphicsGradientStyles.push_back(new TagCloseElement("draw:gradient"));
    }

    TagOpenElement *pStyleStyleElement = new TagOpenElement("style:style");
    WPXString sValue;
    sValue.sprintf("gr%i",  miGraphicsStyleIndex);
    pStyleStyleElement->addAttribute("style:name", sValue);
    pStyleStyleElement->addAttribute("style:family", "graphic");
    pStyleStyleElement->addAttribute("style:parent-style-name", "standard");
    mGraphicsAutomaticStyles.push_back(pStyleStyleElement);

    TagOpenElement *pStyleGraphicsPropertiesElement = new TagOpenElement("style:graphic-properties");

    if(!(mxStyle["draw:stroke"] && mxStyle["draw:stroke"]->getStr() == "none") && mxStyle["svg:stroke-width"] && mxStyle["svg:stroke-width"]->getDouble() > 0.0)
    {
        if (mxStyle["svg:stroke-width"])
            pStyleGraphicsPropertiesElement->addAttribute("svg:stroke-width", mxStyle["svg:stroke-width"]->getStr());

        if (mxStyle["svg:stroke-color"])
            pStyleGraphicsPropertiesElement->addAttribute("svg:stroke-color", mxStyle["svg:stroke-color"]->getStr());
        if (mxStyle["svg:stroke-opacity"] && mxStyle["svg:stroke-opacity"]->getDouble() != 1.0)
            pStyleGraphicsPropertiesElement->addAttribute("svg:stroke-opacity", mxStyle["svg:stroke-opacity"]->getStr());


        if(mxStyle["libwpg:stroke-solid"] && mxStyle["libwpg:stroke-solid"]->getInt())
            pStyleGraphicsPropertiesElement->addAttribute("draw:stroke", "solid");
#if 0
        else
        {
            pStyleGraphicsPropertiesElement->addAttribute("draw:stroke", "dash");
            sValue.sprintf("Dash_%i", miDashIndex-1);
            pStyleGraphicsPropertiesElement->addAttribute("draw:stroke-dash", sValue);
        }
#endif
    }
    else
        pStyleGraphicsPropertiesElement->addAttribute("draw:stroke", "none");

    if(mxStyle["draw:fill"] && mxStyle["draw:fill"]->getStr() == "none")
        pStyleGraphicsPropertiesElement->addAttribute("draw:fill", "none");

    if(mxStyle["draw:fill"] && mxStyle["draw:fill"]->getStr() == "solid")
    {
        pStyleGraphicsPropertiesElement->addAttribute("draw:fill", "solid");
        if (mxStyle["draw:fill-color"])
            pStyleGraphicsPropertiesElement->addAttribute("draw:fill-color", mxStyle["draw:fill-color"]->getStr());
        if (mxStyle["draw:opacity"] && mxStyle["draw:opacity"]->getDouble() != 1.0)
            pStyleGraphicsPropertiesElement->addAttribute("draw:opacity", mxStyle["draw:opacity"]->getStr());
    }

    if(mxStyle["draw:fill"] && mxStyle["draw:fill"]->getStr() == "gradient")
    {
        if (mxGradient.count() >= 2)
        {
            pStyleGraphicsPropertiesElement->addAttribute("draw:fill", "gradient");
            sValue.sprintf("Gradient_%i", miGradientIndex-1);
            pStyleGraphicsPropertiesElement->addAttribute("draw:fill-gradient-name", sValue);
        }
        else
            pStyleGraphicsPropertiesElement->addAttribute("draw:fill", "none");
    }

    mGraphicsAutomaticStyles.push_back(pStyleGraphicsPropertiesElement);
    mGraphicsAutomaticStyles.push_back(new TagCloseElement("style:graphic-properties"));

    mGraphicsAutomaticStyles.push_back(new TagCloseElement("style:style"));
    miGraphicsStyleIndex++;
}

WPXString OdgExporter::doubleToString(const double value)
{
    WPXString tempString;
    tempString.sprintf("%.4f", value);
    std::string decimalPoint(localeconv()->decimal_point);
    if ((decimalPoint.size() == 0) || (decimalPoint == "."))
        return tempString;
    std::string stringValue(tempString.cstr());
    if (!stringValue.empty())
    {
        std::string::size_type pos;
        while ((pos = stringValue.find(decimalPoint)) != std::string::npos)
            stringValue.replace(pos,decimalPoint.size(),".");
    }
    return WPXString(stringValue.c_str());
}
