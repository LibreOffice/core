/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information visit http://libwpg.sourceforge.net
 */

#ifndef __ODGEXPORTER_HXX__
#define __ODGEXPORTER_HXX__

#include <libwpd/libwpd.h>
#include <libwpg/libwpg.h>

#include "OdfDocumentHandler.hxx"

class OdgGeneratorPrivate;

class OdgGenerator : public libwpg::WPGPaintInterface
{
public:
    OdgGenerator(OdfDocumentHandler *pHandler, const OdfStreamType streamType);
    ~OdgGenerator();

    void startGraphics(const ::WPXPropertyList &propList);
    void endGraphics();
    void startLayer(const ::WPXPropertyList &propList);
    void endLayer();
    void startEmbeddedGraphics(const ::WPXPropertyList &propList);
    void endEmbeddedGraphics();

    void setStyle(const ::WPXPropertyList &propList, const ::WPXPropertyListVector &gradient);

    void drawRectangle(const ::WPXPropertyList &propList);
    void drawEllipse(const ::WPXPropertyList &propList);
    void drawPolyline(const ::WPXPropertyListVector &vertices);
    void drawPolygon(const ::WPXPropertyListVector &vertices);
    void drawPath(const ::WPXPropertyListVector &path);
    void drawGraphicObject(const ::WPXPropertyList &propList, const ::WPXBinaryData &binaryData);

    void startTextObject(const ::WPXPropertyList &propList, const ::WPXPropertyListVector &path);
    void endTextObject();
    void startTextLine(const ::WPXPropertyList &propList);
    void endTextLine();
    void startTextSpan(const ::WPXPropertyList &propList);
    void endTextSpan();
    void insertText(const ::WPXString &str);

private:
    OdgGeneratorPrivate *mpImpl;
};

#endif // __ODGEXPORTER_HXX__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
