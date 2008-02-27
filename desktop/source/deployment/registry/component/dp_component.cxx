/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_component.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-27 10:22:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "dp_component.hrc"
#include "dp_backend.h"
#include "dp_ucb.h"
#include "rtl/string.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/uri.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "ucbhelper/content.hxx"
#include "comphelper/anytostring.hxx"
#include "comphelper/servicedecl.hxx"
#include "comphelper/sequence.hxx"
#include "xmlscript/xml_helper.hxx"
#include "svtools/inettype.hxx"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/container/XNameContainer.hpp"
#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/container/XSet.hpp"
#include "com/sun/star/registry/XSimpleRegistry.hpp"
#include "com/sun/star/registry/XImplementationRegistration.hpp"
#include "com/sun/star/loader/XImplementationLoader.hpp"
#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/ucb/NameClash.hpp"
#include "com/sun/star/util/XMacroExpander.hpp"
#include <list>
#include <hash_map>
#include <vector>
#include <memory>
#include <algorithm>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_registry {
namespace backend {
namespace component {
namespace {

typedef ::std::list<OUString> t_stringlist;
typedef ::std::vector< ::std::pair<OUString, OUString> > t_stringpairvec;

/** return a vector of bootstrap variables which have been provided
    as command arguments.
*/
::std::vector<OUString> getCmdBootstrapVariables()
{
    ::std::vector<OUString> ret;
    sal_uInt32 count = osl_getCommandArgCount();
    for (sal_uInt32 i = 0; i < count; i++)
    {
        OUString arg;
        osl_getCommandArg(i, &arg.pData);
        if (arg.matchAsciiL("-env:", 5))
            ret.push_back(arg);
    }
    return ret;
}

bool jarManifestHeaderPresent(
    OUString const & url, OUString const & name,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    ::rtl::OUStringBuffer buf;
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.zip://") );
    buf.append(
        ::rtl::Uri::encode(
            url, rtl_UriCharClassRegName, rtl_UriEncodeIgnoreEscapes,
            RTL_TEXTENCODING_UTF8 ) );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("/META-INF/MANIFEST.MF") );
    ::ucbhelper::Content manifestContent;
    OUString line;
    return
        create_ucb_content(
            &manifestContent, buf.makeStringAndClear(), xCmdEnv,
            false /* no throw */ )
        && readLine( &line, name, manifestContent, RTL_TEXTENCODING_ASCII_US );
}

//==============================================================================
class BackendImpl : public ::dp_registry::backend::PackageRegistryBackend
{
    class ComponentPackageImpl : public ::dp_registry::backend::Package
    {
        BackendImpl * getMyBackend() const {
            return static_cast<BackendImpl *>(m_myBackend.get());
        }

        const OUString m_loader;
        Reference<XComponentContext> m_xRemoteContext;

        enum reg {
            REG_UNINIT, REG_VOID, REG_REGISTERED, REG_NOT_REGISTERED
        } m_registered;

        Reference<loader::XImplementationLoader> getComponentInfo(
            t_stringlist * pImplNames, t_stringpairvec * pSingletons,
            Reference<XComponentContext> const & xContext );

        virtual void SAL_CALL disposing();

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

        const Reference<registry::XSimpleRegistry> getRDB() const;

    public:
        inline ComponentPackageImpl(
            ::rtl::Reference<PackageRegistryBackend> const & myBackend,
            OUString const & url, OUString const & name,
            Reference<deployment::XPackageTypeInfo> const & xPackageType,
            OUString const & loader )
            : Package( myBackend, url, name, name /* display-name */,
                       xPackageType ),
              m_loader( loader ),
              m_registered( REG_UNINIT )
            {}
    };
    friend class ComponentPackageImpl;

    class TypelibraryPackageImpl : public ::dp_registry::backend::Package
    {
        BackendImpl * getMyBackend() const {
            return static_cast<BackendImpl *>(m_myBackend.get());
        }

        const bool m_jarFile;
        Reference<container::XHierarchicalNameAccess> m_xTDprov;

        virtual void SAL_CALL disposing();

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
        inline TypelibraryPackageImpl(
            ::rtl::Reference<PackageRegistryBackend> const & myBackend,
            OUString const & url, OUString const & name,
            Reference<deployment::XPackageTypeInfo> const & xPackageType,
            bool jarFile )
            : Package( myBackend, url, name, name /* display-name */,
                       xPackageType ),
              m_jarFile( jarFile )
            {}
    };
    friend class TypelibraryPackageImpl;

    t_stringlist m_jar_typelibs;
    t_stringlist m_rdb_typelibs;
    t_stringlist & getTypelibs( bool jar ) {
        return jar ? m_jar_typelibs : m_rdb_typelibs;
    }

    bool m_unorc_inited;
    bool m_unorc_modified;

    typedef ::std::hash_map< OUString, Reference<XInterface>,
                             ::rtl::OUStringHash > t_string2object;
    t_string2object m_backendObjects;

    // PackageRegistryBackend
    virtual Reference<deployment::XPackage> bindPackage_(
        OUString const & url, OUString const & mediaType,
        Reference<XCommandEnvironment> const & xCmdEnv );

    virtual void SAL_CALL disposing();

    const Reference<deployment::XPackageTypeInfo> m_xDynComponentTypeInfo;
    const Reference<deployment::XPackageTypeInfo> m_xJavaComponentTypeInfo;
    const Reference<deployment::XPackageTypeInfo> m_xPythonComponentTypeInfo;
    const Reference<deployment::XPackageTypeInfo> m_xRDBTypelibTypeInfo;
    const Reference<deployment::XPackageTypeInfo> m_xJavaTypelibTypeInfo;
    Sequence< Reference<deployment::XPackageTypeInfo> > m_typeInfos;

