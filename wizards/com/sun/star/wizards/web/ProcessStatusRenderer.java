/*************************************************************************
 *
 *  $RCSfile: ProcessStatusRenderer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $  $Date: 2004-05-19 13:13:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 */

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
public class ProcessStatusRenderer implements Renderer, WebWizardConst {
    Map strings = new Hashtable(12);

    public ProcessStatusRenderer(WebWizardDialogResources res) {
        strings.put(TASK_EXPORT_DOCUMENTS , res.resTaskExportDocs );
        strings.put(TASK_EXPORT_PREPARE , res.resTaskExportPrepare );
        strings.put(TASK_GENERATE_COPY , res.resTaskGenerateCopy );
        strings.put(TASK_GENERATE_PREPARE , res.resTaskGeneratePrepare );
        strings.put(TASK_GENERATE_XSL , res.resTaskGenerateXsl );
        strings.put(TASK_PREPARE , res.resTaskPrepare );
        //strings.put(TASK_PUBLISH , res.resTaskPublish );
        strings.put(LOCAL_PUBLISHER, res.resTaskPublishLocal );
        strings.put(ZIP_PUBLISHER, res.resTaskPublishZip );
        strings.put(FTP_PUBLISHER, res.resTaskPublishFTP );

        strings.put(TASK_PUBLISH_PREPARE , res.resTaskPublishPrepare );
        strings.put(TASK_FINISH , res.resTaskFinish );
    }

    public String render(Object object) {
        return (String)strings.get(object);
    }

}
