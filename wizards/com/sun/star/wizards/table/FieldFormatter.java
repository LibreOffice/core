/*************************************************************************
*
*  $RCSfile: FieldFormatter.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: pjunck $ $Date: 2004-10-27 13:38:00 $
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

import com.sun.star.awt.FontDescriptor;
import com.sun.star.awt.ItemEvent;
import com.sun.star.awt.XButton;
import com.sun.star.awt.XItemListener;
import com.sun.star.awt.XListBox;
import com.sun.star.awt.XTextComponent;
import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.sdbc.DataType;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.db.TableDescriptor;
import com.sun.star.wizards.ui.PeerConfigHelper;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.UnoDialog;

public class FieldFormatter  implements XItemListener{
    TableWizard CurUnoDialog;
    TableDescriptor curTableDescriptor;
    Object oColumnDescriptorModel;
    XTextComponent txtfieldname;
    XListBox xlstFieldNames;
    XButton btnplus;
    XButton btnminus;
    XButton btnShiftUp;
    XButton btnShiftDown;
    short curtabindex;
    String TOGGLEBUTTONS = "toggleButtons";
    String ADDFIELDNAME = "addFieldName";
    String REMOVEFIELDNAME = "removeFieldName";
    String MODIFYFIELDNAME = "modifyFieldName";
    String[] fieldnames;
    String suntitled;
    Integer IFieldFormatStep;

    public FieldFormatter(TableWizard _CurUnoDialog, TableDescriptor _curTableDescriptor) {
        this.CurUnoDialog = _CurUnoDialog;
        curtabindex = (short) (TableWizard.SOFIELDSFORMATPAGE * 100);
        IFieldFormatStep = new Integer(TableWizard.SOFIELDSFORMATPAGE);
        String sFieldNames = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 25);
        String sfieldinfo = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 20);
        String sbtnplushelptext = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 45);
        String sbtnminushelptext = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 46);

        suntitled = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 43);

        CurUnoDialog.insertLabel("lblFieldNames",
                new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                new Object[] {UIConsts.INTEGERS[8], sFieldNames, new Integer(91), new Integer(27),IFieldFormatStep, new Short(curtabindex++), new Integer(90)});

        try {
            xlstFieldNames = CurUnoDialog.insertListBox("lstfieldnames", 3, null, this,
                    new String[] {"Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                    new Object[] { new Integer(125),"HID:41220", new Integer(92), new Integer(37), IFieldFormatStep, new Short(curtabindex++), new Integer(62)});
        } catch (Exception e) {
            e.printStackTrace(System.out);
        }

        FontDescriptor oFontDesc = new FontDescriptor();
        oFontDesc.Name = "StarSymbol";

        btnShiftUp = CurUnoDialog.insertButton("btnShiftUp", "shiftFieldNameUp", this,
                                new String[] { "Enabled", "FontDescriptor", "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                new Object[] { Boolean.FALSE, oFontDesc, new Integer(14), "HID:41221", String.valueOf((char) 8743), new Integer(158), new Integer(130), IFieldFormatStep, new Short(curtabindex++), new Integer(14)});

        btnShiftDown = CurUnoDialog.insertButton("btnShiftDown", "shiftFieldNameDown", this,
                                new String[] { "Enabled", "FontDescriptor", "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                new Object[] { Boolean.FALSE, oFontDesc, new Integer(14), "HID:41222", String.valueOf((char) 8744), new Integer(158), new Integer(148), IFieldFormatStep, new Short(curtabindex++), new Integer(14) });
        oFontDesc = new FontDescriptor();
        oFontDesc.Weight = com.sun.star.awt.FontWeight.BOLD;
        oFontDesc.Height = (short) 13;
        btnminus = CurUnoDialog.insertButton("btnminus", "removeFieldName", this,
                                new String[] {"FontDescriptor", "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                new Object[] {oFontDesc, new Integer(14), "HID:41223", "-", new Integer(118), new Integer(167), IFieldFormatStep, new Short(curtabindex++), new Integer(14) });

        btnplus = CurUnoDialog.insertButton("btnplus", "addFieldName", this,
                                new String[] {"FontDescriptor", "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                new Object[] { oFontDesc, new Integer(14), "HID:41224", "+", new Integer(137), new Integer(167), IFieldFormatStep, new Short(curtabindex++), new Integer(14) });

        CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedLineModel", "ColDescriptorHeader",
                new String[] { "Height", "Label", "Orientation", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                new Object[] { new Integer(8), sfieldinfo, new Integer(0), new Integer(158), new Integer(27), IFieldFormatStep, new Short(curtabindex++), new Integer(145) });


        CurUnoDialog.insertLabel("lblFieldName",
                new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                new Object[] {UIConsts.INTEGERS[8], sFieldNames, new Integer(158), new Integer(39),IFieldFormatStep, new Short(curtabindex++), new Integer(100)});

        txtfieldname = CurUnoDialog.insertTextField("txtfieldname", MODIFYFIELDNAME, this,
                new String[] {"Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Text", "Width"},
                new Object[] { UIConsts.INTEGER_12, "HID:41225", new Integer(254), new Integer(37), IFieldFormatStep, new Short(curtabindex++),"", new Integer(50)});

        PeerConfigHelper obtnpeerconfig = new PeerConfigHelper(CurUnoDialog.xUnoDialog);
        obtnpeerconfig.setPeerProperties(btnplus, new String[] { "AccessibilityName" },new String[] { sbtnplushelptext });
        obtnpeerconfig.setPeerProperties(btnminus, new String[] { "AccessibilityName" }, new String[] { sbtnminushelptext });
    }


    public void initialize(TableDescriptor _curTableDescriptor, String[] _fieldnames){
        if (oColumnDescriptorModel == null){
        // TODO How can I assign a HelpUrl without Wrapped Target exception
            oColumnDescriptorModel = CurUnoDialog.insertControlModel("com.sun.star.sdb.ColumnDescriptorControlModel", "oColumnDescriptor",
                    new String[] {"Height", "PositionX", "PositionY", "Step", "TabIndex", "Width","EditWidth"}, // "HelpURL"
                    new Object[] {new Integer(77), new Integer(158), new Integer(49), IFieldFormatStep, new Short(curtabindex++), new Integer(146), new Integer(50)});  //, "HID:41226"
            curTableDescriptor = _curTableDescriptor;
            Helper.setUnoPropertyValue(oColumnDescriptorModel, "ActiveConnection", _curTableDescriptor.DBConnection);
        }
        Helper.setUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), "StringItemList", _fieldnames);
        Helper.setUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), "SelectedItems", new short[] {0});
//      try {
//          int i = curTableDescriptor.tempcolumncontainer.size();
//          updateColumnDescriptor("Comments", curTableDescriptor.tempcolumncontainer.elementAt(0)); //add(curTableDescriptor.getColumnbyName(fieldnames[0])));
//      } catch (RuntimeException e) {
//          e.printStackTrace(System.out);
//      }
        updateColumnDescriptor(_fieldnames[0], curTableDescriptor.getColumnbyName(_fieldnames[0]));
        toggleButtons();
        CurUnoDialog.setFocus("lstfieldnames");
    }


    public void toggleButtons(){
        boolean benableShiftUpButton = false;
        boolean benableShiftDownButton = false;
        boolean bfieldnameisselected = Helper.getUnoArrayPropertyValue(UnoDialog.getModel(xlstFieldNames), "SelectedItems") != null;
        int ilistcount = UnoDialog.getListBoxItemCount(xlstFieldNames);
        boolean blistispopulated = (ilistcount > 0);
        if (bfieldnameisselected){
            int iselpos = xlstFieldNames.getSelectedItemPos();
            benableShiftUpButton = (iselpos != 0);
            benableShiftDownButton = (iselpos != ilistcount - 1);
        }
        Helper.setUnoPropertyValue(UnoDialog.getModel(btnShiftUp), "Enabled", new Boolean(benableShiftUpButton));
        Helper.setUnoPropertyValue(UnoDialog.getModel(btnShiftDown), "Enabled", new Boolean(benableShiftDownButton));
        Helper.setUnoPropertyValue(UnoDialog.getModel(btnminus), "Enabled", new Boolean(blistispopulated));
        CurUnoDialog.enablefromStep(TableWizard.SOPRIMARYKEYPAGE, blistispopulated);
    }


    public void addFieldName(){
        String snewfieldname = Desktop.getUniqueName(xlstFieldNames.getItems(), suntitled);
        short icount = xlstFieldNames.getItemCount();
        xlstFieldNames.addItem(snewfieldname, icount);
        Helper.setUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), "SelectedItems", new short[] {icount});
        toggleButtons();
        FieldDescription curfielddescription = new FieldDescription(snewfieldname);
        CurUnoDialog.fielditems.put(snewfieldname, curfielddescription);
        curTableDescriptor.addColumn(curfielddescription.getPropertyValues());
        updateColumnDescriptor(snewfieldname, curTableDescriptor.getColumnbyName(snewfieldname));
    }


    public void removeFieldName(){
        String[] fieldnames = (String[]) Helper.getUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), "StringItemList");
        short ipos = UnoDialog.getSelectedItemPos(xlstFieldNames);
        String fieldname = fieldnames[ipos];
        xlstFieldNames.removeItems(ipos, (short) 1);
        CurUnoDialog.fielditems.remove(fieldname);
        int ilistcount = UnoDialog.getListBoxItemCount(xlstFieldNames);
        if ((ipos) < ilistcount)
            Helper.setUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), "SelectedItems", new short[] {ipos});
        else{
            if (ilistcount > -1){
                ipos = (short) ((short) ilistcount - (short) 1);
                Helper.setUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), "SelectedItems", new short[] {ipos});
            }
        }
        curTableDescriptor.dropColumnbyName(fieldname);
        fieldnames = (String[]) Helper.getUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), "StringItemList");
        boolean benable = ((ipos > -1) && (ipos < fieldnames.length));
        if (benable){
            String snewfieldname = fieldnames[ipos];
            updateColumnDescriptor(snewfieldname, curTableDescriptor.getColumnbyName(snewfieldname));
            toggleButtons();
        }
        else{
//          Helper.setUnoPropertyValue(oColumnDescriptorModel, "Column", Any.VOID);
            Helper.setUnoPropertyValue(UnoDialog.getModel(btnminus), "Enabled", new Boolean(benable));
            CurUnoDialog.enablefromStep(TableWizard.SOPRIMARYKEYPAGE, benable);
        }
    }


    public void modifyFieldName(){
        String newfieldname = txtfieldname.getText();
        String oldfieldname = xlstFieldNames.getSelectedItem();
        if (!newfieldname.equals(oldfieldname)){
            if (curTableDescriptor.modifyColumnName(oldfieldname, newfieldname)){
                FieldDescription curfielddescription = (FieldDescription) CurUnoDialog.fielditems.get(oldfieldname);
                CurUnoDialog.fielditems.remove(oldfieldname);
                curfielddescription.setName(newfieldname);
                CurUnoDialog.fielditems.put(newfieldname, curfielddescription);
                String[] fieldnames = xlstFieldNames.getItems();
                short ipos = xlstFieldNames.getSelectedItemPos();
                fieldnames[ipos] = newfieldname;
                Helper.setUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), "StringItemList", fieldnames);
                Helper.setUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), "SelectedItems", new short[] {ipos});
                CurUnoDialog.enablefromStep(TableWizard.SOPRIMARYKEYPAGE, true);
            }
            else
                CurUnoDialog.enablefromStep(TableWizard.SOPRIMARYKEYPAGE, false);
        }
    }


    public void shiftFieldNameUp(){
        short ipos = xlstFieldNames.getSelectedItemPos();
        String[] snewlist = shiftArrayItem(xlstFieldNames.getItems(), ipos, -1);
        Helper.setUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), "StringItemList", snewlist);
        if ((ipos - 1) > -1)
            xlstFieldNames.selectItemPos((short) (ipos - 1), true);
        toggleButtons();
    }


    public void shiftFieldNameDown(){
        short ipos = xlstFieldNames.getSelectedItemPos();
        String[] snewlist = shiftArrayItem(xlstFieldNames.getItems(), ipos, 1);
        Helper.setUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), "StringItemList", snewlist);
        if ((ipos + 1) < xlstFieldNames.getItemCount())
            xlstFieldNames.selectItemPos((short) (ipos + 1), true);
        toggleButtons();
    }


    public String[] shiftArrayItem(String[] _slist, int _oldindex, int _shiftcount){
        int newindex = _oldindex + _shiftcount;
        if ((newindex >= 0) && (newindex < _slist.length)){
            String buffer = _slist[newindex];
            _slist[newindex] = _slist[_oldindex];
            _slist[_oldindex] = buffer;
        }
        return _slist;
    }


    public boolean updateColumnofColumnDescriptor(){
        Object oColumn = Helper.getUnoPropertyValue(oColumnDescriptorModel, "Column");
        XPropertySet xColPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oColumn);
        if (xColPropertySet != null){
            curTableDescriptor.modifyColumn(txtfieldname.getText(), xColPropertySet);
            return true;
        }
        return false;
    }

    private void updateColumnDescriptor(String _ColumnName, XPropertySet _xColumn){
        updateColumnofColumnDescriptor();
        XPropertySet xNewPropertySet = curTableDescriptor.assignPropertyValues(_xColumn, "");
        if (xNewPropertySet != null)
            Helper.setUnoPropertyValue(oColumnDescriptorModel, "Column", xNewPropertySet);
        txtfieldname.setText(_ColumnName);
    }


    public XPropertySet clonePropertySet(XPropertySet _xPropertySet){
        XPropertySet xlocpropertyset = null;
        return xlocpropertyset;
    }

    public void itemStateChanged(ItemEvent arg0) {
        String fieldname = xlstFieldNames.getSelectedItem();
        XPropertySet xColumn = curTableDescriptor.getColumnbyName(fieldname);
        updateColumnDescriptor(fieldname, xColumn);
        toggleButtons();
    }

    public String[] getFieldNames(){
        return (String[]) Helper.getUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), "StringItemList");
    }

    /* (non-Javadoc)
     * @see com.sun.star.lang.XEventListener#disposing(com.sun.star.lang.EventObject)
     */
    public void disposing(EventObject arg0) {
        // TODO Auto-generated method stub
    }

}
