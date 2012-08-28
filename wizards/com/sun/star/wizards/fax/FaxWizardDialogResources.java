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
package com.sun.star.wizards.fax;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Resource;

public class FaxWizardDialogResources extends Resource
{

    final static String UNIT_NAME = "dbwizres";
    final static String MODULE_NAME = "dbw";
    final static int RID_FAXWIZARDDIALOG_START = 3200;
    final static int RID_FAXWIZARDCOMMUNICATION_START = 3270;
    final static int RID_FAXWIZARDGREETING_START = 3280;
    final static int RID_FAXWIZARDSALUTATION_START = 3290;
    final static int RID_FAXWIZARDROADMAP_START = 3300;
    final static int RID_RID_COMMON_START = 500;
    protected String[] RoadmapLabels = new String[7];
    protected String[] SalutationLabels = new String[4];
    protected String[] GreetingLabels = new String[4];
    protected String[] CommunicationLabels = new String[3];
    String resOverwriteWarning;
    String resTemplateDescription;
    String resFaxWizardDialog_title;
    String resLabel9_value;
    String resoptBusinessFax_value;
    String resoptPrivateFax_value;
    String reschkUseLogo_value;
    String reschkUseSubject_value;
    String reschkUseSalutation_value;
    String reschkUseGreeting_value;
    String reschkUseFooter_value;
    String resoptSenderPlaceholder_value;
    String resoptSenderDefine_value;
    String restxtTemplateName_value;
    String resoptCreateFax_value;
    String resoptMakeChanges_value;
    String reslblBusinessStyle_value;
    String reslblPrivateStyle_value;
    String reslblIntroduction_value;
    String reslblSenderAddress_value;
    String reslblSenderName_value;
    String reslblSenderStreet_value;
    String reslblPostCodeCity_value;
    String reslblFooter_value;
    String reslblFinalExplanation1_value;
    String reslblFinalExplanation2_value;
    String reslblTemplateName_value;
    String reslblTemplatePath_value;
    String reslblProceed_value;
    String reslblTitle1_value;
    String reslblTitle3_value;
    String reslblTitle4_value;
    String reslblTitle5_value;
    String reslblTitle6_value;
    String reschkFooterNextPages_value;
    String reschkFooterPageNumbers_value;
    String reschkUseDate_value;
    String reschkUseCommunicationType_value;
    String resLabel1_value;
    String resoptReceiverPlaceholder_value;
    String resoptReceiverDatabase_value;
    String resLabel2_value;

    String resToPlaceHolder = "#to#";
    String resFromPlaceHolder = "#from#";
    String resFaxconstPlaceHolder = "#faxconst#";
    String resTelconstPlaceHolder = "#telconst#";
    String resEmailconstPlaceHolder = "#emailconst#";
    String resConsist1PlaceHolder = "#consist1#";
    String resConsist2PlaceHolder = "#consist2#";
    String resConsist3PlaceHolder = "#consist3#";

    String resToPlaceHolder_value;
    String resFromPlaceHolder_value;
    String resFaxconstPlaceHolder_value;
    String resTelconstPlaceHolder_value;
    String resEmailconstPlaceHolder_value;
    String resConsist1PlaceHolder_value;
    String resConsist2PlaceHolder_value;
    String resConsist3PlaceHolder_value;

    String resPrivateFaxBottle;
    String resPrivateFaxLines;
    String resPrivateFaxMarine;
    String resBusinessFaxClassic;
    String resBusinessFaxClassicPrivate;
    String resBusinessFaxModern;
    String resBusinessFaxModernPrivate;

