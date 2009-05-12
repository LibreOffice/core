/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XInputStreamImpl.java,v $
 * $Revision: 1.4 $
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

import com.sun.star.io.XInputStream;
import java.io.InputStream;
import java.io.IOException;

public class XInputStreamImpl implements XInputStream
{
    InputStream is;
    public XInputStreamImpl( InputStream is )
    {
        this.is = is;
    }

    public int readBytes( /*OUT*/byte[][] aData, /*IN*/int nBytesToRead ) throws com.sun.star.io.NotConnectedException, com.sun.star.io.BufferSizeExceededException, com.sun.star.io.IOException
    {
        aData[ 0 ] = new byte[ nBytesToRead ];

        int totalBytesRead = 0;

        try
        {
            int bytesRead = 0;
            while ( ( bytesRead = is.read( aData[ 0 ], totalBytesRead, nBytesToRead ) ) > 0 && ( totalBytesRead < nBytesToRead ) )
            {
                totalBytesRead += bytesRead;
                nBytesToRead -= bytesRead;
            }
        }
        catch ( IOException e )
        {
            throw new com.sun.star.io.IOException( e.toString() );
        }
        catch ( IndexOutOfBoundsException aie )
        {
            throw new com.sun.star.io.BufferSizeExceededException( aie.toString() );
        }
        return totalBytesRead;
    }

    public int readSomeBytes( /*OUT*/byte[][] aData, /*IN*/int nMaxBytesToRead ) throws com.sun.star.io.NotConnectedException, com.sun.star.io.BufferSizeExceededException, com.sun.star.io.IOException
    {
        int bytesToRead = nMaxBytesToRead;
        int availableBytes = available();
        if ( availableBytes < nMaxBytesToRead )
        {
            bytesToRead = availableBytes;
        }
        int read =  readBytes( aData, bytesToRead );
        return read;
    }

    public void skipBytes( /*IN*/int nBytesToSkip ) throws com.sun.star.io.NotConnectedException, com.sun.star.io.BufferSizeExceededException, com.sun.star.io.IOException
    {
        long bytesSkipped = 0;
        try
        {
            bytesSkipped = is.skip( (long)nBytesToSkip );
        }
        catch ( IOException e )
        {
            throw new com.sun.star.io.IOException( e.toString() );
        }
    }

    public int available(  ) throws com.sun.star.io.NotConnectedException, com.sun.star.io.IOException
    {
        int bytesAvail = 0;
        try
        {
            bytesAvail = is.available();
        }
        catch ( IOException e )
        {
            throw new com.sun.star.io.IOException( e.toString() );
        }
        return bytesAvail;
    }

    public void closeInput(  ) throws com.sun.star.io.NotConnectedException, com.sun.star.io.IOException
    {
        try
        {
            is.close();
        }
        catch( IOException e )
        {
            throw new com.sun.star.io.IOException( e.toString() );
        }
    }

}
