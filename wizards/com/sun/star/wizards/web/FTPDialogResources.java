/*************************************************************************
 *
 *  $RCSfile: FTPDialogResources.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $  $Date: 2004-05-19 13:11:54 $
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

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Resource;

public class FTPDialogResources extends Resource  {

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
    String resConnecting ;

public FTPDialogResources(XMultiServiceFactory xmsf) {
  super(xmsf,UNIT_NAME,MODULE_NAME);

  /**
   * Delete the String, uncomment the getResText method
   *
   */
    resFTPDialog_title          = getResText(RID_FTPDIALOG_START + 0);
    reslblUsername_value        = getResText(RID_FTPDIALOG_START + 1);
    reslblPassword_value        = getResText(RID_FTPDIALOG_START + 2);
    resbtnConnect_value         = getResText(RID_FTPDIALOG_START + 3);
    resln1_value                = getResText(RID_FTPDIALOG_START + 4);
    reslblFTPAddress_value      = getResText(RID_FTPDIALOG_START + 5);
    resln2_value                = getResText(RID_FTPDIALOG_START + 6);
    resln3_value                = getResText(RID_FTPDIALOG_START + 7);
    resbtnDir_value             = getResText(RID_FTPDIALOG_START + 8);
    resFTPDisconnected          = getResText(RID_FTPDIALOG_START + 9);
    resFTPConnected             = getResText(RID_FTPDIALOG_START + 10);
    resFTPUserPwdWrong          = getResText(RID_FTPDIALOG_START + 11);
    resFTPServerNotFound        = getResText(RID_FTPDIALOG_START + 12);
    resFTPRights                = getResText(RID_FTPDIALOG_START + 13);
    resFTPHostUnreachable       = getResText(RID_FTPDIALOG_START + 14);
    resFTPUnknownError          = getResText(RID_FTPDIALOG_START + 15);
    resFTPDirectory             = getResText(RID_FTPDIALOG_START + 16);
    resIllegalFolder            = getResText(RID_FTPDIALOG_START + 17);
    resConnecting               = getResText(RID_FTPDIALOG_START + 18);

    resbtnCancel_value          = getResText(RID_COMMON_START + 11);
    resbtnOK_value              = getResText(RID_COMMON_START + 18);
    resbtnHelp_value            = getResText(RID_COMMON_START + 15);

    restxtDir_value             = "/";

  }
}
