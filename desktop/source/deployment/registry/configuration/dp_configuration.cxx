/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_configuration.cxx,v $
 * $Revision: 1.15 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "dp_configuration.hrc"
#include "dp_backend.h"
#include "dp_persmap.h"
#include "dp_ucb.h"
#include "dp_xml.h"
#include "rtl/string.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/uri.hxx"
#include "rtl/memory.h"
#include "osl/file.hxx"
#include "ucbhelper/content.hxx"
#include "comphelper/anytostring.hxx"
#include "comphelper/servicedecl.hxx"
#include "xmlscript/xml_helper.hxx"
#include "svl/inettype.hxx"
#include "com/sun/star/ucb/NameClash.hpp"
#include "com/sun/star/io/XActiveDataSink.hpp"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/util/XRefreshable.hpp"
#include "com/sun/star/configuration/backend/XLayerImporter.hpp"
#include <memory>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_registry {
namespace backend {
namespace configuration {
namespace {

//==============================================================================
class BackendImpl : public ::dp_registry::backend::PackageRegistryBackend
{
    class PackageImpl : public ::dp_registry::backend::Package
    {
        BackendImpl * getMyBackend() const ;

        const bool m_isSchema;

        // Package
        virtual beans::Optional< beans::Ambiguous<sal_Bool> > isRegistered_(
            ::osl::ResettableMutexGuard & guard,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv );
        virtual void processPackage_(
            ::osl::ResettableMutexGuard & guard,
            bool registerPackage,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv );

    public:
        inline PackageImpl(
            ::rtl::Reference<PackageRegistryBackend> const & myBackend,
            OUString const & url, OUString const & name,
            Reference<deployment::XPackageTypeInfo> const & xPackageType,
            bool isSchema )
            : Package( myBackend, url, name, name /* display-name */,
                       xPackageType ),
              m_isSchema( isSchema )
            {}
    };
    friend class PackageImpl;

    Reference< ::com::sun::star::configuration::backend::XLayerImporter >
    m_xMergeImporter;
    OUString m_configLayer;

    // PackageRegistryBackend
    virtual Reference<deployment::XPackage> bindPackage_(
        OUString const & url, OUString const & mediaType,
        Reference<XCommandEnvironment> const & xCmdEnv );

    void xcu_merge_in( OUString const & url,
                       Reference< XCommandEnvironment > const & xCmdEnv );
    void xcs_merge_in( OUString const & url,
                       Reference< XCommandEnvironment > const & xCmdEnv );
    ::std::auto_ptr<PersistentMap> m_registeredPackages;
    OUString const & getConfigLayer();
    Reference<util::XRefreshable> m_defaultProvider;

    const Reference<deployment::XPackageTypeInfo> m_xConfDataTypeInfo;
    const Reference<deployment::XPackageTypeInfo> m_xConfSchemaTypeInfo;
    Sequence< Reference<deployment::XPackageTypeInfo> > m_typeInfos;

public:
    BackendImpl( Sequence<Any> const & args,
                 Reference<XComponentContext> const & xComponentContext );

