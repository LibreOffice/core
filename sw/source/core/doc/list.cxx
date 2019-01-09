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
#include <pam.hxx>
#include <SwNodeNum.hxx>

// implementation class for SwList
class SwListImpl
{
    public:
        SwListImpl( const OUString& sListId,
                    SwNumRule& rDefaultListStyle,
                    const SwNodes& rNodes );
        ~SwListImpl() COVERITY_NOEXCEPT_FALSE;

        const OUString& GetListId() const { return msListId;}

        const OUString& GetDefaultListStyleName() const { return msDefaultListStyleName;}

        void InsertListItem( SwNodeNum& rNodeNum, bool isHiddenRedlines,
                             const int nLevel );
        static void RemoveListItem( SwNodeNum& rNodeNum );

        void InvalidateListTree();
        void ValidateListTree();

        void MarkListLevel( const int nListLevel,
                            const bool bValue );

        bool IsListLevelMarked( const int nListLevel ) const;

        // unique identifier of the list
        const OUString msListId;
        // default list style for the list items, identified by the list style name
        OUString msDefaultListStyleName;

        // list trees for certain document ranges
        struct tListTreeForRange
        {
            /// tree always corresponds to document model
            std::unique_ptr<SwNodeNum> pRoot;
            /// Tree that is missing those nodes that are merged or hidden
            /// by delete redlines; this is only used if there is a layout
            /// that has IsHideRedlines() enabled.
            /// A second tree is needed because not only are the numbers in
            /// the nodes different, the structure of the tree may be different
            /// as well, if a high-level node is hidden its children go under
            /// the previous node on the same level.
            /// The nodes of pRootRLHidden are a subset of the nodes of pRoot.
            std::unique_ptr<SwNodeNum> pRootRLHidden;
            /// top-level SwNodes section
            std::unique_ptr<SwPaM> pSection;
            tListTreeForRange(std::unique_ptr<SwNodeNum> p1, std::unique_ptr<SwNodeNum> p2, std::unique_ptr<SwPaM> p3)
                : pRoot(std::move(p1)), pRootRLHidden(std::move(p2)), pSection(std::move(p3)) {}
        };
        typedef std::vector<tListTreeForRange> tListTrees;
        tListTrees maListTrees;

        int mnMarkedListLevel;

        void NotifyItemsOnListLevel( const int nLevel );
};

SwListImpl::SwListImpl( const OUString& sListId,
                        SwNumRule& rDefaultListStyle,
                        const SwNodes& rNodes )
    : msListId( sListId ),
      msDefaultListStyleName( rDefaultListStyle.GetName() ),
      maListTrees(),
      mnMarkedListLevel( MAXLEVEL )
{
    // create empty list trees for the document ranges
    const SwNode* pNode = rNodes[0];
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
            sal_uLong nIndex = pNode->GetIndex();
            nIndex++;
            pNode = rNodes[nIndex];
        }
    }
    while ( pNode != &rNodes.GetEndOfContent() );
}

SwListImpl::~SwListImpl() COVERITY_NOEXCEPT_FALSE
{
    for ( auto& rNumberTree : maListTrees )
    {
        SwNodeNum::HandleNumberTreeRootNodeDelete(*(rNumberTree.pRoot));
        SwNodeNum::HandleNumberTreeRootNodeDelete(*(rNumberTree.pRootRLHidden));
    }
}

void SwListImpl::InsertListItem( SwNodeNum& rNodeNum, bool const isHiddenRedlines,
                                 const int nLevel )
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
            pRoot->AddChild(&rNodeNum, nLevel);
            break;
        }
    }
}

void SwListImpl::RemoveListItem( SwNodeNum& rNodeNum )
{
    rNodeNum.RemoveMe();
}

void SwListImpl::InvalidateListTree()
{
    for ( auto& rNumberTree : maListTrees )
    {
        rNumberTree.pRoot->InvalidateTree();
        rNumberTree.pRootRLHidden->InvalidateTree();
    }
}

void SwListImpl::ValidateListTree()
{
    for ( auto& rNumberTree : maListTrees )
    {
        rNumberTree.pRoot->NotifyInvalidChildren();
        rNumberTree.pRootRLHidden->NotifyInvalidChildren();
    }
}

void SwListImpl::MarkListLevel( const int nListLevel,
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

bool SwListImpl::IsListLevelMarked( const int nListLevel ) const
{
    return nListLevel == mnMarkedListLevel;
}

void SwListImpl::NotifyItemsOnListLevel( const int nLevel )
{
    for ( auto& rNumberTree : maListTrees )
    {
        rNumberTree.pRoot->NotifyNodesOnListLevel( nLevel );
        rNumberTree.pRootRLHidden->NotifyNodesOnListLevel( nLevel );
    }
}

SwList::SwList( const OUString& sListId,
                SwNumRule& rDefaultListStyle,
                const SwNodes& rNodes )
    : mpListImpl( new SwListImpl( sListId, rDefaultListStyle, rNodes ) )
{
}

SwList::~SwList()
{
}

const OUString & SwList::GetListId() const
{
    return mpListImpl->GetListId();
}

const OUString & SwList::GetDefaultListStyleName() const
{
    return mpListImpl->GetDefaultListStyleName();
}

void SwList::SetDefaultListStyleName(OUString const& rNew)
{
    mpListImpl->msDefaultListStyleName = rNew;
}

void SwList::InsertListItem( SwNodeNum& rNodeNum, bool const isHiddenRedlines,
                             const int nLevel )
{
    mpListImpl->InsertListItem( rNodeNum, isHiddenRedlines, nLevel );
}

void SwList::RemoveListItem( SwNodeNum& rNodeNum )
{
    SwListImpl::RemoveListItem( rNodeNum );
}

void SwList::InvalidateListTree()
{
    mpListImpl->InvalidateListTree();
}

void SwList::ValidateListTree()
{
    mpListImpl->ValidateListTree();
}

void SwList::MarkListLevel( const int nListLevel,
                                  const bool bValue )
{
    mpListImpl->MarkListLevel( nListLevel, bValue );
}

bool SwList::IsListLevelMarked( const int nListLevel ) const
{
    return mpListImpl->IsListLevelMarked( nListLevel );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
