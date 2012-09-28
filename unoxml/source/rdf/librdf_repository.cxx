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

#include "librdf_repository.hxx"

#include <string.h>

#include <set>
#include <map>
#include <functional>
#include <algorithm>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/bind.hpp>

#include <libxslt/security.h>
#include <libxml/parser.h>

#include <redland.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/io/XSeekableInputStream.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/rdf/XDocumentRepository.hpp>
#include <com/sun/star/rdf/XLiteral.hpp>
#include <com/sun/star/rdf/FileFormat.hpp>
#include <com/sun/star/rdf/URIs.hpp>
#include <com/sun/star/rdf/BlankNode.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/Literal.hpp>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/basemutex.hxx>

#include <comphelper/stlunosequence.hxx>
#include <comphelper/sequenceasvector.hxx>
#include <comphelper/makesequence.hxx>
#include <comphelper/xmltools.hxx>

#include <com/sun/star/embed/XEncryptionProtectedSource2.hpp>

/**
    Implementation of the service com.sun.star.rdf.Repository.

    This implementation uses the Redland RDF library (librdf).

    There are several classes involved:
    librdf_TypeConverter:   helper class to convert data types redland <-> uno
    librdf_Repository:      the main repository, does almost all the work
    librdf_NamedGraph:      the XNamedGraph, forwards everything to repository
    librdf_GraphResult:     an XEnumeration<Statement>
    librdf_QuerySelectResult:   an XEnumeration<sequence<XNode>>

    @author mst
 */

/// anonymous implementation namespace
namespace {

class librdf_NamedGraph;
class librdf_Repository;

using namespace ::com::sun::star;

typedef std::map< ::rtl::OUString, ::rtl::Reference<librdf_NamedGraph> >
    NamedGraphMap_t;

const char s_sparql [] = "sparql";
const char s_nsRDFs [] = "http://www.w3.org/2000/01/rdf-schema#";
const char s_label  [] = "label";
const char s_nsOOo  [] = "http://openoffice.org/2004/office/rdfa/";

////////////////////////////////////////////////////////////////////////////

//FIXME: this approach is not ideal. can we use blind nodes instead?
bool isInternalContext(librdf_node *i_pNode) throw ()
{
    OSL_ENSURE(i_pNode, "isInternalContext: context null");
    OSL_ENSURE(librdf_node_is_resource(i_pNode),
        "isInternalContext: context not resource");
    if (i_pNode) {
        librdf_uri *pURI(librdf_node_get_uri(i_pNode));
        OSL_ENSURE(pURI, "isInternalContext: URI null");
        if (pURI) {
            unsigned char *pContextURI(librdf_uri_as_string(pURI));
            OSL_ENSURE(pContextURI,
                "isInternalContext: URI string null");
            // if prefix matches reserved uri, it is RDFa context
            if (!strncmp(reinterpret_cast<char *>(pContextURI),
                    s_nsOOo, sizeof(s_nsOOo)-1)) {
                return true;
            }
        }
        return false;
    }
    return true;
}


////////////////////////////////////////////////////////////////////////////

// n.b.: librdf destructor functions dereference null pointers!
//       so they need to be wrapped to be usable with boost::shared_ptr.
static void safe_librdf_free_world(librdf_world *const world)
{
#if 1
    (void)world; // leak it
#else
    // disable this for now: it calls xmlCleanupParser, which now aborts
    // (see desktop/source/app/main.c)
    if (world) { librdf_free_world(world); }
#endif
}
static void safe_librdf_free_model(librdf_model *const model)
{
    if (model) { librdf_free_model(model); }
}
static void safe_librdf_free_node(librdf_node* node)
{
    if (node) { librdf_free_node(node); }
}
static void safe_librdf_free_parser(librdf_parser *const parser)
{
    if (parser) { librdf_free_parser(parser); }
}
static void safe_librdf_free_query(librdf_query *const query)
{
    if (query) { librdf_free_query(query); }
}
static void
safe_librdf_free_query_results(librdf_query_results *const query_results)
{
    if (query_results) { librdf_free_query_results(query_results); }
}
static void safe_librdf_free_serializer(librdf_serializer *const serializer)
{
    if (serializer) { librdf_free_serializer(serializer); }
}
static void safe_librdf_free_statement(librdf_statement *const statement)
{
    if (statement) { librdf_free_statement(statement); }
}
static void safe_librdf_free_storage(librdf_storage *const storage)
{
    if (storage) { librdf_free_storage(storage); }
}
static void safe_librdf_free_stream(librdf_stream *const stream)
{
    if (stream) { librdf_free_stream(stream); }
}
static void safe_librdf_free_uri(librdf_uri *const uri)
{
    if (uri) { librdf_free_uri(uri); }
}


////////////////////////////////////////////////////////////////////////////

/** converts between librdf types and UNO API types.
 */
class librdf_TypeConverter
{
public:
    librdf_TypeConverter(
            uno::Reference< uno::XComponentContext > const & i_xContext,
            librdf_Repository &i_rRep)
        : m_xContext(i_xContext)
        , m_rRep(i_rRep)
    { };

