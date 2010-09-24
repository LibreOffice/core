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
