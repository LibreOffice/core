/*************************************************************************
 *
 *  $RCSfile: wtratree.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include "wtratree.hxx"



/** @ATTENTION
    For reasons of speed, class WordTransTree works with two simple
    char arrays, sOutput and sInput, instead of secure containers or
    streams. So be extremely careful, when changing this code!!!
**/



// NOT FULLY DECLARED SERVICES
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "wtranode.hxx"


const BRANCH_T  BR_END          = 0;
const BRANCH_T  BR_NONALPHA     = 1;
const BRANCH_T  BR_HOTKEY       = 2;
const BRANCH_T  BR_BACKSLASH    = 3;
const BRANCH_T  BR_ALPHABASE    = 4;    /// @ATTENTION  All branches not valid for words must be smaller than this value!
const BRANCH_T  BR_AE           = 30;
const BRANCH_T  BR_OE           = 31;
const BRANCH_T  BR_UE           = 32;
const BRANCH_T  BR_SZ           = 33;
const BRANCH_T  BR_MAX          = 34;   /// @ATTENTION  Must be updated always!

const BRANCH_T  BR_START        = 0;





WordTransTree::WordTransTree(CharSet  i_nWorkingCharSet)
    :   sInput(0),
        nInputLength(0),
        pInputEnd(0),
        sOutput(0),
        nOutputMaxLength(0),
        dpParsingTreeTop(0),
        pUnknownAlpha(0),
        // cChar2Branch
        c_AE(u_char('Ä')), c_OE(u_char('Ö')), c_UE(u_char('Ü')),
        c_ae(u_char('ä')),  c_oe(u_char('ö')), c_ue(u_char('ü')),
        pInputCurTokenStart(0),
        pInputPosition(0),
        pOutputPosition(0),
        pCurParseNode(0),
        eCurResult(OK),
        cCurHotkey(0),
        cCurHotkeySign(u_char('~'))
{
    // Initialize parsing tree:
    pUnknownAlpha = new WTT_Node(BR_ALPHABASE,0,0); // This will be deleted as part of the parsing tree.
    for ( UINT8 i = BR_ALPHABASE; i < C_NR_OF_BRANCHES; i++)
    {
        pUnknownAlpha->SetBranch(i,pUnknownAlpha);
    }  // end for

    dpParsingTreeTop = new WTT_Node(BR_START,0,pUnknownAlpha);

    WTT_Node * dpNonAlpha = new WTT_Node(BR_NONALPHA,0,0);

    dpNonAlpha->SetBranch(BR_NONALPHA,dpNonAlpha);
    dpParsingTreeTop->SetBranch(BR_NONALPHA,dpNonAlpha);

    WTT_Node * dpBackslash = new WTT_Node(BR_BACKSLASH,dpNonAlpha,dpNonAlpha);
    dpBackslash->SetBranch(BR_END,0);

    dpParsingTreeTop->SetBranch(BR_BACKSLASH,dpBackslash);
    dpNonAlpha->SetBranch(BR_BACKSLASH,dpBackslash);


    // Initialize character set:
    SetCharSet(i_nWorkingCharSet);

    if (C_BR_ALPHABASE != BR_ALPHABASE || C_NR_OF_BRANCHES != BR_MAX)
    {
        fprintf(stderr, "Assertion failed: file %s line %d.", __FILE__,  __LINE__);
        exit(1);
    }
}

void
WordTransTree::SetCharSet(CharSet i_nWorkingCharSet)
{
    ByteString sConvert("ÄÖÜäöüß");
    const u_char * pConvert = (const u_char * ) ( sConvert.Convert(RTL_TEXTENCODING_MS_1252, i_nWorkingCharSet).GetBuffer() );

    INT16 i = 0;
    for ( ; i < C_NR_OF_POSSIBLE_CHARS; ++i )
    {
        cChar2Branch[i] = BR_NONALPHA;
    }  // end for
    for ( i = 'a'; i <= 'z'; ++i )
    {
        cChar2Branch[i] = BR_ALPHABASE + i - 'a';
    }  // end for
    for ( i = 'A'; i <= 'Z'; ++i )
    {
        cChar2Branch[i] = BR_ALPHABASE + i - 'A';
    }  // end for
    cChar2Branch[pConvert[0]] = BR_AE;
    cChar2Branch[pConvert[1]] = BR_OE;
    cChar2Branch[pConvert[2]] = BR_UE;
    cChar2Branch[pConvert[3]] = BR_AE;
    cChar2Branch[pConvert[4]] = BR_OE;
    cChar2Branch[pConvert[5]] = BR_UE;
    cChar2Branch[pConvert[6]] = BR_SZ;

    cChar2Branch[u_char('~')] = BR_HOTKEY;
    cChar2Branch[u_char('&')] = BR_HOTKEY;


    c_AE = pConvert[0];
    c_OE = pConvert[1];
    c_UE = pConvert[2];
    c_ae = pConvert[3];
    c_oe = pConvert[4];
    c_ue = pConvert[5];
}