    librdf_world *createWorld() const;
    librdf_storage *createStorage(librdf_world *i_pWorld) const;
    librdf_model *createModel(librdf_world *i_pWorld,
        librdf_storage * i_pStorage) const;
    librdf_uri* mkURI( librdf_world* i_pWorld,
        const uno::Reference< rdf::XURI > & i_xURI) const;
    librdf_node* mkResource( librdf_world* i_pWorld,
        const uno::Reference< rdf::XResource > & i_xResource) const;
    librdf_node* mkNode( librdf_world* i_pWorld,
        const uno::Reference< rdf::XNode > & i_xNode) const;
    librdf_statement* mkStatement( librdf_world* i_pWorld,
        const uno::Reference< rdf::XResource > & i_xSubject,
        const uno::Reference< rdf::XURI > & i_xPredicate,
        const uno::Reference< rdf::XNode > & i_xObject) const;
    uno::Reference<rdf::XURI> convertToXURI(librdf_uri* i_pURI) const;
    uno::Reference<rdf::XURI> convertToXURI(librdf_node* i_pURI) const;
    uno::Reference<rdf::XResource>
        convertToXResource(librdf_node* i_pNode) const;
    uno::Reference<rdf::XNode> convertToXNode(librdf_node* i_pNode) const;
    rdf::Statement
        convertToStatement(librdf_statement* i_pStmt, librdf_node* i_pContext)
        const;

private:
    uno::Reference< uno::XComponentContext > m_xContext;
    librdf_Repository & m_rRep;
};


////////////////////////////////////////////////////////////////////////////

/** implements the repository service.
 */
class librdf_Repository:
    private boost::noncopyable,
//    private ::cppu::BaseMutex,
    public ::cppu::WeakImplHelper3<
        lang::XServiceInfo,
        rdf::XDocumentRepository,
        lang::XInitialization>
{
public:

    explicit librdf_Repository(
        uno::Reference< uno::XComponentContext > const & i_xContext);
    virtual ~librdf_Repository();

    // ::com::sun::star::lang::XServiceInfo:
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService(
            const ::rtl::OUString & ServiceName) throw (uno::RuntimeException);
    virtual uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames() throw (uno::RuntimeException);

    // ::com::sun::star::rdf::XRepository:
    virtual uno::Reference< rdf::XBlankNode > SAL_CALL createBlankNode()
        throw (uno::RuntimeException);
    virtual uno::Reference<rdf::XNamedGraph> SAL_CALL importGraph(
            ::sal_Int16 i_Format,
            const uno::Reference< io::XInputStream > & i_xInStream,
            const uno::Reference< rdf::XURI > & i_xGraphName,
            const uno::Reference< rdf::XURI > & i_xBaseURI)
        throw (uno::RuntimeException, lang::IllegalArgumentException,
            datatransfer::UnsupportedFlavorException,
            container::ElementExistException, rdf::ParseException,
            rdf::RepositoryException, io::IOException);
    virtual void SAL_CALL exportGraph(::sal_Int16 i_Format,
            const uno::Reference< io::XOutputStream > & i_xOutStream,
            const uno::Reference< rdf::XURI > & i_xGraphName,
            const uno::Reference< rdf::XURI > & i_xBaseURI)
        throw (uno::RuntimeException, lang::IllegalArgumentException,
            datatransfer::UnsupportedFlavorException,
            container::NoSuchElementException, rdf::RepositoryException,
            io::IOException);
    virtual uno::Sequence< uno::Reference< rdf::XURI > > SAL_CALL
        getGraphNames() throw (uno::RuntimeException, rdf::RepositoryException);
    virtual uno::Reference< rdf::XNamedGraph > SAL_CALL getGraph(
            const uno::Reference< rdf::XURI > & i_xGraphName)
        throw (uno::RuntimeException, lang::IllegalArgumentException,
            rdf::RepositoryException);
    virtual uno::Reference< rdf::XNamedGraph > SAL_CALL createGraph(
            const uno::Reference< rdf::XURI > & i_xGraphName)
        throw (uno::RuntimeException, lang::IllegalArgumentException,
            container::ElementExistException, rdf::RepositoryException);
    virtual void SAL_CALL destroyGraph(
            const uno::Reference< rdf::XURI > & i_xGraphName)
        throw (uno::RuntimeException, lang::IllegalArgumentException,
            container::NoSuchElementException, rdf::RepositoryException);
    virtual uno::Reference< container::XEnumeration > SAL_CALL getStatements(
            const uno::Reference< rdf::XResource > & i_xSubject,
            const uno::Reference< rdf::XURI > & i_xPredicate,
            const uno::Reference< rdf::XNode > & i_xObject)
        throw (uno::RuntimeException,
            rdf::RepositoryException);
    virtual uno::Reference< rdf::XQuerySelectResult > SAL_CALL
            querySelect(const ::rtl::OUString & i_rQuery)
        throw (uno::RuntimeException, rdf::QueryException,
            rdf::RepositoryException);
    virtual uno::Reference< container::XEnumeration > SAL_CALL
        queryConstruct(const ::rtl::OUString & i_rQuery)
        throw (uno::RuntimeException, rdf::QueryException,
            rdf::RepositoryException);
    virtual ::sal_Bool SAL_CALL queryAsk(const ::rtl::OUString & i_rQuery)
        throw (uno::RuntimeException, rdf::QueryException,
            rdf::RepositoryException);

    // ::com::sun::star::rdf::XDocumentRepository:
    virtual void SAL_CALL setStatementRDFa(
            const uno::Reference< rdf::XResource > & i_xSubject,
            const uno::Sequence< uno::Reference< rdf::XURI > > & i_rPredicates,
            const uno::Reference< rdf::XMetadatable > & i_xObject,
            const ::rtl::OUString & i_rRDFaContent,
            const uno::Reference< rdf::XURI > & i_xRDFaDatatype)
        throw (uno::RuntimeException, lang::IllegalArgumentException,
            rdf::RepositoryException);
    virtual void SAL_CALL removeStatementRDFa(
            const uno::Reference< rdf::XMetadatable > & i_xElement)
        throw (uno::RuntimeException, lang::IllegalArgumentException,
            rdf::RepositoryException);
    virtual beans::Pair< uno::Sequence<rdf::Statement>, sal_Bool > SAL_CALL
        getStatementRDFa(uno::Reference< rdf::XMetadatable > const& i_xElement)
        throw (uno::RuntimeException, lang::IllegalArgumentException,
            rdf::RepositoryException);
    virtual uno::Reference< container::XEnumeration > SAL_CALL
        getStatementsRDFa(
            const uno::Reference< rdf::XResource > & i_xSubject,
            const uno::Reference< rdf::XURI > & i_xPredicate,
            const uno::Reference< rdf::XNode > & i_xObject)
        throw (uno::RuntimeException,
            rdf::RepositoryException);

    // ::com::sun::star::lang::XInitialization:
    virtual void SAL_CALL initialize(
            const uno::Sequence< ::com::sun::star::uno::Any > & i_rArguments)
        throw (uno::RuntimeException, uno::Exception);

    // XNamedGraph forwards ---------------------------------------------
    const NamedGraphMap_t::iterator SAL_CALL clearGraph(
            const uno::Reference< rdf::XURI > & i_xName,
            bool i_Internal = false );
    void addStatementGraph(
            const uno::Reference< rdf::XResource > & i_xSubject,
            const uno::Reference< rdf::XURI > & i_xPredicate,
            const uno::Reference< rdf::XNode > & i_xObject,
            const uno::Reference< rdf::XURI > & i_xName,
            bool i_Internal = false );
//        throw (uno::RuntimeException, lang::IllegalArgumentException,
//            container::NoSuchElementException, rdf::RepositoryException);
    void removeStatementsGraph(
            const uno::Reference< rdf::XResource > & i_xSubject,
            const uno::Reference< rdf::XURI > & i_xPredicate,
            const uno::Reference< rdf::XNode > & i_xObject,
            const uno::Reference< rdf::XURI > & i_xName );
//        throw (uno::RuntimeException, lang::IllegalArgumentException,
//            container::NoSuchElementException, rdf::RepositoryException);
    uno::Reference< container::XEnumeration > getStatementsGraph(
            const uno::Reference< rdf::XResource > & i_xSubject,
            const uno::Reference< rdf::XURI > & i_xPredicate,
            const uno::Reference< rdf::XNode > & i_xObject,
            const uno::Reference< rdf::XURI > & i_xName,
            bool i_Internal = false );
//        throw (uno::RuntimeException, lang::IllegalArgumentException,
//            container::NoSuchElementException, rdf::RepositoryException);

    const librdf_TypeConverter& getTypeConverter() { return m_TypeConverter; };

private:

    uno::Reference< uno::XComponentContext > m_xContext;

    /// librdf global data
    /** N.B.: The redland documentation gives the impression that you can have
              as many librdf_worlds as you like. This is true in the same sense
              that you can physically be in as many places as you like.
              Well, you can, just not at the same time.
              The ugly truth is that destroying a librdf_world kills a bunch
              of static variables; other librdf_worlds become very unhappy
              when they access these.
              And of course this is not documented anywhere that I could find.
              So we allocate a single world, and refcount that.
     */
    static boost::shared_ptr<librdf_world> m_pWorld;
    /// refcount
    static sal_uInt32 m_NumInstances;
    /// mutex for m_pWorld - redland is not as threadsafe as is often claimed
    static osl::Mutex m_aMutex;

    // NB: sequence of the shared pointers is important!
    /// librdf repository storage
    boost::shared_ptr<librdf_storage> m_pStorage;
    /// librdf repository model
    boost::shared_ptr<librdf_model> m_pModel;

    /// all named graphs
    NamedGraphMap_t m_NamedGraphs;

    /// type conversion helper
    librdf_TypeConverter m_TypeConverter;

    /// set of xml:ids of elements with xhtml:content
    ::std::set< ::rtl::OUString > m_RDFaXHTMLContentSet;
};


////////////////////////////////////////////////////////////////////////////

/** result of operations that return a graph, i.e.,
    an XEnumeration of statements.
 */
class librdf_GraphResult:
    private boost::noncopyable,
    public ::cppu::WeakImplHelper1<
        container::XEnumeration>
{
public:

    librdf_GraphResult(librdf_Repository *i_pRepository,
            ::osl::Mutex & i_rMutex,
            boost::shared_ptr<librdf_stream> const& i_pStream,
            boost::shared_ptr<librdf_node> const& i_pContext,
            boost::shared_ptr<librdf_query>  const& i_pQuery =
                boost::shared_ptr<librdf_query>() )
        : m_xRep(i_pRepository)
        , m_rMutex(i_rMutex)
        , m_pQuery(i_pQuery)
        , m_pContext(i_pContext)
        , m_pStream(i_pStream)
    { };

    virtual ~librdf_GraphResult() {}

    // ::com::sun::star::container::XEnumeration:
    virtual ::sal_Bool SAL_CALL hasMoreElements()
        throw (uno::RuntimeException);
    virtual uno::Any SAL_CALL nextElement()
        throw (uno::RuntimeException, container::NoSuchElementException,
            lang::WrappedTargetException);

private:
    // NB: this is not a weak pointer: streams _must_ be deleted before the
    //     storage they point into, so we keep the repository alive here
    // also, sequence is important: the stream must be destroyed first.
    ::rtl::Reference< librdf_Repository > m_xRep;
    // needed for synchronizing access to librdf (it doesnt do win32 threading)
    ::osl::Mutex & m_rMutex;
    // the query (in case this is a result of a graph query)
    // not that the redland documentation spells this out explicity, but
    // queries must be freed only after all the results are completely read
    boost::shared_ptr<librdf_query>  const m_pQuery;
    boost::shared_ptr<librdf_node>   const m_pContext;
    boost::shared_ptr<librdf_stream> const m_pStream;

    librdf_node* getContext() const;
};


// ::com::sun::star::container::XEnumeration:
::sal_Bool SAL_CALL
librdf_GraphResult::hasMoreElements() throw (uno::RuntimeException)
{
    ::osl::MutexGuard g(m_rMutex);
    return m_pStream.get() && !librdf_stream_end(m_pStream.get());
}

librdf_node* librdf_GraphResult::getContext() const
{
    if (!m_pStream.get() || librdf_stream_end(m_pStream.get()))
        return NULL;
    SAL_WNODEPRECATED_DECLARATIONS_PUSH;
    librdf_node *pCtxt( static_cast<librdf_node *>
        (librdf_stream_get_context(m_pStream.get())) );
    SAL_WNODEPRECATED_DECLARATIONS_POP;
    if (pCtxt)
        return pCtxt;
    return m_pContext.get();
}

::com::sun::star::uno::Any SAL_CALL
librdf_GraphResult::nextElement()
throw (uno::RuntimeException, container::NoSuchElementException,
    lang::WrappedTargetException)
{
    ::osl::MutexGuard g(m_rMutex);
    if (!m_pStream.get() || !librdf_stream_end(m_pStream.get())) {
        librdf_node * pCtxt = getContext();

        librdf_statement *pStmt( librdf_stream_get_object(m_pStream.get()) );
        if (!pStmt) {
            rdf::QueryException e(
                "librdf_GraphResult::nextElement: "
                "librdf_stream_get_object failed", *this);
            throw lang::WrappedTargetException(
                "librdf_GraphResult::nextElement: "
                "librdf_stream_get_object failed", *this,
                    uno::makeAny(e));
        }
        // NB: pCtxt may be null here if this is result of a graph query
        if (pCtxt && isInternalContext(pCtxt)) {
            pCtxt = 0; // XML ID context is implementation detail!
        }
        rdf::Statement Stmt(
            m_xRep->getTypeConverter().convertToStatement(pStmt, pCtxt) );
        // NB: this will invalidate current item.
        librdf_stream_next(m_pStream.get());
        return uno::makeAny(Stmt);
    } else {
        throw container::NoSuchElementException();
    }
}


////////////////////////////////////////////////////////////////////////////

/** result of tuple queries ("SELECT").
 */
class librdf_QuerySelectResult:
    private boost::noncopyable,
    public ::cppu::WeakImplHelper1<
        rdf::XQuerySelectResult>
{
public:

    librdf_QuerySelectResult(librdf_Repository *i_pRepository,
            ::osl::Mutex & i_rMutex,
            boost::shared_ptr<librdf_query>  const& i_pQuery,
            boost::shared_ptr<librdf_query_results> const& i_pQueryResult,
            uno::Sequence< ::rtl::OUString > const& i_rBindingNames )
        : m_xRep(i_pRepository)
        , m_rMutex(i_rMutex)
        , m_pQuery(i_pQuery)
        , m_pQueryResult(i_pQueryResult)
        , m_BindingNames(i_rBindingNames)
    { };

    virtual ~librdf_QuerySelectResult() {}

    // ::com::sun::star::container::XEnumeration:
    virtual ::sal_Bool SAL_CALL hasMoreElements()
        throw (uno::RuntimeException);
    virtual uno::Any SAL_CALL nextElement()
        throw (uno::RuntimeException, container::NoSuchElementException,
            lang::WrappedTargetException);

    // ::com::sun::star::rdf::XQuerySelectResult:
    virtual uno::Sequence< ::rtl::OUString > SAL_CALL getBindingNames()
        throw (uno::RuntimeException);

private:

    // NB: this is not a weak pointer: streams _must_ be deleted before the
    //     storage they point into, so we keep the repository alive here
    // also, sequence is important: the stream must be destroyed first.
    ::rtl::Reference< librdf_Repository > m_xRep;
    // needed for synchronizing access to librdf (it doesnt do win32 threading)
    ::osl::Mutex & m_rMutex;
    // not that the redland documentation spells this out explicity, but
    // queries must be freed only after all the results are completely read
    boost::shared_ptr<librdf_query>  m_pQuery;
    boost::shared_ptr<librdf_query_results> m_pQueryResult;
    uno::Sequence< ::rtl::OUString > m_BindingNames;
};


// ::com::sun::star::container::XEnumeration:
::sal_Bool SAL_CALL
librdf_QuerySelectResult::hasMoreElements() throw (uno::RuntimeException)
{
    ::osl::MutexGuard g(m_rMutex);
    return !librdf_query_results_finished(m_pQueryResult.get());
}

class NodeArrayDeleter : public std::unary_function<librdf_node**, void>
{
    const int m_Count;

public:
    NodeArrayDeleter(int i_Count) : m_Count(i_Count) { }