    public FaxWizardDialogResources(XMultiServiceFactory xmsf)
    {
        super(xmsf, UNIT_NAME, MODULE_NAME);
        /**
         * Delete the String, uncomment the getResText method
         *
         */
        resFaxWizardDialog_title = getResText(RID_FAXWIZARDDIALOG_START + 1);
        resLabel9_value = getResText(RID_FAXWIZARDDIALOG_START + 2);
        resoptBusinessFax_value = getResText(RID_FAXWIZARDDIALOG_START + 3);
        resoptPrivateFax_value = getResText(RID_FAXWIZARDDIALOG_START + 4);
        reschkUseLogo_value = getResText(RID_FAXWIZARDDIALOG_START + 5);
        reschkUseSubject_value = getResText(RID_FAXWIZARDDIALOG_START + 6);
        reschkUseSalutation_value = getResText(RID_FAXWIZARDDIALOG_START + 7);
        reschkUseGreeting_value = getResText(RID_FAXWIZARDDIALOG_START + 8);
        reschkUseFooter_value = getResText(RID_FAXWIZARDDIALOG_START + 9);
        resoptSenderPlaceholder_value = getResText(RID_FAXWIZARDDIALOG_START + 10);
        resoptSenderDefine_value = getResText(RID_FAXWIZARDDIALOG_START + 11);
        restxtTemplateName_value = getResText(RID_FAXWIZARDDIALOG_START + 12);
        resoptCreateFax_value = getResText(RID_FAXWIZARDDIALOG_START + 13);
        resoptMakeChanges_value = getResText(RID_FAXWIZARDDIALOG_START + 14);
        reslblBusinessStyle_value = getResText(RID_FAXWIZARDDIALOG_START + 15);
        reslblPrivateStyle_value = getResText(RID_FAXWIZARDDIALOG_START + 16);
        reslblIntroduction_value = getResText(RID_FAXWIZARDDIALOG_START + 17);
        reslblSenderAddress_value = getResText(RID_FAXWIZARDDIALOG_START + 18);
        reslblSenderName_value = getResText(RID_FAXWIZARDDIALOG_START + 19);
        reslblSenderStreet_value = getResText(RID_FAXWIZARDDIALOG_START + 20);
        reslblPostCodeCity_value = getResText(RID_FAXWIZARDDIALOG_START + 21);
        reslblFooter_value = getResText(RID_FAXWIZARDDIALOG_START + 22);
        reslblFinalExplanation1_value = getResText(RID_FAXWIZARDDIALOG_START + 23);
        reslblFinalExplanation2_value = getResText(RID_FAXWIZARDDIALOG_START + 24);
        reslblTemplateName_value = getResText(RID_FAXWIZARDDIALOG_START + 25);
        reslblTemplatePath_value = getResText(RID_FAXWIZARDDIALOG_START + 26);
        reslblProceed_value = getResText(RID_FAXWIZARDDIALOG_START + 27);
        reslblTitle1_value = getResText(RID_FAXWIZARDDIALOG_START + 28);
        reslblTitle3_value = getResText(RID_FAXWIZARDDIALOG_START + 29);
        reslblTitle4_value = getResText(RID_FAXWIZARDDIALOG_START + 30);
        reslblTitle5_value = getResText(RID_FAXWIZARDDIALOG_START + 31);
        reslblTitle6_value = getResText(RID_FAXWIZARDDIALOG_START + 32);
        reschkFooterNextPages_value = getResText(RID_FAXWIZARDDIALOG_START + 33);
        reschkFooterPageNumbers_value = getResText(RID_FAXWIZARDDIALOG_START + 34);
        reschkUseDate_value = getResText(RID_FAXWIZARDDIALOG_START + 35);
        reschkUseCommunicationType_value = getResText(RID_FAXWIZARDDIALOG_START + 36);
        resLabel1_value = getResText(RID_FAXWIZARDDIALOG_START + 37);
        resoptReceiverPlaceholder_value = getResText(RID_FAXWIZARDDIALOG_START + 38);
        resoptReceiverDatabase_value = getResText(RID_FAXWIZARDDIALOG_START + 39);
        resLabel2_value = getResText(RID_FAXWIZARDDIALOG_START + 40);

        resToPlaceHolder_value = getResText(RID_FAXWIZARDDIALOG_START + 41);
        resFromPlaceHolder_value = getResText(RID_FAXWIZARDDIALOG_START + 42);
        resFaxconstPlaceHolder_value = getResText(RID_FAXWIZARDDIALOG_START + 43);
        resTelconstPlaceHolder_value = getResText(RID_FAXWIZARDDIALOG_START + 44);
        resEmailconstPlaceHolder_value = getResText(RID_FAXWIZARDDIALOG_START + 45);
        resConsist1PlaceHolder_value = getResText(RID_FAXWIZARDDIALOG_START + 46);
        resConsist2PlaceHolder_value = getResText(RID_FAXWIZARDDIALOG_START + 47);
        resConsist3PlaceHolder_value = getResText(RID_FAXWIZARDDIALOG_START + 48);

        resPrivateFaxBottle = getResText(RID_FAXWIZARDDIALOG_START + 49);
        resPrivateFaxLines = getResText(RID_FAXWIZARDDIALOG_START + 50);
        resPrivateFaxMarine = getResText(RID_FAXWIZARDDIALOG_START + 51);
        resBusinessFaxClassic = getResText(RID_FAXWIZARDDIALOG_START + 52);
        resBusinessFaxClassicPrivate = getResText(RID_FAXWIZARDDIALOG_START + 53);
        resBusinessFaxModern = getResText(RID_FAXWIZARDDIALOG_START + 54);
        resBusinessFaxModernPrivate = getResText(RID_FAXWIZARDDIALOG_START + 55);

        loadRoadmapResources();
        loadSalutationResources();
        loadGreetingResources();
        loadCommunicationResources();
        loadCommonResources();
    }

    private void loadCommonResources()
    {
        resOverwriteWarning = getResText(RID_RID_COMMON_START + 19);
        resTemplateDescription = getResText(RID_RID_COMMON_START + 20);
    }

    private void loadRoadmapResources()
    {
        for (int i = 1; i < 6; i++)
        {
            RoadmapLabels[i] = getResText(RID_FAXWIZARDROADMAP_START + i);
        }
    }

    private void loadSalutationResources()
    {
        for (int i = 1; i < 5; i++)
        {
            SalutationLabels[i - 1] = getResText(RID_FAXWIZARDSALUTATION_START + i);
        }
    }

    private void loadGreetingResources()
    {
        for (int i = 1; i < 5; i++)
        {
            GreetingLabels[i - 1] = getResText(RID_FAXWIZARDGREETING_START + i);
        }
    }

    private void loadCommunicationResources()
    {
        for (int i = 1; i < 4; i++)
        {
            CommunicationLabels[i - 1] = getResText(RID_FAXWIZARDCOMMUNICATION_START + i);
        }
    }
}
