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

#include <algorithm>
#include <functional>
#include <SwNumberTree.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>

#include <cassert>

using std::vector;
using std::find;

SwNumberTreeNode::SwNumberTreeNode()
    : mChildren(),
      mpParent( nullptr ),
      mnNumber( 0 ),
      mbContinueingPreviousSubTree( false ),
      mbPhantom( false ),
      mItLastValid()
{
    mItLastValid = mChildren.end();
}

SwNumberTreeNode::~SwNumberTreeNode()
{
    if (GetChildCount() > 0)
    {
        if (HasOnlyPhantoms())
        {
            delete *mChildren.begin();

            mChildren.clear();
            mItLastValid = mChildren.end();
        }
        else
        {
            OSL_FAIL("lost children!");
        }
    }

    OSL_ENSURE( IsPhantom() || mpParent == nullptr, ": I'm not supposed to have a parent.");

    mpParent = reinterpret_cast<SwNumberTreeNode *>(0xdeadbeef);

    OSL_ENSURE(mChildren.empty(), "children left!");
}

SwNumberTreeNode * SwNumberTreeNode::CreatePhantom()
{
    SwNumberTreeNode * pNew = nullptr;

    if (! mChildren.empty() &&
        (*mChildren.begin())->IsPhantom())
    {
        OSL_FAIL("phantom already present");
    }
    else
    {
        pNew = Create();
        pNew->mbPhantom = true;
        pNew->mpParent = this;

        std::pair<tSwNumberTreeChildren::iterator, bool> aInsert =
            mChildren.insert(pNew);

        if (! aInsert.second)
        {
            OSL_FAIL("insert of phantom failed!");

            delete pNew;
            pNew = nullptr;
        }
    }

    return pNew;
}

SwNumberTreeNode * SwNumberTreeNode::GetRoot() const
{
    SwNumberTreeNode * pResult = mpParent;

    if (pResult)
        while (pResult->mpParent)
            pResult = pResult->mpParent;

    return pResult;
}

void SwNumberTreeNode::ClearObsoletePhantoms()
{
    tSwNumberTreeChildren::iterator aIt = mChildren.begin();

    if (aIt != mChildren.end() && (*aIt)->IsPhantom())
    {
        (*aIt)->ClearObsoletePhantoms();

        if ((*aIt)->mChildren.empty())
        {
            // #i60652#
            // Because <mChildren.erase(aIt)> could destroy the element, which
            // is referenced by <mItLastValid>, it's needed to adjust
            // <mItLastValid> before erasing <aIt>.
            SetLastValid(mChildren.end());

            delete *aIt;
            mChildren.erase(aIt);
        }
    }
}

