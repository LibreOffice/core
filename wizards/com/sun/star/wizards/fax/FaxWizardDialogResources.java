/*
 ************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FaxWizardDialogResources.java,v $
 *
 * $Revision: 1.3.192.1 $
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