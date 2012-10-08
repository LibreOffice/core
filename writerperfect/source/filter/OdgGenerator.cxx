/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information visit http://libwpg.sourceforge.net
 */

#include "FilterInternal.hxx"

#include "OdgGenerator.hxx"
#include "DocumentElement.hxx"
#include "OdfDocumentHandler.hxx"
#include "TextRunStyle.hxx"
#include "FontStyle.hxx"
#include <locale.h>
#include <math.h>
#include <string>
#include <map>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Workaround for the incapacity of draw to have multiple page
// sizes in the same document. Once that limitation is lifted,
// remove this
#define MULTIPAGE_WORKAROUND 1

namespace
{

static inline double getAngle(double bx, double by)
{
    return fmod(2*M_PI + (by > 0.0 ? 1.0 : -1.0) * acos( bx / sqrt(bx * bx + by * by) ), 2*M_PI);
}

static void getEllipticalArcBBox(double x0, double y0,
                                 double rx, double ry, double phi, bool largeArc, bool sweep, double x, double y,
                                 double &xmin, double &ymin, double &xmax, double &ymax)
{
    phi *= M_PI/180;
    if (rx < 0.0)
        rx *= -1.0;
    if (ry < 0.0)
        ry *= -1.0;

    if (rx == 0.0 || ry == 0.0)
    {
        xmin = (x0 < x ? x0 : x);
        xmax = (x0 > x ? x0 : x);
        ymin = (y0 < y ? y0 : y);
        ymax = (y0 > y ? y0 : y);
        return;
    }

    // F.6.5.1
    const double x1prime = cos(phi)*(x0 - x)/2 + sin(phi)*(y0 - y)/2;
    const double y1prime = -sin(phi)*(x0 - x)/2 + cos(phi)*(y0 - y)/2;

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
            xmin = (x0 < x ? x0 : x);
            xmax = (x0 > x ? x0 : x);
            ymin = (y0 < y ? y0 : y);
            ymax = (y0 > y ? y0 : y);
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
    double cx = cxprime*cos(phi) - cyprime*sin(phi) + (x0 + x)/2;
    double cy = cxprime*sin(phi) + cyprime*cos(phi) + (y0 + y)/2;

    // now compute bounding box of the whole ellipse

    // Parametric equation of an ellipse:
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
        double tmpY = cy + rx*cos(txmin)*sin(phi) + ry*sin(txmin)*cos(phi);
        txmin = getAngle(xmin - cx, tmpY - cy);
        tmpY = cy + rx*cos(txmax)*sin(phi) + ry*sin(txmax)*cos(phi);
        txmax = getAngle(xmax - cx, tmpY - cy);

        tymin = atan(ry/(tan(phi)*rx));
        tymax = atan(ry/(tan(phi)*rx))+M_PI;
        ymin = cy + rx*cos(tymin)*sin(phi) + ry*sin(tymin)*cos(phi);
        ymax = cy + rx*cos(tymax)*sin(phi) + ry*sin(tymax)*cos(phi);
        double tmpX = cx + rx*cos(tymin)*cos(phi) - ry*sin(tymin)*sin(phi);
        tymin = getAngle(tmpX - cx, ymin - cy);
        tmpX = cx + rx*cos(tymax)*cos(phi) - ry*sin(tymax)*sin(phi);
        tymax = getAngle(tmpX - cx, ymax - cy);
    }
    if (xmin > xmax)
    {
        std::swap(xmin,xmax);
        std::swap(txmin,txmax);
    }
    if (ymin > ymax)
    {
        std::swap(ymin,ymax);
        std::swap(tymin,tymax);
    }
    double angle1 = getAngle(x0 - cx, y0 - cy);
    double angle2 = getAngle(x - cx, y - cy);

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
        xmin = x0 < x ? x0 : x;
    // Check txmax
    if ((!otherArc && (angle1 > txmax || angle2 < txmax)) || (otherArc && !(angle1 > txmax || angle2 < txmax)))
        xmax = x0 > x ? x0 : x;
    // Check tymin
    if ((!otherArc && (angle1 > tymin || angle2 < tymin)) || (otherArc && !(angle1 > tymin || angle2 < tymin)))
        ymin = y0 < y ? y0 : y;
    // Check tymax
    if ((!otherArc && (angle1 > tymax || angle2 < tymax)) || (otherArc && !(angle1 > tymax || angle2 < tymax)))
        ymax = y0 > y ? y0 : y;
}

static inline double quadraticExtreme(double t, double a, double b, double c)
{
    return (1.0-t)*(1.0-t)*a + 2.0*(1.0-t)*t*b + t*t*c;
}

static inline double quadraticDerivative(double a, double b, double c)
{
    double denominator = a - 2.0*b + c;
    if (fabs(denominator) != 0.0)
        return (a - b)/denominator;
    return -1.0;
}

static void getQuadraticBezierBBox(double x0, double y0, double x1, double y1, double x, double y,
                                   double &xmin, double &ymin, double &xmax, double &ymax)
{
    xmin = x0 < x ? x0 : x;
    xmax = x0 > x ? x0 : x;
    ymin = y0 < y ? y0 : y;
    ymax = y0 > y ? y0 : y;

    double t = quadraticDerivative(x0, x1, x);
    if(t>=0 && t<=1)
    {
        double tmpx = quadraticExtreme(t, x0, x1, x);
        xmin = tmpx < xmin ? tmpx : xmin;
        xmax = tmpx > xmax ? tmpx : xmax;
    }

    t = quadraticDerivative(y0, y1, y);
    if(t>=0 && t<=1)
    {
        double tmpy = quadraticExtreme(t, y0, y1, y);
        ymin = tmpy < ymin ? tmpy : ymin;
        ymax = tmpy > ymax ? tmpy : ymax;
    }
}

static inline double cubicBase(double t, double a, double b, double c, double d)
{
    return (1.0-t)*(1.0-t)*(1.0-t)*a + 3.0*(1.0-t)*(1.0-t)*t*b + 3.0*(1.0-t)*t*t*c + t*t*t*d;
}

static void getCubicBezierBBox(double x0, double y0, double x1, double y1, double x2, double y2, double x, double y,
                               double &xmin, double &ymin, double &xmax, double &ymax)
{
    xmin = x0 < x ? x0 : x;
    xmax = x0 > x ? x0 : x;
    ymin = y0 < y ? y0 : y;
    ymax = y0 > y ? y0 : y;

    for (double t = 0.0; t <= 1.0; t+=0.01)
    {
        double tmpx = cubicBase(t, x0, x1, x2, x);
        xmin = tmpx < xmin ? tmpx : xmin;
        xmax = tmpx > xmax ? tmpx : xmax;
        double tmpy = cubicBase(t, y0, y1, y2, y);
        ymin = tmpy < ymin ? tmpy : ymin;
        ymax = tmpy > ymax ? tmpy : ymax;
    }
}


static WPXString doubleToString(const double value)
{
    WPXString tempString;
    tempString.sprintf("%.4f", value);
#ifndef ANDROID
    std::string decimalPoint(localeconv()->decimal_point);
#else
    std::string decimalPoint(".");
#endif
    if (decimalPoint.empty() || (decimalPoint == "."))
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

} // anonymous namespace