    OUString m_commonRDB;
    OUString m_nativeRDB;
    Reference<registry::XSimpleRegistry> m_xCommonRDB;
    Reference<registry::XSimpleRegistry> m_xNativeRDB;

    void unorc_verify_init( Reference<XCommandEnvironment> const & xCmdEnv );
    void unorc_flush( Reference<XCommandEnvironment> const & xCmdEnv );

    Reference<XInterface> getObject( OUString const & id );
    Reference<XInterface> insertObject(
        OUString const & id, Reference<XInterface> const & xObject );
    void releaseObject( OUString const & id );

    bool addToUnoRc( bool jarFile, OUString const & url,
                     Reference<XCommandEnvironment> const & xCmdEnv );
    bool removeFromUnoRc( bool jarFile, OUString const & url,
                          Reference<XCommandEnvironment> const & xCmdEnv );
    bool hasInUnoRc( bool jarFile, OUString const & url );

public:
    BackendImpl( Sequence<Any> const & args,
                 Reference<XComponentContext> const & xComponentContext );

    // XPackageRegistry
    virtual Sequence< Reference<deployment::XPackageTypeInfo> > SAL_CALL
    getSupportedPackageTypes() throw (RuntimeException);

    using PackageRegistryBackend::disposing;
};

//______________________________________________________________________________
const Reference<registry::XSimpleRegistry>
BackendImpl::ComponentPackageImpl::getRDB() const
{
    BackendImpl * that = getMyBackend();
    if (m_loader.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM("com.sun.star.loader.SharedLibrary") ))
        return that->m_xNativeRDB;
    else
        return that->m_xCommonRDB;
}

//______________________________________________________________________________
void BackendImpl::ComponentPackageImpl::disposing()
{
    m_xRemoteContext.clear();
    Package::disposing();
}

//______________________________________________________________________________
void BackendImpl::TypelibraryPackageImpl::disposing()
{
    m_xTDprov.clear();
    Package::disposing();
}

//______________________________________________________________________________
void BackendImpl::disposing()
{
    try {
        m_backendObjects = t_string2object();
        if (m_xNativeRDB.is()) {
            m_xNativeRDB->close();
            m_xNativeRDB.clear();
        }
        if (m_xCommonRDB.is()) {
            m_xCommonRDB->close();
            m_xCommonRDB.clear();
        }
        unorc_flush( Reference<XCommandEnvironment>() );

        PackageRegistryBackend::disposing();
    }
    catch (RuntimeException &) {
        throw;
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException(
            OUSTR("caught unexpected exception while disposing..."),
            static_cast<OWeakObject *>(this), exc );
    }
}

//______________________________________________________________________________
BackendImpl::BackendImpl(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xComponentContext )
    : PackageRegistryBackend( args, xComponentContext ),
      m_unorc_inited( false ),
      m_unorc_modified( false ),
      m_xDynComponentTypeInfo( new Package::TypeInfo(
                                   OUSTR("application/"
                                         "vnd.sun.star.uno-component;"
                                         "type=native;platform=") +
                                   getPlatformString(),
                                   OUSTR("*" SAL_DLLEXTENSION),
                                   getResourceString(RID_STR_DYN_COMPONENT),
                                   RID_IMG_COMPONENT,
                                   RID_IMG_COMPONENT_HC ) ),
      m_xJavaComponentTypeInfo( new Package::TypeInfo(
                                    OUSTR("application/"
                                          "vnd.sun.star.uno-component;"
                                          "type=Java"),
                                    OUSTR("*.jar"),
                                    getResourceString(RID_STR_JAVA_COMPONENT),
                                    RID_IMG_JAVA_COMPONENT,
                                    RID_IMG_JAVA_COMPONENT_HC ) ),
      m_xPythonComponentTypeInfo( new Package::TypeInfo(
                                      OUSTR("application/"
                                            "vnd.sun.star.uno-component;"
                                            "type=Python"),
                                      OUSTR("*.py"),
                                      getResourceString(
                                          RID_STR_PYTHON_COMPONENT),
                                      RID_IMG_COMPONENT,
                                      RID_IMG_COMPONENT_HC ) ),
      m_xRDBTypelibTypeInfo( new Package::TypeInfo(
                                 OUSTR("application/"
                                       "vnd.sun.star.uno-typelibrary;"
                                       "type=RDB"),
                                 OUSTR("*.rdb"),
                                 getResourceString(RID_STR_RDB_TYPELIB),
                                 RID_IMG_TYPELIB, RID_IMG_TYPELIB_HC ) ),
      m_xJavaTypelibTypeInfo( new Package::TypeInfo(
                                  OUSTR("application/"
                                        "vnd.sun.star.uno-typelibrary;"
                                        "type=Java"),
                                  OUSTR("*.jar"),
                                  getResourceString(RID_STR_JAVA_TYPELIB),
                                  RID_IMG_JAVA_TYPELIB,
                                  RID_IMG_JAVA_TYPELIB_HC ) ),
      m_typeInfos( 5 )
{
    m_typeInfos[ 0 ] = m_xDynComponentTypeInfo;
    m_typeInfos[ 1 ] = m_xJavaComponentTypeInfo;
    m_typeInfos[ 2 ] = m_xPythonComponentTypeInfo;
    m_typeInfos[ 3 ] = m_xRDBTypelibTypeInfo;
    m_typeInfos[ 4 ] = m_xJavaTypelibTypeInfo;

    const Reference<XCommandEnvironment> xCmdEnv;

    if (transientMode())
    {
        // in-mem rdbs:
        // common rdb for java, native rdb for shared lib components
        m_xCommonRDB.set(
            xComponentContext->getServiceManager()->createInstanceWithContext(
                OUSTR("com.sun.star.registry.SimpleRegistry"),
                xComponentContext ), UNO_QUERY_THROW );
        m_xCommonRDB->open( OUString() /* in-mem */,
                            false /* ! read-only */, true /* create */ );
        m_xNativeRDB.set(
            xComponentContext->getServiceManager()->createInstanceWithContext(
                OUSTR("com.sun.star.registry.SimpleRegistry"),
                xComponentContext ), UNO_QUERY_THROW );
        m_xNativeRDB->open( OUString() /* in-mem */,
                            false /* ! read-only */, true /* create */ );
    }
    else
    {
        unorc_verify_init( xCmdEnv );

        if (! m_readOnly) {
            ::ucbhelper::Content cacheDir( getCachePath(), xCmdEnv );
            ::ucbhelper::Content oldRDB;
            // switch common rdb:
            if (m_commonRDB.getLength() > 0)
                create_ucb_content(
                    &oldRDB, makeURL( getCachePath(), m_commonRDB ),
                    xCmdEnv, false /* no throw */ );
            m_commonRDB = m_commonRDB.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM("common.rdb") )
                ? OUSTR("common_.rdb") : OUSTR("common.rdb");
            if (oldRDB.get().is())
            {
                if (! cacheDir.transferContent(
                        oldRDB, ::ucbhelper::InsertOperation_COPY,
                        m_commonRDB, NameClash::OVERWRITE ))
                    throw RuntimeException(
                        OUSTR("UCB transferContent() failed!"), 0 );
                oldRDB = ::ucbhelper::Content();
            }
            // switch native rdb:
            if (m_nativeRDB.getLength() > 0)
                create_ucb_content(
                    &oldRDB, makeURL( getCachePath(), m_nativeRDB ),
                    xCmdEnv, false /* no throw */ );
            const OUString plt_rdb( getPlatformString() + OUSTR(".rdb") );
            const OUString plt_rdb_( getPlatformString() + OUSTR("_.rdb") );
            m_nativeRDB = m_nativeRDB.equals( plt_rdb ) ? plt_rdb_ : plt_rdb;
            if (oldRDB.get().is())
            {
                if (! cacheDir.transferContent(
                        oldRDB, ::ucbhelper::InsertOperation_COPY,
                        m_nativeRDB, NameClash::OVERWRITE ))
                    throw RuntimeException(
                        OUSTR("UCB transferContent() failed!"), 0 );
            }

            // UNO is bootstrapped, flush for next process start:
            m_unorc_modified = true;
            unorc_flush( Reference<XCommandEnvironment>() );
        }

        // common rdb for java, native rdb for shared lib components
        if (m_commonRDB.getLength() > 0) {
            m_xCommonRDB.set(
                xComponentContext->getServiceManager()
                ->createInstanceWithContext(
                    OUSTR("com.sun.star.registry.SimpleRegistry"),
                    xComponentContext ), UNO_QUERY_THROW );
            m_xCommonRDB->open(
                makeURL( expandUnoRcUrl(getCachePath()), m_commonRDB ),
                m_readOnly, !m_readOnly );
        }
        if (m_nativeRDB.getLength() > 0) {
            m_xNativeRDB.set(
                xComponentContext->getServiceManager()
                ->createInstanceWithContext(
                    OUSTR("com.sun.star.registry.SimpleRegistry"),
                    xComponentContext ), UNO_QUERY_THROW );
            m_xNativeRDB->open(
                makeURL( expandUnoRcUrl(getCachePath()), m_nativeRDB ),
                m_readOnly, !m_readOnly );
        }
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
    OUString mediaType(mediaType_);
    if (mediaType.getLength() == 0 ||
        mediaType.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM(
                "application/vnd.sun.star.uno-component") ) ||
        mediaType.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM(
                "application/vnd.sun.star.uno-typelibrary") ))
    {
        // detect exact media-type:
        ::ucbhelper::Content ucbContent;
        if (create_ucb_content( &ucbContent, url, xCmdEnv )) {
            const OUString title( ucbContent.getPropertyValue(
                                      StrTitle::get() ).get<OUString>() );
            if (title.endsWithIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM(SAL_DLLEXTENSION) ))
            {
                mediaType = OUSTR("application/vnd.sun.star.uno-component;"
                                  "type=native;platform=") +
                    getPlatformString();
            }
            else if (title.endsWithIgnoreAsciiCaseAsciiL(
                         RTL_CONSTASCII_STRINGPARAM(".jar") ))
            {
                if (jarManifestHeaderPresent(
                        url, OUSTR("RegistrationClassName"), xCmdEnv ))
                    mediaType = OUSTR(
                        "application/vnd.sun.star.uno-component;type=Java");
                if (mediaType.getLength() == 0)
                    mediaType = OUSTR(
                        "application/vnd.sun.star.uno-typelibrary;type=Java");
            }
            else if (title.endsWithIgnoreAsciiCaseAsciiL(
                         RTL_CONSTASCII_STRINGPARAM(".py") ))
                mediaType =
                    OUSTR("application/vnd.sun.star.uno-component;type=Python");
            else if (title.endsWithIgnoreAsciiCaseAsciiL(
                         RTL_CONSTASCII_STRINGPARAM(".rdb") ))
                mediaType =
                    OUSTR("application/vnd.sun.star.uno-typelibrary;type=RDB");
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
            const OUString name( ucbContent.getPropertyValue(
                                     StrTitle::get() ).get<OUString>() );
            if (subType.EqualsIgnoreCaseAscii("vnd.sun.star.uno-component"))
            {
                // xxx todo: probe and evaluate component xml description

                INetContentTypeParameter const * param = params.find(
                    ByteString("platform") );
                if (param == 0 || platform_fits( param->m_sValue )) {
                    param = params.find( ByteString("type") );
                    if (param != 0)
                    {
                        String const & value = param->m_sValue;
                        if (value.EqualsIgnoreCaseAscii("native")) {
                            return new BackendImpl::ComponentPackageImpl(
                                this, url, name, m_xDynComponentTypeInfo,
                                OUSTR("com.sun.star.loader.SharedLibrary") );
                        }
                        if (value.EqualsIgnoreCaseAscii("Java")) {
                            return new BackendImpl::ComponentPackageImpl(
                                this, url, name, m_xJavaComponentTypeInfo,
                                OUSTR("com.sun.star.loader.Java2") );
                        }
                        if (value.EqualsIgnoreCaseAscii("Python")) {
                            return new BackendImpl::ComponentPackageImpl(
                                this, url, name, m_xPythonComponentTypeInfo,
                                OUSTR("com.sun.star.loader.Python") );
                        }
                    }
                }
            }
            else if (subType.EqualsIgnoreCaseAscii(
                         "vnd.sun.star.uno-typelibrary"))
            {
                INetContentTypeParameter const * param = params.find(
                    ByteString("type") );
                if (param != 0) {
                    String const & value = param->m_sValue;
                    if (value.EqualsIgnoreCaseAscii("RDB"))
                    {
                        return new BackendImpl::TypelibraryPackageImpl(
                            this, url, name, m_xRDBTypelibTypeInfo,
                            false /* rdb */ );
                    }
                    if (value.EqualsIgnoreCaseAscii("Java")) {
                        return new BackendImpl::TypelibraryPackageImpl(
                            this, url, name, m_xJavaTypelibTypeInfo,
                            true /* jar */ );
                    }
                }
            }
        }
    }
    throw lang::IllegalArgumentException(
        StrUnsupportedMediaType::get() + mediaType,
        static_cast<OWeakObject *>(this),
        static_cast<sal_Int16>(-1) );
}