    void operator() (librdf_node** io_pArray) const throw ()
    {
        std::for_each(io_pArray, io_pArray + m_Count, safe_librdf_free_node);
        delete[] io_pArray;
    }
};

::com::sun::star::uno::Any SAL_CALL
librdf_QuerySelectResult::nextElement()
throw (uno::RuntimeException, container::NoSuchElementException,
    lang::WrappedTargetException)
{
    ::osl::MutexGuard g(m_rMutex);
    if (!librdf_query_results_finished(m_pQueryResult.get())) {
        sal_Int32 count(m_BindingNames.getLength());
        OSL_ENSURE(count >= 0, "negative length?");
        boost::shared_array<librdf_node*> pNodes( new librdf_node*[count],
            NodeArrayDeleter(count));
        for (int i = 0; i < count; ++i) {
            pNodes[i] = 0;
        }
        if (librdf_query_results_get_bindings(m_pQueryResult.get(), NULL,
                    pNodes.get()))
        {
            rdf::QueryException e(
                "librdf_QuerySelectResult::nextElement: "
                "librdf_query_results_get_bindings failed", *this);
            throw lang::WrappedTargetException(
                "librdf_QuerySelectResult::nextElement: "
                "librdf_query_results_get_bindings failed", *this,
                uno::makeAny(e));
        }
        uno::Sequence< uno::Reference< rdf::XNode > > ret(count);
        for (int i = 0; i < count; ++i) {
            ret[i] = m_xRep->getTypeConverter().convertToXNode(pNodes[i]);
        }
        // NB: this will invalidate current item.
        librdf_query_results_next(m_pQueryResult.get());
        return uno::makeAny(ret);
    } else {
        throw container::NoSuchElementException();
    }
}

// ::com::sun::star::rdf::XQuerySelectResult:
uno::Sequence< ::rtl::OUString > SAL_CALL
librdf_QuerySelectResult::getBindingNames() throw (uno::RuntimeException)
{
    return m_BindingNames;
}


////////////////////////////////////////////////////////////////////////////

/** represents a named graph, and forwards all the work to repository.
 */
class librdf_NamedGraph:
    private boost::noncopyable,
    public ::cppu::WeakImplHelper1<
        rdf::XNamedGraph>
{
public:
    librdf_NamedGraph(librdf_Repository * i_pRep,
            uno::Reference<rdf::XURI> const & i_xName)
        : m_wRep(i_pRep)
        , m_pRep(i_pRep)
        , m_xName(i_xName)
    { };

    virtual ~librdf_NamedGraph() {}

    // ::com::sun::star::rdf::XNode:
    virtual ::rtl::OUString SAL_CALL getStringValue()
        throw (uno::RuntimeException);

    // ::com::sun::star::rdf::XURI:
    virtual ::rtl::OUString SAL_CALL getNamespace()
        throw (uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getLocalName()
        throw (uno::RuntimeException);

    // ::com::sun::star::rdf::XNamedGraph:
    virtual uno::Reference<rdf::XURI> SAL_CALL getName()
        throw (uno::RuntimeException);
    virtual void SAL_CALL clear()
        throw (uno::RuntimeException,
            container::NoSuchElementException, rdf::RepositoryException);
    virtual void SAL_CALL addStatement(
            const uno::Reference< rdf::XResource > & i_xSubject,
            const uno::Reference< rdf::XURI > & i_xPredicate,
            const uno::Reference< rdf::XNode > & i_xObject)
        throw (uno::RuntimeException, lang::IllegalArgumentException,
            container::NoSuchElementException, rdf::RepositoryException);
    virtual void SAL_CALL removeStatements(
            const uno::Reference< rdf::XResource > & i_xSubject,
            const uno::Reference< rdf::XURI > & i_xPredicate,
            const uno::Reference< rdf::XNode > & i_xObject)
        throw (uno::RuntimeException,
            container::NoSuchElementException, rdf::RepositoryException);
    virtual uno::Reference< container::XEnumeration > SAL_CALL getStatements(
            const uno::Reference< rdf::XResource > & i_xSubject,
            const uno::Reference< rdf::XURI > & i_xPredicate,
            const uno::Reference< rdf::XNode > & i_xObject)
        throw (uno::RuntimeException,
            container::NoSuchElementException, rdf::RepositoryException);

private:

    /// weak reference: this is needed to check if m_pRep is valid
    uno::WeakReference< rdf::XRepository > m_wRep;
    librdf_Repository *m_pRep;
    uno::Reference< rdf::XURI > m_xName;
};


// ::com::sun::star::rdf::XNode:
::rtl::OUString SAL_CALL librdf_NamedGraph::getStringValue()
throw (uno::RuntimeException)
{
    return m_xName->getStringValue();
}

// ::com::sun::star::rdf::XURI:
::rtl::OUString SAL_CALL librdf_NamedGraph::getNamespace()
throw (uno::RuntimeException)
{
    return m_xName->getNamespace();
}

::rtl::OUString SAL_CALL librdf_NamedGraph::getLocalName()
throw (uno::RuntimeException)
{
    return m_xName->getLocalName();
}

// ::com::sun::star::rdf::XNamedGraph:
uno::Reference< rdf::XURI > SAL_CALL librdf_NamedGraph::getName()
throw (uno::RuntimeException)
{
    return m_xName;
}

void SAL_CALL librdf_NamedGraph::clear()
throw (uno::RuntimeException,
    container::NoSuchElementException, rdf::RepositoryException)
{
    uno::Reference< rdf::XRepository > xRep( m_wRep );
    if (!xRep.is()) {
        throw rdf::RepositoryException(
            "librdf_NamedGraph::clear: repository is gone", *this);
    }
    try {
        m_pRep->clearGraph(m_xName);
    } catch (lang::IllegalArgumentException &) {
        throw uno::RuntimeException();
    }
}

void SAL_CALL librdf_NamedGraph::addStatement(
    const uno::Reference< rdf::XResource > & i_xSubject,
    const uno::Reference< rdf::XURI > & i_xPredicate,
    const uno::Reference< rdf::XNode > & i_xObject)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    container::NoSuchElementException, rdf::RepositoryException)
{
    uno::Reference< rdf::XRepository > xRep( m_wRep );
    if (!xRep.is()) {
        throw rdf::RepositoryException(
            "librdf_NamedGraph::addStatement: repository is gone", *this);
    }
    m_pRep->addStatementGraph(i_xSubject, i_xPredicate, i_xObject, m_xName);
}

void SAL_CALL librdf_NamedGraph::removeStatements(
    const uno::Reference< rdf::XResource > & i_xSubject,
    const uno::Reference< rdf::XURI > & i_xPredicate,
    const uno::Reference< rdf::XNode > & i_xObject)
throw (uno::RuntimeException,
    container::NoSuchElementException, rdf::RepositoryException)
{
    uno::Reference< rdf::XRepository > xRep( m_wRep );
    if (!xRep.is()) {
        throw rdf::RepositoryException(
            "librdf_NamedGraph::removeStatements: repository is gone", *this);
    }
    m_pRep->removeStatementsGraph(i_xSubject, i_xPredicate, i_xObject, m_xName);
}

uno::Reference< container::XEnumeration > SAL_CALL
librdf_NamedGraph::getStatements(
    const uno::Reference< rdf::XResource > & i_xSubject,
    const uno::Reference< rdf::XURI > & i_xPredicate,
    const uno::Reference< rdf::XNode > & i_xObject)
throw (uno::RuntimeException,
    container::NoSuchElementException, rdf::RepositoryException)
{
    uno::Reference< rdf::XRepository > xRep( m_wRep );
    if (!xRep.is()) {
        throw rdf::RepositoryException(
            "librdf_NamedGraph::getStatements: repository is gone", *this);
    }
    return m_pRep->getStatementsGraph(
            i_xSubject, i_xPredicate, i_xObject, m_xName);
}


////////////////////////////////////////////////////////////////////////////

boost::shared_ptr<librdf_world> librdf_Repository::m_pWorld;
sal_uInt32 librdf_Repository::m_NumInstances = 0;
osl::Mutex librdf_Repository::m_aMutex;

librdf_Repository::librdf_Repository(
        uno::Reference< uno::XComponentContext > const & i_xContext)
    : /*BaseMutex(),*/ m_xContext(i_xContext)
//    m_pWorld  (static_cast<librdf_world  *>(0), safe_librdf_free_world  ),
    , m_pStorage(static_cast<librdf_storage*>(0), safe_librdf_free_storage)
    , m_pModel  (static_cast<librdf_model  *>(0), safe_librdf_free_model  )
    , m_NamedGraphs()
    , m_TypeConverter(i_xContext, *this)
{
    OSL_ENSURE(i_xContext.is(), "librdf_Repository: null context");

    ::osl::MutexGuard g(m_aMutex);
    if (!m_NumInstances++) {
        m_pWorld.reset(m_TypeConverter.createWorld(), safe_librdf_free_world);
    }
}

librdf_Repository::~librdf_Repository()
{
    // must destroy these before world!
    m_pModel.reset();
    m_pStorage.reset();

    // FIXME: so it turns out that calling librdf_free_world will
    //   (via raptor_sax2_finish) call xmlCleanupParser, which will
    //   free libxml2's globals! ARRRGH!!! => never call librdf_free_world
#if 0
    ::osl::MutexGuard g(m_aMutex);
    if (!--m_NumInstances) {
        m_pWorld.reset();
    }
#endif
}

// com.sun.star.uno.XServiceInfo:
::rtl::OUString SAL_CALL librdf_Repository::getImplementationName()
throw (uno::RuntimeException)
{
    return comp_librdf_Repository::_getImplementationName();
}

::sal_Bool SAL_CALL librdf_Repository::supportsService(
    ::rtl::OUString const & serviceName) throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > serviceNames
        = comp_librdf_Repository::_getSupportedServiceNames();
    for (::sal_Int32 i = 0; i < serviceNames.getLength(); ++i) {
        if (serviceNames[i] == serviceName)
            return sal_True;
    }
    return sal_False;
}

uno::Sequence< ::rtl::OUString > SAL_CALL
librdf_Repository::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return comp_librdf_Repository::_getSupportedServiceNames();
}

// ::com::sun::star::rdf::XRepository:
uno::Reference< rdf::XBlankNode > SAL_CALL librdf_Repository::createBlankNode()
throw (uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    const boost::shared_ptr<librdf_node> pNode(
        librdf_new_node_from_blank_identifier(m_pWorld.get(), NULL),
        safe_librdf_free_node);
    if (!pNode) {
        throw uno::RuntimeException(
            "librdf_Repository::createBlankNode: "
            "librdf_new_node_from_blank_identifier failed", *this);
    }
    const unsigned char * id (librdf_node_get_blank_identifier(pNode.get()));
    if (!id) {
        throw uno::RuntimeException(
            "librdf_Repository::createBlankNode: "
            "librdf_node_get_blank_identifier failed", *this);
    }
    const ::rtl::OUString nodeID(::rtl::OUString::createFromAscii(
        reinterpret_cast<const char *>(id)));
    try {
        return rdf::BlankNode::create(m_xContext, nodeID);
    } catch (const lang::IllegalArgumentException & iae) {
        throw lang::WrappedTargetRuntimeException(
                "librdf_Repository::createBlankNode: "
                "illegal blank node label", *this, uno::makeAny(iae));
    }
}

bool formatNeedsBaseURI(::sal_Int16 i_Format)
{
    (void) i_Format; //FIXME any which dont?
    return true;
}

xmlParserInputPtr myExtEntityLoader( const char* /*URL*/, const char* /*ID*/, xmlParserCtxtPtr /*context*/)
{
	return NULL;
}

