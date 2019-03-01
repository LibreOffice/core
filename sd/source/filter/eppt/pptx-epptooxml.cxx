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
#include <sax/fshelper.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
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

#include <oox/export/utils.hxx>

#include "pptx-animations.hxx"
#include "../ppt/pptanimations.hxx"

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <utility>
#if OSL_DEBUG_LEVEL > 1
#include <com/sun/star/drawing/RectanglePoint.hpp>
#endif

// presentation namespaces
#define PNMSS         FSNS(XML_xmlns, XML_a),   OUStringToOString(this->getNamespaceURL(OOX_NS(dml)), RTL_TEXTENCODING_UTF8).getStr(), \
                      FSNS(XML_xmlns, XML_p),   OUStringToOString(this->getNamespaceURL(OOX_NS(ppt)), RTL_TEXTENCODING_UTF8).getStr(), \
                      FSNS(XML_xmlns, XML_r),   OUStringToOString(this->getNamespaceURL(OOX_NS(officeRel)), RTL_TEXTENCODING_UTF8).getStr(), \
                      FSNS(XML_xmlns, XML_p14), OUStringToOString(this->getNamespaceURL(OOX_NS(p14)), RTL_TEXTENCODING_UTF8).getStr(), \
                      FSNS(XML_xmlns, XML_p15), OUStringToOString(this->getNamespaceURL(OOX_NS(p15)), RTL_TEXTENCODING_UTF8).getStr(), \
                      FSNS(XML_xmlns, XML_mc),  OUStringToOString(this->getNamespaceURL(OOX_NS(mce)), RTL_TEXTENCODING_UTF8).getStr()


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
using ::com::sun::star::container::XIndexAccess;
using ::sax_fastparser::FSHelperPtr;
using namespace oox::drawingml;
using namespace oox::core;

#if OSL_DEBUG_LEVEL > 1
void dump_pset(Reference< XPropertySet > const& rXPropSet);
#endif

namespace oox
{
namespace core
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
    ShapeExport&        WritePageShape(const Reference< XShape >& xShape, PageType ePageType, bool bPresObj);

    // helper parts
    bool WritePlaceholder(const Reference< XShape >& xShape, PlaceholderType ePlaceholder, bool bMaster);
};


namespace
{
void WriteSndAc(const FSHelperPtr& pFS, const OUString& sSoundRelId, const OUString& sSoundName)
{
        pFS->startElementNS(XML_p, XML_sndAc, FSEND);
        pFS->startElementNS(XML_p, XML_stSnd, FSEND);
        pFS->singleElementNS(XML_p, XML_snd,
                FSNS(XML_r, XML_embed), sSoundRelId.isEmpty() ? nullptr : USS(sSoundRelId),
                XML_name, sSoundName.isEmpty() ? nullptr : USS(sSoundName), FSEND);
        pFS->endElement(FSNS(XML_p, XML_stSnd));
        pFS->endElement(FSNS(XML_p, XML_sndAc));
}
}

}
}

enum PPTXLayout
{
    LAYOUT_BLANK,
    LAYOUT_TITLE_SLIDE,
    LAYOUT_TITLE_CONTENT,
    LAYOUT_TITLE_2CONTENT,
    LAYOUT_TITLE,
    LAYOUT_CENTERED_TEXT,
    LAYOUT_TITLE_2CONTENT_CONTENT,
    LAYOUT_TITLE_CONTENT_2CONTENT,
    LAYOUT_TITLE_2CONTENT_OVER_CONTENT,
    LAYOUT_TITLE_CONTENT_OVER_CONTENT,
    LAYOUT_TITLE_4CONTENT,
    LAYOUT_TITLE_6CONTENT,
    LAYOUT_SIZE
};

struct PPTXLayoutInfo
{
    int const nType;
    const char* sName;
    const char* sType;
};

static const PPTXLayoutInfo aLayoutInfo[LAYOUT_SIZE] =
{
    { 20, "Blank Slide", "blank" },
    { 0, "Title Slide", "tx" },
    { 1, "Title, Content", "obj" },
    { 3, "Title, 2 Content", "twoObj" },
    { 19, "Title Only", "titleOnly" },
    { 32, "Centered Text", "objOnly" },                       // not exactly, but close
    { 15, "Title, 2 Content and Content", "twoObjAndObj" },
    { 12, "Title Content and 2 Content", "objAndTwoObj" },
    { 16, "Title, 2 Content over Content", "twoObjOverTx" },      // not exactly, but close
    { 14, "Title, Content over Content", "objOverTx" },           // not exactly, but close
    { 18, "Title, 4 Content", "fourObj" },
    { 34, "Title, 6 Content", "blank" }                           // not defined => blank
};

int PowerPointExport::GetPPTXLayoutId(int nOffset)
{
    int nId = LAYOUT_BLANK;

    SAL_INFO("sd.eppt", "GetPPTXLayoutId " << nOffset);

    switch (nOffset)
    {
    case 0:
        nId = LAYOUT_TITLE_SLIDE;
        break;
    case 1:
        nId = LAYOUT_TITLE_CONTENT;
        break;
    case 3:
        nId = LAYOUT_TITLE_2CONTENT;
        break;
    case 19:
        nId = LAYOUT_TITLE;
        break;
    case 15:
        nId = LAYOUT_TITLE_2CONTENT_CONTENT;
        break;
    case 12:
        nId = LAYOUT_TITLE_CONTENT_2CONTENT;
        break;
    case 16:
        nId = LAYOUT_TITLE_2CONTENT_OVER_CONTENT;
        break;
    case 14:
        nId = LAYOUT_TITLE_CONTENT_OVER_CONTENT;
        break;
    case 18:
        nId = LAYOUT_TITLE_4CONTENT;
        break;
    case 32:
        nId = LAYOUT_CENTERED_TEXT;
        break;
    case 34:
        nId = LAYOUT_TITLE_6CONTENT;
        break;
    case 20:
    default:
        nId = LAYOUT_BLANK;
        break;
    }

    return nId;
}

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
    GetFS()->singleElementNS(XML_p, XML_nvPr, FSEND);

    return *this;
}

ShapeExport& PowerPointShapeExport::WriteTextShape(const Reference< XShape >& xShape)
{
    OUString sShapeType = xShape->getShapeType();

    SAL_INFO("sd.eppt", "shape(text) : " << USS(sShapeType));

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

    SAL_INFO("sd.eppt", "shape(unknown): " << USS(sShapeType));

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
        SAL_WARN("sd.eppt", "unknown shape not handled: " << USS(sShapeType));

    return *this;
}