//##############################################################################

//______________________________________________________________________________
void BackendImpl::unorc_verify_init(
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    if (transientMode())
        return;
    const ::osl::MutexGuard guard( getMutex() );
    if (! m_unorc_inited)
    {
        // common rc:
        ::ucbhelper::Content ucb_content;
        if (create_ucb_content(
                &ucb_content,
                makeURL( getCachePath(), OUSTR("unorc") ),
                xCmdEnv, false /* no throw */ ))
        {
            OUString line;
            if (readLine( &line, OUSTR("UNO_JAVA_CLASSPATH="), ucb_content,
                          RTL_TEXTENCODING_UTF8 ))
            {
                sal_Int32 index = sizeof ("UNO_JAVA_CLASSPATH=") - 1;
                do {
                    OUString token( line.getToken( 0, ' ', index ).trim() );
                    if (token.getLength() > 0) {
                        // cleanup, check if existing:
                        if (create_ucb_content(
                                0, expandUnoRcTerm(token), xCmdEnv,
                                false /* no throw */ )) {
                            m_jar_typelibs.push_back( token );
                        }
                        else
                            OSL_ENSURE(
                                0, "### invalid UNO_JAVA_CLASSPATH entry!" );
                    }
                }
                while (index >= 0);
            }
            if (readLine( &line, OUSTR("UNO_TYPES="), ucb_content,
                          RTL_TEXTENCODING_UTF8 )) {
                sal_Int32 index = sizeof ("UNO_TYPES=") - 1;
                do {
                    OUString token( line.getToken( 0, ' ', index ).trim() );
                    if (token.getLength() > 0) {
                        if (token[ 0 ] == '?')
                            token = token.copy( 1 );
                        // cleanup, check if existing:
                        if (create_ucb_content(
                                0, expandUnoRcTerm(token),
                                xCmdEnv, false /* no throw */ )) {
                            m_rdb_typelibs.push_back( token );
                        }
                        else
                            OSL_ENSURE( 0, "### invalid UNO_TYPES entry!" );
                    }
                }
                while (index >= 0);
            }
            if (readLine( &line, OUSTR("UNO_SERVICES="), ucb_content,
                          RTL_TEXTENCODING_UTF8 )) {
                sal_Int32 start = sizeof ("UNO_SERVICES=?$ORIGIN/") - 1;
                sal_Int32 sep = line.indexOf( ' ', start );
                OSL_ASSERT( sep > 0 );
                m_commonRDB = line.copy( start, sep - start );
            }

            // native rc:
            if (create_ucb_content(
                    &ucb_content,
                    makeURL( getCachePath(), getPlatformString() + OUSTR("rc")),
                    xCmdEnv, false /* no throw */ )) {
                if (readLine( &line, OUSTR("UNO_SERVICES="), ucb_content,
                              RTL_TEXTENCODING_UTF8 )) {
                    m_nativeRDB = line.copy(
                        sizeof ("UNO_SERVICES=?$ORIGIN/") - 1 );
                }
            }
        }
        m_unorc_modified = false;
        m_unorc_inited = true;
    }
}