void SwNumberTreeNode::ValidateHierarchical(const SwNumberTreeNode * pNode) const
{
    tSwNumberTreeChildren::const_iterator aValidateIt =
        GetIterator(pNode);

    if (aValidateIt != mChildren.end())
    {
        OSL_ENSURE((*aValidateIt)->mpParent == this, "wrong parent");

        tSwNumberTreeChildren::const_iterator aIt = mItLastValid;

        // -->
        // improvement:
        // - Only one time checked for <mChildren.end()>.
        // - Less checks for each loop run.
        // correction:
        // - consider case that current node isn't counted and isn't the first
        // child of its parent. In this case the number of last counted child
        // of the previous node determines the start value for the following
        // children loop, if all children have to be validated and the first
        // one doesn't restart the counting.
        SwNumberTree::tSwNumTreeNumber nTmpNumber( 0 );
        if (aIt != mChildren.end())
            nTmpNumber = (*aIt)->mnNumber;
        else
        {
            aIt = mChildren.begin();
            (*aIt)->mbContinueingPreviousSubTree = false;

            // determine default start value
            // consider the case that the first child isn't counted.
            nTmpNumber = (*aIt)->GetStartValue();
            if ( !(*aIt)->IsCounted() &&
                 ( !(*aIt)->HasCountedChildren() || (*aIt)->IsPhantom() ) )
            {
                --nTmpNumber;
            }

            // determine special start value for the case that first child
            // doesn't restart the numbering and the parent node isn't counted
            // and isn't the first child.
            const bool bParentCounted( IsCounted() &&
                                       ( !IsPhantom() ||
                                         HasPhantomCountedParent() ) );
            if ( !(*aIt)->IsRestart() &&
                 GetParent() && !bParentCounted )
            {
                tSwNumberTreeChildren::const_iterator aParentChildIt =
                                                GetParent()->GetIterator( this );
                while ( aParentChildIt != GetParent()->mChildren.begin() )
                {
                    --aParentChildIt;
                    SwNumberTreeNode* pPrevNode( *aParentChildIt );
                    if ( pPrevNode->GetChildCount() > 0 )
                    {
                        (*aIt)->mbContinueingPreviousSubTree = true;
                        nTmpNumber = (*(pPrevNode->mChildren.rbegin()))->GetNumber();
                        if ( (*aIt)->IsCounted() &&
                             ( !(*aIt)->IsPhantom() ||
                               (*aIt)->HasPhantomCountedParent() ) )
                        {
                            ++nTmpNumber;
                        }
                        break;
                    }
                    else if ( pPrevNode->IsCounted() )
                    {
                        break;
                    }
                    else
                    {
                        // Previous node has no children and is not counted.
                        // Thus, next turn and check for the previous node.
                    }
                }
            }

            (*aIt)->mnNumber = nTmpNumber;
        }

        while (aIt != aValidateIt)
        {
            ++aIt;
            (*aIt)->mbContinueingPreviousSubTree = false;

            // --> only for counted nodes the number
            // has to be adjusted, compared to the previous node.
            // this condition is hold also for nodes, which restart the numbering.
            if ( (*aIt)->IsCounted() )
            {
                if ((*aIt)->IsRestart())
                    nTmpNumber = (*aIt)->GetStartValue();
                else
                    ++nTmpNumber;
            }

            (*aIt)->mnNumber = nTmpNumber;
        }

        SetLastValid(aIt, true);
    }
}

void SwNumberTreeNode::ValidateContinuous(const SwNumberTreeNode * pNode) const
{
    tSwNumberTreeChildren::const_iterator aIt = mItLastValid;

    do
    {
        if (aIt == mChildren.end())
        {
            aIt = mChildren.begin();
        }
        else
            ++aIt;

        if (aIt != mChildren.end())
        {
            SwNumberTree::tSwNumTreeNumber nTmpNumber = 0;
            SwNumberTreeNode * pPred = (*aIt)->GetPred();

            // #i64311#
            // correct consideration of phantoms
            // correct consideration of restart at a number tree node
            if ( pPred )
            {
                if ( !(*aIt)->IsCounted() )
                    // #i65284#
                    nTmpNumber = pPred->GetNumber( pPred->GetParent() != (*aIt)->GetParent() );
                else
                {
                    if ( (*aIt)->IsRestart() )
                        nTmpNumber = (*aIt)->GetStartValue();
                    else
                        nTmpNumber = pPred->GetNumber( pPred->GetParent() != (*aIt)->GetParent() ) + 1;
                }
            }
            else
            {
                if ( !(*aIt)->IsCounted() )
                    nTmpNumber = GetStartValue() - 1;
                else
                {
                    if ( (*aIt)->IsRestart() )
                        nTmpNumber = (*aIt)->GetStartValue();
                    else
                       nTmpNumber = GetStartValue();
                }
            }

            (*aIt)->mnNumber = nTmpNumber;
        }
    }
    while (aIt != mChildren.end() && *aIt != pNode);

    // #i74748# - applied patch from garnier_romain
    // number tree node has to be validated.
    SetLastValid( aIt, true );
}

void SwNumberTreeNode::Validate(const SwNumberTreeNode * pNode) const
{
    if (! IsValid(pNode))
    {
        if (IsContinuous())
            ValidateContinuous(pNode);
        else
            ValidateHierarchical(pNode);
    }
}

