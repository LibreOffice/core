/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <stdio.h>
#include <oox/drawingml/clrscheme.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <oox/token/relationship.hxx>
#include <oox/ole/vbaproject.hxx>
#include "epptooxml.hxx"
#include <oox/export/shapes.hxx>

#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/xmltools.hxx>
#include <sax/fshelper.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <tools/UnitConversion.hxx>
#include <tools/datetime.hxx>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/office/XAnnotationEnumeration.hpp>
#include <com/sun/star/office/XAnnotationAccess.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <comphelper/diagnose_ex.hxx>

#include <oox/export/utils.hxx>
#include <oox/export/ThemeExport.hxx>
#include <docmodel/theme/Theme.hxx>

#include "pptx-animations.hxx"
#include "../ppt/pptanimations.hxx"

#include <i18nlangtag/languagetag.hxx>
#include <svx/sdrmasterpagedescriptor.hxx>
#include <svx/svdpage.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdogrp.hxx>
#include <svx/ColorSets.hxx>
#include <sdmod.hxx>
#include <sdpage.hxx>

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <utility>
#if OSL_DEBUG_LEVEL > 1
#include <com/sun/star/drawing/RectanglePoint.hpp>
#endif

// presentation namespaces
#define PNMSS         FSNS(XML_xmlns, XML_a),   this->getNamespaceURL(OOX_NS(dml)), \
                      FSNS(XML_xmlns, XML_p),   this->getNamespaceURL(OOX_NS(ppt)), \
                      FSNS(XML_xmlns, XML_r),   this->getNamespaceURL(OOX_NS(officeRel)), \
                      FSNS(XML_xmlns, XML_p14), this->getNamespaceURL(OOX_NS(p14)), \
                      FSNS(XML_xmlns, XML_p15), this->getNamespaceURL(OOX_NS(p15)), \
                      FSNS(XML_xmlns, XML_mc),  this->getNamespaceURL(OOX_NS(mce))

// presentationPr namespace
#define PPRNMSS       FSNS(XML_xmlns, XML_a),   this->getNamespaceURL(OOX_NS(dml)), \
                      FSNS(XML_xmlns, XML_r),   this->getNamespaceURL(OOX_NS(officeRel)), \
                      FSNS(XML_xmlns, XML_p),   this->getNamespaceURL(OOX_NS(ppt))

using namespace ::com::sun::star;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::geometry;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::office;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::ppt;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;
using ::sax_fastparser::FSHelperPtr;
using namespace oox::drawingml;
using namespace oox::core;

#if OSL_DEBUG_LEVEL > 1
void dump_pset(Reference< XPropertySet > const& rXPropSet);
#endif

namespace oox::core
{

class PowerPointShapeExport : public ShapeExport
{
    PowerPointExport&   mrExport;
    PageType            mePageType;
    bool                mbMaster;
public:
    PowerPointShapeExport(FSHelperPtr pFS, ShapeHashMap* pShapeMap, PowerPointExport* pFB);
    void                SetMaster(bool bMaster);
    void                SetPageType(PageType ePageType);
    ShapeExport&        WriteNonVisualProperties(const Reference< XShape >& xShape) override;
    ShapeExport&        WriteTextShape(const Reference< XShape >& xShape) override;
    ShapeExport&        WriteUnknownShape(const Reference< XShape >& xShape) override;
    ShapeExport&        WritePlaceholderShape(const Reference< XShape >& xShape, PlaceholderType ePlaceholder);
    /** Writes a placeholder shape that references the placeholder on the master slide */
    ShapeExport&        WritePlaceholderReferenceShape(PlaceholderType ePlaceholder, sal_Int32 nReferencedPlaceholderIdx, PageType ePageType, const Reference<XPropertySet>& rXPagePropSet);
    ShapeExport&        WritePageShape(const Reference< XShape >& xShape, PageType ePageType, bool bPresObj);
    /** Writes textbody of a placeholder that references the placeholder on the master slide */
    ShapeExport&        WritePlaceholderReferenceTextBody(PlaceholderType ePlaceholder, PageType ePageType, const Reference<XPropertySet> xPagePropSet);

    // helper parts
    bool WritePlaceholder(const Reference< XShape >& xShape, PlaceholderType ePlaceholder, bool bMaster);
};


namespace
{
void WriteSndAc(const FSHelperPtr& pFS, const OUString& sSoundRelId, const OUString& sSoundName)
{
        pFS->startElementNS(XML_p, XML_sndAc);
        pFS->startElementNS(XML_p, XML_stSnd);
        pFS->singleElementNS(XML_p, XML_snd, FSNS(XML_r, XML_embed),
                             sax_fastparser::UseIf(sSoundRelId, !sSoundRelId.isEmpty()), XML_name,
                             sax_fastparser::UseIf(sSoundName, !sSoundName.isEmpty()));
        pFS->endElement(FSNS(XML_p, XML_stSnd));
        pFS->endElement(FSNS(XML_p, XML_sndAc));
}

const char* getPlaceholderTypeName(PlaceholderType ePlaceholder)
{
    switch (ePlaceholder)
    {
        case SlideImage:
            return "sldImg";
        case Notes:
            return "body";
        case Header:
            return "hdr";
        case Footer:
            return "ftr";
        case SlideNumber:
            return "sldNum";
        case DateAndTime:
            return "dt";
        case Outliner:
            return "body";
        case Title:
            return "title";
        case Subtitle:
            return "subTitle";
        default:
            SAL_INFO("sd.eppt", "warning: unhandled placeholder type: " << ePlaceholder);
            return "";
    }
}
}
}

namespace {

struct PPTXLayoutInfo
{
    int nType;
    const char* sName;
    const char* sType;
};

}

const PPTXLayoutInfo aLayoutInfo[OOXML_LAYOUT_SIZE] =
{
    { 0, "Title Slide", "title" },
    { 1, "Title and text", "tx" },
    { 2, "Title and chart", "chart" },
    { 3, "Title, text on left, text on right", "twoObj" },
    { 4, "Title, text on left and chart on right", "txAndChart" },
    { 6, "Title, text on left, clip art on right", "txAndClipArt" },
    { 6, "Title, text on left, media on right", "txAndMedia" },
    { 7, "Title, chart on left and text on right", "chartAndTx" },
    { 8, "Title and table", "tbl" },
    { 9, "Title, clipart on left, text on right", "clipArtAndTx" },
    { 10, "Title, text on left, object on right", "txAndObj" },
    { 1, "Title and object", "obj" },
    { 12, "Title, text on left, two objects on right", "txAndTwoObj" },
    { 13, "Title, object on left, text on right", "objAndTx" },
    { 14, "Title, object on top, text on bottom", "objOverTx" },
    { 15, "Title, two objects on left, text on right", "twoObjAndTx" },
    { 16, "Title, two objects on top, text on bottom", "twoObjOverTx" },
    { 17, "Title, text on top, object on bottom", "txOverObj" },
    { 18, "Title and four objects", "fourObj" },
    { 19, "Title Only", "titleOnly" },
    { 20, "Blank Slide", "blank" },
    { 21, "Vertical title on right, vertical text on top, chart on bottom", "vertTitleAndTxOverChart" },
    { 22, "Vertical title on right, vertical text on left", "vertTitleAndTx" },
    { 23, "Title and vertical text body", "vertTx" },
    { 24, "Title, clip art on left, vertical text on right", "clipArtAndVertTx" },
    { 20, "Title, two objects each with text", "twoTxTwoObj" },
    { 15, "Title, two objects on left, one object on right", "twoObjAndObj" },
    { 20, "Title, object and caption text", "objTx" },
    { 20, "Title, picture, and caption text", "picTx" },
    { 20, "Section header title and subtitle text", "secHead" },
    { 32, "Object only", "objOnly" },
    { 12, "Title, one object on left, two objects on right", "objAndTwoObj" },
    { 20, "Title, media on left, text on right", "mediaAndTx" },
    { 34, "Title, 6 Content", "blank" }, // not defined in OOXML => blank
    { 2, "Title and diagram", "dgm" },
    { 0, "Custom layout defined by user", "cust" },
};

PowerPointShapeExport::PowerPointShapeExport(FSHelperPtr pFS, ShapeHashMap* pShapeMap,
        PowerPointExport* pFB)
    : ShapeExport(XML_p, std::move(pFS), pShapeMap, pFB)
    , mrExport(*pFB)
    , mePageType(UNDEFINED)
    , mbMaster(false)
{
}

void PowerPointShapeExport::SetMaster(bool bMaster)
{
    mbMaster = bMaster;
}

void PowerPointShapeExport::SetPageType(PageType ePageType)
{
    mePageType = ePageType;
}

ShapeExport& PowerPointShapeExport::WriteNonVisualProperties(const Reference< XShape >&)
{
    GetFS()->singleElementNS(XML_p, XML_nvPr);

    return *this;
}

ShapeExport& PowerPointShapeExport::WriteTextShape(const Reference< XShape >& xShape)
{
    OUString sShapeType = xShape->getShapeType();

    SAL_INFO("sd.eppt", "shape(text) : " << sShapeType.toUtf8());

    if (sShapeType == "com.sun.star.drawing.TextShape" || sShapeType == "com.sun.star.drawing.GraphicObjectShape")
    {
        ShapeExport::WriteTextShape(xShape);
    }
    else if (sShapeType == "com.sun.star.presentation.DateTimeShape")
    {
        if (!WritePlaceholder(xShape, DateAndTime, mbMaster))
            ShapeExport::WriteTextShape(xShape);
    }
    else if (sShapeType == "com.sun.star.presentation.FooterShape")
    {
        if (!WritePlaceholder(xShape, Footer, mbMaster))
            ShapeExport::WriteTextShape(xShape);
    }
    else if (sShapeType == "com.sun.star.presentation.HeaderShape")
    {
        if (!WritePlaceholder(xShape, Header, mbMaster))
            ShapeExport::WriteTextShape(xShape);
    }
    else if (sShapeType == "com.sun.star.presentation.NotesShape")
    {
        if (mePageType == NOTICE && mrExport.GetPresObj())
            WritePlaceholderShape(xShape, Notes);
        else
            ShapeExport::WriteTextShape(xShape);
    }
    else if (sShapeType == "com.sun.star.presentation.OutlinerShape")
    {
        if (!WritePlaceholder(xShape, Outliner, mbMaster))
            ShapeExport::WriteTextShape(xShape);
    }
    else if (sShapeType == "com.sun.star.presentation.SlideNumberShape")
    {
        if (!WritePlaceholder(xShape, SlideNumber, mbMaster))
            ShapeExport::WriteTextShape(xShape);
    }
    else if (sShapeType == "com.sun.star.presentation.TitleTextShape")
    {
        if (!WritePlaceholder(xShape, Title, mbMaster))
            ShapeExport::WriteTextShape(xShape);
    }
    else
        SAL_WARN("sd.eppt", "PowerPointShapeExport::WriteTextShape: shape of type '" << sShapeType << "' is ignored");

    return *this;
}

