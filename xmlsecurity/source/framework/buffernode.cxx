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


#include "elementmark.hxx"
#include "elementcollector.hxx"
#include "buffernode.hxx"
#include <com/sun/star/xml/crypto/sax/ConstOfSecurityId.hpp>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>

BufferNode::BufferNode( const css::uno::Reference< css::xml::wrapper::XXMLElementWrapper >& xXMLElement )
    :m_pParent(nullptr),
     m_pBlocker(nullptr),
     m_bAllReceived(false),
     m_xXMLElement(xXMLElement)
{
}

bool BufferNode::isECOfBeforeModifyIncluded(sal_Int32 nIgnoredSecurityId) const
/****** BufferNode/isECOfBeforeModifyIncluded ********************************
 *
 *   NAME
 *  isECOfBeforeModifyIncluded -- checks whether there is some
 *  ElementCollector on this BufferNode, that has BEFORE-MODIFY priority.
 *
 *   SYNOPSIS
 *  bExist = isECOfBeforeModifyIncluded(nIgnoredSecurityId);
 *
 *   FUNCTION
 *  checks each ElementCollector on this BufferNode, if all following
 *  conditions are satisfied, then returns true:
 *  1. the ElementCollector's priority is BEFOREMODIFY;
 *  2. the ElementCollector's securityId can't be ignored.
 *  otherwise, returns false.
 *
 *   INPUTS
 *  nIgnoredSecurityId -    the security Id to be ignored. If it equals
 *                          to UNDEFINEDSECURITYID, then no security Id
 *                          will be ignored.
 *
 *   RESULT
 *  bExist - true if a match found, false otherwise
 ******************************************************************************/
{
    return std::any_of(m_vElementCollectors.cbegin(), m_vElementCollectors.cend(),
        [nIgnoredSecurityId](const ElementCollector* pElementCollector) {
            return (nIgnoredSecurityId == css::xml::crypto::sax::ConstOfSecurityId::UNDEFINEDSECURITYID ||
                    pElementCollector->getSecurityId() != nIgnoredSecurityId) &&
                   (pElementCollector->getPriority() == css::xml::crypto::sax::ElementMarkPriority_BEFOREMODIFY);
        });
}

void BufferNode::setReceivedAll()
/****** BufferNode/setReceiveAll *********************************************
 *
 *   NAME
 *  setReceivedAll -- indicates that the element in this BufferNode has
 *  been completely buffered.
 *
 *   SYNOPSIS
 *  setReceivedAll();
 *
 *   FUNCTION
 *  sets the all-received flag and launches ElementCollector's notify
 *  process.
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  empty
 ******************************************************************************/
{
    m_bAllReceived = true;
    elementCollectorNotify();
}


void BufferNode::addElementCollector(const ElementCollector* pElementCollector)
/****** BufferNode/addElementCollector ***************************************
 *
 *   NAME
 *  addElementCollector -- adds a new ElementCollector to this BufferNode.
 *
 *   SYNOPSIS
 *  addElementCollector(pElementCollector);
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  pElementCollector - the ElementCollector to be added
 *
 *   RESULT
 *  empty
 ******************************************************************************/
{
    m_vElementCollectors.push_back( pElementCollector );
    const_cast<ElementCollector*>(pElementCollector)->setBufferNode(this);
}

void BufferNode::removeElementCollector(const ElementCollector* pElementCollector)
/****** BufferNode/removeElementCollector ************************************
 *
 *   NAME
 *  removeElementCollector -- removes an ElementCollector from this
 *  BufferNode.
 *
 *   SYNOPSIS
 *  removeElementCollector(pElementCollector);
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  pElementCollector - the ElementCollector to be removed
 *
 *   RESULT
 *  empty
 ******************************************************************************/
{
    auto ii = std::find(m_vElementCollectors.begin(), m_vElementCollectors.end(), pElementCollector);
    if (ii != m_vElementCollectors.end())
    {
        m_vElementCollectors.erase( ii );
        const_cast<ElementCollector*>(pElementCollector)->setBufferNode(nullptr);
    }
}


