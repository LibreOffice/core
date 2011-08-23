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
#include "oox/drawingml/shapecontext.hxx"
#include "oox/drawingml/shapestylecontext.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/customshapegeometry.hxx"
#include "oox/drawingml/textbodycontext.hxx"
#include "tokens.hxx"

using rtl::OUString;
using namespace oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

// CT_Shape
ShapeContext::ShapeContext( ContextHandler& rParent, ShapePtr pMasterShapePtr, ShapePtr pShapePtr )
: ContextHandler( rParent )
, mpMasterShapePtr( pMasterShapePtr )
, mpShapePtr( pShapePtr )
{
}

ShapeContext::~ShapeContext()
{
    if ( mpMasterShapePtr.get() && mpShapePtr.get() )
        mpMasterShapePtr->addChild( mpShapePtr );
}

ShapePtr ShapeContext::getShape()
{
    return mpShapePtr;
}

void ShapeContext::endFastElement( sal_Int32 /* aElementToken */ ) throw( SAXException, RuntimeException )
{
}

Reference< XFastContextHandler > ShapeContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( getToken( aElementToken ) )
    {
    // nvSpPr CT_ShapeNonVisual begin
//	case XML_drElemPr:
//		break;
    case XML_cNvPr:
    {
        AttributeList aAttribs( xAttribs );
        mpShapePtr->setHidden( aAttribs.getBool( XML_hidden, false ) );
        mpShapePtr->setId( xAttribs->getOptionalValue( XML_id ) );
        mpShapePtr->setName( xAttribs->getOptionalValue( XML_name ) );
        break;
    }
    case XML_ph:
        mpShapePtr->setSubType( xAttribs->getOptionalValueToken( XML_type, XML_obj ) );
        mpShapePtr->setSubTypeIndex( xAttribs->getOptionalValue( XML_idx ).toInt32() );
        break;
    // nvSpPr CT_ShapeNonVisual end

    case XML_spPr:
        xRet = new ShapePropertiesContext( *this, *mpShapePtr );
        break;

    case XML_style:
        xRet = new ShapeStyleContext( *this, *mpShapePtr );
        break;

    case XML_txBody:
    {
        TextBodyPtr xTextBody( new TextBody );
        mpShapePtr->setTextBody( xTextBody );
        xRet = new TextBodyContext( *this, *xTextBody );
        break;
    }
    }

    if( !xRet.is() )
    {
        uno::Reference<XFastContextHandler> xTmp(this);
        xRet.set( xTmp );
    }

    return xRet;
}


} }
