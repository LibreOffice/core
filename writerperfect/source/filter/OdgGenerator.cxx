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

#include "OdgGenerator.hxx"
#include "DocumentElement.hxx"
#include "OdfDocumentHandler.hxx"
#include <locale.h>
#include <math.h>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Workaround for the incapacity of draw to have multiple page
// sizes in the same document. Once that limitation is lifted,
// remove this
#define MULTIPAGE_WORKAROUND 1

static WPXString doubleToString(const double value)
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

class OdgGeneratorPrivate
{
public:
    OdgGeneratorPrivate(OdfDocumentHandler *pHandler, const OdfStreamType streamType);
    ~OdgGeneratorPrivate();
    void _writeGraphicsStyle();
    void _drawPolySomething(const ::WPXPropertyListVector& vertices, bool isClosed);
    void _drawPath(const WPXPropertyListVector& path);
    // body elements
    std::vector <DocumentElement *> mBodyElements;

    // graphics styles
    std::vector<DocumentElement *> mGraphicsStrokeDashStyles;
    std::vector<DocumentElement *> mGraphicsGradientStyles;
    std::vector<DocumentElement *> mGraphicsAutomaticStyles;

    // page styles
    std::vector<DocumentElement *> mPageAutomaticStyles;
    std::vector<DocumentElement *> mPageMasterStyles;

    OdfDocumentHandler *mpHandler;

    ::WPXPropertyList mxStyle;
    ::WPXPropertyListVector mxGradient;
    int miGradientIndex;
    int miDashIndex;
    int miGraphicsStyleIndex;
    int miPageIndex;
    double mfWidth, mfMaxWidth;
    double mfHeight, mfMaxHeight;

    const OdfStreamType mxStreamType;
};

OdgGeneratorPrivate::OdgGeneratorPrivate(OdfDocumentHandler *pHandler, const OdfStreamType streamType):
    mpHandler(pHandler),
    miGradientIndex(1),
    miDashIndex(1),
    miGraphicsStyleIndex(1),
    miPageIndex(1),
    mfWidth(0.0),
    mfMaxWidth(0.0),
    mfHeight(0.0),
    mfMaxHeight(0.0),
    mxStreamType(streamType)
{
}

OdgGeneratorPrivate::~OdgGeneratorPrivate()
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

    for (std::vector<DocumentElement *>::iterator iterPageAutomaticStyles = mPageAutomaticStyles.begin();
        iterPageAutomaticStyles != mPageAutomaticStyles.end(); ++iterPageAutomaticStyles)
    {
        delete((*iterPageAutomaticStyles));
    }

    for (std::vector<DocumentElement *>::iterator iterPageMasterStyles = mPageMasterStyles.begin();
        iterPageMasterStyles != mPageMasterStyles.end(); ++iterPageMasterStyles)
    {
        delete((*iterPageMasterStyles));
    }
}

OdgGenerator::OdgGenerator(OdfDocumentHandler *pHandler, const OdfStreamType streamType):
    mpImpl(new OdgGeneratorPrivate(pHandler, streamType))
{
    mpImpl->mpHandler->startDocument();
    TagOpenElement tmpOfficeDocumentContent(
        (mpImpl->mxStreamType == ODF_FLAT_XML) ? "office:document" : (
        (mpImpl->mxStreamType == ODF_CONTENT_XML) ? "office:document-content" : (
        (mpImpl->mxStreamType == ODF_STYLES_XML) ? "office:document-styles" : (
        (mpImpl->mxStreamType == ODF_SETTINGS_XML) ? "office:document-settings" : (
        (mpImpl->mxStreamType == ODF_META_XML) ? "office:document-meta" : "office:document" )))));
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
    if (mpImpl->mxStreamType == ODF_FLAT_XML)
        tmpOfficeDocumentContent.addAttribute("office:mimetype", "application/vnd.oasis.opendocument.graphics");
    tmpOfficeDocumentContent.write(mpImpl->mpHandler);
}

