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
    public int cp_Norm;
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