void BufferNode::setBlocker(const ElementMark* pBlocker)
/****** BufferNode/setBlocker ************************************************
 *
 *   NAME
 *  setBlocker -- adds a blocker to this BufferNode.
 *
 *   SYNOPSIS
 *  setBlocker(pBlocker);
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  pBlocker - the new blocker to be attached
 *
 *   RESULT
 *  empty
 *
 *   NOTES
 *  Because there is only one blocker permitted for a BufferNode, so the
 *  old blocker on this BufferNode, if there is one, will be overcasted.
 ******************************************************************************/
{
    OSL_ASSERT(!(m_pBlocker != nullptr && pBlocker != nullptr));

    m_pBlocker = const_cast<ElementMark*>(pBlocker);
    if (m_pBlocker != nullptr)
    {
        m_pBlocker->setBufferNode(this);
    }
}

OUString BufferNode::printChildren() const
/****** BufferNode/printChildren *********************************************
 *
 *   NAME
 *  printChildren -- prints children information into a string.
 *
 *   SYNOPSIS
 *  result = printChildren();
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  result - the information string
 ******************************************************************************/
{
    OUStringBuffer rc;

    for( const ElementCollector* ii : m_vElementCollectors )
    {
        rc.append("BufID=" + OUString::number(ii->getBufferId()));

        if (ii->getModify())
        {
            rc.append("[M]");
        }

        rc.append(",Pri=");

        switch (ii->getPriority())
        {
            case css::xml::crypto::sax::ElementMarkPriority_BEFOREMODIFY:
                rc.append("BEFOREMODIFY");
                break;
            case css::xml::crypto::sax::ElementMarkPriority_AFTERMODIFY:
                rc.append("AFTERMODIFY");
                break;
            default:
                rc.append("UNKNOWN");
                break;
        }

        rc.append("(SecID=" + OUString::number(ii->getSecurityId()) + ") ");
    }

    return rc.makeStringAndClear();
}

bool BufferNode::hasAnything() const
/****** BufferNode/hasAnything ***********************************************
 *
 *   NAME
 *  hasAnything -- checks whether there is any ElementCollector or blocker
 *  on this BufferNode.
 *
 *   SYNOPSIS
 *  bExist = hasAnything();
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  bExist - true if there is, false otherwise.
 ******************************************************************************/
{
    return (m_pBlocker || !m_vElementCollectors.empty());
}

bool BufferNode::hasChildren() const
/****** BufferNode/hasChildren ***********************************************
 *
 *   NAME
 *  hasChildren -- checks whether this BufferNode has any child
 *  BufferNode.
 *
 *   SYNOPSIS
 *  bExist = hasChildren();
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  bExist - true if there is, false otherwise.
 ******************************************************************************/
{
    return (!m_vChildren.empty());
}

std::vector< std::unique_ptr<BufferNode> > const & BufferNode::getChildren() const
{
    return m_vChildren;
}

std::vector< std::unique_ptr<BufferNode> > BufferNode::releaseChildren()
{
    return std::move(m_vChildren);
}

const BufferNode* BufferNode::getFirstChild() const
/****** BufferNode/getFirstChild *********************************************
 *
 *   NAME
 *  getFirstChild -- retrieves the first child BufferNode.
 *
 *   SYNOPSIS
 *  child = getFirstChild();
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  child - the first child BufferNode, or NULL if there is no child
 *          BufferNode.
 ******************************************************************************/
{
    BufferNode* rc = nullptr;

    if (!m_vChildren.empty())
    {
        rc = m_vChildren.front().get();
    }

    return rc;
}