OdgGenerator::~OdgGenerator()
{
    if ((mpImpl->mxStreamType == ODF_FLAT_XML) || (mpImpl->mxStreamType == ODF_SETTINGS_XML))
    {
        TagOpenElement("office:settings").write(mpImpl->mpHandler);

        TagOpenElement configItemSetOpenElement("config:config-item-set");
        configItemSetOpenElement.addAttribute("config:name", "ooo:view-settings");
        configItemSetOpenElement.write(mpImpl->mpHandler);

        TagOpenElement configItemOpenElement("config:config-item");

        configItemOpenElement.addAttribute("config:name", "VisibleAreaTop");
        configItemOpenElement.addAttribute("config:type", "int");
        configItemOpenElement.write(mpImpl->mpHandler);
        mpImpl->mpHandler->characters("0");
        mpImpl->mpHandler->endElement("config:config-item");

        configItemOpenElement.addAttribute("config:name", "VisibleAreaLeft");
        configItemOpenElement.addAttribute("config:type", "int");
        configItemOpenElement.write(mpImpl->mpHandler);
        mpImpl->mpHandler->characters("0");
        mpImpl->mpHandler->endElement("config:config-item");

        configItemOpenElement.addAttribute("config:name", "VisibleAreaWidth");
        configItemOpenElement.addAttribute("config:type", "int");
        configItemOpenElement.write(mpImpl->mpHandler);
        WPXString sWidth; sWidth.sprintf("%li", (unsigned long)(2540 * mpImpl->mfMaxWidth));
        mpImpl->mpHandler->characters(sWidth);
        mpImpl->mpHandler->endElement("config:config-item");

        configItemOpenElement.addAttribute("config:name", "VisibleAreaHeight");
        configItemOpenElement.addAttribute("config:type", "int");
        configItemOpenElement.write(mpImpl->mpHandler);
        WPXString sHeight; sHeight.sprintf("%li", (unsigned long)(2540 * mpImpl->mfMaxHeight));
        mpImpl->mpHandler->characters(sHeight);
        mpImpl->mpHandler->endElement("config:config-item");

        mpImpl->mpHandler->endElement("config:config-item-set");

        mpImpl->mpHandler->endElement("office:settings");
    }


    if ((mpImpl->mxStreamType == ODF_FLAT_XML) || (mpImpl->mxStreamType == ODF_STYLES_XML))
    {
        TagOpenElement("office:styles").write(mpImpl->mpHandler);

        for (std::vector<DocumentElement *>::const_iterator iterGraphicsStrokeDashStyles = mpImpl->mGraphicsStrokeDashStyles.begin();
            iterGraphicsStrokeDashStyles != mpImpl->mGraphicsStrokeDashStyles.end(); ++iterGraphicsStrokeDashStyles)
        {
            (*iterGraphicsStrokeDashStyles)->write(mpImpl->mpHandler);
        }

        for (std::vector<DocumentElement *>::const_iterator iterGraphicsGradientStyles = mpImpl->mGraphicsGradientStyles.begin();
            iterGraphicsGradientStyles != mpImpl->mGraphicsGradientStyles.end(); ++iterGraphicsGradientStyles)
        {
            (*iterGraphicsGradientStyles)->write(mpImpl->mpHandler);
        }

        mpImpl->mpHandler->endElement("office:styles");
    }


    if ((mpImpl->mxStreamType == ODF_FLAT_XML) || (mpImpl->mxStreamType == ODF_CONTENT_XML) || (mpImpl->mxStreamType == ODF_STYLES_XML))
    {
        TagOpenElement("office:automatic-styles").write(mpImpl->mpHandler);
    }

    if ((mpImpl->mxStreamType == ODF_FLAT_XML) || (mpImpl->mxStreamType == ODF_CONTENT_XML))
    {
        // writing out the graphics automatic styles
        for (std::vector<DocumentElement *>::iterator iterGraphicsAutomaticStyles = mpImpl->mGraphicsAutomaticStyles.begin();
            iterGraphicsAutomaticStyles != mpImpl->mGraphicsAutomaticStyles.end(); ++iterGraphicsAutomaticStyles)
        {
            (*iterGraphicsAutomaticStyles)->write(mpImpl->mpHandler);
        }
    }
#ifdef MULTIPAGE_WORKAROUND
    if ((mpImpl->mxStreamType == ODF_FLAT_XML) || (mpImpl->mxStreamType == ODF_STYLES_XML))
    {
        TagOpenElement tmpStylePageLayoutOpenElement("style:page-layout");
        tmpStylePageLayoutOpenElement.addAttribute("style:name", "PM0");
        tmpStylePageLayoutOpenElement.write(mpImpl->mpHandler);

        TagOpenElement tmpStylePageLayoutPropertiesOpenElement("style:page-layout-properties");
        tmpStylePageLayoutPropertiesOpenElement.addAttribute("fo:margin-top", "0in");
        tmpStylePageLayoutPropertiesOpenElement.addAttribute("fo:margin-bottom", "0in");
        tmpStylePageLayoutPropertiesOpenElement.addAttribute("fo:margin-left", "0in");
        tmpStylePageLayoutPropertiesOpenElement.addAttribute("fo:margin-right", "0in");
        WPXString sValue;
        sValue = doubleToString(mpImpl->mfMaxWidth); sValue.append("in");
        tmpStylePageLayoutPropertiesOpenElement.addAttribute("fo:page-width", sValue);
        sValue = doubleToString(mpImpl->mfMaxHeight); sValue.append("in");
        tmpStylePageLayoutPropertiesOpenElement.addAttribute("fo:page-height", sValue);
        tmpStylePageLayoutPropertiesOpenElement.addAttribute("style:print-orientation", "portrait");
        tmpStylePageLayoutPropertiesOpenElement.write(mpImpl->mpHandler);

        mpImpl->mpHandler->endElement("style:page-layout-properties");

        mpImpl->mpHandler->endElement("style:page-layout");

        TagOpenElement tmpStyleStyleOpenElement("style:style");
        tmpStyleStyleOpenElement.addAttribute("style:name", "dp1");
        tmpStyleStyleOpenElement.addAttribute("style:family", "drawing-page");
        tmpStyleStyleOpenElement.write(mpImpl->mpHandler);

        TagOpenElement tmpStyleDrawingPagePropertiesOpenElement("style:drawing-page-properties");
        // tmpStyleDrawingPagePropertiesOpenElement.addAttribute("draw:background-size", "border");
        tmpStyleDrawingPagePropertiesOpenElement.addAttribute("draw:fill", "none");
        tmpStyleDrawingPagePropertiesOpenElement.write(mpImpl->mpHandler);

        mpImpl->mpHandler->endElement("style:drawing-page-properties");

        mpImpl->mpHandler->endElement("style:style");
    }
#else
    if ((mpImpl->mxStreamType == ODF_FLAT_XML) || (mpImpl->mxStreamType == ODF_STYLES_XML))
    {
        // writing out the page automatic styles
        for (std::vector<DocumentElement *>::iterator iterPageAutomaticStyles = mpImpl->mPageAutomaticStyles.begin();
            iterPageAutomaticStyles != mpImpl->mPageAutomaticStyles.end(); iterPageAutomaticStyles++)
        {
            (*iterPageAutomaticStyles)->write(mpImpl->mpHandler);
        }
    }
#endif
    if ((mpImpl->mxStreamType == ODF_FLAT_XML) || (mpImpl->mxStreamType == ODF_CONTENT_XML) || (mpImpl->mxStreamType == ODF_STYLES_XML))
    {
        mpImpl->mpHandler->endElement("office:automatic-styles");
    }

    if ((mpImpl->mxStreamType == ODF_FLAT_XML) || (mpImpl->mxStreamType == ODF_STYLES_XML))
    {
        TagOpenElement("office:master-styles").write(mpImpl->mpHandler);

        for (std::vector<DocumentElement *>::const_iterator pageMasterIter = mpImpl->mPageMasterStyles.begin();
            pageMasterIter != mpImpl->mPageMasterStyles.end(); ++pageMasterIter)
        {
            (*pageMasterIter)->write(mpImpl->mpHandler);
        }
        mpImpl->mpHandler->endElement("office:master-styles");
    }

    if ((mpImpl->mxStreamType == ODF_FLAT_XML) || (mpImpl->mxStreamType == ODF_CONTENT_XML))
    {
        TagOpenElement("office:body").write(mpImpl->mpHandler);

        TagOpenElement("office:drawing").write(mpImpl->mpHandler);

        for (std::vector<DocumentElement *>::const_iterator bodyIter = mpImpl->mBodyElements.begin();
            bodyIter != mpImpl->mBodyElements.end(); ++bodyIter)
        {
            (*bodyIter)->write(mpImpl->mpHandler);
        }

        mpImpl->mpHandler->endElement("office:drawing");
        mpImpl->mpHandler->endElement("office:body");
    }

    mpImpl->mpHandler->endElement(
        (mpImpl->mxStreamType == ODF_FLAT_XML) ? "office:document" : (
        (mpImpl->mxStreamType == ODF_CONTENT_XML) ? "office:document-content" : (
        (mpImpl->mxStreamType == ODF_STYLES_XML) ? "office:document-styles" : (
        (mpImpl->mxStreamType == ODF_SETTINGS_XML) ? "office:document-settings" : (
        (mpImpl->mxStreamType == ODF_META_XML) ? "office:document-meta" : "office:document" )))));

    mpImpl->mpHandler->endDocument();

    if (mpImpl)
        delete mpImpl;
}

