/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bootstrapcontext.cxx,v $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "bootstrapcontext.hxx"
#include "datalock.hxx"
#include <uno/current_context.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include "utility.hxx"

namespace configmgr
{
// ---------------------------------------------------------------------------
#define IMPL_ITEM_PREFIX_     "/implementations/com.sun.star.com.configuration.bootstrap.ComponentContext/"
#define IMPL_ITEM_PASSTHRU    IMPL_ITEM_PREFIX_"isPassthrough"
#define IMPL_ITEM_BASECONTEXT IMPL_ITEM_PREFIX_"theBaseContext"
#define A_SERVICEMANAGER "com.sun.star.lang.theServiceManager"
// ---------------------------------------------------------------------------

#define OUSTR( text )     rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( text ) )
#define OU2ASCII( str ) ( rtl::OUStringToOString(str,RTL_TEXTENCODING_ASCII_US) .getStr() )
// ---------------------------------------------------------------------------
#if 0
static void testComplete()
{
    uno::Reference< uno::XInterface > test = * new ComponentContext(uno::Reference< uno::XComponentContext >,uno::Sequence < beans::NamedValue >(),true);
}
#endif
// ---------------------------------------------------------------------------

ComponentContext::ComponentContext(uno::Reference< uno::XComponentContext > const & _xContext)
: cppu::WeakComponentImplHelper3 < uno::XComponentContext, uno::XCurrentContext, lang::XServiceInfo >(UnoApiLock::getLock())
, m_xContext(_xContext)
, m_hBootstrapData(NULL)
, m_xServiceManager()
{
}
// ---------------------------------------------------------------------------

ComponentContext::~ComponentContext()
{
    if (m_hBootstrapData) rtl_bootstrap_args_close(m_hBootstrapData);
}
// ---------------------------------------------------------------------------

void ComponentContext::initialize( const rtl::OUString& _aURL )
{
    UnoApiLock aLock;

    OSL_ASSERT(!m_hBootstrapData);
    m_hBootstrapData = rtl_bootstrap_args_open(_aURL.pData);

    uno::Reference< lang::XComponent > xOwner(m_xContext, uno::UNO_QUERY);

    if (xOwner.is()) DisposingForwarder::forward(xOwner,this);

    if (!m_xContext.is())
    {
        OSL_ENSURE(rBHelper.bDisposed,"ComponentContext::initialize - Context unexpectedly missing");
        throw lang::DisposedException(OUSTR("Parent context has been disposed early"),*this);
    }
}
// ---------------------------------------------------------------------------

// ComponentHelper
void SAL_CALL ComponentContext::disposing()
{
    UnoApiLock aLock;

    m_xContext.clear();

    if (m_hBootstrapData)
    {
        rtl_bootstrap_args_close(m_hBootstrapData);
        m_hBootstrapData = NULL;
    }
}
// ---------------------------------------------------------------------------

rtl::OUString ComponentContext::getBootstrapURL() const
{
    rtl::OUString aResult;

    UnoApiLock aLock;

    if (m_hBootstrapData)
    {
        rtl_bootstrap_get_iniName_from_handle(m_hBootstrapData,&aResult.pData);
    }
    else
    {
        OSL_TRACE( "configmgr: No bootstrap data URL set");
    }

    return aResult;
}
// ---------------------------------------------------------------------------

uno::Reference< lang::XMultiComponentFactory > SAL_CALL
    ComponentContext::getServiceManager(  )
        throw (uno::RuntimeException)
{
    UnoApiLock aLock;

    if (!m_xServiceManager.is())
    {
        uno::Reference< uno::XComponentContext > xBase = basecontext();
        if (!xBase.is())
            throw lang::DisposedException(OUSTR("Parent context has been disposed"),*this);

        uno::Reference< lang::XMultiComponentFactory > xBaseServiceManager = xBase->getServiceManager();
        OSL_ENSURE( xBaseServiceManager.is(), "Base context has no service manager");

        if (xBaseServiceManager.is())
        {
            // create new smgr based on delegate's one
            m_xServiceManager.set(
                xBaseServiceManager->createInstanceWithContext( OUSTR("com.sun.star.comp.stoc.OServiceManagerWrapper"), xBase ),
                uno::UNO_QUERY );
            // patch DefaultContext property of new one
            uno::Reference< beans::XPropertySet > xProps( m_xServiceManager, uno::UNO_QUERY );
            OSL_ASSERT( xProps.is() );
            if (xProps.is())
            {
                uno::Reference< XComponentContext > xThis( this );
                xProps->setPropertyValue( OUSTR("DefaultContext"), uno::makeAny( xThis ) );
            }
            else
                OSL_ENSURE(!m_xServiceManager.is(), "Cannot set Default Context of Service Manager Wrapper: no property set");
        }
    }
    return m_xServiceManager;
}
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------

sal_Bool ComponentContext::isPassthrough(uno::Reference< uno::XComponentContext > const & _xContext)
{
    OSL_ENSURE(_xContext.is(),"Unexpected NULL context");
    if (!_xContext.is()) return false;

    sal_Bool bValue = false;
    _xContext->getValueByName(OUSTR(IMPL_ITEM_PASSTHRU)) >>= bValue;
    return bValue;
}
// ---------------------------------------------------------------------------

beans::NamedValue ComponentContext::makePassthroughMarker(sal_Bool bPassthrough)
{
    return beans::NamedValue(OUSTR(IMPL_ITEM_PASSTHRU),uno::makeAny(bPassthrough));
}
// ---------------------------------------------------------------------------

bool ComponentContext::lookupInContext( uno::Any & _rValue, const rtl::OUString& _aName ) const
{
    uno::Reference< uno::XComponentContext > xBase = basecontext();
    if (!xBase.is())
        throw lang::DisposedException(OUSTR("Parent context has been disposed"),const_cast<ComponentContext&>(*this));

    if (_aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM( IMPL_ITEM_BASECONTEXT )))
    {
        _rValue = uno::makeAny(xBase);
        return true;
    }

