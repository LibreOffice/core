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
package com.sun.star.wizards.table;

import com.sun.star.awt.ActionEvent;
import com.sun.star.awt.FontDescriptor;
import com.sun.star.awt.ItemEvent;
import com.sun.star.awt.TextEvent;
import com.sun.star.awt.XButton;
import com.sun.star.awt.XItemListener;
import com.sun.star.awt.XListBox;
import com.sun.star.awt.XTextComponent;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.db.TableDescriptor;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.UnoDialog;
import com.sun.star.wizards.ui.event.XActionListenerAdapter;
import com.sun.star.wizards.ui.event.XTextListenerAdapter;

public class FieldFormatter implements XItemListener
{

    private TableWizard CurUnoDialog;
    private TableDescriptor curTableDescriptor;
    private Object oColumnDescriptorModel;
    private XTextComponent txtfieldname;
    private XListBox xlstFieldNames;
    private XButton btnminus;
    private XButton btnShiftUp;
    private XButton btnShiftDown;
    private short curtabindex;
    private String suntitled;
    private Integer IFieldFormatStep;

    public FieldFormatter(TableWizard _CurUnoDialog)
    {
        this.CurUnoDialog = _CurUnoDialog;
        curtabindex = (short) (TableWizard.SOFIELDSFORMATPAGE * 100);
        IFieldFormatStep = Integer.valueOf(TableWizard.SOFIELDSFORMATPAGE);
        String sFieldName = CurUnoDialog.m_oResource.getResText("RID_TABLE_23");
        String sFieldNames = CurUnoDialog.m_oResource.getResText("RID_TABLE_25");
        String sfieldinfo = CurUnoDialog.m_oResource.getResText("RID_TABLE_20");
        String sbtnplushelptext = CurUnoDialog.m_oResource.getResText("RID_TABLE_45");
        String sbtnminushelptext = CurUnoDialog.m_oResource.getResText("RID_TABLE_46");

        suntitled = CurUnoDialog.m_oResource.getResText("RID_TABLE_43");

        CurUnoDialog.insertLabel("lblFieldNames",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], sFieldNames, 91, 27, IFieldFormatStep, Short.valueOf(curtabindex++), 90
                });

        try
        {
            xlstFieldNames = CurUnoDialog.insertListBox("lstfieldnames", 3, null, this,
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        133, "HID:WIZARDS_HID_DLGTABLE_LB_SELFIELDNAMES", 92, 37, IFieldFormatStep, Short.valueOf(curtabindex++), 62
                    });
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }

        FontDescriptor oFontDesc = new FontDescriptor();
        oFontDesc.Name = "StarSymbol";

        btnShiftUp = CurUnoDialog.insertButton("btnShiftUp", new XActionListenerAdapter() {
                    @Override
                    public void actionPerformed(ActionEvent event) {
                        shiftFieldNameUp();
                    }
                },
                new String[]
                {
                    PropertyNames.PROPERTY_ENABLED, PropertyNames.FONT_DESCRIPTOR, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.FALSE, oFontDesc, 14, "HID:WIZARDS_HID_DLGTABLE_CMDMOVEFIELDUP", String.valueOf((char) 8743), 158, 139, IFieldFormatStep, Short.valueOf(curtabindex++), 14
                });

        btnShiftDown = CurUnoDialog.insertButton("btnShiftDown", new XActionListenerAdapter() {
                    @Override
                    public void actionPerformed(ActionEvent event) {
                        shiftFieldNameDown();
                    }
                },
                new String[]
                {
                    PropertyNames.PROPERTY_ENABLED, PropertyNames.FONT_DESCRIPTOR, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.FALSE, oFontDesc, 14, "HID:WIZARDS_HID_DLGTABLE_CMDMOVEFIELDDOWN", String.valueOf((char) 8744), 158, 156, IFieldFormatStep, Short.valueOf(curtabindex++), 14
                });
        oFontDesc = new FontDescriptor();
        oFontDesc.Weight = com.sun.star.awt.FontWeight.BOLD;
        oFontDesc.Height = (short) 13;
        btnminus = CurUnoDialog.insertButton("btnminus", new XActionListenerAdapter() {
                    @Override
                    public void actionPerformed(ActionEvent event) {
                        removeFieldName();
                    }
                },
                new String[]
                {
                    PropertyNames.FONT_DESCRIPTOR, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    oFontDesc, 14, "HID:WIZARDS_HID_DLGTABLE_CMDMINUS", "-", 118, 175, IFieldFormatStep, Short.valueOf(curtabindex++), 14
                });

        XButton btnplus = CurUnoDialog.insertButton("btnplus", new XActionListenerAdapter() {
                    @Override
                    public void actionPerformed(ActionEvent event) {
                        addFieldName();
                    }
                },
                new String[]
                {
                    PropertyNames.FONT_DESCRIPTOR, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    oFontDesc, 14, "HID:WIZARDS_HID_DLGTABLE_CMDPLUS", "+", 137, 175, IFieldFormatStep, Short.valueOf(curtabindex++), 14
                });

        CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedLineModel", "ColDescriptorHeader",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.ORIENTATION, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    8, sfieldinfo, 0, 158, 27, IFieldFormatStep, Short.valueOf(curtabindex++), 165
                });


        CurUnoDialog.insertLabel("lblFieldName",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], sFieldName, 158, 39, IFieldFormatStep, Short.valueOf(curtabindex++), 94
                });

        txtfieldname = CurUnoDialog.insertTextField("txtfieldname", new XTextListenerAdapter() {
                    @Override
                    public void textChanged(TextEvent event) {
                        modifyFieldName();
                    }
                },
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Text", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGER_12, "HID:WIZARDS_HID_DLGTABLE_COLNAME", 274, 37, IFieldFormatStep, Short.valueOf(curtabindex++), PropertyNames.EMPTY_STRING, 50
                });
        txtfieldname.addTextListener(CurUnoDialog);
        CurUnoDialog.getPeerConfiguration().setAccessibleName(btnplus, sbtnplushelptext);
        CurUnoDialog.getPeerConfiguration().setAccessibleName(btnminus, sbtnminushelptext);
    }

    public void initialize(TableDescriptor _curTableDescriptor, String[] _fieldnames)
    {
        if (oColumnDescriptorModel == null)
        {
            oColumnDescriptorModel = CurUnoDialog.insertControlModel("com.sun.star.sdb.ColumnDescriptorControlModel", "oColumnDescriptor",
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH, "EditWidth"
                    }, // PropertyNames.PROPERTY_HELPURL
                    new Object[]
                    {
                        85, 158, 49, IFieldFormatStep, Short.valueOf(curtabindex++), 166, 50
                    });  //, "HID:WIZARDS_HID_DLGTABLE_COLMODIFIER"
            curTableDescriptor = _curTableDescriptor;
            Helper.setUnoPropertyValue(oColumnDescriptorModel, PropertyNames.ACTIVE_CONNECTION, _curTableDescriptor.DBConnection);
            txtfieldname.setMaxTextLen((short) this.curTableDescriptor.getMaxColumnNameLength());
        }
        else
        {
            int nStep = ((Integer) Helper.getUnoPropertyValue(oColumnDescriptorModel, PropertyNames.PROPERTY_STEP)).intValue();
            if (nStep > IFieldFormatStep.intValue())
            {
                Helper.setUnoPropertyValue(oColumnDescriptorModel, PropertyNames.PROPERTY_STEP, IFieldFormatStep);
                CurUnoDialog.repaintDialogStep();
            }
        }
        Helper.setUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), PropertyNames.STRING_ITEM_LIST, _fieldnames);
        Helper.setUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), PropertyNames.SELECTED_ITEMS, new short[]
                {
                    0
                });
        updateColumnDescriptor(_fieldnames[0], curTableDescriptor.getByName(_fieldnames[0]));
        toggleButtons();
        CurUnoDialog.setFocus("lstfieldnames");
    }

    private void toggleButtons()
    {
        boolean benableShiftUpButton = false;
        boolean benableShiftDownButton = false;
        boolean bfieldnameisselected = Helper.getUnoArrayPropertyValue(UnoDialog.getModel(xlstFieldNames), PropertyNames.SELECTED_ITEMS) != null;
        int ilistcount = /* xlstFieldNames.getItemCount();*/ UnoDialog.getListBoxItemCount(xlstFieldNames);
        boolean blistispopulated = (ilistcount > 0);
        if (bfieldnameisselected)
        {
            int iselpos = xlstFieldNames.getSelectedItemPos();
            benableShiftUpButton = (iselpos != 0);
            benableShiftDownButton = (iselpos != ilistcount - 1);
        }
        Helper.setUnoPropertyValue(UnoDialog.getModel(btnShiftUp), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(benableShiftUpButton));
        Helper.setUnoPropertyValue(UnoDialog.getModel(btnShiftDown), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(benableShiftDownButton));
        Helper.setUnoPropertyValue(UnoDialog.getModel(btnminus), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(blistispopulated));
        CurUnoDialog.setcompleted(TableWizard.SOFIELDSFORMATPAGE, blistispopulated);
    }

    private void addFieldName()
    {
        String snewfieldname = Desktop.getUniqueName(xlstFieldNames.getItems(), suntitled, PropertyNames.EMPTY_STRING);
        short icount = xlstFieldNames.getItemCount();
        if (CurUnoDialog.verifyfieldcount(icount))
        {
            xlstFieldNames.addItem(snewfieldname, icount);
            Helper.setUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), PropertyNames.SELECTED_ITEMS, new short[]
                    {
                        icount
                    });
            toggleButtons();
            FieldDescription curfielddescription = new FieldDescription(snewfieldname);
            CurUnoDialog.fielditems.put(snewfieldname, curfielddescription);
            curTableDescriptor.addColumn(curfielddescription.getPropertyValues());
            updateColumnDescriptor(snewfieldname, curTableDescriptor.getByName(snewfieldname));
            CurUnoDialog.setControlVisible("oColumnDescriptor", true);
            CurUnoDialog.repaintDialogStep();
        }
    }

    private void removeFieldName()
    {
        String[] fieldnames = (String[]) Helper.getUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), PropertyNames.STRING_ITEM_LIST);
        short ipos = UnoDialog.getSelectedItemPos(xlstFieldNames);
        String fieldname = fieldnames[ipos];
        xlstFieldNames.removeItems(ipos, (short) 1);
        CurUnoDialog.fielditems.remove(fieldname);
        int ilistcount = /* xlstFieldNames.getItemCount();*/ UnoDialog.getListBoxItemCount(xlstFieldNames);
        if ((ipos) < ilistcount)
        {
            Helper.setUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), PropertyNames.SELECTED_ITEMS, new short[]
                    {
                        ipos
                    });
        }
        else
        {
            if (ilistcount > -1)
            {
                ipos = (short) ((short) ilistcount - (short) 1);
                Helper.setUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), PropertyNames.SELECTED_ITEMS, new short[]
                        {
                            ipos
                        });
            }
        }
        curTableDescriptor.dropColumnbyName(fieldname);
        fieldnames = (String[]) Helper.getUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), PropertyNames.STRING_ITEM_LIST);
        boolean benable = ((ipos > -1) && (ipos < fieldnames.length));
        if (benable)
        {
            String snewfieldname = fieldnames[ipos];
            updateColumnDescriptor(snewfieldname, curTableDescriptor.getByName(snewfieldname));
            toggleButtons();
        }
        else
        {
            Helper.setUnoPropertyValue(UnoDialog.getModel(txtfieldname), "Text", PropertyNames.EMPTY_STRING);
            Helper.setUnoPropertyValue(UnoDialog.getModel(btnminus), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(benable));
            CurUnoDialog.setcompleted(TableWizard.SOFIELDSFORMATPAGE, benable);
        }
        Helper.setUnoPropertyValue(UnoDialog.getModel(btnminus), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(benable));
        CurUnoDialog.setControlVisible("oColumnDescriptor", benable);
        CurUnoDialog.repaintDialogStep();
    }

    private void modifyFieldName()
    {
        String newfieldname = txtfieldname.getText();
        String oldfieldname = xlstFieldNames.getSelectedItem();
        if (!newfieldname.equals(oldfieldname))
        {
            if (curTableDescriptor.modifyColumnName(oldfieldname, newfieldname))
            {
                Object oColumn = Helper.getUnoPropertyValue(oColumnDescriptorModel, "Column");
                Helper.setUnoPropertyValue(oColumn, PropertyNames.PROPERTY_NAME, newfieldname);
                FieldDescription curfielddescription = CurUnoDialog.fielditems.get(oldfieldname);
                CurUnoDialog.fielditems.remove(oldfieldname);
                curfielddescription.setName(newfieldname);
                CurUnoDialog.fielditems.put(newfieldname, curfielddescription);
                String[] fieldnames = xlstFieldNames.getItems();
                short ipos = xlstFieldNames.getSelectedItemPos();
                fieldnames[ipos] = newfieldname;
                Helper.setUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), PropertyNames.STRING_ITEM_LIST, fieldnames);
                Helper.setUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), PropertyNames.SELECTED_ITEMS, new short[]
                        {
                            ipos
                        });
            }
        }
    }

    private void shiftFieldNameUp()
    {
        short ipos = xlstFieldNames.getSelectedItemPos();
        String[] snewlist = shiftArrayItem(xlstFieldNames.getItems(), ipos, -1);
        Helper.setUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), PropertyNames.STRING_ITEM_LIST, snewlist);
        if ((ipos - 1) > -1)
        {
            Helper.setUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), PropertyNames.SELECTED_ITEMS, new short[]
                    {
                        (short) (ipos - 1)
                    });
            curTableDescriptor.moveColumn(ipos, ipos - 1);
        }
        toggleButtons();
    }

    private void shiftFieldNameDown()
    {
        short ipos = xlstFieldNames.getSelectedItemPos();
        String[] snewlist = shiftArrayItem(xlstFieldNames.getItems(), ipos, 1);
        Helper.setUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), PropertyNames.STRING_ITEM_LIST, snewlist);
        if ((ipos + 1) < xlstFieldNames.getItemCount())
        {
            Helper.setUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), PropertyNames.SELECTED_ITEMS, new short[]
                    {
                        (short) (ipos + 1)
                    });
            curTableDescriptor.moveColumn(ipos, ipos + 1);
        }
        toggleButtons();
    }

    private String[] shiftArrayItem(String[] _slist, int _oldindex, int _shiftcount)
    {
        int newindex = _oldindex + _shiftcount;
        if ((newindex >= 0) && (newindex < _slist.length))
        {
            String buffer = _slist[newindex];
            _slist[newindex] = _slist[_oldindex];
            _slist[_oldindex] = buffer;
        }
        return _slist;
    }

    public boolean updateColumnofColumnDescriptor()
    {
        Object oColumn = Helper.getUnoPropertyValue(oColumnDescriptorModel, "Column");
        XPropertySet xColPropertySet = UnoRuntime.queryInterface(XPropertySet.class, oColumn);
        if (xColPropertySet != null)
        {
            curTableDescriptor.modifyColumn(txtfieldname.getText(), xColPropertySet);
            return true;
        }
        return false;
    }

    private void updateColumnDescriptor(String _ColumnName, XPropertySet _xColumn)
    {
        updateColumnofColumnDescriptor();
        XPropertySet xNewPropertySet = curTableDescriptor.clonePropertySet(_ColumnName, _xColumn);
        if (xNewPropertySet != null)
        {
            Helper.setUnoPropertyValue(oColumnDescriptorModel, "Column", xNewPropertySet);
        }
        txtfieldname.setText(_ColumnName);
    }



    public void itemStateChanged(ItemEvent arg0)
    {
        String fieldname = xlstFieldNames.getSelectedItem();
        XPropertySet xColumn = curTableDescriptor.getByName(fieldname);
        updateColumnDescriptor(fieldname, xColumn);
        toggleButtons();
    }

    public boolean iscompleted()
    {
        String[] sfieldnames = (String[]) Helper.getUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), PropertyNames.STRING_ITEM_LIST);
        return sfieldnames.length > 0;
    }

    public String[] getFieldNames()
    {
        return (String[]) Helper.getUnoPropertyValue(UnoDialog.getModel(xlstFieldNames), PropertyNames.STRING_ITEM_LIST);
    }

    /* (non-Javadoc)
     * @see com.sun.star.lang.XEventListener#disposing(com.sun.star.lang.EventObject)
     */
    public void disposing(EventObject arg0)
    {
        // TODO Auto-generated method stub
    }
}