void BufferNode::addChild(std::unique_ptr<BufferNode> pChild, sal_Int32 nPosition)
/****** BufferNode/addChild(pChild,nPosition) ********************************
 *
 *   NAME
 *  addChild -- inserts a child BufferNode at specific position.
 *
 *   SYNOPSIS
 *  addChild(pChild, nPosition);
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  pChild -    the child BufferNode to be added.
 *  nPosition - the position where the new child locates.
 *
 *   RESULT
 *  empty
 *
 *   NOTES
 *  If the nPosition is -1, then the new child BufferNode is appended
 *  at the end.
 ******************************************************************************/
{
    if (nPosition == -1)
    {
        m_vChildren.push_back( std::move(pChild) );
    }
    else
    {
        m_vChildren.insert(m_vChildren.begin() + nPosition, std::move(pChild));
    }
}

void BufferNode::addChild(std::unique_ptr<BufferNode> pChild)
/****** BufferNode/addChild() ************************************************
 *
 *   NAME
 *  addChild -- add a new child BufferNode.
 *
 *   SYNOPSIS
 *  addChild(pChild);
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  pChild -    the child BufferNode to be added.
 *
 *   RESULT
 *  empty
 *
 *   NOTES
 *  The new child BufferNode is appended at the end.
 ******************************************************************************/
{
    addChild(std::move(pChild), -1);
}

void BufferNode::removeChild(const BufferNode* pChild)
/****** BufferNode/removeChild ***********************************************
 *
 *   NAME
 *  removeChild -- removes and deletes a child BufferNode from the children list.
 *
 *   SYNOPSIS
 *  removeChild(pChild);
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  pChild - the child BufferNode to be removed
 *
 *   RESULT
 *  empty
 ******************************************************************************/
{
    auto ii = std::find_if(m_vChildren.begin(), m_vChildren.end(),
                [pChild] (const std::unique_ptr<BufferNode>& i)
                { return i.get() == pChild; });
    if (ii != m_vChildren.end())
        m_vChildren.erase( ii );
}

sal_Int32 BufferNode::indexOfChild(const BufferNode* pChild) const
/****** BufferNode/indexOfChild **********************************************
 *
 *   NAME
 *  indexOfChild -- gets the index of a child BufferNode.
 *
 *   SYNOPSIS
 *  index = indexOfChild(pChild);
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  pChild - the child BufferNode whose index to be gotten
 *
 *   RESULT
 *  index - the index of that child BufferNode. If that child BufferNode
 *          is not found, -1 is returned.
 ******************************************************************************/
{
    auto ii = std::find_if(m_vChildren.begin(), m_vChildren.end(),
            [pChild] (const std::unique_ptr<BufferNode>& i)
            { return i.get() == pChild; });
    if (ii == m_vChildren.end())
        return -1;

    return std::distance(m_vChildren.begin(), ii);
}


void BufferNode::setParent(const BufferNode* pParent)
{
    m_pParent = const_cast<BufferNode*>(pParent);
}

const BufferNode* BufferNode::getNextSibling() const
/****** BufferNode/getNextSibling ********************************************
 *
 *   NAME
 *  getNextSibling -- retrieves the next sibling BufferNode.
 *
 *   SYNOPSIS
 *  sibling = getNextSibling();
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  sibling - the next sibling BufferNode, or NULL if there is none.
 ******************************************************************************/
{
    BufferNode* rc = nullptr;

    if (m_pParent != nullptr)
    {
        rc = const_cast<BufferNode*>(m_pParent->getNextChild(this));
    }

    return rc;
}

const BufferNode* BufferNode::isAncestor(const BufferNode* pDescendant) const
/****** BufferNode/isAncestor ************************************************
 *
 *   NAME
 *  isAncestor -- checks whether this BufferNode is an ancestor of another
 *  BufferNode.
 *
 *   SYNOPSIS
 *  bIs = isAncestor(pDescendant);
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  pDescendant -   the BufferNode to be checked as a descendant
 *
 *   RESULT
 *  bIs -   true if this BufferNode is an ancestor of the pDescendant,
 *          false otherwise.
 ******************************************************************************/
{
    BufferNode* rc = nullptr;

    if (pDescendant != nullptr)
    {
        auto ii = std::find_if(m_vChildren.cbegin(), m_vChildren.cend(),
            [&pDescendant](const std::unique_ptr<BufferNode>& pChild) {
                return (pChild.get() == pDescendant) || (pChild->isAncestor(pDescendant) != nullptr);
            });

        if (ii != m_vChildren.end())
            rc = ii->get();
    }

    return rc;
}