//void SAL_CALL
uno::Reference<rdf::XNamedGraph> SAL_CALL
librdf_Repository::importGraph(::sal_Int16 i_Format,
    const uno::Reference< io::XInputStream > & i_xInStream,
    const uno::Reference< rdf::XURI > & i_xGraphName,
    const uno::Reference< rdf::XURI > & i_xBaseURI)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    datatransfer::UnsupportedFlavorException,
    container::ElementExistException, rdf::ParseException,
    rdf::RepositoryException, io::IOException)
{
    ::osl::MutexGuard g(m_aMutex);
    if (!i_xInStream.is()) {
        throw lang::IllegalArgumentException(
            "librdf_Repository::importGraph: stream is null", *this, 1);
    }
    //FIXME: other formats
    if (i_Format != rdf::FileFormat::RDF_XML) {
        throw datatransfer::UnsupportedFlavorException(
                "librdf_Repository::importGraph: file format not supported", *this);
    }
    if (!i_xGraphName.is()) {
        throw lang::IllegalArgumentException(
                "librdf_Repository::importGraph: graph name is null", *this, 2);
    }
    if (i_xGraphName->getStringValue().matchAsciiL(s_nsOOo, sizeof(s_nsOOo)-1))
    {
        throw lang::IllegalArgumentException(
                "librdf_Repository::importGraph: URI is reserved", *this, 0);
    }
    if (formatNeedsBaseURI(i_Format) && !i_xBaseURI.is()) {
        throw lang::IllegalArgumentException(
                "librdf_Repository::importGraph: base URI is null", *this, 3);
    }
    OSL_ENSURE(i_xBaseURI.is(), "no base uri");
    const ::rtl::OUString baseURIU( i_xBaseURI->getStringValue() );
    if (baseURIU.indexOf('#') >= 0) {
        throw lang::IllegalArgumentException(
                "librdf_Repository::importGraph: base URI is not absolute", *this, 3);
    }

    const ::rtl::OUString contextU( i_xGraphName->getStringValue() );
    if (m_NamedGraphs.find(contextU) != m_NamedGraphs.end()) {
        throw container::ElementExistException(
                "librdf_Repository::importGraph: graph with given URI exists", *this);
    }
    const ::rtl::OString context(
        ::rtl::OUStringToOString(contextU, RTL_TEXTENCODING_UTF8) );

    const boost::shared_ptr<librdf_node> pContext(
        librdf_new_node_from_uri_string(m_pWorld.get(),
            reinterpret_cast<const unsigned char*> (context.getStr())),
        safe_librdf_free_node);
    if (!pContext) {
        throw uno::RuntimeException(
            "librdf_Repository::importGraph: librdf_new_node_from_uri_string failed", *this);
    }

    const ::rtl::OString baseURI(
        ::rtl::OUStringToOString(baseURIU, RTL_TEXTENCODING_UTF8) );
    const boost::shared_ptr<librdf_uri> pBaseURI(
        librdf_new_uri(m_pWorld.get(),
            reinterpret_cast<const unsigned char*> (baseURI.getStr())),
        safe_librdf_free_uri);
    if (!pBaseURI) {
        throw uno::RuntimeException( "librdf_Repository::importGraph: librdf_new_uri failed", *this);
    }

    const boost::shared_ptr<librdf_parser> pParser(
        librdf_new_parser(m_pWorld.get(), "rdfxml", NULL, NULL),
        safe_librdf_free_parser);
    if (!pParser) {
        throw uno::RuntimeException(
                "librdf_Repository::importGraph: "
                "librdf_new_parser failed", *this);
    }

    xmlExternalEntityLoader oldExtEntityLoader = xmlGetExternalEntityLoader();
    xmlSetExternalEntityLoader( myExtEntityLoader);

    uno::Sequence<sal_Int8> buf;
    uno::Reference<io::XSeekable> xSeekable(i_xInStream, uno::UNO_QUERY);
    // UGLY: if only that redland junk could read streams...
    const sal_Int64 sz( xSeekable.is() ? xSeekable->getLength() : 1 << 20 );
    // exceptions are propagated
    i_xInStream->readBytes( buf, static_cast<sal_Int32>( sz ) );
    const boost::shared_ptr<librdf_stream> pStream(
        librdf_parser_parse_counted_string_as_stream(pParser.get(),
            reinterpret_cast<const unsigned char*>(buf.getConstArray()),
            buf.getLength(), pBaseURI.get()),
        safe_librdf_free_stream);
    if (!pStream) {
        throw rdf::ParseException(
            "librdf_Repository::importGraph: "
            "librdf_parser_parse_counted_string_as_stream failed", *this);
    }
    m_NamedGraphs.insert(std::make_pair(contextU,
        new librdf_NamedGraph(this, i_xGraphName)));
    if (librdf_model_context_add_statements(m_pModel.get(),
            pContext.get(), pStream.get())) {
        throw rdf::RepositoryException(
            "librdf_Repository::importGraph: "
            "librdf_model_context_add_statements failed", *this);
    }

    xmlSetExternalEntityLoader( oldExtEntityLoader);
    return getGraph(i_xGraphName);
}

void addChaffWhenEncryptedStorage(const uno::Reference< io::XOutputStream > &rStream, unsigned char* pBuffer, size_t length)
{
    if (!length)
        return;

    uno::Reference< embed::XEncryptionProtectedSource2 > xEncr(rStream,
        uno::UNO_QUERY);

    bool bAddChaff = xEncr.is() && xEncr->hasEncryptionData();

    // exceptions are propagated
    if (!bAddChaff)
    {
        const uno::Sequence<sal_Int8> buf(
            reinterpret_cast<sal_Int8*>(pBuffer), length);
        rStream->writeBytes(buf);
    }
    else
    {
        unsigned char *postcomment =
            (unsigned char*)strchr((const char*)pBuffer, '\n');
        if (postcomment != NULL)
        {
            ++postcomment;

            size_t preamblelen = postcomment - pBuffer;

            uno::Sequence<sal_Int8> buf(
                reinterpret_cast<sal_Int8*>(pBuffer), preamblelen);
            rStream->writeBytes(buf);

            rtl::OStringBuffer aComment;
            aComment.append("<!--");
            aComment.append(comphelper::xml::makeXMLChaff());
            aComment.append("-->");

            buf = uno::Sequence<sal_Int8>(
                reinterpret_cast<const sal_Int8*>(aComment.getStr()), aComment.getLength());
            rStream->writeBytes(buf);

            buf = uno::Sequence<sal_Int8>(
                reinterpret_cast<sal_Int8*>(postcomment), length-preamblelen);
            rStream->writeBytes(buf);
        }
    }
}

void SAL_CALL
librdf_Repository::exportGraph(::sal_Int16 i_Format,
    const uno::Reference< io::XOutputStream > & i_xOutStream,
    const uno::Reference< rdf::XURI > & i_xGraphName,
    const uno::Reference< rdf::XURI > & i_xBaseURI)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    datatransfer::UnsupportedFlavorException,
    container::NoSuchElementException, rdf::RepositoryException,
    io::IOException)
{
    ::osl::MutexGuard g(m_aMutex);
    if (!i_xOutStream.is()) {
        throw lang::IllegalArgumentException(
                "librdf_Repository::exportGraph: stream is null", *this, 1);
    }
    // FIXME: other formats
    if (i_Format != rdf::FileFormat::RDF_XML) {
        throw datatransfer::UnsupportedFlavorException(
                "librdf_Repository::exportGraph: "
                "file format not supported", *this);
    }
    if (!i_xGraphName.is()) {
        throw lang::IllegalArgumentException(
                "librdf_Repository::exportGraph: "
                "graph name is null", *this, 2);
    }
    if (formatNeedsBaseURI(i_Format) && !i_xBaseURI.is()) {
        throw lang::IllegalArgumentException(
                "librdf_Repository::exportGraph: "
                "base URI is null", *this, 3);
    }
    OSL_ENSURE(i_xBaseURI.is(), "no base uri");
    const ::rtl::OUString baseURIU( i_xBaseURI->getStringValue() );
    if (baseURIU.indexOf('#') >= 0) {
        throw lang::IllegalArgumentException(
                "librdf_Repository::exportGraph: "
                "base URI is not absolute", *this, 3);
    }

    const ::rtl::OUString contextU( i_xGraphName->getStringValue() );
    if (m_NamedGraphs.find(contextU) == m_NamedGraphs.end()) {
        throw container::NoSuchElementException(
                "librdf_Repository::exportGraph: "
                "no graph with given URI exists", *this);
    }
    const ::rtl::OString context(
        ::rtl::OUStringToOString(contextU, RTL_TEXTENCODING_UTF8) );

    const boost::shared_ptr<librdf_node> pContext(
        librdf_new_node_from_uri_string(m_pWorld.get(),
            reinterpret_cast<const unsigned char*> (context.getStr())),
        safe_librdf_free_node);
    if (!pContext) {
        throw uno::RuntimeException(
            "librdf_Repository::exportGraph: "
            "librdf_new_node_from_uri_string failed", *this);
    }
    const ::rtl::OString baseURI(
        ::rtl::OUStringToOString(baseURIU, RTL_TEXTENCODING_UTF8) );
    const boost::shared_ptr<librdf_uri> pBaseURI(
        librdf_new_uri(m_pWorld.get(),
            reinterpret_cast<const unsigned char*> (baseURI.getStr())),
        safe_librdf_free_uri);
    if (!pBaseURI) {
        throw uno::RuntimeException(
            "librdf_Repository::exportGraph: "
            "librdf_new_uri failed", *this);
    }

    const boost::shared_ptr<librdf_stream> pStream(
        librdf_model_context_as_stream(m_pModel.get(), pContext.get()),
        safe_librdf_free_stream);
    if (!pStream) {
        throw rdf::RepositoryException(
            "librdf_Repository::exportGraph: "
            "librdf_model_context_as_stream failed", *this);
    }
    const char *format("rdfxml");
    // #i116443#: abbrev breaks when certain URIs are used as data types
//    const char *format("rdfxml-abbrev");
    const boost::shared_ptr<librdf_serializer> pSerializer(
        librdf_new_serializer(m_pWorld.get(), format, NULL, NULL),
        safe_librdf_free_serializer);
    if (!pSerializer) {
        throw uno::RuntimeException(
            "librdf_Repository::exportGraph: "
            "librdf_new_serializer failed", *this);
    }

    const boost::shared_ptr<librdf_uri> pRelativeURI(
        librdf_new_uri(m_pWorld.get(), reinterpret_cast<const unsigned char*>
                ("http://feature.librdf.org/raptor-relativeURIs")),
                 safe_librdf_free_uri);
    const boost::shared_ptr<librdf_uri> pWriteBaseURI(
        librdf_new_uri(m_pWorld.get(), reinterpret_cast<const unsigned char*>
            ("http://feature.librdf.org/raptor-writeBaseURI")),
             safe_librdf_free_uri);
    const boost::shared_ptr<librdf_node> p0(
        librdf_new_node_from_literal(m_pWorld.get(),
            reinterpret_cast<const unsigned char*> ("0"), NULL, 0),
        safe_librdf_free_node);
    const boost::shared_ptr<librdf_node> p1(
        librdf_new_node_from_literal(m_pWorld.get(),
            reinterpret_cast<const unsigned char*> ("1"), NULL, 0),
        safe_librdf_free_node);
    if (!pWriteBaseURI || !pRelativeURI || !p0 || !p1) {
        throw uno::RuntimeException(
            "librdf_Repository::exportGraph: "
            "librdf_new_uri or librdf_new_node_from_literal failed", *this);
    }

    // make URIs relative to base URI
    if (librdf_serializer_set_feature(pSerializer.get(),
        pRelativeURI.get(), p1.get()))
    {
        throw uno::RuntimeException(
            "librdf_Repository::exportGraph: "
            "librdf_serializer_set_feature relativeURIs failed", *this);
    }
    // but do not write the base URI to the file!
    if (librdf_serializer_set_feature(pSerializer.get(),
        pWriteBaseURI.get(), p0.get()))
    {
        throw uno::RuntimeException(
            "librdf_Repository::exportGraph: "
            "librdf_serializer_set_feature writeBaseURI failed", *this);
    }

    size_t length;
    const boost::shared_ptr<unsigned char> pBuf(
        librdf_serializer_serialize_stream_to_counted_string(
            pSerializer.get(), pBaseURI.get(), pStream.get(), &length), free);
    if (!pBuf) {
        throw rdf::RepositoryException(
            "librdf_Repository::exportGraph: "
            "librdf_serializer_serialize_stream_to_counted_string failed",
            *this);
    }
    addChaffWhenEncryptedStorage(i_xOutStream, pBuf.get(), length);
}

