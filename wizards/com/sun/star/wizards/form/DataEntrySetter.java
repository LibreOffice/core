/*************************************************************************
 *
 *  $RCSfile: DataEntrySetter.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pjunck $  $Date: 2004-10-27 13:33:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Berend Cornelius
 *
 */

package com.sun.star.wizards.form;
import com.sun.star.awt.XCheckBox;
import com.sun.star.awt.XRadioButton;
import com.sun.star.beans.PropertyValue;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.ui.UnoDialog;
import com.sun.star.wizards.ui.WizardDialog;
import com.sun.star.wizards.ui.UIConsts;


public class DataEntrySetter{
    WizardDialog CurUnoDialog;
    short curtabindex;

    XRadioButton optNewDataOnly;
    XRadioButton optDisplayAllData;
    XCheckBox chknomodification;
    XCheckBox chknodeletion;
    XCheckBox chknoaddition;

    public DataEntrySetter(WizardDialog _CurUnoDialog) {
        this.CurUnoDialog = _CurUnoDialog;
        curtabindex = (short) (FormWizard.SODATAPAGE * 100);
        Integer IDataStep = new Integer(FormWizard.SODATAPAGE);
        String sNewDataOnly = CurUnoDialog.oResource.getResText(UIConsts.RID_FORM + 44);    //
        String sDisplayAllData = CurUnoDialog.oResource.getResText(UIConsts.RID_FORM + 46); //
        String sNoModification = CurUnoDialog.oResource.getResText(UIConsts.RID_FORM + 47); // AllowUpdates
        String sNoDeletion = CurUnoDialog.oResource.getResText(UIConsts.RID_FORM + 48);     // AllowDeletes
        String sNoAddition = CurUnoDialog.oResource.getResText(UIConsts.RID_FORM + 49);     // AlowInserts
        String sdontdisplayExistingData = CurUnoDialog.oResource.getResText(UIConsts.RID_FORM + 45);

      optNewDataOnly = CurUnoDialog.insertRadioButton("optNewDataOnly", "toggleCheckBoxes", this,
        new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
        new Object[] {UIConsts.INTEGERS[8], "HID:34461", sNewDataOnly, new Integer(98), new Integer(25),IDataStep, new Short(curtabindex++), new Integer(195)}
      );

      optDisplayAllData = CurUnoDialog.insertRadioButton("optDisplayAllData", "toggleCheckBoxes", this,
        new String[] {"Height", "HelpURL",  "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"},
        new Object[] {UIConsts.INTEGERS[8],"HID:34462", sDisplayAllData, new Integer(98), new Integer(50), new Short((short)1),IDataStep, new Short(curtabindex++), new Integer(197)}
      );
      chknomodification = CurUnoDialog.insertCheckBox("chknomodification", null,
        new String[] {"Height", "HelpURL",  "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"},
        new Object[] {UIConsts.INTEGERS[8], "HID:34463", sNoModification, new Integer(103), new Integer(62), new Short((short)0),IDataStep, new Short(curtabindex++), new Integer(189)}
      );
      chknodeletion = CurUnoDialog.insertCheckBox("chknodeletion", null,
        new String[] {"Height", "HelpURL",  "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"},
        new Object[] {UIConsts.INTEGERS[8], "HID:34464", sNoDeletion, new Integer(103), new Integer(74), new Short((short)0),IDataStep, new Short(curtabindex++), new Integer(189)}
      );
      chknoaddition = CurUnoDialog.insertCheckBox("chknoaddition", null,
        new String[] {"Height", "HelpURL",  "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"},
        new Object[] {UIConsts.INTEGERS[8], "HID:34465", sNoAddition, new Integer(103), new Integer(86), new Short((short)0),IDataStep, new Short(curtabindex++), new Integer(191)}
      );
      CurUnoDialog.insertLabel("lbldontdisplayExistingData",
        new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
        new Object[] { new Integer(8), sdontdisplayExistingData, new Integer(107), new Integer(33),IDataStep, new Short(curtabindex++), new Integer(134)}
      );
    }


    public PropertyValue[] getFormProperties(){
        PropertyValue[] retProperties;
        if (optDisplayAllData.getState()){
            retProperties = new PropertyValue[3];
            boolean bAllowUpdates = (((Short) Helper.getUnoPropertyValue(UnoDialog.getModel(chknomodification), "State")).shortValue()) != 1;
            boolean bAllowDeletes = (((Short) Helper.getUnoPropertyValue(UnoDialog.getModel(chknodeletion), "State")).shortValue()) != 1;
            boolean bAllowInserts = (((Short) Helper.getUnoPropertyValue(UnoDialog.getModel(chknoaddition), "State")).shortValue()) != 1;
            retProperties[0] = Properties.createProperty("AllowUpdates", new Boolean(bAllowUpdates));
            retProperties[1] = Properties.createProperty("AllowDeletes", new Boolean(bAllowDeletes));
            retProperties[2] = Properties.createProperty("AllowInserts", new Boolean(bAllowInserts));
        }
        else{
            retProperties = new PropertyValue[1];
            retProperties[0] = Properties.createProperty("IgnoreResult", new Boolean(true));
        }
        return retProperties;

    }


    public void toggleCheckBoxes(){
        boolean bdisplayalldata = optDisplayAllData.getState();
        Helper.setUnoPropertyValue(UnoDialog.getModel(chknomodification), "Enabled", new Boolean(bdisplayalldata));
        Helper.setUnoPropertyValue(UnoDialog.getModel(chknodeletion), "Enabled", new Boolean(bdisplayalldata));
        Helper.setUnoPropertyValue(UnoDialog.getModel(chknoaddition), "Enabled", new Boolean(bdisplayalldata));
    }


}
