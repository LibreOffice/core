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


#include "framework/saxeventkeeperimpl.hxx"
#include "buffernode.hxx"
#include "elementmark.hxx"
#include "elementcollector.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/crypto/sax/ConstOfSecurityId.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxcsax = com::sun::star::xml::csax;
namespace cssxw = com::sun::star::xml::wrapper;
namespace cssxs = com::sun::star::xml::sax;

#define IMPLEMENTATION_NAME "com.sun.star.xml.security.framework.SAXEventKeeperImpl"

SAXEventKeeperImpl::SAXEventKeeperImpl( )
    :m_pRootBufferNode(nullptr),
     m_pCurrentBufferNode(nullptr),
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
    if (m_pRootBufferNode != nullptr)
    {
        m_pRootBufferNode->freeAllChildren();
        delete m_pRootBufferNode;
    }

    m_pRootBufferNode = m_pCurrentBufferNode = m_pCurrentBlockingBufferNode = nullptr;

    /*
     * delete all unfreed ElementMarks
     */
    m_vNewElementCollectors.clear();
    m_pNewBlocker = nullptr;

    std::vector< const ElementMark* >::const_iterator ii = m_vElementMarkBuffers.begin();
    for( ; ii != m_vElementMarkBuffers.end(); ++ii )
    {
        delete (*ii);
    }
    m_vElementMarkBuffers.clear();
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
    if (pBufferNode != m_pCurrentBufferNode)
    {
        if ( m_pCurrentBufferNode == m_pRootBufferNode &&
             m_xSAXEventKeeperStatusChangeListener.is())
        {
            m_xSAXEventKeeperStatusChangeListener->collectionStatusChanged(true);
        }

        if (pBufferNode->getParent() == nullptr)
        {
            m_pCurrentBufferNode->addChild(pBufferNode);
            pBufferNode->setParent(m_pCurrentBufferNode);
        }

        m_pCurrentBufferNode = pBufferNode;
    }
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

        if (!m_vNewElementCollectors.empty())
        {
            std::vector< const ElementCollector* >::const_iterator ii = m_vNewElementCollectors.begin();

            for( ; ii != m_vNewElementCollectors.end(); ++ii )
            {
                pBufferNode->addElementCollector(*ii);
            }

            m_vNewElementCollectors.clear();
        }
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

    std::vector< const ElementMark* >::const_iterator ii = m_vElementMarkBuffers.begin();

    for( ; ii != m_vElementMarkBuffers.end(); ++ii )
    {
        if ( nId == (*ii)->getBufferId())
        {
            pElementMark = const_cast<ElementMark*>(*ii);
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
    std::vector< const ElementMark* >::iterator ii = m_vElementMarkBuffers.begin();

    for( ; ii != m_vElementMarkBuffers.end(); ++ii )
    {
        if ( nId == (*ii)->getBufferId())
        {
            /*
             * checks whether this ElementMark still in the new ElementCollect array
             */
            std::vector< const ElementCollector* >::iterator jj = m_vNewElementCollectors.begin();
            for( ; jj != m_vNewElementCollectors.end(); ++jj )
            {
                if ((*ii) == (*jj))
                {
                    m_vNewElementCollectors.erase(jj);
                    break;
                }
            }

            /*
             * checks whether this ElementMark is the new Blocker
             */
            if ((*ii) == m_pNewBlocker)
            {
                m_pNewBlocker = nullptr;
            }

            /*
             * destroy the ElementMark
             */
            delete (*ii);

            m_vElementMarkBuffers.erase( ii );
            break;
        }
    }
}

OUString SAXEventKeeperImpl::printBufferNode(
    BufferNode* pBufferNode, sal_Int32 nIndent) const
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
    OUString rc;

    for ( int i=0; i<nIndent; ++i )
    {
        rc += " ";
    }

    if (pBufferNode == m_pCurrentBufferNode)
    {
        rc += "[%]";
    }

    if (pBufferNode == m_pCurrentBlockingBufferNode)
    {
        rc += "[B]";
    }

    rc += " " + m_xXMLDocument->getNodeName(pBufferNode->getXMLElement());

    BufferNode* pParent = const_cast<BufferNode*>(pBufferNode->getParent());
    if (pParent != nullptr)
    {
        rc += "[" + m_xXMLDocument->getNodeName(pParent->getXMLElement()) + "]";
    }

    rc += ":EC=" + pBufferNode->printChildren() + " BR=";

    ElementMark * pBlocker = pBufferNode->getBlocker();
    if (pBlocker != nullptr)
    {
        rc += OUString::number( pBlocker->getBufferId() ) + "(SecId="
            + OUString::number( pBlocker->getSecurityId() ) + ") ";
    }
    rc += "\n";

    std::vector< const BufferNode* >* vChildren = pBufferNode->getChildren();
    std::vector< const BufferNode* >::const_iterator jj = vChildren->begin();
    for( ; jj != vChildren->end(); ++jj )
    {
        rc += printBufferNode(const_cast<BufferNode *>(*jj), nIndent+4);
    }

    delete vChildren;

    return rc;
}

