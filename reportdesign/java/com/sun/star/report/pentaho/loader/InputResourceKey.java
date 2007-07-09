/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: InputResourceKey.java,v $
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

import java.io.Serializable;

/**
 * Creation-Date: Feb 22, 2007, 8:51:42 PM
 *
 * @author Thomas Morgner
 */
public class InputResourceKey implements Serializable
{
  private static final long serialVersionUID = 2819901838705793075L;

  private Object inputRepositoryId;
  private String path;

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
