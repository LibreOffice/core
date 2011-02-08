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
