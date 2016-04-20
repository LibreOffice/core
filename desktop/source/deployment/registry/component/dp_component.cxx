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


#include "dp_component.hrc"
#include "dp_backend.h"
#include "dp_platform.hxx"
#include "dp_ucb.h"
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/uri.hxx>
#include <sal/log.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <ucbhelper/content.hxx>
#include <comphelper/anytostring.hxx>
#include <comphelper/servicedecl.hxx>
#include <comphelper/sequence.hxx>
#include <xmlscript/xml_helper.hxx>
#include <svl/inettype.hxx>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/util/theMacroExpander.hpp>
#include <algorithm>
#include <list>
#include <memory>
#include <unordered_map>
#include <vector>
#include "dp_compbackenddb.hxx"

using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;

namespace dp_registry {
namespace backend {
namespace component {
namespace {

typedef ::std::list<OUString> t_stringlist;
typedef ::std::vector< ::std::pair<OUString, OUString> > t_stringpairvec;

#define IMPLEMENTATION_NAME  "com.sun.star.comp.deployment.component.PackageRegistryBackend"

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
        if (arg.match("-env:"))
            ret.push_back(arg);
    }
    return ret;
}

bool jarManifestHeaderPresent(
    OUString const & url, OUString const & name,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    OUStringBuffer buf;
    buf.append( "vnd.sun.star.zip://" );
    buf.append(
        ::rtl::Uri::encode(
            url, rtl_UriCharClassRegName, rtl_UriEncodeIgnoreEscapes,
            RTL_TEXTENCODING_UTF8 ) );
    buf.append( "/META-INF/MANIFEST.MF" );
    ::ucbhelper::Content manifestContent;
    OUString line;
    return
        create_ucb_content(
            &manifestContent, buf.makeStringAndClear(), xCmdEnv,
            false /* no throw */ )
        && readLine( &line, name, manifestContent, RTL_TEXTENCODING_ASCII_US );
}


class BackendImpl : public ::dp_registry::backend::PackageRegistryBackend
{
    class ComponentPackageImpl : public ::dp_registry::backend::Package
    {
        BackendImpl * getMyBackend() const;

        const OUString m_loader;

        enum reg {
            REG_UNINIT, REG_VOID, REG_REGISTERED, REG_NOT_REGISTERED, REG_MAYBE_REGISTERED
        } m_registered;

        void getComponentInfo(
            ComponentBackendDb::Data * data,
            std::vector< css::uno::Reference< css::uno::XInterface > > *
                factories,
            Reference<XComponentContext> const & xContext );

        void componentLiveInsertion(
            ComponentBackendDb::Data const & data,
            std::vector< css::uno::Reference< css::uno::XInterface > > const &
                factories);

        void componentLiveRemoval(ComponentBackendDb::Data const & data);

        virtual void SAL_CALL disposing() override;

        // Package
        virtual beans::Optional< beans::Ambiguous<sal_Bool> > isRegistered_(
            ::osl::ResettableMutexGuard & guard,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv ) override;
        virtual void processPackage_(
            ::osl::ResettableMutexGuard & guard,
            bool registerPackage,
            bool startup,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv ) override;

        const Reference<registry::XSimpleRegistry> getRDB() const;

    public:
        ComponentPackageImpl(
            ::rtl::Reference<PackageRegistryBackend> const & myBackend,
            OUString const & url, OUString const & name,
            Reference<deployment::XPackageTypeInfo> const & xPackageType,
            OUString const & loader, bool bRemoved,
            OUString const & identifier);
    };
    friend class ComponentPackageImpl;

    class ComponentsPackageImpl : public ::dp_registry::backend::Package
    {
        BackendImpl * getMyBackend() const;

        // Package
        virtual beans::Optional< beans::Ambiguous<sal_Bool> > isRegistered_(
            ::osl::ResettableMutexGuard & guard,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv ) override;
        virtual void processPackage_(
            ::osl::ResettableMutexGuard & guard,
            bool registerPackage,
            bool startup,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv ) override;
    public:
        ComponentsPackageImpl(
            ::rtl::Reference<PackageRegistryBackend> const & myBackend,
            OUString const & url, OUString const & name,
            Reference<deployment::XPackageTypeInfo> const & xPackageType,
            bool bRemoved, OUString const & identifier);
    };
    friend class ComponentsPackageImpl;

    class TypelibraryPackageImpl : public ::dp_registry::backend::Package
    {
        BackendImpl * getMyBackend() const;

        const bool m_jarFile;

        virtual void SAL_CALL disposing() override;

        // Package
        virtual beans::Optional< beans::Ambiguous<sal_Bool> > isRegistered_(
            ::osl::ResettableMutexGuard & guard,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv ) override;
        virtual void processPackage_(
            ::osl::ResettableMutexGuard & guard,
            bool registerPackage,
            bool startup,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv ) override;

    public:
        TypelibraryPackageImpl(
            ::rtl::Reference<PackageRegistryBackend> const & myBackend,
            OUString const & url, OUString const & name,
            Reference<deployment::XPackageTypeInfo> const & xPackageType,
            bool jarFile, bool bRemoved,
            OUString const & identifier);
    };
    friend class TypelibraryPackageImpl;

    /** Serves for unregistering packages that were registered on a
        different platform. This can happen if one has remotely mounted
        /home, for example.
     */
    class OtherPlatformPackageImpl : public ::dp_registry::backend::Package
    {
    public:
        OtherPlatformPackageImpl(
            ::rtl::Reference<PackageRegistryBackend> const & myBackend,
            OUString const & url, OUString const & name,
            Reference<deployment::XPackageTypeInfo> const & xPackageType,
            bool bRemoved, OUString const & identifier, OUString const& rPlatform);

    private:
        BackendImpl * getMyBackend() const;

        const Reference<registry::XSimpleRegistry> impl_openRDB() const;
        const Reference<XInterface> impl_createInstance(OUString const& rService) const;

        // Package
        virtual beans::Optional< beans::Ambiguous<sal_Bool> > isRegistered_(
            ::osl::ResettableMutexGuard & guard,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv ) override;
        virtual void processPackage_(
            ::osl::ResettableMutexGuard & guard,
            bool registerPackage,
            bool startup,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv ) override;

    private:
        OUString const m_aPlatform;
    };
    friend class OtherPlatformPackageImpl;

    t_stringlist m_jar_typelibs;
    t_stringlist m_rdb_typelibs;
    t_stringlist m_components;

    enum RcItem { RCITEM_JAR_TYPELIB, RCITEM_RDB_TYPELIB, RCITEM_COMPONENTS };

    t_stringlist & getRcItemList( RcItem kind ) {
        switch (kind)
        {
        case RCITEM_JAR_TYPELIB:
            return m_jar_typelibs;
        case RCITEM_RDB_TYPELIB:
            return m_rdb_typelibs;
        default: // case RCITEM_COMPONENTS
            return m_components;
        }
    }

    bool m_unorc_inited;
    bool m_unorc_modified;
    bool bSwitchedRdbFiles;

    typedef std::unordered_map< OUString, Reference<XInterface>,
                                OUStringHash > t_string2object;
    t_string2object m_backendObjects;

    // PackageRegistryBackend
    virtual Reference<deployment::XPackage> bindPackage_(
        OUString const & url, OUString const & mediaType,
        bool bRemoved, OUString const & identifier,
        Reference<XCommandEnvironment> const & xCmdEnv ) override;

    virtual void SAL_CALL disposing() override;

