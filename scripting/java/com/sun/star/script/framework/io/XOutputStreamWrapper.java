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


package com.sun.star.script.framework.io;
import java.io.*;
import com.sun.star.io.XOutputStream;


public class XOutputStreamWrapper extends OutputStream {
        XOutputStream m_xOutputStream;
        public XOutputStreamWrapper(XOutputStream xOs ) {
            this.m_xOutputStream = xOs;
        }
        public void write(int b)
                        throws java.io.IOException
        {
            if ( m_xOutputStream == null )
            {
                throw new java.io.IOException("Stream is null");
            }
            byte[] bytes = new byte[1];
            bytes[0] = (byte) b;
            try
            {
                m_xOutputStream.writeBytes( bytes );
            }
            catch ( com.sun.star.io.IOException ioe )
            {
                throw new java.io.IOException(ioe.getMessage());
            }
        }
        public void write(byte[] b)
                        throws java.io.IOException
        {

            if ( m_xOutputStream == null )
            {
                throw new java.io.IOException( "Stream is null" );
            }
            try
            {
                m_xOutputStream.writeBytes( b );
            }
            catch ( com.sun.star.io.IOException ioe )
            {
                throw new java.io.IOException(ioe.getMessage());
            }
        }
        public void write( byte[] b, int off, int len )
                        throws java.io.IOException
        {
            if ( m_xOutputStream == null )
            {
                throw new java.io.IOException( "Stream is null" );
            }
            byte[] bytes = new byte[len];
            for ( int i=off; i< off+len; i++ )
            {
                bytes[i] = b[i];
            }
            try
            {
                m_xOutputStream.writeBytes(bytes);
            }
            catch ( com.sun.star.io.IOException ioe )
            {
                throw new java.io.IOException(ioe.getMessage());
            }
        }

        public void flush()
            throws java.io.IOException
        {
            if ( m_xOutputStream == null )
            {
                throw new java.io.IOException( "Stream is null" );
            }
            try
            {
                m_xOutputStream.flush();
            }
            catch ( com.sun.star.io.IOException ioe )
            {
                throw new java.io.IOException(ioe.getMessage());
            }
        }
        public void close()
            throws java.io.IOException
        {
            if ( m_xOutputStream == null )
            {
                throw new java.io.IOException( "Stream is null" );
            }
            try
            {
                m_xOutputStream.closeOutput();
            }
            catch ( com.sun.star.io.IOException ioe )
            {
                throw new java.io.IOException(ioe.getMessage());
            }
        }

    }

