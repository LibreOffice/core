/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: InputRepositoryResourceData.java,v $
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

import java.io.IOException;
import java.io.InputStream;

import com.sun.star.report.InputRepository;
import org.jfree.resourceloader.ResourceKey;
import org.jfree.resourceloader.ResourceLoadingException;
import org.jfree.resourceloader.ResourceManager;
import org.jfree.resourceloader.loader.AbstractResourceData;

public class InputRepositoryResourceData extends AbstractResourceData
{
  private InputRepository inputRepository;
  private ResourceKey key;
  private String resourceIdentifer;

  public InputRepositoryResourceData (final ResourceKey key,
                                      final InputRepository repository)
  {
    this.key = key;
    this.inputRepository = repository;
    final InputResourceKey rkey = (InputResourceKey) key.getIdentifier();
    final String identifier = rkey.getPath();
    this.resourceIdentifer = identifier.substring("sun:oo://".length());
  }

  public Object getAttribute (String key)
  {
    // we dont support attributes here ..
    return null;
  }

  public ResourceKey getKey ()
  {
    return key;
  }

  public InputStream getResourceAsStream (ResourceManager caller)
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

  public long getVersion (ResourceManager caller)
  {
    return inputRepository.getVersion(resourceIdentifer);
  }
}
