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

#include "precompiled_sfx2.hxx"

#include <sfx2/DocumentMetadataAccess.hxx>

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/task/ErrorCodeIOException.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <com/sun/star/rdf/FileFormat.hpp>
#include <com/sun/star/rdf/URIs.hpp>
#include <com/sun/star/rdf/Statement.hpp>
#include <com/sun/star/rdf/Literal.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/Repository.hpp>

#include <rtl/uuid.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/uri.hxx>
#include <rtl/bootstrap.hxx>

#include <comphelper/interaction.hxx>
#include <comphelper/makesequence.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <comphelper/sequenceasvector.hxx>
#include <comphelper/storagehelper.hxx>

#include <sfx2/docfile.hxx>
#include <sfx2/XmlIdRegistry.hxx>

#include <libxml/tree.h>    // for xmlValidateNCName

#include <boost/bind.hpp>
#include <boost/shared_array.hpp>
#include <boost/tuple/tuple.hpp>

#include <vector>
#include <set>
#include <map>
#include <functional>
#include <algorithm>

#include <unotools/ucbhelper.hxx>
#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarPkgUrlReferenceFactory.hpp>


/*
 Note: in the context of this implementation, all rdf.QueryExceptions and
 rdf.RepositoryExceptions are RuntimeExceptions, and will be reported as such.

 This implementation assumes that it is only used with ODF documents, not mere
 ODF packages. In other words, we enforce that metadata files must not be
 called reserved names.
 */

using namespace ::com::sun::star;

namespace sfx2 {


bool isValidNCName(::rtl::OUString const & i_rIdref)
{
    const ::rtl::OString id(
        ::rtl::OUStringToOString(i_rIdref, RTL_TEXTENCODING_UTF8) );
    return !(xmlValidateNCName(
        reinterpret_cast<const unsigned char*>(id.getStr()), 0));
}


static const char s_content [] = "content.xml";
static const char s_styles  [] = "styles.xml";
static const char s_meta    [] = "meta.xml";
static const char s_settings[] = "settings.xml";
static const char s_manifest[] = "manifest.rdf";
static const char s_rdfxml  [] = "application/rdf+xml";
static const char s_odfmime [] = "application/vnd.oasis.opendocument.";


static bool isContentFile(::rtl::OUString const & i_rPath)
{
    return i_rPath.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(s_content));
}

static bool isStylesFile (::rtl::OUString const & i_rPath)
{
    return i_rPath.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(s_styles));
}

static bool isReservedFile(::rtl::OUString const & i_rPath)
{
    return isContentFile(i_rPath)
        || isStylesFile(i_rPath)
        || i_rPath.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(s_meta))
        || i_rPath.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(s_settings));
}


uno::Reference<rdf::XURI> createBaseURI(
    uno::Reference<uno::XComponentContext> const & i_xContext,
    uno::Reference<embed::XStorage> const & i_xStorage,
    ::rtl::OUString const & i_rPkgURI, ::rtl::OUString const & i_rSubDocument)
{
    if (!i_xContext.is() || !i_xStorage.is() || !i_rPkgURI.getLength()) {
        throw uno::RuntimeException();
    }

    // #i108078# workaround non-hierarchical vnd.sun.star.expand URIs
    // this really should be done somewhere else, not here.
    ::rtl::OUString pkgURI(i_rPkgURI);
    if (pkgURI.matchIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.expand:")))
    {
        // expand it here (makeAbsolute requires hierarchical URI)
        pkgURI = pkgURI.copy( RTL_CONSTASCII_LENGTH("vnd.sun.star.expand:") );
        if (pkgURI.getLength() != 0) {
            pkgURI = ::rtl::Uri::decode(
                    pkgURI, rtl_UriDecodeStrict, RTL_TEXTENCODING_UTF8);
            if (pkgURI.getLength() == 0) {
                throw uno::RuntimeException();
            }
            ::rtl::Bootstrap::expandMacros(pkgURI);
        }
    }

    const uno::Reference<lang::XMultiComponentFactory> xServiceFactory(
        i_xContext->getServiceManager(), uno::UNO_SET_THROW);
    const uno::Reference<uri::XUriReferenceFactory> xUriFactory(
        xServiceFactory->createInstanceWithContext(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.uri.UriReferenceFactory")), i_xContext),
        uno::UNO_QUERY_THROW);
    uno::Reference< uri::XUriReference > xBaseURI;

    const uno::Reference< uri::XUriReference > xPkgURI(
        xUriFactory->parse(pkgURI), uno::UNO_SET_THROW );
    xPkgURI->clearFragment();

    // need to know whether the storage is a FileSystemStorage
    // XServiceInfo would be better, but it is not implemented
//    if ( pkgURI.getLength() && ::utl::UCBContentHelper::IsFolder(pkgURI) )
    if (true) {
        xBaseURI.set( xPkgURI, uno::UNO_SET_THROW );
    }
    ::rtl::OUStringBuffer buf;
    if (!xBaseURI->getUriReference().endsWithAsciiL("/", 1))
    {
        const sal_Int32 count( xBaseURI->getPathSegmentCount() );
        if (count > 0)
        {
            const ::rtl::OUString last( xBaseURI->getPathSegment(count - 1) );
            buf.append(last);
        }
        buf.append(static_cast<sal_Unicode>('/'));
    }
    if (i_rSubDocument.getLength())
    {
        buf.append(i_rSubDocument);
        buf.append(static_cast<sal_Unicode>('/'));
    }
    const ::rtl::OUString Path(buf.makeStringAndClear());
    if (Path.getLength())
    {
        const uno::Reference< uri::XUriReference > xPathURI(
            xUriFactory->parse(Path), uno::UNO_SET_THROW );
        xBaseURI.set(
            xUriFactory->makeAbsolute(xBaseURI, xPathURI,
                true, uri::RelativeUriExcessParentSegments_ERROR),
            uno::UNO_SET_THROW);
    }

    return rdf::URI::create(i_xContext, xBaseURI->getUriReference());
}


struct DocumentMetadataAccess_Impl
{
    // note: these are all initialized in constructor, and loadFromStorage
    const uno::Reference<uno::XComponentContext> m_xContext;
    const IXmlIdRegistrySupplier & m_rXmlIdRegistrySupplier;
    uno::Reference<rdf::XURI> m_xBaseURI;
    uno::Reference<rdf::XRepository> m_xRepository;
    uno::Reference<rdf::XNamedGraph> m_xManifest;
    DocumentMetadataAccess_Impl(
            uno::Reference<uno::XComponentContext> const& i_xContext,
            IXmlIdRegistrySupplier const & i_rRegistrySupplier)
      : m_xContext(i_xContext)
      , m_rXmlIdRegistrySupplier(i_rRegistrySupplier)
      , m_xBaseURI()
      , m_xRepository()
      , m_xManifest()
    {
        OSL_ENSURE(m_xContext.is(), "context null");
    }
};

// this is... a hack.
template<sal_Int16 Constant>
/*static*/ uno::Reference<rdf::XURI>
getURI(uno::Reference< uno::XComponentContext > const & i_xContext)
{
    static uno::Reference< rdf::XURI > xURI(
        rdf::URI::createKnown(i_xContext, Constant), uno::UNO_QUERY_THROW);
    return xURI;
}


/** would storing the file to a XStorage succeed? */
static bool isFileNameValid(const ::rtl::OUString & i_rFileName)
{
    if (i_rFileName.getLength() <= 0) return false;
    if (i_rFileName[0] == '/')        return false; // no absolute paths!
    sal_Int32 idx(0);
    do {
      const ::rtl::OUString segment(
        i_rFileName.getToken(0, static_cast<sal_Unicode> ('/'), idx) );
      if (!segment.getLength()      ||  // no empty segments
          segment.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("."))  ||  // no . segments
          segment.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("..")) ||  // no .. segments
          !::comphelper::OStorageHelper::IsValidZipEntryFileName(
              segment, sal_False))      // no invalid characters
                                      return false;
    } while (idx >= 0);
    return true;
}

