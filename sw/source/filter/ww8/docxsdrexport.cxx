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
#include <editeng/charrotateitem.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdogrp.hxx>
#include <oox/token/tokens.hxx>
#include <oox/export/drawingml.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/export/utils.hxx>
#include <oox/export/vmlexport.hxx>
#include <oox/token/properties.hxx>

#include <frmatr.hxx>
#include <frmfmt.hxx>
#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <fmtcntnt.hxx>
#include <ndtxt.hxx>
#include <txatbase.hxx>
#include <fmtautofmt.hxx>
#include <fmtfsize.hxx>

#include <docxsdrexport.hxx>
#include <docxexport.hxx>
#include <docxexportfilter.hxx>
#include <writerhelper.hxx>
#include <comphelper/seqstream.hxx>

using namespace com::sun::star;
using namespace oox;

namespace
{

uno::Sequence<beans::PropertyValue> lclGetProperty(uno::Reference<drawing::XShape> rShape, const OUString& rPropName)
{
    uno::Sequence<beans::PropertyValue> aResult;
    uno::Reference<beans::XPropertySet> xPropertySet(rShape, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySetInfo> xPropSetInfo;

    if (!xPropertySet.is())
        return aResult;

    xPropSetInfo = xPropertySet->getPropertySetInfo();
    if (xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(rPropName))
    {
        xPropertySet->getPropertyValue(rPropName) >>= aResult;
    }
    return aResult;
}

OUString lclGetAnchorIdFromGrabBag(const SdrObject* pObj)
{
    OUString aResult;
    uno::Reference<drawing::XShape> xShape(const_cast<SdrObject*>(pObj)->getUnoShape(), uno::UNO_QUERY);
    uno::Sequence< beans::PropertyValue > propList =
        lclGetProperty(xShape, "FrameInteropGrabBag");
    for (sal_Int32 nProp = 0; nProp < propList.getLength(); ++nProp)
    {
        OUString aPropName = propList[nProp].Name;
        if (aPropName == "AnchorId")
        {
            propList[nProp].Value >>= aResult;
            break;
        }
    }
    return aResult;
}

void lclMovePositionWithRotation(awt::Point& aPos, sal_Int64 nRotation)
{
    // code from ImplEESdrWriter::ImplFlipBoundingBox (filter/source/msfilter/eschesdo.cxx)
    // TODO: refactor

    if (nRotation == 0)
        return;

    if (nRotation < 0)
        nRotation = (36000 + nRotation) % 36000;
    if (nRotation % 18000 == 0)
        nRotation = 0;
    while (nRotation > 9000)
        nRotation = (18000 - (nRotation % 18000));

    double fVal = (double) nRotation * F_PI18000;
    double  fCos = cos(fVal);
    double  fSin = sin(fVal);

    double  nWidthHalf = (double) aPos.X / 2;
    double  nHeightHalf = (double) aPos.Y / 2;

    double nXDiff = fSin * nHeightHalf + fCos * nWidthHalf  - nWidthHalf;
    double nYDiff = fSin * nWidthHalf  + fCos * nHeightHalf - nHeightHalf;

    aPos.X += nXDiff;
    aPos.Y += nYDiff;
}

}

ExportDataSaveRestore::ExportDataSaveRestore(DocxExport& rExport, sal_uLong nStt, sal_uLong nEnd, sw::Frame* pParentFrame)
    : m_rExport(rExport)
{
    m_rExport.SaveData(nStt, nEnd);
    m_rExport.mpParentFrame = pParentFrame;
}

ExportDataSaveRestore::~ExportDataSaveRestore()
{
    m_rExport.RestoreData();
}

/// Holds data used by DocxSdrExport only.
struct DocxSdrExport::Impl
{
    DocxSdrExport& m_rSdrExport;
    DocxExport& m_rExport;
    sax_fastparser::FSHelperPtr m_pSerializer;
    oox::drawingml::DrawingML* m_pDrawingML;
    const Size* m_pFlyFrameSize;
    bool m_bTextFrameSyntax;
    bool m_bDMLTextFrameSyntax;
    sax_fastparser::FastAttributeList* m_pFlyAttrList;
    sax_fastparser::FastAttributeList* m_pTextboxAttrList;
    OStringBuffer m_aTextFrameStyle;
    bool m_bFrameBtLr;
    bool m_bFlyFrameGraphic;
    sax_fastparser::FastAttributeList* m_pFlyFillAttrList;
    sax_fastparser::FastAttributeList* m_pFlyWrapAttrList;
    sax_fastparser::FastAttributeList* m_pBodyPrAttrList;
    sax_fastparser::FastAttributeList* m_pDashLineStyleAttr;
    bool m_bIsInDMLTextFrame;

    Impl(DocxSdrExport& rSdrExport, DocxExport& rExport, sax_fastparser::FSHelperPtr pSerializer, oox::drawingml::DrawingML* pDrawingML)
        : m_rSdrExport(rSdrExport),
          m_rExport(rExport),
          m_pSerializer(pSerializer),
          m_pDrawingML(pDrawingML),
          m_pFlyFrameSize(0),
          m_bTextFrameSyntax(false),
          m_bDMLTextFrameSyntax(false),
          m_pFlyAttrList(0),
          m_pTextboxAttrList(0),
          m_bFrameBtLr(false),
          m_bFlyFrameGraphic(false),
          m_pFlyFillAttrList(0),
          m_pFlyWrapAttrList(0),
          m_pBodyPrAttrList(0),
          m_pDashLineStyleAttr(0),
          m_bIsInDMLTextFrame(false)
    {
    }

    ~Impl()
    {
        delete m_pFlyAttrList, m_pFlyAttrList = NULL;
        delete m_pTextboxAttrList, m_pTextboxAttrList = NULL;
    }

    /// Writes wp wrapper code around an SdrObject, which itself is written using drawingML syntax.

