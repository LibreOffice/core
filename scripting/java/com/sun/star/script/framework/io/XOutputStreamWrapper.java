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

