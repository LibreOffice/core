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

#include "computedexpression.hxx"

#include <vector>

// forward declaractions
namespace com::sun::star::xml::dom
{
class XNodeList;
namespace events
{
class XEventListener;
}
}

namespace xforms
{
/** PathExpression represents an XPath Expression and caches results */
class PathExpression final : public ComputedExpression
{
public:
    typedef std::vector<css::uno::Reference<css::xml::dom::XNode>> NodeVector_t;

private:
    /// the node-list result from the last bind (cached from mxResult)
    NodeVector_t maNodes;

    /// get expression for evaluation
    OUString _getExpressionForEvaluation() const;

public:
    PathExpression();
    ~PathExpression();

    /// set the expression string
    /// (overridden to do remove old listeners)
    /// (also defines simple expressions)
    void setExpression(const OUString& rExpression);

    /// evaluate the expression relative to the content node.
    void evaluate(const xforms::EvaluationContext& rContext);

    // get the result of this expression as node/node list/...
    css::uno::Reference<css::xml::dom::XNode> getNode() const;
    const NodeVector_t& getNodeList() const { return maNodes; }
    css::uno::Reference<css::xml::dom::XNodeList> getXNodeList() const;
};

} // namespace xforms

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
