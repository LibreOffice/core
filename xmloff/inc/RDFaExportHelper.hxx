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

#ifndef RDFAEXPORTHELPER_HXX
#define RDFAEXPORTHELPER_HXX

#include <com/sun/star/uno/Reference.h>

#include <rtl/ustring.hxx>

#include <map>


namespace com { namespace sun { namespace star {
    namespace rdf { class XBlankNode; }
    namespace rdf { class XMetadatable; }
    namespace rdf { class XDocumentRepository; }
} } }

class SvXMLExport;

namespace xmloff {

class SAL_DLLPRIVATE RDFaExportHelper
{
private:
    SvXMLExport & m_rExport;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::rdf::XDocumentRepository> m_xRepository;

    typedef ::std::map< OUString, OUString >
        BlankNodeMap_t;

    BlankNodeMap_t m_BlankNodeMap;

    long m_Counter;

    OUString
    LookupBlankNode( ::com::sun::star::uno::Reference<
        ::com::sun::star::rdf::XBlankNode> const & i_xBlankNode);

public:
    RDFaExportHelper(SvXMLExport & i_rExport);

    void
    AddRDFa(::com::sun::star::uno::Reference<
        ::com::sun::star::rdf::XMetadatable> const & i_xMetadatable);
};

} // namespace xmloff

#endif // RDFAEXPORTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
