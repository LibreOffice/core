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

import com.sun.star.awt.XWindowPeer;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.web.data.CGDocument;
import com.sun.star.wizards.web.data.CGPublish;

/**
 * used to interact error accuring when generating the
 * web-site to the user.
 * This class renders the different errors,
 * replaceing some strings from the resources with
 * content of the given arguments, depending on the error
 * that accured.
 */
public class ProcessErrorHandler extends AbstractErrorHandler
        implements WebWizardConst,
        ProcessErrors
{

    private static final String FILENAME = "%FILENAME";
    private static final String URL = "%URL";
    private static final String ERROR = "%ERROR";
    WebWizardDialogResources resources;

    public ProcessErrorHandler(XMultiServiceFactory xmsf, XWindowPeer peer, WebWizardDialogResources res)
    {
        super(xmsf, peer);
        resources = res;
    }

    protected String getMessageFor(Exception ex, Object obj, int ix, int errType)
    {

        switch (ix)
        {

            case ERROR_MKDIR:
                return JavaTools.replaceSubString(resources.resErrDocExport, ((CGDocument) obj).localFilename, FILENAME);
            case ERROR_EXPORT_MKDIR:
                return JavaTools.replaceSubString(resources.resErrMkDir, ((CGDocument) obj).localFilename, FILENAME);
            case ERROR_DOC_VALIDATE:
                return JavaTools.replaceSubString(resources.resErrDocInfo, ((CGDocument) obj).localFilename, FILENAME);
            case ERROR_EXPORT_IO:
                return JavaTools.replaceSubString(resources.resErrExportIO, ((CGDocument) obj).localFilename, FILENAME);
            case ERROR_EXPORT_SECURITY:
                return JavaTools.replaceSubString(resources.resErrSecurity, ((CGDocument) obj).localFilename, FILENAME);
            case ERROR_GENERATE_XSLT:
                return resources.resErrTOC;
            case ERROR_GENERATE_COPY:
                return resources.resErrTOCMedia;
            case ERROR_PUBLISH:
                return JavaTools.replaceSubString(resources.resErrPublish, ((CGPublish) obj).cp_URL, URL);
            case ERROR_EXPORT:
            case ERROR_PUBLISH_MEDIA:
                return resources.resErrPublishMedia;
            case ERROR_CLEANUP:
                return resources.resErrUnexpected;

            default:
                return JavaTools.replaceSubString(resources.resErrUnknown, ex.getClass().getName() + "/" + obj.getClass().getName() + "/" + ix, ERROR);
        }
    }
}
