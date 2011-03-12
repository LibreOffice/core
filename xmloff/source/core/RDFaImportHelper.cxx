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

#include "precompiled_xmloff.hxx"

#include "RDFaImportHelper.hxx"

#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>

#include <comphelper/sequenceasvector.hxx>

#include <tools/string.hxx> // for GetAbsoluteReference

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
    ::rtl::OUString GetAbsoluteReference(::rtl::OUString const & i_rURI) const
    {
        if (!i_rURI.getLength() || i_rURI[0] == '#')
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
    ::rtl::OUString ReadCURIE(::rtl::OUString const & i_rCURIE) const;

    std::vector< ::rtl::OUString >
    ReadCURIEs(::rtl::OUString const & i_rCURIEs) const;

    ::rtl::OUString
    ReadURIOrSafeCURIE( ::rtl::OUString const & i_rURIOrSafeCURIE) const;
};

/** helper to insert RDFa statements into the RDF repository */
class SAL_DLLPRIVATE RDFaInserter
{
    const uno::Reference<uno::XComponentContext> m_xContext;
    uno::Reference< rdf::XDocumentRepository > m_xRepository;

    typedef ::std::map< ::rtl::OUString, uno::Reference< rdf::XBlankNode > >
        BlankNodeMap_t;

    BlankNodeMap_t m_BlankNodeMap;

public:
    RDFaInserter(uno::Reference<uno::XComponentContext> const & i_xContext,
            uno::Reference< rdf::XDocumentRepository > const & i_xRepository)
        : m_xContext(i_xContext)
        , m_xRepository(i_xRepository)
    {}

    uno::Reference< rdf::XBlankNode >
    LookupBlankNode(::rtl::OUString const & i_rNodeId );

    uno::Reference< rdf::XURI >
    MakeURI( ::rtl::OUString const & i_rURI) const;

    uno::Reference< rdf::XResource>
    MakeResource( ::rtl::OUString const & i_rResource);

    void InsertRDFaEntry(struct RDFaEntry const & i_rEntry);
};

/** store parsed RDFa attributes */
struct SAL_DLLPRIVATE ParsedRDFaAttributes
{
    ::rtl::OUString m_About;
    ::std::vector< ::rtl::OUString > m_Properties;
    ::rtl::OUString m_Content;
    ::rtl::OUString m_Datatype;

    ParsedRDFaAttributes(
            ::rtl::OUString const & i_rAbout,
            ::std::vector< ::rtl::OUString > const & i_rProperties,
            ::rtl::OUString const & i_rContent,
            ::rtl::OUString const & i_rDatatype)
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

////////////////////////////////////////////////////////////////////////////


static inline bool isWS(const sal_Unicode i_Char)
{
    return ('\t' == i_Char) || ('\n' == i_Char) || ('\r' == i_Char)
        || (' ' == i_Char);
}

static ::rtl::OUString splitAtWS(::rtl::OUString & io_rString)
{
    const sal_Int32 len( io_rString.getLength() );
    sal_Int32 idxstt(0);
    while ((idxstt < len) && ( isWS(io_rString[idxstt])))
        ++idxstt; // skip leading ws
    sal_Int32 idxend(idxstt);
    while ((idxend < len) && (!isWS(io_rString[idxend])))
        ++idxend; // the CURIE
    const ::rtl::OUString ret(io_rString.copy(idxstt, idxend - idxstt));
    io_rString = io_rString.copy(idxend); // rest
    return ret;
}

::rtl::OUString
RDFaReader::ReadCURIE(::rtl::OUString const & i_rCURIE) const
{
    // the RDFa spec says that a prefix is required (it may be empty: ":foo")
    const sal_Int32 idx( i_rCURIE.indexOf(':') );
    if (idx >= 0)
    {
        ::rtl::OUString Prefix;
        ::rtl::OUString LocalName;
        ::rtl::OUString Namespace;
        sal_uInt16 nKey( GetImport().GetNamespaceMap()._GetKeyByAttrName(
            i_rCURIE, &Prefix, &LocalName, &Namespace) );
        if (Prefix.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("_")))
        {
            // eeek, it's a bnode!
            // "_" is not a valid URI scheme => we can identify bnodes
            return i_rCURIE;
        }
        else
        {
            OSL_ENSURE(XML_NAMESPACE_NONE != nKey, "no namespace?");
            if ((XML_NAMESPACE_UNKNOWN != nKey) &&
                (XML_NAMESPACE_XMLNS   != nKey))
            {
                // N.B.: empty LocalName is valid!
                const ::rtl::OUString URI(Namespace + LocalName);
                return GetAbsoluteReference(URI);
            }
            else
            {
                OSL_TRACE( "ReadCURIE: invalid CURIE: invalid prefix" );
                return ::rtl::OUString();
            }
        }
    }
    else
    {
        OSL_TRACE( "ReadCURIE: invalid CURIE: no prefix" );
        return ::rtl::OUString();
    }
}

::std::vector< ::rtl::OUString >
RDFaReader::ReadCURIEs(::rtl::OUString const & i_rCURIEs) const
{
    std::vector< ::rtl::OUString > vec;
    ::rtl::OUString CURIEs(i_rCURIEs);
    do {
      ::rtl::OUString curie( splitAtWS(CURIEs) );
      if (curie.getLength())
      {
          const ::rtl::OUString uri(ReadCURIE(curie));
          if (uri.getLength())
          {
              vec.push_back(uri);
          }
      }
    }
    while (CURIEs.getLength());
    if (!vec.size())
    {
        OSL_TRACE( "ReadCURIEs: invalid CURIEs" );
    }
    return vec;
}

