/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FTPDialogResources.java,v $
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
