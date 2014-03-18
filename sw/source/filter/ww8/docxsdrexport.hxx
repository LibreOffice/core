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
#include <com/sun/star/xml/dom/XDocument.hpp>
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

namespace sw
{
class Frame;
}
class SwFrmFmt;
class SwNode;

class DocxExport;

/// Helper class, so that the DocxExport::RestoreData() call will always happen.
class ExportDataSaveRestore
{
private:
    DocxExport& m_rExport;
public:
    ExportDataSaveRestore(DocxExport& rExport, sal_uLong nStt, sal_uLong nEnd, sw::Frame* pParentFrame);
    ~ExportDataSaveRestore();
};

/// Handles DOCX export of drawings.
class DocxSdrExport
{
    struct Impl;
    boost::shared_ptr<Impl> m_pImpl;
public:
    DocxSdrExport(DocxExport& rExport, sax_fastparser::FSHelperPtr pSerializer, oox::drawingml::DrawingML* pDrawingML);
    ~DocxSdrExport();

    void setSerializer(sax_fastparser::FSHelperPtr pSerializer);
    /// When exporting fly frames, this holds the real size of the frame.
    const Size* getFlyFrameSize();
    bool getTextFrameSyntax();
    bool getDMLTextFrameSyntax();
    sax_fastparser::FastAttributeList*& getFlyAttrList();
    void setFlyAttrList(sax_fastparser::FastAttributeList* pAttrList);
    /// Attributes of the next v:textbox element.
    sax_fastparser::FastAttributeList* getTextboxAttrList();
    OStringBuffer& getTextFrameStyle();
    /// Same, as DocxAttributeOutput::m_bBtLr, but for textframe rotation.
    bool getFrameBtLr();
    void setFrameBtLr(bool bFrameBtLr);
    bool getFlyFrameGraphic();
    void setFlyFrameGraphic(bool bFlyFrameGraphic);
    sax_fastparser::FastAttributeList*& getFlyFillAttrList();
    sax_fastparser::FastAttributeList* getFlyWrapAttrList();
    void setFlyWrapAttrList(sax_fastparser::FastAttributeList* pAttrList);
    /// Attributes of <wps:bodyPr>, used during DML export of text frames.
    sax_fastparser::FastAttributeList* getBodyPrAttrList();
    sax_fastparser::FastAttributeList*& getDashLineStyle();

    void startDMLAnchorInline(const SwFrmFmt* pFrmFmt, const Size& rSize);
    void endDMLAnchorInline(const SwFrmFmt* pFrmFmt);
    /// Writes a drawing as VML data.
    void writeVMLDrawing(const SdrObject* sdrObj, const SwFrmFmt& rFrmFmt,const Point& rNdTopLeft);
    /// Writes a drawing as DML.
    void writeDMLDrawing(const SdrObject* pSdrObj, const SwFrmFmt* pFrmFmt, int nAnchorId);
    /// Writes shape in both DML and VML format.
    void writeDMLAndVMLDrawing(const SdrObject* sdrObj, const SwFrmFmt& rFrmFmt,const Point& rNdTopLeft, int nAnchorId);
    /// Write <a:effectLst>, the effect list.
    void writeDMLEffectLst(const SwFrmFmt& rFrmFmt);
    /// Writes a diagram (smartart).
    void writeDiagram(const SdrObject* sdrObject, const SwFrmFmt& rFrmFmt, int nAnchorId);
    void writeDiagramRels(com::sun::star::uno::Reference< com::sun::star::xml::dom::XDocument> xDom,
                          com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< com::sun::star::uno::Any > > xRelSeq,
                          com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > xOutStream, const OUString& sGrabBagProperyName);
    /// Writes text frame in DML format.
    void writeDMLTextFrame(sw::Frame* pParentFrame, int nAnchorId);
    /// Writes text frame in VML format.
    void writeVMLTextFrame(sw::Frame* pParentFrame);
    /// Undo the text direction mangling done by the frame btLr handler in writerfilter::dmapper::DomainMapper::lcl_startCharacterGroup()
    bool checkFrameBtlr(SwNode* pStartNode, sax_fastparser::FastAttributeList* pTextboxAttrList = 0);
};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_DOCXSDREXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
