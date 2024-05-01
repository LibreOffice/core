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

#include <connectivity/sqlparse.hxx>
#include <connectivity/internalnode.hxx>

using namespace connectivity;


OSQLInternalNode::OSQLInternalNode(const char* pNewValue,
                                   SQLNodeType eNodeType,
                                   sal_uInt32 nNodeID)
                 : OSQLParseNode(pNewValue,eNodeType,nNodeID)
{
    assert(OSQLParser::s_pGarbageCollector && "Collector not initialized");
    (*OSQLParser::s_pGarbageCollector)->push_back(this);
}


OSQLInternalNode::OSQLInternalNode(std::string_view NewValue,
                                 SQLNodeType eNodeType,
                                 sal_uInt32 nNodeID)
                :OSQLParseNode(NewValue,eNodeType,nNodeID)
{
    assert(OSQLParser::s_pGarbageCollector && "Collector not initialized");
    (*OSQLParser::s_pGarbageCollector)->push_back(this);
}

OSQLInternalNode::OSQLInternalNode(const OUString &NewValue,
                                 SQLNodeType eNodeType,
                                 sal_uInt32 nNodeID)
                :OSQLParseNode(NewValue,eNodeType,nNodeID)
{
    assert(OSQLParser::s_pGarbageCollector && "Collector not initialized");
    (*OSQLParser::s_pGarbageCollector)->push_back(this);
}

OSQLInternalNode::~OSQLInternalNode()
{
    // remove the node from the garbage list
    assert(OSQLParser::s_pGarbageCollector && "Collector not initialized");
    (*OSQLParser::s_pGarbageCollector)->erase(this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
