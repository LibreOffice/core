/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StorageRepository.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 09:48:16 $
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

package com.sun.star.report;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.embed.ElementModes;
import com.sun.star.embed.InvalidStorageException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.*;
import com.sun.star.embed.XStorage;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.io.XStream;
import com.sun.star.lib.uno.adapter.XInputStreamToInputStreamAdapter;
import com.sun.star.lib.uno.adapter.XOutputStreamToOutputStreamAdapter;

/**
 * A directory holds all the contents here.
 *
 *
 * @author Ocke Janssen
 */
public class StorageRepository implements InputRepository, OutputRepository {
    private XStorage input;
    private XStorage output;

    /**
     *
     * @param input
     * @param output
     * @throws java.io.IOException
     */
    public StorageRepository(final XStorage input,final XStorage output) throws IOException {
        this.input = input;
        this.output = output;
        if ( output == null || input == null )
            throw new IOException("Need a valid storage not NULL.");
    }

    public InputStream createInputStream(String name) throws IOException {
        try{
            final XStream xStream = (XStream) UnoRuntime.queryInterface(XStream.class,input.openStreamElement(name,ElementModes.READ));
            return new BufferedInputStream(new XInputStreamToInputStreamAdapter(xStream.getInputStream()),102400);
        }catch(com.sun.star.uno.Exception e){
            throw new IOException("createInputStream");
        }
    }

    /**
     * Creates an output stream for writing the data. If there is an entry with
     * that name already contained in the repository, try to overwrite it.
     *
     * @param name
     * @param mimeType
     * @return the outputstream
     * @throws IOException if opening the stream fails
     */
    public OutputStream createOutputStream(String name, String mimeType) throws IOException {
        try{
            final XStream stream = (XStream) UnoRuntime.queryInterface(XStream.class,output.openStreamElement(name,ElementModes.WRITE|ElementModes.TRUNCATE));
            stream.getInputStream().closeInput();
            if (mimeType != null)
            {
                final XPropertySet prop = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,stream);
                prop.setPropertyValue("MediaType",mimeType);
            }
            return new BufferedOutputStream(new XOutputStreamToOutputStreamAdapter(stream.getOutputStream()),204800);
        }catch(com.sun.star.uno.Exception e){
            throw new IOException("createOutputStream");
        }
    }

    public boolean exists(String name) {
        try {
            return output.isStreamElement(name);
        } catch (InvalidStorageException ex) {
            ex.printStackTrace();
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            ex.printStackTrace();
        } catch (NoSuchElementException ex) {
            // We expect this exception, no need to log it.
        }
        return false;
    }

    public boolean isWritable(String name) {
        return true;
    }

    public Object getId() {
        return "1";
    }

    public long getVersion(String name) {
        return 1;
    }

  public boolean isReadable(String name)
  {
    try {
        if ( input != null )
            return input.isStreamElement(name);
    } catch (InvalidStorageException ex) {
        ex.printStackTrace();
    } catch (com.sun.star.lang.IllegalArgumentException ex) {
        ex.printStackTrace();
    } catch (NoSuchElementException ex) {
        ex.printStackTrace();
    }
    return false;
  }
}
