/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: GroupReadHandler.java,v $
 * $Revision: 1.7 $
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
package com.sun.star.report.pentaho.parser.rpt;

import com.sun.star.report.OfficeToken;
import java.util.ArrayList;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.OfficeGroup;
import com.sun.star.report.pentaho.model.OfficeGroupInstanceSection;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import java.util.List;
import org.jfree.report.JFreeReportInfo;
import org.jfree.report.expressions.FormulaExpression;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;
import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

public class GroupReadHandler extends ElementReadHandler
{

    private GroupSectionReadHandler groupHeader;
    private GroupSectionReadHandler groupFooter;
    private GroupReadHandler childGroup;
    private RootTableReadHandler detailSection;
    private final OfficeGroup group;
    private final OfficeGroupInstanceSection groupInstanceSection;
    private final List functionHandlers;
    private final ReportReadHandler rh;

    public GroupReadHandler(final ReportReadHandler _rh)
    {
        rh = _rh;
        group = new OfficeGroup();
        groupInstanceSection = new OfficeGroupInstanceSection();
        groupInstanceSection.setNamespace(JFreeReportInfo.REPORT_NAMESPACE);
        groupInstanceSection.setType("group-instance");
        group.addNode(groupInstanceSection);
        functionHandlers = new ArrayList();
    }

    /**
     * Starts parsing.
     *
     * @param attrs the attributes.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void startParsing(final Attributes attrs) throws SAXException
    {
        super.startParsing(attrs);

        final String groupExpr = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "group-expression");
        if ( groupExpr != null && !"".equals(groupExpr) )
        {
            final FormulaExpression function = new FormulaExpression();
            function.setFormula(groupExpr);
            groupInstanceSection.setGroupingExpression(function);
        }
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
        if (!OfficeNamespaces.OOREPORT_NS.equals(uri))
        {
            return null;
        }
        if ("function".equals(tagName))
        {
            final FunctionReadHandler erh = new FunctionReadHandler();
            functionHandlers.add(erh);
            return erh;
        }
        if ("group-header".equals(tagName))
        {
            groupHeader = new GroupSectionReadHandler();
            return groupHeader;
        }
        if ("group".equals(tagName))
        {
            childGroup = new GroupReadHandler(rh);
            return childGroup;
        }
        if ("detail".equals(tagName))
        {
            detailSection = new DetailRootTableReadHandler();
            rh.setDetail(detailSection);
            return detailSection;
        }
        if ("group-footer".equals(tagName))
        {
            ((Element)((Section)rh.getDetail().getElement()).getNode(0)).setAttribute(JFreeReportInfo.REPORT_NAMESPACE,"has-group-footer", OfficeToken.TRUE);
            groupFooter = new GroupSectionReadHandler();
            return groupFooter;
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
        for (int i = 0; i < functionHandlers.size(); i++)
        {
            final FunctionReadHandler handler =
                    (FunctionReadHandler) functionHandlers.get(i);
            groupInstanceSection.addExpression(handler.getExpression());
        }

        if (groupHeader != null)
        {
            groupInstanceSection.addNode(groupHeader.getElement());
        }

        final Section groupBody = new Section();
        groupBody.setNamespace(JFreeReportInfo.REPORT_NAMESPACE);
        groupBody.setType("group-body");
        groupInstanceSection.addNode(groupBody);
        // XOR: Either the detail or the group section can be set ..
        if (detailSection != null)
        {
            groupBody.addNode(detailSection.getElement());
        }
        else if (childGroup != null)
        {
            groupBody.addNode(childGroup.getElement());
        }

        if (groupFooter != null)
        {
            groupInstanceSection.addNode(groupFooter.getElement());
        }
    }

    public Element getElement()
    {
        return group;
    }
}
