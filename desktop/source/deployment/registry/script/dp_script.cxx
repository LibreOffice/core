/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_script.cxx,v $
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
#include "com/sun/star/script/XLibraryContainer.hpp"
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarExpandUrl.hpp>
#include <memory>

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

//==============================================================================
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
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv );

    public:
        PackageImpl(
            ::rtl::Reference<BackendImpl> const & myBackend,
            OUString const & url,
            Reference<XCommandEnvironment> const &xCmdEnv,
            OUString const & scriptURL, OUString const & dialogURL );
    };
    friend class PackageImpl;

    // PackageRegistryBackend
    virtual Reference<deployment::XPackage> bindPackage_(
        OUString const & url, OUString const & mediaType,
        Reference<XCommandEnvironment> const & xCmdEnv );

    rtl::OUString getRegisteredFlagFileURL( Reference< deployment::XPackage > xPackage );
    rtl::OUString expandURL( const rtl::OUString& aURL );
    Reference< ucb::XSimpleFileAccess > getFileAccess( void );
    Reference< ucb::XSimpleFileAccess > m_xSFA;

    const Reference<deployment::XPackageTypeInfo> m_xBasicLibTypeInfo;
    const Reference<deployment::XPackageTypeInfo> m_xDialogLibTypeInfo;
    Sequence< Reference<deployment::XPackageTypeInfo> > m_typeInfos;

public:
    BackendImpl( Sequence<Any> const & args,
                 Reference<XComponentContext> const & xComponentContext );

    // XUpdatable
    virtual void SAL_CALL update() throw (RuntimeException);

    // XPackageRegistry
    virtual Sequence< Reference<deployment::XPackageTypeInfo> > SAL_CALL
    getSupportedPackageTypes() throw (RuntimeException);
};

//______________________________________________________________________________
BackendImpl::PackageImpl::PackageImpl(
    ::rtl::Reference<BackendImpl> const & myBackend,
    OUString const & url,
    Reference<XCommandEnvironment> const &xCmdEnv,
    OUString const & scriptURL, OUString const & dialogURL )
    : Package( myBackend.get(), url,
               OUString(), OUString(), // will be late-initialized
               scriptURL.getLength() > 0 ? myBackend->m_xBasicLibTypeInfo
                                         : myBackend->m_xDialogLibTypeInfo ),
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
                               RID_IMG_SCRIPTLIB, RID_IMG_SCRIPTLIB_HC ) ),
      m_xDialogLibTypeInfo( new Package::TypeInfo(
                                OUSTR("application/"
                                      "vnd.sun.star.dialog-library"),
                                OUString() /* no file filter */,
                                getResourceString(RID_STR_DIALOG_LIB),
                                RID_IMG_DIALOGLIB, RID_IMG_DIALOGLIB_HC ) ),
      m_typeInfos( 2 )
{
    m_typeInfos[ 0 ] = m_xBasicLibTypeInfo;
    m_typeInfos[ 1 ] = m_xDialogLibTypeInfo;

    OSL_ASSERT( ! transientMode() );
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
            if (subType.EqualsIgnoreCaseAscii("vnd.sun.star.basic-library"))
            {
                OUString dialogURL( makeURL( url, OUSTR("dialog.xlb") ) );
                if (! create_ucb_content(
                        0, dialogURL, xCmdEnv, false /* no throw */ )) {
                    dialogURL = OUString();
                }
                return new PackageImpl( this, url, xCmdEnv,
                                        makeURL( url, OUSTR("script.xlb") ),
                                        dialogURL );
            }
            else if (subType.EqualsIgnoreCaseAscii(
                         "vnd.sun.star.dialog-library")) {
                return new PackageImpl( this, url, xCmdEnv,
                                        OUString() /* no script lib */,
                                        makeURL( url, OUSTR("dialog.xlb") ) );
            }
        }
    }
    throw lang::IllegalArgumentException(
        StrUnsupportedMediaType::get() + mediaType,
        static_cast<OWeakObject *>(this),
        static_cast<sal_Int16>(-1) );
}

rtl::OUString BackendImpl::getRegisteredFlagFileURL( Reference< deployment::XPackage > xPackage )
{
    rtl::OUString aRetURL;
    if( !xPackage.is() )
        return aRetURL;
    rtl::OUString aHelpURL = xPackage->getURL();
    aRetURL = expandURL( aHelpURL );
    aRetURL += rtl::OUString::createFromAscii( "/RegisteredFlag" );
    return aRetURL;
}

