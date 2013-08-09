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
        SwListImpl( const String sListId,
                    SwNumRule& rDefaultListStyle,
                    const SwNodes& rNodes );
        ~SwListImpl();

        const String GetListId() const;

        const String GetDefaultListStyleName() const;

        void InsertListItem( SwNodeNum& rNodeNum,
                             const int nLevel );
        void RemoveListItem( SwNodeNum& rNodeNum );

        void InvalidateListTree();
        void ValidateListTree();

        void MarkListLevel( const int nListLevel,
                            const bool bValue );

        bool IsListLevelMarked( const int nListLevel ) const;

    private:
        // unique identifier of the list
        const String msListId;
        // default list style for the list items, identified by the list style name
        String msDefaultListStyleName;

        // list trees for certain document ranges
        typedef std::pair<SwNodeNum*, SwPaM*> tListTreeForRange;
        typedef std::vector<tListTreeForRange> tListTrees;
        tListTrees maListTrees;

        int mnMarkedListLevel;

        void NotifyItemsOnListLevel( const int nLevel );
};

SwListImpl::SwListImpl( const String sListId,
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

        SwNodeNum* pNumberTreeRootNode = new SwNodeNum( &rDefaultListStyle );
        SwPaM* pPam = new SwPaM( *(aPam.Start()), *(aPam.End()) );
        tListTreeForRange aListTreeForRange( pNumberTreeRootNode, pPam );
        maListTrees.push_back( aListTreeForRange );

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

SwListImpl::~SwListImpl()
{
    tListTrees::iterator aNumberTreeIter;
    for ( aNumberTreeIter = maListTrees.begin();
          aNumberTreeIter != maListTrees.end();
          ++aNumberTreeIter )
    {
        SwNodeNum::HandleNumberTreeRootNodeDelete( *((*aNumberTreeIter).first) );
        delete (*aNumberTreeIter).first;
        delete (*aNumberTreeIter).second;
    }
}

const String SwListImpl::GetListId() const
{
    return msListId;
}

const String SwListImpl::GetDefaultListStyleName() const
{
    return msDefaultListStyleName;
}

void SwListImpl::InsertListItem( SwNodeNum& rNodeNum,
                                 const int nLevel )
{
    const SwPosition aPosOfNodeNum( rNodeNum.GetPosition() );
    const SwNodes* pNodesOfNodeNum = &(aPosOfNodeNum.nNode.GetNode().GetNodes());

    tListTrees::const_iterator aNumberTreeIter;
    for ( aNumberTreeIter = maListTrees.begin();
          aNumberTreeIter != maListTrees.end();
          ++aNumberTreeIter )
    {
        const SwPosition* pStart = (*aNumberTreeIter).second->Start();
        const SwPosition* pEnd = (*aNumberTreeIter).second->End();
        const SwNodes* pRangeNodes = &(pStart->nNode.GetNode().GetNodes());

        if ( pRangeNodes == pNodesOfNodeNum &&
             *pStart <= aPosOfNodeNum && aPosOfNodeNum <= *pEnd)
        {
            (*aNumberTreeIter).first->AddChild( &rNodeNum, nLevel );

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
    tListTrees::iterator aNumberTreeIter;
    for ( aNumberTreeIter = maListTrees.begin();
          aNumberTreeIter != maListTrees.end();
          ++aNumberTreeIter )
    {
        (*aNumberTreeIter).first->InvalidateTree();
    }
}

void SwListImpl::ValidateListTree()
{
    tListTrees::iterator aNumberTreeIter;
    for ( aNumberTreeIter = maListTrees.begin();
          aNumberTreeIter != maListTrees.end();
          ++aNumberTreeIter )
    {
        (*aNumberTreeIter).first->NotifyInvalidChildren();
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
    tListTrees::iterator aNumberTreeIter;
    for ( aNumberTreeIter = maListTrees.begin();
          aNumberTreeIter != maListTrees.end();
          ++aNumberTreeIter )
    {
        (*aNumberTreeIter).first->NotifyNodesOnListLevel( nLevel );
    }
}

// SwList ---------------------------------------------------------------------
SwList::SwList( const String sListId,
                SwNumRule& rDefaultListStyle,
                const SwNodes& rNodes )
    : mpListImpl( new SwListImpl( sListId, rDefaultListStyle, rNodes ) )
{
}

SwList::~SwList()
{
    delete mpListImpl;
}

const String SwList::GetListId() const
{
    return mpListImpl->GetListId();
}

const String SwList::GetDefaultListStyleName() const
{
    return mpListImpl->GetDefaultListStyleName();
}

void SwList::InsertListItem( SwNodeNum& rNodeNum,
                             const int nLevel )
{
    mpListImpl->InsertListItem( rNodeNum, nLevel );
}

void SwList::RemoveListItem( SwNodeNum& rNodeNum )
{
    mpListImpl->RemoveListItem( rNodeNum );
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