class OdgGeneratorPrivate
{
public:
    OdgGeneratorPrivate(OdfDocumentHandler *pHandler, const OdfStreamType streamType);
    ~OdgGeneratorPrivate();
    void _writeGraphicsStyle();
    void _drawPolySomething(const ::WPXPropertyListVector &vertices, bool isClosed);
    void _drawPath(const WPXPropertyListVector &path);
    // body elements
    std::vector <DocumentElement *> mBodyElements;

    // graphics styles
    std::vector<DocumentElement *> mGraphicsStrokeDashStyles;
    std::vector<DocumentElement *> mGraphicsGradientStyles;
    std::vector<DocumentElement *> mGraphicsBitmapStyles;
    std::vector<DocumentElement *> mGraphicsMarkerStyles;
    std::vector<DocumentElement *> mGraphicsAutomaticStyles;

    // page styles
    std::vector<DocumentElement *> mPageAutomaticStyles;
    std::vector<DocumentElement *> mPageMasterStyles;

    // paragraph styles
    ParagraphStyleManager mParagraphManager;

    // span styles
    SpanStyleManager mSpanManager;

    // font styles
    FontStyleManager mFontManager;

    OdfDocumentHandler *mpHandler;

    ::WPXPropertyList mxStyle;
    ::WPXPropertyListVector mxGradient;
    ::WPXPropertyListVector mxMarker;
    int miGradientIndex;
    int miBitmapIndex;
    int miStartMarkerIndex;
    int miEndMarkerIndex;
    int miDashIndex;
    int miGraphicsStyleIndex;
    int miPageIndex;
    double mfWidth, mfMaxWidth;
    double mfHeight, mfMaxHeight;

    const OdfStreamType mxStreamType;

    bool mbIsTextBox;
    bool mbIsTextLine;
    bool mbIsTextOnPath;
};

OdgGeneratorPrivate::OdgGeneratorPrivate(OdfDocumentHandler *pHandler, const OdfStreamType streamType):
    mBodyElements(),
    mGraphicsStrokeDashStyles(),
    mGraphicsGradientStyles(),
    mGraphicsBitmapStyles(),
    mGraphicsAutomaticStyles(),
    mPageAutomaticStyles(),
    mPageMasterStyles(),
    mParagraphManager(),
    mSpanManager(),
    mFontManager(),
    mpHandler(pHandler),
    mxStyle(), mxGradient(),
    mxMarker(),
    miGradientIndex(1),
    miBitmapIndex(1),
    miStartMarkerIndex(1),
    miEndMarkerIndex(1),
    miDashIndex(1),
    miGraphicsStyleIndex(1),
    miPageIndex(1),
    mfWidth(0.0),
    mfMaxWidth(0.0),
    mfHeight(0.0),
    mfMaxHeight(0.0),
    mxStreamType(streamType),
    mbIsTextBox(false),
    mbIsTextLine(false),
    mbIsTextOnPath(false)
{
}

