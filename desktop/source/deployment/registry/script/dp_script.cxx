/*************************************************************************
 *
 *  $RCSfile: dp_script.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:11:55 $
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

#include "dp_script.hrc"
#include "dp_lib_container.h"
#include "dp_backend.h"
#include "dp_ucb.h"
#include "rtl/uri.hxx"
#include "ucbhelper/content.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "svtools/inettype.hxx"
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

//==============================================================================
class BackendImpl : public ::dp_registry::backend::PackageRegistryBackend
{
protected:
    // PackageRegistryBackend
    virtual Reference<deployment::XPackage> bindPackage_(
        OUString const & url, OUString const & mediaType,
        Reference<XCommandEnvironment> const & xCmdEnv );

public:
    Reference<css::script::XLibraryContainer> m_xScriptLibs;
    Reference<css::script::XLibraryContainer> m_xDialogLibs;
    ::std::auto_ptr<LibraryContainer> m_basic_script_libs;
    ::std::auto_ptr<LibraryContainer> m_dialog_libs;

    OUString m_strBasicLib;
    OUString m_strDialogLib;

    BackendImpl(
        Sequence<Any> const & args,
        Reference<XComponentContext> const & xComponentContext,
        OUString const & implName,
        Sequence<OUString> const & supportedMediaTypes );
};

//==============================================================================
class PackageImpl : public ::dp_registry::backend::Package
{
protected:
    OUString m_scriptURL;
    OUString m_dialogURL;
    OUString m_dialogName;

    inline BackendImpl * getMyBackend() const {
        return static_cast<BackendImpl *>(m_myBackend.get()); }

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

    inline PackageImpl(
        ::rtl::Reference<BackendImpl> const & myBackend,
        OUString const & url,
        OUString const & mediaType,
        OUString const & scriptURL, OUString const & dialogURL )
        : Package( myBackend.get(), url, mediaType,
                   OUString(), OUString(), // will be late-initialized
                   scriptURL.getLength() > 0
                   ? myBackend->m_strBasicLib : myBackend->m_strDialogLib ),
          m_scriptURL( scriptURL ),
          m_dialogURL( dialogURL )
        {}

public:
    static PackageImpl * create(
        ::rtl::Reference<BackendImpl> const & myBackend,
        OUString const & url,
        OUString const & mediaType,
        Reference<XCommandEnvironment> const &xCmdEnv,
        OUString const & scriptURL, OUString const & dialogURL );

    // XPackage
    virtual Any SAL_CALL getIcon( sal_Bool highContrast, sal_Bool smallIcon )
        throw (RuntimeException);
};

//______________________________________________________________________________
PackageImpl * PackageImpl::create(
    ::rtl::Reference<BackendImpl> const & myBackend,
    OUString const & url,
    OUString const & mediaType,
    Reference<XCommandEnvironment> const &xCmdEnv,
    OUString const & scriptURL, OUString const & dialogURL )
{
    ::std::auto_ptr<PackageImpl> ret(
        new PackageImpl( myBackend, url, mediaType, scriptURL, dialogURL ) );

    // name, displayName:
    if (dialogURL.getLength() > 0)
        ret->m_dialogName = LibraryContainer::get_libname(
            dialogURL, xCmdEnv, myBackend->getComponentContext() );
    if (scriptURL.getLength() > 0)
        ret->m_name = LibraryContainer::get_libname(
            scriptURL, xCmdEnv, myBackend->getComponentContext() );
    else
        ret->m_name = ret->m_dialogName;
    ret->m_displayName = ret->m_name;

    return ret.release();
}

//______________________________________________________________________________
BackendImpl::BackendImpl(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xComponentContext,
    OUString const & implName,
    Sequence<OUString> const & supportedMediaTypes )
    : PackageRegistryBackend(
        args, xComponentContext, implName, supportedMediaTypes ),
      m_strBasicLib( getResourceString(RID_STR_BASIC_LIB) ),
      m_strDialogLib( getResourceString(RID_STR_DIALOG_LIB) )
{
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
            ? OUSTR("vnd.sun.star.expand:${$SYSBINDIR/"
                    SAL_CONFIGFILE("bootstrap")
                    ":UserInstallation}/user/basic")
            : OUSTR("vnd.sun.star.expand:${$SYSBINDIR/"
                    SAL_CONFIGFILE("bootstrap")
                    ":BaseInstallation}/share/basic") );
        m_basic_script_libs.reset(
            new LibraryContainer(
                make_url( basic_path, OUSTR("/script.xlc") ),
                getMutex(),
                xComponentContext ) );
        m_dialog_libs.reset(
            new LibraryContainer(
                make_url( basic_path, OUSTR("/dialog.xlc") ),
                getMutex(),
                xComponentContext ) );
    }
}

//==============================================================================
OUString SAL_CALL getImplementationName()
{
    return OUSTR("com.sun.star.comp.deployment.script.PackageRegistryBackend");
}

//==============================================================================
Reference<XInterface> SAL_CALL create(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xComponentContext )
{
    OUString const mediaTypes [] = {
        OUSTR("application/vnd.sun.star.basic-library"),
        OUSTR("application/vnd.sun.star.dialog-library")
    };
    return static_cast< ::cppu::OWeakObject * >(
        new BackendImpl(
            args, xComponentContext, getImplementationName(),
            Sequence<OUString >( mediaTypes, ARLEN(mediaTypes) ) ) );
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
        if (create_ucb_content( &ucbContent, url, xCmdEnv ) &&
            ucbContent.isFolder())
        {
            // probe for script.xlb:
            if (create_ucb_content(
                    0, make_url( url, OUSTR("script.xlb") ),
                    xCmdEnv, false /* no throw */ ))
                mediaType = OUSTR("application/vnd.sun.star.basic-library");
            // probe for dialog.xlb:
            else if (create_ucb_content(
                         0, make_url( url, OUSTR("dialog.xlb") ),
                         xCmdEnv, false /* no throw */ ))
                mediaType = OUSTR("application/vnd.sun.star.dialog-library");
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
            if (subType.EqualsIgnoreCaseAscii("vnd.sun.star.basic-library"))
            {
                OUString dialogURL( make_url( url, OUSTR("dialog.xlb") ) );
                if (! create_ucb_content(
                        0, dialogURL, xCmdEnv, false /* no throw */ ))
                    dialogURL = OUString();
                return PackageImpl::create(
                    this, url, mediaType, xCmdEnv,
                    make_url( url, OUSTR("script.xlb") ), dialogURL );
            }
            else if (subType.EqualsIgnoreCaseAscii(
                         "vnd.sun.star.dialog-library"))
            {
                return PackageImpl::create(
                    this, url, mediaType, xCmdEnv,
                    OUString(), make_url( url, OUSTR("dialog.xlb") ) );
            }
        }
    }
    throw lang::IllegalArgumentException(
        m_strUnsupportedMediaType + mediaType,
        static_cast<OWeakObject *>(this),
        static_cast<sal_Int16>(-1) );
}

//##############################################################################

// XPackage
//______________________________________________________________________________
Any PackageImpl::getIcon( sal_Bool highContrast, sal_Bool smallIcon )
    throw (RuntimeException)
{
    OSL_ASSERT( smallIcon );
    if (smallIcon) {
        sal_uInt16 ret;
        if (m_scriptURL.getLength() > 0)
            ret = highContrast ? RID_IMG_SCRIPTLIB_HC : RID_IMG_SCRIPTLIB;
        else
            ret = highContrast ? RID_IMG_DIALOGLIB_HC : RID_IMG_DIALOGLIB;
        return makeAny(ret);
    }
    return Package::getIcon( highContrast, smallIcon );
}

// Package
//______________________________________________________________________________
beans::Optional< beans::Ambiguous<sal_Bool> > PackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard & guard,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    bool reg;
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
void PackageImpl::processPackage_(
    ::osl::ResettableMutexGuard & guard,
    bool registerPackage,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    if (registerPackage)
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

} // namespace script
} // namespace backend
} // namespace dp_registry

