package com.sun.star.wizards.letter;

import com.sun.star.wizards.common.ConfigGroup;

public class CGLetter extends ConfigGroup {

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
    public int cp_Salutation;
    public int cp_Greeting;
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