cssu::Sequence< cssu::Reference< cssxw::XXMLElementWrapper > >
    SAXEventKeeperImpl::collectChildWorkingElement(BufferNode* pBufferNode)
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
    std::vector< const BufferNode* >* vChildren = pBufferNode->getChildren();

    cssu::Sequence < cssu::Reference<
        cssxw::XXMLElementWrapper > > aChildrenCollection ( vChildren->size());

    std::vector< const BufferNode* >::const_iterator ii = vChildren->begin();

    sal_Int32 nIndex = 0;
    for( ; ii != vChildren->end(); ++ii )
    {
        aChildrenCollection[nIndex] = (*ii)->getXMLElement();
        nIndex++;
    }

    delete vChildren;

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
 *  operation can delete all useless bufferred SAX events which are only
 *  needed by the Blocker to be deleted.
 ******************************************************************************/
{
    if (!pBufferNode->hasAnything())
    {
        BufferNode* pParent = const_cast<BufferNode*>(pBufferNode->getParent());

            /*
             * delete the XML data
             */
        if (pParent == m_pRootBufferNode)
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
                cssu::Sequence< cssu::Reference< cssxw::XXMLElementWrapper > >
                    aChildElements = collectChildWorkingElement(m_pRootBufferNode);

                    /*
                     * the clearUselessData only clearup the content in the
                     * node, not the node itself.
                     */
                m_xXMLDocument->clearUselessData(m_pRootBufferNode->getXMLElement(),
                    aChildElements,
                    bIsNotBlocking?(nullptr):
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
                cssu::Sequence< cssu::Reference< cssxw::XXMLElementWrapper > >
                    aChildElements = collectChildWorkingElement(pBufferNode);

                    /*
                     * the clearUselessData only clearup the content in the
                     * node, not the node itself.
                     */
                m_xXMLDocument->clearUselessData(pBufferNode->getXMLElement(),
                    aChildElements,
                    bIsBlockInside?(m_pCurrentBlockingBufferNode->getXMLElement()):
                               (nullptr));

                    /*
                     * remove the node if it is empty, then if its parent is also
                     * empty, remove it, then if the next parent is also empty,
                     * remove it,..., until parent become null.
                     */
                m_xXMLDocument->collapse( pBufferNode->getXMLElement() );
            }
        }

        sal_Int32 nIndex = pParent->indexOfChild(pBufferNode);

        std::vector< const BufferNode* >* vChildren = pBufferNode->getChildren();
        pParent->removeChild(pBufferNode);
        pBufferNode->setParent(nullptr);

        std::vector< const BufferNode * >::const_iterator ii = vChildren->begin();
        for( ; ii != vChildren->end(); ++ii )
        {
            const_cast<BufferNode *>(*ii)->setParent(pParent);
            pParent->addChild(*ii, nIndex);
            nIndex++;
        }

        delete vChildren;

        /*
         * delete the BufferNode
         */
        delete pBufferNode;
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
        std::vector< sal_Int32 >::iterator pId = m_vReleasedElementMarkBuffers.begin();
        sal_Int32 nId = *pId;
        m_vReleasedElementMarkBuffers.erase( pId );

        ElementMark* pElementMark = findElementMarkBuffer(nId);

        if (pElementMark != nullptr)
        {
            if (cssxc::sax::ElementMarkType_ELEMENTCOLLECTOR
                == pElementMark->getType())
            /*
             * it is a EC
             */
            {
                ElementCollector* pElementCollector = static_cast<ElementCollector*>(pElementMark);

                cssxc::sax::ElementMarkPriority nPriority = pElementCollector->getPriority();
                /*
                     * Delete the EC from the buffer node.
                     */
                BufferNode* pBufferNode = pElementCollector->getBufferNode();
                pBufferNode->removeElementCollector(pElementCollector);

                if ( nPriority == cssxc::sax::ElementMarkPriority_BEFOREMODIFY)
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
    cssxc::sax::ElementMarkPriority nPriority,
    bool bModifyElement,
    const cssu::Reference< cssxc::sax::XReferenceResolvedListener >& xReferenceResolvedListener)
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
            cssxc::sax::ConstOfSecurityId::UNDEFINEDSECURITYID,
            nId,
            nPriority,
            bModifyElement,
            xReferenceResolvedListener);

    m_vElementMarkBuffers.push_back( pElementCollector );

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

    m_pNewBlocker = new ElementMark(cssxc::sax::ConstOfSecurityId::UNDEFINEDSECURITYID, nId);
    m_vElementMarkBuffers.push_back( m_pNewBlocker );

    return nId;
}