void SwNumberTreeNode::GetNumberVector_(SwNumberTree::tNumberVector & rVector,
                                        bool bValidate) const
{
    if (mpParent)
    {
        mpParent->GetNumberVector_(rVector, bValidate);
        rVector.push_back(GetNumber(bValidate));
    }
}

SwNumberTreeNode * SwNumberTreeNode::GetFirstNonPhantomChild()
{
    if (IsPhantom())
        return (*mChildren.begin())->GetFirstNonPhantomChild();

    return this;
}

/** Moves all children of this node that are greater than a given node
    to the destination node.
*/
void SwNumberTreeNode::MoveGreaterChildren( SwNumberTreeNode& _rCompareNode,
                                            SwNumberTreeNode& _rDestNode )
{
    if ( mChildren.empty() )
        return;

    // determine first child, which has to move to <_rDestNode>
    tSwNumberTreeChildren::iterator aItUpper( mChildren.end() );
    if ((*mChildren.begin())->IsPhantom() &&
        _rCompareNode.LessThan(*(*mChildren.begin())->GetFirstNonPhantomChild()))
    {
        aItUpper = mChildren.begin();
    }
    else
    {
        aItUpper = mChildren.upper_bound(&_rCompareNode);
    }

    // move children
    if (aItUpper != mChildren.end())
    {
        tSwNumberTreeChildren::iterator aIt;
        for (aIt = aItUpper; aIt != mChildren.end(); ++aIt)
            (*aIt)->mpParent = &_rDestNode;

        _rDestNode.mChildren.insert(aItUpper, mChildren.end());

        // #i60652#
        // Because <mChildren.erase(aItUpper, mChildren.end())> could destroy
        // the element, which is referenced by <mItLastValid>, it's needed to
        // adjust <mItLastValid> before erasing <aIt>.
        SetLastValid( mChildren.end() );

        mChildren.erase(aItUpper, mChildren.end());

        // #i60652#
        if ( !mChildren.empty() )
        {
            SetLastValid( --(mChildren.end()) );
        }
    }

#ifdef DBG_UTIL
    IsSane(false);
    _rDestNode.IsSane(true);
#endif
}

void SwNumberTreeNode::MoveChildren(SwNumberTreeNode * pDest)
{
    if (! mChildren.empty())
    {
        tSwNumberTreeChildren::iterator aItBegin = mChildren.begin();
        SwNumberTreeNode * pMyFirst = *mChildren.begin();

        // #i60652#
        // Because <mChildren.erase(aItBegin)> could destroy the element,
        // which is referenced by <mItLastValid>, it's needed to adjust
        // <mItLastValid> before erasing <aItBegin>.
        SetLastValid(mChildren.end());

        if (pMyFirst->IsPhantom())
        {
            SwNumberTreeNode * pDestLast = nullptr;

            if (pDest->mChildren.empty())
                pDestLast = pDest->CreatePhantom();
            else
                pDestLast = *pDest->mChildren.rbegin();

            pMyFirst->MoveChildren(pDestLast);

            delete pMyFirst;
            mChildren.erase(aItBegin);

            aItBegin = mChildren.begin();
        }

        for (auto& rpChild : mChildren)
            rpChild->mpParent = pDest;

        pDest->mChildren.insert(mChildren.begin(), mChildren.end());
        mChildren.clear();
        // <stl::set.clear()> destroys all existing iterators.
        // Thus, <mItLastValid> is also destroyed and reset becomes necessary
        mItLastValid = mChildren.end();
    }

    OSL_ENSURE(mChildren.empty(), "MoveChildren failed!");

#ifdef DBG_UTIL
    IsSane(false);
    pDest->IsSane(false);
#endif
}

