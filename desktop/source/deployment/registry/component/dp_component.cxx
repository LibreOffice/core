/*************************************************************************
 *
 *  $RCSfile: dp_component.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-11 12:13:15 $
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
#include "dp_persmap.h"
#include "rtl/string.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/uri.hxx"
#include "osl/file.hxx"
#include "osl/module.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "ucbhelper/content.hxx"
#include "comphelper/anytostring.hxx"
#include "xmlscript/xml_helper.hxx"
#include "svtools/inettype.hxx"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/container/XSet.hpp"
#include "com/sun/star/beans/StringPair.hpp"
#include "com/sun/star/registry/XSimpleRegistry.hpp"
#include "com/sun/star/registry/XImplementationRegistration.hpp"
#include "com/sun/star/loader/XImplementationLoader.hpp"
#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/sdbc/XResultSet.hpp"
#include "com/sun/star/sdbc/XRow.hpp"
#include "com/sun/star/ucb/NameClash.hpp"
#include "com/sun/star/container/XSet.hpp"
#include <hash_set>
#include <hash_map>
#include <vector>
#include <memory>
#include <algorithm>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;
using ::osl::File;

namespace dp_registry
{
namespace backend
{
namespace component
{

typedef ::std::hash_set<
    ::rtl::OUString, ::rtl::OUStringHash > t_stringset;
typedef ::std::vector<beans::StringPair> t_stringpairvec;

struct ComponentInfo
{
    t_stringset m_implNames;
    t_stringpairvec m_singletons;
};

//==============================================================================
class BackendImpl : public ::dp_registry::backend::PackageRegistryBackend
{
protected:
    t_stringset m_jar_typelibs;
    t_stringset m_rdb_typelibs;
    t_stringset & getTypelibs( bool jar )
        { return jar ? m_jar_typelibs : m_rdb_typelibs; }

    bool m_unorc_inited;
    bool m_unorc_modified;

    OUString m_inflated_dir, m_inflated_reg_dir;
    ::std::auto_ptr<PersistentMap> m_urlToTemp;

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

    bool getInflatedCopy(
        OUString & inflatedURL /* init: inout */,
        OUString const & url,
        Reference<XCommandEnvironment> const & xCmdEnv,
        bool create = true );
    void removeInflatedCopy(
        OUString const & url,
        Reference<XCommandEnvironment> const & xCmdEnv );

    bool addToUnoRc(
        bool jarFile, OUString const & url,
        Reference<XCommandEnvironment> const & xCmdEnv );
    bool hasInUnoRc(
        bool jarFile, OUString const & url,
        Reference<XCommandEnvironment> const & xCmdEnv );
    bool removeFromUnoRc(
        bool jarFile, OUString const & url,
        Reference<XCommandEnvironment> const & xCmdEnv );

    inline BackendImpl(
        Reference<XComponentContext> const & xComponentContext,
        OUString const & implName,
        Sequence<OUString> const & supported_media_types )
        : PackageRegistryBackend(
            xComponentContext, implName, supported_media_types ),
          m_unorc_inited( false ),
          m_unorc_modified( false ),
          m_strDynamicComponent( getResourceString(RID_STR_DYN_COMPONENT) ),
          m_strJavaComponent( getResourceString(RID_STR_JAVA_COMPONENT) ),
          m_strPythonComponent( getResourceString(RID_STR_PYTHON_COMPONENT) ),
          m_strRDBTypelib( getResourceString(RID_STR_RDB_TYPELIB) ),
          m_strJavaTypelib( getResourceString(RID_STR_JAVA_TYPELIB) )
        {}

    // XInitialization
    virtual void SAL_CALL initialize( Sequence<Any> const & args )
        throw (Exception);
};

//==============================================================================
class ComponentPackageImpl : public ::dp_registry::backend::Package
{
    OUString m_inflated_url;
    OUString m_loader;
    Reference<XComponentContext> m_xRemoteContext;

    enum { REG_UNINIT, REG_VOID, REG_REGISTERED, REG_NOT_REGISTERED }
    m_registered;

    inline BackendImpl * getMyBackend() const
        { return static_cast<BackendImpl *>(m_myBackend.get()); }

    virtual void SAL_CALL disposing();

