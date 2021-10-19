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

#include <list.hxx>

#include <vector>
#include <numrule.hxx>
#include <ndarr.hxx>
#include <node.hxx>

SwList::SwList( const OUString& sListId,
                        SwNumRule& rDefaultListStyle,
                        const SwNodes& rNodes )
    : msListId( sListId ),
      msDefaultListStyleName( rDefaultListStyle.GetName() ),
      mnMarkedListLevel( MAXLEVEL )
{
    // create empty list trees for the document ranges
    const SwNode* pNode = rNodes[SwNodeOffset(0)];
    do
    {
        SwPaM aPam( *pNode, *pNode->EndOfSectionNode() );

        maListTrees.emplace_back(
            std::make_unique<SwNodeNum>( &rDefaultListStyle ),
            std::make_unique<SwNodeNum>( &rDefaultListStyle ),
            std::make_unique<SwPaM>( *(aPam.Start()), *(aPam.End()) ));

        pNode = pNode->EndOfSectionNode();
        if (pNode != &rNodes.GetEndOfContent())
        {
            SwNodeOffset nIndex = pNode->GetIndex();
            nIndex++;
            pNode = rNodes[nIndex];
        }
    }
    while ( pNode != &rNodes.GetEndOfContent() );
}

SwList::~SwList() COVERITY_NOEXCEPT_FALSE
{
    for ( auto& rNumberTree : maListTrees )
    {
        SwNodeNum::HandleNumberTreeRootNodeDelete(*(rNumberTree.pRoot));
        SwNodeNum::HandleNumberTreeRootNodeDelete(*(rNumberTree.pRootRLHidden));
    }
}

bool SwList::HasNodes() const
{
    for (auto const& rNumberTree : maListTrees)
    {
        if (rNumberTree.pRoot->GetChildCount() != 0)
        {
            return true;
        }
    }
    return false;
}

void SwList::InsertListItem(SwNodeNum& rNodeNum, bool const isHiddenRedlines,
                            const int nLevel, const SwDoc& rDoc)
{
    const SwPosition aPosOfNodeNum( rNodeNum.GetPosition() );
    const SwNodes* pNodesOfNodeNum = &(aPosOfNodeNum.nNode.GetNode().GetNodes());

    for ( const auto& rNumberTree : maListTrees )
    {
        const SwPosition* pStart = rNumberTree.pSection->Start();
        const SwPosition* pEnd = rNumberTree.pSection->End();
        const SwNodes* pRangeNodes = &(pStart->nNode.GetNode().GetNodes());

        if ( pRangeNodes == pNodesOfNodeNum &&
             *pStart <= aPosOfNodeNum && aPosOfNodeNum <= *pEnd)
        {
            auto const& pRoot(isHiddenRedlines
                    ? rNumberTree.pRootRLHidden
                    : rNumberTree.pRoot);
            pRoot->AddChild(&rNodeNum, nLevel, rDoc);
            break;
        }
    }
}

void SwList::RemoveListItem(SwNodeNum& rNodeNum, const SwDoc& rDoc)
{
    rNodeNum.RemoveMe(rDoc);
}

void SwList::InvalidateListTree()
{
    for ( const auto& rNumberTree : maListTrees )
    {
        rNumberTree.pRoot->InvalidateTree();
        rNumberTree.pRootRLHidden->InvalidateTree();
    }
}

void SwList::ValidateListTree(const SwDoc& rDoc)
{
    for ( auto& rNumberTree : maListTrees )
    {
        rNumberTree.pRoot->NotifyInvalidChildren(rDoc);
        rNumberTree.pRootRLHidden->NotifyInvalidChildren(rDoc);
    }
}

void SwList::MarkListLevel( const int nListLevel,
                                const bool bValue )
{
    if ( bValue )
    {
        if ( nListLevel != mnMarkedListLevel )
        {
            if ( mnMarkedListLevel != MAXLEVEL )
            {
                // notify former marked list nodes
                NotifyItemsOnListLevel( mnMarkedListLevel );
            }

            mnMarkedListLevel = nListLevel;

            // notify new marked list nodes
            NotifyItemsOnListLevel( mnMarkedListLevel );
        }
    }
    else
    {
        if ( mnMarkedListLevel != MAXLEVEL )
        {
            // notify former marked list nodes
            NotifyItemsOnListLevel( mnMarkedListLevel );
        }

        mnMarkedListLevel = MAXLEVEL;
    }
}

bool SwList::IsListLevelMarked( const int nListLevel ) const
{
    return nListLevel == mnMarkedListLevel;
}

void SwList::NotifyItemsOnListLevel( const int nLevel )
{
    for ( auto& rNumberTree : maListTrees )
    {
        rNumberTree.pRoot->NotifyNodesOnListLevel( nLevel );
        rNumberTree.pRootRLHidden->NotifyNodesOnListLevel( nLevel );
    }
}

void SwList::SetDefaultListStyleName(OUString const& rNew)
{
    msDefaultListStyleName = rNew;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
