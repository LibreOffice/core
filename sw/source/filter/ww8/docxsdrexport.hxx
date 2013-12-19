/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_DOCXSDREXPORT_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_DOCXSDREXPORT_HXX

#include <boost/shared_ptr.hpp>
#include <sax/fshelper.hxx>

namespace oox
{
namespace drawingml
{
class DrawingML;
}
}
class Size;
class Point;
class SdrObject;

class SwFrmFmt;

class DocxExport;

/// Handles DOCX export of drawings.
class DocxSdrExport
{
    struct Impl;
    boost::shared_ptr<Impl> m_pImpl;
public:
    DocxSdrExport(DocxExport& rExport, sax_fastparser::FSHelperPtr pSerializer, oox::drawingml::DrawingML* pDrawingML);
    ~DocxSdrExport();

    void setSerializer(sax_fastparser::FSHelperPtr pSerializer);

    void startDMLAnchorInline(const SwFrmFmt* pFrmFmt, const Size& rSize);
    void endDMLAnchorInline(const SwFrmFmt* pFrmFmt);
    /// Writes a drawing as VML data.
    void writeVMLDrawing(const SdrObject* sdrObj, const SwFrmFmt& rFrmFmt,const Point& rNdTopLeft);
    /// Writes shape in both DML and VML format.
    void writeDMLAndVMLDrawing(const SdrObject* sdrObj, const SwFrmFmt& rFrmFmt,const Point& rNdTopLeft, int nAnchorId);
    /// Writes a diagram (smartart).
    void writeDiagram(const SdrObject* sdrObject, const Size& size);
    /// Write <a:effectLst>, the effect list.
    void writeDMLEffectLst(const SwFrmFmt& rFrmFmt);
};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_DOCXSDREXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
