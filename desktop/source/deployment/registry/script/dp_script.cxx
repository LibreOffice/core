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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "dp_script.hrc"
#include "dp_lib_container.h"
#include "dp_backend.h"
#include "dp_ucb.h"
#include "rtl/uri.hxx"
#include "ucbhelper/content.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/implbase1.hxx"
#include "comphelper/servicedecl.hxx"
#include "svl/inettype.hxx"
#include "com/sun/star/util/XUpdatable.hpp"
#include "com/sun/star/script/XLibraryContainer3.hpp"
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <memory>
#include "dp_scriptbackenddb.hxx"

using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;
namespace css = ::com::sun::star;

namespace dp_registry {
namespace backend {
namespace script {
namespace {

typedef ::cppu::ImplInheritanceHelper1<
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
            Reference<XCommandEnvironment> const & xCmdEnv );
        virtual void processPackage_(
            ::osl::ResettableMutexGuard & guard,
            bool registerPackage,
            bool startup,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv );

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
        sal_Bool bRemoved, OUString const & identifier,
        Reference<XCommandEnvironment> const & xCmdEnv );

    void addDataToDb(OUString const & url);
    bool hasActiveEntry(OUString const & url);
    void revokeEntryFromDb(OUString const & url);

    const Reference<deployment::XPackageTypeInfo> m_xBasicLibTypeInfo;
    const Reference<deployment::XPackageTypeInfo> m_xDialogLibTypeInfo;
    Sequence< Reference<deployment::XPackageTypeInfo> > m_typeInfos;
    std::auto_ptr<ScriptBackendDb> m_backendDb;
public:
    BackendImpl( Sequence<Any> const & args,
                 Reference<XComponentContext> const & xComponentContext );

    // XUpdatable
    virtual void SAL_CALL update() throw (RuntimeException);

