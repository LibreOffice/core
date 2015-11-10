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

    if (m_pBufferNode != nullptr)
    {
        m_pBufferNode->addElementCollector(pClonedOne);
    }

    return pClonedOne;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
