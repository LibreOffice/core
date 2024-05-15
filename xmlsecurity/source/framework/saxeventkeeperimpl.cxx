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


#include <framework/saxeventkeeperimpl.hxx>
#include "buffernode.hxx"
#include "elementmark.hxx"
#include "elementcollector.hxx"
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/crypto/sax/ConstOfSecurityId.hpp>
#include <com/sun/star/xml/wrapper/XXMLDocumentWrapper.hpp>
#include <com/sun/star/xml/csax/XCompressedDocumentHandler.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>

#include <algorithm>

SAXEventKeeperImpl::SAXEventKeeperImpl( )
    :m_pCurrentBufferNode(nullptr),
     m_nNextElementMarkId(1),
     m_pNewBlocker(nullptr),
     m_pCurrentBlockingBufferNode(nullptr),
     m_bIsReleasing(false),
     m_bIsForwarding(false)
{
    m_vElementMarkBuffers.reserve(2);
    m_vNewElementCollectors.reserve(2);
    m_vReleasedElementMarkBuffers.reserve(2);
}

SAXEventKeeperImpl::~SAXEventKeeperImpl()
{
    /*
     * delete the BufferNode tree
     */
    m_pRootBufferNode.reset();

    m_pCurrentBufferNode = m_pCurrentBlockingBufferNode = nullptr;

    /*
     * delete all unfreed ElementMarks
     */
    m_vNewElementCollectors.clear();
    m_pNewBlocker = nullptr;
}

void SAXEventKeeperImpl::setCurrentBufferNode(BufferNode* pBufferNode)
/****** SAXEventKeeperImpl/setCurrentBufferNode ******************************
 *
 *   NAME
 *  setCurrentBufferNode -- set a new active BufferNode.
 *
 *   SYNOPSIS
 *  setCurrentBufferNode( pBufferNode );
 *
 *   FUNCTION
 *  connects this BufferNode into the BufferNode tree as a child of the
 *  current active BufferNode. Then makes this BufferNode as the current
 *  active BufferNode.
 *  If the previous active BufferNode points to the root
 *  BufferNode, which means that no buffering operation was proceeding,
 *  then notifies the status change listener that buffering  operation
 *  will begin at once.
 *
 *   INPUTS
 *  pBufferNode - a BufferNode which will be the new active BufferNode
 ******************************************************************************/
{
    if (pBufferNode == m_pCurrentBufferNode)
        return;

    if ( m_pCurrentBufferNode == m_pRootBufferNode.get() &&
         m_xSAXEventKeeperStatusChangeListener.is())
    {
        m_xSAXEventKeeperStatusChangeListener->collectionStatusChanged(true);
    }

    if (pBufferNode->getParent() == nullptr)
    {
        m_pCurrentBufferNode->addChild(std::unique_ptr<BufferNode>(pBufferNode));
        pBufferNode->setParent(m_pCurrentBufferNode);
    }

    m_pCurrentBufferNode = pBufferNode;
}

BufferNode* SAXEventKeeperImpl::addNewElementMarkBuffers()
/****** SAXEventKeeperImpl/addNewElementMarkBuffers **************************
 *
 *   NAME
 *  addNewElementMarkBuffers -- add new ElementCollectors and new Blocker.
 *
 *   SYNOPSIS
 *  pBufferNode = addNewElementMarkBuffers( );
 *
 *   FUNCTION
 *  if there are new ElementCollector or new Blocker to be added, then
 *  connect all of them with the current BufferNode. In case of the
 *  current BufferNode doesn't exist, creates one.
 *  Clears up the new ElementCollector list and the new Blocker pointer.
 *
 *   RESULT
 *  pBufferNode - the BufferNode that has been connected with both new
 *                ElementCollectors and new Blocker.
 ******************************************************************************/
{
    BufferNode* pBufferNode = nullptr;

    if (m_pNewBlocker || !m_vNewElementCollectors.empty() )
    {
        /*
         * When the current BufferNode is right pointing to the current
         * working element in the XMLDocumentWrapper component, then
         * no new BufferNode is needed to create.
         * This situation can only happen in the "Forwarding" mode.
         */
        if ( (m_pCurrentBufferNode != nullptr) &&
             (m_xXMLDocument->isCurrent(m_pCurrentBufferNode->getXMLElement())))
        {
            pBufferNode = m_pCurrentBufferNode;
        }
        else
        {
            pBufferNode = new BufferNode(m_xXMLDocument->getCurrentElement());
        }

        if (m_pNewBlocker != nullptr)
        {
            pBufferNode->setBlocker(m_pNewBlocker);

            /*
             * If no blocking before, then notify the status change listener that
             * the SAXEventKeeper has entered "blocking" status, during which, no
             * SAX events will be forwarded to the next document handler.
             */
            if (m_pCurrentBlockingBufferNode == nullptr)
            {
                m_pCurrentBlockingBufferNode = pBufferNode;

                if (m_xSAXEventKeeperStatusChangeListener.is())
                {
                    m_xSAXEventKeeperStatusChangeListener->blockingStatusChanged(true);
                }
            }

            m_pNewBlocker = nullptr;
        }

        for( const auto& i : m_vNewElementCollectors )
        {
            pBufferNode->addElementCollector(i);
        }
        m_vNewElementCollectors.clear();
    }

    return pBufferNode;
}

