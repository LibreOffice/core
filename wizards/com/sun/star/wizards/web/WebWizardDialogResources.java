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
import com.sun.star.wizards.common.Configuration;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.common.Resource;
import com.sun.star.wizards.document.OfficeDocument;

public class WebWizardDialogResources
{

    String resSessionName;
    String resSizeTemplate;
    String resSpecifyNewFileLocation;
    String resCreatedTemplate;
    String resUpdatedTemplate;
    String resSlides;
    String resDelSessionConfirm;
    String resErrIsDirectory;
    String resErrDocValidate;
    String resStep1;
    String resStep2;
    String resStep3;
    String resStep4;
    String resStep5;
    String resStep6;
    String resStep7;
    String resDefaultArchiveFilename;
    String resValidatingDocuments;
    String resErrUnexpected;
    String resLoadingSession;
    String resErrExportIO;
    String resErrPublishMedia;
    String resErrPublish;
    String resErrUnknown;
    //String resErrDocClose;
    String resErrSecurity;
    String resErrMkDir;
    //String resErrDocOpen;
    String resErrDocExport;
    String resErrDocInfo;
    String resErrTOCMedia;
    String resErrTOC;
    final static String UNIT_NAME = "dbwizres";
    final static String MODULE_NAME = "dbw";
    final static int RID_WEBWIZARDDIALOG_START = 4000;
    final static int RID_COMMON_START = 500;
    final static int RID_DB_COMMON_START = 1000;
    String resWebWizardDialog_title;
    String reslblIntroduction_value;
    String resbtnDelSession_value;
    String reslbIntroTitle_value;
    String reslblContentTitle_value;
    String reslnDocsInfo_value;
    String reslblDocTitle_value;
    String reslblDocInfo_value;
    String reslblDocAuthor_value;
    String reslblDocExportFormat_value;
    String reslblTitleGeneralPage_value;
    String reslblSiteTitle_value;
    String reslblSiteDesc_value;
    String reslblSiteCreated_value;
    String reslblEmail_value;
    String reslblCopyright_value;
    String reslblSiteUpdated_value;
    String resbtnPreview_value;
    String reslblTitlePublish_value;
    String reslblCreateSite_value;
    String reschkLocalDir_value;
    String resbtnLocalDir_value;
    String reschkFTP_value;
    String resbtnFTP_value;
    String reschkZip_value;
    String resbtnZip_value;
    String reschkSaveSettings_value;
    String reslblSaveSettings_value;
    String reslblLoadSettings_value;
    String reslblSiteContent_value;
    String resbtnAddDoc_value;
    String resbtnRemoveDoc_value;
    String reslblLayoutTitle_value;
    String reslblStyleTitle_value;
    String reslblStyle_value;
    String reslblLayouts_value;
    String reschbDocDesc_value;
    String reschbDocAuthor_value;
    String reschkDocCreated_value;
    String reschkDocChanged_value;
    String reschkDocFilename_value;
    String reschkDocFormat_value;
    String reschkDocFormatIcon_value;
    String reschkDocPages_value;
    String reschkDocSize_value;
    String resFixedLine1_value;
    String reslblLayout2Title_value;
    String reslblDisplay_value;
    String reslblOptimizeFor_value;
    String resoptOptimize640x480_value;
    String resoptOptimize800x600_value;
    String resoptOptimize1024x768_value;
    final String resbtnDocUp_value = String.valueOf((char) 8743);
    final String resbtnDocDown_value = String.valueOf((char) 8744);
    String reslblBackground;
    String reslblIconset;
    String reslblIconSetInfo;
    String reslblMetaData;
    String resBtnChooseBackground;
    String resBtnChooseIconset;    //String resGenerate;
    //String resPublish;
    //String resExport;
    String resCancel;
    String resClose;
    String resCounter;
    String resStatusDialogTitle;
    String resPublishDir;
    String resFTPTargetExistsAsfile;
    String resLocalTargetExistsAsfile;
    String resZipTargetExists;
    String resFTPTargetNotEmpty;
    String resLocalTragetNotEmpty;
    String resSessionExists;
    String resTaskExport;
    String resTaskExportDocs;
    String resTaskExportPrepare;
    String resTaskGenerateCopy;
    String resTaskGeneratePrepare;
    String resTaskGenerateXsl;
    String resTaskPrepare;
    String resTaskPublishPrepare;
    String resTaskPublishLocal;
    String resTaskPublishFTP;
    String resTaskPublishZip;
    String resTaskFinish;
    String resPages;
    String resImages;
    String resAllFiles;
    String resZipFiles;
    String resGifFiles;
    String resOK;
    String resHelp;
    String resBackgroundsDialog;
    String resBackgroundsDialogCaption;
    String resIconsDialog;
    String resIconsDialogCaption;
    String resOther;
    String resDeselect;
    String resFinishedSuccess;
    String resFinishedNoSuccess;
    String resSessionNameNone;
    String reslblFTPDisabled;
    String resBackgroundNone;
    String resIconsetNone;
    String prodName;
    String resFTPTargetCouldNotCreate;
    String resFTPTargetCreate;
    String resZipTargetIsDir;
    String resLocalTargetCouldNotCreate;
    String resLocalTargetCreate;
    Resource oResource;

