/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XOutputStreamWrapper.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:59:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
package com.sun.star.script.framework.io;
import java.io.*;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.io.XStream;
import com.sun.star.util.XModifiable;
import com.sun.star.script.framework.log.*;


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

