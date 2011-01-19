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

#include "nodelist.hxx"
#include "../dom/document.hxx"

namespace XPath
{
    CNodeList::CNodeList(
                ::rtl::Reference<DOM::CDocument> const& pDocument,
                boost::shared_ptr<xmlXPathObject> const& rxpathObj)
        : m_pDocument(pDocument)
        , m_pNodeSet(0)
    {
        if (rxpathObj != NULL && rxpathObj->type == XPATH_NODESET)
        {
            m_pNodeSet = rxpathObj->nodesetval;
            m_pXPathObj = rxpathObj;
        }
    }

    /**
    The number of nodes in the list.
    */
    sal_Int32 SAL_CALL CNodeList::getLength() throw (RuntimeException)
    {
        sal_Int32 value = 0;
        if (m_pNodeSet != NULL)
            value = xmlXPathNodeSetGetLength(m_pNodeSet);
        return value;
    }

    /**
    Returns the indexth item in the collection.
    */
    Reference< XNode > SAL_CALL CNodeList::item(sal_Int32 index)
        throw (RuntimeException)
    {
        if (0 == m_pNodeSet) {
            return 0;
        }
        Reference< XNode > const xNode(
            DOM::CNode::getCNode(xmlXPathNodeSetItem(m_pNodeSet, index)).get());
        return xNode;
    }
}

