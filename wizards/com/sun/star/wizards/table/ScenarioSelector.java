/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
package com.sun.star.wizards.table;

import java.util.Vector;

import com.sun.star.awt.ItemEvent;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XItemListener;
import com.sun.star.awt.XListBox;
import com.sun.star.awt.XRadioButton;
import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.Locale;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Configuration;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.db.TableDescriptor;
import com.sun.star.wizards.ui.FieldSelection;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.UnoDialog;
import com.sun.star.wizards.ui.XFieldSelectionListener;

/**
 * @author Administrator
 *
 * To change the template for this generated type comment go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
public class ScenarioSelector extends FieldSelection implements XItemListener, XFieldSelectionListener
{
    final static int PRIVATE = 0;
    final static int BUSINESS = 1;

    private XFixedText lblExplanation;
    private XFixedText lblCategories;
    private XRadioButton optBusiness;
    private XRadioButton optPrivate;
    private XListBox xTableListBox;
    private TableWizard CurTableWizardUnoDialog;
    private TableDescriptor curtabledescriptor;
    private CGCategory oCGCategory;
    protected CGTable oCGTable;
    private String SELECTCATEGORY = "selectCategory";
    private int curcategory;
    protected boolean bcolumnnameislimited;
    private int imaxcolumnchars;
    private String smytable;
    private Locale aLocale;
    private XMultiServiceFactory xMSF;

    public ScenarioSelector(TableWizard _CurUnoDialog, TableDescriptor _curtabledescriptor, String _reslblFields, String _reslblSelFields)
    {
        super(_CurUnoDialog, TableWizard.SOMAINPAGE, 91, 108, 230, 80, _reslblFields, _reslblSelFields, 41209, true);
        CurTableWizardUnoDialog =  _CurUnoDialog;
        xMSF = CurUnoDialog.xMSF;
        aLocale = Configuration.getOfficeLocale(xMSF);
        curtabledescriptor = _curtabledescriptor;
        imaxcolumnchars = this.curtabledescriptor.getMaxColumnNameLength();
        bcolumnnameislimited = (imaxcolumnchars > 0) && (imaxcolumnchars < 16);
        addFieldSelectionListener(this);
        short pretabindex = (short) (50);
        String sExplanation = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 14);
        String sCategories = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 15);
        String sBusiness = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 16);
        String sPrivate = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 17);
        String sTableNames = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 18);
        smytable = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 44);
        Integer IMAINSTEP = new Integer(TableWizard.SOMAINPAGE);
        oCGCategory = new CGCategory(CurUnoDialog.xMSF);
        oCGTable = new CGTable(CurUnoDialog.xMSF);
        lblExplanation = CurUnoDialog.insertLabel("lblScenarioExplanation",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    32, sExplanation, Boolean.TRUE, 91, 27, IMAINSTEP, new Short(pretabindex++), 233
                });

        lblCategories = CurUnoDialog.insertLabel("lblCategories",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    8, sCategories, 91, 60, IMAINSTEP, new Short(pretabindex++), 100
                });

        optBusiness = CurTableWizardUnoDialog.insertRadioButton("optBusiness", SELECTCATEGORY, this,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGTABLE_OPTBUSINESS", sBusiness, 98, 70, new Short((short) 1), IMAINSTEP, new Short(pretabindex++), 78
                });

        optPrivate = CurTableWizardUnoDialog.insertRadioButton("optPrivate", SELECTCATEGORY, this,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGTABLE_OPTPRIVATE", sPrivate, 182, 70, IMAINSTEP, new Short(pretabindex++), 90
                });

        CurUnoDialog.insertLabel("lblTableNames",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    8, sTableNames, 91, 82, IMAINSTEP, new Short(pretabindex++), 80
                });

        try
        {
            xTableListBox = CurUnoDialog.insertListBox("lstTableNames", 3, null, this,
                    new String[]
                    {
                        "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, "LineCount", PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        Boolean.TRUE, 12, "HID:WIZARDS_HID_DLGTABLE_LBTABLES", new Short(UnoDialog.getListBoxLineCount()), 91, 92, IMAINSTEP, new Short(pretabindex++), getListboxWidth()
                    });
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
        initializeCategory(BUSINESS);
    }

    public void selectCategory()
    {
        if (optBusiness.getState())
        {
            initializeCategory(BUSINESS);
        }
        else
        {
            initializeCategory(PRIVATE);
        }
    }

    public void initializeCategory(int _iCategory)
    {
        try
        {
            oCGCategory.initialize(
                _iCategory == PRIVATE ? "private" : "business");
            xTableListBox.removeItems((short) 0, xTableListBox.getItemCount());
            xTableListBox.addItems(oCGCategory.getTableNames(), (short) _iCategory);
            initializeTable(0);
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    public void initializeTable(int _iTable)
    {
        Helper.setUnoPropertyValue(UnoDialog.getModel(xTableListBox), PropertyNames.SELECTED_ITEMS, new short[]
                {
                    (short) _iTable
                });
        oCGTable.initialize(oCGCategory.xNameAccessTablesNode, _iTable);
        super.initialize(oCGTable.getFieldNames(bcolumnnameislimited, imaxcolumnchars), true);
    }

    public String[] getSelectedFieldNames()
    {
        String[] displayfieldnames = super.getSelectedFieldNames();
        Vector<String> afieldnameVector = new Vector<String>();
        int a = 0;
        for (int i = 0; i < displayfieldnames.length; i++)
        {
            try
            {
                FieldDescription ofielddescription = (FieldDescription) CurTableWizardUnoDialog.fielditems.get(displayfieldnames[i]);
                if (ofielddescription != null)
                {
                    afieldnameVector.addElement(ofielddescription.getName());
                    a++;
                }
            }
            catch (RuntimeException e)
            {
                e.printStackTrace(System.out);
            }
        }
        String[] fieldnames = new String[a];
        afieldnameVector.toArray(fieldnames);
        return fieldnames;
    }

    public boolean iscompleted()
    {
        return getSelectedFieldNames().length > 0;
    }

    public void addColumnsToDescriptor()
    {
        XNameAccess xNameAccessFieldNode;
        String[] fieldnames = getSelectedFieldNames();
        for (int i = 0; i < fieldnames.length; i++)
        {
            try
            {
                FieldDescription curfielddescription = (FieldDescription) CurTableWizardUnoDialog.fielditems.get(fieldnames[i]);
                PropertyValue[] aProperties = curfielddescription.getPropertyValues();
                this.curtabledescriptor.addColumn(aProperties);
            }
            catch (RuntimeException e)
            {
                e.printStackTrace(System.out);
            }
        }
    }

    public String getTableName()
    {
        return xTableListBox.getSelectedItem();
    }

    public String getFirstTableName()
    {
        String[] fieldnames = super.getSelectedFieldNames();
        if ((fieldnames.length) > 0)
        {
            for (int i = 0; i < CurTableWizardUnoDialog.fielditems.size(); i++)
            {
                String stablename = PropertyNames.EMPTY_STRING;
                try
                {
                    FieldDescription ofielddescription = (FieldDescription) CurTableWizardUnoDialog.fielditems.get(fieldnames[i]);
                    stablename = ofielddescription.gettablename();
                }
                catch (RuntimeException e)
                {
                    e.printStackTrace(System.out);
                }
                if (!stablename.equals(PropertyNames.EMPTY_STRING))
                {
                    return stablename;
                }
            }
        }
        return smytable;
    }

    public void fillupTables()
    {
//      if (optBusiness.getState())
//      else
    }

    public void fillupFieldsListbox()
    {
        super.emptyFieldsListBoxes();
    }


    /* (non-Javadoc)
     * @see com.sun.star.awt.XItemListener#itemStateChanged(com.sun.star.awt.ItemEvent)
     */
    public void itemStateChanged(ItemEvent arg0)
    {
        initializeTable((int) xTableListBox.getSelectedItemPos());
    }

    /* (non-Javadoc)
     * @see com.sun.star.lang.XEventListener#disposing(com.sun.star.lang.EventObject)
     */
    public void disposing(EventObject arg0)
    {
    }
    protected int ID;

    public int getID()
    {
        return ID;
    }

    public void setID(String sIncSuffix)
    {
    }

    public void shiftFromLeftToRight(String[] SelItems, String[] NewItems)
    {
        if (!CurTableWizardUnoDialog.verifyfieldcount(NewItems.length))
        {
            for (int i = 0; i < SelItems.length; i++)
            {
                int selindex = JavaTools.FieldInList(NewItems, SelItems[i]);
                super.xSelectedFieldsListBox.removeItems((short) selindex, (short) 1);
                /**TODO In this context the items should be better placed at their original position.
                 * but how is this position retrieved?
                 */
                super.xFieldsListBox.addItem(SelItems[i], xFieldsListBox.getItemCount());
            }
        }
        else
        {
            for (int i = 0; i < NewItems.length; i++)
            {
                int iduplicate;
                if (CurTableWizardUnoDialog.fielditems.containsKey(NewItems[i]))
                {
                    iduplicate = JavaTools.getDuplicateFieldIndex(NewItems, NewItems[i]);
                    if (iduplicate != -1)
                    {
                        XNameAccess xNameAccessFieldNode;
                        String sdisplayname = Desktop.getUniqueName(NewItems, NewItems[iduplicate], PropertyNames.EMPTY_STRING);
                        FieldDescription curfielddescription = new FieldDescription(xMSF, aLocale, this, sdisplayname, NewItems[iduplicate], imaxcolumnchars);
                        CurTableWizardUnoDialog.fielditems.put(sdisplayname, curfielddescription);
                        NewItems[iduplicate] = sdisplayname;
                        setSelectedFieldNames(NewItems);
                    }
                }
                else
                {
                    CurTableWizardUnoDialog.fielditems.put(NewItems[i], new FieldDescription(xMSF, aLocale, this, NewItems[i], NewItems[i], imaxcolumnchars));
                }
            }
        }
        CurTableWizardUnoDialog.setcompleted(TableWizard.SOMAINPAGE, NewItems.length > 0);
    }

    public void shiftFromRightToLeft(String[] SelItems, String[] NewItems)
    {
        for (int i = 0; i < SelItems.length; i++)
        {
            if (CurTableWizardUnoDialog.fielditems.containsKey(SelItems[i]))
            {
                CurTableWizardUnoDialog.fielditems.remove(SelItems[i]);
                this.curtabledescriptor.dropColumnbyName(SelItems[i]);
            }
        }
        CurTableWizardUnoDialog.setcompleted(TableWizard.SOMAINPAGE, NewItems.length > 0);
    }

    public void moveItemDown(String item)
    {
    }

    public void moveItemUp(String item)
    {
    }
}