::rtl::OUString
RDFaReader::ReadURIOrSafeCURIE(::rtl::OUString const & i_rURIOrSafeCURIE) const
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
            OSL_TRACE( "ReadURIOrSafeCURIE: invalid SafeCURIE" );
            return ::rtl::OUString();
        }
    }
    else
    {
        if (i_rURIOrSafeCURIE.matchAsciiL("_:", 2)) // blank node
        {
            OSL_TRACE( "ReadURIOrSafeCURIE: invalid URI: scheme is _" );
            return ::rtl::OUString();
        }
        else
        {
            return GetAbsoluteReference(i_rURIOrSafeCURIE);
        }
    }
}

////////////////////////////////////////////////////////////////////////////

uno::Reference< rdf::XBlankNode >
RDFaInserter::LookupBlankNode(::rtl::OUString const & i_rNodeId )
{
    uno::Reference< rdf::XBlankNode > & rEntry( m_BlankNodeMap[ i_rNodeId ] );
    if (!rEntry.is())
    {
        rEntry = m_xRepository->createBlankNode();
    }
    return rEntry;
}

uno::Reference< rdf::XURI >
RDFaInserter::MakeURI( ::rtl::OUString const & i_rURI) const
{
    if (i_rURI.matchAsciiL("_:", 2)) // blank node
    {
        OSL_TRACE("MakeURI: cannot create URI for blank node");
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
            OSL_ENSURE(false, "MakeURI: cannot create URI");
            return 0;
        }
    }
}

uno::Reference< rdf::XResource>
RDFaInserter::MakeResource( ::rtl::OUString const & i_rResource)
{
    if (i_rResource.matchAsciiL("_:", 2)) // blank node
    {
        // we cannot use the blank node label as-is: it must be distinct
        // from labels in other graphs, so create fresh ones per XML stream
        // N.B.: content.xml and styles.xml are distinct graphs
        ::rtl::OUString name( i_rResource.copy(2) );
        const uno::Reference< rdf::XBlankNode > xBNode( LookupBlankNode(name) );
        OSL_ENSURE(xBNode.is(), "no blank node?");
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
    OSL_ENSURE(i_rEntry.m_xObject.is(),
        "InsertRDFaEntry: invalid arg: null object");
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
    if (i_rEntry.m_pRDFaAttributes->m_Datatype.getLength())
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
        OSL_ENSURE(false, "InsertRDFaEntry: setStatementRDFa failed?");
    }
}

////////////////////////////////////////////////////////////////////////////

RDFaImportHelper::RDFaImportHelper(const SvXMLImport & i_rImport)
    : m_rImport(i_rImport)
{
}

RDFaImportHelper::~RDFaImportHelper()
{
}

::boost::shared_ptr<ParsedRDFaAttributes>
RDFaImportHelper::ParseRDFa(
    ::rtl::OUString const & i_rAbout,
    ::rtl::OUString const & i_rProperty,
    ::rtl::OUString const & i_rContent,
    ::rtl::OUString const & i_rDatatype)
{
    if (!i_rProperty.getLength())
    {
        OSL_TRACE("AddRDFa: invalid input: xhtml:property empty");
        return ::boost::shared_ptr<ParsedRDFaAttributes>();
    }
    // must parse CURIEs here: need namespace declaration context
    RDFaReader reader(GetImport());
    const ::rtl::OUString about( reader.ReadURIOrSafeCURIE(i_rAbout) );
    if (!about.getLength()) {
        return ::boost::shared_ptr<ParsedRDFaAttributes>();
    }
    const ::std::vector< ::rtl::OUString > properties(
        reader.ReadCURIEs(i_rProperty) );
    if (!properties.size()) {
        return ::boost::shared_ptr<ParsedRDFaAttributes>();
    }
    const ::rtl::OUString datatype( i_rDatatype.getLength()
        ?   reader.ReadCURIE(i_rDatatype)
        :   ::rtl::OUString() );
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
        OSL_ENSURE(false, "AddRDFa: invalid arg: null textcontent");
        return;
    }
    if (!i_pRDFaAttributes.get())
    {
        OSL_ENSURE(false, "AddRDFa: invalid arg: null RDFa attributes");
        return;
    }
    m_RDFaEntries.push_back(RDFaEntry(i_xObject, i_pRDFaAttributes));
}

void
RDFaImportHelper::ParseAndAddRDFa(
    uno::Reference<rdf::XMetadatable> const & i_xObject,
    ::rtl::OUString const & i_rAbout,
    ::rtl::OUString const & i_rProperty,
    ::rtl::OUString const & i_rContent,
    ::rtl::OUString const & i_rDatatype)
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
    OSL_ENSURE(i_xModel.is(), "InsertRDFa: invalid arg: model null");
    if (!i_xModel.is()) return;
    const uno::Reference< rdf::XDocumentRepository > xRepository(
        i_xModel->getRDFRepository(), uno::UNO_QUERY);
    OSL_ENSURE(xRepository.is(), "InsertRDFa: no DocumentRepository?");
    if (!xRepository.is()) return;
    RDFaInserter inserter(GetImport().GetComponentContext(), xRepository);
    ::std::for_each(m_RDFaEntries.begin(), m_RDFaEntries.end(),
        ::boost::bind(&RDFaInserter::InsertRDFaEntry, &inserter, _1));
}

} // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
