/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/sax/XSAXSerializable.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/unoprnms.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdogrp.hxx>
#include <oox/token/tokens.hxx>
#include <oox/export/drawingml.hxx>
#include <oox/export/utils.hxx>
#include <oox/export/vmlexport.hxx>

#include <frmatr.hxx>
#include <frmfmt.hxx>
#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>

#include <docxsdrexport.hxx>
#include <docxexport.hxx>
#include <docxexportfilter.hxx>

using namespace com::sun::star;
using namespace oox;

/// Holds data used by DocxSdrExport only.
struct DocxSdrExport::Impl
{
    DocxSdrExport& m_rSdrExport;
    DocxExport& m_rExport;
    sax_fastparser::FSHelperPtr m_pSerializer;
    oox::drawingml::DrawingML* m_pDrawingML;

    Impl(DocxSdrExport& rSdrExport, DocxExport& rExport, sax_fastparser::FSHelperPtr pSerializer, oox::drawingml::DrawingML* pDrawingML)
        : m_rSdrExport(rSdrExport),
          m_rExport(rExport),
          m_pSerializer(pSerializer),
          m_pDrawingML(pDrawingML)
    {
    }
    /// Writes wp wrapper code around an SdrObject, which itself is written using drawingML syntax.
    void writeDMLDrawing(const SdrObject* pSdrObj, const SwFrmFmt* pFrmFmt, int nAnchorId);
};

DocxSdrExport::DocxSdrExport(DocxExport& rExport, sax_fastparser::FSHelperPtr pSerializer, oox::drawingml::DrawingML* pDrawingML)
    : m_pImpl(new Impl(*this, rExport, pSerializer, pDrawingML))
{
}

DocxSdrExport::~DocxSdrExport()
{
}

void DocxSdrExport::setSerializer(sax_fastparser::FSHelperPtr pSerializer)
{
    m_pImpl->m_pSerializer = pSerializer;
}

