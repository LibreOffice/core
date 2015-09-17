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

#include "constraintlistcontext.hxx"
#include "oox/helper/attributelist.hxx"
#include <osl/diagnose.h>

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

// CT_ConstraintLists
ConstraintListContext::ConstraintListContext( ContextHandler2Helper& rParent,
                                              const AttributeList&,
                                              const LayoutAtomPtr &pNode )
    : ContextHandler2( rParent )
    , mpNode( pNode )
{
    OSL_ENSURE( pNode, "Node must NOT be NULL" );
}

ConstraintListContext::~ConstraintListContext()
{
}

ContextHandlerRef
ConstraintListContext::onCreateContext( ::sal_Int32 aElement,
                                        const AttributeList& rAttribs )
{
    switch( aElement )
    {
    case DGM_TOKEN( constr ):
    {
        std::shared_ptr< ConstraintAtom > pNode( new ConstraintAtom() );
        mpNode->addChild( pNode );

        pNode->setFor( rAttribs.getToken( XML_for, XML_none ) );
        pNode->setForName( rAttribs.getString( XML_forName, "" ) );
        pNode->setPointType( rAttribs.getToken( XML_ptType, XML_none ) );
        pNode->setType( rAttribs.getToken( XML_type, XML_none ) );
        pNode->setRefFor( rAttribs.getToken( XML_refFor, XML_none ) );
        pNode->setRefForName( rAttribs.getString( XML_refForName, "" ) );
        pNode->setRefType( rAttribs.getToken( XML_refType, XML_none ) );
        pNode->setRefPointType( rAttribs.getToken( XML_refPtType, XML_none ) );
        pNode->setFactor( rAttribs.getDouble( XML_fact, 1.0 ) );
        pNode->setValue( rAttribs.getDouble( XML_val, 0.0 ) );
        pNode->setOperator( rAttribs.getToken( XML_op, XML_none ) );
        break;
    }
    default:
        break;
    }

    return this;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
