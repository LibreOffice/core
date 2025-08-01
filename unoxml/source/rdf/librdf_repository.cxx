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

#include <string.h>

#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string_view>
#include <iterator>
#include <algorithm>
#include <atomic>

#include <optional>

#include <libxslt/security.h>

#include <redland.h>

#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/rdf/ParseException.hpp>
#include <com/sun/star/rdf/QueryException.hpp>
#include <com/sun/star/rdf/RepositoryException.hpp>
#include <com/sun/star/rdf/XDocumentRepository.hpp>
#include <com/sun/star/rdf/XLiteral.hpp>
#include <com/sun/star/rdf/FileFormat.hpp>
#include <com/sun/star/rdf/BlankNode.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/Literal.hpp>

#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/weakref.hxx>

#include <comphelper/sequence.hxx>
#include <comphelper/xmltools.hxx>

#include <com/sun/star/embed/XEncryptionProtectedSource2.hpp>
#include <utility>

/**
    Implementation of the service com.sun.star.rdf.Repository.

    This implementation uses the Redland RDF library (librdf).

    There are several classes involved:
    librdf_TypeConverter:   helper class to convert data types redland <-> uno
    librdf_Repository:      the main repository, does almost all the work
    librdf_NamedGraph:      the XNamedGraph, forwards everything to repository
    librdf_GraphResult:     an XEnumeration<Statement>
    librdf_QuerySelectResult:   an XEnumeration<sequence<XNode>>

 */

