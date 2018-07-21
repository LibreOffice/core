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
#include <o3tl/any.hxx>
#include <oox/drawingml/chart/chartconverter.hxx>
#include <oox/drawingml/clrscheme.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <oox/token/relationship.hxx>
#include <oox/ole/vbaproject.hxx>
#include "epptooxml.hxx"
#include "epptdef.hxx"
#include <oox/export/shapes.hxx>

#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/storagehelper.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/factory.hxx>
#include <sax/fshelper.hxx>
#include <rtl/ustrbuf.hxx>
#include <filter/msfilter/escherex.hxx>
#include <tools/poly.hxx>

#include <com/sun/star/animations/AnimationAdditiveMode.hpp>
#include <com/sun/star/animations/AnimationCalcMode.hpp>
#include <com/sun/star/animations/AnimationFill.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/AnimationRestart.hpp>
#include <com/sun/star/animations/AnimationTransformType.hpp>
#include <com/sun/star/animations/AnimationValueType.hpp>
#include <com/sun/star/animations/Event.hpp>
#include <com/sun/star/animations/EventTrigger.hpp>
#include <com/sun/star/animations/Timing.hpp>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <com/sun/star/animations/ValuePair.hpp>
#include <com/sun/star/animations/XAnimateMotion.hpp>
#include <com/sun/star/animations/XAnimateTransform.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/XCommand.hpp>
#include <com/sun/star/animations/XTransitionFilter.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/RectanglePoint.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/office/XAnnotationEnumeration.hpp>
#include <com/sun/star/office/XAnnotationAccess.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/presentation/EffectCommands.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/presentation/EffectPresetClass.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <oox/export/utils.hxx>
#include <oox/ppt/pptfilterhelpers.hxx>

#include "pptexanimations.hxx"
#include "../ppt/pptanimations.hxx"

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <utility>

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
    int nType;
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

    // PPTM needs a different media type for the presentation.xml stream.
    OUString aMediaType("application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml");
    if (mbPptm)
        aMediaType = "application/vnd.ms-powerpoint.presentation.macroEnabled.main+xml";

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

#define GETA(propName) \
    ImplGetPropertyValue( mXPagePropSet, #propName )

#define GET(variable, propName) \
    if ( GETA(propName) ) \
        mAny >>= variable;

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
    GET(eFadeEffect, Effect);

    sal_Int16 nTransitionType = 0, nTransitionSubtype = 0;
    sal_Int8 nPPTTransitionType = 0;
    sal_uInt8 nDirection = 0;

    if (GETA(TransitionType) && (mAny >>= nTransitionType) &&
            GETA(TransitionSubtype) && (mAny >>= nTransitionSubtype))
        nPPTTransitionType = GetTransition(nTransitionType, nTransitionSubtype, eFadeEffect, nDirection);

    if (!nPPTTransitionType && eFadeEffect != FadeEffect_NONE)
        nPPTTransitionType = GetTransition(eFadeEffect, nDirection);

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
                SAL_FALLTHROUGH;
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
    if (GETA(TransitionDuration))
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
    else if (GETA(Speed))
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

    if (GETA(Change))
        mAny >>= changeType;

    // 1 means automatic, 2 half automatic - not sure what it means - at least I don't see it in UI
    if (changeType == 1 && GETA(Duration))
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

    pFS->endElementNS(XML_p, XML_transition);

    if (nTransition14 || pPresetTransition || isTransitionDurationSet)
    {
        pFS->endElement(FSNS(XML_mc, XML_Fallback));
        pFS->endElement(FSNS(XML_mc, XML_AlternateContent));
    }
}

void PowerPointExport::WriteAnimationProperty(const FSHelperPtr& pFS, const Any& rAny, sal_Int32 nToken)
{
    if (!rAny.hasValue())
        return;

    sal_uInt32 nRgb;
    double fDouble;

    uno::TypeClass aClass = rAny.getValueType().getTypeClass();
    bool bWriteToken = nToken &&
        (  aClass == TypeClass_LONG
        || aClass == TypeClass_DOUBLE
        || aClass == TypeClass_STRING );

    if (bWriteToken)
        pFS->startElementNS(XML_p, XML_to, FSEND);

    switch (rAny.getValueType().getTypeClass())
    {
    case TypeClass_LONG:
        rAny >>= nRgb;
        pFS->singleElementNS(XML_a, XML_srgbClr,
                             XML_val, I32SHEX(nRgb),
                             FSEND);
        break;
    case TypeClass_DOUBLE:
        rAny >>= fDouble;
        pFS->singleElementNS(XML_p, XML_fltVal,
            XML_val, DS(fDouble),
            FSEND);
        break;
    case TypeClass_STRING:
        pFS->singleElementNS(XML_p, XML_strVal,
                             XML_val, USS(*o3tl::doAccess<OUString>(rAny)),
                             FSEND);
        break;
    default:
        break;
    }

    if (bWriteToken)
        pFS->endElementNS(XML_p, nToken);
}