    void textFrameShadow(const SwFrmFmt& rFrmFmt);
    bool isSupportedDMLShape(com::sun::star::uno::Reference<com::sun::star::drawing::XShape> xShape);
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

const Size* DocxSdrExport::getFlyFrameSize()
{
    return m_pImpl->m_pFlyFrameSize;
}

bool DocxSdrExport::getTextFrameSyntax()
{
    return m_pImpl->m_bTextFrameSyntax;
}

bool DocxSdrExport::getDMLTextFrameSyntax()
{
    return m_pImpl->m_bDMLTextFrameSyntax;
}

sax_fastparser::FastAttributeList*& DocxSdrExport::getFlyAttrList()
{
    return m_pImpl->m_pFlyAttrList;
}

void DocxSdrExport::setFlyAttrList(sax_fastparser::FastAttributeList* pAttrList)
{
    m_pImpl->m_pFlyAttrList = pAttrList;
}

sax_fastparser::FastAttributeList* DocxSdrExport::getTextboxAttrList()
{
    return m_pImpl->m_pTextboxAttrList;
}

OStringBuffer& DocxSdrExport::getTextFrameStyle()
{
    return m_pImpl->m_aTextFrameStyle;
}

bool DocxSdrExport::getFrameBtLr()
{
    return m_pImpl->m_bFrameBtLr;
}

void DocxSdrExport::setFrameBtLr(bool bFrameBtLr)
{
    m_pImpl->m_bFrameBtLr = bFrameBtLr;
}

bool DocxSdrExport::getFlyFrameGraphic()
{
    return m_pImpl->m_bFlyFrameGraphic;
}

void DocxSdrExport::setFlyFrameGraphic(bool bFlyFrameGraphic)
{
    m_pImpl->m_bFlyFrameGraphic = bFlyFrameGraphic;
}

sax_fastparser::FastAttributeList*& DocxSdrExport::getFlyFillAttrList()
{
    return m_pImpl->m_pFlyFillAttrList;
}

sax_fastparser::FastAttributeList* DocxSdrExport::getFlyWrapAttrList()
{
    return m_pImpl->m_pFlyWrapAttrList;
}

sax_fastparser::FastAttributeList* DocxSdrExport::getBodyPrAttrList()
{
    return m_pImpl->m_pBodyPrAttrList;
}

sax_fastparser::FastAttributeList*& DocxSdrExport::getDashLineStyle()
{
    return m_pImpl->m_pDashLineStyleAttr;
}

void DocxSdrExport::setFlyWrapAttrList(sax_fastparser::FastAttributeList* pAttrList)
{
    m_pImpl->m_pFlyWrapAttrList = pAttrList;
}

void DocxSdrExport::startDMLAnchorInline(const SwFrmFmt* pFrmFmt, const Size& rSize)
{
    m_pImpl->m_pSerializer->startElementNS(XML_w, XML_drawing, FSEND);

    const SvxLRSpaceItem pLRSpaceItem = pFrmFmt->GetLRSpace(false);
    const SvxULSpaceItem pULSpaceItem = pFrmFmt->GetULSpace(false);

    bool isAnchor;

    if (m_pImpl->m_bFlyFrameGraphic)
    {
        isAnchor = false; // make Graphic object inside DMLTextFrame & VMLTextFrame as Inline
    }
    else
    {
        isAnchor = pFrmFmt->GetAnchor().GetAnchorId() != FLY_AS_CHAR;
    }
    if (isAnchor)
    {
        sax_fastparser::FastAttributeList* attrList = m_pImpl->m_pSerializer->createAttrList();
        bool bOpaque = pFrmFmt->GetOpaque().GetValue();
        awt::Point aPos(pFrmFmt->GetHoriOrient().GetPos(), pFrmFmt->GetVertOrient().GetPos());
        const SdrObject* pObj = pFrmFmt->FindRealSdrObject();
        if (pObj != NULL)
        {
            // SdrObjects know their layer, consider that instead of the frame format.
            bOpaque = pObj->GetLayer() != pFrmFmt->GetDoc()->GetHellId() && pObj->GetLayer() != pFrmFmt->GetDoc()->GetInvisibleHellId();

            lclMovePositionWithRotation(aPos, pObj->GetRotateAngle());
        }
        attrList->add(XML_behindDoc, bOpaque ? "0" : "1");
        attrList->add(XML_distT, OString::number(TwipsToEMU(pULSpaceItem.GetUpper())).getStr());
        attrList->add(XML_distB, OString::number(TwipsToEMU(pULSpaceItem.GetLower())).getStr());
        attrList->add(XML_distL, OString::number(TwipsToEMU(pLRSpaceItem.GetLeft())).getStr());
        attrList->add(XML_distR, OString::number(TwipsToEMU(pLRSpaceItem.GetRight())).getStr());
        attrList->add(XML_simplePos, "0");
        attrList->add(XML_locked, "0");
        attrList->add(XML_layoutInCell, "1");
        attrList->add(XML_allowOverlap, "1");   // TODO
        if (pObj != NULL)
            // It seems 0 and 1 have special meaning: just start counting from 2 to avoid issues with that.
            attrList->add(XML_relativeHeight, OString::number(pObj->GetOrdNum() + 2));
        else
            // relativeHeight is mandatory attribute, if value is not present, we must write default value
            attrList->add(XML_relativeHeight, "0");
        if (pObj != NULL)
        {
            OUString sAnchorId = lclGetAnchorIdFromGrabBag(pObj);
            if (!sAnchorId.isEmpty())
                attrList->addNS(XML_wp14, XML_anchorId, OUStringToOString(sAnchorId, RTL_TEXTENCODING_UTF8));
        }
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
        case text::RelOrientation::PAGE_RIGHT:
            relativeFromH = "page";
            alignH = "right";
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
            m_pImpl->m_pSerializer->write(TwipsToEMU(aPos.X));
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
            m_pImpl->m_pSerializer->write(TwipsToEMU(aPos.Y));
            m_pImpl->m_pSerializer->endElementNS(XML_wp, XML_posOffset);
        }
        m_pImpl->m_pSerializer->endElementNS(XML_wp, XML_positionV);
    }
    else
    {
        sax_fastparser::FastAttributeList* aAttrList = m_pImpl->m_pSerializer->createAttrList();
        aAttrList->add(XML_distT, OString::number(TwipsToEMU(pULSpaceItem.GetUpper())).getStr());
        aAttrList->add(XML_distB, OString::number(TwipsToEMU(pULSpaceItem.GetLower())).getStr());
        aAttrList->add(XML_distL, OString::number(TwipsToEMU(pLRSpaceItem.GetLeft())).getStr());
        aAttrList->add(XML_distR, OString::number(TwipsToEMU(pLRSpaceItem.GetRight())).getStr());
        const SdrObject* pObj = pFrmFmt->FindRealSdrObject();
        if (pObj != NULL)
        {
            OUString sAnchorId = lclGetAnchorIdFromGrabBag(pObj);
            if (!sAnchorId.isEmpty())
                aAttrList->addNS(XML_wp14, XML_anchorId, OUStringToOString(sAnchorId, RTL_TEXTENCODING_UTF8));
        }
        m_pImpl->m_pSerializer->startElementNS(XML_wp, XML_inline, aAttrList);
    }