OdgGeneratorPrivate::~OdgGeneratorPrivate()
{

    for (std::vector<DocumentElement *>::iterator iterBody = mBodyElements.begin(); iterBody != mBodyElements.end(); ++iterBody)
    {
        delete (*iterBody);
        (*iterBody) = 0;
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

    for (std::vector<DocumentElement *>::iterator iterGraphicsBitmapStyles = mGraphicsBitmapStyles.begin();
            iterGraphicsBitmapStyles != mGraphicsBitmapStyles.end(); ++iterGraphicsBitmapStyles)
    {
        delete((*iterGraphicsBitmapStyles));
    }

    for (std::vector<DocumentElement *>::iterator iterGraphicsMarkerStyles = mGraphicsMarkerStyles.begin();
            iterGraphicsMarkerStyles != mGraphicsMarkerStyles.end(); ++iterGraphicsMarkerStyles)
    {
        delete((*iterGraphicsMarkerStyles));
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

    mParagraphManager.clean();
    mSpanManager.clean();
    mFontManager.clean();
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
        WPXString sWidth;
        sWidth.sprintf("%li", (unsigned long)(2540 * mpImpl->mfMaxWidth));
        mpImpl->mpHandler->characters(sWidth);
        mpImpl->mpHandler->endElement("config:config-item");

        configItemOpenElement.addAttribute("config:name", "VisibleAreaHeight");
        configItemOpenElement.addAttribute("config:type", "int");
        configItemOpenElement.write(mpImpl->mpHandler);
        WPXString sHeight;
        sHeight.sprintf("%li", (unsigned long)(2540 * mpImpl->mfMaxHeight));
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

        for (std::vector<DocumentElement *>::const_iterator iterGraphicsBitmapStyles = mpImpl->mGraphicsBitmapStyles.begin();
                iterGraphicsBitmapStyles != mpImpl->mGraphicsBitmapStyles.end(); ++iterGraphicsBitmapStyles)
        {
            (*iterGraphicsBitmapStyles)->write(mpImpl->mpHandler);
        }

        for (std::vector<DocumentElement *>::const_iterator iterGraphicsMarkerStyles = mpImpl->mGraphicsMarkerStyles.begin();
                iterGraphicsMarkerStyles != mpImpl->mGraphicsMarkerStyles.end(); ++iterGraphicsMarkerStyles)
        {
            (*iterGraphicsMarkerStyles)->write(mpImpl->mpHandler);
        }
        mpImpl->mpHandler->endElement("office:styles");
    }


    if ((mpImpl->mxStreamType == ODF_FLAT_XML) || (mpImpl->mxStreamType == ODF_CONTENT_XML) || (mpImpl->mxStreamType == ODF_STYLES_XML))
    {
        mpImpl->mFontManager.writeFontsDeclaration(mpImpl->mpHandler);

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
        mpImpl->mParagraphManager.write(mpImpl->mpHandler);
        mpImpl->mSpanManager.write(mpImpl->mpHandler);
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
        sValue = doubleToString(mpImpl->mfMaxWidth);
        sValue.append("in");
        tmpStylePageLayoutPropertiesOpenElement.addAttribute("fo:page-width", sValue);
        sValue = doubleToString(mpImpl->mfMaxHeight);
        sValue.append("in");
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
                iterPageAutomaticStyles != mpImpl->mPageAutomaticStyles.end(); ++iterPageAutomaticStyles)
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

void OdgGenerator::setStyle(const ::WPXPropertyList &propList, const ::WPXPropertyListVector &gradient)
{
    mpImpl->mxStyle.clear();
    mpImpl->mxStyle = propList;
    mpImpl->mxGradient = gradient;
}

void OdgGenerator::startLayer(const ::WPXPropertyList & /* propList */)
{
    mpImpl->mBodyElements.push_back(new TagOpenElement("draw:g"));
}

void OdgGenerator::endLayer()
{
    mpImpl->mBodyElements.push_back(new TagCloseElement("draw:g"));
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
    sValue = doubleToString(2 * propList["svg:rx"]->getDouble());
    sValue.append("in");
    pDrawEllipseElement->addAttribute("svg:width", sValue);
    sValue = doubleToString(2 * propList["svg:ry"]->getDouble());
    sValue.append("in");
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
        sValue = "rotate(";
        sValue.append(doubleToString(radrotation));
        sValue.append(") ");
        sValue.append("translate(");
        sValue.append(doubleToString(propList["svg:cx"]->getDouble() - propList["svg:rx"]->getDouble() - deltax));
        sValue.append("in, ");
        sValue.append(doubleToString(propList["svg:cy"]->getDouble() - propList["svg:ry"]->getDouble() - deltay));
        sValue.append("in)");
        pDrawEllipseElement->addAttribute("draw:transform", sValue);
    }
    else
    {
        sValue = doubleToString(propList["svg:cx"]->getDouble()-propList["svg:rx"]->getDouble());
        sValue.append("in");
        pDrawEllipseElement->addAttribute("svg:x", sValue);
        sValue = doubleToString(propList["svg:cy"]->getDouble()-propList["svg:ry"]->getDouble());
        sValue.append("in");
        pDrawEllipseElement->addAttribute("svg:y", sValue);
    }
    mpImpl->mBodyElements.push_back(pDrawEllipseElement);
    mpImpl->mBodyElements.push_back(new TagCloseElement("draw:ellipse"));
}

void OdgGenerator::drawPolyline(const ::WPXPropertyListVector &vertices)
{
    mpImpl->_drawPolySomething(vertices, false);
}

void OdgGenerator::drawPolygon(const ::WPXPropertyListVector &vertices)
{
    mpImpl->_drawPolySomething(vertices, true);
}

void OdgGeneratorPrivate::_drawPolySomething(const ::WPXPropertyListVector &vertices, bool isClosed)
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

        for (unsigned long ii = 0; ii < vertices.count(); ++ii)
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

void OdgGeneratorPrivate::_drawPath(const WPXPropertyListVector &path)
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

    double px = 0.0, py = 0.0, qx = 0.0, qy = 0.0;
    double lastX = 0.0;
    double lastY = 0.0;

    for(unsigned k = 0; k < path.count(); ++k)
    {
        if (!path[k]["svg:x"] || !path[k]["svg:y"])
            continue;
        if (isFirstPoint)
        {
            px = path[k]["svg:x"]->getDouble();
            py = path[k]["svg:y"]->getDouble();
            qx = px;
            qy = py;
            lastX = px;
            lastY = py;
            isFirstPoint = false;
        }
        px = (px > path[k]["svg:x"]->getDouble()) ? path[k]["svg:x"]->getDouble() : px;
        py = (py > path[k]["svg:y"]->getDouble()) ? path[k]["svg:y"]->getDouble() : py;
        qx = (qx < path[k]["svg:x"]->getDouble()) ? path[k]["svg:x"]->getDouble() : qx;
        qy = (qy < path[k]["svg:y"]->getDouble()) ? path[k]["svg:y"]->getDouble() : qy;

        double xmin, xmax, ymin, ymax;

        if(path[k]["libwpg:path-action"]->getStr() == "C")
        {
            getCubicBezierBBox(lastX, lastY, path[k]["svg:x1"]->getDouble(), path[k]["svg:y1"]->getDouble(),
                               path[k]["svg:x2"]->getDouble(), path[k]["svg:y2"]->getDouble(),
                               path[k]["svg:x"]->getDouble(), path[k]["svg:y"]->getDouble(), xmin, ymin, xmax, ymax);

            px = (px > xmin ? xmin : px);
            py = (py > ymin ? ymin : py);
            qx = (qx < xmax ? xmax : qx);
            qy = (qy < ymax ? ymax : qy);
        }
        if(path[k]["libwpg:path-action"]->getStr() == "Q")
        {
            getQuadraticBezierBBox(lastX, lastY, path[k]["svg:x1"]->getDouble(), path[k]["svg:y1"]->getDouble(),
                                   path[k]["svg:x"]->getDouble(), path[k]["svg:y"]->getDouble(), xmin, ymin, xmax, ymax);

            px = (px > xmin ? xmin : px);
            py = (py > ymin ? ymin : py);
            qx = (qx < xmax ? xmax : qx);
            qy = (qy < ymax ? ymax : qy);
        }
        if(path[k]["libwpg:path-action"]->getStr() == "A")
        {
            getEllipticalArcBBox(lastX, lastY, path[k]["svg:rx"]->getDouble(), path[k]["svg:ry"]->getDouble(),
                                 path[k]["libwpg:rotate"] ? path[k]["libwpg:rotate"]->getDouble() : 0.0,
                                 path[k]["libwpg:large-arc"] ? path[k]["libwpg:large-arc"]->getInt() : 1,
                                 path[k]["libwpg:sweep"] ? path[k]["libwpg:sweep"]->getInt() : 1,
                                 path[k]["svg:x"]->getDouble(), path[k]["svg:y"]->getDouble(), xmin, ymin, xmax, ymax);

            px = (px > xmin ? xmin : px);
            py = (py > ymin ? ymin : py);
            qx = (qx < xmax ? xmax : qx);
            qy = (qy < ymax ? ymax : qy);
        }
        lastX = path[k]["svg:x"]->getDouble();
        lastY = path[k]["svg:y"]->getDouble();
    }


    WPXString sValue;
    _writeGraphicsStyle();
    TagOpenElement *pDrawPathElement = new TagOpenElement("draw:path");
    sValue.sprintf("gr%i", miGraphicsStyleIndex-1);
    pDrawPathElement->addAttribute("draw:style-name", sValue);
    pDrawPathElement->addAttribute("draw:layer", "layout");
    sValue = doubleToString(px);
    sValue.append("in");
    pDrawPathElement->addAttribute("svg:x", sValue);
    sValue = doubleToString(py);
    sValue.append("in");
    pDrawPathElement->addAttribute("svg:y", sValue);
    sValue = doubleToString((qx - px));
    sValue.append("in");
    pDrawPathElement->addAttribute("svg:width", sValue);
    sValue = doubleToString((qy - py));
    sValue.append("in");
    pDrawPathElement->addAttribute("svg:height", sValue);
    sValue.sprintf("%i %i %i %i", 0, 0, (unsigned)(2540*(qx - px)), (unsigned)(2540*(qy - py)));
    pDrawPathElement->addAttribute("svg:viewBox", sValue);

    sValue.clear();
    for(unsigned i = 0; i < path.count(); ++i)
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
                             (unsigned)((path[i]["svg:y1"]->getDouble()-py)*2540), (unsigned)((path[i]["svg:x2"]->getDouble()-px)*2540),
                             (unsigned)((path[i]["svg:y2"]->getDouble()-py)*2540), (unsigned)((path[i]["svg:x"]->getDouble()-px)*2540),
                             (unsigned)((path[i]["svg:y"]->getDouble()-py)*2540));
            sValue.append(sElement);
        }
        else if (path[i]["libwpg:path-action"]->getStr() == "Q")
        {
            sElement.sprintf("Q%i %i %i %i", (unsigned)((path[i]["svg:x1"]->getDouble()-px)*2540),
                             (unsigned)((path[i]["svg:y1"]->getDouble()-py)*2540), (unsigned)((path[i]["svg:x"]->getDouble()-px)*2540),
                             (unsigned)((path[i]["svg:y"]->getDouble()-py)*2540));
            sValue.append(sElement);
        }
        else if (path[i]["libwpg:path-action"]->getStr() == "A")
        {
            sElement.sprintf("A%i %i %i %i %i %i %i", (unsigned)((path[i]["svg:rx"]->getDouble())*2540),
                             (unsigned)((path[i]["svg:ry"]->getDouble())*2540), (path[i]["libwpg:rotate"] ? path[i]["libwpg:rotate"]->getInt() : 0),
                             (path[i]["libwpg:large-arc"] ? path[i]["libwpg:large-arc"]->getInt() : 1),
                             (path[i]["libwpg:sweep"] ? path[i]["libwpg:sweep"]->getInt() : 1),
                             (unsigned)((path[i]["svg:x"]->getDouble()-px)*2540), (unsigned)((path[i]["svg:y"]->getDouble()-py)*2540));
            sValue.append(sElement);
        }
        else if (path[i]["libwpg:path-action"]->getStr() == "Z")
            sValue.append(" Z");
    }
    pDrawPathElement->addAttribute("svg:d", sValue);
    mBodyElements.push_back(pDrawPathElement);
    mBodyElements.push_back(new TagCloseElement("draw:path"));
}