ElementMark* SAXEventKeeperImpl::findElementMarkBuffer(sal_Int32 nId) const
/****** SAXEventKeeperImpl/findElementMarkBuffer *****************************
 *
 *   NAME
 *  findElementMarkBuffer -- finds an ElementMark.
 *
 *   SYNOPSIS
 *  pElementMark = findElementMarkBuffer( nId );
 *
 *   FUNCTION
 *  searches an ElementMark with the particular Id in the ElementMark
 *  list.
 *
 *   INPUTS
 *  nId - the Id of the ElementMark to be searched.
 *
 *   RESULT
 *  pElementMark - the ElementMark with the particular Id, or NULL when
 *                 no such Id exists.
 ******************************************************************************/
{
    ElementMark* pElementMark = nullptr;

    for( auto&& ii : m_vElementMarkBuffers )
    {
        if ( nId == ii->getBufferId())
        {
            pElementMark = const_cast<ElementMark*>(ii.get());
            break;
        }
    }

    return pElementMark;
}

void SAXEventKeeperImpl::removeElementMarkBuffer(sal_Int32 nId)
/****** SAXEventKeeperImpl/removeElementMarkBuffer ***************************
 *
 *   NAME
 *  removeElementMarkBuffer -- removes an ElementMark
 *
 *   SYNOPSIS
 *  removeElementMarkBuffer( nId );
 *
 *   FUNCTION
 *  removes an ElementMark with the particular Id in the ElementMark list.
 *
 *   INPUTS
 *  nId - the Id of the ElementMark to be removed.
 ******************************************************************************/
{
    auto ii = std::find_if(m_vElementMarkBuffers.begin(), m_vElementMarkBuffers.end(),
        [nId](std::unique_ptr<const ElementMark>& rElementMark) { return nId == rElementMark->getBufferId(); }
    );
    if (ii == m_vElementMarkBuffers.end())
        return;

    /*
     * checks whether this ElementMark still in the new ElementCollect array
     */
    auto jj = std::find_if(m_vNewElementCollectors.begin(), m_vNewElementCollectors.end(),
        [&ii](const ElementCollector* pElementCollector) { return ii->get() == pElementCollector; }
    );
    if (jj != m_vNewElementCollectors.end())
        m_vNewElementCollectors.erase(jj);

    /*
     * checks whether this ElementMark is the new Blocker
     */
    if (ii->get() == m_pNewBlocker)
    {
        m_pNewBlocker = nullptr;
    }

    m_vElementMarkBuffers.erase( ii );
}

OUString SAXEventKeeperImpl::printBufferNode(
    BufferNode const * pBufferNode, sal_Int32 nIndent) const
/****** SAXEventKeeperImpl/printBufferNode ***********************************
 *
 *   NAME
 *  printBufferNode -- retrieves the information of a BufferNode and its
 *  branch.
 *
 *   SYNOPSIS
 *  info = printBufferNode( pBufferNode, nIndent );
 *
 *   FUNCTION
 *  all retrieved information includes:
 *  1. whether it is the current BufferNode;
 *  2. whether it is the current blocking BufferNode;
 *  3. the name of the parent element;
 *  4. the name of this element;
 *  5. all ElementCollectors working on this BufferNode;
 *  6. the Blocker working on this BufferNode;
 *  7. all child BufferNodes' information.
 *
 *   INPUTS
 *  pBufferNode -   the BufferNode from where information will be retrieved.
 *  nIndent -   how many space characters prefixed before the output
 *              message.
 *
 *   RESULT
 *  info - the information string
 ******************************************************************************/
{
    OUStringBuffer rc;

    for ( int i=0; i<nIndent; ++i )
    {
        rc.append(" ");
    }

    if (pBufferNode == m_pCurrentBufferNode)
    {
        rc.append("[%]");
    }

    if (pBufferNode == m_pCurrentBlockingBufferNode)
    {
        rc.append("[B]");
    }

    rc.append(" " + m_xXMLDocument->getNodeName(pBufferNode->getXMLElement()));

    BufferNode* pParent = const_cast<BufferNode*>(pBufferNode->getParent());
    if (pParent != nullptr)
    {
        rc.append("[" + m_xXMLDocument->getNodeName(pParent->getXMLElement()) + "]");
    }

    rc.append(":EC=" + pBufferNode->printChildren() + " BR=");

    ElementMark * pBlocker = pBufferNode->getBlocker();
    if (pBlocker != nullptr)
    {
        rc.append( OUString::number(pBlocker->getBufferId()) +
            "(SecId=" + OUString::number( pBlocker->getSecurityId() ) + ") ");
    }
    rc.append("\n");

    std::vector< std::unique_ptr<BufferNode> > const & vChildren = pBufferNode->getChildren();
    for( const auto& jj : vChildren )
    {
        rc.append(printBufferNode(jj.get(), nIndent+4));
    }

    return rc.makeStringAndClear();
}

