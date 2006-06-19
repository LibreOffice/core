/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: domimplementation.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 00:45:31 $
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

#include "domimplementation.hxx"

namespace DOM
{
    CDOMImplementation* CDOMImplementation::aDOMImplementation = new CDOMImplementation();
    CDOMImplementation* CDOMImplementation::get()
    {
        return CDOMImplementation::aDOMImplementation;
    }

    /**
    Creates a DOM Document object of the specified type with its document element.
    */
    Reference <XDocument > SAL_CALL CDOMImplementation::createDocument(
           const OUString& namespaceURI,
           const OUString& qualifiedName,
           const Reference< XDocumentType >& doctype)
        throw (RuntimeException)
    {
      OUString aNamespaceURI = namespaceURI;
      OUString aQName = qualifiedName;
      Reference< XDocumentType > aType = doctype;
        return Reference<XDocument>();
    }

    /**
    Creates an empty DocumentType node.
    */
    Reference< XDocumentType > SAL_CALL CDOMImplementation::createDocumentType(
            const OUString& qualifiedName, const OUString& publicId, const OUString& systemId)
        throw (RuntimeException)
    {
      OUString qName = qualifiedName;
      OUString aPublicId = publicId;
      OUString aSystemId = systemId;
        return Reference<XDocumentType>();
    }
    /**
    Test if the DOM implementation implements a specific feature.
    */
    sal_Bool SAL_CALL CDOMImplementation::hasFeature(const OUString& feature, const OUString& ver)
        throw (RuntimeException)
    {
      OUString aFeature = feature;
      OUString aVersion = ver;
        return sal_False;
    }
}
