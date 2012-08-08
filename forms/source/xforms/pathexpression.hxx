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

#ifndef _PATHEXPRESSION_HXX
#define _PATHEXPRESSION_HXX

#include "computedexpression.hxx"

#include <vector>

// forward declaractions
namespace com { namespace sun { namespace star
{
    namespace xml { namespace dom
    {
        class XNodeList;
        namespace events { class XEventListener; }
    } }
} } }



namespace xforms
{

/** PathExpression represents an XPath Expression and caches results */
class PathExpression : public ComputedExpression
{
public:
    typedef std::vector<com::sun::star::uno::Reference<com::sun::star::xml::dom::XNode> > NodeVector_t;

private:
    /// the node-list result from the last bind (cached from mxResult)
    NodeVector_t maNodes;

protected:
    /// get expression for evaluation
    const rtl::OUString _getExpressionForEvaluation() const;


public:
    PathExpression();
    ~PathExpression();

    /// set the expression string
    /// (overridden to do remove old listeners)
    /// (also defines simple expressions)
    void setExpression( const rtl::OUString& rExpression );


    /// evaluate the expression relative to the content node.
    bool evaluate( const xforms::EvaluationContext& rContext );


    // get the result of this expression as node/node list/...
    com::sun::star::uno::Reference<com::sun::star::xml::dom::XNode> getNode() const;
    const NodeVector_t getNodeList() const;
    com::sun::star::uno::Reference<com::sun::star::xml::dom::XNodeList> getXNodeList() const;

};

} // namespace xforms

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