css::uno::Sequence< css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > >
    SAXEventKeeperImpl::collectChildWorkingElement(BufferNode const * pBufferNode)
/****** SAXEventKeeperImpl/collectChildWorkingElement ************************
 *
 *   NAME
 *  collectChildWorkingElement -- collects a BufferNode's all child
 *  Elements.
 *
 *   SYNOPSIS
 *  list = collectChildWorkingElement( pBufferNode );
 *
 *   INPUTS
 *  pBufferNode - the BufferNode whose child Elements will be collected.
 *
 *   RESULT
 *  list - the child Elements list.
 ******************************************************************************/
{
    std::vector< std::unique_ptr<BufferNode> > const & vChildren = pBufferNode->getChildren();

    css::uno::Sequence < css::uno::Reference<
        css::xml::wrapper::XXMLElementWrapper > > aChildrenCollection ( vChildren.size());

    std::transform(vChildren.begin(), vChildren.end(), aChildrenCollection.getArray(),
                   [](const auto& i) { return i->getXMLElement(); });

    return aChildrenCollection;
}

void SAXEventKeeperImpl::smashBufferNode(
    BufferNode* pBufferNode, bool bClearRoot) const
/****** SAXEventKeeperImpl/smashBufferNode ***********************************
 *
 *   NAME
 *  smashBufferNode -- removes a BufferNode along with its working
 *  element.
 *
 *   SYNOPSIS
 *  smashBufferNode( pBufferNode, bClearRoot );
 *
 *   FUNCTION
 *  removes the BufferNode's working element from the DOM document, while
 *  reserves all ancestor paths for its child BufferNodes.
 *  when any of the BufferNode's ancestor element is useless, removes it
 *  too.
 *  removes the BufferNode from the BufferNode tree.
 *
 *   INPUTS
 *  pBufferNode -   the BufferNode to be removed
 *  bClearRoot -    whether the root element also needs to be cleared up.
 *
 *   NOTES
 *  when removing a Blocker's BufferNode, the bClearRoot flag should be
 *  true. Because a Blocker can buffer many SAX events which are not used
 *  by any other ElementCollector or Blocker.
 *  When the bClearRoot is set to true, the root BufferNode will be first
 *  cleared, with a stop flag setting at the next Blocking BufferNode. This
 *  operation can delete all useless buffered SAX events which are only
 *  needed by the Blocker to be deleted.
 ******************************************************************************/
{
    if (pBufferNode->hasAnything())
        return;

    BufferNode* pParent = const_cast<BufferNode*>(pBufferNode->getParent());

    /*
     * delete the XML data
     */
    if (pParent == m_pRootBufferNode.get())
    {
        bool bIsNotBlocking = (m_pCurrentBlockingBufferNode == nullptr);
        bool bIsBlockInside = false;
        bool bIsBlockingAfterward = false;

        /*
         * If this is a blocker, then remove any out-element data
         * which caused by blocking. The removal process will stop
         * at the next blocker to avoid removing any useful data.
         */
        if (bClearRoot)
        {
            css::uno::Sequence< css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > >
                aChildElements = collectChildWorkingElement(m_pRootBufferNode.get());

            /*
             * the clearUselessData only clearup the content in the
             * node, not the node itself.
             */
            m_xXMLDocument->clearUselessData(m_pRootBufferNode->getXMLElement(),
                aChildElements,
                bIsNotBlocking?nullptr:
                               (m_pCurrentBlockingBufferNode->getXMLElement()));

            /*
             * remove the node if it is empty, then if its parent is also
             * empty, remove it, then if the next parent is also empty,
             * remove it,..., until parent become null.
             */
            m_xXMLDocument->collapse( m_pRootBufferNode->getXMLElement() );
        }

        /*
         * if blocking, check the relationship between this BufferNode and
         * the current blocking BufferNode.
         */
        if ( !bIsNotBlocking )
        {
            /*
             * the current blocking BufferNode is a descendant of this BufferNode.
             */
            bIsBlockInside = (nullptr != pBufferNode->isAncestor(m_pCurrentBlockingBufferNode));

            /*
             * the current blocking BufferNode locates behind this BufferNode in tree
             * order.
             */
            bIsBlockingAfterward = pBufferNode->isPrevious(m_pCurrentBlockingBufferNode);
        }

        /*
         * this BufferNode's working element needs to be deleted only when
         * 1. there is no blocking, or
         * 2. the current blocking BufferNode is a descendant of this BufferNode,
         *    (then in the BufferNode's working element, the useless data before the blocking
         *     element should be deleted.) or
         * 3. the current blocking BufferNode is locates behind this BufferNode in tree,
         *    (then the useless data between the blocking element and the working element
         *     should be deleted.).
         * Otherwise, this working element should not be deleted.
         */
        if ( bIsNotBlocking || bIsBlockInside || bIsBlockingAfterward )
        {
            css::uno::Sequence< css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > >
                aChildElements = collectChildWorkingElement(pBufferNode);

            /*
             * the clearUselessData only clearup the content in the
             * node, not the node itself.
             */
            m_xXMLDocument->clearUselessData(pBufferNode->getXMLElement(),
                aChildElements,
                bIsBlockInside?(m_pCurrentBlockingBufferNode->getXMLElement()):
                           nullptr);

            /*
             * remove the node if it is empty, then if its parent is also
             * empty, remove it, then if the next parent is also empty,
             * remove it,..., until parent become null.
             */
            m_xXMLDocument->collapse( pBufferNode->getXMLElement() );
        }
    }

    sal_Int32 nIndex = pParent->indexOfChild(pBufferNode);

    std::vector< std::unique_ptr<BufferNode> > vChildren = pBufferNode->releaseChildren();
    pParent->removeChild(pBufferNode); // delete buffernode

    for( auto& i : vChildren )
    {
        i->setParent(pParent);
        pParent->addChild(std::move(i), nIndex);
        nIndex++;
    }
}