uno::Sequence< uno::Reference< rdf::XURI > > SAL_CALL
librdf_Repository::getGraphNames()
throw (uno::RuntimeException, rdf::RepositoryException)
{
    ::osl::MutexGuard g(m_aMutex);
    ::comphelper::SequenceAsVector< uno::Reference<rdf::XURI> > ret;
    std::transform(m_NamedGraphs.begin(), m_NamedGraphs.end(),
        std::back_inserter(ret),
        boost::bind(&rdf::XNamedGraph::getName,
            boost::bind(&NamedGraphMap_t::value_type::second, _1)));
    return ret.getAsConstList();
}

uno::Reference< rdf::XNamedGraph > SAL_CALL
librdf_Repository::getGraph(const uno::Reference< rdf::XURI > & i_xGraphName)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    rdf::RepositoryException)
{
    ::osl::MutexGuard g(m_aMutex);
    if (!i_xGraphName.is()) {
        throw lang::IllegalArgumentException(
                "librdf_Repository::getGraph: URI is null", *this, 0);
    }
    const NamedGraphMap_t::iterator iter(
        m_NamedGraphs.find(i_xGraphName->getStringValue()) );
    if (iter != m_NamedGraphs.end()) {
        return uno::Reference<rdf::XNamedGraph>(iter->second.get());
    } else {
        return 0;
    }
}

uno::Reference< rdf::XNamedGraph > SAL_CALL
librdf_Repository::createGraph(const uno::Reference< rdf::XURI > & i_xGraphName)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    container::ElementExistException, rdf::RepositoryException)
{
    ::osl::MutexGuard g(m_aMutex);
    if (!i_xGraphName.is()) {
        throw lang::IllegalArgumentException(
                "librdf_Repository::createGraph: URI is null", *this, 0);
    }
    if (i_xGraphName->getStringValue().matchAsciiL(s_nsOOo, sizeof(s_nsOOo)-1))
    {
        throw lang::IllegalArgumentException(
                "librdf_Repository::createGraph: URI is reserved", *this, 0);
    }

    // NB: librdf does not have a concept of graphs as such;
    //     a librdf named graph exists iff the model contains a statement with
    //     the graph name as context
    const ::rtl::OUString contextU( i_xGraphName->getStringValue() );
    if (m_NamedGraphs.find(contextU) != m_NamedGraphs.end()) {
        throw container::ElementExistException(
                "librdf_Repository::createGraph: graph with given URI exists", *this);
    }
    m_NamedGraphs.insert(std::make_pair(contextU,
        new librdf_NamedGraph(this, i_xGraphName)));
    return uno::Reference<rdf::XNamedGraph>(
        m_NamedGraphs.find(contextU)->second.get());
}

void SAL_CALL
librdf_Repository::destroyGraph(
        const uno::Reference< rdf::XURI > & i_xGraphName)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    container::NoSuchElementException, rdf::RepositoryException)
{
    ::osl::MutexGuard g(m_aMutex);
    const NamedGraphMap_t::iterator iter( clearGraph(i_xGraphName) );
    m_NamedGraphs.erase(iter);
}

static bool isMetadatableWithoutMetadata(
    uno::Reference<uno::XInterface> const & i_xNode)
{
    const uno::Reference<rdf::XMetadatable> xMeta( i_xNode, uno::UNO_QUERY );
    return (xMeta.is() && xMeta->getMetadataReference().Second.isEmpty());
}

uno::Reference< container::XEnumeration > SAL_CALL
librdf_Repository::getStatements(
    const uno::Reference< rdf::XResource > & i_xSubject,
    const uno::Reference< rdf::XURI > & i_xPredicate,
    const uno::Reference< rdf::XNode > & i_xObject)
throw (uno::RuntimeException, rdf::RepositoryException)
{
    if (isMetadatableWithoutMetadata(i_xSubject)   ||
        isMetadatableWithoutMetadata(i_xPredicate) ||
        isMetadatableWithoutMetadata(i_xObject))
    {
        return new librdf_GraphResult(this, m_aMutex,
            ::boost::shared_ptr<librdf_stream>(),
            ::boost::shared_ptr<librdf_node>());
    }

    ::osl::MutexGuard g(m_aMutex);
    const boost::shared_ptr<librdf_statement> pStatement(
        m_TypeConverter.mkStatement(m_pWorld.get(),
            i_xSubject, i_xPredicate, i_xObject),
        safe_librdf_free_statement);
    OSL_ENSURE(pStatement, "mkStatement failed");

    const boost::shared_ptr<librdf_stream> pStream(
        librdf_model_find_statements(m_pModel.get(), pStatement.get()),
        safe_librdf_free_stream);
    if (!pStream) {
        throw rdf::RepositoryException(
            "librdf_Repository::getStatements: "
            "librdf_model_find_statements failed", *this);
    }

    return new librdf_GraphResult(this, m_aMutex, pStream,
        ::boost::shared_ptr<librdf_node>());
}


uno::Reference< rdf::XQuerySelectResult > SAL_CALL
librdf_Repository::querySelect(const ::rtl::OUString & i_rQuery)
throw (uno::RuntimeException, rdf::QueryException, rdf::RepositoryException)
{
    ::osl::MutexGuard g(m_aMutex);
    const ::rtl::OString query(
        ::rtl::OUStringToOString(i_rQuery, RTL_TEXTENCODING_UTF8) );
    const boost::shared_ptr<librdf_query> pQuery(
        librdf_new_query(m_pWorld.get(), s_sparql, NULL,
            reinterpret_cast<const unsigned char*> (query.getStr()), NULL),
        safe_librdf_free_query);
    if (!pQuery) {
        throw rdf::QueryException(
            "librdf_Repository::querySelect: "
            "librdf_new_query failed", *this);
    }
    const boost::shared_ptr<librdf_query_results> pResults(
        librdf_model_query_execute(m_pModel.get(), pQuery.get()),
        safe_librdf_free_query_results);
    if (!pResults || !librdf_query_results_is_bindings(pResults.get())) {
        throw rdf::QueryException(
            "librdf_Repository::querySelect: "
            "query result is null or not bindings", *this);
    }

    const int count( librdf_query_results_get_bindings_count(pResults.get()) );
    if (count >= 0) {
        uno::Sequence< ::rtl::OUString > names(count);
        for (int i = 0; i < count; ++i) {
            const char* name( librdf_query_results_get_binding_name(
                pResults.get(), i) );
            if (!name) {
                throw rdf::QueryException(
                    "librdf_Repository::querySelect: binding is null", *this);
            }

            names[i] = ::rtl::OUString::createFromAscii(name);
        }

        return new librdf_QuerySelectResult(this, m_aMutex,
            pQuery, pResults, names);

    } else {
        throw rdf::QueryException(
            "librdf_Repository::querySelect: "
            "librdf_query_results_get_bindings_count failed", *this);
    }
}

uno::Reference< container::XEnumeration > SAL_CALL
librdf_Repository::queryConstruct(const ::rtl::OUString & i_rQuery)
throw (uno::RuntimeException, rdf::QueryException, rdf::RepositoryException)
{
    ::osl::MutexGuard g(m_aMutex);
    const ::rtl::OString query(
        ::rtl::OUStringToOString(i_rQuery, RTL_TEXTENCODING_UTF8) );
    const boost::shared_ptr<librdf_query> pQuery(
        librdf_new_query(m_pWorld.get(), s_sparql, NULL,
            reinterpret_cast<const unsigned char*> (query.getStr()), NULL),
        safe_librdf_free_query);
    if (!pQuery) {
        throw rdf::QueryException(
            "librdf_Repository::queryConstruct: "
            "librdf_new_query failed", *this);
    }
    const boost::shared_ptr<librdf_query_results> pResults(
        librdf_model_query_execute(m_pModel.get(), pQuery.get()),
        safe_librdf_free_query_results);
    if (!pResults || !librdf_query_results_is_graph(pResults.get())) {
        throw rdf::QueryException(
            "librdf_Repository::queryConstruct: "
            "query result is null or not graph", *this);
    }
    const boost::shared_ptr<librdf_stream> pStream(
        librdf_query_results_as_stream(pResults.get()),
        safe_librdf_free_stream);
    if (!pStream) {
        throw rdf::QueryException(
            "librdf_Repository::queryConstruct: "
            "librdf_query_results_as_stream failed", *this);
    }

    return new librdf_GraphResult(this, m_aMutex, pStream,
                                  ::boost::shared_ptr<librdf_node>(), pQuery);
}

::sal_Bool SAL_CALL
librdf_Repository::queryAsk(const ::rtl::OUString & i_rQuery)
throw (uno::RuntimeException, rdf::QueryException, rdf::RepositoryException)
{
    ::osl::MutexGuard g(m_aMutex);

    const ::rtl::OString query(
        ::rtl::OUStringToOString(i_rQuery, RTL_TEXTENCODING_UTF8) );
    const boost::shared_ptr<librdf_query> pQuery(
        librdf_new_query(m_pWorld.get(), s_sparql, NULL,
            reinterpret_cast<const unsigned char*> (query.getStr()), NULL),
        safe_librdf_free_query);
    if (!pQuery) {
        throw rdf::QueryException(
            "librdf_Repository::queryAsk: "
            "librdf_new_query failed", *this);
    }
    const boost::shared_ptr<librdf_query_results> pResults(
        librdf_model_query_execute(m_pModel.get(), pQuery.get()),
        safe_librdf_free_query_results);
    if (!pResults || !librdf_query_results_is_boolean(pResults.get())) {
        throw rdf::QueryException(
            "librdf_Repository::queryAsk: "
            "query result is null or not boolean", *this);
    }
    return librdf_query_results_get_boolean(pResults.get())
        ? sal_True : sal_False;
}