    const Reference<deployment::XPackageTypeInfo> m_xDynComponentTypeInfo;
    const Reference<deployment::XPackageTypeInfo> m_xJavaComponentTypeInfo;
    const Reference<deployment::XPackageTypeInfo> m_xPythonComponentTypeInfo;
    const Reference<deployment::XPackageTypeInfo> m_xComponentsTypeInfo;
    const Reference<deployment::XPackageTypeInfo> m_xRDBTypelibTypeInfo;
    const Reference<deployment::XPackageTypeInfo> m_xJavaTypelibTypeInfo;
    Sequence< Reference<deployment::XPackageTypeInfo> > m_typeInfos;

    OUString m_commonRDB;
    OUString m_nativeRDB;

    //URLs of the original rdbs (before any switching):
    OUString m_commonRDB_orig;
    OUString m_nativeRDB_orig;

    std::unique_ptr<ComponentBackendDb> m_backendDb;

    void addDataToDb(OUString const & url, ComponentBackendDb::Data const & data);
    ComponentBackendDb::Data readDataFromDb(OUString const & url);
    void revokeEntryFromDb(OUString const & url);

    Reference<registry::XSimpleRegistry> m_xCommonRDB;
    Reference<registry::XSimpleRegistry> m_xNativeRDB;

    void unorc_verify_init( Reference<XCommandEnvironment> const & xCmdEnv );
    void unorc_flush( Reference<XCommandEnvironment> const & xCmdEnv );

    Reference<XInterface> getObject( OUString const & id );
    Reference<XInterface> insertObject(
        OUString const & id, Reference<XInterface> const & xObject );
    void releaseObject( OUString const & id );

    bool addToUnoRc( RcItem kind, OUString const & url,
                     Reference<XCommandEnvironment> const & xCmdEnv );
    bool removeFromUnoRc( RcItem kind, OUString const & url,
                          Reference<XCommandEnvironment> const & xCmdEnv );
    bool hasInUnoRc( RcItem kind, OUString const & url );

    css::uno::Reference< css::uno::XComponentContext > getRootContext() const;

public:
    BackendImpl( Sequence<Any> const & args,
                 Reference<XComponentContext> const & xComponentContext );

    // XPackageRegistry
    virtual Sequence< Reference<deployment::XPackageTypeInfo> > SAL_CALL
    getSupportedPackageTypes() throw (RuntimeException, std::exception) override;

    virtual void SAL_CALL packageRemoved(OUString const & url, OUString const & mediaType)
        throw (deployment::DeploymentException,
               uno::RuntimeException, std::exception) override;

    using PackageRegistryBackend::disposing;

