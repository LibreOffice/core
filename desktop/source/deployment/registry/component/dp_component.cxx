/*************************************************************************
 *
 *  $RCSfile: dp_component.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:10:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "dp_component.hrc"
#include "dp_backend.h"
#include "dp_ucb.h"
#include "rtl/string.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/uri.hxx"
#include "osl/module.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "ucbhelper/content.hxx"
#include "comphelper/anytostring.hxx"
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

typedef ::std::list< ::rtl::OUString > t_stringlist;
typedef ::std::vector< ::std::pair<OUString, OUString> > t_stringpairvec;

//==============================================================================
class BackendImpl : public ::dp_registry::backend::PackageRegistryBackend
{
protected:
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

    OUString m_strDynamicComponent;
    OUString m_strJavaComponent;
    OUString m_strPythonComponent;
    OUString m_strRDBTypelib;
    OUString m_strJavaTypelib;

public:
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
    bool hasInUnoRc( bool jarFile, OUString const & url,
                     Reference<XCommandEnvironment> const & xCmdEnv );
    bool removeFromUnoRc( bool jarFile, OUString const & url,
                          Reference<XCommandEnvironment> const & xCmdEnv );

    BackendImpl( Sequence<Any> const & args,
                 Reference<XComponentContext> const & xComponentContext,
                 OUString const & implName,
                 Sequence<OUString> const & supported_media_types );
};

//==============================================================================
class ComponentPackageImpl : public ::dp_registry::backend::Package
{
    OUString m_loader;
    Reference<XComponentContext> m_xRemoteContext;

    enum {
        REG_UNINIT, REG_VOID, REG_REGISTERED, REG_NOT_REGISTERED
    } m_registered;

    inline BackendImpl * getMyBackend() const {
        return static_cast<BackendImpl *>(m_myBackend.get());
    }

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

    Reference<registry::XSimpleRegistry> getRDB() const;

public:
    inline ComponentPackageImpl(
        ::rtl::Reference<PackageRegistryBackend> const & myBackend,
        OUString const & url, OUString const & mediaType,
        OUString const & name, OUString const & description,
        OUString const & loader )
        : Package( myBackend, url, mediaType,
                   name, name /* display-name */, description ),
          m_loader( loader ),
          m_registered( REG_UNINIT )
        {}

    // XPackage
    virtual Any SAL_CALL getIcon( sal_Bool highContrast, sal_Bool smallIcon )
        throw (RuntimeException);
};

//______________________________________________________________________________
Any ComponentPackageImpl::getIcon( sal_Bool highContrast, sal_Bool smallIcon )
    throw (RuntimeException)
{
    OSL_ASSERT( smallIcon );
    if (smallIcon) {
        sal_uInt16 ret;
        if (m_loader.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM("com.sun.star.loader.Java2") ))
            ret = highContrast
                ? RID_IMG_JAVA_COMPONENT_HC : RID_IMG_JAVA_COMPONENT;
        else
            ret = highContrast
                ? RID_IMG_COMPONENT_HC : RID_IMG_COMPONENT;
        return makeAny(ret);
    }
    return Package::getIcon( highContrast, smallIcon );
}

//______________________________________________________________________________
Reference<registry::XSimpleRegistry> ComponentPackageImpl::getRDB() const
{
    BackendImpl * that = getMyBackend();
    if (m_loader.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM("com.sun.star.loader.SharedLibrary") ))
        return that->m_xNativeRDB;
    else
        return that->m_xCommonRDB;
}

//______________________________________________________________________________
void ComponentPackageImpl::disposing()
{
    m_xRemoteContext.clear();
    Package::disposing();
}

//==============================================================================
class TypelibraryPackageImpl : public ::dp_registry::backend::Package
{
    bool m_jarFile;
    Reference<container::XHierarchicalNameAccess> m_xTDprov;