rtl::OUString BackendImpl::expandURL( const rtl::OUString& aURL )
{
    static Reference< util::XMacroExpander > xMacroExpander;
    static Reference< uri::XUriReferenceFactory > xFac;

    if( !xMacroExpander.is() || !xFac.is() )
    {
        Reference<XComponentContext> const & xContext = getComponentContext();
        if( xContext.is() )
        {
            xFac = Reference< uri::XUriReferenceFactory >(
                xContext->getServiceManager()->createInstanceWithContext( rtl::OUString::createFromAscii(
                "com.sun.star.uri.UriReferenceFactory"), xContext ) , UNO_QUERY );
        }
        if( !xFac.is() )
        {
            throw RuntimeException(
                ::rtl::OUString::createFromAscii(
                "dp_registry::backend::help::BackendImpl::expandURL(), "
                "could not instatiate UriReferenceFactory." ),
                Reference< XInterface >() );
        }

        xMacroExpander = Reference< util::XMacroExpander >(
            xContext->getValueByName(
            ::rtl::OUString::createFromAscii( "/singletons/com.sun.star.util.theMacroExpander" ) ),
            UNO_QUERY_THROW );
     }

    rtl::OUString aRetURL = aURL;
    if( xMacroExpander.is() )
    {
        Reference< uri::XUriReference > uriRef;
        for (;;)
        {
            uriRef = Reference< uri::XUriReference >( xFac->parse( aRetURL ), UNO_QUERY );
            if ( uriRef.is() )
            {
                Reference < uri::XVndSunStarExpandUrl > sxUri( uriRef, UNO_QUERY );
                if( !sxUri.is() )
                    break;

                aRetURL = sxUri->expand( xMacroExpander );
            }
        }
     }
    return aRetURL;
}

Reference< ucb::XSimpleFileAccess > BackendImpl::getFileAccess( void )
{
    if( !m_xSFA.is() )
    {
        Reference<XComponentContext> const & xContext = getComponentContext();
        if( xContext.is() )
        {
            m_xSFA = Reference< ucb::XSimpleFileAccess >(
                xContext->getServiceManager()->createInstanceWithContext(
                    rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ),
                    xContext ), UNO_QUERY );
        }
        if( !m_xSFA.is() )
        {
            throw RuntimeException(
                ::rtl::OUString::createFromAscii(
                "dp_registry::backend::help::BackendImpl::getFileAccess(), "
                "could not instatiate SimpleFileAccess." ),
                Reference< XInterface >() );
        }
    }
    return m_xSFA;
}

//##############################################################################

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
//______________________________________________________________________________
beans::Optional< beans::Ambiguous<sal_Bool> >
BackendImpl::PackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard &,
    ::rtl::Reference<AbortChannel> const &,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    (void)xCmdEnv;

    BackendImpl * that = getMyBackend();
    Reference< deployment::XPackage > xThisPackage( this );
    rtl::OUString aRegisteredFlagFile = that->getRegisteredFlagFileURL( xThisPackage );

    Reference< ucb::XSimpleFileAccess > xSFA = that->getFileAccess();
    bool bReg = xSFA->exists( aRegisteredFlagFile );

    return beans::Optional< beans::Ambiguous<sal_Bool> >(
        true /* IsPresent */,
        beans::Ambiguous<sal_Bool>( bReg, false /* IsAmbiguous */ ) );
}

//______________________________________________________________________________
void BackendImpl::PackageImpl::processPackage_(
    ::osl::ResettableMutexGuard &,
    bool doRegisterPackage,
    ::rtl::Reference<AbortChannel> const &,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    (void)xCmdEnv;

    BackendImpl * that = getMyBackend();

    Reference< deployment::XPackage > xThisPackage( this );
    rtl::OUString aRegisteredFlagFile = that->getRegisteredFlagFileURL( xThisPackage );
    Reference< ucb::XSimpleFileAccess > xSFA = that->getFileAccess();
    Reference<XComponentContext> const & xComponentContext = that->getComponentContext();

    bool bScript = (m_scriptURL.getLength() > 0);
    Reference<css::script::XLibraryContainer> xScriptLibs;

    bool bDialog = (m_dialogURL.getLength() > 0);
    Reference<css::script::XLibraryContainer> xDialogLibs;

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

    if( !doRegisterPackage )
    {
        if( xSFA->exists( aRegisteredFlagFile ) )
        {
            xSFA->kill( aRegisteredFlagFile );

            if( bScript && xScriptLibs.is() && xScriptLibs->hasByName( m_name ) )
                xScriptLibs->removeLibrary( m_name );

            if( bDialog && xDialogLibs.is() && xDialogLibs->hasByName( m_dialogName ) )
                xDialogLibs->removeLibrary( m_dialogName );
        }
        return;
    }

    if( xSFA->exists( aRegisteredFlagFile ) )
        return;     // Already registered

    // Update LibraryContainer
    bool bScriptSuccess = false;
    const bool bReadOnly = false;
    if( bScript && xScriptLibs.is() && !xScriptLibs->hasByName( m_name ) )
    {
        xScriptLibs->createLibraryLink( m_name, m_scriptURL, bReadOnly );
        bScriptSuccess = xScriptLibs->hasByName( m_name );
    }

    bool bDialogSuccess = false;
    if( bDialog && xDialogLibs.is() && !xDialogLibs->hasByName( m_dialogName ) )
    {
        xDialogLibs->createLibraryLink( m_dialogName, m_dialogURL, bReadOnly );
        bDialogSuccess = xDialogLibs->hasByName( m_dialogName );
    }

    bool bSuccess = bScript || bDialog;     // Something must have happened
    if( bRunning )
        if( (bScript && !bScriptSuccess) || (bDialog && !bDialogSuccess) )
            bSuccess = false;

    if( bSuccess && !xSFA->exists( aRegisteredFlagFile ) )
    {
        Reference< io::XOutputStream > xOutputStream = xSFA->openFileWrite( aRegisteredFlagFile );
        if( xOutputStream.is() )
            xOutputStream->closeOutput();
    }
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