void DocxSdrExport::startDMLAnchorInline(const SwFrmFmt* pFrmFmt, const Size& rSize)
{
    m_pImpl->m_pSerializer->startElementNS(XML_w, XML_drawing, FSEND);

    const SvxLRSpaceItem pLRSpaceItem = pFrmFmt->GetLRSpace(false);
    const SvxULSpaceItem pULSpaceItem = pFrmFmt->GetULSpace(false);

    bool isAnchor = pFrmFmt->GetAnchor().GetAnchorId() != FLY_AS_CHAR;
    if (isAnchor)
    {
        ::sax_fastparser::FastAttributeList* attrList = m_pImpl->m_pSerializer->createAttrList();
        attrList->add(XML_behindDoc, pFrmFmt->GetOpaque().GetValue() ? "0" : "1");
        attrList->add(XML_distT, OString::number(TwipsToEMU(pULSpaceItem.GetUpper())).getStr());
        attrList->add(XML_distB, OString::number(TwipsToEMU(pULSpaceItem.GetLower())).getStr());
        attrList->add(XML_distL, OString::number(TwipsToEMU(pLRSpaceItem.GetLeft())).getStr());
        attrList->add(XML_distR, OString::number(TwipsToEMU(pLRSpaceItem.GetRight())).getStr());
        attrList->add(XML_simplePos, "0");
        attrList->add(XML_locked, "0");
        attrList->add(XML_layoutInCell, "1");
        attrList->add(XML_allowOverlap, "1");   // TODO
        if (const SdrObject* pObj = pFrmFmt->FindRealSdrObject())
            attrList->add(XML_relativeHeight, OString::number(pObj->GetOrdNum()));
        sax_fastparser::XFastAttributeListRef xAttrList(attrList);
        m_pImpl->m_pSerializer->startElementNS(XML_wp, XML_anchor, xAttrList);
        m_pImpl->m_pSerializer->singleElementNS(XML_wp, XML_simplePos, XML_x, "0", XML_y, "0", FSEND);   // required, unused
        const char* relativeFromH;
        const char* relativeFromV;
        const char* alignH = NULL;
        const char* alignV = NULL;
        switch (pFrmFmt->GetVertOrient().GetRelationOrient())
        {
        case text::RelOrientation::PAGE_PRINT_AREA:
            relativeFromV = "margin";
            break;
        case text::RelOrientation::PAGE_FRAME:
            relativeFromV = "page";
            break;
        case text::RelOrientation::FRAME:
            relativeFromV = "paragraph";
            break;
        case text::RelOrientation::TEXT_LINE:
        default:
            relativeFromV = "line";
            break;
        }
        switch (pFrmFmt->GetVertOrient().GetVertOrient())
        {
        case text::VertOrientation::TOP:
        case text::VertOrientation::CHAR_TOP:
        case text::VertOrientation::LINE_TOP:
            if (pFrmFmt->GetVertOrient().GetRelationOrient() == text::RelOrientation::TEXT_LINE)
                alignV = "bottom";
            else
                alignV = "top";
            break;
        case text::VertOrientation::BOTTOM:
        case text::VertOrientation::CHAR_BOTTOM:
        case text::VertOrientation::LINE_BOTTOM:
            if (pFrmFmt->GetVertOrient().GetRelationOrient() == text::RelOrientation::TEXT_LINE)
                alignV = "top";
            else
                alignV = "bottom";
            break;
        case text::VertOrientation::CENTER:
        case text::VertOrientation::CHAR_CENTER:
        case text::VertOrientation::LINE_CENTER:
            alignV = "center";
            break;
        default:
            break;
        }
        switch (pFrmFmt->GetHoriOrient().GetRelationOrient())
        {
        case text::RelOrientation::PAGE_PRINT_AREA:
            relativeFromH = "margin";
            break;
        case text::RelOrientation::PAGE_FRAME:
            relativeFromH = "page";
            break;
        case text::RelOrientation::CHAR:
            relativeFromH = "character";
            break;
        case text::RelOrientation::FRAME:
        default:
            relativeFromH = "column";
            break;
        }
        switch (pFrmFmt->GetHoriOrient().GetHoriOrient())
        {
        case text::HoriOrientation::LEFT:
            alignH = "left";
            break;
        case text::HoriOrientation::RIGHT:
            alignH = "right";
            break;
        case text::HoriOrientation::CENTER:
            alignH = "center";
            break;
        case text::HoriOrientation::INSIDE:
            alignH = "inside";
            break;
        case text::HoriOrientation::OUTSIDE:
            alignH = "outside";
            break;
        default:
            break;
        }
        m_pImpl->m_pSerializer->startElementNS(XML_wp, XML_positionH, XML_relativeFrom, relativeFromH, FSEND);
        if (alignH != NULL)
        {
            m_pImpl->m_pSerializer->startElementNS(XML_wp, XML_align, FSEND);
            m_pImpl->m_pSerializer->write(alignH);
            m_pImpl->m_pSerializer->endElementNS(XML_wp, XML_align);
        }
        else
        {
            m_pImpl->m_pSerializer->startElementNS(XML_wp, XML_posOffset, FSEND);
            m_pImpl->m_pSerializer->write(TwipsToEMU(pFrmFmt->GetHoriOrient().GetPos()));
            m_pImpl->m_pSerializer->endElementNS(XML_wp, XML_posOffset);
        }
        m_pImpl->m_pSerializer->endElementNS(XML_wp, XML_positionH);
        m_pImpl->m_pSerializer->startElementNS(XML_wp, XML_positionV, XML_relativeFrom, relativeFromV, FSEND);
        if (alignV != NULL)
        {
            m_pImpl->m_pSerializer->startElementNS(XML_wp, XML_align, FSEND);
            m_pImpl->m_pSerializer->write(alignV);
            m_pImpl->m_pSerializer->endElementNS(XML_wp, XML_align);
        }
        else
        {
            m_pImpl->m_pSerializer->startElementNS(XML_wp, XML_posOffset, FSEND);
            m_pImpl->m_pSerializer->write(TwipsToEMU(pFrmFmt->GetVertOrient().GetPos()));
            m_pImpl->m_pSerializer->endElementNS(XML_wp, XML_posOffset);
        }
        m_pImpl->m_pSerializer->endElementNS(XML_wp, XML_positionV);
    }
    else
    {
        m_pImpl->m_pSerializer->startElementNS(XML_wp, XML_inline,
                                               XML_distT, OString::number(TwipsToEMU(pULSpaceItem.GetUpper())).getStr(),
                                               XML_distB, OString::number(TwipsToEMU(pULSpaceItem.GetLower())).getStr(),
                                               XML_distL, OString::number(TwipsToEMU(pLRSpaceItem.GetLeft())).getStr(),
                                               XML_distR, OString::number(TwipsToEMU(pLRSpaceItem.GetRight())).getStr(),
                                               FSEND);
    }

    // now the common parts
    // extent of the image
    OString aWidth(OString::number(TwipsToEMU(rSize.Width())));
    OString aHeight(OString::number(TwipsToEMU(rSize.Height())));
    m_pImpl->m_pSerializer->singleElementNS(XML_wp, XML_extent,
                                            XML_cx, aWidth.getStr(),
                                            XML_cy, aHeight.getStr(),
                                            FSEND);

    // effectExtent, extent including the effect (shadow only for now)
    SvxShadowItem aShadowItem = pFrmFmt->GetShadow();
    OString aLeftExt("0"), aRightExt("0"), aTopExt("0"), aBottomExt("0");
    if (aShadowItem.GetLocation() != SVX_SHADOW_NONE)
    {
        OString aShadowWidth(OString::number(TwipsToEMU(aShadowItem.GetWidth())));
        switch (aShadowItem.GetLocation())
        {
        case SVX_SHADOW_TOPLEFT:
            aTopExt = aLeftExt = aShadowWidth;
            break;
        case SVX_SHADOW_TOPRIGHT:
            aTopExt = aRightExt = aShadowWidth;
            break;
        case SVX_SHADOW_BOTTOMLEFT:
            aBottomExt = aLeftExt = aShadowWidth;
            break;
        case SVX_SHADOW_BOTTOMRIGHT:
            aBottomExt = aRightExt = aShadowWidth;
            break;
        case SVX_SHADOW_NONE:
        case SVX_SHADOW_END:
            break;
        }
    }

    m_pImpl->m_pSerializer->singleElementNS(XML_wp, XML_effectExtent,
                                            XML_l, aLeftExt,
                                            XML_t, aTopExt,
                                            XML_r, aRightExt,
                                            XML_b, aBottomExt,
                                            FSEND);

    if (isAnchor)
    {
        switch (pFrmFmt->GetSurround().GetValue())
        {
        case SURROUND_NONE:
            m_pImpl->m_pSerializer->singleElementNS(XML_wp, XML_wrapTopAndBottom, FSEND);
            break;
        case SURROUND_THROUGHT:
            m_pImpl->m_pSerializer->singleElementNS(XML_wp, XML_wrapNone, FSEND);
            break;
        case SURROUND_PARALLEL:
            m_pImpl->m_pSerializer->singleElementNS(XML_wp, XML_wrapSquare,
                                                    XML_wrapText, "bothSides", FSEND);
            break;
        case SURROUND_IDEAL:
        default:
            m_pImpl->m_pSerializer->singleElementNS(XML_wp, XML_wrapSquare,
                                                    XML_wrapText, "largest", FSEND);
            break;
        }
    }
}

