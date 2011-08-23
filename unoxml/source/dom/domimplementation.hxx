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

#ifndef _DOMIMPLEMENTATION_HXX
#define _DOMIMPLEMENTATION_HXX

#include <map>
#include <sal/types.h>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XDocumentType.hpp>
#include <com/sun/star/xml/dom/XDOMImplementation.hpp>
#include <com/sun/star/xml/dom/XDOMImplementation.hpp>

using ::rtl::OUString;
using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;

namespace DOM
{
    class CDOMImplementation : public cppu::WeakImplHelper1< XDOMImplementation >
    {

    public:
        static CDOMImplementation* aDOMImplementation;
        static CDOMImplementation* get();

        /**
        Creates a DOM Document object of the specified type with its document element.
        */
        virtual Reference <XDocument > SAL_CALL createDocument(const OUString& namespaceURI, const OUString& qualifiedName, const Reference< XDocumentType >& doctype)
            throw (RuntimeException);

        /**
        Creates an empty DocumentType node.
        */
        virtual Reference< XDocumentType > SAL_CALL createDocumentType(const OUString& qualifiedName, const OUString& publicId, const OUString& systemId)
            throw (RuntimeException);

        /**
        Test if the DOM implementation implements a specific feature.
        */
        virtual sal_Bool SAL_CALL hasFeature(const OUString& feature, const OUString& ver)
            throw (RuntimeException);
        };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