    uno::Any aCtxValue = xBase->getValueByName( _aName );

    if (aCtxValue.hasValue())
    {
        _rValue = aCtxValue;
        return true;
    }
    else
        return false;
}
// ---------------------------------------------------------------------------

bool ComponentContext::lookupInBootstrap( uno::Any & _rValue, const rtl::OUString& _aName ) const
{
    UnoApiLock aLock;
    rtl::OUString sResult;
    if ( rtl_bootstrap_get_from_handle( m_hBootstrapData, _aName.pData, &sResult.pData, 0) )
    {
        _rValue <<= sResult;
        return true;
    }
    else
        return false;
}
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

static const char k_TunneledContext[] = "/services/com.sun.star.configuration.bootstrap.Context";

class UnoContextTunnel::Tunnel
: public ::cppu::WeakImplHelper2< uno::XCurrentContext, lang::XUnoTunnel >
{
    uno::Reference< uno::XComponentContext >         m_xTunneledContext;
    uno::Reference< uno::XCurrentContext >  m_xOldContext;
    uno::Any        m_aFailure;
public:
    Tunnel(uno::Reference< uno::XComponentContext > const & xTunneledContext, uno::Reference< uno::XCurrentContext > const & xOldContext)
    : m_xTunneledContext(xTunneledContext)
    , m_xOldContext(xOldContext)
    , m_aFailure()
    {}

    virtual uno::Any SAL_CALL
        getValueByName( const rtl::OUString& name )
            throw (uno::RuntimeException)
    {
        if (name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(k_TunneledContext) ) )
        {
            return uno::makeAny(m_xTunneledContext);
        }
        else if (m_xOldContext.is())
        {
            return m_xOldContext->getValueByName(name);
        }
        else
        {
            return uno::Any();
        }
    }

    virtual sal_Int64 SAL_CALL
        getSomething( const uno::Sequence< sal_Int8 >& aIdentifier )
            throw (uno::RuntimeException)
    {
        if (getTunnelId() == aIdentifier)
            return reinterpret_cast<sal_Int64>(&m_aFailure);
        else
            return 0;
    }

    static uno::Any * getFailure(uno::Reference< lang::XUnoTunnel > const & xTunnel);

    static uno::Sequence< sal_Int8 > getTunnelId();
};
// ---------------------------------------------------------------------------