ShapeExport& PowerPointShapeExport::WriteUnknownShape(const Reference< XShape >& xShape)
{
    OUString sShapeType = xShape->getShapeType();

    SAL_INFO("sd.eppt", "shape(unknown): " << sShapeType.toUtf8());

    if (sShapeType == "com.sun.star.presentation.PageShape")
    {
        WritePageShape(xShape, mePageType, mrExport.GetPresObj());
    }
    else if (sShapeType == "com.sun.star.presentation.SubtitleShape")
    {
        if(mePageType != MASTER)
        {
            if (!WritePlaceholder(xShape, Subtitle, mbMaster))
                ShapeExport::WriteTextShape(xShape);
        }
    }
    else
        SAL_WARN("sd.eppt", "unknown shape not handled: " << sShapeType.toUtf8());

    return *this;
}

PowerPointExport::PowerPointExport(const Reference< XComponentContext >& rContext, const uno::Sequence<uno::Any>& rArguments)
    : XmlFilterBase(rContext)
    , mnLayoutFileIdMax(1)
    , mnSlideIdMax(1 << 8)
    , mnSlideMasterIdMax(1U << 31)
    , mnAnimationNodeIdMax(1)
    , mnDiagramId(1)
    , mbCreateNotes(false)
    , mnPlaceholderIndexMax(1)
{
    comphelper::SequenceAsHashMap aArgumentsMap(rArguments);
    mbPptm = aArgumentsMap.getUnpackedValueOrDefault(u"IsPPTM"_ustr, false);
    mbExportTemplate = aArgumentsMap.getUnpackedValueOrDefault(u"IsTemplate"_ustr, false);
}

PowerPointExport::~PowerPointExport()
{
}

