/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportReadHandler.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:09 $
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


package com.sun.star.report.pentaho.parser.rpt;

import java.util.ArrayList;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.OfficeReport;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;
import org.jfree.report.JFreeReportInfo;
import org.jfree.xmlns.parser.XmlReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

public class ReportReadHandler extends ElementReadHandler
{
  private RootTableReadHandler pageHeader;
  private RootTableReadHandler pageFooter;
  private RootTableReadHandler reportHeader;
  private RootTableReadHandler reportFooter;
  private RootTableReadHandler detail;
  private GroupReadHandler groups;
  private OfficeReport rootSection;
  private ArrayList functionHandlers;

  public ReportReadHandler()
  {
    rootSection = new OfficeReport();
    rootSection.setAttribute(OfficeNamespaces.INTERNAL_NS, "simple-report-structure", Boolean.TRUE);
    functionHandlers = new ArrayList();
  }

  /**
   * Returns the handler for a child element.
   *
   * @param tagName the tag name.
   * @param atts    the attributes.
   * @return the handler or null, if the tagname is invalid.
   * @throws org.xml.sax.SAXException if there is a parsing error.
   */
  protected XmlReadHandler getHandlerForChild(final String uri,
                                              final String tagName,
                                              final Attributes atts)
      throws SAXException
  {
    if (OfficeNamespaces.OOREPORT_NS.equals(uri) == false)
    {
      return null;
    }
    if ("function".equals(tagName))
    {
      final FunctionReadHandler erh = new FunctionReadHandler();
      functionHandlers.add(erh);
      return erh;
    }
    if ("page-header".equals(tagName))
    {
      pageHeader = new RootTableReadHandler();
      return pageHeader;
    }
    if ("report-header".equals(tagName))
    {
      reportHeader = new RootTableReadHandler();
      return reportHeader;
    }
    if ("report-footer".equals(tagName))
    {
      reportFooter = new RootTableReadHandler();
      return reportFooter;
    }
    if ("page-footer".equals(tagName))
    {
      pageFooter = new RootTableReadHandler();
      return pageFooter;
    }
    if ("detail".equals(tagName))
    {
      detail = new DetailRootTableReadHandler();
      return detail;
    }
    if ("group".equals(tagName))
    {
      groups = new GroupReadHandler();
      return groups;
    }
    return null;
  }

  /**
   * Done parsing.
   *
   * @throws org.xml.sax.SAXException if there is a parsing error.
   */
  protected void doneParsing() throws SAXException
  {
    if (pageHeader != null)
    {
      rootSection.setPageHeader(pageHeader.getElement());
    }
    if (pageFooter != null)
    {
      rootSection.setPageFooter(pageFooter.getElement());
    }
    if (reportHeader != null)
    {
      rootSection.setReportHeader(reportHeader.getElement());
    }

    final Section groupBody = new Section();
    groupBody.setNamespace(OfficeNamespaces.INTERNAL_NS);
    groupBody.setType("report-body");
    rootSection.setBodySection(groupBody);

    // XOR: Either the detail or the group section can be set ..
    if (groups != null)
    {
      groupBody.addNode(groups.getElement());
    }
    else if (detail != null)
    {
      groupBody.addNode(detail.getElement());
    }

    if (reportFooter != null)
    {
      rootSection.setReportFooter(reportFooter.getElement());
    }

    for (int i = 0; i < functionHandlers.size(); i++)
    {
      final FunctionReadHandler handler =
          (FunctionReadHandler) functionHandlers.get(i);
      rootSection.addExpression(handler.getExpression());
    }
  }


  public Element getElement()
  {
    return rootSection;
  }
}