//______________________________________________________________________________
void BackendImpl::unorc_flush( Reference<XCommandEnvironment> const & xCmdEnv )
{
    if (transientMode())
        return;
    if (!m_unorc_inited || !m_unorc_modified)
        return;

    ::rtl::OStringBuffer buf;
    // UNO_USER_PACKAGES_CACHE, UNO_SHARED_PACKAGES_CACHE have to be resolved
    // locally:
    if (m_eContext == CONTEXT_USER) {
        buf.append( RTL_CONSTASCII_STRINGPARAM(
                        "UNO_USER_PACKAGES_CACHE=$ORIGIN/../..") );
    }
    else if (m_eContext == CONTEXT_SHARED) {
        buf.append( RTL_CONSTASCII_STRINGPARAM(
                        "UNO_SHARED_PACKAGES_CACHE=$ORIGIN/../..") );
    }
    else
        OSL_ASSERT(0);
    buf.append(LF);

    if (! m_jar_typelibs.empty())
    {
        t_stringlist::const_iterator iPos( m_jar_typelibs.begin() );
        t_stringlist::const_iterator const iEnd( m_jar_typelibs.end() );
        buf.append( RTL_CONSTASCII_STRINGPARAM("UNO_JAVA_CLASSPATH=") );
        while (iPos != iEnd) {
            // encoded ASCII file-urls:
            const ::rtl::OString item(
                ::rtl::OUStringToOString( *iPos, RTL_TEXTENCODING_ASCII_US ) );
            buf.append( item );
            ++iPos;
            if (iPos != iEnd)
                buf.append( ' ' );
        }
        buf.append(LF);
    }
    if (! m_rdb_typelibs.empty())
    {
        t_stringlist::const_iterator iPos( m_rdb_typelibs.begin() );
        t_stringlist::const_iterator const iEnd( m_rdb_typelibs.end() );
        buf.append( RTL_CONSTASCII_STRINGPARAM("UNO_TYPES=") );
        while (iPos != iEnd) {
            buf.append( '?' );
            // encoded ASCII file-urls:
            const ::rtl::OString item(
                ::rtl::OUStringToOString( *iPos, RTL_TEXTENCODING_ASCII_US ) );
            buf.append( item );
            ++iPos;
            if (iPos != iEnd)
                buf.append( ' ' );
        }
        buf.append(LF);
    }
    if (m_commonRDB.getLength() > 0 || m_nativeRDB.getLength() > 0)
    {
        buf.append( RTL_CONSTASCII_STRINGPARAM("UNO_SERVICES=?$ORIGIN/") );
        buf.append( ::rtl::OUStringToOString(
                        m_commonRDB, RTL_TEXTENCODING_ASCII_US ) );
        if (m_nativeRDB.getLength() > 0)
        {
            buf.append( RTL_CONSTASCII_STRINGPARAM(
                            " ${$ORIGIN/${_OS}_${_ARCH}rc:UNO_SERVICES}") );
            buf.append(LF);

            // write native rc:
            ::rtl::OStringBuffer buf2;
            buf2.append( RTL_CONSTASCII_STRINGPARAM("UNO_SERVICES=?$ORIGIN/") );
            buf2.append( ::rtl::OUStringToOString(
                             m_nativeRDB, RTL_TEXTENCODING_ASCII_US ) );
            buf2.append(LF);

            const Reference<io::XInputStream> xData(
                ::xmlscript::createInputStream(
                    ::rtl::ByteSequence(
                        reinterpret_cast<sal_Int8 const *>(buf2.getStr()),
                        buf2.getLength() ) ) );
            ::ucbhelper::Content ucb_content(
                makeURL( getCachePath(), getPlatformString() + OUSTR("rc") ),
                xCmdEnv );
            ucb_content.writeStream( xData, true /* replace existing */ );
        }
    }

    // write unorc:
    const Reference<io::XInputStream> xData(
        ::xmlscript::createInputStream(
            ::rtl::ByteSequence(
                reinterpret_cast<sal_Int8 const *>(buf.getStr()),
                buf.getLength() ) ) );
    ::ucbhelper::Content ucb_content(
        makeURL( getCachePath(), OUSTR("unorc") ), xCmdEnv );
    ucb_content.writeStream( xData, true /* replace existing */ );

    m_unorc_modified = false;
}

