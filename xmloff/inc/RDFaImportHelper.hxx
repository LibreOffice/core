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

#pragma once

#include <memory>
#include <vector>

#include <com/sun/star/uno/Reference.h>
#include <rtl/ustring.hxx>

namespace com::sun::star {
    namespace uno { class XComponentContext; }
    namespace rdf { class XMetadatable; }
    namespace rdf { class XRepositorySupplier; }
}

class SvXMLImport;

namespace xmloff {

struct RDFaEntry;
struct ParsedRDFaAttributes;

class RDFaImportHelper
{

private:
    const SvXMLImport & m_rImport;

    ::std::vector< RDFaEntry > m_RDFaEntries;

    const SvXMLImport & GetImport() const { return m_rImport; }


public:
    RDFaImportHelper(const SvXMLImport & i_rImport);

    ~RDFaImportHelper();

    /** Parse RDFa attributes */
    std::shared_ptr<ParsedRDFaAttributes> ParseRDFa(
        OUString const & i_rAbout,
        OUString const & i_rProperty,
        OUString const & i_rContent,
        OUString const & i_rDatatype);

    /** Add a RDFa statement; must have been parsed with ParseRDFa */
    void AddRDFa(
        css::uno::Reference< css::rdf::XMetadatable> const & i_xObject,
        std::shared_ptr<ParsedRDFaAttributes> const & i_pRDFaAttributes);

    /** Parse and add a RDFa statement; parameters are XML attribute values */
    void ParseAndAddRDFa(
        css::uno::Reference< css::rdf::XMetadatable> const & i_xObject,
        OUString const & i_rAbout,
        OUString const & i_rProperty,
        OUString const & i_rContent,
        OUString const & i_rDatatype);

    /** Insert all added statements into the RDF repository.
        <p> This is done <em>after</em> the input file has been read,
        to prevent collision between generated ids and ids in the file.</p>
     */
    void InsertRDFa( css::uno::Reference< css::rdf::XRepositorySupplier > const & i_xModel);
};

} // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
