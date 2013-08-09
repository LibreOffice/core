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

#include "RDFaImportHelper.hxx"

#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>

#include <comphelper/sequenceasvector.hxx>

#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/XDocumentMetadataAccess.hpp>
#include <com/sun/star/rdf/XDocumentRepository.hpp>

#include <rtl/ustring.hxx>

#include <boost/bind.hpp>
#include <boost/iterator_adaptors.hpp>
#ifndef BOOST_ITERATOR_ADAPTOR_DWA053000_HPP_ // from iterator_adaptors.hpp
// N.B.: the check for the header guard _of a specific version of boost_
//       is here so this may work on different versions of boost,
//       which sadly put the goods in different header files
#include <boost/iterator/transform_iterator.hpp>
#endif

#include <map>
#include <iterator>
#include <functional>
#include <algorithm>

using namespace ::com::sun::star;

namespace xmloff {

/** a bit of context for parsing RDFa attributes */
class SAL_DLLPRIVATE RDFaReader
{
    const SvXMLImport & m_rImport;

    const SvXMLImport & GetImport() const { return m_rImport; }

    //FIXME: this is an ugly hack to workaround buggy SvXMLImport::GetAbsolute
    OUString GetAbsoluteReference(OUString const & i_rURI) const
    {
        if (i_rURI.isEmpty() || i_rURI[0] == '#')
        {
            return GetImport().GetBaseURL() + i_rURI;
        }
        else
        {
            return GetImport().GetAbsoluteReference(i_rURI);
        }
    }

public:
    RDFaReader(SvXMLImport const & i_rImport)
        : m_rImport(i_rImport)
    { }

    // returns URI or blank node!
    OUString ReadCURIE(OUString const & i_rCURIE) const;

    std::vector< OUString >
    ReadCURIEs(OUString const & i_rCURIEs) const;

    OUString
    ReadURIOrSafeCURIE( OUString const & i_rURIOrSafeCURIE) const;
};

/** helper to insert RDFa statements into the RDF repository */
class SAL_DLLPRIVATE RDFaInserter
{
    const uno::Reference<uno::XComponentContext> m_xContext;
    uno::Reference< rdf::XDocumentRepository > m_xRepository;

    typedef ::std::map< OUString, uno::Reference< rdf::XBlankNode > >
        BlankNodeMap_t;

    BlankNodeMap_t m_BlankNodeMap;

public:
    RDFaInserter(uno::Reference<uno::XComponentContext> const & i_xContext,
            uno::Reference< rdf::XDocumentRepository > const & i_xRepository)
        : m_xContext(i_xContext)
        , m_xRepository(i_xRepository)
    {}

    uno::Reference< rdf::XBlankNode >
    LookupBlankNode(OUString const & i_rNodeId );

    uno::Reference< rdf::XURI >
    MakeURI( OUString const & i_rURI) const;

    uno::Reference< rdf::XResource>
    MakeResource( OUString const & i_rResource);

    void InsertRDFaEntry(struct RDFaEntry const & i_rEntry);
};

/** store parsed RDFa attributes */
struct SAL_DLLPRIVATE ParsedRDFaAttributes
{
    OUString m_About;
    ::std::vector< OUString > m_Properties;
    OUString m_Content;
    OUString m_Datatype;

    ParsedRDFaAttributes(
            OUString const & i_rAbout,
            ::std::vector< OUString > const & i_rProperties,
            OUString const & i_rContent,
            OUString const & i_rDatatype)
        : m_About(i_rAbout)
        , m_Properties(i_rProperties)
        , m_Content(i_rContent)
        , m_Datatype(i_rDatatype)
    { }
};

/** store metadatable object and its RDFa attributes */
struct SAL_DLLPRIVATE RDFaEntry
{
    uno::Reference<rdf::XMetadatable> m_xObject;
    ::boost::shared_ptr<ParsedRDFaAttributes> m_pRDFaAttributes;

