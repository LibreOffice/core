/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Finalizer.java,v $
 * $Revision: 1.10 $
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

import com.sun.star.awt.FocusEvent;
import com.sun.star.awt.TextEvent;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.XFocusListener;
import com.sun.star.awt.XListBox;
import com.sun.star.awt.XRadioButton;
import com.sun.star.awt.XTextComponent;
import com.sun.star.awt.XTextListener;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.sdbc.SQLException;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Configuration;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.SystemDialog;
import com.sun.star.wizards.db.DBMetaData;
import com.sun.star.wizards.db.TableDescriptor;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.*;

public class Finalizer
{

    TableWizard CurUnoDialog;
    short curtabindex;
    XRadioButton optModifyTable;
    XRadioButton optWorkWithTable;
    XRadioButton optStartFormWizard;
    XTextComponent txtTableName;
    XListBox xCatalogListBox;
    XListBox xSchemaListBox;
    TableDescriptor curtabledescriptor;
    public String SETCOMPLETIONFLAG = "setCompletionFlag";
    public static int WORKWITHTABLEMODE = 0;
    public static int MODIFYTABLEMODE = 1;
    public static int STARTFORMWIZARDMODE = 2;

    public Finalizer(TableWizard _CurUnoDialog, TableDescriptor _curtabledescriptor)
    {
        try
        {
            this.CurUnoDialog = _CurUnoDialog;
            this.curtabledescriptor = _curtabledescriptor;
            curtabindex = (short) (TableWizard.SOFINALPAGE * 100);
            Integer IFINALSTEP = new Integer(TableWizard.SOFINALPAGE);
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
                        "Height", "Label", "PositionX", "PositionY", "Step", "Width"
                    },
                    new Object[]
                    {
                        UIConsts.INTEGERS[8], slblTableName, new Integer(97), new Integer(25), IFINALSTEP, new Integer(220)
                    });
            txtTableName = CurUnoDialog.insertTextField("txtTableName", SETCOMPLETIONFLAG, this,
                    new String[]
                    {
                        "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Text", "Width"
                    },
                    new Object[]
                    {
                        UIConsts.INTEGER_12, "HID:41240", new Integer(97), new Integer(35), IFINALSTEP, new Short(curtabindex++), "", new Integer(223)
                    });
            txtTableName.addTextListener(CurUnoDialog);
            txtTableName.setMaxTextLen((short) this.curtabledescriptor.getMaxTableNameLength());
            if (this.curtabledescriptor.xDBMetaData.supportsCatalogsInTableDefinitions())
            {
                if (sCatalogNames != null)
                {
                    if (sCatalogNames.length > 0)
                    {
                        bsupportsCatalogs = true;
                        String sCatalog = "";
                        try
                        {
                            sCatalog = curtabledescriptor.DBConnection.getCatalog();
                        }
                        catch (SQLException e1)
                        {
                            e1.printStackTrace(System.out);
                        }
                        CurUnoDialog.insertLabel("lblCatalog",
                                new String[]
                                {
                                    "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"
                                },
                                new Object[]
                                {
                                    new Integer(8), slblCatalog, new Integer(nListBoxPosX), new Integer(52), IFINALSTEP, new Short(curtabindex++), new Integer(120)
                                });

                        try
                        {
                            xCatalogListBox = CurUnoDialog.insertListBox("lstCatalog", null, null,
                                    new String[]
                                    {
                                        "Dropdown", "Height", "HelpURL", "LineCount", "PositionX", "PositionY", "Step", "StringItemList", "TabIndex", "Width"
                                    },
                                    new Object[]
                                    {
                                        Boolean.TRUE, new Integer(12), "HID:41244", new Short(UnoDialog.getListBoxLineCount()), new Integer(nListBoxPosX), new Integer(62), IFINALSTEP, sCatalogNames, new Short(curtabindex++), new Integer(80)
                                    });
                            int isel = JavaTools.FieldInList(sCatalogNames, sCatalog);
                            if (isel < 0)
                            {
                                isel = 0;
                            }
                            CurUnoDialog.setControlProperty("lstCatalog", "SelectedItems", new short[]
                                    {
                                        (short) isel
                                    });
                        }
                        catch (Exception e)
                        {
                            e.printStackTrace(System.out);
                        }
                        nListBoxPosX = 200;
                    }
                }
            }
            if (this.curtabledescriptor.xDBMetaData.supportsSchemasInTableDefinitions())
            {
                if (sSchemaNames != null)
                {
                    if (sSchemaNames.length > 0)
                    {
                        bsupportsSchemata = true;
                        String sSchema = "";
                        try
                        {
                            sSchema = (String) curtabledescriptor.xDataSourcePropertySet.getPropertyValue("User");
                        }
                        catch (Exception e1)
                        {
                            e1.printStackTrace(System.out);
                        }
                        CurUnoDialog.insertLabel("lblSchema",
                                new String[]
                                {
                                    "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"
                                },
                                new Object[]
                                {
                                    new Integer(8), slblSchema, new Integer(nListBoxPosX), new Integer(52), IFINALSTEP, new Short(curtabindex++), new Integer(80)
                                });

                        try
                        {
                            xSchemaListBox = CurUnoDialog.insertListBox("lstSchema", null, null,
                                    new String[]
                                    {
                                        "Dropdown", "Height", "HelpURL", "LineCount", "PositionX", "PositionY", "Step", "StringItemList", "TabIndex", "Width"
                                    },
                                    new Object[]
                                    {
                                        Boolean.TRUE, new Integer(12), "HID:41245", new Short(UnoDialog.getListBoxLineCount()), new Integer(nListBoxPosX), new Integer(62), IFINALSTEP, sSchemaNames, new Short(curtabindex++), new Integer(80)
                                    });
                            int isel = JavaTools.FieldInList(sSchemaNames, sSchema);
                            if (isel < 0)
                            {
                                isel = 0;
                            }
                            CurUnoDialog.setControlProperty("lstSchema", "SelectedItems", new short[]
                                    {
                                        (short) isel
                                    });
                        }
                        catch (Exception e)
                        {
                            e.printStackTrace(System.out);
                        }
                    }
                }
            }
            if ((!bsupportsCatalogs) && (!bsupportsSchemata))
            {
                CurUnoDialog.insertLabel("lblcongratulations",
                        new String[]
                        {
                            "Height", "Label", "MultiLine", "PositionX", "PositionY", "Step", "TabIndex", "Width"
                        },
                        new Object[]
                        {
                            new Integer(16), sCongratulations, Boolean.TRUE, new Integer(97), new Integer(62), IFINALSTEP, new Short(curtabindex++), new Integer(226)
                        });
            }
            else
            {
                ndiffPosY = 10;
            }
            CurUnoDialog.insertLabel("lblProceed",
                    new String[]
                    {
                        "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"
                    },
                    new Object[]
                    {
                        UIConsts.INTEGERS[8], slblProceed, new Integer(97), new Integer(82 + ndiffPosY), IFINALSTEP, new Short(curtabindex++), new Integer(227)
                    });
            optWorkWithTable = CurUnoDialog.insertRadioButton("optWorkWithTable", null,
                    new String[]
                    {
                        "Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"
                    },
                    new Object[]
                    {
                        UIConsts.INTEGERS[8], "HID:41242", sWorkWithTable, new Integer(101), new Integer(97 + ndiffPosY), new Short((short) 1), IFINALSTEP, new Short(curtabindex++), new Integer(177)
                    });
            optModifyTable = CurUnoDialog.insertRadioButton("optModifyTable", null,
                    new String[]
                    {
                        "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"
                    },
                    new Object[]
                    {
                        UIConsts.INTEGERS[8], "HID:41241", sModifyTable, new Integer(101), new Integer(109 + ndiffPosY), IFINALSTEP, new Short(curtabindex++), new Integer(177)
                    });
            optStartFormWizard = CurUnoDialog.insertRadioButton("optStartFormWizard", null,
                    new String[]
                    {
                        "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"
                    },
                    new Object[]
                    {
                        UIConsts.INTEGERS[8], "HID:41243", sStartFormWizard, new Integer(101), new Integer(121 + ndiffPosY), IFINALSTEP, new Short(curtabindex++), new Integer(177)
                    });
        }
        catch (SQLException e)
        {
            e.printStackTrace(System.out);
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

    public void setTableName(String _tablename)
    {
        if (txtTableName.getText().equals(""))
        {
            String ssuffix = Desktop.getIncrementSuffix(curtabledescriptor.getTableNamesAsNameAccess(), getComposedTableName(_tablename));
            txtTableName.setText(_tablename + ssuffix);
            setCompletionFlag();
        }
    }

    public String getTableName()
    {
        return txtTableName.getText();
    }

    public String getTableName(String _firsttablename)
    {
        if (txtTableName.getText().equals(""))
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
            return "";
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
            return "";
        }
    }

    public boolean iscompleted()
    {
        return (txtTableName.getText().length() > 0);
    }

    public void setCompletionFlag()
    {
        CurUnoDialog.setcompleted(TableWizard.SOFINALPAGE, iscompleted());
    }

    public void setFocusToTableNameControl()
    {
        CurUnoDialog.setFocus("txtTableName");
    }
}
