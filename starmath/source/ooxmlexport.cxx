/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ooxmlexport.hxx"
#include <node.hxx>

#include <oox/token/tokens.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <oox/mathml/export.hxx>

using namespace oox;
using namespace oox::core;

SmOoxmlExport::SmOoxmlExport(const SmNode *const pIn, OoxmlVersion const v,
        drawingml::DocumentType const documentType)
: SmWordExportBase( pIn )
, version( v )
, m_DocumentType(documentType)
{
}

void SmOoxmlExport::ConvertFromStarMath( const ::sax_fastparser::FSHelperPtr& serializer, const sal_Int8 nAlign )
{
    if( GetTree() == nullptr )
        return;
    m_pSerializer = serializer;

    //Formula alignment situations:
    //
    //  1)Inline(as before):
    //
    //      <m:oMath>
    //          <m:r>  ... </m:r>
    //      </m:oMath>
    //
    //  2)Aligned:
    //
    //      <m:oMathPara>
    //          <m:oMathParaPr>
    //              <m:jc m:val="left|right|center">
    //          </m:oMathParaPr>
    //          <m:oMath>
    //              <m:r>  ... </m:r>
    //          </m:oMath>
    //      </m:oMathPara>

    if (nAlign != FormulaExportBase::eFormulaAlign::INLINE)
    {
        m_pSerializer->startElementNS(XML_m, XML_oMathPara,
            FSNS(XML_xmlns, XML_m), "http://schemas.openxmlformats.org/officeDocument/2006/math");
        m_pSerializer->startElementNS(XML_m, XML_oMathParaPr);
        if (nAlign == FormulaExportBase::eFormulaAlign::CENTER)
            m_pSerializer->singleElementNS(XML_m, XML_jc, FSNS(XML_m, XML_val), "center");
        if (nAlign == FormulaExportBase::eFormulaAlign::GROUPEDCENTER)
            m_pSerializer->singleElementNS(XML_m, XML_jc, FSNS(XML_m, XML_val), "center");
        if (nAlign == FormulaExportBase::eFormulaAlign::LEFT)
            m_pSerializer->singleElementNS(XML_m, XML_jc, FSNS(XML_m, XML_val), "left");
        if (nAlign == FormulaExportBase::eFormulaAlign::RIGHT)
            m_pSerializer->singleElementNS(XML_m, XML_jc, FSNS(XML_m, XML_val), "right");
        m_pSerializer->endElementNS(XML_m, XML_oMathParaPr);
        m_pSerializer->startElementNS(XML_m, XML_oMath);
        HandleNode(GetTree(), 0);
        m_pSerializer->endElementNS(XML_m, XML_oMath);
        m_pSerializer->endElementNS(XML_m, XML_oMathPara);
    }
    else //else, inline as was before
    {
        m_pSerializer->startElementNS(XML_m, XML_oMath,
            FSNS(XML_xmlns, XML_m), "http://schemas.openxmlformats.org/officeDocument/2006/math");
        HandleNode( GetTree(), 0 );
        m_pSerializer->endElementNS( XML_m, XML_oMath );
    }
}

// NOTE: This is still work in progress and unfinished, but it already covers a good
// part of the ooxml math stuff.

