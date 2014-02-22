/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "pathexpression.hxx"
#include "unohelper.hxx"
#include "evaluationcontext.hxx"
#include "NameContainer.hxx"

#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>
#include <com/sun/star/xml/dom/events/XEventListener.hpp>
#include <com/sun/star/xml/dom/events/XEventTarget.hpp>
#include <com/sun/star/xml/xpath/XXPathObject.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustrbuf.hxx>

#include <unotools/textsearch.hxx>

#include <algorithm>
#include <functional>


using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::xml::dom::XNode;
using com::sun::star::xml::dom::XNodeList;
using com::sun::star::xml::dom::events::XEventListener;
using com::sun::star::xml::dom::events::XEventTarget;
using com::sun::star::container::XNameContainer;
using com::sun::star::xml::xpath::XXPathObject;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::UNO_QUERY_THROW;
using com::sun::star::xml::dom::NodeType_TEXT_NODE;
using com::sun::star::xml::xpath::XPathObjectType_XPATH_UNDEFINED;
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
    
    ComputedExpression::setExpression( rExpression );

    
    
    
    
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

bool PathExpression::evaluate( const EvaluationContext& rContext )
{
    
    
    if( mxResult.is() && isSimpleExpression() )
        return true;

    bool bResult = _evaluate( rContext, _getExpressionForEvaluation() );

    
    maNodes.clear();
    if( mxResult.is() )
    {
        
        Reference<XNodeList> xNodeList = mxResult->getNodeList();
        OSL_ENSURE( xNodeList.is(), "empty object (instead of empty list)" );
        sal_Int32 nLength = xNodeList.is() ? xNodeList->getLength() : 0;
        for( sal_Int32 n = 0; n < nLength; n++ )
            maNodes.push_back( xNodeList->item( n ) );
    }

    return bResult;
}


Reference<XNode> PathExpression::getNode() const
{
    Reference<XNode> xResult;
    if( ! maNodes.empty() )
        xResult = *maNodes.begin();
    return xResult;
}

const PathExpression::NodeVector_t PathExpression::getNodeList() const
{
    return maNodes;
}

Reference<XNodeList> PathExpression::getXNodeList() const
{
    return mxResult.is() ? mxResult->getNodeList() : Reference<XNodeList>();
}


} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
