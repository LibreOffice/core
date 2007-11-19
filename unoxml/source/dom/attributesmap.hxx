/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: attributesmap.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-19 16:41:03 $
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

#ifndef _ATTRIBUTESMAP_HXX
#define _ATTRIBUTESMAP_HXX

#include <map>
#include <sal/types.h>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNamedNodeMap.hpp>
#include "node.hxx"
#include "element.hxx"
#include "attr.hxx"

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;

namespace DOM
{
    class CAttributesMap : public cppu::WeakImplHelper1< XNamedNodeMap >
    {
    private:
        const CElement* m_pElement;
    public:
        CAttributesMap(const CElement* aDocType);

        /**
        The number of nodes in this map.
        */
        virtual sal_Int32 SAL_CALL getLength() throw (RuntimeException);

        /**
        Retrieves a node specified by local name
        */
        virtual Reference< XNode > SAL_CALL getNamedItem(const OUString& name) throw (RuntimeException);

        /**
        Retrieves a node specified by local name and namespace URI.
        */
        virtual Reference< XNode > SAL_CALL getNamedItemNS(const OUString& namespaceURI, const OUString& localName) throw (RuntimeException);

        /**
        Returns the indexth item in the map.
        */
        virtual Reference< XNode > SAL_CALL item(sal_Int32 index) throw (RuntimeException);

        /**
        Removes a node specified by name.
        */
        virtual Reference< XNode > SAL_CALL removeNamedItem(const OUString& name) throw (RuntimeException);

        /**
        // Removes a node specified by local name and namespace URI.
        */
        virtual Reference< XNode > SAL_CALL removeNamedItemNS(const OUString& namespaceURI, const OUString& localName) throw (RuntimeException);

        /**
        // Adds a node using its nodeName attribute.
        */
        virtual Reference< XNode > SAL_CALL setNamedItem(const Reference< XNode >& arg) throw (RuntimeException);

        /**
        Adds a node using its namespaceURI and localName.
        */
        virtual Reference< XNode > SAL_CALL setNamedItemNS(const Reference< XNode >& arg) throw (RuntimeException);
    };
}

#endif
