/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include "oox/core/namespaces.hxx"
#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/customshapegeometry.hxx"
#include "oox/drawingml/textbodycontext.hxx"
#include "tokens.hxx"
#include "properties.hxx"

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
        //	case NMSP_PPT|XML_drElemPr:
        //		break;
        case NMSP_PPT|XML_cNvPr:
        {
            AttributeList aAttribs( xAttribs );
            mpShapePtr->setHidden( aAttribs.getBool( XML_hidden, false ) );
            mpShapePtr->setId( xAttribs->getOptionalValue( XML_id ) );
            mpShapePtr->setName( xAttribs->getOptionalValue( XML_name ) );
            break;
        }
        case NMSP_PPT|XML_ph:
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
                            case XML_ctrTitle :		// slide/layout
                                  nFirstPlaceholder = XML_ctrTitle;
                                  nSecondPlaceholder = XML_title;
                              break;

                              case XML_subTitle :		// slide/layout
                                  nFirstPlaceholder = XML_subTitle;
                                  nSecondPlaceholder = XML_title;
                              break;

                             case XML_obj :			// slide/layout
                                  nFirstPlaceholder = XML_obj;
                                  nSecondPlaceholder = XML_body;
                              break;

                            case XML_dt :			// slide/layout/master/notes/notesmaster/handoutmaster
                              case XML_sldNum :		// slide/layout/master/notes/notesmaster/handoutmaster
                              case XML_ftr :			// slide/layout/master/notes/notesmaster/handoutmaster
                              case XML_hdr :			// notes/notesmaster/handoutmaster
                              case XML_body :			// slide/layout/master/notes/notesmaster
                              case XML_title :		// slide/layout/master/
                              case XML_chart :		// slide/layout
                              case XML_tbl :			// slide/layout
                              case XML_clipArt :		// slide/layout
                              case XML_dgm :			// slide/layout
                              case XML_media :		// slide/layout
                              case XML_sldImg :		// notes/notesmaster
                              case XML_pic :			// slide/layout
                                  nFirstPlaceholder = nSubType;
                              default:
                                  break;
                        }
                          if ( nFirstPlaceholder )
                          {
                              oox::drawingml::ShapePtr pPlaceholder;
                              if ( eShapeLocation == Layout )		// for layout objects the referenced object can be found within the same shape tree
                                pPlaceholder = findPlaceholder( nFirstPlaceholder, nSecondPlaceholder, -1, mpSlidePersistPtr->getShapes()->getChildren() );
                              else if ( eShapeLocation == Slide )	// normal slide shapes have to search within the corresponding master tree for referenced objects
                              {
                                  SlidePersistPtr pMasterPersist( mpSlidePersistPtr->getMasterPersist() );
                                  if ( pMasterPersist.get() )
                                    pPlaceholder = findPlaceholder( nFirstPlaceholder, nSecondPlaceholder,
                                pPPTShapePtr->getSubTypeIndex(), pMasterPersist->getShapes()->getChildren() );
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

        case NMSP_PPT|XML_spPr:
            xRet = new PPTShapePropertiesContext( *this, *mpShapePtr );
            break;

        case NMSP_PPT|XML_style:
            xRet = new oox::drawingml::ShapeStyleContext( *this, *mpShapePtr );
            break;

        case NMSP_PPT|XML_txBody:
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
