/*************************************************************************
 *
 *  $RCSfile: wtratree.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: nf $ $Date: 2001-06-26 12:56:08 $
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


#ifndef TX3_WTRATREE_HXX
#define TX3_WTRATREE_HXX

// USED
    // Base Classes
    // Components
    // Parameters
#include <tools/string.hxx>

const INT16     C_NR_OF_WTT_RESULTS = 5;
const INT16     C_NR_OF_POSSIBLE_CHARS = 256;


typedef unsigned char u_char;
typedef const char * constr;


class WTT_Node;


/** @task
    This class implements the functionality, that class WordTransformer
    offers.
    WordTransformer is dependant of this class, but NOT the other way!
**/
class WordTransTree
{
  public:
    enum E_Result
    {
        OK = 0,
        HOTKEY_LOST,
        OUTPUT_OVERFLOW
    };


    //  LIFECYCLE
                        WordTransTree(
                            CharSet             i_nWorkingCharSet = RTL_TEXTENCODING_MS_1252);
    void                SetCharSet(
                            CharSet             i_nWorkingCharSet);
                        ~WordTransTree();

    void                AddWordPair(
                            const ByteString &      i_sOldString,
                            const ByteString &      i_sReplaceString );

    // OPERATIONS
    void                InitTransformation(
                            const char *        i_sInput,               /// [!=0], a range of i_nInputLength must be valid memory for read.
                            UINT32              i_nInputLength,
                            UINT32              i_nOutputMaxLength = STRING_MAXLEN - 12 );
    E_Result            TransformNextToken();

    // INQUIRY
    BOOL                TextEndReached() const;
    const char *        Output() const;

        // These 3 functions are valid between two calls of
        //   TransformNextToken():
    E_Result            CurResult() const;
    ByteString          CurReplacedString() const;
    ByteString          CurReplacingString() const;
    char                CurHotkey() const;

  private:
    // SERVICE FUNCTONS
    UINT8               CalculateBranch(
                            u_char              i_cInputChar ) const;

    void                Handle_Hotkey();
    void                Handle_TokenToKeep();
    void                Handle_TokenToTransform();

    // DATA
        // Fixed data
    const u_char *      sInput;
    UINT32              nInputLength;
    const u_char *      pInputEnd;

    u_char *            sOutput;                // DYN
    UINT32              nOutputMaxLength;

    WTT_Node *          dpParsingTreeTop;       // DYN
    WTT_Node *          pUnknownAlpha;
    u_char              cChar2Branch[C_NR_OF_POSSIBLE_CHARS];
    u_char              c_AE, c_OE, c_UE, c_ae, c_oe, c_ue;

        // Working data
    const u_char *      pInputCurTokenStart;
    const u_char *      pInputPosition;
    u_char *            pOutputPosition;
    WTT_Node *          pCurParseNode;

        // Data which are valid only after a completed call to TransformNextToken()
    E_Result            eCurResult;
    u_char              cCurHotkey;             // Letter wich is used as hotkey
    u_char              cCurHotkeySign;         // Letter which is used to assign hotkey ('~'or '&') .
};







inline BOOL
WordTransTree::TextEndReached() const
    { return pInputPosition == pInputEnd; }
inline const char *
WordTransTree::Output() const
    { return TextEndReached() ? (constr) sOutput : ""; }
inline WordTransTree::E_Result
WordTransTree::CurResult() const
    { return eCurResult; }
inline ByteString
WordTransTree::CurReplacedString() const
    { return ByteString((constr) pInputCurTokenStart,pInputPosition-pInputCurTokenStart); }
inline char
WordTransTree::CurHotkey() const
    { return cCurHotkey; }
inline UINT8
WordTransTree::CalculateBranch(u_char i_cInputChar) const
    { return cChar2Branch[i_cInputChar]; }



#endif



