/*
 ************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CGLetter.java,v $
 *
 * $Revision: 1.4.192.1 $
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