    inline BackendImpl * getMyBackend() const {
        return static_cast<BackendImpl *>(m_myBackend.get());
    }

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
        OUString const & url, OUString const & mediaType,
        OUString const & name, OUString const & description, bool jarFile )
        : Package( myBackend, url, mediaType, name, name /* display-name */,
                   description ),
          m_jarFile( jarFile )
        {}

    // XPackage
    virtual Any SAL_CALL getIcon( sal_Bool highContrast, sal_Bool smallIcon )
        throw (RuntimeException);
};

//______________________________________________________________________________
Any TypelibraryPackageImpl::getIcon( sal_Bool highContrast, sal_Bool smallIcon )
    throw (RuntimeException)
{
    OSL_ASSERT( smallIcon );
    if (smallIcon) {
        sal_uInt16 ret;
        if (m_jarFile)
            ret = highContrast
                ? RID_IMG_JAVA_TYPELIB_HC : RID_IMG_JAVA_TYPELIB;
        else
            ret = highContrast
                ? RID_IMG_TYPELIB_HC : RID_IMG_TYPELIB;
        return makeAny(ret);
    }
    return Package::getIcon( highContrast, smallIcon );
}

//______________________________________________________________________________
void TypelibraryPackageImpl::disposing()
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
    Reference<XComponentContext> const & xComponentContext,
    OUString const & implName,
    Sequence<OUString> const & supported_media_types )
    : PackageRegistryBackend(
        args, xComponentContext, implName, supported_media_types ),
      m_unorc_inited( false ),
      m_unorc_modified( false ),
      m_strDynamicComponent( getResourceString(RID_STR_DYN_COMPONENT) ),
      m_strJavaComponent( getResourceString(RID_STR_JAVA_COMPONENT) ),
      m_strPythonComponent( getResourceString(RID_STR_PYTHON_COMPONENT) ),
      m_strRDBTypelib( getResourceString(RID_STR_RDB_TYPELIB) ),
      m_strJavaTypelib( getResourceString(RID_STR_JAVA_TYPELIB) )
{
    Reference<XCommandEnvironment> xCmdEnv;

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
            ::ucb::Content cacheDir( getCachePath(), xCmdEnv );
            ::ucb::Content oldRDB;
            // switch common rdb:
            if (m_commonRDB.getLength() > 0)
                create_ucb_content(
                    &oldRDB, make_url( getCachePath(), m_commonRDB ),
                    xCmdEnv, false /* no throw */ );
            m_commonRDB = m_commonRDB.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM("common.rdb") )
                ? OUSTR("common_.rdb") : OUSTR("common.rdb");
            if (oldRDB.get().is())
            {
                if (! cacheDir.transferContent(
                        oldRDB, ::ucb::InsertOperation_COPY,
                        m_commonRDB, NameClash::OVERWRITE ))
                    throw RuntimeException(
                        OUSTR("UCB transferContent() failed!"), 0 );
                oldRDB = ::ucb::Content();
            }
            // switch native rdb:
            if (m_nativeRDB.getLength() > 0)
                create_ucb_content(
                    &oldRDB, make_url( getCachePath(), m_nativeRDB ),
                    xCmdEnv, false /* no throw */ );
            OUString plt_rdb( getPlatformString() + OUSTR(".rdb") );
            OUString plt_rdb_( getPlatformString() + OUSTR("_.rdb") );
            m_nativeRDB = m_nativeRDB.equals( plt_rdb ) ? plt_rdb_ : plt_rdb;
            if (oldRDB.get().is())
            {
                if (! cacheDir.transferContent(
                        oldRDB, ::ucb::InsertOperation_COPY,
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
                expand_url( make_url( getCachePath(), m_commonRDB ) ),
                m_readOnly, !m_readOnly );
        }
        if (m_nativeRDB.getLength() > 0) {
            m_xNativeRDB.set(
                xComponentContext->getServiceManager()
                ->createInstanceWithContext(
                    OUSTR("com.sun.star.registry.SimpleRegistry"),
                    xComponentContext ), UNO_QUERY_THROW );
            m_xNativeRDB->open(
                expand_url( make_url( getCachePath(), m_nativeRDB ) ),
                m_readOnly, !m_readOnly );
        }
    }
}