//------------------------------------------------------------------------------
inline OUString makeRcTerm( OUString const & url )
{
    OSL_ASSERT( url.matchAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                     "vnd.sun.star.expand:") ) );
    if (url.matchAsciiL( RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.expand:") )) {
        // cut protocol:
        OUString rcterm( url.copy( sizeof ("vnd.sun.star.expand:") - 1 ) );
        // decode uric class chars:
        rcterm = ::rtl::Uri::decode(
            rcterm, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
        return rcterm;
    }
    else
        return url;
}

//______________________________________________________________________________
bool BackendImpl::addToUnoRc( bool jarFile, OUString const & url_,
                              Reference<XCommandEnvironment> const & xCmdEnv )
{
    const OUString rcterm( makeRcTerm(url_) );
    const ::osl::MutexGuard guard( getMutex() );
    unorc_verify_init( xCmdEnv );
    t_stringlist & rSet = getTypelibs(jarFile);
    if (::std::find( rSet.begin(), rSet.end(), rcterm ) == rSet.end()) {
        rSet.push_front( rcterm ); // prepend to list, thus overriding
        // write immediately:
        m_unorc_modified = true;
        unorc_flush( xCmdEnv );
        return true;
    }
    else
        return false;
}

//______________________________________________________________________________
bool BackendImpl::removeFromUnoRc(
    bool jarFile, OUString const & url_,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    const OUString rcterm( makeRcTerm(url_) );
    const ::osl::MutexGuard guard( getMutex() );
    unorc_verify_init( xCmdEnv );
    getTypelibs(jarFile).remove( rcterm );
    // write immediately:
    m_unorc_modified = true;
    unorc_flush( xCmdEnv );
    return true;
}

//______________________________________________________________________________
bool BackendImpl::hasInUnoRc(
    bool jarFile, OUString const & url_ )
{
    const OUString rcterm( makeRcTerm(url_) );
    const ::osl::MutexGuard guard( getMutex() );
    t_stringlist const & rSet = getTypelibs(jarFile);
    return ::std::find( rSet.begin(), rSet.end(), rcterm ) != rSet.end();
}

//______________________________________________________________________________
void BackendImpl::releaseObject( OUString const & id )
{
    const ::osl::MutexGuard guard( getMutex() );
    if ( m_backendObjects.erase( id ) != 1 )
    {
        OSL_ASSERT( false );
    }
}

//______________________________________________________________________________
Reference<XInterface> BackendImpl::getObject( OUString const & id )
{
    const ::osl::MutexGuard guard( getMutex() );
    const t_string2object::const_iterator iFind( m_backendObjects.find( id ) );
    if (iFind == m_backendObjects.end())
        return Reference<XInterface>();
    else
        return iFind->second;
}

//______________________________________________________________________________
Reference<XInterface> BackendImpl::insertObject(
    OUString const & id, Reference<XInterface> const & xObject )
{
    const ::osl::MutexGuard guard( getMutex() );
    const ::std::pair<t_string2object::iterator, bool> insertion(
        m_backendObjects.insert( t_string2object::value_type(
                                     id, xObject ) ) );
    return insertion.first->second;
}

//------------------------------------------------------------------------------
Reference<XComponentContext> raise_uno_process(
    Reference<XComponentContext> const & xContext,
    ::rtl::Reference<AbortChannel> const & abortChannel )
{
    OSL_ASSERT( xContext.is() );

    ::rtl::OUString url(
        Reference<util::XMacroExpander>(
            xContext->getValueByName(
                OUSTR("/singletons/com.sun.star.util.theMacroExpander") ),
            UNO_QUERY_THROW )->
        expandMacros( OUSTR("$URE_BIN_DIR/uno") ) );

    ::rtl::OUStringBuffer buf;
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("uno:pipe,name=") );
    OUString pipeId( generateRandomPipeId() );
    buf.append( pipeId );
    buf.appendAscii(
        RTL_CONSTASCII_STRINGPARAM(";urp;uno.ComponentContext") );
    const OUString connectStr( buf.makeStringAndClear() );

    // raise core UNO process to register/run a component,
    // javavm service uses unorc next to executable to retrieve deployed
    // jar typelibs

    ::std::vector<OUString> args;
#if OSL_DEBUG_LEVEL <= 1
    args.push_back( OUSTR("--quiet") );
#endif
    args.push_back( OUSTR("--singleaccept") );
    args.push_back( OUSTR("-u") );
    args.push_back( connectStr );
    // don't inherit from unorc:
    args.push_back( OUSTR("-env:INIFILENAME=") );

    //now add the bootstrap variables which were supplied on the command line
    ::std::vector<OUString> bootvars = getCmdBootstrapVariables();
    args.insert(args.end(), bootvars.begin(), bootvars.end());

    oslProcess hProcess = raiseProcess(
        url, comphelper::containerToSequence(args) );
    try {
        return Reference<XComponentContext>(
            resolveUnoURL( connectStr, xContext, abortChannel.get() ),
            UNO_QUERY_THROW );
    }
    catch (...) {
        // try to terminate process:
        if ( osl_terminateProcess( hProcess ) != osl_Process_E_None )
        {
            OSL_ASSERT( false );
        }
        throw;
    }
}