BufferNode* SAXEventKeeperImpl::findNextBlockingBufferNode(
    BufferNode* pStartBufferNode)
/****** SAXEventKeeperImpl/findNextBlockingBufferNode ************************
 *
 *   NAME
 *  findNextBlockingBufferNode -- finds the next blocking BufferNode
 *  behind the particular BufferNode.
 *
 *   SYNOPSIS
 *  pBufferNode = findNextBlockingBufferNode( pStartBufferNode );
 *
 *   INPUTS
 *  pStartBufferNode - the BufferNode from where to search the next
 *                     blocking BufferNode.
 *
 *   RESULT
 *  pBufferNode - the next blocking BufferNode, or NULL if no such
 *                BufferNode exists.
 ******************************************************************************/
{
    BufferNode* pNext = nullptr;

    if (pStartBufferNode != nullptr)
    {
        pNext = pStartBufferNode;

        while (nullptr != (pNext = const_cast<BufferNode*>(pNext->getNextNodeByTreeOrder())))
        {
            if (pNext->getBlocker() != nullptr)
            {
                break;
            }
        }
    }

    return pNext;
}

void SAXEventKeeperImpl::diffuse(BufferNode* pBufferNode)
/****** SAXEventKeeperImpl/diffuse *******************************************
 *
 *   NAME
 *  diffuse -- diffuse the notification.
 *
 *   SYNOPSIS
 *  diffuse( pBufferNode );
 *
 *   FUNCTION
 *  diffuse the collecting completion notification from the specific
 *  BufferNode along its parent link, until an ancestor which is not
 *  completely received is met.
 *
 *   INPUTS
 *  pBufferNode - the BufferNode from which the notification will be
 *                diffused.
 ******************************************************************************/
{
    BufferNode* pParent = pBufferNode;

    while(pParent->isAllReceived())
    {
        pParent->elementCollectorNotify();
        pParent = const_cast<BufferNode*>(pParent->getParent());
    }
}

