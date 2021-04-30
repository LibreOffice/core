/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "domimplementation.hxx"

#include <rtl/instance.hxx>
#include <osl/diagnose.h>

using namespace css::uno;
using namespace css::xml::dom;

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
    void SAL_CALL CDOMImplementation::acquire() noexcept { }
    void SAL_CALL CDOMImplementation::release() noexcept { }

    /**
    Creates a DOM Document object of the specified type with its document element.
    */
    Reference <XDocument > SAL_CALL CDOMImplementation::createDocument(
           OUString const& /*rNamespaceURI*/,
           OUString const& /*rQualifiedName*/,
           Reference< XDocumentType > const& /*xDoctype*/)
    {
        OSL_ENSURE(false,
            "CDOMImplementation::createDocument: not implemented (#i113683#)");
        return Reference<XDocument>();
    }

    /**
    Creates an empty DocumentType node.
    */
    Reference< XDocumentType > SAL_CALL CDOMImplementation::createDocumentType(
            OUString const& /*rQualifiedName*/,
            OUString const& /*rPublicId*/, OUString const& /*rSystemId*/)
    {
        OSL_ENSURE(false, "CDOMImplementation::createDocumentType: "
                "not implemented (#i113683#)");
        return Reference<XDocumentType>();
    }

    /**
    Test if the DOM implementation implements a specific feature.
    */
    sal_Bool SAL_CALL
    CDOMImplementation::hasFeature(OUString const& /*feature*/, OUString const& /*ver*/)
    {
        OSL_ENSURE(false,
            "CDOMImplementation::hasFeature: not implemented (#i113683#)");
        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