//------------------------------------------------------------------------------
Reference<loader::XImplementationLoader>
BackendImpl::ComponentPackageImpl::getComponentInfo(
    t_stringlist * pImplNames, t_stringpairvec * pSingletons,
    Reference<XComponentContext> const & xContext )
{
    const Reference<loader::XImplementationLoader> xLoader(
        xContext->getServiceManager()->createInstanceWithContext(
            m_loader, xContext ), UNO_QUERY );
    if (! xLoader.is())
        return Reference<loader::XImplementationLoader>();

    // HACK: highly dependent on stoc/source/servicemanager
    //       and stoc/source/implreg implementation which rely on the same
    //       services.rdb format!

    const Reference<registry::XSimpleRegistry> xMemReg(
        xContext->getServiceManager()->createInstanceWithContext(
            OUSTR("com.sun.star.registry.SimpleRegistry"), xContext ),
        UNO_QUERY_THROW );
    xMemReg->open( OUString() /* in mem */, false, true );
    xLoader->writeRegistryInfo( xMemReg->getRootKey(), OUString(), getURL() );

    const Sequence< Reference<registry::XRegistryKey> > keys(
        xMemReg->getRootKey()->openKeys() );
    for ( sal_Int32 pos = keys.getLength(); pos--; )
    {
        Reference<registry::XRegistryKey> const & xImplKey = keys[ pos ];
        const OUString implName(
            xImplKey->getKeyName().copy( 1 /*leading slash*/ ) );

        // check for singletons:
        const Reference<registry::XRegistryKey> xSingletonKey(
            xImplKey->openKey( OUSTR("UNO/SINGLETONS") ) );
        if (xSingletonKey.is() && xSingletonKey->isValid())
        {
            const Sequence< Reference<registry::XRegistryKey> > singletonKeys(
                xSingletonKey->openKeys() );
            for ( sal_Int32 i = singletonKeys.getLength(); i--; )
            {
                Reference<registry::XRegistryKey> const & xSingleton =
                    singletonKeys[ i ];
                pSingletons->push_back(
                    ::std::pair<OUString, OUString>(
                        xSingleton->getKeyName().copy(
                            implName.getLength() +
                            sizeof ("//UNO/SINGLETONS/") - 1 ),
                        xSingleton->getStringValue() ) );
            }
        }
        else
        {
            pImplNames->push_back( implName );
        }
    }

    return xLoader;
}

