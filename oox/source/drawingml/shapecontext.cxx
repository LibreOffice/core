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

#include <oox/helper/attributelist.hxx>
#include <oox/drawingml/shapecontext.hxx>
#include <drawingml/shapepropertiescontext.hxx>
#include <drawingml/shapestylecontext.hxx>
#include <drawingml/misccontexts.hxx>
#include <drawingml/lineproperties.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <drawingml/customshapegeometry.hxx>
#include <drawingml/textbodycontext.hxx>
#include <drawingml/textbodypropertiescontext.hxx>
#include "hyperlinkcontext.hxx"
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <sal/log.hxx>

using namespace oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

// CT_Shape
ShapeContext::ShapeContext( ContextHandler2Helper const & rParent, ShapePtr const & pMasterShapePtr, ShapePtr const & pShapePtr )
: ContextHandler2( rParent )
, mpMasterShapePtr( pMasterShapePtr )
, mpShapePtr( pShapePtr )
{
    if( mpMasterShapePtr.get() && mpShapePtr.get() )
        mpMasterShapePtr->addChild( mpShapePtr );
}

ShapeContext::~ShapeContext()
{
}

ContextHandlerRef ShapeContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( getBaseToken( aElementToken ) )
    {
    // nvSpPr CT_ShapeNonVisual begin
//  case XML_drElemPr:
//      break;
    case XML_cNvPr:
    {
        mpShapePtr->setHidden( rAttribs.getBool( XML_hidden, false ) );
        mpShapePtr->setId( rAttribs.getString( XML_id ).get() );
        mpShapePtr->setName( rAttribs.getString( XML_name ).get() );
        mpShapePtr->setDescription( rAttribs.getString( XML_descr ).get() );
        break;
    }
    case XML_hlinkMouseOver:
    case XML_hlinkClick:
        return new HyperLinkContext( *this, rAttribs,  getShape()->getShapeProperties() );
    case XML_ph:
        mpShapePtr->setSubType( rAttribs.getToken( XML_type, XML_obj ) );
        if( rAttribs.hasAttribute( XML_idx ) )
            mpShapePtr->setSubTypeIndex( rAttribs.getString( XML_idx ).get().toInt32() );
        break;
    // nvSpPr CT_ShapeNonVisual end

    case XML_spPr:
        return new ShapePropertiesContext( *this, *mpShapePtr );

    case XML_style:
        return new ShapeStyleContext( *this, *mpShapePtr );

    case XML_txBody:
    case XML_txbxContent:
    {
        if (!mpShapePtr->getTextBody())
            mpShapePtr->setTextBody( std::make_shared<TextBody>() );
        return new TextBodyContext( *this, mpShapePtr );
    }
    case XML_txXfrm:
    {
        const TextBodyPtr& rShapePtr = mpShapePtr->getTextBody();
        if (rShapePtr)
            rShapePtr->getTextProperties().moRotation = rAttribs.getInteger( XML_rot );
        return nullptr;
    }
    case XML_cNvSpPr:
        break;
    case XML_spLocks:
        break;
    case XML_bodyPr:
        if (!mpShapePtr->getTextBody())
            mpShapePtr->setTextBody( std::make_shared<TextBody>() );
        return new TextBodyPropertiesContext( *this, rAttribs, mpShapePtr );
        break;
    case XML_txbx:
        break;
    case XML_cNvPicPr:
        break;
    case XML_nvPicPr:
        break;
    case XML_relIds:
        break;
    case XML_nvSpPr:
        break;
    default:
        SAL_WARN("oox", "ShapeContext::onCreateContext: unhandled element: " << getBaseToken(aElementToken));
        break;
    }

    return this;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