void SwNumberTreeNode::AddChild( SwNumberTreeNode * pChild,
                                 const int nDepth )
{
    /*
       Algorithm:

       Search first child A that is greater than pChild,
         A may be the end of children.
       If nDepth > 0 then
       {
          if A is first child then
            create new phantom child B at beginning of child list
          else
            B is A

          Add child to B with depth nDepth - 1.
       }
       else
       {
         Insert pNode before A.

         if A has predecessor B then
           remove children of B that are greater as A and insert them as
             children of A.
       }

*/

    if ( nDepth < 0 )
    {
        OSL_FAIL( "<SwNumberTreeNode::AddChild(..)> - parameter <nDepth> out of valid range. Serious defect." );
        return;
    }

    if ( pChild->GetParent() != nullptr || pChild->GetChildCount() > 0 )
    {
        OSL_FAIL("only orphans allowed.");
        return;
    }

    if (nDepth > 0)
    {
        tSwNumberTreeChildren::iterator aInsertDeepIt =
            mChildren.upper_bound(pChild);

        OSL_ENSURE(! (aInsertDeepIt != mChildren.end() &&
                  (*aInsertDeepIt)->IsPhantom()), " unexpected phantom");

        if (aInsertDeepIt == mChildren.begin())
        {
            SwNumberTreeNode * pNew = CreatePhantom();

            SetLastValid(mChildren.end());

            if (pNew)
                pNew->AddChild(pChild, nDepth - 1);
        }
        else
        {
            --aInsertDeepIt;
            (*aInsertDeepIt)->AddChild(pChild, nDepth - 1);
        }

    }
    else
    {
        pChild->PreAdd();
        std::pair<tSwNumberTreeChildren::iterator, bool> aResult =
            mChildren.insert(pChild);

        if (aResult.second)
        {
            pChild->mpParent = this;
            bool bNotification = pChild->IsNotificationEnabled();
            tSwNumberTreeChildren::iterator aInsertedIt = aResult.first;

            if (aInsertedIt != mChildren.begin())
            {
                tSwNumberTreeChildren::iterator aPredIt = aInsertedIt;
                --aPredIt;

                // -->
                // Move greater children of previous node to new child.
                // This has to be done recursively on the children levels.
                // Initialize loop variables <pPrevChildNode> and <pDestNode>
                // for loop on children levels.
                SwNumberTreeNode* pPrevChildNode( *aPredIt );
                SwNumberTreeNode* pDestNode( pChild );
                while ( pDestNode && pPrevChildNode &&
                        pPrevChildNode->GetChildCount() > 0 )
                {
                    // move children
                    pPrevChildNode->MoveGreaterChildren( *pChild, *pDestNode );

                    // prepare next loop:
                    // - search of last child of <pPrevChildNode
                    // - If found, determine destination node
                    if ( pPrevChildNode->GetChildCount() > 0 )
                    {
                        tSwNumberTreeChildren::reverse_iterator aIt =
                                        pPrevChildNode->mChildren.rbegin();
                        pPrevChildNode = *aIt;
                        // determine new destination node
                        if ( pDestNode->GetChildCount() > 0 )
                        {
                            pDestNode = *(pDestNode->mChildren.begin());
                            if ( !pDestNode->IsPhantom() )
                            {
                                pDestNode = pDestNode->mpParent->CreatePhantom();
                            }
                        }
                        else
                        {
                            pDestNode = pDestNode->CreatePhantom();
                        }
                    }
                    else
                    {
                        // ready -> break loop.
                        break;
                    }
                }
                // assure that unnessary created phantoms at <pChild> are deleted.
                pChild->ClearObsoletePhantoms();

                if ((*aPredIt)->IsValid())
                    SetLastValid(aPredIt);
            }
            else
                SetLastValid(mChildren.end());

            ClearObsoletePhantoms();

            if( bNotification )
            {
                // invalidation of not counted parent
                // and notification of its siblings.
                if ( !IsCounted() )
                {
                    InvalidateMe();
                    NotifyInvalidSiblings();
                }
                NotifyInvalidChildren();
            }
        }
    }

#ifdef DBG_UTIL
    IsSane(false);
#endif
}

