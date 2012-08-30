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
package com.sun.star.report.pentaho.loader;

import com.sun.star.report.InputRepository;

import java.net.URL;

import java.util.HashMap;
import java.util.Map;

import org.pentaho.reporting.libraries.resourceloader.ResourceData;
import org.pentaho.reporting.libraries.resourceloader.ResourceException;
import org.pentaho.reporting.libraries.resourceloader.ResourceKey;
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
    @SuppressWarnings("unchecked")
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
