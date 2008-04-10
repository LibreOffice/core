/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: StorageRepository.java,v $
 * $Revision: 1.5 $
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
package com.sun.star.report;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.embed.ElementModes;
import com.sun.star.embed.InvalidStorageException;
import com.sun.star.embed.StorageWrappedTargetException;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import java.io.*;
import com.sun.star.embed.XStorage;
import com.sun.star.embed.XTransactedObject;
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
public class StorageRepository implements InputRepository, OutputRepository
{

    private XStorage input;
    private XStorage output;

    /**
     *
     * @param input
     * @param output
     * @throws java.io.IOException
     */
    public StorageRepository(final XStorage input, final XStorage output)
    {
        this.input = input;
        this.output = output;
    }

    public StorageRepository(final XStorage storage, final boolean isOutput)
    {
        if (isOutput)
        {
            this.output = storage;
        }
        else
        {
            this.input = storage;
        }
    }

    public InputStream createInputStream(String name) throws IOException
    {
        if (input == null)
        {
            throw new IOException("input is NULL");
        }
        try
        {
            final XStream xStream = (XStream) UnoRuntime.queryInterface(XStream.class, input.openStreamElement(name, ElementModes.READ));
            return new BufferedInputStream(new XInputStreamToInputStreamAdapter(xStream.getInputStream()), 102400);
        }
        catch (com.sun.star.uno.Exception e)
        {
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
    public OutputStream createOutputStream(String name, String mimeType) throws IOException
    {
        if (output == null)
        {
            throw new IOException("output is NULL");
        }
        try
        {
            final XStream stream = (XStream) UnoRuntime.queryInterface(XStream.class, output.openStreamElement(name, ElementModes.WRITE | ElementModes.TRUNCATE));
            stream.getInputStream().closeInput();
            if (mimeType != null)
            {
                final XPropertySet prop = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, stream);
                prop.setPropertyValue("MediaType", mimeType);
            }
            return new BufferedOutputStream(new XOutputStreamToOutputStreamAdapter(stream.getOutputStream()), 204800);
        }
        catch (com.sun.star.uno.Exception e)
        {
            throw new IOException("createOutputStream");
        }
    }

    public boolean exists(String name)
    {
        boolean ret = false;
        try
        {
            ret = output.isStreamElement(name);
        }
        catch (InvalidStorageException ex)
        {
            ex.printStackTrace();
        }
        catch (com.sun.star.lang.IllegalArgumentException ex)
        {
            ex.printStackTrace();
        }
        catch (NoSuchElementException ex)
        {
        // We expect this exception, no need to log it.
        }
        return ret;
    }

    public boolean isWritable(String name)
    {
        return true;
    }

    public Object getId()
    {
        return "1";
    }

    public long getVersion(String name)
    {
        return 1;
    }

    public boolean isReadable(String name)
    {
        try
        {
            if (input != null)
            {
                return input.isStreamElement(name);
            }
        }
        catch (InvalidStorageException ex)
        {
            ex.printStackTrace();
        }
        catch (com.sun.star.lang.IllegalArgumentException ex)
        {
            ex.printStackTrace();
        }
        catch (NoSuchElementException ex)
        {
            ex.printStackTrace();
        }
        return false;
    }

    public InputRepository openInputRepository(String name) throws IOException
    {
        try
        {
            String temp = name;
            if (name.startsWith("./"))
            {
                temp = name.substring(2);
            }
            if (!input.isStorageElement(temp))
            {
                throw new IOException();
            }
            final XStorage storage = (XStorage) UnoRuntime.queryInterface(XStorage.class, input.openStorageElement(temp, ElementModes.READ));
            return new StorageRepository(storage, false);
        }
        catch (NoSuchElementException ex)
        {
            ex.printStackTrace();
        }
        catch (WrappedTargetException ex)
        {
            ex.printStackTrace();
        }
        catch (InvalidStorageException ex)
        {
            ex.printStackTrace();
        }
        catch (IllegalArgumentException ex)
        {
            ex.printStackTrace();
        }
        catch (com.sun.star.io.IOException ex)
        {
            ex.printStackTrace();
        }
        throw new IOException();
    }

    public OutputRepository openOutputRepository(String name) throws IOException
    {
        try
        {
            String temp = name;
            if (name.startsWith("./"))
            {
                temp = name.substring(2);
            }
            final XStorage storage = (XStorage) UnoRuntime.queryInterface(XStorage.class, output.openStorageElement(temp, ElementModes.WRITE));
            return new StorageRepository(storage, true);
        }
        catch (IllegalArgumentException ex)
        {
            ex.printStackTrace();
        }
        catch (InvalidStorageException ex)
        {
            ex.printStackTrace();
        }
        catch (com.sun.star.io.IOException ex)
        {
            ex.printStackTrace();
        }
        catch (StorageWrappedTargetException ex)
        {
            ex.printStackTrace();
        }
        throw new IOException();
    }

    public void closeInputRepository()
    {
        if (input != null)
        {
            input.dispose();
        }
    }

    public void closeOutputRepository()
    {
        if (output != null)
        {
            try
            {
                final XTransactedObject obj = (XTransactedObject) UnoRuntime.queryInterface(XTransactedObject.class, output);
                if (obj != null)
                {
                    obj.commit();
                }
            }
            catch (com.sun.star.io.IOException ex)
            {
                ex.printStackTrace();
            }
            catch (WrappedTargetException ex)
            {
                ex.printStackTrace();
            }
            output.dispose();
        }

    }

    public boolean existsStorage(String name)
    {
        try
        {
            return output.isStorageElement(name);
        }
        catch (InvalidStorageException ex)
        {
            ex.printStackTrace();
        }
        catch (com.sun.star.lang.IllegalArgumentException ex)
        {
            ex.printStackTrace();
        }
        catch (NoSuchElementException ex)
        {
        // We expect this exception, no need to log it.
        }
        return false;
    }
}
