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
#include "precompiled_io.hxx"
#include <osl/mutex.hxx>

#include <uno/mapping.hxx>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implementationentry.hxx>
#include "cppuhelper/unourl.hxx"
#include "rtl/malformeduriexception.hxx"

#include <com/sun/star/connection/XAcceptor.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include "acceptor.hxx"

#define IMPLEMENTATION_NAME "com.sun.star.comp.io.Acceptor"
#define SERVICE_NAME "com.sun.star.connection.Acceptor"

using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::connection;

namespace io_acceptor
{
    rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

    class OAcceptor : public WeakImplHelper2< XAcceptor, XServiceInfo >
    {
    public:
        OAcceptor(const Reference< XComponentContext > & xCtx);
        virtual ~OAcceptor();
    public:
        // Methods
        virtual Reference< XConnection > SAL_CALL accept( const OUString& sConnectionDescription )
            throw( AlreadyAcceptingException,
                   ConnectionSetupException,
                   IllegalArgumentException,
                   RuntimeException);
        virtual void SAL_CALL stopAccepting(  ) throw( RuntimeException);

    public: // XServiceInfo
                virtual OUString              SAL_CALL getImplementationName() throw();
                virtual Sequence< OUString >  SAL_CALL getSupportedServiceNames(void) throw();
                virtual sal_Bool              SAL_CALL supportsService(const OUString& ServiceName) throw();

    private:
        PipeAcceptor *m_pPipe;
        SocketAcceptor *m_pSocket;
        Mutex m_mutex;
        OUString m_sLastDescription;
        sal_Bool m_bInAccept;

        Reference< XMultiComponentFactory > _xSMgr;
        Reference< XComponentContext > _xCtx;
        Reference<XAcceptor>         _xAcceptor;
    };


    OAcceptor::OAcceptor( const Reference< XComponentContext > & xCtx )
        : m_pPipe( 0 )
        , m_pSocket( 0 )
        , m_bInAccept( sal_False )
        , _xSMgr( xCtx->getServiceManager() )
        , _xCtx( xCtx )
    {
        g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    }

    OAcceptor::~OAcceptor()
    {
        if( m_pPipe )
        {
            delete m_pPipe;
        }
        if( m_pSocket )
        {
            delete m_pSocket;
        }
        g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
    }

    struct BeingInAccept
    {
        BeingInAccept( sal_Bool *pFlag,const OUString & sConnectionDescription  ) throw( AlreadyAcceptingException)
            : m_pFlag( pFlag )
            {
                  if( *m_pFlag )
                  {
                      OUString sMessage( RTL_CONSTASCII_USTRINGPARAM( "AlreadyAcceptingException :" ) );
                      sMessage += sConnectionDescription;
                      throw AlreadyAcceptingException( sMessage , Reference< XInterface > () );
                  }
                  *m_pFlag = sal_True;
            }
        ~BeingInAccept()
            {
                *m_pFlag = sal_False;
            }
        sal_Bool *m_pFlag;
    };