void OdgGenerator::startGraphics(const ::WPXPropertyList &propList)
{
    if (propList["svg:width"])
    {
        mpImpl->mfWidth = propList["svg:width"]->getDouble();
        mpImpl->mfMaxWidth = mpImpl->mfMaxWidth < mpImpl->mfWidth ? mpImpl->mfWidth : mpImpl->mfMaxWidth;
    }

    if (propList["svg:height"])
    {
        mpImpl->mfHeight = propList["svg:height"]->getDouble();
        mpImpl->mfMaxHeight = mpImpl->mfMaxHeight < mpImpl->mfHeight ? mpImpl->mfHeight : mpImpl->mfMaxHeight;
    }

    TagOpenElement *pStyleMasterPageOpenElement = new TagOpenElement("style:master-page");

    TagOpenElement *pDrawPageOpenElement = new TagOpenElement("draw:page");

    TagOpenElement *pStylePageLayoutOpenElement = new TagOpenElement("style:page-layout");

    WPXString sValue;
    sValue.sprintf("page%i", mpImpl->miPageIndex);
    pDrawPageOpenElement->addAttribute("draw:name", sValue);
#ifdef MULTIPAGE_WORKAROUND
    pStyleMasterPageOpenElement->addAttribute("style:page-layout-name", "PM0");
    pStylePageLayoutOpenElement->addAttribute("style:page-layout-name", "PM0");
#else
    sValue.sprintf("PM%i", mpImpl->miPageIndex);
    pStyleMasterPageOpenElement->addAttribute("style:page-layout-name", sValue);
    pStylePageLayoutOpenElement->addAttribute("style:name", sValue);
#endif

    mpImpl->mPageAutomaticStyles.push_back(pStylePageLayoutOpenElement);

    TagOpenElement *pStylePageLayoutPropertiesOpenElement = new TagOpenElement("style:page-layout-properties");
    pStylePageLayoutPropertiesOpenElement->addAttribute("fo:margin-top", "0in");
    pStylePageLayoutPropertiesOpenElement->addAttribute("fo:margin-bottom", "0in");
    pStylePageLayoutPropertiesOpenElement->addAttribute("fo:margin-left", "0in");
    pStylePageLayoutPropertiesOpenElement->addAttribute("fo:margin-right", "0in");
    sValue.sprintf("%s%s", doubleToString(mpImpl->mfWidth).cstr(), "in");
    pStylePageLayoutPropertiesOpenElement->addAttribute("fo:page-width", sValue);
    sValue.sprintf("%s%s", doubleToString(mpImpl->mfHeight).cstr(), "in");
    pStylePageLayoutPropertiesOpenElement->addAttribute("fo:page-height", sValue);
    pStylePageLayoutPropertiesOpenElement->addAttribute("style:print-orientation", "portrait");
    mpImpl->mPageAutomaticStyles.push_back(pStylePageLayoutPropertiesOpenElement);

    mpImpl->mPageAutomaticStyles.push_back(new TagCloseElement("style:page-layout-properties"));

    mpImpl->mPageAutomaticStyles.push_back(new TagCloseElement("style:page-layout"));

#ifdef MULTIPAGE_WORKAROUND
    pDrawPageOpenElement->addAttribute("draw:style-name", "dp1");
    pStyleMasterPageOpenElement->addAttribute("draw:style-name", "dp1");
#else
    sValue.sprintf("dp%i", mpImpl->miPageIndex);
    pDrawPageOpenElement->addAttribute("draw:style-name", sValue);
    pStyleMasterPageOpenElement->addAttribute("draw:style-name", sValue);
#endif

    TagOpenElement *pStyleStyleOpenElement = new TagOpenElement("style:style");
    pStyleStyleOpenElement->addAttribute("style:name", sValue);
    pStyleStyleOpenElement->addAttribute("style:family", "drawing-page");
    mpImpl->mPageAutomaticStyles.push_back(pStyleStyleOpenElement);

#ifdef MULTIPAGE_WORKAROUND
    pDrawPageOpenElement->addAttribute("draw:master-page-name", "Default");
    pStyleMasterPageOpenElement->addAttribute("style:name", "Default");
#else
    sValue.sprintf("Page%i", mpImpl->miPageIndex);
    pDrawPageOpenElement->addAttribute("draw:master-page-name", sValue);
    pStyleMasterPageOpenElement->addAttribute("style:name", sValue);
#endif

    mpImpl->mBodyElements.push_back(pDrawPageOpenElement);

    mpImpl->mPageMasterStyles.push_back(pStyleMasterPageOpenElement);
    mpImpl->mPageMasterStyles.push_back(new TagCloseElement("style:master-page"));


    TagOpenElement *pStyleDrawingPagePropertiesOpenElement = new TagOpenElement("style:drawing-page-properties");
    // pStyleDrawingPagePropertiesOpenElement->addAttribute("draw:background-size", "border");
    pStyleDrawingPagePropertiesOpenElement->addAttribute("draw:fill", "none");
    mpImpl->mPageAutomaticStyles.push_back(pStyleDrawingPagePropertiesOpenElement);

    mpImpl->mPageAutomaticStyles.push_back(new TagCloseElement("style:drawing-page-properties"));

    mpImpl->mPageAutomaticStyles.push_back(new TagCloseElement("style:style"));
}

