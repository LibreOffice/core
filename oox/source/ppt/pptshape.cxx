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

#include "oox/ppt/pptshape.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/textbody.hxx"

#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/text/XText.hpp>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include "oox/ppt/slidepersist.hxx"

using rtl::OUString;
using namespace ::oox::core;
using namespace ::oox::drawingml;
using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;

namespace oox { namespace ppt {

PPTShape::PPTShape( const oox::ppt::ShapeLocation eShapeLocation, const sal_Char* pServiceName )
: Shape( pServiceName )
, meShapeLocation( eShapeLocation )
, mbReferenced( sal_False )
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

    OSL_TRACE( "subtype style: %s", lclDebugSubType( nSubType ) );

    switch( nSubType )
    {
        case XML_ctrTitle :
        case XML_title :
        case XML_subTitle :
            pTextListStyle = rSlidePersist.getMasterPersist().get() ? rSlidePersist.getMasterPersist()->getTitleTextStyle() : rSlidePersist.getTitleTextStyle();
            break;
        case XML_obj :
            pTextListStyle = rSlidePersist.getMasterPersist().get() ? rSlidePersist.getMasterPersist()->getBodyTextStyle() : rSlidePersist.getBodyTextStyle();
            break;
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
        const awt::Rectangle* pShapeRect,
        ::oox::drawingml::ShapeIdMap* pShapeMap )
{
    OSL_TRACE("add shape id: %s location: %s subtype: %d service: %s", rtl::OUStringToOString(msId, RTL_TEXTENCODING_UTF8 ).getStr(), meShapeLocation == Master ? "master" : meShapeLocation == Slide ? "slide" : meShapeLocation == Layout ? "layout" : "other", mnSubType, rtl::OUStringToOString(msServiceName, RTL_TEXTENCODING_UTF8 ).getStr());
    // only placeholder from layout are being inserted
    if ( mnSubType && ( meShapeLocation == Master ) )
        return;
    try
    {
        rtl::OUString sServiceName( msServiceName );
        if( !sServiceName.isEmpty() )
        {
            oox::drawingml::TextListStylePtr aMasterTextListStyle;
            Reference< lang::XMultiServiceFactory > xServiceFact( rFilterBase.getModel(), UNO_QUERY_THROW );
            sal_Bool bClearText = sal_False;

            if ( sServiceName != "com.sun.star.drawing.GraphicObjectShape" &&
                 sServiceName != "com.sun.star.drawing.OLE2Shape" )
            {
                const rtl::OUString sOutlinerShapeService( "com.sun.star.presentation.OutlinerShape"  );
                OSL_TRACE("has master: %p", rSlidePersist.getMasterPersist().get());
                switch( mnSubType )
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
                        if ( ( meShapeLocation == Master ) || ( meShapeLocation == Layout ) )
                            sServiceName = rtl::OUString();
                        else {
                            sServiceName = "com.sun.star.presentation.SubtitleShape";
                            aMasterTextListStyle = rSlidePersist.getMasterPersist().get() ? rSlidePersist.getMasterPersist()->getTitleTextStyle() : rSlidePersist.getTitleTextStyle();
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
                        if ( rSlidePersist.isNotesPage() )
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
                        bClearText = sal_True;
                    break;
                    case XML_hdr :
                        sServiceName = "com.sun.star.presentation.HeaderShape";
                        bClearText = sal_True;
                    break;
                    case XML_ftr :
                        sServiceName = "com.sun.star.presentation.FooterShape";
                        bClearText = sal_True;
                    break;
                    case XML_sldNum :
                        sServiceName = "com.sun.star.presentation.SlideNumberShape";
                        bClearText = sal_True;
                    break;
                    case XML_sldImg :
                        sServiceName = "com.sun.star.presentation.PageShape";
                    break;
                    case XML_chart :
                        if ( meShapeLocation == Layout )
                            sServiceName = sOutlinerShapeService;
                        else
                            sServiceName = "com.sun.star.presentation.ChartShape";
                    break;
                    case XML_tbl :
                        if ( meShapeLocation == Layout )
                            sServiceName = sOutlinerShapeService;
                        else
                            sServiceName = "com.sun.star.presentation.TableShape";
                    break;
                    case XML_pic :
                        if ( meShapeLocation == Layout )
                            sServiceName = sOutlinerShapeService;
                        else
                            sServiceName = "com.sun.star.presentation.GraphicObjectShape";
                    break;
                    case XML_media :
                        if ( meShapeLocation == Layout )
                            sServiceName = sOutlinerShapeService;
                        else
                            sServiceName = "com.sun.star.presentation.MediaShape";
                    break;
                    default:
                        if ( mnSubType && meShapeLocation == Layout )
                            sServiceName = sOutlinerShapeService;
                    break;
                }
            }

            OSL_TRACE("shape service: %s", rtl::OUStringToOString(sServiceName, RTL_TEXTENCODING_UTF8 ).getStr());

            if( mnSubType && getSubTypeIndex().has() && meShapeLocation == Layout ) {
                oox::drawingml::ShapePtr pPlaceholder = PPTShape::findPlaceholderByIndex( getSubTypeIndex().get(), rSlidePersist.getShapes()->getChildren(), true );
                if (!pPlaceholder.get())
                    pPlaceholder = PPTShape::findPlaceholder( mnSubType, rSlidePersist.getShapes()->getChildren(), true );

                if (pPlaceholder.get()) {
                    if( maSize.Width == 0 || maSize.Height == 0 ) {
                        awt::Size aSize = maSize;
                        if( maSize.Width == 0 )
                            aSize.Width = pPlaceholder->getSize().Width;
                        if( maSize.Height == 0 )
                            aSize.Height = pPlaceholder->getSize().Height;
                        setSize( aSize );
                        if ( maPosition.X == 0 || maPosition.Y == 0 ) {
                            awt::Point aPosition = maPosition;
                            if( maPosition.X == 0 )
                                aPosition.X = pPlaceholder->getPosition().X;
                            if( maPosition.Y == 0 )
                                aPosition.Y = pPlaceholder->getPosition().Y;
                            setPosition( aPosition );
                        }
                    }
                }
            }

            // use placeholder index if possible
            if( mnSubType && getSubTypeIndex().has() && rSlidePersist.getMasterPersist().get() ) {
                oox::drawingml::ShapePtr pPlaceholder = PPTShape::findPlaceholderByIndex( getSubTypeIndex().get(), rSlidePersist.getMasterPersist()->getShapes()->getChildren() );
                // TODO: Check if this is required for non-notes slides as well...
                if( rSlidePersist.isNotesPage() && pPlaceholder.get() && pPlaceholder->getSubType() != getSubType() )
                    pPlaceholder.reset();

                if( pPlaceholder.get()) {
                    OSL_TRACE("found placeholder with index: %d and type: %s", getSubTypeIndex().get(), lclDebugSubType( mnSubType ));
                }
                if( pPlaceholder.get() ) {
                    PPTShape* pPPTPlaceholder = dynamic_cast< PPTShape* >( pPlaceholder.get() );
                    TextListStylePtr pNewTextListStyle ( new TextListStyle() );

                    if( pPlaceholder->getTextBody() ) {

                        pNewTextListStyle->apply( pPlaceholder->getTextBody()->getTextListStyle() );
                        if( pPlaceholder->getMasterTextListStyle().get() )
                            pNewTextListStyle->apply( *pPlaceholder->getMasterTextListStyle() );

                        // OSL_TRACE("placeholder body style");
                        // pPlaceholder->getTextBody()->getTextListStyle().dump();
                        // OSL_TRACE("master text list style");
                        // pPlaceholder->getMasterTextListStyle()->dump();

                        aMasterTextListStyle = pNewTextListStyle;
                        // OSL_TRACE("combined master text list style");
                        // aMasterTextListStyle->dump();
                    }
                    if( pPPTPlaceholder->mpPlaceholder.get() ) {
                        OSL_TRACE("placeholder has parent placeholder: %s type: %s index: %d",
                                  rtl::OUStringToOString( pPPTPlaceholder->mpPlaceholder->getId(), RTL_TEXTENCODING_UTF8 ).getStr(),
                                  lclDebugSubType( pPPTPlaceholder->mpPlaceholder->getSubType() ),
                                  pPPTPlaceholder->mpPlaceholder->getSubTypeIndex().get() );
                        OSL_TRACE("has textbody %d", pPPTPlaceholder->mpPlaceholder->getTextBody() != NULL );
                        TextListStylePtr pPlaceholderStyle = getSubTypeTextListStyle( rSlidePersist, pPPTPlaceholder->mpPlaceholder->getSubType() );
                        if( pPPTPlaceholder->mpPlaceholder->getTextBody() )
                            pNewTextListStyle->apply( pPPTPlaceholder->mpPlaceholder->getTextBody()->getTextListStyle() );
                        if( pPlaceholderStyle.get() ) {
                            pNewTextListStyle->apply( *pPlaceholderStyle );
                            //pPlaceholderStyle->dump();
                        }
                    }
                } else if( !mpPlaceholder.get() ) {
                    aMasterTextListStyle.reset();
                }
                OSL_TRACE("placeholder id: %s", pPlaceholder.get() ? rtl::OUStringToOString(pPlaceholder->getId(), RTL_TEXTENCODING_UTF8 ).getStr() : "not found");
            }

            if ( !sServiceName.isEmpty() )
            {
                if ( !aMasterTextListStyle.get() )
                {
                    bool isOther = !getTextBody().get() && !sServiceName.equalsAscii("com.sun.star.drawing.GroupShape");
                    TextListStylePtr aSlideStyle = isOther ? rSlidePersist.getOtherTextStyle() : rSlidePersist.getDefaultTextStyle();
                    // Combine from MasterSlide details as well.
                    if( rSlidePersist.getMasterPersist().get() )
                    {
                        aMasterTextListStyle = isOther ? rSlidePersist.getMasterPersist()->getOtherTextStyle() : rSlidePersist.getMasterPersist()->getDefaultTextStyle();
                        if( aSlideStyle.get() )
                            aMasterTextListStyle->apply( *aSlideStyle.get() );
                    }
                    else
                    {
                        aMasterTextListStyle = aSlideStyle;
                    }
                }

            if( aMasterTextListStyle.get() && getTextBody().get() ) {
                TextListStylePtr aCombinedTextListStyle (new TextListStyle());

                aCombinedTextListStyle->apply( *aMasterTextListStyle.get() );

                if( mpPlaceholder.get() && mpPlaceholder->getTextBody().get() )
                    aCombinedTextListStyle->apply( mpPlaceholder->getTextBody()->getTextListStyle() );
                aCombinedTextListStyle->apply( getTextBody()->getTextListStyle() );

                setMasterTextListStyle( aCombinedTextListStyle );
            } else
                setMasterTextListStyle( aMasterTextListStyle );

            Reference< XShape > xShape( createAndInsert( rFilterBase, sServiceName, pTheme, rxShapes, pShapeRect, bClearText, mpPlaceholder.get() != NULL, aTransformation, getFillProperties() ) );
                if ( !rSlidePersist.isMasterPage() && rSlidePersist.getPage().is() && ( (sal_Int32)mnSubType == XML_title ) )
                 {
                    try
                    {
                        rtl::OUString aTitleText;
                        Reference< XTextRange > xText( xShape, UNO_QUERY_THROW );
                        aTitleText = xText->getString();
                        if ( !aTitleText.isEmpty() && ( aTitleText.getLength() < 64 ) )    // just a magic value, but we don't want to set slide names which are too long
                        {
                            Reference< container::XNamed > xName( rSlidePersist.getPage(), UNO_QUERY_THROW );
                            xName->setName( aTitleText );
                        }
                    }
                    catch( uno::Exception& )
                    {

                    }
                }
                if( pShapeMap && !msId.isEmpty() )
                {
                    (*pShapeMap)[ msId ] = shared_from_this();
                }

                // if this is a group shape, we have to add also each child shape
                Reference< XShapes > xShapes( xShape, UNO_QUERY );
                if ( xShapes.is() )
                    addChildren( rFilterBase, *this, pTheme, xShapes, pShapeRect ? *pShapeRect : awt::Rectangle( maPosition.X, maPosition.Y, maSize.Width, maSize.Height ), pShapeMap, aTransformation );
            }
        }
    }
    catch( const Exception&  )
    {
    }
}

void PPTShape::applyShapeReference( const oox::drawingml::Shape& rReferencedShape, bool bUseText )
{
    Shape::applyShapeReference( rReferencedShape, bUseText );
}

oox::drawingml::ShapePtr PPTShape::findPlaceholder( const sal_Int32 nMasterPlaceholder, std::vector< oox::drawingml::ShapePtr >& rShapes, bool bMasterOnly )
{
    oox::drawingml::ShapePtr aShapePtr;
    std::vector< oox::drawingml::ShapePtr >::reverse_iterator aRevIter( rShapes.rbegin() );
    while( aRevIter != rShapes.rend() )
    {
        if ( (*aRevIter)->getSubType() == nMasterPlaceholder &&
             ( !bMasterOnly ||
               ( dynamic_cast< PPTShape* >( (*aRevIter).get() ) && dynamic_cast< PPTShape* >( (*aRevIter).get() )->getShapeLocation() == Master ) ) )
        {
            aShapePtr = *aRevIter;
            break;
        }
        std::vector< oox::drawingml::ShapePtr >& rChildren = (*aRevIter)->getChildren();
        aShapePtr = findPlaceholder( nMasterPlaceholder, rChildren, bMasterOnly );
        if ( aShapePtr.get() )
            break;
        ++aRevIter;
    }
    return aShapePtr;
}

oox::drawingml::ShapePtr PPTShape::findPlaceholderByIndex( const sal_Int32 nIdx, std::vector< oox::drawingml::ShapePtr >& rShapes, bool bMasterOnly )
{
    oox::drawingml::ShapePtr aShapePtr;

    std::vector< oox::drawingml::ShapePtr >::reverse_iterator aRevIter( rShapes.rbegin() );
    while( aRevIter != rShapes.rend() )
    {
        if ( (*aRevIter)->getSubTypeIndex().has() && (*aRevIter)->getSubTypeIndex().get() == nIdx &&
             ( !bMasterOnly ||
               ( dynamic_cast< PPTShape* >( (*aRevIter).get() ) && dynamic_cast< PPTShape* >( (*aRevIter).get() )->getShapeLocation() == Master ) ) )
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