void SAXEventKeeperImpl::releaseElementMarkBuffer()
/****** SAXEventKeeperImpl/releaseElementMarkBuffer **************************
 *
 *   NAME
 *  releaseElementMarkBuffer -- releases useless ElementMarks
 *
 *   SYNOPSIS
 *  releaseElementMarkBuffer( );
 *
 *   FUNCTION
 *  releases each ElementMark in the releasing list
 *  m_vReleasedElementMarkBuffers.
 *  The operation differs between an ElementCollector and a Blocker.
 ******************************************************************************/
{
    m_bIsReleasing = true;
    while (!m_vReleasedElementMarkBuffers.empty())
    {
        auto pId = m_vReleasedElementMarkBuffers.begin();
        sal_Int32 nId = *pId;
        m_vReleasedElementMarkBuffers.erase( pId );

        ElementMark* pElementMark = findElementMarkBuffer(nId);

        if (pElementMark != nullptr)
        {
            if (css::xml::crypto::sax::ElementMarkType_ELEMENTCOLLECTOR
                == pElementMark->getType())
            /*
             * it is a EC
             */
            {
                ElementCollector* pElementCollector = static_cast<ElementCollector*>(pElementMark);

                css::xml::crypto::sax::ElementMarkPriority nPriority = pElementCollector->getPriority();
                /*
                 * Delete the EC from the buffer node.
                 */
                BufferNode* pBufferNode = pElementCollector->getBufferNode();
                pBufferNode->removeElementCollector(pElementCollector);

                if ( nPriority == css::xml::crypto::sax::ElementMarkPriority_BEFOREMODIFY)
                {
                    pBufferNode->notifyBranch();
                }

                /*
                 * delete the ElementMark
                 */
                pElementCollector = nullptr;
                pElementMark = nullptr;
                removeElementMarkBuffer(nId);

                /*
                 * delete the BufferNode
                 */
                diffuse(pBufferNode);
                smashBufferNode(pBufferNode, false);
            }
            else
            /*
             * it is a Blocker
             */
            {
                /*
                 * Delete the TH from the buffer node.
                 */
                BufferNode *pBufferNode = pElementMark->getBufferNode();
                assert(pBufferNode);
                pBufferNode->setBlocker(nullptr);

                /*
                 * If there is a following handler and no blocking now, then
                 * forward this event
                 */
                if (m_pCurrentBlockingBufferNode == pBufferNode)
                {
                    /*
                     * Before forwarding, the next blocking point needs to be
                     * found.
                     */
                    m_pCurrentBlockingBufferNode = findNextBlockingBufferNode(pBufferNode);

                    /*
                     * Forward the blocked events between these two STHs.
                     */
                    if (m_xNextHandler.is())
                    {
                        BufferNode* pTempCurrentBufferNode = m_pCurrentBufferNode;
                        BufferNode* pTempCurrentBlockingBufferNode = m_pCurrentBlockingBufferNode;

                        m_pCurrentBufferNode = pBufferNode;
                        assert(pBufferNode);
                        m_pCurrentBlockingBufferNode = nullptr;

                        m_bIsForwarding = true;

                        m_xXMLDocument->generateSAXEvents(
                            m_xNextHandler,
                            this,
                            pBufferNode->getXMLElement(),
                            (pTempCurrentBlockingBufferNode == nullptr)?nullptr:(pTempCurrentBlockingBufferNode->getXMLElement()));

                        m_bIsForwarding = false;

                        m_pCurrentBufferNode = pTempCurrentBufferNode;
                        if (m_pCurrentBlockingBufferNode == nullptr)
                        {
                            m_pCurrentBlockingBufferNode = pTempCurrentBlockingBufferNode;
                        }
                    }

                    if (m_pCurrentBlockingBufferNode == nullptr &&
                        m_xSAXEventKeeperStatusChangeListener.is())
                    {
                        m_xSAXEventKeeperStatusChangeListener->blockingStatusChanged(false);
                    }
                }

                /*
                 * delete the ElementMark
                 */
                pElementMark = nullptr;
                removeElementMarkBuffer(nId);

                /*
                 * delete the BufferNode
                 */
                diffuse(pBufferNode);
                smashBufferNode(pBufferNode, true);
            }
        }
    }

    m_bIsReleasing = false;

    if (!m_pRootBufferNode->hasAnything() &&
        !m_pRootBufferNode->hasChildren() &&
        m_xSAXEventKeeperStatusChangeListener.is())
    {
        m_xSAXEventKeeperStatusChangeListener->bufferStatusChanged(true);
    }
}

void SAXEventKeeperImpl::markElementMarkBuffer(sal_Int32 nId)
/****** SAXEventKeeperImpl/markElementMarkBuffer *****************************
 *
 *   NAME
 *  markElementMarkBuffer -- marks an ElementMark to be released
 *
 *   SYNOPSIS
 *  markElementMarkBuffer( nId );
 *
 *   FUNCTION
 *  puts the ElementMark with the particular Id into the releasing list,
 *  checks whether the releasing process is running, if not then launch
 *  this process.
 *
 *   INPUTS
 *  nId - the Id of the ElementMark which will be released
 ******************************************************************************/
{
    m_vReleasedElementMarkBuffers.push_back( nId );
    if ( !m_bIsReleasing )
    {
        releaseElementMarkBuffer();
    }
}

