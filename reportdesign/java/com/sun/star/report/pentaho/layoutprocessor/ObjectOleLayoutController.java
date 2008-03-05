/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ObjectOleLayoutController.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:32:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
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
package com.sun.star.report.pentaho.layoutprocessor;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.ObjectOleElement;
import java.util.Iterator;
import java.util.Vector;
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
        final Vector masterfields = element.getMasterfields();
        final DataRow view = getFlowController().getMasterRow().getGlobalView();
        for (Iterator iter = masterfields.iterator(); iter.hasNext();)
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

    protected LayoutController delegateContentGeneration(ReportTarget target) throws ReportProcessingException, ReportDataFactoryException, DataSourceException
    {
        final ObjectOleElement element = (ObjectOleElement) getNode();
        final String url = element.getUrl();
        if (url != null)
        {
            final AttributeMap ole = new AttributeMap();
            ole.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, Element.NAMESPACE_ATTRIBUTE, OfficeNamespaces.INTERNAL_NS);
            ole.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, Element.TYPE_ATTRIBUTE, "object-ole");
            ole.setAttribute(OfficeNamespaces.INTERNAL_NS, "href", url);
            ole.setAttribute(OfficeNamespaces.INTERNAL_NS, "class-id", element.getClassid());
            final Vector masterfields = element.getMasterfields();
            Vector values = new Vector();
            final DataRow view = getFlowController().getMasterRow().getGlobalView();
            for (Iterator iter = masterfields.iterator(); iter.hasNext();)
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
            ole.setAttribute(OfficeNamespaces.INTERNAL_NS, "master-columns", masterfields);
            ole.setAttribute(OfficeNamespaces.INTERNAL_NS, "master-values", values);
            ole.setAttribute(OfficeNamespaces.INTERNAL_NS, "detail-columns", element.getDetailfields());

            target.startElement(ole);
            target.endElement(ole);
        }

        return join(getFlowController());
    }
}