/** split a uri hierarchy into first segment and rest */
static bool
splitPath(::rtl::OUString const & i_rPath,
    ::rtl::OUString & o_rDir, ::rtl::OUString& o_rRest)
{
    const sal_Int32 idx(i_rPath.indexOf(static_cast<sal_Unicode>('/')));
    if (idx < 0 || idx >= i_rPath.getLength()) {
        o_rDir = ::rtl::OUString();
        o_rRest = i_rPath;
        return true;
    } else if (idx == 0 || idx == i_rPath.getLength() - 1) {
        // input must not start or end with '/'
        return false;
    } else {
        o_rDir  = (i_rPath.copy(0, idx));
        o_rRest = (i_rPath.copy(idx+1));
        return true;
    }
}

static bool
splitXmlId(::rtl::OUString const & i_XmlId,
    ::rtl::OUString & o_StreamName, ::rtl::OUString& o_Idref )
{
    const sal_Int32 idx(i_XmlId.indexOf(static_cast<sal_Unicode>('#')));
    if ((idx <= 0) || (idx >= i_XmlId.getLength() - 1)) {
        return false;
    } else {
        o_StreamName = (i_XmlId.copy(0, idx));
        o_Idref      = (i_XmlId.copy(idx+1));
        return isValidXmlId(o_StreamName, o_Idref);
    }
}


static uno::Reference<rdf::XURI>
getURIForStream(struct DocumentMetadataAccess_Impl& i_rImpl,
    ::rtl::OUString const& i_rPath)
{
    const uno::Reference<rdf::XURI> xURI(
        rdf::URI::createNS( i_rImpl.m_xContext,
            i_rImpl.m_xBaseURI->getStringValue(), i_rPath),
        uno::UNO_SET_THROW);
    return xURI;
}

/** add statements declaring i_xResource to be a file of type i_xType with
    path i_rPath to manifest, with optional additional types i_pTypes */
static void
addFile(struct DocumentMetadataAccess_Impl & i_rImpl,
    uno::Reference<rdf::XURI> const& i_xType,
    ::rtl::OUString const & i_rPath,
    const uno::Sequence < uno::Reference< rdf::XURI > > * i_pTypes = 0)
{
    try {
        const uno::Reference<rdf::XURI> xURI( getURIForStream(
            i_rImpl, i_rPath) );

        i_rImpl.m_xManifest->addStatement(i_rImpl.m_xBaseURI.get(),
            getURI<rdf::URIs::PKG_HASPART>(i_rImpl.m_xContext),
            xURI.get());
        i_rImpl.m_xManifest->addStatement(xURI.get(),
            getURI<rdf::URIs::RDF_TYPE>(i_rImpl.m_xContext),
            i_xType.get());
        if (i_pTypes) {
            for (sal_Int32 i = 0; i < i_pTypes->getLength(); ++i) {
                i_rImpl.m_xManifest->addStatement(xURI.get(),
                    getURI<rdf::URIs::RDF_TYPE>(i_rImpl.m_xContext),
                    (*i_pTypes)[i].get());
            }
        }
    } catch (uno::RuntimeException &) {
        throw;
    } catch (uno::Exception & e) {
        throw lang::WrappedTargetRuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "addFile: exception")), /*this*/0, uno::makeAny(e));
    }
}

/** add content.xml or styles.xml to manifest */
static bool
addContentOrStylesFileImpl(struct DocumentMetadataAccess_Impl & i_rImpl,
    const ::rtl::OUString & i_rPath)
{
    uno::Reference<rdf::XURI> xType;
    if (isContentFile(i_rPath)) {
        xType.set(getURI<rdf::URIs::ODF_CONTENTFILE>(i_rImpl.m_xContext));
    } else if (isStylesFile(i_rPath)) {
        xType.set(getURI<rdf::URIs::ODF_STYLESFILE>(i_rImpl.m_xContext));
    } else {
        return false;
    }
    addFile(i_rImpl, xType.get(), i_rPath);
    return true;
}

/** add metadata file to manifest */
static void
addMetadataFileImpl(struct DocumentMetadataAccess_Impl & i_rImpl,
    const ::rtl::OUString & i_rPath,
    const uno::Sequence < uno::Reference< rdf::XURI > > & i_rTypes)
{
    addFile(i_rImpl,
            getURI<rdf::URIs::PKG_METADATAFILE>(i_rImpl.m_xContext),
            i_rPath, &i_rTypes);
}

/** remove a file from the manifest */
static void
removeFile(struct DocumentMetadataAccess_Impl & i_rImpl,
    uno::Reference<rdf::XURI> const& i_xPart)
{
    if (!i_xPart.is()) throw uno::RuntimeException();
    try {
        i_rImpl.m_xManifest->removeStatements(i_rImpl.m_xBaseURI.get(),
            getURI<rdf::URIs::PKG_HASPART>(i_rImpl.m_xContext),
            i_xPart.get());
        i_rImpl.m_xManifest->removeStatements(i_xPart.get(),
            getURI<rdf::URIs::RDF_TYPE>(i_rImpl.m_xContext), 0);
    } catch (uno::RuntimeException &) {
        throw;
    } catch (uno::Exception & e) {
        throw lang::WrappedTargetRuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("removeFile: exception")),
            0, uno::makeAny(e));
    }
}

