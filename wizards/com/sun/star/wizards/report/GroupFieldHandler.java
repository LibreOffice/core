/*************************************************************************
 *
 *  $RCSfile: GroupFieldHandler.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $Date: 2004/04/23 13:04:32 $
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

package com.sun.star.wizards.report;

import com.sun.star.wizards.ui.*;
import com.sun.star.wizards.db.*;

import java.util.Vector;


public class GroupFieldHandler extends FieldSelection{
ReportDocument CurReportDocument;
Vector GroupFieldVector = new Vector();
QueryMetaData CurDBMetaData;
WizardDialog oWizardDialog;

    public GroupFieldHandler(ReportDocument _CurReportDocument, WizardDialog _CurUnoDialog){
    super(_CurUnoDialog, ReportWizard.SOGROUPPAGE, 95, 27, 210, 127,
            _CurUnoDialog.oResource.getResText(UIConsts.RID_REPORT + 19),
            _CurUnoDialog.oResource.getResText(UIConsts.RID_REPORT + 50), 34340, false);

    try{
        this.oWizardDialog = _CurUnoDialog;
        this.CurReportDocument = _CurReportDocument;
        this.CurDBMetaData = CurReportDocument.CurDBMetaData;
        CurUnoDialog.setControlProperty("lstFields_2", "MultiSelection", new Boolean(false));
        CurUnoDialog.setControlProperty("lstSelFields_2", "MultiSelection", new Boolean(false));
        addFieldSelectionListener(new FieldSelectionListener());
        CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblBlindTextNote_1",
            new String[] {"Enabled", "Height", "Label", "MultiLine", "PositionX", "PositionY", "Step", "Width"},
            new Object[] {new Boolean(false), new Integer(18), ReportWizard.sBlindTextNote, new Boolean(true), new Integer(95), new Integer(158), new Integer(ReportWizard.SOGROUPPAGE), new Integer(209)});
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public void initialize(){
    try{
        CurDBMetaData.setfieldtitles();
        Vector NormalFieldsVector = new Vector();
        Vector SelFieldsVector = new Vector();
        for (int i = 0; i < CurDBMetaData.FieldNames.length; i++){
            String sfieldtitle = CurDBMetaData.getFieldTitle(CurDBMetaData.FieldNames[i]);
            if (CurReportDocument.isGroupField(CurDBMetaData.FieldNames[i]))
                SelFieldsVector.add(sfieldtitle);
            else
                NormalFieldsVector.add(sfieldtitle);
        }
        String[] SelFields = new String[SelFieldsVector.size()];
        SelFieldsVector.toArray(SelFields);
        String[] NormalFields = new String[NormalFieldsVector.size()];
        NormalFieldsVector.toArray(NormalFields);
        super.initialize(NormalFields, SelFields, true);
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}


    public void removeGroupFieldNames(){
        emptyFieldsListBoxes();
        GroupFieldVector.removeAllElements();
        CurUnoDialog.setControlProperty("lblBlindTextNote_1", "Enabled", new Boolean(false));
    }

    public void getGroupFieldNames(CommandMetaData CurDBMetaData){
        String[] GroupFieldNames = new String[GroupFieldVector.size()];
        GroupFieldVector.copyInto(GroupFieldNames);
        CurDBMetaData.GroupFieldNames = GroupFieldNames;
    }


    protected class FieldSelectionListener implements com.sun.star.wizards.ui.XFieldSelectionListener{

        public void moveItemDown(String Selitem){
            CurReportDocument.refreshGroupFields(xSelFieldsListBox.getItems());
        }

        public void moveItemUp(String item){
            CurReportDocument.refreshGroupFields(xSelFieldsListBox.getItems());
        }

        public void shiftFromLeftToRight(String[] Selitems, String[] Newitems) {
            String CurGroupTitle = Selitems[0];
            int iSelCount = xSelFieldsListBox.getItemCount();
            String[] CurGroupNames = xFieldsListBox.getItems();
            CurReportDocument.addGroupNametoDocument(CurGroupNames, CurGroupTitle, GroupFieldVector, ReportWizard.ReportPath, iSelCount);
            CurUnoDialog.setControlProperty("lblBlindTextNote_1", "Enabled", new Boolean(true));
            oWizardDialog.setStepEnabled(ReportWizard.SOSORTPAGE, (xFieldsListBox.getItemCount() > 0));
        }

        public void shiftFromRightToLeft(String[] OldSelitems, String[] Newitems){
            int iSelPos = OldSelitems.length;
            if (iSelPos > 0){
                String OldGroupTitle = OldSelitems[0];
                String[] NewSelList = xSelFieldsListBox.getItems();
                CurReportDocument.removeGroupName(NewSelList, OldGroupTitle, GroupFieldVector);
                String[] NewSelGroupNames = xSelFieldsListBox.getItems();
                CurUnoDialog.setControlProperty("lblBlindTextNote_1", "Enabled", new Boolean(NewSelGroupNames.length == 0));
                oWizardDialog.setStepEnabled(ReportWizard.SOSORTPAGE, (xFieldsListBox.getItemCount() > 0));
            }
        }

        public int getID(){
            return 2;
        }

        public void setID(String sIncSuffix){
        }

    }
}
