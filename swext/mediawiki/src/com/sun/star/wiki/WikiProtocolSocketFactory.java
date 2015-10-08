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

package com.sun.star.wiki;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.security.KeyStore;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.X509TrustManager;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;

class WikiProtocolSocketFactory extends SSLSocketFactory
{
    private SSLContext m_aSSLContext;

    private synchronized SSLContext GetNotSoSecureSSLContext()
    {
        if ( m_aSSLContext != null ) {
            return m_aSSLContext;
        }
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
                            throw new RuntimeException( "No access to the default trust manager!", e );
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

        if ( m_aSSLContext == null )
            throw new RuntimeException("failed to create SSLContext");

        return m_aSSLContext;
    }

    @Override
    public Socket createSocket(InetAddress address, int port)
        throws IOException
    {
        return GetNotSoSecureSSLContext().getSocketFactory().createSocket(address, port);
    }

    @Override
    public Socket createSocket(InetAddress address, int port, InetAddress localAddress, int localPort)
        throws IOException
    {
        return GetNotSoSecureSSLContext().getSocketFactory().createSocket(address, port, localAddress, localPort);
    }

    @Override
    public Socket createSocket( String sHost, int nPort, InetAddress clientHost, int clientPort )
        throws IOException, UnknownHostException
    {
        return GetNotSoSecureSSLContext().getSocketFactory().createSocket( sHost, nPort, clientHost, clientPort );
    }

    @Override
    public Socket createSocket( String sHost, int nPort )
        throws IOException, UnknownHostException
    {
        return GetNotSoSecureSSLContext().getSocketFactory().createSocket( sHost, nPort );
    }

    @Override
    public Socket createSocket( Socket aSocket, String sHost, int nPort, boolean bAutoClose )
        throws IOException
    {
        return GetNotSoSecureSSLContext().getSocketFactory().createSocket( aSocket, sHost, nPort, bAutoClose );
    }

    @Override
    public String[] getDefaultCipherSuites()
    {
        return GetNotSoSecureSSLContext().getSocketFactory().getDefaultCipherSuites();
    }

    @Override
    public String[] getSupportedCipherSuites()
    {
        return GetNotSoSecureSSLContext().getSocketFactory().getSupportedCipherSuites();
    }

    @Override
    public boolean equals(Object obj)
    {
        return ((obj != null) && obj.getClass().equals(WikiProtocolSocketFactory.class));
    }

    @Override
    public int hashCode()
    {
        return WikiProtocolSocketFactory.class.hashCode();
    }
}

// A factory that creates streams that log everything that's written
// to stderr - useful for debugging encrypted TLS connections
class LoggingProtocolSocketFactory extends SSLSocketFactory
{
    private SSLContext m_aSSLContext;

    private static class LogSocket extends SSLSocket
    {
        private SSLSocket m_Socket;

        public LogSocket(Socket socket)
        {
            m_Socket = (SSLSocket) socket;
        }

        private static class LogStream extends java.io.FilterOutputStream
        {
            public LogStream(java.io.OutputStream stream)
            {
                super(stream);
            }

            @Override
            public void write(byte[] buf, int offset, int len)
                throws IOException
            {
                System.err.println("LogStream.write: \"" + new String(buf, offset, len, "UTF-8") + "\"");
                out.write(buf, offset, len);
            }
        }

        @Override
        public java.io.OutputStream getOutputStream() throws IOException
        {
            return new LogStream(m_Socket.getOutputStream());
        }