    // now the common parts
    // extent of the image
    OString aWidth(OString::number(TwipsToEMU(rSize.Width())));
    OString aHeight(OString::number(TwipsToEMU(rSize.Height())));
    m_pImpl->m_pSerializer->singleElementNS(XML_wp, XML_extent,
                                            XML_cx, (rSize.Width() > 0 ? aWidth.getStr() : "0"),
                                            XML_cy, (rSize.Height() > 0 ? aHeight.getStr() : "0"),
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
    bool isAnchor;
    if (m_pImpl->m_bFlyFrameGraphic)
    {
        isAnchor = false; // end Inline Graphic object inside DMLTextFrame
    }
    else
    {
        isAnchor = pFrmFmt->GetAnchor().GetAnchorId() != FLY_AS_CHAR;
    }
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

void DocxSdrExport::writeDMLDrawing(const SdrObject* pSdrObject, const SwFrmFmt* pFrmFmt, int nAnchorId)
{
    uno::Reference<drawing::XShape> xShape(const_cast<SdrObject*>(pSdrObject)->getUnoShape(), uno::UNO_QUERY_THROW);
    if (!m_pImpl->isSupportedDMLShape(xShape))
        return;

    sax_fastparser::FSHelperPtr pFS = m_pImpl->m_pSerializer;
    Size aSize(pSdrObject->GetLogicRect().GetWidth(), pSdrObject->GetLogicRect().GetHeight());
    startDMLAnchorInline(pFrmFmt, aSize);

    sax_fastparser::FastAttributeList* pDocPrAttrList = pFS->createAttrList();
    pDocPrAttrList->add(XML_id, OString::number(nAnchorId).getStr());
    pDocPrAttrList->add(XML_name, OUStringToOString(pSdrObject->GetName(), RTL_TEXTENCODING_UTF8));
    if (!pSdrObject->GetTitle().isEmpty())
        pDocPrAttrList->add(XML_title, OUStringToOString(pSdrObject->GetTitle(), RTL_TEXTENCODING_UTF8));
    if (!pSdrObject->GetDescription().isEmpty())
        pDocPrAttrList->add(XML_descr, OUStringToOString(pSdrObject->GetDescription(), RTL_TEXTENCODING_UTF8));
    sax_fastparser::XFastAttributeListRef xDocPrAttrListRef(pDocPrAttrList);
    pFS->singleElementNS(XML_wp, XML_docPr, xDocPrAttrListRef);

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

    bool bLockedCanvas = false;
    uno::Sequence< beans::PropertyValue > propList =
        lclGetProperty(xShape, "InteropGrabBag");
    for (sal_Int32 nProp=0; nProp < propList.getLength(); ++nProp)
    {
        OUString propName = propList[nProp].Name;
        if (propName == "LockedCanvas")
        {
            /*
             * Export as Locked Canvas only if the drawing
             * was originally a Locked Canvas and is now inside a Text Frame.
             */
            bLockedCanvas = m_pImpl->m_bIsInDMLTextFrame;
            break;
        }
    }
    if (bLockedCanvas)
        pFS->startElementNS(XML_lc, XML_lockedCanvas,
                            FSNS(XML_xmlns, XML_lc), "http://schemas.openxmlformats.org/drawingml/2006/lockedCanvas",
                            FSEND);

    m_pImpl->m_rExport.OutputDML(xShape);

    if (bLockedCanvas)
        pFS->endElementNS(XML_lc, XML_lockedCanvas);
    pFS->endElementNS(XML_a, XML_graphicData);
    pFS->endElementNS(XML_a, XML_graphic);

    // Relative size of the drawing.
    if (pSdrObject->GetRelativeWidth())
    {
        // At the moment drawinglayer objects are always relative from page.
        pFS->startElementNS(XML_wp14, XML_sizeRelH,
                            XML_relativeFrom, (pSdrObject->GetRelativeWidthRelation() == text::RelOrientation::FRAME ? "margin" : "page"),
                            FSEND);
        pFS->startElementNS(XML_wp14, XML_pctWidth, FSEND);
        pFS->writeEscaped(OUString::number(*pSdrObject->GetRelativeWidth() * 100 * oox::drawingml::PER_PERCENT));
        pFS->endElementNS(XML_wp14, XML_pctWidth);
        pFS->endElementNS(XML_wp14, XML_sizeRelH);
    }
    if (pSdrObject->GetRelativeHeight())
    {
        pFS->startElementNS(XML_wp14, XML_sizeRelV,
                            XML_relativeFrom, (pSdrObject->GetRelativeHeightRelation() == text::RelOrientation::FRAME ? "margin" : "page"),
                            FSEND);
        pFS->startElementNS(XML_wp14, XML_pctHeight, FSEND);
        pFS->writeEscaped(OUString::number(*pSdrObject->GetRelativeHeight() * 100 * oox::drawingml::PER_PERCENT));
        pFS->endElementNS(XML_wp14, XML_pctHeight);
        pFS->endElementNS(XML_wp14, XML_sizeRelV);
    }

    endDMLAnchorInline(pFrmFmt);
}

void DocxSdrExport::Impl::textFrameShadow(const SwFrmFmt& rFrmFmt)
{
    SvxShadowItem aShadowItem = rFrmFmt.GetShadow();
    if (aShadowItem.GetLocation() == SVX_SHADOW_NONE)
        return;

    OString aShadowWidth(OString::number(double(aShadowItem.GetWidth()) / 20) + "pt");
    OString aOffset;
    switch (aShadowItem.GetLocation())
    {
    case SVX_SHADOW_TOPLEFT:
        aOffset = "-" + aShadowWidth + ",-" + aShadowWidth;
        break;
    case SVX_SHADOW_TOPRIGHT:
        aOffset = aShadowWidth + ",-" + aShadowWidth;
        break;
    case SVX_SHADOW_BOTTOMLEFT:
        aOffset = "-" + aShadowWidth + "," + aShadowWidth;
        break;
    case SVX_SHADOW_BOTTOMRIGHT:
        aOffset = aShadowWidth + "," + aShadowWidth;
        break;
    case SVX_SHADOW_NONE:
    case SVX_SHADOW_END:
        break;
    }
    if (aOffset.isEmpty())
        return;

    OString aShadowColor = msfilter::util::ConvertColor(aShadowItem.GetColor());
    m_pSerializer->singleElementNS(XML_v, XML_shadow,
                                   XML_on, "t",
                                   XML_color, "#" + aShadowColor,
                                   XML_offset, aOffset,
                                   FSEND);
}

bool DocxSdrExport::Impl::isSupportedDMLShape(uno::Reference<drawing::XShape> xShape)
{
    bool supported = true;

    uno::Reference<lang::XServiceInfo> xServiceInfo(xShape, uno::UNO_QUERY_THROW);
    if (xServiceInfo->supportsService("com.sun.star.drawing.PolyPolygonShape") || xServiceInfo->supportsService("com.sun.star.drawing.PolyLineShape"))
        supported = false;

    return supported;
}

void DocxSdrExport::writeDMLAndVMLDrawing(const SdrObject* sdrObj, const SwFrmFmt& rFrmFmt,const Point& rNdTopLeft, int nAnchorId)
{
    // Depending on the shape type, we actually don't write the shape as DML.
    OUString sShapeType;
    sal_uInt32 nMirrorFlags = 0;
    uno::Reference<drawing::XShape> xShape(const_cast<SdrObject*>(sdrObj)->getUnoShape(), uno::UNO_QUERY_THROW);
    MSO_SPT eShapeType = EscherPropertyContainer::GetCustomShapeType(xShape, nMirrorFlags, sShapeType);

    if (eShapeType != ESCHER_ShpInst_TextPlainText && m_pImpl->isSupportedDMLShape(xShape))
    {
        m_pImpl->m_pSerializer->startElementNS(XML_mc, XML_AlternateContent, FSEND);

        const SdrObjGroup* pObjGroup = PTR_CAST(SdrObjGroup, sdrObj);
        m_pImpl->m_pSerializer->startElementNS(XML_mc, XML_Choice,
                                               XML_Requires, (pObjGroup ? "wpg" : "wps"),
                                               FSEND);
        writeDMLDrawing(sdrObj, &rFrmFmt, nAnchorId);
        m_pImpl->m_pSerializer->endElementNS(XML_mc, XML_Choice);

        m_pImpl->m_pSerializer->startElementNS(XML_mc, XML_Fallback, FSEND);
        writeVMLDrawing(sdrObj, rFrmFmt, rNdTopLeft);
        m_pImpl->m_pSerializer->endElementNS(XML_mc, XML_Fallback);

        m_pImpl->m_pSerializer->endElementNS(XML_mc, XML_AlternateContent);
    }
    else
        writeVMLDrawing(sdrObj, rFrmFmt, rNdTopLeft);
}

// Converts ARGB transparency (0..255) to drawingml alpha (opposite, and 0..100000)
OString lcl_ConvertTransparency(const Color& rColor)
{
    if (rColor.GetTransparency() > 0)
    {
        sal_Int32 nTransparencyPercent = 100 - float(rColor.GetTransparency()) / 2.55;
        return OString::number(nTransparencyPercent * oox::drawingml::PER_PERCENT);
    }
    else
        return OString("");
}

void DocxSdrExport::writeDMLEffectLst(const SwFrmFmt& rFrmFmt)
{
    SvxShadowItem aShadowItem = rFrmFmt.GetShadow();

    // Output effects
    if (aShadowItem.GetLocation() != SVX_SHADOW_NONE)
    {
        // Distance is measured diagonally from corner
        double nShadowDist = sqrt((double)aShadowItem.GetWidth()*aShadowItem.GetWidth()*2.0);
        OString aShadowDist(OString::number(TwipsToEMU(nShadowDist)));
        OString aShadowColor = msfilter::util::ConvertColor(aShadowItem.GetColor());
        OString aShadowAlpha = lcl_ConvertTransparency(aShadowItem.GetColor());
        sal_uInt32 nShadowDir = 0;
        switch (aShadowItem.GetLocation())
        {
        case SVX_SHADOW_TOPLEFT:
            nShadowDir = 13500000;
            break;
        case SVX_SHADOW_TOPRIGHT:
            nShadowDir = 18900000;
            break;
        case SVX_SHADOW_BOTTOMLEFT:
            nShadowDir = 8100000;
            break;
        case SVX_SHADOW_BOTTOMRIGHT:
            nShadowDir = 2700000;
            break;
        case SVX_SHADOW_NONE:
        case SVX_SHADOW_END:
            break;
        }
        OString aShadowDir(OString::number(nShadowDir));

        m_pImpl->m_pSerializer->startElementNS(XML_a, XML_effectLst, FSEND);
        m_pImpl->m_pSerializer->startElementNS(XML_a, XML_outerShdw,
                                               XML_dist, aShadowDist.getStr(),
                                               XML_dir, aShadowDir.getStr(), FSEND);
        if (aShadowAlpha.isEmpty())
            m_pImpl->m_pSerializer->singleElementNS(XML_a, XML_srgbClr,
                                                    XML_val, aShadowColor.getStr(), FSEND);
        else
        {
            m_pImpl->m_pSerializer->startElementNS(XML_a, XML_srgbClr, XML_val, aShadowColor.getStr(), FSEND);
            m_pImpl->m_pSerializer->singleElementNS(XML_a, XML_alpha, XML_val, aShadowAlpha.getStr(), FSEND);
            m_pImpl->m_pSerializer->endElementNS(XML_a, XML_srgbClr);
        }
        m_pImpl->m_pSerializer->endElementNS(XML_a, XML_outerShdw);
        m_pImpl->m_pSerializer->endElementNS(XML_a, XML_effectLst);
    }

}

void DocxSdrExport::writeDiagramRels(uno::Reference<xml::dom::XDocument> xDom,
                                     uno::Sequence< uno::Sequence< uno::Any > > xRelSeq,
                                     uno::Reference< io::XOutputStream > xOutStream, const OUString& sGrabBagProperyName)
{
    // add image relationships of OOXData, OOXDiagram
    OUString sType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/image");
    uno::Reference< xml::sax::XSAXSerializable > xSerializer(xDom, uno::UNO_QUERY);
    uno::Reference< xml::sax::XWriter > xWriter = xml::sax::Writer::create(comphelper::getProcessComponentContext());
    xWriter->setOutputStream(xOutStream);

    // retrieve the relationships from Sequence
    for (sal_Int32 j = 0; j < xRelSeq.getLength(); j++)
    {
        // diagramDataRelTuple[0] => RID,
        // diagramDataRelTuple[1] => xInputStream
        // diagramDataRelTuple[2] => extension
        uno::Sequence< uno::Any > diagramDataRelTuple = xRelSeq[j];

        OUString sRelId, sExtension;
        diagramDataRelTuple[0] >>= sRelId;
        diagramDataRelTuple[2] >>= sExtension;
        OUString sContentType;
        if (sExtension == ".jpeg")
            sContentType = "image/jpeg";
        else if (sExtension == ".WMF")
            sContentType = "image/x-wmf";
        sRelId = sRelId.copy(3);

        StreamDataSequence dataSeq;
        diagramDataRelTuple[1] >>= dataSeq;
        uno::Reference<io::XInputStream> dataImagebin(new ::comphelper::SequenceInputStream(dataSeq));

        OUString sFragment("../media/");
        sFragment += sGrabBagProperyName + OUString::number(j) + sExtension;

        PropertySet aProps(xOutStream);
        aProps.setAnyProperty(PROP_RelId, uno::makeAny(sal_Int32(sRelId.toInt32())));

        m_pImpl->m_rExport.GetFilter().addRelation(xOutStream, sType, sFragment);

        sFragment = sFragment.replaceFirst("..","word");
        uno::Reference< io::XOutputStream > xBinOutStream = m_pImpl->m_rExport.GetFilter().openFragmentStream(sFragment, sContentType);

        try
        {
            sal_Int32 nBufferSize = 512;
            uno::Sequence< sal_Int8 > aDataBuffer(nBufferSize);
            sal_Int32 nRead;
            do
            {
                nRead = dataImagebin->readBytes(aDataBuffer, nBufferSize);
                if (nRead)
                {
                    if (nRead < nBufferSize)
                    {
                        nBufferSize = nRead;
                        aDataBuffer.realloc(nRead);
                    }
                    xBinOutStream->writeBytes(aDataBuffer);
                }
            }
            while (nRead);
            xBinOutStream->flush();
        }
        catch (const uno::Exception& rException)
        {
            SAL_WARN("sw.ww8", "DocxSdrExport::writeDiagramRels Failed to copy grabbaged Image: " << rException.Message);
        }
        dataImagebin->closeInput();
    }
}

void DocxSdrExport::writeDiagram(const SdrObject* sdrObject, const SwFrmFmt& rFrmFmt,  int nAnchorId)
{
    sax_fastparser::FSHelperPtr pFS = m_pImpl->m_pSerializer;
    uno::Reference< drawing::XShape > xShape(((SdrObject*)sdrObject)->getUnoShape(), uno::UNO_QUERY);
    uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);

    uno::Reference<xml::dom::XDocument> dataDom;
    uno::Reference<xml::dom::XDocument> layoutDom;
    uno::Reference<xml::dom::XDocument> styleDom;
    uno::Reference<xml::dom::XDocument> colorDom;
    uno::Reference<xml::dom::XDocument> drawingDom;
    uno::Sequence< uno::Sequence< uno::Any > > xDataRelSeq;
    uno::Sequence< uno::Any > diagramDrawing;

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
            propList[nProp].Value >>= diagramDrawing;
        else if (propName == "OOXDiagramDataRels")
            propList[nProp].Value >>= xDataRelSeq;
    }