    RDFaEntry(uno::Reference<rdf::XMetadatable> const & i_xObject,
            ::boost::shared_ptr<ParsedRDFaAttributes> const& i_pRDFaAttributes)
        : m_xObject(i_xObject)
        , m_pRDFaAttributes(i_pRDFaAttributes)
    { }
};

static inline bool isWS(const sal_Unicode i_Char)
{
    return ('\t' == i_Char) || ('\n' == i_Char) || ('\r' == i_Char)
        || (' ' == i_Char);
}

static OUString splitAtWS(OUString & io_rString)
{
    const sal_Int32 len( io_rString.getLength() );
    sal_Int32 idxstt(0);
    while ((idxstt < len) && ( isWS(io_rString[idxstt])))
        ++idxstt; // skip leading ws
    sal_Int32 idxend(idxstt);
    while ((idxend < len) && (!isWS(io_rString[idxend])))
        ++idxend; // the CURIE
    const OUString ret(io_rString.copy(idxstt, idxend - idxstt));
    io_rString = io_rString.copy(idxend); // rest
    return ret;
}

OUString
RDFaReader::ReadCURIE(OUString const & i_rCURIE) const
{
    // the RDFa spec says that a prefix is required (it may be empty: ":foo")
    const sal_Int32 idx( i_rCURIE.indexOf(':') );
    if (idx >= 0)
    {
        OUString Prefix;
        OUString LocalName;
        OUString Namespace;
        sal_uInt16 nKey( GetImport().GetNamespaceMap()._GetKeyByAttrName(
            i_rCURIE, &Prefix, &LocalName, &Namespace) );
        if ( Prefix == "_" )
        {
            // eeek, it's a bnode!
            // "_" is not a valid URI scheme => we can identify bnodes
            return i_rCURIE;
        }
        else
        {
            SAL_WARN_IF(XML_NAMESPACE_NONE == nKey, "xmloff.core", "no namespace?");
            if ((XML_NAMESPACE_UNKNOWN != nKey) &&
                (XML_NAMESPACE_XMLNS   != nKey))
            {
                // N.B.: empty LocalName is valid!
                const OUString URI(Namespace + LocalName);
                return GetAbsoluteReference(URI);
            }
            else
            {
                SAL_INFO("xmloff.core", "ReadCURIE: invalid CURIE: invalid prefix" );
                return OUString();
            }
        }
    }
    SAL_INFO("xmloff.core", "ReadCURIE: invalid CURIE: no prefix" );
    return OUString();
}

::std::vector< OUString >
RDFaReader::ReadCURIEs(OUString const & i_rCURIEs) const
{
    std::vector< OUString > vec;
    OUString CURIEs(i_rCURIEs);
    do {
      OUString curie( splitAtWS(CURIEs) );
      if (!curie.isEmpty())
      {
          const OUString uri(ReadCURIE(curie));
          if (!uri.isEmpty())
          {
              vec.push_back(uri);
          }
      }
    }
    while (!CURIEs.isEmpty());
    if (vec.empty())
    {
        SAL_INFO("xmloff.core", "ReadCURIEs: invalid CURIEs" );
    }
    return vec;
}

OUString
RDFaReader::ReadURIOrSafeCURIE(OUString const & i_rURIOrSafeCURIE) const
{
    const sal_Int32 len(i_rURIOrSafeCURIE.getLength());
    if (len && (i_rURIOrSafeCURIE[0] == '['))
    {
        if ((len >= 2) && (i_rURIOrSafeCURIE[len - 1] == ']'))
        {
            return ReadCURIE(i_rURIOrSafeCURIE.copy(1, len - 2));
        }
        else
        {
            SAL_INFO("xmloff.core", "ReadURIOrSafeCURIE: invalid SafeCURIE" );
            return OUString();
        }
    }
    else
    {
        if (i_rURIOrSafeCURIE.matchAsciiL("_:", 2)) // blank node
        {
            SAL_INFO("xmloff.core", "ReadURIOrSafeCURIE: invalid URI: scheme is _" );
            return OUString();
        }
        else
        {
            return GetAbsoluteReference(i_rURIOrSafeCURIE);
        }
    }
}

uno::Reference< rdf::XBlankNode >
RDFaInserter::LookupBlankNode(OUString const & i_rNodeId )
{
    uno::Reference< rdf::XBlankNode > & rEntry( m_BlankNodeMap[ i_rNodeId ] );
    if (!rEntry.is())
    {
        rEntry = m_xRepository->createBlankNode();
    }
    return rEntry;
}

uno::Reference< rdf::XURI >
RDFaInserter::MakeURI( OUString const & i_rURI) const
{
    if (i_rURI.matchAsciiL("_:", 2)) // blank node
    {
        SAL_INFO("xmloff.core", "MakeURI: cannot create URI for blank node");
        return 0;
    }
    else
    {
        try
        {
            return rdf::URI::create( m_xContext, i_rURI );
        }
        catch (uno::Exception &)
        {
            SAL_WARN("xmloff.core", "MakeURI: cannot create URI");
            return 0;
        }
    }
}

uno::Reference< rdf::XResource>
RDFaInserter::MakeResource( OUString const & i_rResource)
{
    if (i_rResource.matchAsciiL("_:", 2)) // blank node
    {
        // we cannot use the blank node label as-is: it must be distinct
        // from labels in other graphs, so create fresh ones per XML stream
        // N.B.: content.xml and styles.xml are distinct graphs
        OUString name( i_rResource.copy(2) );
        const uno::Reference< rdf::XBlankNode > xBNode( LookupBlankNode(name) );
        SAL_WARN_IF(!xBNode.is(), "xmloff.core", "no blank node?");
        return uno::Reference<rdf::XResource>( xBNode, uno::UNO_QUERY);
    }
    else
    {
        return uno::Reference<rdf::XResource>( MakeURI( i_rResource ),
            uno::UNO_QUERY);
    }
}

/** i wrote this because c++ implementations cannot agree on which variant
    of boost::bind and std::mem_fun_ref applied to Reference::is compiles */
class ref_is_null :
    public ::std::unary_function<sal_Bool, const uno::Reference<rdf::XURI> & >
{
public:
    sal_Bool operator() (const uno::Reference<rdf::XURI> & i_rRef)
    {
        return !i_rRef.is();
    }
};

void RDFaInserter::InsertRDFaEntry(
    struct RDFaEntry const & i_rEntry)
{
    SAL_WARN_IF(!i_rEntry.m_xObject.is(), "xmloff.core", "InsertRDFaEntry: invalid arg: null object");
    if (!i_rEntry.m_xObject.is()) return;