void SwNumberTreeNode::RemoveChild(SwNumberTreeNode * pChild)
{
    /*
       Algorithm:

       if pChild has predecessor A then
         B is A
       else
         create phantom child B at beginning of child list

       Move children of pChild to B.
    */

    if (pChild->IsPhantom())
    {
        OSL_FAIL("not applicable to phantoms!");

        return;
    }

    tSwNumberTreeChildren::const_iterator aRemoveIt = GetIterator(pChild);

    if (aRemoveIt != mChildren.end())
    {
        SwNumberTreeNode * pRemove = *aRemoveIt;

        pRemove->mpParent = nullptr;

        tSwNumberTreeChildren::const_iterator aItPred = mChildren.end();

        if (aRemoveIt == mChildren.begin())
        {
            if (! pRemove->mChildren.empty())
            {
                CreatePhantom();

                aItPred = mChildren.begin();
            }
        }
        else
        {
            aItPred = aRemoveIt;
            --aItPred;
        }

        if (! pRemove->mChildren.empty())
        {
            pRemove->MoveChildren(*aItPred);
            (*aItPred)->InvalidateTree();
            (*aItPred)->NotifyInvalidChildren();
        }

        // #i60652#
        // Because <mChildren.erase(aRemoveIt)> could destroy the element,
        // which is referenced by <mItLastValid>, it's needed to adjust
        // <mItLastValid> before erasing <aRemoveIt>.
        if (aItPred != mChildren.end() && (*aItPred)->IsPhantom())
            SetLastValid(mChildren.end());
        else
            SetLastValid(aItPred);

        mChildren.erase(aRemoveIt);

        NotifyInvalidChildren();
    }
    else
    {
        OSL_FAIL("RemoveChild: failed!");
    }

    pChild->PostRemove();
}

void SwNumberTreeNode::RemoveMe()
{
    if (mpParent)
    {
        SwNumberTreeNode * pSavedParent = mpParent;

        pSavedParent->RemoveChild(this);

        while (pSavedParent && pSavedParent->IsPhantom() &&
               pSavedParent->HasOnlyPhantoms())
            pSavedParent = pSavedParent->GetParent();

        if (pSavedParent)
            pSavedParent->ClearObsoletePhantoms();

#ifdef DBG_UTIL
        IsSane(false);
#endif
    }
}

bool SwNumberTreeNode::IsValid() const
{
    return mpParent && mpParent->IsValid(this);
}

SwNumberTree::tSwNumTreeNumber SwNumberTreeNode::GetNumber(bool bValidate)
    const
{
    if (bValidate && mpParent)
        mpParent->Validate(this);

    return mnNumber;
}


SwNumberTree::tNumberVector SwNumberTreeNode::GetNumberVector() const
{
    vector<SwNumberTree::tSwNumTreeNumber> aResult;

    GetNumberVector_(aResult);

    return aResult;
}

bool SwNumberTreeNode::IsValid(const SwNumberTreeNode * pChild) const
{
  bool bResult = false;

  if (mItLastValid != mChildren.end())
  {
      if (pChild && pChild->mpParent == this)
      {
          bResult = ! (*mItLastValid)->LessThan(*pChild);
      }
  }

  return bResult;
}


bool SwNumberTreeNode::HasOnlyPhantoms() const
{
    bool bResult = false;

    if (GetChildCount() == 1)
    {
        tSwNumberTreeChildren::const_iterator aIt = mChildren.begin();

        bResult = (*aIt)->IsPhantom() && (*aIt)->HasOnlyPhantoms();
    }
    else if (GetChildCount() == 0)
        bResult = true;

    return bResult;
}

bool SwNumberTreeNode::IsCounted() const
{
    return !IsPhantom() ||
            ( IsCountPhantoms() && HasCountedChildren() );
}

