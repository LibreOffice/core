/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package com.sun.star.wizards.common;

// import com.sun.star.wizards.common.Resource;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.ui.UIConsts;

public class TerminateWizardException extends Exception
{

    public TerminateWizardException(XMultiServiceFactory xMSF)
    {
        Resource oResource = new Resource(xMSF, "AutoPilot", "dbw");
        String sErrorMessage = oResource.getResText(UIConsts.RID_DB_COMMON + 6);
        SystemDialog.showMessageBox(xMSF, "ErrorBox", com.sun.star.awt.VclWindowPeerAttribute.OK, sErrorMessage);
        printStackTrace(System.out);
    }
}
