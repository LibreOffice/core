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

















