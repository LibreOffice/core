/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ProcessErrorHandler.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:56:58 $
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
package com.sun.star.wizards.web;

import com.sun.star.awt.XWindowPeer;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.web.data.CGDocument;
import com.sun.star.wizards.web.data.CGPublish;

/**
 * @author rpiterman
 * used to interact error accuring when generating the
 * web-site to the user.
 * This class renders the different errors,
 * replaceing some strings from the resources with
 * content of the given arguments, depending on the error
 * that accured.
 */
public class ProcessErrorHandler extends AbstractErrorHandler
    implements  WebWizardConst,
                ProcessErrors
{

    private static final String FILENAME= "%FILENAME";
    private static final String URL = "%URL";
    private static final String ERROR = "%ERROR";


    WebWizardDialogResources resources;

    public ProcessErrorHandler(XMultiServiceFactory xmsf, XWindowPeer peer, WebWizardDialogResources res) {
        super(xmsf, peer);
        resources = res;
    }

    protected String getMessageFor(Exception ex, Object obj, int ix, int errType) {

        switch (ix) {

            case ERROR_MKDIR :
                return JavaTools.replaceSubString(resources.resErrDocExport, ((CGDocument) obj).localFilename, FILENAME);

            case ERROR_EXPORT :
                return "";
            case ERROR_EXPORT_MKDIR :
                return JavaTools.replaceSubString(resources.resErrMkDir, ((CGDocument) obj).localFilename, FILENAME);
            case ERROR_DOC_VALIDATE :
                return JavaTools.replaceSubString(resources.resErrDocInfo, ((CGDocument) obj).localFilename, FILENAME);
            case ERROR_EXPORT_IO :
                return JavaTools.replaceSubString(resources.resErrExportIO, ((CGDocument) obj).localFilename, FILENAME);
            case ERROR_EXPORT_SECURITY :
                return JavaTools.replaceSubString(resources.resErrSecurity, ((CGDocument) obj).localFilename, FILENAME);

            case ERROR_GENERATE_XSLT :
                return resources.resErrTOC;
            case ERROR_GENERATE_COPY :
                return resources.resErrTOCMedia;

            case ERROR_PUBLISH :
                return JavaTools.replaceSubString(resources.resErrPublish, ((CGPublish) obj).cp_URL, URL);
            case ERROR_PUBLISH_MEDIA :
                return resources.resErrPublishMedia;

            case ERROR_CLEANUP :
                return resources.resErrUnexpected;

            default :
                return JavaTools.replaceSubString(resources.resErrUnknown, ex.getClass().getName() + "/" + obj.getClass().getName() + "/" + ix, ERROR);
        }
    }
}