    // Package
    virtual bool isRegistered_(
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
    if (smallIcon)
    {
        sal_uInt16 ret;
        if (m_loader.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM("com.sun.star.loader.Java2") ))
        {
            ret = highContrast
                ? RID_IMG_JAVA_COMPONENT_HC : RID_IMG_JAVA_COMPONENT;
        }
        else
        {
            ret = highContrast
                ? RID_IMG_COMPONENT_HC : RID_IMG_COMPONENT;
        }
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
    OUString m_inflated_url;
    bool m_jarFile;
    Reference<container::XHierarchicalNameAccess> m_xTDprov;

    inline BackendImpl * getMyBackend() const
        { return static_cast<BackendImpl *>(m_myBackend.get()); }

    virtual void SAL_CALL disposing();

    // Package
    virtual bool isRegistered_(
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
    if (smallIcon)
    {
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
    try
    {
        m_backendObjects = t_string2object();
        if (m_xNativeRDB.is())
        {
            m_xNativeRDB->close();
            m_xNativeRDB.clear();
        }
        if (m_xCommonRDB.is())
        {
            m_xCommonRDB->close();
            m_xCommonRDB.clear();
        }
        m_urlToTemp.reset(0);
        unorc_flush( Reference<XCommandEnvironment>() );

        PackageRegistryBackend::disposing();
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException(
            OUSTR("caught unexpected exception while disposing..."),
            static_cast<OWeakObject *>(this), exc );
    }
}

struct predicate_equalsIgnoreAsciiCase
{
    OUString m_str;
    predicate_equalsIgnoreAsciiCase( OUString const & str ) : m_str( str ) {}
    bool operator () ( t_string2string_map::value_type const & v ) const
        { return v.second.equalsIgnoreAsciiCase( m_str ); }
};

//______________________________________________________________________________
void BackendImpl::initialize( Sequence<Any> const & args )
    throw (Exception)
{
    PackageRegistryBackend::initialize( args );
    Reference<XCommandEnvironment> xCmdEnv;

    if (transientMode())
    {
        // in-mem rdbs:
        // common rdb for java, native rdb for shared lib components
        m_xCommonRDB.set(
            getComponentContext()->getServiceManager()
            ->createInstanceWithContext(
                OUSTR("com.sun.star.registry.SimpleRegistry"),
                getComponentContext() ), UNO_QUERY_THROW );
        m_xCommonRDB->open( OUString() /* in-mem */,
                            false /* ! read-only */, true /* create */ );
        m_xNativeRDB.set(
            getComponentContext()->getServiceManager()
            ->createInstanceWithContext(
                OUSTR("com.sun.star.registry.SimpleRegistry"),
                getComponentContext() ), UNO_QUERY_THROW );
        m_xNativeRDB->open( OUString() /* in-mem */,
                            false /* ! read-only */, true /* create */ );
        // inflated in-mem DB:
        m_urlToTemp.reset( new PersistentMap );
    }
    else
    {
        unorc_verify_init( xCmdEnv );

        if (! m_readOnly)
        {
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
        if (m_commonRDB.getLength() > 0)
        {
            m_xCommonRDB.set(
                getComponentContext()->getServiceManager()
                ->createInstanceWithContext(
                    OUSTR("com.sun.star.registry.SimpleRegistry"),
                    getComponentContext() ), UNO_QUERY_THROW );
            m_xCommonRDB->open(
                expand_url( make_url( getCachePath(), m_commonRDB ) ),
                m_readOnly, !m_readOnly );
        }
        if (m_nativeRDB.getLength() > 0)
        {
            m_xNativeRDB.set(
                getComponentContext()->getServiceManager()
                ->createInstanceWithContext(
                    OUSTR("com.sun.star.registry.SimpleRegistry"),
                    getComponentContext() ), UNO_QUERY_THROW );
            m_xNativeRDB->open(
                expand_url( make_url( getCachePath(), m_nativeRDB ) ),
                m_readOnly, !m_readOnly );
        }

        // ensure "inflated" folder:
        m_inflated_reg_dir = make_url( getCachePath(), OUSTR("inflated") );
        ::ucb::Content inflated_dir_content;
        create_folder( &inflated_dir_content, m_inflated_reg_dir,
                       Reference<XCommandEnvironment>(), !m_readOnly );
        m_inflated_dir = expand_url( m_inflated_reg_dir );

        // scan for zombie temp dirs:
        OUString inflated_db_url(
            make_url( getCachePath(), OUSTR("inflated.db") ) );
        m_urlToTemp.reset( new PersistentMap( inflated_db_url, m_readOnly ) );

        if (! m_readOnly)
        {
            t_string2string_map urlToTemp( m_urlToTemp->getEntries() );

            OUString strTitle = OUSTR("Title");
            Reference<sdbc::XResultSet> xResultSet(
                inflated_dir_content.createCursor(
                    Sequence<OUString>( &strTitle, 1 ),
                    ::ucb::INCLUDE_DOCUMENTS_ONLY ) );
            // get all inflated temp directories:
            ::std::vector<OUString> tempEntries;
            while (xResultSet->next())
            {
                OUString title(
                    Reference<sdbc::XRow>(
                        xResultSet, UNO_QUERY_THROW )->getString(
                            1 /* Title */ ) );
                tempEntries.push_back( ::rtl::Uri::encode(
                                           title, rtl_UriCharClassPchar,
                                           rtl_UriEncodeIgnoreEscapes,
                                           RTL_TEXTENCODING_UTF8 ) );
            }

            for ( ::std::size_t pos = 0; pos < tempEntries.size(); ++pos )
            {
                OUString const & tempEntry = tempEntries[ pos ];
                if (::std::find_if(
                        urlToTemp.begin(), urlToTemp.end(),
                        predicate_equalsIgnoreAsciiCase( tempEntry ) )
                    == urlToTemp.end())
                {
                    // temp entry not needed anymore:
                    OUString url( make_url( m_inflated_reg_dir, tempEntry ) );
                    erase_path( url,
                                Reference<XCommandEnvironment>(),
                                false /* no throw: ignore errors */ );
                    erase_path( url + OUSTR("_"),
                                Reference<XCommandEnvironment>(),
                                false /* no throw: ignore errors */ );
                }
                OSL_ASSERT( ! m_urlToTemp->has( tempEntry ) );
            }
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
    Reference<XComponentContext> const & xComponentContext )
    SAL_THROW( (Exception) )
{
    OUString const mediaTypes [] = {
        OUSTR("application/vnd.sun.star.uno-component"),
        OUSTR("application/vnd.sun.star.uno-typelibrary")
    };
    return static_cast< ::cppu::OWeakObject * >(
        new BackendImpl(
            xComponentContext, getImplementationName(),
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
        if (create_ucb_content( &ucbContent, url, xCmdEnv ))
        {
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
                    RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.pkg://") );
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
                if (param == 0 || platform_fits( param->m_sValue ))
                {
                    param = params.find( ByteString("type") );
                    if (param != 0)
                    {
                        String const & value = param->m_sValue;
                        if (value.EqualsIgnoreCaseAscii("native"))
                        {
                            return new ComponentPackageImpl(
                                this, url, mediaType, name,
                                m_strDynamicComponent,
                                OUSTR("com.sun.star.loader.SharedLibrary") );
                        }
                        if (value.EqualsIgnoreCaseAscii("Java"))
                        {
                            return new ComponentPackageImpl(
                                this, url, mediaType, name,
                                m_strJavaComponent,
                                OUSTR("com.sun.star.loader.Java2") );
                        }
                        if (value.EqualsIgnoreCaseAscii("Python"))
                        {
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
                if (param != 0)
                {
                    String const & value = param->m_sValue;
                    if (value.EqualsIgnoreCaseAscii("RDB"))
                    {
                        return new TypelibraryPackageImpl(
                            this, url, mediaType, name,
                            m_strRDBTypelib, false /* rdb */ );
                    }
                    if (value.EqualsIgnoreCaseAscii("Java"))
                    {
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
    for ( ; pos < len; ++pos )
    {
        sal_Unicode c = str[ pos ];
        switch (c)
        {
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
    for ( ; pos < len; ++pos )
    {
        sal_Unicode c = str[ pos ];
        if (c == '\\')
        {
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
                do
                {
                    OUString token( line.getToken( 0, ' ', index ).trim() );
                    if (token.getLength() > 0 &&
                        // cleanup, check if existing:
                        create_ucb_content( 0, decodeRcEntry(token), xCmdEnv,
                                            false /* no throw */ ))
                    {
                        m_jar_typelibs.insert( token );
                    }
                }
                while (index >= 0);
            }
            if (readLine( &line, OUSTR("UNO_TYPES="), ucb_content,
                          RTL_TEXTENCODING_UTF8 ))
            {
                sal_Int32 index = sizeof ("UNO_TYPES=") - 1;
                do
                {
                    OUString token( line.getToken( 0, ' ', index ).trim() );
                    if (token.getLength() > 0)
                    {
                        if (token[ 0 ] == '?')
                            token = token.copy( 1 );
                        // cleanup, check if existing:
                        if (create_ucb_content( 0, decodeRcEntry(token),
                                                xCmdEnv, false /* no throw */ ))
                        {
                            m_rdb_typelibs.insert( token );
                        }
                    }
                }
                while (index >= 0);
            }
            if (readLine( &line, OUSTR("UNO_SERVICES="), ucb_content,
                          RTL_TEXTENCODING_UTF8 ))
            {
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
                    xCmdEnv, false /* no throw */ ))
            {
                if (readLine( &line, OUSTR("UNO_SERVICES="), ucb_content,
                              RTL_TEXTENCODING_UTF8 ))
                {
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
        t_stringset::const_iterator iPos( m_jar_typelibs.begin() );
        t_stringset::const_iterator const iEnd( m_jar_typelibs.end() );
        buf.append( RTL_CONSTASCII_STRINGPARAM("UNO_JAVA_CLASSPATH=") );
        while (iPos != iEnd)
        {
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
        t_stringset::const_iterator iPos( m_rdb_typelibs.begin() );
        t_stringset::const_iterator const iEnd( m_rdb_typelibs.end() );
        buf.append( RTL_CONSTASCII_STRINGPARAM("UNO_TYPES=") );
        while (iPos != iEnd)
        {
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
    ::std::pair<t_stringset::iterator, bool> insertion(
        getTypelibs(jarFile).insert( rcurl ) );
    m_unorc_modified |= insertion.second;
    // write immediately:
    unorc_flush( xCmdEnv );
    return insertion.second;
}

//______________________________________________________________________________
bool BackendImpl::removeFromUnoRc(
    bool jarFile, OUString const & rcurl,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    ::osl::MutexGuard guard( getMutex() );
    unorc_verify_init( xCmdEnv );
    ::std::size_t erased = getTypelibs(jarFile).erase( rcurl );
    bool ret = (erased > 0);
    m_unorc_modified |= ret;
    OSL_ASSERT( ret );
    // write immediately:
    unorc_flush( xCmdEnv );
    return ret;
}

//______________________________________________________________________________
bool BackendImpl::hasInUnoRc(
    bool jarFile, OUString const & url,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    ::osl::MutexGuard guard( getMutex() );
    t_stringset const & rSet = getTypelibs(jarFile);
    return rSet.find( url ) != rSet.end();
}


//------------------------------------------------------------------------------
static void copyDeep( OUString const & source_url, OUString const & dest_url,
                      Reference<XCommandEnvironment> const & xCmdEnv )
{
    ::ucb::Content source_content;
    if (! create_ucb_content( &source_content, source_url, xCmdEnv,
                              false /* no throw */ ))
    {
#if OSL_DEBUG_LEVEL > 0
        {
            ::rtl::OStringBuffer buf;
            buf.append( "### file does not exist, "
                        "but is referenced in the package: " );
            buf.append( ::rtl::OUStringToOString(
                            source_url, RTL_TEXTENCODING_UTF8 ) );
            OSL_ENSURE( 0, buf.makeStringAndClear().getStr() );
        }
#endif
        return;
    }

    // prepare destination folder:
    ::ucb::Content ucb_dest_folder;
    create_folder( &ucb_dest_folder,
                   dest_url.copy( 0, dest_url.lastIndexOf( '/' ) ),
                   xCmdEnv );

    if (! ucb_dest_folder.transferContent(
            source_content, ::ucb::InsertOperation_COPY,
            OUString(), NameClash::OVERWRITE ))
        throw RuntimeException(
            OUSTR("::ucb::Content::transferContent() failed!"), 0 );

    // copy dependent files:
    if (source_url.endsWithIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM(".jar") ))
    {
        // .jar manifest Class-Path dependencies:
        ::rtl::OUStringBuffer buf;
        buf.appendAscii(
            RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.pkg://") );
        buf.append( ::rtl::Uri::encode( source_url,
                                        rtl_UriCharClassRegName,
                                        rtl_UriEncodeIgnoreEscapes,
                                        RTL_TEXTENCODING_UTF8 ) );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("/META-INF/MANIFEST.MF") );
        ::ucb::Content manifest_content;
        if (create_ucb_content(
                &manifest_content,
                buf.makeStringAndClear(), xCmdEnv, false /* no throw */ ))
        {
            OUString line;
            if (readLine( &line, OUSTR("Class-Path"),
                          manifest_content, RTL_TEXTENCODING_ASCII_US ))
            {
                sal_Int32 index = line.indexOf( ':' );
                if (index >= 0)
                {
                    OUString dest_dir(
                        dest_url.copy( 0, dest_url.lastIndexOf( '/' ) ) );
                    ++index;
                    do
                    {
                        OUString rel_path(
                            line.getToken( 0, ' ', index ).trim() );
                        if (rel_path.getLength() > 0)
                        {
                            OUString source(
                                make_url(
                                    source_url.copy(
                                        0, source_url.lastIndexOf( '/' ) ),
                                    rel_path ) );
                            copyDeep( source, make_url( dest_dir, rel_path ),
                                      xCmdEnv );
                        }
                    }
                    while (index >= 0);
                }
            }
        }
    }
}

//______________________________________________________________________________
bool BackendImpl::getInflatedCopy(
    OUString & inflatedURL, OUString const & url,
    Reference<XCommandEnvironment> const & xCmdEnv,
    bool create )
{
    ::osl::MutexGuard guard( getMutex() );
    OUString package( url.copy( url.lastIndexOf( '/' ) + 1 ) );
    OUString tempEntry;
    if (m_urlToTemp->get( &tempEntry, url ))
    {
        if (inflatedURL.getLength() == 0)
        {
            OUString dir( make_url( m_inflated_reg_dir, tempEntry ) );
            dir += OUSTR("_");
            inflatedURL = make_url( dir, package );
        }
        return true;
    }
    else if (! create)
    {
        return false;
    }
    else if (inflatedURL.getLength() > 0)
    {
        // reuse entry:
        // cut reg dir, package:
        OUString tempEntry(
            inflatedURL.copy( m_inflated_reg_dir.getLength() + 1,
                              inflatedURL.lastIndexOf('/') -
                              m_inflated_reg_dir.getLength() - 1 ) );
        // cut _:
        tempEntry = tempEntry.copy( 0, tempEntry.getLength() - 1 );
        m_urlToTemp->put( url, tempEntry );
        return true;
    }

    OUString dir;
    // create new temp dir:
    File::RC rc;
    if (transientMode())
    {
        rc = File::createTempFile( 0, 0, &dir );
        tempEntry = dir.copy( dir.lastIndexOf( '/' ) + 1 );
    }
    else
    {
        rc = File::createTempFile( &m_inflated_dir, 0, &dir );
        OSL_ASSERT( dir.matchIgnoreAsciiCase( m_inflated_dir ) );
        tempEntry = dir.copy( dir.lastIndexOf( '/' ) + 1 );
        dir = make_url( m_inflated_reg_dir, tempEntry );
    }
    if (rc != File::E_None)
        throw RuntimeException(
            OUSTR("::osl::File::createTempFile() failed!"), 0 );
    dir += OUSTR("_");

    inflatedURL = make_url( dir, package );
    copyDeep( url, inflatedURL, xCmdEnv );

    // add new entry to DB:
    m_urlToTemp->put( url, tempEntry );
    return true;
}

//______________________________________________________________________________
void BackendImpl::removeInflatedCopy(
    OUString const & url, Reference<XCommandEnvironment> const & xCmdEnv )
{
    ::osl::MutexGuard guard( getMutex() );
    bool erased = m_urlToTemp->erase( url );
    OSL_ENSURE( erased, "entry not in inflated DB!?" );
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
    if (s_programDir.getLength() == 0)
    {
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
    try
    {
        return Reference<XComponentContext>(
            resolveUnoURL( connectStr, xContext, abortChannel.get() ),
            UNO_QUERY_THROW );
    }
    catch (...)
    {
        // try to terminate process:
        oslProcessError rc = osl_terminateProcess( hProcess );
        OSL_ASSERT( rc == osl_Process_E_None );
        throw;
    }
}

//------------------------------------------------------------------------------
static ComponentInfo * getComponentInfo(
    OUString const & loader, OUString const & url,
    Reference<XComponentContext> const & xContext )
{
    // HACK: highly dependent on stoc/source/servicemanager
    //       and stoc/source/implreg implementation which rely on the same
    //       services.rdb format!

    Reference<registry::XSimpleRegistry> xMemReg(
        xContext->getServiceManager()->createInstanceWithContext(
            OUSTR("com.sun.star.registry.SimpleRegistry"), xContext ),
        UNO_QUERY_THROW );
    xMemReg->open( OUString() /* in mem */, false, true );

    Reference<loader::XImplementationLoader> xLoader(
        xContext->getServiceManager()->createInstanceWithContext(
            loader, xContext ), UNO_QUERY_THROW );
    xLoader->writeRegistryInfo( xMemReg->getRootKey(), OUString(), url );

    ::std::auto_ptr<ComponentInfo> info( new ComponentInfo );

    Sequence< Reference<registry::XRegistryKey> > keys(
        xMemReg->getRootKey()->openKeys() );
    Reference<registry::XRegistryKey> const * pkeys = keys.getConstArray();
    for ( sal_Int32 pos = keys.getLength(); pos--; )
    {
        Reference<registry::XRegistryKey> const & xImplKey = pkeys[ pos ];
        OUString implName( xImplKey->getKeyName().copy( 1 /*leading slash*/ ) );
        info->m_implNames.insert( implName );

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
                info->m_singletons.push_back(
                    beans::StringPair( xSingleton->getKeyName().copy(
                                           implName.getLength() +
                                           sizeof ("//UNO/SINGLETONS/") - 1 ),
                                       xSingleton->getStringValue() ) );
            }
        }
    }

    return info.release();
}

// Package
//______________________________________________________________________________
bool ComponentPackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard & guard,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    if (m_registered == REG_UNINIT)
    {
        m_registered = REG_NOT_REGISTERED;
        if (that->getInflatedCopy( m_inflated_url, m_url, xCmdEnv,
                                   false /* don't create new one */ ))
        {
            Reference<registry::XSimpleRegistry> xRDB( getRDB() );
            if (xRDB.is())
            {
                // lookup rdb for location URL:
                Reference<registry::XRegistryKey> xRootKey(
                    xRDB->getRootKey() );
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
                    if (xKey.is() && xKey->isValid())
                    {
                        OUString location( xKey->getAsciiValue() );
                        if (location.equalsIgnoreAsciiCase( m_inflated_url ))
                            break;
                    }
                }
                if (pos >= 0)
                    m_registered = REG_REGISTERED;
            }
        }
    }
    if (m_registered == REG_VOID)
        throw beans::UnknownPropertyException(
            OUSTR("registration status unknown: ") + m_url,
            static_cast<OWeakObject *>(this) );
    return m_registered == REG_REGISTERED;
}

//______________________________________________________________________________
void ComponentPackageImpl::processPackage_(
    ::osl::ResettableMutexGuard & guard,
    bool registerPackage,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    that->getInflatedCopy( m_inflated_url, m_url, xCmdEnv );
    bool java = m_loader.equalsAsciiL(
        RTL_CONSTASCII_STRINGPARAM("com.sun.star.loader.Java2") );

    m_registered = REG_VOID;
    if (registerPackage)
    {
        if (java) // xxx todo: add to CLASSPATH until we have an
                  // own extendable classloader, the sandbox
                  // classloader is insufficient, because the bridge
                  // and class com.sun.star.uno.Type load classes
                  // using a different classloader, so add first before
                  // raising the uno process
            that->addToUnoRc( java, encodeForRcFile(m_inflated_url), xCmdEnv );

        if (! m_xRemoteContext.is())
        {
            m_xRemoteContext.set(
                that->getObject( m_inflated_url ), UNO_QUERY );
            if (! m_xRemoteContext.is())
            {
                m_xRemoteContext.set(
                    that->insertObject( m_inflated_url,
                                        raise_uno_process(
                                            that->getComponentContext(),
                                            abortChannel ) ),
                    UNO_QUERY_THROW );
            }
        }
        Reference<registry::XImplementationRegistration> xImplReg(
            m_xRemoteContext->getServiceManager()->createInstanceWithContext(
                OUSTR("com.sun.star.registry.ImplementationRegistration"),
                m_xRemoteContext ), UNO_QUERY_THROW );

        Reference<registry::XSimpleRegistry> xServicesRDB( getRDB() );
        xImplReg->registerImplementation(
            m_loader, m_inflated_url /* loaders will expand */, xServicesRDB );

        ::std::auto_ptr<ComponentInfo> info(
            getComponentInfo( m_loader, m_inflated_url, m_xRemoteContext ) );

        Reference<loader::XImplementationLoader> xLoader(
            m_xRemoteContext->getServiceManager()->createInstanceWithContext(
                m_loader, m_xRemoteContext ), UNO_QUERY_THROW );

        // factories live insertion:
        Reference<container::XSet> xSet(
            that->getComponentContext()->getServiceManager(), UNO_QUERY_THROW );
        for ( t_stringset::const_iterator iPos( info->m_implNames.begin() );
              iPos != info->m_implNames.end(); ++iPos )
        {
            checkAborted( abortChannel );

            OUString const & implName = *iPos;
            // activate factory:
            Reference<XInterface> xFactory(
                xLoader->activate(
                    implName, OUString(), m_inflated_url,
                    xServicesRDB->getRootKey()->openKey(
                        OUSTR("/IMPLEMENTATIONS/") + implName ) ) );
            try
            {
                xSet->insert( makeAny(xFactory) );
            }
            // ignore if factory has already been inserted:
            catch (container::ElementExistException &)
            {
                OSL_ENSURE( 0, "### factory already registered?" );
            }
        }

        // singletons live insertion: xxx todo
        xSet.set( that->getComponentContext(), UNO_QUERY/*_THROW*/ );
        if (xSet.is()) {
        for ( t_stringpairvec::const_iterator iPos(
                  info->m_singletons.begin() );
              iPos != info->m_singletons.end(); ++iPos )
        {
            beans::StringPair const & sp = *iPos;
            try
            {
                xSet->insert( makeAny(sp) );
            }
            catch (container::ElementExistException &) // ignore
            {
                OSL_ENSURE( 0, "### unexpected, insert ought to overwrite "
                            "existing entries..." );
            }
        }
        }

        m_registered = REG_REGISTERED;
    }
    else // revokePackage()
    {
        Reference<XComponentContext> xContext;
        if (m_xRemoteContext.is()) // has been activated in this process
            xContext = m_xRemoteContext;
        else // has been deployed in former times
            xContext = that->getComponentContext();
        Reference<registry::XImplementationRegistration> xImplReg(
            xContext->getServiceManager()->createInstanceWithContext(
                OUSTR("com.sun.star.registry.ImplementationRegistration"),
                xContext ), UNO_QUERY_THROW );

        ::std::auto_ptr<ComponentInfo> info(
            getComponentInfo( m_loader, m_inflated_url, xContext ) );

        // factories live removal:
        Reference<container::XSet> xSet(
            that->getComponentContext()->getServiceManager(), UNO_QUERY_THROW );
        for ( t_stringset::const_iterator iPos( info->m_implNames.begin() );
              iPos != info->m_implNames.end(); ++iPos )
        {
            OUString const & implName = *iPos;
            try
            {
                xSet->remove( makeAny(implName) );
            }
            // ignore if factory has not been live deployed:
            catch (container::NoSuchElementException &)
            {
            }
        }

        // singletons live insertion: xxx todo
        xSet.set( that->getComponentContext(), UNO_QUERY/*_THROW*/ );
        if (xSet.is()) {
        for ( t_stringpairvec::const_iterator iPos(
                  info->m_singletons.begin() );
              iPos != info->m_singletons.end(); ++iPos )
        {
            beans::StringPair const & sp = *iPos;
            try
            {
                xSet->remove( makeAny(sp.First) );
            }
            // ignore if singleton has not been live deployed:
            catch (container::ElementExistException &)
            {
            }
        }
        }

        Reference<registry::XSimpleRegistry> xServicesRDB( getRDB() );
        xImplReg->revokeImplementation(
            m_inflated_url /* loaders will expand */, xServicesRDB );

        if (java) // xxx todo: add to CLASSPATH until we have an
                  // own extendable classloader, the sandbox
                  // classloader is insufficient, because the bridge
                  // and class com.sun.star.uno.Type load classes
                  // using a different classloader
            that->removeFromUnoRc(
                java, encodeForRcFile(m_inflated_url), xCmdEnv );

        if (m_xRemoteContext.is())
        {
            that->releaseObject( m_inflated_url );
            m_xRemoteContext.clear();
        }
        that->removeInflatedCopy( m_url, xCmdEnv );

        m_registered = REG_NOT_REGISTERED;
    }
}

//##############################################################################

// Package
//______________________________________________________________________________
bool TypelibraryPackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard & guard,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    if (! that->getInflatedCopy( m_inflated_url, m_url, xCmdEnv,
                                 false /* don't create new one */ ))
        return false;
    return that->hasInUnoRc(
        m_jarFile, encodeForRcFile(m_inflated_url), xCmdEnv );
}

//______________________________________________________________________________
void TypelibraryPackageImpl::processPackage_(
    ::osl::ResettableMutexGuard & guard,
    bool registerPackage,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    if (registerPackage)
    {
        that->getInflatedCopy( m_inflated_url, m_url, xCmdEnv );
        // xxx todo: check jar, rdb compatibility to existing types

        that->addToUnoRc( m_jarFile, encodeForRcFile(m_inflated_url), xCmdEnv );

        // live insertion:
        if (m_jarFile)
        {
            // xxx todo add to classpath at runtime:
        }
        else // RDB:
        {
            Reference<XComponentContext> const & xContext =
                that->getComponentContext();
            if (! m_xTDprov.is())
            {
                m_xTDprov.set( that->getObject( m_inflated_url ), UNO_QUERY );
                if (! m_xTDprov.is())
                {
                    Reference<registry::XSimpleRegistry> xReg(
                        xContext->getServiceManager()
                        ->createInstanceWithContext(
                            OUSTR("com.sun.star.registry.SimpleRegistry"),
                            xContext ), UNO_QUERY_THROW );
                    xReg->open( expand_url( m_inflated_url ),
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
                    {
                        m_xTDprov.set(
                            that->insertObject(
                                m_inflated_url, xTDprov ), UNO_QUERY_THROW );
                    }
                }
            }
            if (m_xTDprov.is())
            {
                Reference<container::XSet> xSet(
                    xContext->getValueByName(
                        OUSTR("/singletons/com.sun.star."
                              "reflection.theTypeDescriptionManager") ),
                    UNO_QUERY_THROW );
                xSet->insert( makeAny(m_xTDprov) );
            }
        }
    }
    else // revokePackage()
    {
        if (that->getInflatedCopy( m_inflated_url, m_url, xCmdEnv,
                                   false /* don't create new one */ ))
        {
            // xxx todo: revoking types at runtime, possible, sensible?

            that->removeFromUnoRc(
                m_jarFile, encodeForRcFile(m_inflated_url), xCmdEnv );

            if (m_xTDprov.is())
            {
                // remove live:
                Reference<container::XSet> xSet(
                    that->getComponentContext()->getValueByName(
                        OUSTR("/singletons/com.sun.star."
                              "reflection.theTypeDescriptionManager") ),
                    UNO_QUERY_THROW );
                xSet->remove( makeAny(m_xTDprov) );

                that->releaseObject( m_inflated_url );
                m_xTDprov.clear();
            }

            that->removeInflatedCopy( m_url, xCmdEnv );
        }
    }
}

} // namespace component
} // namespace backend
} // namespace dp_registry

