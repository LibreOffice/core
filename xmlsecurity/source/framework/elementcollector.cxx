/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlsecurity.hxx"

#include "elementmark.hxx"
#include "elementcollector.hxx"
#include "buffernode.hxx"
#include <com/sun/star/xml/crypto/sax/ConstOfSecurityId.hpp>

namespace cssu = com::sun::star::uno;
namespace cssxc = com::sun::star::xml::crypto;

ElementCollector::ElementCollector(
    sal_Int32 nSecurityId,
    sal_Int32 nBufferId,
    cssxc::sax::ElementMarkPriority nPriority,
    bool bToModify,
    const com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::sax::XReferenceResolvedListener >&
        xReferenceResolvedListener)
    :ElementMark(nSecurityId, nBufferId),
     m_nPriority(nPriority),
     m_bToModify(bToModify),
      m_bAbleToNotify(false),
      m_bNotified(false),
     m_xReferenceResolvedListener(xReferenceResolvedListener)
/****** ElementCollector/ElementCollector *************************************
 *
 *   NAME
 *  ElementCollector -- constructor method
 *
 *   SYNOPSIS
 *  ElementCollector(nSecurityId, nBufferId, nPriority, bToModify
 *                   xReferenceResolvedListener);
 *
 *   FUNCTION
 *  construct an ElementCollector object.
 *
 *   INPUTS
 *  nSecurityId -   represents which security entity the buffer node is
 *          related with. Either a signature or an encryption is
 *          a security entity.
 *  nBufferId - the id of the element bufferred in the document
 *          wrapper component. The document wrapper component
 *          uses this id to search the particular bufferred
 *          element.
 *  nPriority - the priority value. ElementCollector with lower
 *          priority value can't notify until all ElementCollectors
 *          with higher priority value have notified.
 *  bToModify - A flag representing whether this ElementCollector
 *          notification will cause the modification of its working
 *                  element.
 *  xReferenceResolvedListener
 *            - the listener that this ElementCollector notifies to.
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    m_type = cssxc::sax::ElementMarkType_ELEMENTCOLLECTOR;
}

/*
bool ElementCollector::isInternalNotificationSuppressed() const
{
    return m_bInternalNotificationSuppressed;
}
*/

cssxc::sax::ElementMarkPriority ElementCollector::getPriority() const
{
    return m_nPriority;
}

bool ElementCollector::getModify() const
{
    return m_bToModify;
}

void ElementCollector::notifyListener()
/****** ElementCollector/notifyListener ***************************************
 *
 *   NAME
 *  notifyListener -- enable the ability to notify the listener
 *
 *   SYNOPSIS
 *  notifyListener();
 *
 *   FUNCTION
 *  enable the ability to notify the listener and try to notify then.
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
    m_bAbleToNotify = true;
    doNotify();
}

bool ElementCollector::isAbleToNotify() const
{
    return m_bAbleToNotify;
}

void ElementCollector::setReferenceResolvedListener(
    const cssu::Reference< cssxc::sax::XReferenceResolvedListener >& xReferenceResolvedListener)
/****** ElementCollector/setReferenceResolvedListener *************************
 *
 *   NAME
 *  setReferenceResolvedListener -- configures a listener for the buffer
 *  node in this object
 *
 *   SYNOPSIS
 *  setReferenceResolvedListener(xReferenceResolvedListener);
 *
 *   FUNCTION
 *  configures a new listener and try to notify then.
 *
 *   INPUTS
 *  xReferenceResolvedListener - the new listener
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    m_xReferenceResolvedListener = xReferenceResolvedListener;
    doNotify();
}

void ElementCollector::setSecurityId(sal_Int32 nSecurityId)
/****** ElementCollector/setSecurityId ****************************************
 *
 *   NAME
 *  setSecurityId -- configures the security Id of the buffer node
 *
 *   SYNOPSIS
 *  setSecurityId(nSecurityId);
 *
 *   FUNCTION
 *  configures the security Id and try to notify then
 *
 *   INPUTS
 *  nSecurityId - the security Id
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    m_nSecurityId = nSecurityId;
    doNotify();
}

void ElementCollector::doNotify()
/****** ElementCollector/doNotify *********************************************
 *
 *   NAME
 *  doNotify -- tries to notify the listener
 *
 *   SYNOPSIS
 *  doNotify();
 *
 *   FUNCTION
 *  notifies the listener when all below conditions are satisfied:
 *  the listener has not been notified;
 *  the notify right is granted;
 *  the listener has already been configured;
 *  the security id has already been configure
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
    if (!m_bNotified &&
        m_bAbleToNotify &&
        m_xReferenceResolvedListener.is() &&
        m_nSecurityId != cssxc::sax::ConstOfSecurityId::UNDEFINEDSECURITYID)
    {
        m_bNotified = true;
        m_xReferenceResolvedListener->referenceResolved(m_nBufferId);
    }
}

ElementCollector* ElementCollector::clone(
    sal_Int32 nBufferId,
    cssxc::sax::ElementMarkPriority nPriority ) const
/****** ElementCollector/clone ************************************************
 *
 *   NAME
 *  clone -- duplicates this ElementCollector object
 *
 *   SYNOPSIS
 *  cloned = clone(nBufferId, nPriority);
 *
 *   FUNCTION
 *  duplicates this ElementCollector object with new buffer Id, priority.
 *
 *   INPUTS
 *  nBufferId - the buffer node's Id
 *  nPriority - the priority
 *
 *   RESULT
 *  clone -     a new ElementCollector
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    ElementCollector* pClonedOne
        = new ElementCollector(m_nSecurityId,
                       nBufferId, nPriority, m_bToModify,
                       m_xReferenceResolvedListener);

    if (m_bAbleToNotify)
    {
        pClonedOne->notifyListener();
    }

    if (m_pBufferNode != NULL)
    {
        m_pBufferNode->addElementCollector(pClonedOne);
    }

    return pClonedOne;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
