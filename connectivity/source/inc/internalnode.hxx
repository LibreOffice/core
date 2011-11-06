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


#ifndef _CONNECTIVITY_SQLINTERNALNODE_HXX
#define _CONNECTIVITY_SQLINTERNALNODE_HXX

#include "connectivity/dbtoolsdllapi.hxx"
#include <connectivity/sqlnode.hxx>

namespace connectivity
{
    //==========================================================================
    //= OSQLInternalNode
    //==========================================================================
    /** special node for avoiding memory leaks
    */
    class OSQLInternalNode : public OSQLParseNode
    {
    public:
        OSQLInternalNode(const sal_Char* pNewValue,
                         SQLNodeType eNodeType,
                         sal_uInt32 nNodeID = 0);
        OSQLInternalNode(const ::rtl::OString& _rNewValue,
                         SQLNodeType eNodeType,
                         sal_uInt32 nNodeID = 0);
        OSQLInternalNode(const sal_Unicode* pNewValue,
                         SQLNodeType eNodeType,
                         sal_uInt32 nNodeID = 0);
        OSQLInternalNode(const ::rtl::OUString& _rNewValue,
                         SQLNodeType eNodeType,
                         sal_uInt32 nNodeID = 0);

        virtual ~OSQLInternalNode();
    };
}

#endif  //_CONNECTIVITY_SQLINTERNALNODE_HXX
