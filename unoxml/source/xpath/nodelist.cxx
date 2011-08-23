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

#include "nodelist.hxx"
#include "../dom/node.hxx"

namespace XPath
{
    CNodeList::CNodeList(boost::shared_ptr<xmlXPathObject>& rxpathObj, const Reference< XNode >& contextNode)
        : m_xContextNode(contextNode)
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
    Reference< XNode > SAL_CALL CNodeList::item(sal_Int32 index) throw (RuntimeException)
    {
        Reference< XNode > aNode;
        if (m_pNodeSet != NULL)
            aNode = Reference< XNode >(DOM::CNode::get(xmlXPathNodeSetItem(m_pNodeSet, index)));
        return aNode;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
