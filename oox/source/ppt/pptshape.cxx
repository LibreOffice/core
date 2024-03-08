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

#include <oox/ppt/pptshape.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <drawingml/customshapeproperties.hxx>
#include <drawingml/textbody.hxx>
#include <drawingml/textparagraph.hxx>
#include <drawingml/textfield.hxx>
#include <editeng/flditem.hxx>

#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <sal/log.hxx>
#include <oox/ppt/slidepersist.hxx>
#include <oox/token/tokens.hxx>
#include <oox/token/properties.hxx>
#include <o3tl/string_view.hxx>

using namespace ::oox::core;
using namespace ::oox::drawingml;
using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::presentation;

namespace oox::ppt {

PPTShape::PPTShape( const oox::ppt::ShapeLocation eShapeLocation, const OUString& rServiceName )
: Shape( rServiceName )
, meShapeLocation( eShapeLocation )
, mbReferenced( false )
, mbHasNoninheritedShapeProperties( false )
{
}

PPTShape::~PPTShape()
{
}

static const char* lclDebugSubType( sal_Int32 nType )
{
    switch (nType) {
        case XML_ctrTitle :
            return "ctrTitle";
        case XML_title :
            return "title";
        case XML_subTitle :
            return "subTitle";
        case XML_obj :
            return "obj";
        case XML_body :
            return "body";
        case XML_dt :
            return "dt";
        case XML_hdr :
            return "hdr";
        case XML_ftr :
            return "frt";
        case XML_sldNum :
            return "sldNum";
        case XML_sldImg :
            return "sldImg";
    }

    return "unknown - please extend lclDebugSubType";
}

namespace
{
bool ShapeHasNoVisualPropertiesOnImport(const oox::ppt::PPTShape& rPPTShape)
{
    return  !rPPTShape.hasNonInheritedShapeProperties()
            && !rPPTShape.hasShapeStyleRefs()
            && !rPPTShape.getTextBody()->hasVisualRunProperties()
            && !rPPTShape.getTextBody()->hasNoninheritedBodyProperties()
            && !rPPTShape.getTextBody()->hasListStyleOnImport()
            && !rPPTShape.getTextBody()->hasParagraphProperties();
}
}

oox::drawingml::TextListStylePtr PPTShape::getSubTypeTextListStyle( const SlidePersist& rSlidePersist, sal_Int32 nSubType )
{
    oox::drawingml::TextListStylePtr pTextListStyle;

    SAL_INFO("oox.ppt", "subtype style: " << lclDebugSubType( nSubType ) );

    switch( nSubType )
    {
        case XML_ctrTitle :
        case XML_title :
            pTextListStyle = rSlidePersist.getMasterPersist() ? rSlidePersist.getMasterPersist()->getTitleTextStyle() : rSlidePersist.getTitleTextStyle();
            break;
        case XML_subTitle :
        case XML_obj :
        case XML_body :
            if ( rSlidePersist.isNotesPage() )
                pTextListStyle = rSlidePersist.getMasterPersist() ? rSlidePersist.getMasterPersist()->getNotesTextStyle() : rSlidePersist.getNotesTextStyle();
            else
                pTextListStyle = rSlidePersist.getMasterPersist() ? rSlidePersist.getMasterPersist()->getBodyTextStyle() : rSlidePersist.getBodyTextStyle();
            break;
    }

    return pTextListStyle;
}

bool PPTShape::IsPlaceHolderCandidate(const SlidePersist& rSlidePersist) const
{
    if (meShapeLocation != Slide)
        return false;
    if (rSlidePersist.isNotesPage())
        return false;
    auto pTextBody = getTextBody();
    if (!pTextBody)
        return false;
    auto rParagraphs = pTextBody->getParagraphs();
    if (rParagraphs.size() != 1)
        return false;
    if (rParagraphs.front()->getRuns().size() != 1)
        return false;
    // If the placeholder has a shape other than rectangle,
    // we have to place it in the slide as a CustomShape.
    if (!mpCustomShapePropertiesPtr->representsDefaultShape())
        return false;
    return ShapeHasNoVisualPropertiesOnImport(*this);
}

void PPTShape::addShape(
        oox::core::XmlFilterBase& rFilterBase,
        const SlidePersist& rSlidePersist,
        const oox::drawingml::Theme* pTheme,
        const Reference< XShapes >& rxShapes,
        basegfx::B2DHomMatrix& aTransformation,
        ::oox::drawingml::ShapeIdMap* pShapeMap )
{
    SAL_INFO("oox.ppt","add shape id: " << msId << " location: " << ((meShapeLocation == Master) ? "master" : ((meShapeLocation == Slide) ? "slide" : ((meShapeLocation == Layout) ? "layout" : "other"))) << " subtype: " << mnSubType << " service: " << msServiceName);
    // only placeholder from layout are being inserted
    if ( mnSubType && ( meShapeLocation == Master ) )
        return;

    OUString sServiceName( msServiceName );
    if (sServiceName.isEmpty())
        return;
    try
    {
        oox::drawingml::TextListStylePtr aMasterTextListStyle;
        Reference<lang::XMultiServiceFactory> xServiceFact(rFilterBase.getModel(), UNO_QUERY_THROW);
        bool bClearText = false;

        if (sServiceName != "com.sun.star.drawing.GraphicObjectShape" &&
            sServiceName != "com.sun.star.drawing.OLE2Shape")
        {
            static constexpr OUString sOutlinerShapeService(u"com.sun.star.presentation.OutlinerShape"_ustr);
            SAL_INFO("oox.ppt","has master: " << std::hex << rSlidePersist.getMasterPersist().get());
            switch (mnSubType)
            {
                case XML_ctrTitle :
                case XML_title :
                {
                    sServiceName = "com.sun.star.presentation.TitleTextShape";
                    aMasterTextListStyle = rSlidePersist.getMasterPersist() ? rSlidePersist.getMasterPersist()->getTitleTextStyle() : rSlidePersist.getTitleTextStyle();
                }
                break;
                case XML_subTitle :
                {
                    if ((meShapeLocation == Master) || (meShapeLocation == Layout))
                        sServiceName = OUString();
                    else {
                        sServiceName = "com.sun.star.presentation.SubtitleShape";
                        aMasterTextListStyle = rSlidePersist.getMasterPersist() ? rSlidePersist.getMasterPersist()->getBodyTextStyle() : rSlidePersist.getBodyTextStyle();
                    }
                }
                break;
                   case XML_obj :
                {
                    sServiceName = sOutlinerShapeService;
                    if (getSubTypeIndex().has_value())
                        aMasterTextListStyle = rSlidePersist.getMasterPersist() ? rSlidePersist.getMasterPersist()->getBodyTextStyle() : rSlidePersist.getBodyTextStyle();
                }
                break;
                case XML_body :
                {
                    if (rSlidePersist.isNotesPage())
                    {
                        sServiceName = "com.sun.star.presentation.NotesShape";
                        aMasterTextListStyle = rSlidePersist.getMasterPersist() ? rSlidePersist.getMasterPersist()->getNotesTextStyle() : rSlidePersist.getNotesTextStyle();
                    }
                    else
                    {
                        sServiceName = sOutlinerShapeService;
                        aMasterTextListStyle = rSlidePersist.getMasterPersist() ? rSlidePersist.getMasterPersist()->getBodyTextStyle() : rSlidePersist.getBodyTextStyle();
                    }
                }
                break;
                case XML_dt :
                    if (IsPlaceHolderCandidate(rSlidePersist))
                    {
                        TextRunPtr& pTextRun = getTextBody()->getParagraphs().front()->getRuns().front();
                        oox::drawingml::TextField* pTextField = dynamic_cast<oox::drawingml::TextField*>(pTextRun.get());
                        if (pTextField)
                        {
                            OUString aType = pTextField->getType();
                            if ( aType.startsWith("datetime") )
                            {
                                SvxDateFormat eDateFormat = drawingml::TextField::getLODateFormat(aType);
                                SvxTimeFormat eTimeFormat = drawingml::TextField::getLOTimeFormat(aType);
                                Reference< XPropertySet > xPropertySet( rSlidePersist.getPage(), UNO_QUERY );

                                if( eDateFormat != SvxDateFormat::AppDefault
                                    || eTimeFormat != SvxTimeFormat::AppDefault )
                                {
                                    // DateTimeFormat property looks for the date in 4 LSBs
                                    // and looks for time format in the 4 bits after that
                                    sal_Int32 nDateTimeFormat = static_cast<sal_Int32>(eDateFormat) |
                                                                static_cast<sal_Int32>(eTimeFormat) << 4;
                                    xPropertySet->setPropertyValue( "IsDateTimeVisible", Any(true) );
                                    xPropertySet->setPropertyValue( "IsDateTimeFixed", Any(false) );
                                    xPropertySet->setPropertyValue( "DateTimeFormat", Any(nDateTimeFormat) );
                                    return;
                                }
                            }
                        }
                    }
                    sServiceName = "com.sun.star.presentation.DateTimeShape";
                    bClearText = true;
                break;
                case XML_hdr :
                    sServiceName = "com.sun.star.presentation.HeaderShape";
                    bClearText = true;
                break;
                case XML_ftr :
                    if (IsPlaceHolderCandidate(rSlidePersist))
                    {
                        const OUString& rFooterText = getTextBody()->toString();

                        if( !rFooterText.isEmpty() )
                        {
                            // if it is possible to get the footer as a property the LO way,
                            // get it and discard the shape
                            Reference< XPropertySet > xPropertySet( rSlidePersist.getPage(), UNO_QUERY );
                            xPropertySet->setPropertyValue( "IsFooterVisible", Any( true ) );
                            xPropertySet->setPropertyValue( "FooterText", Any(rFooterText) );
                            return;
                        }
                    }
                    sServiceName = "com.sun.star.presentation.FooterShape";
                    bClearText = true;
                break;
                case XML_sldNum :
                    if (IsPlaceHolderCandidate(rSlidePersist))
                    {
                        TextRunPtr& pTextRun
                            = getTextBody()->getParagraphs().front()->getRuns().front();
                        oox::drawingml::TextField* pTextField
                            = dynamic_cast<oox::drawingml::TextField*>(pTextRun.get());
                        if (pTextField && pTextField->getType() == "slidenum")
                        {
                            // if it is possible to get the slidenum placeholder as a property
                            // do that and discard the shape
                            Reference<XPropertySet> xPropertySet(rSlidePersist.getPage(),
                                                                 UNO_QUERY);
                            xPropertySet->setPropertyValue("IsPageNumberVisible", Any(true));
                            return;
                        }
                    }
                    sServiceName = "com.sun.star.presentation.SlideNumberShape";
                    bClearText = true;
                break;
                case XML_sldImg :
                    sServiceName = "com.sun.star.presentation.PageShape";
                break;
                case XML_chart :
                    if (meShapeLocation == Layout)
                        sServiceName = sOutlinerShapeService;
                    else
                        sServiceName = "com.sun.star.presentation.ChartShape";
                break;
                case XML_tbl :
                    if (meShapeLocation == Layout)
                        sServiceName = sOutlinerShapeService;
                    else
                        sServiceName = "com.sun.star.presentation.TableShape";
                break;
                case XML_pic :
                    if (meShapeLocation == Layout)
                        sServiceName = sOutlinerShapeService;
                    else
                        sServiceName = "com.sun.star.presentation.GraphicObjectShape";
                break;
                case XML_media :
                    if (meShapeLocation == Layout)
                        sServiceName = sOutlinerShapeService;
                    else
                        sServiceName = "com.sun.star.presentation.MediaShape";
                break;
                default:
                    if (mnSubType && meShapeLocation == Layout)
                        sServiceName = sOutlinerShapeService;
                break;
            }
        }

        // Since it is not possible to represent custom shaped placeholders in Impress
        // Need to use service name css.drawing.CustomShape if they have a non default shape.
        // This workaround has the drawback of them not really being processed as placeholders
        // so it is only done for slide footers...
        bool convertInSlideMode = meShapeLocation == Slide &&
            (mnSubType == XML_sldNum || mnSubType == XML_dt || mnSubType == XML_ftr || mnSubType == XML_body);
        bool convertInLayoutMode = meShapeLocation == Layout && (mnSubType == XML_body);
        if ((convertInSlideMode || convertInLayoutMode) && !mpCustomShapePropertiesPtr->representsDefaultShape())
        {
            sServiceName = "com.sun.star.drawing.CustomShape";
        }

        SAL_INFO("oox.ppt","shape service: " << sServiceName);

        if (mnSubType && getSubTypeIndex().has_value() && meShapeLocation == Layout)
        {
            oox::drawingml::ShapePtr pPlaceholder = PPTShape::findPlaceholderByIndex( getSubTypeIndex().value(), rSlidePersist.getShapes()->getChildren(), true );
            if (!pPlaceholder)
                pPlaceholder = PPTShape::findPlaceholder( mnSubType, 0, getSubTypeIndex(), rSlidePersist.getShapes()->getChildren(), true );

            if (pPlaceholder) {
                if (maSize.Width == 0 || maSize.Height == 0) {
                    awt::Size aSize = maSize;
                    if (maSize.Width == 0)
                        aSize.Width = pPlaceholder->getSize().Width;
                    if (maSize.Height == 0)
                        aSize.Height = pPlaceholder->getSize().Height;
                    setSize( aSize );
                    if (maPosition.X == 0 || maPosition.Y == 0) {
                        awt::Point aPosition = maPosition;
                        if (maPosition.X == 0)
                            aPosition.X = pPlaceholder->getPosition().X;
                        if (maPosition.Y == 0)
                            aPosition.Y = pPlaceholder->getPosition().Y;
                        setPosition( aPosition );
                    }
                }
            }
        }

        // use placeholder index if possible
        if (mnSubType && getSubTypeIndex().has_value() && rSlidePersist.getMasterPersist())
        {
            oox::drawingml::ShapePtr pPlaceholder = PPTShape::findPlaceholderByIndex(getSubTypeIndex().value(), rSlidePersist.getMasterPersist()->getShapes()->getChildren());
            // TODO: Check if this is required for non-notes slides as well...
            if (rSlidePersist.isNotesPage() && pPlaceholder && pPlaceholder->getSubType() != getSubType())
                pPlaceholder.reset();

            if (pPlaceholder) {
                SAL_INFO("oox.ppt","found placeholder with index: " << getSubTypeIndex().value() << " and type: " << lclDebugSubType( mnSubType ));
                PPTShape* pPPTPlaceholder = dynamic_cast< PPTShape* >( pPlaceholder.get() );
                TextListStylePtr pNewTextListStyle = std::make_shared<TextListStyle>();

                if (pPlaceholder->getTextBody()) {

                    pNewTextListStyle->apply( pPlaceholder->getTextBody()->getTextListStyle() );
                    if (pPlaceholder->getMasterTextListStyle())
                        pNewTextListStyle->apply( *pPlaceholder->getMasterTextListStyle() );

                    // SAL_INFO("oox.ppt","placeholder body style");
                    // pPlaceholder->getTextBody()->getTextListStyle().dump();
                    // SAL_INFO("oox.ppt","master text list style");
                    // pPlaceholder->getMasterTextListStyle()->dump();

                    aMasterTextListStyle = pNewTextListStyle;
                    // SAL_INFO("oox.ppt","combined master text list style");
                    // aMasterTextListStyle->dump();
                }
                if (pPPTPlaceholder && pPPTPlaceholder->mpPlaceholder) {
                    SAL_INFO("oox.ppt","placeholder has parent placeholder: " << pPPTPlaceholder->mpPlaceholder->getId() << " type: " << lclDebugSubType( pPPTPlaceholder->mpPlaceholder->getSubType() ) << " index: " << pPPTPlaceholder->mpPlaceholder->getSubTypeIndex().value() );
                    SAL_INFO("oox.ppt","has textbody " << (pPPTPlaceholder->mpPlaceholder->getTextBody() != nullptr) );
                    TextListStylePtr pPlaceholderStyle = getSubTypeTextListStyle( rSlidePersist, pPPTPlaceholder->mpPlaceholder->getSubType() );
                    if (pPPTPlaceholder->mpPlaceholder->getTextBody())
                        pNewTextListStyle->apply( pPPTPlaceholder->mpPlaceholder->getTextBody()->getTextListStyle() );
                    if (pPlaceholderStyle) {
                        pNewTextListStyle->apply( *pPlaceholderStyle );
                        //pPlaceholderStyle->dump();
                    }
                }
            } else if (!mpPlaceholder) {
                aMasterTextListStyle.reset();
            }
            SAL_INFO("oox.ppt","placeholder id: " << (pPlaceholder ? pPlaceholder->getId() : "not found"));
        }

        if (!sServiceName.isEmpty())
        {
            if (!aMasterTextListStyle)
            {
                bool isOther = !getTextBody() && sServiceName != "com.sun.star.drawing.GroupShape";
                TextListStylePtr aSlideStyle = isOther ? rSlidePersist.getOtherTextStyle() : rSlidePersist.getDefaultTextStyle();
                // Combine from MasterSlide details as well.
                if (rSlidePersist.getMasterPersist())
                {
                    aMasterTextListStyle = isOther ? rSlidePersist.getMasterPersist()->getOtherTextStyle() : rSlidePersist.getMasterPersist()->getDefaultTextStyle();
                    if (aSlideStyle)
                        aMasterTextListStyle->apply( *aSlideStyle );
                }
                else
                {
                    aMasterTextListStyle = aSlideStyle;
                }
            }

            if( aMasterTextListStyle && getTextBody() ) {
                TextListStylePtr aCombinedTextListStyle = std::make_shared<TextListStyle>();

                aCombinedTextListStyle->apply( *aMasterTextListStyle );

                if( mpPlaceholder && mpPlaceholder->getTextBody() )
                    aCombinedTextListStyle->apply( mpPlaceholder->getTextBody()->getTextListStyle() );
                aCombinedTextListStyle->apply( getTextBody()->getTextListStyle() );

                setMasterTextListStyle( aCombinedTextListStyle );
            } else
                setMasterTextListStyle( aMasterTextListStyle );

            Reference< XShape > xShape( createAndInsert( rFilterBase, sServiceName, pTheme, rxShapes, bClearText, bool(mpPlaceholder), aTransformation, getFillProperties() ) );

            // Apply text properties on placeholder text inside this placeholder shape
            if (meShapeLocation == Slide && mpPlaceholder && getTextBody() && getTextBody()->isEmpty())
            {
                Reference < XText > xText(mxShape, UNO_QUERY);
                if (xText.is())
                {
                    TextCharacterProperties aCharStyleProperties;
                    getTextBody()->ApplyStyleEmpty(rFilterBase, xText, aCharStyleProperties, mpMasterTextListStyle);
                }
            }
            if (pShapeMap)
            {
                // bnc#705982 - if optional model id reference is
                // there, use that to obtain target shape
                if (!msModelId.isEmpty())
                {
                    (*pShapeMap)[ msModelId ] = shared_from_this();
                }
                else if (!msId.isEmpty())
                {
                    (*pShapeMap)[ msId ] = shared_from_this();
                }
            }

            // we will be losing whatever information there is in the footer placeholder on master/layout slides
            // since they should have the "<footer>" textfield in them in order to make LibreOffice process them as expected
            // likewise DateTime placeholder data on master/layout slides will be lost and replaced
            if( (mnSubType == XML_ftr || mnSubType == XML_dt) && meShapeLocation != Slide )
            {
                OUString aFieldType;
                if( mnSubType == XML_ftr )
                    aFieldType = "com.sun.star.presentation.TextField.Footer";
                else
                    aFieldType = "com.sun.star.presentation.TextField.DateTime";
                Reference < XTextField > xField( xServiceFact->createInstance( aFieldType ), UNO_QUERY );
                Reference < XText > xText(mxShape, UNO_QUERY);
                if(xText.is())
                {
                    xText->setString("");
                    Reference < XTextCursor > xTextCursor = xText->createTextCursor();
                    xText->insertTextContent( xTextCursor, xField, false);
                }
            }

            OUString sURL;
            std::vector<std::pair<OUString, Reference<XShape>>> aURLShapes;
            // if this is a group shape, we have to add also each child shape
            Reference<XShapes> xShapes(xShape, UNO_QUERY);
            if (xShapes.is())
            {
                // temporarily remember setting
                NamedShapePairs* pDiagramFontHeights(rFilterBase.getDiagramFontHeights());

                // for shapes unequal to FRAMETYPE_DIAGRAM do
                // disable DiagramFontHeights recording
                if (meFrameType != FRAMETYPE_DIAGRAM)
                    rFilterBase.setDiagramFontHeights(nullptr);

                addChildren( rFilterBase, *this, pTheme, xShapes, pShapeMap, aTransformation );

                // restore remembered setting
                rFilterBase.setDiagramFontHeights(pDiagramFontHeights);

                for (size_t i = 0; i < this->getChildren().size(); i++)
                {
                    this->getChildren()[i]->getShapeProperties().getProperty(PROP_URL) >>= sURL;
                    if (!sURL.isEmpty())
                    {
                        Reference<XShape> xChild = this->getChildren()[i]->getXShape();
                        aURLShapes.push_back({ sURL, xChild });
                    }
                }
            }

            if (meFrameType == FRAMETYPE_DIAGRAM)
            {
                keepDiagramCompatibilityInfo();
            }

            // Support advanced DiagramHelper
            if (FRAMETYPE_DIAGRAM == meFrameType)
            {
                propagateDiagramHelper();
            }

            getShapeProperties().getProperty(PROP_URL) >>= sURL;
            if (!sURL.isEmpty() && !xShapes.is())
                aURLShapes.push_back({ sURL, xShape });

            if (!aURLShapes.empty())
            {
                for (auto const& URLShape : aURLShapes)
                {
                    Reference<XEventsSupplier> xEventsSupplier(URLShape.second, UNO_QUERY);
                    if (!xEventsSupplier.is())
                        return;

                    Reference<XNameReplace> xEvents(xEventsSupplier->getEvents());
                    if (!xEvents.is())
                        return;

                    OUString sAPIEventName;
                    sal_Int32 nPropertyCount = 2;
                    css::presentation::ClickAction meClickAction;
                    uno::Sequence<beans::PropertyValue> aProperties;

                    std::map<OUString, css::presentation::ClickAction> ActionMap = {
                        { "#action?jump=nextslide", ClickAction_NEXTPAGE },
                        { "#action?jump=previousslide", ClickAction_PREVPAGE },
                        { "#action?jump=firstslide", ClickAction_FIRSTPAGE },
                        { "#action?jump=lastslide", ClickAction_LASTPAGE },
                        { "#action?jump=endshow", ClickAction_STOPPRESENTATION },
                    };

                    sURL = URLShape.first;
                    std::map<OUString, css::presentation::ClickAction>::const_iterator aIt
                        = ActionMap.find(sURL);
                    aIt != ActionMap.end() ? meClickAction = aIt->second
                                           : meClickAction = ClickAction_BOOKMARK;

                    // ClickAction_BOOKMARK and ClickAction_DOCUMENT share the same event
                    // so check here if it's a bookmark or a document
                    if (meClickAction == ClickAction_BOOKMARK)
                    {
                        if (!sURL.startsWith("#"))
                            meClickAction = ClickAction_DOCUMENT;
                        else
                        {
                            sURL = OUString::Concat("page")
                                   + sURL.subView(sURL.lastIndexOf(' ') + 1);
                        }
                        nPropertyCount += 1;
                    }

                    aProperties.realloc(nPropertyCount);
                    beans::PropertyValue* pProperties = aProperties.getArray();

                    pProperties->Name = "EventType";
                    pProperties->Handle = -1;
                    pProperties->Value <<= OUString("Presentation");
                    pProperties->State = beans::PropertyState_DIRECT_VALUE;
                    pProperties++;

                    pProperties->Name = "ClickAction";
                    pProperties->Handle = -1;
                    pProperties->Value <<= meClickAction;
                    pProperties->State = beans::PropertyState_DIRECT_VALUE;
                    pProperties++;

                    switch (meClickAction)
                    {
                        case ClickAction_BOOKMARK:
                        case ClickAction_DOCUMENT:
                            pProperties->Name = "Bookmark";
                            pProperties->Handle = -1;
                            pProperties->Value <<= sURL;
                            pProperties->State = beans::PropertyState_DIRECT_VALUE;
                            break;
                        default:
                            break;
                    }

                    sAPIEventName = "OnClick";
                    xEvents->replaceByName(sAPIEventName, uno::Any(aProperties));
                }
            }
        }
    }
    catch (const Exception&)
    {
    }
}

namespace
{
    bool ShapeLocationIsMaster(oox::drawingml::Shape *pInShape)
    {
        PPTShape* pShape = dynamic_cast<PPTShape*>(pInShape);
        return pShape && pShape->getShapeLocation() == Master;
    }
}

// Function to find placeholder (ph) for a shape. No idea how MSO implements this, but
// this order seems to work quite well
// (probably it's unnecessary complicated / wrong. i.e. tdf#104202):
// 1. ph with nFirstSubType and the same oSubTypeIndex
// 2. ph with nFirstSubType
// 3. ph with nSecondSubType and the same oSubTypeIndex
// 4. ph with nSecondSubType
// 5. ph with the same oSubTypeIndex

oox::drawingml::ShapePtr PPTShape::findPlaceholder( sal_Int32 nFirstSubType, sal_Int32 nSecondSubType,
    const std::optional< sal_Int32 >& oSubTypeIndex, std::vector< oox::drawingml::ShapePtr >& rShapes, bool bMasterOnly )
{
    class Placeholders
    {
    public:
        Placeholders()
            : aChoice(5) // resize to 5
        {
        }

        void add(const oox::drawingml::ShapePtr& aShape, sal_Int32 nFirstSubType, sal_Int32 nSecondSubType, const std::optional< sal_Int32 >& oSubTypeIndex)
        {
            if (!aShape)
                return;

            // get flags
            const bool bSameFirstSubType = aShape->getSubType() == nFirstSubType;
            const bool bSameSecondSubType = aShape->getSubType() == nSecondSubType;
            const bool bSameIndex = aShape->getSubTypeIndex() == oSubTypeIndex;

            // get prio
            int aPrioIndex = -1;
            if (bSameIndex && bSameFirstSubType)
                aPrioIndex = 0;
            else if (!bSameIndex && bSameFirstSubType)
                aPrioIndex = 1;
            else if (bSameIndex && bSameSecondSubType)
                aPrioIndex = 2;
            else if (!bSameIndex && bSameSecondSubType)
                aPrioIndex = 3;
            else if (bSameIndex)
                aPrioIndex = 4;

            // add
            if (aPrioIndex != -1)
            {
                if (!aChoice.at(aPrioIndex))
                {
                    aChoice.at(aPrioIndex) = aShape;
                }
            }
        }

        // return according to prio
        oox::drawingml::ShapePtr getByPrio() const
        {
            for (const oox::drawingml::ShapePtr& aShape : aChoice)
            {
                if (aShape)
                {
                    return aShape;
                }
            }

            return oox::drawingml::ShapePtr();
        }

        bool hasByPrio(size_t aIndex) const
        {
            return bool(aChoice.at(aIndex));
        }

    private:
        std::vector< oox::drawingml::ShapePtr > aChoice;

    } aPlaceholders;

    // check all shapes
    std::vector< oox::drawingml::ShapePtr >::reverse_iterator aRevIter( rShapes.rbegin() );
    for (; aRevIter != rShapes.rend(); ++aRevIter)
    {
        // check shape
        if (!bMasterOnly || ShapeLocationIsMaster((*aRevIter).get()))
        {
            const oox::drawingml::ShapePtr& aShape = *aRevIter;
            aPlaceholders.add(aShape, nFirstSubType, nSecondSubType, oSubTypeIndex);
        }

        // check children
        std::vector< oox::drawingml::ShapePtr >& rChildren = (*aRevIter)->getChildren();
        if (!rChildren.empty())
        {
            const oox::drawingml::ShapePtr aShape = findPlaceholder( nFirstSubType, nSecondSubType, oSubTypeIndex, rChildren, bMasterOnly );
            if (aShape)
            {
                aPlaceholders.add(aShape, nFirstSubType, nSecondSubType, oSubTypeIndex);
            }
        }

        if (aPlaceholders.hasByPrio(0))
        {
            break;
        }
    }

    // return something according to prio
    return aPlaceholders.getByPrio();
}

oox::drawingml::ShapePtr PPTShape::findPlaceholderByIndex( const sal_Int32 nIdx, std::vector< oox::drawingml::ShapePtr >& rShapes, bool bMasterOnly )
{
    oox::drawingml::ShapePtr aShapePtr;

    std::vector< oox::drawingml::ShapePtr >::reverse_iterator aRevIter( rShapes.rbegin() );
    while( aRevIter != rShapes.rend() )
    {
        if ( (*aRevIter)->getSubTypeIndex().has_value() && (*aRevIter)->getSubTypeIndex().value() == nIdx &&
             ( !bMasterOnly || ShapeLocationIsMaster((*aRevIter).get()) ) )
        {
            aShapePtr = *aRevIter;
            break;
        }
        std::vector< oox::drawingml::ShapePtr >& rChildren = (*aRevIter)->getChildren();
        aShapePtr = findPlaceholderByIndex( nIdx, rChildren, bMasterOnly );
        if ( aShapePtr )
            break;
        ++aRevIter;
    }
    return aShapePtr;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
