/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: InputRepositoryLoader.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
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


package com.sun.star.report.pentaho.loader;

import java.net.URL;
import java.util.Map;
import java.util.HashMap;

import com.sun.star.report.InputRepository;
import org.jfree.resourceloader.ResourceData;
import org.jfree.resourceloader.ResourceKey;
import org.jfree.resourceloader.ResourceKeyCreationException;
import org.jfree.resourceloader.ResourceLoader;
import org.jfree.resourceloader.ResourceLoadingException;
import org.jfree.resourceloader.ResourceManager;
import org.jfree.resourceloader.loader.LoaderUtils;

public class InputRepositoryLoader implements ResourceLoader
{
  private InputRepository inputRepository;
  private ResourceManager resourceManager;

  public InputRepositoryLoader (final InputRepository inputRepository)
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
    if (InputRepositoryLoader.class.getName().equals(key.getSchema()))
    {
      return true;
    }
    return false;
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
    if (value instanceof String == false)
    {
      return null;
    }
    final String strVal = (String) value;
    if (strVal.startsWith("sun:oo://") == false)
    {
      return null;
    }
    return new ResourceKey(InputRepositoryLoader.class.getName(),
        new InputResourceKey(inputRepository.getId(), strVal), factoryKeys);
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
    if (isSupportedKey(parent) == false)
    {
      throw new ResourceKeyCreationException("Assertation: Unsupported parent key type");
    }

    final InputResourceKey parentKey = (InputResourceKey) parent.getIdentifier();
    final String resource;
    if (path.startsWith("sun:oo://"))
    {
      resource = path;
    }
    else if (path.startsWith("/"))
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

  public ResourceData load (final ResourceKey key)
          throws ResourceLoadingException
  {
    if (isSupportedKey(key) == false)
    {
      throw new ResourceLoadingException("None of my keys.");
    }

    return new InputRepositoryResourceData(key, inputRepository);
  }

  public void setResourceManager (final ResourceManager manager)
  {
    this.resourceManager = manager;
  }

  public ResourceManager getResourceManager ()
  {
    return resourceManager;
  }
}
