/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FilterExporter.java,v $
 * $Revision: 1.5 $
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
package com.sun.star.wizards.web.export;

import com.sun.star.io.IOException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.ui.event.Task;
import com.sun.star.wizards.web.data.CGDocument;
import com.sun.star.wizards.web.data.CGExporter;

/**
 * @author rpiterman
 *
 * An exporter which is configured with a filter name, and
 * uses the specified filter to export documents.
 */
public class FilterExporter extends AbstractExporter
{

    protected String filterName;
    protected Properties props = new Properties();

    /* (non-Javadoc)
     * @see com.sun.star.wizards.web.export.Exporter#export(java.lang.Object, java.io.File, com.sun.star.wizards.web.data.CGSettings, com.sun.star.lang.XMultiServiceFactory)
     */
    public boolean export(CGDocument source, String target, XMultiServiceFactory xmsf, Task task) throws IOException
    {

        boolean result = true;
        Object document = null;

        try
        {
            document = openDocument(source, xmsf);
            task.advance(true);
            storeToURL(document, target, filterName, props.getProperties());
            task.advance(true);

        }
        catch (IOException iox)
        {
            iox.printStackTrace(System.out);
            result = false;
            throw iox;
        }
        finally
        {
            closeDocument(document, xmsf);
            calcFileSize(source, target, xmsf);
            task.advance(true);
        }
        return result;

    }

    /* (non-Javadoc)
     * @see com.sun.star.wizards.web.export.Exporter#init(com.sun.star.wizards.web.data.CGExporter)
     */
    public void init(CGExporter exporter_)
    {
        super.init(exporter_);
        filterName = getArgument("Filter", exporter_);
    }
}