void OdgGenerator::endGraphics()
{
    mpImpl->mBodyElements.push_back(new TagCloseElement("draw:page"));
    mpImpl->miPageIndex++;
}

void OdgGenerator::setStyle(const ::WPXPropertyList & propList, const ::WPXPropertyListVector& gradient)
{
    mpImpl->mxStyle = propList;
    mpImpl->mxGradient = gradient;
}

void OdgGenerator::startLayer(const ::WPXPropertyList & /* propList */)
{
}

void OdgGenerator::endLayer()
{
}

void OdgGenerator::drawRectangle(const ::WPXPropertyList &propList)
{
    mpImpl->_writeGraphicsStyle();
    TagOpenElement *pDrawRectElement = new TagOpenElement("draw:rect");
    WPXString sValue;
    sValue.sprintf("gr%i", mpImpl->miGraphicsStyleIndex-1);
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
    mpImpl->mBodyElements.push_back(pDrawRectElement);
    mpImpl->mBodyElements.push_back(new TagCloseElement("draw:rect"));
}

void OdgGenerator::drawEllipse(const ::WPXPropertyList &propList)
{
    mpImpl->_writeGraphicsStyle();
    TagOpenElement *pDrawEllipseElement = new TagOpenElement("draw:ellipse");
    WPXString sValue;
    sValue.sprintf("gr%i", mpImpl->miGraphicsStyleIndex-1);
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
    mpImpl->mBodyElements.push_back(pDrawEllipseElement);
    mpImpl->mBodyElements.push_back(new TagCloseElement("draw:ellipse"));
}