    // XPackageRegistry
    virtual Sequence< Reference<deployment::XPackageTypeInfo> > SAL_CALL
    getSupportedPackageTypes() throw (RuntimeException);
};

//______________________________________________________________________________
BackendImpl::BackendImpl(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xComponentContext )
    : PackageRegistryBackend( args, xComponentContext ),
      m_defaultProvider( xComponentContext->getValueByName(
                             OUSTR("/singletons/com.sun.star."
                                   "configuration.theDefaultProvider") ),
                         UNO_QUERY ),
      m_xConfDataTypeInfo( new Package::TypeInfo(
                               OUSTR("application/"
                                     "vnd.sun.star.configuration-data"),
                               OUSTR("*.xcu"),
                               getResourceString(RID_STR_CONF_DATA),
                               RID_IMG_CONF_XML, RID_IMG_CONF_XML_HC ) ),
      m_xConfSchemaTypeInfo( new Package::TypeInfo(
                                 OUSTR("application/"
                                       "vnd.sun.star.configuration-schema"),
                                 OUSTR("*.xcs"),
                                 getResourceString(RID_STR_CONF_SCHEMA),
                                 RID_IMG_CONF_XML, RID_IMG_CONF_XML_HC ) ),
      m_typeInfos( 2 )
{
    m_typeInfos[ 0 ] = m_xConfDataTypeInfo;
    m_typeInfos[ 1 ] = m_xConfSchemaTypeInfo;

    OSL_ASSERT( m_defaultProvider.is() );

    if (transientMode()) {
        m_registeredPackages.reset( new PersistentMap );
    }
    else {
        m_registeredPackages.reset(
            new PersistentMap(
                makeURL( getCachePath(), OUSTR("registered_packages.db") ),
                m_readOnly ) );
        if (! m_readOnly)
            create_folder( 0, getConfigLayer(),
                           Reference<XCommandEnvironment>() );
    }
}

// XPackageRegistry
//______________________________________________________________________________
Sequence< Reference<deployment::XPackageTypeInfo> >
BackendImpl::getSupportedPackageTypes() throw (RuntimeException)
{
    return m_typeInfos;
}

// PackageRegistryBackend
//______________________________________________________________________________
Reference<deployment::XPackage> BackendImpl::bindPackage_(
    OUString const & url, OUString const & mediaType_,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    OUString mediaType( mediaType_ );
    if (mediaType.getLength() == 0)
    {
        // detect media-type:
        ::ucbhelper::Content ucbContent;
        if (create_ucb_content( &ucbContent, url, xCmdEnv ))
        {
            const OUString title( ucbContent.getPropertyValue(
                                      StrTitle::get() ).get<OUString>() );
            if (title.endsWithIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM(".xcu") )) {
                mediaType = OUSTR("application/"
                                  "vnd.sun.star.configuration-data");
            }
            if (title.endsWithIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM(".xcs") )) {
                mediaType = OUSTR("application/"
                                  "vnd.sun.star.configuration-schema");
            }
        }
        if (mediaType.getLength() == 0)
            throw lang::IllegalArgumentException(
                StrCannotDetectMediaType::get() + url,
                static_cast<OWeakObject *>(this), static_cast<sal_Int16>(-1) );
    }

    String type, subType;
    INetContentTypeParameterList params;
    if (INetContentTypes::parse( mediaType, type, subType, &params ))
    {
        if (type.EqualsIgnoreCaseAscii("application"))
        {
            ::ucbhelper::Content ucbContent( url, xCmdEnv );
            if (subType.EqualsIgnoreCaseAscii(
                    "vnd.sun.star.configuration-data")) {
                return new PackageImpl(
                    this, url, ucbContent.getPropertyValue(
                        StrTitle::get() ).get<OUString>(),
                    m_xConfDataTypeInfo, false /* data file */ );
            }
            else if (subType.EqualsIgnoreCaseAscii(
                         "vnd.sun.star.configuration-schema")) {
                return new PackageImpl(
                    this, url, ucbContent.getPropertyValue(
                        StrTitle::get() ).get<OUString>(),
                    m_xConfSchemaTypeInfo, true /* schema file */ );
            }
        }
    }
    throw lang::IllegalArgumentException(
        StrUnsupportedMediaType::get() + mediaType,
        static_cast<OWeakObject *>(this),
        static_cast<sal_Int16>(-1) );
}

//______________________________________________________________________________
OUString const & BackendImpl::getConfigLayer()
{
    if (m_configLayer.getLength() == 0)
    {
        OUString path(
            makeURL( expandUnoRcUrl( getCachePath() ), OUSTR("registry") ) );
        ::osl::FileBase::RC rc = ::osl::File::getAbsoluteFileURL(
            OUString(), path, m_configLayer );
        if (rc != ::osl::FileBase::E_None)
            throw RuntimeException(
                OUSTR("making file URL absolute failed: ") + path,
                static_cast<OWeakObject *>(this) );
    }
    return m_configLayer;
}

//==============================================================================
class SchemaFileRoot : public ::dp_misc::XmlRootElement
{
public:
    OUString m_name;
    OUString m_package;

    inline SchemaFileRoot()
        : XmlRootElement( OUSTR("http://openoffice.org/2001/registry"),
                          OUSTR("component-schema") )
        {}

