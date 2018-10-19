/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <editeng/svxenum.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <numrule.hxx>
#include <SwNodeNum.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <IDocumentListItems.hxx>
#include <doc.hxx>

SwNodeNum::SwNodeNum(SwTextNode* pTextNode, bool const isHiddenRedlines)
    : mpTextNode( pTextNode )
    , mpNumRule( nullptr )
    , m_isHiddenRedlines(isHiddenRedlines)
{
}

SwNodeNum::SwNodeNum( SwNumRule* pNumRule )
    : mpTextNode( nullptr )
    , mpNumRule( pNumRule )
    , m_isHiddenRedlines(false)
{
}

SwNodeNum::~SwNodeNum()
{
}


void SwNodeNum::ChangeNumRule( SwNumRule& rNumRule )
{
    OSL_ENSURE( GetNumRule() && GetTextNode(),
            "<SwNodeNum::ChangeNumRule(..)> - missing list style and/or text node. Serious defect -> please informm OD." );
    if ( GetNumRule() && GetTextNode() )
    {
        GetNumRule()->RemoveTextNode( *(GetTextNode()) );
    }

    mpNumRule = &rNumRule;

    if ( GetNumRule() && GetTextNode() )
    {
        GetNumRule()->AddTextNode( *(GetTextNode()) );
    }
}

SwPosition SwNodeNum::GetPosition() const
{
    OSL_ENSURE( GetTextNode(),
            "<SwNodeNum::GetPosition()> - no text node set at <SwNodeNum> instance" );
    return SwPosition(*mpTextNode);
}

SwNumberTreeNode * SwNodeNum::Create() const
{
    SwNodeNum * pResult = new SwNodeNum( GetNumRule() );

    return pResult;
}

void SwNodeNum::PreAdd()
{
    OSL_ENSURE( GetTextNode(),
            "<SwNodeNum::PreAdd()> - no text node set at <SwNodeNum> instance" );
    if ( !GetNumRule() && GetTextNode() )
    {
        mpNumRule = GetTextNode()->GetNumRule();
    }
    OSL_ENSURE( GetNumRule(),
            "<SwNodeNum::PreAdd()> - no list style set at <SwNodeNum> instance" );
    if (!m_isHiddenRedlines && GetNumRule() && GetTextNode())
    {
        GetNumRule()->AddTextNode( *(GetTextNode()) );
    }

    if (!m_isHiddenRedlines)
    {
        if ( GetTextNode() &&
             GetTextNode()->GetNodes().IsDocNodes() )
        {
            GetTextNode()->getIDocumentListItems().addListItem( *this );
        }
    }
}

void SwNodeNum::PostRemove()
{
    OSL_ENSURE( GetTextNode(),
            "<SwNodeNum::PostRemove()> - no text node set at <SwNodeNum> instance" );
    OSL_ENSURE( GetNumRule(),
            "<SwNodeNum::PostRemove()> - no list style set at <SwNodeNum> instance" );

    if (!m_isHiddenRedlines && GetTextNode())
    {
        GetTextNode()->getIDocumentListItems().removeListItem( *this );
    }

    if ( GetNumRule() )
    {
        if (!m_isHiddenRedlines && GetTextNode())
        {
            GetNumRule()->RemoveTextNode( *(GetTextNode()) );
        }
        mpNumRule = nullptr;
    }
}

bool SwNodeNum::IsNotifiable() const
{
    bool aResult = true;

    if ( GetTextNode() )
        aResult = GetTextNode()->IsNotifiable();

    return aResult;
}

bool SwNodeNum::IsNotificationEnabled() const
{
    bool aResult = true;

    if ( GetTextNode() )
        aResult = GetTextNode()->IsNotificationEnabled();

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

    if ( GetTextNode() )
    {
        // #i59559#
        // <SwTextNode::IsCounted()> determines, if a text node is counted for numbering
        aResult = GetTextNode()->IsCountedInList();
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
                "<SwNodeNum::HasCountedChildren()> - unexpected type of child" );
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
             !GetTextNode() ||               // root node
             GetTextNode()->HasNumber() ||   // text node
             GetTextNode()->HasBullet() );   // text node
}