void OdgGenerator::drawPath(const WPXPropertyListVector &path)
{
    mpImpl->_drawPath(path);
}

void OdgGenerator::drawGraphicObject(const ::WPXPropertyList &propList, const ::WPXBinaryData &binaryData)
{
    if (!propList["libwpg:mime-type"] || propList["libwpg:mime-type"]->getStr().len() <= 0)
        return;
    if (!propList["svg:x"] || !propList["svg:y"] || !propList["svg:width"] || !propList["svg:height"])
        return;

    bool flipX(propList["draw:mirror-horizontal"] && propList["draw:mirror-horizontal"]->getInt());
    bool flipY(propList["draw:mirror-vertical"] && propList["draw:mirror-vertical"]->getInt());
    if ((flipX && !flipY) || (!flipX && flipY))
        mpImpl->mxStyle.insert("style:mirror", "horizontal");
    else
        mpImpl->mxStyle.insert("style:mirror", "none");
    if (propList["draw:color-mode"])
        mpImpl->mxStyle.insert("draw:color-mode", propList["draw:color-mode"]->getStr());
    if (propList["draw:luminance"])
        mpImpl->mxStyle.insert("draw:luminance", propList["draw:luminance"]->getStr());
    if (propList["draw:contrast"])
        mpImpl->mxStyle.insert("draw:contrast", propList["draw:contrast"]->getStr());
    if (propList["draw:gamma"])
        mpImpl->mxStyle.insert("draw:gamma", propList["draw:gamma"]->getStr());
    if (propList["draw:red"])
        mpImpl->mxStyle.insert("draw:red", propList["draw:red"]->getStr());
    if (propList["draw:green"])
        mpImpl->mxStyle.insert("draw:green", propList["draw:green"]->getStr());
    if (propList["draw:blue"])
        mpImpl->mxStyle.insert("draw:blue", propList["draw:blue"]->getStr());


    mpImpl->_writeGraphicsStyle();

    double x = propList["svg:x"]->getDouble();
    double y = propList["svg:y"]->getDouble();
    double height = propList["svg:height"]->getDouble();
    double width = propList["svg:width"]->getDouble();

    if (flipY)
    {
        x += width;
        y += height;
        width *= -1.0;
        height *= -1.0;
    }

    double angle(propList["libwpg:rotate"] ? - M_PI * propList["libwpg:rotate"]->getDouble() / 180.0 : 0.0);
    if (angle != 0.0)
    {
        double deltax((width*cos(angle)+height*sin(angle)-width)/2.0);
        double deltay((-width*sin(angle)+height*cos(angle)-height)/2.0);
        x -= deltax;
        y -= deltay;
    }

    WPXPropertyList framePropList;

    framePropList.insert("svg:x", x);
    framePropList.insert("svg:y", y);
    framePropList.insert("svg:height", height);
    framePropList.insert("svg:width", width);

    TagOpenElement *pDrawFrameElement = new TagOpenElement("draw:frame");

    WPXString sValue;
    sValue.sprintf("gr%i", mpImpl->miGraphicsStyleIndex-1);
    pDrawFrameElement->addAttribute("draw:style-name", sValue);

    pDrawFrameElement->addAttribute("svg:height", framePropList["svg:height"]->getStr());
    pDrawFrameElement->addAttribute("svg:width", framePropList["svg:width"]->getStr());

    if (angle != 0.0)
    {
        framePropList.insert("libwpg:rotate", angle, WPX_GENERIC);
        sValue.sprintf("rotate (%s) translate(%s, %s)",
                       framePropList["libwpg:rotate"]->getStr().cstr(),
                       framePropList["svg:x"]->getStr().cstr(),
                       framePropList["svg:y"]->getStr().cstr());
        pDrawFrameElement->addAttribute("draw:transform", sValue);
    }
    else
    {
        pDrawFrameElement->addAttribute("svg:x", framePropList["svg:x"]->getStr());
        pDrawFrameElement->addAttribute("svg:y", framePropList["svg:y"]->getStr());
    }
    mpImpl->mBodyElements.push_back(pDrawFrameElement);

    if (propList["libwpg:mime-type"]->getStr() == "object/ole")
        mpImpl->mBodyElements.push_back(new TagOpenElement("draw:object-ole"));
    else
        mpImpl->mBodyElements.push_back(new TagOpenElement("draw:image"));

    mpImpl->mBodyElements.push_back(new TagOpenElement("office:binary-data"));

    ::WPXString base64Binary = binaryData.getBase64Data();
    mpImpl->mBodyElements.push_back(new CharDataElement(base64Binary.cstr()));

    mpImpl->mBodyElements.push_back(new TagCloseElement("office:binary-data"));

    if (propList["libwpg:mime-type"]->getStr() == "object/ole")
        mpImpl->mBodyElements.push_back(new TagCloseElement("draw:object-ole"));
    else
        mpImpl->mBodyElements.push_back(new TagCloseElement("draw:image"));

    mpImpl->mBodyElements.push_back(new TagCloseElement("draw:frame"));
}

