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

#include "oox/ppt/timenode.hxx"
#include "oox/ppt/pptshape.hxx"
#include "oox/ppt/slidepersist.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "oox/drawingml/shapepropertymap.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/vml/vmldrawing.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "drawingml/textliststyle.hxx"
#include "drawingml/textparagraphproperties.hxx"

#include <osl/diagnose.h>

#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>

using namespace ::com::sun::star;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::animations;

namespace oox { namespace ppt {

SlidePersist::SlidePersist( XmlFilterBase& rFilter, bool bMaster, bool bNotes,
    const css::uno::Reference< css::drawing::XDrawPage >& rxPage,
        oox::drawingml::ShapePtr pShapesPtr, const drawingml::TextListStylePtr & pDefaultTextStyle )
: mpDrawingPtr( new oox::vml::Drawing( rFilter, rxPage, oox::vml::VMLDRAWING_POWERPOINT ) )
, mxPage( rxPage )
, maShapesPtr( pShapesPtr )
, mnLayoutValueToken( 0 )
, mbMaster( bMaster )
, mbNotes ( bNotes )
, maDefaultTextStylePtr( pDefaultTextStyle )
, maTitleTextStylePtr( new oox::drawingml::TextListStyle )
, maBodyTextStylePtr( new oox::drawingml::TextListStyle )
, maNotesTextStylePtr( new oox::drawingml::TextListStyle )
, maOtherTextStylePtr( new oox::drawingml::TextListStyle )
{
#if OSL_DEBUG_LEVEL > 0
    mxDebugPage = mxPage;
#endif
}

#if OSL_DEBUG_LEVEL > 0
css::uno::WeakReference< css::drawing::XDrawPage > SlidePersist::mxDebugPage;
#endif

SlidePersist::~SlidePersist()
{
}

sal_Int16 SlidePersist::getLayoutFromValueToken()
{
    sal_Int16 nLayout = 20;     // 20 == blanc (so many magic numbers :-( the description at com.sun.star.presentation.DrawPage.Layout does not help)
    switch( mnLayoutValueToken )
    {
        case XML_blank:             nLayout = 20; break;
        case XML_chart:             nLayout =  2; break;
        case XML_chartAndTx:        nLayout =  7; break;
        case XML_clipArtAndTx:      nLayout =  9; break;
        case XML_clipArtAndVertTx:  nLayout = 24; break;
        case XML_fourObj:           nLayout = 18; break;
        case XML_obj:               nLayout = 11; break;
        case XML_objAndTx:          nLayout = 13; break;
        case XML_objOverTx:         nLayout = 14; break;
        case XML_tbl:               nLayout =  8; break;
        case XML_title:             nLayout =  0; break;
        case XML_titleOnly:         nLayout = 19; break;
        case XML_twoObj:
        case XML_twoColTx:          nLayout =  3; break;
        case XML_twoObjAndTx:       nLayout = 15; break;
        case XML_twoObjOverTx:      nLayout = 16; break;
        case XML_tx:                nLayout =  1; break;
        case XML_txAndChart:        nLayout =  4; break;
        case XML_txAndClipArt:      nLayout =  6; break;
        case XML_txAndMedia:        nLayout =  6; break;
        case XML_txAndObj:          nLayout = 10; break;
        case XML_txAndTwoObj:       nLayout = 12; break;
        case XML_txOverObj:         nLayout = 17; break;
        case XML_vertTitleAndTx:    nLayout = 22; break;
        case XML_vertTitleAndTxOverChart: nLayout = 21; break;
        case XML_vertTx:            nLayout = 23; break;

        case XML_twoTxTwoObj:
        case XML_twoObjAndObj:
        case XML_objTx:
        case XML_picTx:
        case XML_secHead:
        case XML_objOnly:
        case XML_objAndTwoObj:
        case XML_mediaAndTx:
        case XML_dgm:
        case XML_cust:
        default:
            nLayout = 20;
    }
    return nLayout;
}

void SlidePersist::createXShapes( XmlFilterBase& rFilterBase )
{
    applyTextStyles( rFilterBase );

    Reference< XShapes > xShapes( getPage(), UNO_QUERY );

    std::vector< oox::drawingml::ShapePtr >& rShapes( maShapesPtr->getChildren() );
    const std::vector< oox::drawingml::ShapePtr >::const_iterator aShapesEnd( rShapes.end() );
    for (std::vector< oox::drawingml::ShapePtr >::const_iterator aShapesIter( rShapes.begin() );
         aShapesIter != aShapesEnd ; ++aShapesIter)
    {
        std::vector< oox::drawingml::ShapePtr >& rChildren( (*aShapesIter)->getChildren() );
        const std::vector< oox::drawingml::ShapePtr >::const_iterator aChildEnd( rChildren.end() );
        for (std::vector< oox::drawingml::ShapePtr >::const_iterator aChildIter( rChildren.begin() );
             aChildIter != aChildEnd ; ++aChildIter)
        {
            PPTShape* pPPTShape = dynamic_cast< PPTShape* >( (*aChildIter).get() );
            basegfx::B2DHomMatrix aTransformation;
            if ( pPPTShape )
                pPPTShape->addShape( rFilterBase, *this, getTheme().get(), xShapes, aTransformation, nullptr, &getShapeMap() );
            else
                (*aChildIter)->addShape( rFilterBase, getTheme().get(), xShapes, aTransformation, maShapesPtr->getFillProperties(), nullptr, &getShapeMap() );
        }
    }

    Reference< XAnimationNodeSupplier > xNodeSupplier( getPage(), UNO_QUERY);
    if( xNodeSupplier.is() )
    {
        Reference< XAnimationNode > xNode( xNodeSupplier->getAnimationNode() );
        if( xNode.is() && !maTimeNodeList.empty() )
        {
            SlidePersistPtr pSlidePtr( shared_from_this() );
            TimeNodePtr pNode(maTimeNodeList.front());
            OSL_ENSURE( pNode, "pNode" );

            pNode->setNode( rFilterBase, xNode, pSlidePtr );
        }
    }
}

void SlidePersist::createBackground( const XmlFilterBase& rFilterBase )
{
    if ( mpBackgroundPropertiesPtr )
    {
        sal_Int32 nPhClr = mpBackgroundPropertiesPtr->getBestSolidColor().getColor( rFilterBase.getGraphicHelper() );

        std::vector<sal_Int32> aPropertyIds = (oox::drawingml::ShapePropertyInfo::DEFAULT).maPropertyIds;
        aPropertyIds[oox::drawingml::ShapePropertyId::SHAPEPROP_FillGradient] = PROP_FillGradientName;
        oox::drawingml::ShapePropertyInfo aPropInfo( aPropertyIds.data(), true, false, true, false );
        oox::drawingml::ShapePropertyMap aPropMap( rFilterBase.getModelObjectHelper(), aPropInfo );
        mpBackgroundPropertiesPtr->pushToPropMap( aPropMap, rFilterBase.getGraphicHelper(), 0, nPhClr );
        PropertySet( mxPage ).setProperty( PROP_Background, aPropMap.makePropertySet() );
    }
}

void setTextStyle( Reference< beans::XPropertySet >& rxPropSet, const XmlFilterBase& rFilter,
    oox::drawingml::TextListStylePtr& pTextListStylePtr, int nLevel )
{
    ::oox::drawingml::TextParagraphPropertiesPtr pTextParagraphPropertiesPtr( pTextListStylePtr->getListStyle()[ nLevel ] );
    if( pTextParagraphPropertiesPtr == nullptr )
    {
        // no properties. return
        return;
    }

    PropertyMap& rTextParagraphPropertyMap( pTextParagraphPropertiesPtr->getTextParagraphPropertyMap() );

    PropertySet aPropSet( rxPropSet );
    aPropSet.setProperties( rTextParagraphPropertyMap );
    pTextParagraphPropertiesPtr->getTextCharacterProperties().pushToPropSet( aPropSet, rFilter );
}

void SlidePersist::applyTextStyles( const XmlFilterBase& rFilterBase )
{
    if ( mbMaster )
    {
        try
        {
            Reference< style::XStyleFamiliesSupplier > aXStyleFamiliesSupplier( rFilterBase.getModel(), UNO_QUERY_THROW );
            Reference< container::XNameAccess > aXNameAccess( aXStyleFamiliesSupplier->getStyleFamilies() );
            Reference< container::XNamed > aXNamed( mxPage, UNO_QUERY_THROW );

            if ( aXNameAccess.is() && aXNamed.is() )
            {
                oox::drawingml::TextListStylePtr pTextListStylePtr;
                OUString aStyle;
                OUString aFamily;

                const OUString sOutline( "outline1" );
                const OUString sTitle( "title" );
                const OUString sStandard( "standard" );
                const OUString sSubtitle( "subtitle" );

                for( int i = 0; i < 4; i++ )    // todo: aggregation of bodystyle (subtitle)
                {
                    switch( i )
                    {
                        case 0 :    // title style
                        {
                            pTextListStylePtr = maTitleTextStylePtr;
                            aStyle = sTitle;
                            aFamily= aXNamed->getName();
                            break;
                        }
                        case 1 :    // body style
                        {
                            pTextListStylePtr = maBodyTextStylePtr;
                            aStyle = sOutline;
                            aFamily= aXNamed->getName();
                            break;
                        }
                        case 3 :    // notes style
                        {
                            pTextListStylePtr = maNotesTextStylePtr;
                            aStyle = sTitle;
                            aFamily= aXNamed->getName();
                            break;
                        }
                        case 4 :    // standard style
                        {
                            pTextListStylePtr = maOtherTextStylePtr;
                            aStyle = sStandard;
                            aFamily = "graphics";
                            break;
                        }
                        case 5 :    // subtitle
                        {
                            pTextListStylePtr = maBodyTextStylePtr;
                            aStyle = sSubtitle;
                            aFamily = aXNamed->getName();
                            break;
                        }
                    }
                    Reference< container::XNameAccess > xFamilies;
                    if ( aXNameAccess->hasByName( aFamily ) )
                    {
                        if( aXNameAccess->getByName( aFamily ) >>= xFamilies )
                        {
                            if ( xFamilies->hasByName( aStyle ) )
                            {
                                Reference< style::XStyle > aXStyle;
                                if ( xFamilies->getByName( aStyle ) >>= aXStyle )
                                {
                                    Reference< beans::XPropertySet > xPropSet( aXStyle, UNO_QUERY_THROW );
                                    setTextStyle( xPropSet, rFilterBase, maDefaultTextStylePtr, 0 );
                                    setTextStyle( xPropSet, rFilterBase, pTextListStylePtr, 0 );
                                    if ( i == 1 /* BodyStyle */ )
                                    {
                                        for ( int nLevel = 1; nLevel < 5; nLevel++ )
                                        {
                                            {
                                                sal_Char pOutline[ 9 ] = "outline1";
                                                pOutline[ 7 ] = static_cast< sal_Char >( '0' + nLevel );
                                                OUString sOutlineStyle( OUString::createFromAscii( pOutline ) );
                                                if ( xFamilies->hasByName( sOutlineStyle ) )
                                                {
                                                    xFamilies->getByName( sOutlineStyle ) >>= aXStyle;
                                                    if( aXStyle.is() )
                                                        xPropSet.set( aXStyle, UNO_QUERY_THROW );
                                                }
                                            }
                                            setTextStyle( xPropSet, rFilterBase, maDefaultTextStylePtr, nLevel );
                                            setTextStyle( xPropSet, rFilterBase, pTextListStylePtr, nLevel );
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        catch( const Exception& )
        {
        }
    }
}

void SlidePersist::hideShapesAsMasterShapes()
{
    std::vector< oox::drawingml::ShapePtr >& rShapes( maShapesPtr->getChildren() );
    std::vector< oox::drawingml::ShapePtr >::iterator aShapesIter( rShapes.begin() );
    while( aShapesIter != rShapes.end() )
    {
        while( aShapesIter != rShapes.end() )
        {
            std::vector< oox::drawingml::ShapePtr >& rChildren( (*aShapesIter++)->getChildren() );
            std::vector< oox::drawingml::ShapePtr >::iterator aChildIter( rChildren.begin() );
            while( aChildIter != rChildren.end() ) {
                PPTShape* pPPTShape = dynamic_cast< PPTShape* >( (*aChildIter++).get() );
                if (!pPPTShape)
                    continue;
                OSL_TRACE("hide shape with id: %s", OUStringToOString(pPPTShape->getId(), RTL_TEXTENCODING_UTF8 ).getStr());
                pPPTShape->setHiddenMasterShape( true );
            }
        }
    }
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