    diagramDrawing[0] >>= drawingDom;
    // check that we have the 4 mandatory XDocuments
    // if not, there was an error importing and we won't output anything
    if (!dataDom.is() || !layoutDom.is() || !styleDom.is() || !colorDom.is())
        return;

    // write necessary tags to document.xml
    Size aSize(sdrObject->GetSnapRect().GetWidth(), sdrObject->GetSnapRect().GetHeight());
    startDMLAnchorInline(&rFrmFmt, aSize);

    // generate an unique id
    sax_fastparser::FastAttributeList* pDocPrAttrList = pFS->createAttrList();
    pDocPrAttrList->add(XML_id, OString::number(nAnchorId).getStr());
    OUString sName = "Diagram" + OUString::number(nAnchorId);
    pDocPrAttrList->add(XML_name, OUStringToOString(sName, RTL_TEXTENCODING_UTF8).getStr());
    sax_fastparser::XFastAttributeListRef xDocPrAttrListRef(pDocPrAttrList);
    pFS->singleElementNS(XML_wp, XML_docPr, xDocPrAttrListRef);

    sal_Int32 diagramCount;
    diagramCount = nAnchorId;

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
    endDMLAnchorInline(&rFrmFmt);

    uno::Reference< xml::sax::XSAXSerializable > serializer;
    uno::Reference< xml::sax::XWriter > writer = xml::sax::Writer::create(comphelper::getProcessComponentContext());