        @Override public void addHandshakeCompletedListener(javax.net.ssl.HandshakeCompletedListener listener) { m_Socket.addHandshakeCompletedListener(listener); }
        @Override public String[] getEnabledCipherSuites() { return m_Socket.getEnabledCipherSuites(); }
        @Override public String[] getEnabledProtocols() { return m_Socket.getEnabledProtocols(); }
        @Override public boolean getEnableSessionCreation() { return m_Socket.getEnableSessionCreation(); }
        @Override public boolean getNeedClientAuth() { return m_Socket.getNeedClientAuth(); }
        @Override public javax.net.ssl.SSLSession getSession() { return m_Socket.getSession(); }
        @Override public javax.net.ssl.SSLParameters getSSLParameters() { return m_Socket.getSSLParameters(); }
        @Override public String[] getSupportedCipherSuites() { return m_Socket.getSupportedCipherSuites(); }
        @Override public String[] getSupportedProtocols() { return m_Socket.getSupportedProtocols(); }
        @Override public boolean getUseClientMode() { return m_Socket.getUseClientMode(); }
        @Override public boolean getWantClientAuth() { return m_Socket.getWantClientAuth(); }
        @Override public void removeHandshakeCompletedListener(javax.net.ssl.HandshakeCompletedListener listener) { m_Socket.removeHandshakeCompletedListener(listener); }
        @Override public void setEnabledCipherSuites(String[] suites) { m_Socket.setEnabledCipherSuites(suites); }
        @Override public void setEnabledProtocols(String[] protocols) { m_Socket.setEnabledProtocols(protocols); }
        @Override public void setEnableSessionCreation(boolean flag) { m_Socket.setEnableSessionCreation(flag); }
        @Override public void setNeedClientAuth(boolean need) { m_Socket.setNeedClientAuth(need); }
        @Override public void setSSLParameters(javax.net.ssl.SSLParameters params) { m_Socket.setSSLParameters(params); }
        @Override public void setUseClientMode(boolean mode) { m_Socket.setUseClientMode(mode); }
        @Override public void setWantClientAuth(boolean want) { m_Socket.setWantClientAuth(want); }
        @Override public void startHandshake() throws IOException { m_Socket.startHandshake(); }

        @Override public void bind(java.net.SocketAddress bindpoint) throws IOException { m_Socket.bind(bindpoint); }
        @Override public void close() throws IOException { m_Socket.close(); }
        @Override public void connect(java.net.SocketAddress endpoint) throws IOException { m_Socket.connect(endpoint); }
        @Override public void connect(java.net.SocketAddress endpoint, int timeout) throws IOException { m_Socket.connect(endpoint, timeout); }
        @Override public java.nio.channels.SocketChannel getChannel() { return m_Socket.getChannel(); }
        @Override public InetAddress getInetAddress() { return m_Socket.getInetAddress(); }
        @Override public java.io.InputStream getInputStream() throws IOException { return m_Socket.getInputStream(); }
        @Override public boolean getKeepAlive() throws java.net.SocketException { return m_Socket.getKeepAlive(); }
        @Override public InetAddress getLocalAddress() { return m_Socket.getLocalAddress(); }
        @Override public int getLocalPort() { return m_Socket.getLocalPort(); }
        @Override public java.net.SocketAddress getLocalSocketAddress() { return m_Socket.getLocalSocketAddress(); }
        @Override public boolean getOOBInline() throws java.net.SocketException { return m_Socket.getOOBInline(); }
        @Override public int getPort() { return m_Socket.getPort(); }
        @Override public int getReceiveBufferSize() throws java.net.SocketException { return m_Socket.getReceiveBufferSize(); }
        @Override public java.net.SocketAddress getRemoteSocketAddress() { return m_Socket.getRemoteSocketAddress(); }
        @Override public boolean getReuseAddress() throws java.net.SocketException { return m_Socket.getReuseAddress(); }
        @Override public int getSendBufferSize() throws java.net.SocketException { return m_Socket.getSendBufferSize(); }
        @Override public int getSoLinger() throws java.net.SocketException { return m_Socket.getSoLinger(); }
        @Override public int getSoTimeout() throws java.net.SocketException { return m_Socket.getSoTimeout(); }
        @Override public boolean getTcpNoDelay() throws java.net.SocketException { return m_Socket.getTcpNoDelay(); }
        @Override public int getTrafficClass() throws java.net.SocketException { return m_Socket.getTrafficClass(); }
        @Override public boolean isBound() { return m_Socket.isBound(); }
        @Override public boolean isClosed() { return m_Socket.isClosed(); }
        @Override public boolean isConnected() { return m_Socket.isConnected(); }
        @Override public boolean isInputShutdown() { return m_Socket.isInputShutdown(); }
        @Override public boolean isOutputShutdown() { return m_Socket.isOutputShutdown(); }
        @Override public void sendUrgentData(int data) throws IOException { m_Socket.sendUrgentData(data); }
        @Override public void setKeepAlive(boolean on) throws java.net.SocketException { m_Socket.setKeepAlive(on); }
        @Override public void setOOBInline(boolean on) throws java.net.SocketException { m_Socket.setOOBInline(on); }
        @Override public void setPerformancePreferences(int connectionTime, int latency, int bandwidth) { m_Socket.setPerformancePreferences(connectionTime, latency, bandwidth); }
        @Override public void setReceiveBufferSize(int size) throws java.net.SocketException { m_Socket.setReceiveBufferSize(size); }
        @Override public void setReuseAddress(boolean on) throws java.net.SocketException { m_Socket.setReuseAddress(on); }
        @Override public void setSendBufferSize(int size) throws java.net.SocketException { m_Socket.setSendBufferSize(size); }
        @Override public void setSoLinger(boolean on, int linger) throws java.net.SocketException { m_Socket.setSoLinger(on, linger); }
        @Override public void setSoTimeout(int timeout) throws java.net.SocketException{ m_Socket.setSoTimeout(timeout); }
        @Override public void setTcpNoDelay(boolean on) throws java.net.SocketException{ m_Socket.setTcpNoDelay(on); }
        @Override public void setTrafficClass(int tc) throws java.net.SocketException { m_Socket.setTrafficClass(tc); }
        @Override public void shutdownInput() throws IOException { m_Socket.shutdownInput(); }
        @Override public void shutdownOutput() throws IOException { m_Socket.shutdownOutput(); }
        @Override public String toString() { return m_Socket.toString(); }

    }

