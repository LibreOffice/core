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

namespace cssu = com::sun::star::uno;
namespace cssxw = com::sun::star::xml::wrapper;
namespace cssxc = com::sun::star::xml::crypto;

BufferNode::BufferNode( const cssu::Reference< cssxw::XXMLElementWrapper >& xXMLElement )
    :m_pParent(NULL),
     m_pBlocker(NULL),
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    bool rc = false;
    std::vector< const ElementCollector* >::const_iterator ii = m_vElementCollectors.begin();

    for( ; ii != m_vElementCollectors.end() ; ++ii )
    {
        ElementCollector* pElementCollector = (ElementCollector*)*ii;

        if ((nIgnoredSecurityId == cssxc::sax::ConstOfSecurityId::UNDEFINEDSECURITYID ||
             pElementCollector->getSecurityId() != nIgnoredSecurityId) &&
            (pElementCollector->getPriority() == cssxc::sax::ElementMarkPriority_BEFOREMODIFY))
        {
            rc = true;
            break;
        }
    }

    return rc;
}

void BufferNode::setReceivedAll()
/****** BufferNode/setReceiveAll *********************************************
 *
 *   NAME
 *  setReceivedAll -- indicates that the element in this BufferNode has
 *  been compeletely bufferred.
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    m_bAllReceived = true;
    elementCollectorNotify();
}

bool BufferNode::isAllReceived() const
{
    return m_bAllReceived;
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    m_vElementCollectors.push_back( pElementCollector );
    ((ElementCollector*)pElementCollector)->setBufferNode(this);
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    std::vector< const ElementCollector* >::iterator ii = m_vElementCollectors.begin();

    for( ; ii != m_vElementCollectors.end() ; ++ii )
    {
        if( *ii == pElementCollector )
        {
            m_vElementCollectors.erase( ii );
            ((ElementCollector*)pElementCollector)->setBufferNode(NULL);
            break;
        }
    }
}

ElementMark* BufferNode::getBlocker() const
{
    return m_pBlocker;
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
 *  Because there is only one blocker permited for a BufferNode, so the
 *  old blocker on this BufferNode, if there is one, will be overcasted.
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    OSL_ASSERT(!(m_pBlocker != NULL && pBlocker != NULL));

    m_pBlocker = (ElementMark*)pBlocker;
    if (m_pBlocker != NULL)
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    OUString rc;
    std::vector< const ElementCollector* >::const_iterator ii = m_vElementCollectors.begin();

    for( ; ii != m_vElementCollectors.end() ; ++ii )
    {
        rc += OUString( "BufID=" );
        rc += OUString::number((*ii)->getBufferId());

        if (((ElementCollector*)(*ii))->getModify())
        {
            rc += OUString( "[M]" );
        }

        rc += OUString( ",Pri=" );

        switch (((ElementCollector*)(*ii))->getPriority())
        {
            case cssxc::sax::ElementMarkPriority_BEFOREMODIFY:
                rc += OUString( "BEFOREMODIFY" );
                break;
            case cssxc::sax::ElementMarkPriority_AFTERMODIFY:
                rc += OUString( "AFTERMODIFY" );
                break;
            default:
                rc += OUString( "UNKNOWN" );
                break;
        }

        rc += OUString( "(" );
        rc += OUString( "SecID=" );
        rc += OUString::number(((ElementCollector*)(*ii))->getSecurityId());
        rc += OUString( ")" );
        rc += OUString( " " );
    }

    return rc;
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    return (!m_vChildren.empty());
}

std::vector< const BufferNode* >* BufferNode::getChildren() const
{
    return new std::vector< const BufferNode* >( m_vChildren );
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    BufferNode* rc = NULL;

    if (!m_vChildren.empty())
    {
        rc = (BufferNode*)m_vChildren.front();
    }

    return (const BufferNode*)rc;
}

void BufferNode::addChild(const BufferNode* pChild, sal_Int32 nPosition)
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    if (nPosition == -1)
    {
        m_vChildren.push_back( pChild );
    }
    else
    {
        std::vector< const BufferNode* >::iterator ii = m_vChildren.begin();
        ii += nPosition;
        m_vChildren.insert(ii, pChild);
    }
}

void BufferNode::addChild(const BufferNode* pChild)
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    addChild(pChild, -1);
}

void BufferNode::removeChild(const BufferNode* pChild)
/****** BufferNode/removeChild ***********************************************
 *
 *   NAME
 *  removeChild -- removes a child BufferNode from the children list.
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    std::vector< const BufferNode* >::iterator ii = m_vChildren.begin();

    for( ; ii != m_vChildren.end() ; ++ii )
    {
        if( *ii == pChild )
        {
            m_vChildren.erase( ii );
            break;
        }
    }
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    sal_Int32 nIndex = 0;
    bool bFound = false;

    std::vector< const BufferNode * >::const_iterator ii = m_vChildren.begin();

    for( ; ii != m_vChildren.end() ; ++ii )
    {
        if( *ii == pChild )
        {
            bFound = true;
            break;
        }
        nIndex++;
    }

    if (!bFound )
    {
        nIndex = -1;
    }

    return nIndex;
}

const BufferNode* BufferNode::getParent() const
{
    return m_pParent;
}

void BufferNode::setParent(const BufferNode* pParent)
{
    m_pParent = (BufferNode*)pParent;
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    BufferNode* rc = NULL;

    if (m_pParent != NULL)
    {
        rc = (BufferNode*)m_pParent->getNextChild(this);
    }

    return (const BufferNode*)rc;
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    BufferNode* rc = NULL;

    if (pDescendant != NULL)
    {
        std::vector< const BufferNode* >::const_iterator ii = m_vChildren.begin();

        for( ; ii != m_vChildren.end() ; ++ii )
        {
            BufferNode* pChild = (BufferNode*)*ii;

            if (pChild == pDescendant)
            {
                rc = pChild;
                break;
            }

            if (pChild->isAncestor(pDescendant) != NULL)
            {
                rc = pChild;
                break;
            }
        }
    }

    return (const BufferNode*)rc;
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    bool rc = false;

    BufferNode* pNextBufferNode = (BufferNode*)getNextNodeByTreeOrder();
    while (pNextBufferNode != NULL)
    {
        if (pNextBufferNode == pFollowing)
        {
            rc = true;
            break;
        }

        pNextBufferNode = (BufferNode*)(pNextBufferNode->getNextNodeByTreeOrder());
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
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
    BufferNode* pNextSibling = (BufferNode*)getNextSibling();
    if (pNextSibling != NULL)
    {
        return pNextSibling;
    }

        /*
         * Otherwise, it this buffer node has parent, then return
         * its parent's following sibling.
         */
        BufferNode* pNode = (BufferNode*)this;
    BufferNode* pParent;
    BufferNode* pNextSiblingParent = NULL;

    do
    {
        if (pNode == NULL)
        {
            break;
        }

        pParent = (BufferNode*)pNode->getParent();
        if (pParent != NULL)
        {
            pNextSiblingParent = (BufferNode*)pParent->getNextSibling();
        }
        pNode = pParent;

    }while (pNextSiblingParent == NULL);

    return pNextSiblingParent;
}