    // write data file
    serializer.set(dataDom, uno::UNO_QUERY);
    uno::Reference< io::XOutputStream > xDataOutputStream = m_pImpl->m_rExport.GetFilter().openFragmentStream(
                "word/" + dataFileName, "application/vnd.openxmlformats-officedocument.drawingml.diagramData+xml");
    writer->setOutputStream(xDataOutputStream);
    serializer->serialize(uno::Reference< xml::sax::XDocumentHandler >(writer, uno::UNO_QUERY_THROW),
                          uno::Sequence< beans::StringPair >());

    // write the associated Images and rels for data file
    writeDiagramRels(dataDom, xDataRelSeq, xDataOutputStream, OUString("OOXDiagramDataRels"));

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
        uno::Reference< io::XOutputStream > xDrawingOutputStream = m_pImpl->m_rExport.GetFilter().openFragmentStream("word/" + drawingFileName,
                "application/vnd.openxmlformats-officedocument.drawingml.diagramDrawing+xml");
        writer->setOutputStream(xDrawingOutputStream);
        serializer->serialize(uno::Reference< xml::sax::XDocumentHandler >(writer, uno::UNO_QUERY_THROW),
                              uno::Sequence< beans::StringPair >());

        // write the associated Images and rels for drawing file
        uno::Sequence< uno::Sequence< uno::Any > > xDrawingRelSeq;
        diagramDrawing[1] >>= xDrawingRelSeq;
        writeDiagramRels(drawingDom, xDrawingRelSeq, xDrawingOutputStream, OUString("OOXDiagramDrawingRels"));
    }
}

