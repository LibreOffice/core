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

#include <memory>

#include <rtl/strbuf.hxx>
#include <sax/fshelper.hxx>
#include <tools/solar.h>

namespace rtl
{
template <typename> class Reference;
}
namespace oox::drawingml { class DrawingML; }
class Size;
class SdrObject;
class SvxBoxItem;

namespace ww8
{
class Frame;
}
class SwFrameFormat;

class DocxExport;

/// Helper class, so that the DocxExport::RestoreData() call will always happen.
class ExportDataSaveRestore
{
private:
    DocxExport& m_rExport;
public:
    ExportDataSaveRestore(DocxExport& rExport, sal_uLong nStt, sal_uLong nEnd, ww8::Frame const* pParentFrame);
    ~ExportDataSaveRestore();
};

/// Handles DOCX export of drawings.
class DocxSdrExport
{
    struct Impl;
    std::unique_ptr<Impl> m_pImpl;
public:
    DocxSdrExport(DocxExport& rExport, const sax_fastparser::FSHelperPtr& pSerializer, oox::drawingml::DrawingML* pDrawingML);
    ~DocxSdrExport();

    void setSerializer(const sax_fastparser::FSHelperPtr& pSerializer);
    /// When exporting fly frames, this holds the real size of the frame.
    const Size* getFlyFrameSize() const;
    bool getTextFrameSyntax() const;
    bool getDMLTextFrameSyntax() const;
    rtl::Reference<sax_fastparser::FastAttributeList>& getFlyAttrList();
    /// Attributes of the next v:textbox element.
    rtl::Reference<sax_fastparser::FastAttributeList>& getTextboxAttrList();
    OStringBuffer& getTextFrameStyle();

    /// Set if paragraph sdt open in the current drawing.
    void setParagraphSdtOpen(bool bParagraphSdtOpen);

    bool IsDrawingOpen() const;
    bool IsDMLAndVMLDrawingOpen() const;
    bool IsParagraphHasDrawing() const;
    void setParagraphHasDrawing(bool bParagraphHasDrawing);
    rtl::Reference<sax_fastparser::FastAttributeList>& getFlyFillAttrList();
    void setFlyWrapAttrList(rtl::Reference<sax_fastparser::FastAttributeList> const & pAttrList);
    /// Attributes of <wps:bodyPr>, used during DML export of text frames.
    sax_fastparser::FastAttributeList* getBodyPrAttrList();
    rtl::Reference<sax_fastparser::FastAttributeList>& getDashLineStyle();

    void startDMLAnchorInline(const SwFrameFormat* pFrameFormat, const Size& rSize);
    void endDMLAnchorInline(const SwFrameFormat* pFrameFormat);
    /// Writes a drawing as VML data.
    void writeVMLDrawing(const SdrObject* sdrObj, const SwFrameFormat& rFrameFormat);
    /// Writes a drawing as DML.
    void writeDMLDrawing(const SdrObject* pSdrObject, const SwFrameFormat* pFrameFormat, int nAnchorId);
    /// Writes shape in both DML and VML format.
    void writeDMLAndVMLDrawing(const SdrObject* sdrObj, const SwFrameFormat& rFrameFormat, int nAnchorId);
    /// Write <a:effectLst>, the effect list.
    void writeDMLEffectLst(const SwFrameFormat& rFrameFormat);
    /// Writes a diagram (smartart).
    void writeDiagram(const SdrObject* sdrObject, const SwFrameFormat& rFrameFormat, int nDiagramId);
    /// Writes text frame in DML format.
    void writeDMLTextFrame(ww8::Frame const* pParentFrame, int nAnchorId, bool bTextBoxOnly = false);
    /// Writes text frame in VML format.
    void writeVMLTextFrame(ww8::Frame const* pParentFrame, bool bTextBoxOnly = false);
    /// Is this a standalone TextFrame, or used as a TextBox of a shape?
    static bool isTextBox(const SwFrameFormat& rFrameFormat);
    /// Writes text from Textbox for <w:framePr>
    void writeOnlyTextOfFrame(ww8::Frame const* pParentFrame);
    /// Writes the drawingML <a:ln> markup of a box item.
    void writeBoxItemLine(const SvxBoxItem& rBox);
};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_DOCXSDREXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