sal_Int32 SAXEventKeeperImpl::createElementCollector(
    css::xml::crypto::sax::ElementMarkPriority nPriority,
    bool bModifyElement,
    const css::uno::Reference< css::xml::crypto::sax::XReferenceResolvedListener >& xReferenceResolvedListener)
/****** SAXEventKeeperImpl/createElementCollector ****************************
 *
 *   NAME
 *  createElementCollector -- creates a new ElementCollector on the
 *  incoming element.
 *
 *   SYNOPSIS
 *  nId = createElementCollector( nSecurityId, nPriority,
 *                               bModifyElement,
 *                               xReferenceResolvedListener );
 *
 *   FUNCTION
 *  allocs a new Id, then create an ElementCollector with this Id value.
 *  Add the new created ElementCollector to the new ElementCollecotor list.
 *
 *   INPUTS
 *  nPriority -     the priority of the new ElementCollector
 *  bModifyElement -whether this BufferNode will modify the content of
 *                  the corresponding element it works on
 *  xReferenceResolvedListener - the listener for the new ElementCollector.
 *
 *   RESULT
 *  nId - the Id of the new ElementCollector
 ******************************************************************************/
{
    sal_Int32 nId = m_nNextElementMarkId;
    m_nNextElementMarkId ++;

    ElementCollector* pElementCollector
        = new ElementCollector(
            nId,
            nPriority,
            bModifyElement,
            xReferenceResolvedListener);

    m_vElementMarkBuffers.push_back(
        std::unique_ptr<const ElementMark>(pElementCollector));

    /*
     * All the new EC to initial EC array.
     */
    m_vNewElementCollectors.push_back( pElementCollector );

    return nId;
}


sal_Int32 SAXEventKeeperImpl::createBlocker()
/****** SAXEventKeeperImpl/createBlocker *************************************
 *
 *   NAME
 *  createBlocker -- creates a new Blocker on the incoming element.
 *
 *   SYNOPSIS
 *  nId = createBlocker( nSecurityId );
 *
 *   RESULT
 *  nId - the Id of the new Blocker
 ******************************************************************************/
{
    sal_Int32 nId = m_nNextElementMarkId;
    m_nNextElementMarkId ++;

    OSL_ASSERT(m_pNewBlocker == nullptr);

    m_pNewBlocker = new ElementMark(css::xml::crypto::sax::ConstOfSecurityId::UNDEFINEDSECURITYID, nId);
    m_vElementMarkBuffers.push_back(
        std::unique_ptr<const ElementMark>(m_pNewBlocker));

    return nId;
}

/* XSAXEventKeeper */
sal_Int32 SAL_CALL SAXEventKeeperImpl::addElementCollector(  )
{
    return createElementCollector(
        css::xml::crypto::sax::ElementMarkPriority_AFTERMODIFY,
        false,
        nullptr);
}

void SAL_CALL SAXEventKeeperImpl::removeElementCollector( sal_Int32 id )
{
    markElementMarkBuffer(id);
}

sal_Int32 SAL_CALL SAXEventKeeperImpl::addBlocker(  )
{
    return createBlocker();
}

void SAL_CALL SAXEventKeeperImpl::removeBlocker( sal_Int32 id )
{
    markElementMarkBuffer(id);
}

sal_Bool SAL_CALL SAXEventKeeperImpl::isBlocking(  )
{
    return (m_pCurrentBlockingBufferNode != nullptr);
}

css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > SAL_CALL
    SAXEventKeeperImpl::getElement( sal_Int32 id )
{
    css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > rc;

    ElementMark* pElementMark = findElementMarkBuffer(id);
    if (pElementMark != nullptr)
    {
        rc = pElementMark->getBufferNode()->getXMLElement();
    }

    return rc;
}

void SAL_CALL SAXEventKeeperImpl::setElement(
    sal_Int32 id,
    const css::uno::Reference< css::xml::wrapper::XXMLElementWrapper >& aElement )
{
    if (aElement.is())
    {
        m_xXMLDocument->rebuildIDLink(aElement);

        ElementMark* pElementMark = findElementMarkBuffer(id);

        if (pElementMark != nullptr)
        {
            BufferNode* pBufferNode = pElementMark->getBufferNode();
            if (pBufferNode != nullptr)
            {
                const bool bIsCurrent = m_xXMLDocument->isCurrent(pBufferNode->getXMLElement());
                pBufferNode->setXMLElement(aElement);

                if (bIsCurrent)
                {
                    m_xXMLDocument->setCurrentElement(aElement);
                }
            }
        }
    }
    else
    {
        removeElementCollector( id );
    }
}