// Package
//______________________________________________________________________________
beans::Optional< beans::Ambiguous<sal_Bool> >
BackendImpl::ComponentPackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard &,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & )
{
    if (m_registered == REG_UNINIT)
    {
        m_registered = REG_NOT_REGISTERED;
        const Reference<registry::XSimpleRegistry> xRDB( getRDB() );
        if (xRDB.is())
        {
            // lookup rdb for location URL:
            const Reference<registry::XRegistryKey> xRootKey(
                xRDB->getRootKey() );
            const Reference<registry::XRegistryKey> xImplKey(
                xRootKey->openKey( OUSTR("IMPLEMENTATIONS") ) );
            Sequence<OUString> implNames;
            if (xImplKey.is() && xImplKey->isValid())
                implNames = xImplKey->getKeyNames();
            OUString const * pImplNames = implNames.getConstArray();
            sal_Int32 pos = implNames.getLength();
            for ( ; pos--; )
            {
                checkAborted( abortChannel );
                const OUString key(
                    pImplNames[ pos ] + OUSTR("/UNO/LOCATION") );
                const Reference<registry::XRegistryKey> xKey(
                    xRootKey->openKey(key) );
                if (xKey.is() && xKey->isValid()) {
                    const OUString location( xKey->getAsciiValue() );
                    if (location.equalsIgnoreAsciiCase( getURL() ))
                        break;
                }
            }
            if (pos >= 0)
                m_registered = REG_REGISTERED;
        }
    }
    return beans::Optional< beans::Ambiguous<sal_Bool> >(
        true /* IsPresent */,
        beans::Ambiguous<sal_Bool>(
            m_registered == REG_REGISTERED,
            m_registered == REG_VOID /* IsAmbiguous */ ) );
}