PowerPointExport::PowerPointExport(const Reference< XComponentContext >& rContext, const uno::Sequence<uno::Any>& rArguments)
    : XmlFilterBase(rContext)
    , PPTWriterBase()
    , mnLayoutFileIdMax(1)
    , mnSlideIdMax(1 << 8)
    , mnSlideMasterIdMax(1U << 31)
    , mnAnimationNodeIdMax(1)
    , mbCreateNotes(false)
{
    comphelper::SequenceAsHashMap aArgumentsMap(rArguments);
    mbPptm = aArgumentsMap.getUnpackedValueOrDefault("IsPPTM", false);
    mbExportTemplate = aArgumentsMap.getUnpackedValueOrDefault("IsTemplate", false);
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
        uno::Reference< beans::XPropertySet > xSettings(xFactory->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY);
        try
        {
            xSettings->getPropertyValue("LoadReadonly") >>= bSecurityOptOpenReadOnly;
        }
        catch( Exception& )
        {
        }
        exportDocumentProperties(xDocProps, bSecurityOptOpenReadOnly);
    }

    exportCustomFragments();
}

bool PowerPointExport::importDocument() throw()
{
    return false;
}

bool PowerPointExport::exportDocument()
{
    DrawingML::ResetCounters();
    maShapeMap.clear();

    mXModel.set(getModel(), UNO_QUERY);

    //write document properties
    writeDocumentProperties();

    addRelation(oox::getRelationship(Relationship::OFFICEDOCUMENT), "ppt/presentation.xml");

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

    mPresentationFS = openFragmentStreamWithSerializer("ppt/presentation.xml", aMediaType);

    addRelation(mPresentationFS->getOutputStream(),
                oox::getRelationship(Relationship::THEME),
                "theme/theme1.xml");

    mPresentationFS->startElementNS(XML_p, XML_presentation, PNMSS, FSEND);

    mXStatusIndicator.set(getStatusIndicator(), UNO_QUERY);

    std::vector< PropertyValue > aProperties;
    PropertyValue aProperty;
    aProperty.Name = "BaseURI";
    aProperty.Value <<= getFileUrl();
    aProperties.push_back(aProperty);

    exportPPT(aProperties);

    mPresentationFS->singleElementNS(XML_p, XML_sldSz,
                                     XML_cx, IS(PPTtoEMU(maDestPageSize.Width)),
                                     XML_cy, IS(PPTtoEMU(maDestPageSize.Height)),
                                     FSEND);
    // for some reason if added before slides list it will not load the slides (alas with error reports) in mso
    mPresentationFS->singleElementNS(XML_p, XML_notesSz,
                                     XML_cx, IS(PPTtoEMU(maNotesPageSize.Width)),
                                     XML_cy, IS(PPTtoEMU(maNotesPageSize.Height)),
                                     FSEND);

    WriteAuthors();

    WriteVBA();

    mPresentationFS->endElementNS(XML_p, XML_presentation);
    mPresentationFS.reset();
    // Free all FSHelperPtr, to flush data before committing storage
    mpSlidesFSArray.clear();

    commitStorage();

    maShapeMap.clear();
    maAuthors.clear();

    return true;
}

::oox::ole::VbaProject* PowerPointExport::implCreateVbaProject() const
{
    return new ::oox::ole::VbaProject(getComponentContext(), getModel(), "Impress");
}

