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


#include "dp_script.hrc"
#include "dp_lib_container.h"
#include "dp_backend.h"
#include "dp_ucb.h"
#include <rtl/uri.hxx>
#include <ucbhelper/content.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/servicedecl.hxx>
#include <svl/inettype.hxx>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/script/XLibraryContainer3.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <memory>
#include "dp_scriptbackenddb.hxx"

using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;

namespace dp_registry {
namespace backend {
namespace script {
namespace {

typedef ::cppu::ImplInheritanceHelper<
    ::dp_registry::backend::PackageRegistryBackend, util::XUpdatable > t_helper;

class BackendImpl : public t_helper
{
    class PackageImpl : public ::dp_registry::backend::Package
    {
        BackendImpl * getMyBackend() const;

        const OUString m_scriptURL;
        const OUString m_dialogURL;
        OUString m_dialogName;

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
        PackageImpl(
            ::rtl::Reference<BackendImpl> const & myBackend,
            OUString const & url,
            Reference<XCommandEnvironment> const &xCmdEnv,
            OUString const & scriptURL, OUString const & dialogURL,
            bool bRemoved, OUString const & identifier);
    };
    friend class PackageImpl;

    // PackageRegistryBackend
    virtual Reference<deployment::XPackage> bindPackage_(
        OUString const & url, OUString const & mediaType,
        bool bRemoved, OUString const & identifier,
        Reference<XCommandEnvironment> const & xCmdEnv ) override;

    void addDataToDb(OUString const & url);
    bool hasActiveEntry(OUString const & url);
    void revokeEntryFromDb(OUString const & url);

    const Reference<deployment::XPackageTypeInfo> m_xBasicLibTypeInfo;
    const Reference<deployment::XPackageTypeInfo> m_xDialogLibTypeInfo;
    Sequence< Reference<deployment::XPackageTypeInfo> > m_typeInfos;
    std::unique_ptr<ScriptBackendDb> m_backendDb;
public:
    BackendImpl( Sequence<Any> const & args,
                 Reference<XComponentContext> const & xComponentContext );

    // XUpdatable
    virtual void SAL_CALL update() throw (RuntimeException, std::exception) override;

