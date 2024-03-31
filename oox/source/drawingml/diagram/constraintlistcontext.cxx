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
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace ::oox::core;

namespace oox::drawingml {

// CT_ConstraintLists
ConstraintListContext::ConstraintListContext( ContextHandler2Helper const & rParent,
                                              const LayoutAtomPtr &pNode )
    : ContextHandler2( rParent )
    , mpNode( pNode )
{
    assert( pNode && "Node must NOT be NULL" );
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
        auto pNode = std::make_shared<ConstraintAtom>(mpNode->getLayoutNode());
        LayoutAtom::connect(mpNode, pNode);

        Constraint& rConstraint = pNode->getConstraint();
        rConstraint.mnFor = rAttribs.getToken( XML_for, XML_none );
        rConstraint.msForName = rAttribs.getStringDefaulted( XML_forName);
        rConstraint.mnPointType = rAttribs.getToken( XML_ptType, XML_none );
        rConstraint.mnType = rAttribs.getToken( XML_type, XML_none );
        rConstraint.mnRefFor = rAttribs.getToken( XML_refFor, XML_none );
        rConstraint.msRefForName = rAttribs.getStringDefaulted( XML_refForName);
        rConstraint.mnRefType = rAttribs.getToken( XML_refType, XML_none );
        rConstraint.mnRefPointType = rAttribs.getToken( XML_refPtType, XML_none );
        rConstraint.mfFactor = rAttribs.getDouble( XML_fact, 1.0 );
        rConstraint.mfValue = rAttribs.getDouble( XML_val, 0.0 );
        rConstraint.mnOperator = rAttribs.getToken( XML_op, XML_none );
        break;
    }
    default:
        break;
    }

    return this;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
