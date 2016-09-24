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

import com.sun.star.awt.TextEvent;
import com.sun.star.awt.XListBox;
import com.sun.star.awt.XRadioButton;
import com.sun.star.awt.XTextComponent;
import com.sun.star.sdbc.SQLException;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.db.TableDescriptor;
import com.sun.star.wizards.ui.*;
import com.sun.star.wizards.ui.event.XTextListenerAdapter;

public class Finalizer
{

    private TableWizard CurUnoDialog;
    private XRadioButton optModifyTable;
    private XRadioButton optWorkWithTable;
    private XTextComponent txtTableName;
    private XListBox xCatalogListBox;
    private XListBox xSchemaListBox;
    private TableDescriptor curtabledescriptor;
    private static int WORKWITHTABLEMODE = 0;
    public static int MODIFYTABLEMODE = 1;
    public static int STARTFORMWIZARDMODE = 2;

    public Finalizer(TableWizard _CurUnoDialog, TableDescriptor _curtabledescriptor)
    {
        try
        {
            this.CurUnoDialog = _CurUnoDialog;
            this.curtabledescriptor = _curtabledescriptor;
            short curtabindex = (short) (TableWizard.SOFINALPAGE * 100);
            Integer IFINALSTEP = Integer.valueOf(TableWizard.SOFINALPAGE);
            String slblTableName = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 34);
            String slblProceed = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 36);
            String sWorkWithTable = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 38);
            String sStartFormWizard = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 39);
            String sModifyTable = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 37);
            String sCongratulations = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 35);
            String slblCatalog = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 49);
            String slblSchema = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 50);
            String[] sCatalogNames = curtabledescriptor.getCatalogNames();
            String[] sSchemaNames = curtabledescriptor.getSchemaNames();
            int nListBoxPosX = 97;
            int ndiffPosY = 0;
            boolean bsupportsSchemata = false;
            boolean bsupportsCatalogs = false;

            CurUnoDialog.insertLabel("lblTableName",
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        UIConsts.INTEGERS[8], slblTableName, 97, 25, IFINALSTEP, 220
                    });
            txtTableName = CurUnoDialog.insertTextField("txtTableName", new XTextListenerAdapter() {
                        @Override
                        public void textChanged(TextEvent event) {
                            setCompletionFlag();
                        }
                    },
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Text", PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        UIConsts.INTEGER_12, "HID:WIZARDS_HID_DLGTABLE_TXT_NAME", 97, 35, IFINALSTEP, Short.valueOf(curtabindex++), PropertyNames.EMPTY_STRING, 223
                    });
            txtTableName.addTextListener(CurUnoDialog);
            txtTableName.setMaxTextLen((short) this.curtabledescriptor.getMaxTableNameLength());
            if (this.curtabledescriptor.xDBMetaData.supportsCatalogsInTableDefinitions())
            {
                if (sCatalogNames != null && sCatalogNames.length > 0)
                {
                    bsupportsCatalogs = true;
                    String sCatalog = PropertyNames.EMPTY_STRING;
                    try
                    {
                        sCatalog = curtabledescriptor.DBConnection.getCatalog();
                    }
                    catch (SQLException e1)
                    {
                        e1.printStackTrace(System.err);
                    }
                    CurUnoDialog.insertLabel("lblCatalog",
                            new String[]
                            {
                                PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                            },
                            new Object[]
                            {
                                8, slblCatalog, Integer.valueOf(nListBoxPosX), 52, IFINALSTEP, Short.valueOf(curtabindex++), 120
                            });

                    try
                    {
                        xCatalogListBox = CurUnoDialog.insertListBox("lstCatalog", null, null,
                                new String[]
                                {
                                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, "LineCount", PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.STRING_ITEM_LIST, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                                },
                                new Object[]
                                {
                                    Boolean.TRUE, 12, "HID:WIZARDS_HID_DLGTABLE_LST_CATALOG", Short.valueOf(UnoDialog.getListBoxLineCount()), Integer.valueOf(nListBoxPosX), 62, IFINALSTEP, sCatalogNames, Short.valueOf(curtabindex++), 80
                                });
                        int isel = JavaTools.FieldInList(sCatalogNames, sCatalog);
                        if (isel < 0)
                        {
                            isel = 0;
                        }
                        CurUnoDialog.setControlProperty("lstCatalog", PropertyNames.SELECTED_ITEMS, new short[]
                                {
                                    (short) isel
                                });
                    }
                    catch (Exception e)
                    {
                        e.printStackTrace(System.err);
                    }
                    nListBoxPosX = 200;
                }
            }
            if (this.curtabledescriptor.xDBMetaData.supportsSchemasInTableDefinitions())
            {
                if (sSchemaNames != null && sSchemaNames.length > 0)
                {
                    bsupportsSchemata = true;
                    String sSchema = PropertyNames.EMPTY_STRING;
                    try
                    {
                        sSchema = (String) curtabledescriptor.getDataSourcePropertySet().getPropertyValue("User");
                    }
                    catch (Exception e1)
                    {
                        e1.printStackTrace(System.err);
                    }
                    CurUnoDialog.insertLabel("lblSchema",
                            new String[]
                            {
                                PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                            },
                            new Object[]
                            {
                                8, slblSchema, Integer.valueOf(nListBoxPosX), 52, IFINALSTEP, Short.valueOf(curtabindex++), 80
                            });

                    try
                    {
                        xSchemaListBox = CurUnoDialog.insertListBox("lstSchema", null, null,
                                new String[]
                                {
                                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, "LineCount", PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.STRING_ITEM_LIST, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                                },
                                new Object[]
                                {
                                    Boolean.TRUE, 12, "HID:WIZARDS_HID_DLGTABLE_LST_SCHEMA", Short.valueOf(UnoDialog.getListBoxLineCount()), Integer.valueOf(nListBoxPosX), 62, IFINALSTEP, sSchemaNames, Short.valueOf(curtabindex++), 80
                                });
                        int isel = JavaTools.FieldInList(sSchemaNames, sSchema);
                        if (isel < 0)
                        {
                            isel = 0;
                        }
                        CurUnoDialog.setControlProperty("lstSchema", PropertyNames.SELECTED_ITEMS, new short[]
                                {
                                    (short) isel
                                });
                    }
                    catch (Exception e)
                    {
                        e.printStackTrace(System.err);
                    }
                }
            }
            if ((!bsupportsCatalogs) && (!bsupportsSchemata))
            {
                CurUnoDialog.insertLabel("lblcongratulations",
                        new String[]
                        {
                            PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                        },
                        new Object[]
                        {
                            16, sCongratulations, Boolean.TRUE, 97, 62, IFINALSTEP, Short.valueOf(curtabindex++), 226
                        });
            }
            else
            {
                ndiffPosY = 10;
            }
            CurUnoDialog.insertLabel("lblProceed",
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        UIConsts.INTEGERS[8], slblProceed, 97, Integer.valueOf(82 + ndiffPosY), IFINALSTEP, Short.valueOf(curtabindex++), 227
                    });
            optWorkWithTable = CurUnoDialog.insertRadioButton("optWorkWithTable", null,
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGTABLE_OPT_WORKWITHTABLE", sWorkWithTable, 101, Integer.valueOf(97 + ndiffPosY), Short.valueOf((short) 1), IFINALSTEP, Short.valueOf(curtabindex++), 177
                    });
            optModifyTable = CurUnoDialog.insertRadioButton("optModifyTable", null,
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGTABLE_OPT_MODIFYTABLE", sModifyTable, 101, Integer.valueOf(109 + ndiffPosY), IFINALSTEP, Short.valueOf(curtabindex++), 177
                    });
            CurUnoDialog.insertRadioButton("optStartFormWizard", null,
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGTABLE_OPT_STARTFORMWIZARD", sStartFormWizard, 101, Integer.valueOf(121 + ndiffPosY), IFINALSTEP, Short.valueOf(curtabindex++), 177
                    });
        }
        catch (SQLException e)
        {
            e.printStackTrace(System.err);
        }
    }

    public void initialize(String _firsttablename)
    {
        setTableName(_firsttablename);
    }

    public int finish()
    {
        if (optWorkWithTable.getState())
        {
            return WORKWITHTABLEMODE;
        }
        else if (optModifyTable.getState())
        {
            return MODIFYTABLEMODE;
        }
        else
        {
            return STARTFORMWIZARDMODE;
        }
    }

    public String getComposedTableName(String _stablename)
    {
        String scatalogname = null;
        String sschemaname = null;
        if (xCatalogListBox != null)
        {
            scatalogname = xCatalogListBox.getSelectedItem();
        }
        if (xSchemaListBox != null)
        {
            sschemaname = xSchemaListBox.getSelectedItem();
        }
        return curtabledescriptor.getComposedTableName(scatalogname, sschemaname, _stablename);
    }

    private void setTableName(String _tablename)
    {
        if (txtTableName.getText().equals(PropertyNames.EMPTY_STRING))
        {
            String ssuffix = Desktop.getIncrementSuffix(curtabledescriptor.getTableNamesAsNameAccess(), getComposedTableName(_tablename));
            txtTableName.setText(_tablename + ssuffix);
            setCompletionFlag();
        }
    }

    public String getTableName(String _firsttablename)
    {
        if (txtTableName.getText().equals(PropertyNames.EMPTY_STRING))
        {
            setTableName(_firsttablename);
        }
        return txtTableName.getText();
    }

    public String getSchemaName()
    {
        if (xSchemaListBox != null)
        {
            return this.xSchemaListBox.getSelectedItem();
        }
        else
        {
            return PropertyNames.EMPTY_STRING;
        }
    }

    public String getCatalogName()
    {
        if (xCatalogListBox != null)
        {
            return this.xCatalogListBox.getSelectedItem();
        }
        else
        {
            return PropertyNames.EMPTY_STRING;
        }
    }

    public boolean iscompleted()
    {
        return (txtTableName.getText().length() > 0);
    }

    private void setCompletionFlag()
    {
        CurUnoDialog.setcompleted(TableWizard.SOFINALPAGE, iscompleted());
    }

    public void setFocusToTableNameControl()
    {
        CurUnoDialog.setFocus("txtTableName");
    }
}