    const uno::Reference< rdf::XResource > xSubject(
        MakeResource( i_rEntry.m_pRDFaAttributes->m_About ) );
    if (!xSubject.is())
    {
        return; // invalid
    }

    ::comphelper::SequenceAsVector< uno::Reference< rdf::XURI > > predicates;

    predicates.reserve(i_rEntry.m_pRDFaAttributes->m_Properties.size());

    ::std::remove_copy_if(
        ::boost::make_transform_iterator(
            i_rEntry.m_pRDFaAttributes->m_Properties.begin(),
            ::boost::bind(&RDFaInserter::MakeURI, this, _1)),
        // argh, this must be the same type :(
        ::boost::make_transform_iterator(
            i_rEntry.m_pRDFaAttributes->m_Properties.end(),
            ::boost::bind(&RDFaInserter::MakeURI, this, _1)),
        ::std::back_inserter(predicates),
        ref_is_null() );
        // compiles only on wntmsci12
//        ::boost::bind( ::std::logical_not<sal_Bool>(), ::boost::bind<sal_Bool>(&uno::Reference<rdf::XURI>::is, _1)));
        // compiles on unxsoli4, wntsci12, but not unxlngi6
//        ::boost::bind( ::std::logical_not<sal_Bool>(), ::boost::bind<sal_Bool, com::sun::star::uno::Reference<rdf::XURI> >(&uno::Reference<rdf::XURI>::is, _1)));
        // compiles on unxsoli4, unxlngi6, but not wntsci12
//        ::std::not1( ::std::mem_fun_ref(&uno::Reference<rdf::XURI>::is)) );

    if (!predicates.size())
    {
        return; // invalid
    }

    uno::Reference<rdf::XURI> xDatatype;
    if (!i_rEntry.m_pRDFaAttributes->m_Datatype.isEmpty())
    {
        xDatatype = MakeURI( i_rEntry.m_pRDFaAttributes->m_Datatype );
    }