    @Override
    public Socket createSocket(InetAddress address, int port)
        throws IOException
    {
        return new LogSocket(((SSLSocketFactory) SSLSocketFactory.getDefault()).createSocket(address, port));
    }

    @Override
    public Socket createSocket(InetAddress address, int port, InetAddress localAddress, int localPort)
        throws IOException
    {
        return new LogSocket(((SSLSocketFactory) SSLSocketFactory.getDefault()).createSocket(address, port, localAddress, localPort));
    }

    @Override
    public Socket createSocket( String sHost, int nPort, InetAddress clientHost, int clientPort )
        throws IOException, UnknownHostException
    {
        return new LogSocket(((SSLSocketFactory) SSLSocketFactory.getDefault()).createSocket(sHost, nPort, clientHost, clientPort));
    }

    @Override
    public Socket createSocket( String sHost, int nPort )
        throws IOException, UnknownHostException
    {
        return new LogSocket(((SSLSocketFactory) SSLSocketFactory.getDefault()).createSocket(sHost, nPort));
    }

    @Override
    public Socket createSocket( Socket aSocket, String sHost, int nPort, boolean bAutoClose )
        throws IOException
    {
        return new LogSocket(((SSLSocketFactory) SSLSocketFactory.getDefault()).createSocket(aSocket, sHost, nPort, bAutoClose));
    }

    @Override
    public String[] getDefaultCipherSuites()
    {
        // have to implement abstract method, just use the default
        return ((SSLSocketFactory) SSLSocketFactory.getDefault()).getDefaultCipherSuites();
    }

    @Override
    public String[] getSupportedCipherSuites()
    {
        // have to implement abstract method, just use the default
        return ((SSLSocketFactory) SSLSocketFactory.getDefault()).getSupportedCipherSuites();
    }

    @Override
    public boolean equals(Object obj)
    {
        return ((obj != null) && obj.getClass().equals(LoggingProtocolSocketFactory.class));
    }

    @Override
    public int hashCode()
    {
        return LoggingProtocolSocketFactory.class.hashCode();
    }
}