// ::com::sun::star::rdf::XDocumentRepository:
void SAL_CALL librdf_Repository::setStatementRDFa(
    const uno::Reference< rdf::XResource > & i_xSubject,
    const uno::Sequence< uno::Reference< rdf::XURI > > & i_rPredicates,
    const uno::Reference< rdf::XMetadatable > & i_xObject,
    const ::rtl::OUString & i_rRDFaContent,
    const uno::Reference< rdf::XURI > & i_xRDFaDatatype)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    rdf::RepositoryException)
{
    static const ::rtl::OUString s_cell("com.sun.star.table.Cell");
    static const ::rtl::OUString s_cellprops("com.sun.star.text.CellProperties"); // for writer
    static const ::rtl::OUString s_paragraph("com.sun.star.text.Paragraph");
    static const ::rtl::OUString s_bookmark("com.sun.star.text.Bookmark");
    static const ::rtl::OUString s_meta("com.sun.star.text.InContentMetadata");

    if (!i_xSubject.is()) {
        throw lang::IllegalArgumentException(
            "librdf_Repository::setStatementRDFa: Subject is null", *this, 0);
    }
    if (!i_rPredicates.getLength()) {
        throw lang::IllegalArgumentException(
            "librdf_Repository::setStatementRDFa: no Predicates",
            *this, 1);
    }
    for (sal_Int32 i = 0; i < i_rPredicates.getLength(); ++i) {
        if (!i_rPredicates[i].is()) {
            throw lang::IllegalArgumentException(
                    "librdf_Repository::setStatementRDFa: Predicate is null",
                *this, 1);
        }
    }
    if (!i_xObject.is()) {
        throw lang::IllegalArgumentException(
            "librdf_Repository::setStatementRDFa: Object is null", *this, 2);
    }
    const uno::Reference<lang::XServiceInfo> xService(i_xObject,
        uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextRange> xTextRange;
    if (xService->supportsService(s_cell) ||
        xService->supportsService(s_cellprops) ||
        xService->supportsService(s_paragraph))
    {
        xTextRange.set(i_xObject, uno::UNO_QUERY_THROW);
    }
    else if (xService->supportsService(s_bookmark) ||
             xService->supportsService(s_meta))
    {
        const uno::Reference<text::XTextContent> xTextContent(i_xObject,
            uno::UNO_QUERY_THROW);
        xTextRange = xTextContent->getAnchor();
    }
    if (!xTextRange.is()) {
        throw lang::IllegalArgumentException(
            "librdf_Repository::setStatementRDFa: "
            "Object does not support RDFa", *this, 2);
    }
    // ensure that the metadatable has an XML ID
    i_xObject->ensureMetadataReference();
    const beans::StringPair mdref( i_xObject->getMetadataReference() );
    if ((mdref.First.isEmpty()) || (mdref.Second.isEmpty())) {
        throw uno::RuntimeException(
                "librdf_Repository::setStatementRDFa: "
                "ensureMetadataReference did not", *this);
    }
    ::rtl::OUString const sXmlId(mdref.First + "#" + mdref.Second);
    uno::Reference<rdf::XURI> xXmlId;
    try {
        xXmlId.set( rdf::URI::create(m_xContext,
                ::rtl::OUString::createFromAscii(s_nsOOo) + sXmlId),
            uno::UNO_QUERY_THROW);
    } catch (const lang::IllegalArgumentException & iae) {
        throw lang::WrappedTargetRuntimeException(
                "librdf_Repository::setStatementRDFa: "
                "cannot create URI for XML ID", *this, uno::makeAny(iae));
    }

    ::osl::MutexGuard g(m_aMutex);
    ::rtl::OUString const content( (i_rRDFaContent.isEmpty())
            ? xTextRange->getString()
            : i_rRDFaContent );
    uno::Reference<rdf::XNode> xContent;
    try {
        if (i_xRDFaDatatype.is()) {
            xContent.set(rdf::Literal::createWithType(m_xContext,
                    content, i_xRDFaDatatype),
                uno::UNO_QUERY_THROW);
        } else {
            xContent.set(rdf::Literal::create(m_xContext, content),
                uno::UNO_QUERY_THROW);
        }
    } catch (const lang::IllegalArgumentException & iae) {
        throw lang::WrappedTargetRuntimeException(
                "librdf_Repository::setStatementRDFa: "
                "cannot create literal", *this, uno::makeAny(iae));
    }
    removeStatementRDFa(i_xObject);
    if (i_rRDFaContent.isEmpty()) {
        m_RDFaXHTMLContentSet.erase(sXmlId);
    } else {
        m_RDFaXHTMLContentSet.insert(sXmlId);
    }
    ::std::for_each(::comphelper::stl_begin(i_rPredicates),
        ::comphelper::stl_end(i_rPredicates),
        ::boost::bind( &librdf_Repository::addStatementGraph,
            this, i_xSubject, _1, xContent, xXmlId, true));
}

void SAL_CALL librdf_Repository::removeStatementRDFa(
    const uno::Reference< rdf::XMetadatable > & i_xElement)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    rdf::RepositoryException)
{
    if (!i_xElement.is()) {
        throw lang::IllegalArgumentException(
            "librdf_Repository::removeStatementRDFa: Element is null",
            *this, 0);
    }

    const beans::StringPair mdref( i_xElement->getMetadataReference() );
    if ((mdref.First.isEmpty()) || (mdref.Second.isEmpty())) {
        return; // nothing to do...
    }
    uno::Reference<rdf::XURI> xXmlId;
    try {
        xXmlId.set( rdf::URI::create(m_xContext,
                ::rtl::OUString::createFromAscii(s_nsOOo)
                + mdref.First + "#"
                + mdref.Second),
            uno::UNO_QUERY_THROW);
    } catch (const lang::IllegalArgumentException & iae) {
        throw lang::WrappedTargetRuntimeException(
                "librdf_Repository::removeStatementRDFa: "
                "cannot create URI for XML ID", *this, uno::makeAny(iae));
    }
    // clearGraph does locking, not needed here
    clearGraph(xXmlId, true);
}

beans::Pair< uno::Sequence<rdf::Statement>, sal_Bool > SAL_CALL
librdf_Repository::getStatementRDFa(
    const uno::Reference< rdf::XMetadatable > & i_xElement)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    rdf::RepositoryException)
{
    if (!i_xElement.is()) {
        throw lang::IllegalArgumentException(
            "librdf_Repository::getStatementRDFa: Element is null", *this, 0);
    }
    const beans::StringPair mdref( i_xElement->getMetadataReference() );
    if ((mdref.First.isEmpty()) || (mdref.Second.isEmpty())) {
        return beans::Pair< uno::Sequence<rdf::Statement>, sal_Bool >();
    }
    ::rtl::OUString const sXmlId(mdref.First + "#" + mdref.Second);
    uno::Reference<rdf::XURI> xXmlId;
    try {
        xXmlId.set( rdf::URI::create(m_xContext,
                ::rtl::OUString::createFromAscii(s_nsOOo) + sXmlId),
            uno::UNO_QUERY_THROW);
    } catch (const lang::IllegalArgumentException & iae) {
        throw lang::WrappedTargetRuntimeException(
                "librdf_Repository::getStatementRDFa: "
                "cannot create URI for XML ID", *this, uno::makeAny(iae));
    }

    ::osl::MutexGuard g(m_aMutex);
    ::comphelper::SequenceAsVector< rdf::Statement > ret;
    const uno::Reference<container::XEnumeration> xIter(
        getStatementsGraph(0, 0, 0, xXmlId, true) );
    OSL_ENSURE(xIter.is(), "getStatementRDFa: no result?");
    if (!xIter.is()) throw uno::RuntimeException();
    while (xIter->hasMoreElements()) {
        rdf::Statement stmt;
        if (!(xIter->nextElement() >>= stmt)) {
            OSL_FAIL("getStatementRDFa: result of wrong type?");
        } else {
            ret.push_back(stmt);
        }
    }
    return beans::Pair< uno::Sequence<rdf::Statement>, sal_Bool >(
            ret.getAsConstList(), 0 != m_RDFaXHTMLContentSet.count(sXmlId));
}

extern "C"
librdf_statement *rdfa_context_stream_map_handler(
    librdf_stream *i_pStream, void *, librdf_statement *i_pStatement)
{
    OSL_ENSURE(i_pStream, "rdfa_context_stream_map_handler: stream null");
    if (i_pStream) {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH;
        librdf_node *pCtxt( static_cast<librdf_node *>
            (librdf_stream_get_context(i_pStream)) );
        SAL_WNODEPRECATED_DECLARATIONS_POP;
        OSL_ENSURE(pCtxt, "rdfa_context_stream_map_handler: context null");
        if (pCtxt && isInternalContext(pCtxt)) {
            return i_pStatement;
        }
    }
    return 0;
};

uno::Reference< container::XEnumeration > SAL_CALL
librdf_Repository::getStatementsRDFa(
    const uno::Reference< rdf::XResource > & i_xSubject,
    const uno::Reference< rdf::XURI > & i_xPredicate,
    const uno::Reference< rdf::XNode > & i_xObject)
throw (uno::RuntimeException, rdf::RepositoryException)
{
    if (isMetadatableWithoutMetadata(i_xSubject)   ||
        isMetadatableWithoutMetadata(i_xPredicate) ||
        isMetadatableWithoutMetadata(i_xObject))
    {
        return new librdf_GraphResult(this, m_aMutex,
            ::boost::shared_ptr<librdf_stream>(),
            ::boost::shared_ptr<librdf_node>());
    }

    ::osl::MutexGuard g(m_aMutex);
    const boost::shared_ptr<librdf_statement> pStatement(
        m_TypeConverter.mkStatement(m_pWorld.get(),
            i_xSubject, i_xPredicate, i_xObject),
        safe_librdf_free_statement);
    OSL_ENSURE(pStatement, "mkStatement failed");

    const boost::shared_ptr<librdf_stream> pStream(
        librdf_model_find_statements(m_pModel.get(), pStatement.get()),
        safe_librdf_free_stream);
    if (!pStream) {
        throw rdf::RepositoryException(
            "librdf_Repository::getStatementsRDFa: "
            "librdf_model_find_statements failed", *this);
    }

    if (librdf_stream_add_map(pStream.get(), rdfa_context_stream_map_handler,
                0, 0)) {
        throw rdf::RepositoryException(
            "librdf_Repository::getStatementsRDFa: "
            "librdf_stream_add_map failed", *this);
    }

    return new librdf_GraphResult(this, m_aMutex, pStream,
                                  ::boost::shared_ptr<librdf_node>());
}

// ::com::sun::star::lang::XInitialization:
void SAL_CALL librdf_Repository::initialize(
    const uno::Sequence< ::com::sun::star::uno::Any > & i_rArguments)
throw (uno::RuntimeException, uno::Exception)
{
    (void) i_rArguments;

    ::osl::MutexGuard g(m_aMutex);

//    m_pWorld.reset(m_TypeConverter.createWorld(), safe_librdf_free_world);
    m_pStorage.reset(m_TypeConverter.createStorage(m_pWorld.get()),
        safe_librdf_free_storage);
    m_pModel.reset(m_TypeConverter.createModel(
        m_pWorld.get(), m_pStorage.get()), safe_librdf_free_model);
}

const NamedGraphMap_t::iterator SAL_CALL librdf_Repository::clearGraph(
        const uno::Reference< rdf::XURI > & i_xGraphName, bool i_Internal)
//    throw (uno::RuntimeException, container::NoSuchElementException,
//        rdf::RepositoryException)
{
    if (!i_xGraphName.is()) {
        throw lang::IllegalArgumentException(
                "librdf_Repository::clearGraph: URI is null", *this, 0);
    }
    ::osl::MutexGuard g(m_aMutex);
    const ::rtl::OUString contextU( i_xGraphName->getStringValue() );
    const NamedGraphMap_t::iterator iter( m_NamedGraphs.find(contextU) );
    if (!i_Internal && iter == m_NamedGraphs.end()) {
        throw container::NoSuchElementException(
                "librdf_Repository::clearGraph: "
                "no graph with given URI exists", *this);
    }
    const ::rtl::OString context(
        ::rtl::OUStringToOString(contextU, RTL_TEXTENCODING_UTF8) );

    const boost::shared_ptr<librdf_node> pContext(
        librdf_new_node_from_uri_string(m_pWorld.get(),
            reinterpret_cast<const unsigned char*> (context.getStr())),
        safe_librdf_free_node);
    if (!pContext) {
        throw uno::RuntimeException(
            "librdf_Repository::clearGraph: "
            "librdf_new_node_from_uri_string failed", *this);
    }
    if (librdf_model_context_remove_statements(m_pModel.get(), pContext.get()))
    {
        throw rdf::RepositoryException(
            "librdf_Repository::clearGraph: "
            "librdf_model_context_remove_statements failed", *this);
    }
    return iter;
}

