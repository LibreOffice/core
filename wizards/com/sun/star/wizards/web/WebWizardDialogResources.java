package com.sun.star.wizards.web;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Configuration;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.common.Resource;
import com.sun.star.wizards.document.OfficeDocument;

public class WebWizardDialogResources extends Resource {

    String resSessionName;
    String resCreatedTemplate;
    String resUpdatedTemplate;
    String resSlides;
    String resDelSessionConfirm;
    String resErrProxies;
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
    String resbtnLoadSession_value;
    String resbtnDelSession_value;
    String reslbIntroTitle_value;
    String reslblContentTitle_value;
    String reslblDocuments_value;
    String reslnDocsInfo_value;
    String reslblDocTitle_value;
    String reslblDocInfo_value;
    String reslblDocAuthor_value;
    String reslblDocExportFormat_value;
    String reslblTitleGeneralPage_value;
    String reslblSiteTitle_value;
    String reslblFavIcon_value;
    String reslblSiteDesc_value;
    String reslblSiteKeywords_value;
    String reslblSiteCreated_value;
    String reslblRevisit_value;
    String reslblEmail_value;
    String reslblCopyright_value;
    String reslblSiteUpdated_value;
    String reslblRevisitDays_value;
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
    String resbtnBackgrounds_value;
    String resbtnIconSets_value;
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
    String resbtnFavIcon_value;
    String resoptOptimize640x480_value;
    String resoptOptimize800x600_value;
    String resoptOptimize1024x768_value;

    final String resbtnDocUp_value = String.valueOf((char) 8743);
    final String resbtnDocDown_value = String.valueOf((char) 8744);

    //String resGenerate;
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

    String resSODocs;
    String resMSDocs;
    String resImages;
    String resAllFiles;
    String resZipFiles;
    String resIconFiles;
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

    String prodName;

