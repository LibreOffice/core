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

static inline double getAngle(double bx, double by)
{
    return fmod(2*M_PI + (by > 0.0 ? 1.0 : -1.0) * acos( bx / sqrt(bx * bx + by * by) ), 2*M_PI);
}

static void getEllipticalArcBBox(double x1, double y1,
                                 double rx, double ry, double phi, bool largeArc, bool sweep, double x2, double y2,
                                 double &xmin, double &ymin, double &xmax, double &ymax)
{
    if (rx < 0.0)
        rx *= -1.0;
    if (ry < 0.0)
        ry *= -1.0;

    if (rx == 0.0 || ry == 0.0)
    {
        xmin = (x1 < x2 ? x1 : x2);
        xmax = (x1 > x2 ? x1 : x2);
        ymin = (y1 < y2 ? y1 : y2);
        ymax = (y1 > y2 ? y1 : y2);
        return;
    }

    // F.6.5.1
    const double x1prime = cos(phi)*(x1 - x2)/2 + sin(phi)*(y1 - y2)/2;
    const double y1prime = -sin(phi)*(x1 - x2)/2 + cos(phi)*(y1 - y2)/2;

    // F.6.5.2
    double radicant = (rx*rx*ry*ry - rx*rx*y1prime*y1prime - ry*ry*x1prime*x1prime)/(rx*rx*y1prime*y1prime + ry*ry*x1prime*x1prime);
    double cxprime = 0.0;
    double cyprime = 0.0;
    if (radicant < 0.0)
    {
        double ratio = rx/ry;
        radicant = y1prime*y1prime + x1prime*x1prime/(ratio*ratio);
        if (radicant < 0.0)
        {
            xmin = (x1 < x2 ? x1 : x2);
            xmax = (x1 > x2 ? x1 : x2);
            ymin = (y1 < y2 ? y1 : y2);
            ymax = (y1 > y2 ? y1 : y2);
            return;
        }
        ry=sqrt(radicant);
        rx=ratio*ry;
    }
    else
    {
        double factor = (largeArc==sweep ? -1.0 : 1.0)*sqrt(radicant);

        cxprime = factor*rx*y1prime/ry;
        cyprime = -factor*ry*x1prime/rx;
    }

    // F.6.5.3
    double cx = cxprime*cos(phi) - cyprime*sin(phi) + (x1 + x2)/2;
    double cy = cxprime*sin(phi) + cyprime*cos(phi) + (y1 + y2)/2;

    // now compute bounding box of the whole ellipse

    // Parametrick equation of an ellipse:
    // x(theta) = cx + rx*cos(theta)*cos(phi) - ry*sin(theta)*sin(phi)
    // y(theta) = cy + rx*cos(theta)*sin(phi) + ry*sin(theta)*cos(phi)

    // Compute local extrems
    // 0 = -rx*sin(theta)*cos(phi) - ry*cos(theta)*sin(phi)
    // 0 = -rx*sin(theta)*sin(phi) - ry*cos(theta)*cos(phi)

    // Local extrems for X:
    // theta = -atan(ry*tan(phi)/rx)
    // and
    // theta = M_PI -atan(ry*tan(phi)/rx)

    // Local extrems for Y:
    // theta = atan(ry/(tan(phi)*rx))
    // and
    // theta = M_PI + atan(ry/(tan(phi)*rx))

    double txmin, txmax, tymin, tymax;

    // First handle special cases
    if (phi == 0 || phi == M_PI)
    {
        xmin = cx - rx;
        txmin = getAngle(-rx, 0);
        xmax = cx + rx;
        txmax = getAngle(rx, 0);
        ymin = cy - ry;
        tymin = getAngle(0, -ry);
        ymax = cy + ry;
        tymax = getAngle(0, ry);
    }
    else if (phi == M_PI / 2.0 || phi == 3.0*M_PI/2.0)
    {
        xmin = cx - ry;
        txmin = getAngle(-ry, 0);
        xmax = cx + ry;
        txmax = getAngle(ry, 0);
        ymin = cy - rx;
        tymin = getAngle(0, -rx);
        ymax = cy + rx;
        tymax = getAngle(0, rx);
    }
    else
    {
        txmin = -atan(ry*tan(phi)/rx);
        txmax = M_PI - atan (ry*tan(phi)/rx);
        xmin = cx + rx*cos(txmin)*cos(phi) - ry*sin(txmin)*sin(phi);
        xmax = cx + rx*cos(txmax)*cos(phi) - ry*sin(txmax)*sin(phi);
        if (xmin > xmax)
        {
            std::swap(xmin,xmax);
            std::swap(txmin,txmax);
        }
        double tmpY = cy + rx*cos(txmin)*sin(phi) + ry*sin(txmin)*cos(phi);
        txmin = getAngle(xmin - cx, tmpY - cy);
        tmpY = cy + rx*cos(txmax)*sin(phi) + ry*sin(txmax)*cos(phi);
        txmax = getAngle(xmax - cx, tmpY - cy);


        tymin = atan(ry/(tan(phi)*rx));
        tymax = atan(ry/(tan(phi)*rx))+M_PI;
        ymin = cy + rx*cos(tymin)*sin(phi) + ry*sin(tymin)*cos(phi);
        ymax = cy + rx*cos(tymax)*sin(phi) + ry*sin(tymax)*cos(phi);
        if (ymin > ymax)
        {
            std::swap(ymin,ymax);
            std::swap(tymin,tymax);
        }
        double tmpX = cx + rx*cos(tymin)*cos(phi) - ry*sin(tymin)*sin(phi);
        tymin = getAngle(tmpX - cx, ymin - cy);
        tmpX = cx + rx*cos(tymax)*cos(phi) - ry*sin(tymax)*sin(phi);
        tymax = getAngle(tmpX - cx, ymax - cy);
    }
    double angle1 = getAngle(x1 - cx, y1 - cy);
    double angle2 = getAngle(x2 - cx, y2 - cy);

    // for sweep == 0 it is normal to have delta theta < 0
    // but we don't care about the rotation direction for bounding box
    if (!sweep)
        std::swap(angle1, angle2);

    // We cannot check directly for whether an angle is included in
    // an interval of angles that cross the 360/0 degree boundary
    // So here we will have to check for their absence in the complementary
    // angle interval
    bool otherArc = false;
    if (angle1 > angle2)
    {
        std::swap(angle1, angle2);
        otherArc = true;
    }

    // Check txmin
    if ((!otherArc && (angle1 > txmin || angle2 < txmin)) || (otherArc && !(angle1 > txmin || angle2 < txmin)))
        xmin = x1 < x2 ? x1 : x2;
    // Check txmax
    if ((!otherArc && (angle1 > txmax || angle2 < txmax)) || (otherArc && !(angle1 > txmax || angle2 < txmax)))
        xmax = x1 > x2 ? x1 : x2;
    // Check tymin
    if ((!otherArc && (angle1 > tymin || angle2 < tymin)) || (otherArc && !(angle1 > tymin || angle2 < tymin)))
        ymin = y1 < y2 ? y1 : y2;
    // Check tymax
    if ((!otherArc && (angle1 > tymax || angle2 < tymax)) || (otherArc && !(angle1 > tymax || angle2 < tymax)))
        ymax = y1 > y2 ? y1 : y2;
}