void librdf_Repository::addStatementGraph(
    const uno::Reference< rdf::XResource > & i_xSubject,
    const uno::Reference< rdf::XURI > & i_xPredicate,
    const uno::Reference< rdf::XNode > & i_xObject,
    const uno::Reference< rdf::XURI > & i_xGraphName,
    bool i_Internal)
//throw (uno::RuntimeException, lang::IllegalArgumentException,
//    container::NoSuchElementException, rdf::RepositoryException)
{
    if (!i_xSubject.is()) {
        throw lang::IllegalArgumentException(
            "librdf_Repository::addStatement: Subject is null", *this, 0);
    }
    if (!i_xPredicate.is()) {
        throw lang::IllegalArgumentException(
            "librdf_Repository::addStatement: Predicate is null",
            *this, 1);
    }
    if (!i_xObject.is()) {
        throw lang::IllegalArgumentException(
            "librdf_Repository::addStatement: Object is null", *this, 2);
    }

    ::osl::MutexGuard g(m_aMutex);
    const ::rtl::OUString contextU( i_xGraphName->getStringValue() );
    if (!i_Internal && (m_NamedGraphs.find(contextU) == m_NamedGraphs.end())) {
        throw container::NoSuchElementException(
                "librdf_Repository::addStatement: "
                "no graph with given URI exists", *this);
    }
    const ::rtl::OString context(
        ::rtl::OUStringToOString(contextU, RTL_TEXTENCODING_UTF8) );

    const boost::shared_ptr<librdf_node> pContext(
        librdf_new_node_from_uri_string(m_pWorld.get(),
            reinterpret_cast<const unsigned char*> (context.getStr())),
        safe_librdf_free_node);
    if (!pContext) {
        throw uno::RuntimeException(
            "librdf_Repository::addStatement: "
            "librdf_new_node_from_uri_string failed", *this);
    }
    const boost::shared_ptr<librdf_statement> pStatement(
        m_TypeConverter.mkStatement(m_pWorld.get(),
            i_xSubject, i_xPredicate, i_xObject),
        safe_librdf_free_statement);
    OSL_ENSURE(pStatement, "mkStatement failed");

    // Test for duplicate statement
    // librdf_model_add_statement disallows duplicates while
    // librdf_model_context_add_statement allows duplicates
    {
        const boost::shared_ptr<librdf_stream> pStream(
            librdf_model_find_statements_in_context(m_pModel.get(),
                pStatement.get(), pContext.get()),
            safe_librdf_free_stream);
        if (pStream && !librdf_stream_end(pStream.get()))
            return;
    }

    if (librdf_model_context_add_statement(m_pModel.get(),
            pContext.get(), pStatement.get())) {
        throw rdf::RepositoryException(
            "librdf_Repository::addStatement: "
            "librdf_model_context_add_statement failed", *this);
    }
}

void librdf_Repository::removeStatementsGraph(
    const uno::Reference< rdf::XResource > & i_xSubject,
    const uno::Reference< rdf::XURI > & i_xPredicate,
    const uno::Reference< rdf::XNode > & i_xObject,
    const uno::Reference< rdf::XURI > & i_xGraphName)
//throw (uno::RuntimeException, lang::IllegalArgumentException,
//    container::NoSuchElementException, rdf::RepositoryException)
{
    if (isMetadatableWithoutMetadata(i_xSubject)   ||
        isMetadatableWithoutMetadata(i_xPredicate) ||
        isMetadatableWithoutMetadata(i_xObject))
    {
        return;
    }

    ::osl::MutexGuard g(m_aMutex);
    const ::rtl::OUString contextU( i_xGraphName->getStringValue() );
    if (m_NamedGraphs.find(contextU) == m_NamedGraphs.end()) {
        throw container::NoSuchElementException(
                "librdf_Repository::removeStatements: "
                "no graph with given URI exists", *this);
    }
    const ::rtl::OString context(
        ::rtl::OUStringToOString(contextU, RTL_TEXTENCODING_UTF8) );

    const boost::shared_ptr<librdf_node> pContext(
        librdf_new_node_from_uri_string(m_pWorld.get(),
            reinterpret_cast<const unsigned char*> (context.getStr())),
        safe_librdf_free_node);
    if (!pContext) {
        throw uno::RuntimeException(
            "librdf_Repository::removeStatements: "
            "librdf_new_node_from_uri_string failed", *this);
    }
    const boost::shared_ptr<librdf_statement> pStatement(
        m_TypeConverter.mkStatement(m_pWorld.get(),
            i_xSubject, i_xPredicate, i_xObject),
        safe_librdf_free_statement);
    OSL_ENSURE(pStatement, "mkStatement failed");

    const boost::shared_ptr<librdf_stream> pStream(
        librdf_model_find_statements_in_context(m_pModel.get(),
            pStatement.get(), pContext.get()),
        safe_librdf_free_stream);
    if (!pStream) {
        throw rdf::RepositoryException(
            "librdf_Repository::removeStatements: "
            "librdf_model_find_statements_in_context failed", *this);
    }

    if (!librdf_stream_end(pStream.get())) {
        do {
            librdf_statement *pStmt( librdf_stream_get_object(pStream.get()) );
            if (!pStmt) {
                throw rdf::RepositoryException(
                    "librdf_Repository::removeStatements: "
                    "librdf_stream_get_object failed", *this);
            }
            if (librdf_model_context_remove_statement(m_pModel.get(),
                    pContext.get(), pStmt)) {
                throw rdf::RepositoryException(
                    "librdf_Repository::removeStatements: "
                    "librdf_model_context_remove_statement failed", *this);
            }
        } while (!librdf_stream_next(pStream.get()));
    }
}

uno::Reference< container::XEnumeration >
librdf_Repository::getStatementsGraph(
    const uno::Reference< rdf::XResource > & i_xSubject,
    const uno::Reference< rdf::XURI > & i_xPredicate,
    const uno::Reference< rdf::XNode > & i_xObject,
    const uno::Reference< rdf::XURI > & i_xGraphName,
    bool i_Internal)
//throw (uno::RuntimeException, lang::IllegalArgumentException,
//    container::NoSuchElementException, rdf::RepositoryException)
{
    // N.B.: if any of subject, predicate, object is an XMetadatable, and
    // has no metadata reference, then there cannot be any node in the graph
    // representing it; in order to prevent side effect
    // (ensureMetadataReference), check for this condition and return
    if (isMetadatableWithoutMetadata(i_xSubject)   ||
        isMetadatableWithoutMetadata(i_xPredicate) ||
        isMetadatableWithoutMetadata(i_xObject))
    {
        return new librdf_GraphResult(this, m_aMutex,
            ::boost::shared_ptr<librdf_stream>(),
            ::boost::shared_ptr<librdf_node>());
    }

    ::osl::MutexGuard g(m_aMutex);
    const ::rtl::OUString contextU( i_xGraphName->getStringValue() );
    if (!i_Internal && (m_NamedGraphs.find(contextU) == m_NamedGraphs.end())) {
        throw container::NoSuchElementException(
                "librdf_Repository::getStatements: "
                "no graph with given URI exists", *this);
    }
    const ::rtl::OString context(
        ::rtl::OUStringToOString(contextU, RTL_TEXTENCODING_UTF8) );

    const boost::shared_ptr<librdf_node> pContext(
        librdf_new_node_from_uri_string(m_pWorld.get(),
            reinterpret_cast<const unsigned char*> (context.getStr())),
        safe_librdf_free_node);
    if (!pContext) {
        throw uno::RuntimeException(
            "librdf_Repository::getStatements: "
            "librdf_new_node_from_uri_string failed", *this);
    }
    const boost::shared_ptr<librdf_statement> pStatement(
        m_TypeConverter.mkStatement(m_pWorld.get(),
            i_xSubject, i_xPredicate, i_xObject),
        safe_librdf_free_statement);
    OSL_ENSURE(pStatement, "mkStatement failed");

    const boost::shared_ptr<librdf_stream> pStream(
        librdf_model_find_statements_in_context(m_pModel.get(),
            pStatement.get(), pContext.get()),
        safe_librdf_free_stream);
    if (!pStream) {
        throw rdf::RepositoryException(
            "librdf_Repository::getStatements: "
            "librdf_model_find_statements_in_context failed", *this);
    }

    // librdf_model_find_statements_in_context is buggy and does not put
    // the context into result statements; pass it to librdf_GraphResult here
    return new librdf_GraphResult(this, m_aMutex, pStream, pContext);
}

librdf_world *librdf_TypeConverter::createWorld() const
{
    // create and initialize world
    librdf_world *pWorld( librdf_new_world() );
    if (!pWorld) {
        throw uno::RuntimeException(
            "librdf_TypeConverter::createWorld: librdf_new_world failed",
            m_rRep);
    }
    //FIXME logger, digest, features?
    xsltSecurityPrefsPtr origprefs = xsltGetDefaultSecurityPrefs();
    librdf_world_open(pWorld);
    xsltSecurityPrefsPtr newprefs = xsltGetDefaultSecurityPrefs();
    if (newprefs != origprefs) {
        // #i110523# restore libxslt global configuration
        // (gratuitously overwritten by raptor_init_parser_grddl_common)
        // (this is the only reason unordf is linked against libxslt)
        xsltSetDefaultSecurityPrefs(origprefs);
    }
    return pWorld;
}

librdf_storage *
librdf_TypeConverter::createStorage(librdf_world *i_pWorld) const
{
    librdf_storage *pStorage(
//        librdf_new_storage(i_pWorld, "memory", NULL, "contexts='yes'") );
        librdf_new_storage(i_pWorld, "hashes", NULL,
            "contexts='yes',hash-type='memory'") );
    if (!pStorage) {
        throw uno::RuntimeException(
            "librdf_TypeConverter::createStorage: librdf_new_storage failed",
            m_rRep);
    }
    return pStorage;
}

librdf_model *librdf_TypeConverter::createModel(
    librdf_world *i_pWorld, librdf_storage * i_pStorage) const
{
    librdf_model *pRepository( librdf_new_model(i_pWorld, i_pStorage, NULL) );
    if (!pRepository) {
        throw uno::RuntimeException(
            "librdf_TypeConverter::createModel: librdf_new_model failed",
            m_rRep);
    }
    //FIXME
#if 0
    {
        librdf_uri * ctxt = librdf_new_uri(i_pWorld, reinterpret_cast<const unsigned char *>(LIBRDF_MODEL_FEATURE_CONTEXTS));
        librdf_node * contexts = librdf_model_get_feature(repository, ctxt);
        if (!contexts)
            throw;
        std::cout << "value of contexts feature: ";
        prtNode(contexts);
        std::cout << std::endl;
        // librdf_model_set_feature(repository, LIBRDF_FEATURE_CONTEXTS, ...);
        safe_librdf_free_node(contexts);
        safe_librdf_free_uri(ctxt);
    }
#endif
    return pRepository;
}