WordTransTree::~WordTransTree()
{
    delete dpParsingTreeTop;
    if (sOutput != 0)
        delete [] sOutput;
}

void
WordTransTree::AddWordPair( const ByteString &      i_sOldString,
                            const ByteString &      i_sReplaceString )
{
    if (i_sOldString.Len() == 0)
        return;

    pCurParseNode = dpParsingTreeTop;
    WTT_Node * pBranch = 0;
    char cBranch = 0;

    for ( constr pOld = i_sOldString.GetBuffer();
          *pOld != 0;
          pOld++ )
    {
        cBranch = CalculateBranch(*pOld);
        pBranch = pCurParseNode->GetNextNode(cBranch);
        if (pBranch == 0 || pBranch == pUnknownAlpha)
        {
            pBranch = new WTT_Node(cBranch,0,pUnknownAlpha);
            pCurParseNode->SetBranch(cBranch,pBranch);
        }
        pCurParseNode = pBranch;
    }   // end for
    pCurParseNode->SetAsTokenToReplace(i_sReplaceString);
}

void
WordTransTree::InitTransformation( const char * i_sInput,
                                   UINT32       i_nInputLength,
                                   UINT32       i_nOutputMaxLength )
{
    sInput = (const u_char *)i_sInput;
    nInputLength = i_nInputLength;
    pInputEnd = &sInput[i_nInputLength];

    pInputCurTokenStart = sInput;
    pInputPosition = sInput;

    if (nOutputMaxLength < i_nOutputMaxLength)
    {
        if (sOutput != 0)
            delete [] sOutput;
        sOutput = new unsigned char[i_nOutputMaxLength];
        nOutputMaxLength = i_nOutputMaxLength;
    }
    pOutputPosition = sOutput;
}

/** pInputCurTokenStart and CurParseNode are updated just when
    starting this function. After its end they must not be changed
    till this functon is called again.
    Outside this function pInputPositon and pOutputPosition are both
    on the first not transformed char in their respective array.
**/
WordTransTree::E_Result
WordTransTree::TransformNextToken()
{
    pInputCurTokenStart = pInputPosition;
    pCurParseNode = dpParsingTreeTop;
    cCurHotkey = 0;
    eCurResult = OK;

    WTT_Node * pBranch = 0;
    UINT8 cBranch = 0;

    for ( pCurParseNode = dpParsingTreeTop;
          pInputPosition != pInputEnd;
          ++pInputPosition )
    {
        cBranch = CalculateBranch(*pInputPosition);
        pBranch = pCurParseNode->GetNextNode( cBranch );
        if (pBranch != 0)
        {
            pCurParseNode = pBranch;
        }
        else
        {
            if (cBranch == BR_HOTKEY)   // current letter is '~' or '&'.
            {
                // Logic of the following. There are 9 possible cases -
                // A = alphabetic letter, NA = non alphabetic, TB = token begin,
                // Eot = end of text:
                //   1. A~A          set hotkey to following letter, continue
                //   2. A~NA         token end
                //   3. A~Eot        token end
                //   4. NA~A         token end
                //   5. NA~NA        continue
                //   6. A~Eof        continue
                //   7. TB~A         set hotkey to following letter, continue
                //   8. TB~NA        continue
                //   9. TB~Eot       continue

                // bNext and Prev are true, if there are alphabetic letters:
                BOOL bNext =  pInputPosition + 1 != pInputEnd
                                    ?   CalculateBranch(pInputPosition[1]) >= BR_ALPHABASE
                                    :   FALSE;
                BOOL bPrev = pCurParseNode->Value() >= BR_ALPHABASE;

                if ( bNext && (bPrev || pCurParseNode == dpParsingTreeTop) )
                {   // case 1. and 7.
                    Handle_Hotkey();
                    continue;
                }
                else if  (!bPrev && !bNext)
                {   // case 5.,6.,8.,9.
                    continue;
                }

                // Case 2.,3.,4. :
                //  so this should be handled as an end of a token.
            }
            if (pCurParseNode->TokenType() == WTT_Node::token_to_keep)
            {
                Handle_TokenToKeep();
                return eCurResult;
            }
            else
            {
                Handle_TokenToTransform();
                return eCurResult;
            }   // endif (pCurParseNode->TokenType() == WTT_Node::token_to_keep)
        }   // endif (pBranch == 0) else
    }   // end for

    // If here, the text end is reached
    if (pCurParseNode->TokenType() == WTT_Node::token_to_keep)
    {
        Handle_TokenToKeep();
        return eCurResult;
    }
    else
    {
        Handle_TokenToTransform();
        return eCurResult;
    }
}