    //Will be called from ComponentPackageImpl
    void initServiceRdbFiles();
};


BackendImpl::ComponentPackageImpl::ComponentPackageImpl(
    ::rtl::Reference<PackageRegistryBackend> const & myBackend,
    OUString const & url, OUString const & name,
    Reference<deployment::XPackageTypeInfo> const & xPackageType,
    OUString const & loader, bool bRemoved,
    OUString const & identifier)
    : Package( myBackend, url, name, name /* display-name */,
               xPackageType, bRemoved, identifier),
      m_loader( loader ),
      m_registered( REG_UNINIT )
{}

const Reference<registry::XSimpleRegistry>
BackendImpl::ComponentPackageImpl::getRDB() const
{
    BackendImpl * that = getMyBackend();

    //Late "initialization" of the services rdb files
    //This is to prevent problems when running several
    //instances of OOo with root rights in parallel. This
    //would otherwise cause problems when copying the rdbs.
    //See  http://qa.openoffice.org/issues/show_bug.cgi?id=99257
    {
        const ::osl::MutexGuard guard( getMutex() );
        if (!that->bSwitchedRdbFiles)
        {
            that->bSwitchedRdbFiles = true;
            that->initServiceRdbFiles();
        }
    }
    if ( m_loader == "com.sun.star.loader.SharedLibrary" )
        return that->m_xNativeRDB;
    else
        return that->m_xCommonRDB;
}

BackendImpl * BackendImpl::ComponentPackageImpl::getMyBackend() const
{
    BackendImpl * pBackend = static_cast<BackendImpl *>(m_myBackend.get());
    if (nullptr == pBackend)
    {
        //Throws a DisposedException
        check();
        //We should never get here...
        throw RuntimeException(
            "Failed to get the BackendImpl",
            static_cast<OWeakObject*>(const_cast<ComponentPackageImpl *>(this)));
    }
    return pBackend;
}


void BackendImpl::ComponentPackageImpl::disposing()
{
    Package::disposing();
}


void BackendImpl::TypelibraryPackageImpl::disposing()
{
    Package::disposing();
}


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
    catch (const RuntimeException &) {
        throw;
    }
    catch (const Exception &) {
        Any exc( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException(
            "caught unexpected exception while disposing...",
            static_cast<OWeakObject *>(this), exc );
    }
}


void BackendImpl::initServiceRdbFiles()
{
    const Reference<XCommandEnvironment> xCmdEnv;

    ::ucbhelper::Content cacheDir( getCachePath(), xCmdEnv, m_xComponentContext );
    ::ucbhelper::Content oldRDB;
    // switch common rdb:
    if (!m_commonRDB_orig.isEmpty())
    {
        (void)create_ucb_content(
            &oldRDB, makeURL( getCachePath(), m_commonRDB_orig),
            xCmdEnv, false /* no throw */ );
    }
    m_commonRDB = m_commonRDB_orig == "common.rdb" ? OUString("common_.rdb") : OUString("common.rdb");
    if (oldRDB.get().is())
    {
        if (! cacheDir.transferContent(
                oldRDB, ::ucbhelper::InsertOperation_COPY,
                m_commonRDB, NameClash::OVERWRITE ))
        {

            throw RuntimeException( "UCB transferContent() failed!", nullptr );
        }
        oldRDB = ::ucbhelper::Content();
    }
    // switch native rdb:
    if (!m_nativeRDB_orig.isEmpty())
    {
        (void)create_ucb_content(
            &oldRDB, makeURL(getCachePath(), m_nativeRDB_orig),
            xCmdEnv, false /* no throw */ );
    }
    const OUString plt_rdb( getPlatformString() + ".rdb" );
    const OUString plt_rdb_( getPlatformString() + "_.rdb" );
    m_nativeRDB = (m_nativeRDB_orig == plt_rdb ) ? plt_rdb_ : plt_rdb;
    if (oldRDB.get().is())
    {
        if (! cacheDir.transferContent(
                oldRDB, ::ucbhelper::InsertOperation_COPY,
                m_nativeRDB, NameClash::OVERWRITE ))
            throw RuntimeException( "UCB transferContent() failed!", nullptr );
    }

    // UNO is bootstrapped, flush for next process start:
    m_unorc_modified = true;
    unorc_flush( Reference<XCommandEnvironment>() );


    // common rdb for java, native rdb for shared lib components
    if (!m_commonRDB.isEmpty()) {
        m_xCommonRDB.set(
            m_xComponentContext->getServiceManager()
            ->createInstanceWithContext(
            "com.sun.star.registry.SimpleRegistry",
            m_xComponentContext ), UNO_QUERY_THROW );
        m_xCommonRDB->open(
            makeURL( expandUnoRcUrl(getCachePath()), m_commonRDB ),
            false, true);
    }
    if (!m_nativeRDB.isEmpty()) {
        m_xNativeRDB.set(
            m_xComponentContext->getServiceManager()
            ->createInstanceWithContext(
            "com.sun.star.registry.SimpleRegistry",
            m_xComponentContext ), UNO_QUERY_THROW );
        m_xNativeRDB->open(
            makeURL( expandUnoRcUrl(getCachePath()), m_nativeRDB ),
            false, true);
    }
}

BackendImpl::BackendImpl(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xComponentContext )
    : PackageRegistryBackend( args, xComponentContext ),
      m_unorc_inited( false ),
      m_unorc_modified( false ),
      bSwitchedRdbFiles(false),
      m_xDynComponentTypeInfo( new Package::TypeInfo(
             "application/vnd.sun.star.uno-component;type=native;platform=" +
                                   getPlatformString(),
                                   "*" SAL_DLLEXTENSION,
                                   getResourceString(RID_STR_DYN_COMPONENT),
                                   RID_IMG_COMPONENT) ),
      m_xJavaComponentTypeInfo( new Package::TypeInfo(
             "application/vnd.sun.star.uno-component;type=Java",
                                    "*.jar",
                                    getResourceString(RID_STR_JAVA_COMPONENT),
                                    RID_IMG_JAVA_COMPONENT) ),
      m_xPythonComponentTypeInfo( new Package::TypeInfo(
             "application/vnd.sun.star.uno-component;type=Python",
                                      "*.py",
                                      getResourceString(
                                          RID_STR_PYTHON_COMPONENT),
                                      RID_IMG_COMPONENT ) ),
      m_xComponentsTypeInfo( new Package::TypeInfo(
                                 "application/vnd.sun.star.uno-components",
                                 "*.components",
                                 getResourceString(RID_STR_COMPONENTS),
                                 RID_IMG_COMPONENT ) ),
      m_xRDBTypelibTypeInfo( new Package::TypeInfo(
             "application/vnd.sun.star.uno-typelibrary;type=RDB",
                                 "*.rdb",
                                 getResourceString(RID_STR_RDB_TYPELIB),
                                 RID_IMG_TYPELIB ) ),
      m_xJavaTypelibTypeInfo( new Package::TypeInfo(
             "application/vnd.sun.star.uno-typelibrary;type=Java",
                                  "*.jar",
                                  getResourceString(RID_STR_JAVA_TYPELIB),
                                  RID_IMG_JAVA_TYPELIB ) ),
      m_typeInfos( 6 )
{
    m_typeInfos[ 0 ] = m_xDynComponentTypeInfo;
    m_typeInfos[ 1 ] = m_xJavaComponentTypeInfo;
    m_typeInfos[ 2 ] = m_xPythonComponentTypeInfo;
    m_typeInfos[ 3 ] = m_xComponentsTypeInfo;
    m_typeInfos[ 4 ] = m_xRDBTypelibTypeInfo;
    m_typeInfos[ 5 ] = m_xJavaTypelibTypeInfo;

    const Reference<XCommandEnvironment> xCmdEnv;

    if (transientMode())
    {
        // in-mem rdbs:
        // common rdb for java, native rdb for shared lib components
        m_xCommonRDB.set(
            xComponentContext->getServiceManager()->createInstanceWithContext(
                "com.sun.star.registry.SimpleRegistry",
                xComponentContext ), UNO_QUERY_THROW );
        m_xCommonRDB->open( OUString() /* in-mem */,
                            false /* ! read-only */, true /* create */ );
        m_xNativeRDB.set(
            xComponentContext->getServiceManager()->createInstanceWithContext(
                "com.sun.star.registry.SimpleRegistry",
                xComponentContext ), UNO_QUERY_THROW );
        m_xNativeRDB->open( OUString() /* in-mem */,
                            false /* ! read-only */, true /* create */ );
    }
    else
    {
        unorc_verify_init( xCmdEnv );
        OUString dbFile = makeURL(getCachePath(), "backenddb.xml");
        m_backendDb.reset(
            new ComponentBackendDb(getComponentContext(), dbFile));
    }
}

void BackendImpl::addDataToDb(
    OUString const & url, ComponentBackendDb::Data const & data)
{
    if (m_backendDb.get())
        m_backendDb->addEntry(url, data);
}

ComponentBackendDb::Data BackendImpl::readDataFromDb(OUString const & url)
{
    ComponentBackendDb::Data data;
    if (m_backendDb.get())
        data = m_backendDb->getEntry(url);
    return data;
}

void BackendImpl::revokeEntryFromDb(OUString const & url)
{
    if (m_backendDb.get())
        m_backendDb->revokeEntry(url);
}

// XPackageRegistry

Sequence< Reference<deployment::XPackageTypeInfo> >
BackendImpl::getSupportedPackageTypes() throw (RuntimeException, std::exception)
{
    return m_typeInfos;
}

void BackendImpl::packageRemoved(OUString const & url, OUString const & /*mediaType*/)
        throw (deployment::DeploymentException,
               uno::RuntimeException, std::exception)
{
    if (m_backendDb.get())
        m_backendDb->removeEntry(url);
}

// PackageRegistryBackend

Reference<deployment::XPackage> BackendImpl::bindPackage_(
    OUString const & url, OUString const & mediaType_,
    bool bRemoved, OUString const & identifier,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    OUString mediaType(mediaType_);
    if ( mediaType.isEmpty() || mediaType == "application/vnd.sun.star.uno-component" || mediaType == "application/vnd.sun.star.uno-typelibrary" )
    {
        // detect exact media-type:
        ::ucbhelper::Content ucbContent;
        if (create_ucb_content( &ucbContent, url, xCmdEnv )) {
            const OUString title( StrTitle::getTitle( ucbContent ) );
            if (title.endsWithIgnoreAsciiCase(SAL_DLLEXTENSION))
            {
                mediaType = "application/vnd.sun.star.uno-component;type=native;platform=" +
                    getPlatformString();
            }
            else if (title.endsWithIgnoreAsciiCase(".jar"))
            {
                if (jarManifestHeaderPresent(
                        url, "RegistrationClassName", xCmdEnv ))
                    mediaType = "application/vnd.sun.star.uno-component;type=Java";
                if (mediaType.isEmpty())
                    mediaType = "application/vnd.sun.star.uno-typelibrary;type=Java";
            }
            else if (title.endsWithIgnoreAsciiCase(".py"))
                mediaType = "application/vnd.sun.star.uno-component;type=Python";
            else if (title.endsWithIgnoreAsciiCase(".rdb"))
                mediaType = "application/vnd.sun.star.uno-typelibrary;type=RDB";
        }
        if (mediaType.isEmpty())
            throw lang::IllegalArgumentException(
                StrCannotDetectMediaType::get() + url,
                static_cast<OWeakObject *>(this), static_cast<sal_Int16>(-1) );
    }

    OUString type, subType;
    INetContentTypeParameterList params;
    if (INetContentTypes::parse( mediaType, type, subType, &params ))
    {
        if (type.equalsIgnoreAsciiCase("application"))
        {
            OUString name;
            if (!bRemoved)
            {
                ::ucbhelper::Content ucbContent( url, xCmdEnv, m_xComponentContext );
                name = StrTitle::getTitle( ucbContent );
            }

            if (subType.equalsIgnoreAsciiCase("vnd.sun.star.uno-component"))
            {
                // xxx todo: probe and evaluate component xml description

                auto const iter = params.find(OString("platform"));
                bool bPlatformFits(iter == params.end());
                OUString aPlatform;
                if (!bPlatformFits) // platform is specified, we have to check
                {
                    aPlatform = iter->second.m_sValue;
                    bPlatformFits = platform_fits(aPlatform);
                }
                // If the package is being removed, do not care whether
                // platform fits. We won't be using it anyway.
                if (bPlatformFits || bRemoved) {
                    auto const iterType = params.find(OString("type"));
                    if (iterType != params.end())
                    {
                        OUString const & value = iterType->second.m_sValue;
                        if (value.equalsIgnoreAsciiCase("native")) {
                            if (bPlatformFits)
                                return new BackendImpl::ComponentPackageImpl(
                                    this, url, name, m_xDynComponentTypeInfo,
                                    "com.sun.star.loader.SharedLibrary",
                                    bRemoved, identifier);
                            else
                                return new BackendImpl::OtherPlatformPackageImpl(
                                    this, url, name, m_xDynComponentTypeInfo,
                                    bRemoved, identifier, aPlatform);
                        }
                        if (value.equalsIgnoreAsciiCase("Java")) {
                            return new BackendImpl::ComponentPackageImpl(
                                this, url, name, m_xJavaComponentTypeInfo,
                                "com.sun.star.loader.Java2",
                                bRemoved, identifier);
                        }
                        if (value.equalsIgnoreAsciiCase("Python")) {
                            return new BackendImpl::ComponentPackageImpl(
                                this, url, name, m_xPythonComponentTypeInfo,
                                "com.sun.star.loader.Python",
                                bRemoved, identifier);
                        }
                    }
                }
            }
            else if (subType.equalsIgnoreAsciiCase("vnd.sun.star.uno-components"))
            {
                auto const iter = params.find(OString("platform"));
                if (iter == params.end() || platform_fits(iter->second.m_sValue)) {
                    return new BackendImpl::ComponentsPackageImpl(
                        this, url, name, m_xComponentsTypeInfo, bRemoved,
                        identifier);
                }
            }
            else if (subType.equalsIgnoreAsciiCase( "vnd.sun.star.uno-typelibrary"))
            {
                auto const iter = params.find(OString("type"));
                if (iter != params.end()) {
                    OUString const & value = iter->second.m_sValue;
                    if (value.equalsIgnoreAsciiCase("RDB"))
                    {
                        return new BackendImpl::TypelibraryPackageImpl(
                            this, url, name, m_xRDBTypelibTypeInfo,
                            false /* rdb */, bRemoved, identifier);
                    }
                    if (value.equalsIgnoreAsciiCase("Java")) {
                        return new BackendImpl::TypelibraryPackageImpl(
                            this, url, name, m_xJavaTypelibTypeInfo,
                            true /* jar */, bRemoved, identifier);
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
                makeURL( getCachePath(), "unorc" ),
                xCmdEnv, false /* no throw */ ))
        {
            OUString line;
            if (readLine( &line, "UNO_JAVA_CLASSPATH=", ucb_content,
                          RTL_TEXTENCODING_UTF8 ))
            {
                sal_Int32 index = sizeof ("UNO_JAVA_CLASSPATH=") - 1;
                do {
                    OUString token( line.getToken( 0, ' ', index ).trim() );
                    if (!token.isEmpty())
                    {
                        if (create_ucb_content(
                                nullptr, expandUnoRcTerm(token), xCmdEnv,
                                false /* no throw */ ))
                        {
                            //The jar file may not exist anymore if a shared or bundled
                            //extension was removed, but it can still be in the unorc
                            //After running XExtensionManager::synchronize, the unorc is
                            //cleaned up
                            m_jar_typelibs.push_back( token );
                        }
                    }
                }
                while (index >= 0);
            }
            if (readLine( &line, "UNO_TYPES=", ucb_content,
                          RTL_TEXTENCODING_UTF8 )) {
                sal_Int32 index = sizeof ("UNO_TYPES=") - 1;
                do {
                    OUString token( line.getToken( 0, ' ', index ).trim() );
                    if (!token.isEmpty())
                    {
                        if (token[ 0 ] == '?')
                            token = token.copy( 1 );
                         if (create_ucb_content(
                                nullptr, expandUnoRcTerm(token), xCmdEnv,
                                false /* no throw */ ))
                         {
                             //The RDB file may not exist anymore if a shared or bundled
                             //extension was removed, but it can still be in the unorc.
                             //After running XExtensionManager::synchronize, the unorc is
                             //cleaned up
                             m_rdb_typelibs.push_back( token );
                         }
                    }
                }
                while (index >= 0);
            }
            if (readLine( &line, "UNO_SERVICES=", ucb_content,
                          RTL_TEXTENCODING_UTF8 ))
            {
                // The UNO_SERVICES line always has the BNF form
                //  "UNO_SERVICES="
                //  ("?$ORIGIN/" <common-rdb>)?                        -- first
                //  "${$ORIGIN/${_OS}_${_ARCH}rc:UNO_SERVICES}"?       -- second
                //  ("?" ("BUNDLED_EXTENSIONS" |                       -- third
                //   "UNO_SHARED_PACKAGES_CACHE" | "UNO_USER_PACKAGES_CACHE")
                //   ...)*
                // so can unambiguously be split into its three parts:
                int state = 1;
                for (sal_Int32 i = RTL_CONSTASCII_LENGTH("UNO_SERVICES=");
                     i >= 0;)
                {
                    OUString token(line.getToken(0, ' ', i));
                    if (!token.isEmpty())
                    {
                        if (state == 1 && token.match("?$ORIGIN/"))
                        {
                            m_commonRDB_orig = token.copy(
                                RTL_CONSTASCII_LENGTH("?$ORIGIN/"));
                            state = 2;
                        }
                        else if ( state <= 2 && token == "${$ORIGIN/${_OS}_${_ARCH}rc:UNO_SERVICES}" )
                        {
                            state = 3;
                        }
                        else
                        {
                            if (token[0] == '?')
                            {
                                token = token.copy(1);
                            }
                            m_components.push_back(token);
                            state = 3;
                        }
                    }
                }
            }

            // native rc:
            if (create_ucb_content(
                    &ucb_content,
                    makeURL( getCachePath(), getPlatformString() + "rc"),
                    xCmdEnv, false /* no throw */ )) {
                if (readLine( &line, "UNO_SERVICES=", ucb_content,
                              RTL_TEXTENCODING_UTF8 )) {
                    m_nativeRDB_orig = line.copy(
                        sizeof ("UNO_SERVICES=?$ORIGIN/") - 1 );
                }
            }
        }
        m_unorc_modified = false;
        m_unorc_inited = true;
    }
}


void BackendImpl::unorc_flush( Reference<XCommandEnvironment> const & xCmdEnv )
{
    if (transientMode())
        return;
    if (!m_unorc_inited || !m_unorc_modified)
        return;

    OStringBuffer buf;

    buf.append("ORIGIN=");
    OUString sOrigin = dp_misc::makeRcTerm(m_cachePath);
    OString osOrigin = OUStringToOString(sOrigin, RTL_TEXTENCODING_UTF8);
    buf.append(osOrigin);
    buf.append(LF);

    if (! m_jar_typelibs.empty())
    {
        t_stringlist::const_iterator iPos( m_jar_typelibs.begin() );
        t_stringlist::const_iterator const iEnd( m_jar_typelibs.end() );
        buf.append( "UNO_JAVA_CLASSPATH=" );
        while (iPos != iEnd) {
            // encoded ASCII file-urls:
            const OString item(
                OUStringToOString( *iPos, RTL_TEXTENCODING_ASCII_US ) );
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
        buf.append( "UNO_TYPES=" );
        while (iPos != iEnd) {
            buf.append( '?' );
            // encoded ASCII file-urls:
            const OString item(
                OUStringToOString( *iPos, RTL_TEXTENCODING_ASCII_US ) );
            buf.append( item );
            ++iPos;
            if (iPos != iEnd)
                buf.append( ' ' );
        }
        buf.append(LF);
    }

    // If we duplicated the common or native rdb then we must use those urls
    //otherwise we use those of the original files. That is, m_commonRDB_orig
    //and m_nativeRDB_orig;
    OUString sCommonRDB(m_commonRDB.isEmpty() ? m_commonRDB_orig : m_commonRDB );
    OUString sNativeRDB(m_nativeRDB.isEmpty() ? m_nativeRDB_orig : m_nativeRDB );

    if (!sCommonRDB.isEmpty() || !sNativeRDB.isEmpty() ||
        !m_components.empty())
    {
        buf.append( "UNO_SERVICES=" );
        bool space = false;
        if (!sCommonRDB.isEmpty())
        {
            buf.append( "?$ORIGIN/" );
            buf.append( OUStringToOString(
                            sCommonRDB, RTL_TEXTENCODING_ASCII_US ) );
            space = true;
        }
        if (!sNativeRDB.isEmpty())
        {
            if (space)
            {
                buf.append(' ');
            }
            buf.append( "${$ORIGIN/${_OS}_${_ARCH}rc:UNO_SERVICES}" );
            space = true;

            // write native rc:
            OStringBuffer buf2;
            buf2.append("ORIGIN=");
            buf2.append(osOrigin);
            buf2.append(LF);
            buf2.append( "UNO_SERVICES=?$ORIGIN/" );
            buf2.append( OUStringToOString(
                             sNativeRDB, RTL_TEXTENCODING_ASCII_US ) );
            buf2.append(LF);

            const Reference<io::XInputStream> xData(
                ::xmlscript::createInputStream(
                    ::rtl::ByteSequence(
                        reinterpret_cast<sal_Int8 const *>(buf2.getStr()),
                        buf2.getLength() ) ) );
            ::ucbhelper::Content ucb_content(
                makeURL( getCachePath(), getPlatformString() + "rc" ),
                xCmdEnv, m_xComponentContext );
            ucb_content.writeStream( xData, true /* replace existing */ );
        }
        for (t_stringlist::iterator i(m_components.begin());
             i != m_components.end(); ++i)
        {
            if (space)
            {
                buf.append(' ');
            }
            buf.append('?');
            buf.append(OUStringToOString(*i, RTL_TEXTENCODING_UTF8));
            space = true;
        }
        buf.append(LF);
    }

    // write unorc:
    const Reference<io::XInputStream> xData(
        ::xmlscript::createInputStream(
            ::rtl::ByteSequence(
                reinterpret_cast<sal_Int8 const *>(buf.getStr()),
                buf.getLength() ) ) );
    ::ucbhelper::Content ucb_content(
        makeURL( getCachePath(), "unorc" ), xCmdEnv, m_xComponentContext );
    ucb_content.writeStream( xData, true /* replace existing */ );

    m_unorc_modified = false;
}


bool BackendImpl::addToUnoRc( RcItem kind, OUString const & url_,
                              Reference<XCommandEnvironment> const & xCmdEnv )
{
    const OUString rcterm( dp_misc::makeRcTerm(url_) );
    const ::osl::MutexGuard guard( getMutex() );
    unorc_verify_init( xCmdEnv );
    t_stringlist & rSet = getRcItemList(kind);
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


bool BackendImpl::removeFromUnoRc(
    RcItem kind, OUString const & url_,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    const OUString rcterm( dp_misc::makeRcTerm(url_) );
    const ::osl::MutexGuard guard( getMutex() );
    unorc_verify_init( xCmdEnv );
    getRcItemList(kind).remove( rcterm );
    // write immediately:
    m_unorc_modified = true;
    unorc_flush( xCmdEnv );
    return true;
}


bool BackendImpl::hasInUnoRc(
    RcItem kind, OUString const & url_ )
{
    const OUString rcterm( dp_misc::makeRcTerm(url_) );
    const ::osl::MutexGuard guard( getMutex() );
    t_stringlist const & rSet = getRcItemList(kind);
    return ::std::find( rSet.begin(), rSet.end(), rcterm ) != rSet.end();
}

css::uno::Reference< css::uno::XComponentContext > BackendImpl::getRootContext()
    const
{
    css::uno::Reference< css::uno::XComponentContext > rootContext(
        getComponentContext()->getValueByName("_root"),
        css::uno::UNO_QUERY);
    return rootContext.is() ? rootContext : getComponentContext();
}


void BackendImpl::releaseObject( OUString const & id )
{
    const ::osl::MutexGuard guard( getMutex() );
    m_backendObjects.erase( id );
}


Reference<XInterface> BackendImpl::getObject( OUString const & id )
{
    const ::osl::MutexGuard guard( getMutex() );
    const t_string2object::const_iterator iFind( m_backendObjects.find( id ) );
    if (iFind == m_backendObjects.end())
        return Reference<XInterface>();
    else
        return iFind->second;
}


Reference<XInterface> BackendImpl::insertObject(
    OUString const & id, Reference<XInterface> const & xObject )
{
    const ::osl::MutexGuard guard( getMutex() );
    const ::std::pair<t_string2object::iterator, bool> insertion(
        m_backendObjects.insert( t_string2object::value_type(
                                     id, xObject ) ) );
    return insertion.first->second;
}


Reference<XComponentContext> raise_uno_process(
    Reference<XComponentContext> const & xContext,
    ::rtl::Reference<AbortChannel> const & abortChannel )
{
    OSL_ASSERT( xContext.is() );

    OUString url( util::theMacroExpander::get(xContext)->expandMacros( "$URE_BIN_DIR/uno" ) );

    const OUString connectStr = "uno:pipe,name=" + generateRandomPipeId() + ";urp;uno.ComponentContext";

    // raise core UNO process to register/run a component,
    // javavm service uses unorc next to executable to retrieve deployed
    // jar typelibs

    ::std::vector<OUString> args{
#if OSL_DEBUG_LEVEL == 0
        "--quiet",
#endif
        "--singleaccept",
        "-u",
        connectStr,
        // don't inherit from unorc:
        "-env:INIFILENAME=" };

    //now add the bootstrap variables which were supplied on the command line
    ::std::vector<OUString> bootvars = getCmdBootstrapVariables();
    args.insert(args.end(), bootvars.begin(), bootvars.end());

    oslProcess hProcess;
    try {
        hProcess = raiseProcess(
            url, comphelper::containerToSequence(args) );
    }
    catch (...) {
        OUString sMsg = "error starting process: " + url;
        for(const auto& arg : args)
            sMsg += " " + arg;
        throw uno::RuntimeException(sMsg);
    }
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


namespace {

void extractComponentData(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    css::uno::Reference< css::registry::XRegistryKey > const & registry,
    ComponentBackendDb::Data * data,
    std::vector< css::uno::Reference< css::uno::XInterface > > * factories,
    css::uno::Reference< css::loader::XImplementationLoader > const &
        componentLoader,
    OUString const & componentUrl)
{
    OSL_ASSERT(
        context.is() && registry.is() && data != nullptr && componentLoader.is());
    OUString registryName(registry->getKeyName());
    sal_Int32 prefix = registryName.getLength();
    if (!registryName.endsWith("/")) {
        prefix += RTL_CONSTASCII_LENGTH("/");
    }
    css::uno::Sequence< css::uno::Reference< css::registry::XRegistryKey > >
        keys(registry->openKeys());
    css::uno::Reference< css::lang::XMultiComponentFactory > smgr(
        context->getServiceManager(), css::uno::UNO_QUERY_THROW);
    for (sal_Int32 i = 0; i < keys.getLength(); ++i) {
        OUString name(keys[i]->getKeyName().copy(prefix));
        data->implementationNames.push_back(name);
        css::uno::Reference< css::registry::XRegistryKey > singletons(
            keys[i]->openKey("UNO/SINGLETONS"));
        if (singletons.is()) {
            sal_Int32 prefix2 = keys[i]->getKeyName().getLength() +
                RTL_CONSTASCII_LENGTH("/UNO/SINGLETONS/");
            css::uno::Sequence<
                css::uno::Reference< css::registry::XRegistryKey > >
                singletonKeys(singletons->openKeys());
            for (sal_Int32 j = 0; j < singletonKeys.getLength(); ++j) {
                data->singletons.push_back(
                    std::pair< OUString, OUString >(
                        singletonKeys[j]->getKeyName().copy(prefix2), name));
            }
        }
        if (factories != nullptr) {
            factories->push_back(
                componentLoader->activate(
                    name, OUString(), componentUrl, keys[i]));
        }
    }
}

}

void BackendImpl::ComponentPackageImpl::getComponentInfo(
    ComponentBackendDb::Data * data,
    std::vector< css::uno::Reference< css::uno::XInterface > > * factories,
    Reference<XComponentContext> const & xContext )
{
    const Reference<loader::XImplementationLoader> xLoader(
        xContext->getServiceManager()->createInstanceWithContext(
            m_loader, xContext ), UNO_QUERY );
    if (! xLoader.is())
    {
        throw css::deployment::DeploymentException(
            "cannot instantiate loader " + m_loader,
            static_cast< OWeakObject * >(this), Any());
    }

    // HACK: highly dependent on stoc/source/servicemanager
    //       and stoc/source/implreg implementation which rely on the same
    //       services.rdb format!
    //       .../UNO/LOCATION and .../UNO/ACTIVATOR appear not to be written by
    //       writeRegistryInfo, however, but are known, fixed values here, so
    //       can be passed into extractComponentData
    OUString url(getURL());
    const Reference<registry::XSimpleRegistry> xMemReg(
        xContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.registry.SimpleRegistry", xContext ),
        UNO_QUERY_THROW );
    xMemReg->open( OUString() /* in mem */, false, true );
    xLoader->writeRegistryInfo( xMemReg->getRootKey(), OUString(), url );
    extractComponentData(
        xContext, xMemReg->getRootKey(), data, factories, xLoader, url);
}

void BackendImpl::ComponentPackageImpl::componentLiveInsertion(
    ComponentBackendDb::Data const & data,
    std::vector< css::uno::Reference< css::uno::XInterface > > const &
        factories)
{
    css::uno::Reference< css::uno::XComponentContext > rootContext(
        getMyBackend()->getRootContext());
    css::uno::Reference< css::container::XSet > set(
        rootContext->getServiceManager(), css::uno::UNO_QUERY_THROW);
    std::vector< css::uno::Reference< css::uno::XInterface > >::const_iterator
        factory(factories.begin());
    for (t_stringlist::const_iterator i(data.implementationNames.begin());
         i != data.implementationNames.end(); ++i)
    {
        try {
            set->insert(css::uno::Any(*factory++));
        } catch (const container::ElementExistException &) {
            OSL_TRACE(
                "implementation %s already registered",
                OUStringToOString(*i, RTL_TEXTENCODING_UTF8).getStr());
        }
    }
    if (!data.singletons.empty()) {
        css::uno::Reference< css::container::XNameContainer > cont(
            rootContext, css::uno::UNO_QUERY_THROW);
        for (t_stringpairvec::const_iterator i(data.singletons.begin());
             i != data.singletons.end(); ++i)
        {
            OUString name("/singletons/" + i->first);
            //TODO: Update should be atomic:
            try {
                cont->removeByName( name + "/arguments");
            } catch (const container::NoSuchElementException &) {}
            try {
                cont->insertByName( name + "/service", css::uno::Any(i->second));
            } catch (const container::ElementExistException &) {
                cont->replaceByName( name + "/service", css::uno::Any(i->second));
            }
            try {
                cont->insertByName(name, css::uno::Any());
            } catch (const container::ElementExistException &) {
                OSL_TRACE(
                    "singleton %s already registered",
                    OUStringToOString(
                        i->first, RTL_TEXTENCODING_UTF8).getStr());
                cont->replaceByName(name, css::uno::Any());
            }
        }
    }
}

void BackendImpl::ComponentPackageImpl::componentLiveRemoval(
    ComponentBackendDb::Data const & data)
{
    css::uno::Reference< css::uno::XComponentContext > rootContext(
        getMyBackend()->getRootContext());
    css::uno::Reference< css::container::XSet > set(
        rootContext->getServiceManager(), css::uno::UNO_QUERY_THROW);
    for (t_stringlist::const_iterator i(data.implementationNames.begin());
         i != data.implementationNames.end(); ++i)
    {
        try {
            set->remove(css::uno::Any(*i));
        } catch (const css::container::NoSuchElementException &) {
            // ignore if factory has not been live deployed
        }
    }
    if (!data.singletons.empty()) {
        css::uno::Reference< css::container::XNameContainer > cont(
            rootContext, css::uno::UNO_QUERY_THROW);
        for (t_stringpairvec::const_iterator i(data.singletons.begin());
             i != data.singletons.end(); ++i)
        {
            OUString name("/singletons/" + i->first);
            //TODO: Removal should be atomic:
            try {
                cont->removeByName(name);
            } catch (const container::NoSuchElementException &) {}
            try {
                cont->removeByName( name + "/service" );
            } catch (const container::NoSuchElementException &) {}
            try {
                cont->removeByName( name + "/arguments" );
            } catch (const container::NoSuchElementException &) {}
        }
    }
}

// Package

//We could use here BackendImpl::hasActiveEntry. However, this check is just as well.
//And it also shows the problem if another extension has overwritten an implementation
//entry, because it contains the same service implementation
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
            bool bAmbiguousComponentName = false;
            // lookup rdb for location URL:
            const Reference<registry::XRegistryKey> xRootKey(
                xRDB->getRootKey() );
            const Reference<registry::XRegistryKey> xImplKey(
                xRootKey->openKey( "IMPLEMENTATIONS" ) );
            Sequence<OUString> implNames;
            if (xImplKey.is() && xImplKey->isValid())
                implNames = xImplKey->getKeyNames();
            OUString const * pImplNames = implNames.getConstArray();
            sal_Int32 pos = implNames.getLength();
            for ( ; pos--; )
            {
                checkAborted( abortChannel );
                const OUString key(
                    pImplNames[ pos ] + "/UNO/LOCATION" );
                const Reference<registry::XRegistryKey> xKey(
                    xRootKey->openKey(key) );
                if (xKey.is() && xKey->isValid())
                {
                    const OUString location( xKey->getAsciiValue() );
                    if (location.equalsIgnoreAsciiCase( getURL() ))
                    {
                        break;
                    }
                    else
                    {
                        //try to match only the file name
                        OUString thisUrl(getURL());
                        OUString thisFileName(thisUrl.copy(thisUrl.lastIndexOf('/')));

                        OUString locationFileName(location.copy(location.lastIndexOf('/')));
                        if (locationFileName.equalsIgnoreAsciiCase(thisFileName))
                            bAmbiguousComponentName = true;
                    }
                }
            }
            if (pos >= 0)
                m_registered = REG_REGISTERED;
            else if (bAmbiguousComponentName)
                m_registered = REG_MAYBE_REGISTERED;
        }
    }

    //Different extensions can use the same service implementations. Then the extensions
    //which was installed last will overwrite the one from the other extension. That is
    //the registry will contain the path (the location) of the library or jar of the
    //second extension. In this case isRegistered called for the lib of the first extension
    //would return "not registered". That would mean that during uninstallation
    //XPackage::registerPackage is not called, because it just was not registered. This is,
    //however, necessary for jar files. Registering and unregistering update
    //uno_packages/cache/registry/com.sun.star.comp.deployment.component.PackageRegistryBackend/unorc
    //Therefore, we will return always "is ambiguous" if the path of this component cannot
    //be found in the registry and if there is another path and both have the same file name (but
    //the rest of the path is different).
    //If the caller cannot precisely determine that this package was registered, then it must
    //call registerPackage.
    bool bAmbiguous = m_registered == REG_VOID // REG_VOID == we are in the progress of unregistration
        || m_registered == REG_MAYBE_REGISTERED;
    return beans::Optional< beans::Ambiguous<sal_Bool> >(
        true /* IsPresent */,
        beans::Ambiguous<sal_Bool>(
            m_registered == REG_REGISTERED, bAmbiguous) );
}


void BackendImpl::ComponentPackageImpl::processPackage_(
    ::osl::ResettableMutexGuard &,
    bool doRegisterPackage,
    bool startup,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    OUString url(getURL());
    if (doRegisterPackage) {
        ComponentBackendDb::Data data;
        css::uno::Reference< css::uno::XComponentContext > context;
        if (startup) {
            context = that->getComponentContext();
        } else {
            context.set(that->getObject(url), css::uno::UNO_QUERY);
            if (!context.is()) {
                context.set(
                    that->insertObject(
                        url,
                        raise_uno_process(
                            that->getComponentContext(), abortChannel)),
                    css::uno::UNO_QUERY_THROW);
            }
        }
        css::uno::Reference< css::registry::XImplementationRegistration> impreg(
            context->getServiceManager()->createInstanceWithContext(
                "com.sun.star.registry.ImplementationRegistration",
                context),
            css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::registry::XSimpleRegistry > rdb(getRDB());
        impreg->registerImplementation(m_loader, url, rdb);
        // Only write to unorc after successful registration; it may fail if
        // there is no suitable java
        if (m_loader == "com.sun.star.loader.Java2" && !jarManifestHeaderPresent(url, "UNO-Type-Path", xCmdEnv))
        {
            that->addToUnoRc(RCITEM_JAR_TYPELIB, url, xCmdEnv);
            data.javaTypeLibrary = true;
        }
        std::vector< css::uno::Reference< css::uno::XInterface > > factories;
        getComponentInfo(&data, startup ? nullptr : &factories, context);
        if (!startup) {
            try {
                componentLiveInsertion(data, factories);
            } catch (css::uno::Exception & e) {
                SAL_INFO(
                    "desktop.deployment", "caught Exception " << e.Message);
                try {
                    impreg->revokeImplementation(url, rdb);
                } catch (css::uno::RuntimeException & e2) {
                    SAL_WARN(
                        "desktop.deployment",
                        "ignored RuntimeException " << e2.Message);
                }
                throw;
            }
        }
        m_registered = REG_REGISTERED;
        that->addDataToDb(url, data);
    } else { // revoke
        m_registered = REG_VOID;
        ComponentBackendDb::Data data(that->readDataFromDb(url));
        css::uno::Reference< css::uno::XComponentContext > context(
            that->getObject(url), css::uno::UNO_QUERY);
        bool remoteContext = context.is();
        if (!remoteContext) {
            context = that->getComponentContext();
        }
        if (!startup) {
            componentLiveRemoval(data);
        }
        css::uno::Reference< css::registry::XImplementationRegistration >(
            context->getServiceManager()->createInstanceWithContext(
                "com.sun.star.registry.ImplementationRegistration",
                context),
            css::uno::UNO_QUERY_THROW)->revokeImplementation(url, getRDB());
        if (data.javaTypeLibrary) {
            that->removeFromUnoRc(RCITEM_JAR_TYPELIB, url, xCmdEnv);
        }
        if (remoteContext) {
            that->releaseObject(url);
        }
        m_registered = REG_NOT_REGISTERED;
        getMyBackend()->revokeEntryFromDb(url);
    }
}

BackendImpl::TypelibraryPackageImpl::TypelibraryPackageImpl(
    ::rtl::Reference<PackageRegistryBackend> const & myBackend,
    OUString const & url, OUString const & name,
    Reference<deployment::XPackageTypeInfo> const & xPackageType,
    bool jarFile, bool bRemoved, OUString const & identifier)
    : Package( myBackend, url, name, name /* display-name */,
               xPackageType, bRemoved, identifier),
      m_jarFile( jarFile )
{
}

// Package
BackendImpl * BackendImpl::TypelibraryPackageImpl::getMyBackend() const
{
    BackendImpl * pBackend = static_cast<BackendImpl *>(m_myBackend.get());
    if (nullptr == pBackend)
    {
        //May throw a DisposedException
        check();
        //We should never get here...
        throw RuntimeException( "Failed to get the BackendImpl",
            static_cast<OWeakObject*>(const_cast<TypelibraryPackageImpl *>(this)));
    }
    return pBackend;
}

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
            that->hasInUnoRc(
                m_jarFile ? RCITEM_JAR_TYPELIB : RCITEM_RDB_TYPELIB, getURL() ),
            false /* IsAmbiguous */ ) );
}


void BackendImpl::TypelibraryPackageImpl::processPackage_(
    ::osl::ResettableMutexGuard &,
    bool doRegisterPackage,
    bool /*startup*/,
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
            css::uno::Reference< css::container::XSet >(
                that->getComponentContext()->getValueByName(
                    "/singletons"
                    "/com.sun.star.reflection.theTypeDescriptionManager"),
                css::uno::UNO_QUERY_THROW)->insert(
                    css::uno::makeAny(expandUnoRcUrl(url)));
        }

