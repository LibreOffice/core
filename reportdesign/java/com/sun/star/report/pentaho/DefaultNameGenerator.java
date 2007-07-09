/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DefaultNameGenerator.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:04 $
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


package com.sun.star.report.pentaho;

import java.io.IOException;

import com.sun.star.report.OutputRepository;

public class DefaultNameGenerator
{
  private OutputRepository outputRepository;

  public DefaultNameGenerator(final OutputRepository outputRepository)
  {
    if (outputRepository == null)
    {
      throw new NullPointerException();
    }
    this.outputRepository = outputRepository;
  }

  /**
   * Generates a new, unique name for storing resources in the output repository. Assuming that proper synchronization
   * has been applied, the generated name will be unique within that repository.
   *
   * @param namePrefix a user defined name for that resource.
   * @param mimeType   the mime type of the resource to be stored in the repository.
   * @return the generated, fully qualified name.
   */
  public String generateName(final String namePrefix, final String mimeType)
      throws IOException
  {
    final String name;
    if (namePrefix != null)
    {
      name = namePrefix;
    }
    else
    {
      name = "file";
    }

    final String suffix = getSuffixForType(mimeType);
    final String firstFileName = name + "." + suffix;
    if (outputRepository.exists(firstFileName) == false)
    {
      return firstFileName;
    }
    int counter = 0;
    while (true)
    {
      if (counter < 0) // wraparound should not happen..
      {
        throw new IOException();
      }
      final String filename = name + counter + "." + suffix;
      if (outputRepository.exists(filename) == false)
      {
        return filename;
      }
      counter += 1;
    }
  }

  protected String getSuffixForType(final String mimeType)
  {
    if ("image/png".equals(mimeType))
    {
      return "png";
    }
    if ("image/jpeg".equals(mimeType))
    {
      return "jpg";
    }
    if ("image/gif".equals(mimeType))
    {
      return "gif";
    }

    // todo ... use a flexible mapping ...
    return "dat";
  }
}