void PowerPointExport::writeDocumentProperties()
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(mXModel, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocProps = xDPS->getDocumentProperties();

    if (xDocProps.is())
    {
        bool bSecurityOptOpenReadOnly = false;
        uno::Reference< lang::XMultiServiceFactory > xFactory(mXModel, uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet > xSettings(xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY);
        try
        {
            xSettings->getPropertyValue(u"LoadReadonly"_ustr) >>= bSecurityOptOpenReadOnly;
        }
        catch( Exception& )
        {
        }
        exportDocumentProperties(xDocProps, bSecurityOptOpenReadOnly);
    }

    exportCustomFragments();
}

bool PowerPointExport::importDocument() noexcept
{
    return false;
}

bool PowerPointExport::exportDocument()
{
    drawingml::DrawingML::ResetMlCounters();
    auto& rGraphicExportCache = drawingml::GraphicExportCache::get();

    rGraphicExportCache.push();

    maShapeMap.clear();

    mXModel = getModel();

    //write document properties
    writeDocumentProperties();

    addRelation(oox::getRelationship(Relationship::OFFICEDOCUMENT), u"ppt/presentation.xml");

    OUString aMediaType;
    if (mbPptm)
    {
        if (mbExportTemplate)
        {
            aMediaType = "application/vnd.ms-powerpoint.template.macroEnabled.main+xml";
        }
        else
        {
            aMediaType = "application/vnd.ms-powerpoint.presentation.macroEnabled.main+xml";
        }
    }
    else
    {
        if (mbExportTemplate)
        {
            aMediaType = "application/vnd.openxmlformats-officedocument.presentationml.template.main+xml";
        }
        else
        {
            aMediaType = "application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml";
        }
    }

    mPresentationFS = openFragmentStreamWithSerializer(u"ppt/presentation.xml"_ustr, aMediaType);

    addRelation(mPresentationFS->getOutputStream(),
                oox::getRelationship(Relationship::THEME),
                u"theme/theme1.xml");

    mPresentationFS->startElementNS(XML_p, XML_presentation, PNMSS);

    mXStatusIndicator = getStatusIndicator();

    std::vector< PropertyValue > aProperties;
    PropertyValue aProperty;
    aProperty.Name = "BaseURI";
    aProperty.Value <<= getFileUrl();
    aProperties.push_back(aProperty);

    exportPPT(aProperties);

    mPresentationFS->singleElementNS(XML_p, XML_sldSz,
                                     XML_cx, OString::number(PPTtoEMU(maDestPageSize.Width)),
                                     XML_cy, OString::number(PPTtoEMU(maDestPageSize.Height)));
    // for some reason if added before slides list it will not load the slides (alas with error reports) in mso
    mPresentationFS->singleElementNS(XML_p, XML_notesSz,
                                     XML_cx, OString::number(PPTtoEMU(maNotesPageSize.Width)),
                                     XML_cy, OString::number(PPTtoEMU(maNotesPageSize.Height)));

    WriteCustomSlideShow();

    WritePresentationProps();

    WriteAuthors();

    WriteVBA();

    WriteModifyVerifier();

    mPresentationFS->endElementNS(XML_p, XML_presentation);
    mPresentationFS->endDocument();
    mPresentationFS.reset();
    // Free all FSHelperPtr, to flush data before committing storage
    for (auto& serializer : mpSlidesFSArray)
    {
        if (!serializer)
            continue;
        serializer->endDocument();
    }
    mpSlidesFSArray.clear();

    commitStorage();

    rGraphicExportCache.pop();

    maShapeMap.clear();
    maAuthors.clear();
    maRelId.clear();

    return true;
}

::oox::ole::VbaProject* PowerPointExport::implCreateVbaProject() const
{
    return new ::oox::ole::VbaProject(getComponentContext(), getModel(), u"Impress");
}

void PowerPointExport::WriteCustomSlideShow()
{
    Reference<XCustomPresentationSupplier> aXCPSup(mXModel, css::uno::UNO_QUERY);
    if (!aXCPSup.is() || !aXCPSup->getCustomPresentations()->hasElements())
        return;

    mPresentationFS->startElementNS(XML_p, XML_custShowLst);

    Reference<XDrawPagesSupplier> xDPS(getModel(), uno::UNO_QUERY_THROW);
    Reference<XDrawPages> xDrawPages(xDPS->getDrawPages(), uno::UNO_SET_THROW);
    Reference<XNameContainer> aXNameCont(aXCPSup->getCustomPresentations());
    const Sequence<OUString> aNameSeq(aXNameCont->getElementNames());

    OUString sRelId;
    sal_uInt32 nCustomShowIndex = 0;
    sal_Int32 nSlideCount = xDrawPages->getCount();

    for (OUString const& customShowName : aNameSeq)
    {
        mPresentationFS->startElementNS(XML_p, XML_custShow, XML_name, customShowName, XML_id,
                                        OUString::number(nCustomShowIndex++));

        mAny = aXNameCont->getByName(customShowName);
        Reference<XIndexContainer> aXIContainer;
        if (mAny >>= aXIContainer)
        {
            mPresentationFS->startElementNS(XML_p, XML_sldLst);

            sal_Int32 nCustomShowSlideCount = aXIContainer->getCount();
            for (sal_Int32 i = 0; i < nCustomShowSlideCount; ++i)
            {
                Reference<XDrawPage> aXCustomShowDrawPage;
                aXIContainer->getByIndex(i) >>= aXCustomShowDrawPage;
                Reference<XNamed> aXName(aXCustomShowDrawPage, UNO_QUERY_THROW);
                OUString sCustomShowSlideName = aXName->getName();

                for (sal_Int32 j = 0; j < nSlideCount; ++j)
                {
                    Reference<XDrawPage> xDrawPage;
                    xDrawPages->getByIndex(j) >>= xDrawPage;
                    Reference<XNamed> xNamed(xDrawPage, UNO_QUERY_THROW);
                    OUString sSlideName = xNamed->getName();

                    if (sCustomShowSlideName == sSlideName)
                    {
                        sRelId = maRelId[j];
                        break;
                    }
                }
                mPresentationFS->singleElementNS(XML_p, XML_sld, FSNS(XML_r, XML_id), sRelId);
            }
            mPresentationFS->endElementNS(XML_p, XML_sldLst);
        }
        mPresentationFS->endElementNS(XML_p, XML_custShow);
    }
    mPresentationFS->endElementNS(XML_p, XML_custShowLst);
}

void PowerPointExport::ImplWriteBackground(const FSHelperPtr& pFS, const Reference< XPropertySet >& rXPropSet)
{
    FillStyle aFillStyle(FillStyle_NONE);
    if (ImplGetPropertyValue(rXPropSet, u"FillStyle"_ustr))
        mAny >>= aFillStyle;

    if (aFillStyle == FillStyle_NONE ||
            aFillStyle == FillStyle_HATCH)
        return;

    pFS->startElementNS(XML_p, XML_bg);
    pFS->startElementNS(XML_p, XML_bgPr);

    PowerPointShapeExport aDML(pFS, &maShapeMap, this);
    aDML.SetBackgroundDark(mbIsBackgroundDark);
    aDML.WriteFill(rXPropSet, maPageSize);

    pFS->endElementNS(XML_p, XML_bgPr);
    pFS->endElementNS(XML_p, XML_bg);
}

#define MAIN_GROUP \
     "<p:nvGrpSpPr>\
        <p:cNvPr id=\"1\" name=\"\"/>\
        <p:cNvGrpSpPr/>\
        <p:nvPr/>\
      </p:nvGrpSpPr>\
      <p:grpSpPr>\
        <a:xfrm>\
          <a:off x=\"0\" y=\"0\"/>\
          <a:ext cx=\"0\" cy=\"0\"/>\
          <a:chOff x=\"0\" y=\"0\"/>\
          <a:chExt cx=\"0\" cy=\"0\"/>\
        </a:xfrm>\
      </p:grpSpPr>"

const char* PowerPointExport::GetSideDirection(sal_uInt8 nDirection)
{
    const char* pDirection = nullptr;

    switch (nDirection)
    {
    case 0:
        pDirection = "l";
        break;
    case 1:
        pDirection = "u";
        break;
    case 2:
        pDirection = "r";
        break;
    case 3:
        pDirection = "d";
        break;
    }

    return pDirection;
}

const char* PowerPointExport::GetCornerDirection(sal_uInt8 nDirection)
{
    const char* pDirection = nullptr;

    switch (nDirection)
    {
    case 4:
        pDirection = "lu";
        break;
    case 5:
        pDirection = "ru";
        break;
    case 6:
        pDirection = "ld";
        break;
    case 7:
        pDirection = "rd";
        break;
    }

    return pDirection;
}

const char* PowerPointExport::Get8Direction(sal_uInt8 nDirection)
{
    const char* pDirection = GetSideDirection(nDirection);

    if (!pDirection)
        pDirection = GetCornerDirection(nDirection);

    return pDirection;
}

void PowerPointExport::WriteTransition(const FSHelperPtr& pFS)
{
    FadeEffect eFadeEffect = FadeEffect_NONE;
    if (ImplGetPropertyValue(mXPagePropSet, u"Effect"_ustr))
        mAny >>= eFadeEffect;

    sal_Int16 nTransitionType = 0, nTransitionSubtype = 0;
    sal_Int8 nPPTTransitionType = 0;
    sal_uInt8 nDirection = 0;

    OUString sSoundUrl;
    OUString sSoundRelId;
    OUString sSoundName;

    if (ImplGetPropertyValue(mXPagePropSet, u"TransitionType"_ustr) && (mAny >>= nTransitionType) &&
            ImplGetPropertyValue(mXPagePropSet, u"TransitionSubtype"_ustr) && (mAny >>= nTransitionSubtype))
    {
        // FADEOVERCOLOR with black -> fade, with white -> flash
        sal_Int32 nTransitionFadeColor = 0;
        if( ImplGetPropertyValue(mXPagePropSet, u"TransitionFadeColor"_ustr))
            mAny >>= nTransitionFadeColor;
        nPPTTransitionType = GetTransition(nTransitionType, nTransitionSubtype, eFadeEffect, nTransitionFadeColor, nDirection);
    }

    if (!nPPTTransitionType && eFadeEffect != FadeEffect_NONE)
        nPPTTransitionType = GetTransition(eFadeEffect, nDirection);

    if (ImplGetPropertyValue(mXPagePropSet, u"Sound"_ustr) && (mAny >>= sSoundUrl))
        embedEffectAudio(pFS, sSoundUrl, sSoundRelId, sSoundName);

    bool bOOXmlSpecificTransition = false;

    sal_Int32 nTransition = 0;
    const char* pDirection = nullptr;
    const char* pOrientation = nullptr;
    const char* pThruBlk = nullptr;
    const char* pSpokes = nullptr;

    char pSpokesTmp[2] = "0";

    // p14
    sal_Int32 nTransition14 = 0;
    const char* pDirection14 = nullptr;
    const char* pInverted = nullptr;
    const char* pPattern = nullptr; // diamond or hexagon

    //p15
    const char* pPresetTransition = nullptr;

    if (!nPPTTransitionType)
    {
        switch (nTransitionType)
        {
        case animations::TransitionType::BARWIPE:
        {
            if (nTransitionSubtype == animations::TransitionSubType::FADEOVERCOLOR)
            {
                nTransition = XML_cut;
                pThruBlk = "true";
                bOOXmlSpecificTransition = true;
            }
            break;
        }
        case animations::TransitionType::MISCSHAPEWIPE:
        {
            switch (nTransitionSubtype)
            {
            case animations::TransitionSubType::TOPTOBOTTOM: // Turn around
                nTransition = XML_fade;
                nTransition14 = XML_flip;
                pDirection14 = "l";
                bOOXmlSpecificTransition = true;
                break;
            case animations::TransitionSubType::BOTTOMRIGHT: // Rochade
                nTransition = XML_fade;
                nTransition14 = XML_switch;
                pDirection14 = "r";
                bOOXmlSpecificTransition = true;
                break;
            case animations::TransitionSubType::VERTICAL: // Vortex
                nTransition = XML_fade;
                nTransition14 = XML_vortex;
                bOOXmlSpecificTransition = true;
                break;
            case animations::TransitionSubType::HORIZONTAL: // Ripple
                nTransition = XML_fade;
                nTransition14 = XML_ripple;
                bOOXmlSpecificTransition = true;
                break;
            case animations::TransitionSubType::LEFTTORIGHT: // Fall
                nTransition = XML_fade;
                pPresetTransition = "fallOver";
                bOOXmlSpecificTransition = true;
                break;
            case animations::TransitionSubType::CORNERSIN: // Inside turning cube
                pInverted = "true";
                [[fallthrough]];
            case animations::TransitionSubType::CORNERSOUT: // Outside turning cube
                nTransition = XML_fade;
                nTransition14 = XML_prism;
                bOOXmlSpecificTransition = true;
                break;
            case animations::TransitionSubType::DIAMOND: // Glitter
                nTransition = XML_fade;
                nTransition14 = XML_glitter;
                pDirection14 = "l";
                pPattern = "hexagon";
                bOOXmlSpecificTransition = true;
                break;
            case animations::TransitionSubType::HEART: // Honeycomb
                nTransition = XML_fade;
                nTransition14 = XML_honeycomb;
                bOOXmlSpecificTransition = true;
                break;
            }
            break;
        }
        }
    }

    AnimationSpeed animationSpeed = AnimationSpeed_MEDIUM;
    const char* speed = nullptr;
    sal_Int32 advanceTiming = -1;
    sal_Int32 changeType = 0;

    sal_Int32 nTransitionDuration = -1;
    bool isTransitionDurationSet = false;

    // try to use TransitionDuration instead of old Speed property
    if (ImplGetPropertyValue(mXPagePropSet, u"TransitionDuration"_ustr))
    {
        double fTransitionDuration = -1.0;
        mAny >>= fTransitionDuration;
        if (fTransitionDuration >= 0)
        {
            nTransitionDuration = fTransitionDuration * 1000.0;

            // override values because in MS formats meaning of fast/medium/slow is different
            if (nTransitionDuration <= 500)
            {
                // fast is default
                speed = nullptr;
            }
            else if (nTransitionDuration >= 1000)
            {
                speed = "slow";
            }
            else
            {
                speed = "med";
            }

            bool isStandardValue = nTransitionDuration == 500
                || nTransitionDuration == 750
                || nTransitionDuration == 1000;

            if(!isStandardValue)
                isTransitionDurationSet = true;
        }
    }
    else if (ImplGetPropertyValue(mXPagePropSet, u"Speed"_ustr))
    {
        mAny >>= animationSpeed;

        switch (animationSpeed)
        {
        default:
        case AnimationSpeed_MEDIUM:
            speed = "med";
            break;
        case AnimationSpeed_SLOW:
            speed = "slow";
            break;
        case AnimationSpeed_FAST:
            break;
        }
    }

    // check if we resolved what transition to export or time is set
    if (!nPPTTransitionType && !bOOXmlSpecificTransition && !isTransitionDurationSet)
        return;

    if (ImplGetPropertyValue(mXPagePropSet, u"Change"_ustr))
        mAny >>= changeType;

    // 1 means automatic, 2 half automatic - not sure what it means - at least I don't see it in UI
    if (changeType == 1 && ImplGetPropertyValue(mXPagePropSet, u"Duration"_ustr))
        mAny >>= advanceTiming;

    if (!bOOXmlSpecificTransition)
    {
        switch (nPPTTransitionType)
        {
        case PPT_TRANSITION_TYPE_BLINDS:
            nTransition = XML_blinds;
            pDirection = (nDirection == 0) ? "vert" : "horz";
            break;
        case PPT_TRANSITION_TYPE_CHECKER:
            nTransition = XML_checker;
            pDirection = (nDirection == 1) ? "vert" : "horz";
            break;
        case PPT_TRANSITION_TYPE_CIRCLE:
            nTransition = XML_circle;
            break;
        case PPT_TRANSITION_TYPE_COMB:
            nTransition = XML_comb;
            pDirection = (nDirection == 1) ? "vert" : "horz";
            break;
        case PPT_TRANSITION_TYPE_COVER:
            nTransition = XML_cover;
            pDirection = Get8Direction(nDirection);
            break;
        case PPT_TRANSITION_TYPE_DIAMOND:
            nTransition = XML_diamond;
            break;
        case PPT_TRANSITION_TYPE_DISSOLVE:
            nTransition = XML_dissolve;
            break;
        case PPT_TRANSITION_TYPE_FADE:
            nTransition = XML_fade;
            pThruBlk = "true";
            break;
        case PPT_TRANSITION_TYPE_SMOOTHFADE:
            nTransition = XML_fade;
            break;
        case PPT_TRANSITION_TYPE_NEWSFLASH:
            nTransition = XML_newsflash;
            break;
        case PPT_TRANSITION_TYPE_PLUS:
            nTransition = XML_plus;
            break;
        case PPT_TRANSITION_TYPE_PULL:
            nTransition = XML_pull;
            pDirection = Get8Direction(nDirection);
            break;
        case PPT_TRANSITION_TYPE_PUSH:
            nTransition = XML_push;
            pDirection = GetSideDirection(nDirection);
            break;
        case PPT_TRANSITION_TYPE_RANDOM:
            nTransition = XML_random;
            break;
        case PPT_TRANSITION_TYPE_RANDOM_BARS:
            nTransition = XML_randomBar;
            pDirection = (nDirection == 1) ? "vert" : "horz";
            break;
        case PPT_TRANSITION_TYPE_SPLIT:
            nTransition = XML_split;
            pDirection = (nDirection & 1) ? "in" : "out";
            pOrientation = (nDirection < 2) ? "horz" : "vert";
            break;
        case PPT_TRANSITION_TYPE_STRIPS:
            nTransition = XML_strips;
            pDirection = GetCornerDirection(nDirection);
            break;
        case PPT_TRANSITION_TYPE_WEDGE:
            nTransition = XML_wedge;
            break;
        case PPT_TRANSITION_TYPE_WHEEL:
            nTransition = XML_wheel;
            if (nDirection != 4 && nDirection <= 9)
            {
                pSpokesTmp[0] = '0' + nDirection;
                pSpokes = pSpokesTmp;
            }
            break;
        case PPT_TRANSITION_TYPE_WIPE:
            nTransition = XML_wipe;
            pDirection = GetSideDirection(nDirection);
            break;
        case PPT_TRANSITION_TYPE_ZOOM:
            nTransition = XML_zoom;
            pDirection = (nDirection == 1) ? "in" : "out";
            break;
        case PPT_TRANSITION_TYPE_FLASH:
            nTransition14 = XML_flash;
            nTransition = XML_fade;
            bOOXmlSpecificTransition = true;
            break;
        // coverity[dead_error_line] - following conditions exist to avoid compiler warning
        case PPT_TRANSITION_TYPE_NONE:
        default:
            nTransition = 0;
            break;
        }
    }

    bool isAdvanceTimingSet = advanceTiming != -1;
    if (nTransition14 || pPresetTransition || isTransitionDurationSet)
    {
        const char* pRequiresNS = (nTransition14 || isTransitionDurationSet) ? "p14" : "p15";

        pFS->startElement(FSNS(XML_mc, XML_AlternateContent));
        pFS->startElement(FSNS(XML_mc, XML_Choice), XML_Requires, pRequiresNS);

        if(isTransitionDurationSet && isAdvanceTimingSet)
        {
            pFS->startElementNS(XML_p, XML_transition,
                XML_spd, speed,
                XML_advTm, OString::number(advanceTiming * 1000),
                FSNS(XML_p14, XML_dur), OString::number(nTransitionDuration));
        }
        else if(isTransitionDurationSet)
        {
            pFS->startElementNS(XML_p, XML_transition,
                XML_spd, speed,
                FSNS(XML_p14, XML_dur), OString::number(nTransitionDuration));
        }
        else if(isAdvanceTimingSet)
        {
            pFS->startElementNS(XML_p, XML_transition,
                XML_spd, speed,
                XML_advTm, OString::number(advanceTiming * 1000));
        }
        else
        {
            pFS->startElementNS(XML_p, XML_transition, XML_spd, speed);
        }

        if (nTransition14)
        {
            pFS->singleElementNS(XML_p14, nTransition14,
                XML_isInverted, pInverted,
                XML_dir, pDirection14,
                XML_pattern, pPattern);
        }
        else if (pPresetTransition)
        {
            pFS->singleElementNS(XML_p15, XML_prstTrans,
                XML_prst, pPresetTransition);
        }
        else if (isTransitionDurationSet && nTransition)
        {
            pFS->singleElementNS(XML_p, nTransition,
                XML_dir, pDirection,
                XML_orient, pOrientation,
                XML_spokes, pSpokes,
                XML_thruBlk, pThruBlk);
        }

        if (!sSoundRelId.isEmpty())
            WriteSndAc(pFS, sSoundRelId, sSoundName);

        pFS->endElement(FSNS(XML_p, XML_transition));

        pFS->endElement(FSNS(XML_mc, XML_Choice));
        pFS->startElement(FSNS(XML_mc, XML_Fallback));
    }

    pFS->startElementNS(XML_p, XML_transition,
        XML_spd, speed,
        XML_advTm, sax_fastparser::UseIf(OString::number(advanceTiming * 1000), isAdvanceTimingSet));

    if (nTransition)
    {
        pFS->singleElementNS(XML_p, nTransition,
                             XML_dir, pDirection,
                             XML_orient, pOrientation,
                             XML_spokes, pSpokes,
                             XML_thruBlk, pThruBlk);
    }

    if (!sSoundRelId.isEmpty())
        WriteSndAc(pFS, sSoundRelId, sSoundName);

    pFS->endElementNS(XML_p, XML_transition);

    if (nTransition14 || pPresetTransition || isTransitionDurationSet)
    {
        pFS->endElement(FSNS(XML_mc, XML_Fallback));
        pFS->endElement(FSNS(XML_mc, XML_AlternateContent));
    }
}

static OUString lcl_GetInitials(std::u16string_view sName)
{
    OUStringBuffer sRet;

    if (!sName.empty())
    {
        sRet.append(sName[0]);
        size_t nStart = 0, nOffset;

        while ((nOffset = sName.find(' ', nStart)) != std::u16string_view::npos)
        {
            if (nOffset + 1 < sName.size())
                sRet.append(sName[ nOffset + 1 ]);
            nStart = nOffset + 1;
        }
    }

    return sRet.makeStringAndClear();
}

void PowerPointExport::WriteAuthors()
{
    if (maAuthors.empty())
        return;

    FSHelperPtr pFS = openFragmentStreamWithSerializer(u"ppt/commentAuthors.xml"_ustr,
                      u"application/vnd.openxmlformats-officedocument.presentationml.commentAuthors+xml"_ustr);
    addRelation(mPresentationFS->getOutputStream(),
                oox::getRelationship(Relationship::COMMENTAUTHORS),
                u"commentAuthors.xml");

    pFS->startElementNS(XML_p, XML_cmAuthorLst,
                        FSNS(XML_xmlns, XML_p), getNamespaceURL(OOX_NS(ppt)));

    for (const AuthorsMap::value_type& i : maAuthors)
    {
        pFS->singleElementNS(XML_p, XML_cmAuthor,
                             XML_id, OString::number(i.second.nId),
                             XML_name, i.first,
                             XML_initials, lcl_GetInitials(i.first),
                             XML_lastIdx, OString::number(i.second.nLastIndex),
                             XML_clrIdx, OString::number(i.second.nId));
    }

    pFS->endElementNS(XML_p, XML_cmAuthorLst);

    pFS->endDocument();
}

sal_Int32 PowerPointExport::GetAuthorIdAndLastIndex(const OUString& sAuthor, sal_Int32& nLastIndex)
{
    if (maAuthors.count(sAuthor) <= 0)
    {
        struct AuthorComments aAuthorComments;

        aAuthorComments.nId = maAuthors.size();
        aAuthorComments.nLastIndex = 0;

        maAuthors[ sAuthor ] = aAuthorComments;
    }

    nLastIndex = ++maAuthors[ sAuthor ].nLastIndex;

    return maAuthors[ sAuthor ].nId;
}

void PowerPointExport::WritePresentationProps()
{
    Reference<XPresentationSupplier> xPresentationSupplier(mXModel, uno::UNO_QUERY);
    if (!xPresentationSupplier.is())
        return;

    Reference<beans::XPropertySet> xPresentationProps(xPresentationSupplier->getPresentation(),
                                                      uno::UNO_QUERY);
    bool bEndlessVal = xPresentationProps->getPropertyValue(u"IsEndless"_ustr).get<bool>();
    bool bChangeManually = xPresentationProps->getPropertyValue(u"IsAutomatic"_ustr).get<bool>();
    OUString sFirstPage = xPresentationProps->getPropertyValue(u"FirstPage"_ustr).get<OUString>();
    OUString sCustomShow = xPresentationProps->getPropertyValue(u"CustomShow"_ustr).get<OUString>();

    FSHelperPtr pFS = openFragmentStreamWithSerializer(
        u"ppt/presProps.xml"_ustr,
        u"application/vnd.openxmlformats-officedocument.presentationml.presProps+xml"_ustr);

    addRelation(mPresentationFS->getOutputStream(),
                oox::getRelationship(Relationship::PRESPROPS), u"presProps.xml");

    pFS->startElementNS(XML_p, XML_presentationPr, PPRNMSS);

    pFS->startElementNS(XML_p, XML_showPr, XML_loop, sax_fastparser::UseIf("1", bEndlessVal),
                        XML_useTimings, sax_fastparser::UseIf("0", bChangeManually),
                        XML_showNarration, "1");

    Reference<drawing::XDrawPagesSupplier> xDPS(mXModel, uno::UNO_QUERY_THROW);
    Reference<drawing::XDrawPages> xDrawPages(xDPS->getDrawPages(), uno::UNO_SET_THROW);
    if (!sFirstPage.isEmpty())
    {
        sal_Int32 nStartSlide = 1;
        sal_Int32 nEndSlide = xDrawPages->getCount();
        for (sal_Int32 i = 0; i < nEndSlide; i++)
        {
            Reference<drawing::XDrawPage> xDrawPage;
            xDrawPages->getByIndex(i) >>= xDrawPage;
            Reference<container::XNamed> xNamed(xDrawPage, uno::UNO_QUERY_THROW);
            if (xNamed->getName() == sFirstPage)
            {
                nStartSlide = i + 1;
                break;
            }
        }

        pFS->singleElementNS(XML_p, XML_sldRg, XML_st, OUString::number(nStartSlide), XML_end,
                             OUString::number(nEndSlide));
    }

    if (!sCustomShow.isEmpty())
    {
        css::uno::Reference<css::presentation::XCustomPresentationSupplier>
            XCustPresentationSupplier(mXModel, css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::container::XNameContainer> mxCustShows;
        mxCustShows = XCustPresentationSupplier->getCustomPresentations();
        const css::uno::Sequence<OUString> aNameSeq(mxCustShows->getElementNames());

        sal_Int32 nCustShowIndex = 0;
        for (sal_Int32 i = 0; i < aNameSeq.getLength(); i++)
        {
            if (aNameSeq[i] == sCustomShow)
            {
                nCustShowIndex = i;
                break;
            }
        }

        pFS->singleElementNS(XML_p, XML_custShow, XML_id, OUString::number(nCustShowIndex));
    }

    pFS->endElementNS(XML_p, XML_showPr);

    pFS->endElementNS(XML_p, XML_presentationPr);

    pFS->endDocument();
}

bool PowerPointExport::WriteComments(sal_uInt32 nPageNum)
{
    Reference< XAnnotationAccess > xAnnotationAccess(mXDrawPage, uno::UNO_QUERY);
    if (xAnnotationAccess.is())
    {
        Reference< XAnnotationEnumeration > xAnnotationEnumeration(xAnnotationAccess->createAnnotationEnumeration());

        if (xAnnotationEnumeration->hasMoreElements())
        {
            FSHelperPtr pFS = openFragmentStreamWithSerializer(
                              "ppt/comments/comment" + OUString::number(nPageNum + 1) + ".xml",
                              u"application/vnd.openxmlformats-officedocument.presentationml.comments+xml"_ustr);

            pFS->startElementNS(XML_p, XML_cmLst,
                                FSNS(XML_xmlns, XML_p), this->getNamespaceURL(OOX_NS(ppt)));

            do
            {
                Reference< XAnnotation > xAnnotation(xAnnotationEnumeration->nextElement());
                util::DateTime aDateTime(xAnnotation->getDateTime());
                RealPoint2D aRealPoint2D(xAnnotation->getPosition());
                Reference< XText > xText(xAnnotation->getTextRange());
                sal_Int32 nLastIndex;
                sal_Int32 nId = GetAuthorIdAndLastIndex(xAnnotation->getAuthor(), nLastIndex);
                char cDateTime[sizeof("-32768-65535-65535T65535:65535:65535.4294967295")];
                    // reserve enough space for hypothetical max length

                snprintf(cDateTime, sizeof cDateTime, "%02" SAL_PRIdINT32 "-%02" SAL_PRIuUINT32 "-%02" SAL_PRIuUINT32 "T%02" SAL_PRIuUINT32 ":%02" SAL_PRIuUINT32 ":%02" SAL_PRIuUINT32 ".%09" SAL_PRIuUINT32, sal_Int32(aDateTime.Year), sal_uInt32(aDateTime.Month), sal_uInt32(aDateTime.Day), sal_uInt32(aDateTime.Hours), sal_uInt32(aDateTime.Minutes), sal_uInt32(aDateTime.Seconds), aDateTime.NanoSeconds);

                pFS->startElementNS(XML_p, XML_cm,
                                    XML_authorId, OString::number(nId),
                                    XML_dt, cDateTime,
                                    XML_idx, OString::number(nLastIndex));

                pFS->singleElementNS(XML_p, XML_pos,
                                     XML_x, OString::number(std::round(convertMm100ToMasterUnit(aRealPoint2D.X * 100))),
                                     XML_y, OString::number(std::round(convertMm100ToMasterUnit(aRealPoint2D.Y * 100))));

                pFS->startElementNS(XML_p, XML_text);
                pFS->write(xText->getString());
                pFS->endElementNS(XML_p, XML_text);

                pFS->endElementNS(XML_p, XML_cm);

            }
            while (xAnnotationEnumeration->hasMoreElements());

            pFS->endElementNS(XML_p, XML_cmLst);

            pFS->endDocument();

            return true;
        }
    }

    return false;
}

void PowerPointExport::WriteVBA()
{
    if (!mbPptm)
        return;

    uno::Reference<document::XStorageBasedDocument> xStorageBasedDocument(getModel(), uno::UNO_QUERY);
    if (!xStorageBasedDocument.is())
        return;

    uno::Reference<embed::XStorage> xDocumentStorage = xStorageBasedDocument->getDocumentStorage();
    OUString aMacrosName(u"_MS_VBA_Macros"_ustr);
    if (!xDocumentStorage.is() || !xDocumentStorage->hasByName(aMacrosName))
        return;

    const sal_Int32 nOpenMode = embed::ElementModes::READ;
    uno::Reference<io::XInputStream> xMacrosStream(xDocumentStorage->openStreamElement(aMacrosName, nOpenMode), uno::UNO_QUERY);
    if (!xMacrosStream.is())
        return;

    uno::Reference<io::XOutputStream> xOutputStream = openFragmentStream(u"ppt/vbaProject.bin"_ustr, u"application/vnd.ms-office.vbaProject"_ustr);
    comphelper::OStorageHelper::CopyInputToOutput(xMacrosStream, xOutputStream);

    // Write the relationship.
    addRelation(mPresentationFS->getOutputStream(), oox::getRelationship(Relationship::VBAPROJECT), u"vbaProject.bin");
}

void PowerPointExport::WriteModifyVerifier()
{
    Sequence<PropertyValue> aInfo;

    try
    {
        Reference<lang::XMultiServiceFactory> xFactory(mXModel, UNO_QUERY);
        Reference<XPropertySet> xDocSettings(
            xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), UNO_QUERY);
        xDocSettings->getPropertyValue(u"ModifyPasswordInfo"_ustr) >>= aInfo;
    }
    catch (const Exception&)
    {
    }

    if (aInfo.hasElements())
    {
        OUString sAlgorithm, sSalt, sHash;
        sal_Int32 nCount = 0;
        for (auto& prop : aInfo)
        {
            if (prop.Name == "algorithm-name")
                prop.Value >>= sAlgorithm;
            else if (prop.Name == "salt")
                prop.Value >>= sSalt;
            else if (prop.Name == "iteration-count")
                prop.Value >>= nCount;
            else if (prop.Name == "hash")
                prop.Value >>= sHash;
        }
        if (!sAlgorithm.isEmpty() && !sSalt.isEmpty() && !sHash.isEmpty())
        {
            sal_Int32 nAlgorithmSid = 0;
            if (sAlgorithm == "MD2")
                nAlgorithmSid = 1;
            else if (sAlgorithm == "MD4")
                nAlgorithmSid = 2;
            else if (sAlgorithm == "MD5")
                nAlgorithmSid = 3;
            else if (sAlgorithm == "SHA-1")
                nAlgorithmSid = 4;
            else if (sAlgorithm == "MAC")
                nAlgorithmSid = 5;
            else if (sAlgorithm == "RIPEMD")
                nAlgorithmSid = 6;
            else if (sAlgorithm == "RIPEMD-160")
                nAlgorithmSid = 7;
            else if (sAlgorithm == "HMAC")
                nAlgorithmSid = 9;
            else if (sAlgorithm == "SHA-256")
                nAlgorithmSid = 12;
            else if (sAlgorithm == "SHA-384")
                nAlgorithmSid = 13;
            else if (sAlgorithm == "SHA-512")
                nAlgorithmSid = 14;

            if (nAlgorithmSid != 0)
                mPresentationFS->singleElementNS(XML_p, XML_modifyVerifier,
                    XML_cryptProviderType, "rsaAES",
                    XML_cryptAlgorithmClass, "hash",
                    XML_cryptAlgorithmType, "typeAny",
                    XML_cryptAlgorithmSid, OString::number(nAlgorithmSid),
                    XML_spinCount, OString::number(nCount),
                    XML_saltData, sSalt,
                    XML_hashData, sHash);
        }
    }
}

void PowerPointExport::ImplWriteSlide(sal_uInt32 nPageNum, sal_uInt32 nMasterNum, sal_uInt16 /* nMode */,
                                      bool bHasBackground, Reference< XPropertySet > const& aXBackgroundPropSet)
{
    SAL_INFO("sd.eppt", "write slide: " << nPageNum << "\n----------------");

    // slides list
    if (nPageNum == 0)
        mPresentationFS->startElementNS(XML_p, XML_sldIdLst);

    // add explicit relation of presentation to this slide
    OUString sRelId = addRelation(mPresentationFS->getOutputStream(),
                                  oox::getRelationship(Relationship::SLIDE),
                                  Concat2View("slides/slide" + OUString::number(nPageNum + 1) +".xml"));

    mPresentationFS->singleElementNS(XML_p, XML_sldId,
                                     XML_id, OString::number(GetNewSlideId()),
                                     FSNS(XML_r, XML_id), sRelId);

    maRelId.push_back(sRelId);

    if (nPageNum == mnPages - 1)
        mPresentationFS->endElementNS(XML_p, XML_sldIdLst);

    FSHelperPtr pFS = openFragmentStreamWithSerializer(
                        "ppt/slides/slide" + OUString::number(nPageNum + 1) + ".xml",
                        u"application/vnd.openxmlformats-officedocument.presentationml.slide+xml"_ustr);

    if (mpSlidesFSArray.size() < mnPages)
        mpSlidesFSArray.resize(mnPages);
    mpSlidesFSArray[ nPageNum ] = pFS;

    const char* pShow = nullptr;
    const char* pShowMasterShape = nullptr;

    if (ImplGetPropertyValue(mXPagePropSet, u"Visible"_ustr))
    {
        bool bShow(false);
        if ((mAny >>= bShow) && !bShow)
            pShow = "0";
    }

    if (ImplGetPropertyValue(mXPagePropSet, u"IsBackgroundObjectsVisible"_ustr))
    {
        bool bShowMasterShape(false);
        if ((mAny >>= bShowMasterShape) && !bShowMasterShape)
            pShowMasterShape = "0";
    }

    pFS->startElementNS(XML_p, XML_sld, PNMSS, XML_show, pShow, XML_showMasterSp, pShowMasterShape);

    pFS->startElementNS(XML_p, XML_cSld);

    // background
    if (bHasBackground)
    {
        ImplWriteBackground(pFS, aXBackgroundPropSet);
    }

    WriteShapeTree(pFS, NORMAL, false);

    pFS->endElementNS(XML_p, XML_cSld);

    WriteTransition(pFS);
    WriteAnimations(pFS, mXDrawPage, *this);

    pFS->endElementNS(XML_p, XML_sld);

    // add implicit relation to slide layout
    addRelation(pFS->getOutputStream(),
                oox::getRelationship(Relationship::SLIDELAYOUT),
                Concat2View("../slideLayouts/slideLayout" +
                    OUString::number(GetLayoutFileId(GetLayoutOffset(mXPagePropSet), nMasterNum)) +
                    ".xml"));

    if (WriteComments(nPageNum))
        // add implicit relation to slide comments
        addRelation(pFS->getOutputStream(),
                    oox::getRelationship(Relationship::COMMENTS),
                    Concat2View("../comments/comment" + OUString::number(nPageNum + 1) + ".xml"));

    SAL_INFO("sd.eppt", "----------------");
}

void PowerPointExport::ImplWriteNotes(sal_uInt32 nPageNum)
{
    if (!mbCreateNotes || !ContainsOtherShapeThanPlaceholders())
        return;

    SAL_INFO("sd.eppt", "write Notes " << nPageNum << "\n----------------");

    FSHelperPtr pFS = openFragmentStreamWithSerializer(
                        "ppt/notesSlides/notesSlide" +
                        OUString::number(nPageNum + 1) +
                        ".xml",
                        u"application/vnd.openxmlformats-officedocument.presentationml.notesSlide+xml"_ustr);

    pFS->startElementNS(XML_p, XML_notes, PNMSS);

    pFS->startElementNS(XML_p, XML_cSld);

    WriteShapeTree(pFS, NOTICE, false);

    pFS->endElementNS(XML_p, XML_cSld);

    pFS->endElementNS(XML_p, XML_notes);

    // add implicit relation to slide
    addRelation(pFS->getOutputStream(),
                oox::getRelationship(Relationship::SLIDE),
                Concat2View("../slides/slide" + OUString::number(nPageNum + 1) + ".xml"));

    // add slide implicit relation to notes
    if (nPageNum < mpSlidesFSArray.size())
        addRelation(mpSlidesFSArray[ nPageNum ]->getOutputStream(),
                    oox::getRelationship(Relationship::NOTESSLIDE),
                    Concat2View("../notesSlides/notesSlide" + OUString::number(nPageNum + 1) + ".xml"));

    // add implicit relation to notes master
    addRelation(pFS->getOutputStream(),
                oox::getRelationship(Relationship::NOTESMASTER),
                u"../notesMasters/notesMaster1.xml");

    SAL_INFO("sd.eppt", "-----------------");

    pFS->endDocument();
}

void PowerPointExport::AddLayoutIdAndRelation(const FSHelperPtr& pFS, sal_Int32 nLayoutFileId)
{
    // add implicit relation of slide master to slide layout
    OUString sRelId = addRelation(pFS->getOutputStream(),
                                  oox::getRelationship(Relationship::SLIDELAYOUT),
                                  Concat2View("../slideLayouts/slideLayout" + OUString::number(nLayoutFileId) + ".xml"));

    pFS->singleElementNS(XML_p, XML_sldLayoutId,
                         XML_id, OString::number(GetNewSlideMasterId()),
                         FSNS(XML_r, XML_id), sRelId);
}

void PowerPointExport::ImplWriteSlideMaster(sal_uInt32 nPageNum, Reference< XPropertySet > const& aXBackgroundPropSet)
{
    SAL_INFO("sd.eppt", "write master slide: " << nPageNum << "\n--------------");

    // slides list
    if (nPageNum == 0)
        mPresentationFS->startElementNS(XML_p, XML_sldMasterIdLst);

    OUString sRelId = addRelation(mPresentationFS->getOutputStream(),
                                  oox::getRelationship(Relationship::SLIDEMASTER),
                                  Concat2View("slideMasters/slideMaster" + OUString::number(nPageNum + 1) + ".xml"));

    mPresentationFS->singleElementNS(XML_p, XML_sldMasterId,
                                     XML_id, OString::number(GetNewSlideMasterId()),
                                     FSNS(XML_r, XML_id), sRelId);

    if (nPageNum == mnMasterPages - 1)
        mPresentationFS->endElementNS(XML_p, XML_sldMasterIdLst);

    FSHelperPtr pFS =
        openFragmentStreamWithSerializer("ppt/slideMasters/slideMaster" +
                                          OUString::number(nPageNum + 1) + ".xml",
                                         u"application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml"_ustr);

    SdrPage* pMasterPage = SdPage::getImplementation(mXDrawPage);
    model::Theme* pTheme = nullptr;
    if (pMasterPage)
    {
        pTheme = pMasterPage->getSdrPageProperties().getTheme().get();
    }

    // write theme per master
    WriteTheme(nPageNum, pTheme);

    // add implicit relation to the presentation theme
    addRelation(pFS->getOutputStream(),
                oox::getRelationship(Relationship::THEME),
                Concat2View("../theme/theme" + OUString::number(nPageNum + 1) + ".xml"));

    pFS->startElementNS(XML_p, XML_sldMaster, PNMSS);

    pFS->startElementNS(XML_p, XML_cSld);

    if (aXBackgroundPropSet)
        ImplWriteBackground(pFS, aXBackgroundPropSet);
    WriteShapeTree(pFS, MASTER, true);

    pFS->endElementNS(XML_p, XML_cSld);

    css::uno::Reference< css::beans::XPropertySet > xDocPropSet(getModel(), uno::UNO_QUERY);
    if (xDocPropSet.is())
    {
        uno::Sequence<beans::PropertyValue> aGrabBag;
        if (xDocPropSet->getPropertySetInfo()->hasPropertyByName(u"InteropGrabBag"_ustr))
            xDocPropSet->getPropertyValue(u"InteropGrabBag"_ustr) >>= aGrabBag;

        std::vector<OUString> aClrMap;
        aClrMap.reserve(12);
        uno::Sequence<beans::PropertyValue> aClrMapPropValue;
        if(aGrabBag.hasElements())
        {
            for (const auto& rProp : aGrabBag)
            {
                if (rProp.Name == "OOXColorMap")
                {
                    rProp.Value >>= aClrMapPropValue;
                    break;
                }
            }
        }

        if (aClrMapPropValue.getLength())
        {
            OUString sName;
            sal_Int32 nToken = XML_TOKEN_INVALID;
            for(const auto& item : aClrMapPropValue)
            {
                item.Value >>= nToken;
                switch (nToken)
                {
                    case XML_dk1:      sName = u"dk1"_ustr;      break;
                    case XML_lt1:      sName = u"lt1"_ustr;      break;
                    case XML_dk2:      sName = u"dk2"_ustr;      break;
                    case XML_lt2:      sName = u"lt2"_ustr;      break;
                    case XML_accent1:  sName = u"accent1"_ustr;  break;
                    case XML_accent2:  sName = u"accent2"_ustr;  break;
                    case XML_accent3:  sName = u"accent3"_ustr;  break;
                    case XML_accent4:  sName = u"accent4"_ustr;  break;
                    case XML_accent5:  sName = u"accent5"_ustr;  break;
                    case XML_accent6:  sName = u"accent6"_ustr;  break;
                    case XML_hlink:    sName = u"hlink"_ustr;    break;
                    case XML_folHlink: sName = u"folHlink"_ustr; break;
                }
                aClrMap.push_back(sName);
            }
            assert(aClrMap.size() == 12 && "missing entries for ClrMap");
        }
        else
        {
            // default clrMap to export ".odp" files to ".pptx"
            aClrMap = { u"lt1"_ustr,     u"dk1"_ustr,     u"lt2"_ustr,     u"dk2"_ustr,
                        u"accent1"_ustr, u"accent2"_ustr, u"accent3"_ustr, u"accent4"_ustr,
                        u"accent5"_ustr, u"accent6"_ustr, u"hlink"_ustr,   u"folHlink"_ustr };
        }

        pFS->singleElementNS(XML_p, XML_clrMap,
                             XML_bg1, aClrMap[0],
                             XML_tx1, aClrMap[1],
                             XML_bg2, aClrMap[2],
                             XML_tx2, aClrMap[3],
                             XML_accent1, aClrMap[4],
                             XML_accent2, aClrMap[5],
                             XML_accent3, aClrMap[6],
                             XML_accent4, aClrMap[7],
                             XML_accent5, aClrMap[8],
                             XML_accent6, aClrMap[9],
                             XML_hlink, aClrMap[10],
                             XML_folHlink, aClrMap[11]);
    }

    // use master's id type as they have same range, mso does that as well
    pFS->startElementNS(XML_p, XML_sldLayoutIdLst);

    auto getLayoutsUsedForMaster = [](SdrPage* pMaster) -> std::unordered_set<sal_Int32>
    {
        if (!pMaster)
            return {};

        std::unordered_set<sal_Int32> aUsedLayouts{};
        for (const auto* pPageUser : pMaster->GetPageUsers())
        {
            const auto* pMasterPageDescriptor
                = dynamic_cast<const sdr::MasterPageDescriptor*>(pPageUser);

            if (!pMasterPageDescriptor)
                continue;

            AutoLayout eLayout
                = static_cast<SdPage&>(pMasterPageDescriptor->GetOwnerPage()).GetAutoLayout();
            aUsedLayouts.insert(eLayout);
        }
        return aUsedLayouts;
    };

    std::unordered_set<sal_Int32> aLayouts = getLayoutsUsedForMaster(pMasterPage);

    css::uno::Reference< css::beans::XPropertySet > xPagePropSet;
    xPagePropSet.set(mXDrawPage, UNO_QUERY);
    if (xPagePropSet.is())
    {
        uno::Any aAny;
        if (GetPropertyValue(aAny, xPagePropSet, u"SlideLayout"_ustr))
            aLayouts.insert(aAny.get<sal_Int32>());
    }

    OUString aSlideName;
    Reference< XNamed > xNamed(mXDrawPage, UNO_QUERY);
    if (xNamed.is())
        aSlideName = xNamed->getName();

    for (auto nLayout : aLayouts)
    {
        ImplWritePPTXLayout(nLayout, nPageNum, aSlideName);
        AddLayoutIdAndRelation(pFS, GetLayoutFileId(nLayout, nPageNum));
    }

    pFS->endElementNS(XML_p, XML_sldLayoutIdLst);

    pFS->endElementNS(XML_p, XML_sldMaster);

    SAL_INFO("sd.eppt", "----------------");

    pFS->endDocument();
}

sal_Int32 PowerPointExport::GetLayoutFileId(sal_Int32 nOffset, sal_uInt32 nMasterNum)
{
    SAL_INFO("sd.eppt", "GetLayoutFileId offset: " << nOffset << " master: " << nMasterNum);
    if (mLayoutInfo[ nOffset ].mnFileIdArray.size() <= nMasterNum)
        return 0;

    return mLayoutInfo[ nOffset ].mnFileIdArray[ nMasterNum ];
}

void PowerPointExport::ImplWritePPTXLayout(sal_Int32 nOffset, sal_uInt32 nMasterNum, const OUString& aSlideName)
{
    SAL_INFO("sd.eppt", "write layout: " << nOffset);

    Reference< drawing::XDrawPagesSupplier > xDPS(getModel(), uno::UNO_QUERY);
    Reference< drawing::XDrawPages > xDrawPages = xDPS->getDrawPages();
    Reference< drawing::XDrawPage > xSlide = xDrawPages->insertNewByIndex(xDrawPages->getCount());

#if OSL_DEBUG_LEVEL >= 2
    if (xSlide.is())
        printf("new page created\n");
#endif

    Reference< beans::XPropertySet > xPropSet(xSlide, uno::UNO_QUERY);
    xPropSet->setPropertyValue(u"Layout"_ustr, Any(short(aLayoutInfo[ nOffset ].nType)));
#if OSL_DEBUG_LEVEL > 1
    dump_pset(xPropSet);
#endif
    mXPagePropSet.set(xSlide, UNO_QUERY);
    mXShapes = xSlide;

    if (mLayoutInfo[ nOffset ].mnFileIdArray.size() < mnMasterPages)
    {
        mLayoutInfo[ nOffset ].mnFileIdArray.resize(mnMasterPages);
    }

    if (mLayoutInfo[ nOffset ].mnFileIdArray[ nMasterNum ] != 0)
        return;

    FSHelperPtr pFS
        = openFragmentStreamWithSerializer("ppt/slideLayouts/slideLayout" +
                                            OUString::number(mnLayoutFileIdMax) + ".xml",
                                           u"application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml"_ustr);

    // add implicit relation of slide layout to slide master
    addRelation(pFS->getOutputStream(),
                oox::getRelationship(Relationship::SLIDEMASTER),
                Concat2View("../slideMasters/slideMaster" + OUString::number(nMasterNum + 1) + ".xml"));

    pFS->startElementNS(XML_p, XML_sldLayout,
                        PNMSS,
                        XML_type, aLayoutInfo[ nOffset ].sType,
                        XML_preserve, "1");

    if (!aSlideName.isEmpty())
    {
        pFS->startElementNS(XML_p, XML_cSld,
            XML_name, aSlideName);
    }
    else
    {
        pFS->startElementNS(XML_p, XML_cSld,
            XML_name, aLayoutInfo[nOffset].sName);
    }
    //pFS->write( MINIMAL_SPTREE ); // TODO: write actual shape tree
    WriteShapeTree(pFS, LAYOUT, true);

    pFS->endElementNS(XML_p, XML_cSld);

    pFS->endElementNS(XML_p, XML_sldLayout);

    mLayoutInfo[ nOffset ].mnFileIdArray[ nMasterNum ] = mnLayoutFileIdMax;

    mnLayoutFileIdMax ++;

    xDrawPages->remove(xSlide);

    pFS->endDocument();
}

void PowerPointExport::WriteShapeTree(const FSHelperPtr& pFS, PageType ePageType, bool bMaster)
{
    PowerPointShapeExport aDML(pFS, &maShapeMap, this);
    aDML.SetMaster(bMaster);
    aDML.SetPageType(ePageType);
    aDML.SetBackgroundDark(mbIsBackgroundDark);

    pFS->startElementNS(XML_p, XML_spTree);
    pFS->write(MAIN_GROUP);

    ResetGroupTable(mXShapes->getCount());

    while (GetNextGroupEntry())
    {

        sal_uInt32 nGroups = GetGroupsClosed();
        for (sal_uInt32 i = 0; i < nGroups; i++)
        {
            SAL_INFO("sd.eppt", "leave group");
        }

        if (GetShapeByIndex(GetCurrentGroupIndex(), true))
        {
            SAL_INFO("sd.eppt", "mType: " << mType);
            const SdrObjGroup* pDiagramCandidate(dynamic_cast<const SdrObjGroup*>(SdrObject::getSdrObjectFromXShape(mXShape)));
            const bool bIsDiagram(nullptr != pDiagramCandidate && pDiagramCandidate->isDiagram());

            if (bIsDiagram)
                WriteDiagram(pFS, aDML, mXShape, mnDiagramId++);
            else
                aDML.WriteShape(mXShape);
        }
    }

    if ( ePageType == NORMAL || ePageType == LAYOUT )
        WritePlaceholderReferenceShapes(aDML, ePageType);
    pFS->endElementNS(XML_p, XML_spTree);
}

ShapeExport& PowerPointShapeExport::WritePageShape(const Reference< XShape >& xShape, PageType ePageType, bool bPresObj)
{
    if ((ePageType == NOTICE && bPresObj) || ePageType == LAYOUT || ePageType == MASTER)
        return WritePlaceholderShape(xShape, SlideImage);

    return WriteTextShape(xShape);
}

bool PowerPointShapeExport::WritePlaceholder(const Reference< XShape >& xShape, PlaceholderType ePlaceholder, bool bMaster)
{
    SAL_INFO("sd.eppt", "WritePlaceholder " << bMaster << " " << ShapeExport::NonEmptyText(xShape));
    if (!xShape)
        return false;
    try
    {
        Reference<XPropertySet> xShapeProps(xShape, UNO_QUERY);
        if (xShapeProps->getPropertyValue(u"IsPresentationObject"_ustr).get<bool>())
        {
            WritePlaceholderShape(xShape, ePlaceholder);

            return true;
        }
    }
    catch (Exception&)
    {
        return false;
    }
    return false;
}

ShapeExport& PowerPointShapeExport::WritePlaceholderShape(const Reference< XShape >& xShape, PlaceholderType ePlaceholder)
{
    Reference<XPropertySet> xProps(xShape, UNO_QUERY);
    bool bUseBackground(false);
    if (xProps.is() && xProps->getPropertySetInfo()->hasPropertyByName(u"FillUseSlideBackground"_ustr))
        xProps->getPropertyValue(u"FillUseSlideBackground"_ustr) >>= bUseBackground;

    if (bUseBackground)
        mpFS->startElementNS(XML_p, XML_sp, XML_useBgFill, "1");
    else
        mpFS->startElementNS(XML_p, XML_sp);

    // non visual shape properties
    mpFS->startElementNS(XML_p, XML_nvSpPr);
    const OString aPlaceholderID("PlaceHolder " + OString::number(mnShapeIdMax++));
    WriteNonVisualDrawingProperties(xShape, aPlaceholderID.getStr());
    mpFS->startElementNS(XML_p, XML_cNvSpPr);
    mpFS->singleElementNS(XML_a, XML_spLocks, XML_noGrp, "1");
    mpFS->endElementNS(XML_p, XML_cNvSpPr);
    mpFS->startElementNS(XML_p, XML_nvPr);

    bool bUsePlaceholderIndex
        = ePlaceholder == Footer || ePlaceholder == DateAndTime || ePlaceholder == SlideNumber;
    const char* pType = getPlaceholderTypeName(ePlaceholder);

    SAL_INFO("sd.eppt", "write placeholder " << pType);
    if (bUsePlaceholderIndex)
    {
        mpFS->singleElementNS(
            XML_p, XML_ph, XML_type, pType, XML_idx,
            OString::number(
                static_cast<PowerPointExport*>(GetFB())->CreateNewPlaceholderIndex(xShape)));
    }
    else
    {
        if ((mePageType == PageType::LAYOUT || mePageType == PageType::NORMAL)
            && ePlaceholder == Outliner)
            mpFS->singleElementNS(XML_p, XML_ph);
        else
            mpFS->singleElementNS(XML_p, XML_ph, XML_type, pType);
    }
    mpFS->endElementNS(XML_p, XML_nvPr);
    mpFS->endElementNS(XML_p, XML_nvSpPr);

    // visual shape properties
    mpFS->startElementNS(XML_p, XML_spPr);
    WriteShapeTransformation(xShape, XML_a);
    WritePresetShape("rect"_ostr);
    if (xProps.is())
    {
        WriteBlipFill(xProps, u"Graphic"_ustr);
        // Do not forget to export the visible properties.
        WriteFill( xProps, xShape->getSize());
        WriteOutline( xProps );
        WriteShapeEffects( xProps );

        bool bHas3DEffectinShape = false;
        uno::Sequence<beans::PropertyValue> grabBag;
        if (xProps->getPropertySetInfo()->hasPropertyByName(u"InteropGrabBag"_ustr))
            xProps->getPropertyValue(u"InteropGrabBag"_ustr) >>= grabBag;

        for (auto const& it : grabBag)
            if (it.Name == "3DEffectProperties")
                bHas3DEffectinShape = true;

        if( bHas3DEffectinShape)
            Write3DEffects( xProps, /*bIsText=*/false );
    }
    mpFS->endElementNS(XML_p, XML_spPr);

    WriteTextBox(xShape, XML_p, /*bWritePropertiesAsLstStyles=*/bUsePlaceholderIndex);

    mpFS->endElementNS(XML_p, XML_sp);

    return *this;
}

ShapeExport& PowerPointShapeExport::WritePlaceholderReferenceShape(
    PlaceholderType ePlaceholder, sal_Int32 nReferencedPlaceholderIdx, PageType ePageType,
    const Reference<XPropertySet>& rXPagePropSet)
{
    mpFS->startElementNS(XML_p, XML_sp);

    // non visual shape properties
    mpFS->startElementNS(XML_p, XML_nvSpPr);
    const OString aPlaceholderID("PlaceHolder " + OString::number(mnShapeIdMax++));
    GetFS()->singleElementNS(XML_p, XML_cNvPr, XML_id, OString::number(mnShapeIdMax), XML_name,
                             aPlaceholderID);

    mpFS->startElementNS(XML_p, XML_cNvSpPr);
    mpFS->singleElementNS(XML_a, XML_spLocks, XML_noGrp, "1");
    mpFS->endElementNS(XML_p, XML_cNvSpPr);
    mpFS->startElementNS(XML_p, XML_nvPr);

    const char* pType = getPlaceholderTypeName(ePlaceholder);
    mpFS->singleElementNS(XML_p, XML_ph, XML_type, pType, XML_idx,
                          OString::number(nReferencedPlaceholderIdx));
    mpFS->endElementNS(XML_p, XML_nvPr);
    mpFS->endElementNS(XML_p, XML_nvSpPr);

    // visual shape properties
    mpFS->startElementNS(XML_p, XML_spPr);
    mpFS->endElementNS(XML_p, XML_spPr);

    WritePlaceholderReferenceTextBody(ePlaceholder, ePageType, rXPagePropSet);

    mpFS->endElementNS(XML_p, XML_sp);

    return *this;
}

ShapeExport& PowerPointShapeExport::WritePlaceholderReferenceTextBody(
    PlaceholderType ePlaceholder, PageType ePageType, const Reference<XPropertySet> xPagePropSet)
{
    mpFS->startElementNS(XML_p, XML_txBody);
    mpFS->singleElementNS(XML_a, XML_bodyPr);
    mpFS->startElementNS(XML_a, XML_p);

    switch (ePlaceholder)
    {
        case Header:
            break;
        case Footer:
        {
            OUString aFooterText;
            if (ePageType == LAYOUT)
            {
                aFooterText = "Footer";
            }
            else
            {
                xPagePropSet->getPropertyValue(u"FooterText"_ustr) >>= aFooterText;
            }
            mpFS->startElementNS(XML_a, XML_r);
            mpFS->startElementNS(XML_a, XML_t);
            mpFS->writeEscaped(aFooterText);
            mpFS->endElementNS(XML_a, XML_t);
            mpFS->endElementNS(XML_a, XML_r);
            break;
        }
        case SlideNumber:
        {
            OUString aSlideNum;
            sal_Int32 nSlideNum = 0;
            if (ePageType == LAYOUT)
            {
                aSlideNum = "<#>";
            }
            else
            {
                xPagePropSet->getPropertyValue(u"Number"_ustr) >>= nSlideNum;
                aSlideNum = OUString::number(nSlideNum);
            }
            OString aUUID(comphelper::xml::generateGUIDString());
            mpFS->startElementNS(XML_a, XML_fld, XML_id, aUUID, XML_type, "slidenum");
            mpFS->startElementNS(XML_a, XML_t);
            mpFS->writeEscaped(aSlideNum);
            mpFS->endElementNS(XML_a, XML_t);
            mpFS->endElementNS(XML_a, XML_fld);
            break;
        }
        case DateAndTime:
        {
            OUString aDateTimeType = u"datetime1"_ustr;
            bool bIsDateTimeFixed = false;
            xPagePropSet->getPropertyValue(u"IsDateTimeFixed"_ustr) >>= bIsDateTimeFixed;

            OUString aDateTimeText = u"Date"_ustr;
            const LanguageTag& rLanguageTag = Application::GetSettings().GetLanguageTag();

            if(ePageType != LAYOUT && !bIsDateTimeFixed)
            {
                sal_Int32 nDateTimeFormat = 0;
                xPagePropSet->getPropertyValue(u"DateTimeFormat"_ustr) >>= nDateTimeFormat;

                // 4 LSBs represent the date
                SvxDateFormat eDate = static_cast<SvxDateFormat>(nDateTimeFormat & 0x0f);
                // the 4 bits after the date bits represent the time
                SvxTimeFormat eTime = static_cast<SvxTimeFormat>(nDateTimeFormat >> 4);
                aDateTimeType = GetDatetimeTypeFromDateTime(eDate, eTime);

                if (aDateTimeType == "datetime")
                    aDateTimeType = "datetime1";

                ::DateTime aDateTime( ::DateTime::SYSTEM );

                aDateTimeText = SvxDateTimeField::GetFormatted(
                    aDateTime, aDateTime, eDate,
                    eTime, *(SD_MOD()->GetNumberFormatter()),
                    rLanguageTag.getLanguageType());
            }

            if(!bIsDateTimeFixed)
            {
                OString aUUID(comphelper::xml::generateGUIDString());
                mpFS->startElementNS(XML_a, XML_fld, XML_id, aUUID, XML_type, aDateTimeType);
            }
            else
            {
                xPagePropSet->getPropertyValue(u"DateTimeText"_ustr) >>= aDateTimeText;
                mpFS->startElementNS(XML_a, XML_r);
            }

            mpFS->startElementNS(XML_a, XML_rPr, XML_lang, rLanguageTag.getBcp47MS());
            mpFS->endElementNS(XML_a, XML_rPr);

            mpFS->startElementNS(XML_a, XML_t);
            mpFS->writeEscaped(aDateTimeText);
            mpFS->endElementNS(XML_a, XML_t);

            mpFS->endElementNS(XML_a, bIsDateTimeFixed ? XML_r : XML_fld);
            break;
        }
        default:
            SAL_INFO("sd.eppt", "warning: no defined textbody for referenced placeholder type: "
                                    << ePlaceholder);
    }
    mpFS->endElementNS(XML_a, XML_p);
    mpFS->endElementNS(XML_p, XML_txBody);

    return *this;
}

void PowerPointExport::WriteDefaultColorSchemes(const FSHelperPtr& pFS)
{
    for (int nId = PredefinedClrSchemeId::dk2; nId != PredefinedClrSchemeId::Count; nId++)
    {
        OUString sName(getPredefinedClrNames(static_cast<PredefinedClrSchemeId>(nId)));
        sal_Int32 nColor = 0;

        switch (nId)
        {
        case PredefinedClrSchemeId::dk2:
            nColor = 0x1F497D;
            break;
        case PredefinedClrSchemeId::lt2:
            nColor = 0xEEECE1;
            break;
        case PredefinedClrSchemeId::accent1:
            nColor = 0x4F81BD;
            break;
        case PredefinedClrSchemeId::accent2:
            nColor = 0xC0504D;
            break;
        case PredefinedClrSchemeId::accent3:
            nColor = 0x9BBB59;
            break;
        case PredefinedClrSchemeId::accent4:
            nColor = 0x8064A2;
            break;
        case PredefinedClrSchemeId::accent5:
            nColor = 0x4BACC6;
            break;
        case PredefinedClrSchemeId::accent6:
            nColor = 0xF79646;
            break;
        case PredefinedClrSchemeId::hlink:
            nColor = 0x0000FF;
            break;
        case PredefinedClrSchemeId::folHlink:
            nColor = 0x800080;
            break;
        }

        OUString sOpenColorScheme = "<a:" + sName + ">";
        pFS->write(sOpenColorScheme);

        pFS->singleElementNS(XML_a, XML_srgbClr, XML_val, I32SHEX(nColor));

        OUString sCloseColorScheme = "</a:" + sName + ">";
        pFS->write(sCloseColorScheme);
    }
}

void PowerPointExport::WriteTheme(sal_Int32 nThemeNum, model::Theme* pTheme)
{
    if (!pTheme)
        return;
    OUString sThemePath = "ppt/theme/theme" + OUString::number(nThemeNum + 1) + ".xml";

    oox::ThemeExport aThemeExport(this, DOCUMENT_PPTX);

    aThemeExport.write(sThemePath, *pTheme);
}

bool PowerPointExport::ImplCreateDocument()
{
    mbCreateNotes = false;

    for (sal_uInt32 i = 0; i < mnPages; i++)
    {
        if (!GetPageByIndex(i, NOTICE))
            return false;

        if (ContainsOtherShapeThanPlaceholders())
        {
            mbCreateNotes = true;
            break;
        }
    }

    return true;
}

void PowerPointExport::WriteNotesMaster()
{
    SAL_INFO("sd.eppt", "write Notes master\n---------------");

    mPresentationFS->startElementNS(XML_p, XML_notesMasterIdLst);

    OUString sRelId = addRelation(mPresentationFS->getOutputStream(),
                                  oox::getRelationship(Relationship::NOTESMASTER),
                                  u"notesMasters/notesMaster1.xml");

    mPresentationFS->singleElementNS(XML_p, XML_notesMasterId,
                                     FSNS(XML_r, XML_id), sRelId);

    mPresentationFS->endElementNS(XML_p, XML_notesMasterIdLst);

    FSHelperPtr pFS =
        openFragmentStreamWithSerializer(u"ppt/notesMasters/notesMaster1.xml"_ustr,
                                         u"application/vnd.openxmlformats-officedocument.presentationml.notesMaster+xml"_ustr);
    // write theme per master

    // TODO: Need to implement theme support for note master, so the
    // note master has his own theme associated.

    // For now just use the default theme
    auto const* pDefaultColorSet = svx::ColorSets::get().getColorSet(u"LibreOffice");
    if (pDefaultColorSet)
    {
        auto pTheme = std::make_shared<model::Theme>("Office Theme");
        pTheme->setColorSet(std::make_shared<model::ColorSet>(*pDefaultColorSet));

        WriteTheme(mnMasterPages, pTheme.get());

        // add implicit relation to the presentation theme
        addRelation(pFS->getOutputStream(),
                    oox::getRelationship(Relationship::THEME),
                    Concat2View("../theme/theme" + OUString::number(mnMasterPages + 1) + ".xml"));
    }

    pFS->startElementNS(XML_p, XML_notesMaster, PNMSS);

    pFS->startElementNS(XML_p, XML_cSld);

    Reference< XPropertySet > aXBackgroundPropSet;
    if (ImplGetPropertyValue(mXPagePropSet, u"Background"_ustr) &&
            (mAny >>= aXBackgroundPropSet))
        ImplWriteBackground(pFS, aXBackgroundPropSet);

    WriteShapeTree(pFS, NOTICE, true);

    pFS->endElementNS(XML_p, XML_cSld);

    // color map - now it uses colors from hardcoded theme, once we eventually generate theme, this might need update
    pFS->singleElementNS(XML_p, XML_clrMap,
                         XML_bg1, "lt1",
                         XML_bg2, "lt2",
                         XML_tx1, "dk1",
                         XML_tx2, "dk2",
                         XML_accent1, "accent1",
                         XML_accent2, "accent2",
                         XML_accent3, "accent3",
                         XML_accent4, "accent4",
                         XML_accent5, "accent5",
                         XML_accent6, "accent6",
                         XML_hlink, "hlink",
                         XML_folHlink, "folHlink");

    pFS->endElementNS(XML_p, XML_notesMaster);

    SAL_INFO("sd.eppt", "----------------");

    pFS->endDocument();
}

void PowerPointExport::embedEffectAudio(const FSHelperPtr& pFS, const OUString& sUrl, OUString& sRelId, OUString& sName)
{
    comphelper::LifecycleProxy aProxy;

    if (!sUrl.endsWithIgnoreAsciiCase(".wav"))
        return;

    uno::Reference<io::XInputStream> xAudioStream;
    try
    {
        if (sUrl.startsWith("vnd.sun.star.Package:"))
        {
            uno::Reference<document::XStorageBasedDocument> xStorageBasedDocument(getModel(), uno::UNO_QUERY);
            if (!xStorageBasedDocument.is())
                return;

            uno::Reference<embed::XStorage> xDocumentStorage = xStorageBasedDocument->getDocumentStorage();
            if (!xDocumentStorage.is())
                return;

            uno::Reference<io::XStream> xStream = comphelper::OStorageHelper::GetStreamAtPackageURL(xDocumentStorage, sUrl,
                                                        css::embed::ElementModes::READ, aProxy);

            if (xStream.is())
                xAudioStream = xStream->getInputStream();
        }
        else
            xAudioStream = comphelper::OStorageHelper::GetInputStreamFromURL(sUrl, getComponentContext());
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION("sd", "PowerPointExport::embedEffectAudio");
    }

    if (!xAudioStream.is())
        return;

    int nLastSlash = sUrl.lastIndexOf('/');
    sName = sUrl.copy(nLastSlash >= 0 ? nLastSlash + 1 : 0);

    OUString sPath = "../media/" + sName;
    sRelId = addRelation(pFS->getOutputStream(),
                        oox::getRelationship(Relationship::AUDIO), sPath);

    uno::Reference<io::XOutputStream> xOutputStream = openFragmentStream(sPath.replaceAt(0, 2, u"/ppt"),
            u"audio/x-wav"_ustr);

    comphelper::OStorageHelper::CopyInputToOutput(xAudioStream, xOutputStream);
}

sal_Int32 PowerPointExport::GetShapeID(const Reference<XShape>& rXShape)
{
    return ShapeExport::GetShapeID(rXShape, &maShapeMap);
}

sal_Int32 PowerPointExport::GetNextAnimationNodeID()
{
    return mnAnimationNodeIdMax++;
}

bool PowerPointExport::ImplCreateMainNotes()
{
    if (mbCreateNotes)
        WriteNotesMaster();

    return true;
}

OUString PowerPointExport::getImplementationName()
{
    return u"com.sun.star.comp.Impress.oox.PowerPointExport"_ustr;
}

void PowerPointExport::WriteDiagram(const FSHelperPtr& pFS, PowerPointShapeExport& rDML, const css::uno::Reference<css::drawing::XShape>& rXShape, int nDiagramId)
{
    SAL_INFO("sd.eppt", "writing Diagram " + OUString::number(nDiagramId));
    pFS->startElementNS(XML_p, XML_graphicFrame);
    rDML.WriteDiagram(rXShape, nDiagramId);
    pFS->endElementNS(XML_p, XML_graphicFrame);
}

void PowerPointExport::WritePlaceholderReferenceShapes(PowerPointShapeExport& rDML, PageType ePageType)
{
    bool bCheckProps = ePageType == NORMAL;
    Reference<XShape> xShape;
    Any aAny;
    OUString aText;
    if (ePageType == LAYOUT
        || (bCheckProps && PropValue::GetPropertyValue(aAny, mXPagePropSet, u"IsFooterVisible"_ustr, true)
            && aAny == true && GetPropertyValue(aAny, mXPagePropSet, u"FooterText"_ustr, true)
            && (aAny >>= aText) && !aText.isEmpty()))
    {
        if ((xShape = GetReferencedPlaceholderXShape(Footer, ePageType)))
        {
            const auto iter = maPlaceholderShapeToIndexMap.find(xShape);
            assert(iter != maPlaceholderShapeToIndexMap.end());
            rDML.WritePlaceholderReferenceShape(Footer,
                                                iter->second,
                                                ePageType, mXPagePropSet);
        }
    }

    if (ePageType == LAYOUT
        || (bCheckProps
            && PropValue::GetPropertyValue(aAny, mXPagePropSet, u"IsPageNumberVisible"_ustr, true)
            && aAny == true))
    {
        if ((xShape = GetReferencedPlaceholderXShape(SlideNumber, ePageType)))
        {
            const auto iter = maPlaceholderShapeToIndexMap.find(xShape);
            assert(iter != maPlaceholderShapeToIndexMap.end());
            rDML.WritePlaceholderReferenceShape(SlideNumber,
                                                iter->second,
                                                ePageType, mXPagePropSet);
        }
    }

    if (ePageType == LAYOUT
        || (bCheckProps
            && PropValue::GetPropertyValue(aAny, mXPagePropSet, u"IsDateTimeVisible"_ustr, true)
            && aAny == true
            && ((GetPropertyValue(aAny, mXPagePropSet, u"DateTimeText"_ustr, true) && (aAny >>= aText)
                 && !aText.isEmpty())
                || mXPagePropSet->getPropertyValue(u"IsDateTimeFixed"_ustr) == false)))
    {
        if ((xShape = GetReferencedPlaceholderXShape(DateAndTime, ePageType)))
        {
            const auto iter = maPlaceholderShapeToIndexMap.find(xShape);
            assert(iter != maPlaceholderShapeToIndexMap.end());
            rDML.WritePlaceholderReferenceShape(DateAndTime,
                                                iter->second,
                                                ePageType, mXPagePropSet);
        }
    }
}

sal_Int32 PowerPointExport::CreateNewPlaceholderIndex(const css::uno::Reference<XShape> &rXShape)
{
    maPlaceholderShapeToIndexMap.insert({rXShape, mnPlaceholderIndexMax});
    return mnPlaceholderIndexMax++;
}

Reference<XShape> PowerPointExport::GetReferencedPlaceholderXShape(const PlaceholderType eType,
                                                        PageType ePageType) const
{
    PresObjKind ePresObjKind = PresObjKind::NONE;
    switch (eType)
    {
        case oox::core::None:
            break;
        case oox::core::SlideImage:
            break;
        case oox::core::Notes:
            break;
        case oox::core::Header:
            ePresObjKind = PresObjKind::Header;
            break;
        case oox::core::Footer:
            ePresObjKind = PresObjKind::Footer;
            break;
        case oox::core::SlideNumber:
            ePresObjKind = PresObjKind::SlideNumber;
            break;
        case oox::core::DateAndTime:
            ePresObjKind = PresObjKind::DateTime;
            break;
        case oox::core::Outliner:
            break;
        case oox::core::Title:
            ePresObjKind = PresObjKind::Title;
            break;
        case oox::core::Subtitle:
            break;
    }
    if (ePresObjKind != PresObjKind::NONE)
    {
        SdPage* pMasterPage;
        if (ePageType == LAYOUT)
        {
            // since Layout pages do not have drawpages themselves - mXDrawPage is still the master they reference to..
            pMasterPage = SdPage::getImplementation(mXDrawPage);
        }
        else
        {
            pMasterPage = &static_cast<SdPage&>(SdPage::getImplementation(mXDrawPage)->TRG_GetMasterPage());
        }
        if (SdrObject* pMasterFooter = pMasterPage->GetPresObj(ePresObjKind))
            return GetXShapeForSdrObject(pMasterFooter);
    }
    return nullptr;
}

// UNO component
extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
css_comp_Impress_oox_PowerPointExport(uno::XComponentContext* rxCtxt,
                                      uno::Sequence<css::uno::Any> const& rArguments)
{
    return cppu::acquire(new PowerPointExport(rxCtxt, rArguments));
}

#if OSL_DEBUG_LEVEL > 1
void dump_pset(Reference< XPropertySet > const& rXPropSet)
{
    Reference< XPropertySetInfo > info = rXPropSet->getPropertySetInfo();
    Sequence< beans::Property > props = info->getProperties();

    for (int i=0; i < props.getLength(); i++)
    {
        OString name = OUStringToOString(props [i].Name, RTL_TEXTENCODING_UTF8);

        Any value = rXPropSet->getPropertyValue(props [i].Name);

        OUString strValue;
        sal_Int32 intValue;
        bool boolValue;
        RectanglePoint pointValue;

        if (value >>= strValue)
            SAL_INFO("sd.eppt", name << " = \"" << strValue << "\"");
        else if (value >>= intValue)
            SAL_INFO("sd.eppt", name << " = " << intValue << "(hex : " << std::hex << intValue << ")");
        else if (value >>= boolValue)
            SAL_INFO("sd.eppt", name << " = " << boolValue << "           (bool)");
        else if (value >>= pointValue)
            SAL_INFO("sd.eppt", name << " = " << static_cast<int>(pointValue) << "    (RectanglePoint)");
        else
            SAL_INFO("sd.eppt", "???          <unhandled type>");
    }
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