void DocxSdrExport::endDMLAnchorInline(const SwFrmFmt* pFrmFmt)
{
    bool isAnchor = pFrmFmt->GetAnchor().GetAnchorId() != FLY_AS_CHAR;
    m_pImpl->m_pSerializer->endElementNS(XML_wp, isAnchor ? XML_anchor : XML_inline);

    m_pImpl->m_pSerializer->endElementNS(XML_w, XML_drawing);
}

void DocxSdrExport::writeVMLDrawing(const SdrObject* sdrObj, const SwFrmFmt& rFrmFmt,const Point& rNdTopLeft)
{
    bool bSwapInPage = false;
    if (!(sdrObj)->GetPage())
    {
        if (SdrModel* pModel = m_pImpl->m_rExport.pDoc->GetDrawModel())
        {
            if (SdrPage* pPage = pModel->GetPage(0))
            {
                bSwapInPage = true;
                const_cast< SdrObject* >(sdrObj)->SetPage(pPage);
            }
        }
    }

    m_pImpl->m_pSerializer->startElementNS(XML_w, XML_pict, FSEND);
    m_pImpl->m_pDrawingML->SetFS(m_pImpl->m_pSerializer);
    // See WinwordAnchoring::SetAnchoring(), these are not part of the SdrObject, have to be passed around manually.

    SwFmtHoriOrient rHoriOri = (rFrmFmt).GetHoriOrient();
    SwFmtVertOrient rVertOri = (rFrmFmt).GetVertOrient();
    m_pImpl->m_rExport.VMLExporter().AddSdrObject(*(sdrObj),
            rHoriOri.GetHoriOrient(), rVertOri.GetVertOrient(),
            rHoriOri.GetRelationOrient(),
            rVertOri.GetRelationOrient(), (&rNdTopLeft), true);
    m_pImpl->m_pSerializer->endElementNS(XML_w, XML_pict);

    if (bSwapInPage)
        const_cast< SdrObject* >(sdrObj)->SetPage(0);
}

