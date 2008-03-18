/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwNumberTree.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 15:54:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <algorithm>
#include <functional>
#include <errhdl.hxx>
#include <SwNumberTree.hxx>

using std::vector;
using std::find;

#ifndef PRODUCT
unsigned long SwNumberTreeNode::nInstances = 0;
#endif

SwNumberTreeNode::SwNumberTreeNode()
    : mpParent(NULL), mnNumber(0), mbPhantom(false)
{
    mItLastValid = mChildren.end();

#ifndef PRODUCT
    mnSerial = nInstances;
    nInstances++;
#endif
}

SwNumberTreeNode::SwNumberTreeNode(const SwNumberTreeNode & )
    : mpParent(NULL), mnNumber(0),
      mbPhantom(false), mItLastValid(mChildren.end())
{
#ifndef PRODUCT
    mnSerial = nInstances;
    nInstances++;
#endif
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
            ASSERT(false, "lost children!");
        }
    }

    ASSERT( IsPhantom() || mpParent == NULL, ": I'm not supposed to have a parent.");

#ifndef PRODUCT
    nInstances--;
#endif

    mpParent = (SwNumberTreeNode *) 0xdeadbeef;

    ASSERT(mChildren.empty(), "children left!");
}

SwNumberTreeNode * SwNumberTreeNode::CreatePhantom()
{
    SwNumberTreeNode * pNew = NULL;

    if (! mChildren.empty() &&
        (*mChildren.begin())->IsPhantom())
    {
        ASSERT(false, "phantom already present");
    }
    else
    {
        pNew = Create();

        pNew->SetPhantom(true);
        pNew->mpParent = this;

        std::pair<tSwNumberTreeChildren::iterator, bool> aInsert =
            mChildren.insert(pNew);

        if (! aInsert.second)
        {
            ASSERT(false, "insert of phantom failed!");

            delete pNew;
            pNew = NULL;
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

SwNumberTreeNode * SwNumberTreeNode::GetFirstChild() const
{
    SwNumberTreeNode * pResult = 0;

    tSwNumberTreeChildren::iterator aIt = mChildren.begin();

    if (aIt != mChildren.end() )
        pResult = *aIt;

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
            // --> OD 2006-01-17 #i60652#
            // Because <mChildren.erase(aIt)> could destroy the element, which
            // is referenced by <mItLastValid>, it's needed to adjust
            // <mItLastValid> before erasing <aIt>.
            SetLastValid(mChildren.end());
            // <--

            delete *aIt;
            mChildren.erase(aIt);
        }
    }
}

void SwNumberTreeNode::ValidateHierarchical(const SwNumberTreeNode * pNode) const
{
    tSwNumberTreeChildren::iterator aValidateIt =
        GetIterator(pNode);

    if (aValidateIt != mChildren.end())
    {
        ASSERT((*aValidateIt)->mpParent == this, "wrong parent");

        tSwNumberTreeChildren::iterator aIt = mItLastValid;

        // --> OD 2005-10-19 #126009#
        // improvement:
        // - Only one time checked for <mChildren.end()>.
        // - Less checks for each loop run.
        // correction:
        // - consider case that current node isn't counted and isn't the first
        // child of its parent. In this case the number of last counted child
        // of the previous node determines the start value for the following
        // children loop, if all children have to be validated and the first
        // one doesn't restart the counting.
//        tSwNumTreeNumber nTmpNumber = 0;
//        if (aIt != mChildren.end())
//            nTmpNumber = (*aIt)->mnNumber;
//        while (aIt != aValidateIt)
//        {
//            if (aIt == mChildren.end())
//                aIt = mChildren.begin();
//            else
//            {
//                aIt++;
//                if ((*aIt)->IsCounted())
//                    nTmpNumber++;
//            }
//            if ((*aIt)->IsRestart() || aIt == mChildren.begin())
//                nTmpNumber = (*aIt)->GetStart();
//            (*aIt)->mnNumber = nTmpNumber;
//        }
        tSwNumTreeNumber nTmpNumber( 0 );
        if (aIt != mChildren.end())
            nTmpNumber = (*aIt)->mnNumber;
        else
        {
            aIt = mChildren.begin();
            // determine default start value
            // consider the case that the first child isn't counted.
            nTmpNumber = (*aIt)->GetStart();
            if ( !(*aIt)->IsCounted() &&
                 ( !(*aIt)->HasCountedChildren() || (*aIt)->IsPhantom() ) )
            {
                --nTmpNumber;
            }

            // determine special start value for the case that first child
            // doesn't restart the numbering and the parent node isn't counted
            // and isn't the first child.
            // --> OD 2005-10-27 #126009#
            const bool bParentCounted( IsCounted() &&
                                       ( !IsPhantom() ||
                                         HasPhantomCountedParent() ) );
            // <--
            if ( !(*aIt)->IsRestart() &&
                 GetParent() && !bParentCounted )
            {
                tSwNumberTreeChildren::iterator aParentChildIt =
                                                GetParent()->GetIterator( this );
                while ( aParentChildIt != GetParent()->mChildren.begin() )
                {
                    --aParentChildIt;
                    SwNumberTreeNode* pPrevNode( *aParentChildIt );
                    if ( pPrevNode->GetChildCount() > 0 )
                    {
                        nTmpNumber = (*(pPrevNode->mChildren.rbegin()))->GetNumber();
                        // --> OD 2005-10-27 #126009#
                        if ( (*aIt)->IsCounted() &&
                             ( !(*aIt)->IsPhantom() ||
                               (*aIt)->HasPhantomCountedParent() ) )
                        // <--
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

            // --> OD 2005-10-19 #126009# - only for counted nodes the number
            // has to be adjusted, compared to the previous node.
            // this condition is hold also for nodes, which restart the numbering.
            if ( (*aIt)->IsCounted() )
            {
                if ((*aIt)->IsRestart())
                    nTmpNumber = (*aIt)->GetStart();
                else
                    ++nTmpNumber;
            }
            // <--

            (*aIt)->mnNumber = nTmpNumber;
        }
        // <--

        SetLastValid(aIt, true);
    }
}

void SwNumberTreeNode::ValidateContinuous(const SwNumberTreeNode * pNode) const
{
    tSwNumberTreeChildren::iterator aIt = mItLastValid;

    tSwNumTreeNumber nTmpNumber = 0;

    do
    {
        if (aIt == mChildren.end())
        {
            aIt = mChildren.begin();

            nTmpNumber = GetStart();
        }
        else
            aIt++;

        if (aIt != mChildren.end())
        {
            SwNumberTreeNode * pPred = (*aIt)->GetPred();

            // --> OD 2006-04-21 #i64311#
            // correct consideration of phantoms
            // correct consideration of restart at a number tree node
            if ( pPred )
            {
                if ( !(*aIt)->IsCounted() )
                    // --> OD 2006-05-12 #i65284#
                    nTmpNumber = pPred->GetNumber( pPred->GetParent() != (*aIt)->GetParent() );
                    // <--
                else
                {
                    if ( (*aIt)->IsRestart() )
                        nTmpNumber = (*aIt)->GetStart();
                    else
                        nTmpNumber = pPred->GetNumber( pPred->GetParent() != (*aIt)->GetParent() ) + 1;
                }
            }
            else
            {
                if ( !(*aIt)->IsCounted() )
                    nTmpNumber = GetStart() - 1;
                else
                {
                    if ( (*aIt)->IsRestart() )
                        nTmpNumber = (*aIt)->GetStart();
                    else
                       nTmpNumber = GetStart();
                }
            }
            // <--

            (*aIt)->mnNumber = nTmpNumber;
        }
    }
    while (aIt != mChildren.end() && *aIt != pNode);

    SetLastValid(aIt);
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

void SwNumberTreeNode::ValidateTree()
{
    if (! IsContinuous())
    {
        {
            tSwNumberTreeChildren::reverse_iterator aIt = mChildren.rbegin();

            if (aIt != mChildren.rend())
                Validate(*aIt);
        }
        {
            tSwNumberTreeChildren::iterator aIt;

            for (aIt = mChildren.begin(); aIt != mChildren.end(); aIt++)
                (*aIt)->ValidateTree();
        }
    }
    else
    {
        SwNumberTreeNode * pNode = GetLastDescendant();

        if (pNode && pNode->mpParent)
            pNode->mpParent->Validate(pNode);
    }
}

void SwNumberTreeNode::_GetNumberVector(vector<tSwNumTreeNumber> & rVector,
                                        bool bValidate) const
{
    if (mpParent)
    {
        mpParent->_GetNumberVector(rVector, bValidate);
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

    OD 2005-10-14 #125991#
*/
void SwNumberTreeNode::MoveGreaterChildren( SwNumberTreeNode& _rCompareNode,
                                            SwNumberTreeNode& _rDestNode )
{
    if ( mChildren.size() == 0 )
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
        for (aIt = aItUpper; aIt != mChildren.end(); aIt++)
            (*aIt)->mpParent = &_rDestNode;

        _rDestNode.mChildren.insert(aItUpper, mChildren.end());

        // --> OD 2006-01-17 #i60652#
        // Because <mChildren.erase(aItUpper, mChildren.end())> could destroy
        // the element, which is referenced by <mItLastValid>, it's needed to
        // adjust <mItLastValid> before erasing <aIt>.
        SetLastValid( mChildren.end() );
        // <--

        mChildren.erase(aItUpper, mChildren.end());

        // --> OD 2006-01-17 #i60652#
        if ( !mChildren.empty() )
        {
            SetLastValid( --(mChildren.end()) );
        }
        // <--
    }

#ifdef __SW_NUMBER_TREE_SANITY_CHECK
    if (! IsSane(false) || ! IsSane(&_rDestNode))
        clog << __FUNCTION__ << "insanity!" << endl;
#endif
}

void SwNumberTreeNode::MoveChildren(SwNumberTreeNode * pDest)
{
    if (! mChildren.empty())
    {
        tSwNumberTreeChildren::iterator aItBegin = mChildren.begin();
        SwNumberTreeNode * pMyFirst = *mChildren.begin();

        // --> OD 2006-01-17 #i60652#
        // Because <mChildren.erase(aItBegin)> could destroy the element,
        // which is referenced by <mItLastValid>, it's needed to adjust
        // <mItLastValid> before erasing <aItBegin>.
        SetLastValid(mChildren.end());
        // <--

        if (pMyFirst->IsPhantom())
        {
            SwNumberTreeNode * pDestLast = NULL;

            if (pDest->mChildren.empty())
                pDestLast = pDest->CreatePhantom();
            else
                pDestLast = *pDest->mChildren.rbegin();

            pMyFirst->MoveChildren(pDestLast);

            delete pMyFirst;
            mChildren.erase(aItBegin);

            aItBegin = mChildren.begin();
        }

        tSwNumberTreeChildren::iterator aIt;
        for (aIt = mChildren.begin(); aIt != mChildren.end(); aIt++)
            (*aIt)->mpParent = pDest;

        pDest->mChildren.insert(mChildren.begin(), mChildren.end());
        mChildren.clear();
        // --> OD 2006-03-08 #131436#
        // <stl::set.clear()> destroys all existing iterators.
        // Thus, <mItLastValid> is also destroyed and reset becomes necessary
        mItLastValid = mChildren.end();
        // <--
    }

    ASSERT (mChildren.empty(), "MoveChildren failed!");

#ifdef __SW_NUMBER_TREE_SANITY_CHECK
    ASSERT(IsSane(false) && pDest->IsSane(false), "insanity!");
#endif
}

void SwNumberTreeNode::AddChild(SwNumberTreeNode * pChild, unsigned int nDepth)
{
    /*
       Algorithm:

       Search first child A that is greater than pChild,
         A may be the end of childs.
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

    if (pChild->GetParent() != NULL || pChild->GetChildCount() > 0)
    {
        ASSERT(false, "only orphans allowed.");

        return;
    }

    if (nDepth > 0)
    {
        tSwNumberTreeChildren::iterator aInsertDeepIt =
            mChildren.upper_bound(pChild);

        ASSERT(! (aInsertDeepIt != mChildren.end() &&
                  (*aInsertDeepIt)->IsPhantom()), " unexspected phantom");


        if (aInsertDeepIt == mChildren.begin())
        {
            SwNumberTreeNode * pNew = CreatePhantom();

            SetLastValid(mChildren.end());

            if (pNew)
                pNew->AddChild(pChild, nDepth - 1);
        }
        else
        {
            aInsertDeepIt--;
            (*aInsertDeepIt)->AddChild(pChild, nDepth - 1);
        }

    }
    else
    {
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
                aPredIt--;

                // --> OD 2005-10-14 #125991#
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
                // <--

                if ((*aPredIt)->IsValid())
                    SetLastValid(aPredIt);
            }
            else
                SetLastValid(mChildren.end());

            ClearObsoletePhantoms();

            if( bNotification )
            {
                // --> OD 2005-10-20 #126009# - invalidation of not counted parent
                // and notification of its siblings.
                if ( !IsCounted() )
                {
                    InvalidateMe();
                    NotifyInvalidSiblings();
                }
                // <--
                NotifyInvalidChildren();
            }
        }
    }

#ifdef __SW_NUMBER_TREE_SANITY_CHECK
    if (! IsSane(false))
        clog << __FUNCTION__ << ": insanity!" << endl;
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
        ASSERT(false, "not applicable to phantoms!");

        return;
    }

    tSwNumberTreeChildren::iterator aRemoveIt = GetIterator(pChild);

    if (aRemoveIt != mChildren.end())
    {
        SwNumberTreeNode * pRemove = *aRemoveIt;

        pRemove->mpParent = NULL;

        tSwNumberTreeChildren::iterator aItPred = mChildren.end();

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

            aItPred--;
        }

        if (! pRemove->mChildren.empty())
        {
            pRemove->MoveChildren(*aItPred);
        }

        // --> OD 2006-01-17 #i60652#
        // Because <mChildren.erase(aRemoveIt)> could destroy the element,
        // which is referenced by <mItLastValid>, it's needed to adjust
        // <mItLastValid> before erasing <aRemoveIt>.
        if (aItPred != mChildren.end() && (*aItPred)->IsPhantom())
            SetLastValid(mChildren.end());
        else
            SetLastValid(aItPred);
        // <--

        mChildren.erase(aRemoveIt);

        if (aItPred != mChildren.end())
            NotifyInvalidChildren();
    }
    else
    {
        ASSERT(false, "RemoveChild: failed!");
    }
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

#ifdef __SW_NUMBER_TREE_SANITY_CHECK
        if (! IsSane(false))
            clog << __FUNCTION__ << ": insanity!" << endl;
#endif
    }
}

bool SwNumberTreeNode::IsValid() const
{
    return mpParent ? mpParent->IsValid(this) : false;
}

SwNumberTreeNode::tSwNumTreeNumber SwNumberTreeNode::GetNumber(bool bValidate)
    const
{
    if (bValidate && mpParent)
        mpParent->Validate(this);

    return mnNumber;
}

vector<SwNumberTreeNode::tSwNumTreeNumber>
SwNumberTreeNode::GetNumberVector() const
{
    vector<tSwNumTreeNumber> aResult;

    _GetNumberVector(aResult);

    return aResult;
}

bool SwNumberTreeNode::IsRestart() const
{
    return false;
}

SwNumberTreeNode::tSwNumTreeNumber SwNumberTreeNode::GetStart() const
{
    return 1;
}

bool SwNumberTreeNode::IsCountPhantoms() const
{
    ASSERT( false,
            "<SwNumberTreeNode::IsCountPhantoms()> should not be called - this is a serious defect - please inform OD" );
    return true;
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

bool SwNumberTreeNode::IsPhantom() const
{
    return mbPhantom;
}

void SwNumberTreeNode::SetPhantom(bool _bPhantom)
{
    mbPhantom = _bPhantom;
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

// --> OD 2005-10-27 #126009#
bool SwNumberTreeNode::HasPhantomCountedParent() const
{
    bool bRet( false );

    ASSERT( IsPhantom(),
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

// <--
bool SwNumberTreeNode::IsContinuous() const
{
    ASSERT( false,
            "<SwNumberTreeNode::IsContinuous()> should not be called - this is a serious defect - please inform OD" );
    return false;
}

bool SwNumberTreeNode::IsNotifiable() const
{
    return false;
}

bool SwNumberTreeNode::IsFirst(const SwNumberTreeNode * pNode) const
{
    tSwNumberTreeChildren::iterator aIt = mChildren.begin();

    if ((*aIt)->IsPhantom())
        aIt++;

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

            // --> OD 2007-10-02 #b6600435#
            // If node isn't the first child, it is the second child and the
            // first child is a phanton. In this case check, if the first phantom
            // child have only phanton childs
            if ( bResult &&
                 this != *(GetParent()->mChildren.begin()) &&
                 !(*(GetParent()->mChildren.begin()))->HasOnlyPhantoms() )
            {
                bResult = false;
            }
            // <--
        }
        else
            bResult = false;
    }

    return bResult;
}

int SwNumberTreeNode::GetLevel() const
{
    if (mpParent)
        return mpParent->GetLevel() + 1;

    return -1;
}

SwNumberTreeNode::tSwNumberTreeChildren::size_type
SwNumberTreeNode::GetChildCount() const
{
    return mChildren.size();
}

#ifdef __SW_NUMBER_TREE_SANITY_CHECK
bool SwNumberTreeNode::IsSane(bool bRecursive) const
{
    vector<const SwNumberTreeNode*> aParents;

    return IsSane(bRecursive, aParents);
}

bool SwNumberTreeNode::IsSane(bool bRecursive,
                              vector<const SwNumberTreeNode *> rParents)
    const
{
    bool bResult = true;

    tSwNumberTreeChildren::const_iterator aIt;

    if (find(rParents.begin(), rParents.end(), this) != rParents.end())
    {
        ASSERT(false, " I'm my own ancestor!");

        bResult = false;
    }

    if (! rParents.empty() && rParents.back() != mpParent)
    {
        ASSERT(false, " I'm a bastard!");

        bResult = false;
    }

    rParents.push_back(this);

    bool bFirst = true;
    for (aIt = mChildren.begin(); aIt != mChildren.end(); aIt++)
    {
        if (*aIt)
        {
            if ((*aIt)->IsPhantom())
            {
                if ((*aIt)->HasOnlyPhantoms())
                {
                    bResult = false;
                }

                if (! bFirst)
                {
                    ASSERT(false, " found phantom not at first position.");

                    bResult = false;
                }
            }

            if ((*aIt)->mpParent != (SwNumberTreeNode *) this)
            {
                ASSERT(false, "found a bastard");

                bResult = false;
            }

            if (mpParent)
            {
                if  (!(*aIt)->IsPhantom() && (*aIt)->LessThan(*this))
                {
                    ASSERT(false, " found child less than me");

                    bResult = false;
                }
            }
        }
        else
        {
            ASSERT(false, "found child that is NULL");
            bResult = false;
        }

    if (bRecursive)
      bResult = (*aIt)->IsSane(bRecursive, rParents) && bResult;
    }

    rParents.pop_back();

    return bResult;
}
#endif // __SW_NUMBER_TREE_SANITY_CHECK

SwNumberTreeNode::tSwNumberTreeChildren::iterator
SwNumberTreeNode::GetIterator(const SwNumberTreeNode * pChild) const
{
    tSwNumberTreeChildren::iterator aItResult =
        mChildren.find(const_cast<SwNumberTreeNode *>(pChild));

    ASSERT( aItResult != mChildren.end(),
            "something went wrong getting the iterator for a child");

    return aItResult;
}

String SwNumberTreeNode::print(const String & rIndent,
                               const String & rMyIndent,
                               int nDepth) const
{
  String aStr = rIndent;
  aStr += ToString();
  aStr += String("\n", RTL_TEXTENCODING_ASCII_US);

  if (nDepth != 0)
  {
      if (nDepth < 0)
          nDepth = -1;

      tSwNumberTreeChildren::const_iterator aIt;
      for (aIt = mChildren.begin(); aIt != mChildren.end(); aIt++)
      {
          String aTmpStr(rIndent);

          aTmpStr += rMyIndent;
          aStr += (*aIt)->print(aTmpStr, rMyIndent, nDepth - 1);
      }
  }

  return aStr;
}

unsigned long SwNumberTreeNode::GetCount(bool bCountPhantoms) const
{
    unsigned long aResult = 0;

    tSwNumberTreeChildren::const_iterator aIt;

    for (aIt = mChildren.begin(); aIt != mChildren.end(); aIt++)
    {
        if ( bCountPhantoms || ! (*aIt)->IsPhantom())
            aResult++;

        aResult += (*aIt)->GetCount(bCountPhantoms);
    }

    return aResult;
}

#ifndef PRODUCT
unsigned long SwNumberTreeNode::GetInstances()
{
    return nInstances;
}

unsigned long SwNumberTreeNode::GetSerial()
{
    return mnSerial;
}
#endif

bool SwNumberTreeNodeLessThan(const SwNumberTreeNode * pA,
                              const SwNumberTreeNode * pB)
{
  bool bResult = false;

  if (pA == NULL && pB != NULL)
    bResult = true;
  else if (pA != NULL && pB != NULL)
    bResult = pA->LessThan(*pB);

  return bResult;
}

SwNumberTreeNode * SwNumberTreeNode::GetLastDescendant() const
{
    SwNumberTreeNode * pResult = NULL;
    tSwNumberTreeChildren::reverse_iterator aIt = mChildren.rbegin();

    if (aIt != mChildren.rend())
    {
        pResult = (*aIt)->GetLastDescendant();

        if (! pResult)
            pResult = *aIt;
    }

    return pResult;
}

SwNumberTreeNode * SwNumberTreeNode::GetPred() const
{
    SwNumberTreeNode * pResult = NULL;

    if (mpParent)
    {
        tSwNumberTreeChildren::iterator aIt =
            mpParent->GetIterator(this);

        if ( aIt == mpParent->mChildren.begin() )
        {
            // --> OD 2006-04-24 #i64311#
            // root node is no valid predecessor
            pResult = mpParent->GetParent() ? mpParent : NULL;
            // <--
        }
        else
        {
            aIt--;

            pResult = (*aIt)->GetLastDescendant();

            if (! pResult)
                pResult = (*aIt);
        }
    }

    return pResult;
}

void SwNumberTreeNode::SetLastValid
                    ( SwNumberTreeNode::tSwNumberTreeChildren::iterator aItValid,
                      bool bValidating ) const
{
    ASSERT( (aItValid == mChildren.end() || GetIterator(*aItValid) != mChildren.end()),
            "last-valid iterator");

    if (
        bValidating ||
        aItValid == mChildren.end() ||
         (mItLastValid != mChildren.end() &&
          (*aItValid)->LessThan(**mItLastValid))
        )
    {
        mItLastValid = aItValid;
        // --> OD 2005-10-19 #126009# - invalidation of children of next not
        // counted is needed
        if ( GetParent() )
        {
            tSwNumberTreeChildren::iterator aParentChildIt =
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
        // <--
    }

    {
        if (IsContinuous())
        {
            tSwNumberTreeChildren::iterator aIt = mItLastValid;

            if (aIt != mChildren.end())
                aIt++;
            else
                aIt = mChildren.begin();

            while (aIt != mChildren.end())
            {
                (*aIt)->InvalidateTree();

                aIt++;
            }

            SetLastValid(bValidating);
        }
    }
}

void SwNumberTreeNode::SetLastValid(bool bValidating) const
{
    if (mpParent)
    {
        tSwNumberTreeChildren::iterator aIt = mpParent->GetIterator(this);

        mpParent->SetLastValid(aIt, bValidating);
    }
}

void SwNumberTreeNode::InvalidateTree() const
{
    // do not call SetInvalid, would cause loop !!!
    mItLastValid = mChildren.end();

    tSwNumberTreeChildren::iterator aIt;

    for (aIt = mChildren.begin(); aIt != mChildren.end(); aIt++)
        (*aIt)->InvalidateTree();
}

void SwNumberTreeNode::Invalidate(SwNumberTreeNode * pChild)
{
    if (pChild->IsValid())
    {
        tSwNumberTreeChildren::iterator aIt = GetIterator(pChild);

        if (aIt != mChildren.begin())
            aIt--;
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

void SwNumberTreeNode::NotifyNode()
{
}

void SwNumberTreeNode::Notify()
{
    if (IsNotifiable())
    {
        if (! IsPhantom())
            NotifyNode();

        tSwNumberTreeChildren::iterator aIt;

        for (aIt = mChildren.begin(); aIt != mChildren.end(); aIt++)
            (*aIt)->Notify();
    }
}

void SwNumberTreeNode::NotifyInvalidChildren()
{
    if (IsNotifiable())
    {
        tSwNumberTreeChildren::iterator aIt = mItLastValid;

        if (aIt == mChildren.end())
            aIt = mChildren.begin();
        else
            aIt++;

        while (aIt != mChildren.end())
        {
            (*aIt)->Notify();

            aIt++;
        }
        // --> OD 2005-10-19 #126009# - notification of next not counted node
        // is also needed.
        if ( GetParent() )
        {
            tSwNumberTreeChildren::iterator aParentChildIt =
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

        // <--
    }

    if (IsContinuous() && mpParent)
        mpParent->NotifyInvalidChildren();
}

void SwNumberTreeNode::NotifyInvalidSiblings()
{
    if (mpParent != NULL)
        mpParent->NotifyInvalidChildren();
}

// --> OD 2007-09-07 #i81002#
const SwNumberTreeNode* SwNumberTreeNode::GetPrecedingNodeOf(
                                        const SwNumberTreeNode& rNode ) const
{
    const SwNumberTreeNode* pPrecedingNode( 0 );

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

    if ( pPrecedingNode == 0 && GetRoot() )
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
// <--
