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

SmOoxml::SmOoxml( const String &rIn, const SmNode* pIn, OoxmlVersion v )
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

// NOTE: This is still work in progress and unfinished, but it already covers a good
// part of the ooxml math stuff.

void SmOoxml::HandleNode( const SmNode* pNode, int nLevel )
{
//    fprintf(stderr,"XX %d %d %d\n", nLevel, pNode->GetType(), pNode->GetNumSubNodes());
    switch(pNode->GetType())
    {
        case NATTRIBUT:
            HandleAttribute( static_cast< const SmAttributNode* >( pNode ), nLevel );
            break;
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
#endif
        case NOPER:
            HandleOperator( static_cast< const SmOperNode* >( pNode ), nLevel );
            break;
        case NUNHOR:
            HandleUnaryOperation( static_cast< const SmUnHorNode* >( pNode ), nLevel );
            break;
        case NBINHOR:
            HandleBinaryOperation( static_cast< const SmBinHorNode* >( pNode ), nLevel );
            break;
        case NBINVER:
            HandleFractions(pNode,nLevel);
            break;
        case NROOT:
            HandleRoot( static_cast< const SmRootNode* >( pNode ), nLevel );
            break;
#if 0
        case NSPECIAL:
            {
            const SmTextNode* pText= static_cast< const SmTextNode* >( pNode );
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
        case NSUBSUP:
            HandleSubSupScript( static_cast< const SmSubSupNode* >( pNode ), nLevel );
            break;
        case NEXPRESSION:
            HandleAllSubNodes( pNode, nLevel );
            break;
        case NTABLE:
            //Root Node, PILE equivalent, i.e. vertical stack
            HandleTable(pNode,nLevel);
            break;
        case NMATRIX:
            HandleMatrix( static_cast< const SmMatrixNode* >( pNode ), nLevel );
            break;
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
void SmOoxml::HandleTable( const SmNode* pNode, int nLevel )
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

void SmOoxml::HandleAllSubNodes( const SmNode* pNode, int nLevel )
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
void SmOoxml::HandleVerticalStack( const SmNode* pNode, int nLevel, int firstItem )
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

void SmOoxml::HandleText( const SmNode* pNode, int /*nLevel*/)
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
    SmTextNode* pTemp=(SmTextNode* )pNode;
//    fprintf(stderr, "T %s\n", rtl::OUStringToOString( pTemp->GetText(), RTL_TEXTENCODING_UTF8 ).getStr());
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

void SmOoxml::HandleFractions( const SmNode* pNode, int nLevel, const char* type )
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

void SmOoxml::HandleUnaryOperation( const SmUnHorNode* pNode, int nLevel )
{
    // update HandleMath() when adding new items
//    fprintf(stderr,"UNARY %d\n", pNode->Symbol()->GetToken().eType );
    switch( pNode->Symbol()->GetToken().eType )
    {
        default:
            HandleAllSubNodes( pNode, nLevel );
            break;
    }
}

void SmOoxml::HandleBinaryOperation( const SmBinHorNode* pNode, int nLevel )
{
//    fprintf(stderr,"BINARY %d\n", pNode->Symbol()->GetToken().eType );
    // update HandleMath() when adding new items
    switch( pNode->Symbol()->GetToken().eType )
    {
        case TDIVIDEBY:
            return HandleFractions( pNode, nLevel, "lin" );
        default:
            HandleAllSubNodes( pNode, nLevel );
            break;
    }
}

void SmOoxml::HandleAttribute( const SmAttributNode* pNode, int nLevel )
{
    switch( pNode->Attribute()->GetToken().eType )
    {
        case TCHECK: // TODO check these all are really accents
        case TACUTE:
        case TGRAVE:
        case TCIRCLE:
        case TWIDETILDE:
        case TWIDEHAT:
        {
            m_pSerializer->startElementNS( XML_m, XML_acc, FSEND );
            m_pSerializer->startElementNS( XML_m, XML_accPr, FSEND );
            rtl::OString value = rtl::OUStringToOString(
                rtl::OUString( pNode->Attribute()->GetToken().cMathChar ), RTL_TEXTENCODING_UTF8 );
            m_pSerializer->singleElementNS( XML_m, XML_chr, FSNS( XML_m, XML_val ), value.getStr(), FSEND );
            m_pSerializer->endElementNS( XML_m, XML_accPr );
            m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
            HandleNode( pNode->Body(), nLevel + 1 );
            m_pSerializer->endElementNS( XML_m, XML_e );
            m_pSerializer->endElementNS( XML_m, XML_acc );
            break;
        }
        default:
            HandleAllSubNodes( pNode, nLevel );
            break;
    }
}

void SmOoxml::HandleMath( const SmNode* pNode, int nLevel )
{
//    fprintf(stderr,"MATH %d\n", pNode->GetToken().eType);
    switch( pNode->GetToken().eType )
    {
        case TDIVIDEBY:
        case TACUTE:
        // these are handled elsewhere, e.g. when handling BINHOR
            OSL_ASSERT( false );
        default:
            HandleText( pNode, nLevel );
            break;
    }
}

void SmOoxml::HandleRoot( const SmRootNode* pNode, int nLevel )
{
    m_pSerializer->startElementNS( XML_m, XML_rad, FSEND );
    if( const SmNode* argument = pNode->Argument())
    {
        m_pSerializer->startElementNS( XML_m, XML_deg, FSEND );
        HandleNode( argument, nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_deg );
    }
    else
    {
        m_pSerializer->startElementNS( XML_m, XML_radPr, FSEND );
        m_pSerializer->singleElementNS( XML_m, XML_degHide, FSNS( XML_m, XML_val ), "1", FSEND );
        m_pSerializer->endElementNS( XML_m, XML_radPr );
        m_pSerializer->singleElementNS( XML_m, XML_deg, FSEND ); // empty but present
    }
    m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
    HandleNode( pNode->Body(), nLevel + 1 );
    m_pSerializer->endElementNS( XML_m, XML_e );
    m_pSerializer->endElementNS( XML_m, XML_rad );
}

void SmOoxml::HandleOperator( const SmOperNode* pNode, int nLevel )
{
    fprintf( stderr, "OPER %d\n", pNode->GetToken().eType );
    const SmSubSupNode* subsup = pNode->GetSubNode( 0 )->GetType() == NSUBSUP
        ? static_cast< const SmSubSupNode* >( pNode->GetSubNode( 0 )) : NULL;
    const SmNode* operation = subsup != NULL ? subsup->GetBody() : pNode->GetSubNode( 0 );
    OSL_ASSERT( operation->GetType() == NMATH && static_cast< const SmTextNode* >( operation )->GetText().Len() == 1 );
    sal_Unicode chr = Convert( static_cast< const SmTextNode* >( operation )->GetText().GetChar( 0 ));
    m_pSerializer->startElementNS( XML_m, XML_nary, FSEND );
    m_pSerializer->startElementNS( XML_m, XML_naryPr, FSEND );
    rtl::OString chrValue = rtl::OUStringToOString( rtl::OUString( chr ), RTL_TEXTENCODING_UTF8 );
    m_pSerializer->singleElementNS( XML_m, XML_char, FSNS( XML_m, XML_val ), chrValue.getStr(), FSEND );
    if( subsup == NULL || subsup->GetSubSup( CSUB ) == NULL )
        m_pSerializer->singleElementNS( XML_m, XML_subHide, FSNS( XML_m, XML_val ), "1", FSEND );
    if( subsup == NULL || subsup->GetSubSup( CSUP ) == NULL )
        m_pSerializer->singleElementNS( XML_m, XML_supHide, FSNS( XML_m, XML_val ), "1", FSEND );
    m_pSerializer->endElementNS( XML_m, XML_naryPr );
    if( subsup == NULL || subsup->GetSubSup( CSUB ) == NULL )
        m_pSerializer->singleElementNS( XML_m, XML_sub, FSEND );
    else
    {
        m_pSerializer->startElementNS( XML_m, XML_sub, FSEND );
        HandleNode( subsup->GetSubSup( CSUB ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_sub );
    }
    if( subsup == NULL || subsup->GetSubSup( CSUP ) == NULL )
        m_pSerializer->singleElementNS( XML_m, XML_sup, FSEND );
    else
    {
        m_pSerializer->startElementNS( XML_m, XML_sup, FSEND );
        HandleNode( subsup->GetSubSup( CSUP ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_sup );
    }
    m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
    HandleNode( pNode->GetSubNode( 1 ), nLevel + 1 ); // body
    m_pSerializer->endElementNS( XML_m, XML_e );
    m_pSerializer->endElementNS( XML_m, XML_nary );
}

void SmOoxml::HandleSubSupScript( const SmSubSupNode* pNode, int nLevel )
{
    // set flags to a bitfield of which sub/sup items exists
    int flags = ( pNode->GetSubSup( CSUB ) != NULL ? ( 1 << CSUB ) : 0 )
            | ( pNode->GetSubSup( CSUP ) != NULL ? ( 1 << CSUP ) : 0 )
            | ( pNode->GetSubSup( RSUB ) != NULL ? ( 1 << RSUB ) : 0 )
            | ( pNode->GetSubSup( RSUP ) != NULL ? ( 1 << RSUP ) : 0 )
            | ( pNode->GetSubSup( LSUB ) != NULL ? ( 1 << RSUB ) : 0 )
            | ( pNode->GetSubSup( LSUP ) != NULL ? ( 1 << LSUP ) : 0 );
    if( flags == 0 ) // none
        return;
    HandleSubSupScriptInternal( pNode, nLevel, flags );
}

void SmOoxml::HandleSubSupScriptInternal( const SmSubSupNode* pNode, int nLevel, int flags )
{
    if( flags == ( 1 << RSUP | 1 << RSUB ))
    { // m:sSubSup
        m_pSerializer->startElementNS( XML_m, XML_sSubSup, FSEND );
        m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
        HandleNode( pNode->GetBody(), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_e );
        m_pSerializer->startElementNS( XML_m, XML_sub, FSEND );
        HandleNode( pNode->GetSubSup( RSUB ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_sub );
        m_pSerializer->startElementNS( XML_m, XML_sup, FSEND );
        HandleNode( pNode->GetSubSup( RSUP ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_sup );
        m_pSerializer->endElementNS( XML_m, XML_sSubSup );
    }
    else if( flags == 1 << RSUB )
    { // m:sSub
        m_pSerializer->startElementNS( XML_m, XML_sSub, FSEND );
        m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
        HandleNode( pNode->GetBody(), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_e );
        m_pSerializer->startElementNS( XML_m, XML_sub, FSEND );
        HandleNode( pNode->GetSubSup( RSUB ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_sub );
        m_pSerializer->endElementNS( XML_m, XML_sSub );
    }
    else if( flags == 1 << RSUP )
    { // m:sSup
        m_pSerializer->startElementNS( XML_m, XML_sSup, FSEND );
        m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
        HandleNode( pNode->GetBody(), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_e );
        m_pSerializer->startElementNS( XML_m, XML_sup, FSEND );
        HandleNode( pNode->GetSubSup( RSUP ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_sup );
        m_pSerializer->endElementNS( XML_m, XML_sSup );
    }
    else if( flags == ( 1 << LSUP | 1 << LSUB ))
    { // m:sPre
        m_pSerializer->startElementNS( XML_m, XML_sPre, FSEND );
        m_pSerializer->startElementNS( XML_m, XML_sub, FSEND );
        HandleNode( pNode->GetSubSup( LSUB ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_sub );
        m_pSerializer->startElementNS( XML_m, XML_sup, FSEND );
        HandleNode( pNode->GetSubSup( LSUP ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_sup );
        m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
        HandleNode( pNode->GetBody(), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_e );
        m_pSerializer->endElementNS( XML_m, XML_sSubSup );
    }
    else
    {
        OSL_FAIL( "Unhandled sub/sup combination" );
        HandleAllSubNodes( pNode, nLevel );
    }
}

void SmOoxml::HandleMatrix( const SmMatrixNode* pNode, int nLevel )
{
    m_pSerializer->startElementNS( XML_m, XML_m, FSEND );
    for( int row = 0; row < pNode->GetNumRows(); ++row )
    {
        m_pSerializer->startElementNS( XML_m, XML_mr, FSEND );
        for( int col = 0; col < pNode->GetNumCols(); ++col )
        {
            m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
            if( const SmNode* node = pNode->GetSubNode( row * pNode->GetNumCols() + col ))
                HandleNode( node, nLevel + 1 );
            m_pSerializer->endElementNS( XML_m, XML_e );
        }
        m_pSerializer->endElementNS( XML_m, XML_mr );
    }
    m_pSerializer->endElementNS( XML_m, XML_m );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