css::uno::Reference< css::xml::sax::XDocumentHandler > SAL_CALL SAXEventKeeperImpl::setNextHandler(
    const css::uno::Reference< css::xml::sax::XDocumentHandler >& xNewHandler )
{
    css::uno::Reference< css::xml::sax::XDocumentHandler > xOldHandler = m_xNextHandler;

    m_xNextHandler = xNewHandler;
    return xOldHandler;
}

OUString SAL_CALL SAXEventKeeperImpl::printBufferNodeTree()
{
    OUString rc = "ElementMarkBuffers: size = "
        + OUString::number(m_vElementMarkBuffers.size())
        + "\nCurrentBufferNode: "
        + m_xXMLDocument->getNodeName(m_pCurrentBufferNode->getXMLElement())
        + "\n" + printBufferNode(m_pRootBufferNode.get(), 0);

    return rc;
}

css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > SAL_CALL SAXEventKeeperImpl::getCurrentBlockingNode()
{
    css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > rc;

    if (m_pCurrentBlockingBufferNode != nullptr)
    {
        rc = m_pCurrentBlockingBufferNode->getXMLElement();
    }

    return rc;
}

/* XSecuritySAXEventKeeper */
sal_Int32 SAL_CALL SAXEventKeeperImpl::addSecurityElementCollector(
    css::xml::crypto::sax::ElementMarkPriority priority,
    sal_Bool modifyElement )
{
    return createElementCollector(
        priority,
        modifyElement,
        nullptr);
}

void SAL_CALL SAXEventKeeperImpl::setSecurityId( sal_Int32 id, sal_Int32 securityId )
{
    ElementMark* pElementMark = findElementMarkBuffer(id);
    if (pElementMark != nullptr)
    {
        pElementMark->setSecurityId(securityId);
    }
}


/* XReferenceResolvedBroadcaster */
void SAL_CALL SAXEventKeeperImpl::addReferenceResolvedListener(
    sal_Int32 referenceId,
    const css::uno::Reference< css::xml::crypto::sax::XReferenceResolvedListener >& listener )
{
    ElementCollector* pElementCollector = static_cast<ElementCollector*>(findElementMarkBuffer(referenceId));
    if (pElementCollector != nullptr)
    {
        pElementCollector->setReferenceResolvedListener(listener);
    }
}

void SAL_CALL SAXEventKeeperImpl::removeReferenceResolvedListener(
    sal_Int32 /*referenceId*/,
    const css::uno::Reference< css::xml::crypto::sax::XReferenceResolvedListener >&)
{
}

/* XSAXEventKeeperStatusChangeBroadcaster */
void SAL_CALL SAXEventKeeperImpl::addSAXEventKeeperStatusChangeListener(
    const css::uno::Reference< css::xml::crypto::sax::XSAXEventKeeperStatusChangeListener >& listener )
{
    m_xSAXEventKeeperStatusChangeListener = listener;
}

void SAL_CALL SAXEventKeeperImpl::removeSAXEventKeeperStatusChangeListener(
    const css::uno::Reference< css::xml::crypto::sax::XSAXEventKeeperStatusChangeListener >&)
{
}

/* XDocumentHandler */
void SAL_CALL SAXEventKeeperImpl::startDocument(  )
{
    if ( m_xNextHandler.is())
    {
        m_xNextHandler->startDocument();
    }
}

void SAL_CALL SAXEventKeeperImpl::endDocument(  )
{
    if ( m_xNextHandler.is())
    {
        m_xNextHandler->endDocument();
    }
}

void SAL_CALL SAXEventKeeperImpl::startElement(
    const OUString& aName,
    const css::uno::Reference< css::xml::sax::XAttributeList >& xAttribs )
{
    /*
     * If there is a following handler and no blocking now, then
     * forward this event
     */
    if ((m_pCurrentBlockingBufferNode == nullptr) &&
        (m_xNextHandler.is()) &&
        (!m_bIsForwarding) &&
        (m_pNewBlocker == nullptr))
    {
        m_xNextHandler->startElement(aName, xAttribs);
    }
    /*
     * If not forwarding, buffer this startElement.
     */
    if (!m_bIsForwarding)
    {
        sal_Int32 nLength = xAttribs->getLength();
        css::uno::Sequence< css::xml::csax::XMLAttribute > aAttributes (nLength);
        auto aAttributesRange = asNonConstRange(aAttributes);

        for ( int i = 0; i<nLength; ++i )
        {
            aAttributesRange[i].sName = xAttribs->getNameByIndex(static_cast<short>(i));
            aAttributesRange[i].sValue =xAttribs->getValueByIndex(static_cast<short>(i));
        }

        m_xCompressedDocumentHandler->compressedStartElement(aName, aAttributes);
    }

    BufferNode* pBufferNode = addNewElementMarkBuffers();
    if (pBufferNode != nullptr)
    {
        setCurrentBufferNode(pBufferNode);
    }
}

