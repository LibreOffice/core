/*************************************************************************
 *
 *  $RCSfile: acceptor.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-12 15:50:58 $
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
#include <osl/mutex.hxx>

#include <uno/mapping.hxx>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/connection/XAcceptor.hpp>

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
    typedef WeakImplHelper1< XAcceptor > MyImplHelper;

    class OAcceptor : public MyImplHelper
    {
    public:
        OAcceptor(Reference< XMultiServiceFactory > xMultiServiceFactory);
        ~OAcceptor();

    public:
        // Methods
        virtual Reference< XConnection > SAL_CALL accept( const OUString& sConnectionDescription )
            throw( AlreadyAcceptingException,
                   ConnectionSetupException,
                   IllegalArgumentException,
                   RuntimeException);
        virtual void SAL_CALL stopAccepting(  ) throw( RuntimeException);

        PipeAcceptor *m_pPipe;
        SocketAcceptor *m_pSocket;
        Mutex m_mutex;
        OUString m_sLastDescription;
        sal_Bool m_bInAccept;

        Reference<XMultiServiceFactory> _xMultiServiceFactory;
        Reference<XAcceptor>            _xAcceptor;
    };


    OAcceptor::OAcceptor(Reference< XMultiServiceFactory > xMultiServiceFactory)
        : m_pPipe( 0 )
        ,m_pSocket( 0 )
        ,_xMultiServiceFactory(xMultiServiceFactory)
        , m_bInAccept( sal_False )
    {}

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
    }

    // helper class
    class TokenContainer
    {
    public:
        TokenContainer( const OUString &sString );

        ~TokenContainer()
        {
            delete [] m_aTokens;
        }

        inline OUString & getToken( sal_Int32 nElement )
        {
            return m_aTokens[nElement];
        }

        inline sal_Int32 getTokenCount()
        {
            return m_nTokenCount;
        }

        OUString *m_aTokens;
        sal_Int32 m_nTokenCount;
    };

    TokenContainer::TokenContainer( const OUString & sString ) :
        m_nTokenCount( 0 ),
        m_aTokens( 0 )
    {
        // split into separate tokens
        sal_Int32 i = 0,nMax;

        nMax = sString.getLength();
        for( i = 0 ; i < nMax ; i ++ )
        {
            if( ',' == sString.pData->buffer[i] )
            {
                m_nTokenCount ++;
            }
        }

        if( sString.getLength() )
        {
            m_nTokenCount ++;
        }
        if( m_nTokenCount )
        {
            m_aTokens = new OUString[m_nTokenCount];
            sal_Int32 nIndex = 0;
            for( i = 0 ; i < m_nTokenCount ; i ++ )
            {
                sal_Int32 nLastIndex = nIndex;
                nIndex = sString.indexOf( ( sal_Unicode ) ',' , nIndex );
                if( -1 == nIndex )
                {
                    m_aTokens[i] = sString.copy( nLastIndex );
                    break;
                }
                else
                {
                    m_aTokens[i] = sString.copy( nLastIndex , nIndex-nLastIndex );
                }
                m_aTokens[i] = m_aTokens[i].trim();
                nIndex ++;
            }
        }
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
#ifdef DEBUG
        OString tmp = OUStringToOString(sConnectionDescription, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE("acceptor %s\n", tmp.getStr());
#endif
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
            TokenContainer container( sConnectionDescription );
            if( ! container.getTokenCount() )
            {
                throw IllegalArgumentException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "empty connection string" ) ),
                    Reference< XInterface > (),
                    0 );
            }

            if( 0 == container.getToken(0).compareToAscii("pipe") )
            {
                OUString sName;
                sal_Int32 i;
                for( i = 1 ; i < container.getTokenCount() ; i ++ )
                {
                    sal_Int32 nIndex = container.getToken(i).indexOf( '=' );
                    if( -1 != nIndex )
                    {
                        OUString aName = container.getToken(i).copy( 0 , nIndex ).trim().toLowerCase();
                        if( nIndex < container.getToken(i).getLength() )
                        {
                            OUString oValue = container.getToken(i).copy(
                                nIndex+1 , container.getToken(i).getLength() - nIndex -1 ).trim();
                            if ( 0 == aName.compareToAscii("name") )
                            {
                                sName = oValue;
                            }
                        }
                    }
                }
                m_pPipe = new PipeAcceptor(sName , sConnectionDescription );

                try
                {
                    m_pPipe->init();
                }
                catch( ... )
                {
                    {
                        MutexGuard guard( m_mutex );
                        delete m_pPipe;
                        m_pPipe = 0;
                    }
                    throw;
                }
            }
            else if ( 0 == container.getToken(0).compareToAscii("socket") )
            {
                OUString sHost = OUString( RTL_CONSTASCII_USTRINGPARAM("localhost"));
                sal_uInt16 nPort;
                sal_Int32 i;

                for( i = 1 ;i < container.getTokenCount() ; i ++ )
                {
                    sal_Int32 nIndex = container.getToken(i).indexOf( '=' );
                    if( -1 != nIndex )
                    {
                        OUString aName = container.getToken(i).copy( 0 , nIndex ).trim().toLowerCase();
                        if( nIndex < container.getToken(i).getLength() )
                        {
                            OUString oValue = container.getToken(i).copy( nIndex+1 , container.getToken(i).getLength() - nIndex -1 ).trim();
                            if( aName.compareToAscii("host") == 0 )
                            {
                                sHost = oValue;
                            }
                            else if( aName.compareToAscii("port") == 0 )
                            {
                                nPort = (sal_uInt16) oValue.toInt32();
                            }
                        }
                    }
                }

                m_pSocket = new SocketAcceptor( sHost , nPort, sConnectionDescription );

                try
                {
                    m_pSocket->init();
                }
                catch( ... )
                {
                    {
                        MutexGuard guard( m_mutex );
                        delete m_pSocket;
                        m_pSocket = 0;
                    }
                    throw;
                }
            }
            else
            {
                OUString delegatee = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.connection.Acceptor."));
                delegatee += container.getToken(0);

#ifdef DEBUG
                OString tmp = OUStringToOString(delegatee, RTL_TEXTENCODING_ASCII_US);
                OSL_TRACE("trying to get service %s\n", tmp.getStr());
#endif
                _xAcceptor = Reference<XAcceptor>(_xMultiServiceFactory->createInstance(delegatee), UNO_QUERY);

                if(!_xAcceptor.is())
                {
                    OUString message(RTL_CONSTASCII_USTRINGPARAM("Acceptor: unknown delegatee "));
                    message += delegatee;

                    throw ConnectionSetupException(message, Reference<XInterface>());
                }
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
            sal_Int32 index = sConnectionDescription.indexOf((sal_Unicode) ',');

            r = _xAcceptor->accept(m_sLastDescription.copy(index + 1).trim());
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


    Reference< XInterface > SAL_CALL acceptor_CreateInstance( const Reference< XMultiServiceFactory > & xMultiServiceFactory)
    {
        return Reference < XInterface >( ( OWeakObject * ) new OAcceptor(xMultiServiceFactory) );
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
                seqNames.getArray()[0] = OUString::createFromAscii( SERVICE_NAME );
                pNames = &seqNames;
            }
        }
        return *pNames;
    }

}

using namespace io_acceptor;



extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString::createFromAscii("/" IMPLEMENTATION_NAME "/UNO/SERVICES" ) ) );

            const Sequence< OUString > & rSNL = acceptor_getSupportedServiceNames();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLEMENTATION_NAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            acceptor_CreateInstance, acceptor_getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}