void PowerPointExport::WriteAnimateValues(const FSHelperPtr& pFS, const Reference< XAnimate >& rXAnimate)
{
    const Sequence< double > aKeyTimes = rXAnimate->getKeyTimes();
    if (aKeyTimes.getLength() <= 0)
        return;
    const Sequence< Any > aValues = rXAnimate->getValues();
    const OUString& sFormula = rXAnimate->getFormula();
    const OUString& rAttributeName = rXAnimate->getAttributeName();

    SAL_INFO("sd.eppt", "animate values, formula: " << USS(sFormula));

    pFS->startElementNS(XML_p, XML_tavLst, FSEND);

    for (int i = 0; i < aKeyTimes.getLength(); i++)
    {
        SAL_INFO("sd.eppt", "animate value " << i << ": " << aKeyTimes[i]);
        if (aValues[ i ].hasValue())
        {
            pFS->startElementNS(XML_p, XML_tav,
                                XML_fmla, sFormula.isEmpty() ? nullptr : USS(sFormula),
                                XML_tm, I32S(static_cast<sal_Int32>(aKeyTimes[ i ]*100000.0)),
                                FSEND);
            pFS->startElementNS(XML_p, XML_val, FSEND);
            ValuePair aPair;
            if (aValues[ i ] >>= aPair)
            {
                WriteAnimationProperty(pFS, AnimationExporter::convertAnimateValue(aPair.First, rAttributeName));
                WriteAnimationProperty(pFS, AnimationExporter::convertAnimateValue(aPair.Second, rAttributeName));
            }
            else
                WriteAnimationProperty(pFS, AnimationExporter::convertAnimateValue(aValues[ i ], rAttributeName));

            pFS->endElementNS(XML_p, XML_val);
            pFS->endElementNS(XML_p, XML_tav);
        }
    }

    pFS->endElementNS(XML_p, XML_tavLst);
}

void PowerPointExport::WriteAnimateTo(const FSHelperPtr& pFS, const Any& rValue, const OUString& rAttributeName)
{
    if (!rValue.hasValue())
        return;

    SAL_INFO("sd.eppt", "to attribute name: " << USS(rAttributeName));

    sal_uInt32 nColor;
    if (rValue >>= nColor)
    {
        // RGB color
        WriteAnimationProperty(pFS, rValue, XML_to);
    }
    else
        WriteAnimationProperty(pFS, AnimationExporter::convertAnimateValue(rValue, rAttributeName), XML_to);
}

void PowerPointExport::WriteAnimationAttributeName(const FSHelperPtr& pFS, const OUString& rAttributeName)
{
    if (rAttributeName.isEmpty())
        return;

    pFS->startElementNS(XML_p, XML_attrNameLst, FSEND);

    SAL_INFO("sd.eppt", "write attribute name: " << USS(rAttributeName));

    if (rAttributeName == "X;Y")
    {
        pFS->startElementNS(XML_p, XML_attrName, FSEND);
        pFS->writeEscaped("ppt_x");
        pFS->endElementNS(XML_p, XML_attrName);

        pFS->startElementNS(XML_p, XML_attrName, FSEND);
        pFS->writeEscaped("ppt_y");
        pFS->endElementNS(XML_p, XML_attrName);
    }
    else
    {
        const oox::ppt::ImplAttributeNameConversion *attrConv = oox::ppt::getAttributeConversionList();
        const char* pAttribute = nullptr;

        while(attrConv->mpAPIName != nullptr)
        {
            if(rAttributeName.equalsAscii(attrConv->mpAPIName))
            {
                pAttribute = attrConv->mpMSName;
                break;
            }
            attrConv++;
        }

        if (pAttribute)
        {
            pFS->startElementNS(XML_p, XML_attrName, FSEND);
            pFS->writeEscaped(pAttribute);
            pFS->endElementNS(XML_p, XML_attrName);
        }
        else
        {
            SAL_WARN("sd.eppt", "unhandled animation attribute name: " << rAttributeName);
        }
    }

    pFS->endElementNS(XML_p, XML_attrNameLst);
}

