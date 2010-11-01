/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* libwpg
 * Copyright (C) 2006 Ariya Hidayat (ariya@kde.org)
 * Copyright (C) 2007 Fridrich Strba (fridrich_strba@bluewin.ch)
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

#ifndef __ODGEXPORTER_HXX__
#define __ODGEXPORTER_HXX__

#include <iostream>
#include <sstream>
#include <string>

#include <libwpd/WPXString.h>
#include <libwpg/libwpg.h>
#include <libwpg/WPGBinaryData.h>
#include "filter/DocumentElement.hxx"
#include "filter/DocumentHandler.hxx"

class OdgExporter : public libwpg::WPGPaintInterface {
public:
    OdgExporter(DocumentHandler *pHandler);
    ~OdgExporter();

    void startDocument(double width, double height);
    void startGraphics(double width, double height) { startDocument(width, height); }
    void endDocument();
    void endGraphics() { endDocument(); };
    void startLayer(unsigned int id);
    void endLayer(unsigned int id);

    void setPen(const libwpg::WPGPen& pen);
    void setBrush(const libwpg::WPGBrush& brush);
    void setFillRule(FillRule rule);

    void drawRectangle(const libwpg::WPGRect& rect, double rx, double ry);
    void drawEllipse(const libwpg::WPGPoint& center, double rx, double ry);
    void drawPolygon(const libwpg::WPGPointArray& vertices);
    void drawPath(const libwpg::WPGPath& path);
    void drawBitmap(const libwpg::WPGBitmap& bitmap);
    void drawImageObject(const libwpg::WPGBinaryData& binaryData);

private:
    std::vector <DocumentElement *> mpBodyElements;
    std::vector <DocumentElement *> mpStylesElements;
    DocumentHandler *mpHandler;

    libwpg::WPGPen m_pen;
    libwpg::WPGBrush m_brush;
    FillRule m_fillRule;
    int m_gradientIndex;
    int m_dashIndex;
    int m_styleIndex;
    void writeStyle();
    WPXString doubleToString(const double value);
};

#endif // __ODGEXPORTER_HXX__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
