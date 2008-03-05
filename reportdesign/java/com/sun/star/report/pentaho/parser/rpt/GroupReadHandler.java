/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GroupReadHandler.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:43:16 $
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
import com.sun.star.report.pentaho.model.OfficeGroup;
import com.sun.star.report.pentaho.model.OfficeGroupInstanceSection;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import org.jfree.report.expressions.FormulaExpression;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;
import org.jfree.xmlns.parser.XmlReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

public class GroupReadHandler extends ElementReadHandler
{

    private GroupSectionReadHandler groupHeader;
    private GroupSectionReadHandler groupFooter;
    private GroupReadHandler childGroup;
    private RootTableReadHandler detailSection;
    private OfficeGroup group;
    private OfficeGroupInstanceSection groupInstanceSection;
    private ArrayList functionHandlers;

    public GroupReadHandler()
    {
        group = new OfficeGroup();
        groupInstanceSection = new OfficeGroupInstanceSection();
        groupInstanceSection.setNamespace(OfficeNamespaces.INTERNAL_NS);
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
        if (groupExpr != null)
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
            childGroup = new GroupReadHandler();
            return childGroup;
        }
        if ("detail".equals(tagName))
        {
            detailSection = new DetailRootTableReadHandler();
            return detailSection;
        }
        if ("group-footer".equals(tagName))
        {
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
        groupBody.setNamespace(OfficeNamespaces.INTERNAL_NS);
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
