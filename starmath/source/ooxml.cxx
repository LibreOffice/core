/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Lubos Lunak <l.lunak@suse.cz> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_starmath.hxx"

#include "ooxml.hxx"

#include <oox/token/tokens.hxx>

using namespace oox;
using namespace oox::core;

// TODO duped from MathType

static sal_Unicode Convert(sal_Unicode nIn)
{
    //Find the best match in accepted unicode for our private area symbols
    static sal_Unicode aStarMathPrivateToUnicode[] =
    {
        0x2030, 0xF613, 0xF612, 0x002B, 0x003C, 0x003E, 0xE425, 0xE421, 0xE088, 0x2208,
        0x0192, 0x2026, 0x2192, 0x221A, 0x221A, 0x221A, 0xE090, 0x005E, 0x02C7, 0x02D8,
        0x00B4, 0x0060, 0x02DC, 0x00AF, 0x0362, 0xE099, 0xE09A, 0x20DB, 0xE09C, 0xE09D,
        0x0028, 0x0029, 0x2220, 0x22AF, 0xE0A2, 0xE0A3, 0xE0A4, 0xE0A5, 0xE0A6, 0xE0A7,
        0x002F, 0x005C, 0x274F, 0xE0AB, 0x0393, 0x0394, 0x0398, 0x039b, 0x039e, 0x03A0,
        0x03a3, 0x03a5, 0x03a6, 0x03a8, 0x03A9, 0x03B1, 0x03B2, 0x03b3, 0x03b4, 0x03b5,
        0x03b6, 0x03b7, 0x03b8, 0x03b9, 0x03ba, 0x03bb, 0x03bc, 0x03bd, 0x03be, 0x03bf,
        0x03c0, 0x03c1, 0x03c3, 0x03c4, 0x03c5, 0x03c6, 0x03c7, 0x03c8, 0x03c9, 0x03b5,
        0x03d1, 0x03d6, 0xE0D2, 0x03db, 0x2118, 0x2202, 0x2129, 0xE0D7, 0xE0D8, 0x22A4,
        0xE0DA, 0x2190, 0x2191, 0x2193
    };
    if ((nIn >= 0xE080) && (nIn <= 0xE0DD))
        nIn = aStarMathPrivateToUnicode[nIn-0xE080];

    //For whatever unicode glyph that equation editor doesn't ship with that
    //we have a possible match we can munge it to.
    switch (nIn)
    {
        case 0x2223:
            nIn = '|';
            break;
        default:
            break;
    }

    return nIn;
}

SmOoxml::SmOoxml(String &rIn,SmNode *pIn,OoxmlVersion v)
: str( rIn )
, pTree( pIn )
, version( v )
{
}

bool SmOoxml::ConvertFromStarMath( ::sax_fastparser::FSHelperPtr serializer )
{
    if( pTree == NULL )
        return false;
    m_pSerializer = serializer;
    m_pSerializer->startElementNS( XML_m, XML_oMath,
        FSNS( XML_xmlns, XML_m ), "http://schemas.openxmlformats.org/officeDocument/2006/math", FSEND );
    HandleNode( pTree, 0 );
    m_pSerializer->endElementNS( XML_m, XML_oMath );
    return true;
}

