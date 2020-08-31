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

#include "rulelistcontext.hxx"
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

namespace oox::drawingml
{
RuleListContext::RuleListContext(ContextHandler2Helper const& rParent, const LayoutAtomPtr& pNode)
    : ContextHandler2(rParent)
    , mpNode(pNode)
{
    assert(pNode);
}

RuleListContext::~RuleListContext() = default;

core::ContextHandlerRef RuleListContext::onCreateContext(sal_Int32 nElement,
                                                         const AttributeList& rAttribs)
{
    switch (nElement)
    {
        case DGM_TOKEN(rule):
        {
            auto pNode = std::make_shared<RuleAtom>(mpNode->getLayoutNode());
            LayoutAtom::connect(mpNode, pNode);

            Rule& rRule = pNode->getRule();
            rRule.msForName = rAttribs.getString(XML_forName, "");
            break;
        }
        default:
            break;
    }

    return this;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
