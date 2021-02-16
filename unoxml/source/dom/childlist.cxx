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

#include "childlist.hxx"

#include <libxml/tree.h>

#include <node.hxx>
#include "document.hxx"

using namespace css::uno;
using namespace css::xml::dom;

namespace DOM
{
    CChildList::CChildList(::rtl::Reference<CNode> const& pBase,
                ::osl::Mutex & rMutex)
        : m_pNode(pBase)
        , m_rMutex(rMutex)
    {
    }

    /**
    The number of nodes in the list.
    */
    sal_Int32 SAL_CALL CChildList::getLength()
    {
        ::osl::MutexGuard const g(m_rMutex);

        sal_Int32 length = 0;
        if (m_pNode != nullptr)
        {
            xmlNodePtr cur = m_pNode->GetNodePtr();
            if (nullptr != cur) {
                cur = cur->children;
            }
            while (cur != nullptr)
            {
                length++;
                cur = cur->next;
            }
        }
        return length;

    }
    /**
    Returns the indexth item in the collection.
    */
    Reference< XNode > SAL_CALL CChildList::item(sal_Int32 index)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (m_pNode != nullptr)
        {
            xmlNodePtr cur = m_pNode->GetNodePtr();
            if (nullptr != cur) {
                cur = cur->children;
            }
            while (cur != nullptr)
            {
                if (index-- == 0) {
                    return m_pNode->GetOwnerDocument().GetCNode(cur);
                }
                cur = cur->next;
            }
        }
        return nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
