/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <editeng/svxenum.hxx>
#include <numrule.hxx>
#include <SwNodeNum.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <stdio.h>
// #i83479#
#include <IDocumentListItems.hxx>
#include <doc.hxx>

SwNodeNum::SwNodeNum( SwTxtNode* pTxtNode )
    : SwNumberTreeNode(),
      mpTxtNode( pTxtNode ),
      mpNumRule( 0 )
{
}

SwNodeNum::SwNodeNum( SwNumRule* pNumRule )
    : SwNumberTreeNode(),
      mpTxtNode( 0 ),
      mpNumRule( pNumRule )
{
}

SwNodeNum::~SwNodeNum()
{
}

SwTxtNode * SwNodeNum::GetTxtNode() const
{
    return mpTxtNode;
}

SwNumRule * SwNodeNum::GetNumRule() const
{
    return mpNumRule;
}

void SwNodeNum::ChangeNumRule( SwNumRule& rNumRule )
{
    OSL_ENSURE( GetNumRule() && GetTxtNode(),
            "<SwNodeNum::ChangeNumRule(..)> - missing list style and/or text node. Serious defect -> please informm OD." );
    if ( GetNumRule() && GetTxtNode() )
    {
        GetNumRule()->RemoveTxtNode( *(GetTxtNode()) );
    }

    mpNumRule = &rNumRule;

    if ( GetNumRule() && GetTxtNode() )
    {
        GetNumRule()->AddTxtNode( *(GetTxtNode()) );
    }
}

SwPosition SwNodeNum::GetPosition() const
{
    OSL_ENSURE( GetTxtNode(),
            "<SwNodeNum::GetPosition()> - no text node set at <SwNodeNum> instance" );
    return SwPosition(*mpTxtNode);
}

SwNumberTreeNode * SwNodeNum::Create() const
{
    SwNodeNum * pResult = new SwNodeNum( GetNumRule() );

    return pResult;
}

void SwNodeNum::PreAdd()
{
    OSL_ENSURE( GetTxtNode(),
            "<SwNodeNum::PreAdd()> - no text node set at <SwNodeNum> instance" );
    if ( !GetNumRule() && GetTxtNode() )
    {
        mpNumRule = GetTxtNode()->GetNumRule();
    }
    OSL_ENSURE( GetNumRule(),
            "<SwNodeNum::PreAdd()> - no list style set at <SwNodeNum> instance" );
    if ( GetNumRule() && GetTxtNode() )
    {
        GetNumRule()->AddTxtNode( *(GetTxtNode()) );
    }


    {
        if ( GetTxtNode() &&
             GetTxtNode()->GetNodes().IsDocNodes() )
        {
            GetTxtNode()->getIDocumentListItems().addListItem( *this );
        }
    }
}

void SwNodeNum::PostRemove()
{
    OSL_ENSURE( GetTxtNode(),
            "<SwNodeNum::PostRemove()> - no text node set at <SwNodeNum> instance" );
    OSL_ENSURE( GetNumRule(),
            "<SwNodeNum::PostRemove()> - no list style set at <SwNodeNum> instance" );

    if ( GetTxtNode() )
    {
        GetTxtNode()->getIDocumentListItems().removeListItem( *this );
    }

    if ( GetNumRule() )
    {
        if ( GetTxtNode() )
        {
            GetNumRule()->RemoveTxtNode( *(GetTxtNode()) );
        }
        mpNumRule = 0;
    }
}

bool SwNodeNum::IsNotifiable() const
{
    bool aResult = true;

    if ( GetTxtNode() )
        aResult = GetTxtNode()->IsNotifiable();

    return aResult;
}

bool SwNodeNum::IsNotificationEnabled() const
{
    bool aResult = true;

    if ( GetTxtNode() )
        aResult = GetTxtNode()->IsNotificationEnabled();

    return aResult;
}

bool SwNodeNum::IsContinuous() const
{
    bool aResult = false;

    // #i64311#
    if ( GetNumRule() )
    {
        aResult = mpNumRule->IsContinusNum();
    }
    else if ( GetParent() )
    {
        aResult = GetParent()->IsContinuous();
    }
    else
    {
        OSL_FAIL( "<SwNodeNum::IsContinuous()> - OD debug" );
    }

    return aResult;
}

bool SwNodeNum::IsCounted() const
{
    bool aResult = false;

    if ( GetTxtNode() )
    {
        // #i59559#
        // <SwTxtNode::IsCounted()> determines, if a text node is counted for numbering
        aResult = GetTxtNode()->IsCountedInList();
    }
    else
        aResult = SwNumberTreeNode::IsCounted();

    return aResult;
}

// #i64010#
bool SwNodeNum::HasCountedChildren() const
{
    bool bResult = false;

    tSwNumberTreeChildren::const_iterator aIt;

    for (aIt = mChildren.begin(); aIt != mChildren.end(); ++aIt)
    {
        SwNodeNum* pChild( dynamic_cast<SwNodeNum*>(*aIt) );
        OSL_ENSURE( pChild,
                "<SwNodeNum::HasCountedChildren()> - unexcepted type of child -> please inform OD" );
        if ( pChild &&
             ( pChild->IsCountedForNumbering() ||
               pChild->HasCountedChildren() ) )
        {
            bResult = true;

            break;
        }
    }

    return bResult;
}
// #i64010#
bool SwNodeNum::IsCountedForNumbering() const
{
    return IsCounted() &&
           ( IsPhantom() ||                 // phantoms
             !GetTxtNode() ||               // root node
             GetTxtNode()->HasNumber() ||   // text node
             GetTxtNode()->HasBullet() );   // text node
}


void SwNodeNum::NotifyNode()
{
    ValidateMe();

    if (mpTxtNode)
    {
        mpTxtNode->NumRuleChgd();
    }
}

