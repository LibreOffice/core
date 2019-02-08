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

#include <sal/log.hxx>

#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include <oox/helper/attributelist.hxx>
#include <oox/ppt/pptshape.hxx>
#include <oox/ppt/pptshapecontext.hxx>
#include <oox/ppt/pptshapepropertiescontext.hxx>
#include <oox/ppt/slidepersist.hxx>
#include <drawingml/shapestylecontext.hxx>
#include <drawingml/misccontexts.hxx>
#include <drawingml/lineproperties.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <drawingml/customshapegeometry.hxx>
#include <drawingml/textbodycontext.hxx>
#include <drawingml/transform2dcontext.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>

using namespace oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace ppt {

// CT_Shape
PPTShapeContext::PPTShapeContext( ContextHandler2Helper const & rParent, const SlidePersistPtr& rSlidePersistPtr, const oox::drawingml::ShapePtr& pMasterShapePtr, const oox::drawingml::ShapePtr& pShapePtr )
: oox::drawingml::ShapeContext( rParent, pMasterShapePtr, pShapePtr )
, mpSlidePersistPtr( rSlidePersistPtr )
{
}

ContextHandlerRef PPTShapeContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    if( getNamespace( aElementToken ) == NMSP_dsp )
        aElementToken = NMSP_ppt | getBaseToken( aElementToken );

    switch( aElementToken )
    {
        // nvSpPr CT_ShapeNonVisual begin
        //  case PPT_TOKEN( drElemPr ):
        //      break;
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
                                  nSecondPlaceholder = XML_body;
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
                                  break;
                              default:
                                  break;
                        }
                        if ( nFirstPlaceholder )
                          {
                              oox::drawingml::ShapePtr pPlaceholder;
                              if ( eShapeLocation == Layout )       // for layout objects the referenced object can be found within the same shape tree
                              {
                                  pPlaceholder = PPTShape::findPlaceholder( nFirstPlaceholder, nSecondPlaceholder,
                                          pPPTShapePtr->getSubTypeIndex(), mpSlidePersistPtr->getShapes()->getChildren(), true );
                              }
                              else if ( eShapeLocation == Slide )   // normal slide shapes have to search within the corresponding master tree for referenced objects
                              {
                                  SlidePersistPtr pMasterPersist( mpSlidePersistPtr->getMasterPersist() );
                                  if ( pMasterPersist.get() )
                                  {
                                      pPlaceholder = PPTShape::findPlaceholder( nFirstPlaceholder, nSecondPlaceholder,
                                              pPPTShapePtr->getSubTypeIndex(), pMasterPersist->getShapes()->getChildren() );
                                  }
                              }
                              if ( pPlaceholder.get() )
                              {
                                  SAL_INFO("oox.ppt","shape " << mpShapePtr->getId() <<
                                          " will get shape reference " << pPlaceholder->getId() << " applied");
                                  mpShapePtr->applyShapeReference( *pPlaceholder );
                                  PPTShape* pPPTShape = dynamic_cast< PPTShape* >( pPlaceholder.get() );
                                  if ( pPPTShape )
                                      pPPTShape->setReferenced( true );
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
            return new PPTShapePropertiesContext( *this, *mpShapePtr );

        case PPT_TOKEN( style ):
            return new oox::drawingml::ShapeStyleContext( *this, *mpShapePtr );

        case PPT_TOKEN( txBody ):
        {
            oox::drawingml::TextBodyPtr xTextBody( new oox::drawingml::TextBody( mpShapePtr->getTextBody() ) );
            xTextBody->getTextProperties().maPropertyMap.setProperty( PROP_FontIndependentLineSpacing, true );
            mpShapePtr->setTextBody( xTextBody );
            return new oox::drawingml::TextBodyContext( *this, mpShapePtr );
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
