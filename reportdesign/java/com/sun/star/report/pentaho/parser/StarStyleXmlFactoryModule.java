/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StarStyleXmlFactoryModule.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:08 $
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


package com.sun.star.report.pentaho.parser;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.parser.office.DocumentStylesReadHandler;
import org.jfree.xmlns.parser.XmlDocumentInfo;
import org.jfree.xmlns.parser.XmlFactoryModule;
import org.jfree.xmlns.parser.XmlReadHandler;

public class StarStyleXmlFactoryModule implements XmlFactoryModule
{
  public StarStyleXmlFactoryModule()
  {
  }

  public XmlReadHandler createReadHandler (final XmlDocumentInfo documentInfo)
  {
    return new DocumentStylesReadHandler();
  }

  public int getDocumentSupport (final XmlDocumentInfo documentInfo)
  {
    final String rootNamespace = documentInfo.getRootElementNameSpace();
    if (OfficeNamespaces.OFFICE_NS.equals(rootNamespace) == false)
    {
      return XmlFactoryModule.NOT_RECOGNIZED;
    }
    else if ("document-styles".equals(documentInfo.getRootElement()))
    {
      return XmlFactoryModule.RECOGNIZED_BY_NAMESPACE;
    }
    return XmlFactoryModule.NOT_RECOGNIZED;
  }

  public String getDefaultNamespace(final XmlDocumentInfo documentInfo)
  {
    return null;
  }
}