bool SwNodeNum::LessThan(const SwNumberTreeNode & rNode) const
{
    bool bResult = false;
    const SwNodeNum & rTmpNode = static_cast<const SwNodeNum &>(rNode);

    if (mpTxtNode == NULL && rTmpNode.mpTxtNode != NULL)
        bResult = true;
    else if (mpTxtNode != NULL && rTmpNode.mpTxtNode != NULL)
    {
        // #i83479# - refactoring
        // simplify comparison by comparing the indexes of the text nodes
        bResult = ( mpTxtNode->GetIndex() < rTmpNode.mpTxtNode->GetIndex() ) ? true : false;
    }

    return bResult;
}

bool SwNodeNum::IsRestart() const
{
    bool bIsRestart = false;

    if ( GetTxtNode() )
    {
        bIsRestart = GetTxtNode()->IsListRestart();
    }

    return bIsRestart;
}

bool SwNodeNum::IsCountPhantoms() const
{
    bool bResult = true;

    // #i64311#
    // phantoms aren't counted in consecutive numbering rules
    if ( mpNumRule )
        bResult = !mpNumRule->IsContinusNum() &&
                  mpNumRule->IsCountPhantoms();
    else
    {
        OSL_FAIL( "<SwNodeNum::IsCountPhantoms(): missing numbering rule - please inform OD" );
    }

    return bResult;
}

SwNumberTree::tSwNumTreeNumber SwNodeNum::GetStartValue() const
{
    SwNumberTree::tSwNumTreeNumber aResult = 1;

    if ( IsRestart() && GetTxtNode() )
    {
        aResult = GetTxtNode()->GetActualListStartValue();
    }
    else
    {
        SwNumRule * pRule = GetNumRule();

        if (pRule)
        {
            int nLevel = GetParent() ? GetLevelInListTree() : 0;

            if (nLevel >= 0 && nLevel < MAXLEVEL)
            {
                const SwNumFmt * pFmt = pRule->GetNumFmt( static_cast<sal_uInt16>(nLevel));

                if (pFmt)
                    aResult = pFmt->GetStart();
            }
        }
    }

    return aResult;
}

void SwNodeNum::HandleNumberTreeRootNodeDelete( SwNodeNum& rNodeNum )
{
    SwNodeNum* pRootNode = rNodeNum.GetParent()
                           ? dynamic_cast<SwNodeNum*>(rNodeNum.GetRoot())
                           : &rNodeNum;
    if ( !pRootNode )
    {
        // no root node -> nothing do.
        return;
    }

    // unregister all number tree node entries, which correspond to a text node,
    // about the deletion of the number tree root node.
    _UnregisterMeAndChildrenDueToRootDelete( *pRootNode );
}

void SwNodeNum::_UnregisterMeAndChildrenDueToRootDelete( SwNodeNum& rNodeNum )
{
    const bool bIsPhantom( rNodeNum.IsPhantom() );
    tSwNumberTreeChildren::size_type nAllowedChildCount( 0 );
    bool bDone( false );
    while ( !bDone &&
            rNodeNum.GetChildCount() > nAllowedChildCount )
    {
        SwNodeNum* pChildNode( dynamic_cast<SwNodeNum*>((*rNodeNum.mChildren.begin())) );
        if ( !pChildNode )
        {
            OSL_FAIL( "<SwNodeNum::_UnregisterMeAndChildrenDueToRootDelete(..)> - unknown number tree node child" );
            ++nAllowedChildCount;
            continue;
        }

        // Unregistering the last child of a phantom will destroy the phantom.
        // Thus <rNodeNum> will be destroyed and access on <rNodeNum> has to
        // be suppressed.
        if ( bIsPhantom && rNodeNum.GetChildCount() == 1 )
        {
            bDone = true;
        }

        _UnregisterMeAndChildrenDueToRootDelete( *pChildNode );
    }

    if ( !bIsPhantom )
    {
        SwTxtNode* pTxtNode( rNodeNum.GetTxtNode() );
        if ( pTxtNode )
        {
            pTxtNode->RemoveFromList();
            // --> clear all list attributes and the list style
            std::set<sal_uInt16> aResetAttrsArray;
            aResetAttrsArray.insert( aResetAttrsArray.end(), RES_PARATR_LIST_ID );
            aResetAttrsArray.insert( aResetAttrsArray.end(), RES_PARATR_LIST_LEVEL );
            aResetAttrsArray.insert( aResetAttrsArray.end(), RES_PARATR_LIST_ISRESTART );
            aResetAttrsArray.insert( aResetAttrsArray.end(), RES_PARATR_LIST_RESTARTVALUE );
            aResetAttrsArray.insert( aResetAttrsArray.end(), RES_PARATR_LIST_ISCOUNTED );
            aResetAttrsArray.insert( aResetAttrsArray.end(), RES_PARATR_NUMRULE );
            SwPaM aPam( *pTxtNode );
            pTxtNode->GetDoc()->ResetAttrs( aPam, false,
                                            aResetAttrsArray,
                                            false );
        }
    }
}

// #i81002#
const SwNodeNum* SwNodeNum::GetPrecedingNodeNumOf( const SwTxtNode& rTxtNode ) const
{
    const SwNodeNum* pPrecedingNodeNum( 0 );

    // #i83479#
    SwNodeNum aNodeNumForTxtNode( const_cast<SwTxtNode*>(&rTxtNode) );

    pPrecedingNodeNum = dynamic_cast<const SwNodeNum*>(
                            GetRoot()
                            ? GetRoot()->GetPrecedingNodeOf( aNodeNumForTxtNode )
                            : GetPrecedingNodeOf( aNodeNumForTxtNode ) );

    return pPrecedingNodeNum;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
