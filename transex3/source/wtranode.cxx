/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wtranode.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 08:20:43 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_transex3.hxx"


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

    bIsOnDeleting = TRUE;   // Avoid double deleting of multiple used nodes.

    for (int i = 0; i < C_NR_OF_BRANCHES; i++)
    {
        if (aBranches[i] != 0 ? ! aBranches[i]->IsOnDeleting() : FALSE)
        {
            delete aBranches[i];
        }
    }  // end for
}

















