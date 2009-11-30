/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: InputRepositoryResourceData.java,v $
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


package com.sun.star.report.pentaho.loader;

import java.io.IOException;
import java.io.InputStream;

import com.sun.star.report.InputRepository;
import org.pentaho.reporting.libraries.resourceloader.ResourceKey;
import org.pentaho.reporting.libraries.resourceloader.ResourceLoadingException;
import org.pentaho.reporting.libraries.resourceloader.ResourceManager;
import org.pentaho.reporting.libraries.resourceloader.loader.AbstractResourceData;

public class InputRepositoryResourceData extends AbstractResourceData
{
  private final InputRepository inputRepository;
  private final ResourceKey key;
  private final String resourceIdentifer;

  public InputRepositoryResourceData (final ResourceKey key,
                                      final InputRepository repository)
  {
    this.key = key;
    this.inputRepository = repository;
    final InputResourceKey rkey = (InputResourceKey) key.getIdentifier();
    final String identifier = rkey.getPath();
    this.resourceIdentifer = identifier.substring("sun:oo://".length());
  }

  public Object getAttribute (final String key)
  {
    // we dont support attributes here ..
    return null;
  }

  public ResourceKey getKey ()
  {
    return key;
  }

  public InputStream getResourceAsStream (final ResourceManager caller)
          throws ResourceLoadingException
  {
    try
    {
      return inputRepository.createInputStream(resourceIdentifer);
    }
    catch (IOException e)
    {
      throw new ResourceLoadingException
              ("Failed to create input stream for " + resourceIdentifer, e);
    }
  }

  public long getVersion (final ResourceManager caller)
  {
    return inputRepository.getVersion(resourceIdentifer);
  }
}