        that->addToUnoRc( m_jarFile ? RCITEM_JAR_TYPELIB : RCITEM_RDB_TYPELIB,
                          url, xCmdEnv );
    }
    else // revokePackage()
    {
        that->removeFromUnoRc(
            m_jarFile ? RCITEM_JAR_TYPELIB : RCITEM_RDB_TYPELIB, url, xCmdEnv );

        // revoking types at runtime, possible, sensible?
        if (!m_jarFile) {
            css::uno::Reference< css::container::XSet >(
                that->getComponentContext()->getValueByName(
                    "/singletons"
                    "/com.sun.star.reflection.theTypeDescriptionManager"),
                css::uno::UNO_QUERY_THROW)->remove(
                    css::uno::makeAny(expandUnoRcUrl(url)));
        }
    }
}

BackendImpl::OtherPlatformPackageImpl::OtherPlatformPackageImpl(
    ::rtl::Reference<PackageRegistryBackend> const & myBackend,
    OUString const & url, OUString const & name,
    Reference<deployment::XPackageTypeInfo> const & xPackageType,
    bool bRemoved, OUString const & identifier, OUString const& rPlatform)
    : Package(myBackend, url, name, name, xPackageType, bRemoved, identifier)
    , m_aPlatform(rPlatform)
{
    OSL_PRECOND(bRemoved, "this class can only be used for removing packages!");
}