void DocxSdrExport::writeDMLTextFrame(sw::Frame* pParentFrame, int nAnchorId)
{
    m_pImpl->m_bIsInDMLTextFrame = true;
    sax_fastparser::FSHelperPtr pFS = m_pImpl->m_pSerializer;
    const SwFrmFmt& rFrmFmt = pParentFrame->GetFrmFmt();
    const SwNodeIndex* pNodeIndex = rFrmFmt.GetCntnt().GetCntntIdx();

    sal_uLong nStt = pNodeIndex ? pNodeIndex->GetIndex()+1                  : 0;
    sal_uLong nEnd = pNodeIndex ? pNodeIndex->GetNode().EndOfSectionIndex() : 0;

    //Save data here and restore when out of scope
    ExportDataSaveRestore aDataGuard(m_pImpl->m_rExport, nStt, nEnd, pParentFrame);

    // When a frame has some low height, but automatically expanded due
    // to lots of contents, this size contains the real size.
    const Size aSize = pParentFrame->GetSize();

    startDMLAnchorInline(&rFrmFmt, aSize);

    sax_fastparser::FastAttributeList* pDocPrAttrList = pFS->createAttrList();
    pDocPrAttrList->add(XML_id, OString::number(nAnchorId).getStr());
    pDocPrAttrList->add(XML_name, OUStringToOString(rFrmFmt.GetName(), RTL_TEXTENCODING_UTF8).getStr());
    sax_fastparser::XFastAttributeListRef xDocPrAttrListRef(pDocPrAttrList);
    pFS->singleElementNS(XML_wp, XML_docPr, xDocPrAttrListRef);

    pFS->startElementNS(XML_a, XML_graphic,
                        FSNS(XML_xmlns, XML_a), "http://schemas.openxmlformats.org/drawingml/2006/main",
                        FSEND);
    pFS->startElementNS(XML_a, XML_graphicData,
                        XML_uri, "http://schemas.microsoft.com/office/word/2010/wordprocessingShape",
                        FSEND);
    pFS->startElementNS(XML_wps, XML_wsp, FSEND);
    pFS->singleElementNS(XML_wps, XML_cNvSpPr,
                         XML_txBox, "1",
                         FSEND);

    uno::Any aRotation ;
    uno::Reference< drawing::XShape > xShape;
    const SdrObject* pSdrObj = rFrmFmt.FindRealSdrObject();
    if (pSdrObj)
        xShape = uno::Reference< drawing::XShape >(const_cast<SdrObject*>(pSdrObj)->getUnoShape(), uno::UNO_QUERY);
    uno::Reference< beans::XPropertySet > xPropertySet(xShape, uno::UNO_QUERY);
    uno::Reference< beans::XPropertySetInfo > xPropSetInfo;
    if (xPropertySet.is())
        xPropSetInfo = xPropertySet->getPropertySetInfo();
    sal_Int32 nRotation = 0;
    if (xPropSetInfo.is() && xPropSetInfo->hasPropertyByName("FrameInteropGrabBag"))
    {
        uno::Sequence< beans::PropertyValue > propList;
        xPropertySet->getPropertyValue("FrameInteropGrabBag") >>= propList;
        for (sal_Int32 nProp=0; nProp < propList.getLength(); ++nProp)
        {
            OUString propName = propList[nProp].Name;
            if (propName == "mso-rotation-angle")
            {
                aRotation = propList[nProp].Value ;
                break;
            }
        }
    }
    aRotation >>= nRotation ;
    OString sRotation(OString::number(nRotation));
    // Shape properties
    pFS->startElementNS(XML_wps, XML_spPr, FSEND);
    if (nRotation)
    {
        pFS->startElementNS(XML_a, XML_xfrm,
                            XML_rot, sRotation.getStr(),
                            FSEND);
    }
    else
    {
        pFS->startElementNS(XML_a, XML_xfrm, FSEND);
    }
    pFS->singleElementNS(XML_a, XML_off,
                         XML_x, "0",
                         XML_y, "0",
                         FSEND);
    OString aWidth(OString::number(TwipsToEMU(aSize.Width())));
    OString aHeight(OString::number(TwipsToEMU(aSize.Height())));
    pFS->singleElementNS(XML_a, XML_ext,
                         XML_cx, aWidth.getStr(),
                         XML_cy, aHeight.getStr(),
                         FSEND);
    pFS->endElementNS(XML_a, XML_xfrm);
    OUString shapeType = "rect";
    if (xPropSetInfo.is() && xPropSetInfo->hasPropertyByName("FrameInteropGrabBag"))
    {
        uno::Sequence< beans::PropertyValue > propList;
        xPropertySet->getPropertyValue("FrameInteropGrabBag") >>= propList;
        for (sal_Int32 nProp=0; nProp < propList.getLength(); ++nProp)
        {
            OUString propName = propList[nProp].Name;
            if (propName == "mso-orig-shape-type")
            {
                propList[nProp].Value >>= shapeType;
                break;
            }
        }
    }

    pFS->singleElementNS(XML_a, XML_prstGeom,
                         XML_prst, OUStringToOString(shapeType, RTL_TEXTENCODING_UTF8).getStr(),
                         FSEND);
    m_pImpl->m_bDMLTextFrameSyntax = true;
    m_pImpl->m_pBodyPrAttrList = pFS->createAttrList();
    {
        drawing::TextVerticalAdjust eAdjust = drawing::TextVerticalAdjust_TOP;
        if (xPropSetInfo.is() && xPropSetInfo->hasPropertyByName("TextVerticalAdjust"))
            xPropertySet->getPropertyValue("TextVerticalAdjust") >>= eAdjust;
        m_pImpl->m_pBodyPrAttrList->add(XML_anchor, oox::drawingml::GetTextVerticalAdjust(eAdjust));
    }
    m_pImpl->m_rExport.OutputFormat(pParentFrame->GetFrmFmt(), false, false, true);
    m_pImpl->m_bDMLTextFrameSyntax = false;
    writeDMLEffectLst(rFrmFmt);
    pFS->endElementNS(XML_wps, XML_spPr);

    m_pImpl->m_rExport.mpParentFrame = NULL;
    pFS->startElementNS(XML_wps, XML_txbx, FSEND);
    pFS->startElementNS(XML_w, XML_txbxContent, FSEND);

    m_pImpl->m_bFrameBtLr = checkFrameBtlr(m_pImpl->m_rExport.pDoc->GetNodes()[nStt], 0);
    m_pImpl->m_bFlyFrameGraphic = true;
    m_pImpl->m_rExport.WriteText();
    m_pImpl->m_bFlyFrameGraphic = false;
    m_pImpl->m_bFrameBtLr = false;

    pFS->endElementNS(XML_w, XML_txbxContent);
    pFS->endElementNS(XML_wps, XML_txbx);
    sax_fastparser::XFastAttributeListRef xBodyPrAttrList(m_pImpl->m_pBodyPrAttrList);
    m_pImpl->m_pBodyPrAttrList = NULL;
    pFS->startElementNS(XML_wps, XML_bodyPr, xBodyPrAttrList);
    // AutoSize of the Text Frame.
    const SwFmtFrmSize& rSize = rFrmFmt.GetFrmSize();
    pFS->singleElementNS(XML_a, (rSize.GetHeightSizeType() == ATT_VAR_SIZE ? XML_spAutoFit : XML_noAutofit), FSEND);
    pFS->endElementNS(XML_wps, XML_bodyPr);

    pFS->endElementNS(XML_wps, XML_wsp);
    pFS->endElementNS(XML_a, XML_graphicData);
    pFS->endElementNS(XML_a, XML_graphic);

    // Relative size of the Text Frame.
    if (rSize.GetWidthPercent())
    {
        pFS->startElementNS(XML_wp14, XML_sizeRelH,
                            XML_relativeFrom, (rSize.GetWidthPercentRelation() == text::RelOrientation::PAGE_FRAME ? "page" : "margin"),
                            FSEND);
        pFS->startElementNS(XML_wp14, XML_pctWidth, FSEND);
        pFS->writeEscaped(OUString::number(rSize.GetWidthPercent() * oox::drawingml::PER_PERCENT));
        pFS->endElementNS(XML_wp14, XML_pctWidth);
        pFS->endElementNS(XML_wp14, XML_sizeRelH);
    }
    if (rSize.GetHeightPercent())
    {
        pFS->startElementNS(XML_wp14, XML_sizeRelV,
                            XML_relativeFrom, (rSize.GetHeightPercentRelation() == text::RelOrientation::PAGE_FRAME ? "page" : "margin"),
                            FSEND);
        pFS->startElementNS(XML_wp14, XML_pctHeight, FSEND);
        pFS->writeEscaped(OUString::number(rSize.GetHeightPercent() * oox::drawingml::PER_PERCENT));
        pFS->endElementNS(XML_wp14, XML_pctHeight);
        pFS->endElementNS(XML_wp14, XML_sizeRelV);
    }

    endDMLAnchorInline(&rFrmFmt);
    m_pImpl->m_bIsInDMLTextFrame = false;
}