static ::std::vector< uno::Reference< rdf::XURI > >
getAllParts(struct DocumentMetadataAccess_Impl & i_rImpl)
{
    ::std::vector< uno::Reference< rdf::XURI > > ret;
    try {
        const uno::Reference<container::XEnumeration> xEnum(
            i_rImpl.m_xManifest->getStatements( i_rImpl.m_xBaseURI.get(),
                getURI<rdf::URIs::PKG_HASPART>(i_rImpl.m_xContext), 0),
            uno::UNO_SET_THROW);
        while (xEnum->hasMoreElements()) {
            rdf::Statement stmt;
            if (!(xEnum->nextElement() >>= stmt)) {
                throw uno::RuntimeException();
            }
            const uno::Reference<rdf::XURI> xPart(stmt.Object,
                uno::UNO_QUERY);
            if (!xPart.is()) continue;
            ret.push_back(xPart);
        }
        return ret;
    } catch (uno::RuntimeException &) {
        throw;
    } catch (uno::Exception & e) {
        throw lang::WrappedTargetRuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("getAllParts: exception")),
            0, uno::makeAny(e));
    }
}

static bool
isPartOfType(struct DocumentMetadataAccess_Impl & i_rImpl,
    uno::Reference<rdf::XURI> const & i_xPart,
    uno::Reference<rdf::XURI> const & i_xType)
{
    if (!i_xPart.is() || !i_xType.is()) throw uno::RuntimeException();
    try {
        const uno::Reference<container::XEnumeration> xEnum(
            i_rImpl.m_xManifest->getStatements(i_xPart.get(),
                getURI<rdf::URIs::RDF_TYPE>(i_rImpl.m_xContext),
                i_xType.get()),
            uno::UNO_SET_THROW);
        return (xEnum->hasMoreElements());
    } catch (uno::RuntimeException &) {
        throw;
    } catch (uno::Exception & e) {
        throw lang::WrappedTargetRuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("isPartOfType: exception")),
            0, uno::makeAny(e));
    }
}


static ucb::InteractiveAugmentedIOException
mkException( ::rtl::OUString const & i_rMessage,
    ucb::IOErrorCode const i_ErrorCode,
    ::rtl::OUString const & i_rUri, ::rtl::OUString const & i_rResource)
{
    ucb::InteractiveAugmentedIOException iaioe;
    iaioe.Message = i_rMessage;
    iaioe.Classification = task::InteractionClassification_ERROR;
    iaioe.Code = i_ErrorCode;

    const beans::PropertyValue uriProp(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Uri")),
        -1, uno::makeAny(i_rUri), static_cast<beans::PropertyState>(0));
    const beans::PropertyValue rnProp(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ResourceName")),
        -1, uno::makeAny(i_rResource), static_cast<beans::PropertyState>(0));
    iaioe.Arguments = ::comphelper::makeSequence(
        uno::makeAny(uriProp), uno::makeAny(rnProp));
    return iaioe;
}

/** error handling policy.
    <p>If a handler is given, ask it how to proceed:
    <ul><li>(default:) cancel import, raise exception</li>
        <li>ignore the error and continue</li>
        <li>retry the action that led to the error</li></ul></p>
    N.B.: must not be called before DMA is fully initalized!
    @returns true iff caller should retry
 */
static bool
handleError( ucb::InteractiveAugmentedIOException const & i_rException,
    const uno::Reference<task::XInteractionHandler> & i_xHandler)
{
    if (!i_xHandler.is()) {
        throw lang::WrappedTargetException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DocumentMetadataAccess::loadMetadataFromStorage: exception")),
            /* *this*/ 0, uno::makeAny(i_rException));
    }

    ::rtl::Reference< ::comphelper::OInteractionRequest > pRequest(
        new ::comphelper::OInteractionRequest(uno::makeAny(i_rException)) );
    ::rtl::Reference< ::comphelper::OInteractionRetry > pRetry(
        new ::comphelper::OInteractionRetry );
    ::rtl::Reference< ::comphelper::OInteractionApprove > pApprove(
        new ::comphelper::OInteractionApprove );
    ::rtl::Reference< ::comphelper::OInteractionAbort > pAbort(
        new ::comphelper::OInteractionAbort );

    pRequest->addContinuation( pApprove.get() );
    pRequest->addContinuation( pAbort.get() );
    // actually call the handler
    i_xHandler->handle( pRequest.get() );
    if (pRetry->wasSelected()) {
        return true;
    } else if (pApprove->wasSelected()) {
        return false;
    } else {
        OSL_ENSURE(pAbort->wasSelected(), "no continuation selected?");
        throw lang::WrappedTargetException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DocumentMetadataAccess::loadMetadataFromStorage: exception")),
            /* *this*/ 0, uno::makeAny(i_rException));
    }
}

/** check if storage has content.xml/styles.xml;
    e.g. ODB files seem to only have content.xml */
static void
collectFilesFromStorage(uno::Reference<embed::XStorage> const& i_xStorage,
    ::rtl::OUString i_Path,
    std::set< ::rtl::OUString > & o_rFiles)
{
    static ::rtl::OUString content(RTL_CONSTASCII_USTRINGPARAM(s_content));
    static ::rtl::OUString styles(RTL_CONSTASCII_USTRINGPARAM(s_styles ));
    try {
        if (i_xStorage->hasByName(content) &&
            i_xStorage->isStreamElement(content))
        {
            o_rFiles.insert(i_Path + content);
        }
        if (i_xStorage->hasByName(styles) &&
            i_xStorage->isStreamElement(styles))
        {
            o_rFiles.insert(i_Path + styles);
        }
    } catch (uno::Exception &) {
        OSL_TRACE("collectFilesFromStorage: exception?");
    }
}