BackendImpl *
BackendImpl::OtherPlatformPackageImpl::getMyBackend() const
{
    BackendImpl * pBackend = static_cast<BackendImpl *>(m_myBackend.get());
    if (nullptr == pBackend)
    {
        //Throws a DisposedException
        check();
        //We should never get here...
        throw RuntimeException("Failed to get the BackendImpl",
            static_cast<OWeakObject*>(const_cast<OtherPlatformPackageImpl*>(this)));
    }
    return pBackend;
}

Reference<registry::XSimpleRegistry> const
BackendImpl::OtherPlatformPackageImpl::impl_openRDB() const
{
    OUString const aRDB(m_aPlatform + ".rdb");
    OUString const aRDBPath(makeURL(getMyBackend()->getCachePath(), aRDB));

    Reference<registry::XSimpleRegistry> xRegistry;

    try
    {
        xRegistry.set(
                impl_createInstance("com.sun.star.registry.SimpleRegistry"),
                UNO_QUERY)
            ;
        if (xRegistry.is())
            xRegistry->open(expandUnoRcUrl(aRDBPath), false, false);
    }
    catch (registry::InvalidRegistryException const&)
    {
        // If the registry does not exist, we do not need to bother at all
        xRegistry.set(nullptr);
    }

    SAL_WARN_IF( !xRegistry.is(), "desktop.deployment", "could not create registry for the package's platform");
    return xRegistry;
}

