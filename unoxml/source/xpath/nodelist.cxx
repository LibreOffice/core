/*************************************************************************
 *
 *  $RCSfile: nodelist.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lo $ $Date: 2004-01-28 16:31:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2004 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

