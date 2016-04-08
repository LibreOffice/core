/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include "ooxmlexport.hxx"

#include <oox/token/tokens.hxx>
#include <rtl/ustring.hxx>

using namespace oox;
using namespace oox::core;

SmOoxmlExport::SmOoxmlExport(const SmNode *const pIn, OoxmlVersion const v,
        drawingml::DocumentType const documentType)
: SmWordExportBase( pIn )
, version( v )
, m_DocumentType(documentType)
{
}

bool SmOoxmlExport::ConvertFromStarMath( const ::sax_fastparser::FSHelperPtr& serializer )
{
    if( m_pTree == nullptr )
        return false;
    m_pSerializer = serializer;
    m_pSerializer->startElementNS( XML_m, XML_oMath,
        FSNS( XML_xmlns, XML_m ), "http://schemas.openxmlformats.org/officeDocument/2006/math", FSEND );
    HandleNode( m_pTree, 0 );
    m_pSerializer->endElementNS( XML_m, XML_oMath );
    return true;
}

// NOTE: This is still work in progress and unfinished, but it already covers a good
// part of the ooxml math stuff.

void SmOoxmlExport::HandleVerticalStack( const SmNode* pNode, int nLevel )
{
    m_pSerializer->startElementNS( XML_m, XML_eqArr, FSEND );
    int size = pNode->GetNumSubNodes();
    for( int i = 0;
         i < size;
         ++i )
    {
        m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
        HandleNode( pNode->GetSubNode( i ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_e );
    }
    m_pSerializer->endElementNS( XML_m, XML_eqArr );
}

void SmOoxmlExport::HandleText( const SmNode* pNode, int /*nLevel*/)
{
    m_pSerializer->startElementNS( XML_m, XML_r, FSEND );

    if( pNode->GetToken().eType == TTEXT ) // literal text (in quotes)
    {
        m_pSerializer->startElementNS( XML_m, XML_rPr, FSEND );
        m_pSerializer->singleElementNS( XML_m, XML_lit, FSEND );
        m_pSerializer->singleElementNS( XML_m, XML_nor, FSEND );
        m_pSerializer->endElementNS( XML_m, XML_rPr );
    }
    if (drawingml::DOCUMENT_DOCX == m_DocumentType && ECMA_DIALECT == version)
    { // HACK: MSOffice2007 does not import characters properly unless this font is explicitly given
        m_pSerializer->startElementNS( XML_w, XML_rPr, FSEND );
        m_pSerializer->singleElementNS( XML_w, XML_rFonts, FSNS( XML_w, XML_ascii ), "Cambria Math",
            FSNS( XML_w, XML_hAnsi ), "Cambria Math", FSEND );
        m_pSerializer->endElementNS( XML_w, XML_rPr );
    }
    m_pSerializer->startElementNS( XML_m, XML_t, FSNS( XML_xml, XML_space ), "preserve", FSEND );
    const SmTextNode* pTemp = static_cast<const SmTextNode* >(pNode);
    SAL_INFO( "starmath.ooxml", "Text:" << OUStringToOString( pTemp->GetText(), RTL_TEXTENCODING_UTF8 ).getStr());
    OUStringBuffer buf(pTemp->GetText());
    for(sal_Int32 i=0;i<pTemp->GetText().getLength();i++)
    {
#if 0
        if ((nPendingAttributes) &&
            (i == ((pTemp->GetText().getLength()+1)/2)-1))
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
        buf[i] = SmTextNode::ConvertSymbolToUnicode(buf[i]);
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
            (i == ((pTemp->GetText().getLength()+1)/2)-1))
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
    m_pSerializer->writeEscaped(buf.makeStringAndClear());
    m_pSerializer->endElementNS( XML_m, XML_t );
    m_pSerializer->endElementNS( XML_m, XML_r );
}

void SmOoxmlExport::HandleFractions( const SmNode* pNode, int nLevel, const char* type )
{
    m_pSerializer->startElementNS( XML_m, XML_f, FSEND );
    if( type != nullptr )
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

void SmOoxmlExport::HandleAttribute( const SmAttributNode* pNode, int nLevel )
{
    switch( pNode->Attribute()->GetToken().eType )
    {
        case TCHECK:
        case TACUTE:
        case TGRAVE:
        case TBREVE:
        case TCIRCLE:
        case TVEC:
        case TTILDE:
        case THAT:
        case TDOT:
        case TDDOT:
        case TDDDOT:
        case TWIDETILDE:
        case TWIDEHAT:
        case TWIDEVEC:
        case TBAR:
        {
            m_pSerializer->startElementNS( XML_m, XML_acc, FSEND );
            m_pSerializer->startElementNS( XML_m, XML_accPr, FSEND );
            OString value = OUStringToOString(
                OUString( pNode->Attribute()->GetToken().cMathChar ), RTL_TEXTENCODING_UTF8 );
            m_pSerializer->singleElementNS( XML_m, XML_chr, FSNS( XML_m, XML_val ), value.getStr(), FSEND );
            m_pSerializer->endElementNS( XML_m, XML_accPr );
            m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
            HandleNode( pNode->Body(), nLevel + 1 );
            m_pSerializer->endElementNS( XML_m, XML_e );
            m_pSerializer->endElementNS( XML_m, XML_acc );
            break;
        }
        case TOVERLINE:
        case TUNDERLINE:
            m_pSerializer->startElementNS( XML_m, XML_bar, FSEND );
            m_pSerializer->startElementNS( XML_m, XML_barPr, FSEND );
            m_pSerializer->singleElementNS( XML_m, XML_pos, FSNS( XML_m, XML_val ),
                ( pNode->Attribute()->GetToken().eType == TUNDERLINE ) ? "bot" : "top", FSEND );
            m_pSerializer->endElementNS( XML_m, XML_barPr );
            m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
            HandleNode( pNode->Body(), nLevel + 1 );
            m_pSerializer->endElementNS( XML_m, XML_e );
            m_pSerializer->endElementNS( XML_m, XML_bar );
            break;
        case TOVERSTRIKE:
            m_pSerializer->startElementNS( XML_m, XML_borderBox, FSEND );
            m_pSerializer->startElementNS( XML_m, XML_borderBoxPr, FSEND );
            m_pSerializer->singleElementNS( XML_m, XML_hideTop, FSNS( XML_m, XML_val ), "1", FSEND );
            m_pSerializer->singleElementNS( XML_m, XML_hideBot, FSNS( XML_m, XML_val ), "1", FSEND );
            m_pSerializer->singleElementNS( XML_m, XML_hideLeft, FSNS( XML_m, XML_val ), "1", FSEND );
            m_pSerializer->singleElementNS( XML_m, XML_hideRight, FSNS( XML_m, XML_val ), "1", FSEND );
            m_pSerializer->singleElementNS( XML_m, XML_strikeH, FSNS( XML_m, XML_val ), "1", FSEND );
            m_pSerializer->endElementNS( XML_m, XML_borderBoxPr );
            m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
            HandleNode( pNode->Body(), nLevel + 1 );
            m_pSerializer->endElementNS( XML_m, XML_e );
            m_pSerializer->endElementNS( XML_m, XML_borderBox );
            break;
        default:
            HandleAllSubNodes( pNode, nLevel );
            break;
    }
}

void SmOoxmlExport::HandleRoot( const SmRootNode* pNode, int nLevel )
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

static OString mathSymbolToString( const SmNode* node )
{
    assert( node->GetType() == NMATH || node->GetType() == NMATHIDENT );
    const SmTextNode* txtnode = static_cast< const SmTextNode* >( node );
    assert( txtnode->GetText().getLength() == 1 );
    sal_Unicode chr = SmTextNode::ConvertSymbolToUnicode( txtnode->GetText()[0] );
    return OUStringToOString( OUString( chr ), RTL_TEXTENCODING_UTF8 );
}

void SmOoxmlExport::HandleOperator( const SmOperNode* pNode, int nLevel )
{
    SAL_INFO( "starmath.ooxml", "Operator: " << int( pNode->GetToken().eType ));
    switch( pNode->GetToken().eType )
    {
        case TINT:
        case TINTD:
        case TIINT:
        case TIIINT:
        case TLINT:
        case TLLINT:
        case TLLLINT:
        case TPROD:
        case TCOPROD:
        case TSUM:
        {
            const SmSubSupNode* subsup = pNode->GetSubNode( 0 )->GetType() == NSUBSUP
                ? static_cast< const SmSubSupNode* >( pNode->GetSubNode( 0 )) : nullptr;
            const SmNode* operation = subsup != nullptr ? subsup->GetBody() : pNode->GetSubNode( 0 );
            m_pSerializer->startElementNS( XML_m, XML_nary, FSEND );
            m_pSerializer->startElementNS( XML_m, XML_naryPr, FSEND );
            m_pSerializer->singleElementNS( XML_m, XML_chr,
                FSNS( XML_m, XML_val ), mathSymbolToString( operation ).getStr(), FSEND );
            if( subsup == nullptr || subsup->GetSubSup( CSUB ) == nullptr )
                m_pSerializer->singleElementNS( XML_m, XML_subHide, FSNS( XML_m, XML_val ), "1", FSEND );
            if( subsup == nullptr || subsup->GetSubSup( CSUP ) == nullptr )
                m_pSerializer->singleElementNS( XML_m, XML_supHide, FSNS( XML_m, XML_val ), "1", FSEND );
            m_pSerializer->endElementNS( XML_m, XML_naryPr );
            if( subsup == nullptr || subsup->GetSubSup( CSUB ) == nullptr )
                m_pSerializer->singleElementNS( XML_m, XML_sub, FSEND );
            else
            {
                m_pSerializer->startElementNS( XML_m, XML_sub, FSEND );
                HandleNode( subsup->GetSubSup( CSUB ), nLevel + 1 );
                m_pSerializer->endElementNS( XML_m, XML_sub );
            }
            if( subsup == nullptr || subsup->GetSubSup( CSUP ) == nullptr )
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
            break;
        }
        case TLIM:
            m_pSerializer->startElementNS( XML_m, XML_func, FSEND );
            m_pSerializer->startElementNS( XML_m, XML_fName, FSEND );
            m_pSerializer->startElementNS( XML_m, XML_limLow, FSEND );
            m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
            HandleNode( pNode->GetSymbol(), nLevel + 1 );
            m_pSerializer->endElementNS( XML_m, XML_e );
            m_pSerializer->startElementNS( XML_m, XML_lim, FSEND );
            if( const SmSubSupNode* subsup = pNode->GetSubNode( 0 )->GetType() == NSUBSUP
                ? static_cast< const SmSubSupNode* >( pNode->GetSubNode( 0 )) : nullptr )
            {
                if( subsup->GetSubSup( CSUB ) != nullptr )
                    HandleNode( subsup->GetSubSup( CSUB ), nLevel + 1 );
            }
            m_pSerializer->endElementNS( XML_m, XML_lim );
            m_pSerializer->endElementNS( XML_m, XML_limLow );
            m_pSerializer->endElementNS( XML_m, XML_fName );
            m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
            HandleNode( pNode->GetSubNode( 1 ), nLevel + 1 ); // body
            m_pSerializer->endElementNS( XML_m, XML_e );
            m_pSerializer->endElementNS( XML_m, XML_func );
            break;
        default:
            SAL_WARN("starmath.ooxml", "Unhandled operation");
            HandleAllSubNodes( pNode, nLevel );
            break;
    }
}

void SmOoxmlExport::HandleSubSupScriptInternal( const SmSubSupNode* pNode, int nLevel, int flags )
{
// docx supports only a certain combination of sub/super scripts, but LO can have any,
// so try to merge it using several tags if necessary
    if( flags == 0 ) // none
        return;
    if(( flags & ( 1 << RSUP | 1 << RSUB )) == ( 1 << RSUP | 1 << RSUB ))
    { // m:sSubSup
        m_pSerializer->startElementNS( XML_m, XML_sSubSup, FSEND );
        m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
        flags &= ~( 1 << RSUP | 1 << RSUB );
        if( flags == 0 )
            HandleNode( pNode->GetBody(), nLevel + 1 );
        else
            HandleSubSupScriptInternal( pNode, nLevel, flags );
        m_pSerializer->endElementNS( XML_m, XML_e );
        m_pSerializer->startElementNS( XML_m, XML_sub, FSEND );
        HandleNode( pNode->GetSubSup( RSUB ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_sub );
        m_pSerializer->startElementNS( XML_m, XML_sup, FSEND );
        HandleNode( pNode->GetSubSup( RSUP ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_sup );
        m_pSerializer->endElementNS( XML_m, XML_sSubSup );
    }
    else if(( flags & ( 1 << RSUB )) == 1 << RSUB )
    { // m:sSub
        m_pSerializer->startElementNS( XML_m, XML_sSub, FSEND );
        m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
        flags &= ~( 1 << RSUB );
        if( flags == 0 )
            HandleNode( pNode->GetBody(), nLevel + 1 );
        else
            HandleSubSupScriptInternal( pNode, nLevel, flags );
        m_pSerializer->endElementNS( XML_m, XML_e );
        m_pSerializer->startElementNS( XML_m, XML_sub, FSEND );
        HandleNode( pNode->GetSubSup( RSUB ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_sub );
        m_pSerializer->endElementNS( XML_m, XML_sSub );
    }
    else if(( flags & ( 1 << RSUP )) == 1 << RSUP )
    { // m:sSup
        m_pSerializer->startElementNS( XML_m, XML_sSup, FSEND );
        m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
        flags &= ~( 1 << RSUP );
        if( flags == 0 )
            HandleNode( pNode->GetBody(), nLevel + 1 );
        else
            HandleSubSupScriptInternal( pNode, nLevel, flags );
        m_pSerializer->endElementNS( XML_m, XML_e );
        m_pSerializer->startElementNS( XML_m, XML_sup, FSEND );
        HandleNode( pNode->GetSubSup( RSUP ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_sup );
        m_pSerializer->endElementNS( XML_m, XML_sSup );
    }
    else if(( flags & ( 1 << LSUP | 1 << LSUB )) == ( 1 << LSUP | 1 << LSUB ))
    { // m:sPre
        m_pSerializer->startElementNS( XML_m, XML_sPre, FSEND );
        m_pSerializer->startElementNS( XML_m, XML_sub, FSEND );
        HandleNode( pNode->GetSubSup( LSUB ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_sub );
        m_pSerializer->startElementNS( XML_m, XML_sup, FSEND );
        HandleNode( pNode->GetSubSup( LSUP ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_sup );
        m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
        flags &= ~( 1 << LSUP | 1 << LSUB );
        if( flags == 0 )
            HandleNode( pNode->GetBody(), nLevel + 1 );
        else
            HandleSubSupScriptInternal( pNode, nLevel, flags );
        m_pSerializer->endElementNS( XML_m, XML_e );
        m_pSerializer->endElementNS( XML_m, XML_sPre );
    }
    else if(( flags & ( 1 << CSUB )) == ( 1 << CSUB ))
    { // m:limLow looks like a good element for central superscript
        m_pSerializer->startElementNS( XML_m, XML_limLow, FSEND );
        m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
        flags &= ~( 1 << CSUB );
        if( flags == 0 )
            HandleNode( pNode->GetBody(), nLevel + 1 );
        else
            HandleSubSupScriptInternal( pNode, nLevel, flags );
        m_pSerializer->endElementNS( XML_m, XML_e );
        m_pSerializer->startElementNS( XML_m, XML_lim, FSEND );
        HandleNode( pNode->GetSubSup( CSUB ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_lim );
        m_pSerializer->endElementNS( XML_m, XML_limLow );
    }
    else if(( flags & ( 1 << CSUP )) == ( 1 << CSUP ))
    { // m:limUpp looks like a good element for central superscript
        m_pSerializer->startElementNS( XML_m, XML_limUpp, FSEND );
        m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
        flags &= ~( 1 << CSUP );
        if( flags == 0 )
            HandleNode( pNode->GetBody(), nLevel + 1 );
        else
            HandleSubSupScriptInternal( pNode, nLevel, flags );
        m_pSerializer->endElementNS( XML_m, XML_e );
        m_pSerializer->startElementNS( XML_m, XML_lim, FSEND );
        HandleNode( pNode->GetSubSup( CSUP ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_lim );
        m_pSerializer->endElementNS( XML_m, XML_limUpp );
    }
    else
    {
        SAL_WARN("starmath.ooxml", "Unhandled sub/sup combination");
        // TODO do not do anything, this should be probably an assert()
        // HandleAllSubNodes( pNode, nLevel );
    }
}

void SmOoxmlExport::HandleMatrix( const SmMatrixNode* pNode, int nLevel )
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

void SmOoxmlExport::HandleBrace( const SmBraceNode* pNode, int nLevel )
{
    m_pSerializer->startElementNS( XML_m, XML_d, FSEND );
    m_pSerializer->startElementNS( XML_m, XML_dPr, FSEND );

    //check if the node has an opening brace
    if( TNONE == pNode->GetSubNode(0)->GetToken().eType )
        m_pSerializer->singleElementNS( XML_m, XML_begChr,
            FSNS( XML_m, XML_val ), "", FSEND );
    else
        m_pSerializer->singleElementNS( XML_m, XML_begChr,
            FSNS( XML_m, XML_val ), mathSymbolToString( pNode->OpeningBrace()).getStr(), FSEND );

    std::vector< const SmNode* > subnodes;
    if( pNode->Body()->GetType() == NBRACEBODY )
    {
        const SmBracebodyNode* body = static_cast< const SmBracebodyNode* >( pNode->Body());
        bool separatorWritten = false; // assume all separators are the same
        for( int i = 0; i < body->GetNumSubNodes(); ++i )
        {
            const SmNode* subnode = body->GetSubNode( i );
            if (subnode->GetType() == NMATH || subnode->GetType() == NMATHIDENT)
            { // do not write, but write what separator it is
                const SmMathSymbolNode* math = static_cast< const SmMathSymbolNode* >( subnode );
                if( !separatorWritten )
                {
                    m_pSerializer->singleElementNS( XML_m, XML_sepChr,
                        FSNS( XML_m, XML_val ), mathSymbolToString( math ).getStr(), FSEND );
                    separatorWritten = true;
                }
            }
            else
                subnodes.push_back( subnode );
        }
    }
    else
        subnodes.push_back( pNode->Body());

    if( TNONE == pNode->GetSubNode(2)->GetToken().eType )
        m_pSerializer->singleElementNS( XML_m, XML_endChr,
            FSNS( XML_m, XML_val ), "", FSEND );
    else
        m_pSerializer->singleElementNS( XML_m, XML_endChr,
            FSNS( XML_m, XML_val ), mathSymbolToString( pNode->ClosingBrace()).getStr(), FSEND );

    m_pSerializer->endElementNS( XML_m, XML_dPr );
    for( size_t i = 0; i < subnodes.size(); ++i )
    {
        m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
        HandleNode( subnodes[ i ], nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_e );
    }
    m_pSerializer->endElementNS( XML_m, XML_d );
}

void SmOoxmlExport::HandleVerticalBrace( const SmVerticalBraceNode* pNode, int nLevel )
{
    SAL_INFO( "starmath.ooxml", "Vertical: " << int( pNode->GetToken().eType ));
    switch( pNode->GetToken().eType )
    {
        case TOVERBRACE:
        case TUNDERBRACE:
        {
            bool top = ( pNode->GetToken().eType == TOVERBRACE );
            m_pSerializer->startElementNS( XML_m, top ? XML_limUpp : XML_limLow, FSEND );
            m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
            m_pSerializer->startElementNS( XML_m, XML_groupChr, FSEND );
            m_pSerializer->startElementNS( XML_m, XML_groupChrPr, FSEND );
            m_pSerializer->singleElementNS( XML_m, XML_chr,
                FSNS( XML_m, XML_val ), mathSymbolToString( pNode->Brace()).getStr(), FSEND );
            // TODO not sure if pos and vertJc are correct
            m_pSerializer->singleElementNS( XML_m, XML_pos,
                FSNS( XML_m, XML_val ), top ? "top" : "bot", FSEND );
            m_pSerializer->singleElementNS( XML_m, XML_vertJc, FSNS( XML_m, XML_val ), top ? "bot" : "top", FSEND );
            m_pSerializer->endElementNS( XML_m, XML_groupChrPr );
            m_pSerializer->startElementNS( XML_m, XML_e, FSEND );
            HandleNode( pNode->Body(), nLevel + 1 );
            m_pSerializer->endElementNS( XML_m, XML_e );
            m_pSerializer->endElementNS( XML_m, XML_groupChr );
            m_pSerializer->endElementNS( XML_m, XML_e );
            m_pSerializer->startElementNS( XML_m, XML_lim, FSEND );
            HandleNode( pNode->Script(), nLevel + 1 );
            m_pSerializer->endElementNS( XML_m, XML_lim );
            m_pSerializer->endElementNS( XML_m, top ? XML_limUpp : XML_limLow );
            break;
        }
        default:
            SAL_WARN("starmath.ooxml", "Unhandled vertical brace");
            HandleAllSubNodes( pNode, nLevel );
            break;
    }
}

void SmOoxmlExport::HandleBlank()
{
    m_pSerializer->startElementNS( XML_m, XML_r, FSEND );
    m_pSerializer->startElementNS( XML_m, XML_t, FSNS( XML_xml, XML_space ), "preserve", FSEND );
    m_pSerializer->write( " " );
    m_pSerializer->endElementNS( XML_m, XML_t );
    m_pSerializer->endElementNS( XML_m, XML_r );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