ByteString
WordTransTree::CurReplacingString() const
{
    return pCurParseNode->ReplaceString();
}

void
WordTransTree::Handle_Hotkey()
{
    if (cCurHotkey == 0)    // Avoid to replace the first found hotkey by
                            //   a later one - though this shouldn't happen anyway.
    {
        cCurHotkey = (pInputPosition+1) != pInputEnd ? pInputPosition[1] : 0;
        cCurHotkeySign = *pInputPosition;
    }
}

void
WordTransTree::Handle_TokenToKeep()
{
    UINT32 nTokenLength = pInputPosition-pInputCurTokenStart;

    memcpy(pOutputPosition,pInputCurTokenStart,nTokenLength);

    pOutputPosition += nTokenLength;
    *pOutputPosition = '\0';
}

void
WordTransTree::Handle_TokenToTransform()
{
    BOOL bHaveHotkey = CalculateBranch(cCurHotkey) >= BR_ALPHABASE;
    const ByteString & rReplace = pCurParseNode->ReplaceString();

    // Find position of hotkey in replace-string:
    USHORT nHotkeyPos = bHaveHotkey
                            ?   rReplace.Search(char(cCurHotkey))
                            :   STRING_NOTFOUND;
    if (nHotkeyPos == STRING_NOTFOUND && bHaveHotkey)
    {
        if (cCurHotkey < 128)
        {
            if (islower(cCurHotkey))
                nHotkeyPos = rReplace.Search(toupper(char(cCurHotkey)));
            else
                nHotkeyPos = rReplace.Search(tolower(char(cCurHotkey)));
        }
        else    // cCurHotkey >= 128
        {
            if (cCurHotkey == c_ae)
                nHotkeyPos = rReplace.Search(char(c_AE));
            else if (cCurHotkey == c_oe)
                nHotkeyPos = rReplace.Search(char(c_OE));
            else if (cCurHotkey == c_ue)
                nHotkeyPos = rReplace.Search(char(c_UE));
            else if (cCurHotkey == c_AE)
                nHotkeyPos = rReplace.Search(char(c_ae));
            else if (cCurHotkey == c_OE)
                nHotkeyPos = rReplace.Search(char(c_oe));
            else if (cCurHotkey == c_UE)
                nHotkeyPos = rReplace.Search(char(c_ue));
        }   // endif (cCurHotkey < 128) else

        if (nHotkeyPos == STRING_NOTFOUND)
        {
            eCurResult = HOTKEY_LOST;
            bHaveHotkey = FALSE;
        }
    }   // endif (nHotkeyPos == STRING_NOT_FOUND && bHaveHotkey)


    UINT32 nOutputTokenLength = rReplace.Len() + (bHaveHotkey ? 1 : 0);

    if (bHaveHotkey)
    {
        memcpy( pOutputPosition,
                pCurParseNode->ReplaceString().GetBuffer(),
                nHotkeyPos );
        *(pOutputPosition + nHotkeyPos) = cCurHotkeySign;
        memcpy( pOutputPosition + nHotkeyPos + 1,
                pCurParseNode->ReplaceString().GetBuffer() + nHotkeyPos,
                nOutputTokenLength - nHotkeyPos - 1);
    }
    else
    {
        memcpy( pOutputPosition,
                pCurParseNode->ReplaceString().GetBuffer(),
                nOutputTokenLength );
    }

    // Convert first letter into upper if necessary:
    u_char cInStart = CalculateBranch(*pInputCurTokenStart) == BR_HOTKEY
                            ?   pInputCurTokenStart[1]
                            :   pInputCurTokenStart[0] ;
    u_char * pOutStart = nHotkeyPos == 0
                            ?   pOutputPosition + 1
                            :   pOutputPosition ;
    if (isupper(cInStart) || cInStart > 127)
    {   // Possibly cInStart is upper character:
        if (isupper(cInStart) || cInStart == c_AE || cInStart == c_OE || cInStart == c_UE)
        {   // Surely cInStart is upper character:
            u_char cOutStart = *pOutStart;
            if (cOutStart < 128)
                *pOutStart = toupper(cOutStart);
            else if (cOutStart == c_ae)
                *pOutStart = c_AE;
            else if (cOutStart == c_oe)
                *pOutStart = c_OE;
            else if (cOutStart == c_ue)
                *pOutStart = c_UE;
        }
    }   // endif (isupper(cInStart) || cInStart > 127)

    pOutputPosition += nOutputTokenLength;
    *pOutputPosition = '\0';
}

