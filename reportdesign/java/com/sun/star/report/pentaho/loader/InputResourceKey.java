/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: InputResourceKey.java,v $
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

import java.io.Serializable;

/**
 * Creation-Date: Feb 22, 2007, 8:51:42 PM
 *
 * @author Thomas Morgner
 */
public class InputResourceKey implements Serializable
{
  private static final long serialVersionUID = 2819901838705793075L;

  private final Object inputRepositoryId;
  private final String path;

  public InputResourceKey(final Object inputRepositoryId, final String path)
  {
    this.inputRepositoryId = inputRepositoryId;
    this.path = path;
  }

  public Object getInputRepositoryId()
  {
    return inputRepositoryId;
  }

  public String getPath()
  {
    return path;
  }

  public String toString()
  {
    return "InputResourceKey{" +
           "inputRepositoryId=" + inputRepositoryId +
           ", path='" + path + '\'' +
           '}';
  }
}
