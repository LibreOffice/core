package com.sun.star.wizards.fax;

import com.sun.star.wizards.common.ConfigGroup;

public class CGFaxWizard extends ConfigGroup {

    public int cp_FaxType;
    public CGFax cp_BusinessFax = new CGFax();
    public CGFax cp_PrivateFax = new CGFax();

}