bool SwNumberTreeNode::HasPhantomCountedParent() const
{
    bool bRet( false );

    OSL_ENSURE( IsPhantom(),
            "<SwNumberTreeNode::HasPhantomCountedParent()> - wrong usage of method - it's only for phantoms" );
    if ( IsPhantom() && mpParent )
    {
        if ( mpParent == GetRoot() )
        {
            bRet = true;
        }
        else if ( !mpParent->IsPhantom() )
        {
            bRet = mpParent->IsCounted();
        }
        else
        {
            bRet = mpParent->IsCounted() && mpParent->HasPhantomCountedParent();
        }
    }

    return bRet;
}

bool SwNumberTreeNode::IsFirst(const SwNumberTreeNode * pNode) const
{
    tSwNumberTreeChildren::const_iterator aIt = mChildren.begin();

    if ((*aIt)->IsPhantom())
        ++aIt;

    return *aIt == pNode;
}

bool SwNumberTreeNode::IsFirst() const
{
    bool bResult = true;

    if (GetParent())
    {
        if (GetParent()->IsFirst(this))
        {
            SwNumberTreeNode * pNode = GetParent();

            while (pNode)
            {
                if (!pNode->IsPhantom() && pNode->GetParent())
                {
                    bResult = false;
                    break;
                }

                pNode = pNode->GetParent();
            }

            // If node isn't the first child, it is the second child and the
            // first child is a phanton. In this case check, if the first phantom
            // child have only phanton children
            if ( bResult &&
                 this != *(GetParent()->mChildren.begin()) &&
                 !(*(GetParent()->mChildren.begin()))->HasOnlyPhantoms() )
            {
                bResult = false;
            }
        }
        else
            bResult = false;
    }

    return bResult;
}

void SwNumberTreeNode::SetLevelInListTree( const int nLevel )
{
    if ( nLevel < 0 )
    {
        OSL_FAIL( "<SwNumberTreeNode::SetLevelInListTree(..)> - parameter <nLevel> out of valid range. Serious defect." );
        return;
    }

    OSL_ENSURE( GetParent(),
            "<SwNumberTreeNode::SetLevelInListTree(..)> - can only be called for number tree nodes in a list tree" );
    if ( GetParent() )
    {
        if ( nLevel != GetLevelInListTree() )
        {
            SwNumberTreeNode* pRootTreeNode = GetRoot();
            OSL_ENSURE( pRootTreeNode,
                    "<SwNumberTreeNode::SetLevelInListTree(..)> - no root tree node found. Serious defect." );

            RemoveMe();
            pRootTreeNode->AddChild( this, nLevel );
        }
    }
}

int SwNumberTreeNode::GetLevelInListTree() const
{
    if (mpParent)
        return mpParent->GetLevelInListTree() + 1;

    return -1;
}

SwNumberTreeNode::tSwNumberTreeChildren::size_type
SwNumberTreeNode::GetChildCount() const
{
    return mChildren.size();
}

#ifdef DBG_UTIL
void SwNumberTreeNode::IsSane(bool bRecursive) const
{
    vector<const SwNumberTreeNode*> aParents;

    return IsSane(bRecursive, aParents);
}

void SwNumberTreeNode::IsSane(bool bRecursive,
                              vector<const SwNumberTreeNode *> rParents)
    const
{
    assert(find(rParents.begin(), rParents.end(), this) == rParents.end());

    assert(rParents.empty() || rParents.back() == mpParent);

    rParents.push_back(this);

    bool bFirst = true;
    for (const auto& rpChild : mChildren)
    {
        if (rpChild)
        {
            if (rpChild->IsPhantom())
            {
                SAL_WARN_IF(rpChild->HasOnlyPhantoms(), "sw.core",
                        "HasOnlyPhantoms: is this an error?");

                assert(bFirst && "found phantom not at first position.");
            }

            assert(rpChild->mpParent == this);

            if (mpParent)
            {
                assert(rpChild->IsPhantom() || !rpChild->LessThan(*this));
            }
        }
        else
        {
            assert(!"found child that is NULL");
        }

        if (bRecursive)
        {
            rpChild->IsSane(bRecursive, rParents);
        }

        bFirst = false;
    }

    rParents.pop_back();
}
#endif // DBG_UTIL

