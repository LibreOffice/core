/*************************************************************************
 *
 *  $RCSfile: connector.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:24:18 $
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

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/connection/XConnector.hpp>

#include "connector.hxx"

#define IMPLEMENTATION_NAME "com.sun.star.comp.io.Connector"
#define SERVICE_NAME "com.sun.star.connection.Connector"

using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::vos;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::connection;

namespace stoc_connector
{
    typedef WeakImplHelper1< XConnector > MyImplHelper;

    class OConnector : public MyImplHelper
    {
        Reference< XMultiServiceFactory > _xMultiServiceFactory;

    public:
        OConnector(Reference< XMultiServiceFactory > xMultiServiceFactory);

        // Methods
        virtual Reference< XConnection > SAL_CALL OConnector::connect(
            const OUString& sConnectionDescription )
            throw( NoConnectException, ConnectionSetupException, RuntimeException);

    };

    OConnector::OConnector(Reference< XMultiServiceFactory > xMultiServiceFactory)
        : _xMultiServiceFactory(xMultiServiceFactory)
    {
    }


    Reference< XConnection > SAL_CALL OConnector::connect( const OUString& sConnectionDescription )
        throw( NoConnectException, ConnectionSetupException, RuntimeException)
    {
#ifdef DEBUG
        OString tmp = OUStringToOString(sConnectionDescription, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE("connector %s\n", tmp.getStr());
#endif

        // parse string
        OUString aTokens[10];

        // split into separate tokens
        sal_Int32 i = 0,nIndex = 0;

        for(i=0; i < 10 ; i ++ )
        {
            sal_Int32 nLastIndex = nIndex;
            nIndex = sConnectionDescription.indexOf( ( sal_Unicode ) ',' , nIndex );
            if( -1 == nIndex )
            {
                aTokens[i] = sConnectionDescription.copy( nLastIndex );
                break;
            }
            else
            {
                aTokens[i] = sConnectionDescription.copy( nLastIndex , nIndex-nLastIndex );
            }
            aTokens[i] = aTokens[i].trim();
            nIndex ++;
        }

        Reference< XConnection > r;

        if( OUString( RTL_CONSTASCII_USTRINGPARAM("pipe")) == aTokens[0]  )
        {
            OUString sName;
            sal_Bool bIgnoreFirstClose = sal_False;;

            for( i = 1 ; i < 10 ; i ++ )
            {
                sal_Int32 nIndex = aTokens[i].indexOf( '=' );
                if( -1 != nIndex )
                {
                    OUString aName = aTokens[i].copy( 0 , nIndex ).trim().toLowerCase();
                    if( nIndex < aTokens[i].getLength() )
                    {
                        OUString oValue = aTokens[i].copy( nIndex+1 , aTokens[i].getLength() - nIndex -1 ).trim();
                        if( aName == OUString( RTL_CONSTASCII_USTRINGPARAM("ignorefirstclose") ))
                        {
                            bIgnoreFirstClose = (
                                OUString( RTL_CONSTASCII_USTRINGPARAM("1")) == oValue );
                        }
                        else if ( aName == OUString( RTL_CONSTASCII_USTRINGPARAM("name") ))
                        {
                            sName = oValue;
                        }
                    }
                }
            }

            PipeConnection *pConn = new PipeConnection(sName , bIgnoreFirstClose );
#ifdef ENABLEUNICODE
            if( pConn->m_pipe.create( sName.pData, ::vos::OPipe::TOption_Open ) )
#else
            OString o = OUStringToOString( sName ,  RTL_TEXTENCODING_ASCII_US );
            if( pConn->m_pipe.create( o.pData->buffer, ::vos::OPipe::TOption_Open ) )
#endif
            {
                r = Reference < XConnection > ( (XConnection * ) pConn );
            }
            else
            {
                OUString sMessage = OUString::createFromAscii( "Connector : couldn't connect to pipe " );
                sMessage += sName;
                sMessage += OUString::createFromAscii( "(" );
                sMessage += OUString::valueOf( (sal_Int32 ) pConn->m_pipe.getError() );
                sMessage += OUString::createFromAscii( ")" );
                delete pConn;
                throw NoConnectException( sMessage ,Reference< XInterface > () );
            }
        }
        else if( OUString( RTL_CONSTASCII_USTRINGPARAM("socket")) == aTokens[0] )
        {
            OUString sHost;
            sal_uInt16 nPort;
            sal_Bool bIgnoreFirstClose = sal_False;;

            for( i = 1 ; i < 10 ; i ++ )
            {
                sal_Int32 nIndex = aTokens[i].indexOf( '=' );
                if( -1 != nIndex )
                {
                    OUString aName = aTokens[i].copy( 0 , nIndex ).trim().toLowerCase();
                    if( nIndex < aTokens[i].getLength() )
                    {
                        OUString oValue = aTokens[i].copy( nIndex+1 , aTokens[i].getLength() - nIndex -1 ).trim();
                        if( aName == OUString( RTL_CONSTASCII_USTRINGPARAM("ignorefirstclose")) )
                        {
                            bIgnoreFirstClose = (
                                OUString( RTL_CONSTASCII_USTRINGPARAM("1"))  == oValue );
                        }
                        else if( OUString( RTL_CONSTASCII_USTRINGPARAM("host")) == aName )
                        {
                            sHost = oValue;
                        }
                        else if( OUString( RTL_CONSTASCII_USTRINGPARAM("port")) == aName )
                        {
                            nPort = oValue.toInt32();
                        }
                    }
                }
            }

            SocketConnection *pConn = new SocketConnection( sHost ,
                                                            nPort ,
                                                            bIgnoreFirstClose );
            if( ! pConn->m_socket.bind( pConn->m_addr ) )
            {
                OUString sMessage = OUString::createFromAscii( "Connector : couldn't bind socket (" );
                OUString sError;
                pConn->m_socket.getError( sError );
                sMessage += sError;
                sMessage += OUString::createFromAscii( ")" );
                delete pConn;
                throw ConnectionSetupException( sMessage, Reference < XInterface >() );
            }
            pConn->m_socket.setTcpNoDelay( 1 );
#ifdef ENABLEUNICODE
            OInetSocketAddr AddrTarget( sHost.pData, nPort );
#else
            OString o = OUStringToOString( sHost , RTL_TEXTENCODING_ASCII_US );
            OInetSocketAddr AddrTarget(o.pData->buffer, nPort );
#endif
            if(pConn->m_socket.connect(AddrTarget) != osl_Socket_Ok)
            {
                OUString sMessage = OUString::createFromAscii( "Connector : couldn't connect to socket (" );
                OUString sError;
                pConn->m_socket.getError( sError );
                sMessage += sError;
                sMessage += OUString::createFromAscii( ")" );
                delete pConn;
                throw NoConnectException( sMessage, Reference < XInterface > () );
            }
            r = Reference< XConnection > ( (XConnection * ) pConn );
        }
        else if( OUString( RTL_CONSTASCII_USTRINGPARAM( "channel" )) == aTokens[0] )
        {
            OUString sResource;
            for( i = 1 ; i < 10 ; i ++ )
            {
                sal_Int32 nIndex = aTokens[i].indexOf( '=' );
                if( -1 != nIndex )
                {
                    OUString aName = aTokens[i].copy( 0 , nIndex ).trim().toLowerCase();
                    if( nIndex < aTokens[i].getLength() )
                    {
                        OUString oValue = aTokens[i].copy( nIndex+1 , aTokens[i].getLength() - nIndex -1 ).trim();
                        if( OUString( RTL_CONSTASCII_USTRINGPARAM("resource")) == aName )
                        {
                            sResource = oValue;
                        }
                    }
                }
            }

            ChannelConnection *pConn = new ChannelConnection( sResource );

            r = Reference< XConnection > ( (XConnection * ) pConn );
        }
        else
        {
            OUString delegatee = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.connection.Connector."));
            delegatee += aTokens[0];

#ifdef DEBUG
            OString tmp = OUStringToOString(delegatee, RTL_TEXTENCODING_ASCII_US);
            OSL_TRACE("trying to get service %s\n", tmp.getStr());
#endif
            Reference<XConnector> xConnector(_xMultiServiceFactory->createInstance(delegatee), UNO_QUERY);

            if(!xConnector.is())
            {
                OUString message(RTL_CONSTASCII_USTRINGPARAM("Connector: unknown delegatee "));
                message += delegatee;

                throw ConnectionSetupException(message, Reference<XInterface>());
            }

            sal_Int32 index = sConnectionDescription.indexOf((sal_Unicode) ',');

            r = xConnector->connect(sConnectionDescription.copy(index + 1).trim());
        }

        return r;
    }


    Reference< XInterface > SAL_CALL connector_CreateInstance( const Reference< XMultiServiceFactory > & xMultiServiceFactory)
    {
        return Reference < XInterface >( ( OWeakObject * ) new OConnector(xMultiServiceFactory) );
    }

    Sequence< OUString > connector_getSupportedServiceNames()
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

using namespace stoc_connector;


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

            const Sequence< OUString > & rSNL = connector_getSupportedServiceNames();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSHURE( sal_False, "### InvalidRegistryException!" );
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
            connector_CreateInstance, connector_getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}


