/*************************************************************************
 *
 *  $RCSfile: computedexpression.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 10:49:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "computedexpression.hxx"
#include "unohelper.hxx"
#include "evaluationcontext.hxx"
#include "NameContainer.hxx"

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/xml/dom/NodeType.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/xpath/XXPathAPI.hpp>
#include <com/sun/star/xml/xpath/XXPathObject.hpp>
#include <com/sun/star/xml/xpath/XXPathExtension.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <unotools/textsearch.hxx>
#include <comphelper/processfactory.hxx>

using rtl::OUString;
using com::sun::star::beans::NamedValue;
using com::sun::star::uno::Any;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::lang::XInitialization;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::xml::dom::XNode;
using com::sun::star::container::XNameContainer;
using com::sun::star::xml::xpath::XXPathAPI;
using com::sun::star::xml::xpath::XXPathExtension;
using com::sun::star::xml::xpath::XXPathObject;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::UNO_QUERY_THROW;
using com::sun::star::xml::xpath::XPathObjectType_XPATH_UNDEFINED;




namespace xforms
{

ComputedExpression::ComputedExpression()
    : msExpression(),
      mxNamespaces( new NameContainer<OUString>() ),
      mbIsEmpty( true ),
      mbIsSimple( true ),
      mxResult()
{
}

ComputedExpression::~ComputedExpression()
{
}


OUString ComputedExpression::getExpression() const
{
    return msExpression;
}

void ComputedExpression::setExpression( const OUString& rExpression )
{
    // set new expression, and clear pre-computed results
    msExpression = rExpression;
    mbIsEmpty = _checkExpression( " *" );
    mbIsSimple = false;
    mxResult.clear();
}


Reference<XNameContainer> ComputedExpression::getNamespaces() const
{
    return mxNamespaces;
}

void ComputedExpression::setNamespaces(
    const Reference<XNameContainer>& xNamespaces )
{
    OSL_ENSURE( xNamespaces.is(), "need namespaces" );
    OSL_ENSURE( xNamespaces->getElementType() ==
                getCppuType( static_cast<OUString*>( NULL ) ),
                "namespaces must be string container" );

    mxNamespaces = xNamespaces;
}


bool ComputedExpression::_checkExpression( const sal_Char* pExpression ) const
{
    OSL_ENSURE( pExpression != NULL, "no expression?" );

    // call RegExp engine
    utl::SearchParam aSearchParam( String( pExpression,
                                           RTL_TEXTENCODING_ASCII_US ),
                                   utl::SearchParam::SRCH_REGEXP );
    // invalid locale! (we don't use char classes)
    utl::TextSearch aTextSearch( aSearchParam, 0 );

    xub_StrLen nLength =
        static_cast<xub_StrLen>( msExpression.getLength() );
    xub_StrLen nStart = 0;
    xub_StrLen nEnd = nLength;
    int nSearch = aTextSearch.SearchFrwrd( msExpression, &nStart, &nEnd );

    // our expression is static only if 1) we found our regexp, and 2)
    // the regexp goes from beginning to end.
    return ( nLength == 0  ||  nSearch != 0 )
        && ( nStart == 0  &&  nEnd == nLength );
}

/// do we have an actual expression?
bool ComputedExpression::isEmptyExpression() const
{
    return mbIsEmpty;
}

bool ComputedExpression::isSimpleExpression() const
{
    // actual work is done by setExpression
    return mbIsEmpty || mbIsSimple;
}


const OUString ComputedExpression::_getExpressionForEvaluation() const
{
    // the default implementation is to do nothing...
    return msExpression;
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
    catch( const RuntimeException& )
    {
        ; // ignore exception -> mxResult will be empty
    }

    return hasValue();
}

bool ComputedExpression::evaluate( const EvaluationContext& rContext )
{
    // for simple expression we don't need to re-evaluate (if we have
    // an older result); neither for empty expressions
    if( mbIsEmpty  ||  mxResult.is() && mbIsSimple )
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

Reference<XXPathObject> ComputedExpression::getXPath()
{
    return mxResult;
}

OUString ComputedExpression::getString( const rtl::OUString& rDefault ) const
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
    Reference<XXPathAPI> xXPath( createInstance(
                            OUSTRING( "com.sun.star.xml.xpath.XPathAPI" ) ),
                                 UNO_QUERY_THROW );
    OSL_ENSURE( xXPath.is(), "cannot get XPath API" );

    // register xforms extension#
    Sequence< Any > aSequence(2);
    NamedValue aValue;
    aValue.Name = OUSTRING("Model");
    aValue.Value <<= aContext.mxModel;
    aSequence[0] <<= aValue;
    aValue.Name = OUSTRING("ContextNode");
    aValue.Value <<= aContext.mxContextNode;
    aSequence[1] <<= aValue;
    Reference<XMultiServiceFactory> aFactory = comphelper::getProcessServiceFactory();
    Reference< XXPathExtension > aExtension( aFactory->createInstanceWithArguments(
        OUSTRING( "com.sun.star.comp.xml.xpath.XFormsExtension"), aSequence), UNO_QUERY_THROW);
    xXPath->registerExtensionInstance(aExtension);

    // register namespaces
    OSL_ENSURE( mxNamespaces.is(), "no namespaces!" );
    Sequence<OUString> aPrefixes = mxNamespaces->getElementNames();
    sal_Int32 nCount = aPrefixes.getLength();
    const OUString* pPrefixes = aPrefixes.getConstArray();
    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        const OUString* pNamePrefix = &pPrefixes[i];
        OUString sNameURL;
        mxNamespaces->getByName( *pNamePrefix ) >>= sNameURL;
        xXPath->registerNS( *pNamePrefix, sNameURL );
    }

    // done, so return xXPath-object
    return xXPath;
}


} // namespace xforms