/** import a metadata file into repository */
static void
readStream(struct DocumentMetadataAccess_Impl & i_rImpl,
    uno::Reference< embed::XStorage > const & i_xStorage,
    ::rtl::OUString const & i_rPath,
    ::rtl::OUString const & i_rBaseURI)
{
    ::rtl::OUString dir;
    ::rtl::OUString rest;
    try {
        if (!splitPath(i_rPath, dir, rest)) throw uno::RuntimeException();
        if (dir.getLength() == 0) {
            if (i_xStorage->isStreamElement(i_rPath)) {
                const uno::Reference<io::XStream> xStream(
                    i_xStorage->openStreamElement(i_rPath,
                        embed::ElementModes::READ), uno::UNO_SET_THROW);
                const uno::Reference<io::XInputStream> xInStream(
                    xStream->getInputStream(), uno::UNO_SET_THROW );
                const uno::Reference<rdf::XURI> xBaseURI(
                    rdf::URI::create(i_rImpl.m_xContext, i_rBaseURI));
                const uno::Reference<rdf::XURI> xURI(
                    rdf::URI::createNS(i_rImpl.m_xContext,
                        i_rBaseURI, i_rPath));
                i_rImpl.m_xRepository->importGraph(rdf::FileFormat::RDF_XML,
                    xInStream, xURI, xBaseURI);
            } else {
                throw mkException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "readStream: is not a stream")),
                    ucb::IOErrorCode_NO_FILE, i_rBaseURI + i_rPath, i_rPath);
            }
        } else {
            if (i_xStorage->isStorageElement(dir)) {
                const uno::Reference<embed::XStorage> xDir(
                    i_xStorage->openStorageElement(dir,
                        embed::ElementModes::READ));
                const uno::Reference< beans::XPropertySet > xDirProps(xDir,
                    uno::UNO_QUERY_THROW);
                try {
                    ::rtl::OUString mimeType;
                    xDirProps->getPropertyValue(
                            ::comphelper::MediaDescriptor::PROP_MEDIATYPE() )
                        >>= mimeType;
                    if (mimeType.matchAsciiL(s_odfmime, sizeof(s_odfmime) - 1))
                    {
                        OSL_TRACE("readStream: "
                            "refusing to recurse into embedded document");
                        return;
                    }
                } catch (uno::Exception &) { }
                ::rtl::OUStringBuffer buf(i_rBaseURI);
                buf.append(dir).append(static_cast<sal_Unicode>('/'));
                readStream(i_rImpl, xDir, rest, buf.makeStringAndClear() );
            } else {
                throw mkException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "readStream: is not a directory")),
                    ucb::IOErrorCode_NO_DIRECTORY, i_rBaseURI + dir, dir);
            }
        }
    } catch (container::NoSuchElementException & e) {
        throw mkException(e.Message, ucb::IOErrorCode_NOT_EXISTING_PATH,
            i_rBaseURI + i_rPath, i_rPath);
    } catch (io::IOException & e) {
        throw mkException(e.Message, ucb::IOErrorCode_CANT_READ,
            i_rBaseURI + i_rPath, i_rPath);
    } catch (rdf::ParseException & e) {
        throw mkException(e.Message, ucb::IOErrorCode_WRONG_FORMAT,
            i_rBaseURI + i_rPath, i_rPath);
    }
}

/** import a metadata file into repository */
static void
importFile(struct DocumentMetadataAccess_Impl & i_rImpl,
    uno::Reference<embed::XStorage> const & i_xStorage,
    ::rtl::OUString const & i_rBaseURI,
    uno::Reference<task::XInteractionHandler> const & i_xHandler,
    ::rtl::OUString i_rPath)
{
retry:
    try {
        readStream(i_rImpl, i_xStorage, i_rPath, i_rBaseURI);
    } catch (ucb::InteractiveAugmentedIOException & e) {
        if (handleError(e, i_xHandler)) goto retry;
    } catch (uno::RuntimeException &) {
        throw;
    } catch (uno::Exception & e) {
        throw lang::WrappedTargetRuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("importFile: exception")),
            0, uno::makeAny(e));
    }
}

/** actually write a metadata file to the storage */
static void
exportStream(struct DocumentMetadataAccess_Impl & i_rImpl,
    uno::Reference< embed::XStorage > const & i_xStorage,
    uno::Reference<rdf::XURI> const & i_xGraphName,
    ::rtl::OUString const & i_rFileName,
    ::rtl::OUString const & i_rBaseURI)
{
    const uno::Reference<io::XStream> xStream(
        i_xStorage->openStreamElement(i_rFileName,
            embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE),
        uno::UNO_SET_THROW);
    const uno::Reference< beans::XPropertySet > xStreamProps(xStream,
        uno::UNO_QUERY);
    if (xStreamProps.is()) { // this is NOT supported in FileSystemStorage
        xStreamProps->setPropertyValue(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MediaType")),
            uno::makeAny(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s_rdfxml))));
    }
    const uno::Reference<io::XOutputStream> xOutStream(
        xStream->getOutputStream(), uno::UNO_SET_THROW );
    const uno::Reference<rdf::XURI> xBaseURI(
        rdf::URI::create(i_rImpl.m_xContext, i_rBaseURI));
    i_rImpl.m_xRepository->exportGraph(rdf::FileFormat::RDF_XML,
        xOutStream, i_xGraphName, xBaseURI);
}

/** write a metadata file to the storage */
static void
writeStream(struct DocumentMetadataAccess_Impl & i_rImpl,
    uno::Reference< embed::XStorage > const & i_xStorage,
    uno::Reference<rdf::XURI> const & i_xGraphName,
    ::rtl::OUString const & i_rPath,
    ::rtl::OUString const & i_rBaseURI)
{
    ::rtl::OUString dir;
    ::rtl::OUString rest;
    if (!splitPath(i_rPath, dir, rest)) throw uno::RuntimeException();
    try {
        if (dir.getLength() == 0) {
            exportStream(i_rImpl, i_xStorage, i_xGraphName, i_rPath,
                i_rBaseURI);
        } else {
            const uno::Reference<embed::XStorage> xDir(
                i_xStorage->openStorageElement(dir,
                    embed::ElementModes::WRITE));
            const uno::Reference< beans::XPropertySet > xDirProps(xDir,
                uno::UNO_QUERY_THROW);
            try {
                ::rtl::OUString mimeType;
                xDirProps->getPropertyValue(
                        ::comphelper::MediaDescriptor::PROP_MEDIATYPE() )
                    >>= mimeType;
                if (mimeType.matchAsciiL(s_odfmime, sizeof(s_odfmime) - 1)) {
                    OSL_TRACE("writeStream: "
                        "refusing to recurse into embedded document");
                    return;
                }
            } catch (uno::Exception &) { }
            ::rtl::OUStringBuffer buf(i_rBaseURI);
            buf.append(dir).append(static_cast<sal_Unicode>('/'));
            writeStream(i_rImpl, xDir, i_xGraphName, rest,
                buf.makeStringAndClear());
        }
        const uno::Reference<embed::XTransactedObject> xTransaction(
            i_xStorage, uno::UNO_QUERY);
        if (xTransaction.is()) {
            xTransaction->commit();
        }
    } catch (uno::RuntimeException &) {
        throw;
    } catch (io::IOException &) {
        throw;
    }
}

