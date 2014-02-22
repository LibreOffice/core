/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "elementmark.hxx"
#include "buffernode.hxx"

namespace cssxc = com::sun::star::xml::crypto;

ElementMark::ElementMark(sal_Int32 nSecurityId, sal_Int32 nBufferId)
    :m_pBufferNode(NULL),
     m_nSecurityId(nSecurityId),
     m_nBufferId(nBufferId),
     m_type(cssxc::sax::ElementMarkType_ELEMENTMARK)
/****** ElementMark/ElementMark ***********************************************
 *
 *   NAME
 *  ElementMark -- constructor method
 *
 *   SYNOPSIS
 *  ElementMark(nSecurityId, nBufferId);
 *
 *   FUNCTION
 *  construct an ElementMark object.
 *
 *   INPUTS
 *  nSecurityId -   represents which security entity the buffer node is
 *          related with. Either a signature or an encryption is
 *          a security entity.
 *  nBufferId - the id of the element bufferred in the document
 *          wrapper component. The document wrapper component
 *          uses this id to search the particular bufferred
 *          element.
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
}

BufferNode* ElementMark::getBufferNode() const
{
    return m_pBufferNode;
}

void ElementMark::setBufferNode(const BufferNode* pBufferNode)
{
    m_pBufferNode = (BufferNode*)pBufferNode;
}

sal_Int32 ElementMark::getSecurityId() const
{
    return m_nSecurityId;
}

void ElementMark::setSecurityId(sal_Int32 nSecurityId)
{
    m_nSecurityId = nSecurityId;
}

com::sun::star::xml::crypto::sax::ElementMarkType ElementMark::getType() const
{
    return m_type;
}

sal_Int32 ElementMark::getBufferId() const
{
    return m_nBufferId;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