    Reference< XConnection > OAcceptor::accept( const OUString &sConnectionDescription )
        throw( AlreadyAcceptingException,
               ConnectionSetupException,
               IllegalArgumentException,
               RuntimeException)
    {
        OSL_TRACE(
            "acceptor %s\n",
            OUStringToOString(
                sConnectionDescription, RTL_TEXTENCODING_ASCII_US).getStr());
        // if there is a thread alread accepting in this object, throw an exception.
        struct BeingInAccept guard( &m_bInAccept, sConnectionDescription );

        Reference< XConnection > r;
        if( m_sLastDescription.getLength() &&
            m_sLastDescription != sConnectionDescription )
        {
            // instantiate another acceptor for different ports
            OUString sMessage = OUString( RTL_CONSTASCII_USTRINGPARAM(
                "acceptor::accept called multiple times with different conncetion strings\n" ) );
            throw ConnectionSetupException( sMessage, Reference< XInterface > () );
        }

        if( ! m_sLastDescription.getLength() )
        {
            // setup the acceptor
            try
            {
                cppu::UnoUrlDescriptor aDesc(sConnectionDescription);
                if (aDesc.getName().equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM("pipe")))
                {
                    rtl::OUString aName(
                        aDesc.getParameter(
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                              "name"))));

                    m_pPipe = new PipeAcceptor(aName, sConnectionDescription);

                    try
                    {
                        m_pPipe->init();
                    }
                    catch( ... )
                    {
                        {
                            MutexGuard g( m_mutex );
                            delete m_pPipe;
                            m_pPipe = 0;
                        }
                        throw;
                    }
                }
                else if (aDesc.getName().equalsAsciiL(
                             RTL_CONSTASCII_STRINGPARAM("socket")))
                {
                    rtl::OUString aHost;
                    if (aDesc.hasParameter(
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("host"))))
                        aHost = aDesc.getParameter(
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("host")));
                    else
                        aHost = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                  "localhost"));
                    sal_uInt16 nPort = static_cast< sal_uInt16 >(
                        aDesc.getParameter(
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("port"))).
                        toInt32());
                    bool bTcpNoDelay
                        = aDesc.getParameter(
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                              "tcpnodelay"))).toInt32() != 0;

                    m_pSocket = new SocketAcceptor(
                        aHost, nPort, bTcpNoDelay, sConnectionDescription);

                    try
                    {
                        m_pSocket->init();
                    }
                    catch( ... )
                    {
                        {
                            MutexGuard g( m_mutex );
                            delete m_pSocket;
                            m_pSocket = 0;
                        }
                        throw;
                    }
                }
                else
                {
                    OUString delegatee = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.connection.Acceptor."));
                    delegatee += aDesc.getName();

                    OSL_TRACE(
                        "trying to get service %s\n",
                        OUStringToOString(
                            delegatee, RTL_TEXTENCODING_ASCII_US).getStr());
                    _xAcceptor = Reference<XAcceptor>(
                        _xSMgr->createInstanceWithContext(delegatee, _xCtx), UNO_QUERY);

                    if(!_xAcceptor.is())
                    {
                        OUString message(RTL_CONSTASCII_USTRINGPARAM("Acceptor: unknown delegatee "));
                        message += delegatee;

                        throw ConnectionSetupException(message, Reference<XInterface>());
                    }
                }
            }
            catch (rtl::MalformedUriException & rEx)
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

    void SAL_CALL OAcceptor::stopAccepting(  ) throw( RuntimeException)
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

    OUString acceptor_getImplementationName()
    {
        return OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
    }

    Reference< XInterface > SAL_CALL acceptor_CreateInstance( const Reference< XComponentContext > & xCtx)
    {
        return Reference < XInterface >( ( OWeakObject * ) new OAcceptor(xCtx) );
    }

    Sequence< OUString > acceptor_getSupportedServiceNames()
    {
        static Sequence < OUString > *pNames = 0;
        if( ! pNames )
        {
            MutexGuard guard( Mutex::getGlobalMutex() );
            if( !pNames )
            {
                static Sequence< OUString > seqNames(1);
                seqNames.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM(SERVICE_NAME));
                pNames = &seqNames;
            }
        }
        return *pNames;
    }

        OUString OAcceptor::getImplementationName() throw()
    {
        return acceptor_getImplementationName();
    }

        sal_Bool OAcceptor::supportsService(const OUString& ServiceName) throw()
    {
        Sequence< OUString > aSNL = getSupportedServiceNames();
        const OUString * pArray = aSNL.getConstArray();

        for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
            if( pArray[i] == ServiceName )
                return sal_True;

        return sal_False;
    }

        Sequence< OUString > OAcceptor::getSupportedServiceNames(void) throw()
    {
        return acceptor_getSupportedServiceNames();
    }


}

using namespace io_acceptor;

static struct ImplementationEntry g_entries[] =
{
    {
        acceptor_CreateInstance, acceptor_getImplementationName ,
        acceptor_getSupportedServiceNames, createSingleComponentFactory ,
        &g_moduleCount.modCnt , 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C"
{

sal_Bool SAL_CALL component_canUnload( TimeValue *pTime )
{
    return g_moduleCount.canUnload( &g_moduleCount , pTime );
}

//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
