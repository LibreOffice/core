/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile$
 * $Revision$
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

#include "constraintlistcontext.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/core/namespaces.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using ::rtl::OUString;

namespace oox { namespace drawingml {

// CT_ConstraintLists
ConstraintListContext::ConstraintListContext( ContextHandler& rParent,
                                              const Reference< XFastAttributeList >&,
                                              const LayoutAtomPtr &pNode )
    : ContextHandler( rParent )
    , mpNode( pNode )
{
    OSL_ENSURE( pNode, "Node must NOT be NULL" );
}


ConstraintListContext::~ConstraintListContext()
{
}

void SAL_CALL ConstraintListContext::endFastElement( ::sal_Int32 )
    throw (SAXException, RuntimeException)
{
}

Reference< XFastContextHandler > SAL_CALL
ConstraintListContext::createFastChildContext( ::sal_Int32 aElement,
                                               const Reference< XFastAttributeList >& xAttribs )
    throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    OUString aEmptyStr;

    switch( aElement )
    {
    case NMSP_DIAGRAM|XML_constr:
    {
        ConstraintAtomPtr pNode( new ConstraintAtom() );
        mpNode->addChild( pNode );

        AttributeList aAttribs( xAttribs );
        pNode->setFor( aAttribs.getToken( XML_for, XML_none ) );
        pNode->setForName( aAttribs.getString( XML_forName, aEmptyStr ) );
        pNode->setPointType( aAttribs.getToken( XML_ptType, XML_none ) );
        pNode->setType( aAttribs.getToken( XML_type, XML_none ) );
        pNode->setRefFor( aAttribs.getToken( XML_refFor, XML_none ) );
        pNode->setRefForName( aAttribs.getString( XML_refForName, aEmptyStr ) );
        pNode->setRefType( aAttribs.getToken( XML_refType, XML_none ) );
        pNode->setRefPointType( aAttribs.getToken( XML_refPtType, XML_none ) );
        pNode->setFactor( aAttribs.getDouble( XML_fact, 1.0 ) );
        pNode->setValue( aAttribs.getDouble( XML_val, 0.0 ) );
        pNode->setOperator( aAttribs.getToken( XML_op, XML_none ) );
        break;
    }
    default:
        break;
    }
    if( !xRet.is() )
        xRet.set(this);

    return xRet;
}


} }
