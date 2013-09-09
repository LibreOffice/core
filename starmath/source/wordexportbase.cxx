/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include "wordexportbase.hxx"

#include <rtl/ustring.hxx>

SmWordExportBase::SmWordExportBase( const SmNode* pIn )
: m_pTree( pIn )
{
}

SmWordExportBase::~SmWordExportBase()
{
}

void SmWordExportBase::HandleNode( const SmNode* pNode, int nLevel )
{
    SAL_INFO( "starmath.wordbase", "Node: " << nLevel << " " << int( pNode->GetType()) << " " << pNode->GetNumSubNodes());
    switch(pNode->GetType())
    {
        case NATTRIBUT:
            HandleAttribute( static_cast< const SmAttributNode* >( pNode ), nLevel );
            break;
        case NTEXT:
            HandleText(pNode,nLevel);
            break;
        case NVERTICAL_BRACE:
            HandleVerticalBrace( static_cast< const SmVerticalBraceNode* >( pNode ), nLevel );
            break;
        case NBRACE:
            HandleBrace( static_cast< const SmBraceNode* >( pNode ), nLevel );
            break;
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
        case NSPECIAL:
        {
            const SmTextNode* pText= static_cast< const SmTextNode* >( pNode );
            //if the token str and the result text are the same then this
            //is to be seen as text, else assume its a mathchar
            if (pText->GetText() == OUString(pText->GetToken().aText))
                HandleText(pText,nLevel);
            else
                HandleMath(pText,nLevel);
            break;
        }
        case NMATH:
        case NMATHIDENT:
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
#endif
        case NPLACE:
            // explicitly do nothing, MSOffice treats that as a placeholder if item is missing
            break;
        case NBLANK:
            HandleBlank();
            break;
        default:
            HandleAllSubNodes( pNode, nLevel );
            break;
    }
}

//Root Node, PILE equivalent, i.e. vertical stack
void SmWordExportBase::HandleTable( const SmNode* pNode, int nLevel )
{
    //The root of the starmath is a table, if
    //we convert this them each iteration of
    //conversion from starmath to Word will
    //add an extra unnecessary level to the
    //Word output stack which would grow
    //without bound in a multi step conversion
    if( nLevel || pNode->GetNumSubNodes() > 1 )
        HandleVerticalStack( pNode, nLevel );
    else
        HandleAllSubNodes( pNode, nLevel );
}

void SmWordExportBase::HandleAllSubNodes( const SmNode* pNode, int nLevel )
{
    int size = pNode->GetNumSubNodes();
    for( int i = 0;
         i < size;
         ++i )
    {
// TODO remove when all types of nodes are handled properly
        if( pNode->GetSubNode( i ) == NULL )
        {
            SAL_WARN("starmath.wordbase", "Subnode is NULL, parent node not handled properly");
            continue;
        }
        HandleNode( pNode->GetSubNode( i ), nLevel + 1 );
    }
}

void SmWordExportBase::HandleUnaryOperation( const SmUnHorNode* pNode, int nLevel )
{
    // update HandleMath() when adding new items
    SAL_INFO( "starmath.wordbase", "Unary: " << int( pNode->GetToken().eType ));

// Avoid MSVC warning C4065: switch statement contains 'default' but no 'case' labels
//    switch( pNode->GetToken().eType )
//    {
//        default:
            HandleAllSubNodes( pNode, nLevel );
//            break;
//    }
}

void SmWordExportBase::HandleBinaryOperation( const SmBinHorNode* pNode, int nLevel )
{
    SAL_INFO( "starmath.wordbase", "Binary: " << int( pNode->Symbol()->GetToken().eType ));
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

void SmWordExportBase::HandleMath( const SmNode* pNode, int nLevel )
{
    SAL_INFO( "starmath.wordbase", "Math: " << int( pNode->GetToken().eType ));
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

void SmWordExportBase::HandleSubSupScript( const SmSubSupNode* pNode, int nLevel )
{
    // set flags to a bitfield of which sub/sup items exists
    int flags = ( pNode->GetSubSup( CSUB ) != NULL ? ( 1 << CSUB ) : 0 )
            | ( pNode->GetSubSup( CSUP ) != NULL ? ( 1 << CSUP ) : 0 )
            | ( pNode->GetSubSup( RSUB ) != NULL ? ( 1 << RSUB ) : 0 )
            | ( pNode->GetSubSup( RSUP ) != NULL ? ( 1 << RSUP ) : 0 )
            | ( pNode->GetSubSup( LSUB ) != NULL ? ( 1 << LSUB ) : 0 )
            | ( pNode->GetSubSup( LSUP ) != NULL ? ( 1 << LSUP ) : 0 );
    HandleSubSupScriptInternal( pNode, nLevel, flags );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