void DocxSdrExport::writeVMLTextFrame(sw::Frame* pParentFrame)
{
    sax_fastparser::FSHelperPtr pFS = m_pImpl->m_pSerializer;
    const SwFrmFmt& rFrmFmt = pParentFrame->GetFrmFmt();
    const SwNodeIndex* pNodeIndex = rFrmFmt.GetCntnt().GetCntntIdx();

    sal_uLong nStt = pNodeIndex ? pNodeIndex->GetIndex()+1                  : 0;
    sal_uLong nEnd = pNodeIndex ? pNodeIndex->GetNode().EndOfSectionIndex() : 0;

    //Save data here and restore when out of scope
    ExportDataSaveRestore aDataGuard(m_pImpl->m_rExport, nStt, nEnd, pParentFrame);

    // When a frame has some low height, but automatically expanded due
    // to lots of contents, this size contains the real size.
    const Size aSize = pParentFrame->GetSize();
    m_pImpl->m_pFlyFrameSize = &aSize;

    m_pImpl->m_bTextFrameSyntax = true;
    m_pImpl->m_pFlyAttrList = pFS->createAttrList();
    m_pImpl->m_pTextboxAttrList = pFS->createAttrList();
    m_pImpl->m_aTextFrameStyle = "position:absolute";
    m_pImpl->m_rExport.OutputFormat(pParentFrame->GetFrmFmt(), false, false, true);
    m_pImpl->m_pFlyAttrList->add(XML_style, m_pImpl->m_aTextFrameStyle.makeStringAndClear());

    const SdrObject* pObject = pParentFrame->GetFrmFmt().FindRealSdrObject();
    if (pObject != NULL)
    {
        OUString sAnchorId = lclGetAnchorIdFromGrabBag(pObject);
        if (!sAnchorId.isEmpty())
            m_pImpl->m_pFlyAttrList->addNS(XML_w14, XML_anchorId, OUStringToOString(sAnchorId, RTL_TEXTENCODING_UTF8));
    }

    sax_fastparser::XFastAttributeListRef xFlyAttrList(m_pImpl->m_pFlyAttrList);
    m_pImpl->m_pFlyAttrList = NULL;
    m_pImpl->m_bFrameBtLr = checkFrameBtlr(m_pImpl->m_rExport.pDoc->GetNodes()[nStt], m_pImpl->m_pTextboxAttrList);
    sax_fastparser::XFastAttributeListRef xTextboxAttrList(m_pImpl->m_pTextboxAttrList);
    m_pImpl->m_pTextboxAttrList = NULL;
    m_pImpl->m_bTextFrameSyntax = false;
    m_pImpl->m_pFlyFrameSize = 0;
    m_pImpl->m_rExport.mpParentFrame = NULL;

    pFS->startElementNS(XML_w, XML_pict, FSEND);
    pFS->startElementNS(XML_v, XML_rect, xFlyAttrList);
    m_pImpl->textFrameShadow(rFrmFmt);
    if (m_pImpl->m_pFlyFillAttrList)
    {
        sax_fastparser::XFastAttributeListRef xFlyFillAttrList(m_pImpl->m_pFlyFillAttrList);
        m_pImpl->m_pFlyFillAttrList = NULL;
        pFS->singleElementNS(XML_v, XML_fill, xFlyFillAttrList);
    }
    if (m_pImpl->m_pDashLineStyleAttr)
    {
        sax_fastparser::XFastAttributeListRef xDashLineStyleAttr(m_pImpl->m_pDashLineStyleAttr);
        m_pImpl->m_pDashLineStyleAttr = NULL;
        pFS->singleElementNS(XML_v, XML_stroke, xDashLineStyleAttr);
    }
    pFS->startElementNS(XML_v, XML_textbox, xTextboxAttrList);
    pFS->startElementNS(XML_w, XML_txbxContent, FSEND);
    m_pImpl->m_bFlyFrameGraphic = true;
    m_pImpl->m_rExport.WriteText();
    m_pImpl->m_bFlyFrameGraphic = false;
    pFS->endElementNS(XML_w, XML_txbxContent);
    pFS->endElementNS(XML_v, XML_textbox);

    if (m_pImpl->m_pFlyWrapAttrList)
    {
        sax_fastparser::XFastAttributeListRef xFlyWrapAttrList(m_pImpl->m_pFlyWrapAttrList);
        m_pImpl->m_pFlyWrapAttrList = NULL;
        pFS->singleElementNS(XML_w10, XML_wrap, xFlyWrapAttrList);
    }

    pFS->endElementNS(XML_v, XML_rect);
    pFS->endElementNS(XML_w, XML_pict);
    m_pImpl->m_bFrameBtLr = false;
}

