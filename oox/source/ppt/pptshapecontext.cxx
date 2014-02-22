/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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

using namespace oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace ppt {


PPTShapeContext::PPTShapeContext( ContextHandler2Helper& rParent, const SlidePersistPtr pSlidePersistPtr, oox::drawingml::ShapePtr pMasterShapePtr, oox::drawingml::ShapePtr pShapePtr )
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
            if( !oSubTypeIndex.has() && aChoiceShapePtr1 == 0 )
                aChoiceShapePtr1 = *aRevIter;
            else if( aChoiceShapePtr2 == 0 )
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
    if( aShapePtr == 0 )
        return aChoiceShapePtr1 ? aChoiceShapePtr1 : aChoiceShapePtr2;
    return aShapePtr;
}


oox::drawingml::ShapePtr findPlaceholder( sal_Int32 nFirstPlaceholder, sal_Int32 nSecondPlaceholder,
    const OptValue< sal_Int32 >& oSubTypeIndex, std::vector< oox::drawingml::ShapePtr >& rShapes )
{
    oox::drawingml::ShapePtr pPlaceholder = findPlaceholder( nFirstPlaceholder, oSubTypeIndex, rShapes );
    return !nSecondPlaceholder || pPlaceholder.get() ? pPlaceholder : findPlaceholder( nSecondPlaceholder, oSubTypeIndex, rShapes );
}

ContextHandlerRef PPTShapeContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    if( getNamespace( aElementToken ) == NMSP_dsp )
        aElementToken = NMSP_ppt | getBaseToken( aElementToken );

    switch( aElementToken )
    {
        
        
        
        case PPT_TOKEN( cNvPr ):
        {
            mpShapePtr->setHidden( rAttribs.getBool( XML_hidden, false ) );
            mpShapePtr->setId( rAttribs.getString( XML_id ).get() );
            mpShapePtr->setName( rAttribs.getString( XML_name ).get() );
            break;
        }
        case PPT_TOKEN( ph ):
        {
            sal_Int32 nSubType( rAttribs.getToken( XML_type, XML_obj ) );
            mpShapePtr->setSubType( nSubType );
            if( rAttribs.hasAttribute( XML_idx ) )
                mpShapePtr->setSubTypeIndex( rAttribs.getString( XML_idx ).get().toInt32() );
            if ( nSubType )
            {
                PPTShape* pPPTShapePtr = dynamic_cast< PPTShape* >( mpShapePtr.get() );
                if ( pPPTShapePtr )
                {
                    oox::ppt::ShapeLocation eShapeLocation = pPPTShapePtr->getShapeLocation();
                    if ( ( eShapeLocation == Slide ) || ( eShapeLocation == Layout ) )
                    {
                        
                        sal_Int32 nFirstPlaceholder = 0;
                        sal_Int32 nSecondPlaceholder = 0;
                        switch( nSubType )
                        {
                            case XML_ctrTitle :     
                                  nFirstPlaceholder = XML_ctrTitle;
                                  nSecondPlaceholder = XML_title;
                              break;

                              case XML_subTitle :       
                                  nFirstPlaceholder = XML_subTitle;
                                  nSecondPlaceholder = XML_title;
                              break;

                             case XML_obj :         
                                  nFirstPlaceholder = XML_obj;
                                  nSecondPlaceholder = XML_body;
                              break;

                            case XML_dt :           
                              case XML_sldNum :     
                              case XML_ftr :            
                              case XML_hdr :            
                              case XML_body :           
                              case XML_title :      
                              case XML_chart :      
                              case XML_tbl :            
                              case XML_clipArt :        
                              case XML_dgm :            
                              case XML_media :      
                              case XML_sldImg :     
                              case XML_pic :            
                                  nFirstPlaceholder = nSubType;
                              default:
                                  break;
                        }
                          if ( nFirstPlaceholder )
                          {
                              oox::drawingml::ShapePtr pPlaceholder;
                              if ( eShapeLocation == Layout )       
                              {
                                  if( pPPTShapePtr->getSubTypeIndex().has() )
                                      pPlaceholder = PPTShape::findPlaceholderByIndex( pPPTShapePtr->getSubTypeIndex().get(), mpSlidePersistPtr->getShapes()->getChildren() );
                                  if ( !pPlaceholder.get() )
                                      pPlaceholder = findPlaceholder( nFirstPlaceholder, nSecondPlaceholder, pPPTShapePtr->getSubTypeIndex(),
                                                                      mpSlidePersistPtr->getShapes()->getChildren() );
                              }
                              else if ( eShapeLocation == Slide )   
                              {
                                  SlidePersistPtr pMasterPersist( mpSlidePersistPtr->getMasterPersist() );
                                  if ( pMasterPersist.get() ) {
                                      if( pPPTShapePtr->getSubTypeIndex().has() )
                                          pPlaceholder = PPTShape::findPlaceholderByIndex( pPPTShapePtr->getSubTypeIndex().get(), pMasterPersist->getShapes()->getChildren() );
                                      
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
                                  OSL_TRACE("shape %s will get shape reference %s applied", OUStringToOString(mpShapePtr->getId(), RTL_TEXTENCODING_UTF8 ).getStr(), OUStringToOString(pPlaceholder->getId(), RTL_TEXTENCODING_UTF8 ).getStr());
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

        

        case PPT_TOKEN( spPr ):
            return new PPTShapePropertiesContext( *this, *mpShapePtr );

        case PPT_TOKEN( style ):
            return new oox::drawingml::ShapeStyleContext( *this, *mpShapePtr );

        case PPT_TOKEN( txBody ):
        {
            oox::drawingml::TextBodyPtr xTextBody( new oox::drawingml::TextBody( mpShapePtr->getTextBody() ) );
            xTextBody->getTextProperties().maPropertyMap[ PROP_FontIndependentLineSpacing ] <<= static_cast< sal_Bool >( sal_True );
            mpShapePtr->setTextBody( xTextBody );
            return new oox::drawingml::TextBodyContext( *this, *xTextBody );
        }
        case PPT_TOKEN( txXfrm ):
        {
            return new oox::drawingml::Transform2DContext( *this, rAttribs, *mpShapePtr, true );
        }
    }

    return this;
}


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
