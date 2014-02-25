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

package complex.storages;

import com.sun.star.uno.XInterface;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import com.sun.star.io.XStream;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.io.XTruncate;
import com.sun.star.io.XSeekable;


public class BorderedStream
    implements XStream, XInputStream, XOutputStream, XTruncate, XSeekable
{
    int m_nMaxSize;
    int m_nCurSize;
    int m_nCurPos;
    byte m_pBytes[];

    public BorderedStream( int nMaxSize )
    {
        m_nMaxSize = nMaxSize;
        m_nCurSize = 0;
        m_nCurPos = 0;
        m_pBytes = new byte[m_nMaxSize];
    }


    // XStream



    public synchronized XInputStream getInputStream()
        throws com.sun.star.uno.RuntimeException
    {
        return (XInputStream)UnoRuntime.queryInterface( XInputStream.class, this );
    }


    public synchronized XOutputStream getOutputStream()
        throws com.sun.star.uno.RuntimeException
    {
        return (XOutputStream)UnoRuntime.queryInterface( XOutputStream.class, this );
    }


    // XInputStream



    public synchronized int readBytes( byte[][] aData, int nBytesToRead )
        throws  com.sun.star.io.NotConnectedException, com.sun.star.io.BufferSizeExceededException, com.sun.star.io.IOException, com.sun.star.uno.RuntimeException
    {
        int nRead = 0;
        if ( m_pBytes != null && nBytesToRead > 0 )
        {
            int nAvailable = m_nCurSize - m_nCurPos;
            if ( nBytesToRead > nAvailable )
                nBytesToRead = nAvailable;

            aData[0] = new byte[nBytesToRead];
            for ( int nInd = 0; nInd < nBytesToRead; nInd++ )
                aData[0][nInd] = m_pBytes[m_nCurPos+nInd];

            nRead = nBytesToRead;
            m_nCurPos += nRead;
        }
        else
        {
            aData[0] = new byte[0];
        }

        return nRead;
    }


    public synchronized int readSomeBytes( byte[][] aData, int nMaxBytesToRead )
        throws com.sun.star.io.NotConnectedException, com.sun.star.io.BufferSizeExceededException, com.sun.star.io.IOException, com.sun.star.uno.RuntimeException
    {
        return readBytes( aData, nMaxBytesToRead );
    }


    public synchronized void skipBytes( int nBytesToSkip  )
        throws com.sun.star.io.NotConnectedException, com.sun.star.io.BufferSizeExceededException, com.sun.star.io.IOException, com.sun.star.uno.RuntimeException
    {
        if ( nBytesToSkip < 0 )
            throw new com.sun.star.io.IOException(); // illegal argument

        if ( m_nCurSize - m_nCurPos > nBytesToSkip )
            m_nCurPos += nBytesToSkip;
        else
            m_nCurPos = m_nCurSize;
    }


    public synchronized int available()
        throws com.sun.star.io.NotConnectedException, com.sun.star.io.IOException, com.sun.star.uno.RuntimeException
    {
        return 0;
    }


    public synchronized void closeInput()
        throws com.sun.star.io.NotConnectedException, com.sun.star.io.IOException, com.sun.star.uno.RuntimeException
    {
        // no need to do anything
    }



    // XOutputStream



    public synchronized void writeBytes( byte[] aData  )
        throws com.sun.star.io.NotConnectedException, com.sun.star.io.BufferSizeExceededException, com.sun.star.io.IOException, com.sun.star.uno.RuntimeException
    {
        if ( m_pBytes != null && aData.length > 0 )
        {
            if ( aData.length > m_nMaxSize - m_nCurPos )
                throw new com.sun.star.io.IOException();

            for ( int nInd = 0; nInd < aData.length; nInd++ )
                m_pBytes[m_nCurPos+nInd] = aData[nInd];

            m_nCurPos += aData.length;
            if ( m_nCurPos > m_nCurSize )
                m_nCurSize = m_nCurPos;
        }
    }


    public synchronized void flush()
        throws com.sun.star.io.NotConnectedException, com.sun.star.io.BufferSizeExceededException, com.sun.star.io.IOException, com.sun.star.uno.RuntimeException
    {
        // nothing to do
    }


    public synchronized void closeOutput()
        throws com.sun.star.io.NotConnectedException, com.sun.star.io.BufferSizeExceededException, com.sun.star.io.IOException, com.sun.star.uno.RuntimeException
    {
        // nothing to do
    }



    // XTruncate



    public synchronized void truncate()
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException
    {
        m_nCurSize = 0;
        m_nCurPos = 0;
    }



    // XSeekable



    public synchronized void seek( long location )
        throws com.sun.star.lang.IllegalArgumentException, com.sun.star.io.IOException, com.sun.star.uno.RuntimeException
    {
        if ( location > (long)m_nCurSize )
            throw new com.sun.star.lang.IllegalArgumentException();

        m_nCurPos = (int)location;
    }


    public synchronized long getPosition()
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException
    {
        return (long)m_nCurPos;
    }


    public synchronized long getLength()
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException
    {
        return (long)m_nCurSize;
    }
};