static void
initLoading(struct DocumentMetadataAccess_Impl & i_rImpl,
    const uno::Reference< embed::XStorage > & i_xStorage,
    const uno::Reference<rdf::XURI> & i_xBaseURI,
    const uno::Reference<task::XInteractionHandler> & i_xHandler)
{
retry:
    // clear old data
    i_rImpl.m_xManifest.clear();
    // init BaseURI
    i_rImpl.m_xBaseURI = i_xBaseURI;

    // create repository
    i_rImpl.m_xRepository.clear();
    i_rImpl.m_xRepository.set(rdf::Repository::create(i_rImpl.m_xContext),
            uno::UNO_SET_THROW);

    const ::rtl::OUString manifest (
            ::rtl::OUString::createFromAscii(s_manifest));
    const ::rtl::OUString baseURI( i_xBaseURI->getStringValue() );
    // try to delay raising errors until after initialization is done
    uno::Any rterr;
    ucb::InteractiveAugmentedIOException iaioe;
    bool err(false);

    const uno::Reference <rdf::XURI> xManifest(
        getURIForStream(i_rImpl, manifest));
    try {
        readStream(i_rImpl, i_xStorage, manifest, baseURI);
    } catch (ucb::InteractiveAugmentedIOException & e) {
        // no manifest.rdf: this is not an error in ODF < 1.2
        if (!(ucb::IOErrorCode_NOT_EXISTING_PATH == e.Code)) {
            iaioe = e;
            err = true;
        }
    } catch (uno::Exception & e) {
        rterr <<= e;
    }

    // init manifest graph
    const uno::Reference<rdf::XNamedGraph> xManifestGraph(
        i_rImpl.m_xRepository->getGraph(xManifest));
    i_rImpl.m_xManifest.set(xManifestGraph.is() ? xManifestGraph :
        i_rImpl.m_xRepository->createGraph(xManifest), uno::UNO_SET_THROW);
    const uno::Reference<container::XEnumeration> xEnum(
        i_rImpl.m_xManifest->getStatements(0,
            getURI<rdf::URIs::RDF_TYPE>(i_rImpl.m_xContext),
            getURI<rdf::URIs::PKG_DOCUMENT>(i_rImpl.m_xContext).get()));

    // document statement
    i_rImpl.m_xManifest->addStatement(i_rImpl.m_xBaseURI.get(),
        getURI<rdf::URIs::RDF_TYPE>(i_rImpl.m_xContext),
        getURI<rdf::URIs::PKG_DOCUMENT>(i_rImpl.m_xContext).get());

    OSL_ENSURE(i_rImpl.m_xBaseURI.is(), "base URI is null");
    OSL_ENSURE(i_rImpl.m_xRepository.is(), "repository is null");
    OSL_ENSURE(i_rImpl.m_xManifest.is(), "manifest is null");

    if (rterr.hasValue()) {
        throw lang::WrappedTargetRuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "DocumentMetadataAccess::loadMetadataFromStorage: "
                "exception")), 0, rterr);
    }

    if (err) {
        if (handleError(iaioe, i_xHandler)) goto retry;
    }
}

/** init Impl struct */
static void init(struct DocumentMetadataAccess_Impl & i_rImpl)
{
    try {

        i_rImpl.m_xManifest.set(i_rImpl.m_xRepository->createGraph(
            getURIForStream(i_rImpl,
                ::rtl::OUString::createFromAscii(s_manifest))),
            uno::UNO_SET_THROW);

        // insert the document statement
        i_rImpl.m_xManifest->addStatement(i_rImpl.m_xBaseURI.get(),
            getURI<rdf::URIs::RDF_TYPE>(i_rImpl.m_xContext),
            getURI<rdf::URIs::PKG_DOCUMENT>(i_rImpl.m_xContext).get());
    } catch (uno::Exception & e) {
        throw lang::WrappedTargetRuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("init: unexpected exception")), 0,
            uno::makeAny(e));
    }

    // add top-level content files
    if (!addContentOrStylesFileImpl(i_rImpl,
            ::rtl::OUString::createFromAscii(s_content))) {
        throw uno::RuntimeException();
    }
    if (!addContentOrStylesFileImpl(i_rImpl,
            ::rtl::OUString::createFromAscii(s_styles))) {
        throw uno::RuntimeException();
    }
}



DocumentMetadataAccess::DocumentMetadataAccess(
        uno::Reference< uno::XComponentContext > const & i_xContext,
        const IXmlIdRegistrySupplier & i_rRegistrySupplier)
    : m_pImpl(new DocumentMetadataAccess_Impl(i_xContext, i_rRegistrySupplier))
{
    // no initalization: must call loadFrom...
}

DocumentMetadataAccess::DocumentMetadataAccess(
        uno::Reference< uno::XComponentContext > const & i_xContext,
        const IXmlIdRegistrySupplier & i_rRegistrySupplier,
        ::rtl::OUString const & i_rURI)
    : m_pImpl(new DocumentMetadataAccess_Impl(i_xContext, i_rRegistrySupplier))
{
    OSL_ENSURE(i_rURI.getLength(), "DMA::DMA: no URI given!");
    OSL_ENSURE(i_rURI.endsWithAsciiL("/", 1), "DMA::DMA: URI without / given!");
    if (!i_rURI.endsWithAsciiL("/", 1)) throw uno::RuntimeException();
    m_pImpl->m_xBaseURI.set(rdf::URI::create(m_pImpl->m_xContext, i_rURI));
    m_pImpl->m_xRepository.set(rdf::Repository::create(m_pImpl->m_xContext),
            uno::UNO_SET_THROW);

    // init repository
    init(*m_pImpl);

    OSL_ENSURE(m_pImpl->m_xBaseURI.is(), "base URI is null");
    OSL_ENSURE(m_pImpl->m_xRepository.is(), "repository is null");
    OSL_ENSURE(m_pImpl->m_xManifest.is(), "manifest is null");
}

DocumentMetadataAccess::~DocumentMetadataAccess()
{
}

// ::com::sun::star::rdf::XRepositorySupplier:
uno::Reference< rdf::XRepository > SAL_CALL
DocumentMetadataAccess::getRDFRepository() throw (uno::RuntimeException)
{
    OSL_ENSURE(m_pImpl->m_xRepository.is(), "repository not initialized");
    return m_pImpl->m_xRepository;
}

// ::com::sun::star::rdf::XNode:
::rtl::OUString SAL_CALL
DocumentMetadataAccess::getStringValue() throw (uno::RuntimeException)
{
    return m_pImpl->m_xBaseURI->getStringValue();
}

// ::com::sun::star::rdf::XURI:
::rtl::OUString SAL_CALL
DocumentMetadataAccess::getNamespace() throw (uno::RuntimeException)
{
    return m_pImpl->m_xBaseURI->getNamespace();
}

::rtl::OUString SAL_CALL
DocumentMetadataAccess::getLocalName() throw (uno::RuntimeException)
{
    return m_pImpl->m_xBaseURI->getLocalName();
}

// ::com::sun::star::rdf::XDocumentMetadataAccess:
uno::Reference< rdf::XMetadatable > SAL_CALL
DocumentMetadataAccess::getElementByMetadataReference(
    const ::com::sun::star::beans::StringPair & i_rReference)
throw (uno::RuntimeException)
{
    const IXmlIdRegistry * pReg(
        m_pImpl->m_rXmlIdRegistrySupplier.GetXmlIdRegistry() );
    if (!pReg) {
        throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DocumentMetadataAccess::getElementByXmlId: no registry")), *this);
    }
    return pReg->GetElementByMetadataReference(i_rReference);
}

uno::Reference< rdf::XMetadatable > SAL_CALL
DocumentMetadataAccess::getElementByURI(
    const uno::Reference< rdf::XURI > & i_xURI )
throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    if (!i_xURI.is()) {
        throw lang::IllegalArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DocumentMetadataAccess::getElementByURI: URI is null")), *this, 0);
    }

    const ::rtl::OUString baseURI( m_pImpl->m_xBaseURI->getStringValue() );
    const ::rtl::OUString name( i_xURI->getStringValue() );
    if (!name.match(baseURI)) {
        return 0;
    }
    const ::rtl::OUString relName( name.copy(baseURI.getLength()) );
    ::rtl::OUString path;
    ::rtl::OUString idref;
    if (!splitXmlId(relName, path, idref)) {
        return 0;
    }

    return getElementByMetadataReference( beans::StringPair(path, idref) );
}


uno::Sequence< uno::Reference< rdf::XURI > > SAL_CALL
DocumentMetadataAccess::getMetadataGraphsWithType(
    const uno::Reference<rdf::XURI> & i_xType)
throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    if (!i_xType.is()) {
        throw lang::IllegalArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DocumentMetadataAccess::getMetadataGraphsWithType: "
            "type is null")), *this, 0);
    }

    ::comphelper::SequenceAsVector< uno::Reference< rdf::XURI > > ret;
    const ::std::vector< uno::Reference< rdf::XURI > > parts(
        getAllParts(*m_pImpl) );
    ::std::remove_copy_if(parts.begin(), parts.end(),
        ::std::back_inserter(ret),
        ::boost::bind(
            ::std::logical_not<bool>(),
            ::boost::bind(&isPartOfType, ::boost::ref(*m_pImpl), _1, i_xType) ));
    return ret.getAsConstList();
}

uno::Reference<rdf::XURI> SAL_CALL
DocumentMetadataAccess::addMetadataFile(const ::rtl::OUString & i_rFileName,
    const uno::Sequence < uno::Reference< rdf::XURI > > & i_rTypes)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    container::ElementExistException)
{
    if (!isFileNameValid(i_rFileName)) {
        throw lang::IllegalArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DocumentMetadataAccess::addMetadataFile: invalid FileName")),
            *this, 0);
    }
    if (isReservedFile(i_rFileName)) {
        throw lang::IllegalArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DocumentMetadataAccess::addMetadataFile:"
            "invalid FileName: reserved")), *this, 0);
    }
    for (sal_Int32 i = 0; i < i_rTypes.getLength(); ++i) {
        if (!i_rTypes[i].is()) {
            throw lang::IllegalArgumentException(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "DocumentMetadataAccess::addMetadataFile: "
                    "null type")), *this, 2);
        }
    }

    const uno::Reference<rdf::XURI> xGraphName(
        getURIForStream(*m_pImpl, i_rFileName) );

    try {
        m_pImpl->m_xRepository->createGraph(xGraphName);
    } catch (rdf::RepositoryException & e) {
        throw lang::WrappedTargetRuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "DocumentMetadataAccess::addMetadataFile: exception")),
            *this, uno::makeAny(e));
        // note: all other exceptions are propagated
    }

    addMetadataFileImpl(*m_pImpl, i_rFileName, i_rTypes);
    return xGraphName;
}

uno::Reference<rdf::XURI> SAL_CALL
DocumentMetadataAccess::importMetadataFile(::sal_Int16 i_Format,
    const uno::Reference< io::XInputStream > & i_xInStream,
    const ::rtl::OUString & i_rFileName,
    const uno::Reference< rdf::XURI > & i_xBaseURI,
    const uno::Sequence < uno::Reference< rdf::XURI > > & i_rTypes)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    datatransfer::UnsupportedFlavorException,
    container::ElementExistException, rdf::ParseException, io::IOException)
{
    if (!isFileNameValid(i_rFileName)) {
        throw lang::IllegalArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DocumentMetadataAccess::importMetadataFile: invalid FileName")),
            *this, 0);
    }
    if (isReservedFile(i_rFileName)) {
        throw lang::IllegalArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DocumentMetadataAccess::importMetadataFile:"
            "invalid FileName: reserved")), *this, 0);
    }
    for (sal_Int32 i = 0; i < i_rTypes.getLength(); ++i) {
        if (!i_rTypes[i].is()) {
            throw lang::IllegalArgumentException(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "DocumentMetadataAccess::importMetadataFile: null type")),
                *this, 5);
        }
    }

    const uno::Reference<rdf::XURI> xGraphName(
        getURIForStream(*m_pImpl, i_rFileName) );

    try {
        m_pImpl->m_xRepository->importGraph(
            i_Format, i_xInStream, xGraphName, i_xBaseURI);
    } catch (rdf::RepositoryException & e) {
        throw lang::WrappedTargetRuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "DocumentMetadataAccess::importMetadataFile: "
                "RepositoryException")), *this, uno::makeAny(e));
        // note: all other exceptions are propagated
    }

    // add to manifest
    addMetadataFileImpl(*m_pImpl, i_rFileName, i_rTypes);
    return xGraphName;
}

void SAL_CALL
DocumentMetadataAccess::removeMetadataFile(
    const uno::Reference< rdf::XURI > & i_xGraphName)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    container::NoSuchElementException)
{
    try {
        m_pImpl->m_xRepository->destroyGraph(i_xGraphName);
    } catch (rdf::RepositoryException & e) {
        throw lang::WrappedTargetRuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "DocumentMetadataAccess::removeMetadataFile: "
                "RepositoryException")), *this, uno::makeAny(e));
        // note: all other exceptions are propagated
    }

    // remove file from manifest
    removeFile(*m_pImpl, i_xGraphName.get());
}

void SAL_CALL
DocumentMetadataAccess::addContentOrStylesFile(
    const ::rtl::OUString & i_rFileName)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    container::ElementExistException)
{
    if (!isFileNameValid(i_rFileName)) {
        throw lang::IllegalArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DocumentMetadataAccess::addContentOrStylesFile: "
            "invalid FileName")), *this, 0);
    }

    if (!addContentOrStylesFileImpl(*m_pImpl, i_rFileName)) {
        throw lang::IllegalArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DocumentMetadataAccess::addContentOrStylesFile: "
            "invalid FileName: must end with content.xml or styles.xml")),
            *this, 0);
    }
}

void SAL_CALL
DocumentMetadataAccess::removeContentOrStylesFile(
    const ::rtl::OUString & i_rFileName)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    container::NoSuchElementException)
{
    if (!isFileNameValid(i_rFileName)) {
        throw lang::IllegalArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DocumentMetadataAccess::removeContentOrStylesFile: "
            "invalid FileName")), *this, 0);
    }

    try {
        const uno::Reference<rdf::XURI> xPart(
            getURIForStream(*m_pImpl, i_rFileName) );
        const uno::Reference<container::XEnumeration> xEnum(
            m_pImpl->m_xManifest->getStatements( m_pImpl->m_xBaseURI.get(),
                getURI<rdf::URIs::PKG_HASPART>(m_pImpl->m_xContext),
                xPart.get()),
            uno::UNO_SET_THROW);
        if (!xEnum->hasMoreElements()) {
            throw container::NoSuchElementException(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "DocumentMetadataAccess::removeContentOrStylesFile: "
                    "cannot find stream in manifest graph: ")) + i_rFileName,
                *this);
        }

        // remove file from manifest
        removeFile(*m_pImpl, xPart);

    } catch (uno::RuntimeException &) {
        throw;
    } catch (uno::Exception & e) {
        throw lang::WrappedTargetRuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "DocumentMetadataAccess::removeContentOrStylesFile: exception")),
            *this, uno::makeAny(e));
    }
}