bool BufferNode::isPrevious(const BufferNode* pFollowing) const
/****** BufferNode/isPrevious ************************************************
 *
 *   NAME
 *  isPrevious -- checks whether this BufferNode is ahead of another
 *  BufferNode in the tree order.
 *
 *   SYNOPSIS
 *  bIs = isPrevious(pFollowing);
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  pFollowing -    the BufferNode to be checked as a following
 *
 *   RESULT
 *  bIs -   true if this BufferNode is ahead in the tree order, false
 *          otherwise.
 ******************************************************************************/
{
    bool rc = false;

    BufferNode* pNextBufferNode = const_cast<BufferNode*>(getNextNodeByTreeOrder());
    while (pNextBufferNode != nullptr)
    {
        if (pNextBufferNode == pFollowing)
        {
            rc = true;
            break;
        }

        pNextBufferNode = const_cast<BufferNode*>(pNextBufferNode->getNextNodeByTreeOrder());
    }

    return rc;
}

const BufferNode* BufferNode::getNextNodeByTreeOrder() const
/****** BufferNode/getNextNodeByTreeOrder ************************************
 *
 *   NAME
 *  getNextNodeByTreeOrder -- retrieves the next BufferNode in the tree
 *  order.
 *
 *   SYNOPSIS
 *  next = getNextNodeByTreeOrder();
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  next -  the BufferNode following this BufferNode in the tree order,
 *          or NULL if there is none.
 *
 *   NOTES
 *  The "next" node in tree order is defined as:
 *  1. If a node has children, then the first child is;
 *  2. otherwise, if it has a following sibling, then this sibling node is;
 *  3. otherwise, if it has a parent node, the parent's next sibling
 *     node is;
 *  4. otherwise, no "next" node exists.
 ******************************************************************************/
{
    /*
     * If this buffer node has m_vChildren, then return the first
     * child.
     */
    if (hasChildren())
    {
        return getFirstChild();
    }

    /*
     * Otherwise, it this buffer node has a following sibling,
     * then return that sibling.
     */
    BufferNode* pNextSibling = const_cast<BufferNode*>(getNextSibling());
    if (pNextSibling != nullptr)
    {
        return pNextSibling;
    }

    /*
     * Otherwise, it this buffer node has parent, then return
     * its parent's following sibling.
     */
    BufferNode* pNode = const_cast<BufferNode*>(this);
    BufferNode* pParent;
    BufferNode* pNextSiblingParent = nullptr;

    do
    {
        if (pNode == nullptr)
        {
            break;
        }

        pParent = const_cast<BufferNode*>(pNode->getParent());
        if (pParent != nullptr)
        {
            pNextSiblingParent = const_cast<BufferNode*>(pParent->getNextSibling());
        }
        pNode = pParent;

    } while (pNextSiblingParent == nullptr);

    return pNextSiblingParent;
}


void BufferNode::setXMLElement( const css::uno::Reference< css::xml::wrapper::XXMLElementWrapper >& xXMLElement )
{
    m_xXMLElement = xXMLElement;
}

void BufferNode::notifyBranch()
/****** BufferNode/notifyBranch **********************************************
 *
 *   NAME
 *  notifyBranch -- notifies each BufferNode in the branch of this
 *  BufferNode in the tree order.
 *
 *   SYNOPSIS
 *  notifyBranch();
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  empty
 ******************************************************************************/
{
    for( std::unique_ptr<BufferNode>& pBufferNode : m_vChildren )
    {
        pBufferNode->elementCollectorNotify();
        pBufferNode->notifyBranch();
    }
}

