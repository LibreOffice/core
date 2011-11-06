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
#include "precompiled_l10ntools.hxx"


#include "wtranode.hxx"


// NOT FULLY DECLARED SERVICES


const ByteString    sEmptyString("");


WTT_Node::WTT_Node( UINT8      i_nValue,
                    WTT_Node * i_pDefaultBranch,
                    WTT_Node * i_pDefaultBranchForAlphas )
    :   nValue(i_nValue),
        eType(token_to_keep),
        sReplaceString(sEmptyString),
        // aBranches,
        bIsOnDeleting(char(0))
{
    int i = 0;
    for ( ; i < C_BR_ALPHABASE; i++ )
    {
        aBranches[i] = i_pDefaultBranch;
    }  // end for
    for ( ; i < C_NR_OF_BRANCHES; i++ )
    {
        aBranches[i] = i_pDefaultBranchForAlphas;
    }
}

void
WTT_Node::SetBranch( UINT8      i_cBranch,
                     WTT_Node * i_pNode )
{
    if (i_cBranch < C_NR_OF_BRANCHES)
    {
        aBranches[i_cBranch] = i_pNode;
    }
}

void
WTT_Node::SetAsTokenToReplace(const ByteString & i_sReplaceString)
{
    sReplaceString = i_sReplaceString;
    eType = token_to_replace;
}

WTT_Node::~WTT_Node()
{
    // Delete the tree hanging below this node:

    bIsOnDeleting = sal_True;   // Avoid double deleting of multiple used nodes.

    for (int i = 0; i < C_NR_OF_BRANCHES; i++)
    {
        if (aBranches[i] != 0 ? ! aBranches[i]->IsOnDeleting() : sal_False)
        {
            delete aBranches[i];
        }
    }  // end for
}

