//==============================================================================
OUString SAL_CALL getImplementationName()
{
    return OUSTR(
        "com.sun.star.comp.deployment.component.PackageRegistryBackend");
}

//==============================================================================
Reference<XInterface> SAL_CALL create(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xComponentContext )
{
    OUString const mediaTypes [] = {
        OUSTR("application/vnd.sun.star.uno-component"),
        OUSTR("application/vnd.sun.star.uno-typelibrary")
    };
    return static_cast< ::cppu::OWeakObject * >(
        new BackendImpl(
            args, xComponentContext, getImplementationName(),
            Sequence<OUString>( mediaTypes, ARLEN(mediaTypes) ) ) );
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
        ::ucb::Content ucbContent;
        if (create_ucb_content( &ucbContent, url, xCmdEnv )) {
            OUString title(
                extract_throw<OUString>(
                    ucbContent.getPropertyValue( OUSTR("Title") ) ) );
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
                // read .jar manifest file:
                ::rtl::OUStringBuffer buf;
                buf.appendAscii(
                    RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.zip://") );
                buf.append( ::rtl::Uri::encode( url,
                                                rtl_UriCharClassRegName,
                                                rtl_UriEncodeIgnoreEscapes,
                                                RTL_TEXTENCODING_UTF8 ) );
                buf.appendAscii(
                    RTL_CONSTASCII_STRINGPARAM("/META-INF/MANIFEST.MF") );
                ::ucb::Content manifestContent;
                if (create_ucb_content( &manifestContent,
                                        buf.makeStringAndClear(),
                                        xCmdEnv, false /* no throw */ ))
                {
                    OUString line;
                    if (readLine( &line, OUSTR("RegistrationClassName"),
                                  manifestContent, RTL_TEXTENCODING_ASCII_US ))
                        mediaType = OUSTR(
                            "application/vnd.sun.star.uno-component;type=Java");
                }
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
                m_strCannotDetectMediaType + url,
                static_cast<OWeakObject *>(this), static_cast<sal_Int16>(-1) );
    }

    String type, subType;
    INetContentTypeParameterList params;
    if (INetContentTypes::parse( mediaType, type, subType, &params ))
    {
        if (type.EqualsIgnoreCaseAscii("application"))
        {
            ::ucb::Content ucbContent( url, xCmdEnv );
            OUString name(
                extract_throw<OUString>(
                    ucbContent.getPropertyValue( OUSTR("Title") ) ) );

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
                            return new ComponentPackageImpl(
                                this, url, mediaType, name,
                                m_strDynamicComponent,
                                OUSTR("com.sun.star.loader.SharedLibrary") );
                        }
                        if (value.EqualsIgnoreCaseAscii("Java")) {
                            return new ComponentPackageImpl(
                                this, url, mediaType, name,
                                m_strJavaComponent,
                                OUSTR("com.sun.star.loader.Java2") );
                        }
                        if (value.EqualsIgnoreCaseAscii("Python")) {
                            return new ComponentPackageImpl(
                                this, url, mediaType, name,
                                m_strPythonComponent,
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
                        return new TypelibraryPackageImpl(
                            this, url, mediaType, name,
                            m_strRDBTypelib, false /* rdb */ );
                    }
                    if (value.EqualsIgnoreCaseAscii("Java")) {
                        return new TypelibraryPackageImpl(
                            this, url, mediaType, name,
                            m_strJavaTypelib, true /* jar */ );
                    }
                }
            }
        }
    }
    throw lang::IllegalArgumentException(
        m_strUnsupportedMediaType + mediaType,
        static_cast<OWeakObject *>(this),
        static_cast<sal_Int16>(-1) );
}

//##############################################################################

