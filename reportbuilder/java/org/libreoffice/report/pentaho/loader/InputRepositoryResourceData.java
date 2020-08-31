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
package org.libreoffice.report.pentaho.loader;

import org.libreoffice.report.InputRepository;

import java.io.IOException;
import java.io.InputStream;

import org.pentaho.reporting.libraries.resourceloader.ResourceKey;
import org.pentaho.reporting.libraries.resourceloader.ResourceLoadingException;
import org.pentaho.reporting.libraries.resourceloader.ResourceManager;
import org.pentaho.reporting.libraries.resourceloader.loader.AbstractResourceData;


public class InputRepositoryResourceData extends AbstractResourceData
{

    private final InputRepository inputRepository;
    private final ResourceKey key;
    private final String resourceIdentifier;

    public InputRepositoryResourceData(final ResourceKey key,
            final InputRepository repository)
    {
        this.key = key;
        this.inputRepository = repository;
        final InputResourceKey rkey = (InputResourceKey) key.getIdentifier();
        final String identifier = rkey.getPath();
        this.resourceIdentifier = identifier.substring("sun:oo://".length());
    }

    public Object getAttribute(final String key)
    {
        // we don't support attributes here ..
        return null;
    }

    public ResourceKey getKey()
    {
        return key;
    }

    public InputStream getResourceAsStream(final ResourceManager caller)
            throws ResourceLoadingException
    {
        try
        {
            return inputRepository.createInputStream(resourceIdentifier);
        }
        catch (IOException e)
        {
            throw new ResourceLoadingException("Failed to create input stream for " + resourceIdentifier, e);
        }
    }

    public long getVersion(final ResourceManager caller)
    {
        return inputRepository.getVersion(resourceIdentifier);
    }
}
