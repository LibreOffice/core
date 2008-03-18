/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_script.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 13:47:44 $
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

#include "dp_script.hrc"
#include "dp_lib_container.h"
#include "dp_backend.h"
#include "dp_ucb.h"
#include "rtl/uri.hxx"
#include "ucbhelper/content.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/implbase1.hxx"
#include "comphelper/servicedecl.hxx"
#include "svtools/inettype.hxx"
#include "com/sun/star/util/XUpdatable.hpp"
#include "com/sun/star/script/XLibraryContainer.hpp"
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
        BackendImpl * getMyBackend() const {
            return static_cast<BackendImpl *>(m_myBackend.get());
        }

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

    Reference<css::script::XLibraryContainer> m_xScriptLibs;
    Reference<css::script::XLibraryContainer> m_xDialogLibs;
    ::std::auto_ptr<LibraryContainer> m_basic_script_libs;
    ::std::auto_ptr<LibraryContainer> m_dialog_libs;

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
    if (office_is_running())
    {
        m_xScriptLibs.set(
            xComponentContext->getServiceManager()->createInstanceWithContext(
                OUSTR("com.sun.star.script.ApplicationScriptLibraryContainer"),
                xComponentContext ), UNO_QUERY_THROW );
        m_xDialogLibs.set(
            xComponentContext->getServiceManager()->createInstanceWithContext(
                OUSTR("com.sun.star.script.ApplicationDialogLibraryContainer"),
                xComponentContext ), UNO_QUERY_THROW );
    }
    else
    {
        OUString basic_path(
            m_eContext == CONTEXT_USER
            ? OUSTR("vnd.sun.star.expand:${$BRAND_BASE_DIR/program/"
                    SAL_CONFIGFILE("bootstrap")
                    ":UserInstallation}/user/basic")
            : OUSTR("vnd.sun.star.expand:${$BRAND_BASE_DIR/program/"
                    SAL_CONFIGFILE("bootstrap")
                    ":BaseInstallation}/share/basic") );
        m_basic_script_libs.reset(
            new LibraryContainer(
                makeURL( basic_path, OUSTR("script.xlc") ),
                getMutex(),
                xComponentContext ) );
        m_dialog_libs.reset(
            new LibraryContainer(
                makeURL( basic_path, OUSTR("dialog.xlc") ),
                getMutex(),
                xComponentContext ) );
    }
}

// XUpdatable
//______________________________________________________________________________
void BackendImpl::update() throw (RuntimeException)
{
    const Reference<XCommandEnvironment> xCmdEnv;
    if (m_basic_script_libs.get() != 0) {
        try {
            m_basic_script_libs->init(xCmdEnv);
            m_basic_script_libs->flush(xCmdEnv);
        }
        catch (RuntimeException &) {
            throw;
        }
        catch (Exception & exc) {
            (void) exc;
            OSL_ENSURE( 0, ::rtl::OUStringToOString(
                            exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
    }
    if (m_dialog_libs.get() != 0) {
        try {
            m_dialog_libs->init(xCmdEnv);
            m_dialog_libs->flush(xCmdEnv);
        }
        catch (RuntimeException &) {
            throw;
        }
        catch (Exception & exc) {
            (void) exc;
            OSL_ENSURE( 0, ::rtl::OUStringToOString(
                            exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
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

//##############################################################################

// Package
//______________________________________________________________________________
beans::Optional< beans::Ambiguous<sal_Bool> >
BackendImpl::PackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard &,
    ::rtl::Reference<AbortChannel> const &,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    bool reg = false;
    if (m_scriptURL.getLength() > 0) {
        if (that->m_xScriptLibs.is())
            reg = that->m_xScriptLibs->hasByName( m_name );
        else
            reg = that->m_basic_script_libs->has( m_name, xCmdEnv );
    }
    else {
        if (that->m_xDialogLibs.is())
            reg = that->m_xDialogLibs->hasByName( m_dialogName );
        else
            reg = that->m_dialog_libs->has( m_dialogName, xCmdEnv );
    }
    return beans::Optional< beans::Ambiguous<sal_Bool> >(
        true /* IsPresent */,
        beans::Ambiguous<sal_Bool>( reg, false /* IsAmbiguous */ ) );
}

//______________________________________________________________________________
void BackendImpl::PackageImpl::processPackage_(
    ::osl::ResettableMutexGuard &,
    bool doRegisterPackage,
    ::rtl::Reference<AbortChannel> const &,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    if (doRegisterPackage)
    {
        if (m_scriptURL.getLength() > 0) {
            if (that->m_xScriptLibs.is())
                that->m_xScriptLibs->createLibraryLink(
                    m_name, m_scriptURL, false /* ! read-only */ );
            else
                that->m_basic_script_libs->insert(
                    m_name, m_scriptURL, xCmdEnv );
        }
        if (m_dialogURL.getLength() > 0) {
            if (that->m_xDialogLibs.is())
                that->m_xDialogLibs->createLibraryLink(
                    m_dialogName, m_dialogURL, false /* ! read-only */ );
            else
                that->m_dialog_libs->insert(
                    m_dialogName, m_dialogURL, xCmdEnv );
        }
    }
    else // revokePackage()
    {
        try {
            if (m_scriptURL.getLength() > 0) {
                if (that->m_xScriptLibs.is())
                    that->m_xScriptLibs->removeLibrary( m_name );
                else
                    that->m_basic_script_libs->remove(
                        m_name, m_scriptURL, xCmdEnv );
            }
            if (m_dialogURL.getLength() > 0) {
                if (that->m_xDialogLibs.is())
                    that->m_xDialogLibs->removeLibrary( m_dialogName );
                else
                    that->m_dialog_libs->remove(
                        m_dialogName, m_dialogURL, xCmdEnv );
            }
        }
        catch (lang::WrappedTargetException & exc) {
            // unwrap WrappedTargetException:
            ::cppu::throwException( exc.TargetException );
        }
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