//------------------------------------------------------------------------------
static inline OUString encodeForRcFile( OUString const & str )
{
    // escape $\{} (=> rtl bootstrap files)
    ::rtl::OUStringBuffer buf;
    sal_Int32 pos = 0;
    sal_Int32 len = str.getLength();
    for ( ; pos < len; ++pos ) {
        sal_Unicode c = str[ pos ];
        switch (c) {
        case '$':
        case '\\':
        case '{':
        case '}':
            buf.append( static_cast< sal_Unicode >('\\') );
            break;
        }
        buf.append( c );
    }
    return buf.makeStringAndClear();
}

//------------------------------------------------------------------------------
static inline OUString decodeRcEntry( OUString const & str )
{
    ::rtl::OUStringBuffer buf;
    sal_Int32 pos = 0;
    sal_Int32 len = str.getLength();
    for ( ; pos < len; ++pos ) {
        sal_Unicode c = str[ pos ];
        if (c == '\\') {
            ++pos;
            if (pos < len)
                c = str[ pos ];
        }
        buf.append( c );
    }
    return buf.makeStringAndClear();
}

//______________________________________________________________________________
void BackendImpl::unorc_verify_init(
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    if (transientMode())
        return;
    ::osl::MutexGuard guard( getMutex() );
    if (! m_unorc_inited)
    {
        // common rc:
        ::ucb::Content ucb_content;
        if (create_ucb_content(
                &ucb_content,
                make_url( getCachePath(), OUSTR("unorc") ),
                xCmdEnv, false /* no throw */ ))
        {
            OUString line;
            if (readLine( &line, OUSTR("UNO_JAVA_CLASSPATH="), ucb_content,
                          RTL_TEXTENCODING_UTF8 ))
            {
                sal_Int32 index = sizeof ("UNO_JAVA_CLASSPATH=") - 1;
                do {
                    OUString token( line.getToken( 0, ' ', index ).trim() );
                    if (token.getLength() > 0 &&
                        // cleanup, check if existing:
                        create_ucb_content( 0, decodeRcEntry(token), xCmdEnv,
                                            false /* no throw */ )) {
                        m_jar_typelibs.push_back( token );
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
                                0, decodeRcEntry(token),
                                xCmdEnv, false /* no throw */ )) {
                            m_rdb_typelibs.push_back( token );
                        }
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
                    make_url( getCachePath(),
                              getPlatformString() + OUSTR("rc") ),
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
    if (! m_jar_typelibs.empty())
    {
        t_stringlist::const_iterator iPos( m_jar_typelibs.begin() );
        t_stringlist::const_iterator const iEnd( m_jar_typelibs.end() );
        buf.append( RTL_CONSTASCII_STRINGPARAM("UNO_JAVA_CLASSPATH=") );
        while (iPos != iEnd) {
            // encoded ASCII file-urls:
            ::rtl::OString item(
                ::rtl::OUStringToOString( *iPos, RTL_TEXTENCODING_ASCII_US ) );
            buf.append( item );
            ++iPos;
            if (iPos != iEnd)
                buf.append( ' ' );
        }
        buf.append( LF );
    }
    if (! m_rdb_typelibs.empty())
    {
        t_stringlist::const_iterator iPos( m_rdb_typelibs.begin() );
        t_stringlist::const_iterator const iEnd( m_rdb_typelibs.end() );
        buf.append( RTL_CONSTASCII_STRINGPARAM("UNO_TYPES=") );
        while (iPos != iEnd) {
            buf.append( '?' );
            // encoded ASCII file-urls:
            ::rtl::OString item(
                ::rtl::OUStringToOString( *iPos, RTL_TEXTENCODING_ASCII_US ) );
            buf.append( item );
            ++iPos;
            if (iPos != iEnd)
                buf.append( ' ' );
        }
        buf.append( LF );
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
            buf.append( LF );

            // write native rc:
            ::rtl::OStringBuffer buf2;
            buf2.append( RTL_CONSTASCII_STRINGPARAM("UNO_SERVICES=?$ORIGIN/") );
            buf2.append( ::rtl::OUStringToOString(
                             m_nativeRDB, RTL_TEXTENCODING_ASCII_US ) );
            buf2.append( LF );

            Reference<io::XInputStream> xData(
                ::xmlscript::createInputStream(
                    ::rtl::ByteSequence(
                        reinterpret_cast<sal_Int8 const *>(buf2.getStr()),
                        buf2.getLength() ) ) );
            ::ucb::Content ucb_content(
                make_url( getCachePath(),
                          getPlatformString() + OUSTR("rc") ), xCmdEnv );
            ucb_content.writeStream( xData, true /* replace existing */ );
        }
    }

    // write unorc:
    Reference<io::XInputStream> xData(
        ::xmlscript::createInputStream(
            ::rtl::ByteSequence(
                reinterpret_cast<sal_Int8 const *>(buf.getStr()),
                buf.getLength() ) ) );
    ::ucb::Content ucb_content(
        make_url( getCachePath(), OUSTR("unorc") ), xCmdEnv );
    ucb_content.writeStream( xData, true /* replace existing */ );

    m_unorc_modified = false;
}

//______________________________________________________________________________
bool BackendImpl::addToUnoRc( bool jarFile, OUString const & rcurl,
                              Reference<XCommandEnvironment> const & xCmdEnv )
{
    ::osl::MutexGuard guard( getMutex() );
    unorc_verify_init( xCmdEnv );
    t_stringlist & rSet = getTypelibs(jarFile);
    if (::std::find( rSet.begin(), rSet.end(), rcurl ) == rSet.end()) {
        rSet.push_front( rcurl ); // prepend to list, thus overriding
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
    bool jarFile, OUString const & rcurl,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    ::osl::MutexGuard guard( getMutex() );
    unorc_verify_init( xCmdEnv );
    getTypelibs(jarFile).remove( rcurl );
    // write immediately:
    m_unorc_modified = true;
    unorc_flush( xCmdEnv );
    return true;
}

//______________________________________________________________________________
bool BackendImpl::hasInUnoRc(
    bool jarFile, OUString const & url,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    ::osl::MutexGuard guard( getMutex() );
    t_stringlist const & rSet = getTypelibs(jarFile);
    return ::std::find( rSet.begin(), rSet.end(), url ) != rSet.end();
}

//______________________________________________________________________________
void BackendImpl::releaseObject( OUString const & id )
{
    ::osl::MutexGuard guard( getMutex() );
    ::std::size_t erased = m_backendObjects.erase( id );
    OSL_ASSERT( erased == 1 );
}

//______________________________________________________________________________
Reference<XInterface> BackendImpl::getObject( OUString const & id )
{
    ::osl::MutexGuard guard( getMutex() );
    t_string2object::const_iterator const iFind( m_backendObjects.find( id ) );
    if (iFind == m_backendObjects.end())
        return Reference<XInterface>();
    else
        return iFind->second;
}

//______________________________________________________________________________
Reference<XInterface> BackendImpl::insertObject(
    OUString const & id, Reference<XInterface> const & xObject )
{
    ::osl::MutexGuard guard( getMutex() );
    ::std::pair<t_string2object::iterator, bool> insertion(
        m_backendObjects.insert( t_string2object::value_type(
                                     id, xObject ) ) );
    return insertion.first->second;
}

//------------------------------------------------------------------------------
static Reference<XComponentContext> raise_uno_process(
    Reference<XComponentContext> const & xContext,
    ::rtl::Reference<AbortChannel> const & abortChannel )
{
    static OUString s_programDir;
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if (s_programDir.getLength() == 0) {
            ::osl::Module::getUrlFromAddress( (void *) raise_uno_process,
                                              s_programDir );
            s_programDir = s_programDir.copy(
                0, s_programDir.lastIndexOf( '/' ) );
        }
    }

    ::rtl::OUStringBuffer buf;
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("uno:pipe,name=") );
    OUString pipeId( generateRandomPipeId() );
    buf.append( pipeId );
    buf.appendAscii(
        RTL_CONSTASCII_STRINGPARAM(";urp;uno.ComponentContext") );
    OUString connectStr( buf.makeStringAndClear() );

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
    OSL_ASSERT( buf.getLength() == 0 );
    // use only preinstalled services:
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("-env:UNO_SERVICES=") );
    buf.append( s_programDir );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("/services.rdb") );
    args.push_back( buf.makeStringAndClear() );

    oslProcess hProcess = raiseProcess(
        s_programDir + OUSTR("/uno"),
        Sequence<OUString>( &args[ 0 ], args.size() ) );
    try {
        return Reference<XComponentContext>(
            resolveUnoURL( connectStr, xContext, abortChannel.get() ),
            UNO_QUERY_THROW );
    }
    catch (...) {
        // try to terminate process:
        oslProcessError rc = osl_terminateProcess( hProcess );
        OSL_ASSERT( rc == osl_Process_E_None );
        throw;
    }
}