SwNumberTreeNode::tSwNumberTreeChildren::const_iterator
SwNumberTreeNode::GetIterator(const SwNumberTreeNode * pChild) const
{
    tSwNumberTreeChildren::const_iterator aItResult =
        mChildren.find(const_cast<SwNumberTreeNode *>(pChild));

    OSL_ENSURE( aItResult != mChildren.end(),
            "something went wrong getting the iterator for a child");

    return aItResult;
}

bool SwNumberTreeNodeLessThan(const SwNumberTreeNode * pA,
                              const SwNumberTreeNode * pB)
{
  bool bResult = false;

  if (pA == nullptr && pB != nullptr)
    bResult = true;
  else if (pA != nullptr && pB != nullptr)
    bResult = pA->LessThan(*pB);

  return bResult;
}

SwNumberTreeNode * SwNumberTreeNode::GetLastDescendant() const
{
    SwNumberTreeNode * pResult = nullptr;
    tSwNumberTreeChildren::const_reverse_iterator aIt = mChildren.rbegin();

    if (aIt != mChildren.rend())
    {
        pResult = (*aIt)->GetLastDescendant();

        if (! pResult)
            pResult = *aIt;
    }

    return pResult;
}

bool SwNumberTreeNode::LessThan(const SwNumberTreeNode & rTreeNode) const
{
    return this < &rTreeNode;
}

SwNumberTreeNode * SwNumberTreeNode::GetPred(bool bSibling) const
{
    SwNumberTreeNode * pResult = nullptr;

    if (mpParent)
    {
        tSwNumberTreeChildren::const_iterator aIt =
            mpParent->GetIterator(this);

        if ( aIt == mpParent->mChildren.begin() )
        {
            // #i64311#
            // root node is no valid predecessor
            pResult = mpParent->GetParent() ? mpParent : nullptr;
        }
        else
        {
            --aIt;

            if ( !bSibling )
                pResult = (*aIt)->GetLastDescendant();
            else
                pResult = (*aIt);

            if (! pResult)
                pResult = (*aIt);
        }
    }

    return pResult;
}

void SwNumberTreeNode::SetLastValid
                    ( const SwNumberTreeNode::tSwNumberTreeChildren::const_iterator& aItValid,
                      bool bValidating ) const
{
    OSL_ENSURE( (aItValid == mChildren.end() || GetIterator(*aItValid) != mChildren.end()),
            "last-valid iterator");

    if (
        bValidating ||
        aItValid == mChildren.end() ||
         (mItLastValid != mChildren.end() &&
          (*aItValid)->LessThan(**mItLastValid))
        )
    {
        mItLastValid = aItValid;
        // invalidation of children of next not counted is needed
        if ( GetParent() )
        {
            tSwNumberTreeChildren::const_iterator aParentChildIt =
                                            GetParent()->GetIterator( this );
            ++aParentChildIt;
            if ( aParentChildIt != GetParent()->mChildren.end() )
            {
                SwNumberTreeNode* pNextNode( *aParentChildIt );
                if ( !pNextNode->IsCounted() )
                {
                    pNextNode->InvalidateChildren();
                }
            }
        }
    }

    {
        if (IsContinuous())
        {
            tSwNumberTreeChildren::const_iterator aIt = mItLastValid;

            if (aIt != mChildren.end())
                ++aIt;
            else
                aIt = mChildren.begin();

            while (aIt != mChildren.end())
            {
                (*aIt)->InvalidateTree();

                ++aIt;
            }

            if (mpParent)
            {
                mpParent->SetLastValid(mpParent->GetIterator(this), bValidating);
            }
        }
    }
}

void SwNumberTreeNode::InvalidateTree() const
{
    // do not call SetInvalid, would cause loop !!!
    mItLastValid = mChildren.end();

    for (const auto& rpChild : mChildren)
        rpChild->InvalidateTree();
}