    // XRoot
    virtual Reference< xml::input::XElement > SAL_CALL startRootElement(
        sal_Int32 uid, OUString const & localname,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
};

//______________________________________________________________________________
Reference< xml::input::XElement > SchemaFileRoot::startRootElement(
    sal_Int32 uid, OUString const & localname,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // check root element:
    XmlRootElement::startRootElement( uid, localname, xAttributes );

    // "name" attribute
    m_name = xAttributes->getValueByUidName( getUid(), OUSTR("name") );
    if (m_name.getLength() == 0)
        throw xml::sax::SAXException(
            OUSTR("missing schema name attribute!"),
            static_cast< OWeakObject * >(this), Any() );

    // "package" attribute
    m_package = xAttributes->getValueByUidName( getUid(), OUSTR("package") );
    if (m_package.getLength() == 0)
        throw xml::sax::SAXException(
            OUSTR("missing schema package attribute!"),
            static_cast<OWeakObject *>(this), Any() );

    // don't go deeper...
    return Reference<xml::input::XElement>();
}

//______________________________________________________________________________
void BackendImpl::xcs_merge_in(
    OUString const & url,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    // parse out schema package:
    SchemaFileRoot * root = new SchemaFileRoot;
    Reference<xml::input::XRoot> xRoot( root );
    ::ucbhelper::Content ucb_content( url, xCmdEnv );
    xml_parse( xRoot, ucb_content, getComponentContext() );

    OUString dest_folder(
        makeURL( getConfigLayer(), OUSTR("schema/") + ::rtl::Uri::encode(
                     root->m_package, rtl_UriCharClassPchar,
                     rtl_UriEncodeIgnoreEscapes,
                     RTL_TEXTENCODING_UTF8 ).replace( '.', '/' ) ) );
    OUString title( root->m_name + OUSTR(".xcs") );
    OUString dest_url( makeURL( dest_folder, ::rtl::Uri::encode(
                                    title, rtl_UriCharClassPchar,
                                    rtl_UriEncodeIgnoreEscapes,
                                    RTL_TEXTENCODING_UTF8 ) ) );
    // assure dest folder is existing:
    ::ucbhelper::Content ucb_dest_folder;
    create_folder( &ucb_dest_folder, dest_folder, xCmdEnv );
    if (! ucb_dest_folder.transferContent(
            ::ucbhelper::Content( url, xCmdEnv ),
            ::ucbhelper::InsertOperation_COPY,
            title, NameClash::OVERWRITE ))
        throw RuntimeException(
            OUSTR("::ucb::Content::transferContent() failed!"), 0 );
}

//##############################################################################

//------------------------------------------------------------------------------
OUString encodeForXml( OUString const & text )
{
    // encode conforming xml:
    sal_Int32 len = text.getLength();
    ::rtl::OUStringBuffer buf;
    for ( sal_Int32 pos = 0; pos < len; ++pos )
    {
        sal_Unicode c = text[ pos ];
        switch (c) {
        case '<':
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("&lt;") );
            break;
        case '>':
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("&gt;") );
            break;
        case '&':
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("&amp;") );
            break;
        case '\'':
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("&apos;") );
            break;
        case '\"':
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("&quot;") );
            break;
        default:
            buf.append( c );
            break;
        }
    }
    return buf.makeStringAndClear();
}

