/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package com.sun.star.wizards.web.export;

import com.sun.star.io.IOException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.ui.event.Task;
import com.sun.star.wizards.web.data.CGDocument;
import com.sun.star.wizards.web.data.CGExporter;

/**
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
            iox.printStackTrace(System.err);
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