    public WebWizardDialogResources(XMultiServiceFactory xmsf, Resource _oResource)
    {
        oResource = _oResource;
        /**
         * Delete the String, uncomment the getResText method
         * 
         */
        resWebWizardDialog_title = oResource.getResText(RID_WEBWIZARDDIALOG_START + 1);
        reslblIntroduction_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 2);
        resbtnDelSession_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 4);
        reslbIntroTitle_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 5);
        reslblContentTitle_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 6);
        reslnDocsInfo_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 8);
        reslblDocTitle_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 9);
        reslblDocInfo_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 10);
        reslblDocAuthor_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 11);
        reslblDocExportFormat_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 12);
        reslblTitleGeneralPage_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 13);
        reslblSiteTitle_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 14);
        reslblSiteDesc_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 16);
        reslblSiteCreated_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 18);
        reslblEmail_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 20);
        reslblCopyright_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 21);
        reslblSiteUpdated_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 22);
        resbtnPreview_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 24);
        reslblTitlePublish_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 25);
        reslblCreateSite_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 26);
        reschkLocalDir_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 27);
        resbtnLocalDir_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 28);
        reschkFTP_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 29);
        //
        resbtnFTP_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 31);
        reschkZip_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 32);
        resbtnZip_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 33);
        reschkSaveSettings_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 34);
        reslblSaveSettings_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 35);
        reslblLoadSettings_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 36);
        reslblSiteContent_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 37);
        resbtnAddDoc_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 38);
        resbtnRemoveDoc_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 39);
        reslblLayoutTitle_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 40);
        reslblStyleTitle_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 41);
        reslblStyle_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 42);
        //
        reslblLayouts_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 48);
        //
        reschbDocDesc_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 50);
        reschbDocAuthor_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 51);
        reschkDocCreated_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 52);
        reschkDocChanged_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 53);
        reschkDocFilename_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 54);
        reschkDocFormat_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 55);
        reschkDocFormatIcon_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 56);
        reschkDocPages_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 57);
        reschkDocSize_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 58);
        resFixedLine1_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 59);
        reslblLayout2Title_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 60);
        reslblDisplay_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 61);
        reslblOptimizeFor_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 62);
        resoptOptimize640x480_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 64);
        resoptOptimize800x600_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 65);
        resoptOptimize1024x768_value = oResource.getResText(RID_WEBWIZARDDIALOG_START + 66);
        //resExport                         = oResource.getResText(RID_WEBWIZARDDIALOG_START + 67);
        //resGenerate                   = oResource.getResText(RID_WEBWIZARDDIALOG_START + 68);
        //resPublish                        = oResource.getResText(RID_WEBWIZARDDIALOG_START + 69);
        resStatusDialogTitle = oResource.getResText(RID_WEBWIZARDDIALOG_START + 70);
        resCounter = oResource.getResText(RID_WEBWIZARDDIALOG_START + 71);
        resPublishDir = oResource.getResText(RID_WEBWIZARDDIALOG_START + 72);
        resFTPTargetExistsAsfile = oResource.getResText(RID_WEBWIZARDDIALOG_START + 73);
        resLocalTargetExistsAsfile = oResource.getResText(RID_WEBWIZARDDIALOG_START + 74);
        resZipTargetExists = oResource.getResText(RID_WEBWIZARDDIALOG_START + 75);
        resFTPTargetNotEmpty = oResource.getResText(RID_WEBWIZARDDIALOG_START + 76);
        resLocalTragetNotEmpty = oResource.getResText(RID_WEBWIZARDDIALOG_START + 77);
        resSessionExists = oResource.getResText(RID_WEBWIZARDDIALOG_START + 78);
        resTaskExportDocs = oResource.getResText(RID_WEBWIZARDDIALOG_START + 79);
        resTaskExportPrepare = oResource.getResText(RID_WEBWIZARDDIALOG_START + 80);
        resTaskGenerateCopy = oResource.getResText(RID_WEBWIZARDDIALOG_START + 81);
        resTaskGeneratePrepare = oResource.getResText(RID_WEBWIZARDDIALOG_START + 82);
        resTaskGenerateXsl = oResource.getResText(RID_WEBWIZARDDIALOG_START + 83);
        resTaskPrepare = oResource.getResText(RID_WEBWIZARDDIALOG_START + 84);
        resTaskPublishPrepare = oResource.getResText(RID_WEBWIZARDDIALOG_START + 86);
        resTaskPublishLocal = oResource.getResText(RID_WEBWIZARDDIALOG_START + 87);
        resTaskPublishFTP = oResource.getResText(RID_WEBWIZARDDIALOG_START + 88);
        resTaskPublishZip = oResource.getResText(RID_WEBWIZARDDIALOG_START + 89);
        resTaskFinish = oResource.getResText(RID_WEBWIZARDDIALOG_START + 90);
        //resPages                      = oResource.getResText(RID_WEBWIZARDDIALOG_START + 91);
        resImages = oResource.getResText(RID_WEBWIZARDDIALOG_START + 94);
        resAllFiles = oResource.getResText(RID_WEBWIZARDDIALOG_START + 95);
        resZipFiles = oResource.getResText(RID_WEBWIZARDDIALOG_START + 96);
        resBackgroundsDialog = oResource.getResText(RID_WEBWIZARDDIALOG_START + 98);
        resBackgroundsDialogCaption = oResource.getResText(RID_WEBWIZARDDIALOG_START + 99);
        resIconsDialog = oResource.getResText(RID_WEBWIZARDDIALOG_START + 100);
        resIconsDialogCaption = oResource.getResText(RID_WEBWIZARDDIALOG_START + 101);
        resOther = oResource.getResText(RID_WEBWIZARDDIALOG_START + 102);
        resDeselect = oResource.getResText(RID_WEBWIZARDDIALOG_START + 103);
        resFinishedSuccess = oResource.getResText(RID_WEBWIZARDDIALOG_START + 104);
        resFinishedNoSuccess = oResource.getResText(RID_WEBWIZARDDIALOG_START + 105);
        resErrTOC = oResource.getResText(RID_WEBWIZARDDIALOG_START + 106);
        resErrTOCMedia = oResource.getResText(RID_WEBWIZARDDIALOG_START + 107);
        resErrDocInfo = oResource.getResText(RID_WEBWIZARDDIALOG_START + 108);
        resErrDocExport = oResource.getResText(RID_WEBWIZARDDIALOG_START + 109);
        resErrMkDir = oResource.getResText(RID_WEBWIZARDDIALOG_START + 110);
        resErrSecurity = oResource.getResText(RID_WEBWIZARDDIALOG_START + 111);
        resErrExportIO = oResource.getResText(RID_WEBWIZARDDIALOG_START + 112);
        resErrPublishMedia = oResource.getResText(RID_WEBWIZARDDIALOG_START + 113);
        resErrPublish = oResource.getResText(RID_WEBWIZARDDIALOG_START + 114);
        resErrUnknown = oResource.getResText(RID_WEBWIZARDDIALOG_START + 115);
        resErrDocValidate = oResource.getResText(RID_WEBWIZARDDIALOG_START + 116);
        resErrIsDirectory = oResource.getResText(RID_WEBWIZARDDIALOG_START + 117);
        resLoadingSession = oResource.getResText(RID_WEBWIZARDDIALOG_START + 118);
        resErrUnexpected = oResource.getResText(RID_WEBWIZARDDIALOG_START + 119);
        resValidatingDocuments = oResource.getResText(RID_WEBWIZARDDIALOG_START + 120);
        resDefaultArchiveFilename = oResource.getResText(RID_WEBWIZARDDIALOG_START + 121);
        resStep1 = oResource.getResText(RID_WEBWIZARDDIALOG_START + 122);
        resStep2 = oResource.getResText(RID_WEBWIZARDDIALOG_START + 123);
        resStep3 = oResource.getResText(RID_WEBWIZARDDIALOG_START + 124);
        resStep4 = oResource.getResText(RID_WEBWIZARDDIALOG_START + 125);
        resStep5 = oResource.getResText(RID_WEBWIZARDDIALOG_START + 126);
        resStep6 = oResource.getResText(RID_WEBWIZARDDIALOG_START + 127);
        resStep7 = oResource.getResText(RID_WEBWIZARDDIALOG_START + 128);
        resDelSessionConfirm = oResource.getResText(RID_WEBWIZARDDIALOG_START + 130);
        resPages = oResource.getResText(RID_WEBWIZARDDIALOG_START + 131);
        resSlides = oResource.getResText(RID_WEBWIZARDDIALOG_START + 132);
        resCreatedTemplate = oResource.getResText(RID_WEBWIZARDDIALOG_START + 133);
        resUpdatedTemplate = oResource.getResText(RID_WEBWIZARDDIALOG_START + 134);
        resSessionName = oResource.getResText(RID_WEBWIZARDDIALOG_START + 135);

        reslblBackground = oResource.getResText(RID_WEBWIZARDDIALOG_START + 136);
        reslblIconset = oResource.getResText(RID_WEBWIZARDDIALOG_START + 137);
        reslblIconSetInfo = oResource.getResText(RID_WEBWIZARDDIALOG_START + 138);
        reslblMetaData = oResource.getResText(RID_WEBWIZARDDIALOG_START + 139);
        resBtnChooseBackground = oResource.getResText(RID_WEBWIZARDDIALOG_START + 140);
        resBtnChooseIconset = oResource.getResText(RID_WEBWIZARDDIALOG_START + 141);
        resSessionNameNone = oResource.getResText(RID_WEBWIZARDDIALOG_START + 142);
        reslblFTPDisabled = oResource.getResText(RID_WEBWIZARDDIALOG_START + 143);
        resBackgroundNone = oResource.getResText(RID_WEBWIZARDDIALOG_START + 144);
        resIconsetNone = oResource.getResText(RID_WEBWIZARDDIALOG_START + 145);
        resFTPTargetCouldNotCreate = oResource.getResText(RID_WEBWIZARDDIALOG_START + 146);
        resFTPTargetCreate = oResource.getResText(RID_WEBWIZARDDIALOG_START + 147);
        resZipTargetIsDir = oResource.getResText(RID_WEBWIZARDDIALOG_START + 148);
        resLocalTargetCouldNotCreate = oResource.getResText(RID_WEBWIZARDDIALOG_START + 149);
        resLocalTargetCreate = oResource.getResText(RID_WEBWIZARDDIALOG_START + 150);
        resSizeTemplate = oResource.getResText(RID_WEBWIZARDDIALOG_START + 151);
        resSpecifyNewFileLocation = oResource.getResText(RID_WEBWIZARDDIALOG_START + 152);

        resClose = oResource.getResText(RID_COMMON_START + 17);
        resCancel = oResource.getResText(RID_COMMON_START + 11);
        resOK = oResource.getResText(RID_COMMON_START + 18);
        resHelp = oResource.getResText(RID_COMMON_START + 15);

        try
        {
            prodName = Configuration.getProductName(xmsf);
            resGifFiles = (String) Properties.getPropertyValue(OfficeDocument.getTypeMediaDescriptor(xmsf, "gif_Graphics_Interchange"), "UIName");
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }

    }
}