//______________________________________________________________________________
void BackendImpl::xcu_merge_in(
    OUString const & url, Reference< XCommandEnvironment > const & xCmdEnv )
{
    // looking for %origin%:
    ::ucbhelper::Content ucb_content( url, xCmdEnv );
    ::rtl::ByteSequence bytes( readFile( ucb_content ) );
    ::rtl::ByteSequence filtered( bytes.getLength() * 2,
                                  ::rtl::BYTESEQ_NODEFAULT );
    bool use_filtered = false;
    ::rtl::OString origin;
    sal_Char const * pBytes = reinterpret_cast<sal_Char const *>(
        bytes.getConstArray());
    sal_Size nBytes = bytes.getLength();
    sal_Int32 write_pos = 0;
    while (nBytes > 0)
    {
        sal_Int32 index = rtl_str_indexOfChar_WithLength( pBytes, nBytes, '%' );
        if (index < 0) {
            if (! use_filtered) // opt
                break;
            index = nBytes;
        }

        if ((write_pos + index) > filtered.getLength())
            filtered.realloc( (filtered.getLength() + index) * 2 );
        rtl_copyMemory( filtered.getArray() + write_pos, pBytes, index );
        write_pos += index;
        pBytes += index;
        nBytes -= index;
        if (nBytes == 0)
            break;

        // consume %:
        ++pBytes;
        --nBytes;
        sal_Char const * pAdd = "%";
        sal_Int32 nAdd = 1;
        if (nBytes > 1 && pBytes[ 0 ] == '%')
        {
            // %% => %
            ++pBytes;
            --nBytes;
            use_filtered = true;
        }
        else if (rtl_str_shortenedCompare_WithLength(
                     pBytes, nBytes,
                     RTL_CONSTASCII_STRINGPARAM("origin%"),
                     sizeof ("origin%") - 1 ) == 0)
        {
            if (origin.getLength() == 0) {
                // encode only once
                origin = ::rtl::OUStringToOString(
                    encodeForXml( url.copy( 0, url.lastIndexOf( '/' ) ) ),
                    // xxx todo: encode always for UTF-8? => lookup doc-header?
                    RTL_TEXTENCODING_UTF8 );
            }
            pAdd = origin.getStr();
            nAdd = origin.getLength();
            pBytes += (sizeof ("origin%") - 1);
            nBytes -= (sizeof ("origin%") - 1);
            use_filtered = true;
        }
        if ((write_pos + nAdd) > filtered.getLength())
            filtered.realloc( (filtered.getLength() + nAdd) * 2 );
        rtl_copyMemory( filtered.getArray() + write_pos, pAdd, nAdd );
        write_pos += nAdd;
    }
    if (use_filtered && write_pos < filtered.getLength())
        filtered.realloc( write_pos );

    Reference<XComponentContext> const & xContext = getComponentContext();
    if (! m_xMergeImporter.is()) {
        m_xMergeImporter.set(
            xContext->getServiceManager()->createInstanceWithContext(
                OUSTR("com.sun.star.configuration.backend.MergeImporter"),
                xContext ), UNO_QUERY_THROW );
    }

    Reference< ::com::sun::star::configuration::backend::XLayer > xLayer(
        xContext->getServiceManager()->createInstanceWithContext(
            OUSTR("com.sun.star.configuration.backend.xml.LayerParser"),
            xContext ), UNO_QUERY_THROW );

    Reference< io::XActiveDataSink > xActiveDataSink( xLayer, UNO_QUERY_THROW );
    if (use_filtered)
        xActiveDataSink->setInputStream( ::xmlscript::createInputStream(
                                             filtered ) );
    else
        ucb_content.openStream( xActiveDataSink );

    if (transientMode())
        m_xMergeImporter->importLayer( xLayer );
    else
        m_xMergeImporter->importLayerForEntity( xLayer, getConfigLayer() );
}

//##############################################################################

// Package
//______________________________________________________________________________
BackendImpl * BackendImpl::PackageImpl::getMyBackend() const
{
    BackendImpl * pBackend = static_cast<BackendImpl *>(m_myBackend.get());
    if (NULL == pBackend)
    {
        //May throw a DisposedException
        check();
        //We should never get here...
        throw RuntimeException(
            OUSTR("Failed to get the BackendImpl"),
            static_cast<OWeakObject*>(const_cast<PackageImpl *>(this)));
    }
    return pBackend;
}

beans::Optional< beans::Ambiguous<sal_Bool> >
BackendImpl::PackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard &,
    ::rtl::Reference<AbortChannel> const &,
    Reference<XCommandEnvironment> const & )
{
    BackendImpl * that = getMyBackend();
    return beans::Optional< beans::Ambiguous<sal_Bool> >(
        true /* IsPresent */,
        beans::Ambiguous<sal_Bool>(
            that->m_registeredPackages->has(
                rtl::OUStringToOString( m_url, RTL_TEXTENCODING_UTF8 ) ),
            false /* IsAmbiguous */ ) );
}

