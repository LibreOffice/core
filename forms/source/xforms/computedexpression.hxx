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

#pragma once

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.hxx>

// forward declaractions
namespace com::sun::star
{
    namespace xml
    {
        namespace dom { class XNode; }
        namespace dom { class XNodeset; }
        namespace xpath { class XXPathAPI; }
        namespace xpath { class XXPathObject; }
    }
    namespace container { class XNameContainer; }
}
namespace xforms { class EvaluationContext; }


namespace xforms
{

/** ComputedExpression represents an XPath Expression and caches results.
 *
 * As this class has no virtual methods, it should never be used
 * polymorphically. */
class ComputedExpression
{
    /// the expression string
    OUString msExpression;

    /// is msExpression empty?
    bool mbIsEmpty;

protected:
    /// is msExpression a simple expression?
    bool mbIsSimple;

    /// the result from the last bind
    css::uno::Reference<css::xml::xpath::XXPathObject> mxResult;


    /// implementation of isSimpleExpression
    bool _checkExpression( const char* pExpression ) const;

    /// allow manipulation of the expression before it is evaluated
    // the default implementation is to do nothing...
    const OUString& _getExpressionForEvaluation() const { return msExpression; }

    /// obtain a (suitable) XPathAPI implementation
    static css::uno::Reference<css::xml::xpath::XXPathAPI> _getXPathAPI(const xforms::EvaluationContext& aContext);

    /// evaluate the expression relative to the content node.
    bool _evaluate( const xforms::EvaluationContext& rContext,
                    const OUString& sExpression );


public:
    ComputedExpression();
    ~ComputedExpression();


    /// get the expression string
    const OUString& getExpression() const { return msExpression;}

    /// set a new expression string
    void setExpression( const OUString& rExpression );

    /// do we have an actual expression?
    bool isEmptyExpression() const { return mbIsEmpty;}

    /// heuristically determine whether this expression is 'simple',
    /// i.e. whether its value will change depending on the values
    /// of other nodes
    bool isSimpleExpression() const;


    /// evaluate the expression relative to the content node.
    bool evaluate( const xforms::EvaluationContext& rContext );


    /// does this expression have a value?
    bool hasValue() const;


    /// remove value/evaluate results
    void clear();


    // get the result of this expression as string/bool/...
    // (Results will be based on the last call of evaluate(..). The caller
    // must call evaluate to ensure current results.)
    css::uno::Reference<css::xml::xpath::XXPathObject> const & getXPath() const { return mxResult;}
    bool getBool( bool bDefault = false ) const;
    OUString getString() const;

};

} // namespace xforms

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