void SAL_CALL SAXEventKeeperImpl::endElement( const OUString& aName )
{
    const bool bIsCurrent = m_xXMLDocument->isCurrent(m_pCurrentBufferNode->getXMLElement());

    /*
     * If there is a following handler and no blocking now, then
     * forward this event
     */
    if ((m_pCurrentBlockingBufferNode == nullptr) &&
        (m_xNextHandler.is()) &&
        (!m_bIsForwarding))
    {
        m_xNextHandler->endElement(aName);
    }

    if ((m_pCurrentBlockingBufferNode != nullptr) ||
        (m_pCurrentBufferNode != m_pRootBufferNode.get()) ||
        (!m_xXMLDocument->isCurrentElementEmpty()))
    {
        if (!m_bIsForwarding)
        {
            m_xCompressedDocumentHandler->compressedEndElement(aName);
        }

        /*
        * If the current buffer node has not notified yet, and
        * the current buffer node is waiting for the current element,
        * then let it notify.
        */
        if (bIsCurrent && (m_pCurrentBufferNode != m_pRootBufferNode.get()))
        {
            BufferNode* pOldCurrentBufferNode = m_pCurrentBufferNode;
            m_pCurrentBufferNode = const_cast<BufferNode*>(m_pCurrentBufferNode->getParent());

            pOldCurrentBufferNode->setReceivedAll();

            if ((m_pCurrentBufferNode == m_pRootBufferNode.get()) &&
                m_xSAXEventKeeperStatusChangeListener.is())
            {
                m_xSAXEventKeeperStatusChangeListener->collectionStatusChanged(false);
            }
        }
    }
    else
    {
        if (!m_bIsForwarding)
        {
            m_xXMLDocument->removeCurrentElement();
        }
    }
}

void SAL_CALL SAXEventKeeperImpl::characters( const OUString& aChars )
{
    if (m_bIsForwarding)
        return;

    if ((m_pCurrentBlockingBufferNode == nullptr) && m_xNextHandler.is())
    {
        m_xNextHandler->characters(aChars);
    }

    if ((m_pCurrentBlockingBufferNode != nullptr) ||
        (m_pCurrentBufferNode != m_pRootBufferNode.get()))
    {
        m_xCompressedDocumentHandler->compressedCharacters(aChars);
    }
}

void SAL_CALL SAXEventKeeperImpl::ignorableWhitespace( const OUString& aWhitespaces )
{
    characters( aWhitespaces );
}

void SAL_CALL SAXEventKeeperImpl::processingInstruction(
    const OUString& aTarget, const OUString& aData )
{
    if (m_bIsForwarding)
        return;

    if ((m_pCurrentBlockingBufferNode == nullptr) && m_xNextHandler.is())
    {
        m_xNextHandler->processingInstruction(aTarget, aData);
    }

    if ((m_pCurrentBlockingBufferNode != nullptr) ||
        (m_pCurrentBufferNode != m_pRootBufferNode.get()))
    {
        m_xCompressedDocumentHandler->compressedProcessingInstruction(aTarget, aData);
    }
}

void SAL_CALL SAXEventKeeperImpl::setDocumentLocator( const css::uno::Reference< css::xml::sax::XLocator >&)
{
}

/* XInitialization */
void SAL_CALL SAXEventKeeperImpl::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    OSL_ASSERT(aArguments.getLength() == 1);

    aArguments[0] >>= m_xXMLDocument;
    m_xDocumentHandler.set( m_xXMLDocument, css::uno::UNO_QUERY );
    m_xCompressedDocumentHandler.set( m_xXMLDocument, css::uno::UNO_QUERY );

    m_pRootBufferNode.reset( new BufferNode(m_xXMLDocument->getCurrentElement()) );
    m_pCurrentBufferNode = m_pRootBufferNode.get();
}

OUString SAXEventKeeperImpl_getImplementationName ()
{
    return u"com.sun.star.xml.security.framework.SAXEventKeeperImpl"_ustr;
}

css::uno::Sequence< OUString > SAXEventKeeperImpl_getSupportedServiceNames(  )
{
    return { u"com.sun.star.xml.crypto.sax.SAXEventKeeper"_ustr };
}

/* XServiceInfo */
OUString SAL_CALL SAXEventKeeperImpl::getImplementationName(  )
{
    return SAXEventKeeperImpl_getImplementationName();
}

sal_Bool SAL_CALL SAXEventKeeperImpl::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL SAXEventKeeperImpl::getSupportedServiceNames(  )
{
    return SAXEventKeeperImpl_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