/* XSAXEventKeeper */
sal_Int32 SAL_CALL SAXEventKeeperImpl::addElementCollector(  )
{
    return createElementCollector(
        cssxc::sax::ElementMarkPriority_AFTERMODIFY,
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

cssu::Reference< cssxw::XXMLElementWrapper > SAL_CALL
    SAXEventKeeperImpl::getElement( sal_Int32 id )
{
    cssu::Reference< cssxw::XXMLElementWrapper > rc;

    ElementMark* pElementMark = findElementMarkBuffer(id);
    if (pElementMark != nullptr)
    {
        rc = pElementMark->getBufferNode()->getXMLElement();
    }

    return rc;
}

void SAL_CALL SAXEventKeeperImpl::setElement(
    sal_Int32 id,
    const cssu::Reference< cssxw::XXMLElementWrapper >& aElement )
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
                    bool bIsCurrent = m_xXMLDocument->isCurrent(pBufferNode->getXMLElement());
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

cssu::Reference< cssxs::XDocumentHandler > SAL_CALL SAXEventKeeperImpl::setNextHandler(
    const cssu::Reference< cssxs::XDocumentHandler >& xNewHandler )
{
    cssu::Reference< cssxs::XDocumentHandler > xOldHandler = m_xNextHandler;

    m_xNextHandler = xNewHandler;
    return xOldHandler;
}

OUString SAL_CALL SAXEventKeeperImpl::printBufferNodeTree()
{
    OUString rc;

    rc += "ElementMarkBuffers: size = "
        + OUString::number(m_vElementMarkBuffers.size())
        + "\nCurrentBufferNode: "
        + m_xXMLDocument->getNodeName(m_pCurrentBufferNode->getXMLElement())
        + "\n" + printBufferNode(m_pRootBufferNode, 0);

    return rc;
}

cssu::Reference< cssxw::XXMLElementWrapper > SAL_CALL SAXEventKeeperImpl::getCurrentBlockingNode()
{
    cssu::Reference< cssxw::XXMLElementWrapper > rc;

    if (m_pCurrentBlockingBufferNode != nullptr)
    {
        rc = m_pCurrentBlockingBufferNode->getXMLElement();
    }

    return rc;
}

/* XSecuritySAXEventKeeper */
sal_Int32 SAL_CALL SAXEventKeeperImpl::addSecurityElementCollector(
    cssxc::sax::ElementMarkPriority priority,
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
    const cssu::Reference< cssxc::sax::XReferenceResolvedListener >& listener )
{
    ElementCollector* pElementCollector = static_cast<ElementCollector*>(findElementMarkBuffer(referenceId));
    if (pElementCollector != nullptr)
    {
        pElementCollector->setReferenceResolvedListener(listener);
    }
}

void SAL_CALL SAXEventKeeperImpl::removeReferenceResolvedListener(
    sal_Int32 /*referenceId*/,
    const cssu::Reference< cssxc::sax::XReferenceResolvedListener >&)
{
}

/* XSAXEventKeeperStatusChangeBroadcaster */
void SAL_CALL SAXEventKeeperImpl::addSAXEventKeeperStatusChangeListener(
    const cssu::Reference< cssxc::sax::XSAXEventKeeperStatusChangeListener >& listener )
{
    m_xSAXEventKeeperStatusChangeListener = listener;
}

void SAL_CALL SAXEventKeeperImpl::removeSAXEventKeeperStatusChangeListener(
    const cssu::Reference< cssxc::sax::XSAXEventKeeperStatusChangeListener >&)
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
    const cssu::Reference< cssxs::XAttributeList >& xAttribs )
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
        cssu::Sequence< cssxcsax::XMLAttribute > aAttributes (nLength);

        for ( int i = 0; i<nLength; ++i )
        {
            aAttributes[i].sName = xAttribs->getNameByIndex((short)i);
            aAttributes[i].sValue =xAttribs->getValueByIndex((short)i);
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
        bool bIsCurrent = m_xXMLDocument->isCurrent(m_pCurrentBufferNode->getXMLElement());

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
        (m_pCurrentBufferNode != m_pRootBufferNode) ||
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
           if (bIsCurrent && (m_pCurrentBufferNode != m_pRootBufferNode))
        {
            BufferNode* pOldCurrentBufferNode = m_pCurrentBufferNode;
            m_pCurrentBufferNode = const_cast<BufferNode*>(m_pCurrentBufferNode->getParent());

            pOldCurrentBufferNode->setReceivedAll();

            if ((m_pCurrentBufferNode == m_pRootBufferNode) &&
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
    if (!m_bIsForwarding)
    {
        if ((m_pCurrentBlockingBufferNode == nullptr) && m_xNextHandler.is())
        {
            m_xNextHandler->characters(aChars);
        }

        if ((m_pCurrentBlockingBufferNode != nullptr) ||
            (m_pCurrentBufferNode != m_pRootBufferNode))
        {
            m_xCompressedDocumentHandler->compressedCharacters(aChars);
        }
    }
}

void SAL_CALL SAXEventKeeperImpl::ignorableWhitespace( const OUString& aWhitespaces )
{
    characters( aWhitespaces );
}

void SAL_CALL SAXEventKeeperImpl::processingInstruction(
    const OUString& aTarget, const OUString& aData )
{
    if (!m_bIsForwarding)
    {
        if ((m_pCurrentBlockingBufferNode == nullptr) && m_xNextHandler.is())
        {
            m_xNextHandler->processingInstruction(aTarget, aData);
        }

        if ((m_pCurrentBlockingBufferNode != nullptr) ||
            (m_pCurrentBufferNode != m_pRootBufferNode))
        {
            m_xCompressedDocumentHandler->compressedProcessingInstruction(aTarget, aData);
        }
    }
}

void SAL_CALL SAXEventKeeperImpl::setDocumentLocator( const cssu::Reference< cssxs::XLocator >&)
{
}

/* XInitialization */
void SAL_CALL SAXEventKeeperImpl::initialize( const cssu::Sequence< cssu::Any >& aArguments )
{
    OSL_ASSERT(aArguments.getLength() == 1);

    aArguments[0] >>= m_xXMLDocument;
    m_xDocumentHandler.set( m_xXMLDocument, cssu::UNO_QUERY );
    m_xCompressedDocumentHandler.set( m_xXMLDocument, cssu::UNO_QUERY );

    m_pRootBufferNode = new BufferNode(m_xXMLDocument->getCurrentElement());
    m_pCurrentBufferNode = m_pRootBufferNode;
}

OUString SAXEventKeeperImpl_getImplementationName ()
{
    return OUString ( IMPLEMENTATION_NAME );
}

cssu::Sequence< OUString > SAL_CALL SAXEventKeeperImpl_getSupportedServiceNames(  )
{
    cssu::Sequence<OUString> aRet { "com.sun.star.xml.crypto.sax.SAXEventKeeper" };
    return aRet;
}

cssu::Reference< cssu::XInterface > SAL_CALL SAXEventKeeperImpl_createInstance(
    const cssu::Reference< cssl::XMultiServiceFactory > &)
{
    return static_cast<cppu::OWeakObject*>(new SAXEventKeeperImpl());
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

cssu::Sequence< OUString > SAL_CALL SAXEventKeeperImpl::getSupportedServiceNames(  )
{
    return SAXEventKeeperImpl_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
