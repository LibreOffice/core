/*************************************************************************
 *
 *  $RCSfile: dp_manager.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:07:07 $
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

#if ! defined INCLUDED_DP_MANAGER_H
#define INCLUDED_DP_MANAGER_H

#include "dp_manager.hrc"
#include "dp_misc.h"
#include "dp_interact.h"
#include "dp_persmap.h"
#include "rtl/ref.hxx"
#include "cppuhelper/compbase1.hxx"
#include "cppuhelper/implbase2.hxx"
#include "ucbhelper/content.hxx"
#include "com/sun/star/deployment/XPackageRegistry.hpp"
#include "com/sun/star/deployment/XPackageManager.hpp"
#include <memory>


namespace css = ::com::sun::star;

namespace dp_manager {

typedef ::cppu::WeakComponentImplHelper1<
    css::deployment::XPackageManager > t_pm_helper;

//==============================================================================
class PackageManagerImpl : private ::dp_misc::MutexHolder, public t_pm_helper
{
    css::uno::Reference<css::uno::XComponentContext> m_xComponentContext;
    ::rtl::OUString m_context;
    ::rtl::OUString m_registryCache;
    bool m_readOnly;

    ::rtl::OUString m_activePackages;
    ::rtl::OUString m_activePackages_expanded;
    ::std::auto_ptr< ::dp_misc::PersistentMap > m_activePackagesDB;

    css::uno::Reference<css::ucb::XProgressHandler> m_xLogFile;
    inline void logIntern( css::uno::Any const & status );
    void fireModified();

    css::uno::Reference<css::deployment::XPackageRegistry> m_xRegistry;

    void initRegistryBackends(
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv );
    void initActivationLayer(
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv );
    ::rtl::OUString detectMediaType(
        ::rtl::OUString const & title, ::ucb::Content const & ucbContent,
        bool throw_exc = true );
    ::rtl::OUString insertToActivationLayer(
        ::rtl::OUString const & title, ::rtl::OUString const & mediaType,
        ::ucb::Content const & sourceContent );

    void checkAborted(
        ::rtl::Reference< ::dp_misc::AbortChannel > const & abortChannel );

    class CmdEnvWrapperImpl
        : public ::cppu::WeakImplHelper2< css::ucb::XCommandEnvironment,
                                          css::ucb::XProgressHandler >
    {
        css::uno::Reference<css::ucb::XProgressHandler> m_xLogFile;
        css::uno::Reference<css::ucb::XProgressHandler> m_xUserProgress;
        css::uno::Reference<css::task::XInteractionHandler>
        m_xUserInteractionHandler;

    public:
        virtual ~CmdEnvWrapperImpl();
        CmdEnvWrapperImpl(
            css::uno::Reference<css::ucb::XCommandEnvironment>
            const & xUserCmdEnv,
            css::uno::Reference<css::ucb::XProgressHandler> const & xLogFile );

        // XCommandEnvironment
        virtual css::uno::Reference<css::task::XInteractionHandler> SAL_CALL
        getInteractionHandler() throw (css::uno::RuntimeException);
        virtual css::uno::Reference<css::ucb::XProgressHandler> SAL_CALL
        getProgressHandler() throw (css::uno::RuntimeException);

        // XProgressHandler
        virtual void SAL_CALL push( css::uno::Any const & Status )
            throw (css::uno::RuntimeException);
        virtual void SAL_CALL update( css::uno::Any const & Status )
            throw (css::uno::RuntimeException);
        virtual void SAL_CALL pop() throw (css::uno::RuntimeException);
    };

protected:
    inline void check();
    virtual void SAL_CALL disposing();

    virtual ~PackageManagerImpl();
    inline PackageManagerImpl(
        css::uno::Reference<css::uno::XComponentContext>
        const & xComponentContext, ::rtl::OUString const & context )
        : t_pm_helper( getMutex() ),
          m_xComponentContext( xComponentContext ),
          m_context( context ),
          m_readOnly( false )
        {}

public:
    static css::uno::Reference<css::deployment::XPackageManager> create(
        css::uno::Reference<css::uno::XComponentContext>
        const & xComponentContext, ::rtl::OUString const & context );

    // XComponent
    virtual void SAL_CALL dispose() throw (css::uno::RuntimeException);
    virtual void SAL_CALL addEventListener(
        css::uno::Reference<css::lang::XEventListener> const & xListener )
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener(
        css::uno::Reference<css::lang::XEventListener> const & xListener )
        throw (css::uno::RuntimeException);

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener(
        css::uno::Reference<css::util::XModifyListener> const & xListener )
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener(
        css::uno::Reference<css::util::XModifyListener> const & xListener )
        throw (css::uno::RuntimeException);

    // XPackageManager
    virtual ::rtl::OUString SAL_CALL getContext()
        throw (css::uno::RuntimeException);
    virtual css::uno::Reference<css::task::XAbortChannel> SAL_CALL
    createAbortChannel() throw (css::uno::RuntimeException);

    virtual css::uno::Reference<css::deployment::XPackage> SAL_CALL addPackage(
        ::rtl::OUString const & url, ::rtl::OUString const & mediaType,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::lang::IllegalArgumentException,
               css::uno::RuntimeException);

    void removePackage_(
        ::rtl::OUString const & name,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv );
    virtual void SAL_CALL removePackage(
        ::rtl::OUString const & name,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::lang::IllegalArgumentException,
               css::uno::RuntimeException);

    ::rtl::OUString getDeployPath( ::rtl::OUString const & name,
                                   ::rtl::OUString * pMediaType = 0,
                                   bool ignoreAlienPlatforms = false );
    css::uno::Reference<css::deployment::XPackage> SAL_CALL getDeployedPackage_(
        ::rtl::OUString const & name,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
        bool ignoreAlienPlatforms = false );
    virtual css::uno::Reference<css::deployment::XPackage> SAL_CALL
    getDeployedPackage(
        ::rtl::OUString const & name,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::ucb::CommandFailedException,
               css::lang::IllegalArgumentException, css::uno::RuntimeException);

    css::uno::Sequence< css::uno::Reference<css::deployment::XPackage> >
    getDeployedPackages_(
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv );
    virtual css::uno::Sequence< css::uno::Reference<css::deployment::XPackage> >
    SAL_CALL getDeployedPackages(
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::lang::IllegalArgumentException,
               css::uno::RuntimeException);

    virtual void SAL_CALL reinstallDeployedPackages(
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::lang::IllegalArgumentException,
               css::uno::RuntimeException);
};

//______________________________________________________________________________
inline void PackageManagerImpl::check()
{
    ::osl::MutexGuard guard( getMutex() );
    if (rBHelper.bInDispose || rBHelper.bDisposed)
        throw css::lang::DisposedException(
            OUSTR("PackageManager instance has already been disposed!"),
            static_cast< ::cppu::OWeakObject * >(this) );
}

//______________________________________________________________________________
inline void PackageManagerImpl::logIntern( css::uno::Any const & status )
{
    if (m_xLogFile.is())
        m_xLogFile->update( status );
}

}

#endif

