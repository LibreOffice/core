/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


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
    WordTransTree is dependant of this class, but NOT the other way!
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
