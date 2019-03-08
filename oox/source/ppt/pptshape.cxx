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
#include <drawingml/textbody.hxx>
#include <drawingml/table/tableproperties.hxx>

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/text/XText.hpp>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <sal/log.hxx>
#include <oox/ppt/slidepersist.hxx>
#include <oox/token/tokens.hxx>
#include <unotools/fltrcfg.hxx>

using namespace ::oox::core;
using namespace ::oox::drawingml;
using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;

namespace oox { namespace ppt {

PPTShape::PPTShape( const oox::ppt::ShapeLocation eShapeLocation, const sal_Char* pServiceName )
: Shape( pServiceName )
, meShapeLocation( eShapeLocation )
, mbReferenced( false )
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

oox::drawingml::TextListStylePtr PPTShape::getSubTypeTextListStyle( const SlidePersist& rSlidePersist, sal_Int32 nSubType )
{
    oox::drawingml::TextListStylePtr pTextListStyle;

    SAL_INFO("oox.ppt", "subtype style: " << lclDebugSubType( nSubType ) );

    switch( nSubType )
    {
        case XML_ctrTitle :
        case XML_title :
            pTextListStyle = rSlidePersist.getMasterPersist().get() ? rSlidePersist.getMasterPersist()->getTitleTextStyle() : rSlidePersist.getTitleTextStyle();
            break;
        case XML_subTitle :
        case XML_obj :
        case XML_body :
            if ( rSlidePersist.isNotesPage() )
                pTextListStyle = rSlidePersist.getMasterPersist().get() ? rSlidePersist.getMasterPersist()->getNotesTextStyle() : rSlidePersist.getNotesTextStyle();
            else
                pTextListStyle = rSlidePersist.getMasterPersist().get() ? rSlidePersist.getMasterPersist()->getBodyTextStyle() : rSlidePersist.getBodyTextStyle();
            break;
    }

    return pTextListStyle;
}

void PPTShape::addShape(
        oox::core::XmlFilterBase& rFilterBase,
        SlidePersist& rSlidePersist,
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
            const OUString sOutlinerShapeService("com.sun.star.presentation.OutlinerShape");
            SAL_INFO("oox.ppt","has master: " << std::hex << rSlidePersist.getMasterPersist().get());
            switch (mnSubType)
            {
                case XML_ctrTitle :
                case XML_title :
                {
                    sServiceName = "com.sun.star.presentation.TitleTextShape";
                    aMasterTextListStyle = rSlidePersist.getMasterPersist().get() ? rSlidePersist.getMasterPersist()->getTitleTextStyle() : rSlidePersist.getTitleTextStyle();
                }
                break;
                case XML_subTitle :
                {
                    if ((meShapeLocation == Master) || (meShapeLocation == Layout))
                        sServiceName = OUString();
                    else {
                        sServiceName = "com.sun.star.presentation.SubtitleShape";
                        aMasterTextListStyle = rSlidePersist.getMasterPersist().get() ? rSlidePersist.getMasterPersist()->getBodyTextStyle() : rSlidePersist.getBodyTextStyle();
                    }
                }
                break;
                   case XML_obj :
                {
                    sServiceName = sOutlinerShapeService;
                    aMasterTextListStyle = rSlidePersist.getMasterPersist().get() ? rSlidePersist.getMasterPersist()->getBodyTextStyle() : rSlidePersist.getBodyTextStyle();
                }
                break;
                case XML_body :
                {
                    if (rSlidePersist.isNotesPage())
                    {
                        sServiceName = "com.sun.star.presentation.NotesShape";
                        aMasterTextListStyle = rSlidePersist.getMasterPersist().get() ? rSlidePersist.getMasterPersist()->getNotesTextStyle() : rSlidePersist.getNotesTextStyle();
                    }
                    else
                    {
                        sServiceName = sOutlinerShapeService;
                        aMasterTextListStyle = rSlidePersist.getMasterPersist().get() ? rSlidePersist.getMasterPersist()->getBodyTextStyle() : rSlidePersist.getBodyTextStyle();
                    }
                }
                break;
                case XML_dt :
                    sServiceName = "com.sun.star.presentation.DateTimeShape";
                    bClearText = true;
                break;
                case XML_hdr :
                    sServiceName = "com.sun.star.presentation.HeaderShape";
                    bClearText = true;
                break;
                case XML_ftr :
                    sServiceName = "com.sun.star.presentation.FooterShape";
                    bClearText = true;
                break;
                case XML_sldNum :
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

        if (sServiceName != "com.sun.star.drawing.TableShape")
        {
            if (TextBodyPtr pTextBody = getTextBody())
            {
                sal_Int32 nNumCol = pTextBody->getTextProperties().mnNumCol;
                if (nNumCol > 1)
                {
                    // This shape is not a table, but has multiple columns,
                    // represent that as a table.
                    sServiceName = "com.sun.star.drawing.TableShape";
                    oox::drawingml::table::TablePropertiesPtr pTableProperties = getTableProperties();
                    pTableProperties->pullFromTextBody(pTextBody, maSize.Width);
                    setTextBody(nullptr);
                }
            }
        }

        SAL_INFO("oox.ppt","shape service: " << sServiceName);

        if (mnSubType && getSubTypeIndex().has() && meShapeLocation == Layout)
        {
            oox::drawingml::ShapePtr pPlaceholder = PPTShape::findPlaceholderByIndex( getSubTypeIndex().get(), rSlidePersist.getShapes()->getChildren(), true );
            if (!pPlaceholder.get())
                pPlaceholder = PPTShape::findPlaceholder( mnSubType, 0, getSubTypeIndex(), rSlidePersist.getShapes()->getChildren(), true );

            if (pPlaceholder.get()) {
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
        if (mnSubType && getSubTypeIndex().has() && rSlidePersist.getMasterPersist().get())
        {
            oox::drawingml::ShapePtr pPlaceholder = PPTShape::findPlaceholderByIndex(getSubTypeIndex().get(), rSlidePersist.getMasterPersist()->getShapes()->getChildren());
            // TODO: Check if this is required for non-notes slides as well...
            if (rSlidePersist.isNotesPage() && pPlaceholder.get() && pPlaceholder->getSubType() != getSubType())
                pPlaceholder.reset();

            if (pPlaceholder.get()) {
                SAL_INFO("oox.ppt","found placeholder with index: " << getSubTypeIndex().get() << " and type: " << lclDebugSubType( mnSubType ));
            }
            if (pPlaceholder.get()) {
                PPTShape* pPPTPlaceholder = dynamic_cast< PPTShape* >( pPlaceholder.get() );
                TextListStylePtr pNewTextListStyle ( new TextListStyle() );

                if (pPlaceholder->getTextBody()) {

                    pNewTextListStyle->apply( pPlaceholder->getTextBody()->getTextListStyle() );
                    if (pPlaceholder->getMasterTextListStyle().get())
                        pNewTextListStyle->apply( *pPlaceholder->getMasterTextListStyle() );

                    // SAL_INFO("oox.ppt","placeholder body style");
                    // pPlaceholder->getTextBody()->getTextListStyle().dump();
                    // SAL_INFO("oox.ppt","master text list style");
                    // pPlaceholder->getMasterTextListStyle()->dump();

                    aMasterTextListStyle = pNewTextListStyle;
                    // SAL_INFO("oox.ppt","combined master text list style");
                    // aMasterTextListStyle->dump();
                }
                if (pPPTPlaceholder && pPPTPlaceholder->mpPlaceholder.get()) {
                    SAL_INFO("oox.ppt","placeholder has parent placeholder: " << pPPTPlaceholder->mpPlaceholder->getId() << " type: " << lclDebugSubType( pPPTPlaceholder->mpPlaceholder->getSubType() ) << " index: " << pPPTPlaceholder->mpPlaceholder->getSubTypeIndex().get() );
                    SAL_INFO("oox.ppt","has textbody " << (pPPTPlaceholder->mpPlaceholder->getTextBody() != nullptr) );
                    TextListStylePtr pPlaceholderStyle = getSubTypeTextListStyle( rSlidePersist, pPPTPlaceholder->mpPlaceholder->getSubType() );
                    if (pPPTPlaceholder->mpPlaceholder->getTextBody())
                        pNewTextListStyle->apply( pPPTPlaceholder->mpPlaceholder->getTextBody()->getTextListStyle() );
                    if (pPlaceholderStyle.get()) {
                        pNewTextListStyle->apply( *pPlaceholderStyle );
                        //pPlaceholderStyle->dump();
                    }
                }
            } else if (!mpPlaceholder.get()) {
                aMasterTextListStyle.reset();
            }
            SAL_INFO("oox.ppt","placeholder id: " << (pPlaceholder.get() ? pPlaceholder->getId() : "not found"));
        }

        if (!sServiceName.isEmpty())
        {
            if (!aMasterTextListStyle.get())
            {
                bool isOther = !getTextBody().get() && sServiceName != "com.sun.star.drawing.GroupShape";
                TextListStylePtr aSlideStyle = isOther ? rSlidePersist.getOtherTextStyle() : rSlidePersist.getDefaultTextStyle();
                // Combine from MasterSlide details as well.
                if (rSlidePersist.getMasterPersist().get())
                {
                    aMasterTextListStyle = isOther ? rSlidePersist.getMasterPersist()->getOtherTextStyle() : rSlidePersist.getMasterPersist()->getDefaultTextStyle();
                    if (aSlideStyle.get())
                        aMasterTextListStyle->apply( *aSlideStyle );
                }
                else
                {
                    aMasterTextListStyle = aSlideStyle;
                }
            }

            if( aMasterTextListStyle.get() && getTextBody().get() ) {
                TextListStylePtr aCombinedTextListStyle (new TextListStyle());

                aCombinedTextListStyle->apply( *aMasterTextListStyle );

                if( mpPlaceholder.get() && mpPlaceholder->getTextBody().get() )
                    aCombinedTextListStyle->apply( mpPlaceholder->getTextBody()->getTextListStyle() );
                aCombinedTextListStyle->apply( getTextBody()->getTextListStyle() );

                setMasterTextListStyle( aCombinedTextListStyle );
            } else
                setMasterTextListStyle( aMasterTextListStyle );

            Reference< XShape > xShape( createAndInsert( rFilterBase, sServiceName, pTheme, rxShapes, bClearText, mpPlaceholder.get() != nullptr, aTransformation, getFillProperties() ) );
            if (!rSlidePersist.isMasterPage() && rSlidePersist.getPage().is() && mnSubType == XML_title)
             {
                try
                {
                    OUString aTitleText;
                    Reference<XTextRange> xText(xShape, UNO_QUERY_THROW);
                    aTitleText = xText->getString();
                    if (!aTitleText.isEmpty() && (aTitleText.getLength() < 64))    // just a magic value, but we don't want to set slide names which are too long
                    {
                        Reference<container::XNamed> xName(rSlidePersist.getPage(), UNO_QUERY_THROW);
                        xName->setName(aTitleText);
                    }
                }
                catch (uno::Exception&)
                {

                }
            }

            // Apply text properties on placeholder text inside this placeholder shape
            if (meShapeLocation == Slide && mpPlaceholder.get() != nullptr && getTextBody() && getTextBody()->isEmpty())
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

            // if this is a group shape, we have to add also each child shape
            Reference<XShapes> xShapes(xShape, UNO_QUERY);
            if (xShapes.is())
                addChildren( rFilterBase, *this, pTheme, xShapes, pShapeMap, aTransformation );

            if (meFrameType == FRAMETYPE_DIAGRAM)
                keepDiagramCompatibilityInfo();
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
// this order seems to work quite well (probably it's unnecessary complicated / wrong):
// 1. ph with nFirstSubType and the same oSubTypeIndex
// 2. ph with nFirstSubType
// 3. ph with nSecondSubType and the same oSubTypeIndex
// 4. ph with nSecondSubType
// 5. ph with the same oSubTypeIndex
oox::drawingml::ShapePtr PPTShape::findPlaceholder( sal_Int32 nFirstSubType, sal_Int32 nSecondSubType,
    const OptValue< sal_Int32 >& oSubTypeIndex, std::vector< oox::drawingml::ShapePtr >& rShapes, bool bMasterOnly )
{
    oox::drawingml::ShapePtr aShapePtr;
    oox::drawingml::ShapePtr aChoiceShapePtr1;
    oox::drawingml::ShapePtr aChoiceShapePtr2;
    oox::drawingml::ShapePtr aChoiceShapePtr3;
    oox::drawingml::ShapePtr aChoiceShapePtr4;
    std::vector< oox::drawingml::ShapePtr >::reverse_iterator aRevIter( rShapes.rbegin() );
    while (aRevIter != rShapes.rend())
    {
        if (!bMasterOnly || ShapeLocationIsMaster((*aRevIter).get()))
        {
            if ((*aRevIter)->getSubTypeIndex() == oSubTypeIndex)
            {
                if ((*aRevIter)->getSubType() == nFirstSubType)
                {
                    aShapePtr = *aRevIter;
                    break;
                }
                else if ((*aRevIter)->getSubType() == nSecondSubType && !aChoiceShapePtr2.get())
                    aChoiceShapePtr2 = *aRevIter;
                else if (!aChoiceShapePtr4.get())
                    aChoiceShapePtr4 = *aRevIter;
            }
            else if ((*aRevIter)->getSubType() == nFirstSubType && !aChoiceShapePtr1.get())
                aChoiceShapePtr1 = *aRevIter;
            else if ((*aRevIter)->getSubType() == nSecondSubType && !aChoiceShapePtr3.get())
                aChoiceShapePtr3 = *aRevIter;
        }
        std::vector< oox::drawingml::ShapePtr >& rChildren = (*aRevIter)->getChildren();
        aChoiceShapePtr4 = findPlaceholder( nFirstSubType, nSecondSubType, oSubTypeIndex, rChildren, bMasterOnly );
        if (aChoiceShapePtr4.get())
        {
            if (aChoiceShapePtr4->getSubType() == nFirstSubType)
            {
                if (aChoiceShapePtr4->getSubTypeIndex() == oSubTypeIndex)
                    aShapePtr = aChoiceShapePtr4;
                else
                    aChoiceShapePtr1 = aChoiceShapePtr4;
            }
            else if (aChoiceShapePtr4->getSubType() == nSecondSubType)
            {
                if (aChoiceShapePtr4->getSubTypeIndex() == oSubTypeIndex)
                    aChoiceShapePtr2 = aChoiceShapePtr4;
                else
                    aChoiceShapePtr3 = aChoiceShapePtr4;
            }
        }
        if (aShapePtr.get())
            break;
        ++aRevIter;
    }
    if (aShapePtr.get())
        return aShapePtr;
    if (aChoiceShapePtr1.get())
        return aChoiceShapePtr1;
    if (aChoiceShapePtr2.get())
        return aChoiceShapePtr2;
    if (aChoiceShapePtr3.get())
        return aChoiceShapePtr3;
    return aChoiceShapePtr4;
}

oox::drawingml::ShapePtr PPTShape::findPlaceholderByIndex( const sal_Int32 nIdx, std::vector< oox::drawingml::ShapePtr >& rShapes, bool bMasterOnly )
{
    oox::drawingml::ShapePtr aShapePtr;

    std::vector< oox::drawingml::ShapePtr >::reverse_iterator aRevIter( rShapes.rbegin() );
    while( aRevIter != rShapes.rend() )
    {
        if ( (*aRevIter)->getSubTypeIndex().has() && (*aRevIter)->getSubTypeIndex().get() == nIdx &&
             ( !bMasterOnly || ShapeLocationIsMaster((*aRevIter).get()) ) )
        {
            aShapePtr = *aRevIter;
            break;
        }
        std::vector< oox::drawingml::ShapePtr >& rChildren = (*aRevIter)->getChildren();
        aShapePtr = findPlaceholderByIndex( nIdx, rChildren, bMasterOnly );
        if ( aShapePtr.get() )
            break;
        ++aRevIter;
    }
    return aShapePtr;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