Reference<XInterface> const
BackendImpl::OtherPlatformPackageImpl::impl_createInstance(OUString const& rService)
const
{
    Reference<XComponentContext> const xContext(getMyBackend()->getComponentContext());
    OSL_ASSERT(xContext.is());
    Reference<XInterface> xService;
    if (xContext.is())
        xService.set(xContext->getServiceManager()->createInstanceWithContext(rService, xContext));
    return xService;
}

beans::Optional<beans::Ambiguous<sal_Bool> >
BackendImpl::OtherPlatformPackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard& /* guard */,
    ::rtl::Reference<AbortChannel> const& /* abortChannel */,
    Reference<XCommandEnvironment> const& /* xCmdEnv */ )
{
    return beans::Optional<beans::Ambiguous<sal_Bool> >(true,
            beans::Ambiguous<sal_Bool>(true, false));
}

void
BackendImpl::OtherPlatformPackageImpl::processPackage_(
    ::osl::ResettableMutexGuard& /* guard */,
    bool bRegisterPackage,
    bool /* bStartup */,
    ::rtl::Reference<AbortChannel> const& /* abortChannel */,
    Reference<XCommandEnvironment> const& /* xCmdEnv */)
{
    OSL_PRECOND(!bRegisterPackage, "this class can only be used for removing packages!");
    (void) bRegisterPackage;

    OUString const aURL(getURL());

    Reference<registry::XSimpleRegistry> const xServicesRDB(impl_openRDB());
    Reference<registry::XImplementationRegistration> const xImplReg(
            impl_createInstance("com.sun.star.registry.ImplementationRegistration"),
            UNO_QUERY)
        ;
    if (xImplReg.is() && xServicesRDB.is())
        xImplReg->revokeImplementation(aURL, xServicesRDB);
    if (xServicesRDB.is())
        xServicesRDB->close();

    getMyBackend()->revokeEntryFromDb(aURL);
}

