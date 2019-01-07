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

#include <oox/ppt/pptshape.hxx>
#include <oox/ppt/pptgraphicshapecontext.hxx>
#include <oox/ppt/pptshapepropertiescontext.hxx>
#include <oox/ppt/slidepersist.hxx>
#include <drawingml/shapestylecontext.hxx>
#include <drawingml/misccontexts.hxx>
#include <drawingml/lineproperties.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <drawingml/customshapegeometry.hxx>
#include <drawingml/textbodycontext.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
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
PPTGraphicShapeContext::PPTGraphicShapeContext( ContextHandler2Helper const & rParent, const SlidePersistPtr& rSlidePersistPtr, const oox::drawingml::ShapePtr& pMasterShapePtr, const oox::drawingml::ShapePtr& pShapePtr )
: oox::drawingml::GraphicShapeContext( rParent, pMasterShapePtr, pShapePtr )
, mpSlidePersistPtr( rSlidePersistPtr )
{
}

ContextHandlerRef PPTGraphicShapeContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
    // nvSpPr CT_ShapeNonVisual begin
//  case NMSP_PPT|XML_drElemPr:
//      break;
    case PPT_TOKEN(cNvPr):
        mpShapePtr->setId( rAttribs.getString( XML_id ).get() );
        mpShapePtr->setName( rAttribs.getString( XML_name ).get() );
        break;
    case PPT_TOKEN(ph):
    {
        sal_Int32 nSubType( rAttribs.getToken( XML_type, XML_obj ) );
        mpShapePtr->setSubType( nSubType );
        OUString sIdx( rAttribs.getString( XML_idx ).get() );
        bool bHasIdx = !sIdx.isEmpty();
        sal_Int32 nIdx = sIdx.toInt32();
        if( rAttribs.hasAttribute( XML_idx ) )
            mpShapePtr->setSubTypeIndex( nIdx );

        if ( nSubType || bHasIdx )
        {
            PPTShape* pPPTShapePtr = dynamic_cast< PPTShape* >( mpShapePtr.get() );
            if ( pPPTShapePtr )
            {
                oox::ppt::ShapeLocation eShapeLocation = pPPTShapePtr->getShapeLocation();
                oox::drawingml::ShapePtr pPlaceholder;

                if ( bHasIdx && eShapeLocation == Slide )
                {
                    // TODO: use id to shape map
                    SlidePersistPtr pMasterPersist( mpSlidePersistPtr->getMasterPersist() );
                    if ( pMasterPersist.get() && rAttribs.hasAttribute( XML_idx ) )
                        pPlaceholder = PPTShape::findPlaceholderByIndex( nIdx, pMasterPersist->getShapes()->getChildren() );
                }
                if ( !pPlaceholder.get() && ( ( eShapeLocation == Slide ) || ( eShapeLocation == Layout ) ) )
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
                        case XML_subTitle :     // slide/layout
                            nFirstPlaceholder = XML_subTitle;
                            nSecondPlaceholder = XML_title;
                            break;
                        case XML_obj :          // slide/layout
                            nFirstPlaceholder = XML_body;
                            break;
                        case XML_dt :           // slide/layout/master/notes/notesmaster/handoutmaster
                        case XML_sldNum :       // slide/layout/master/notes/notesmaster/handoutmaster
                        case XML_ftr :          // slide/layout/master/notes/notesmaster/handoutmaster
                        case XML_hdr :          // notes/notesmaster/handoutmaster
                        case XML_body :         // slide/layout/master/notes/notesmaster
                        case XML_title :        // slide/layout/master/
                        case XML_chart :        // slide/layout
                        case XML_tbl :          // slide/layout
                        case XML_clipArt :      // slide/layout
                        case XML_dgm :          // slide/layout
                        case XML_media :        // slide/layout
                        case XML_sldImg :       // notes/notesmaster
                        case XML_pic :          // slide/layout
                            nFirstPlaceholder = nSubType;
                            break;
                        default:
                            break;
                    }
                    if ( nFirstPlaceholder )
                    {
                        if ( eShapeLocation == Layout )     // for layout objects the referenced object can be found within the same shape tree
                            pPlaceholder = PPTShape::findPlaceholder( nFirstPlaceholder, nSecondPlaceholder,
                                    pPPTShapePtr->getSubTypeIndex(), mpSlidePersistPtr->getShapes()->getChildren(), true );
                        else if ( eShapeLocation == Slide ) // normal slide shapes have to search within the corresponding master tree for referenced objects
                        {
                            SlidePersistPtr pMasterPersist( mpSlidePersistPtr->getMasterPersist() );
                            if ( pMasterPersist.get() )
                                pPlaceholder = PPTShape::findPlaceholder( nFirstPlaceholder, nSecondPlaceholder,
                                        pPPTShapePtr->getSubTypeIndex(), pMasterPersist->getShapes()->getChildren() );
                        }
                    }
                }
                if ( pPlaceholder.get() )
                {
                    bool bUseText = true;
                    switch( pPlaceholder->getSubType() )
                    {
                        case XML_title :
                        case XML_body :
                        case XML_ctrTitle :
                        case XML_subTitle :
                        case XML_dt :
                        case XML_sldNum :
                        case XML_ftr :
                        case XML_hdr :
                        case XML_obj :
                        case XML_chart :
                        case XML_tbl :
                        case XML_clipArt :
                        case XML_dgm :
                        case XML_media :
                        case XML_sldImg :
                        case XML_pic :
                            bUseText = false;
                    }
                    mpShapePtr->applyShapeReference( *pPlaceholder, bUseText );
                    PPTShape* pPPTShape = dynamic_cast< PPTShape* >( pPlaceholder.get() );
                    if ( pPPTShape )
                        pPPTShape->setReferenced( true );
                    pPPTShapePtr->setPlaceholder( pPlaceholder );
                }
            }
        }
        break;
    }
    // nvSpPr CT_ShapeNonVisual end

    case PPT_TOKEN(spPr):
        return new PPTShapePropertiesContext( *this, *mpShapePtr );

    case PPT_TOKEN(style):
        return new oox::drawingml::ShapeStyleContext( *this, *mpShapePtr );

    case PPT_TOKEN(txBody):
    {
        oox::drawingml::TextBodyPtr xTextBody( new oox::drawingml::TextBody );
        mpShapePtr->setTextBody( xTextBody );
        return new oox::drawingml::TextBodyContext( *this, *xTextBody );
    }
    }

    return GraphicShapeContext::onCreateContext( aElementToken, rAttribs );
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