void SmOoxml::HandleNode(SmNode *pNode,int nLevel)
{
    fprintf(stderr,"XX %d %d %d\n", nLevel, pNode->GetType(), pNode->GetNumSubNodes());
    switch(pNode->GetType())
    {
#if 0
        case NATTRIBUT:
            HandleAttributes(pNode,nLevel);
            break;
#endif
        case NTEXT:
            HandleText(pNode,nLevel);
            break;
#if 0
        case NVERTICAL_BRACE:
            HandleVerticalBrace(pNode,nLevel);
            break;
        case NBRACE:
            HandleBrace(pNode,nLevel);
            break;
        case NOPER:
            HandleOperator(pNode,nLevel);
            break;
#endif
        case NBINHOR:
            HandleBinaryOperation(pNode,nLevel);
            break;
        case NBINVER:
            HandleFractions(pNode,nLevel);
            break;
#if 0
        case NROOT:
            HandleRoot(pNode,nLevel);
            break;
        case NSPECIAL:
            {
            SmTextNode *pText=(SmTextNode *)pNode;
            //if the token str and the result text are the same then this
            //is to be seen as text, else assume its a mathchar
            if (pText->GetText() == pText->GetToken().aText)
                HandleText(pText,nLevel);
            else
                HandleMath(pText,nLevel);
            }
            break;
#endif
        case NMATH:
            HandleMath(pNode,nLevel);
            break;
#if 0
        case NSUBSUP:
            HandleSubSupScript(pNode,nLevel);
            break;
#endif
        case NEXPRESSION:
            HandleAllSubNodes( pNode, nLevel );
            break;
        case NTABLE:
            //Root Node, PILE equivalent, i.e. vertical stack
            HandleTable(pNode,nLevel);
            break;
#if 0
        case NMATRIX:
            HandleSmMatrix((SmMatrixNode *)pNode,nLevel);
            break;
#endif
        case NLINE:
            {
// TODO
            HandleAllSubNodes( pNode, nLevel );
            }
            break;
#if 0
        case NALIGN:
            HandleMAlign(pNode,nLevel);
            break;
        case NPLACE:
            // explicitly do nothing, MSOffice treats that as a placeholder if item is missing
            break;
        case NBLANK:
            *pS << sal_uInt8(CHAR);
            *pS << sal_uInt8(0x98);
            if (pNode->GetToken().eType == TSBLANK)
                *pS << sal_uInt16(0xEB04);
            else
                *pS << sal_uInt16(0xEB05);
            break;
#endif
        default:
            HandleAllSubNodes( pNode, nLevel );
            break;
    }
}

//Root Node, PILE equivalent, i.e. vertical stack
void SmOoxml::HandleTable(SmNode *pNode,int nLevel)
{
    //The root of the starmath is a table, if
    //we convert this them each iteration of
    //conversion from starmath to OOXML will
    //add an extra unnecessary level to the
    //OOXML output stack which would grow
    //without bound in a multi step conversion
    if( nLevel || pNode->GetNumSubNodes() > 1 )
        HandleVerticalStack( pNode, nLevel, 0 );
    else
        HandleAllSubNodes( pNode, nLevel );
}

void SmOoxml::HandleAllSubNodes( SmNode* pNode, int nLevel )
{
    int size = pNode->GetNumSubNodes();
    for( int i = 0;
         i < size;
         ++i )
    {
// TODO remove when all types of nodes are handled properly
        if( pNode->GetSubNode( i ) == NULL )
        {
            OSL_FAIL( "Subnode is NULL, parent node not handled properly" );
            continue;
        }
        HandleNode( pNode->GetSubNode( i ), nLevel + 1 );
    }
}

// output vertical stack, firstItem says which child to use as first (if there
// are more than two children, OOXML can have only a vertical stack of two items,
// so create a bigger vertical stack recursively)
void SmOoxml::HandleVerticalStack( SmNode* pNode, int nLevel, int firstItem )
{
    if( firstItem == pNode->GetNumSubNodes() - 1 ) // only one item, just output the item
    {
        HandleNode( pNode->GetSubNode( firstItem ), nLevel + 1 );
        return;
    }
    m_pSerializer->startElementNS( XML_m, XML_f, FSEND );
    m_pSerializer->startElementNS( XML_m, XML_fPr, FSEND );
    m_pSerializer->singleElementNS( XML_m, XML_type, FSNS( XML_m, XML_val ), "noBar", FSEND );
    m_pSerializer->endElementNS( XML_m, XML_fPr );
    m_pSerializer->startElementNS( XML_m, XML_num, FSEND );
    HandleNode( pNode->GetSubNode( firstItem ), nLevel + 1 );
    m_pSerializer->endElementNS( XML_m, XML_num );
    // TODO this nesting means MSOffice will use smaller fonts for nested items,
    // not sure if there's another way to represent a bigger stack than 2 items
    m_pSerializer->startElementNS( XML_m, XML_den, FSEND );
    HandleVerticalStack( pNode, nLevel, firstItem + 1 );
    m_pSerializer->endElementNS( XML_m, XML_den );
    m_pSerializer->endElementNS( XML_m, XML_f );
}