void PowerPointExport::WriteAnimationTarget(const FSHelperPtr& pFS, const Any& rTarget)
{
    sal_Int32 nParagraph = -1;
    bool bParagraphTarget = false;

    Reference< XShape > rXShape;
    rTarget >>= rXShape;

    if (!rXShape.is())
    {
        ParagraphTarget aParagraphTarget;
        if (rTarget >>= aParagraphTarget)
            rXShape = aParagraphTarget.Shape;
        if (rXShape.is())
        {
            nParagraph = static_cast< sal_Int32 >(aParagraphTarget.Paragraph);
            Reference< XSimpleText > xText(rXShape, UNO_QUERY);
            if (xText.is())
            {
                bParagraphTarget = true;
            }
        }
    }

    if (rXShape.is())
    {
        pFS->startElementNS(XML_p, XML_tgtEl, FSEND);
        pFS->startElementNS(XML_p, XML_spTgt,
                            XML_spid, I32S(ShapeExport::GetShapeID(rXShape, &maShapeMap)),
                            FSEND);
        if (bParagraphTarget)
        {
            pFS->startElementNS(XML_p, XML_txEl, FSEND);
            pFS->singleElementNS(XML_p, XML_pRg,
                                 XML_st, I32S(nParagraph),
                                 XML_end, I32S(nParagraph),
                                 FSEND);
            pFS->endElementNS(XML_p, XML_txEl);
        }
        pFS->endElementNS(XML_p, XML_spTgt);
        pFS->endElementNS(XML_p, XML_tgtEl);
    }
}

void PowerPointExport::WriteAnimationNodeAnimate(const FSHelperPtr& pFS, const Reference< XAnimationNode >& rXNode, sal_Int32 nXmlNodeType, bool bMainSeqChild)
{
    Reference< XAnimate > rXAnimate(rXNode, UNO_QUERY);
    if (!rXAnimate.is())
        return;

    const char* pCalcMode = nullptr;
    const char* pValueType = nullptr;
    bool bSimple = (nXmlNodeType != XML_anim);
    bool bTo = true;

    if (!bSimple)
    {
        switch (rXAnimate->getCalcMode())
        {
        case AnimationCalcMode::DISCRETE:
            pCalcMode = "discrete";
            break;
        case AnimationCalcMode::LINEAR:
            pCalcMode = "lin";
            break;
        }

        switch (AnimationExporter::GetValueTypeForAttributeName(rXAnimate->getAttributeName()))
        {
        case AnimationValueType::STRING:
            pValueType = "str";
            break;
        case AnimationValueType::NUMBER:
            pValueType = "num";
            break;
        case AnimationValueType::COLOR:
            pValueType = "clr";
            break;
        }


    }

    if (nXmlNodeType == XML_animMotion)
    {
        OUString aPath;
        Reference<XAnimateMotion> xMotion(rXNode, UNO_QUERY);
        if (xMotion.is())
            xMotion->getPath() >>= aPath;

        pFS->startElementNS(XML_p, nXmlNodeType,
                            XML_origin, "layout",
                            XML_path, OUStringToOString(aPath, RTL_TEXTENCODING_UTF8),
                            FSEND);
    }
    else if (nXmlNodeType == XML_animRot)
    {
        // when const char* is nullptr, the attribute is completely omitted in the output
        const char* pBy = nullptr;
        const char* pFrom = nullptr;
        const char* pTo = nullptr;
        OString aBy, aFrom, aTo;

        Reference<XAnimateTransform> xTransform(rXNode, UNO_QUERY);
        if (xTransform.is())
        {
            double value;
            if (xTransform->getBy() >>= value)
            {
                aBy = OString::number(static_cast<int>(value * PER_DEGREE));
                pBy = aBy.getStr();
            }

            if (xTransform->getFrom() >>= value)
            {
                aFrom = OString::number(static_cast<int>(value * PER_DEGREE));
                pFrom = aFrom.getStr();
            }

            if (xTransform->getTo() >>= value)
            {
                aTo = OString::number(static_cast<int>(value * PER_DEGREE));
                pTo = aTo.getStr();
            }
        }

        pFS->startElementNS(XML_p, nXmlNodeType,
                            XML_by, pBy,
                            XML_from, pFrom,
                            XML_to, pTo,
                            FSEND);
    }
    else if (nXmlNodeType == XML_animClr)
    {
        pFS->startElementNS(XML_p, nXmlNodeType,
            XML_clrSpc, "rgb",
            XML_calcmode, pCalcMode,
            XML_valueType, pValueType,
            FSEND);
    }
    else
    {
        OUString sFrom, sTo, sBy;
        if (rXAnimate.is() && nXmlNodeType == XML_anim)
        {
            OUString sAttributeName = rXAnimate->getAttributeName();
            Any aFrom = AnimationExporter::convertAnimateValue(rXAnimate->getFrom(), sAttributeName);
            aFrom >>= sFrom;
            Any aTo = AnimationExporter::convertAnimateValue(rXAnimate->getTo(), sAttributeName);
            aTo >>= sTo;
            Any aBy = AnimationExporter::convertAnimateValue(rXAnimate->getBy(), sAttributeName);
            aBy >>= sBy;
        }

        pFS->startElementNS(XML_p, nXmlNodeType,
            XML_calcmode, pCalcMode,
            XML_valueType, pValueType,
            XML_from, sFrom.getLength() ? USS(sFrom) : nullptr,
            XML_to, sTo.getLength() ? USS(sTo) : nullptr,
            XML_by, sBy.getLength() ? USS(sBy) : nullptr,
            FSEND);
        bTo = sTo.isEmpty() && sFrom.isEmpty() && sBy.isEmpty();
    }

    WriteAnimationNodeAnimateInside(pFS, rXNode, bMainSeqChild, bSimple, bTo);
    pFS->endElementNS(XML_p, nXmlNodeType);
}

