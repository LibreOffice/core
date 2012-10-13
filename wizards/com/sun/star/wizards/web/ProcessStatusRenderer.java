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
package com.sun.star.wizards.web;

import java.util.HashMap;
import java.util.Map;

import com.sun.star.wizards.common.IRenderer;

/**
 * recieves status calls from the status dialog which
 * apears when the user clicks "create". 
 * allocates strings from the resources to
 * display the current task status.
 * (renders the state to resource strings)
 */
public class ProcessStatusRenderer implements IRenderer, WebWizardConst
{

    Map<String, String> strings = new HashMap<String, String>(12);

    public ProcessStatusRenderer(WebWizardDialogResources res)
    {
        strings.put(TASK_EXPORT_DOCUMENTS, res.resTaskExportDocs);
        strings.put(TASK_EXPORT_PREPARE, res.resTaskExportPrepare);
        strings.put(TASK_GENERATE_COPY, res.resTaskGenerateCopy);
        strings.put(TASK_GENERATE_PREPARE, res.resTaskGeneratePrepare);
        strings.put(TASK_GENERATE_XSL, res.resTaskGenerateXsl);
        strings.put(TASK_PREPARE, res.resTaskPrepare);
        strings.put(LOCAL_PUBLISHER, res.resTaskPublishLocal);
        strings.put(ZIP_PUBLISHER, res.resTaskPublishZip);
        strings.put(FTP_PUBLISHER, res.resTaskPublishFTP);

        strings.put(TASK_PUBLISH_PREPARE, res.resTaskPublishPrepare);
        strings.put(TASK_FINISH, res.resTaskFinish);
    }

    public String render(Object object)
    {
        return strings.get(object);
    }
}
