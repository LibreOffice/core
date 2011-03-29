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

#ifndef RDFAIMPORTHELPER_HXX
#define RDFAIMPORTHELPER_HXX

#include <vector>

#include <boost/shared_ptr.hpp>

#include <com/sun/star/uno/Reference.h>


namespace rtl { class OUString; }

namespace com { namespace sun { namespace star {
    namespace uno { class XComponentContext; }
    namespace rdf { class XMetadatable; }
    namespace rdf { class XRepositorySupplier; }
} } }

class SvXMLImport;

namespace xmloff {

struct RDFaEntry;
struct ParsedRDFaAttributes;

class SAL_DLLPRIVATE RDFaImportHelper
{

private:
    const SvXMLImport & m_rImport;

    typedef ::std::vector< RDFaEntry > RDFaEntries_t;

    RDFaEntries_t m_RDFaEntries;

    const SvXMLImport & GetImport() const { return m_rImport; }


public:
    RDFaImportHelper(const SvXMLImport & i_rImport);

    ~RDFaImportHelper();

    /** Parse RDFa attributes */
    ::boost::shared_ptr<ParsedRDFaAttributes> ParseRDFa(
        ::rtl::OUString const & i_rAbout,
        ::rtl::OUString const & i_rProperty,
        ::rtl::OUString const & i_rContent,
        ::rtl::OUString const & i_rDatatype);

    /** Add a RDFa statement; must have been parsed with ParseRDFa */
    void AddRDFa(
        ::com::sun::star::uno::Reference< ::com::sun::star::rdf::XMetadatable>
            const & i_xObject,
        ::boost::shared_ptr<ParsedRDFaAttributes> & i_pRDFaAttributes);

    /** Parse and add a RDFa statement; parameters are XML attribute values */
    void ParseAndAddRDFa(
        ::com::sun::star::uno::Reference< ::com::sun::star::rdf::XMetadatable>
            const & i_xObject,
        ::rtl::OUString const & i_rAbout,
        ::rtl::OUString const & i_rProperty,
        ::rtl::OUString const & i_rContent,
        ::rtl::OUString const & i_rDatatype);

    /** Insert all added statements into the RDF repository.
        <p> This is done <em>after</em> the input file has been read,
        to prevent collision between generated ids and ids in the file.</p>
     */
    void InsertRDFa( ::com::sun::star::uno::Reference<
        ::com::sun::star::rdf::XRepositorySupplier > const & i_xModel);
};

} // namespace xmloff

#endif // RDFAIMPORTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