void SAL_CALL DocumentMetadataAccess::loadMetadataFromStorage(
    const uno::Reference< embed::XStorage > & i_xStorage,
    const uno::Reference<rdf::XURI> & i_xBaseURI,
    const uno::Reference<task::XInteractionHandler> & i_xHandler)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    lang::WrappedTargetException)
{
    if (!i_xStorage.is()) {
        throw lang::IllegalArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DocumentMetadataAccess::loadMetadataFromStorage: "
            "storage is null")), *this, 0);
    }
    if (!i_xBaseURI.is()) {
        throw lang::IllegalArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DocumentMetadataAccess::loadMetadataFromStorage: "
            "base URI is null")), *this, 1);
    }
    const ::rtl::OUString baseURI( i_xBaseURI->getStringValue());
    if (baseURI.indexOf('#') >= 0) {
        throw lang::IllegalArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DocumentMetadataAccess::loadMetadataFromStorage: "
            "base URI not absolute")), *this, 1);
    }
    if (!baseURI.getLength() || !baseURI.endsWithAsciiL("/", 1)) {
        throw lang::IllegalArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DocumentMetadataAccess::loadMetadataFromStorage: "
            "base URI does not end with slash")), *this, 1);
    }

    initLoading(*m_pImpl, i_xStorage, i_xBaseURI, i_xHandler);

    std::set< ::rtl::OUString > StgFiles;
    collectFilesFromStorage(i_xStorage,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")), StgFiles);

    std::vector< ::rtl::OUString > MfstMetadataFiles;

    try {
        const ::std::vector< uno::Reference< rdf::XURI > > parts(
            getAllParts(*m_pImpl) );
        const uno::Reference<rdf::XURI> xContentFile(
            getURI<rdf::URIs::ODF_CONTENTFILE>(m_pImpl->m_xContext));
        const uno::Reference<rdf::XURI> xStylesFile(
            getURI<rdf::URIs::ODF_STYLESFILE>(m_pImpl->m_xContext));
        const uno::Reference<rdf::XURI> xMetadataFile(
            getURI<rdf::URIs::PKG_METADATAFILE>(m_pImpl->m_xContext));
        const sal_Int32 len( baseURI.getLength() );
        const ::rtl::OUString manifest (
                ::rtl::OUString::createFromAscii(s_manifest));
        for (::std::vector< uno::Reference< rdf::XURI > >::const_iterator it
                = parts.begin();
                it != parts.end(); ++it) {
            const ::rtl::OUString name((*it)->getStringValue());
            if (!name.match(baseURI)) {
                OSL_TRACE("loadMetadataFromStorage: graph not in document: %s",
                    ::rtl::OUStringToOString(name, RTL_TEXTENCODING_UTF8)
                    .getStr());
                continue;
            }
            const ::rtl::OUString relName( name.copy(len) );
            if (relName == manifest) {
                OSL_TRACE("loadMetadataFromStorage: "
                    "found ourselves a recursive manifest!");
                continue;
            }
            // remove found items from StgFiles
            StgFiles.erase(relName);
            if (isContentFile(relName)) {
                if (!isPartOfType(*m_pImpl, *it, xContentFile)) {
                    const uno::Reference <rdf::XURI> xName(
                        getURIForStream(*m_pImpl, relName) );
                    // add missing type statement
                    m_pImpl->m_xManifest->addStatement(xName.get(),
                        getURI<rdf::URIs::RDF_TYPE>(m_pImpl->m_xContext),
                        xContentFile.get());
                }
            } else if (isStylesFile(relName)) {
                if (!isPartOfType(*m_pImpl, *it, xStylesFile)) {
                    const uno::Reference <rdf::XURI> xName(
                        getURIForStream(*m_pImpl, relName) );
                    // add missing type statement
                    m_pImpl->m_xManifest->addStatement(xName.get(),
                        getURI<rdf::URIs::RDF_TYPE>(m_pImpl->m_xContext),
                        xStylesFile.get());
                }
            } else if (isReservedFile(relName)) {
                OSL_TRACE("loadMetadataFromStorage: "
                    "reserved file name in manifest");
            } else {
                if (isPartOfType(*m_pImpl, *it, xMetadataFile)) {
                    MfstMetadataFiles.push_back(relName);
                }
                // do not add statement for MetadataFile; it could be
                // something else! just ignore it...
            }
        }
    } catch (uno::RuntimeException &) {
        throw;
    } catch (uno::Exception & e) {
        throw lang::WrappedTargetRuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "DocumentMetadataAccess::loadMetadataFromStorage: "
                "exception")), *this, uno::makeAny(e));
    }

    std::for_each(StgFiles.begin(), StgFiles.end(),
        boost::bind(addContentOrStylesFileImpl, boost::ref(*m_pImpl), _1));

    std::for_each(MfstMetadataFiles.begin(), MfstMetadataFiles.end(),
        boost::bind(importFile, boost::ref(*m_pImpl),
            i_xStorage, baseURI, i_xHandler, _1));
}