void OdgGenerator::drawPolyline(const ::WPXPropertyListVector& vertices)
{
    mpImpl->_drawPolySomething(vertices, false);
}

void OdgGenerator::drawPolygon(const ::WPXPropertyListVector& vertices)
{
    mpImpl->_drawPolySomething(vertices, true);
}

void OdgGeneratorPrivate::_drawPolySomething(const ::WPXPropertyListVector& vertices, bool isClosed)
{
    if(vertices.count() < 2)
        return;

    if(vertices.count() == 2)
    {
        _writeGraphicsStyle();
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
        _drawPath(path);
    }
}

void OdgGeneratorPrivate::_drawPath(const WPXPropertyListVector& path)
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
#if 0
        if(path[k]["libwpg:path-action"]->getStr() == "A")
        {
            px = (px > path[k]["svg:x"]->getDouble()-2*path[k]["svg:rx"]->getDouble()) ? path[k]["svg:x"]->getDouble()-2*path[k]["svg:rx"]->getDouble() : px;
            py = (py > path[k]["svg:y"]->getDouble()-2*path[k]["svg:ry"]->getDouble()) ? path[k]["svg:y"]->getDouble()-2*path[k]["svg:ry"]->getDouble() : py;
            qx = (qx < path[k]["svg:x"]->getDouble()+2*path[k]["svg:rx"]->getDouble()) ? path[k]["svg:x"]->getDouble()+2*path[k]["svg:rx"]->getDouble() : qx;
            qy = (qy < path[k]["svg:y"]->getDouble()+2*path[k]["svg:ry"]->getDouble()) ? path[k]["svg:y"]->getDouble()+2*path[k]["svg:ry"]->getDouble() : qy;
        }
