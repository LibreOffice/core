/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

import com.sun.star.io.BufferSizeExceededException;
import com.sun.star.io.NotConnectedException;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XSeekable;

/**
 * XInputStream interface implementation.
 */
public class MyInputStream implements XSeekable, XInputStream {

    /**
     * Member properties
     */
    private int offset = 0;
    private int read = offset;
    private byte[] bigbuffer;

    /**
     * Constructor
     */
    public MyInputStream() {
    }

        // XSeekable. Makes it possible to seek to a certain position within a stream.

    /**
     *   Returns the length of the stream.
     *
     *@return  long  The length of the storage medium on which the stream works.
     */
    public synchronized long getLength()
        throws com.sun.star.io.IOException,com.sun.star.uno.RuntimeException {
        if ( bigbuffer != null ) {
            return bigbuffer.length - offset;
        } else {
            return 0;
        }
    }

    /**
     *   Returns the current offset of the stream.
     *
     *@return  long  The current offset in this stream.
     */
    public synchronized long getPosition()
        throws com.sun.star.io.IOException,com.sun.star.uno.RuntimeException {
        return read - offset ;
    }

    /**
     *  Changes the seek pointer to a new location relative to the beginning of the stream.
     */
    public synchronized void seek(long p0)
        throws IllegalArgumentException, com.sun.star.io.IOException,
            com.sun.star.uno.RuntimeException {
        if( bigbuffer != null ) {
            p0 +=offset;
            read = ( int ) p0;
            if( read < offset || read > bigbuffer.length )
                throw new IllegalArgumentException();
        }
    }

        // XInputStream. This is the basic interface to read data from a stream.

    /**
     *   States how many bytes can be read or skipped without blocking.
     *
     *@return  int  If not available, then returned 0
     */
    public synchronized int available()
        throws NotConnectedException, com.sun.star.io.IOException,
            com.sun.star.uno.RuntimeException {
        if( bigbuffer != null )
            return ( bigbuffer.length - read );
        else
            return 0;
    }

    /**
     *   Closes the stream. .
     */
    public void closeInput()
        throws NotConnectedException,com.sun.star.io.IOException,
        com.sun.star.uno.RuntimeException {
        read = -1;
    }

    /**
     *   Reads the specified number of bytes in the given sequence.
     */
    public synchronized int readBytes(byte[][] p0, int p1)
        throws NotConnectedException, BufferSizeExceededException,
            com.sun.star.io.IOException, com.sun.star.uno.RuntimeException {
        if( bigbuffer != null ) {
            if( read == -1 )
                  return 0;
            int i = 0;
            int available;
            if ( p1 > bigbuffer.length - read )
                available = bigbuffer.length - read;
            else
                available = p1;

            p0[0] = new byte[p1];
            while( available != 0 ) {
                p0[0][i++] = bigbuffer[read++];
                --available;
            }
            return i;
        } else {
            p0[0] = new byte[0];
            return 0;
        }
    }

    /**
     *  Reads the available number of bytes at maximum  nMaxBytesToRead .
     *  This method blocks the thread until at least one byte is available.
     */
    public synchronized int readSomeBytes(byte[][] p0, int p1)
            throws  NotConnectedException,
                    BufferSizeExceededException,
                    com.sun.star.io.IOException,
                    com.sun.star.uno.RuntimeException {
            return readBytes( p0,p1 );
    }

    /**
     *  Skips the next nBytesToSkip bytes (must be positive).
     *  It is up to the implementation whether this method is blocking the thread or not.
     */
    public synchronized void skipBytes(int p0)
        throws  NotConnectedException, BufferSizeExceededException,
            com.sun.star.io.IOException, com.sun.star.uno.RuntimeException {
        read += p0;
        if( read > bigbuffer.length )
            read = bigbuffer.length;

        if( read < offset )
            read = offset;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