void SAL_CALL DocumentMetadataAccess::storeMetadataToStorage(
    const uno::Reference< embed::XStorage > & i_xStorage)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    lang::WrappedTargetException)
{
    if (!i_xStorage.is()) {
        throw lang::IllegalArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DocumentMetadataAccess::storeMetadataToStorage: "
            "storage is null")), *this, 0);
    }

    // export manifest
    const ::rtl::OUString manifest (
            ::rtl::OUString::createFromAscii(s_manifest));
    const uno::Reference <rdf::XURI> xManifest(
        getURIForStream(*m_pImpl, manifest) );
    const ::rtl::OUString baseURI( m_pImpl->m_xBaseURI->getStringValue() );
    try {
        writeStream(*m_pImpl, i_xStorage, xManifest, manifest, baseURI);
    } catch (uno::RuntimeException &) {
        throw;
    } catch (io::IOException & e) {
        throw lang::WrappedTargetException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "storeMetadataToStorage: IO exception")), *this, uno::makeAny(e));
    } catch (uno::Exception & e) {
        throw lang::WrappedTargetRuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "storeMetadataToStorage: exception")), *this, uno::makeAny(e));
    }

    // export metadata streams
    try {
        const uno::Sequence<uno::Reference<rdf::XURI> > graphs(
            m_pImpl->m_xRepository->getGraphNames());
        const sal_Int32 len( baseURI.getLength() );
        for (sal_Int32 i = 0; i < graphs.getLength(); ++i) {
            const uno::Reference<rdf::XURI> xName(graphs[i]);
            const ::rtl::OUString name(xName->getStringValue());
            if (!name.match(baseURI)) {
                OSL_TRACE("storeMetadataToStorage: graph not in document: %s",
                    ::rtl::OUStringToOString(name, RTL_TEXTENCODING_UTF8)
                    .getStr());
                continue;
            }
            const ::rtl::OUString relName( name.copy(len) );
            if (relName == manifest) {
                continue;
            }
            if (!isFileNameValid(relName) || isReservedFile(relName)) {
                OSL_TRACE("storeMetadataToStorage: invalid file name: %s",
                    ::rtl::OUStringToOString(relName, RTL_TEXTENCODING_UTF8)
                    .getStr());
                continue;
            }
            try {
                writeStream(*m_pImpl, i_xStorage, xName, relName, baseURI);
            } catch (uno::RuntimeException &) {
                throw;
            } catch (io::IOException & e) {
                throw lang::WrappedTargetException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "storeMetadataToStorage: IO exception")),
                    *this, uno::makeAny(e));
            } catch (uno::Exception & e) {
                throw lang::WrappedTargetRuntimeException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "storeMetadataToStorage: exception")),
                    *this, uno::makeAny(e));
            }
        }
    } catch (rdf::RepositoryException & e) {
        throw lang::WrappedTargetRuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "storeMetadataToStorage: exception")), *this, uno::makeAny(e));
    }
}

void SAL_CALL
DocumentMetadataAccess::loadMetadataFromMedium(
    const uno::Sequence< beans::PropertyValue > & i_rMedium)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    lang::WrappedTargetException)
{
    uno::Reference<io::XInputStream> xIn;
    ::comphelper::MediaDescriptor md(i_rMedium);
    ::rtl::OUString URL;
    md[ ::comphelper::MediaDescriptor::PROP_URL() ] >>= URL;
    ::rtl::OUString BaseURL;
    md[ ::comphelper::MediaDescriptor::PROP_DOCUMENTBASEURL() ] >>= BaseURL;
    if (md.addInputStream()) {
        md[ ::comphelper::MediaDescriptor::PROP_INPUTSTREAM() ] >>= xIn;
    }
    if (!xIn.is() && (URL.getLength() == 0)) {
        throw lang::IllegalArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DocumentMetadataAccess::loadMetadataFromMedium: "
            "inalid medium: no URL, no input stream")), *this, 0);
    }
    uno::Reference<embed::XStorage> xStorage;
    try {
        const uno::Reference<lang::XMultiServiceFactory> xMsf (
            m_pImpl->m_xContext->getServiceManager(), uno::UNO_QUERY_THROW);
        if (xIn.is()) {
            xStorage = ::comphelper::OStorageHelper::GetStorageFromInputStream(
                            xIn, xMsf);
        } else { // fallback to url
            xStorage = ::comphelper::OStorageHelper::GetStorageFromURL2(
                            URL, embed::ElementModes::READ, xMsf);
        }
    } catch (uno::RuntimeException &) {
        throw;
    } catch (io::IOException &) {
        throw;
    } catch (uno::Exception & e) {
        throw lang::WrappedTargetException(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "DocumentMetadataAccess::loadMetadataFromMedium: "
                    "exception")), *this, uno::makeAny(e));
    }
    if (!xStorage.is()) {
        throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DocumentMetadataAccess::loadMetadataFromMedium: "
            "cannot get Storage")), *this);
    }
    uno::Reference<rdf::XURI> xBaseURI;
    try {
        xBaseURI = createBaseURI(m_pImpl->m_xContext, xStorage, BaseURL);
    } catch (uno::Exception &) {
        // fall back to URL
        try {
            xBaseURI = createBaseURI(m_pImpl->m_xContext, xStorage, URL);
        } catch (uno::Exception &) {
            OSL_FAIL("cannot create base URI");
        }
    }
    uno::Reference<task::XInteractionHandler> xIH;
    md[ ::comphelper::MediaDescriptor::PROP_INTERACTIONHANDLER() ] >>= xIH;
    loadMetadataFromStorage(xStorage, xBaseURI, xIH);
}

void SAL_CALL
DocumentMetadataAccess::storeMetadataToMedium(
    const uno::Sequence< beans::PropertyValue > & i_rMedium)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    lang::WrappedTargetException)
{
    ::comphelper::MediaDescriptor md(i_rMedium);
    ::rtl::OUString URL;
    md[ ::comphelper::MediaDescriptor::PROP_URL() ] >>= URL;
    if (URL.getLength() == 0) {
        throw lang::IllegalArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DocumentMetadataAccess::storeMetadataToMedium: "
            "invalid medium: no URL")), *this, 0);
    }

    SfxMedium aMedium(i_rMedium);
    uno::Reference<embed::XStorage> xStorage(aMedium.GetOutputStorage());

    bool sfx(false);
    if (xStorage.is()) {
        sfx = true;
    } else {
        const uno::Reference<lang::XMultiServiceFactory> xMsf (
            m_pImpl->m_xContext->getServiceManager(), uno::UNO_QUERY_THROW);
        xStorage = ::comphelper::OStorageHelper::GetStorageFromURL2(
                        URL, embed::ElementModes::WRITE, xMsf);
    }

    if (!xStorage.is()) {
        throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DocumentMetadataAccess::storeMetadataToMedium: "
            "cannot get Storage")), *this);
    }
    // set MIME type of the storage
    ::comphelper::MediaDescriptor::const_iterator iter
        = md.find(::comphelper::MediaDescriptor::PROP_MEDIATYPE());
    if (iter != md.end()) {
        uno::Reference< beans::XPropertySet > xProps(xStorage,
            uno::UNO_QUERY_THROW);
        try {
            // this is NOT supported in FileSystemStorage
            xProps->setPropertyValue(
                ::comphelper::MediaDescriptor::PROP_MEDIATYPE(),
                iter->second);
        } catch (uno::Exception &) { }
    }
    storeMetadataToStorage(xStorage);

    if (sfx) {
        const sal_Bool bOk = aMedium.Commit();
        aMedium.Close();
        if ( !bOk ) {
            sal_uInt32 nError = aMedium.GetError();
            if ( nError == ERRCODE_NONE ) {
                nError = ERRCODE_IO_GENERAL;
            }
            task::ErrorCodeIOException ex( ::rtl::OUString(),
                    uno::Reference< uno::XInterface >(), nError);
            throw lang::WrappedTargetException(::rtl::OUString(), *this,
                    uno::makeAny(ex));
        }
    }
}

} // namespace sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
