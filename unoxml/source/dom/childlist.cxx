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

#include "childlist.hxx"
namespace DOM
{
    CChildList::CChildList(CNode const& rBase)
        : m_pNode(rBase.m_aNodePtr)
    {
    }

    /**
    The number of nodes in the list.
    */
    sal_Int32 SAL_CALL CChildList::getLength() throw (RuntimeException)
    {
        sal_Int32 length = 0;
        if (m_pNode != NULL)
        {
            xmlNodePtr cur = m_pNode->children;
            while (cur != NULL)
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
    Reference< XNode > SAL_CALL CChildList::item(sal_Int32 index) throw (RuntimeException)
    {
        Reference< XNode >aNode;
        if (m_pNode != NULL)
        {
            xmlNodePtr cur = m_pNode->children;
            while (cur != NULL)
            {
                if (index-- == 0) {
                    aNode = Reference< XNode >(CNode::getCNode(cur).get());
                    break;
                }
                cur = cur->next;
            }
        }
        return aNode;
    }
}
