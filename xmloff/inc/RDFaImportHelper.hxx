/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