    try
    {
        // N.B.: this will call xMeta->ensureMetadataReference, which is why
        // this must be done _after_ importing the whole XML file,
        // to prevent collision between generated ids and ids in the file
        m_xRepository->setStatementRDFa(xSubject, predicates.getAsConstList(),
            i_rEntry.m_xObject,
            i_rEntry.m_pRDFaAttributes->m_Content, xDatatype);
    }
    catch (uno::Exception &)
    {
        SAL_WARN("xmloff.core", "InsertRDFaEntry: setStatementRDFa failed?");
    }
}

RDFaImportHelper::RDFaImportHelper(const SvXMLImport & i_rImport)
    : m_rImport(i_rImport)
{
}

RDFaImportHelper::~RDFaImportHelper()
{
}

::boost::shared_ptr<ParsedRDFaAttributes>
RDFaImportHelper::ParseRDFa(
    OUString const & i_rAbout,
    OUString const & i_rProperty,
    OUString const & i_rContent,
    OUString const & i_rDatatype)
{
    if (i_rProperty.isEmpty())
    {
        SAL_INFO("xmloff.core", "AddRDFa: invalid input: xhtml:property empty");
        return ::boost::shared_ptr<ParsedRDFaAttributes>();
    }
    // must parse CURIEs here: need namespace declaration context
    RDFaReader reader(GetImport());
    const OUString about( reader.ReadURIOrSafeCURIE(i_rAbout) );
    if (about.isEmpty()) {
        return ::boost::shared_ptr<ParsedRDFaAttributes>();
    }
    const ::std::vector< OUString > properties(
        reader.ReadCURIEs(i_rProperty) );
    if (!properties.size()) {
        return ::boost::shared_ptr<ParsedRDFaAttributes>();
    }
    const OUString datatype( !i_rDatatype.isEmpty()
        ?   reader.ReadCURIE(i_rDatatype)
        :   OUString() );
    return ::boost::shared_ptr<ParsedRDFaAttributes>(
            new ParsedRDFaAttributes(about, properties, i_rContent, datatype));
}

void
RDFaImportHelper::AddRDFa(
    uno::Reference<rdf::XMetadatable> const & i_xObject,
    ::boost::shared_ptr<ParsedRDFaAttributes> & i_pRDFaAttributes)
{
    if (!i_xObject.is())
    {
        SAL_WARN("xmloff.core", "AddRDFa: invalid arg: null textcontent");
        return;
    }
    if (!i_pRDFaAttributes.get())
    {
        SAL_WARN("xmloff.core", "AddRDFa: invalid arg: null RDFa attributes");
        return;
    }
    m_RDFaEntries.push_back(RDFaEntry(i_xObject, i_pRDFaAttributes));
}

void
RDFaImportHelper::ParseAndAddRDFa(
    uno::Reference<rdf::XMetadatable> const & i_xObject,
    OUString const & i_rAbout,
    OUString const & i_rProperty,
    OUString const & i_rContent,
    OUString const & i_rDatatype)
{
    ::boost::shared_ptr<ParsedRDFaAttributes> pAttributes(
        ParseRDFa(i_rAbout, i_rProperty, i_rContent, i_rDatatype) );
    if (pAttributes.get())
    {
        AddRDFa(i_xObject, pAttributes);
    }
}

void RDFaImportHelper::InsertRDFa(
    uno::Reference< rdf::XRepositorySupplier> const & i_xModel)
{
    SAL_WARN_IF(!i_xModel.is(), "xmloff.core", "InsertRDFa: invalid arg: model null");
    if (!i_xModel.is()) return;
    const uno::Reference< rdf::XDocumentRepository > xRepository(
        i_xModel->getRDFRepository(), uno::UNO_QUERY);
    SAL_WARN_IF(!xRepository.is(), "xmloff.core", "InsertRDFa: no DocumentRepository?");
    if (!xRepository.is()) return;
    RDFaInserter inserter(GetImport().GetComponentContext(), xRepository);
    ::std::for_each(m_RDFaEntries.begin(), m_RDFaEntries.end(),
        ::boost::bind(&RDFaInserter::InsertRDFaEntry, &inserter, _1));
}

} // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
