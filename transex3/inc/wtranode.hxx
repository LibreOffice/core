/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wtranode.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:57:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    BOOL                IsOnDeleting() const;
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
inline BOOL
WTT_Node::IsOnDeleting() const
    { return bIsOnDeleting; }
inline const ByteString &
WTT_Node::ReplaceString() const
    { return sReplaceString; }




#endif