void PowerPointExport::WriteAnimationNodeAnimateInside(const FSHelperPtr& pFS, const Reference< XAnimationNode >& rXNode, bool bMainSeqChild, bool bSimple, bool bWriteTo)
{
    Reference< XAnimate > rXAnimate(rXNode, UNO_QUERY);
    if (!rXAnimate.is())
        return;

    const char* pAdditive = nullptr;

    if (!bSimple)
    {
        switch (rXAnimate->getAdditive())
        {
        case AnimationAdditiveMode::BASE:
            pAdditive = "base";
            break;
        case AnimationAdditiveMode::SUM:
            pAdditive = "sum";
            break;
        case AnimationAdditiveMode::REPLACE:
            pAdditive = "repl";
            break;
        case AnimationAdditiveMode::MULTIPLY:
            pAdditive = "mult";
            break;
        case AnimationAdditiveMode::NONE:
            pAdditive = "none";
            break;
        }
    }

    pFS->startElementNS(XML_p, XML_cBhvr,
                        XML_additive, pAdditive,
                        FSEND);
    WriteAnimationNodeCommonPropsStart(pFS, rXNode, true, bMainSeqChild);
    WriteAnimationTarget(pFS, rXAnimate->getTarget());

    Reference<XAnimateTransform> xTransform(rXNode, UNO_QUERY);

    // The attribute name of AnimateTransform is "Transform", we have to fix it.
    OUString sNewAttr;
    if (xTransform.is() && xTransform->getTransformType() == AnimationTransformType::ROTATE)
            sNewAttr = "Rotate";

    WriteAnimationAttributeName(pFS, xTransform.is() ? sNewAttr : rXAnimate->getAttributeName());

    pFS->endElementNS(XML_p, XML_cBhvr);
    WriteAnimateValues(pFS, rXAnimate);
    if (bWriteTo)
        WriteAnimateTo(pFS, rXAnimate->getTo(), rXAnimate->getAttributeName());
}

void PowerPointExport::WriteAnimationCondition(const FSHelperPtr& pFS, const char* pDelay, const char* pEvent, double fDelay, bool bHasFDelay, sal_Int32 nToken)
{
    if (bHasFDelay || pDelay || pEvent)
    {
        pFS->startElementNS(XML_p, nToken, FSEND);

        if (!pEvent)
            pFS->singleElementNS(XML_p, XML_cond,
                                 XML_delay, bHasFDelay ? I64S(static_cast<sal_uInt32>(fDelay*1000.0)) : pDelay,
                                 FSEND);
        else
        {
            pFS->startElementNS(XML_p, XML_cond,
                                XML_delay, bHasFDelay ? I64S(static_cast<sal_uInt32>(fDelay*1000.0)) : pDelay,
                                XML_evt, pEvent,
                                FSEND);

            pFS->startElementNS(XML_p, XML_tgtEl, FSEND);
            pFS->singleElementNS(XML_p, XML_sldTgt, FSEND);
            pFS->endElementNS(XML_p, XML_tgtEl);

            pFS->endElementNS(XML_p, XML_cond);
        }

        pFS->endElementNS(XML_p, nToken);
    }
}

