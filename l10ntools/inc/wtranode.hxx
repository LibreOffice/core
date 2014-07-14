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




#ifndef TX3_WTRANODE_HXX
#define TX3_WTRANODE_HXX

// USED
    // Base Classes
    // Components
    // Parameters
#include <tools/string.hxx>


typedef UINT8 BRANCH_T;



const BRANCH_T C_BR_ALPHABASE   =  4;
const BRANCH_T C_NR_OF_BRANCHES = 34;




/** @task
    This is a node of the parsing-tree which implements the fuctionality of
    class WordTransTree.
    WordTransTree is dependent of this class, but NOT the other way!
**/
class WTT_Node  // WordTransTree-Node
{
  public:
    enum E_TokenType
    {
//      no_token = 0,
        token_to_keep,
        token_to_replace
    };

    // LIFECYCLE
                        WTT_Node(
                            UINT8               i_nValue,   // Own branch-value.
                            WTT_Node *          i_pDefaultBranch,
                            WTT_Node *          i_pDefaultBranchForAlphas );
    void                SetBranch(
                            UINT8               i_cBranch,
                            WTT_Node *          i_pNode );
    void                SetAsTokenToReplace(
                            const ByteString &  i_sReplaceString );
                        ~WTT_Node();

    // OPERATIONS
    WTT_Node *          GetNextNode(
                            UINT8               i_cBranch ); /// [0 .. C_NR_OF_BRANCHES-1], sonst GPF !!!

    // INQUIRY
    E_TokenType         TokenType() const;
    UINT8               Value() const;
    sal_Bool                IsOnDeleting() const;
    const ByteString &  ReplaceString() const;

  private:
    // DATA
    UINT8               nValue;
    E_TokenType         eType;
    ByteString          sReplaceString;
    WTT_Node *          aBranches[C_NR_OF_BRANCHES];    // Mostly DYN pointers.
    char                bIsOnDeleting;
};


inline WTT_Node *
WTT_Node::GetNextNode(UINT8 i_cBranch)
    { return aBranches[i_cBranch]; }
inline WTT_Node::E_TokenType
WTT_Node::TokenType() const
    { return eType; }
inline UINT8
WTT_Node::Value() const
    { return nValue; }
inline sal_Bool
WTT_Node::IsOnDeleting() const
    { return bIsOnDeleting; }
inline const ByteString &
WTT_Node::ReplaceString() const
    { return sReplaceString; }




#endif