#endif
    }
    double vw = qx - px;
    double vh = qy - py;

    _writeGraphicsStyle();

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
        else if (path[i]["libwpg:path-action"]->getStr() == "L"
        // approximate for the time being the elliptic arc by a line
            || path[i]["libwpg:path-action"]->getStr() == "A")
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
#if 0
        else if (path[i]["libwpg:path-action"]->getStr() == "A")
        {
            sElement.sprintf("A%i %i %i %i %i %i %i", (unsigned)((path[i]["svg:rx"]->getDouble())*2540),
                (int)((path[i]["svg:ry"]->getDouble())*2540), (path[i]["libwpg:rotate"] ? path[i]["libwpg:rotate"]->getInt() : 0),
                0, 0, (unsigned)((path[i]["svg:x"]->getDouble()-px)*2540), (unsigned)((path[i]["svg:y"]->getDouble()-py)*2540));
            sValue.append(sElement);
        }
#endif
        else if (path[i]["libwpg:path-action"]->getStr() == "Z" && i >= (path.count() - 1))
            sValue.append(" Z");
    }
    pDrawPathElement->addAttribute("svg:d", sValue);
    mBodyElements.push_back(pDrawPathElement);
    mBodyElements.push_back(new TagCloseElement("draw:path"));
}

void OdgGenerator::drawPath(const WPXPropertyListVector& path)
{
    mpImpl->_drawPath(path);
}

void OdgGenerator::drawGraphicObject(const ::WPXPropertyList &propList, const ::WPXBinaryData& binaryData)
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
    mpImpl->mBodyElements.push_back(pDrawFrameElement);

    mpImpl->mBodyElements.push_back(new TagOpenElement("draw:image"));

    mpImpl->mBodyElements.push_back(new TagOpenElement("office:binary-data"));

    ::WPXString base64Binary = binaryData.getBase64Data();
    mpImpl->mBodyElements.push_back(new CharDataElement(base64Binary.cstr()));

    mpImpl->mBodyElements.push_back(new TagCloseElement("office:binary-data"));

    mpImpl->mBodyElements.push_back(new TagCloseElement("draw:image"));

    mpImpl->mBodyElements.push_back(new TagCloseElement("draw:frame"));
}

void OdgGeneratorPrivate::_writeGraphicsStyle()
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

void OdgGenerator::startEmbeddedGraphics(WPXPropertyList const&)
{
}

void OdgGenerator::endEmbeddedGraphics()
{
}

void OdgGenerator::startTextObject(WPXPropertyList const&, WPXPropertyListVector const&)
{
}

void OdgGenerator::endTextObject()
{
}

void OdgGenerator::startTextLine(WPXPropertyList const&)
{
}

void OdgGenerator::endTextLine()
{
}

void OdgGenerator::startTextSpan(WPXPropertyList const&)
{
}

void OdgGenerator::endTextSpan()
{
}

void OdgGenerator::insertText(WPXString const&)
{
}