BackendImpl * BackendImpl::ComponentsPackageImpl::getMyBackend() const
{
    BackendImpl * pBackend = static_cast<BackendImpl *>(m_myBackend.get());
    if (nullptr == pBackend)
    {
        //Throws a DisposedException
        check();
        //We should never get here...
        throw RuntimeException("Failed to get the BackendImpl",
            static_cast<OWeakObject*>(const_cast<ComponentsPackageImpl *>(this)));
    }
    return pBackend;
}

beans::Optional< beans::Ambiguous<sal_Bool> >
BackendImpl::ComponentsPackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard &,
    ::rtl::Reference<AbortChannel> const &,
    Reference<XCommandEnvironment> const & )
{
    return beans::Optional< beans::Ambiguous<sal_Bool> >(
        true,
        beans::Ambiguous<sal_Bool>(
            getMyBackend()->hasInUnoRc(RCITEM_COMPONENTS, getURL()), false));
}

void BackendImpl::ComponentsPackageImpl::processPackage_(
    ::osl::ResettableMutexGuard &,
    bool doRegisterPackage,
    bool startup,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    OUString url(getURL());
    if (doRegisterPackage) {
        if (!startup) {
            css::uno::Reference< css::uno::XComponentContext > context(
                that->getObject(url), css::uno::UNO_QUERY);
            if (!context.is()) {
                context.set(
                    that->insertObject(
                        url,
                        raise_uno_process(
                            that->getComponentContext(), abortChannel)),
                    css::uno::UNO_QUERY_THROW);
            }
            // This relies on the root component context's service manager
            // supporting the extended XSet semantics:
            css::uno::Sequence< css::beans::NamedValue > args
            {
                { "uri", css::uno::makeAny(expandUnoRcUrl(url)) },
                { "component-context", css::uno::makeAny(context) }
            };
            css::uno::Reference< css::container::XSet > smgr(
                that->getRootContext()->getServiceManager(),
                css::uno::UNO_QUERY_THROW);
            smgr->insert(css::uno::makeAny(args));
        }
        that->addToUnoRc(RCITEM_COMPONENTS, url, xCmdEnv);
    } else { // revoke
        that->removeFromUnoRc(RCITEM_COMPONENTS, url, xCmdEnv);
        if (!startup) {
            // This relies on the root component context's service manager
            // supporting the extended XSet semantics:
            css::uno::Sequence< css::beans::NamedValue > args { { "uri", css::uno::makeAny(expandUnoRcUrl(url)) } };
            css::uno::Reference< css::container::XSet > smgr(
                that->getRootContext()->getServiceManager(),
                css::uno::UNO_QUERY_THROW);
            smgr->remove(css::uno::makeAny(args));
        }
        that->releaseObject(url);
        that->revokeEntryFromDb(url); // in case it got added with old code
    }
}

BackendImpl::ComponentsPackageImpl::ComponentsPackageImpl(
    ::rtl::Reference<PackageRegistryBackend> const & myBackend,
    OUString const & url, OUString const & name,
    Reference<deployment::XPackageTypeInfo> const & xPackageType,
    bool bRemoved, OUString const & identifier)
    : Package( myBackend, url, name, name /* display-name */,
               xPackageType, bRemoved, identifier)
{}

} // anon namespace

namespace sdecl = comphelper::service_decl;
sdecl::class_<BackendImpl, sdecl::with_args<true> > serviceBI;
extern sdecl::ServiceDecl const serviceDecl(
    serviceBI,
    IMPLEMENTATION_NAME,
    BACKEND_SERVICE_NAME );

} // namespace component
} // namespace backend
} // namespace dp_registry


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
