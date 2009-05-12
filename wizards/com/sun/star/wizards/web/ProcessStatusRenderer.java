/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ProcessStatusRenderer.java,v $
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
package com.sun.star.wizards.web;

import java.util.Hashtable;
import java.util.Map;

import com.sun.star.wizards.common.Renderer;

/**
 * @author rpiterman
 * recieves status calls from the status dialog which
 * apears when the user clicks "create".
 * allocates strings from the resources to
 * display the current task status.
 * (renders the state to resource strings)
 */
public class ProcessStatusRenderer implements Renderer, WebWizardConst
{

    Map strings = new Hashtable(12);

    public ProcessStatusRenderer(WebWizardDialogResources res)
    {
        strings.put(TASK_EXPORT_DOCUMENTS, res.resTaskExportDocs);
        strings.put(TASK_EXPORT_PREPARE, res.resTaskExportPrepare);
        strings.put(TASK_GENERATE_COPY, res.resTaskGenerateCopy);
        strings.put(TASK_GENERATE_PREPARE, res.resTaskGeneratePrepare);
        strings.put(TASK_GENERATE_XSL, res.resTaskGenerateXsl);
        strings.put(TASK_PREPARE, res.resTaskPrepare);
        //strings.put(TASK_PUBLISH , res.resTaskPublish );
        strings.put(LOCAL_PUBLISHER, res.resTaskPublishLocal);
        strings.put(ZIP_PUBLISHER, res.resTaskPublishZip);
        strings.put(FTP_PUBLISHER, res.resTaskPublishFTP);

        strings.put(TASK_PUBLISH_PREPARE, res.resTaskPublishPrepare);
        strings.put(TASK_FINISH, res.resTaskFinish);
    }

    public String render(Object object)
    {
        return (String) strings.get(object);
    }
}
