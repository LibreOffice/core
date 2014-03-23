/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

oox::drawingml::ShapePtr findPlaceholder( const sal_Int32 nMasterPlaceholder, sal_Int32 nSubTypeIndex, std::vector< oox::drawingml::ShapePtr >& rShapes )
{
    oox::drawingml::ShapePtr aShapePtr;
    std::vector< oox::drawingml::ShapePtr >::reverse_iterator aRevIter( rShapes.rbegin() );
    while( aRevIter != rShapes.rend() )
    {
        if ( (*aRevIter)->getSubType() == nMasterPlaceholder )
        {
            if ( ( nSubTypeIndex == -1 ) || ( nSubTypeIndex == (*aRevIter)->getSubTypeIndex() ) )
            {
                aShapePtr = *aRevIter;
                break;
            }
        }
        std::vector< oox::drawingml::ShapePtr >& rChildren = (*aRevIter)->getChildren();
        aShapePtr = findPlaceholder( nMasterPlaceholder, nSubTypeIndex, rChildren );
        if ( aShapePtr.get() )
            break;
        aRevIter++;
    }
    return aShapePtr;
}

// if nFirstPlaceholder can't be found, it will be searched for nSecondPlaceholder
oox::drawingml::ShapePtr findPlaceholder( sal_Int32 nFirstPlaceholder, sal_Int32 nSecondPlaceholder,
    sal_Int32 nSubTypeIndex, std::vector< oox::drawingml::ShapePtr >& rShapes )
{
    oox::drawingml::ShapePtr pPlaceholder = findPlaceholder( nFirstPlaceholder, nSubTypeIndex, rShapes );
    return !nSecondPlaceholder || pPlaceholder.get() ? pPlaceholder : findPlaceholder( nSecondPlaceholder, nSubTypeIndex, rShapes );
}

Reference< XFastContextHandler > PPTShapeContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

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
                                pPlaceholder = findPlaceholder( nFirstPlaceholder, nSecondPlaceholder, -1, mpSlidePersistPtr->getShapes()->getChildren() );
                              else if ( eShapeLocation == Slide )   // normal slide shapes have to search within the corresponding master tree for referenced objects
                              {
                                  SlidePersistPtr pMasterPersist( mpSlidePersistPtr->getMasterPersist() );
                                  if ( pMasterPersist.get() )
                                {
                                    if ( mpSlidePersistPtr->isNotesPage() )
                                        pPlaceholder = findPlaceholder( nFirstPlaceholder, nSecondPlaceholder, -1, pMasterPersist->getShapes()->getChildren() );
                                    else
                                        pPlaceholder = findPlaceholder( nFirstPlaceholder, nSecondPlaceholder,
                                        pPPTShapePtr->getSubTypeIndex(), pMasterPersist->getShapes()->getChildren() );
                                }
                              }
                              if ( pPlaceholder.get() )
                              {
                                  mpShapePtr->applyShapeReference( *pPlaceholder.get() );
                                  PPTShape* pPPTShape = dynamic_cast< PPTShape* >( pPlaceholder.get() );
                                  if ( pPPTShape )
                                      pPPTShape->setReferenced( sal_True );
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
            oox::drawingml::TextBodyPtr xTextBody( new oox::drawingml::TextBody );
            xTextBody->getTextProperties().maPropertyMap[ PROP_FontIndependentLineSpacing ] <<= static_cast< sal_Bool >( sal_True );
            mpShapePtr->setTextBody( xTextBody );
            xRet = new oox::drawingml::TextBodyContext( *this, *xTextBody );
            break;
        }
    }

    if( !xRet.is() )
        xRet.set( this );

    return xRet;
}


} }