static WPXString doubleToString(const double value)
{
    WPXString tempString;
    tempString.sprintf("%.4f", value);
#ifndef __ANDROID__
    std::string decimalPoint(localeconv()->decimal_point);
#else
    std::string decimalPoint(".");
#endif
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
    // This must be a mistake and we do not want to crash lower
    if(path[0]["libwpg:path-action"]->getStr() == "Z")
        return;

    // try to find the bounding box
    // this is simple convex hull technique, the bounding box might not be
    // accurate but that should be enough for this purpose
    bool isFirstPoint = true;

    std::vector<double> px, py, qx, qy;
    int boundingBoxIndex = 0;
    px.push_back(0.0); py.push_back(0.0); qx.push_back(0.0); qy.push_back(0.0);
    double lastX = 0.0;
    double lastY = 0.0;

    for(unsigned k = 0; k < path.count(); k++)
    {
        if (!path[k]["svg:x"] || !path[k]["svg:y"])
            continue;

        if(path[k]["libwpg:path-action"]->getStr() == "M" && !isFirstPoint)
        {
            px.push_back(0.0);
            py.push_back(0.0);
            qx.push_back(0.0);
            qy.push_back(0.0);
            boundingBoxIndex++;
            isFirstPoint = true;
        }

        if (isFirstPoint)
        {
            px[boundingBoxIndex] = path[k]["svg:x"]->getDouble();
            py[boundingBoxIndex] = path[k]["svg:y"]->getDouble();
            qx[boundingBoxIndex] = px[boundingBoxIndex];
            qy[boundingBoxIndex] = py[boundingBoxIndex];
            isFirstPoint = false;
        }
        px[boundingBoxIndex] = (px[boundingBoxIndex] > path[k]["svg:x"]->getDouble()) ? path[k]["svg:x"]->getDouble() : px[boundingBoxIndex];
        py[boundingBoxIndex] = (py[boundingBoxIndex] > path[k]["svg:y"]->getDouble()) ? path[k]["svg:y"]->getDouble() : py[boundingBoxIndex];
        qx[boundingBoxIndex] = (qx[boundingBoxIndex] < path[k]["svg:x"]->getDouble()) ? path[k]["svg:x"]->getDouble() : qx[boundingBoxIndex];
        qy[boundingBoxIndex] = (qy[boundingBoxIndex] < path[k]["svg:y"]->getDouble()) ? path[k]["svg:y"]->getDouble() : qy[boundingBoxIndex];

        if(path[k]["libwpg:path-action"]->getStr() == "C")
        {
            px[boundingBoxIndex] = (px[boundingBoxIndex] > path[k]["svg:x1"]->getDouble()) ? path[k]["svg:x1"]->getDouble() : px[boundingBoxIndex];
            py[boundingBoxIndex] = (py[boundingBoxIndex] > path[k]["svg:y1"]->getDouble()) ? path[k]["svg:y1"]->getDouble() : py[boundingBoxIndex];
            qx[boundingBoxIndex] = (qx[boundingBoxIndex] < path[k]["svg:x1"]->getDouble()) ? path[k]["svg:x1"]->getDouble() : qx[boundingBoxIndex];
            qy[boundingBoxIndex] = (qy[boundingBoxIndex] < path[k]["svg:y1"]->getDouble()) ? path[k]["svg:y1"]->getDouble() : qy[boundingBoxIndex];
            px[boundingBoxIndex] = (px[boundingBoxIndex] > path[k]["svg:x2"]->getDouble()) ? path[k]["svg:x2"]->getDouble() : px[boundingBoxIndex];
            py[boundingBoxIndex] = (py[boundingBoxIndex] > path[k]["svg:y2"]->getDouble()) ? path[k]["svg:y2"]->getDouble() : py[boundingBoxIndex];
            qx[boundingBoxIndex] = (qx[boundingBoxIndex] < path[k]["svg:x2"]->getDouble()) ? path[k]["svg:x2"]->getDouble() : qx[boundingBoxIndex];
            qy[boundingBoxIndex] = (qy[boundingBoxIndex] < path[k]["svg:y2"]->getDouble()) ? path[k]["svg:y2"]->getDouble() : qy[boundingBoxIndex];
        }
        if(path[k]["libwpg:path-action"]->getStr() == "A")
        {
            double xmin, xmax, ymin, ymax;

            getEllipticalArcBBox(lastX, lastY, path[k]["svg:rx"]->getDouble(), path[k]["svg:ry"]->getDouble(),
                                 2.0*M_PI*(path[k]["libwpg:rotate"] ? path[k]["libwpg:rotate"]->getDouble() : 0.0),
                                 path[k]["libwpg:large-arc"] ? path[k]["libwpg:large-arc"]->getInt() : 1,
                                 path[k]["libwpg:sweep"] ? path[k]["libwpg:sweep"]->getInt() : 1,
                                 path[k]["svg:x"]->getDouble(), path[k]["svg:y"]->getDouble(), xmin, ymin, xmax, ymax);

            px[boundingBoxIndex] = (px[boundingBoxIndex] > xmin ? xmin : px[boundingBoxIndex]);
            py[boundingBoxIndex] = (py[boundingBoxIndex] > ymin ? ymin : py[boundingBoxIndex]);
            qx[boundingBoxIndex] = (qx[boundingBoxIndex] < xmax ? xmax : qx[boundingBoxIndex]);
            qy[boundingBoxIndex] = (qy[boundingBoxIndex] < ymax ? ymax : qy[boundingBoxIndex]);
        }
        lastX = path[k]["svg:x"]->getDouble();
        lastY = path[k]["svg:y"]->getDouble();
    }


    WPXString sValue;
    boundingBoxIndex = 0;
    _writeGraphicsStyle();
    TagOpenElement *pDrawPathElement = new TagOpenElement("draw:path");
    sValue.sprintf("gr%i", miGraphicsStyleIndex-1);
    pDrawPathElement->addAttribute("draw:style-name", sValue);
    pDrawPathElement->addAttribute("draw:text-style-name", "P1");
    pDrawPathElement->addAttribute("draw:layer", "layout");
    sValue = doubleToString(px[boundingBoxIndex]); sValue.append("in");
    pDrawPathElement->addAttribute("svg:x", sValue);
    sValue = doubleToString(py[boundingBoxIndex]); sValue.append("in");
    pDrawPathElement->addAttribute("svg:y", sValue);
    sValue = doubleToString((qx[boundingBoxIndex] - px[boundingBoxIndex])); sValue.append("in");
    pDrawPathElement->addAttribute("svg:width", sValue);
    sValue = doubleToString((qy[boundingBoxIndex] - py[boundingBoxIndex])); sValue.append("in");
    pDrawPathElement->addAttribute("svg:height", sValue);
    sValue.sprintf("%i %i %i %i", 0, 0, (unsigned)(2540*(qx[boundingBoxIndex] - px[boundingBoxIndex])), (unsigned)(2540*(qy[boundingBoxIndex] - py[boundingBoxIndex])));
    pDrawPathElement->addAttribute("svg:viewBox", sValue);

    sValue.clear();
    isFirstPoint = true;
    for(unsigned i = 0; i < path.count(); i++)
    {
        WPXString sElement;
        if (path[i]["libwpg:path-action"]->getStr() == "M")
        {
            if (!isFirstPoint)
            {
                pDrawPathElement->addAttribute("svg:d", sValue);
                mBodyElements.push_back(pDrawPathElement);
                mBodyElements.push_back(new TagCloseElement("draw:path"));
                boundingBoxIndex++;
                pDrawPathElement = new TagOpenElement("draw:path");
                sValue.sprintf("gr%i", miGraphicsStyleIndex-1);
                pDrawPathElement->addAttribute("draw:style-name", sValue);
                pDrawPathElement->addAttribute("draw:text-style-name", "P1");
                pDrawPathElement->addAttribute("draw:layer", "layout");
                sValue = doubleToString(px[boundingBoxIndex]); sValue.append("in");
                pDrawPathElement->addAttribute("svg:x", sValue);
                sValue = doubleToString(py[boundingBoxIndex]); sValue.append("in");
                pDrawPathElement->addAttribute("svg:y", sValue);
                sValue = doubleToString((qx[boundingBoxIndex] - px[boundingBoxIndex])); sValue.append("in");
                pDrawPathElement->addAttribute("svg:width", sValue);
                sValue = doubleToString((qy[boundingBoxIndex] - py[boundingBoxIndex])); sValue.append("in");
                pDrawPathElement->addAttribute("svg:height", sValue);
                sValue.sprintf("%i %i %i %i", 0, 0, (unsigned)(2540*(qx[boundingBoxIndex] - px[boundingBoxIndex])), (unsigned)(2540*(qy[boundingBoxIndex] - py[boundingBoxIndex])));
                pDrawPathElement->addAttribute("svg:viewBox", sValue);
                sValue.clear();
            }

            // 2540 is 2.54*1000, 2.54 in = 1 inch
            sElement.sprintf("M%i %i", (unsigned)((path[i]["svg:x"]->getDouble()-px[boundingBoxIndex])*2540),
                (unsigned)((path[i]["svg:y"]->getDouble()-py[boundingBoxIndex])*2540));
            sValue.append(sElement);
        }
        else if (path[i]["libwpg:path-action"]->getStr() == "L")
        {
            sElement.sprintf("L%i %i", (unsigned)((path[i]["svg:x"]->getDouble()-px[boundingBoxIndex])*2540),
                (unsigned)((path[i]["svg:y"]->getDouble()-py[boundingBoxIndex])*2540));
            sValue.append(sElement);
        }
        else if (path[i]["libwpg:path-action"]->getStr() == "C")
        {
            sElement.sprintf("C%i %i %i %i %i %i", (unsigned)((path[i]["svg:x1"]->getDouble()-px[boundingBoxIndex])*2540),
                (int)((path[i]["svg:y1"]->getDouble()-py[boundingBoxIndex])*2540), (unsigned)((path[i]["svg:x2"]->getDouble()-px[boundingBoxIndex])*2540),
                (int)((path[i]["svg:y2"]->getDouble()-py[boundingBoxIndex])*2540), (unsigned)((path[i]["svg:x"]->getDouble()-px[boundingBoxIndex])*2540),
                (unsigned)((path[i]["svg:y"]->getDouble()-py[boundingBoxIndex])*2540));
            sValue.append(sElement);
        }
        else if (path[i]["libwpg:path-action"]->getStr() == "A")
        {
            sElement.sprintf("A%i %i %i %i %i %i %i", (unsigned)((path[i]["svg:rx"]->getDouble())*2540),
                (int)((path[i]["svg:ry"]->getDouble())*2540), (path[i]["libwpg:rotate"] ? path[i]["libwpg:rotate"]->getInt() : 0),
                (path[i]["libwpg:large-arc"] ? path[i]["libwpg:large-arc"]->getInt() : 1),
                (path[i]["libwpg:sweep"] ? path[i]["libwpg:sweep"]->getInt() : 1),
                (unsigned)((path[i]["svg:x"]->getDouble()-px[boundingBoxIndex])*2540), (unsigned)((path[i]["svg:y"]->getDouble()-py[boundingBoxIndex])*2540));
            sValue.append(sElement);
        }
        else if (path[i]["libwpg:path-action"]->getStr() == "Z")
            sValue.append(" Z");
        isFirstPoint = false;
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
    if(mxStyle["draw:fill"] && mxStyle["draw:fill"]->getStr() == "gradient")
    {
        TagOpenElement *pDrawGradientElement = new TagOpenElement("draw:gradient");
        if (mxStyle["draw:style"])
            pDrawGradientElement->addAttribute("draw:style", mxStyle["draw:style"]->getStr());
        WPXString sValue;
        sValue.sprintf("Gradient_%i", miGradientIndex++);
        pDrawGradientElement->addAttribute("draw:name", sValue);

        // ODG angle unit is 0.1 degree
        double angle = mxStyle["draw:angle"] ? mxStyle["draw:angle"]->getDouble() : 0.0;
        while(angle < 0)
            angle += 360;
        while(angle > 360)
            angle -= 360;
        sValue.sprintf("%i", (unsigned)(angle*10));
        pDrawGradientElement->addAttribute("draw:angle", sValue);

        if (!mxGradient.count())
        {
            if (mxStyle["draw:start-color"])
                pDrawGradientElement->addAttribute("draw:start-color", mxStyle["draw:start-color"]->getStr());
            if (mxStyle["draw:end-color"])
                pDrawGradientElement->addAttribute("draw:end-color", mxStyle["draw:end-color"]->getStr());

            if (mxStyle["draw:border"])
                pDrawGradientElement->addAttribute("draw:border", mxStyle["draw:border"]->getStr());
            else
                pDrawGradientElement->addAttribute("draw:border", "0%");

            if (mxStyle["svg:cx"])
                pDrawGradientElement->addAttribute("draw:cx", mxStyle["svg:cx"]->getStr());
            else if (mxStyle["draw:cx"])
                pDrawGradientElement->addAttribute("draw:cx", mxStyle["draw:cx"]->getStr());

            if (mxStyle["svg:cy"])
                pDrawGradientElement->addAttribute("draw:cy", mxStyle["svg:cy"]->getStr());
            else if (mxStyle["draw:cx"])
                pDrawGradientElement->addAttribute("draw:cx", mxStyle["svg:cx"]->getStr());

            if (mxStyle["draw:start-intensity"])
                pDrawGradientElement->addAttribute("draw:start-intensity", mxStyle["draw:start-intensity"]->getStr());
            else
                pDrawGradientElement->addAttribute("draw:start-intensity", "100%");

            if (mxStyle["draw:border"])
                pDrawGradientElement->addAttribute("draw:end-intensity", mxStyle["draw:end-intensity"]->getStr());
            else
                pDrawGradientElement->addAttribute("draw:end-intensity", "100%");

            mGraphicsGradientStyles.push_back(pDrawGradientElement);
            mGraphicsGradientStyles.push_back(new TagCloseElement("draw:gradient"));
        }
        else if(mxGradient.count() >= 2)
        {
            sValue.sprintf("%i", (unsigned)(angle*10));
            pDrawGradientElement->addAttribute("draw:angle", sValue);

            pDrawGradientElement->addAttribute("draw:start-color", mxGradient[1]["svg:stop-color"]->getStr());
            pDrawGradientElement->addAttribute("draw:end-color", mxGradient[0]["svg:stop-color"]->getStr());
            if (mxStyle["svg:cx"])
                pDrawGradientElement->addAttribute("draw:cx", mxStyle["svg:cx"]->getStr());
            if (mxStyle["svg:cy"])
                pDrawGradientElement->addAttribute("draw:cy", mxStyle["svg:cy"]->getStr());
            pDrawGradientElement->addAttribute("draw:start-intensity", "100%");
            pDrawGradientElement->addAttribute("draw:end-intensity", "100%");
            pDrawGradientElement->addAttribute("draw:border", "0%");
            mGraphicsGradientStyles.push_back(pDrawGradientElement);
            mGraphicsGradientStyles.push_back(new TagCloseElement("draw:gradient"));
        }
    }

    TagOpenElement *pStyleStyleElement = new TagOpenElement("style:style");
    WPXString sValue;
    sValue.sprintf("gr%i",  miGraphicsStyleIndex);
    pStyleStyleElement->addAttribute("style:name", sValue);
    pStyleStyleElement->addAttribute("style:family", "graphic");
    pStyleStyleElement->addAttribute("style:parent-style-name", "standard");
    mGraphicsAutomaticStyles.push_back(pStyleStyleElement);

    TagOpenElement *pStyleGraphicsPropertiesElement = new TagOpenElement("style:graphic-properties");

    if((mxStyle["draw:stroke"] && mxStyle["draw:stroke"]->getStr() == "none") ||
       (mxStyle["svg:stroke-width"] && mxStyle["svg:stroke-width"]->getDouble() == 0.0) ||
       (mxStyle["svg:stroke-color"] && mxStyle["svg:stroke-color"]->getStr() == "none"))
        pStyleGraphicsPropertiesElement->addAttribute("draw:stroke", "none");
    else
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
        if (!mxGradient.count() || mxGradient.count() >= 2)
        {
            pStyleGraphicsPropertiesElement->addAttribute("draw:fill", "gradient");
            sValue.sprintf("Gradient_%i", miGradientIndex-1);
            pStyleGraphicsPropertiesElement->addAttribute("draw:fill-gradient-name", sValue);
        }
        else
        {
            if (mxGradient[0]["svg:stop-color"])
            {
                pStyleGraphicsPropertiesElement->addAttribute("draw:fill", "solid");
                pStyleGraphicsPropertiesElement->addAttribute("draw:fill-color", mxGradient[0]["svg:stop-color"]->getStr());
            }
            else
                pStyleGraphicsPropertiesElement->addAttribute("draw:fill", "solid");
        }
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

