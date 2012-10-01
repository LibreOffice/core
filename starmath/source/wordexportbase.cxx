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
            OSL_FAIL( "Subnode is NULL, parent node not handled properly" );
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
