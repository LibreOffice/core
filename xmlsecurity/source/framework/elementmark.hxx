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

#ifndef INCLUDED_XMLSECURITY_SOURCE_FRAMEWORK_ELEMENTMARK_HXX
#define INCLUDED_XMLSECURITY_SOURCE_FRAMEWORK_ELEMENTMARK_HXX

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
 ******************************************************************************/
{
protected:
    /* the BufferNode maintained by this object */
    BufferNode* m_pBufferNode;

    /* the security Id */
    sal_Int32 m_nSecurityId;

    /* the buffer Id */
    sal_Int32 const m_nBufferId;

    /*
     * the type value, is one of following values:
     * TYPEOFELEMENTMARK - the default value, represents an blocker if
     *                     not changed
     * TYPEOFELEMENTCOLLECTOR - represents an ElementCollector
     */
    css::xml::crypto::sax::ElementMarkType m_type;

public:
    ElementMark(sal_Int32 nSecurityId, sal_Int32 nBufferId);
    virtual ~ElementMark() {};

    BufferNode* getBufferNode() const { return m_pBufferNode;}
    void setBufferNode(const BufferNode* pBufferNode);
    sal_Int32 getSecurityId() const { return m_nSecurityId;}
    void setSecurityId(sal_Int32 nSecurityId);
    css::xml::crypto::sax::ElementMarkType getType() const { return m_type;}
    sal_Int32 getBufferId() const { return m_nBufferId;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
