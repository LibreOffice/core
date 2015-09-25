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


#include "computedexpression.hxx"
#include "unohelper.hxx"
#include "evaluationcontext.hxx"
#include "NameContainer.hxx"

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/xml/dom/NodeType.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/xpath/XPathAPI.hpp>
#include <com/sun/star/xml/xpath/XXPathObject.hpp>
#include <com/sun/star/xml/xpath/XPathExtension.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/SearchAlgorithms.hpp>

#include <osl/diagnose.h>

#include <unotools/textsearch.hxx>
#include <comphelper/processfactory.hxx>

using com::sun::star::beans::NamedValue;
using namespace com::sun::star::uno;
using com::sun::star::lang::XInitialization;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::xml::dom::XNode;
using com::sun::star::container::XNameContainer;
using com::sun::star::xml::xpath::XPathAPI;
using com::sun::star::xml::xpath::XXPathAPI;
using com::sun::star::xml::xpath::XPathExtension;
using com::sun::star::xml::xpath::XXPathExtension;
using com::sun::star::xml::xpath::XXPathObject;
using com::sun::star::xml::xpath::XPathObjectType_XPATH_UNDEFINED;
using com::sun::star::util::SearchOptions;
using com::sun::star::util::SearchAlgorithms_REGEXP;


namespace xforms
{

ComputedExpression::ComputedExpression()
    : msExpression(),
      mbIsEmpty( true ),
      mbIsSimple( true ),
      mxResult()
{
}

ComputedExpression::~ComputedExpression()
{
}



void ComputedExpression::setExpression( const OUString& rExpression )
{
    // set new expression, and clear pre-computed results
    msExpression = rExpression;
    mbIsEmpty = _checkExpression( " *" );
    mbIsSimple = false;
    mxResult.clear();
}


bool ComputedExpression::_checkExpression( const sal_Char* pExpression ) const
{
    assert(pExpression && "no expression?");

    // call RegExp engine
    SearchOptions aSearchOptions;
    aSearchOptions.algorithmType = SearchAlgorithms_REGEXP;
    aSearchOptions.searchString = OUString( pExpression, strlen(pExpression), RTL_TEXTENCODING_ASCII_US );
    utl::TextSearch aTextSearch( aSearchOptions );

    sal_Int32 nLength =  msExpression.getLength();
    sal_Int32 nStart = 0;
    sal_Int32 nEnd = nLength;
    bool bSearch = aTextSearch.SearchForward( msExpression, &nStart, &nEnd );

    // our expression is static only if 1) we found our regexp, and 2)
    // the regexp goes from beginning to end.
    return ( nLength == 0  ||  bSearch )
        && ( nStart == 0  &&  nEnd == nLength );
}

bool ComputedExpression::isSimpleExpression() const
{
    // actual work is done by setExpression
    return mbIsEmpty || mbIsSimple;
}


bool ComputedExpression::_evaluate(
    const xforms::EvaluationContext& rContext,
    const OUString& sExpression )
{
    OSL_ENSURE( rContext.mxContextNode.is(), "no context node in context" );

    // obtain value by evaluating XPath expression
    mxResult.clear();
    try
    {
        mxResult = _getXPathAPI(rContext)->eval( rContext.mxContextNode,
                                                 sExpression );
    }
    catch( const Exception& )
    {
        ; // ignore exception -> mxResult will be empty
    }

    return hasValue();
}

bool ComputedExpression::evaluate( const EvaluationContext& rContext )
{
    // for simple expression we don't need to re-evaluate (if we have
    // an older result); neither for empty expressions
    if( mbIsEmpty || (mxResult.is() && mbIsSimple) )
        return true;

    return _evaluate( rContext, _getExpressionForEvaluation() );
}


bool ComputedExpression::hasValue() const
{
    return mxResult.is() &&
           mxResult->getObjectType() != XPathObjectType_XPATH_UNDEFINED;
}

void ComputedExpression::clear()
{
    mxResult.clear();
}


OUString ComputedExpression::getString( const OUString& rDefault ) const
{
    return mxResult.is() ? mxResult->getString() : rDefault;
}

bool ComputedExpression::getBool( bool bDefault ) const
{
    return mxResult.is() ? mxResult->getBoolean() : bDefault;
}




Reference<XXPathAPI> ComputedExpression::_getXPathAPI(const xforms::EvaluationContext& aContext)
{
    // create XPath API, then register namespaces
    Reference<XXPathAPI> xXPath( XPathAPI::create( comphelper::getProcessComponentContext() ) );

    // register xforms extension#
    Reference< XComponentContext > aComponentContext = comphelper::getProcessComponentContext();
    Reference< XXPathExtension > aExtension = XPathExtension::createWithModel(aComponentContext, aContext.mxModel, aContext.mxContextNode);
    xXPath->registerExtensionInstance(aExtension);

    // register namespaces
    if( aContext.mxNamespaces.is() )
    {
        Sequence<OUString> aPrefixes =aContext.mxNamespaces->getElementNames();
        sal_Int32 nCount = aPrefixes.getLength();
        const OUString* pPrefixes = aPrefixes.getConstArray();
        for( sal_Int32 i = 0; i < nCount; i++ )
        {
            const OUString* pNamePrefix = &pPrefixes[i];
            OUString sNameURL;
            aContext.mxNamespaces->getByName( *pNamePrefix ) >>= sNameURL;
            xXPath->registerNS( *pNamePrefix, sNameURL );
        }
    }

    // done, so return xXPath-object
    return xXPath;
}


} // namespace xforms

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