//------------------------------------------------------------------------------
Reference<loader::XImplementationLoader>
ComponentPackageImpl::getComponentInfo(
    t_stringlist * pImplNames, t_stringpairvec * pSingletons,
    Reference<XComponentContext> const & xContext )
{
    Reference<loader::XImplementationLoader> xLoader(
        xContext->getServiceManager()->createInstanceWithContext(
            m_loader, xContext ), UNO_QUERY );
    if (! xLoader.is())
        return Reference<loader::XImplementationLoader>();

    // HACK: highly dependent on stoc/source/servicemanager
    //       and stoc/source/implreg implementation which rely on the same
    //       services.rdb format!

    Reference<registry::XSimpleRegistry> xMemReg(
        xContext->getServiceManager()->createInstanceWithContext(
            OUSTR("com.sun.star.registry.SimpleRegistry"), xContext ),
        UNO_QUERY_THROW );
    xMemReg->open( OUString() /* in mem */, false, true );
    xLoader->writeRegistryInfo( xMemReg->getRootKey(), OUString(), getURL() );

    Sequence< Reference<registry::XRegistryKey> > keys(
        xMemReg->getRootKey()->openKeys() );
    Reference<registry::XRegistryKey> const * pkeys = keys.getConstArray();
    for ( sal_Int32 pos = keys.getLength(); pos--; )
    {
        Reference<registry::XRegistryKey> const & xImplKey = pkeys[ pos ];
        OUString implName( xImplKey->getKeyName().copy( 1 /*leading slash*/ ) );
        pImplNames->push_back( implName );

        // check for singletons:
        Reference<registry::XRegistryKey> xSingletonKey(
            xImplKey->openKey( OUSTR("UNO/SINGLETONS") ) );
        if (xSingletonKey.is() && xSingletonKey->isValid())
        {
            Sequence< Reference<registry::XRegistryKey> > singletonKeys(
                xSingletonKey->openKeys() );
            Reference<registry::XRegistryKey> const * psingletonKeys =
                singletonKeys.getConstArray();
            for ( sal_Int32 i = singletonKeys.getLength(); i--; )
            {
                Reference<registry::XRegistryKey> const & xSingleton =
                    psingletonKeys[ i ];
                pSingletons->push_back(
                    ::std::pair<OUString, OUString>(
                        xSingleton->getKeyName().copy(
                            implName.getLength() +
                            sizeof ("//UNO/SINGLETONS/") - 1 ),
                        xSingleton->getStringValue() ) );
            }
        }
    }

    return xLoader;
}

