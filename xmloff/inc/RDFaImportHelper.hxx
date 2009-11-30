/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: RDFaImportHelper.hxx,v $
 * $Revision: 1.1.2.1 $
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

#include <com/sun/star/uno/Reference.h>

#include <vector>


namespace rtl { class OUString; }

namespace com { namespace sun { namespace star {
    namespace uno { class XComponentContext; }
    namespace rdf { class XMetadatable; }
    namespace rdf { class XRepositorySupplier; }
} } }

class SvXMLImport;

namespace xmloff {

struct RDFaEntry;

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

    /** Add a RDFa statement; parameters are XML attribute values */
    void AddRDFa(
        ::com::sun::star::uno::Reference< ::com::sun::star::rdf::XMetadatable>
            i_xObject,
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