void PowerPointExport::WriteAnimationCondition(const FSHelperPtr& pFS, Any const& rAny, bool bWriteEvent, bool bMainSeqChild, sal_Int32 nToken)
{
    bool bHasFDelay = false;
    double fDelay = 0;
    Timing eTiming;
    Event aEvent;
    Reference<XShape> xShape;
    const char* pDelay = nullptr;
    const char* pEvent = nullptr;

    if (rAny >>= fDelay)
        bHasFDelay = true;
    else if (rAny >>= eTiming)
    {
        if (eTiming == Timing_INDEFINITE)
            pDelay = "indefinite";
    }
    else if (rAny >>= aEvent)
    {
        // TODO

        SAL_INFO("sd.eppt", "animation condition event: TODO");
        SAL_INFO("sd.eppt", "event offset has value: " << aEvent.Offset.hasValue() << " trigger: " <<  aEvent.Trigger
                 << " source has value: " <<  aEvent.Source.hasValue());
        if (!bWriteEvent && aEvent.Trigger == EventTrigger::ON_NEXT && bMainSeqChild)
            pDelay = "indefinite";
        else if (bWriteEvent)
        {
            switch (aEvent.Trigger)
            {
            case EventTrigger::ON_NEXT:
                pEvent = "onNext";
                break;
            case EventTrigger::ON_PREV:
                pEvent = "onPrev";
                break;
            case EventTrigger::BEGIN_EVENT:
                pEvent = "begin";
                break;
            case EventTrigger::END_EVENT:
                pEvent = "end";
                break;
            case EventTrigger::ON_BEGIN:
                pEvent = "onBegin";
                break;
            case EventTrigger::ON_END:
                pEvent = "onEnd";
                break;
            case EventTrigger::ON_CLICK:
                pEvent = "onClick";
                break;
            case EventTrigger::ON_DBL_CLICK:
                pEvent = "onDblClick";
                break;
            case EventTrigger::ON_STOP_AUDIO:
                pEvent = "onStopAudio";
                break;
            case EventTrigger::ON_MOUSE_ENTER:
                pEvent = "onMouseOver";   // not exact?
                break;
            case EventTrigger::ON_MOUSE_LEAVE:
                pEvent = "onMouseOut";
                break;
            }
        }

        if (aEvent.Offset >>= fDelay)
        {
            bHasFDelay = true;
            SAL_INFO("sd.eppt", "event offset: " << fDelay);
        }
        else if (aEvent.Offset >>= eTiming)
        {
            if (eTiming == Timing_INDEFINITE)
                pDelay = "indefinite";
            SAL_INFO("sd.eppt", "event offset timing: " << static_cast<int>(eTiming));
        }
    }
    else if (rAny >>= xShape)
    {
        SAL_INFO("sd.eppt", "Got the xShape: " << xShape->getShapeType());
        if (xShape->getShapeType() == "com.sun.star.drawing.MediaShape" || xShape->getShapeType() == "com.sun.star.presentation.MediaShape")
        {
            // write the default
            bHasFDelay = true;
        }
    }

    WriteAnimationCondition(pFS, pDelay, pEvent, fDelay, bHasFDelay, nToken);
}