// this does NOT create a node, only URI
librdf_uri* librdf_TypeConverter::mkURI( librdf_world* i_pWorld,
    const uno::Reference< rdf::XURI > & i_xURI) const
{
    const ::rtl::OString uri(
        ::rtl::OUStringToOString(i_xURI->getStringValue(),
        RTL_TEXTENCODING_UTF8) );
    librdf_uri *pURI( librdf_new_uri(i_pWorld,
        reinterpret_cast<const unsigned char *>(uri.getStr())));
    if (!pURI) {
        throw uno::RuntimeException(
            "librdf_TypeConverter::mkURI: librdf_new_uri failed", 0);
    }
    return pURI;
}

// create blank or URI node
librdf_node* librdf_TypeConverter::mkResource( librdf_world* i_pWorld,
    const uno::Reference< rdf::XResource > & i_xResource) const
{
    if (!i_xResource.is()) return 0;
    uno::Reference< rdf::XBlankNode > xBlankNode(i_xResource, uno::UNO_QUERY);
    if (xBlankNode.is()) {
        const ::rtl::OString label(
            ::rtl::OUStringToOString(xBlankNode->getStringValue(),
            RTL_TEXTENCODING_UTF8) );
        librdf_node *pNode(
            librdf_new_node_from_blank_identifier(i_pWorld,
                reinterpret_cast<const unsigned char*> (label.getStr())));
        if (!pNode) {
            throw uno::RuntimeException(
                "librdf_TypeConverter::mkResource: "
                "librdf_new_node_from_blank_identifier failed", 0);
        }
        return pNode;
    } else { // assumption: everything else is URI
        const ::rtl::OString uri(
            ::rtl::OUStringToOString(i_xResource->getStringValue(),
            RTL_TEXTENCODING_UTF8) );
        librdf_node *pNode(
            librdf_new_node_from_uri_string(i_pWorld,
                reinterpret_cast<const unsigned char*> (uri.getStr())));
        if (!pNode) {
            throw uno::RuntimeException(
                "librdf_TypeConverter::mkResource: "
                "librdf_new_node_from_uri_string failed", 0);
        }
        return pNode;
    }
}

// create blank or URI or literal node
librdf_node* librdf_TypeConverter::mkNode( librdf_world* i_pWorld,
    const uno::Reference< rdf::XNode > & i_xNode) const
{
    if (!i_xNode.is()) return 0;
    uno::Reference< rdf::XResource > xResource(i_xNode, uno::UNO_QUERY);
    if (xResource.is()) {
        return mkResource(i_pWorld, xResource);
    }
    uno::Reference< rdf::XLiteral> xLiteral(i_xNode, uno::UNO_QUERY);
    OSL_ENSURE(xLiteral.is(),
        "mkNode: someone invented a new rdf.XNode and did not tell me");
    if (!xLiteral.is()) return 0;
    const ::rtl::OString val(
        ::rtl::OUStringToOString(xLiteral->getValue(),
        RTL_TEXTENCODING_UTF8) );
    const ::rtl::OString lang(
        ::rtl::OUStringToOString(xLiteral->getLanguage(),
        RTL_TEXTENCODING_UTF8) );
    const uno::Reference< rdf::XURI > xType(xLiteral->getDatatype());
    librdf_node * ret(0);
    if (lang.isEmpty()) {
        if (!xType.is()) {
            ret = librdf_new_node_from_literal(i_pWorld,
                reinterpret_cast<const unsigned char*> (val.getStr()),
                NULL, 0);
        } else {
            const boost::shared_ptr<librdf_uri> pDatatype(
                mkURI(i_pWorld, xType), safe_librdf_free_uri);
            ret = librdf_new_node_from_typed_literal(i_pWorld,
                reinterpret_cast<const unsigned char*> (val.getStr()),
                NULL, pDatatype.get());
        }
    } else {
        if (!xType.is()) {
            ret = librdf_new_node_from_literal(i_pWorld,
                reinterpret_cast<const unsigned char*> (val.getStr()),
                (lang.getStr()), 0);

        } else {
            OSL_FAIL("mkNode: invalid literal");
            return 0;
        }
    }
    if (!ret) {
        throw uno::RuntimeException(
            "librdf_TypeConverter::mkNode: librdf_new_node_from_literal failed", 0);
    }
    return ret;
}

librdf_statement* librdf_TypeConverter::mkStatement( librdf_world* i_pWorld,
    const uno::Reference< rdf::XResource > & i_xSubject,
    const uno::Reference< rdf::XURI > & i_xPredicate,
    const uno::Reference< rdf::XNode > & i_xObject) const
{
    librdf_node* pSubject( mkResource(i_pWorld, i_xSubject) );
    librdf_node* pPredicate(0);
    librdf_node* pObject(0);
    try {
        const uno::Reference<rdf::XResource> xPredicate(i_xPredicate,
            uno::UNO_QUERY);
        pPredicate = mkResource(i_pWorld, xPredicate);
        try {
            pObject = mkNode(i_pWorld, i_xObject);
        } catch (...) {
            safe_librdf_free_node(pPredicate);
            throw;
        }
    } catch (...) {
        safe_librdf_free_node(pSubject);
        throw;
    }
    // NB: this takes ownership of the nodes! (which is really ugly)
    librdf_statement* pStatement( librdf_new_statement_from_nodes(i_pWorld,
        pSubject, pPredicate, pObject) );
    if (!pStatement) {
        throw uno::RuntimeException(
            "librdf_TypeConverter::mkStatement: "
            "librdf_new_statement_from_nodes failed", 0);
    }
    return pStatement;
}

uno::Reference<rdf::XURI>
librdf_TypeConverter::convertToXURI(librdf_uri* i_pURI) const
{
    if (!i_pURI) return 0;
    const unsigned char* uri( librdf_uri_as_string(i_pURI) );
    if (!uri) {
        throw uno::RuntimeException(
            "librdf_TypeConverter::convertToXURI: "
            "librdf_uri_as_string failed", m_rRep);
    }
    ::rtl::OUString uriU( ::rtl::OStringToOUString(
        ::rtl::OString(reinterpret_cast<const sal_Char*>(uri)),
        RTL_TEXTENCODING_UTF8) );
    try {
        return rdf::URI::create(m_xContext, uriU);
    } catch (const lang::IllegalArgumentException & iae) {
        throw lang::WrappedTargetRuntimeException(
                "librdf_TypeConverter::convertToXURI: "
                "illegal uri", m_rRep, uno::makeAny(iae));
    }
}

uno::Reference<rdf::XURI>
librdf_TypeConverter::convertToXURI(librdf_node* i_pNode) const
{
    if (!i_pNode) return 0;
    if (librdf_node_is_resource(i_pNode)) {
        librdf_uri* pURI( librdf_node_get_uri(i_pNode) );
        if (!pURI) {
            throw uno::RuntimeException(
                "librdf_TypeConverter::convertToXURI: "
                "resource has no uri", m_rRep);
        }
        return convertToXURI(pURI);
    } else {
        OSL_FAIL("convertToXURI: unknown librdf_node");
        return 0;
    }
}

uno::Reference<rdf::XResource>
librdf_TypeConverter::convertToXResource(librdf_node* i_pNode) const
{
    if (!i_pNode) return 0;
    if (librdf_node_is_blank(i_pNode)) {
        const unsigned char* label( librdf_node_get_blank_identifier(i_pNode) );
        if (!label) {
            throw uno::RuntimeException(
                "librdf_TypeConverter::convertToXResource: "
                "blank node has no label", m_rRep);
        }
        ::rtl::OUString labelU( ::rtl::OStringToOUString(
            ::rtl::OString(reinterpret_cast<const sal_Char*>(label)),
            RTL_TEXTENCODING_UTF8) );
        try {
            return uno::Reference<rdf::XResource>(
                rdf::BlankNode::create(m_xContext, labelU), uno::UNO_QUERY);
        } catch (const lang::IllegalArgumentException & iae) {
            throw lang::WrappedTargetRuntimeException(
                    "librdf_TypeConverter::convertToXResource: "
                    "illegal blank node label", m_rRep, uno::makeAny(iae));
        }
    } else {
        return uno::Reference<rdf::XResource>(convertToXURI(i_pNode),
            uno::UNO_QUERY);
    }
}

uno::Reference<rdf::XNode>
librdf_TypeConverter::convertToXNode(librdf_node* i_pNode) const
{
    if (!i_pNode) return 0;
    if (!librdf_node_is_literal(i_pNode)) {
        return uno::Reference<rdf::XNode>(convertToXResource(i_pNode),
            uno::UNO_QUERY);
    }
    const unsigned char* value( librdf_node_get_literal_value(i_pNode) );
    if (!value) {
        throw uno::RuntimeException(
            "librdf_TypeConverter::convertToXNode: "
            "literal has no value", m_rRep);
    }
    const char * lang( librdf_node_get_literal_value_language(i_pNode) );
    librdf_uri* pType(
        librdf_node_get_literal_value_datatype_uri(i_pNode) );
    OSL_ENSURE(!lang || !pType, "convertToXNode: invalid literal");
    const ::rtl::OUString valueU( ::rtl::OStringToOUString(
        ::rtl::OString(reinterpret_cast<const sal_Char*>(value)),
        RTL_TEXTENCODING_UTF8) );
    if (lang) {
        const ::rtl::OUString langU( ::rtl::OStringToOUString(
            ::rtl::OString(reinterpret_cast<const sal_Char*>(lang)),
            RTL_TEXTENCODING_UTF8) );
        return uno::Reference<rdf::XNode>(
            rdf::Literal::createWithLanguage(m_xContext, valueU, langU),
            uno::UNO_QUERY);
    } else if (pType) {
        uno::Reference<rdf::XURI> xType(convertToXURI(pType));
        OSL_ENSURE(xType.is(), "convertToXNode: null uri");
        return uno::Reference<rdf::XNode>(
            rdf::Literal::createWithType(m_xContext, valueU, xType),
            uno::UNO_QUERY);
    } else {
        return uno::Reference<rdf::XNode>(
            rdf::Literal::create(m_xContext, valueU),
            uno::UNO_QUERY);
    }
}

rdf::Statement
librdf_TypeConverter::convertToStatement(librdf_statement* i_pStmt,
    librdf_node* i_pContext) const
{
    if (!i_pStmt) {
        throw uno::RuntimeException();
    }
    return rdf::Statement(
        convertToXResource(librdf_statement_get_subject(i_pStmt)),
        convertToXURI(librdf_statement_get_predicate(i_pStmt)),
        convertToXNode(librdf_statement_get_object(i_pStmt)),
        convertToXURI(i_pContext));
}

} // closing anonymous implementation namespace



// component helper namespace
namespace comp_librdf_Repository {

::rtl::OUString SAL_CALL _getImplementationName() {
    return rtl::OUString("librdf_Repository");
}

uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > s(1);
    s[0] = "com.sun.star.rdf.Repository";
    return s;
}

uno::Reference< uno::XInterface > SAL_CALL _create(
    const uno::Reference< uno::XComponentContext > & context)
        SAL_THROW((uno::Exception))
{
    return static_cast< ::cppu::OWeakObject * >(new librdf_Repository(context));
}

} // closing component helper namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