void BufferNode::elementCollectorNotify()
/****** BufferNode/elementCollectorNotify ************************************
 *
 *   NAME
 *  elementCollectorNotify -- notifies this BufferNode.
 *
 *   SYNOPSIS
 *  elementCollectorNotify();
 *
 *   FUNCTION
 *  Notifies this BufferNode if the notification is not suppressed.
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  child - the first child BufferNode, or NULL if there is no child
 *          BufferNode.
 ******************************************************************************/
{
    if (m_vElementCollectors.empty())
        return;

    css::xml::crypto::sax::ElementMarkPriority nMaxPriority = css::xml::crypto::sax::ElementMarkPriority_MINIMUM;
    css::xml::crypto::sax::ElementMarkPriority nPriority;

    /*
     * get the max priority among ElementCollectors on this BufferNode
     */
    for( const ElementCollector* pElementCollector : m_vElementCollectors )
    {
        nPriority = pElementCollector->getPriority();
        if (nPriority > nMaxPriority)
        {
            nMaxPriority = nPriority;
        }
    }

    std::vector< const ElementCollector* > vElementCollectors( m_vElementCollectors );

    for( const ElementCollector* ii : vElementCollectors )
    {
        ElementCollector* pElementCollector = const_cast<ElementCollector*>(ii);
        nPriority = pElementCollector->getPriority();
        bool bToModify = pElementCollector->getModify();

        /*
         * Only ElementCollector with the max priority can
         * perform notify operation.
         * Moreover, if any blocker exists in the subtree of
         * this BufferNode, this ElementCollector can't do notify
         * unless its priority is BEFOREMODIFY.
         */
        if (nPriority == nMaxPriority &&
            (nPriority == css::xml::crypto::sax::ElementMarkPriority_BEFOREMODIFY ||
             !isBlockerInSubTreeIncluded(pElementCollector->getSecurityId())))
        {
            /*
             * If this ElementCollector will modify the buffered element, then
             * special attention must be paid.
             *
             * If there is any ElementCollector in the subtree or any ancestor
             * ElementCollector with PRI_BEFPREMODIFY priority, this
             * ElementCollector can't perform notify operation, otherwise, it
             * will destroy the buffered element, in turn, ElementCollectors
             * mentioned above can't perform their mission.
             */
            //if (!(nMaxPriority == css::xml::crypto::sax::ElementMarkPriority_PRI_MODIFY &&
            if (!(bToModify &&
                 (isECInSubTreeIncluded(pElementCollector->getSecurityId()) ||
                  isECOfBeforeModifyInAncestorIncluded(pElementCollector->getSecurityId()))
               ))
            {
                pElementCollector->notifyListener();
            }
        }
    }
}

bool BufferNode::isECInSubTreeIncluded(sal_Int32 nIgnoredSecurityId) const
/****** BufferNode/isECInSubTreeIncluded *************************************
 *
 *   NAME
 *  isECInSubTreeIncluded -- checks whether there is any ElementCollector
 *  in the branch of this BufferNode.
 *
 *   SYNOPSIS
 *  bExist = isECInSubTreeIncluded(nIgnoredSecurityId);
 *
 *   FUNCTION
 *  checks each BufferNode in the branch of this BufferNode, if there is
 *  an ElementCollector whose signatureId is not ignored, then return
 *  true, otherwise, false returned.
 *
 *   INPUTS
 *  nIgnoredSecurityId -    the security Id to be ignored. If it equals
 *                          to UNDEFINEDSECURITYID, then no security Id
 *                          will be ignored.
 *
 *   RESULT
 *  bExist - true if a match found, false otherwise.
 ******************************************************************************/
{
    bool rc = std::any_of(m_vElementCollectors.begin(), m_vElementCollectors.end(),
        [nIgnoredSecurityId](const ElementCollector* pElementCollector) {
            return nIgnoredSecurityId == css::xml::crypto::sax::ConstOfSecurityId::UNDEFINEDSECURITYID ||
                pElementCollector->getSecurityId() != nIgnoredSecurityId;
    });

    if ( !rc )
    {
        rc = std::any_of(m_vChildren.begin(), m_vChildren.end(),
            [nIgnoredSecurityId](const std::unique_ptr<BufferNode>& pBufferNode) {
                return pBufferNode->isECInSubTreeIncluded(nIgnoredSecurityId);
        });
    }

    return rc;
}

