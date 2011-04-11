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
package com.sun.star.report.pentaho.loader;

import com.sun.star.report.InputRepository;

import java.net.URL;

import java.util.HashMap;
import java.util.Map;

import org.pentaho.reporting.libraries.resourceloader.ResourceData;
import org.pentaho.reporting.libraries.resourceloader.ResourceException;
import org.pentaho.reporting.libraries.resourceloader.ResourceKey;
import org.pentaho.reporting.libraries.resourceloader.ResourceException;
import org.pentaho.reporting.libraries.resourceloader.ResourceKeyCreationException;
import org.pentaho.reporting.libraries.resourceloader.ResourceLoader;
import org.pentaho.reporting.libraries.resourceloader.ResourceLoadingException;
import org.pentaho.reporting.libraries.resourceloader.ResourceManager;
import org.pentaho.reporting.libraries.resourceloader.loader.LoaderUtils;


public class InputRepositoryLoader implements ResourceLoader
{

    private final InputRepository inputRepository;
    private ResourceManager resourceManager;

    public InputRepositoryLoader(final InputRepository inputRepository)
    {
        if (inputRepository == null)
        {
            throw new NullPointerException();
        }
        this.inputRepository = inputRepository;
    }

    /**
     * Checks, whether this resource loader implementation was responsible for
     * creating this key.
     *
     * @param key
     * @return
     */
    public boolean isSupportedKey(final ResourceKey key)
    {
        return InputRepositoryLoader.class.getName().equals(key.getSchema());
    }

    /**
     * Creates a new resource key from the given object and the factory keys.
     *
     * @param value
     * @param factoryKeys
     * @return the created key or null, if the format was not recognized.
     * @throws org.jfree.resourceloader.ResourceKeyCreationException
     *          if creating the key failed.
     */
    public ResourceKey createKey(final Object value,
            final Map factoryKeys)
            throws ResourceKeyCreationException
    {
        if (value instanceof String)
        {
            final String strVal = (String) value;
            if (strVal.startsWith("sun:oo://"))
            {
                return new ResourceKey(InputRepositoryLoader.class.getName(),
                        new InputResourceKey(inputRepository.getId(), strVal), factoryKeys);
            }
        }
        return null;
    }

    /**
     * Derives a new resource key from the given key. If neither a path nor new
     * factory-keys are given, the parent key is returned.
     *
     * @param parent      the parent
     * @param path        the derived path (can be null).
     * @param factoryKeys the optional factory keys (can be null).
     * @return the derived key.
     * @throws org.jfree.resourceloader.ResourceKeyCreationException
     *          if the key cannot be derived for any reason.
     */
    public ResourceKey deriveKey(final ResourceKey parent,
            final String path,
            final Map factoryKeys)
            throws ResourceKeyCreationException
    {
        if (!isSupportedKey(parent))
        {
            throw new ResourceKeyCreationException("Assertation: Unsupported parent key type");
        }

        final InputResourceKey parentKey = (InputResourceKey) parent.getIdentifier();
        final String resource;
        if (path.startsWith("sun:oo://"))
        {
            resource = path;
        }
        else if (path.charAt(0) == '/')
        {
            resource = "sun:oo:/" + path;
        }
        else
        {
            resource = LoaderUtils.mergePaths(parentKey.getPath(), path);
        }
        final Map map;
        if (factoryKeys != null)
        {
            map = new HashMap();
            map.putAll(parent.getFactoryParameters());
            map.putAll(factoryKeys);
        }
        else
        {
            map = parent.getFactoryParameters();
        }
        return new ResourceKey(parent.getSchema(),
                new InputResourceKey(parentKey.getInputRepositoryId(), resource), map);
    }

    public URL toURL(final ResourceKey key)
    {
        return null;
    }

    public ResourceData load(final ResourceKey key)
            throws ResourceLoadingException
    {
        if (!isSupportedKey(key))
        {
            throw new ResourceLoadingException("None of my keys.");
        }

        return new InputRepositoryResourceData(key, inputRepository);
    }

    public void setResourceManager(final ResourceManager manager)
    {
        this.resourceManager = manager;
    }

    public ResourceManager getResourceManager()
    {
        return resourceManager;
    }

    public boolean isSupportedDeserializer(String string)
    {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    public String serialize(ResourceKey rk, ResourceKey rk1) throws ResourceException
    {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    public ResourceKey deserialize(ResourceKey rk, String string) throws ResourceKeyCreationException
    {
        throw new UnsupportedOperationException("Not supported yet.");
    }
}
