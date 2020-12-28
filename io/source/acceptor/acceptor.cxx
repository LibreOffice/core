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

#include <osl/mutex.hxx>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/unourl.hxx>
#include <rtl/malformeduriexception.hxx>

#include <com/sun/star/connection/AlreadyAcceptingException.hpp>
#include <com/sun/star/connection/ConnectionSetupException.hpp>
#include <com/sun/star/connection/XAcceptor.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "acceptor.hxx"
#include <memory>
#include <string_view>

#define IMPLEMENTATION_NAME "com.sun.star.comp.io.Acceptor"
#define SERVICE_NAME "com.sun.star.connection.Acceptor"

using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::connection;

namespace {

    class OAcceptor : public WeakImplHelper< XAcceptor, XServiceInfo >
    {
    public:
        explicit OAcceptor(const Reference< XComponentContext > & xCtx);
        virtual ~OAcceptor() override;
    public:
        // Methods
        virtual Reference< XConnection > SAL_CALL accept( const OUString& sConnectionDescription ) override;
        virtual void SAL_CALL stopAccepting(  ) override;

    public: // XServiceInfo
                virtual OUString              SAL_CALL getImplementationName() override;
                virtual Sequence< OUString >  SAL_CALL getSupportedServiceNames() override;
                virtual sal_Bool              SAL_CALL supportsService(const OUString& ServiceName) override;

    private:
        std::unique_ptr<io_acceptor::PipeAcceptor> m_pPipe;
        std::unique_ptr<io_acceptor::SocketAcceptor> m_pSocket;
        Mutex m_mutex;
        OUString m_sLastDescription;
        bool m_bInAccept;

        Reference< XMultiComponentFactory > _xSMgr;
        Reference< XComponentContext > _xCtx;
        Reference<XAcceptor>         _xAcceptor;
    };

}

OAcceptor::OAcceptor( const Reference< XComponentContext > & xCtx )
    : m_bInAccept( false )
    , _xSMgr( xCtx->getServiceManager() )
    , _xCtx( xCtx )
{}

OAcceptor::~OAcceptor()
{
    m_pPipe.reset();
}

namespace {
struct BeingInAccept
{
    /// @throws AlreadyAcceptingException
    BeingInAccept( bool *pFlag,std::u16string_view sConnectionDescription  )
        : m_pFlag( pFlag )
        {
              if( *m_pFlag )
                  throw AlreadyAcceptingException( OUString::Concat("AlreadyAcceptingException :") + sConnectionDescription );
              *m_pFlag = true;
        }
    ~BeingInAccept()
        {
            *m_pFlag = false;
        }
    bool *m_pFlag;
};
}

Reference< XConnection > OAcceptor::accept( const OUString &sConnectionDescription )
{
    // if there is a thread already accepting in this object, throw an exception.
    struct BeingInAccept guard( &m_bInAccept, sConnectionDescription );

    Reference< XConnection > r;
    if( !m_sLastDescription.isEmpty() &&
        m_sLastDescription != sConnectionDescription )
    {
        // instantiate another acceptor for different ports
        throw ConnectionSetupException( "acceptor::accept called multiple times with different connection strings\n" );
    }

    if( m_sLastDescription.isEmpty() )
    {
        // setup the acceptor
        try
        {
            cppu::UnoUrlDescriptor aDesc(sConnectionDescription);
            if ( aDesc.getName() == "pipe" )
            {
                OUString aName(
                    aDesc.getParameter(
                        "name"));

                m_pPipe.reset(new io_acceptor::PipeAcceptor(aName, sConnectionDescription));

                try
                {
                    m_pPipe->init();
                }
                catch( ... )
                {
                    {
                        MutexGuard g( m_mutex );
                        m_pPipe.reset();
                    }
                    throw;
                }
            }
            else if ( aDesc.getName() == "socket" )
            {
                OUString aHost;
                if (aDesc.hasParameter(
                        "host"))
                    aHost = aDesc.getParameter(
                        "host");
                else
                    aHost = "localhost";
                sal_uInt16 nPort = static_cast< sal_uInt16 >(
                    aDesc.getParameter(
                        "port").
                    toInt32());
                bool bTcpNoDelay
                    = aDesc.getParameter(
                        "tcpnodelay").toInt32() != 0;

                m_pSocket.reset(new io_acceptor::SocketAcceptor(
                    aHost, nPort, bTcpNoDelay, sConnectionDescription));

                try
                {
                    m_pSocket->init();
                }
                catch( ... )
                {
                    {
                        MutexGuard g( m_mutex );
                        m_pSocket.reset();
                    }
                    throw;
                }
            }
            else
            {
                OUString delegatee = "com.sun.star.connection.Acceptor." + aDesc.getName();
                _xAcceptor.set(_xSMgr->createInstanceWithContext(delegatee, _xCtx), UNO_QUERY);

                if(!_xAcceptor.is())
                    throw ConnectionSetupException("Acceptor: unknown delegatee " + delegatee);
            }
        }
        catch (const rtl::MalformedUriException & rEx)
        {
            throw IllegalArgumentException(
                rEx.getMessage(),
                Reference< XInterface > (),
                0 );
        }
        m_sLastDescription = sConnectionDescription;
    }

    if( m_pPipe )
    {
        r = m_pPipe->accept();
    }
    else if( m_pSocket )
    {
        r = m_pSocket->accept();
    }
    else
    {
        r = _xAcceptor->accept(sConnectionDescription);
    }

    return r;
}

void SAL_CALL OAcceptor::stopAccepting(  )
{
    MutexGuard guard( m_mutex );

    if( m_pPipe )
    {
        m_pPipe->stopAccepting();
    }
    else if ( m_pSocket )
    {
        m_pSocket->stopAccepting();
    }
    else if( _xAcceptor.is() )
    {
        _xAcceptor->stopAccepting();
    }

}

OUString OAcceptor::getImplementationName()
{
    return IMPLEMENTATION_NAME;
}

sal_Bool OAcceptor::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > OAcceptor::getSupportedServiceNames()
{
    return { SERVICE_NAME };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
io_OAcceptor_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new OAcceptor(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
