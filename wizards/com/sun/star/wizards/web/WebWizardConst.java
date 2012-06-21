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

public interface WebWizardConst
{

    public static final String LSTLOADSETTINGS_ITEM_CHANGED = "sessionSelected"; // "lstLoadSettingsItemChanged";
    public static final String BTNLOADSESSION_ACTION_PERFORMED = "loadSession"; // "btnLoadSessionActionPerformed";
    public static final String BTNDELSESSION_ACTION_PERFORMED = "delSession"; // "btnDelSessionActionPerformed";
    public static final String BTNADDDOC_ACTION_PERFORMED = "addDocument"; // "btnAddDocActionPerformed";
    public static final String BTNREMOVEDOC_ACTION_PERFORMED = "removeDocument"; // "btnRemoveDocActionPerformed";
    public static final String BTNDOCUP_ACTION_PERFORMED = "docUp"; // "btnDocUpActionPerformed";
    public static final String BTNDOCDOWN_ACTION_PERFORMED = "docDown"; // "btnDocDownActionPerformed";
    public static final String LSTSTYLES_ITEM_CHANGED = "refreshStylePreview"; // "lstStylesItemChanged";
    public static final String BTNBACKGROUNDS_ACTION_PERFORMED = "chooseBackground"; // "btnBackgroundsActionPerformed";
    public static final String BTNICONSETS_ACTION_PERFORMED = "chooseIconset"; // "btnIconSetsActionPerformed";
    public static final String BTNFAVICON_ACTION_PERFORMED = "chooseFavIcon"; // "btnFavIconActionPerformed";
    public static final String BTNPREVIEW_ACTION_PERFORMED = "documentPreview"; // "btnPreviewActionPerformed";
    public static final String BTNFTP_ACTION_PERFORMED = "setFTPPublish"; // "btnFTPActionPerformed";
    public static final String CHKLOCALDIR_ITEM_CHANGED = "checkPublish"; // "chkLocalDirItemChanged";
    public static final String CHKSAVESETTINGS_ITEM_CHANGED = "checkPublish"; // "chkSaveSettingsItemChanged";
    public static final String TXTSAVESETTINGS_TEXT_CHANGED = "checkPublish"; // "txtSaveSettingsTextChanged";
    public static final String BTNLOCALDIR_ACTION_PERFORMED = "setPublishLocalDir"; // "btnLocalDirActionPerformed";
    public static final String BTNZIP_ACTION_PERFORMED = "setZipFilename";// "btnZipActionPerformed";
    public static final String CONFIG_PATH = "/org.openoffice.Office.WebWizard/WebWizard";
    public static final String CONFIG_READ_PARAM = "cp_";
    public static final String TASK = "WWIZ";
    public static final String TASK_PREPARE = "t-prep";
    public static final String LOCAL_PUBLISHER = "local";
    public static final String FTP_PUBLISHER = "ftp";
    public static final String ZIP_PUBLISHER = "zip";
    public static final String TASK_EXPORT = "t_exp";
    public static final String TASK_EXPORT_PREPARE = "t_exp_prep";
    public static final String TASK_EXPORT_DOCUMENTS = "t_exp_docs";
    public static final String TASK_GENERATE_PREPARE = "t_gen_prep";
    public static final String TASK_GENERATE_XSL = "t_gen_x";
    public static final String TASK_GENERATE_COPY = "t_gen_cp";
    public static final String TASK_PUBLISH_PREPARE = "t_pub_prep";
    //public static final String TASK_PUBLISH = "t_pub";
    public static final String TASK_FINISH = "t_fin";
    /**
     * when the user adds more than this number
     * of documents to the list, a status dialog opens.
     */
    public static final int MIN_ADD_FILES_FOR_DIALOG = 2;
}
  