//______________________________________________________________________________
void BackendImpl::ComponentPackageImpl::processPackage_(
    ::osl::ResettableMutexGuard &,
    bool doRegisterPackage,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    const bool java = m_loader.equalsAsciiL(
        RTL_CONSTASCII_STRINGPARAM("com.sun.star.loader.Java2") );
    const OUString url( getURL() );
    bool isJavaTypelib = java &&
        !jarManifestHeaderPresent( url, OUSTR("UNO-Type-Path"), xCmdEnv );

    if (doRegisterPackage)
    {
        if (isJavaTypelib)
            that->addToUnoRc( java, url, xCmdEnv );

        if (! m_xRemoteContext.is()) {
            m_xRemoteContext.set(
                that->getObject( url ), UNO_QUERY );
            if (! m_xRemoteContext.is()) {
                m_xRemoteContext.set(
                    that->insertObject( url, raise_uno_process(
                                            that->getComponentContext(),
                                            abortChannel ) ),
                    UNO_QUERY_THROW );
            }
        }

        const Reference<registry::XSimpleRegistry> xServicesRDB( getRDB() );
        const Reference<registry::XImplementationRegistration> xImplReg(
            m_xRemoteContext->getServiceManager()->createInstanceWithContext(
                OUSTR("com.sun.star.registry.ImplementationRegistration"),
                m_xRemoteContext ), UNO_QUERY_THROW );
        xImplReg->registerImplementation( m_loader, url, xServicesRDB );

        t_stringlist implNames;
        t_stringpairvec singletons;
        const Reference<loader::XImplementationLoader> xLoader(
            getComponentInfo( &implNames, &singletons, m_xRemoteContext ) );

        // factories live insertion:
        const Reference<container::XSet> xSet(
            that->getComponentContext()->getServiceManager(), UNO_QUERY_THROW );
        for ( t_stringlist::const_iterator iPos( implNames.begin() );
              iPos != implNames.end(); ++iPos )
        {
            checkAborted( abortChannel );
            OUString const & implName = *iPos;
            // activate factory:
            const Reference<XInterface> xFactory(
                xLoader->activate(
                    implName, OUString(), url,
                    xServicesRDB->getRootKey()->openKey(
                        OUSTR("/IMPLEMENTATIONS/") + implName ) ) );
            try {
                xSet->insert( Any(xFactory) );
            } // ignore if factory has already been inserted:
            catch (container::ElementExistException &) {
                OSL_ENSURE( 0, "### factory already registered?" );
            }
        }

        if (! singletons.empty())
        {
            // singletons live insertion:
            const Reference<container::XNameContainer> xRootContext(
                that->getComponentContext()->getValueByName(
                    OUSTR("_root") ), UNO_QUERY );
            if (xRootContext.is())
            {
                for ( t_stringpairvec::const_iterator iPos(
                          singletons.begin() );
                      iPos != singletons.end(); ++iPos )
                {
                    ::std::pair<OUString, OUString> const & sp = *iPos;
                    const OUString name( OUSTR("/singletons/") + sp.first );
                    // assure no arguments:
                    try {
                        xRootContext->removeByName( name + OUSTR("/arguments"));
                    } catch (container::NoSuchElementException &) {}
                    // used service:
                    try {
                        xRootContext->insertByName(
                            name + OUSTR("/service"), Any(sp.second) );
                    } catch (container::ElementExistException &) {
                        xRootContext->replaceByName(
                            name + OUSTR("/service"), Any(sp.second) );
                    }
                    // singleton entry:
                    try {
                        xRootContext->insertByName( name, Any() );
                    } catch (container::ElementExistException & exc) {
                        (void) exc; // avoid warnings
                        OSL_ENSURE(
                            0, OUStringToOString(
                                exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
                        xRootContext->replaceByName( name, Any() );
                    }
                }
            }
        }

        m_registered = REG_REGISTERED;
    }
    else // revokePackage()
    {
        // set to VOID during revocation process:
        m_registered = REG_VOID;

        Reference<XComponentContext> xContext;
        if (m_xRemoteContext.is()) // has been activated in this process
            xContext = m_xRemoteContext;
        else // has been deployed in former times
            xContext = that->getComponentContext();

        t_stringlist implNames;
        t_stringpairvec singletons;
        getComponentInfo( &implNames, &singletons, xContext );

        // factories live removal:
        const Reference<container::XSet> xSet(
            that->getComponentContext()->getServiceManager(), UNO_QUERY_THROW );
        for ( t_stringlist::const_iterator iPos( implNames.begin() );
              iPos != implNames.end(); ++iPos )
        {
            OUString const & implName = *iPos;
            try {
                xSet->remove( Any(implName) );
            } // ignore if factory has not been live deployed:
            catch (container::NoSuchElementException &) {
            }
        }

        if (! singletons.empty())
        {
            // singletons live removal:
            const Reference<container::XNameContainer> xRootContext(
                that->getComponentContext()->getValueByName(
                    OUSTR("_root") ), UNO_QUERY );
            if (xRootContext.is())
            {
                for ( t_stringpairvec::const_iterator iPos(
                          singletons.begin() );
                      iPos != singletons.end(); ++iPos )
                {
                    ::std::pair<OUString, OUString> const & sp = *iPos;
                    const OUString name( OUSTR("/singletons/") + sp.first );
                    // arguments:
                    try {
                        xRootContext->removeByName( name + OUSTR("/arguments"));
                    }
                    catch (container::NoSuchElementException &) {}
                    // used service:
                    try {
                        xRootContext->removeByName( name + OUSTR("/service") );
                    }
                    catch (container::NoSuchElementException &) {}
                    // singleton entry:
                    try {
                        xRootContext->removeByName( name );
                    }
                    catch (container::NoSuchElementException & exc) {
                        (void) exc; // avoid warnings
                        OSL_ENSURE(
                            0, OUStringToOString(
                                exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
                    }
                }
            }
        }

        const Reference<registry::XSimpleRegistry> xServicesRDB( getRDB() );
        const Reference<registry::XImplementationRegistration> xImplReg(
            xContext->getServiceManager()->createInstanceWithContext(
                OUSTR("com.sun.star.registry.ImplementationRegistration"),
                xContext ), UNO_QUERY_THROW );
        xImplReg->revokeImplementation( url, xServicesRDB );

        if (isJavaTypelib)
            that->removeFromUnoRc( java, url, xCmdEnv );

        if (m_xRemoteContext.is()) {
            that->releaseObject( url );
            m_xRemoteContext.clear();
        }

        m_registered = REG_NOT_REGISTERED;
    }
}

//##############################################################################

// Package
//______________________________________________________________________________
beans::Optional< beans::Ambiguous<sal_Bool> >
BackendImpl::TypelibraryPackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard &,
    ::rtl::Reference<AbortChannel> const &,
    Reference<XCommandEnvironment> const & )
{
    BackendImpl * that = getMyBackend();
    return beans::Optional< beans::Ambiguous<sal_Bool> >(
        true /* IsPresent */,
        beans::Ambiguous<sal_Bool>(
            that->hasInUnoRc( m_jarFile, getURL() ),
            false /* IsAmbiguous */ ) );
}

//______________________________________________________________________________
void BackendImpl::TypelibraryPackageImpl::processPackage_(
    ::osl::ResettableMutexGuard &,
    bool doRegisterPackage,
    ::rtl::Reference<AbortChannel> const &,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    const OUString url( getURL() );

    if (doRegisterPackage)
    {
        // live insertion:
        if (m_jarFile) {
            // xxx todo add to classpath at runtime: ???
            //SB: It is probably not worth it to add the live inserted type
            // library JAR to the UnoClassLoader in the soffice process.  Any
            // live inserted component JAR that might reference this type
            // library JAR runs in its own uno process, so there is probably no
            // Java code in the soffice process that would see any UNO types
            // introduced by this type library JAR.
        }
        else // RDB:
        {
            Reference<XComponentContext> const & xContext =
                that->getComponentContext();
            if (! m_xTDprov.is())
            {
                m_xTDprov.set( that->getObject( url ), UNO_QUERY );
                if (! m_xTDprov.is())
                {
                    const Reference<registry::XSimpleRegistry> xReg(
                        xContext->getServiceManager()
                        ->createInstanceWithContext(
                            OUSTR("com.sun.star.registry.SimpleRegistry"),
                            xContext ), UNO_QUERY_THROW );
                    xReg->open( expandUnoRcUrl(url),
                                true /* read-only */, false /* ! create */ );
                    const Any arg(xReg);
                    Reference<container::XHierarchicalNameAccess> xTDprov(
                        xContext->getServiceManager()
                        ->createInstanceWithArgumentsAndContext(
                            OUSTR("com.sun.star.comp.stoc."
                                  "RegistryTypeDescriptionProvider"),
                            Sequence<Any>( &arg, 1 ), xContext ), UNO_QUERY );
                    OSL_ASSERT( xTDprov.is() );
                    if (xTDprov.is())
                        m_xTDprov.set( that->insertObject( url, xTDprov ),
                                       UNO_QUERY_THROW );
                }
            }
            if (m_xTDprov.is()) {
                Reference<container::XSet> xSet(
                    xContext->getValueByName(
                        OUSTR("/singletons/com.sun.star."
                              "reflection.theTypeDescriptionManager") ),
                    UNO_QUERY_THROW );
                xSet->insert( Any(m_xTDprov) );
            }
        }

        that->addToUnoRc( m_jarFile, url, xCmdEnv );
    }
    else // revokePackage()
    {
        that->removeFromUnoRc( m_jarFile, url, xCmdEnv );

        // revoking types at runtime, possible, sensible?
        if (!m_xTDprov.is())
            m_xTDprov.set( that->getObject( url ), UNO_QUERY );
        if (m_xTDprov.is()) {
            // remove live:
            const Reference<container::XSet> xSet(
                that->getComponentContext()->getValueByName(
                    OUSTR("/singletons/com.sun.star."
                          "reflection.theTypeDescriptionManager") ),
                UNO_QUERY_THROW );
            xSet->remove( Any(m_xTDprov) );

            that->releaseObject( url );
            m_xTDprov.clear();
        }
    }
}

} // anon namespace

namespace sdecl = comphelper::service_decl;
sdecl::class_<BackendImpl, sdecl::with_args<true> > serviceBI;
extern sdecl::ServiceDecl const serviceDecl(
    serviceBI,
    "com.sun.star.comp.deployment.component.PackageRegistryBackend",
    BACKEND_SERVICE_NAME );

} // namespace component
} // namespace backend
} // namespace dp_registry