uno::Sequence< sal_Int8 > UnoContextTunnel::Tunnel::getTunnelId()
{
    static ::cppu::OImplementationId aTunnelId;
    return aTunnelId.getImplementationId();
}
// ---------------------------------------------------------------------------

uno::Any * UnoContextTunnel::Tunnel::getFailure(uno::Reference< lang::XUnoTunnel > const & xTunnel)
{
    if (xTunnel.is())
    {
    if (sal_Int64 nSomething = xTunnel->getSomething(getTunnelId()))
    {
        return reinterpret_cast<uno::Any *>(nSomething);
    }
    }
    return NULL;
}
// ---------------------------------------------------------------------------

UnoContextTunnel::UnoContextTunnel()
: m_xOldContext( uno::getCurrentContext() )
, m_xActiveTunnel()
{
}
// ---------------------------------------------------------------------------

UnoContextTunnel::~UnoContextTunnel()
{
    uno::setCurrentContext( m_xOldContext );
}
// ---------------------------------------------------------------------------

void UnoContextTunnel::passthru(uno::Reference< uno::XComponentContext > const & xContext)
{
    OSL_ASSERT( xContext.is() );
    if ( ComponentContext::isPassthrough(xContext) )
    {
        this ->tunnel(xContext);
    }
    else
    {
        this->tunnel(NULL);
    }
}
// ---------------------------------------------------------------------------

void UnoContextTunnel::tunnel(uno::Reference< uno::XComponentContext > const & xContext)
{
    Tunnel * pNewTunnel = new Tunnel(xContext,m_xOldContext);
    m_xActiveTunnel = pNewTunnel;
    uno::setCurrentContext( pNewTunnel );
}
// ---------------------------------------------------------------------------

uno::Reference< uno::XComponentContext > UnoContextTunnel::recoverContext(uno::Reference< uno::XComponentContext > const & xFallback )
{
    try
    {
        uno::Reference< uno::XCurrentContext > const xCurrContext = uno::getCurrentContext();

        if (xCurrContext.is())
        {
            rtl::OUString aName(RTL_CONSTASCII_USTRINGPARAM(k_TunneledContext));
            uno::Reference< uno::XComponentContext > xResult;
            if (xCurrContext->getValueByName(aName) >>= xResult)
            {
                if (xResult.is())
                    return xResult;
            }
            else
            {
                OSL_ASSERT( !xResult.is() );
                OSL_ENSURE( !xCurrContext->getValueByName(aName).hasValue(),
                            "Value in context has wrong type");
            }
        }
    }
    catch (uno::Exception &)
    {
        OSL_ENSURE(false, "Unexpected: Exception from accessing current context");
    }

    return xFallback;
}
// ---------------------------------------------------------------------------

uno::Any UnoContextTunnel::recoverFailure(bool bRaise)
{
    if (uno::Any * pFail = UnoContextTunnel::Tunnel::getFailure(m_xActiveTunnel))
    {
        if (bRaise)
        {
            if (pFail->hasValue())
                cppu::throwException(*pFail);
            else
                throw;
        }
        return *pFail;
    }

    return uno::Any();
}
// ---------------------------------------------------------------------------

bool UnoContextTunnel::tunnelFailure(uno::Any const & aException, bool bRaise)
{
    OSL_ASSERT( !aException.hasValue() || aException.getValueTypeClass() == uno::TypeClass_EXCEPTION );

    uno::Reference< lang::XUnoTunnel > xTunnel( uno::getCurrentContext(), uno::UNO_QUERY );

    if (uno::Any * pFail = Tunnel::getFailure(xTunnel))
    {
        *pFail = aException;

        if (bRaise && aException.hasValue())
            cppu::throwException(aException);

        if (bRaise) throw;
        return true;
    }
    else
    {
        if (bRaise) throw;
        return false;
    }
}
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
void DisposingForwarder::disposing( lang::EventObject const & /*rSource*/ )
throw (uno::RuntimeException)
{
    m_xTarget->dispose();
    m_xTarget.clear();
}
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
} // namespace config


