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
package com.sun.star.wizards.letter;

import com.sun.star.wizards.common.ConfigGroup;

public class CGLetter extends ConfigGroup
{

    public int cp_Style;
    public boolean cp_BusinessPaper;
    public CGPaperElementLocation cp_CompanyLogo = new CGPaperElementLocation();
    public CGPaperElementLocation cp_CompanyAddress = new CGPaperElementLocation();
    public boolean cp_PaperCompanyAddressReceiverField;
    public boolean cp_PaperFooter;
    public double cp_PaperFooterHeight;
    public boolean cp_PrintCompanyLogo;
    public boolean cp_PrintCompanyAddressReceiverField;
    public boolean cp_PrintLetterSigns;
    public boolean cp_PrintSubjectLine;
    public boolean cp_PrintSalutation;
    public boolean cp_PrintBendMarks;
    public boolean cp_PrintGreeting;
    public boolean cp_PrintFooter;
    public String cp_Salutation;
    public String cp_Greeting;
    public int cp_SenderAddressType;
    public String cp_SenderCompanyName;
    public String cp_SenderStreet;
    public String cp_SenderPostCode;
    public String cp_SenderState;
    public String cp_SenderCity;
    public int cp_ReceiverAddressType;
    public String cp_Footer;
    public boolean cp_FooterOnlySecondPage;
    public boolean cp_FooterPageNumbers;
    public int cp_CreationType;
    public String cp_TemplateName;
    public String cp_TemplatePath;
}
