/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: notationsmap.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 00:47:59 $
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
  Reference< XNode > SAL_CALL CNotationsMap::getNamedItem(const OUString& /* name */) throw (RuntimeException)
    {
        return Reference< XNode >();
    }

    /**
    Retrieves a node specified by local name and namespace URI.
    */
  Reference< XNode > SAL_CALL CNotationsMap::getNamedItemNS(const OUString& /*namespaceURI*/, const OUString& /*localName*/)
throw (RuntimeException)
    {
        return Reference< XNode >();
    }

    /**
    Returns the indexth item in the map.
    */
  Reference< XNode > SAL_CALL CNotationsMap::item(sal_Int32 /*index*/) throw (RuntimeException)
    {
        return Reference< XNode >();
    }

    /**
    Removes a node specified by name.
    */
  Reference< XNode > SAL_CALL CNotationsMap::removeNamedItem(const OUString& /*name*/) throw (RuntimeException)
    {
        return Reference< XNode >();
    }

    /**
    // Removes a node specified by local name and namespace URI.
    */
  Reference< XNode > SAL_CALL CNotationsMap::removeNamedItemNS(const OUString& /*namespaceURI*/, const OUString& /*localName*/) throw (RuntimeException)
    {
        return Reference< XNode >();
    }

    /**
    // Adds a node using its nodeName attribute.
    */
  Reference< XNode > SAL_CALL CNotationsMap::setNamedItem(const Reference< XNode >& /*arg*/) throw (RuntimeException)
    {
        return Reference< XNode >();
    }

    /**
    Adds a node using its namespaceURI and localName.
    */
  Reference< XNode > SAL_CALL CNotationsMap::setNamedItemNS(const Reference< XNode >& /*arg*/) throw (RuntimeException)
    {
        return Reference< XNode >();
    }
}