/// anonymous implementation namespace
namespace {

class librdf_NamedGraph;
class librdf_Repository;

using namespace ::com::sun::star;

typedef std::map< OUString, ::rtl::Reference<librdf_NamedGraph> >
    NamedGraphMap_t;

const char s_sparql [] = "sparql";
const char s_nsOOo  [] = "http://openoffice.org/2004/office/rdfa/";


//FIXME: this approach is not ideal. can we use blank nodes instead?
bool isInternalContext(librdf_node *i_pNode) noexcept
{
    OSL_ENSURE(i_pNode, "isInternalContext: context null");
    OSL_ENSURE(librdf_node_is_resource(i_pNode),
        "isInternalContext: context not resource");
    if (i_pNode) {
        librdf_uri *pURI(librdf_node_get_uri(i_pNode));
        OSL_ENSURE(pURI, "isInternalContext: URI null");
        if (pURI) {
            unsigned char *pContextURI(librdf_uri_as_string(pURI));
            assert(pContextURI && "isInternalContext: URI string null");
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


// n.b.: librdf destructor functions dereference null pointers!
//       so they need to be wrapped to be usable with std::shared_ptr.
void safe_librdf_free_world(librdf_world *const world)
{
    if (world) { librdf_free_world(world); }
}
void safe_librdf_free_model(librdf_model *const model)
{
    if (model) { librdf_free_model(model); }
}
void safe_librdf_free_node(librdf_node* node)
{
    if (node) { librdf_free_node(node); }
}
void safe_librdf_free_parser(librdf_parser *const parser)
{
    if (parser) { librdf_free_parser(parser); }
}
void safe_librdf_free_query(librdf_query *const query)
{
    if (query) { librdf_free_query(query); }
}
void
safe_librdf_free_query_results(librdf_query_results *const query_results)
{
    if (query_results) { librdf_free_query_results(query_results); }
}
void safe_librdf_free_serializer(librdf_serializer *const serializer)
{
    if (serializer) { librdf_free_serializer(serializer); }
}
void safe_librdf_free_statement(librdf_statement *const statement)
{
    if (statement) { librdf_free_statement(statement); }
}
void safe_librdf_free_storage(librdf_storage *const storage)
{
    if (storage) { librdf_free_storage(storage); }
}
void safe_librdf_free_stream(librdf_stream *const stream)
{
    if (stream) { librdf_free_stream(stream); }
}
void safe_librdf_free_uri(librdf_uri *const uri)
{
    if (uri) { librdf_free_uri(uri); }
}


/** converts between librdf types and UNO API types.
 */
class librdf_TypeConverter
{
public:

    // some wrapper classes to temporarily hold values of UNO XNodes
    struct Node
    {
        virtual ~Node() {}
    };
    struct Resource : public Node { };
    struct URI : public Resource
    {
        OString const value;
        explicit URI(OString i_Value)
            : value(std::move(i_Value))
        { }
    };
    struct BlankNode : public Resource
    {
        OString const value;
        explicit BlankNode(OString i_Value)
            : value(std::move(i_Value))
        { }
    };
    struct Literal : public Node
    {
        OString const value;
        OString const language;
        ::std::optional<OString> const type;
        Literal(OString  i_rValue, OString i_Language,
                ::std::optional<OString> i_Type)
            : value(std::move(i_rValue))
            , language(std::move(i_Language))
            , type(std::move(i_Type))
        { }
    };
    struct Statement
    {
        std::shared_ptr<Resource> const pSubject;
        std::shared_ptr<URI> const pPredicate;
        std::shared_ptr<Node> const pObject;
        Statement(std::shared_ptr<Resource> i_pSubject,
                  std::shared_ptr<URI> i_pPredicate,
                  std::shared_ptr<Node> i_pObject)
            : pSubject(std::move(i_pSubject))
            , pPredicate(std::move(i_pPredicate))
            , pObject(std::move(i_pObject))
        { }
    };

    librdf_TypeConverter(
            uno::Reference< uno::XComponentContext > i_xContext,
            librdf_Repository &i_rRep)
        : m_xContext(std::move(i_xContext))
        , m_rRep(i_rRep)
    { };

    librdf_world *createWorld_Lock() const;
    librdf_storage *createStorage_Lock(librdf_world *i_pWorld) const;
    librdf_model *createModel_Lock(librdf_world *i_pWorld,
        librdf_storage * i_pStorage) const;
    static librdf_uri* mkURI_Lock(librdf_world* i_pWorld,
        const OString & i_rURI);
    static librdf_node* mkResource_Lock(librdf_world* i_pWorld,
        const Resource * i_pResource);
    static librdf_node* mkNode_Lock(librdf_world* i_pWorld,
        const Node * i_pNode);
    static librdf_statement* mkStatement_Lock(librdf_world* i_pWorld,
        Statement const& i_rStatement);
    static std::shared_ptr<Resource> extractResource_NoLock(
        const uno::Reference< rdf::XResource > & i_xResource);
    static void extractResourceToCacheKey_NoLock(
        const uno::Reference< rdf::XResource > & i_xResource,
        OUStringBuffer& rBuf);
    static std::shared_ptr<Node> extractNode_NoLock(
        const uno::Reference< rdf::XNode > & i_xNode);
    static void extractNodeToCacheKey_NoLock(
        const uno::Reference< rdf::XNode > & i_xNode,
        OUStringBuffer& rBuffer);
    static Statement extractStatement_NoLock(
        const uno::Reference< rdf::XResource > & i_xSubject,
        const uno::Reference< rdf::XURI > & i_xPredicate,
        const uno::Reference< rdf::XNode > & i_xObject);
    uno::Reference<rdf::XURI> convertToXURI(librdf_uri* i_pURI) const;
    uno::Reference<rdf::XURI> convertToXURI(librdf_node* i_pURI) const;
    uno::Reference<rdf::XResource>
        convertToXResource(librdf_node* i_pNode) const;
    uno::Reference<rdf::XNode> convertToXNode(librdf_node* i_pNode) const;
    rdf::Statement
        convertToStatement(librdf_statement* i_pStmt, librdf_node* i_pContext)
        const;

private:
    uno::Reference< uno::XComponentContext > const m_xContext;
    librdf_Repository & m_rRep;
};


/** implements the repository service.
 */
class librdf_Repository:
//    private ::cppu::BaseMutex,
    public ::cppu::WeakImplHelper<
        lang::XServiceInfo,
        rdf::XDocumentRepository,
        lang::XInitialization>
{
public:

    explicit librdf_Repository(
        uno::Reference< uno::XComponentContext > const & i_xContext);
    virtual ~librdf_Repository() override;

    // css::lang::XServiceInfo:
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(
            const OUString & ServiceName) override;
    virtual uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames() override;

    // css::rdf::XRepository:
    virtual uno::Reference< rdf::XBlankNode > SAL_CALL createBlankNode() override;
    virtual uno::Reference<rdf::XNamedGraph> SAL_CALL importGraph(
            ::sal_Int16 i_Format,
            const uno::Reference< io::XInputStream > & i_xInStream,
            const uno::Reference< rdf::XURI > & i_xGraphName,
            const uno::Reference< rdf::XURI > & i_xBaseURI) override;
    virtual void SAL_CALL exportGraph(::sal_Int16 i_Format,
            const uno::Reference< io::XOutputStream > & i_xOutStream,
            const uno::Reference< rdf::XURI > & i_xGraphName,
            const uno::Reference< rdf::XURI > & i_xBaseURI) override;
    virtual uno::Sequence< uno::Reference< rdf::XURI > > SAL_CALL
        getGraphNames() override;
    virtual uno::Reference< rdf::XNamedGraph > SAL_CALL getGraph(
            const uno::Reference< rdf::XURI > & i_xGraphName) override;
    virtual uno::Reference< rdf::XNamedGraph > SAL_CALL createGraph(
            const uno::Reference< rdf::XURI > & i_xGraphName) override;
    virtual void SAL_CALL destroyGraph(
            const uno::Reference< rdf::XURI > & i_xGraphName) override;
    virtual uno::Reference< container::XEnumeration > SAL_CALL getStatements(
            const uno::Reference< rdf::XResource > & i_xSubject,
            const uno::Reference< rdf::XURI > & i_xPredicate,
            const uno::Reference< rdf::XNode > & i_xObject) override;
    virtual uno::Reference< rdf::XQuerySelectResult > SAL_CALL
            querySelect(const OUString & i_rQuery) override;
    virtual uno::Reference< container::XEnumeration > SAL_CALL
        queryConstruct(const OUString & i_rQuery) override;
    virtual sal_Bool SAL_CALL queryAsk(const OUString & i_rQuery) override;

    // css::rdf::XDocumentRepository:
    virtual void SAL_CALL setStatementRDFa(
            const uno::Reference< rdf::XResource > & i_xSubject,
            const uno::Sequence< uno::Reference< rdf::XURI > > & i_rPredicates,
            const uno::Reference< rdf::XMetadatable > & i_xObject,
            const OUString & i_rRDFaContent,
            const uno::Reference< rdf::XURI > & i_xRDFaDatatype) override;
    virtual void SAL_CALL removeStatementRDFa(
            const uno::Reference< rdf::XMetadatable > & i_xElement) override;
    virtual beans::Pair< uno::Sequence<rdf::Statement>, sal_Bool > SAL_CALL
        getStatementRDFa(uno::Reference< rdf::XMetadatable > const& i_xElement) override;
    virtual uno::Reference< container::XEnumeration > SAL_CALL
        getStatementsRDFa(
            const uno::Reference< rdf::XResource > & i_xSubject,
            const uno::Reference< rdf::XURI > & i_xPredicate,
            const uno::Reference< rdf::XNode > & i_xObject) override;

    // css::lang::XInitialization:
    virtual void SAL_CALL initialize(
            const uno::Sequence< css::uno::Any > & i_rArguments) override;

    // XNamedGraph forwards ---------------------------------------------
    NamedGraphMap_t::iterator clearGraph_NoLock(
            const OUString & i_rGraphName,
            bool i_Internal = false );
    NamedGraphMap_t::iterator clearGraph_Lock(
            const OUString & i_rGraphName,
            bool i_Internal);
    void addStatementGraph_NoLock(
            const uno::Reference< rdf::XResource > & i_xSubject,
            const uno::Reference< rdf::XURI > & i_xPredicate,
            const uno::Reference< rdf::XNode > & i_xObject,
            const uno::Reference< rdf::XURI > & i_xName );
//        throw (uno::RuntimeException, lang::IllegalArgumentException,
//            container::NoSuchElementException, rdf::RepositoryException);
    void addStatementGraph_Lock(
        librdf_TypeConverter::Statement const& i_rStatement,
        OUString const& i_rGraphName,
        bool i_Internal);
    void removeStatementsGraph_NoLock(
            const uno::Reference< rdf::XResource > & i_xSubject,
            const uno::Reference< rdf::XURI > & i_xPredicate,
            const uno::Reference< rdf::XNode > & i_xObject,
            const uno::Reference< rdf::XURI > & i_xName );
//        throw (uno::RuntimeException, lang::IllegalArgumentException,
//            container::NoSuchElementException, rdf::RepositoryException);
    std::vector<rdf::Statement> getStatementsGraph_NoLock(
            const uno::Reference< rdf::XResource > & i_xSubject,
            const uno::Reference< rdf::XURI > & i_xPredicate,
            const uno::Reference< rdf::XNode > & i_xObject,
            const uno::Reference< rdf::XURI > & i_xName,
            bool i_Internal = false );
//        throw (uno::RuntimeException, lang::IllegalArgumentException,
//            container::NoSuchElementException, rdf::RepositoryException);

    const librdf_TypeConverter& getTypeConverter() const { return m_TypeConverter; };

private:

    librdf_Repository(librdf_Repository const&) = delete;
    librdf_Repository& operator=(librdf_Repository const&) = delete;

    /// this is const, no need to lock m_aMutex to access it
    uno::Reference< uno::XComponentContext > const m_xContext;

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
    static std::shared_ptr<librdf_world> m_pWorld;
    /// refcount
    static sal_uInt32 m_NumInstances;
    /// mutex for m_pWorld - redland is not as threadsafe as is often claimed
    static std::mutex m_aMutex;

    // NB: sequence of the shared pointers is important!
    /// librdf repository storage
    std::shared_ptr<librdf_storage> m_pStorage;
    /// librdf repository model
    std::shared_ptr<librdf_model> m_pModel;

    /// all named graphs
    NamedGraphMap_t m_NamedGraphs;

    /// type conversion helper - stateless
    librdf_TypeConverter m_TypeConverter;

    /// set of xml:ids of elements with xhtml:content
    ::std::set< OUString > m_RDFaXHTMLContentSet;
};


/** result of operations that return a graph, i.e.,
    an XEnumeration of statements.
 */
class librdf_GraphResult:
    public ::cppu::WeakImplHelper<
        container::XEnumeration>
{
public:

    librdf_GraphResult(librdf_Repository *i_pRepository,
            std::mutex & i_rMutex,
            std::shared_ptr<librdf_stream> i_pStream,
            std::shared_ptr<librdf_node> i_pContext,
            std::shared_ptr<librdf_query> i_pQuery =
                std::shared_ptr<librdf_query>() )
        : m_xRep(i_pRepository)
        , m_rMutex(i_rMutex)
        , m_pQuery(std::move(i_pQuery))
        , m_pContext(std::move(i_pContext))
        , m_pStream(std::move(i_pStream))
    { };

    virtual ~librdf_GraphResult() override
    {
        std::scoped_lock g(m_rMutex); // lock mutex when destroying members
        const_cast<std::shared_ptr<librdf_stream>& >(m_pStream).reset();
        const_cast<std::shared_ptr<librdf_node>& >(m_pContext).reset();
        const_cast<std::shared_ptr<librdf_query>& >(m_pQuery).reset();
    }

    // css::container::XEnumeration:
    virtual sal_Bool SAL_CALL hasMoreElements() override;
    virtual uno::Any SAL_CALL nextElement() override;

private:

    librdf_GraphResult(librdf_GraphResult const&) = delete;
    librdf_GraphResult& operator=(librdf_GraphResult const&) = delete;

    // NB: this is not a weak pointer: streams _must_ be deleted before the
    //     storage they point into, so we keep the repository alive here
    // also, sequence is important: the stream must be destroyed first.
    ::rtl::Reference< librdf_Repository > m_xRep;
    // needed for synchronizing access to librdf (it doesn't do win32 threading)
    std::mutex & m_rMutex;
    // the query (in case this is a result of a graph query)
    // not that the redland documentation spells this out explicitly, but
    // queries must be freed only after all the results are completely read
    std::shared_ptr<librdf_query>  const m_pQuery;
    std::shared_ptr<librdf_node>   const m_pContext;
    std::shared_ptr<librdf_stream> const m_pStream;

    librdf_node* getContext_Lock() const;
};


// css::container::XEnumeration:
sal_Bool SAL_CALL
librdf_GraphResult::hasMoreElements()
{
    std::scoped_lock g(m_rMutex);
    return m_pStream && !librdf_stream_end(m_pStream.get());
}

librdf_node* librdf_GraphResult::getContext_Lock() const
{
    if (!m_pStream || librdf_stream_end(m_pStream.get()))
        return nullptr;
    librdf_node *pCtxt(
#if LIBRDF_VERSION >= 10012
        librdf_stream_get_context2(m_pStream.get()) );
#else
        static_cast<librdf_node *>(librdf_stream_get_context(m_pStream.get())) );
#endif
    if (pCtxt)
        return pCtxt;
    return m_pContext.get();
}

css::uno::Any SAL_CALL
librdf_GraphResult::nextElement()
{
    std::scoped_lock g(m_rMutex);
    if (m_pStream && librdf_stream_end(m_pStream.get())) {
        throw container::NoSuchElementException();
    }
    librdf_node * pCtxt = getContext_Lock();

    librdf_statement *pStmt( librdf_stream_get_object(m_pStream.get()) );
    if (!pStmt) {
        rdf::QueryException e(
            u"librdf_GraphResult::nextElement: "
            "librdf_stream_get_object failed"_ustr, *this);
        throw lang::WrappedTargetException(
            u"librdf_GraphResult::nextElement: "
            "librdf_stream_get_object failed"_ustr, *this,
                uno::Any(e));
    }
    // NB: pCtxt may be null here if this is result of a graph query
    if (pCtxt && isInternalContext(pCtxt)) {
        pCtxt = nullptr; // XML ID context is implementation detail!
    }
    rdf::Statement Stmt(
        m_xRep->getTypeConverter().convertToStatement(pStmt, pCtxt) );
    // NB: this will invalidate current item.
    librdf_stream_next(m_pStream.get());
    return uno::Any(Stmt);
}


/** result of operations that return a graph, i.e.,
    an XEnumeration of statements.
 */
class librdf_GraphResult2:
    public ::cppu::WeakImplHelper<
        container::XEnumeration>
{
public:

    librdf_GraphResult2(std::vector<rdf::Statement> statements)
        : m_vStatements(std::move(statements))
    { };

    // css::container::XEnumeration:
    virtual sal_Bool SAL_CALL hasMoreElements() override;
    virtual uno::Any SAL_CALL nextElement() override;

private:

    std::vector<rdf::Statement> m_vStatements;
    std::atomic<std::size_t> m_nIndex = 0;
};


// css::container::XEnumeration:
sal_Bool SAL_CALL
librdf_GraphResult2::hasMoreElements()
{
    return m_nIndex < m_vStatements.size();
}

css::uno::Any SAL_CALL
librdf_GraphResult2::nextElement()
{
    std::size_t const n = m_nIndex++;
    if (m_vStatements.size() <= n)
    {
        m_nIndex = m_vStatements.size(); // avoid overflow
        throw container::NoSuchElementException();
    }
    return uno::Any(m_vStatements[n]);
}

/** result of tuple queries ("SELECT").
 */
class librdf_QuerySelectResult:
    public ::cppu::WeakImplHelper<
        rdf::XQuerySelectResult>
{
public:

    librdf_QuerySelectResult(librdf_Repository *i_pRepository,
            std::mutex & i_rMutex,
            std::shared_ptr<librdf_query> i_pQuery,
            std::shared_ptr<librdf_query_results> i_pQueryResult,
            uno::Sequence< OUString > const& i_rBindingNames )
        : m_xRep(i_pRepository)
        , m_rMutex(i_rMutex)
        , m_pQuery(std::move(i_pQuery))
        , m_pQueryResult(std::move(i_pQueryResult))
        , m_BindingNames(i_rBindingNames)
    { };

    virtual ~librdf_QuerySelectResult() override
    {
        std::scoped_lock g(m_rMutex); // lock mutex when destroying members
        const_cast<std::shared_ptr<librdf_query_results>& >(m_pQueryResult)
            .reset();
        const_cast<std::shared_ptr<librdf_query>& >(m_pQuery).reset();
    }

    // css::container::XEnumeration:
    virtual sal_Bool SAL_CALL hasMoreElements() override;
    virtual uno::Any SAL_CALL nextElement() override;

    // css::rdf::XQuerySelectResult:
    virtual uno::Sequence< OUString > SAL_CALL getBindingNames() override;

private:

    librdf_QuerySelectResult(librdf_QuerySelectResult const&) = delete;
    librdf_QuerySelectResult& operator=(librdf_QuerySelectResult const&) = delete;

    // NB: this is not a weak pointer: streams _must_ be deleted before the
    //     storage they point into, so we keep the repository alive here
    // also, sequence is important: the stream must be destroyed first.
    ::rtl::Reference< librdf_Repository > m_xRep;
    // needed for synchronizing access to librdf (it doesn't do win32 threading)
    std::mutex & m_rMutex;
    // not that the redland documentation spells this out explicitly, but
    // queries must be freed only after all the results are completely read
    std::shared_ptr<librdf_query> const m_pQuery;
    std::shared_ptr<librdf_query_results> const m_pQueryResult;
    uno::Sequence< OUString > const m_BindingNames;
};


// css::container::XEnumeration:
sal_Bool SAL_CALL
librdf_QuerySelectResult::hasMoreElements()
{
    std::scoped_lock g(m_rMutex);
    return !librdf_query_results_finished(m_pQueryResult.get());
}

class NodeArray : private std::vector<librdf_node*>
{
public:
    NodeArray(int cnt) : std::vector<librdf_node*>(cnt) {}

    ~NodeArray() noexcept
    {
        std::for_each(begin(), end(), safe_librdf_free_node);
    }

    using std::vector<librdf_node*>::data;
    using std::vector<librdf_node*>::operator[];
};

css::uno::Any SAL_CALL
librdf_QuerySelectResult::nextElement()
{
    std::scoped_lock g(m_rMutex);
    if (librdf_query_results_finished(m_pQueryResult.get())) {
        throw container::NoSuchElementException();
    }
    sal_Int32 count(m_BindingNames.getLength());
    OSL_ENSURE(count >= 0, "negative length?");
    NodeArray aNodes(count);
    if (librdf_query_results_get_bindings(m_pQueryResult.get(), nullptr,
                aNodes.data()))
    {
        rdf::QueryException e(
            u"librdf_QuerySelectResult::nextElement: "
            "librdf_query_results_get_bindings failed"_ustr, *this);
        throw lang::WrappedTargetException(
            u"librdf_QuerySelectResult::nextElement: "
            "librdf_query_results_get_bindings failed"_ustr, *this,
            uno::Any(e));
    }
    uno::Sequence< uno::Reference< rdf::XNode > > ret(count);
    auto retRange = asNonConstRange(ret);
    for (int i = 0; i < count; ++i) {
        retRange[i] = m_xRep->getTypeConverter().convertToXNode(aNodes[i]);
    }
    // NB: this will invalidate current item.
    librdf_query_results_next(m_pQueryResult.get());
    return uno::Any(ret);
}

// css::rdf::XQuerySelectResult:
uno::Sequence< OUString > SAL_CALL
librdf_QuerySelectResult::getBindingNames()
{
    // const - no lock needed
    return m_BindingNames;
}


/** represents a named graph, and forwards all the work to repository.
 */
class librdf_NamedGraph:
    public ::cppu::WeakImplHelper<
        rdf::XNamedGraph>
{
public:
    librdf_NamedGraph(librdf_Repository * i_pRep,
            uno::Reference<rdf::XURI> i_xName)
        : m_wRep(i_pRep)
        , m_xName(std::move(i_xName))
    { };

    // css::rdf::XNode:
    virtual OUString SAL_CALL getStringValue() override;

    // css::rdf::XURI:
    virtual OUString SAL_CALL getNamespace() override;
    virtual OUString SAL_CALL getLocalName() override;

    // css::rdf::XNamedGraph:
    virtual uno::Reference<rdf::XURI> SAL_CALL getName() override;
    virtual void SAL_CALL clear() override;
    virtual void SAL_CALL addStatement(
            const uno::Reference< rdf::XResource > & i_xSubject,
            const uno::Reference< rdf::XURI > & i_xPredicate,
            const uno::Reference< rdf::XNode > & i_xObject) override;
    virtual void SAL_CALL removeStatements(
            const uno::Reference< rdf::XResource > & i_xSubject,
            const uno::Reference< rdf::XURI > & i_xPredicate,
            const uno::Reference< rdf::XNode > & i_xObject) override;
    virtual uno::Reference< container::XEnumeration > SAL_CALL getStatements(
            const uno::Reference< rdf::XResource > & i_xSubject,
            const uno::Reference< rdf::XURI > & i_xPredicate,
            const uno::Reference< rdf::XNode > & i_xObject) override;

private:

    librdf_NamedGraph(librdf_NamedGraph const&) = delete;
    librdf_NamedGraph& operator=(librdf_NamedGraph const&) = delete;

    static OUString createCacheKey_NoLock(
        const uno::Reference< rdf::XResource > & i_xSubject,
        const uno::Reference< rdf::XURI > & i_xPredicate,
        const uno::Reference< rdf::XNode > & i_xObject);

    /// weak reference: this is needed to check if m_pRep is valid
    unotools::WeakReference< librdf_Repository > const m_wRep;
    uno::Reference< rdf::XURI > const m_xName;

    /// Querying is rather slow, so cache the results.
    std::map<OUString, std::vector<rdf::Statement>> m_aStatementsCache;
    std::mutex m_CacheMutex;
};


// css::rdf::XNode:
OUString SAL_CALL librdf_NamedGraph::getStringValue()
{
    return m_xName->getStringValue();
}

// css::rdf::XURI:
OUString SAL_CALL librdf_NamedGraph::getNamespace()
{
    return m_xName->getNamespace();
}

OUString SAL_CALL librdf_NamedGraph::getLocalName()
{
    return m_xName->getLocalName();
}

// css::rdf::XNamedGraph:
uno::Reference< rdf::XURI > SAL_CALL librdf_NamedGraph::getName()
{
    return m_xName;
}

void SAL_CALL librdf_NamedGraph::clear()
{
    rtl::Reference< librdf_Repository > xRep( m_wRep );
    if (!xRep.is()) {
        throw rdf::RepositoryException(
            u"librdf_NamedGraph::clear: repository is gone"_ustr, *this);
    }
    const OUString contextU( m_xName->getStringValue() );
    try {
        xRep->clearGraph_NoLock(contextU);
    } catch (lang::IllegalArgumentException & ex) {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException( ex.Message,
                        *this, anyEx );
    }
    std::unique_lock g(m_CacheMutex);
    m_aStatementsCache.clear();
}

void SAL_CALL librdf_NamedGraph::addStatement(
    const uno::Reference< rdf::XResource > & i_xSubject,
    const uno::Reference< rdf::XURI > & i_xPredicate,
    const uno::Reference< rdf::XNode > & i_xObject)
{
    rtl::Reference< librdf_Repository > xRep( m_wRep );
    if (!xRep.is()) {
        throw rdf::RepositoryException(
            u"librdf_NamedGraph::addStatement: repository is gone"_ustr, *this);
    }
    {
        std::unique_lock g(m_CacheMutex);
        m_aStatementsCache.clear();
    }
    xRep->addStatementGraph_NoLock(
            i_xSubject, i_xPredicate, i_xObject, m_xName);
}

void SAL_CALL librdf_NamedGraph::removeStatements(
    const uno::Reference< rdf::XResource > & i_xSubject,
    const uno::Reference< rdf::XURI > & i_xPredicate,
    const uno::Reference< rdf::XNode > & i_xObject)
{
    rtl::Reference< librdf_Repository > xRep( m_wRep );
    if (!xRep.is()) {
        throw rdf::RepositoryException(
            u"librdf_NamedGraph::removeStatements: repository is gone"_ustr, *this);
    }
    {
        std::unique_lock g(m_CacheMutex);
        m_aStatementsCache.clear();
    }
    xRep->removeStatementsGraph_NoLock(
            i_xSubject, i_xPredicate, i_xObject, m_xName);
}

OUString librdf_NamedGraph::createCacheKey_NoLock(
    const uno::Reference< rdf::XResource > & i_xSubject,
    const uno::Reference< rdf::XURI > & i_xPredicate,
    const uno::Reference< rdf::XNode > & i_xObject)
{
    OUStringBuffer cacheKey(256);
    librdf_TypeConverter::extractResourceToCacheKey_NoLock(i_xSubject, cacheKey);
    cacheKey.append("\t");
    librdf_TypeConverter::extractResourceToCacheKey_NoLock(i_xPredicate, cacheKey);
    cacheKey.append("\t");
    librdf_TypeConverter::extractNodeToCacheKey_NoLock(i_xObject, cacheKey);
    return cacheKey.makeStringAndClear();
}

uno::Reference< container::XEnumeration > SAL_CALL
librdf_NamedGraph::getStatements(
    const uno::Reference< rdf::XResource > & i_xSubject,
    const uno::Reference< rdf::XURI > & i_xPredicate,
    const uno::Reference< rdf::XNode > & i_xObject)
{
    OUString cacheKey = createCacheKey_NoLock(i_xSubject, i_xPredicate, i_xObject);
    {
        std::unique_lock g(m_CacheMutex);
        auto it = m_aStatementsCache.find(cacheKey);
        if (it != m_aStatementsCache.end()) {
            return new librdf_GraphResult2(it->second);
        }
    }

    rtl::Reference< librdf_Repository > xRep( m_wRep );
    if (!xRep.is()) {
        throw rdf::RepositoryException(
            u"librdf_NamedGraph::getStatements: repository is gone"_ustr, *this);
    }
    std::vector<rdf::Statement> vStatements = xRep->getStatementsGraph_NoLock(
            i_xSubject, i_xPredicate, i_xObject, m_xName);

    {
        std::unique_lock g(m_CacheMutex);
        m_aStatementsCache.emplace(cacheKey, vStatements);
    }
    return new librdf_GraphResult2(std::move(vStatements));
}


std::shared_ptr<librdf_world> librdf_Repository::m_pWorld;
sal_uInt32 librdf_Repository::m_NumInstances = 0;
std::mutex librdf_Repository::m_aMutex;

librdf_Repository::librdf_Repository(
        uno::Reference< uno::XComponentContext > const & i_xContext)
    : /*BaseMutex(),*/ m_xContext(i_xContext)
//    m_pWorld  (static_cast<librdf_world  *>(0), safe_librdf_free_world  ),
    , m_pStorage(static_cast<librdf_storage*>(nullptr), safe_librdf_free_storage)
    , m_pModel  (static_cast<librdf_model  *>(nullptr), safe_librdf_free_model  )
    , m_TypeConverter(i_xContext, *this)
{
    OSL_ENSURE(i_xContext.is(), "librdf_Repository: null context");

    std::scoped_lock g(m_aMutex);
    if (!m_NumInstances++) {
        m_pWorld.reset(m_TypeConverter.createWorld_Lock(),
                safe_librdf_free_world);
    }
}

librdf_Repository::~librdf_Repository()
{
    std::scoped_lock g(m_aMutex);

    // must destroy these before world!
    m_pModel.reset();
    m_pStorage.reset();

    // FIXME: so it turns out that calling librdf_free_world will
    //   (via raptor_sax2_finish) call xmlCleanupParser, which will
    //   free libxml2's globals! ARRRGH!!! => never call librdf_free_world
#if 0
    if (!--m_NumInstances) {
        m_pWorld.reset();
    }
#endif
}

// com.sun.star.uno.XServiceInfo:
OUString SAL_CALL librdf_Repository::getImplementationName()
{
    return u"librdf_Repository"_ustr;
}

sal_Bool SAL_CALL librdf_Repository::supportsService(
    OUString const & serviceName)
{
    return cppu::supportsService(this, serviceName);
}

uno::Sequence< OUString > SAL_CALL
librdf_Repository::getSupportedServiceNames()
{
    return { u"com.sun.star.rdf.Repository"_ustr };
}

// css::rdf::XRepository:
uno::Reference< rdf::XBlankNode > SAL_CALL librdf_Repository::createBlankNode()
{
    std::scoped_lock g(m_aMutex);
    const std::shared_ptr<librdf_node> pNode(
        librdf_new_node_from_blank_identifier(m_pWorld.get(), nullptr),
        safe_librdf_free_node);
    if (!pNode) {
        throw uno::RuntimeException(
            u"librdf_Repository::createBlankNode: "
            "librdf_new_node_from_blank_identifier failed"_ustr, *this);
    }
    const unsigned char * id (librdf_node_get_blank_identifier(pNode.get()));
    if (!id) {
        throw uno::RuntimeException(
            u"librdf_Repository::createBlankNode: "
            "librdf_node_get_blank_identifier failed"_ustr, *this);
    }
    const OUString nodeID(OUString::createFromAscii(
        reinterpret_cast<const char *>(id)));
    try {
        return rdf::BlankNode::create(m_xContext, nodeID);
    } catch (const lang::IllegalArgumentException &) {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException(
                u"librdf_Repository::createBlankNode: "
                "illegal blank node label"_ustr, *this, anyEx);
    }
}

//void SAL_CALL
uno::Reference<rdf::XNamedGraph> SAL_CALL
librdf_Repository::importGraph(::sal_Int16 i_Format,
    const uno::Reference< io::XInputStream > & i_xInStream,
    const uno::Reference< rdf::XURI > & i_xGraphName,
    const uno::Reference< rdf::XURI > & i_xBaseURI)
{
    if (!i_xInStream.is()) {
        throw lang::IllegalArgumentException(
            u"librdf_Repository::importGraph: stream is null"_ustr, *this, 1);
    }
    //FIXME: other formats
    if (i_Format != rdf::FileFormat::RDF_XML) {
        throw datatransfer::UnsupportedFlavorException(
                u"librdf_Repository::importGraph: file format not supported"_ustr, *this);
    }
    if (!i_xGraphName.is()) {
        throw lang::IllegalArgumentException(
                u"librdf_Repository::importGraph: graph name is null"_ustr, *this, 2);
    }
    if (i_xGraphName->getStringValue().startsWith(s_nsOOo))
    {
        throw lang::IllegalArgumentException(
                u"librdf_Repository::importGraph: URI is reserved"_ustr, *this, 0);
    }
    if (!i_xBaseURI.is()) { //FIXME: any i_Format that don't need a base URI?
        throw lang::IllegalArgumentException(
                u"librdf_Repository::importGraph: base URI is null"_ustr, *this, 3);
    }
    OSL_ENSURE(i_xBaseURI.is(), "no base uri");
    const OUString baseURIU( i_xBaseURI->getStringValue() );
    if (baseURIU.indexOf('#') >= 0) {
        throw lang::IllegalArgumentException(
                u"librdf_Repository::importGraph: base URI is not absolute"_ustr, *this, 3);
    }

    const OUString contextU( i_xGraphName->getStringValue() );

    uno::Sequence<sal_Int8> buf;
    uno::Reference<io::XSeekable> xSeekable(i_xInStream, uno::UNO_QUERY);
    // UGLY: if only redland could read streams...
    const sal_Int64 sz( xSeekable.is() ? xSeekable->getLength() : 1 << 20 );
    // exceptions are propagated
    i_xInStream->readBytes( buf, static_cast<sal_Int32>( sz ) );

    if (buf.getLength() == 0) {
        throw rdf::ParseException(
            u"librdf_Repository::importGraph: stream is empty"_ustr, *this);
    }

    std::scoped_lock g(m_aMutex); // don't call i_x* with mutex locked

    if (m_NamedGraphs.find(contextU) != m_NamedGraphs.end()) {
        throw container::ElementExistException(
                u"librdf_Repository::importGraph: graph with given URI exists"_ustr, *this);
    }
    const OString context(
        OUStringToOString(contextU, RTL_TEXTENCODING_UTF8) );

    const std::shared_ptr<librdf_node> pContext(
        librdf_new_node_from_uri_string(m_pWorld.get(),
            reinterpret_cast<const unsigned char*> (context.getStr())),
        safe_librdf_free_node);
    if (!pContext) {
        throw uno::RuntimeException(
            u"librdf_Repository::importGraph: librdf_new_node_from_uri_string failed"_ustr, *this);
    }

    const OString baseURI(
        OUStringToOString(baseURIU, RTL_TEXTENCODING_UTF8) );
    const std::shared_ptr<librdf_uri> pBaseURI(
        librdf_new_uri(m_pWorld.get(),
            reinterpret_cast<const unsigned char*> (baseURI.getStr())),
        safe_librdf_free_uri);
    if (!pBaseURI) {
        throw uno::RuntimeException( u"librdf_Repository::importGraph: librdf_new_uri failed"_ustr, *this);
    }

    const std::shared_ptr<librdf_parser> pParser(
        librdf_new_parser(m_pWorld.get(), "rdfxml", nullptr, nullptr),
        safe_librdf_free_parser);
    if (!pParser) {
        throw uno::RuntimeException(
                u"librdf_Repository::importGraph: "
                "librdf_new_parser failed"_ustr, *this);
    }

    const std::shared_ptr<librdf_stream> pStream(
        librdf_parser_parse_counted_string_as_stream(pParser.get(),
            reinterpret_cast<const unsigned char*>(buf.getConstArray()),
            buf.getLength(), pBaseURI.get()),
        safe_librdf_free_stream);
    if (!pStream) {
        throw rdf::ParseException(
            u"librdf_Repository::importGraph: "
            "librdf_parser_parse_counted_string_as_stream failed"_ustr, *this);
    }
    rtl::Reference<librdf_NamedGraph> const pGraph(
        new librdf_NamedGraph(this, i_xGraphName));
    m_NamedGraphs.insert(std::make_pair(contextU, pGraph));
    if (librdf_model_context_add_statements(m_pModel.get(),
            pContext.get(), pStream.get())) {
        throw rdf::RepositoryException(
            u"librdf_Repository::importGraph: "
            "librdf_model_context_add_statements failed"_ustr, *this);
    }

    return pGraph;
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
            reinterpret_cast<unsigned char*>(strchr(reinterpret_cast<char*>(pBuffer), '\n'));
        if (postcomment != nullptr)
        {
            ++postcomment;

            size_t preamblelen = postcomment - pBuffer;

            uno::Sequence<sal_Int8> buf(
                reinterpret_cast<sal_Int8*>(pBuffer), preamblelen);
            rStream->writeBytes(buf);

            OString aComment =
                "<!--" +
                comphelper::xml::makeXMLChaff() +
                "-->";

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
{
    if (!i_xOutStream.is()) {
        throw lang::IllegalArgumentException(
                u"librdf_Repository::exportGraph: stream is null"_ustr, *this, 1);
    }
    // FIXME: other formats
    if (i_Format != rdf::FileFormat::RDF_XML) {
        throw datatransfer::UnsupportedFlavorException(
                u"librdf_Repository::exportGraph: "
                "file format not supported"_ustr, *this);
    }
    if (!i_xGraphName.is()) {
        throw lang::IllegalArgumentException(
                u"librdf_Repository::exportGraph: "
                "graph name is null"_ustr, *this, 2);
    }
    if (!i_xBaseURI.is()) { //FIXME: any i_Format that don't need a base URI?
        throw lang::IllegalArgumentException(
                u"librdf_Repository::exportGraph: "
                "base URI is null"_ustr, *this, 3);
    }
    OSL_ENSURE(i_xBaseURI.is(), "no base uri");
    const OUString baseURIU( i_xBaseURI->getStringValue() );
    if (baseURIU.indexOf('#') >= 0) {
        throw lang::IllegalArgumentException(
                u"librdf_Repository::exportGraph: "
                "base URI is not absolute"_ustr, *this, 3);
    }

    const OUString contextU( i_xGraphName->getStringValue() );

    std::unique_lock g(m_aMutex); // don't call i_x* with mutex locked

    if (m_NamedGraphs.find(contextU) == m_NamedGraphs.end()) {
        throw container::NoSuchElementException(
                u"librdf_Repository::exportGraph: "
                "no graph with given URI exists"_ustr, *this);
    }
    const OString context(
        OUStringToOString(contextU, RTL_TEXTENCODING_UTF8) );

    const std::shared_ptr<librdf_node> pContext(
        librdf_new_node_from_uri_string(m_pWorld.get(),
            reinterpret_cast<const unsigned char*> (context.getStr())),
        safe_librdf_free_node);
    if (!pContext) {
        throw uno::RuntimeException(
            u"librdf_Repository::exportGraph: "
            "librdf_new_node_from_uri_string failed"_ustr, *this);
    }
    const OString baseURI(
        OUStringToOString(baseURIU, RTL_TEXTENCODING_UTF8) );
    const std::shared_ptr<librdf_uri> pBaseURI(
        librdf_new_uri(m_pWorld.get(),
            reinterpret_cast<const unsigned char*> (baseURI.getStr())),
        safe_librdf_free_uri);
    if (!pBaseURI) {
        throw uno::RuntimeException(
            u"librdf_Repository::exportGraph: "
            "librdf_new_uri failed"_ustr, *this);
    }

    const std::shared_ptr<librdf_stream> pStream(
        librdf_model_context_as_stream(m_pModel.get(), pContext.get()),
        safe_librdf_free_stream);
    if (!pStream) {
        throw rdf::RepositoryException(
            u"librdf_Repository::exportGraph: "
            "librdf_model_context_as_stream failed"_ustr, *this);
    }
    const char * const format("rdfxml");
    // #i116443#: abbrev breaks when certain URIs are used as data types
//    const char *format("rdfxml-abbrev");
    const std::shared_ptr<librdf_serializer> pSerializer(
        librdf_new_serializer(m_pWorld.get(), format, nullptr, nullptr),
        safe_librdf_free_serializer);
    if (!pSerializer) {
        throw uno::RuntimeException(
            u"librdf_Repository::exportGraph: "
            "librdf_new_serializer failed"_ustr, *this);
    }

    const std::shared_ptr<librdf_uri> pRelativeURI(
        librdf_new_uri(m_pWorld.get(), reinterpret_cast<const unsigned char*>
                ("http://feature.librdf.org/raptor-relativeURIs")),
                 safe_librdf_free_uri);
    const std::shared_ptr<librdf_uri> pWriteBaseURI(
        librdf_new_uri(m_pWorld.get(), reinterpret_cast<const unsigned char*>
            ("http://feature.librdf.org/raptor-writeBaseURI")),
             safe_librdf_free_uri);
    const std::shared_ptr<librdf_node> p0(
        librdf_new_node_from_literal(m_pWorld.get(),
            reinterpret_cast<const unsigned char*> ("0"), nullptr, 0),
        safe_librdf_free_node);
    const std::shared_ptr<librdf_node> p1(
        librdf_new_node_from_literal(m_pWorld.get(),
            reinterpret_cast<const unsigned char*> ("1"), nullptr, 0),
        safe_librdf_free_node);
    if (!pWriteBaseURI || !pRelativeURI || !p0 || !p1) {
        throw uno::RuntimeException(
            u"librdf_Repository::exportGraph: "
            "librdf_new_uri or librdf_new_node_from_literal failed"_ustr, *this);
    }

    // make URIs relative to base URI
    if (librdf_serializer_set_feature(pSerializer.get(),
        pRelativeURI.get(), p1.get()))
    {
        throw uno::RuntimeException(
            u"librdf_Repository::exportGraph: "
            "librdf_serializer_set_feature relativeURIs failed"_ustr, *this);
    }
    // but do not write the base URI to the file!
    if (librdf_serializer_set_feature(pSerializer.get(),
        pWriteBaseURI.get(), p0.get()))
    {
        throw uno::RuntimeException(
            u"librdf_Repository::exportGraph: "
            "librdf_serializer_set_feature writeBaseURI failed"_ustr, *this);
    }

    size_t length;
    const std::shared_ptr<unsigned char> pBuf(
        librdf_serializer_serialize_stream_to_counted_string(
            pSerializer.get(), pBaseURI.get(), pStream.get(), &length), free);
    if (!pBuf) {
        throw rdf::RepositoryException(
            u"librdf_Repository::exportGraph: "
            "librdf_serializer_serialize_stream_to_counted_string failed"_ustr,
            *this);
    }

    g.unlock(); // release Mutex before calling i_xOutStream methods

    addChaffWhenEncryptedStorage(i_xOutStream, pBuf.get(), length);
}

uno::Sequence< uno::Reference< rdf::XURI > > SAL_CALL
librdf_Repository::getGraphNames()
{
    std::scoped_lock g(m_aMutex);
    ::std::vector< uno::Reference<rdf::XURI> > ret;
    std::transform(m_NamedGraphs.begin(), m_NamedGraphs.end(),
        std::back_inserter(ret),
        [](std::pair<OUString, ::rtl::Reference<librdf_NamedGraph>> const& it)
            { return it.second->getName(); });
    return comphelper::containerToSequence(ret);
}

uno::Reference< rdf::XNamedGraph > SAL_CALL
librdf_Repository::getGraph(const uno::Reference< rdf::XURI > & i_xGraphName)
{
    if (!i_xGraphName.is()) {
        throw lang::IllegalArgumentException(
                u"librdf_Repository::getGraph: URI is null"_ustr, *this, 0);
    }
    const OUString contextU( i_xGraphName->getStringValue() );

    std::scoped_lock g(m_aMutex);
    const NamedGraphMap_t::iterator iter( m_NamedGraphs.find(contextU) );
    if (iter != m_NamedGraphs.end()) {
        return iter->second;
    } else {
        return nullptr;
    }
}

uno::Reference< rdf::XNamedGraph > SAL_CALL
librdf_Repository::createGraph(const uno::Reference< rdf::XURI > & i_xGraphName)
{
    if (!i_xGraphName.is()) {
        throw lang::IllegalArgumentException(
                u"librdf_Repository::createGraph: URI is null"_ustr, *this, 0);
    }

    const OUString contextU( i_xGraphName->getStringValue() );
    if (contextU.startsWith(s_nsOOo))
    {
        throw lang::IllegalArgumentException(
                u"librdf_Repository::createGraph: URI is reserved"_ustr, *this, 0);
    }

    std::scoped_lock g(m_aMutex); // don't call i_x* with mutex locked

    // NB: librdf does not have a concept of graphs as such;
    //     a librdf named graph exists iff the model contains a statement with
    //     the graph name as context

    if (m_NamedGraphs.find(contextU) != m_NamedGraphs.end()) {
        throw container::ElementExistException(
                u"librdf_Repository::createGraph: graph with given URI exists"_ustr, *this);
    }
    m_NamedGraphs.insert(std::make_pair(contextU,
        new librdf_NamedGraph(this, i_xGraphName)));
    return m_NamedGraphs.find(contextU)->second;
}

void SAL_CALL
librdf_Repository::destroyGraph(
        const uno::Reference< rdf::XURI > & i_xGraphName)
{
    if (!i_xGraphName.is()) {
        throw lang::IllegalArgumentException(
                u"librdf_Repository::destroyGraph: URI is null"_ustr, *this, 0);
    }
    const OUString contextU( i_xGraphName->getStringValue() );

    std::scoped_lock g(m_aMutex); // don't call i_x* with mutex locked

    const NamedGraphMap_t::iterator iter( clearGraph_Lock(contextU, false) );
    m_NamedGraphs.erase(iter);
}

bool isMetadatableWithoutMetadata(
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
{
    if (isMetadatableWithoutMetadata(i_xSubject)   ||
        isMetadatableWithoutMetadata(i_xPredicate) ||
        isMetadatableWithoutMetadata(i_xObject))
    {
        return new librdf_GraphResult(this, m_aMutex,
            std::shared_ptr<librdf_stream>(),
            std::shared_ptr<librdf_node>());
    }

    librdf_TypeConverter::Statement const stmt(
        librdf_TypeConverter::extractStatement_NoLock(
            i_xSubject, i_xPredicate, i_xObject));

    std::scoped_lock g(m_aMutex); // don't call i_x* with mutex locked

    const std::shared_ptr<librdf_statement> pStatement(
        librdf_TypeConverter::mkStatement_Lock(m_pWorld.get(), stmt),
        safe_librdf_free_statement);
    OSL_ENSURE(pStatement, "mkStatement failed");

    const std::shared_ptr<librdf_stream> pStream(
        librdf_model_find_statements(m_pModel.get(), pStatement.get()),
        safe_librdf_free_stream);
    if (!pStream) {
        throw rdf::RepositoryException(
            u"librdf_Repository::getStatements: "
            "librdf_model_find_statements failed"_ustr, *this);
    }

    return new librdf_GraphResult(this, m_aMutex, pStream,
        std::shared_ptr<librdf_node>());
}


uno::Reference< rdf::XQuerySelectResult > SAL_CALL
librdf_Repository::querySelect(const OUString & i_rQuery)
{
    std::scoped_lock g(m_aMutex);
    const OString query(
        OUStringToOString(i_rQuery, RTL_TEXTENCODING_UTF8) );
    const std::shared_ptr<librdf_query> pQuery(
        librdf_new_query(m_pWorld.get(), s_sparql, nullptr,
            reinterpret_cast<const unsigned char*> (query.getStr()), nullptr),
        safe_librdf_free_query);
    if (!pQuery) {
        throw rdf::QueryException(
            u"librdf_Repository::querySelect: "
            "librdf_new_query failed"_ustr, *this);
    }
    const std::shared_ptr<librdf_query_results> pResults(
        librdf_model_query_execute(m_pModel.get(), pQuery.get()),
        safe_librdf_free_query_results);
    if (!pResults || !librdf_query_results_is_bindings(pResults.get())) {
        throw rdf::QueryException(
            u"librdf_Repository::querySelect: "
            "query result is null or not bindings"_ustr, *this);
    }

    const int count( librdf_query_results_get_bindings_count(pResults.get()) );
    if (count < 0) {
        throw rdf::QueryException(
            u"librdf_Repository::querySelect: "
            "librdf_query_results_get_bindings_count failed"_ustr, *this);
    }
    uno::Sequence< OUString > names(count);
    auto namesRange = asNonConstRange(names);
    for (int i = 0; i < count; ++i) {
        const char* name( librdf_query_results_get_binding_name(
            pResults.get(), i) );
        if (!name) {
            throw rdf::QueryException(
                u"librdf_Repository::querySelect: binding is null"_ustr, *this);
        }

        namesRange[i] = OUString::createFromAscii(name);
    }

    return new librdf_QuerySelectResult(this, m_aMutex,
        pQuery, pResults, names);
}

uno::Reference< container::XEnumeration > SAL_CALL
librdf_Repository::queryConstruct(const OUString & i_rQuery)
{
    std::scoped_lock g(m_aMutex);
    const OString query(
        OUStringToOString(i_rQuery, RTL_TEXTENCODING_UTF8) );
    const std::shared_ptr<librdf_query> pQuery(
        librdf_new_query(m_pWorld.get(), s_sparql, nullptr,
            reinterpret_cast<const unsigned char*> (query.getStr()), nullptr),
        safe_librdf_free_query);
    if (!pQuery) {
        throw rdf::QueryException(
            u"librdf_Repository::queryConstruct: "
            "librdf_new_query failed"_ustr, *this);
    }
    const std::shared_ptr<librdf_query_results> pResults(
        librdf_model_query_execute(m_pModel.get(), pQuery.get()),
        safe_librdf_free_query_results);
    if (!pResults || !librdf_query_results_is_graph(pResults.get())) {
        throw rdf::QueryException(
            u"librdf_Repository::queryConstruct: "
            "query result is null or not graph"_ustr, *this);
    }
    const std::shared_ptr<librdf_stream> pStream(
        librdf_query_results_as_stream(pResults.get()),
        safe_librdf_free_stream);
    if (!pStream) {
        throw rdf::QueryException(
            u"librdf_Repository::queryConstruct: "
            "librdf_query_results_as_stream failed"_ustr, *this);
    }

    return new librdf_GraphResult(this, m_aMutex, pStream,
                                  std::shared_ptr<librdf_node>(), pQuery);
}

sal_Bool SAL_CALL
librdf_Repository::queryAsk(const OUString & i_rQuery)
{
    std::scoped_lock g(m_aMutex);

    const OString query(
        OUStringToOString(i_rQuery, RTL_TEXTENCODING_UTF8) );
    const std::shared_ptr<librdf_query> pQuery(
        librdf_new_query(m_pWorld.get(), s_sparql, nullptr,
            reinterpret_cast<const unsigned char*> (query.getStr()), nullptr),
        safe_librdf_free_query);
    if (!pQuery) {
        throw rdf::QueryException(
            u"librdf_Repository::queryAsk: "
            "librdf_new_query failed"_ustr, *this);
    }
    const std::shared_ptr<librdf_query_results> pResults(
        librdf_model_query_execute(m_pModel.get(), pQuery.get()),
        safe_librdf_free_query_results);
    if (!pResults || !librdf_query_results_is_boolean(pResults.get())) {
        throw rdf::QueryException(
            u"librdf_Repository::queryAsk: "
            "query result is null or not boolean"_ustr, *this);
    }
    return bool(librdf_query_results_get_boolean(pResults.get()));
}

// css::rdf::XDocumentRepository:
void SAL_CALL librdf_Repository::setStatementRDFa(
    const uno::Reference< rdf::XResource > & i_xSubject,
    const uno::Sequence< uno::Reference< rdf::XURI > > & i_rPredicates,
    const uno::Reference< rdf::XMetadatable > & i_xObject,
    const OUString & i_rRDFaContent,
    const uno::Reference< rdf::XURI > & i_xRDFaDatatype)
{
    if (!i_xSubject.is()) {
        throw lang::IllegalArgumentException(
            u"librdf_Repository::setStatementRDFa: Subject is null"_ustr, *this, 0);
    }
    if (!i_rPredicates.hasElements()) {
        throw lang::IllegalArgumentException(
            u"librdf_Repository::setStatementRDFa: no Predicates"_ustr,
            *this, 1);
    }
    if (std::any_of(i_rPredicates.begin(), i_rPredicates.end(),
            [](const uno::Reference< rdf::XURI >& rPredicate) { return !rPredicate.is(); })) {
        throw lang::IllegalArgumentException(
            u"librdf_Repository::setStatementRDFa: Predicate is null"_ustr, *this, 1);
    }
    if (!i_xObject.is()) {
        throw lang::IllegalArgumentException(
            u"librdf_Repository::setStatementRDFa: Object is null"_ustr, *this, 2);
    }
    const uno::Reference<lang::XServiceInfo> xService(i_xObject,
        uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextRange> xTextRange;
    if (xService->supportsService(u"com.sun.star.table.Cell"_ustr) ||
        xService->supportsService(u"com.sun.star.text.CellProperties"_ustr) || // for writer
        xService->supportsService(u"com.sun.star.text.Paragraph"_ustr))
    {
        xTextRange.set(i_xObject, uno::UNO_QUERY_THROW);
    }
    else if (xService->supportsService(u"com.sun.star.text.Bookmark"_ustr) ||
             xService->supportsService(u"com.sun.star.text.InContentMetadata"_ustr))
    {
        const uno::Reference<text::XTextContent> xTextContent(i_xObject,
            uno::UNO_QUERY_THROW);
        xTextRange = xTextContent->getAnchor();
    }
    if (!xTextRange.is()) {
        throw lang::IllegalArgumentException(
            u"librdf_Repository::setStatementRDFa: "
            "Object does not support RDFa"_ustr, *this, 2);
    }
    // ensure that the metadatable has an XML ID
    i_xObject->ensureMetadataReference();
    const beans::StringPair mdref( i_xObject->getMetadataReference() );
    if ((mdref.First.isEmpty()) || (mdref.Second.isEmpty())) {
        throw uno::RuntimeException(
                u"librdf_Repository::setStatementRDFa: "
                "ensureMetadataReference did not"_ustr, *this);
    }
    OUString const sXmlId(mdref.First + "#" + mdref.Second);
    OUString const sContext(s_nsOOo + sXmlId);
    OUString const content( (i_rRDFaContent.isEmpty())
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
    } catch (const lang::IllegalArgumentException &) {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException(
                u"librdf_Repository::setStatementRDFa: "
                "cannot create literal"_ustr, *this, anyEx);
    }

    std::shared_ptr<librdf_TypeConverter::Resource> const pSubject(
        librdf_TypeConverter::extractResource_NoLock(i_xSubject));
    std::shared_ptr<librdf_TypeConverter::Node> const pContent(
        librdf_TypeConverter::extractNode_NoLock(xContent));
    ::std::vector< std::shared_ptr<librdf_TypeConverter::Resource> >
        predicates;
    ::std::transform(i_rPredicates.begin(), i_rPredicates.end(),
        ::std::back_inserter(predicates),
        [](uno::Reference<rdf::XURI> const& xURI)
            { return librdf_TypeConverter::extractResource_NoLock(xURI); });

    removeStatementRDFa(i_xObject); // not atomic with insertion?

    std::scoped_lock g(m_aMutex); // don't call i_x* with mutex locked

    if (i_rRDFaContent.isEmpty()) {
        m_RDFaXHTMLContentSet.erase(sXmlId);
    } else {
        m_RDFaXHTMLContentSet.insert(sXmlId);
    }
    try
    {
        for (const auto& rPredicatePtr : predicates)
        {
            addStatementGraph_Lock(
                librdf_TypeConverter::Statement(pSubject,
                    std::dynamic_pointer_cast<librdf_TypeConverter::URI>(rPredicatePtr),
                    pContent),
                sContext, true);
        }
    }
    catch (const container::NoSuchElementException&)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException(
                u"librdf_Repository::setStatementRDFa: "
                "cannot addStatementGraph"_ustr, *this, anyEx);
    }
}

void SAL_CALL librdf_Repository::removeStatementRDFa(
    const uno::Reference< rdf::XMetadatable > & i_xElement)
{
    if (!i_xElement.is()) {
        throw lang::IllegalArgumentException(
            u"librdf_Repository::removeStatementRDFa: Element is null"_ustr,
            *this, 0);
    }

    const beans::StringPair mdref( i_xElement->getMetadataReference() );
    if ((mdref.First.isEmpty()) || (mdref.Second.isEmpty())) {
        return; // nothing to do...
    }

    OUString const sXmlId(s_nsOOo + mdref.First + "#" + mdref.Second);

    clearGraph_NoLock(sXmlId, true);
}

beans::Pair< uno::Sequence<rdf::Statement>, sal_Bool > SAL_CALL
librdf_Repository::getStatementRDFa(
    const uno::Reference< rdf::XMetadatable > & i_xElement)
{
    if (!i_xElement.is()) {
        throw lang::IllegalArgumentException(
            u"librdf_Repository::getStatementRDFa: Element is null"_ustr, *this, 0);
    }
    const beans::StringPair mdref( i_xElement->getMetadataReference() );
    if ((mdref.First.isEmpty()) || (mdref.Second.isEmpty())) {
        return beans::Pair< uno::Sequence<rdf::Statement>, sal_Bool >();
    }
    OUString const sXmlId(mdref.First + "#" + mdref.Second);
    uno::Reference<rdf::XURI> xXmlId;
    try {
        xXmlId.set( rdf::URI::create(m_xContext, s_nsOOo + sXmlId),
            uno::UNO_SET_THROW);
    } catch (const lang::IllegalArgumentException &) {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException(
                u"librdf_Repository::getStatementRDFa: "
                "cannot create URI for XML ID"_ustr, *this, anyEx);
    }

    ::std::vector< rdf::Statement > ret;
    try
    {
        ret = getStatementsGraph_NoLock(nullptr, nullptr, nullptr, xXmlId, true);
    }
    catch (const container::NoSuchElementException&)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException(
                u"librdf_Repository::getStatementRDFa: "
                "cannot getStatementsGraph"_ustr, *this, anyEx);
    }

    std::scoped_lock g(m_aMutex); // don't call i_x* with mutex locked

    return beans::Pair< uno::Sequence<rdf::Statement>, sal_Bool >(
            comphelper::containerToSequence(ret), 0 != m_RDFaXHTMLContentSet.count(sXmlId));
}

extern "C"
librdf_statement *rdfa_context_stream_map_handler(
    librdf_stream *i_pStream, void *, librdf_statement *i_pStatement)
{
    OSL_ENSURE(i_pStream, "rdfa_context_stream_map_handler: stream null");
    if (i_pStream) {
        librdf_node *pCtxt(
#if LIBRDF_VERSION >= 10012
            librdf_stream_get_context2(i_pStream) );
#else
            static_cast<librdf_node *>(librdf_stream_get_context(i_pStream)) );
#endif
        OSL_ENSURE(pCtxt, "rdfa_context_stream_map_handler: context null");
        if (pCtxt && isInternalContext(pCtxt)) {
            return i_pStatement;
        }
    }
    return nullptr;
};

uno::Reference< container::XEnumeration > SAL_CALL
librdf_Repository::getStatementsRDFa(
    const uno::Reference< rdf::XResource > & i_xSubject,
    const uno::Reference< rdf::XURI > & i_xPredicate,
    const uno::Reference< rdf::XNode > & i_xObject)
{
    if (isMetadatableWithoutMetadata(i_xSubject)   ||
        isMetadatableWithoutMetadata(i_xPredicate) ||
        isMetadatableWithoutMetadata(i_xObject))
    {
        return new librdf_GraphResult(this, m_aMutex,
            std::shared_ptr<librdf_stream>(),
            std::shared_ptr<librdf_node>());
    }

    librdf_TypeConverter::Statement const stmt(
        librdf_TypeConverter::extractStatement_NoLock(
            i_xSubject, i_xPredicate, i_xObject));

    std::scoped_lock g(m_aMutex); // don't call i_x* with mutex locked

    const std::shared_ptr<librdf_statement> pStatement(
        librdf_TypeConverter::mkStatement_Lock(m_pWorld.get(), stmt),
        safe_librdf_free_statement);
    OSL_ENSURE(pStatement, "mkStatement failed");

    std::shared_ptr<librdf_stream> pStream(
        librdf_model_find_statements(m_pModel.get(), pStatement.get()),
        safe_librdf_free_stream);
    if (!pStream) {
        throw rdf::RepositoryException(
            u"librdf_Repository::getStatementsRDFa: "
            "librdf_model_find_statements failed"_ustr, *this);
    }

    if (librdf_stream_add_map(pStream.get(), rdfa_context_stream_map_handler,
                nullptr, nullptr)) {
        throw rdf::RepositoryException(
            u"librdf_Repository::getStatementsRDFa: "
            "librdf_stream_add_map failed"_ustr, *this);
    }

    return new librdf_GraphResult(this, m_aMutex, std::move(pStream),
                                  std::shared_ptr<librdf_node>());
}

// css::lang::XInitialization:
void SAL_CALL librdf_Repository::initialize(
    const uno::Sequence< css::uno::Any > &)
{
    std::scoped_lock g(m_aMutex);

//    m_pWorld.reset(m_TypeConverter.createWorld(), safe_librdf_free_world);
    m_pStorage.reset(m_TypeConverter.createStorage_Lock(m_pWorld.get()),
        safe_librdf_free_storage);
    m_pModel.reset(m_TypeConverter.createModel_Lock(
        m_pWorld.get(), m_pStorage.get()), safe_librdf_free_model);
}

NamedGraphMap_t::iterator librdf_Repository::clearGraph_NoLock(
        OUString const& i_rGraphName, bool i_Internal)
//    throw (uno::RuntimeException, container::NoSuchElementException,
//        rdf::RepositoryException)
{
    std::scoped_lock g(m_aMutex);

    return clearGraph_Lock(i_rGraphName, i_Internal);
}

NamedGraphMap_t::iterator librdf_Repository::clearGraph_Lock(
        OUString const& i_rGraphName, bool i_Internal)
{
    // internal: must be called with mutex locked!
    const NamedGraphMap_t::iterator iter( m_NamedGraphs.find(i_rGraphName) );
    if (!i_Internal && iter == m_NamedGraphs.end()) {
        throw container::NoSuchElementException(
                u"librdf_Repository::clearGraph: "
                "no graph with given URI exists"_ustr, *this);
    }
    const OString context(
        OUStringToOString(i_rGraphName, RTL_TEXTENCODING_UTF8) );

    const std::shared_ptr<librdf_node> pContext(
        librdf_new_node_from_uri_string(m_pWorld.get(),
            reinterpret_cast<const unsigned char*> (context.getStr())),
        safe_librdf_free_node);
    if (!pContext) {
        throw uno::RuntimeException(
            u"librdf_Repository::clearGraph: "
            "librdf_new_node_from_uri_string failed"_ustr, *this);
    }
    if (librdf_model_context_remove_statements(m_pModel.get(), pContext.get()))
    {
        throw rdf::RepositoryException(
            u"librdf_Repository::clearGraph: "
            "librdf_model_context_remove_statements failed"_ustr, *this);
    }
    return iter;
}

void librdf_Repository::addStatementGraph_NoLock(
    const uno::Reference< rdf::XResource > & i_xSubject,
    const uno::Reference< rdf::XURI > & i_xPredicate,
    const uno::Reference< rdf::XNode > & i_xObject,
    const uno::Reference< rdf::XURI > & i_xGraphName)
//throw (uno::RuntimeException, lang::IllegalArgumentException,
//    container::NoSuchElementException, rdf::RepositoryException)
{
    if (!i_xSubject.is()) {
        throw lang::IllegalArgumentException(
            u"librdf_Repository::addStatement: Subject is null"_ustr, *this, 0);
    }
    if (!i_xPredicate.is()) {
        throw lang::IllegalArgumentException(
            u"librdf_Repository::addStatement: Predicate is null"_ustr,
            *this, 1);
    }
    if (!i_xObject.is()) {
        throw lang::IllegalArgumentException(
            u"librdf_Repository::addStatement: Object is null"_ustr, *this, 2);
    }

    librdf_TypeConverter::Statement const stmt(
        librdf_TypeConverter::extractStatement_NoLock(
            i_xSubject, i_xPredicate, i_xObject));

    const OUString contextU( i_xGraphName->getStringValue() );

    std::scoped_lock g(m_aMutex); // don't call i_x* with mutex locked

    addStatementGraph_Lock(stmt, contextU, false/*i_Internal*/);
}

void librdf_Repository::addStatementGraph_Lock(
    librdf_TypeConverter::Statement const& i_rStatement,
    OUString const& i_rGraphName,
    bool i_Internal)
{
    if (!i_Internal
        && (m_NamedGraphs.find(i_rGraphName) == m_NamedGraphs.end()))
    {
        throw container::NoSuchElementException(
                u"librdf_Repository::addStatement: "
                "no graph with given URI exists"_ustr, *this);
    }
    const OString context(
        OUStringToOString(i_rGraphName, RTL_TEXTENCODING_UTF8) );

    const std::shared_ptr<librdf_node> pContext(
        librdf_new_node_from_uri_string(m_pWorld.get(),
            reinterpret_cast<const unsigned char*> (context.getStr())),
        safe_librdf_free_node);
    if (!pContext) {
        throw uno::RuntimeException(
            u"librdf_Repository::addStatement: "
            "librdf_new_node_from_uri_string failed"_ustr, *this);
    }
    const std::shared_ptr<librdf_statement> pStatement(
        librdf_TypeConverter::mkStatement_Lock(m_pWorld.get(), i_rStatement),
        safe_librdf_free_statement);
    OSL_ENSURE(pStatement, "mkStatement failed");

    // Test for duplicate statement
    // librdf_model_add_statement disallows duplicates while
    // librdf_model_context_add_statement allows duplicates
    {
        const std::shared_ptr<librdf_stream> pStream(
            librdf_model_find_statements_in_context(m_pModel.get(),
                pStatement.get(), pContext.get()),
            safe_librdf_free_stream);
        if (pStream && !librdf_stream_end(pStream.get()))
            return;
    }

    if (librdf_model_context_add_statement(m_pModel.get(),
            pContext.get(), pStatement.get())) {
        throw rdf::RepositoryException(
            u"librdf_Repository::addStatement: "
            "librdf_model_context_add_statement failed"_ustr, *this);
    }
}

void librdf_Repository::removeStatementsGraph_NoLock(
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

    librdf_TypeConverter::Statement const stmt(
        librdf_TypeConverter::extractStatement_NoLock(
            i_xSubject, i_xPredicate, i_xObject));
    const OUString contextU( i_xGraphName->getStringValue() );

    std::scoped_lock g(m_aMutex); // don't call i_x* with mutex locked

    if (m_NamedGraphs.find(contextU) == m_NamedGraphs.end()) {
        throw container::NoSuchElementException(
                u"librdf_Repository::removeStatements: "
                "no graph with given URI exists"_ustr, *this);
    }
    const OString context(
        OUStringToOString(contextU, RTL_TEXTENCODING_UTF8) );

    const std::shared_ptr<librdf_node> pContext(
        librdf_new_node_from_uri_string(m_pWorld.get(),
            reinterpret_cast<const unsigned char*> (context.getStr())),
        safe_librdf_free_node);
    if (!pContext) {
        throw uno::RuntimeException(
            u"librdf_Repository::removeStatements: "
            "librdf_new_node_from_uri_string failed"_ustr, *this);
    }
    const std::shared_ptr<librdf_statement> pStatement(
        librdf_TypeConverter::mkStatement_Lock(m_pWorld.get(), stmt),
        safe_librdf_free_statement);
    OSL_ENSURE(pStatement, "mkStatement failed");

    const std::shared_ptr<librdf_stream> pStream(
        librdf_model_find_statements_in_context(m_pModel.get(),
            pStatement.get(), pContext.get()),
        safe_librdf_free_stream);
    if (!pStream) {
        throw rdf::RepositoryException(
            u"librdf_Repository::removeStatements: "
            "librdf_model_find_statements_in_context failed"_ustr, *this);
    }

    if (librdf_stream_end(pStream.get()))
        return;

    do {
        librdf_statement *pStmt( librdf_stream_get_object(pStream.get()) );
        if (!pStmt) {
            throw rdf::RepositoryException(
                u"librdf_Repository::removeStatements: "
                "librdf_stream_get_object failed"_ustr, *this);
        }
        if (librdf_model_context_remove_statement(m_pModel.get(),
                pContext.get(), pStmt)) {
            throw rdf::RepositoryException(
                u"librdf_Repository::removeStatements: "
                "librdf_model_context_remove_statement failed"_ustr, *this);
        }
    } while (!librdf_stream_next(pStream.get()));
}

std::vector<rdf::Statement>
librdf_Repository::getStatementsGraph_NoLock(
    const uno::Reference< rdf::XResource > & i_xSubject,
    const uno::Reference< rdf::XURI > & i_xPredicate,
    const uno::Reference< rdf::XNode > & i_xObject,
    const uno::Reference< rdf::XURI > & i_xGraphName,
    bool i_Internal)
//throw (uno::RuntimeException, lang::IllegalArgumentException,
//    container::NoSuchElementException, rdf::RepositoryException)
{
    std::vector<rdf::Statement> ret;

    // N.B.: if any of subject, predicate, object is an XMetadatable, and
    // has no metadata reference, then there cannot be any node in the graph
    // representing it; in order to prevent side effect
    // (ensureMetadataReference), check for this condition and return
    if (isMetadatableWithoutMetadata(i_xSubject)   ||
        isMetadatableWithoutMetadata(i_xPredicate) ||
        isMetadatableWithoutMetadata(i_xObject))
    {
        return ret;
    }

    librdf_TypeConverter::Statement const stmt(
        librdf_TypeConverter::extractStatement_NoLock(
            i_xSubject, i_xPredicate, i_xObject));
    const OUString contextU( i_xGraphName->getStringValue() );

    std::scoped_lock g(m_aMutex); // don't call i_x* with mutex locked

    if (!i_Internal && (m_NamedGraphs.find(contextU) == m_NamedGraphs.end())) {
        throw container::NoSuchElementException(
                u"librdf_Repository::getStatements: "
                "no graph with given URI exists"_ustr, *this);
    }
    const OString context(
        OUStringToOString(contextU, RTL_TEXTENCODING_UTF8) );

    const std::shared_ptr<librdf_node> pContext(
        librdf_new_node_from_uri_string(m_pWorld.get(),
            reinterpret_cast<const unsigned char*> (context.getStr())),
        safe_librdf_free_node);
    if (!pContext) {
        throw uno::RuntimeException(
            u"librdf_Repository::getStatements: "
            "librdf_new_node_from_uri_string failed"_ustr, *this);
    }
    const std::shared_ptr<librdf_statement> pStatement(
        librdf_TypeConverter::mkStatement_Lock(m_pWorld.get(), stmt),
        safe_librdf_free_statement);
    OSL_ENSURE(pStatement, "mkStatement failed");

    const std::shared_ptr<librdf_stream> pStream(
        librdf_model_find_statements_in_context(m_pModel.get(),
            pStatement.get(), pContext.get()),
        safe_librdf_free_stream);
    if (!pStream) {
        throw rdf::RepositoryException(
            u"librdf_Repository::getStatements: "
            "librdf_model_find_statements_in_context failed"_ustr, *this);
    }

    librdf_node *pCtxt1(
#if LIBRDF_VERSION >= 10012
        librdf_stream_get_context2(pStream.get()) );
#else
        static_cast<librdf_node *>(librdf_stream_get_context(pStream.get())) );
#endif
    while (!librdf_stream_end(pStream.get()))
    {
        auto pCtxt = pCtxt1;
        librdf_statement *pStmt( librdf_stream_get_object(pStream.get()) );
        if (!pStmt) {
            rdf::QueryException e(
                u"librdf_GraphResult::nextElement: "
                "librdf_stream_get_object failed"_ustr, *this);
            throw lang::WrappedTargetException(
                u"librdf_GraphResult::nextElement: "
                "librdf_stream_get_object failed"_ustr, *this,
                    uno::Any(e));
        }
        // NB: pCtxt may be null here if this is result of a graph query
        if (pCtxt && isInternalContext(pCtxt)) {
            pCtxt = nullptr; // XML ID context is implementation detail!
        }

        ret.emplace_back(
            getTypeConverter().convertToStatement(pStmt, pCtxt) );

        // NB: this will invalidate current item.
        librdf_stream_next(pStream.get());
    }

    return ret;
}

extern "C"
void librdf_raptor_init(void* /*user_data*/, raptor_world* pRaptorWorld)
{
    // fdo#64672 prevent raptor from setting global libxml2 error handlers
    raptor_world_set_flag(pRaptorWorld,
            RAPTOR_WORLD_FLAG_LIBXML_STRUCTURED_ERROR_SAVE, 0);
    raptor_world_set_flag(pRaptorWorld,
            RAPTOR_WORLD_FLAG_LIBXML_GENERIC_ERROR_SAVE, 0);
}

librdf_world *librdf_TypeConverter::createWorld_Lock() const
{
    // create and initialize world
    librdf_world *pWorld( librdf_new_world() );
    if (!pWorld) {
        throw uno::RuntimeException(
            u"librdf_TypeConverter::createWorld: librdf_new_world failed"_ustr,
            m_rRep);
    }
    librdf_world_set_raptor_init_handler(pWorld, nullptr, &librdf_raptor_init);
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
librdf_TypeConverter::createStorage_Lock(librdf_world *i_pWorld) const
{
    librdf_storage *pStorage(
//        librdf_new_storage(i_pWorld, "memory", NULL, "contexts='yes'") );
        librdf_new_storage(i_pWorld, "hashes", nullptr,
            "contexts='yes',hash-type='memory'") );
    if (!pStorage) {
        throw uno::RuntimeException(
            u"librdf_TypeConverter::createStorage: librdf_new_storage failed"_ustr,
            m_rRep);
    }
    return pStorage;
}

librdf_model *librdf_TypeConverter::createModel_Lock(
    librdf_world *i_pWorld, librdf_storage * i_pStorage) const
{
    librdf_model *pRepository( librdf_new_model(i_pWorld, i_pStorage, nullptr) );
    if (!pRepository) {
        throw uno::RuntimeException(
            u"librdf_TypeConverter::createModel: librdf_new_model failed"_ustr,
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
librdf_uri* librdf_TypeConverter::mkURI_Lock( librdf_world* i_pWorld,
    OString const& i_rURI)
{
    librdf_uri *pURI( librdf_new_uri(i_pWorld,
        reinterpret_cast<const unsigned char *>(i_rURI.getStr())));
    if (!pURI) {
        throw uno::RuntimeException(
            u"librdf_TypeConverter::mkURI: librdf_new_uri failed"_ustr, nullptr);
    }
    return pURI;
}

// extract blank or URI node - call without Mutex locked
std::shared_ptr<librdf_TypeConverter::Resource>
librdf_TypeConverter::extractResource_NoLock(
    const uno::Reference< rdf::XResource > & i_xResource)
{
    if (!i_xResource.is()) {
        return std::shared_ptr<Resource>();
    }
    uno::Reference< rdf::XBlankNode > xBlankNode(i_xResource, uno::UNO_QUERY);
    if (xBlankNode.is()) {
        const OString label(
            OUStringToOString(xBlankNode->getStringValue(),
            RTL_TEXTENCODING_UTF8) );
        return std::make_shared<BlankNode>(label);
    } else { // assumption: everything else is URI
        const OString uri(
            OUStringToOString(i_xResource->getStringValue(),
            RTL_TEXTENCODING_UTF8) );
        return std::make_shared<URI>(uri);
    }
}

void
librdf_TypeConverter::extractResourceToCacheKey_NoLock(
    const uno::Reference< rdf::XResource > & i_xResource, OUStringBuffer& rBuffer)
{
    if (!i_xResource.is()) {
        return;
    }
    uno::Reference< rdf::XBlankNode > xBlankNode(i_xResource, uno::UNO_QUERY);
    if (xBlankNode.is()) {
        rBuffer.append("BlankNode " + xBlankNode->getStringValue());
    } else { // assumption: everything else is URI
        rBuffer.append("URI " + i_xResource->getStringValue());
    }
}

// create blank or URI node
librdf_node* librdf_TypeConverter::mkResource_Lock( librdf_world* i_pWorld,
    Resource const*const i_pResource)
{
    if (!i_pResource) return nullptr;
    BlankNode const*const pBlankNode(
            dynamic_cast<BlankNode const*>(i_pResource));
    if (pBlankNode) {
        librdf_node *pNode(
            librdf_new_node_from_blank_identifier(i_pWorld,
                reinterpret_cast<const unsigned char*>(
                    pBlankNode->value.getStr())));
        if (!pNode) {
            throw uno::RuntimeException(
                u"librdf_TypeConverter::mkResource: "
                "librdf_new_node_from_blank_identifier failed"_ustr, nullptr);
        }
        return pNode;
    } else { // assumption: everything else is URI
        URI const*const pURI(dynamic_cast<URI const*>(i_pResource));
        assert(pURI);
        librdf_node *pNode(
            librdf_new_node_from_uri_string(i_pWorld,
                reinterpret_cast<const unsigned char*>(pURI->value.getStr())));
        if (!pNode) {
            throw uno::RuntimeException(
                u"librdf_TypeConverter::mkResource: "
                "librdf_new_node_from_uri_string failed"_ustr, nullptr);
        }
        return pNode;
    }
}

// extract blank or URI or literal node - call without Mutex locked
std::shared_ptr<librdf_TypeConverter::Node>
librdf_TypeConverter::extractNode_NoLock(
    const uno::Reference< rdf::XNode > & i_xNode)
{
    if (!i_xNode.is()) {
        return std::shared_ptr<Node>();
    }
    uno::Reference< rdf::XResource > xResource(i_xNode, uno::UNO_QUERY);
    if (xResource.is()) {
        return extractResource_NoLock(xResource);
    }
    uno::Reference< rdf::XLiteral> xLiteral(i_xNode, uno::UNO_QUERY);
    OSL_ENSURE(xLiteral.is(),
        "mkNode: someone invented a new rdf.XNode and did not tell me");
    if (!xLiteral.is()) {
        return std::shared_ptr<Node>();
    }
    const OString val(
        OUStringToOString(xLiteral->getValue(),
        RTL_TEXTENCODING_UTF8) );
    const OString lang(
        OUStringToOString(xLiteral->getLanguage(),
        RTL_TEXTENCODING_UTF8) );
    const uno::Reference< rdf::XURI > xType(xLiteral->getDatatype());
    std::optional<OString> type;
    if (xType.is())
    {
        type =
            OUStringToOString(xType->getStringValue(), RTL_TEXTENCODING_UTF8);
    }
    return std::make_shared<Literal>(val, lang, type);
}

// extract blank or URI or literal node - call without Mutex locked
void
librdf_TypeConverter::extractNodeToCacheKey_NoLock(
    const uno::Reference< rdf::XNode > & i_xNode,
    OUStringBuffer& rBuffer)
{
    if (!i_xNode.is()) {
        return;
    }
    uno::Reference< rdf::XResource > xResource(i_xNode, uno::UNO_QUERY);
    if (xResource.is()) {
        return extractResourceToCacheKey_NoLock(xResource, rBuffer);
    }
    uno::Reference< rdf::XLiteral> xLiteral(i_xNode, uno::UNO_QUERY);
    OSL_ENSURE(xLiteral.is(),
        "mkNode: someone invented a new rdf.XNode and did not tell me");
    if (!xLiteral.is()) {
        return;
    }
    rBuffer.append("Literal " + xLiteral->getValue() + "\t" + xLiteral->getLanguage());
    const uno::Reference< rdf::XURI > xType(xLiteral->getDatatype());
    if (xType.is())
        rBuffer.append("\t" + xType->getStringValue());
}

// create blank or URI or literal node
librdf_node* librdf_TypeConverter::mkNode_Lock( librdf_world* i_pWorld,
    Node const*const i_pNode)
{
    if (!i_pNode) return nullptr;
    Resource const*const pResource(dynamic_cast<Resource const*>(i_pNode));
    if (pResource) {
        return mkResource_Lock(i_pWorld, pResource);
    }

    Literal const*const pLiteral(dynamic_cast<Literal const*>(i_pNode));
    assert(pLiteral);
    librdf_node * ret(nullptr);
    if (pLiteral->language.isEmpty()) {
        if (!pLiteral->type) {
            ret = librdf_new_node_from_literal(i_pWorld,
                reinterpret_cast<const unsigned char*>(pLiteral->value.getStr())
                , nullptr, 0);
        } else {
            const std::shared_ptr<librdf_uri> pDatatype(
                mkURI_Lock(i_pWorld, *pLiteral->type),
                safe_librdf_free_uri);
            ret = librdf_new_node_from_typed_literal(i_pWorld,
                reinterpret_cast<const unsigned char*>(pLiteral->value.getStr())
                , nullptr, pDatatype.get());
        }
    } else {
        if (!pLiteral->type) {
            ret = librdf_new_node_from_literal(i_pWorld,
                reinterpret_cast<const unsigned char*>(pLiteral->value.getStr())
                , pLiteral->language.getStr(), 0);
        } else {
            OSL_FAIL("mkNode: invalid literal");
            return nullptr;
        }
    }
    if (!ret) {
        throw uno::RuntimeException(
            u"librdf_TypeConverter::mkNode: librdf_new_node_from_literal failed"_ustr, nullptr);
    }
    return ret;
}

// extract statement - call without Mutex locked
librdf_TypeConverter::Statement librdf_TypeConverter::extractStatement_NoLock(
    const uno::Reference< rdf::XResource > & i_xSubject,
    const uno::Reference< rdf::XURI > & i_xPredicate,
    const uno::Reference< rdf::XNode > & i_xObject)
{
    std::shared_ptr<Resource> pSubject(
            extractResource_NoLock(i_xSubject));
    std::shared_ptr<URI> pPredicate(
        std::dynamic_pointer_cast<URI>(extractResource_NoLock(i_xPredicate)));
    std::shared_ptr<Node> pObject(extractNode_NoLock(i_xObject));
    return Statement(std::move(pSubject), std::move(pPredicate), std::move(pObject));
}

librdf_statement* librdf_TypeConverter::mkStatement_Lock(librdf_world* i_pWorld,
    Statement const& i_rStatement)
{
    librdf_node *const pSubject(
            mkResource_Lock(i_pWorld, i_rStatement.pSubject.get()) );
    librdf_node* pPredicate(nullptr);
    librdf_node* pObject(nullptr);
    try {
        pPredicate = mkResource_Lock(i_pWorld, i_rStatement.pPredicate.get());
        try {
            pObject = mkNode_Lock(i_pWorld, i_rStatement.pObject.get());
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
            u"librdf_TypeConverter::mkStatement: "
            "librdf_new_statement_from_nodes failed"_ustr, nullptr);
    }
    return pStatement;
}

uno::Reference<rdf::XURI>
librdf_TypeConverter::convertToXURI(librdf_uri* i_pURI) const
{
    if (!i_pURI) return nullptr;
    const unsigned char* uri( librdf_uri_as_string(i_pURI) );
    if (!uri) {
        throw uno::RuntimeException(
            u"librdf_TypeConverter::convertToXURI: "
            "librdf_uri_as_string failed"_ustr, m_rRep);
    }
    OUString uriU( OStringToOUString(
        std::string_view(reinterpret_cast<const char*>(uri)),
        RTL_TEXTENCODING_UTF8) );
    try {
        return rdf::URI::create(m_xContext, uriU);
    } catch (const lang::IllegalArgumentException &) {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException(
                u"librdf_TypeConverter::convertToXURI: "
                "illegal uri"_ustr, m_rRep, anyEx);
    }
}

uno::Reference<rdf::XURI>
librdf_TypeConverter::convertToXURI(librdf_node* i_pNode) const
{
    if (!i_pNode) return nullptr;
    if (librdf_node_is_resource(i_pNode)) {
        librdf_uri* pURI( librdf_node_get_uri(i_pNode) );
        if (!pURI) {
            throw uno::RuntimeException(
                u"librdf_TypeConverter::convertToXURI: "
                "resource has no uri"_ustr, m_rRep);
        }
        return convertToXURI(pURI);
    } else {
        OSL_FAIL("convertToXURI: unknown librdf_node");
        return nullptr;
    }
}

uno::Reference<rdf::XResource>
librdf_TypeConverter::convertToXResource(librdf_node* i_pNode) const
{
    if (!i_pNode) return nullptr;
    if (librdf_node_is_blank(i_pNode)) {
        const unsigned char* label( librdf_node_get_blank_identifier(i_pNode) );
        if (!label) {
            throw uno::RuntimeException(
                u"librdf_TypeConverter::convertToXResource: "
                "blank node has no label"_ustr, m_rRep);
        }
        OUString labelU( OStringToOUString(
            std::string_view(reinterpret_cast<const char*>(label)),
            RTL_TEXTENCODING_UTF8) );
        try {
            return rdf::BlankNode::create(m_xContext, labelU);
        } catch (const lang::IllegalArgumentException &) {
            css::uno::Any anyEx = cppu::getCaughtException();
            throw lang::WrappedTargetRuntimeException(
                    u"librdf_TypeConverter::convertToXResource: "
                    "illegal blank node label"_ustr, m_rRep, anyEx);
        }
    } else {
        return convertToXURI(i_pNode);
    }
}

uno::Reference<rdf::XNode>
librdf_TypeConverter::convertToXNode(librdf_node* i_pNode) const
{
    if (!i_pNode) return nullptr;
    if (!librdf_node_is_literal(i_pNode)) {
        return convertToXResource(i_pNode);
    }
    const unsigned char* value( librdf_node_get_literal_value(i_pNode) );
    if (!value) {
        throw uno::RuntimeException(
            u"librdf_TypeConverter::convertToXNode: "
            "literal has no value"_ustr, m_rRep);
    }
    const char * lang( librdf_node_get_literal_value_language(i_pNode) );
    librdf_uri* pType(
        librdf_node_get_literal_value_datatype_uri(i_pNode) );
    OSL_ENSURE(!lang || !pType, "convertToXNode: invalid literal");
    const OUString valueU( OStringToOUString(
        std::string_view(reinterpret_cast<const char*>(value)),
        RTL_TEXTENCODING_UTF8) );
    if (lang) {
        const OUString langU( OStringToOUString(
            std::string_view(lang),
            RTL_TEXTENCODING_UTF8) );
        return rdf::Literal::createWithLanguage(m_xContext, valueU, langU);
    } else if (pType) {
        uno::Reference<rdf::XURI> xType(convertToXURI(pType));
        OSL_ENSURE(xType.is(), "convertToXNode: null uri");
        return rdf::Literal::createWithType(m_xContext, valueU, xType);
    } else {
        return rdf::Literal::create(m_xContext, valueU);
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


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
unoxml_rdfRepository_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new librdf_Repository(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
