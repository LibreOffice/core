/*************************************************************************
 *
 *  $RCSfile: bootstrapcontext.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:54 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONFIGMGR_BOOTSTRAPCONTEXT_HXX_
#define CONFIGMGR_BOOTSTRAPCONTEXT_HXX_

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _RTL_BOOTSTRAP_H_
#include <rtl/bootstrap.h>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCURRENTCONTEXT_HPP_
#include <com/sun/star/uno/XCurrentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

// -----------------------------------------------------------------------------
#define SINGLETON_ "/singletons/"
#define SINGLETON( NAME ) OUString( RTL_CONSTASCII_USTRINGPARAM( SINGLETON_ NAME ) )
// -----------------------------------------------------------------------------
namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace uno   = ::com::sun::star::uno;
    namespace lang  = ::com::sun::star::lang;
    namespace beans = ::com::sun::star::beans;
    using ::rtl::OUString;
// -----------------------------------------------------------------------------
    typedef ::cppu::WeakComponentImplHelper2 <
                    uno::XComponentContext,
                    uno::XCurrentContext
                > ComponentContext_Base;
    /** Base class for customized ComponentContext using bootstrap data and overrides
    */
    class ComponentContext : public ComponentContext_Base
    {
    public:
        typedef uno::Reference< uno::XComponentContext >        Context;
        typedef uno::Reference< lang::XMultiComponentFactory >  ServiceManager;

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
        ComponentContext(Context const & _xContext);

        /// Destroys this BootstrapContext
        ~ComponentContext();

        /// changes the INI file to use for bootstrap data
        void changeBootstrapURL( const OUString& _aURL );

        // gets the INI in use for getting bootstrap data
        OUString getBootstrapURL() const;

        static sal_Bool isPassthrough(Context const & _xContext);

        static Context getBaseContext(Context const & _xContext);

        static beans::NamedValue makePassthroughMarker(sal_Bool bPassthrough = true);
    // interface implementations
    public:
    // XComponentContext & XCurrentContext
        /** Retrieves a value from this context.
            Can be overridden in derived implementations

            @param Name
                The name of the value to retrieve.

            @returns
                The requested value, or <VOID/> if the value is not found.
        */
        virtual uno::Any SAL_CALL
            getValueByName( const OUString& Name )
                throw (uno::RuntimeException) = 0;

    // XComponentContext only
        virtual ServiceManager SAL_CALL
            getServiceManager(  )
                throw (uno::RuntimeException);

    protected:
    // ComponentHelper
        virtual void SAL_CALL disposing();

    protected:
        // two phase construct - also initialized the bootstrap data
        void initialize(const OUString& _aBootstrapURL);

        bool lookupInContext  ( uno::Any & _rValue, const OUString& _aName ) const;
        bool lookupInBootstrap( uno::Any & _rValue, const OUString& _aName ) const;

        osl::Mutex & mutex() const { return m_aMutex; }
        Context const & basecontext() const { osl::MutexGuard lock(mutex()); return m_xContext; }

    private:
        /// The mutex protecting this component
        mutable osl::Mutex  m_aMutex;
        /// The context that most requests are delegated to
        Context             m_xContext;
        /// The bootstrap data consulted as fallback
        rtlBootstrapHandle  m_hBootstrapData;
    };
// -----------------------------------------------------------------------------

    class UnoContextTunnel
    {
    public:
        typedef uno::Reference< uno::XCurrentContext >  CurrentContext;
        typedef uno::Reference< lang::XUnoTunnel >      FailureTunnel;
        typedef uno::Reference< uno::XComponentContext > Context;
    public:
        UnoContextTunnel();
        ~UnoContextTunnel();
        void tunnel(Context const & xContext);
        void passthru(Context const & xContext);
        uno::Any recoverFailure(bool bRaise); // true, if there is a failure

        static Context recoverContext(Context const & xFallback = Context());
        static bool tunnelFailure(uno::Any const & aException, bool bRaise = false);
    private:
        CurrentContext  m_xOldContext;
        FailureTunnel   m_xActiveTunnel;
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