void DocxSdrExport::Impl::writeDMLDrawing(const SdrObject* pSdrObject, const SwFrmFmt* pFrmFmt, int nAnchorId)
{
    sax_fastparser::FSHelperPtr pFS = m_pSerializer;
    Size aSize(pSdrObject->GetSnapRect().GetWidth(), pSdrObject->GetSnapRect().GetHeight());
    m_rSdrExport.startDMLAnchorInline(pFrmFmt, aSize);

    sax_fastparser::FastAttributeList* pDocPrAttrList = pFS->createAttrList();
    pDocPrAttrList->add(XML_id, OString::number(nAnchorId).getStr());
    pDocPrAttrList->add(XML_name, OUStringToOString(pSdrObject->GetName(), RTL_TEXTENCODING_UTF8).getStr());
    sax_fastparser::XFastAttributeListRef xDocPrAttrListRef(pDocPrAttrList);
    pFS->singleElementNS(XML_wp, XML_docPr, xDocPrAttrListRef);

    uno::Reference<drawing::XShape> xShape(const_cast<SdrObject*>(pSdrObject)->getUnoShape(), uno::UNO_QUERY_THROW);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xShape, uno::UNO_QUERY_THROW);
    const char* pNamespace = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";
    if (xServiceInfo->supportsService("com.sun.star.drawing.GroupShape"))
        pNamespace = "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup";
    else if (xServiceInfo->supportsService("com.sun.star.drawing.GraphicObjectShape"))
        pNamespace = "http://schemas.openxmlformats.org/drawingml/2006/picture";
    pFS->startElementNS(XML_a, XML_graphic,
                        FSNS(XML_xmlns, XML_a), "http://schemas.openxmlformats.org/drawingml/2006/main",
                        FSEND);
    pFS->startElementNS(XML_a, XML_graphicData,
                        XML_uri, pNamespace,
                        FSEND);

    m_rExport.OutputDML(xShape);

    pFS->endElementNS(XML_a, XML_graphicData);
    pFS->endElementNS(XML_a, XML_graphic);

    m_rSdrExport.endDMLAnchorInline(pFrmFmt);
}

void DocxSdrExport::writeDMLAndVMLDrawing(const SdrObject* sdrObj, const SwFrmFmt& rFrmFmt,const Point& rNdTopLeft, int nAnchorId)
{
    // Depending on the shape type, we actually don't write the shape as DML.
    OUString sShapeType;
    sal_uInt32 nMirrorFlags = 0;
    uno::Reference<drawing::XShape> xShape(const_cast<SdrObject*>(sdrObj)->getUnoShape(), uno::UNO_QUERY_THROW);
    MSO_SPT eShapeType = EscherPropertyContainer::GetCustomShapeType(xShape, nMirrorFlags, sShapeType);

    if (eShapeType != ESCHER_ShpInst_TextPlainText)
    {
        m_pImpl->m_pSerializer->startElementNS(XML_mc, XML_AlternateContent, FSEND);

        const SdrObjGroup* pObjGroup = PTR_CAST(SdrObjGroup, sdrObj);
        m_pImpl->m_pSerializer->startElementNS(XML_mc, XML_Choice,
                                               XML_Requires, (pObjGroup ? "wpg" : "wps"),
                                               FSEND);
        m_pImpl->writeDMLDrawing(sdrObj, &rFrmFmt, nAnchorId);
        m_pImpl->m_pSerializer->endElementNS(XML_mc, XML_Choice);

        m_pImpl->m_pSerializer->startElementNS(XML_mc, XML_Fallback, FSEND);
        writeVMLDrawing(sdrObj, rFrmFmt, rNdTopLeft);
        m_pImpl->m_pSerializer->endElementNS(XML_mc, XML_Fallback);

        m_pImpl->m_pSerializer->endElementNS(XML_mc, XML_AlternateContent);
    }
    else
        writeVMLDrawing(sdrObj, rFrmFmt, rNdTopLeft);
}

