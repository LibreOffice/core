/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pathexpression.cxx,v $
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"

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
#include <com/sun/star/xml/xpath/XXPathAPI.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustrbuf.hxx>

#include <unotools/textsearch.hxx>

#include <algorithm>
#include <functional>


using rtl::OUString;
using rtl::OUStringBuffer;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::xml::dom::XNode;
using com::sun::star::xml::dom::XNodeList;
using com::sun::star::xml::dom::events::XEventListener;
using com::sun::star::xml::dom::events::XEventTarget;
using com::sun::star::container::XNameContainer;
using com::sun::star::xml::xpath::XXPathAPI;
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

const rtl::OUString PathExpression::_getExpressionForEvaluation() const
{
    OUString sExpr = ComputedExpression::_getExpressionForEvaluation();
    if( sExpr.getLength() == 0 )
        sExpr = OUSTRING(".");
    return sExpr;
}

bool PathExpression::evaluate( const EvaluationContext& rContext )
{
    // for simple expression we don't need to re-bind (if we were bound before)
    // (we will evaluate empty expressions, since they are interpreted as ".")
    if( mxResult.is() && isSimpleExpression() )
        return true;

    bool bResult = _evaluate( rContext, _getExpressionForEvaluation() );

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


} // namespace xforms
