/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bootstrapcontext.hxx,v $
 * $Revision: 1.9 $
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

#ifndef CONFIGMGR_BOOTSTRAPCONTEXT_HXX_
#define CONFIGMGR_BOOTSTRAPCONTEXT_HXX_

#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/implbase1.hxx>
#include <rtl/bootstrap.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XCurrentContext.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

// -----------------------------------------------------------------------------
#define SINGLETON_ "/singletons/"
#define SINGLETON( NAME ) rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SINGLETON_ NAME ) )
// -----------------------------------------------------------------------------
namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace uno   = ::com::sun::star::uno;
    namespace lang  = ::com::sun::star::lang;
    namespace beans = ::com::sun::star::beans;
// -----------------------------------------------------------------------------
    /** Base class for customized ComponentContext using bootstrap data and overrides
    */
    class ComponentContext : public cppu::WeakComponentImplHelper3 < uno::XComponentContext, uno::XCurrentContext, lang::XServiceInfo >
    {
    // creation and destruction
    public:
        /** Constructs a ComponentContext based on the given overrides and context.
            Initially no bootstrap data will be used.

            @param _xContext
                The base context of this component context.
                Values from here take precedence over values from bootstrap data.

            @param _aOverrides
                The overrides used to create this component context.
                These values take precedence over values from the base context or bootstrap data.
        */
        explicit
        ComponentContext(uno::Reference< uno::XComponentContext > const & _xContext);

        /// Destroys this BootstrapContext
        ~ComponentContext();

        // gets the INI in use for getting bootstrap data
        rtl::OUString getBootstrapURL() const;

        static sal_Bool isPassthrough(uno::Reference< uno::XComponentContext > const & _xContext);

        static beans::NamedValue makePassthroughMarker(sal_Bool bPassthrough = true);
    // interface implementations
    public:

    // XComponentContext only
        virtual uno::Reference< lang::XMultiComponentFactory > SAL_CALL
            getServiceManager(  )
                throw (uno::RuntimeException);

    protected:
    // ComponentHelper
        virtual void SAL_CALL disposing();

    protected:
        // two phase construct - also initialized the bootstrap data
        void initialize(const rtl::OUString& _aBootstrapURL);

        bool lookupInContext  ( uno::Any & _rValue, const rtl::OUString& _aName ) const;
        bool lookupInBootstrap( uno::Any & _rValue, const rtl::OUString& _aName ) const;

        uno::Reference< uno::XComponentContext > const & basecontext() const { return m_xContext; }

    private:
        /// The context that most requests are delegated to
        uno::Reference< uno::XComponentContext >             m_xContext;
        /// The bootstrap data consulted as fallback
        rtlBootstrapHandle  m_hBootstrapData;
        /// The service manager associated with this context
        uno::Reference< lang::XMultiComponentFactory >      m_xServiceManager;
    };
// -----------------------------------------------------------------------------

    class UnoContextTunnel
    {
    public:
        UnoContextTunnel();
        ~UnoContextTunnel();
        void tunnel(uno::Reference< uno::XComponentContext > const & xContext);
        void passthru(uno::Reference< uno::XComponentContext > const & xContext);
        uno::Any recoverFailure(bool bRaise); // true, if there is a failure

        static uno::Reference< uno::XComponentContext > recoverContext(uno::Reference< uno::XComponentContext > const & xFallback = uno::Reference< uno::XComponentContext >());
        static bool tunnelFailure(uno::Any const & aException, bool bRaise = false);
    private:
        uno::Reference< uno::XCurrentContext >  m_xOldContext;
        uno::Reference< lang::XUnoTunnel >   m_xActiveTunnel;
        class Tunnel;
    };
// -----------------------------------------------------------------------------

    class DisposingForwarder : public cppu::WeakImplHelper1< lang::XEventListener >
    {
        uno::Reference< lang::XComponent > m_xTarget;

        DisposingForwarder( uno::Reference< lang::XComponent > const & xTarget ) SAL_THROW( () )
        : m_xTarget( xTarget )
        { OSL_ASSERT( m_xTarget.is() ); }

        virtual void SAL_CALL disposing( lang::EventObject const & rSource )
            throw (uno::RuntimeException);
    public:
        // listens at source for disposing, then disposes target
        static inline void forward(
            uno::Reference< lang::XComponent > const & xSource,
            uno::Reference< lang::XComponent > const & xTarget )
            SAL_THROW( (uno::RuntimeException) );
    };
//__________________________________________________________________________________________________
    inline void DisposingForwarder::forward(
        uno::Reference< lang::XComponent > const & xSource,
        uno::Reference< lang::XComponent > const & xTarget )
        SAL_THROW( (uno::RuntimeException) )
    {
        if (xSource.is())
        {
            xSource->addEventListener( new DisposingForwarder( xTarget ) );
        }
    }
// -----------------------------------------------------------------------------
} // namespace configmgr

#endif