//______________________________________________________________________________
void BackendImpl::PackageImpl::processPackage_(
    ::osl::ResettableMutexGuard &,
    bool doRegisterPackage,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    if (doRegisterPackage)
    {
        if (m_isSchema)
        {
            OSL_ENSURE( ! that->transientMode(),
                        "### schema cannot be deployed transiently!" );
            if (! that->transientMode()) {
                that->xcs_merge_in( m_url, xCmdEnv );
                that->m_registeredPackages->put(
                    rtl::OUStringToOString( m_url, RTL_TEXTENCODING_UTF8),
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM(
                            "vnd.sun.star.configuration-schema" ) ) );
            }
        }
        else
        {
            that->xcu_merge_in( m_url, xCmdEnv );
            that->m_registeredPackages->put(
                rtl::OUStringToOString( m_url, RTL_TEXTENCODING_UTF8 ),
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "vnd.sun.star.configuration-data" ) ) );
        }
    }
    else // revoke
    {
        OSL_ASSERT(
            that->m_registeredPackages->has(
                rtl::OUStringToOString( m_url, RTL_TEXTENCODING_UTF8 ) ) );
        t_string2string_map entries( that->m_registeredPackages->getEntries() );
        t_string2string_map::const_iterator iPos( entries.begin() );
        t_string2string_map::const_iterator const iEnd( entries.end() );

        if (m_isSchema)
        {
            if (! that->transientMode())
            {
                ::ucbhelper::Content ucbSaveLayer(
                    makeURL( that->getConfigLayer(), OUSTR("schema") ),
                    xCmdEnv );
                ucbSaveLayer.setPropertyValue(
                    StrTitle::get(), Any( OUSTR("schema.bak") ) );
                try {
                    for ( ; iPos != iEnd; ++iPos )
                    {
                        checkAborted( abortChannel );
                        if (iPos->second == "vnd.sun.star.configuration-schema")
                        {
                            OUString url(
                                rtl::OStringToOUString(
                                    iPos->first, RTL_TEXTENCODING_UTF8 ) );
                            if (!url.equals( m_url )) {
                                that->xcs_merge_in( url, xCmdEnv );
                            }
                        }
                    }
                }
                catch (RuntimeException &) {
                    throw;
                }
                catch (Exception &) {
                    ucbSaveLayer.setPropertyValue(
                        StrTitle::get(), Any( OUSTR("schema") ) );
                    throw;
                }
                that->m_registeredPackages->erase(
                    rtl::OUStringToOString( m_url, RTL_TEXTENCODING_UTF8 ) );
                ucbSaveLayer.executeCommand(
                    OUSTR("delete"), Any( true /* delete physically */ ) );
            }
        }
        else // data
        {
            if (! that->transientMode())
            {
                ::ucbhelper::Content ucbSaveLayer(
                    makeURL( that->getConfigLayer(), OUSTR("data") ),
                    xCmdEnv );
                ucbSaveLayer.setPropertyValue(
                    StrTitle::get(), Any( OUSTR("data.bak") ) );
                try {
                    for ( ; iPos != iEnd; ++iPos )
                    {
                        checkAborted( abortChannel );
                        if (iPos->second == "vnd.sun.star.configuration-data") {
                            OUString url(
                                rtl::OStringToOUString(
                                    iPos->first, RTL_TEXTENCODING_UTF8 ) );
                            if (!url.equals( m_url )) {
                                that->xcu_merge_in( url, xCmdEnv );
                            }
                        }
                    }
                }
                catch (RuntimeException &) {
                    throw;
                }
                catch (Exception &) {
                    ucbSaveLayer.setPropertyValue(
                        StrTitle::get(), Any( OUSTR("data") ) );
                    throw;
                }
                that->m_registeredPackages->erase(
                    rtl::OUStringToOString( m_url, RTL_TEXTENCODING_UTF8 ) );
                ucbSaveLayer.executeCommand(
                    OUSTR("delete"), Any( true /* delete physically */ ) );
            }
        }
    }

    if (!m_isSchema && getMyBackend()->m_defaultProvider.is()) {
        // temp workaround for config bug:
        try {
            getMyBackend()->m_defaultProvider->refresh();
        }
        catch (lang::WrappedTargetRuntimeException & exc) {
            (void) exc;
            OSL_ENSURE( 0, ::rtl::OUStringToOString(
                            ::comphelper::anyToString(exc.TargetException),
                            RTL_TEXTENCODING_UTF8 ).getStr() );
        }
    }
}

} // anon namespace

namespace sdecl = comphelper::service_decl;
sdecl::class_<BackendImpl, sdecl::with_args<true> > serviceBI;
extern sdecl::ServiceDecl const serviceDecl(
    serviceBI,
    "com.sun.star.comp.deployment.configuration.PackageRegistryBackend",
    BACKEND_SERVICE_NAME );

} // namespace configuration
} // namespace backend
} // namespace dp_registry

