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


#include "pathexpression.hxx"
#include "unohelper.hxx"
#include "evaluationcontext.hxx"
#include "NameContainer.hxx"

#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/xpath/XXPathObject.hpp>
#include <rtl/ustrbuf.hxx>

#include <unotools/textsearch.hxx>

#include <algorithm>
#include <functional>


using com::sun::star::uno::Reference;
using com::sun::star::xml::dom::XNode;
using com::sun::star::xml::dom::XNodeList;
using namespace std;


namespace xforms
{

PathExpression::PathExpression()
    : ComputedExpression(),
      maNodes()
{
}

PathExpression::~PathExpression()
{
}


void PathExpression::setExpression( const OUString& rExpression )
{
    // set new expression, and clear pre-computed results
    ComputedExpression::setExpression( rExpression );

    // check expression against regular expression to determine
    // whether it contains only 'simple' (i.e. static) conditions. For
    // now, we check whether it only contains number positions.
    // (TODO: Only works for names containing only ASCII letters+digits.)
    mbIsSimple =
        _checkExpression( "( */@?[a-zA-Z0-9:]+( *\\[ *[0-9 ]+ *\\] *)?)+" );

    maNodes.clear();
}

const OUString PathExpression::_getExpressionForEvaluation() const
{
    OUString sExpr = ComputedExpression::_getExpressionForEvaluation();
    if( sExpr.isEmpty())
        sExpr = ".";
    return sExpr;
}

void PathExpression::evaluate( const EvaluationContext& rContext )
{
    // for simple expression we don't need to re-bind (if we were bound before)
    // (we will evaluate empty expressions, since they are interpreted as ".")
    if( mxResult.is() && isSimpleExpression() )
        return;

    _evaluate( rContext, _getExpressionForEvaluation() );

    // clear old result, and copy new
    maNodes.clear();
    if( mxResult.is() )
    {
        // copy node list
        Reference<XNodeList> xNodeList = mxResult->getNodeList();
        OSL_ENSURE( xNodeList.is(), "empty object (instead of empty list)" );
        sal_Int32 nLength = xNodeList.is() ? xNodeList->getLength() : 0;
        for( sal_Int32 n = 0; n < nLength; n++ )
            maNodes.push_back( xNodeList->item( n ) );
    }
}


Reference<XNode> PathExpression::getNode() const
{
    Reference<XNode> xResult;
    if( ! maNodes.empty() )
        xResult = *maNodes.begin();
    return xResult;
}


Reference<XNodeList> PathExpression::getXNodeList() const
{
    return mxResult.is() ? mxResult->getNodeList() : Reference<XNodeList>();
}


} // namespace xforms

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