void PowerPointExport::WriteAnimationNodeCommonPropsStart(const FSHelperPtr& pFS, const Reference< XAnimationNode >& rXNode, bool bSingle, bool bMainSeqChild)
{
    const char* pDuration = nullptr;
    const char* pRestart = nullptr;
    const char* pNodeType = nullptr;
    const char* pPresetClass = nullptr;
    const char* pFill = nullptr;
    double fDuration = 0;
    Any aAny;

    aAny = rXNode->getDuration();
    if (aAny.hasValue())
    {
        Timing eTiming;

        if (aAny >>= eTiming)
        {
            if (eTiming == Timing_INDEFINITE)
                pDuration = "indefinite";
        }
        else
            aAny >>= fDuration;
    }

    switch (rXNode->getRestart())
    {
    case AnimationRestart::ALWAYS:
        pRestart = "always";
        break;
    case AnimationRestart::WHEN_NOT_ACTIVE:
        pRestart = "whenNotActive";
        break;
    case AnimationRestart::NEVER:
        pRestart = "never";
        break;
    }

    const Sequence< NamedValue > aUserData = rXNode->getUserData();
    const Any* pAny[ DFF_ANIM_PROPERTY_ID_COUNT ];
    AnimationExporter::GetUserData(aUserData, pAny, sizeof(pAny));

    sal_Int16 nType = 0;
    if (pAny[ DFF_ANIM_NODE_TYPE ] && (*pAny[ DFF_ANIM_NODE_TYPE ] >>= nType))
    {
        switch (nType)
        {
        case EffectNodeType::TIMING_ROOT:
            pNodeType = "tmRoot";
            if (!pDuration)
                pDuration = "indefinite";
            if (!pRestart)
                pRestart = "never";
            break;
        case EffectNodeType::MAIN_SEQUENCE:
            pNodeType = "mainSeq";
            pDuration = "indefinite";
            break;
        case EffectNodeType::ON_CLICK:
            pNodeType = "clickEffect";
            break;
        case EffectNodeType::AFTER_PREVIOUS:
            pNodeType = "afterEffect";
            break;
        case EffectNodeType::WITH_PREVIOUS:
            pNodeType = "withEffect";
            break;
        case EffectNodeType::INTERACTIVE_SEQUENCE:
            pNodeType = "interactiveSeq";
            break;
        }
    }

    sal_uInt32 nPresetClass = DFF_ANIM_PRESS_CLASS_USER_DEFINED;
    if (pAny[ DFF_ANIM_PRESET_CLASS ])
    {
        if (*pAny[ DFF_ANIM_PRESET_CLASS ] >>= nPresetClass)
        {
            switch (nPresetClass)
            {
            case EffectPresetClass::ENTRANCE:
                pPresetClass = "entr";
                break;
            case EffectPresetClass::EXIT:
                pPresetClass = "exit";
                break;
            case EffectPresetClass::EMPHASIS:
                pPresetClass = "emph";
                break;
            case EffectPresetClass::MOTIONPATH:
                pPresetClass = "path";
                break;
            case EffectPresetClass::OLEACTION:
                pPresetClass = "verb";  // ?
                break;
            case EffectPresetClass::MEDIACALL:
                pPresetClass = "mediacall";
                break;
            }
        }
    }

    sal_uInt32 nPresetId = 0;
    bool bPresetId = false;
    if (pAny[ DFF_ANIM_PRESET_ID ])
    {
        OUString sPreset;
        if (*pAny[ DFF_ANIM_PRESET_ID ] >>= sPreset)
            nPresetId = AnimationExporter::GetPresetID(sPreset, nPresetClass, bPresetId);
    }

    sal_uInt32 nPresetSubType = 0;
    bool bPresetSubType = false;
    if (pAny[ DFF_ANIM_PRESET_SUB_TYPE ])
    {
        OUString sPresetSubType;
        if (*pAny[ DFF_ANIM_PRESET_SUB_TYPE ] >>= sPresetSubType)
        {
            nPresetSubType = AnimationExporter::TranslatePresetSubType(nPresetClass, nPresetId, sPresetSubType);
            bPresetSubType = true;
        }
    }

    if (nType != EffectNodeType::TIMING_ROOT && nType != EffectNodeType::MAIN_SEQUENCE)
    {
        // it doesn't seem to work right on root and mainseq nodes
        sal_Int16 nFill = AnimationExporter::GetFillMode(rXNode, AnimationFill::AUTO);
        switch (nFill)
        {
        case AnimationFill::FREEZE:
            pFill = "hold";
            break;
        case AnimationFill::HOLD:
            pFill = "hold";
            break;
        case AnimationFill::REMOVE:
            pFill = "remove";
            break;
        case AnimationFill::TRANSITION:
            pFill = "transition";
            break;
        }
    }

    bool bAutoReverse = rXNode->getAutoReverse();

    pFS->startElementNS(XML_p, XML_cTn,
                        XML_id, I64S(mnAnimationNodeIdMax ++),
                        XML_dur, fDuration != 0 ? I32S(static_cast<sal_Int32>(fDuration * 1000.0)) : pDuration,
                        XML_autoRev, bAutoReverse ? "1" : nullptr,
                        XML_restart, pRestart,
                        XML_nodeType, pNodeType,
                        XML_fill, pFill,
                        XML_presetClass, pPresetClass,
                        XML_presetID, bPresetId ? I64S(nPresetId) : nullptr,
                        XML_presetSubtype, bPresetSubType ? I64S(nPresetSubType) : nullptr,
                        FSEND);

    aAny = rXNode->getBegin();
    if (aAny.hasValue())
    {
        Sequence< Any > aCondSeq;

        if (aAny >>= aCondSeq)
        {
            for (int i = 0; i < aCondSeq.getLength(); i ++)
                WriteAnimationCondition(pFS, aCondSeq[ i ], false, bMainSeqChild, XML_stCondLst);
        }
        else
            WriteAnimationCondition(pFS, aAny, false, bMainSeqChild, XML_stCondLst);
    }

    aAny = rXNode->getEnd();
    if (aAny.hasValue())
    {
        Sequence< Any > aCondSeq;

        if (aAny >>= aCondSeq)
        {
            for (int i = 0; i < aCondSeq.getLength(); i ++)
                WriteAnimationCondition(pFS, aCondSeq[ i ], false, bMainSeqChild, XML_endCondLst);
        }
        else
            WriteAnimationCondition(pFS, aAny, false, bMainSeqChild, XML_endCondLst);
    }

    Reference< XEnumerationAccess > xEnumerationAccess(rXNode, UNO_QUERY);
    if (xEnumerationAccess.is())
    {
        Reference< XEnumeration > xEnumeration(xEnumerationAccess->createEnumeration(), UNO_QUERY);
        if (xEnumeration.is())
        {
            SAL_INFO("sd.eppt", "-----");

            if (xEnumeration->hasMoreElements())
            {
                pFS->startElementNS(XML_p, XML_childTnLst, FSEND);

                do
                {
                    Reference< XAnimationNode > xChildNode(xEnumeration->nextElement(), UNO_QUERY);
                    if (xChildNode.is())
                        WriteAnimationNode(pFS, xChildNode, nType == EffectNodeType::MAIN_SEQUENCE);
                }
                while (xEnumeration->hasMoreElements());

                pFS->endElementNS(XML_p, XML_childTnLst);
            }
            SAL_INFO("sd.eppt", "-----");
        }
    }

    if (bSingle)
        pFS->endElementNS(XML_p, XML_cTn);
}