bool BufferNode::isECOfBeforeModifyInAncestorIncluded(sal_Int32 nIgnoredSecurityId) const
/****** BufferNode/isECOfBeforeModifyInAncestorIncluded **********************
 *
 *   NAME
 *  isECOfBeforeModifyInAncestorIncluded -- checks whether there is some
 *  ancestor BufferNode which has ElementCollector with PRI_BEFPREMODIFY
 *  priority.
 *
 *   SYNOPSIS
 *  bExist = isECOfBeforeModifyInAncestorIncluded(nIgnoredSecurityId);
 *
 *   FUNCTION
 *  checks each ancestor BufferNode through the parent link, if there is
 *  an ElementCollector with PRI_BEFPREMODIFY priority and its
 *  signatureId is not ignored, then return true, otherwise, false
 *  returned.
 *
 *   INPUTS
 *  nIgnoredSecurityId -    the security Id to be ignored. If it equals
 *                          to UNDEFINEDSECURITYID, then no security Id
 *                          will be ignored.
 *
 *   RESULT
 *  bExist - true if a match found, false otherwise.
 ******************************************************************************/
{
    bool rc = false;

    BufferNode* pParentNode = m_pParent;
    while (pParentNode != nullptr)
    {
        if (pParentNode->isECOfBeforeModifyIncluded(nIgnoredSecurityId))
        {
            rc = true;
            break;
        }

        pParentNode = const_cast<BufferNode*>(pParentNode->getParent());
    }

    return rc;
}

bool BufferNode::isBlockerInSubTreeIncluded(sal_Int32 nIgnoredSecurityId) const
/****** BufferNode/isBlockerInSubTreeIncluded ********************************
 *
 *   NAME
 *  isBlockerInSubTreeIncluded -- checks whether there is some BufferNode
 *  which has blocker on it
 *
 *   SYNOPSIS
 *  bExist = isBlockerInSubTreeIncluded(nIgnoredSecurityId);
 *
 *   FUNCTION
 *  checks each BufferNode in the branch of this BufferNode, if one has
 *  a blocker on it, and the blocker's securityId is not ignored, then
 *  returns true; otherwise, false returns.
 *
 *   INPUTS
 *  nIgnoredSecurityId -    the security Id to be ignored. If it equals
 *                          to UNDEFINEDSECURITYID, then no security Id
 *                          will be ignored.
 *
 *   RESULT
 *  bExist - true if a match found, false otherwise.
 ******************************************************************************/
{
    return std::any_of(m_vChildren.begin(), m_vChildren.end(),
        [nIgnoredSecurityId](const std::unique_ptr<BufferNode>& pBufferNode) {
            ElementMark* pBlocker = pBufferNode->getBlocker();
            return (pBlocker != nullptr &&
                (nIgnoredSecurityId == css::xml::crypto::sax::ConstOfSecurityId::UNDEFINEDSECURITYID ||
                 pBlocker->getSecurityId() != nIgnoredSecurityId )) ||
                pBufferNode->isBlockerInSubTreeIncluded(nIgnoredSecurityId);
    });
}

const BufferNode* BufferNode::getNextChild(const BufferNode* pChild) const
/****** BufferNode/getNextChild **********************************************
 *
 *   NAME
 *  getNextChild -- get the next child BufferNode.
 *
 *   SYNOPSIS
 *  nextChild = getNextChild();
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  pChild - the child BufferNode whose next node is retrieved.
 *
 *   RESULT
 *  nextChild - the next child BufferNode after the pChild, or NULL if
 *  there is none.
 ******************************************************************************/
{
    BufferNode* rc = nullptr;
    bool bChildFound = false;

    for( std::unique_ptr<BufferNode> const & i : m_vChildren )
    {
        if (bChildFound)
        {
            rc = i.get();
            break;
        }

        if( i.get() == pChild )
        {
            bChildFound = true;
        }
    }

    return rc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
