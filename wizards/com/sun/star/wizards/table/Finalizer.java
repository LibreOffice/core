/*************************************************************************
*
*  $RCSfile: Finalizer.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: pjunck $ $Date: 2004-10-27 13:38:12 $
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
*  Contributor(s): _______________________________________
*
*/

package com.sun.star.wizards.table;

import com.sun.star.awt.FocusEvent;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.XFocusListener;
import com.sun.star.awt.XRadioButton;
import com.sun.star.awt.XTextComponent;
import com.sun.star.awt.XWindow;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Configuration;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.SystemDialog;
import com.sun.star.wizards.db.DBMetaData;
import com.sun.star.wizards.db.TableDescriptor;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.*;


public class Finalizer implements XFocusListener{
    WizardDialog CurUnoDialog;
    short curtabindex;
    XRadioButton optModifyTable;
    XRadioButton optWorkWithTable;
    XRadioButton optStartFormWizard;
    XTextComponent txtTableName;
    DBMetaData curtabledescriptor;
    public String TOGGLEFINISHBUTTON = "toggleFinishButton";
    public static int WORKWITHTABLEMODE = 0;
    public static int MODIFYTABLEMODE = 1;
    public static int STARTFORMWIZARDMODE = 2;

    public Finalizer(WizardDialog _CurUnoDialog, TableDescriptor _curtabledescriptor) {
        this.CurUnoDialog = _CurUnoDialog;
        this.curtabledescriptor = _curtabledescriptor;
        curtabindex = (short) (TableWizard.SOFINALPAGE * 100);
        Integer IFINALSTEP = new Integer(TableWizard.SOFINALPAGE);
        String slblTableName = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 34);
        String slblProceed = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 36);
        String sWorkWithTable = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 38);
        String sStartFormWizard = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 39);
        String sModifyTable = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 37);
        String sCongratulations  = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 35);
        CurUnoDialog.insertLabel("lblTableName",
          new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "Width"},
          new Object[] { UIConsts.INTEGERS[8], slblTableName, new Integer(97), new Integer(25), IFINALSTEP, new Integer(111)}
        );
        txtTableName = CurUnoDialog.insertTextField("txtTableName", TOGGLEFINISHBUTTON, this,
          new String[] {"Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Text", "Width"},
          new Object[] { UIConsts.INTEGER_12, "HID:41240", new Integer(97), new Integer(35),IFINALSTEP, new Short(curtabindex++),"", new Integer(185)}
        );
        XWindow xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, txtTableName);
        xWindow.addFocusListener(this);

        CurUnoDialog.insertLabel("lblcongratulations",
          new String[] {"Height", "Label", "MultiLine", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
          new Object[] { UIConsts.INTEGERS[8], sCongratulations, Boolean.TRUE, new Integer(97), new Integer(62),IFINALSTEP, new Short(curtabindex++), new Integer(154)}
        );
        CurUnoDialog.insertLabel("lblProceed",
          new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
          new Object[] { UIConsts.INTEGERS[8], slblProceed, new Integer(97), new Integer(82),IFINALSTEP, new Short(curtabindex++), new Integer(154)}
        );
        optWorkWithTable = CurUnoDialog.insertRadioButton("optWorkWithTable", null,
          new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"},
          new Object[] { UIConsts.INTEGERS[8], "HID:41242", sWorkWithTable, new Integer(101), new Integer(97), new Short((short)1),IFINALSTEP, new Short(curtabindex++), new Integer(107)}
        );
        optModifyTable = CurUnoDialog.insertRadioButton("optModifyTable", null,
          new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
          new Object[] { UIConsts.INTEGERS[8], "HID:41241", sModifyTable, new Integer(101), new Integer(109),IFINALSTEP, new Short(curtabindex++), new Integer(107)}
        );
        optStartFormWizard = CurUnoDialog.insertRadioButton("optStartFormWizard", null,
          new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
          new Object[] { UIConsts.INTEGERS[8], "HID:41243", sStartFormWizard, new Integer(101), new Integer(121),IFINALSTEP, new Short(curtabindex++), new Integer(107)}
        );
    }

    public void initialize(String _firsttablename){
        toggleFinishButton();
        setTableName(_firsttablename);
    }

    public int finish(){
        if (optWorkWithTable.getState())
            return WORKWITHTABLEMODE;
        else if (optModifyTable.getState())
            return MODIFYTABLEMODE;
        else
            return STARTFORMWIZARDMODE;
    }

    public void setTableName(String _tablename){
        if (txtTableName.getText().equals("")){
            String tablename = Desktop.getUniqueName(curtabledescriptor.getTableNames(), _tablename);
            txtTableName.setText(tablename);
        }
    }

    public String getTableName(String _firsttablename){
        if (txtTableName.getText().equals(""))
            setTableName(_firsttablename);
        return txtTableName.getText();
    }

    public void toggleFinishButton(){
        CurUnoDialog.enableFinishButton(txtTableName.getText().length() > 0);
    }


    public void focusGained(FocusEvent arg0) {
    }



    public void focusLost(FocusEvent arg0) {
        String tablename = txtTableName.getText();
        int i = Desktop.checkforfirstSpecialCharacter(curtabledescriptor.xMSF, tablename, Configuration.getOfficeLocale(curtabledescriptor.xMSF));
        if (i < tablename.length()){
            //TODO better take the locale of the database
            String sSpecialCharWarning = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 41);
            sSpecialCharWarning = JavaTools.replaceSubString(sSpecialCharWarning, tablename, "%TABLENAME");
            sSpecialCharWarning = JavaTools.replaceSubString(sSpecialCharWarning, tablename.substring(i, i+1), "%SPECIALCHAR");
            CurUnoDialog.showMessageBox("WarningBox", VclWindowPeerAttribute.OK, sSpecialCharWarning);
        }
    }

    public void disposing(EventObject arg0) {
    }

}
