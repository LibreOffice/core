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

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.ui.event.Task;
import com.sun.star.wizards.web.data.CGDocument;

public class CopyExporter extends AbstractExporter
{

    /* (non-Javadoc)
     * @see com.sun.star.wizards.web.export.Exporter#export(java.lang.Object, java.io.File, com.sun.star.wizards.web.data.CGSettings, com.sun.star.lang.XMultiServiceFactory)
     */
    public boolean export(CGDocument source, String target, XMultiServiceFactory xmsf, Task task)
    {
        try
        {
            task.advance(true);


            /*
             * this will open the document, and calculate the pages/slides number
             * in it.
             */
            if (exporter.cp_PageType > 0)
            {
                closeDocument(openDocument(source, xmsf), xmsf);
            }

            task.advance(true);

            String newTarget = FileAccess.connectURLs(
                    FileAccess.getParentDir(target), source.urlFilename);

            boolean b = getFileAccess(xmsf).copy(source.cp_URL, newTarget);

            task.advance(true);

            calcFileSize(source, newTarget, xmsf);

            return b;

        }
        catch (Exception ex)
        {
            return false;
        }
    }
}