void SwNodeNum::NotifyNode()
{
    ValidateMe();

    if (mpTextNode)
    {
        mpTextNode->NumRuleChgd();
    }
}

bool SwNodeNum::LessThan(const SwNumberTreeNode & rNode) const
{
    bool bResult = false;
    const SwNodeNum & rTmpNode = static_cast<const SwNodeNum &>(rNode);

    if (mpTextNode == nullptr && rTmpNode.mpTextNode != nullptr)
        bResult = true;
    else if (mpTextNode != nullptr && rTmpNode.mpTextNode != nullptr)
    {
        // #i83479# - refactoring
        // simplify comparison by comparing the indexes of the text nodes
        bResult = ( mpTextNode->GetIndex() < rTmpNode.mpTextNode->GetIndex() );
    }

    return bResult;
}

bool SwNodeNum::IsRestart() const
{
    bool bIsRestart = false;

    if ( GetTextNode() )
    {
        bIsRestart = GetTextNode()->IsListRestart();
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
        OSL_FAIL( "<SwNodeNum::IsCountPhantoms(): missing numbering rule" );
    }

    return bResult;
}

SwNumberTree::tSwNumTreeNumber SwNodeNum::GetStartValue() const
{
    SwNumberTree::tSwNumTreeNumber aResult = 1;

    if ( IsRestart() && GetTextNode() )
    {
        aResult = GetTextNode()->GetActualListStartValue();
    }
    else
    {
        SwNumRule * pRule = GetNumRule();

        if (pRule)
        {
            int nLevel = GetParent() ? GetLevelInListTree() : 0;

            if (nLevel >= 0 && nLevel < MAXLEVEL)
            {
                const SwNumFormat * pFormat = pRule->GetNumFormat( static_cast<sal_uInt16>(nLevel));

                if (pFormat)
                    aResult = pFormat->GetStart();
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
    UnregisterMeAndChildrenDueToRootDelete( *pRootNode );
}

void SwNodeNum::UnregisterMeAndChildrenDueToRootDelete( SwNodeNum& rNodeNum )
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
            OSL_FAIL( "<SwNodeNum::UnregisterMeAndChildrenDueToRootDelete(..)> - unknown number tree node child" );
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

        UnregisterMeAndChildrenDueToRootDelete( *pChildNode );
    }

    if ( !bIsPhantom )
    {
        SwTextNode* pTextNode( rNodeNum.GetTextNode() );
        if ( pTextNode )
        {
            pTextNode->RemoveFromList();
            // --> clear all list attributes and the list style
            std::set<sal_uInt16> aResetAttrsArray;
            aResetAttrsArray.insert( aResetAttrsArray.end(), RES_PARATR_LIST_ID );
            aResetAttrsArray.insert( aResetAttrsArray.end(), RES_PARATR_LIST_LEVEL );
            aResetAttrsArray.insert( aResetAttrsArray.end(), RES_PARATR_LIST_ISRESTART );
            aResetAttrsArray.insert( aResetAttrsArray.end(), RES_PARATR_LIST_RESTARTVALUE );
            aResetAttrsArray.insert( aResetAttrsArray.end(), RES_PARATR_LIST_ISCOUNTED );
            aResetAttrsArray.insert( aResetAttrsArray.end(), RES_PARATR_NUMRULE );
            SwPaM aPam( *pTextNode );
            pTextNode->GetDoc()->ResetAttrs( aPam, false,
                                            aResetAttrsArray,
                                            false );
        }
    }
}

// #i81002#
const SwNodeNum* SwNodeNum::GetPrecedingNodeNumOf( const SwTextNode& rTextNode ) const
{
    const SwNodeNum* pPrecedingNodeNum( nullptr );

    // #i83479#
    SwNodeNum aNodeNumForTextNode( const_cast<SwTextNode*>(&rTextNode), false/*doesn't matter*/ );

    pPrecedingNodeNum = dynamic_cast<const SwNodeNum*>(
                            GetRoot()
                            ? GetRoot()->GetPrecedingNodeOf( aNodeNumForTextNode )
                            : GetPrecedingNodeOf( aNodeNumForTextNode ) );

    return pPrecedingNodeNum;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