void SmOoxmlExport::HandleVerticalStack( const SmNode* pNode, int nLevel )
{
    m_pSerializer->startElementNS(XML_m, XML_eqArr);
    int size = pNode->GetNumSubNodes();
    for( int i = 0;
         i < size;
         ++i )
    {
        m_pSerializer->startElementNS(XML_m, XML_e);
        HandleNode( pNode->GetSubNode( i ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_e );
    }
    m_pSerializer->endElementNS( XML_m, XML_eqArr );
}

void SmOoxmlExport::HandleText( const SmNode* pNode, int /*nLevel*/)
{
    m_pSerializer->startElementNS(XML_m, XML_r);

    if( pNode->GetToken().eType == TTEXT ) // literal text (in quotes)
    {
        m_pSerializer->startElementNS(XML_m, XML_rPr);
        m_pSerializer->singleElementNS(XML_m, XML_lit);
        m_pSerializer->singleElementNS(XML_m, XML_nor);
        m_pSerializer->endElementNS( XML_m, XML_rPr );
    }
    if (drawingml::DOCUMENT_DOCX == m_DocumentType && ECMA_DIALECT == version)
    { // HACK: MSOffice2007 does not import characters properly unless this font is explicitly given
        m_pSerializer->startElementNS(XML_w, XML_rPr);
        m_pSerializer->singleElementNS( XML_w, XML_rFonts, FSNS( XML_w, XML_ascii ), "Cambria Math",
            FSNS( XML_w, XML_hAnsi ), "Cambria Math" );
        m_pSerializer->endElementNS( XML_w, XML_rPr );
    }
    m_pSerializer->startElementNS(XML_m, XML_t, FSNS(XML_xml, XML_space), "preserve");
    const SmTextNode* pTemp = static_cast<const SmTextNode* >(pNode);
    SAL_INFO( "starmath.ooxml", "Text:" << pTemp->GetText());
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
    m_pSerializer->startElementNS(XML_m, XML_f);
    if( type != nullptr )
    {
        m_pSerializer->startElementNS(XML_m, XML_fPr);
        m_pSerializer->singleElementNS(XML_m, XML_type, FSNS(XML_m, XML_val), type);
        m_pSerializer->endElementNS( XML_m, XML_fPr );
    }
    assert( pNode->GetNumSubNodes() == 3 );
    m_pSerializer->startElementNS(XML_m, XML_num);
    HandleNode( pNode->GetSubNode( 0 ), nLevel + 1 );
    m_pSerializer->endElementNS( XML_m, XML_num );
    m_pSerializer->startElementNS(XML_m, XML_den);
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
        case TWIDEHARPOON:
        case TWIDEVEC:
        case TBAR:
        {
            m_pSerializer->startElementNS(XML_m, XML_acc);
            m_pSerializer->startElementNS(XML_m, XML_accPr);
            OString value =
                OUStringToOString( pNode->Attribute()->GetToken().cMathChar , RTL_TEXTENCODING_UTF8 );
            m_pSerializer->singleElementNS(XML_m, XML_chr, FSNS(XML_m, XML_val), value);
            m_pSerializer->endElementNS( XML_m, XML_accPr );
            m_pSerializer->startElementNS(XML_m, XML_e);
            HandleNode( pNode->Body(), nLevel + 1 );
            m_pSerializer->endElementNS( XML_m, XML_e );
            m_pSerializer->endElementNS( XML_m, XML_acc );
            break;
        }
        case TOVERLINE:
        case TUNDERLINE:
            m_pSerializer->startElementNS(XML_m, XML_bar);
            m_pSerializer->startElementNS(XML_m, XML_barPr);
            m_pSerializer->singleElementNS( XML_m, XML_pos, FSNS( XML_m, XML_val ),
                ( pNode->Attribute()->GetToken().eType == TUNDERLINE ) ? "bot" : "top" );
            m_pSerializer->endElementNS( XML_m, XML_barPr );
            m_pSerializer->startElementNS(XML_m, XML_e);
            HandleNode( pNode->Body(), nLevel + 1 );
            m_pSerializer->endElementNS( XML_m, XML_e );
            m_pSerializer->endElementNS( XML_m, XML_bar );
            break;
        case TOVERSTRIKE:
            m_pSerializer->startElementNS(XML_m, XML_borderBox);
            m_pSerializer->startElementNS(XML_m, XML_borderBoxPr);
            m_pSerializer->singleElementNS(XML_m, XML_hideTop, FSNS(XML_m, XML_val), "1");
            m_pSerializer->singleElementNS(XML_m, XML_hideBot, FSNS(XML_m, XML_val), "1");
            m_pSerializer->singleElementNS(XML_m, XML_hideLeft, FSNS(XML_m, XML_val), "1");
            m_pSerializer->singleElementNS(XML_m, XML_hideRight, FSNS(XML_m, XML_val), "1");
            m_pSerializer->singleElementNS(XML_m, XML_strikeH, FSNS(XML_m, XML_val), "1");
            m_pSerializer->endElementNS( XML_m, XML_borderBoxPr );
            m_pSerializer->startElementNS(XML_m, XML_e);
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
    m_pSerializer->startElementNS(XML_m, XML_rad);
    if( const SmNode* argument = pNode->Argument())
    {
        m_pSerializer->startElementNS(XML_m, XML_deg);
        HandleNode( argument, nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_deg );
    }
    else
    {
        m_pSerializer->startElementNS(XML_m, XML_radPr);
        m_pSerializer->singleElementNS(XML_m, XML_degHide, FSNS(XML_m, XML_val), "1");
        m_pSerializer->endElementNS( XML_m, XML_radPr );
        m_pSerializer->singleElementNS(XML_m, XML_deg); // empty but present
    }
    m_pSerializer->startElementNS(XML_m, XML_e);
    HandleNode( pNode->Body(), nLevel + 1 );
    m_pSerializer->endElementNS( XML_m, XML_e );
    m_pSerializer->endElementNS( XML_m, XML_rad );
}

static OString mathSymbolToString( const SmNode* node )
{
    assert( node->GetType() == SmNodeType::Math || node->GetType() == SmNodeType::MathIdent );
    const SmTextNode* txtnode = static_cast< const SmTextNode* >( node );
    assert( txtnode->GetText().getLength() == 1 );
    sal_Unicode chr = SmTextNode::ConvertSymbolToUnicode( txtnode->GetText()[0] );
    return OUStringToOString( OUStringChar( chr ), RTL_TEXTENCODING_UTF8 );
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
            const SmSubSupNode* subsup = pNode->GetSubNode( 0 )->GetType() == SmNodeType::SubSup
                ? static_cast< const SmSubSupNode* >( pNode->GetSubNode( 0 )) : nullptr;
            const SmNode* operation = subsup != nullptr ? subsup->GetBody() : pNode->GetSubNode( 0 );
            m_pSerializer->startElementNS(XML_m, XML_nary);
            m_pSerializer->startElementNS(XML_m, XML_naryPr);
            m_pSerializer->singleElementNS( XML_m, XML_chr,
                FSNS( XML_m, XML_val ), mathSymbolToString(operation) );
            if( subsup == nullptr || subsup->GetSubSup( CSUB ) == nullptr )
                m_pSerializer->singleElementNS(XML_m, XML_subHide, FSNS(XML_m, XML_val), "1");
            if( subsup == nullptr || subsup->GetSubSup( CSUP ) == nullptr )
                m_pSerializer->singleElementNS(XML_m, XML_supHide, FSNS(XML_m, XML_val), "1");
            m_pSerializer->endElementNS( XML_m, XML_naryPr );
            if( subsup == nullptr || subsup->GetSubSup( CSUB ) == nullptr )
                m_pSerializer->singleElementNS(XML_m, XML_sub);
            else
            {
                m_pSerializer->startElementNS(XML_m, XML_sub);
                HandleNode( subsup->GetSubSup( CSUB ), nLevel + 1 );
                m_pSerializer->endElementNS( XML_m, XML_sub );
            }
            if( subsup == nullptr || subsup->GetSubSup( CSUP ) == nullptr )
                m_pSerializer->singleElementNS(XML_m, XML_sup);
            else
            {
                m_pSerializer->startElementNS(XML_m, XML_sup);
                HandleNode( subsup->GetSubSup( CSUP ), nLevel + 1 );
                m_pSerializer->endElementNS( XML_m, XML_sup );
            }
            m_pSerializer->startElementNS(XML_m, XML_e);
            HandleNode( pNode->GetSubNode( 1 ), nLevel + 1 ); // body
            m_pSerializer->endElementNS( XML_m, XML_e );
            m_pSerializer->endElementNS( XML_m, XML_nary );
            break;
        }
        case TLIM:
            m_pSerializer->startElementNS(XML_m, XML_func);
            m_pSerializer->startElementNS(XML_m, XML_fName);
            m_pSerializer->startElementNS(XML_m, XML_limLow);
            m_pSerializer->startElementNS(XML_m, XML_e);
            HandleNode( pNode->GetSymbol(), nLevel + 1 );
            m_pSerializer->endElementNS( XML_m, XML_e );
            m_pSerializer->startElementNS(XML_m, XML_lim);
            if( const SmSubSupNode* subsup = pNode->GetSubNode( 0 )->GetType() == SmNodeType::SubSup
                ? static_cast< const SmSubSupNode* >( pNode->GetSubNode( 0 )) : nullptr )
            {
                if( subsup->GetSubSup( CSUB ) != nullptr )
                    HandleNode( subsup->GetSubSup( CSUB ), nLevel + 1 );
            }
            m_pSerializer->endElementNS( XML_m, XML_lim );
            m_pSerializer->endElementNS( XML_m, XML_limLow );
            m_pSerializer->endElementNS( XML_m, XML_fName );
            m_pSerializer->startElementNS(XML_m, XML_e);
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
        m_pSerializer->startElementNS(XML_m, XML_sSubSup);
        m_pSerializer->startElementNS(XML_m, XML_e);
        flags &= ~( 1 << RSUP | 1 << RSUB );
        if( flags == 0 )
            HandleNode( pNode->GetBody(), nLevel + 1 );
        else
            HandleSubSupScriptInternal( pNode, nLevel, flags );
        m_pSerializer->endElementNS( XML_m, XML_e );
        m_pSerializer->startElementNS(XML_m, XML_sub);
        HandleNode( pNode->GetSubSup( RSUB ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_sub );
        m_pSerializer->startElementNS(XML_m, XML_sup);
        HandleNode( pNode->GetSubSup( RSUP ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_sup );
        m_pSerializer->endElementNS( XML_m, XML_sSubSup );
    }
    else if(( flags & ( 1 << RSUB )) == 1 << RSUB )
    { // m:sSub
        m_pSerializer->startElementNS(XML_m, XML_sSub);
        m_pSerializer->startElementNS(XML_m, XML_e);
        flags &= ~( 1 << RSUB );
        if( flags == 0 )
            HandleNode( pNode->GetBody(), nLevel + 1 );
        else
            HandleSubSupScriptInternal( pNode, nLevel, flags );
        m_pSerializer->endElementNS( XML_m, XML_e );
        m_pSerializer->startElementNS(XML_m, XML_sub);
        HandleNode( pNode->GetSubSup( RSUB ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_sub );
        m_pSerializer->endElementNS( XML_m, XML_sSub );
    }
    else if(( flags & ( 1 << RSUP )) == 1 << RSUP )
    { // m:sSup
        m_pSerializer->startElementNS(XML_m, XML_sSup);
        m_pSerializer->startElementNS(XML_m, XML_e);
        flags &= ~( 1 << RSUP );
        if( flags == 0 )
            HandleNode( pNode->GetBody(), nLevel + 1 );
        else
            HandleSubSupScriptInternal( pNode, nLevel, flags );
        m_pSerializer->endElementNS( XML_m, XML_e );
        m_pSerializer->startElementNS(XML_m, XML_sup);
        HandleNode( pNode->GetSubSup( RSUP ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_sup );
        m_pSerializer->endElementNS( XML_m, XML_sSup );
    }
    else if(( flags & ( 1 << LSUP | 1 << LSUB )) == ( 1 << LSUP | 1 << LSUB ))
    { // m:sPre
        m_pSerializer->startElementNS(XML_m, XML_sPre);
        m_pSerializer->startElementNS(XML_m, XML_sub);
        HandleNode( pNode->GetSubSup( LSUB ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_sub );
        m_pSerializer->startElementNS(XML_m, XML_sup);
        HandleNode( pNode->GetSubSup( LSUP ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_sup );
        m_pSerializer->startElementNS(XML_m, XML_e);
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
        m_pSerializer->startElementNS(XML_m, XML_limLow);
        m_pSerializer->startElementNS(XML_m, XML_e);
        flags &= ~( 1 << CSUB );
        if( flags == 0 )
            HandleNode( pNode->GetBody(), nLevel + 1 );
        else
            HandleSubSupScriptInternal( pNode, nLevel, flags );
        m_pSerializer->endElementNS( XML_m, XML_e );
        m_pSerializer->startElementNS(XML_m, XML_lim);
        HandleNode( pNode->GetSubSup( CSUB ), nLevel + 1 );
        m_pSerializer->endElementNS( XML_m, XML_lim );
        m_pSerializer->endElementNS( XML_m, XML_limLow );
    }
    else if(( flags & ( 1 << CSUP )) == ( 1 << CSUP ))
    { // m:limUpp looks like a good element for central superscript
        m_pSerializer->startElementNS(XML_m, XML_limUpp);
        m_pSerializer->startElementNS(XML_m, XML_e);
        flags &= ~( 1 << CSUP );
        if( flags == 0 )
            HandleNode( pNode->GetBody(), nLevel + 1 );
        else
            HandleSubSupScriptInternal( pNode, nLevel, flags );
        m_pSerializer->endElementNS( XML_m, XML_e );
        m_pSerializer->startElementNS(XML_m, XML_lim);
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
    m_pSerializer->startElementNS(XML_m, XML_m);
    for (size_t row = 0; row < pNode->GetNumRows(); ++row)
    {
        m_pSerializer->startElementNS(XML_m, XML_mr);
        for (size_t col = 0; col < pNode->GetNumCols(); ++col)
        {
            m_pSerializer->startElementNS(XML_m, XML_e);
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
    m_pSerializer->startElementNS(XML_m, XML_d);
    m_pSerializer->startElementNS(XML_m, XML_dPr);

    //check if the node has an opening brace
    if( TNONE == pNode->OpeningBrace()->GetToken().eType )
        m_pSerializer->singleElementNS(XML_m, XML_begChr, FSNS(XML_m, XML_val), "");
    else
        m_pSerializer->singleElementNS( XML_m, XML_begChr,
            FSNS( XML_m, XML_val ), mathSymbolToString( pNode->OpeningBrace()) );

    std::vector< const SmNode* > subnodes;
    if( pNode->Body()->GetType() == SmNodeType::Bracebody )
    {
        const SmBracebodyNode* body = static_cast< const SmBracebodyNode* >( pNode->Body());
        bool separatorWritten = false; // assume all separators are the same
        for (size_t i = 0; i < body->GetNumSubNodes(); ++i)
        {
            const SmNode* subnode = body->GetSubNode( i );
            if (subnode->GetType() == SmNodeType::Math || subnode->GetType() == SmNodeType::MathIdent)
            { // do not write, but write what separator it is
                const SmMathSymbolNode* math = static_cast< const SmMathSymbolNode* >( subnode );
                if( !separatorWritten )
                {
                    m_pSerializer->singleElementNS( XML_m, XML_sepChr,
                        FSNS( XML_m, XML_val ), mathSymbolToString(math) );
                    separatorWritten = true;
                }
            }
            else
                subnodes.push_back( subnode );
        }
    }
    else
        subnodes.push_back( pNode->Body());

    if( TNONE == pNode->ClosingBrace()->GetToken().eType )
        m_pSerializer->singleElementNS(XML_m, XML_endChr, FSNS(XML_m, XML_val), "");
    else
        m_pSerializer->singleElementNS( XML_m, XML_endChr,
            FSNS( XML_m, XML_val ), mathSymbolToString(pNode->ClosingBrace()) );

    m_pSerializer->endElementNS( XML_m, XML_dPr );
    for(const SmNode* subnode : subnodes)
    {
        m_pSerializer->startElementNS(XML_m, XML_e);
        HandleNode( subnode, nLevel + 1 );
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
            m_pSerializer->startElementNS(XML_m, top ? XML_limUpp : XML_limLow);
            m_pSerializer->startElementNS(XML_m, XML_e);
            m_pSerializer->startElementNS(XML_m, XML_groupChr);
            m_pSerializer->startElementNS(XML_m, XML_groupChrPr);
            m_pSerializer->singleElementNS( XML_m, XML_chr,
                FSNS( XML_m, XML_val ), mathSymbolToString(pNode->Brace()) );
            // TODO not sure if pos and vertJc are correct
            m_pSerializer->singleElementNS( XML_m, XML_pos,
                FSNS( XML_m, XML_val ), top ? "top" : "bot" );
            m_pSerializer->singleElementNS(XML_m, XML_vertJc, FSNS(XML_m, XML_val),
                                           top ? "bot" : "top");
            m_pSerializer->endElementNS( XML_m, XML_groupChrPr );
            m_pSerializer->startElementNS(XML_m, XML_e);
            HandleNode( pNode->Body(), nLevel + 1 );
            m_pSerializer->endElementNS( XML_m, XML_e );
            m_pSerializer->endElementNS( XML_m, XML_groupChr );
            m_pSerializer->endElementNS( XML_m, XML_e );
            m_pSerializer->startElementNS(XML_m, XML_lim);
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
    m_pSerializer->startElementNS(XML_m, XML_r);
    m_pSerializer->startElementNS(XML_m, XML_t, FSNS(XML_xml, XML_space), "preserve");
    m_pSerializer->write( " " );
    m_pSerializer->endElementNS( XML_m, XML_t );
    m_pSerializer->endElementNS( XML_m, XML_r );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