    public WebWizardDialogResources(XMultiServiceFactory xmsf) {
        super(xmsf, UNIT_NAME, MODULE_NAME);

        /**
         * Delete the String, uncomment the getResText method
         *
         */
        resWebWizardDialog_title        = getResText(RID_WEBWIZARDDIALOG_START + 1);
        reslblIntroduction_value        = getResText(RID_WEBWIZARDDIALOG_START + 2);
        resbtnLoadSession_value         = getResText(RID_WEBWIZARDDIALOG_START + 3);
        resbtnDelSession_value          = getResText(RID_WEBWIZARDDIALOG_START + 4);
        reslbIntroTitle_value           = getResText(RID_WEBWIZARDDIALOG_START + 5);
        reslblContentTitle_value        = getResText(RID_WEBWIZARDDIALOG_START + 6);
        reslblDocuments_value           = getResText(RID_WEBWIZARDDIALOG_START + 7);
        reslnDocsInfo_value             = getResText(RID_WEBWIZARDDIALOG_START + 8);
        reslblDocTitle_value            = getResText(RID_WEBWIZARDDIALOG_START + 9);
        reslblDocInfo_value             = getResText(RID_WEBWIZARDDIALOG_START + 10);
        reslblDocAuthor_value           = getResText(RID_WEBWIZARDDIALOG_START + 11);
        reslblDocExportFormat_value     = getResText(RID_WEBWIZARDDIALOG_START + 12);
        reslblTitleGeneralPage_value    = getResText(RID_WEBWIZARDDIALOG_START + 13);
        reslblSiteTitle_value           = getResText(RID_WEBWIZARDDIALOG_START + 14);
        reslblFavIcon_value             = getResText(RID_WEBWIZARDDIALOG_START + 15);
        reslblSiteDesc_value            = getResText(RID_WEBWIZARDDIALOG_START + 16);
        reslblSiteKeywords_value        = getResText(RID_WEBWIZARDDIALOG_START + 17);
        reslblSiteCreated_value         = getResText(RID_WEBWIZARDDIALOG_START + 18);
        reslblRevisit_value             = getResText(RID_WEBWIZARDDIALOG_START + 19);
        reslblEmail_value               = getResText(RID_WEBWIZARDDIALOG_START + 20);
        reslblCopyright_value           = getResText(RID_WEBWIZARDDIALOG_START + 21);
        reslblSiteUpdated_value         = getResText(RID_WEBWIZARDDIALOG_START + 22);
        reslblRevisitDays_value         = getResText(RID_WEBWIZARDDIALOG_START + 23);
        resbtnPreview_value             = getResText(RID_WEBWIZARDDIALOG_START + 24);
        reslblTitlePublish_value        = getResText(RID_WEBWIZARDDIALOG_START + 25);
        reslblCreateSite_value          = getResText(RID_WEBWIZARDDIALOG_START + 26);
        reschkLocalDir_value            = getResText(RID_WEBWIZARDDIALOG_START + 27);
        resbtnLocalDir_value            = getResText(RID_WEBWIZARDDIALOG_START + 28);
        reschkFTP_value                 = getResText(RID_WEBWIZARDDIALOG_START + 29);
        //
        resbtnFTP_value                 = getResText(RID_WEBWIZARDDIALOG_START + 31);
        reschkZip_value                 = getResText(RID_WEBWIZARDDIALOG_START + 32);
        resbtnZip_value                 = getResText(RID_WEBWIZARDDIALOG_START + 33);
        reschkSaveSettings_value        = getResText(RID_WEBWIZARDDIALOG_START + 34);
        reslblSaveSettings_value        = getResText(RID_WEBWIZARDDIALOG_START + 35);
        reslblLoadSettings_value        = getResText(RID_WEBWIZARDDIALOG_START + 36);
        reslblSiteContent_value         = getResText(RID_WEBWIZARDDIALOG_START + 37);
        resbtnAddDoc_value              = getResText(RID_WEBWIZARDDIALOG_START + 38);
        resbtnRemoveDoc_value           = getResText(RID_WEBWIZARDDIALOG_START + 39);
        reslblLayoutTitle_value         = getResText(RID_WEBWIZARDDIALOG_START + 40);
        reslblStyleTitle_value          = getResText(RID_WEBWIZARDDIALOG_START + 41);
        reslblStyle_value               = getResText(RID_WEBWIZARDDIALOG_START + 42);
        resbtnBackgrounds_value         = getResText(RID_WEBWIZARDDIALOG_START + 43);
        resbtnIconSets_value            = getResText(RID_WEBWIZARDDIALOG_START + 44);
        //
        reslblLayouts_value             = getResText(RID_WEBWIZARDDIALOG_START + 48);
        //
        reschbDocDesc_value             = getResText(RID_WEBWIZARDDIALOG_START + 50);
        reschbDocAuthor_value           = getResText(RID_WEBWIZARDDIALOG_START + 51);
        reschkDocCreated_value          = getResText(RID_WEBWIZARDDIALOG_START + 52);
        reschkDocChanged_value          = getResText(RID_WEBWIZARDDIALOG_START + 53);
        reschkDocFilename_value         = getResText(RID_WEBWIZARDDIALOG_START + 54);
        reschkDocFormat_value           = getResText(RID_WEBWIZARDDIALOG_START + 55);
        reschkDocFormatIcon_value       = getResText(RID_WEBWIZARDDIALOG_START + 56);
        reschkDocPages_value            = getResText(RID_WEBWIZARDDIALOG_START + 57);
        reschkDocSize_value             = getResText(RID_WEBWIZARDDIALOG_START + 58);
        resFixedLine1_value             = getResText(RID_WEBWIZARDDIALOG_START + 59);
        reslblLayout2Title_value        = getResText(RID_WEBWIZARDDIALOG_START + 60);
        reslblDisplay_value             = getResText(RID_WEBWIZARDDIALOG_START + 61);
        reslblOptimizeFor_value         = getResText(RID_WEBWIZARDDIALOG_START + 62);
        resbtnFavIcon_value             = getResText(RID_WEBWIZARDDIALOG_START + 63);
        resoptOptimize640x480_value     = getResText(RID_WEBWIZARDDIALOG_START + 64);
        resoptOptimize800x600_value     = getResText(RID_WEBWIZARDDIALOG_START + 65);
        resoptOptimize1024x768_value    = getResText(RID_WEBWIZARDDIALOG_START + 66);
        //resExport                         = getResText(RID_WEBWIZARDDIALOG_START + 67);
        //resGenerate                   = getResText(RID_WEBWIZARDDIALOG_START + 68);
        //resPublish                        = getResText(RID_WEBWIZARDDIALOG_START + 69);
        resStatusDialogTitle            = getResText(RID_WEBWIZARDDIALOG_START + 70);
        resCounter                      = getResText(RID_WEBWIZARDDIALOG_START + 71);
        resPublishDir                   = getResText(RID_WEBWIZARDDIALOG_START + 72);
        resFTPTargetExistsAsfile        = getResText(RID_WEBWIZARDDIALOG_START + 73);
        resLocalTargetExistsAsfile      = getResText(RID_WEBWIZARDDIALOG_START + 74);
        resZipTargetExists              = getResText(RID_WEBWIZARDDIALOG_START + 75);
        resFTPTargetNotEmpty            = getResText(RID_WEBWIZARDDIALOG_START + 76);
        resLocalTragetNotEmpty          = getResText(RID_WEBWIZARDDIALOG_START + 77);
        resSessionExists                = getResText(RID_WEBWIZARDDIALOG_START + 78);
        resTaskExportDocs               = getResText(RID_WEBWIZARDDIALOG_START + 79);
        resTaskExportPrepare            = getResText(RID_WEBWIZARDDIALOG_START + 80);
        resTaskGenerateCopy             = getResText(RID_WEBWIZARDDIALOG_START + 81);
        resTaskGeneratePrepare          = getResText(RID_WEBWIZARDDIALOG_START + 82);
        resTaskGenerateXsl              = getResText(RID_WEBWIZARDDIALOG_START + 83);
        resTaskPrepare                  = getResText(RID_WEBWIZARDDIALOG_START + 84);
        resTaskPublishPrepare           = getResText(RID_WEBWIZARDDIALOG_START + 86);
        resTaskPublishLocal             = getResText(RID_WEBWIZARDDIALOG_START + 87);
        resTaskPublishFTP               = getResText(RID_WEBWIZARDDIALOG_START + 88);
        resTaskPublishZip               = getResText(RID_WEBWIZARDDIALOG_START + 89);
        resTaskFinish                   = getResText(RID_WEBWIZARDDIALOG_START + 90);
        //resPages                      = getResText(RID_WEBWIZARDDIALOG_START + 91);
        resSODocs                       = getResText(RID_WEBWIZARDDIALOG_START + 92);
        resMSDocs                       = getResText(RID_WEBWIZARDDIALOG_START + 93);
        resImages                       = getResText(RID_WEBWIZARDDIALOG_START + 94);
        resAllFiles                     = getResText(RID_WEBWIZARDDIALOG_START + 95);
        resZipFiles                     = getResText(RID_WEBWIZARDDIALOG_START + 96);
        resIconFiles                    = getResText(RID_WEBWIZARDDIALOG_START + 97);
        resBackgroundsDialog            = getResText(RID_WEBWIZARDDIALOG_START + 98);
        resBackgroundsDialogCaption     = getResText(RID_WEBWIZARDDIALOG_START + 99);
        resIconsDialog                  = getResText(RID_WEBWIZARDDIALOG_START + 100);
        resIconsDialogCaption           = getResText(RID_WEBWIZARDDIALOG_START + 101);
        resOther                        = getResText(RID_WEBWIZARDDIALOG_START + 102);
        resDeselect                     = getResText(RID_WEBWIZARDDIALOG_START + 103);
        resFinishedSuccess              = getResText(RID_WEBWIZARDDIALOG_START + 104);
        resFinishedNoSuccess            = getResText(RID_WEBWIZARDDIALOG_START + 105);
        resErrTOC                       = getResText(RID_WEBWIZARDDIALOG_START + 106);
        resErrTOCMedia                  = getResText(RID_WEBWIZARDDIALOG_START + 107);
        resErrDocInfo                   = getResText(RID_WEBWIZARDDIALOG_START + 108);
        resErrDocExport                 = getResText(RID_WEBWIZARDDIALOG_START + 109);
        resErrMkDir                     = getResText(RID_WEBWIZARDDIALOG_START + 110);
        resErrSecurity                  = getResText(RID_WEBWIZARDDIALOG_START + 111);
        resErrExportIO                  = getResText(RID_WEBWIZARDDIALOG_START + 112);
        resErrPublishMedia              = getResText(RID_WEBWIZARDDIALOG_START + 113);
        resErrPublish                   = getResText(RID_WEBWIZARDDIALOG_START + 114);
        resErrUnknown                   = getResText(RID_WEBWIZARDDIALOG_START + 115);
        resErrDocValidate               = getResText(RID_WEBWIZARDDIALOG_START + 116);
        resErrIsDirectory               = getResText(RID_WEBWIZARDDIALOG_START + 117);
        resLoadingSession               = getResText(RID_WEBWIZARDDIALOG_START + 118);
        resErrUnexpected                = getResText(RID_WEBWIZARDDIALOG_START + 119);
        resValidatingDocuments          = getResText(RID_WEBWIZARDDIALOG_START + 120);
        resDefaultArchiveFilename       = getResText(RID_WEBWIZARDDIALOG_START + 121);
        resStep1                        = getResText(RID_WEBWIZARDDIALOG_START + 122);
        resStep2                        = getResText(RID_WEBWIZARDDIALOG_START + 123);
        resStep3                        = getResText(RID_WEBWIZARDDIALOG_START + 124);
        resStep4                        = getResText(RID_WEBWIZARDDIALOG_START + 125);
        resStep5                        = getResText(RID_WEBWIZARDDIALOG_START + 126);
        resStep6                        = getResText(RID_WEBWIZARDDIALOG_START + 127);
        resStep7                        = getResText(RID_WEBWIZARDDIALOG_START + 128);
        resErrProxies                   = getResText(RID_WEBWIZARDDIALOG_START + 129);
        resDelSessionConfirm            = getResText(RID_WEBWIZARDDIALOG_START + 130);
        resPages                        = getResText(RID_WEBWIZARDDIALOG_START + 131);
        resSlides                       = getResText(RID_WEBWIZARDDIALOG_START + 132);
        resCreatedTemplate              = getResText(RID_WEBWIZARDDIALOG_START + 133);
        resUpdatedTemplate              = getResText(RID_WEBWIZARDDIALOG_START + 134);
        resSessionName                  = getResText(RID_WEBWIZARDDIALOG_START + 135);

        resClose                        = getResText(RID_COMMON_START + 17);
        resCancel                       = getResText(RID_COMMON_START + 11);
        resOK                           = getResText(RID_COMMON_START + 18);
        resHelp                         = getResText(RID_COMMON_START + 15);

        try {
            prodName        = Configuration.getProductName(xmsf);
            resSODocs       = JavaTools.replaceSubString(resSODocs, prodName, "%PRODNAME");
            resGifFiles     = (String) Properties.getPropertyValue(OfficeDocument.getTypeMediaDescriptor(xmsf, "gif_Graphics_Interchange"), "UIName");
        } catch (Exception ex) {
            ex.printStackTrace();
        }

    }
}