void PowerPointExport::WriteAnimationNodeSeq(const FSHelperPtr& pFS, const Reference< XAnimationNode >& rXNode, sal_Int32, bool bMainSeqChild)
{
    SAL_INFO("sd.eppt", "write animation node SEQ");

    pFS->startElementNS(XML_p, XML_seq, FSEND);

    WriteAnimationNodeCommonPropsStart(pFS, rXNode, true, bMainSeqChild);

    WriteAnimationCondition(pFS, nullptr, "onPrev", 0, true, XML_prevCondLst);
    WriteAnimationCondition(pFS, nullptr, "onNext", 0, true, XML_nextCondLst);

    pFS->endElementNS(XML_p, XML_seq);
}

void PowerPointExport::WriteAnimationNodeEffect(const FSHelperPtr& pFS, const Reference< XAnimationNode >& rXNode, sal_Int32, bool bMainSeqChild)
{
    SAL_INFO("sd.eppt", "write animation node FILTER");
    Reference< XTransitionFilter > xFilter(rXNode, UNO_QUERY);
    if (xFilter.is())
    {
        const char* pFilter = ::ppt::AnimationExporter::FindTransitionName(xFilter->getTransition(), xFilter->getSubtype(), xFilter->getDirection());
        const char* pMode = xFilter->getMode() ? "in" : "out";
        pFS->startElementNS(XML_p, XML_animEffect,
                            XML_filter, pFilter,
                            XML_transition, pMode,
                            FSEND);

        WriteAnimationNodeAnimateInside(pFS, rXNode, bMainSeqChild, false);

        pFS->endElementNS(XML_p, XML_animEffect);
    }
}

void PowerPointExport::WriteAnimationNodeCommand(const FSHelperPtr& pFS, const Reference< XAnimationNode >& rXNode, sal_Int32, bool bMainSeqChild)
{
    SAL_INFO("sd.eppt", "write animation node COMMAND");
    Reference<XCommand> xCommand(rXNode, UNO_QUERY);
    if (xCommand.is())
    {
        const char* pType = "call";
        const char* pCommand = nullptr;
        switch (xCommand->getCommand())
        {
        case EffectCommands::VERB:        pType = "verb"; pCommand = "1"; /* FIXME hardcoded viewing */ break;
        case EffectCommands::PLAY:        pCommand = "play"; break;
        case EffectCommands::TOGGLEPAUSE: pCommand = "togglePause"; break;
        case EffectCommands::STOP:        pCommand = "stop"; break;
        default:
            SAL_WARN("sd.eppt", "unknown command: " << xCommand->getCommand());
            break;
        }

        pFS->startElementNS(XML_p, XML_cmd,
                            XML_type, pType,
                            XML_cmd, pCommand,
                            FSEND);

        WriteAnimationNodeAnimateInside(pFS, rXNode, bMainSeqChild, false);
        pFS->startElementNS(XML_p, XML_cBhvr,
                            FSEND);
        WriteAnimationNodeCommonPropsStart(pFS, rXNode, true, bMainSeqChild);
        WriteAnimationTarget(pFS, xCommand->getTarget());
        pFS->endElementNS(XML_p, XML_cBhvr);

        pFS->endElementNS(XML_p, XML_cmd);
    }
}

