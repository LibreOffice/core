/*************************************************************************
 *
 *  $RCSfile: notationsmap.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 12:26:04 $
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

#include "notationsmap.hxx"

namespace DOM
{
    CNotationsMap::CNotationsMap(const CDocumentType* aDocType)
        : m_pDocType(aDocType)
    {
    }

    /**
    The number of nodes in this map.
    */
    sal_Int32 SAL_CALL CNotationsMap::getLength() throw (RuntimeException)
    {
        return 0;
    }

    /**
    Retrieves a node specified by local name
    */
    Reference< XNode > SAL_CALL CNotationsMap::getNamedItem(const OUString& name) throw (RuntimeException)
    {
        return Reference< XNode >();
    }

    /**
    Retrieves a node specified by local name and namespace URI.
    */
    Reference< XNode > SAL_CALL CNotationsMap::getNamedItemNS(const OUString& namespaceURI,const OUString& localName) throw (RuntimeException)
    {
        return Reference< XNode >();
    }

    /**
    Returns the indexth item in the map.
    */
    Reference< XNode > SAL_CALL CNotationsMap::item(sal_Int32 index) throw (RuntimeException)
    {
        return Reference< XNode >();
    }

    /**
    Removes a node specified by name.
    */
    Reference< XNode > SAL_CALL CNotationsMap::removeNamedItem(const OUString& name) throw (RuntimeException)
    {
        return Reference< XNode >();
    }

    /**
    // Removes a node specified by local name and namespace URI.
    */
    Reference< XNode > SAL_CALL CNotationsMap::removeNamedItemNS(const OUString& namespaceURI, const OUString& localName) throw (RuntimeException)
    {
        return Reference< XNode >();
    }

    /**
    // Adds a node using its nodeName attribute.
    */
    Reference< XNode > SAL_CALL CNotationsMap::setNamedItem(const Reference< XNode >& arg) throw (RuntimeException)
    {
        return Reference< XNode >();
    }

    /**
    Adds a node using its namespaceURI and localName.
    */
    Reference< XNode > SAL_CALL CNotationsMap::setNamedItemNS(const Reference< XNode >& arg) throw (RuntimeException)
    {
        return Reference< XNode >();
    }
}