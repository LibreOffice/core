/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ObjectOleLayoutController.java,v $
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
package com.sun.star.report.pentaho.layoutprocessor;

import com.sun.star.report.SDBCReportDataFactory;
import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.OfficeToken;
import com.sun.star.report.pentaho.model.ObjectOleElement;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import org.jfree.report.DataSourceException;
import org.jfree.report.ReportDataFactoryException;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.flow.layoutprocessor.LayoutController;
import org.jfree.layouting.util.AttributeMap;
import org.jfree.report.DataFlags;
import org.jfree.report.DataRow;
import org.jfree.report.JFreeReportInfo;
import org.jfree.report.structure.Element;

/**
 *
 * @author Ocke Janssen
 */
public class ObjectOleLayoutController extends AbstractReportElementLayoutController
{

    public ObjectOleLayoutController()
    {
    }

    protected boolean isValueChanged()
    {
        final ObjectOleElement element = (ObjectOleElement) getNode();
        final List masterfields = element.getMasterfields();
        final DataRow view = getFlowController().getMasterRow().getGlobalView();
        for (final Iterator iter = masterfields.iterator(); iter.hasNext();)
        {
            final String master = (String) iter.next();
            try
            {
                final DataFlags flags = view.getFlags(master);
                if (flags != null && flags.isChanged())
                {
                    return true;
                }
            }
            catch (DataSourceException e)
            {
            // ignore .. assume that the reference has not changed.
            }
        }
        return false;
    }

    protected LayoutController delegateContentGeneration(final ReportTarget target) throws ReportProcessingException, ReportDataFactoryException, DataSourceException
    {
        final ObjectOleElement element = (ObjectOleElement) getNode();
        final String url = element.getUrl();
        if (url != null)
        {
            final AttributeMap ole = new AttributeMap();
            ole.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, Element.NAMESPACE_ATTRIBUTE, OfficeNamespaces.INTERNAL_NS);
            ole.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, Element.TYPE_ATTRIBUTE, OfficeToken.OBJECT_OLE);
            ole.setAttribute(OfficeNamespaces.INTERNAL_NS, "href", url);
            ole.setAttribute(OfficeNamespaces.INTERNAL_NS, "class-id", element.getClassid());
            final List masterfields = element.getMasterfields();
            final List values = new ArrayList();
            final DataRow view = getFlowController().getMasterRow().getGlobalView();
            for (final Iterator iter = masterfields.iterator(); iter.hasNext();)
            {
                final String master = (String) iter.next();
                try
                {
                    final DataFlags flags = view.getFlags(master);
                    if (flags != null)
                    {
                        values.add(flags.getValue());
                    }
                }
                catch (DataSourceException e)
                {
                // ignore .. assume that the reference has not changed.
                }
            }
            ole.setAttribute(OfficeNamespaces.INTERNAL_NS, SDBCReportDataFactory.MASTER_COLUMNS, masterfields);
            ole.setAttribute(OfficeNamespaces.INTERNAL_NS, SDBCReportDataFactory.MASTER_VALUES, values);
            ole.setAttribute(OfficeNamespaces.INTERNAL_NS, SDBCReportDataFactory.DETAIL_COLUMNS, element.getDetailfields());

            target.startElement(ole);
            target.endElement(ole);
        }

        return join(getFlowController());
    }
}