void PowerPointExport::ImplWriteBackground(const FSHelperPtr& pFS, const Reference< XPropertySet >& rXPropSet)
{
    FillStyle aFillStyle(FillStyle_NONE);
    if (ImplGetPropertyValue(rXPropSet, "FillStyle"))
        mAny >>= aFillStyle;

    if (aFillStyle == FillStyle_NONE ||
            aFillStyle == FillStyle_GRADIENT ||
            aFillStyle == FillStyle_HATCH)
        return;

    pFS->startElementNS(XML_p, XML_bg, FSEND);
    pFS->startElementNS(XML_p, XML_bgPr, FSEND);

    PowerPointShapeExport aDML(pFS, &maShapeMap, this);
    aDML.SetBackgroundDark(mbIsBackgroundDark);
    aDML.WriteFill(rXPropSet);

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
    if (ImplGetPropertyValue(mXPagePropSet, "Effect"))
        mAny >>= eFadeEffect;

    sal_Int16 nTransitionType = 0, nTransitionSubtype = 0;
    sal_Int8 nPPTTransitionType = 0;
    sal_uInt8 nDirection = 0;

    OUString sSoundUrl;
    OUString sSoundRelId;
    OUString sSoundName;

    if (ImplGetPropertyValue(mXPagePropSet, "TransitionType") && (mAny >>= nTransitionType) &&
            ImplGetPropertyValue(mXPagePropSet, "TransitionSubtype") && (mAny >>= nTransitionSubtype))
        nPPTTransitionType = GetTransition(nTransitionType, nTransitionSubtype, eFadeEffect, nDirection);

    if (!nPPTTransitionType && eFadeEffect != FadeEffect_NONE)
        nPPTTransitionType = GetTransition(eFadeEffect, nDirection);

    if (ImplGetPropertyValue(mXPagePropSet, "Sound") && (mAny >>= sSoundUrl))
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
    if (ImplGetPropertyValue(mXPagePropSet, "TransitionDuration"))
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
    else if (ImplGetPropertyValue(mXPagePropSet, "Speed"))
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

    if (ImplGetPropertyValue(mXPagePropSet, "Change"))
        mAny >>= changeType;

    // 1 means automatic, 2 half automatic - not sure what it means - at least I don't see it in UI
    if (changeType == 1 && ImplGetPropertyValue(mXPagePropSet, "Duration"))
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

        pFS->startElement(FSNS(XML_mc, XML_AlternateContent), FSEND);
        pFS->startElement(FSNS(XML_mc, XML_Choice), XML_Requires, pRequiresNS, FSEND);

        if(isTransitionDurationSet && isAdvanceTimingSet)
        {
            pFS->startElementNS(XML_p, XML_transition,
                XML_spd, speed,
                XML_advTm, I32S(advanceTiming * 1000),
                FSNS(XML_p14, XML_dur), I32S(nTransitionDuration),
                FSEND);
        }
        else if(isTransitionDurationSet)
        {
            pFS->startElementNS(XML_p, XML_transition,
                XML_spd, speed,
                FSNS(XML_p14, XML_dur), I32S(nTransitionDuration),
                FSEND);
        }
        else if(isAdvanceTimingSet)
        {
            pFS->startElementNS(XML_p, XML_transition,
                XML_spd, speed,
                XML_advTm, I32S(advanceTiming * 1000),
                FSEND);
        }
        else
        {
            pFS->startElementNS(XML_p, XML_transition,
                XML_spd, speed,
                FSEND);
        }

        if (nTransition14)
        {
            pFS->singleElementNS(XML_p14, nTransition14,
                XML_isInverted, pInverted,
                XML_dir, pDirection14,
                XML_pattern, pPattern,
                FSEND);
        }
        else if (pPresetTransition)
        {
            pFS->singleElementNS(XML_p15, XML_prstTrans,
                XML_prst, pPresetTransition,
                FSEND);
        }
        else if (isTransitionDurationSet && nTransition)
        {
            pFS->singleElementNS(XML_p, nTransition,
                XML_dir, pDirection,
                XML_orient, pOrientation,
                XML_spokes, pSpokes,
                XML_thruBlk, pThruBlk,
                FSEND);
        }

        if (!sSoundRelId.isEmpty())
            WriteSndAc(pFS, sSoundRelId, sSoundName);

        pFS->endElement(FSNS(XML_p, XML_transition));

        pFS->endElement(FSNS(XML_mc, XML_Choice));
        pFS->startElement(FSNS(XML_mc, XML_Fallback), FSEND);
    }

    pFS->startElementNS(XML_p, XML_transition,
        XML_spd, speed,
        XML_advTm, isAdvanceTimingSet ? I32S(advanceTiming * 1000) : nullptr,
        FSEND);

    if (nTransition)
    {
        pFS->singleElementNS(XML_p, nTransition,
                             XML_dir, pDirection,
                             XML_orient, pOrientation,
                             XML_spokes, pSpokes,
                             XML_thruBlk, pThruBlk,
                             FSEND);
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

static OUString lcl_GetInitials(const OUString& sName)
{
    OUStringBuffer sRet;

    if (!sName.isEmpty())
    {
        sRet.append(sName[0]);
        sal_Int32 nStart = 0, nOffset;

        while ((nOffset = sName.indexOf(' ', nStart)) != -1)
        {
            if (nOffset + 1 < sName.getLength())
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

    FSHelperPtr pFS = openFragmentStreamWithSerializer("ppt/commentAuthors.xml",
                      "application/vnd.openxmlformats-officedocument.presentationml.commentAuthors+xml");
    addRelation(mPresentationFS->getOutputStream(),
                oox::getRelationship(Relationship::COMMENTAUTHORS),
                "commentAuthors.xml");

    pFS->startElementNS(XML_p, XML_cmAuthorLst,
                        FSNS(XML_xmlns, XML_p), OUStringToOString(this->getNamespaceURL(OOX_NS(ppt)), RTL_TEXTENCODING_UTF8),
                        FSEND);

    for (const AuthorsMap::value_type& i : maAuthors)
    {
        pFS->singleElementNS(XML_p, XML_cmAuthor,
                             XML_id, I32S(i.second.nId),
                             XML_name, USS(i.first),
                             XML_initials, USS(lcl_GetInitials(i.first)),
                             XML_lastIdx, I32S(i.second.nLastIndex),
                             XML_clrIdx, I32S(i.second.nId),
                             FSEND);
    }

    pFS->endElementNS(XML_p, XML_cmAuthorLst);
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

bool PowerPointExport::WriteComments(sal_uInt32 nPageNum)
{
    Reference< XAnnotationAccess > xAnnotationAccess(mXDrawPage, uno::UNO_QUERY);
    if (xAnnotationAccess.is())
    {
        Reference< XAnnotationEnumeration > xAnnotationEnumeration(xAnnotationAccess->createAnnotationEnumeration());

        if (xAnnotationEnumeration->hasMoreElements())
        {
            FSHelperPtr pFS = openFragmentStreamWithSerializer(OUStringBuffer()
                              .append("ppt/comments/comment")
                              .append(static_cast<sal_Int32>(nPageNum) + 1)
                              .append(".xml")
                              .makeStringAndClear(),
                              "application/vnd.openxmlformats-officedocument.presentationml.comments+xml");

            pFS->startElementNS(XML_p, XML_cmLst,
                                FSNS(XML_xmlns, XML_p), OUStringToOString(this->getNamespaceURL(OOX_NS(ppt)), RTL_TEXTENCODING_UTF8),
                                FSEND);

            do
            {
                Reference< XAnnotation > xAnnotation(xAnnotationEnumeration->nextElement());
                DateTime aDateTime(xAnnotation->getDateTime());
                RealPoint2D aRealPoint2D(xAnnotation->getPosition());
                Reference< XText > xText(xAnnotation->getTextRange());
                sal_Int32 nLastIndex;
                sal_Int32 nId = GetAuthorIdAndLastIndex(xAnnotation->getAuthor(), nLastIndex);
                char cDateTime[sizeof("-32768-65535-65535T65535:65535:65535.4294967295")];
                    // reserve enough space for hypothetical max length

                snprintf(cDateTime, sizeof cDateTime, "%02" SAL_PRIdINT32 "-%02" SAL_PRIuUINT32 "-%02" SAL_PRIuUINT32 "T%02" SAL_PRIuUINT32 ":%02" SAL_PRIuUINT32 ":%02" SAL_PRIuUINT32 ".%09" SAL_PRIuUINT32, sal_Int32(aDateTime.Year), sal_uInt32(aDateTime.Month), sal_uInt32(aDateTime.Day), sal_uInt32(aDateTime.Hours), sal_uInt32(aDateTime.Minutes), sal_uInt32(aDateTime.Seconds), aDateTime.NanoSeconds);

                pFS->startElementNS(XML_p, XML_cm,
                                    XML_authorId, I32S(nId),
                                    XML_dt, cDateTime,
                                    XML_idx, I32S(nLastIndex),
                                    FSEND);

                pFS->singleElementNS(XML_p, XML_pos,
                                     XML_x, I64S(static_cast<sal_Int64>((57600*aRealPoint2D.X + 1270)/2540.0)),
                                     XML_y, I64S(static_cast<sal_Int64>((57600*aRealPoint2D.Y + 1270)/2540.0)),
                                     FSEND);

                pFS->startElementNS(XML_p, XML_text,
                                    FSEND);
                pFS->write(xText->getString());
                pFS->endElementNS(XML_p, XML_text);

                pFS->endElementNS(XML_p, XML_cm);

            }
            while (xAnnotationEnumeration->hasMoreElements());

            pFS->endElementNS(XML_p, XML_cmLst);

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

    uno::Reference<embed::XStorage> xDocumentStorage(xStorageBasedDocument->getDocumentStorage(), uno::UNO_QUERY);
    OUString aMacrosName("_MS_VBA_Macros");
    if (!xDocumentStorage.is() || !xDocumentStorage->hasByName(aMacrosName))
        return;

    const sal_Int32 nOpenMode = embed::ElementModes::READ;
    uno::Reference<io::XInputStream> xMacrosStream(xDocumentStorage->openStreamElement(aMacrosName, nOpenMode), uno::UNO_QUERY);
    if (!xMacrosStream.is())
        return;

    uno::Reference<io::XOutputStream> xOutputStream = openFragmentStream("ppt/vbaProject.bin", "application/vnd.ms-office.vbaProject");
    comphelper::OStorageHelper::CopyInputToOutput(xMacrosStream, xOutputStream);

    // Write the relationship.
    addRelation(mPresentationFS->getOutputStream(), oox::getRelationship(Relationship::VBAPROJECT), "vbaProject.bin");
}

void PowerPointExport::ImplWriteSlide(sal_uInt32 nPageNum, sal_uInt32 nMasterNum, sal_uInt16 /* nMode */,
                                      bool bHasBackground, Reference< XPropertySet > const& aXBackgroundPropSet)
{
    SAL_INFO("sd.eppt", "write slide: " << nPageNum << "\n----------------");

    // slides list
    if (nPageNum == 0)
        mPresentationFS->startElementNS(XML_p, XML_sldIdLst, FSEND);

    // add explicit relation of presentation to this slide
    OUString sRelId = addRelation(mPresentationFS->getOutputStream(),
                                  oox::getRelationship(Relationship::SLIDE),
                                  OUStringBuffer()
                                  .append("slides/slide")
                                  .append(static_cast<sal_Int32>(nPageNum) + 1)
                                  .append(".xml")
                                  .makeStringAndClear());

    mPresentationFS->singleElementNS(XML_p, XML_sldId,
                                     XML_id, I32S(GetNewSlideId()),
                                     FSNS(XML_r, XML_id), USS(sRelId),
                                     FSEND);

    if (nPageNum == mnPages - 1)
        mPresentationFS->endElementNS(XML_p, XML_sldIdLst);

    FSHelperPtr pFS = openFragmentStreamWithSerializer(OUStringBuffer()
                      .append("ppt/slides/slide")
                      .append(static_cast<sal_Int32>(nPageNum) + 1)
                      .append(".xml")
                      .makeStringAndClear(),
                      "application/vnd.openxmlformats-officedocument.presentationml.slide+xml");

    if (mpSlidesFSArray.size() < mnPages)
        mpSlidesFSArray.resize(mnPages);
    mpSlidesFSArray[ nPageNum ] = pFS;

    const char* pShow = nullptr;

    if (ImplGetPropertyValue(mXPagePropSet, "Visible"))
    {
        bool bShow(false);
        if ((mAny >>= bShow) && !bShow)
            pShow = "0";
    }

    pFS->startElementNS(XML_p, XML_sld, PNMSS,
                        XML_show, pShow,
                        FSEND);

    pFS->startElementNS(XML_p, XML_cSld, FSEND);

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
                OUStringBuffer()
                .append("../slideLayouts/slideLayout")
                .append(GetLayoutFileId(GetPPTXLayoutId(GetLayoutOffset(mXPagePropSet)), nMasterNum))
                .append(".xml")
                .makeStringAndClear());

    if (WriteComments(nPageNum))
        // add implicit relation to slide comments
        addRelation(pFS->getOutputStream(),
                    oox::getRelationship(Relationship::COMMENTS),
                    OUStringBuffer()
                    .append("../comments/comment")
                    .append(static_cast<sal_Int32>(nPageNum) + 1)
                    .append(".xml")
                    .makeStringAndClear());

    SAL_INFO("sd.eppt", "----------------");
}

void PowerPointExport::ImplWriteNotes(sal_uInt32 nPageNum)
{
    if (!mbCreateNotes || !ContainsOtherShapeThanPlaceholders())
        return;

    SAL_INFO("sd.eppt", "write Notes " << nPageNum << "\n----------------");

    FSHelperPtr pFS = openFragmentStreamWithSerializer(OUStringBuffer()
                      .append("ppt/notesSlides/notesSlide")
                      .append(static_cast<sal_Int32>(nPageNum) + 1)
                      .append(".xml")
                      .makeStringAndClear(),
                      "application/vnd.openxmlformats-officedocument.presentationml.notesSlide+xml");

    pFS->startElementNS(XML_p, XML_notes, PNMSS, FSEND);

    pFS->startElementNS(XML_p, XML_cSld, FSEND);

    WriteShapeTree(pFS, NOTICE, false);

    pFS->endElementNS(XML_p, XML_cSld);

    pFS->endElementNS(XML_p, XML_notes);

    // add implicit relation to slide
    addRelation(pFS->getOutputStream(),
                oox::getRelationship(Relationship::SLIDE),
                OUStringBuffer()
                .append("../slides/slide")
                .append(static_cast<sal_Int32>(nPageNum) + 1)
                .append(".xml")
                .makeStringAndClear());

    // add slide implicit relation to notes
    if (nPageNum < mpSlidesFSArray.size())
        addRelation(mpSlidesFSArray[ nPageNum ]->getOutputStream(),
                    oox::getRelationship(Relationship::NOTESSLIDE),
                    OUStringBuffer()
                    .append("../notesSlides/notesSlide")
                    .append(static_cast<sal_Int32>(nPageNum) + 1)
                    .append(".xml")
                    .makeStringAndClear());

    // add implicit relation to notes master
    addRelation(pFS->getOutputStream(),
                oox::getRelationship(Relationship::NOTESMASTER),
                "../notesMasters/notesMaster1.xml");

    SAL_INFO("sd.eppt", "-----------------");
}

void PowerPointExport::AddLayoutIdAndRelation(const FSHelperPtr& pFS, sal_Int32 nLayoutFileId)
{
    // add implicit relation of slide master to slide layout
    OUString sRelId = addRelation(pFS->getOutputStream(),
                                  oox::getRelationship(Relationship::SLIDELAYOUT),
                                  OUStringBuffer()
                                  .append("../slideLayouts/slideLayout")
                                  .append(nLayoutFileId)
                                  .append(".xml")
                                  .makeStringAndClear());

    pFS->singleElementNS(XML_p, XML_sldLayoutId,
                         XML_id, I64S(GetNewSlideMasterId()),
                         FSNS(XML_r, XML_id), USS(sRelId),
                         FSEND);
}

void PowerPointExport::ImplWriteSlideMaster(sal_uInt32 nPageNum, Reference< XPropertySet > const& aXBackgroundPropSet)
{
    SAL_INFO("sd.eppt", "write master slide: " << nPageNum << "\n--------------");

    // slides list
    if (nPageNum == 0)
        mPresentationFS->startElementNS(XML_p, XML_sldMasterIdLst, FSEND);

    OUString sRelId = addRelation(mPresentationFS->getOutputStream(),
                                  oox::getRelationship(Relationship::SLIDEMASTER),
                                  OUStringBuffer()
                                  .append("slideMasters/slideMaster")
                                  .append(static_cast<sal_Int32>(nPageNum) + 1)
                                  .append(".xml")
                                  .makeStringAndClear());

    mPresentationFS->singleElementNS(XML_p, XML_sldMasterId,
                                     XML_id, OString::number(GetNewSlideMasterId()).getStr(),
                                     FSNS(XML_r, XML_id), USS(sRelId),
                                     FSEND);

    if (nPageNum == mnMasterPages - 1)
        mPresentationFS->endElementNS(XML_p, XML_sldMasterIdLst);

    FSHelperPtr pFS =
        openFragmentStreamWithSerializer(OUStringBuffer()
                                         .append("ppt/slideMasters/slideMaster")
                                         .append(static_cast<sal_Int32>(nPageNum) + 1)
                                         .append(".xml")
                                         .makeStringAndClear(),
                                         "application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml");

    // write theme per master
    WriteTheme(nPageNum);

    // add implicit relation to the presentation theme
    addRelation(pFS->getOutputStream(),
                oox::getRelationship(Relationship::THEME),
                OUStringBuffer()
                .append("../theme/theme")
                .append(static_cast<sal_Int32>(nPageNum) + 1)
                .append(".xml")
                .makeStringAndClear());

    pFS->startElementNS(XML_p, XML_sldMaster, PNMSS, FSEND);

    pFS->startElementNS(XML_p, XML_cSld, FSEND);

    ImplWriteBackground(pFS, aXBackgroundPropSet);
    WriteShapeTree(pFS, MASTER, true);

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
                         XML_folHlink, "folHlink",
                         FSEND);

    // use master's id type as they have same range, mso does that as well
    pFS->startElementNS(XML_p, XML_sldLayoutIdLst, FSEND);

    for (int i = 0; i < LAYOUT_SIZE; i++)
    {
        sal_Int32 nLayoutFileId = GetLayoutFileId(i, nPageNum);
        if (nLayoutFileId > 0)
        {
            AddLayoutIdAndRelation(pFS, nLayoutFileId);
        }
        else
        {
            ImplWritePPTXLayout(i, nPageNum);
            AddLayoutIdAndRelation(pFS, GetLayoutFileId(i, nPageNum));
        }
    }

    pFS->endElementNS(XML_p, XML_sldLayoutIdLst);

    pFS->endElementNS(XML_p, XML_sldMaster);

    SAL_INFO("sd.eppt", "----------------");
}

sal_Int32 PowerPointExport::GetLayoutFileId(sal_Int32 nOffset, sal_uInt32 nMasterNum)
{
    SAL_INFO("sd.eppt", "GetLayoutFileId offset: " << nOffset << " master: " << nMasterNum);
    if (mLayoutInfo[ nOffset ].mnFileIdArray.size() <= nMasterNum)
        return 0;

    return mLayoutInfo[ nOffset ].mnFileIdArray[ nMasterNum ];
}

void PowerPointExport::ImplWritePPTXLayout(sal_Int32 nOffset, sal_uInt32 nMasterNum)
{
    SAL_INFO("sd.eppt", "write layout: " << nOffset);

    Reference< drawing::XDrawPagesSupplier > xDPS(getModel(), uno::UNO_QUERY);
    Reference< drawing::XDrawPages > xDrawPages(xDPS->getDrawPages(), uno::UNO_QUERY);
    Reference< drawing::XDrawPage > xSlide;
    Reference< container::XIndexAccess > xIndexAccess(xDrawPages, uno::UNO_QUERY);

    xSlide = xDrawPages->insertNewByIndex(xIndexAccess->getCount());

#ifdef DEBUG
    if (xSlide.is())
        printf("new page created\n");
#endif

    Reference< beans::XPropertySet > xPropSet(xSlide, uno::UNO_QUERY);
    xPropSet->setPropertyValue("Layout", makeAny(short(aLayoutInfo[ nOffset ].nType)));
#if OSL_DEBUG_LEVEL > 1
    dump_pset(xPropSet);
#endif
    mXPagePropSet.set(xSlide, UNO_QUERY);
    mXShapes.set(xSlide, UNO_QUERY);

    if (mLayoutInfo[ nOffset ].mnFileIdArray.size() < mnMasterPages)
    {
        mLayoutInfo[ nOffset ].mnFileIdArray.resize(mnMasterPages);
    }

    if (mLayoutInfo[ nOffset ].mnFileIdArray[ nMasterNum ] != 0)
        return;

    FSHelperPtr pFS
        = openFragmentStreamWithSerializer(OUStringBuffer()
                                           .append("ppt/slideLayouts/slideLayout")
                                           .append(mnLayoutFileIdMax)
                                           .append(".xml")
                                           .makeStringAndClear(),
                                           "application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml");

    // add implicit relation of slide layout to slide master
    addRelation(pFS->getOutputStream(),
                oox::getRelationship(Relationship::SLIDEMASTER),
                OUStringBuffer()
                .append("../slideMasters/slideMaster")
                .append(static_cast<sal_Int32>(nMasterNum) + 1)
                .append(".xml")
                .makeStringAndClear());

    pFS->startElementNS(XML_p, XML_sldLayout,
                        PNMSS,
                        XML_type, aLayoutInfo[ nOffset ].sType,
                        XML_preserve, "1",
                        FSEND);

    pFS->startElementNS(XML_p, XML_cSld,
                        XML_name, aLayoutInfo[ nOffset ].sName,
                        FSEND);
    //pFS->write( MINIMAL_SPTREE ); // TODO: write actual shape tree
    WriteShapeTree(pFS, LAYOUT, true);

    pFS->endElementNS(XML_p, XML_cSld);

    pFS->endElementNS(XML_p, XML_sldLayout);

    mLayoutInfo[ nOffset ].mnFileIdArray[ nMasterNum ] = mnLayoutFileIdMax;

    mnLayoutFileIdMax ++;

    xDrawPages->remove(xSlide);
}

void PowerPointExport::WriteShapeTree(const FSHelperPtr& pFS, PageType ePageType, bool bMaster)
{
    PowerPointShapeExport aDML(pFS, &maShapeMap, this);
    aDML.SetMaster(bMaster);
    aDML.SetPageType(ePageType);
    aDML.SetBackgroundDark(mbIsBackgroundDark);

    pFS->startElementNS(XML_p, XML_spTree, FSEND);
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
            aDML.WriteShape(mXShape);
        }
    }

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
    if (bMaster && ShapeExport::NonEmptyText(xShape))
    {
        WritePlaceholderShape(xShape, ePlaceholder);

        return true;
    }

    return false;
}

ShapeExport& PowerPointShapeExport::WritePlaceholderShape(const Reference< XShape >& xShape, PlaceholderType ePlaceholder)
{
    mpFS->startElementNS(XML_p, XML_sp, FSEND);

    // non visual shape properties
    mpFS->startElementNS(XML_p, XML_nvSpPr, FSEND);
    const OString aPlaceholderID("PlaceHolder " + OString::number(mnShapeIdMax++));
    WriteNonVisualDrawingProperties(xShape, aPlaceholderID.getStr());
    mpFS->startElementNS(XML_p, XML_cNvSpPr, FSEND);
    mpFS->singleElementNS(XML_a, XML_spLocks, XML_noGrp, "1", FSEND);
    mpFS->endElementNS(XML_p, XML_cNvSpPr);
    mpFS->startElementNS(XML_p, XML_nvPr, FSEND);

    const char* pType = nullptr;
    switch (ePlaceholder)
    {
    case SlideImage:
        pType = "sldImg";
        break;
    case Notes:
        pType = "body";
        break;
    case Header:
        pType = "hdr";
        break;
    case Footer:
        pType = "ftr";
        break;
    case SlideNumber:
        pType = "sldNum";
        break;
    case DateAndTime:
        pType = "dt";
        break;
    case Outliner:
        pType = "body";
        break;
    case Title:
        pType = "title";
        break;
    case Subtitle:
        pType = "subTitle";
        break;
    default:
        SAL_INFO("sd.eppt", "warning: unhandled placeholder type: " << ePlaceholder);
    }
    SAL_INFO("sd.eppt", "write placeholder " << pType);
    mpFS->singleElementNS(XML_p, XML_ph, XML_type, pType, FSEND);
    mpFS->endElementNS(XML_p, XML_nvPr);
    mpFS->endElementNS(XML_p, XML_nvSpPr);

    // visual shape properties
    mpFS->startElementNS(XML_p, XML_spPr, FSEND);
    WriteShapeTransformation(xShape, XML_a);
    WritePresetShape("rect");
    Reference< XPropertySet > xProps(xShape, UNO_QUERY);
    if (xProps.is())
        WriteBlipFill(xProps, "Graphic");
    mpFS->endElementNS(XML_p, XML_spPr);

    WriteTextBox(xShape, XML_p);

    mpFS->endElementNS(XML_p, XML_sp);

    return *this;
}

#define SYS_COLOR_SCHEMES "      <a:dk1>\
        <a:sysClr val=\"windowText\" lastClr=\"000000\"/>\
      </a:dk1>\
      <a:lt1>\
        <a:sysClr val=\"window\" lastClr=\"FFFFFF\"/>\
      </a:lt1>"

#define MINIMAL_THEME "    <a:fontScheme name=\"Office\">\
      <a:majorFont>\
        <a:latin typeface=\"Arial\"/>\
        <a:ea typeface=\"DejaVu Sans\"/>\
        <a:cs typeface=\"DejaVu Sans\"/>\
      </a:majorFont>\
      <a:minorFont>\
        <a:latin typeface=\"Arial\"/>\
        <a:ea typeface=\"DejaVu Sans\"/>\
        <a:cs typeface=\"DejaVu Sans\"/>\
      </a:minorFont>\
    </a:fontScheme>\
    <a:fmtScheme name=\"Office\">\
      <a:fillStyleLst>\
        <a:solidFill>\
          <a:schemeClr val=\"phClr\"/>\
        </a:solidFill>\
        <a:gradFill rotWithShape=\"1\">\
          <a:gsLst>\
            <a:gs pos=\"0\">\
              <a:schemeClr val=\"phClr\">\
                <a:tint val=\"50000\"/>\
                <a:satMod val=\"300000\"/>\
              </a:schemeClr>\
            </a:gs>\
            <a:gs pos=\"35000\">\
              <a:schemeClr val=\"phClr\">\
                <a:tint val=\"37000\"/>\
                <a:satMod val=\"300000\"/>\
              </a:schemeClr>\
            </a:gs>\
            <a:gs pos=\"100000\">\
              <a:schemeClr val=\"phClr\">\
                <a:tint val=\"15000\"/>\
                <a:satMod val=\"350000\"/>\
              </a:schemeClr>\
            </a:gs>\
          </a:gsLst>\
          <a:lin ang=\"16200000\" scaled=\"1\"/>\
        </a:gradFill>\
        <a:gradFill rotWithShape=\"1\">\
          <a:gsLst>\
            <a:gs pos=\"0\">\
              <a:schemeClr val=\"phClr\">\
                <a:shade val=\"51000\"/>\
                <a:satMod val=\"130000\"/>\
              </a:schemeClr>\
            </a:gs>\
            <a:gs pos=\"80000\">\
              <a:schemeClr val=\"phClr\">\
                <a:shade val=\"93000\"/>\
                <a:satMod val=\"130000\"/>\
              </a:schemeClr>\
            </a:gs>\
            <a:gs pos=\"100000\">\
              <a:schemeClr val=\"phClr\">\
                <a:shade val=\"94000\"/>\
                <a:satMod val=\"135000\"/>\
              </a:schemeClr>\
            </a:gs>\
          </a:gsLst>\
          <a:lin ang=\"16200000\" scaled=\"0\"/>\
        </a:gradFill>\
      </a:fillStyleLst>\
      <a:lnStyleLst>\
        <a:ln w=\"9525\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\">\
          <a:solidFill>\
            <a:schemeClr val=\"phClr\">\
              <a:shade val=\"95000\"/>\
              <a:satMod val=\"105000\"/>\
            </a:schemeClr>\
          </a:solidFill>\
          <a:prstDash val=\"solid\"/>\
        </a:ln>\
        <a:ln w=\"25400\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\">\
          <a:solidFill>\
            <a:schemeClr val=\"phClr\"/>\
          </a:solidFill>\
          <a:prstDash val=\"solid\"/>\
        </a:ln>\
        <a:ln w=\"38100\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\">\
          <a:solidFill>\
            <a:schemeClr val=\"phClr\"/>\
          </a:solidFill>\
          <a:prstDash val=\"solid\"/>\
        </a:ln>\
      </a:lnStyleLst>\
      <a:effectStyleLst>\
        <a:effectStyle>\
          <a:effectLst>\
            <a:outerShdw blurRad=\"40000\" dist=\"20000\" dir=\"5400000\" rotWithShape=\"0\">\
              <a:srgbClr val=\"000000\">\
                <a:alpha val=\"38000\"/>\
              </a:srgbClr>\
            </a:outerShdw>\
          </a:effectLst>\
        </a:effectStyle>\
        <a:effectStyle>\
          <a:effectLst>\
            <a:outerShdw blurRad=\"40000\" dist=\"23000\" dir=\"5400000\" rotWithShape=\"0\">\
              <a:srgbClr val=\"000000\">\
                <a:alpha val=\"35000\"/>\
              </a:srgbClr>\
            </a:outerShdw>\
          </a:effectLst>\
        </a:effectStyle>\
        <a:effectStyle>\
          <a:effectLst>\
            <a:outerShdw blurRad=\"40000\" dist=\"23000\" dir=\"5400000\" rotWithShape=\"0\">\
              <a:srgbClr val=\"000000\">\
                <a:alpha val=\"35000\"/>\
              </a:srgbClr>\
            </a:outerShdw>\
          </a:effectLst>\
          <a:scene3d>\
            <a:camera prst=\"orthographicFront\">\
              <a:rot lat=\"0\" lon=\"0\" rev=\"0\"/>\
            </a:camera>\
            <a:lightRig rig=\"threePt\" dir=\"t\">\
              <a:rot lat=\"0\" lon=\"0\" rev=\"1200000\"/>\
            </a:lightRig>\
          </a:scene3d>\
          <a:sp3d>\
            <a:bevelT w=\"63500\" h=\"25400\"/>\
          </a:sp3d>\
        </a:effectStyle>\
      </a:effectStyleLst>\
      <a:bgFillStyleLst>\
        <a:solidFill>\
          <a:schemeClr val=\"phClr\"/>\
        </a:solidFill>\
        <a:gradFill rotWithShape=\"1\">\
          <a:gsLst>\
            <a:gs pos=\"0\">\
              <a:schemeClr val=\"phClr\">\
                <a:tint val=\"40000\"/>\
                <a:satMod val=\"350000\"/>\
              </a:schemeClr>\
            </a:gs>\
            <a:gs pos=\"40000\">\
              <a:schemeClr val=\"phClr\">\
                <a:tint val=\"45000\"/>\
                <a:shade val=\"99000\"/>\
                <a:satMod val=\"350000\"/>\
              </a:schemeClr>\
            </a:gs>\
            <a:gs pos=\"100000\">\
              <a:schemeClr val=\"phClr\">\
                <a:shade val=\"20000\"/>\
                <a:satMod val=\"255000\"/>\
              </a:schemeClr>\
            </a:gs>\
          </a:gsLst>\
          <a:path path=\"circle\">\
            <a:fillToRect l=\"50000\" t=\"-80000\" r=\"50000\" b=\"180000\"/>\
          </a:path>\
        </a:gradFill>\
        <a:gradFill rotWithShape=\"1\">\
          <a:gsLst>\
            <a:gs pos=\"0\">\
              <a:schemeClr val=\"phClr\">\
                <a:tint val=\"80000\"/>\
                <a:satMod val=\"300000\"/>\
              </a:schemeClr>\
            </a:gs>\
            <a:gs pos=\"100000\">\
              <a:schemeClr val=\"phClr\">\
                <a:shade val=\"30000\"/>\
                <a:satMod val=\"200000\"/>\
              </a:schemeClr>\
            </a:gs>\
          </a:gsLst>\
          <a:path path=\"circle\">\
            <a:fillToRect l=\"50000\" t=\"50000\" r=\"50000\" b=\"50000\"/>\
          </a:path>\
        </a:gradFill>\
      </a:bgFillStyleLst>\
    </a:fmtScheme>"

void PowerPointExport::WriteDefaultColorSchemes(const FSHelperPtr& pFS)
{
    for (int nId = PredefinedClrSchemeId::dk2; nId != PredefinedClrSchemeId::Count; nId++)
    {
        OUString sName = PredefinedClrNames[static_cast<PredefinedClrSchemeId>(nId)];
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

        OUString sOpenColorScheme = OUStringBuffer()
            .append("<a:")
            .append(sName)
            .append(">")
            .makeStringAndClear();
        pFS->write(sOpenColorScheme);

        pFS->singleElementNS(XML_a, XML_srgbClr, XML_val, I32SHEX(nColor), FSEND);

        OUString sCloseColorScheme = OUStringBuffer()
            .append("</a:")
            .append(sName)
            .append(">")
            .makeStringAndClear();
        pFS->write(sCloseColorScheme);
    }
}

bool PowerPointExport::WriteColorSchemes(const FSHelperPtr& pFS, const OUString& rThemePath)
{
    try
    {
        uno::Reference<beans::XPropertySet> xDocProps(getModel(), uno::UNO_QUERY);
        if (xDocProps.is())
        {
            uno::Reference<beans::XPropertySetInfo> xPropsInfo = xDocProps->getPropertySetInfo();

            const OUString aGrabBagPropName = "InteropGrabBag";
            if (xPropsInfo.is() && xPropsInfo->hasPropertyByName(aGrabBagPropName))
            {
                comphelper::SequenceAsHashMap aGrabBag(xDocProps->getPropertyValue(aGrabBagPropName));
                uno::Sequence<beans::PropertyValue> aCurrentTheme;

                aGrabBag.getValue(rThemePath) >>= aCurrentTheme;

                if (!aCurrentTheme.getLength())
                    return false;

                // Order is important
                for (int nId = PredefinedClrSchemeId::dk2; nId != PredefinedClrSchemeId::Count; nId++)
                {
                    OUString sName = PredefinedClrNames[static_cast<PredefinedClrSchemeId>(nId)];
                    sal_Int32 nColor = 0;

                    for (auto aIt = aCurrentTheme.begin(); aIt != aCurrentTheme.end(); aIt++)
                    {
                        if (aIt->Name == sName)
                        {
                            aIt->Value >>= nColor;
                            break;
                        }
                    }

                    OUString sOpenColorScheme = OUStringBuffer()
                        .append("<a:")
                        .append(sName)
                        .append(">")
                        .makeStringAndClear();
                    pFS->write(sOpenColorScheme);

                    pFS->singleElementNS(XML_a, XML_srgbClr, XML_val, I32SHEX(nColor), FSEND);

                    OUString sCloseColorScheme = OUStringBuffer()
                        .append("</a:")
                        .append(sName)
                        .append(">")
                        .makeStringAndClear();
                    pFS->write(sCloseColorScheme);
                }

                // TODO: write complete color schemes & only if successful, protection against partial export
                return true;
            }
        }
    }
    catch (const uno::Exception&)
    {
        SAL_WARN("writerfilter", "Failed to save documents grab bag");
    }

    return false;
}

void PowerPointExport::WriteTheme(sal_Int32 nThemeNum)
{
    OUString sThemePath = OUStringBuffer()
        .append("ppt/theme/theme")
        .append(nThemeNum + 1)
        .append(".xml")
        .makeStringAndClear();

    FSHelperPtr pFS = openFragmentStreamWithSerializer(sThemePath,
                      "application/vnd.openxmlformats-officedocument.theme+xml");

    pFS->startElementNS(XML_a, XML_theme,
                        FSNS(XML_xmlns, XML_a), OUStringToOString(this->getNamespaceURL(OOX_NS(dml)), RTL_TEXTENCODING_UTF8),
                        XML_name, "Office Theme",
                        FSEND);

    pFS->startElementNS(XML_a, XML_themeElements, FSEND);
    pFS->startElementNS(XML_a, XML_clrScheme, XML_name, "Office", FSEND);

    pFS->write(SYS_COLOR_SCHEMES);

    if (!WriteColorSchemes(pFS, sThemePath))
    {
        // if style is not defined, try to use first one
        if (!WriteColorSchemes(pFS, "ppt/theme/theme1.xml"))
        {
            // color schemes are required - use default values
            WriteDefaultColorSchemes(pFS);
        }
    }

    pFS->endElementNS(XML_a, XML_clrScheme);

    // export remaining part
    pFS->write(MINIMAL_THEME);

    pFS->endElementNS(XML_a, XML_themeElements);
    pFS->endElementNS(XML_a, XML_theme);
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

    mPresentationFS->startElementNS(XML_p, XML_notesMasterIdLst, FSEND);

    OUString sRelId = addRelation(mPresentationFS->getOutputStream(),
                                  oox::getRelationship(Relationship::NOTESMASTER),
                                  "notesMasters/notesMaster1.xml");

    mPresentationFS->singleElementNS(XML_p, XML_notesMasterId,
                                     FSNS(XML_r, XML_id), USS(sRelId),
                                     FSEND);

    mPresentationFS->endElementNS(XML_p, XML_notesMasterIdLst);

    FSHelperPtr pFS =
        openFragmentStreamWithSerializer("ppt/notesMasters/notesMaster1.xml",
                                         "application/vnd.openxmlformats-officedocument.presentationml.notesMaster+xml");
    // write theme per master
    WriteTheme(mnMasterPages);

    // add implicit relation to the presentation theme
    addRelation(pFS->getOutputStream(),
                oox::getRelationship(Relationship::THEME),
                OUStringBuffer()
                .append("../theme/theme")
                .append(static_cast<sal_Int32>(mnMasterPages) + 1)
                .append(".xml")
                .makeStringAndClear());

    pFS->startElementNS(XML_p, XML_notesMaster, PNMSS, FSEND);

    pFS->startElementNS(XML_p, XML_cSld, FSEND);

    Reference< XPropertySet > aXBackgroundPropSet;
    if (ImplGetPropertyValue(mXPagePropSet, "Background") &&
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
                         XML_folHlink, "folHlink",
                         FSEND);

    pFS->endElementNS(XML_p, XML_notesMaster);

    SAL_INFO("sd.eppt", "----------------");
}

void PowerPointExport::embedEffectAudio(const FSHelperPtr& pFS, const OUString& sUrl, OUString& sRelId, OUString& sName)
{
    comphelper::LifecycleProxy aProxy;

    if (!sUrl.endsWithIgnoreAsciiCase(".wav"))
        return;

    uno::Reference<io::XInputStream> xAudioStream;
    if (sUrl.startsWith("vnd.sun.star.Package:"))
    {
        uno::Reference<document::XStorageBasedDocument> xStorageBasedDocument(getModel(), uno::UNO_QUERY);
        if (!xStorageBasedDocument.is())
            return;

        uno::Reference<embed::XStorage> xDocumentStorage(xStorageBasedDocument->getDocumentStorage(), uno::UNO_QUERY);
        if (!xDocumentStorage.is())
            return;

        uno::Reference<io::XStream> xStream = comphelper::OStorageHelper::GetStreamAtPackageURL(xDocumentStorage, sUrl,
                                                    css::embed::ElementModes::READ, aProxy);

        if (xStream.is())
            xAudioStream = xStream->getInputStream();
    }
    else
        xAudioStream = comphelper::OStorageHelper::GetInputStreamFromURL(sUrl, getComponentContext());

    if (!xAudioStream.is())
        return;

    int nLastSlash = sUrl.lastIndexOf('/');
    sName = sUrl.copy(nLastSlash >= 0 ? nLastSlash + 1 : 0);

    OUString sPath = OUStringBuffer().append("/media/")
                                     .append(sName)
                                     .makeStringAndClear();

    sRelId = addRelation(pFS->getOutputStream(),
                        oox::getRelationship(Relationship::AUDIO), sPath);

    uno::Reference<io::XOutputStream> xOutputStream = openFragmentStream(sPath, "audio/x-wav");

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
    return OUString("com.sun.star.comp.Impress.oox.PowerPointExport");
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
            SAL_INFO("sd.eppt", name << " = " << pointValue << "    (RectanglePoint)");
        else
            SAL_INFO("sd.eppt", "???          <unhandled type>");
    }
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
