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

#ifndef _ELEMENTMARK_HXX
#define _ELEMENTMARK_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/crypto/sax/ElementMarkType.hpp>

class BufferNode;

class ElementMark
/****** elementmark.hxx/CLASS ElementMark *************************************
 *
 *   NAME
 *  ElementMark -- Class to manipulate an element mark
 *
 *   FUNCTION
 *  This class maintains the security id, buffer id and its type for a
 *  buffer node.
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
protected:
    /* the BufferNode maintained by this object */
    BufferNode* m_pBufferNode;

    /* the security Id */
    sal_Int32 m_nSecurityId;

    /* the buffer Id */
    sal_Int32 m_nBufferId;

    /*
     * the type value, is one of following values:
     * TYPEOFELEMENTMARK - the default value, represents an blocker if
     *                     not changed
     * TYPEOFELEMENTCOLLECTOR - represents an ElementCollector
     */
    com::sun::star::xml::crypto::sax::ElementMarkType m_type;

public:
    ElementMark(sal_Int32 nSecurityId, sal_Int32 nBufferId);
    virtual ~ElementMark() {};

    BufferNode* getBufferNode() const;
    void setBufferNode(const BufferNode* pBufferNode);
    sal_Int32 getSecurityId() const;
    void setSecurityId(sal_Int32 nSecurityId);
    com::sun::star::xml::crypto::sax::ElementMarkType getType() const;
    sal_Int32 getBufferId() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
