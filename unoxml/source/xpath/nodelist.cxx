/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nodelist.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:08:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "nodelist.hxx"
#include "../dom/node.hxx"

namespace XPath
{
    CNodeList::CNodeList(const xmlXPathObjectPtr xpathObj)
        : m_pNodeSet(0)
    {
        if (xpathObj != NULL && xpathObj->type == XPATH_NODESET)
        {
            m_pNodeSet = xpathObj->nodesetval;
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

