/*************************************************************************
*
*  $RCSfile: ScenarioSelector.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: pjunck $ $Date: 2004-10-27 13:38:48 $
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

import java.util.Hashtable;

import com.sun.star.awt.ItemEvent;
import com.sun.star.awt.XButton;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XItemListener;
import com.sun.star.awt.XListBox;
import com.sun.star.awt.XRadioButton;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.EventObject;
//import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Configuration;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.db.TableDescriptor;
import com.sun.star.wizards.ui.FieldSelection;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.UnoDialog;
import com.sun.star.wizards.ui.WizardDialog;
import com.sun.star.wizards.ui.XFieldSelectionListener;

/**
 * @author Administrator
 *
 * To change the template for this generated type comment go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
public class ScenarioSelector extends FieldSelection implements XItemListener, XFieldSelectionListener{

    XFixedText lblExplanation;
    XFixedText lblCategories;
    XRadioButton optBusiness;
    XRadioButton optPrivate;
    XListBox xTableListBox;
    TableWizard CurUnoDialog;
    TableDescriptor curtabledescriptor;
    CGCategory oCGCategory;
    CGTable oCGTable;
    String SELECTCATEGORY = "selectCategory";
    private int curcategory;
    public boolean bcolumnnameislimited;
    private String[] fieldnames;
    String smytable;

    /**
     *
     */
    public ScenarioSelector(TableWizard _CurUnoDialog, TableDescriptor _curtabledescriptor, String _reslblFields, String _reslblSelFields) {
        super(_CurUnoDialog, TableWizard.SOMAINPAGE, 95, 98, 210, 80, _reslblFields, _reslblSelFields, 41209, true );
        CurUnoDialog = (TableWizard) _CurUnoDialog;
        curtabledescriptor = _curtabledescriptor;
        int imaxcolumnchars = this.curtabledescriptor.getMaxColumnNameLength();
        bcolumnnameislimited = (imaxcolumnchars > 0) && (imaxcolumnchars < 16);
        addFieldSelectionListener( this);
        short pretabindex = (short) (50);
        String sExplanation = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 14);
        String sCategories = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 15);
        String sBusiness  = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 16);
        String sPrivate  = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 17);
        String sTableNames = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 18);
        smytable = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 44);
        Integer IMAINSTEP = new Integer(TableWizard.SOMAINPAGE);
        oCGCategory = new CGCategory(CurUnoDialog.xMSF);
        oCGTable = new CGTable(CurUnoDialog.xMSF);
        lblExplanation = CurUnoDialog.insertLabel("lblScenarioExplanation",
          new String[] {"Height", "Label", "MultiLine", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
          new Object[] {new Integer(20), sExplanation, Boolean.TRUE, new Integer(95), new Integer(27), IMAINSTEP, new Short(pretabindex++), new Integer(209)}
        );

        lblCategories = CurUnoDialog.insertLabel("lblCategories",
          new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
          new Object[] {new Integer(8), sCategories, new Integer(95), new Integer(47), IMAINSTEP, new Short(pretabindex++), new Integer(100)}
        );

        optBusiness = CurUnoDialog.insertRadioButton("optBusiness", SELECTCATEGORY, this,
            new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"},
            new Object[] { UIConsts.INTEGERS[8], "HID:41205", sBusiness, new Integer(98), new Integer(57), new Short((short)1),IMAINSTEP, new Short(pretabindex++), new Integer(78)}
        );

        optPrivate = CurUnoDialog.insertRadioButton("optPrivate", SELECTCATEGORY, this,
            new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
            new Object[] { UIConsts.INTEGERS[8], "HID:41206", sPrivate, new Integer(182), new Integer(57),IMAINSTEP, new Short(pretabindex++), new Integer(90)}
        );

        CurUnoDialog.insertLabel("lblTableNames",
                    new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                    new Object[] {new Integer(8), sTableNames, new Integer(95), new Integer(68), IMAINSTEP, new Short(pretabindex++), new Integer(80) });

        try {
            xTableListBox = CurUnoDialog.insertListBox("lstTableNames", 3, null, this,
                        new String[] { "Dropdown", "Height", "HelpURL", "LineCount", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                        new Object[] { Boolean.TRUE, new Integer(12), "HID:41208", new Short("7"), new Integer(95), new Integer(78), IMAINSTEP, new Short(pretabindex++), getListboxWidth()});
        } catch (Exception e) {
            e.printStackTrace(System.out);
        }
        initializeCategory(0);
    }


    public int getCategory(){
        if (optBusiness.getState())
            curcategory = 0;
        else
            curcategory = 1;
        return curcategory;
    }


    public void selectCategory(){
        if (optBusiness.getState())
            initializeCategory(0);
        else
            initializeCategory(1);
    }


    public void initializeCategory(int _iCategory){
    try {
        oCGCategory.initialize(_iCategory);
        xTableListBox.removeItems((short) 0, xTableListBox.getItemCount());
        xTableListBox.addItems(oCGCategory.getTableNames(),(short) _iCategory);
        initializeTable(0);
    } catch (Exception e) {
        e.printStackTrace(System.out);
    }}



    public void initializeTable(int _iTable){
        Helper.setUnoPropertyValue(UnoDialog.getModel(xTableListBox), "SelectedItems", new short[]{(short)_iTable});
        oCGTable.initialize(oCGCategory.xNameAccessTablesNode, _iTable);
        super.initialize(oCGTable.getFieldNames(bcolumnnameislimited), true);
    }


    public String[] getSelectedFieldNames(){
        String[] displayfieldnames = super.getSelectedFieldNames();
        fieldnames = new String[displayfieldnames.length];
        for (int i = 0; i < displayfieldnames.length; i++){
            FieldDescription ofielddescription = (FieldDescription) CurUnoDialog.fielditems.get(displayfieldnames[i]);
            fieldnames[i] = ofielddescription.getName();
        }
        return fieldnames;
    }


    public void addColumnsToDescriptor(){
        XNameAccess xNameAccessFieldNode;
        String[] fieldnames = getSelectedFieldNames();
        for (int i = 0; i < fieldnames.length; i++){
            FieldDescription curfielddescription = (FieldDescription) CurUnoDialog.fielditems.get(fieldnames[i]);
            PropertyValue[] aProperties = curfielddescription.getPropertyValues();
            this.curtabledescriptor.addColumn(aProperties);
//          this.curtabledescriptor.addColumnToTempColumnContainer(aProperties);
        }
    }


/*  public void getProperties(){
    try {
        String[] spropnames = Configuration.getNodeDisplayNames(xNameAccessPropertiesNode);
        for (int i = 0; i < spropnames.length; i++){
            XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xNameAccessPropertiesNode);
            String sname = (String) xPropertySet.getPropertyValue("Name");
            String svalue = (String) xPropertySet.getPropertyValue("Value");
        }
        int i = 0;
    } catch (Exception e) {
        e.printStackTrace(System.out);
//      return null;
    }} */



    public String getTableName(){
        return xTableListBox.getSelectedItem();
    }


    public String getFirstTableName(){
        String[] fieldnames = super.getSelectedFieldNames();
        if ((fieldnames.length) > 0){
            for (int i = 0; i < CurUnoDialog.fielditems.size(); i++){
                FieldDescription ofielddescription = (FieldDescription) CurUnoDialog.fielditems.get(fieldnames[i]);
                String stablename = ofielddescription.gettablename();
                if (!stablename.equals(""))
                    return stablename;
            }
        }
         return smytable;
    }

    public void fillupTables(){
//      if (optBusiness.getState())
//      else
    }

    public void fillupFieldsListbox(){
        super.emptyFieldsListBoxes();
    }


    /* (non-Javadoc)
     * @see com.sun.star.awt.XItemListener#itemStateChanged(com.sun.star.awt.ItemEvent)
     */
    public void itemStateChanged(ItemEvent arg0) {
        initializeTable((int) xTableListBox.getSelectedItemPos());
    }

    /* (non-Javadoc)
     * @see com.sun.star.lang.XEventListener#disposing(com.sun.star.lang.EventObject)
     */
    public void disposing(EventObject arg0) {
        // TODO Auto-generated method stub

    }


     protected int ID;

     public int getID(){
         return ID;
     }

     public void setID(String sIncSuffix){
     }


     public void shiftFromLeftToRight(String[] SelItems, String[] NewItems) {
        CurUnoDialog.enablefromStep(TableWizard.SOFIELDSFORMATPAGE, true);
        for (int i = 0; i < NewItems.length; i++){
            int iduplicate;
            if (CurUnoDialog.fielditems.containsKey(NewItems[i])){
                iduplicate = JavaTools.getDuplicateFieldIndex(NewItems, NewItems[i]);
                if (iduplicate != -1){
                    XNameAccess xNameAccessFieldNode;
                    String sdisplayname = Desktop.getUniqueName(NewItems, NewItems[iduplicate]);
                    FieldDescription curfielddescription = new FieldDescription(this, sdisplayname, NewItems[iduplicate]);
                    CurUnoDialog.fielditems.put(sdisplayname, curfielddescription);
                    NewItems[iduplicate] = sdisplayname;
                    setSelectedFieldNames(NewItems);
                }
            }
            else
                CurUnoDialog.fielditems.put(NewItems[i], new FieldDescription(this, NewItems[i], NewItems[i]));
         }
     }


     public void shiftFromRightToLeft(String[] SelItems, String[] NewItems ) {
        if (NewItems.length == 0)
            CurUnoDialog.enablefromStep(TableWizard.SOFIELDSFORMATPAGE, false);
        for (int i = 0; i < SelItems.length; i++){
            if (CurUnoDialog.fielditems.containsKey(SelItems[i])){
                CurUnoDialog.fielditems.remove(SelItems[i]);
                this.curtabledescriptor.dropColumnbyName(SelItems[i]);
            }
        }
     }

     public void moveItemDown(String item){
     }

     public void moveItemUp(String item){
     }
}
