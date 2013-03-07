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
package org.libreoffice.report;

import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.embed.ElementModes;
import com.sun.star.embed.InvalidStorageException;
import com.sun.star.embed.XStorage;
import com.sun.star.embed.XTransactedObject;
import com.sun.star.io.XStream;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lib.uno.adapter.XInputStreamToInputStreamAdapter;
import com.sun.star.lib.uno.adapter.XOutputStreamToOutputStreamAdapter;
import com.sun.star.uno.UnoRuntime;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * A directory holds all the contents here.
 *
 *
 */
public class StorageRepository implements InputRepository, OutputRepository
{

    private static final Log LOGGER = LogFactory.getLog(SDBCReportDataFactory.class);
    private static final String REPORT_PROCESSING_FAILED = "ReportProcessing failed";
    private XStorage input;
    private XStorage output;
    private final String rootURL;

    /**
     *
     * @param input
     * @param output
     * @param rootURL
     * @throws java.io.IOException
     */
    public StorageRepository(final XStorage input, final XStorage output, final String rootURL)
    {
        this.input = input;
        this.output = output;
        this.rootURL = rootURL;

    }

    public StorageRepository(final XStorage storage, final boolean isOutput, final String rootURL)
    {
        this.rootURL = rootURL;
        if (isOutput)
        {
            this.output = storage;
        }
        else
        {
            this.input = storage;
        }
    }

    public InputStream createInputStream(final String name) throws IOException
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
    public OutputStream createOutputStream(final String name, final String mimeType) throws IOException
    {
        if (output == null)
        {
            throw new IOException("output is NULL");
        }
        try
        {
            final XStream stream = output.openStreamElement(name, ElementModes.WRITE | ElementModes.TRUNCATE);
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

    public boolean exists(final String name)
    {
        try
        {
            return output.isStreamElement(name);
        }
        catch (InvalidStorageException ex)
        {
            LOGGER.error(REPORT_PROCESSING_FAILED, ex);
        }
        catch (com.sun.star.lang.IllegalArgumentException ex)
        {
            LOGGER.error(REPORT_PROCESSING_FAILED, ex);
        }
        catch (NoSuchElementException e)
        {
            // We expect this exception, no need to log it.
        }
        return false;
    }

    public boolean isWritable(final String name)
    {
        return true;
    }

    public Object getId()
    {
        return "1";
    }

    public long getVersion(final String name)
    {
        return 1;
    }

    public boolean isReadable(final String name)
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
            LOGGER.error(REPORT_PROCESSING_FAILED, ex);
        }
        catch (com.sun.star.lang.IllegalArgumentException ex)
        {
            LOGGER.error(REPORT_PROCESSING_FAILED, ex);
        }
        catch (NoSuchElementException ex)
        {
            LOGGER.error(REPORT_PROCESSING_FAILED, ex);
        }
        return false;
    }

    public InputRepository openInputRepository(final String name) throws IOException
    {
        try
        {
            final String temp = shortenName(name);
            if (!input.isStorageElement(temp))
            {
                throw new IOException();
            }
            final XStorage storage = (XStorage) UnoRuntime.queryInterface(XStorage.class, input.openStorageElement(temp, ElementModes.READ));
            return new StorageRepository(storage, false, rootURL);
        }
        catch (NoSuchElementException ex)
        {
            LOGGER.error(REPORT_PROCESSING_FAILED, ex);
        }
        catch (WrappedTargetException ex)
        {
            LOGGER.error(REPORT_PROCESSING_FAILED, ex);
        }
        catch (InvalidStorageException ex)
        {
            LOGGER.error(REPORT_PROCESSING_FAILED, ex);
        }
        catch (IllegalArgumentException ex)
        {
            LOGGER.error(REPORT_PROCESSING_FAILED, ex);
        }
        catch (com.sun.star.io.IOException ex)
        {
            LOGGER.error(REPORT_PROCESSING_FAILED, ex);
        }
        throw new IOException();
    }

    final String shortenName(final String name)
    {
        final String temp;
        if (name.startsWith("./"))
        {
            temp = name.substring(2);
        }
        else
        {
            temp = name;
        }
        return temp;
    }

    public OutputRepository openOutputRepository(final String name, final String mimeType) throws IOException
    {
        try
        {
            final String temp = shortenName(name);
            final XStorage storage = (XStorage) UnoRuntime.queryInterface(XStorage.class, output.openStorageElement(temp, ElementModes.WRITE));
            if (mimeType != null)
            {
                final XPropertySet prop = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, storage);
                prop.setPropertyValue("MediaType", mimeType);
            }
            return new StorageRepository(storage, true, rootURL);
        }
        catch (UnknownPropertyException ex)
        {
            LOGGER.error(REPORT_PROCESSING_FAILED, ex);
        }
        catch (PropertyVetoException ex)
        {
            LOGGER.error(REPORT_PROCESSING_FAILED, ex);
        }
        catch (IllegalArgumentException ex)
        {
            LOGGER.error(REPORT_PROCESSING_FAILED, ex);
        }
        catch (WrappedTargetException ex)
        {
            LOGGER.error(REPORT_PROCESSING_FAILED, ex);
        }
        catch (InvalidStorageException ex)
        {
            LOGGER.error(REPORT_PROCESSING_FAILED, ex);
        }
        catch (com.sun.star.io.IOException ex)
        {
            LOGGER.error(REPORT_PROCESSING_FAILED, ex);
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
                LOGGER.error(REPORT_PROCESSING_FAILED, ex);
            }
            catch (WrappedTargetException ex)
            {
                LOGGER.error(REPORT_PROCESSING_FAILED, ex);
            }
            output.dispose();
        }

    }

    public boolean existsStorage(final String name)
    {
        try
        {
            return output.isStorageElement(name);
        }
        catch (InvalidStorageException ex)
        {
            LOGGER.error(REPORT_PROCESSING_FAILED, ex);
        }
        catch (com.sun.star.lang.IllegalArgumentException ex)
        {
            LOGGER.error(REPORT_PROCESSING_FAILED, ex);
        }
        catch (NoSuchElementException ex)
        {
            // We expect this exception, no need to log it.
        }
        return false;
    }

    public String getRootURL()
    {
        return rootURL;
    }
}
