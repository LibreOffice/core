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

#include <domimplementation.hxx>

#include <rtl/instance.hxx>


namespace DOM
{
    // why the heck is this thing static?
    // perhaps it would be helpful to know what the implementation should
    // do to answer this question...
    namespace {
        struct DOMImplementation
            : public ::rtl::Static<CDOMImplementation, DOMImplementation> {};
    }

    CDOMImplementation* CDOMImplementation::get()
    {
        return & DOMImplementation::get();
    }

    // there is just 1 static instance, so these must not delete it!
    void SAL_CALL CDOMImplementation::acquire() throw () { }
    void SAL_CALL CDOMImplementation::release() throw () { }

    /**
    Creates a DOM Document object of the specified type with its document element.
    */
    Reference <XDocument > SAL_CALL CDOMImplementation::createDocument(
           OUString const& /*rNamespaceURI*/,
           OUString const& /*rQualifiedName*/,
           Reference< XDocumentType > const& /*xDoctype*/)
        throw (RuntimeException)
    {
        return Reference<XDocument>();
    }

    /**
    Creates an empty DocumentType node.
    */
    Reference< XDocumentType > SAL_CALL CDOMImplementation::createDocumentType(
            OUString const& /*rQualifiedName*/,
            OUString const& /*rPublicId*/, OUString const& /*rSystemId*/)
        throw (RuntimeException)
    {
        return Reference<XDocumentType>();
    }

    /**
    Test if the DOM implementation implements a specific feature.
    */
    sal_Bool SAL_CALL
    CDOMImplementation::hasFeature(OUString const& /*feature*/, OUString const& /*ver*/)
        throw (RuntimeException)
    {
        return sal_False;
    }
}
