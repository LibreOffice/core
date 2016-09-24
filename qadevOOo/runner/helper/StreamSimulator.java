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

package helper;

import com.sun.star.uno.UnoRuntime;


import com.sun.star.ucb.XSimpleFileAccess;

/**
 * It simulates an input and output stream and
 * implements the interfaces XInputStream, XOutputStream.
 * So it can be used for testing loading/saving of documents
 * using streams instead of URLs.
 *
 */
public class StreamSimulator implements com.sun.star.io.XInputStream    ,
                                        com.sun.star.io.XOutputStream   ,
                                        com.sun.star.io.XSeekable
{

    /**
     * @member  m_sFileName     name of the corresponding file on disk
     * @member  m_xInStream     the internal input stream for reading
     * @member  m_xOutStream    the internal input stream for writing
     * @member  m_xSeek         points at runtime to m_xInStream or m_xOutStream and make it seekable
     *
     * @member  //m_aProtocol     the external set protocol object for logging messages
     * @member  m_bInWasUsed    indicates, that the input stream interface was used
     * @member  m_bOutWasUsed   indicates, that the output stream interface was used
     */

    private com.sun.star.io.XInputStream    m_xInStream     ;
    private com.sun.star.io.XOutputStream   m_xOutStream    ;
    private com.sun.star.io.XSeekable       m_xSeek         ;





    /**
     * construct a new instance of this class
     * It set the name of the corresponding file on disk, which
     * should be source or target for the following operations on
     * this object. And it regulate if it should function as
     * input or output stream.
     *
     * @param   sFileName
     *              name of the file on disk
     *              Will be used as source (if param bInput==true)
     *              or as target (if param bInput==false).
     *
     * @param   bInput
     *              it specify, which interface should work at this object.
     *              <TRUE/>  => we simulate an input stream
     *              <FALSE/> => we simulate an output stream
     *
     * @throw   com.sun.star.io.NotConnectedException
     *              in case the internal streams to the file on disk couldn't established.
     *              They are necessary. Otherwise this simulator can't really work.
     */
    public StreamSimulator( String  sFileName , boolean bInput ,
        lib.TestParameters param   ) throws com.sun.star.io.NotConnectedException
    {
        try
        {
            XSimpleFileAccess xHelper = UnoRuntime.queryInterface(XSimpleFileAccess.class,
                param.getMSF().createInstance("com.sun.star.ucb.SimpleFileAccess"));

            if (xHelper == null)
                throw new com.sun.star.io.NotConnectedException("ucb helper not available. Can't create streams.");

            if (bInput)
            {
                m_xInStream = xHelper.openFileRead(sFileName);
                m_xSeek = UnoRuntime.queryInterface(
                            com.sun.star.io.XSeekable.class,
                            m_xInStream);
            }
            else
            {
                m_xOutStream = xHelper.openFileWrite(sFileName);
                m_xSeek = UnoRuntime.queryInterface(
                            com.sun.star.io.XSeekable.class,
                            m_xOutStream);
            }
        }
        catch(com.sun.star.uno.Exception exUno)
        {
            throw new com.sun.star.io.NotConnectedException(exUno);
        }
    }

    /**
     * following methods simulates the XInputStream.
     * The notice all actions inside the internal protocol
     * and try to map all necessary functions to the internal
     * open in-stream.
     */
    public int readBytes( /*OUT*/ byte[][] lData        ,
                          /*IN*/  int      nBytesToRead ) throws com.sun.star.io.NotConnectedException      ,
                                                                 com.sun.star.io.BufferSizeExceededException,
                                                                 com.sun.star.io.IOException
    {
        if (m_xInStream == null)
        {
            throw new com.sun.star.io.NotConnectedException("stream not open");
        }

        int nRead = 0;
        try
        {
            nRead = m_xInStream.readBytes(lData,nBytesToRead);
        }
        catch (com.sun.star.io.NotConnectedException       exConnect) {
        }
        catch (com.sun.star.io.BufferSizeExceededException exBuffer ) {
        }
        catch (com.sun.star.io.IOException                 exIO     ) {
        }
        catch (com.sun.star.uno.RuntimeException           exRuntime) {
        }

        return nRead;
    }



    public int readSomeBytes( /*OUT*/ byte[][] lData           ,
                              /*IN*/  int      nMaxBytesToRead ) throws com.sun.star.io.NotConnectedException       ,
                                                                        com.sun.star.io.BufferSizeExceededException ,
                                                                        com.sun.star.io.IOException
    {
        if (m_xInStream == null)
        {
            throw new com.sun.star.io.NotConnectedException("stream not open");
        }

        int nRead = 0;
        try
        {
            nRead = m_xInStream.readSomeBytes(lData,nMaxBytesToRead);
        }
        catch (com.sun.star.io.NotConnectedException       exConnect) {
        }
        catch (com.sun.star.io.BufferSizeExceededException exBuffer ) {
        }
        catch (com.sun.star.io.IOException                 exIO     ) {
        }
        catch (com.sun.star.uno.RuntimeException           exRuntime) {
        }

        return nRead;
    }



    public void skipBytes( /*IN*/ int nBytesToSkip ) throws com.sun.star.io.NotConnectedException       ,
                                                            com.sun.star.io.BufferSizeExceededException ,
                                                            com.sun.star.io.IOException
    {
        if (m_xInStream == null)
        {
            throw new com.sun.star.io.NotConnectedException("stream not open");
        }

        try
        {
            m_xInStream.skipBytes(nBytesToSkip);
        }
        catch (com.sun.star.io.NotConnectedException       exConnect) {
        }
        catch (com.sun.star.io.BufferSizeExceededException exBuffer ) {
        }
        catch (com.sun.star.io.IOException                 exIO     ) {
        }
        catch (com.sun.star.uno.RuntimeException           exRuntime) {
        }
    }



    public int available() throws com.sun.star.io.NotConnectedException,
                                  com.sun.star.io.IOException
    {
        if (m_xInStream == null)
        {
            throw new com.sun.star.io.NotConnectedException("stream not open");
        }

        int nAvailable = 0;
        try
        {
            nAvailable = m_xInStream.available();
        }
        catch (com.sun.star.io.NotConnectedException exConnect) {
        }
        catch (com.sun.star.io.IOException           exIO     ) {
        }
        catch (com.sun.star.uno.RuntimeException     exRuntime) {
        }

        return nAvailable;
    }



    public void closeInput() throws com.sun.star.io.NotConnectedException,
                                    com.sun.star.io.IOException
    {
        if (m_xInStream == null)
        {
            throw new com.sun.star.io.NotConnectedException("stream not open");
        }

        try
        {
            m_xInStream.closeInput();
        }
        catch (com.sun.star.io.NotConnectedException exConnect) {
        }
        catch (com.sun.star.io.IOException           exIO     ) {
        }
        catch (com.sun.star.uno.RuntimeException     exRuntime) {
        }
    }


    /**
     * following methods simulates the XOutputStream.
     * The notice all actions inside the internal protocol
     * and try to map all necessary functions to the internal
     * open out-stream.
     */
    public void writeBytes( /*IN*/byte[] lData ) throws com.sun.star.io.NotConnectedException       ,
                                                        com.sun.star.io.BufferSizeExceededException ,
                                                        com.sun.star.io.IOException
    {
        if (m_xOutStream == null)
        {
            throw new com.sun.star.io.NotConnectedException("stream not open");
        }

        try
        {
            m_xOutStream.writeBytes(lData);
        }
        catch (com.sun.star.io.NotConnectedException       exConnect) {
        }
        catch (com.sun.star.io.BufferSizeExceededException exBuffer ) {
        }
        catch (com.sun.star.io.IOException                 exIO     ) {
        }
        catch (com.sun.star.uno.RuntimeException           exRuntime) {
        }
    }



    public void flush() throws com.sun.star.io.NotConnectedException        ,
                               com.sun.star.io.BufferSizeExceededException  ,
                               com.sun.star.io.IOException
    {
        if (m_xOutStream == null)
        {
            throw new com.sun.star.io.NotConnectedException("stream not open");
        }

        try
        {
            m_xOutStream.flush();
        }
        catch (com.sun.star.io.NotConnectedException       exConnect) {
        }
        catch (com.sun.star.io.BufferSizeExceededException exBuffer ) {
        }
        catch (com.sun.star.io.IOException                 exIO     ) {
        }
        catch (com.sun.star.uno.RuntimeException           exRuntime) {
        }
    }



    public void closeOutput() throws com.sun.star.io.NotConnectedException      ,
                                     com.sun.star.io.BufferSizeExceededException,
                                     com.sun.star.io.IOException
    {
        if (m_xOutStream == null)
        {
            throw new com.sun.star.io.NotConnectedException("stream not open");
        }

        try
        {
            m_xOutStream.closeOutput();
        }
        catch (com.sun.star.io.NotConnectedException       exConnect) {
        }
        catch (com.sun.star.io.BufferSizeExceededException exBuffer ) {
        }
        catch (com.sun.star.io.IOException                 exIO     ) {
        }
        catch (com.sun.star.uno.RuntimeException           exRuntime) {
        }
    }


    /**
     * following methods simulates the XSeekable.
     * The notice all actions inside the internal protocol
     * and try to map all necessary functions to the internal
     * open stream.
     */
    public void seek( /*IN*/long nLocation ) throws com.sun.star.lang.IllegalArgumentException,
                                                    com.sun.star.io.IOException
    {
        if (m_xSeek == null)
        {
            throw new com.sun.star.io.IOException("stream not seekable");
        }

        try
        {
            m_xSeek.seek(nLocation);
        }
        catch (com.sun.star.lang.IllegalArgumentException exArg    ) {
        }
        catch (com.sun.star.io.IOException                exIO     ) {
        }
        catch (com.sun.star.uno.RuntimeException          exRuntime) {
        }
    }



    public long getPosition() throws com.sun.star.io.IOException
    {
        if (m_xSeek == null)
        {
            throw new com.sun.star.io.IOException("stream not seekable");
        }

        long nPos = 0;
        try
        {
            nPos = m_xSeek.getPosition();
        }
        catch (com.sun.star.io.IOException       exIO     ) {
        }
        catch (com.sun.star.uno.RuntimeException exRuntime) {
        }

        return nPos;
    }



    public long getLength() throws com.sun.star.io.IOException
    {
        if (m_xSeek == null)
        {
            throw new com.sun.star.io.IOException("stream not seekable");
        }

        long nLen = 0;
        try
        {
            nLen = m_xSeek.getLength();
        }
        catch (com.sun.star.io.IOException       exIO     ) {
        }
        catch (com.sun.star.uno.RuntimeException exRuntime) {
        }

        return nLen;
    }
}
