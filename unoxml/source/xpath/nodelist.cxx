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

#include <nodelist.hxx>

#include "../dom/document.hxx"

using namespace css::uno;
using namespace css::xml::dom;

namespace XPath
{
    CNodeList::CNodeList(
                ::rtl::Reference<DOM::CDocument> const& pDocument,
                ::osl::Mutex & rMutex,
                std::shared_ptr<xmlXPathObject> const& rxpathObj)
        : m_pDocument(pDocument)
        , m_rMutex(rMutex)
        , m_pNodeSet(0)
    {
        if (rxpathObj != 0 && rxpathObj->type == XPATH_NODESET)
        {
            m_pNodeSet = rxpathObj->nodesetval;
            m_pXPathObj = rxpathObj;
        }
    }

    /**
    The number of nodes in the list.
    */
    sal_Int32 SAL_CALL CNodeList::getLength() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_rMutex);

        sal_Int32 value = 0;
        if (m_pNodeSet != NULL)
            value = xmlXPathNodeSetGetLength(m_pNodeSet);
        return value;
    }

    /**
    Returns the indexth item in the collection.
    */
    Reference< XNode > SAL_CALL CNodeList::item(sal_Int32 index)
        throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (0 == m_pNodeSet) {
            return 0;
        }
        xmlNodePtr const pNode = xmlXPathNodeSetItem(m_pNodeSet, index);
        Reference< XNode > const xNode(m_pDocument->GetCNode(pNode).get());
        return xNode;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