    // XPackageRegistry
    virtual Sequence< Reference<deployment::XPackageTypeInfo> > SAL_CALL
    getSupportedPackageTypes() throw (RuntimeException);
    virtual void SAL_CALL packageRemoved(OUString const & url, OUString const & mediaType)
        throw (deployment::DeploymentException,
               uno::RuntimeException);

};

//______________________________________________________________________________
BackendImpl::PackageImpl::PackageImpl(
    ::rtl::Reference<BackendImpl> const & myBackend,
    OUString const & url,
    Reference<XCommandEnvironment> const &xCmdEnv,
    OUString const & scriptURL, OUString const & dialogURL, bool bRemoved,
    OUString const & identifier)
    : Package( myBackend.get(), url,
               OUString(), OUString(), // will be late-initialized
               scriptURL.getLength() > 0 ? myBackend->m_xBasicLibTypeInfo
               : myBackend->m_xDialogLibTypeInfo, bRemoved, identifier),
      m_scriptURL( scriptURL ),
      m_dialogURL( dialogURL )
{
    // name, displayName:
    if (dialogURL.getLength() > 0) {
        m_dialogName = LibraryContainer::get_libname(
            dialogURL, xCmdEnv, myBackend->getComponentContext() );
    }
    if (scriptURL.getLength() > 0) {
        m_name = LibraryContainer::get_libname(
            scriptURL, xCmdEnv, myBackend->getComponentContext() );
    }
    else
        m_name = m_dialogName;
    m_displayName = m_name;
}

//______________________________________________________________________________
BackendImpl::BackendImpl(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xComponentContext )
    : t_helper( args, xComponentContext ),
      m_xBasicLibTypeInfo( new Package::TypeInfo(
                               OUSTR("application/"
                                     "vnd.sun.star.basic-library"),
                               OUString() /* no file filter */,
                               getResourceString(RID_STR_BASIC_LIB),
                               RID_IMG_SCRIPTLIB) ),
      m_xDialogLibTypeInfo( new Package::TypeInfo(
                                OUSTR("application/"
                                      "vnd.sun.star.dialog-library"),
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
        OUString dbFile = makeURL(getCachePath(), OUSTR("backenddb.xml"));
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
//______________________________________________________________________________
void BackendImpl::update() throw (RuntimeException)
{
    // Nothing to do here after fixing i70283!?
}

// XPackageRegistry
//______________________________________________________________________________
Sequence< Reference<deployment::XPackageTypeInfo> >
BackendImpl::getSupportedPackageTypes() throw (RuntimeException)
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
               uno::RuntimeException)
{
    if (m_backendDb.get())
        m_backendDb->removeEntry(url);
}

// PackageRegistryBackend
//______________________________________________________________________________
Reference<deployment::XPackage> BackendImpl::bindPackage_(
    OUString const & url, OUString const & mediaType_,
    sal_Bool bRemoved, OUString const & identifier,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    OUString mediaType( mediaType_ );
    if (mediaType.getLength() == 0)
    {
        // detect media-type:
        ::ucbhelper::Content ucbContent;
        if (create_ucb_content( &ucbContent, url, xCmdEnv ) &&
            ucbContent.isFolder())
        {
            // probe for script.xlb:
            if (create_ucb_content(
                    0, makeURL( url, OUSTR("script.xlb") ),
                    xCmdEnv, false /* no throw */ ))
                mediaType = OUSTR("application/vnd.sun.star.basic-library");
            // probe for dialog.xlb:
            else if (create_ucb_content(
                         0, makeURL( url, OUSTR("dialog.xlb") ),
                         xCmdEnv, false /* no throw */ ))
                mediaType = OUSTR("application/vnd.sun.star.dialog-library");
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
            OUString dialogURL( makeURL( url, OUSTR("dialog.xlb") ) );
            if (! create_ucb_content(
                    0, dialogURL, xCmdEnv, false /* no throw */ )) {
                dialogURL = OUString();
            }

            if (subType.EqualsIgnoreCaseAscii("vnd.sun.star.basic-library"))
            {
                OUString scriptURL( makeURL( url, OUSTR("script.xlb")));
                if (! create_ucb_content(
                        0, scriptURL, xCmdEnv, false /* no throw */ )) {
                    scriptURL = OUString();
                }

                return new PackageImpl(
                    this, url, xCmdEnv, scriptURL,
                    dialogURL, bRemoved, identifier);
            }
            else if (subType.EqualsIgnoreCaseAscii(
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
        if (sScriptUrl.equals(rScriptURL))
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
            if (sOriginalUrl.match(OUSTR("vnd.sun.star.expand:$UNO_USER_PACKAGES_CACHE"))
                || sOriginalUrl.match(OUSTR("vnd.sun.star.expand:$UNO_SHARED_PACKAGES_CACHE"))
                || sOriginalUrl.match(OUSTR("vnd.sun.star.expand:$BUNDLED_EXTENSIONS")))
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

    bool bScript = (m_scriptURL.getLength() > 0);
    Reference<css::script::XLibraryContainer3> xScriptLibs;

    bool bDialog = (m_dialogURL.getLength() > 0);
    Reference<css::script::XLibraryContainer3> xDialogLibs;

    bool bRunning = office_is_running();
    if( bRunning )
    {
        if( bScript )
        {
            xScriptLibs.set(
                xComponentContext->getServiceManager()->createInstanceWithContext(
                    OUSTR("com.sun.star.script.ApplicationScriptLibraryContainer"),
                    xComponentContext ), UNO_QUERY_THROW );
        }

        if( bDialog )
        {
            xDialogLibs.set(
                xComponentContext->getServiceManager()->createInstanceWithContext(
                    OUSTR("com.sun.star.script.ApplicationDialogLibraryContainer"),
                    xComponentContext ), UNO_QUERY_THROW );
        }
    }
    bool bRegistered = getMyBackend()->hasActiveEntry(getURL());
    if( !doRegisterPackage )
    {
        //We cannot just call removeLibrary(name) because this could remove a
        //script which was added by an extension in a different repository. For
        //example, extension foo is contained in the bundled repository and then
        //the user adds it it to the user repository. The extension manager will
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
    if( bRunning && !startup)
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