bool DocxSdrExport::checkFrameBtlr(SwNode* pStartNode, sax_fastparser::FastAttributeList* pTextboxAttrList)
{
    // The intended usage is to pass either a valid VML or DML attribute list.
    assert(pTextboxAttrList || m_pImpl->m_pBodyPrAttrList);

    if (!pStartNode->IsTxtNode())
        return false;

    SwTxtNode* pTxtNode = static_cast<SwTxtNode*>(pStartNode);

    const SfxPoolItem* pItem = 0; // explicitly init to avoid warnings
    bool bItemSet = false;
    if (pTxtNode->HasSwAttrSet())
    {
        const SwAttrSet& rAttrSet = pTxtNode->GetSwAttrSet();
        bItemSet = rAttrSet.GetItemState(RES_CHRATR_ROTATE, true, &pItem) == SFX_ITEM_SET;
    }

    if (!bItemSet)
    {
        if (!pTxtNode->HasHints())
            return false;

        SwTxtAttr* pTxtAttr = pTxtNode->GetTxtAttrAt(0, RES_TXTATR_AUTOFMT);

        if (!pTxtAttr || pTxtAttr->Which() != RES_TXTATR_AUTOFMT)
            return false;

        boost::shared_ptr<SfxItemSet> pItemSet = pTxtAttr->GetAutoFmt().GetStyleHandle();
        bItemSet = pItemSet->GetItemState(RES_CHRATR_ROTATE, true, &pItem) == SFX_ITEM_SET;
    }

    if (bItemSet)
    {
        const SvxCharRotateItem& rCharRotate = static_cast<const SvxCharRotateItem&>(*pItem);
        if (rCharRotate.GetValue() == 900)
        {
            if (pTextboxAttrList)
                pTextboxAttrList->add(XML_style, "mso-layout-flow-alt:bottom-to-top");
            else
                m_pImpl->m_pBodyPrAttrList->add(XML_vert, "vert270");
            return true;
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
