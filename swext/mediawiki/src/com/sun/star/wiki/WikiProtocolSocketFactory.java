/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package com.sun.star.wiki;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.security.KeyStore;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.X509TrustManager;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import org.apache.commons.httpclient.ConnectTimeoutException;
import org.apache.commons.httpclient.HttpClientError;
import org.apache.commons.httpclient.params.HttpConnectionParams;
import org.apache.commons.httpclient.protocol.SecureProtocolSocketFactory;

class WikiProtocolSocketFactory implements SecureProtocolSocketFactory
{
    private SSLContext m_aSSLContext;

    public WikiProtocolSocketFactory()
    {
        super();
    }

    public synchronized SSLContext GetNotSoSecureSSLContext()
    {
        if ( m_aSSLContext == null )
        {
            TrustManager[] pTrustUnknownCerts = new TrustManager[]
            {
                new X509TrustManager() {
                    private X509TrustManager m_aOrgTrustManager;

                    private X509TrustManager GetOrgTrustManager()
                    {
                        if ( m_aOrgTrustManager == null )
                        {
                            try
                            {
                                TrustManagerFactory aFactory = TrustManagerFactory.getInstance( TrustManagerFactory.getDefaultAlgorithm() );
                                aFactory.init( (KeyStore)null );
                                TrustManager[] pTrustmanagers = aFactory.getTrustManagers();
                                if ( pTrustmanagers.length != 0 && pTrustmanagers[0] != null )
                                    m_aOrgTrustManager = (X509TrustManager)pTrustmanagers[0];
                            }
                            catch( Exception e )
                            {
                                throw new RuntimeException( "No access to the default trust manager!" );
                            }
                        }

                        return m_aOrgTrustManager;
                    }

                    public X509Certificate[] getAcceptedIssuers()
                    {
                        return GetOrgTrustManager().getAcceptedIssuers();
                    }

                    public void checkClientTrusted(X509Certificate[] certs, String authType) throws CertificateException
                    {
                        GetOrgTrustManager().checkClientTrusted( certs, authType );
                    }

                    public void checkServerTrusted(X509Certificate[] certs, String authType) throws CertificateException
                    {
                        if ( certs == null || certs.length == 0 )
                            GetOrgTrustManager().checkServerTrusted( certs, authType );
                        else
                            for ( int nInd = 0; nInd < certs.length; nInd++ )
                                certs[nInd].checkValidity();
                    }
                }
            };

            try
            {
                SSLContext aContext = SSLContext.getInstance("SSL");
                if ( aContext != null )
                {
                    aContext.init( null, pTrustUnknownCerts, null );
                    m_aSSLContext = aContext;
                }
            }
            catch ( Exception e )
            {
            }
        }

        if ( m_aSSLContext == null )
            throw new HttpClientError();

        return m_aSSLContext;
    }

    public Socket createSocket( String sHost, int nPort, InetAddress clientHost, int clientPort )
        throws IOException, UnknownHostException
    {
        return GetNotSoSecureSSLContext().getSocketFactory().createSocket( sHost, nPort, clientHost, clientPort );
    }

    public Socket createSocket( final String sHost, final int nPort, final InetAddress aLocalAddress, final int nLocalPort, final HttpConnectionParams params )
        throws IOException, UnknownHostException, ConnectTimeoutException
    {
        if ( params == null )
            return createSocket( sHost, nPort, aLocalAddress, nLocalPort );

        int nTimeout = params.getConnectionTimeout();
        Socket aSocket = GetNotSoSecureSSLContext().getSocketFactory().createSocket();
        aSocket.bind( new InetSocketAddress( aLocalAddress, nLocalPort ) );
        aSocket.connect( new InetSocketAddress( sHost, nPort ), nTimeout );
        return aSocket;
    }

    public Socket createSocket( String sHost, int nPort )
        throws IOException, UnknownHostException
    {
        return GetNotSoSecureSSLContext().getSocketFactory().createSocket( sHost, nPort );
    }

    public Socket createSocket( Socket aSocket, String sHost, int nPort, boolean bAutoClose )
        throws IOException, UnknownHostException
    {
        return GetNotSoSecureSSLContext().getSocketFactory().createSocket( aSocket, sHost, nPort, bAutoClose );
    }

    public boolean equals(Object obj)
    {
        return ((obj != null) && obj.getClass().equals(WikiProtocolSocketFactory.class));
    }

    public int hashCode()
    {
        return WikiProtocolSocketFactory.class.hashCode();
    }
};