void SmOoxml::HandleText(SmNode *pNode, int /*nLevel*/)
{
    m_pSerializer->startElementNS( XML_m, XML_r, FSEND );

    if( version == ECMA_DIALECT )
    { // HACK: MSOffice2007 does not import characters properly unless this font is explicitly given
        m_pSerializer->startElementNS( XML_w, XML_rPr, FSEND );
        m_pSerializer->singleElementNS( XML_w, XML_rFonts, FSNS( XML_w, XML_ascii ), "Cambria Math",
            FSNS( XML_w, XML_hAnsi ), "Cambria Math", FSEND );
        m_pSerializer->endElementNS( XML_w, XML_rPr );
    }
    m_pSerializer->startElementNS( XML_m, XML_t, FSEND );
    SmTextNode *pTemp=(SmTextNode *)pNode;
    fprintf(stderr, "T %s\n", rtl::OUStringToOString( pTemp->GetText(), RTL_TEXTENCODING_UTF8 ).getStr());
    for(xub_StrLen i=0;i<pTemp->GetText().Len();i++)
    {
#if 0
        if ((nPendingAttributes) &&
            (i == ((pTemp->GetText().Len()+1)/2)-1))
        {
            *pS << sal_uInt8(0x22);     //char, with attributes right
                                //after the character
        }
        else
            *pS << sal_uInt8(CHAR);

        sal_uInt8 nFace = 0x1;
        if (pNode->GetFont().GetItalic() == ITALIC_NORMAL)
            nFace = 0x3;
        else if (pNode->GetFont().GetWeight() == WEIGHT_BOLD)
            nFace = 0x7;
        *pS << sal_uInt8(nFace+128); //typeface
#endif
        sal_uInt16 nChar = pTemp->GetText().GetChar(i);
        m_pSerializer->write( rtl::OUString( Convert(nChar)));

#if 0
        //Mathtype can only have these sort of character
        //attributes on a single character, starmath can put them
        //anywhere, when the entity involved is a text run this is
        //a large effort to place the character attribute on the
        //central mathtype character so that it does pretty much
        //what the user probably has in mind. The attributes
        //filled in here are dummy ones which are replaced in the
        //ATTRIBUT handler if a suitable location for the
        //attributes was found here. Unfortunately it is
        //possible for starmath to place character attributes on
        //entities which cannot occur in mathtype e.g. a Summation
        //symbol so these attributes may be lost
        if ((nPendingAttributes) &&
            (i == ((pTemp->GetText().Len()+1)/2)-1))
        {
            *pS << sal_uInt8(EMBEL);
            while (nPendingAttributes)
            {
                *pS << sal_uInt8(2);
                //wedge the attributes in here and clear
                //the pending stack
                nPendingAttributes--;
            }
            nInsertion=pS->Tell();
            *pS << sal_uInt8(END); //end embel
            *pS << sal_uInt8(END); //end embel
        }
#endif
    }
    m_pSerializer->endElementNS( XML_m, XML_t );
    m_pSerializer->endElementNS( XML_m, XML_r );
}