    // XPackageRegistry
    virtual Sequence< Reference<deployment::XPackageTypeInfo> > SAL_CALL
    getSupportedPackageTypes() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL packageRemoved(OUString const & url, OUString const & mediaType)
        throw (deployment::DeploymentException,
               uno::RuntimeException, std::exception) override;

};


BackendImpl::PackageImpl::PackageImpl(
    ::rtl::Reference<BackendImpl> const & myBackend,
    OUString const & url,
    Reference<XCommandEnvironment> const &xCmdEnv,
    OUString const & scriptURL, OUString const & dialogURL, bool bRemoved,
    OUString const & identifier)
    : Package( myBackend.get(), url,
               OUString(), OUString(), // will be late-initialized
               !scriptURL.isEmpty() ? myBackend->m_xBasicLibTypeInfo
               : myBackend->m_xDialogLibTypeInfo, bRemoved, identifier),
      m_scriptURL( scriptURL ),
      m_dialogURL( dialogURL )
{
    // name, displayName:
    if (!dialogURL.isEmpty()) {
        m_dialogName = LibraryContainer::get_libname(
            dialogURL, xCmdEnv, myBackend->getComponentContext() );
    }
    if (!scriptURL.isEmpty()) {
        m_name = LibraryContainer::get_libname(
            scriptURL, xCmdEnv, myBackend->getComponentContext() );
    }
    else
        m_name = m_dialogName;
    m_displayName = m_name;
}


BackendImpl::BackendImpl(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xComponentContext )
    : t_helper( args, xComponentContext ),
      m_xBasicLibTypeInfo( new Package::TypeInfo(
                               "application/vnd.sun.star.basic-library",
                               OUString() /* no file filter */,
                               getResourceString(RID_STR_BASIC_LIB),
                               RID_IMG_SCRIPTLIB) ),
      m_xDialogLibTypeInfo( new Package::TypeInfo(
                                "application/vnd.sun.star.dialog-library",
                                OUString() /* no file filter */,
                                getResourceString(RID_STR_DIALOG_LIB),
                                RID_IMG_DIALOGLIB) ),
      m_typeInfos( 2 )
{
    m_typeInfos[ 0 ] = m_xBasicLibTypeInfo;
    m_typeInfos[ 1 ] = m_xDialogLibTypeInfo;

    OSL_ASSERT( ! transientMode() );

    if (!transientMode())
    {
        OUString dbFile = makeURL(getCachePath(), "backenddb.xml");
        m_backendDb.reset(
            new ScriptBackendDb(getComponentContext(), dbFile));
    }

}
void BackendImpl::addDataToDb(OUString const & url)
{
    if (m_backendDb.get())
        m_backendDb->addEntry(url);
}

bool BackendImpl::hasActiveEntry(OUString const & url)
{
    if (m_backendDb.get())
        return m_backendDb->hasActiveEntry(url);
    return false;
}

// XUpdatable

void BackendImpl::update() throw (RuntimeException, std::exception)
{
    // Nothing to do here after fixing i70283!?
}

// XPackageRegistry

Sequence< Reference<deployment::XPackageTypeInfo> >
BackendImpl::getSupportedPackageTypes() throw (RuntimeException, std::exception)
{
    return m_typeInfos;
}
void BackendImpl::revokeEntryFromDb(OUString const & url)
{
    if (m_backendDb.get())
        m_backendDb->revokeEntry(url);
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
    OUString mediaType( mediaType_ );
    if (mediaType.isEmpty())
    {
        // detect media-type:
        ::ucbhelper::Content ucbContent;
        if (create_ucb_content( &ucbContent, url, xCmdEnv ) &&
            ucbContent.isFolder())
        {
            // probe for script.xlb:
            if (create_ucb_content(
                    nullptr, makeURL( url, "script.xlb" ),
                    xCmdEnv, false /* no throw */ ))
                mediaType = "application/vnd.sun.star.basic-library";
            // probe for dialog.xlb:
            else if (create_ucb_content(
                         nullptr, makeURL( url, "dialog.xlb" ),
                         xCmdEnv, false /* no throw */ ))
                mediaType = "application/vnd.sun.star.dialog-library";
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
            OUString dialogURL( makeURL( url, "dialog.xlb" ) );
            if (! create_ucb_content(
                    nullptr, dialogURL, xCmdEnv, false /* no throw */ )) {
                dialogURL.clear();
            }

            if (subType.equalsIgnoreAsciiCase("vnd.sun.star.basic-library"))
            {
                OUString scriptURL( makeURL( url, "script.xlb"));
                if (! create_ucb_content(
                        nullptr, scriptURL, xCmdEnv, false /* no throw */ )) {
                    scriptURL.clear();
                }

                return new PackageImpl(
                    this, url, xCmdEnv, scriptURL,
                    dialogURL, bRemoved, identifier);
            }
            else if (subType.equalsIgnoreAsciiCase(
                         "vnd.sun.star.dialog-library")) {
                return new PackageImpl(
                    this, url, xCmdEnv,
                    OUString() /* no script lib */,
                    dialogURL,
                    bRemoved, identifier);
            }
        }
    }
    throw lang::IllegalArgumentException(
        StrUnsupportedMediaType::get() + mediaType,
        static_cast<OWeakObject *>(this),
        static_cast<sal_Int16>(-1) );
}


// Package
BackendImpl * BackendImpl::PackageImpl::getMyBackend() const
{
    BackendImpl * pBackend = static_cast<BackendImpl *>(m_myBackend.get());
    if (nullptr == pBackend)
    {
        //May throw a DisposedException
        check();
        //We should never get here...
        throw RuntimeException(
            "Failed to get the BackendImpl",
            static_cast<OWeakObject*>(const_cast<PackageImpl *>(this)));
    }
    return pBackend;
}

beans::Optional< beans::Ambiguous<sal_Bool> >
BackendImpl::PackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard & /* guard */,
    ::rtl::Reference<AbortChannel> const & /* abortChannel */,
    Reference<XCommandEnvironment> const & /* xCmdEnv */ )
{
    BackendImpl * that = getMyBackend();
    Reference< deployment::XPackage > xThisPackage( this );

    bool registered = that->hasActiveEntry(getURL());
    return beans::Optional< beans::Ambiguous<sal_Bool> >(
        true /* IsPresent */,
        beans::Ambiguous<sal_Bool>( registered, false /* IsAmbiguous */ ) );
}

void
lcl_maybeRemoveScript(
        bool const bExists,
        OUString const& rName,
        OUString const& rScriptURL,
        Reference<css::script::XLibraryContainer3> const& xScriptLibs)
{
    if (bExists && xScriptLibs.is() && xScriptLibs->hasByName(rName))
    {
        const OUString sScriptUrl = xScriptLibs->getOriginalLibraryLinkURL(rName);
        if (sScriptUrl == rScriptURL)
            xScriptLibs->removeLibrary(rName);
    }
}

bool
lcl_maybeAddScript(
        bool const bExists,
        OUString const& rName,
        OUString const& rScriptURL,
        Reference<css::script::XLibraryContainer3> const& xScriptLibs)
{
    if (bExists && xScriptLibs.is())
    {
        bool bCanAdd = true;
        if (xScriptLibs->hasByName(rName))
        {
            const OUString sOriginalUrl = xScriptLibs->getOriginalLibraryLinkURL(rName);
            //We assume here that library names in extensions are unique, which may not be the case
            //ToDo: If the script exist in another extension, then both extensions must have the
            //same id
            if (sOriginalUrl.match("vnd.sun.star.expand:$UNO_USER_PACKAGES_CACHE")
                || sOriginalUrl.match("vnd.sun.star.expand:$UNO_SHARED_PACKAGES_CACHE")
                || sOriginalUrl.match("vnd.sun.star.expand:$BUNDLED_EXTENSIONS")
                || sOriginalUrl.match("$(INST)/share/basic/Access2Base/"))
            {
                xScriptLibs->removeLibrary(rName);
                bCanAdd = true;
            }
            else
            {
                bCanAdd = false;
            }
        }

        if (bCanAdd)
        {
            xScriptLibs->createLibraryLink(rName, rScriptURL, false);
            return xScriptLibs->hasByName(rName);
        }
    }

    return false;
}

void BackendImpl::PackageImpl::processPackage_(
    ::osl::ResettableMutexGuard & /* guard */,
    bool doRegisterPackage,
    bool startup,
    ::rtl::Reference<AbortChannel> const & /* abortChannel */,
    Reference<XCommandEnvironment> const & /* xCmdEnv */ )
{
    BackendImpl * that = getMyBackend();

    Reference< deployment::XPackage > xThisPackage( this );
    Reference<XComponentContext> const & xComponentContext = that->getComponentContext();

    bool bScript = !m_scriptURL.isEmpty();
    Reference<css::script::XLibraryContainer3> xScriptLibs;

    bool bDialog = !m_dialogURL.isEmpty();
    Reference<css::script::XLibraryContainer3> xDialogLibs;

    bool bRunning = !startup && office_is_running();
    if( bRunning )
    {
        if( bScript )
        {
            xScriptLibs.set(
                xComponentContext->getServiceManager()->createInstanceWithContext(
                    "com.sun.star.script.ApplicationScriptLibraryContainer",
                    xComponentContext ), UNO_QUERY_THROW );
        }

        if( bDialog )
        {
            xDialogLibs.set(
                xComponentContext->getServiceManager()->createInstanceWithContext(
                    "com.sun.star.script.ApplicationDialogLibraryContainer",
                    xComponentContext ), UNO_QUERY_THROW );
        }
    }
    bool bRegistered = getMyBackend()->hasActiveEntry(getURL());
    if( !doRegisterPackage )
    {
        //We cannot just call removeLibrary(name) because this could remove a
        //script which was added by an extension in a different repository. For
        //example, extension foo is contained in the bundled repository and then
        //the user adds it to the user repository. The extension manager will
        //then register the new script and revoke the script from the bundled
        //extension. removeLibrary(name) would now remove the script from the
        //user repository. That is, the script of the newly added user extension does
        //not work anymore. Therefore we must check if the currently active
        //script comes in fact from the currently processed extension.

        if (bRegistered)
        {
            //we also prevent and live deployment at startup
            if (!isRemoved() && !startup)
            {
                lcl_maybeRemoveScript(bScript, m_name, m_scriptURL, xScriptLibs);
                lcl_maybeRemoveScript(bDialog, m_dialogName, m_dialogURL, xDialogLibs);
            }
            getMyBackend()->revokeEntryFromDb(getURL());
            return;
        }
    }
    if (bRegistered)
        return;     // Already registered

    // Update LibraryContainer
    bool bScriptSuccess = false;
    bool bDialogSuccess = false;
    if (!startup)
    {
        //If there is a bundled extension, and the user installes the same extension
        //then the script from the bundled extension must be removed. If this does not work
        //then live deployment does not work for scripts.
        bScriptSuccess = lcl_maybeAddScript(bScript, m_name, m_scriptURL, xScriptLibs);
        bDialogSuccess = lcl_maybeAddScript(bDialog, m_dialogName, m_dialogURL, xDialogLibs);
    }
    bool bSuccess = bScript || bDialog;     // Something must have happened
    if( bRunning )
        if( (bScript && !bScriptSuccess) || (bDialog && !bDialogSuccess) )
            bSuccess = false;

    if (bSuccess)
        getMyBackend()->addDataToDb(getURL());
}

} // anon namespace

namespace sdecl = comphelper::service_decl;
sdecl::class_<BackendImpl, sdecl::with_args<true> > serviceBI;
extern sdecl::ServiceDecl const serviceDecl(
    serviceBI,
    "com.sun.star.comp.deployment.script.PackageRegistryBackend",
    BACKEND_SERVICE_NAME );

} // namespace script
} // namespace backend
} // namespace dp_registry

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