void DocxSdrExport::writeDiagram(const SdrObject* sdrObject, const Size& size)
{
    sax_fastparser::FSHelperPtr pFS = m_pImpl->m_pSerializer;
    uno::Reference< drawing::XShape > xShape(((SdrObject*)sdrObject)->getUnoShape(), uno::UNO_QUERY);
    uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);

    uno::Reference<xml::dom::XDocument> dataDom;
    uno::Reference<xml::dom::XDocument> layoutDom;
    uno::Reference<xml::dom::XDocument> styleDom;
    uno::Reference<xml::dom::XDocument> colorDom;
    uno::Reference<xml::dom::XDocument> drawingDom;

    // retrieve the doms from the GrabBag
    OUString pName = UNO_NAME_MISC_OBJ_INTEROPGRABBAG;
    uno::Sequence< beans::PropertyValue > propList;
    xPropSet->getPropertyValue(pName) >>= propList;
    for (sal_Int32 nProp=0; nProp < propList.getLength(); ++nProp)
    {
        OUString propName = propList[nProp].Name;
        if (propName == "OOXData")
            propList[nProp].Value >>= dataDom;
        else if (propName == "OOXLayout")
            propList[nProp].Value >>= layoutDom;
        else if (propName == "OOXStyle")
            propList[nProp].Value >>= styleDom;
        else if (propName == "OOXColor")
            propList[nProp].Value >>= colorDom;
        else if (propName == "OOXDrawing")
            propList[nProp].Value >>= drawingDom;
    }

    // check that we have the 4 mandatory XDocuments
    // if not, there was an error importing and we won't output anything
    if (!dataDom.is() || !layoutDom.is() || !styleDom.is() || !colorDom.is())
        return;

    // write necessary tags to document.xml
    pFS->startElementNS(XML_w, XML_drawing,
                        FSEND);
    pFS->startElementNS(XML_wp, XML_inline,
                        XML_distT, "0", XML_distB, "0", XML_distL, "0", XML_distR, "0",
                        FSEND);

    OString aWidth(OString::number(TwipsToEMU(size.Width())));
    OString aHeight(OString::number(TwipsToEMU(size.Height())));
    pFS->singleElementNS(XML_wp, XML_extent,
                         XML_cx, aWidth.getStr(),
                         XML_cy, aHeight.getStr(),
                         FSEND);
    // TODO - the right effectExtent, extent including the effect
    pFS->singleElementNS(XML_wp, XML_effectExtent,
                         XML_l, "0", XML_t, "0", XML_r, "0", XML_b, "0",
                         FSEND);

    // generate an unique id
    static sal_Int32 diagramCount = 0;
    diagramCount++;
    OUString sName = "Diagram" + OUString::number(diagramCount);

    pFS->singleElementNS(XML_wp, XML_docPr,
                         XML_id, I32S(diagramCount),
                         XML_name, USS(sName),
                         FSEND);

    pFS->singleElementNS(XML_wp, XML_cNvGraphicFramePr,
                         FSEND);

    pFS->startElementNS(XML_a, XML_graphic,
                        FSNS(XML_xmlns, XML_a), "http://schemas.openxmlformats.org/drawingml/2006/main",
                        FSEND);

    pFS->startElementNS(XML_a, XML_graphicData,
                        XML_uri, "http://schemas.openxmlformats.org/drawingml/2006/diagram",
                        FSEND);

    // add data relation
    OUString dataFileName = "diagrams/data" + OUString::number(diagramCount) + ".xml";
    OString dataRelId = OUStringToOString(m_pImpl->m_rExport.GetFilter().addRelation(pFS->getOutputStream(),
                                          "http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramData",
                                          dataFileName, false), RTL_TEXTENCODING_UTF8);

    // add layout relation
    OUString layoutFileName = "diagrams/layout" + OUString::number(diagramCount) + ".xml";
    OString layoutRelId = OUStringToOString(m_pImpl->m_rExport.GetFilter().addRelation(pFS->getOutputStream(),
                                            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramLayout",
                                            layoutFileName, false), RTL_TEXTENCODING_UTF8);

    // add style relation
    OUString styleFileName = "diagrams/quickStyle" + OUString::number(diagramCount) + ".xml";
    OString styleRelId = OUStringToOString(m_pImpl->m_rExport.GetFilter().addRelation(pFS->getOutputStream(),
                                           "http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramQuickStyle",
                                           styleFileName , false), RTL_TEXTENCODING_UTF8);

    // add color relation
    OUString colorFileName = "diagrams/colors" + OUString::number(diagramCount) + ".xml";
    OString colorRelId = OUStringToOString(m_pImpl->m_rExport.GetFilter().addRelation(pFS->getOutputStream(),
                                           "http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramColors",
                                           colorFileName, false), RTL_TEXTENCODING_UTF8);

    OUString drawingFileName;
    if (drawingDom.is())
    {
        // add drawing relation
        drawingFileName = "diagrams/drawing" + OUString::number(diagramCount) + ".xml";
        OUString drawingRelId = m_pImpl->m_rExport.GetFilter().addRelation(pFS->getOutputStream(),
                                "http://schemas.microsoft.com/office/2007/relationships/diagramDrawing",
                                drawingFileName , false);

        // the data dom contains a reference to the drawing relation. We need to update it with the new generated
        // relation value before writing the dom to a file

        // Get the dsp:damaModelExt node from the dom
        uno::Reference< xml::dom::XNodeList > nodeList =
            dataDom->getElementsByTagNameNS("http://schemas.microsoft.com/office/drawing/2008/diagram", "dataModelExt");

        // There must be one element only so get it
        uno::Reference< xml::dom::XNode > node = nodeList->item(0);

        // Get the list of attributes of the node
        uno::Reference< xml::dom::XNamedNodeMap > nodeMap = node->getAttributes();

        // Get the node with the relId attribute and set its new value
        uno::Reference< xml::dom::XNode > relIdNode = nodeMap->getNamedItem("relId");
        relIdNode->setNodeValue(drawingRelId);
    }

    pFS->singleElementNS(XML_dgm, XML_relIds,
                         FSNS(XML_xmlns, XML_dgm), "http://schemas.openxmlformats.org/drawingml/2006/diagram",
                         FSNS(XML_xmlns, XML_r), "http://schemas.openxmlformats.org/officeDocument/2006/relationships",
                         FSNS(XML_r, XML_dm), dataRelId.getStr(),
                         FSNS(XML_r, XML_lo), layoutRelId.getStr(),
                         FSNS(XML_r, XML_qs), styleRelId.getStr(),
                         FSNS(XML_r, XML_cs), colorRelId.getStr(),
                         FSEND);

    pFS->endElementNS(XML_a, XML_graphicData);
    pFS->endElementNS(XML_a, XML_graphic);
    pFS->endElementNS(XML_wp, XML_inline);
    pFS->endElementNS(XML_w, XML_drawing);

    uno::Reference< xml::sax::XSAXSerializable > serializer;
    uno::Reference< xml::sax::XWriter > writer = xml::sax::Writer::create(comphelper::getProcessComponentContext());

    // write data file
    serializer.set(dataDom, uno::UNO_QUERY);
    writer->setOutputStream(m_pImpl->m_rExport.GetFilter().openFragmentStream("word/" + dataFileName,
                            "application/vnd.openxmlformats-officedocument.drawingml.diagramData+xml"));
    serializer->serialize(uno::Reference< xml::sax::XDocumentHandler >(writer, uno::UNO_QUERY_THROW),
                          uno::Sequence< beans::StringPair >());

    // write layout file
    serializer.set(layoutDom, uno::UNO_QUERY);
    writer->setOutputStream(m_pImpl->m_rExport.GetFilter().openFragmentStream("word/" + layoutFileName,
                            "application/vnd.openxmlformats-officedocument.drawingml.diagramLayout+xml"));
    serializer->serialize(uno::Reference< xml::sax::XDocumentHandler >(writer, uno::UNO_QUERY_THROW),
                          uno::Sequence< beans::StringPair >());

    // write style file
    serializer.set(styleDom, uno::UNO_QUERY);
    writer->setOutputStream(m_pImpl->m_rExport.GetFilter().openFragmentStream("word/" + styleFileName,
                            "application/vnd.openxmlformats-officedocument.drawingml.diagramStyle+xml"));
    serializer->serialize(uno::Reference< xml::sax::XDocumentHandler >(writer, uno::UNO_QUERY_THROW),
                          uno::Sequence< beans::StringPair >());

    // write color file
    serializer.set(colorDom, uno::UNO_QUERY);
    writer->setOutputStream(m_pImpl->m_rExport.GetFilter().openFragmentStream("word/" + colorFileName,
                            "application/vnd.openxmlformats-officedocument.drawingml.diagramColors+xml"));
    serializer->serialize(uno::Reference< xml::sax::XDocumentHandler >(writer, uno::UNO_QUERY_THROW),
                          uno::Sequence< beans::StringPair >());

    // write drawing file
    if (drawingDom.is())
    {
        serializer.set(drawingDom, uno::UNO_QUERY);
        writer->setOutputStream(m_pImpl->m_rExport.GetFilter().openFragmentStream("word/" + drawingFileName,
                                "application/vnd.openxmlformats-officedocument.drawingml.diagramDrawing+xml"));
        serializer->serialize(uno::Reference< xml::sax::XDocumentHandler >(writer, uno::UNO_QUERY_THROW),
                              uno::Sequence< beans::StringPair >());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