void SwNumberTreeNode::Invalidate(SwNumberTreeNode const * pChild)
{
    if (pChild->IsValid())
    {
        tSwNumberTreeChildren::const_iterator aIt = GetIterator(pChild);

        if (aIt != mChildren.begin())
            --aIt;
        else
            aIt = mChildren.end();

        SetLastValid(aIt);

    }
}

void SwNumberTreeNode::InvalidateMe()
{
    if (mpParent)
        mpParent->Invalidate(this);
}

void SwNumberTreeNode::ValidateMe()
{
    if (mpParent)
        mpParent->Validate(this);
}

void SwNumberTreeNode::Notify()
{
    if (IsNotifiable())
    {
        if (! IsPhantom())
            NotifyNode();

        for (auto& rpChild : mChildren)
            rpChild->Notify();
    }
}

void SwNumberTreeNode::NotifyInvalidChildren()
{
    if (IsNotifiable())
    {
        tSwNumberTreeChildren::const_iterator aIt = mItLastValid;

        if (aIt == mChildren.end())
            aIt = mChildren.begin();
        else
            ++aIt;

        while (aIt != mChildren.end())
        {
            (*aIt)->Notify();

            ++aIt;
        }
        // notification of next not counted node is also needed.
        if ( GetParent() )
        {
            tSwNumberTreeChildren::const_iterator aParentChildIt =
                                            GetParent()->GetIterator( this );
            ++aParentChildIt;
            if ( aParentChildIt != GetParent()->mChildren.end() )
            {
                SwNumberTreeNode* pNextNode( *aParentChildIt );
                if ( !pNextNode->IsCounted() )
                {
                    pNextNode->NotifyInvalidChildren();
                }
            }
        }

    }

    if (IsContinuous() && mpParent)
        mpParent->NotifyInvalidChildren();
}

void SwNumberTreeNode::NotifyInvalidSiblings()
{
    if (mpParent != nullptr)
        mpParent->NotifyInvalidChildren();
}

// #i81002#
const SwNumberTreeNode* SwNumberTreeNode::GetPrecedingNodeOf(
                                        const SwNumberTreeNode& rNode ) const
{
    const SwNumberTreeNode* pPrecedingNode( nullptr );

    if ( GetChildCount() > 0 )
    {
        tSwNumberTreeChildren::const_iterator aUpperBoundIt =
                mChildren.upper_bound( const_cast<SwNumberTreeNode*>(&rNode) );
        if ( aUpperBoundIt != mChildren.begin() )
        {
            --aUpperBoundIt;
            pPrecedingNode = (*aUpperBoundIt)->GetPrecedingNodeOf( rNode );
        }
    }

    if ( pPrecedingNode == nullptr && GetRoot() )
    {
        // <this> node has no children or the given node precedes all its children
        // and the <this> node isn't the root node.
        // Thus, compare the given node with the <this> node in order to check,
        // if the <this> node precedes the given node.
        if ( !(rNode.LessThan( *this )) )
        {
            pPrecedingNode = this;
        }
    }

    return pPrecedingNode;
}

void SwNumberTreeNode::NotifyNodesOnListLevel( const int nListLevel )
{
    if ( nListLevel < 0 )
    {
        OSL_FAIL( "<SwNumberTreeNode::NotifyNodesOnListLevel(..)> - invalid list level provided" );
        return;
    }

    SwNumberTreeNode* pRootNode = GetParent() ? GetRoot() : this;

    pRootNode->NotifyChildrenOnDepth( nListLevel );
}

void SwNumberTreeNode::NotifyChildrenOnDepth( const int nDepth )
{
    OSL_ENSURE( nDepth >= 0,
            "<SwNumberTreeNode::NotifyChildrenOnDepth(..)> - misusage" );

    for ( const auto& rpChild : mChildren )
    {
        if ( nDepth == 0 )
        {
            rpChild->NotifyNode();
        }
        else
        {
            rpChild->NotifyChildrenOnDepth( nDepth - 1 );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
