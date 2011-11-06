/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "internalnode.hxx"

#include <algorithm>
#include <connectivity/sqlparse.hxx>

using namespace connectivity;

//-----------------------------------------------------------------------------
OSQLInternalNode::OSQLInternalNode(const sal_Char* pNewValue,
                                   SQLNodeType eNodeType,
                                   sal_uInt32 nNodeID)
                 : OSQLParseNode(pNewValue,eNodeType,nNodeID)
{
    OSL_ENSURE(OSQLParser::s_pGarbageCollector, "Collector not initialized");
    (*OSQLParser::s_pGarbageCollector)->push_back(this);
}

//-----------------------------------------------------------------------------
OSQLInternalNode::OSQLInternalNode(const ::rtl::OString &_NewValue,
                                 SQLNodeType eNodeType,
                                 sal_uInt32 nNodeID)
                :OSQLParseNode(_NewValue,eNodeType,nNodeID)
{
    OSL_ENSURE(OSQLParser::s_pGarbageCollector, "Collector not initialized");
    (*OSQLParser::s_pGarbageCollector)->push_back(this);
}

//-----------------------------------------------------------------------------
OSQLInternalNode::OSQLInternalNode(const sal_Unicode* pNewValue,
                                   SQLNodeType eNodeType,
                                   sal_uInt32 nNodeID)
                 :OSQLParseNode(pNewValue,eNodeType,nNodeID)
{
    OSL_ENSURE(OSQLParser::s_pGarbageCollector, "Collector not initialized");
    (*OSQLParser::s_pGarbageCollector)->push_back(this);
}

//-----------------------------------------------------------------------------
OSQLInternalNode::OSQLInternalNode(const ::rtl::OUString &_NewValue,
                                 SQLNodeType eNodeType,
                                 sal_uInt32 nNodeID)
                :OSQLParseNode(_NewValue,eNodeType,nNodeID)
{
    OSL_ENSURE(OSQLParser::s_pGarbageCollector, "Collector not initialized");
    (*OSQLParser::s_pGarbageCollector)->push_back(this);
}


//-----------------------------------------------------------------------------
OSQLInternalNode::~OSQLInternalNode()
{
    // remove the node from the garbage list

    OSL_ENSURE(OSQLParser::s_pGarbageCollector, "Collector not initialized");
    (*OSQLParser::s_pGarbageCollector)->erase(this);
}