void PowerPointExport::WriteAnimationNode(const FSHelperPtr& pFS, const Reference< XAnimationNode >& rXNode, bool bMainSeqChild)
{
    SAL_INFO("sd.eppt", "export node type: " << rXNode->getType());
    sal_Int32 xmlNodeType = -1;
    typedef void (PowerPointExport::*AnimationNodeWriteMethod)(const FSHelperPtr&, const Reference< XAnimationNode >&, sal_Int32, bool);
    AnimationNodeWriteMethod pMethod = nullptr;

    switch (rXNode->getType())
    {
    case AnimationNodeType::PAR:
        xmlNodeType = XML_par;
        break;
    case AnimationNodeType::SEQ:
        pMethod = &PowerPointExport::WriteAnimationNodeSeq;
        break;
    case AnimationNodeType::ANIMATE:
        xmlNodeType = XML_anim;
        pMethod = &PowerPointExport::WriteAnimationNodeAnimate;
        break;
    case AnimationNodeType::ANIMATEMOTION:
        xmlNodeType = XML_animMotion;
        pMethod = &PowerPointExport::WriteAnimationNodeAnimate;
        break;
    case AnimationNodeType::ANIMATETRANSFORM:
        {
            Reference<XAnimateTransform> xTransform(rXNode, UNO_QUERY);
            if (xTransform.is())
            {
                if (xTransform->getTransformType() == AnimationTransformType::SCALE)
                {
                    SAL_WARN("sd.eppt", "SCALE transform type not handled");
                }
                else if (xTransform->getTransformType() == AnimationTransformType::ROTATE)
                {
                    xmlNodeType = XML_animRot;
                    pMethod = &PowerPointExport::WriteAnimationNodeAnimate;
                }
            }
        }
        break;
    case AnimationNodeType::ANIMATECOLOR:
        xmlNodeType = XML_animClr;
        pMethod = &PowerPointExport::WriteAnimationNodeAnimate;
        break;
    case AnimationNodeType::SET:
        xmlNodeType = XML_set;
        pMethod = &PowerPointExport::WriteAnimationNodeAnimate;
        break;
    case AnimationNodeType::TRANSITIONFILTER:
        xmlNodeType = XML_animEffect;
        pMethod = &PowerPointExport::WriteAnimationNodeEffect;
        break;
    case AnimationNodeType::COMMAND:
        xmlNodeType = XML_cmd;
        pMethod = &PowerPointExport::WriteAnimationNodeCommand;
        break;
    default:
        SAL_WARN("sd.eppt", "unhandled animation node: " << rXNode->getType());
        break;
    }

    if (pMethod)
    {
        (this->*pMethod)(pFS, rXNode, xmlNodeType, bMainSeqChild);
        return;
    }

    if (xmlNodeType == -1)
        return;

    pFS->startElementNS(XML_p, xmlNodeType, FSEND);

    WriteAnimationNodeCommonPropsStart(pFS, rXNode, true, bMainSeqChild);

    pFS->endElementNS(XML_p, xmlNodeType);
}

void PowerPointExport::WriteAnimations(const FSHelperPtr& pFS)
{
    Reference< XAnimationNodeSupplier > xNodeSupplier(mXDrawPage, UNO_QUERY);
    if (xNodeSupplier.is())
    {
        const Reference< XAnimationNode > xNode(xNodeSupplier->getAnimationNode());
        if (xNode.is())
        {
            Reference< XEnumerationAccess > xEnumerationAccess(xNode, UNO_QUERY);
            if (xEnumerationAccess.is())
            {
                Reference< XEnumeration > xEnumeration(xEnumerationAccess->createEnumeration(), UNO_QUERY);
                if (xEnumeration.is() && xEnumeration->hasMoreElements())
                {

                    pFS->startElementNS(XML_p, XML_timing, FSEND);
                    pFS->startElementNS(XML_p, XML_tnLst, FSEND);

                    WriteAnimationNode(pFS, xNode, false);

                    pFS->endElementNS(XML_p, XML_tnLst);
                    pFS->endElementNS(XML_p, XML_timing);
                }
            }
        }
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
                char cDateTime[32];

                snprintf(cDateTime, 31, "%02d-%02d-%02dT%02d:%02d:%02d.%09" SAL_PRIuUINT32, aDateTime.Year, aDateTime.Month, aDateTime.Day, aDateTime.Hours, aDateTime.Minutes, aDateTime.Seconds, aDateTime.NanoSeconds);

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

    if (GETA(Visible))
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
    WriteAnimations(pFS);

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
    if (mpSlidesFSArray.size() >= nPageNum)
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

void PowerPointExport::WriteDefaultColorSchemes(FSHelperPtr pFS)
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

bool PowerPointExport::WriteColorSchemes(FSHelperPtr pFS, const OUString& rThemePath)
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

bool PowerPointExport::WriteNotesMaster()
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

    return true;
}

bool PowerPointExport::ImplCreateMainNotes()
{
    if (mbCreateNotes)
        return WriteNotesMaster();

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