// Package
//______________________________________________________________________________
beans::Optional< beans::Ambiguous<sal_Bool> >
ComponentPackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard & guard,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    if (m_registered == REG_UNINIT)
    {
        m_registered = REG_NOT_REGISTERED;
        Reference<registry::XSimpleRegistry> xRDB( getRDB() );
        if (xRDB.is())
        {
            // lookup rdb for location URL:
            Reference<registry::XRegistryKey> xRootKey( xRDB->getRootKey() );
            Reference<registry::XRegistryKey> xImplKey(
                xRootKey->openKey( OUSTR("IMPLEMENTATIONS") ) );
            Sequence<OUString> implNames;
            if (xImplKey.is() && xImplKey->isValid())
                implNames = xImplKey->getKeyNames();
            OUString const * pImplNames = implNames.getConstArray();
            sal_Int32 pos = implNames.getLength();
            for ( ; pos--; )
            {
                checkAborted( abortChannel );
                OUString key( pImplNames[ pos ] + OUSTR("/UNO/LOCATION") );
                Reference<registry::XRegistryKey> xKey(
                    xRootKey->openKey(key) );
                if (xKey.is() && xKey->isValid()) {
                    OUString location( xKey->getAsciiValue() );
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
void ComponentPackageImpl::processPackage_(
    ::osl::ResettableMutexGuard & guard,
    bool registerPackage,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    bool java = m_loader.equalsAsciiL(
        RTL_CONSTASCII_STRINGPARAM("com.sun.star.loader.Java2") );
    OUString url( getURL() );

    m_registered = REG_VOID;
    if (registerPackage)
    {
        if (java) // xxx todo: add to CLASSPATH until we have an
                  // own extendable classloader, the sandbox
                  // classloader is insufficient, because the bridge
                  // and class com.sun.star.uno.Type load classes
                  // using a different classloader, so add first before
                  // raising the uno process
            that->addToUnoRc( java, encodeForRcFile(url), xCmdEnv );

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

        Reference<registry::XSimpleRegistry> xServicesRDB( getRDB() );
        Reference<registry::XImplementationRegistration> xImplReg(
            m_xRemoteContext->getServiceManager()->createInstanceWithContext(
                OUSTR("com.sun.star.registry.ImplementationRegistration"),
                m_xRemoteContext ), UNO_QUERY_THROW );
        xImplReg->registerImplementation( m_loader, url, xServicesRDB );

        t_stringlist implNames;
        t_stringpairvec singletons;
        Reference<loader::XImplementationLoader> xLoader(
            getComponentInfo( &implNames, &singletons, m_xRemoteContext ) );

        // factories live insertion:
        Reference<container::XSet> xSet(
            that->getComponentContext()->getServiceManager(), UNO_QUERY_THROW );
        for ( t_stringlist::const_iterator iPos( implNames.begin() );
              iPos != implNames.end(); ++iPos )
        {
            checkAborted( abortChannel );
            OUString const & implName = *iPos;
            // activate factory:
            Reference<XInterface> xFactory(
                xLoader->activate(
                    implName, OUString(), url,
                    xServicesRDB->getRootKey()->openKey(
                        OUSTR("/IMPLEMENTATIONS/") + implName ) ) );
            try {
                xSet->insert( makeAny(xFactory) );
            } // ignore if factory has already been inserted:
            catch (container::ElementExistException &) {
                OSL_ENSURE( 0, "### factory already registered?" );
            }
        }

        if (! singletons.empty())
        {
            // singletons live insertion:
            Reference<container::XNameContainer> xRootContext(
                that->getComponentContext()->getValueByName(
                    OUSTR("_root") ), UNO_QUERY );
            if (xRootContext.is())
            {
                for ( t_stringpairvec::const_iterator iPos(
                          singletons.begin() );
                      iPos != singletons.end(); ++iPos )
                {
                    ::std::pair<OUString, OUString> const & sp = *iPos;
                    OUString name( OUSTR("/singletons/") + sp.first );
                    // assure no arguments:
                    try {
                        xRootContext->removeByName( name + OUSTR("/arguments"));
                    } catch (container::NoSuchElementException &) {}
                    // used service:
                    try {
                        xRootContext->insertByName(
                            name + OUSTR("/service"), makeAny(sp.second) );
                    } catch (container::ElementExistException &) {
                        xRootContext->replaceByName(
                            name + OUSTR("/service"), makeAny(sp.second) );
                    }
                    // singleton entry:
                    try {
                        xRootContext->insertByName( name, Any() );
                    } catch (container::ElementExistException & exc) {
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
    else
    { // revokePackage()
        Reference<XComponentContext> xContext;
        if (m_xRemoteContext.is()) // has been activated in this process
            xContext = m_xRemoteContext;
        else // has been deployed in former times
            xContext = that->getComponentContext();

        t_stringlist implNames;
        t_stringpairvec singletons;
        getComponentInfo( &implNames, &singletons, xContext );

        // factories live removal:
        Reference<container::XSet> xSet(
            that->getComponentContext()->getServiceManager(), UNO_QUERY_THROW );
        for ( t_stringlist::const_iterator iPos( implNames.begin() );
              iPos != implNames.end(); ++iPos )
        {
            OUString const & implName = *iPos;
            try {
                xSet->remove( makeAny(implName) );
            } // ignore if factory has not been live deployed:
            catch (container::NoSuchElementException &) {
            }
        }

        if (! singletons.empty())
        {
            // singletons live removal:
            Reference<container::XNameContainer> xRootContext(
                that->getComponentContext()->getValueByName(
                    OUSTR("_root") ), UNO_QUERY );
            if (xRootContext.is())
            {
                for ( t_stringpairvec::const_iterator iPos(
                          singletons.begin() );
                      iPos != singletons.end(); ++iPos )
                {
                    ::std::pair<OUString, OUString> const & sp = *iPos;
                    OUString name( OUSTR("/singletons/") + sp.first );
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
                        OSL_ENSURE(
                            0, OUStringToOString(
                                exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
                    }
                }
            }
        }

        Reference<registry::XSimpleRegistry> xServicesRDB( getRDB() );
        Reference<registry::XImplementationRegistration> xImplReg(
            xContext->getServiceManager()->createInstanceWithContext(
                OUSTR("com.sun.star.registry.ImplementationRegistration"),
                xContext ), UNO_QUERY_THROW );
        xImplReg->revokeImplementation( url, xServicesRDB );

        if (java) // xxx todo: add to CLASSPATH until we have an
                  // own extendable classloader, the sandbox
                  // classloader is insufficient, because the bridge
                  // and class com.sun.star.uno.Type load classes
                  // using a different classloader
            that->removeFromUnoRc( java, encodeForRcFile(url), xCmdEnv );

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
TypelibraryPackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard & guard,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    return beans::Optional< beans::Ambiguous<sal_Bool> >(
        true /* IsPresent */,
        beans::Ambiguous<sal_Bool>(
            that->hasInUnoRc( m_jarFile, encodeForRcFile( getURL() ), xCmdEnv ),
            false /* IsAmbiguous */ ) );
}

//______________________________________________________________________________
void TypelibraryPackageImpl::processPackage_(
    ::osl::ResettableMutexGuard & guard,
    bool registerPackage,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    OUString url( getURL() );

    if (registerPackage)
    {
        // live insertion:
        if (m_jarFile) {
            // xxx todo add to classpath at runtime: ???
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
                    Reference<registry::XSimpleRegistry> xReg(
                        xContext->getServiceManager()
                        ->createInstanceWithContext(
                            OUSTR("com.sun.star.registry.SimpleRegistry"),
                            xContext ), UNO_QUERY_THROW );
                    xReg->open( expand_url(url),
                                true /* read-only */, false /* ! create */ );
                    Any arg( makeAny(xReg) );
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
                xSet->insert( makeAny(m_xTDprov) );
            }
        }

        that->addToUnoRc( m_jarFile, encodeForRcFile(url), xCmdEnv );
    }
    else // revokePackage()
    {
        that->removeFromUnoRc(
            m_jarFile, encodeForRcFile(url), xCmdEnv );

        // revoking types at runtime, possible, sensible?
        if (m_xTDprov.is()) {
            // remove live:
            Reference<container::XSet> xSet(
                that->getComponentContext()->getValueByName(
                    OUSTR("/singletons/com.sun.star."
                          "reflection.theTypeDescriptionManager") ),
                UNO_QUERY_THROW );
            xSet->remove( makeAny(m_xTDprov) );

            that->releaseObject( url );
            m_xTDprov.clear();
        }
    }
}

} // namespace component
} // namespace backend
} // namespace dp_registry

