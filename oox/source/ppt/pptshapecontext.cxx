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

#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include "oox/helper/attributelist.hxx"
#include "oox/ppt/pptshape.hxx"
#include "oox/ppt/pptshapecontext.hxx"
#include "oox/ppt/pptshapepropertiescontext.hxx"
#include "oox/ppt/slidepersist.hxx"
#include "oox/drawingml/shapestylecontext.hxx"
#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/customshapegeometry.hxx"
#include "oox/drawingml/textbodycontext.hxx"
#include "oox/drawingml/transform2dcontext.hxx"

using rtl::OUString;
using namespace oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace ppt {

// CT_Shape
PPTShapeContext::PPTShapeContext( ContextHandler& rParent, const SlidePersistPtr pSlidePersistPtr, oox::drawingml::ShapePtr pMasterShapePtr, oox::drawingml::ShapePtr pShapePtr )
: oox::drawingml::ShapeContext( rParent, pMasterShapePtr, pShapePtr )
, mpSlidePersistPtr( pSlidePersistPtr )
{
}

oox::drawingml::ShapePtr findPlaceholder( const sal_Int32 nMasterPlaceholder, const OptValue< sal_Int32 >& oSubTypeIndex, std::vector< oox::drawingml::ShapePtr >& rShapes )
{
    oox::drawingml::ShapePtr aShapePtr;
    oox::drawingml::ShapePtr aChoiceShapePtr1;
    oox::drawingml::ShapePtr aChoiceShapePtr2;
    std::vector< oox::drawingml::ShapePtr >::reverse_iterator aRevIter( rShapes.rbegin() );
    while( aRevIter != rShapes.rend() )
    {
        if ( (*aRevIter)->getSubType() == nMasterPlaceholder )
        {
            if( !oSubTypeIndex.has() && aChoiceShapePtr1 == NULL )
                aChoiceShapePtr1 = *aRevIter;
            else if( aChoiceShapePtr2 == NULL )
                aChoiceShapePtr2 = *aRevIter;
            if( (*aRevIter)->getSubTypeIndex() == oSubTypeIndex )
            {
                aShapePtr = *aRevIter;
                break;
            }
        }
        std::vector< oox::drawingml::ShapePtr >& rChildren = (*aRevIter)->getChildren();
        aShapePtr = findPlaceholder( nMasterPlaceholder, oSubTypeIndex, rChildren );
        if ( aShapePtr.get() )
            break;
        ++aRevIter;
    }
    if( aShapePtr == NULL )
        return aChoiceShapePtr1 ? aChoiceShapePtr1 : aChoiceShapePtr2;
    return aShapePtr;
}

// if nFirstPlaceholder can't be found, it will be searched for nSecondPlaceholder
oox::drawingml::ShapePtr findPlaceholder( sal_Int32 nFirstPlaceholder, sal_Int32 nSecondPlaceholder,
    const OptValue< sal_Int32 >& oSubTypeIndex, std::vector< oox::drawingml::ShapePtr >& rShapes )
{
    oox::drawingml::ShapePtr pPlaceholder = findPlaceholder( nFirstPlaceholder, oSubTypeIndex, rShapes );
    return !nSecondPlaceholder || pPlaceholder.get() ? pPlaceholder : findPlaceholder( nSecondPlaceholder, oSubTypeIndex, rShapes );
}

Reference< XFastContextHandler > PPTShapeContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    if( getNamespace( aElementToken ) == NMSP_dsp )
        aElementToken = NMSP_ppt | getBaseToken( aElementToken );

    switch( aElementToken )
    {
        // nvSpPr CT_ShapeNonVisual begin
        //  case PPT_TOKEN( drElemPr ):
        //      break;
        case PPT_TOKEN( cNvPr ):
        {
            AttributeList aAttribs( xAttribs );
            mpShapePtr->setHidden( aAttribs.getBool( XML_hidden, false ) );
            mpShapePtr->setId( xAttribs->getOptionalValue( XML_id ) );
            mpShapePtr->setName( xAttribs->getOptionalValue( XML_name ) );
            break;
        }
        case PPT_TOKEN( ph ):
        {
            sal_Int32 nSubType( xAttribs->getOptionalValueToken( XML_type, XML_obj ) );
            mpShapePtr->setSubType( nSubType );
            if( xAttribs->hasAttribute( XML_idx ) )
                mpShapePtr->setSubTypeIndex( xAttribs->getOptionalValue( XML_idx ).toInt32() );
            if ( nSubType )
            {
                PPTShape* pPPTShapePtr = dynamic_cast< PPTShape* >( mpShapePtr.get() );
                if ( pPPTShapePtr )
                {
                    oox::ppt::ShapeLocation eShapeLocation = pPPTShapePtr->getShapeLocation();
                    if ( ( eShapeLocation == Slide ) || ( eShapeLocation == Layout ) )
                    {
                        // inheriting properties from placeholder objects by cloning shape
                        sal_Int32 nFirstPlaceholder = 0;
                        sal_Int32 nSecondPlaceholder = 0;
                        switch( nSubType )
                        {
                            case XML_ctrTitle :     // slide/layout
                                  nFirstPlaceholder = XML_ctrTitle;
                                  nSecondPlaceholder = XML_title;
                              break;

                              case XML_subTitle :       // slide/layout
                                  nFirstPlaceholder = XML_subTitle;
                                  nSecondPlaceholder = XML_title;
                              break;

                             case XML_obj :         // slide/layout
                                  nFirstPlaceholder = XML_obj;
                                  nSecondPlaceholder = XML_body;
                              break;

                            case XML_dt :           // slide/layout/master/notes/notesmaster/handoutmaster
                              case XML_sldNum :     // slide/layout/master/notes/notesmaster/handoutmaster
                              case XML_ftr :            // slide/layout/master/notes/notesmaster/handoutmaster
                              case XML_hdr :            // notes/notesmaster/handoutmaster
                              case XML_body :           // slide/layout/master/notes/notesmaster
                              case XML_title :      // slide/layout/master/
                              case XML_chart :      // slide/layout
                              case XML_tbl :            // slide/layout
                              case XML_clipArt :        // slide/layout
                              case XML_dgm :            // slide/layout
                              case XML_media :      // slide/layout
                              case XML_sldImg :     // notes/notesmaster
                              case XML_pic :            // slide/layout
                                  nFirstPlaceholder = nSubType;
                              default:
                                  break;
                        }
                          if ( nFirstPlaceholder )
                          {
                              oox::drawingml::ShapePtr pPlaceholder;
                              if ( eShapeLocation == Layout )       // for layout objects the referenced object can be found within the same shape tree
                              {
                                  if( pPPTShapePtr->getSubTypeIndex().has() )
                                      pPlaceholder = PPTShape::findPlaceholderByIndex( pPPTShapePtr->getSubTypeIndex().get(), mpSlidePersistPtr->getShapes()->getChildren() );
                                  if ( !pPlaceholder.get() )
                                      pPlaceholder = findPlaceholder( nFirstPlaceholder, nSecondPlaceholder, pPPTShapePtr->getSubTypeIndex(),
                                                                      mpSlidePersistPtr->getShapes()->getChildren() );
                              }
                              else if ( eShapeLocation == Slide )   // normal slide shapes have to search within the corresponding master tree for referenced objects
                              {
                                  SlidePersistPtr pMasterPersist( mpSlidePersistPtr->getMasterPersist() );
                                  if ( pMasterPersist.get() ) {
                                      if( pPPTShapePtr->getSubTypeIndex().has() )
                                          pPlaceholder = PPTShape::findPlaceholderByIndex( pPPTShapePtr->getSubTypeIndex().get(), pMasterPersist->getShapes()->getChildren() );
                                      // TODO: Check if this is required for non-notes pages as well...
                                      if ( !pPlaceholder.get() || ( pMasterPersist->isNotesPage() && pPlaceholder->getSubType() != nFirstPlaceholder &&
                                                                                                     pPlaceholder->getSubType() != nSecondPlaceholder ) )
                                      {
                                          pPlaceholder = findPlaceholder( nFirstPlaceholder, nSecondPlaceholder,
                                                                          pPPTShapePtr->getSubTypeIndex(), pMasterPersist->getShapes()->getChildren() );
                                      }
                                  }
                              }
                              if ( pPlaceholder.get() )
                              {
                                  OSL_TRACE("shape %s will get shape reference %s applied", rtl::OUStringToOString(mpShapePtr->getId(), RTL_TEXTENCODING_UTF8 ).getStr(), rtl::OUStringToOString(pPlaceholder->getId(), RTL_TEXTENCODING_UTF8 ).getStr());
                                  mpShapePtr->applyShapeReference( *pPlaceholder.get() );
                                  PPTShape* pPPTShape = dynamic_cast< PPTShape* >( pPlaceholder.get() );
                                  if ( pPPTShape )
                                      pPPTShape->setReferenced( sal_True );
                                  pPPTShapePtr->setPlaceholder( pPlaceholder );
                              }
                          }
                    }
                  }

              }
              break;
        }

        // nvSpPr CT_ShapeNonVisual end

        case PPT_TOKEN( spPr ):
            xRet = new PPTShapePropertiesContext( *this, *mpShapePtr );
            break;

        case PPT_TOKEN( style ):
            xRet = new oox::drawingml::ShapeStyleContext( *this, *mpShapePtr );
            break;

        case PPT_TOKEN( txBody ):
        {
            oox::drawingml::TextBodyPtr xTextBody( new oox::drawingml::TextBody( mpShapePtr->getTextBody() ) );
            xTextBody->getTextProperties().maPropertyMap[ PROP_FontIndependentLineSpacing ] <<= static_cast< sal_Bool >( sal_True );
            mpShapePtr->setTextBody( xTextBody );
            xRet = new oox::drawingml::TextBodyContext( *this, *xTextBody );
            break;
        }
        case PPT_TOKEN( txXfrm ):
        {
            xRet = new oox::drawingml::Transform2DContext( *this, xAttribs, *mpShapePtr, true );
            break;
        }
    }

    if( !xRet.is() )
        xRet.set( this );

    return xRet;
}


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