void SmOoxml::HandleMath(SmNode *pNode,int nLevel)
{
    fprintf(stderr,"MATH %d\n", pNode->GetToken().eType);
    // these are handled elsewhere, e.g. when handling BINHOR
    OSL_ASSERT( pNode->GetToken().eType != TDIVIDEBY );
    HandleText( pNode, nLevel );
// TODO at least some items (e.g. y/2 need to handled as ooxml and not as plain text symbols)
#if 0
    if (pNode->GetToken().eType == TMLINE)
    {
        *pS << sal_uInt8(END);
        *pS << sal_uInt8(LINE);
        bIsReInterpBrace=1;
        return;
    }
    SmMathSymbolNode *pTemp=(SmMathSymbolNode *)pNode;
    for(xub_StrLen i=0;i<pTemp->GetText().Len();i++)
    {
        sal_Unicode nArse = Convert(pTemp->GetText().GetChar(i));
        if ((nArse == 0x2224) || (nArse == 0x2288) || (nArse == 0x2285) ||
            (nArse == 0x2289))
        {
            *pS << sal_uInt8(CHAR|0x20);
        }
        else if ((nPendingAttributes) &&
                (i == ((pTemp->GetText().Len()+1)/2)-1))
            {
                *pS << sal_uInt8(0x22);
            }
        else
            *pS << sal_uInt8(CHAR); //char without formula recognition
        //The typeface seems to be MTEXTRA for unicode characters,
        //though how to determine when mathtype chooses one over
        //the other is unknown. This should do the trick
        //nevertheless.
        sal_uInt8 nBias;
        if ( (nArse == 0x2213) || (nArse == 0x2218) ||
            (nArse == 0x210F) || (
                (nArse >= 0x22EE) && (nArse <= 0x22FF)
            ))
        {
            nBias = 0xB; //typeface
        }
        else if ((nArse > 0x2000) || (nArse == 0x00D7))
            nBias = 0x6; //typeface
        else if (nArse == 0x3d1)
            nBias = 0x4;
        else if ((nArse > 0xFF) && ((nArse < 0x393) || (nArse > 0x3c9)))
            nBias = 0xB; //typeface
        else if ((nArse == 0x2F) || (nArse == 0x2225))
            nBias = 0x2; //typeface
        else
            nBias = 0x3; //typeface

        *pS << sal_uInt8(nSpec+nBias+128); //typeface

        if (nArse == 0x2224)
        {
            *pS << sal_uInt16(0x7C);
            *pS << sal_uInt8(EMBEL);
            *pS << sal_uInt8(0x0A);
            *pS << sal_uInt8(END); //end embel
            *pS << sal_uInt8(END); //end embel
        }
        else if (nArse == 0x2225)
            *pS << sal_uInt16(0xEC09);
        else if (nArse == 0xE421)
            *pS << sal_uInt16(0x2265);
        else if (nArse == 0x230A)
            *pS << sal_uInt16(0xF8F0);
        else if (nArse == 0x230B)
            *pS << sal_uInt16(0xF8FB);
        else if (nArse == 0xE425)
            *pS << sal_uInt16(0x2264);
        else if (nArse == 0x226A)
        {
            *pS << sal_uInt16(0x3C);
            *pS << sal_uInt8(CHAR);
            *pS << sal_uInt8(0x98);
            *pS << sal_uInt16(0xEB01);
            *pS << sal_uInt8(CHAR);
            *pS << sal_uInt8(0x86);
            *pS << sal_uInt16(0x3c);
        }
        else if (nArse == 0x2288)
        {
            *pS << sal_uInt16(0x2286);
            *pS << sal_uInt8(EMBEL);
            *pS << sal_uInt8(0x0A);
            *pS << sal_uInt8(END); //end embel
            *pS << sal_uInt8(END); //end embel
        }
        else if (nArse == 0x2289)
        {
            *pS << sal_uInt16(0x2287);
            *pS << sal_uInt8(EMBEL);
            *pS << sal_uInt8(0x0A);
            *pS << sal_uInt8(END); //end embel
            *pS << sal_uInt8(END); //end embel
        }
        else if (nArse == 0x2285)
        {
            *pS << sal_uInt16(0x2283);
            *pS << sal_uInt8(EMBEL);
            *pS << sal_uInt8(0x0A);
            *pS << sal_uInt8(END); //end embel
            *pS << sal_uInt8(END); //end embel
        }
        else
            *pS << nArse;
    }
    nPendingAttributes = 0;
#endif
}

void SmOoxml::HandleFractions(SmNode *pNode,int nLevel, const char* type)
{
    m_pSerializer->startElementNS( XML_m, XML_f, FSEND );
    if( type != NULL )
    {
        m_pSerializer->startElementNS( XML_m, XML_fPr, FSEND );
        m_pSerializer->singleElementNS( XML_m, XML_type, FSNS( XML_m, XML_val ), type, FSEND );
        m_pSerializer->endElementNS( XML_m, XML_fPr );
    }
    OSL_ASSERT( pNode->GetNumSubNodes() == 3 );
    m_pSerializer->startElementNS( XML_m, XML_num, FSEND );
    HandleNode( pNode->GetSubNode( 0 ), nLevel + 1 );
    m_pSerializer->endElementNS( XML_m, XML_num );
    m_pSerializer->startElementNS( XML_m, XML_den, FSEND );
    HandleNode( pNode->GetSubNode( 2 ), nLevel + 1 );
    m_pSerializer->endElementNS( XML_m, XML_den );
    m_pSerializer->endElementNS( XML_m, XML_f );
}

void SmOoxml::HandleBinaryOperation(SmNode *pNode,int nLevel)
{
    // update OSL_ASSERT in HandleMath() when adding new items
    switch( pNode->GetToken().eType )
    {
        case TDIVIDEBY:
            return HandleFractions( pNode, nLevel, "lin" );
        default:
            HandleAllSubNodes( pNode, nLevel );
            break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
