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

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Resource;

public class FTPDialogResources extends Resource
{

    final static String UNIT_NAME = "dbwizres";
    final static String MODULE_NAME = "dbw";
    final static int RID_FTPDIALOG_START = 4200;
    final static int RID_COMMON_START = 500;
    String resFTPDialog_title;
    String reslblUsername_value;
    String reslblPassword_value;
    String resbtnConnect_value;
    String resbtnOK_value;
    String resbtnHelp_value;
    String resbtnCancel_value;
    String resln1_value;
    String reslblFTPAddress_value;
    String resln2_value;
    String resln3_value;
    String restxtDir_value;
    String resbtnDir_value;
    String resFTPDisconnected;
    String resFTPConnected;
    String resFTPUserPwdWrong;
    String resFTPServerNotFound;
    String resFTPRights;
    String resFTPHostUnreachable;
    String resFTPUnknownError;
    String resFTPDirectory;
    String resIllegalFolder;
    String resConnecting;

    public FTPDialogResources(XMultiServiceFactory xmsf)
    {
        super(xmsf, UNIT_NAME, MODULE_NAME);

        /**
         * Delete the String, uncomment the getResText method
         * 
         */
        resFTPDialog_title = getResText(RID_FTPDIALOG_START + 0);
        reslblUsername_value = getResText(RID_FTPDIALOG_START + 1);
        reslblPassword_value = getResText(RID_FTPDIALOG_START + 2);
        resbtnConnect_value = getResText(RID_FTPDIALOG_START + 3);
        resln1_value = getResText(RID_FTPDIALOG_START + 4);
        reslblFTPAddress_value = getResText(RID_FTPDIALOG_START + 5);
        resln2_value = getResText(RID_FTPDIALOG_START + 6);
        resln3_value = getResText(RID_FTPDIALOG_START + 7);
        resbtnDir_value = getResText(RID_FTPDIALOG_START + 8);
        resFTPDisconnected = getResText(RID_FTPDIALOG_START + 9);
        resFTPConnected = getResText(RID_FTPDIALOG_START + 10);
        resFTPUserPwdWrong = getResText(RID_FTPDIALOG_START + 11);
        resFTPServerNotFound = getResText(RID_FTPDIALOG_START + 12);
        resFTPRights = getResText(RID_FTPDIALOG_START + 13);
        resFTPHostUnreachable = getResText(RID_FTPDIALOG_START + 14);
        resFTPUnknownError = getResText(RID_FTPDIALOG_START + 15);
        resFTPDirectory = getResText(RID_FTPDIALOG_START + 16);
        resIllegalFolder = getResText(RID_FTPDIALOG_START + 17);
        resConnecting = getResText(RID_FTPDIALOG_START + 18);

        resbtnCancel_value = getResText(RID_COMMON_START + 11);
        resbtnOK_value = getResText(RID_COMMON_START + 18);
        resbtnHelp_value = getResText(RID_COMMON_START + 15);

        restxtDir_value = "/";

    }
}