void OdgGeneratorPrivate::_writeGraphicsStyle()
{
    bool bUseOpacityGradient = false;

    if (mxStyle["draw:stroke"] && mxStyle["draw:stroke"]->getStr() == "dash")
    {
        TagOpenElement *pDrawStrokeDashElement = new TagOpenElement("draw:stroke-dash");
        WPXString sValue;
        sValue.sprintf("Dash_%i", miDashIndex++);
        pDrawStrokeDashElement->addAttribute("draw:name", sValue);
        if (mxStyle["svg:stoke-linecap"])
            pDrawStrokeDashElement->addAttribute("draw:style", mxStyle["svg:stroke-linecap"]->getStr());
        else
            pDrawStrokeDashElement->addAttribute("draw:style", "rect");
        if (mxStyle["draw:distance"])
            pDrawStrokeDashElement->addAttribute("draw:distance", mxStyle["draw:distance"]->getStr());
        if (mxStyle["draw:dots1"])
            pDrawStrokeDashElement->addAttribute("draw:dots1", mxStyle["draw:dots1"]->getStr());
        if (mxStyle["draw:dots1-length"])
            pDrawStrokeDashElement->addAttribute("draw:dots1-length", mxStyle["draw:dots1-length"]->getStr());
        if (mxStyle["draw:dots2"])
            pDrawStrokeDashElement->addAttribute("draw:dots2", mxStyle["draw:dots2"]->getStr());
        if (mxStyle["draw:dots2-length"])
            pDrawStrokeDashElement->addAttribute("draw:dots2-length", mxStyle["draw:dots2-length"]->getStr());
        mGraphicsStrokeDashStyles.push_back(pDrawStrokeDashElement);
        mGraphicsStrokeDashStyles.push_back(new TagCloseElement("draw:stroke-dash"));
    }

    if (mxStyle["draw:marker-start-path"])
    {
        WPXString sValue;
        TagOpenElement *pDrawMarkerElement = new TagOpenElement("draw:marker");
        sValue.sprintf("StartMarker_%i", miStartMarkerIndex);
        pDrawMarkerElement->addAttribute("draw:name", sValue);
        if (mxStyle["draw:marker-start-viewbox"])
            pDrawMarkerElement->addAttribute("svg:viewBox", mxStyle["draw:marker-start-viewbox"]->getStr());
        pDrawMarkerElement->addAttribute("svg:d", mxStyle["draw:marker-start-path"]->getStr());
        mGraphicsMarkerStyles.push_back(pDrawMarkerElement);
        mGraphicsMarkerStyles.push_back(new TagCloseElement("draw:marker"));
    }
    if(mxStyle["draw:marker-end-path"])
    {
        WPXString sValue;
        TagOpenElement *pDrawMarkerElement = new TagOpenElement("draw:marker");
        sValue.sprintf("EndMarker_%i", miEndMarkerIndex);
        pDrawMarkerElement->addAttribute("draw:name", sValue);
        if (mxStyle["draw:marker-end-viewbox"])
            pDrawMarkerElement->addAttribute("svg:viewBox", mxStyle["draw:marker-end-viewbox"]->getStr());
        pDrawMarkerElement->addAttribute("svg:d", mxStyle["draw:marker-end-path"]->getStr());
        mGraphicsMarkerStyles.push_back(pDrawMarkerElement);
        mGraphicsMarkerStyles.push_back(new TagCloseElement("draw:marker"));
    }

    if(mxStyle["draw:fill"] && mxStyle["draw:fill"]->getStr() == "gradient")
    {
        TagOpenElement *pDrawGradientElement = new TagOpenElement("draw:gradient");
        TagOpenElement *pDrawOpacityElement = new TagOpenElement("draw:opacity");
        if (mxStyle["draw:style"])
        {
            pDrawGradientElement->addAttribute("draw:style", mxStyle["draw:style"]->getStr());
            pDrawOpacityElement->addAttribute("draw:style", mxStyle["draw:style"]->getStr());
        }
        else
        {
            pDrawGradientElement->addAttribute("draw:style", "linear");
            pDrawOpacityElement->addAttribute("draw:style", "linear");
        }
        WPXString sValue;
        sValue.sprintf("Gradient_%i", miGradientIndex);
        pDrawGradientElement->addAttribute("draw:name", sValue);
        sValue.sprintf("Transparency_%i", miGradientIndex++);
        pDrawOpacityElement->addAttribute("draw:name", sValue);

        // ODG angle unit is 0.1 degree
        double angle = mxStyle["draw:angle"] ? mxStyle["draw:angle"]->getDouble() : 0.0;
        while(angle < 0)
            angle += 360;
        while(angle > 360)
            angle -= 360;
        sValue.sprintf("%i", (unsigned)(angle*10));
        pDrawGradientElement->addAttribute("draw:angle", sValue);
        pDrawOpacityElement->addAttribute("draw:angle", sValue);

        if (!mxGradient.count())
        {
            if (mxStyle["draw:start-color"])
                pDrawGradientElement->addAttribute("draw:start-color", mxStyle["draw:start-color"]->getStr());
            if (mxStyle["draw:end-color"])
                pDrawGradientElement->addAttribute("draw:end-color", mxStyle["draw:end-color"]->getStr());

            if (mxStyle["draw:border"])
            {
                pDrawGradientElement->addAttribute("draw:border", mxStyle["draw:border"]->getStr());
                pDrawOpacityElement->addAttribute("draw:border", mxStyle["draw:border"]->getStr());
            }
            else
            {
                pDrawGradientElement->addAttribute("draw:border", "0%");
                pDrawOpacityElement->addAttribute("draw:border", "0%");
            }

            if (mxStyle["svg:cx"])
            {
                pDrawGradientElement->addAttribute("draw:cx", mxStyle["svg:cx"]->getStr());
                pDrawOpacityElement->addAttribute("draw:cx", mxStyle["svg:cx"]->getStr());
            }
            else if (mxStyle["draw:cx"])
            {
                pDrawGradientElement->addAttribute("draw:cx", mxStyle["draw:cx"]->getStr());
                pDrawOpacityElement->addAttribute("draw:cx", mxStyle["draw:cx"]->getStr());
            }

            if (mxStyle["svg:cy"])
            {
                pDrawGradientElement->addAttribute("draw:cy", mxStyle["svg:cy"]->getStr());
                pDrawOpacityElement->addAttribute("draw:cy", mxStyle["svg:cy"]->getStr());
            }
            else if (mxStyle["draw:cx"])
            {
                pDrawGradientElement->addAttribute("draw:cx", mxStyle["svg:cx"]->getStr());
                pDrawOpacityElement->addAttribute("draw:cx", mxStyle["svg:cx"]->getStr());
            }

            if (mxStyle["draw:start-intensity"])
                pDrawGradientElement->addAttribute("draw:start-intensity", mxStyle["draw:start-intensity"]->getStr());
            else
                pDrawGradientElement->addAttribute("draw:start-intensity", "100%");

            if (mxStyle["draw:end-intensity"])
                pDrawGradientElement->addAttribute("draw:end-intensity", mxStyle["draw:end-intensity"]->getStr());
            else
                pDrawGradientElement->addAttribute("draw:end-intensity", "100%");

            if (mxStyle["libwpg:start-opacity"])
                pDrawOpacityElement->addAttribute("draw:start", mxStyle["libwpg:start-opacity"]->getStr());
            else
                pDrawOpacityElement->addAttribute("draw:start", "100%");

            if (mxStyle["libwpg:end-opacity"])
                pDrawOpacityElement->addAttribute("draw:end", mxStyle["libwpg:end-opacity"]->getStr());
            else
                pDrawOpacityElement->addAttribute("draw:end", "100%");

            mGraphicsGradientStyles.push_back(pDrawGradientElement);
            mGraphicsGradientStyles.push_back(new TagCloseElement("draw:gradient"));

            // Work around a mess in LibreOffice where both opacities of 100% are interpreted as complete transparency
            // Nevertheless, when one is different, immediately, they are interpreted correctly
            if (mxStyle["libwpg:start-opacity"] && mxStyle["libwpg:end-opacity"]
                    && (mxStyle["libwpg:start-opacity"]->getDouble() != 1.0 || mxStyle["libwpg:end-opacity"]->getDouble() != 1.0))
            {
                bUseOpacityGradient = true;
                mGraphicsGradientStyles.push_back(pDrawOpacityElement);
                mGraphicsGradientStyles.push_back(new TagCloseElement("draw:opacity"));
            }
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
            if (mxGradient[1]["svg:stop-opacity"])
            {
                pDrawOpacityElement->addAttribute("draw:start", mxGradient[1]["svg:stop-opacity"]->getStr());
                bUseOpacityGradient = true;
            }
            else
                pDrawOpacityElement->addAttribute("draw:start", "100%");
            if (mxGradient[0]["svg:stop-opacity"])
            {
                pDrawOpacityElement->addAttribute("draw:end", mxGradient[0]["svg:stop-opacity"]->getStr());
                bUseOpacityGradient = true;
            }
            else
                pDrawOpacityElement->addAttribute("draw:end", "100%");
            pDrawGradientElement->addAttribute("draw:border", "0%");
            mGraphicsGradientStyles.push_back(pDrawGradientElement);
            mGraphicsGradientStyles.push_back(new TagCloseElement("draw:gradient"));
            if (bUseOpacityGradient)
            {
                mGraphicsGradientStyles.push_back(pDrawOpacityElement);
                mGraphicsGradientStyles.push_back(new TagCloseElement("draw:opacity"));
            }
        }

        if(!bUseOpacityGradient)
            delete pDrawOpacityElement;
    }

    if(mxStyle["draw:fill"] && mxStyle["draw:fill"]->getStr() == "bitmap" &&
            mxStyle["draw:fill-image"] && mxStyle["libwpg:mime-type"])
    {
        TagOpenElement *pDrawBitmapElement = new TagOpenElement("draw:fill-image");
        WPXString sValue;
        sValue.sprintf("Bitmap_%i", miBitmapIndex++);
        pDrawBitmapElement->addAttribute("draw:name", sValue);
        mGraphicsBitmapStyles.push_back(pDrawBitmapElement);
        mGraphicsBitmapStyles.push_back(new TagOpenElement("office:binary-data"));
        mGraphicsBitmapStyles.push_back(new CharDataElement(mxStyle["draw:fill-image"]->getStr()));
        mGraphicsBitmapStyles.push_back(new TagCloseElement("office:binary-data"));
        mGraphicsBitmapStyles.push_back(new TagCloseElement("draw:fill-image"));
    }

    TagOpenElement *pStyleStyleElement = new TagOpenElement("style:style");
    WPXString sValue;
    sValue.sprintf("gr%i",  miGraphicsStyleIndex);
    pStyleStyleElement->addAttribute("style:name", sValue);
    pStyleStyleElement->addAttribute("style:family", "graphic");
    pStyleStyleElement->addAttribute("style:parent-style-name", "standard");
    mGraphicsAutomaticStyles.push_back(pStyleStyleElement);

    TagOpenElement *pStyleGraphicsPropertiesElement = new TagOpenElement("style:graphic-properties");

    if (mxStyle["draw:color-mode"] && mxStyle["draw:color-mode"]->getStr().len() > 0)
        pStyleGraphicsPropertiesElement->addAttribute("draw:color-mode", mxStyle["draw:color-mode"]->getStr());
    if (mxStyle["draw:luminance"] && mxStyle["draw:luminance"]->getStr().len() > 0)
        pStyleGraphicsPropertiesElement->addAttribute("draw:luminance", mxStyle["draw:luminance"]->getStr());
    if (mxStyle["draw:contrast"] && mxStyle["draw:contrast"]->getStr().len() > 0)
        pStyleGraphicsPropertiesElement->addAttribute("draw:contrast", mxStyle["draw:contrast"]->getStr());
    if (mxStyle["draw:gamma"] && mxStyle["draw:gamma"]->getStr().len() > 0)
        pStyleGraphicsPropertiesElement->addAttribute("draw:gamma", mxStyle["draw:gamma"]->getStr());
    if (mxStyle["draw:red"] && mxStyle["draw:red"]->getStr().len() > 0)
        pStyleGraphicsPropertiesElement->addAttribute("draw:red", mxStyle["draw:red"]->getStr());
    if (mxStyle["draw:green"] && mxStyle["draw:green"]->getStr().len() > 0)
        pStyleGraphicsPropertiesElement->addAttribute("draw:green", mxStyle["draw:green"]->getStr());
    if (mxStyle["draw:blue"] && mxStyle["draw:blue"]->getStr().len() > 0)
        pStyleGraphicsPropertiesElement->addAttribute("draw:blue", mxStyle["draw:blue"]->getStr());

    if (mxStyle["draw:stroke"] && mxStyle["draw:stroke"]->getStr() == "none")
        pStyleGraphicsPropertiesElement->addAttribute("draw:stroke", "none");
    else
    {
        if (mxStyle["svg:stroke-width"])
            pStyleGraphicsPropertiesElement->addAttribute("svg:stroke-width", mxStyle["svg:stroke-width"]->getStr());

        if (mxStyle["svg:stroke-color"])
            pStyleGraphicsPropertiesElement->addAttribute("svg:stroke-color", mxStyle["svg:stroke-color"]->getStr());

        if (mxStyle["svg:stroke-opacity"])
            pStyleGraphicsPropertiesElement->addAttribute("svg:stroke-opacity", mxStyle["svg:stroke-opacity"]->getStr());

        if (mxStyle["svg:stroke-linejoin"])
            pStyleGraphicsPropertiesElement->addAttribute("draw:stroke-linejoin", mxStyle["svg:stroke-linejoin"]->getStr());

        if (mxStyle["svg:stroke-linecap"])
            pStyleGraphicsPropertiesElement->addAttribute("svg:stoke-linecap", mxStyle["svg:stroke-linecap"]->getStr());

        if (mxStyle["draw:stroke"] && mxStyle["draw:stroke"]->getStr() == "dash")
        {
            pStyleGraphicsPropertiesElement->addAttribute("draw:stroke", "dash");
            sValue.sprintf("Dash_%i", miDashIndex-1);
            pStyleGraphicsPropertiesElement->addAttribute("draw:stroke-dash", sValue);
        }
        else
            pStyleGraphicsPropertiesElement->addAttribute("draw:stroke", "solid");
    }

    if(mxStyle["draw:fill"] && mxStyle["draw:fill"]->getStr() == "none")
        pStyleGraphicsPropertiesElement->addAttribute("draw:fill", "none");
    else
    {
        if (mxStyle["draw:shadow"])
            pStyleGraphicsPropertiesElement->addAttribute("draw:shadow", mxStyle["draw:shadow"]->getStr());
        else
            pStyleGraphicsPropertiesElement->addAttribute("draw:shadow", "hidden");
        if (mxStyle["draw:shadow-offset-x"])
            pStyleGraphicsPropertiesElement->addAttribute("draw:shadow-offset-x", mxStyle["draw:shadow-offset-x"]->getStr());
        if (mxStyle["draw:shadow-offset-y"])
            pStyleGraphicsPropertiesElement->addAttribute("draw:shadow-offset-y", mxStyle["draw:shadow-offset-y"]->getStr());
        if (mxStyle["draw:shadow-color"])
            pStyleGraphicsPropertiesElement->addAttribute("draw:shadow-color", mxStyle["draw:shadow-color"]->getStr());
        if (mxStyle["draw:shadow-opacity"])
            pStyleGraphicsPropertiesElement->addAttribute("draw:shadow-opacity", mxStyle["draw:shadow-opacity"]->getStr());
        if (mxStyle["svg:fill-rule"])
            pStyleGraphicsPropertiesElement->addAttribute("svg:fill-rule", mxStyle["svg:fill-rule"]->getStr());
    }

    if(mxStyle["draw:fill"] && mxStyle["draw:fill"]->getStr() == "solid")
    {
        pStyleGraphicsPropertiesElement->addAttribute("draw:fill", "solid");
        if (mxStyle["draw:fill-color"])
            pStyleGraphicsPropertiesElement->addAttribute("draw:fill-color", mxStyle["draw:fill-color"]->getStr());
        if (mxStyle["draw:opacity"])
            pStyleGraphicsPropertiesElement->addAttribute("draw:opacity", mxStyle["draw:opacity"]->getStr());
    }

    if(mxStyle["draw:fill"] && mxStyle["draw:fill"]->getStr() == "gradient")
    {
        if (!mxGradient.count() || mxGradient.count() >= 2)
        {
            pStyleGraphicsPropertiesElement->addAttribute("draw:fill", "gradient");
            sValue.sprintf("Gradient_%i", miGradientIndex-1);
            pStyleGraphicsPropertiesElement->addAttribute("draw:fill-gradient-name", sValue);
            if (bUseOpacityGradient)
            {
                sValue.sprintf("Transparency_%i", miGradientIndex-1);
                pStyleGraphicsPropertiesElement->addAttribute("draw:opacity-name", sValue);
            }
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

    if(mxStyle["draw:fill"] && mxStyle["draw:fill"]->getStr() == "bitmap")
    {
        if (mxStyle["draw:fill-image"] && mxStyle["libwpg:mime-type"])
        {
            pStyleGraphicsPropertiesElement->addAttribute("draw:fill", "bitmap");
            sValue.sprintf("Bitmap_%i", miBitmapIndex-1);
            pStyleGraphicsPropertiesElement->addAttribute("draw:fill-image-name", sValue);
            if (mxStyle["svg:width"])
                pStyleGraphicsPropertiesElement->addAttribute("draw:fill-image-width", mxStyle["svg:width"]->getStr());
            if (mxStyle["svg:height"])
                pStyleGraphicsPropertiesElement->addAttribute("draw:fill-image-height", mxStyle["svg:height"]->getStr());
            if (mxStyle["style:repeat"])
                pStyleGraphicsPropertiesElement->addAttribute("style:repeat", mxStyle["style:repeat"]->getStr());
            if (mxStyle["draw:fill-image-ref-point"])
                pStyleGraphicsPropertiesElement->addAttribute("draw:fill-image-ref-point", mxStyle["draw:fill-image-ref-point"]->getStr());
            if (mxStyle["draw:fill-image-ref-point-x"])
                pStyleGraphicsPropertiesElement->addAttribute("draw:fill-image-ref-point-x", mxStyle["draw:fill-image-ref-point-x"]->getStr());
            if (mxStyle["draw:fill-image-ref-point-y"])
                pStyleGraphicsPropertiesElement->addAttribute("draw:fill-image-ref-point-y", mxStyle["draw:fill-image-ref-point-y"]->getStr());
        }
        else
            pStyleGraphicsPropertiesElement->addAttribute("draw:fill", "none");
    }


    if(mxStyle["draw:marker-start-path"])
    {
        sValue.sprintf("StartMarker_%i", miStartMarkerIndex++);
        pStyleGraphicsPropertiesElement->addAttribute("draw:marker-start", sValue);
        if (mxStyle["draw:marker-start-width"])
            pStyleGraphicsPropertiesElement->addAttribute("draw:marker-start-width", mxStyle["draw:marker-start-width"]->getStr());
        else
            pStyleGraphicsPropertiesElement->addAttribute("draw:marker-start-width", "0.118in");
    }
    if (mxStyle["draw:marker-end-path"])
    {
        sValue.sprintf("EndMarker_%i", miEndMarkerIndex++);
        pStyleGraphicsPropertiesElement->addAttribute("draw:marker-end", sValue);
        if (mxStyle["draw:marker-end-width"])
            pStyleGraphicsPropertiesElement->addAttribute("draw:marker-end-width", mxStyle["draw:marker-end-width"]->getStr());
        else
            pStyleGraphicsPropertiesElement->addAttribute("draw:marker-end-width", "0.118in");
    }
    if (mxStyle["style:mirror"])
        pStyleGraphicsPropertiesElement->addAttribute("style:mirror", mxStyle["style:mirror"]->getStr());

    mGraphicsAutomaticStyles.push_back(pStyleGraphicsPropertiesElement);
    mGraphicsAutomaticStyles.push_back(new TagCloseElement("style:graphic-properties"));

    mGraphicsAutomaticStyles.push_back(new TagCloseElement("style:style"));
    miGraphicsStyleIndex++;
}

void OdgGenerator::startEmbeddedGraphics(const WPXPropertyList &)
{
}

void OdgGenerator::endEmbeddedGraphics()
{
}

void OdgGenerator::startTextObject(const WPXPropertyList &propList, const WPXPropertyListVector &)
{
    TagOpenElement *pDrawFrameOpenElement = new TagOpenElement("draw:frame");
    TagOpenElement *pStyleStyleOpenElement = new TagOpenElement("style:style");

    WPXString sValue;
    sValue.sprintf("gr%i",  mpImpl->miGraphicsStyleIndex++);
    pStyleStyleOpenElement->addAttribute("style:name", sValue);
    pStyleStyleOpenElement->addAttribute("style:family", "graphic");
    pStyleStyleOpenElement->addAttribute("style:parent-style-name", "standard");
    mpImpl->mGraphicsAutomaticStyles.push_back(pStyleStyleOpenElement);

    pDrawFrameOpenElement->addAttribute("draw:style-name", sValue);
    pDrawFrameOpenElement->addAttribute("draw:layer", "layout");

    TagOpenElement *pStyleGraphicPropertiesOpenElement = new TagOpenElement("style:graphic-properties");
    pStyleGraphicPropertiesOpenElement->addAttribute("draw:stroke", "none");
    pStyleGraphicPropertiesOpenElement->addAttribute("svg:stroke-color", "#000000");
    pStyleGraphicPropertiesOpenElement->addAttribute("draw:fill", "none");
    pStyleGraphicPropertiesOpenElement->addAttribute("draw:fill-color", "#ffffff");

    double x = 0.0;
    double y = 0.0;
    double height = 0.0;
    double width = 0.0;
    if (propList["svg:x"])
        x = propList["svg:x"]->getDouble();
    if (propList["svg:y"])
        y = propList["svg:y"]->getDouble();
    if (propList["svg:width"])
        width = propList["svg:width"]->getDouble();
    if (propList["svg:height"])
        height = propList["svg:height"]->getDouble();

    double angle(propList["libwpg:rotate"] ? - M_PI * propList["libwpg:rotate"]->getDouble() / 180.0 : 0.0);
    if (angle != 0.0)
    {
        double deltax((width*cos(angle)+height*sin(angle)-width)/2.0);
        double deltay((-width*sin(angle)+height*cos(angle)-height)/2.0);
        x -= deltax;
        y -= deltay;
    }

    if (!propList["svg:width"] && !propList["svg:height"])
    {
        if (!propList["fo:min-width"])
        {
            pDrawFrameOpenElement->addAttribute("fo:min-width", "1in");
            pStyleGraphicPropertiesOpenElement->addAttribute("fo:min-width", "1in");
        }
        pDrawFrameOpenElement->addAttribute("svg:width", "10in");
    }
    else
    {
        if(propList["svg:width"])
            pDrawFrameOpenElement->addAttribute("svg:width", propList["svg:width"]->getStr());
        if(propList["svg:height"])
            pDrawFrameOpenElement->addAttribute("svg:height", propList["svg:height"]->getStr());
    }
    if (propList["fo:min-width"])
    {
        pDrawFrameOpenElement->addAttribute("fo:min-width", propList["fo:min-width"]->getStr());
        pStyleGraphicPropertiesOpenElement->addAttribute("fo:min-width", propList["fo:min-width"]->getStr());
    }
    if (propList["fo:min-height"])
    {
        pDrawFrameOpenElement->addAttribute("fo:min-height", propList["fo:min-height"]->getStr());
        pStyleGraphicPropertiesOpenElement->addAttribute("fo:min-height", propList["fo:min-height"]->getStr());
    }
    if (propList["fo:max-width"])
    {
        pDrawFrameOpenElement->addAttribute("fo:max-width", propList["fo:max-height"]->getStr());
        pStyleGraphicPropertiesOpenElement->addAttribute("fo:max-width", propList["fo:max-width"]->getStr());
    }
    if (propList["fo:max-height"])
    {
        pDrawFrameOpenElement->addAttribute("fo:max-height", propList["fo:max-height"]->getStr());
        pStyleGraphicPropertiesOpenElement->addAttribute("fo:max-height", propList["fo:max-height"]->getStr());
    }
    if (propList["fo:padding-top"])
    {
        pDrawFrameOpenElement->addAttribute("fo:padding-top", propList["fo:padding-top"]->getStr());
        pStyleGraphicPropertiesOpenElement->addAttribute("fo:padding-top", propList["fo:padding-top"]->getStr());
    }
    if (propList["fo:padding-bottom"])
    {
        pDrawFrameOpenElement->addAttribute("fo:padding-bottom", propList["fo:padding-bottom"]->getStr());
        pStyleGraphicPropertiesOpenElement->addAttribute("fo:padding-bottom", propList["fo:padding-bottom"]->getStr());
    }
    if (propList["fo:padding-left"])
    {
        pDrawFrameOpenElement->addAttribute("fo:padding-left", propList["fo:padding-left"]->getStr());
        pStyleGraphicPropertiesOpenElement->addAttribute("fo:padding-left", propList["fo:padding-left"]->getStr());
    }
    if (propList["fo:padding-right"])
    {
        pDrawFrameOpenElement->addAttribute("fo:padding-right", propList["fo:padding-right"]->getStr());
        pStyleGraphicPropertiesOpenElement->addAttribute("fo:padding-right", propList["fo:padding-right"]->getStr());
    }
    if (propList["draw:textarea-vertical-align"])
    {
        pDrawFrameOpenElement->addAttribute("draw:textarea-vertical-align", propList["draw:textarea-vertical-align"]->getStr());
        pStyleGraphicPropertiesOpenElement->addAttribute("draw:textarea-vertical-align", propList["draw:textarea-vertical-align"]->getStr());
    }
    if (propList["draw:fill"])
    {
        pDrawFrameOpenElement->addAttribute("draw:fill", propList["draw:fill"]->getStr());
        pStyleGraphicPropertiesOpenElement->addAttribute("draw:fill", propList["draw:fill"]->getStr());
    }
    if (propList["draw:fill-color"])
    {
        pDrawFrameOpenElement->addAttribute("draw:fill-color", propList["draw:fill-color"]->getStr());
        pStyleGraphicPropertiesOpenElement->addAttribute("draw:fill-color", propList["draw:fill-color"]->getStr());
    }
    if (propList["draw:opacity"])
    {
        pDrawFrameOpenElement->addAttribute("draw:opacity", propList["draw:opacity"]->getStr());
        pStyleGraphicPropertiesOpenElement->addAttribute("draw:opacity", propList["draw:opacity"]->getStr());
    }

    WPXProperty *svg_x = WPXPropertyFactory::newInchProp(x);
    WPXProperty *svg_y = WPXPropertyFactory::newInchProp(y);
    if (angle != 0.0)
    {
        WPXProperty *libwpg_rotate = WPXPropertyFactory::newDoubleProp(angle);
        sValue.sprintf("rotate (%s) translate(%s, %s)",
                       libwpg_rotate->getStr().cstr(),
                       svg_x->getStr().cstr(),
                       svg_y->getStr().cstr());
        delete libwpg_rotate;
        pDrawFrameOpenElement->addAttribute("draw:transform", sValue);
    }
    else
    {
        if (propList["svg:x"])
            pDrawFrameOpenElement->addAttribute("svg:x", svg_x->getStr());
        if (propList["svg:y"])
            pDrawFrameOpenElement->addAttribute("svg:y", svg_y->getStr());
    }
    delete svg_x;
    delete svg_y;
    mpImpl->mBodyElements.push_back(pDrawFrameOpenElement);
    mpImpl->mBodyElements.push_back(new TagOpenElement("draw:text-box"));
    mpImpl->mGraphicsAutomaticStyles.push_back(pStyleGraphicPropertiesOpenElement);
    mpImpl->mGraphicsAutomaticStyles.push_back(new TagCloseElement("style:graphic-properties"));
    mpImpl->mGraphicsAutomaticStyles.push_back(new TagCloseElement("style:style"));
    mpImpl->mbIsTextBox = true;
}

void OdgGenerator::endTextObject()
{
    if (mpImpl->mbIsTextBox)
    {
        mpImpl->mBodyElements.push_back(new TagCloseElement("draw:text-box"));
        mpImpl->mBodyElements.push_back(new TagCloseElement("draw:frame"));
        mpImpl->mbIsTextBox = false;
    }
}

void OdgGenerator::startTextLine(const WPXPropertyList &propList)
{
    WPXPropertyList finalPropList(propList);
    finalPropList.insert("style:parent-style-name", "Standard");
    WPXString paragName = mpImpl->mParagraphManager.findOrAdd(finalPropList, WPXPropertyListVector());


    // create a document element corresponding to the paragraph, and append it to our list of document elements
    TagOpenElement *pParagraphOpenElement = new TagOpenElement("text:p");
    pParagraphOpenElement->addAttribute("text:style-name", paragName);
    mpImpl->mBodyElements.push_back(pParagraphOpenElement);
}

void OdgGenerator::endTextLine()
{
    mpImpl->mBodyElements.push_back(new TagCloseElement("text:p"));
}

void OdgGenerator::startTextSpan(const WPXPropertyList &propList)
{
    if (propList["style:font-name"])
        mpImpl->mFontManager.findOrAdd(propList["style:font-name"]->getStr().cstr());

    WPXString sName = mpImpl->mSpanManager.findOrAdd(propList);

    TagOpenElement *pSpanOpenElement = new TagOpenElement("text:span");
    pSpanOpenElement->addAttribute("text:style-name", sName.cstr());
    mpImpl->mBodyElements.push_back(pSpanOpenElement);
}

void OdgGenerator::endTextSpan()
{
    mpImpl->mBodyElements.push_back(new TagCloseElement("text:span"));
}

void OdgGenerator::insertText(const WPXString &text)
{
    WPXString out;
    WPXString::Iter i(text);
    for (i.rewind(); i.next();)
    {
        if ((*i()) == '\n' || (*i()) == '\t')
        {
            if (out.len() != 0)
            {
                DocumentElement *pText = new TextElement(out);
                mpImpl->mBodyElements.push_back(pText);
                out.clear();
            }
            if ((*i()) == '\n')
            {
                mpImpl->mBodyElements.push_back(new TagOpenElement("text:line-break"));
                mpImpl->mBodyElements.push_back(new TagCloseElement("text:line-break"));
            }
            else if ((*i()) == '\t')
            {
                mpImpl->mBodyElements.push_back(new TagOpenElement("text:tab"));
                mpImpl->mBodyElements.push_back(new TagCloseElement("text:tab"));
            }
        }
        else
        {
            out.append(i());
        }
    }
    if (out.len() != 0)
    {
        DocumentElement *pText = new TextElement(out);
        mpImpl->mBodyElements.push_back(pText);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