cssu::Reference< cssxw::XXMLElementWrapper > BufferNode::getXMLElement() const
{
    return m_xXMLElement;
}

void BufferNode::setXMLElement( const cssu::Reference< cssxw::XXMLElementWrapper >& xXMLElement )
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    std::vector< const BufferNode* >::const_iterator ii = m_vChildren.begin();

    for( ; ii != m_vChildren.end() ; ++ii )
    {
        BufferNode* pBufferNode = (BufferNode*)*ii;
        pBufferNode->elementCollectorNotify();
        pBufferNode->notifyBranch();
    }
}

void BufferNode::notifyAncestor()
/****** BufferNode/notifyAncestor ********************************************
 *
 *   NAME
 *  notifyAncestor -- notifies each ancestor BufferNode through the parent
 *  link.
 *
 *   SYNOPSIS
 *  notifyAncestor();
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    BufferNode* pParent = m_pParent;
    while (pParent != NULL)
    {
        pParent->notifyAncestor();
        pParent = (BufferNode*)pParent->getParent();
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    if (!m_vElementCollectors.empty())
    {
        cssxc::sax::ElementMarkPriority nMaxPriority = cssxc::sax::ElementMarkPriority_MINIMUM;
        cssxc::sax::ElementMarkPriority nPriority;

        /*
         * get the max priority among ElementCollectors on this BufferNode
         */
        std::vector< const ElementCollector* >::const_iterator ii = m_vElementCollectors.begin();
        for( ; ii != m_vElementCollectors.end() ; ++ii )
        {
            ElementCollector* pElementCollector = (ElementCollector*)*ii;
            nPriority = pElementCollector->getPriority();
            if (nPriority > nMaxPriority)
            {
                nMaxPriority = nPriority;
            }
        }

        std::vector< const ElementCollector* > vElementCollectors( m_vElementCollectors );
        ii = vElementCollectors.begin();

        for( ; ii != vElementCollectors.end() ; ++ii )
        {
            ElementCollector* pElementCollector = (ElementCollector*)*ii;
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
                (nPriority == cssxc::sax::ElementMarkPriority_BEFOREMODIFY ||
                 !isBlockerInSubTreeIncluded(pElementCollector->getSecurityId())))
            {
                /*
                 * If this ElementCollector will modify the bufferred element, then
                 * special attention must be paid.
                 *
                 * If there is any ElementCollector in the subtree or any ancestor
                 * ElementCollector with PRI_BEFPREMODIFY priority, this
                 * ElementCollector can't perform notify operation, otherwise, it
                 * will destroy the bufferred element, in turn, ElementCollectors
                 * mentioned above can't perform their mission.
                 */
                //if (!(nMaxPriority == cssxc::sax::ElementMarkPriority_PRI_MODIFY &&
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    bool rc = false;

    std::vector< const ElementCollector* >::const_iterator jj = m_vElementCollectors.begin();

    for( ; jj != m_vElementCollectors.end() ; ++jj )
    {
        ElementCollector* pElementCollector = (ElementCollector*)*jj;
        if (nIgnoredSecurityId == cssxc::sax::ConstOfSecurityId::UNDEFINEDSECURITYID ||
             pElementCollector->getSecurityId() != nIgnoredSecurityId)
        {
            rc = true;
            break;
        }
    }

    if ( !rc )
    {
        std::vector< const BufferNode* >::const_iterator ii = m_vChildren.begin();

        for( ; ii != m_vChildren.end() ; ++ii )
        {
            BufferNode* pBufferNode = (BufferNode*)*ii;

            if ( pBufferNode->isECInSubTreeIncluded(nIgnoredSecurityId))
            {
                rc = true;
                break;
            }
        }
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    bool rc = false;

    BufferNode* pParentNode = m_pParent;
    while (pParentNode != NULL)
    {
        if (pParentNode->isECOfBeforeModifyIncluded(nIgnoredSecurityId))
        {
            rc = true;
            break;
        }

        pParentNode = (BufferNode*)pParentNode->getParent();
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    bool rc = false;

    std::vector< const BufferNode* >::const_iterator ii = m_vChildren.begin();

    for( ; ii != m_vChildren.end() ; ++ii )
    {
        BufferNode* pBufferNode = (BufferNode*)*ii;
        ElementMark* pBlocker = pBufferNode->getBlocker();

        if (pBlocker != NULL &&
            (nIgnoredSecurityId == cssxc::sax::ConstOfSecurityId::UNDEFINEDSECURITYID ||
            pBlocker->getSecurityId() != nIgnoredSecurityId ))
        {
            rc = true;
            break;
        }

        if (rc || pBufferNode->isBlockerInSubTreeIncluded(nIgnoredSecurityId))
        {
            rc = true;
            break;
        }
    }

    return rc;
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
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    BufferNode* rc = NULL;
    bool bChildFound = false;

    std::vector< const BufferNode* >::const_iterator ii = m_vChildren.begin();
    for( ; ii != m_vChildren.end() ; ++ii )
    {
        if (bChildFound)
        {
            rc = (BufferNode*)*ii;
            break;
        }

        if( *ii == pChild )
        {
            bChildFound = true;
        }
    }

    return (const BufferNode*)rc;
}


void BufferNode::freeAllChildren()
/****** BufferNode/freeAllChildren *******************************************
 *
 *   NAME
 *  freeAllChildren -- free all his child BufferNode.
 *
 *   SYNOPSIS
 *  freeAllChildren();
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    std::vector< const BufferNode* >::const_iterator ii = m_vChildren.begin();
    for( ; ii != m_vChildren.end() ; ++ii )
    {
        BufferNode *pChild = (BufferNode *)(*ii);
        pChild->freeAllChildren();
        delete pChild;
    }

    m_vChildren.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
