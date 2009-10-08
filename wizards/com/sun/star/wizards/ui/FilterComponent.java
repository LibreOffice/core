/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FilterComponent.java,v $
 * $Revision: 1.14 $
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
package com.sun.star.wizards.ui;

// import java.util.Vector;
import com.sun.star.awt.ItemEvent;
import com.sun.star.awt.TextEvent;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XListBox;
import com.sun.star.awt.XRadioButton;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdbc.DataType;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.wizards.common.NumberFormatter;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.db.FieldColumn;
import com.sun.star.wizards.db.QueryMetaData;

public class FilterComponent
{

    private Integer IStep;
    private int iStartPosX;
    private int iStartPosY;
    // int Count;
    private int RowCount;
    // private int FilterCount;
    private static String[] sLogicOperators;
    // private /* public */ String[] FieldNames;
    private XRadioButton optMatchAll;
    private XRadioButton optMatchAny;
    // private String soptMatchAll;
    // private String soptMatchAny;
    // private String[] sHeadLines;
    private String slblFieldNames;
    private String slblOperators;
    private String slblValue;
    WizardDialog CurUnoDialog;
    private int BaseID = 2300;
    private String sIncSuffix;
    private ControlRow[] oControlRows;
    // private Vector FilterNames;
    private String sDuplicateCondition;
    final int SOLSTFIELDNAME = 3;
    final int SOLSTOPERATOR = 4;
    final int SOTXTVALUE = 5;
    final int SOOPTORMODE = 100;
    final int SOOPTANDMODE = 101;
    QueryMetaData oQueryMetaData;
    int iDateTimeFormat;
    int iDateFormat;
    int iTimeFormat;
    private PropertyValue[][] filterconditions;
    private short curtabindex;
    XMultiServiceFactory xMSF;
    final int SO_FIRSTFIELDNAME = 1;
    final int SO_SECONDFIELDNAME = 2;
    final int SO_THIRDFIELDNAME = 3;
    final int SO_FOURTHFIELDNAME = 4;
    int[] SO_FIELDNAMELIST = new int[]
    {
        SO_FIRSTFIELDNAME, SO_SECONDFIELDNAME, SO_THIRDFIELDNAME, SO_FOURTHFIELDNAME
    };
    final int SO_FIRSTCONDITION = 5;
    final int SO_SECONDCONDITION = 6;
    final int SO_THIRDCONDITION = 7;
    final int SO_FOURTHCONDITION = 8;
    int[] SO_CONDITIONLIST = new int[]
    {
        SO_FIRSTCONDITION, SO_SECONDCONDITION, SO_THIRDCONDITION, SO_FOURTHCONDITION
    };
    final int SO_FIRSTTEXTFIELD = 1;
    final int SO_SECONDTEXTFIELD = 2;
    final int SO_THIRDTEXTFIELD = 3;
    final int SO_FOURTHTEXTFIELD = 4;
    int[] SO_TEXTFIELDLIST = new int[]
    {
        SO_FIRSTTEXTFIELD, SO_SECONDTEXTFIELD, SO_THIRDTEXTFIELD, SO_FOURTHTEXTFIELD
    };
    final int SO_OPTQUERYMODE = 5;
    int SOI_MATCHALL = 0;
    int SOI_MATCHANY = 1;
    // int ifilterstate = SOI_MATCHALL;
    int curHelpID;

    class ItemListenerImpl implements com.sun.star.awt.XItemListener
    {

        public void itemStateChanged(com.sun.star.awt.ItemEvent EventObject)
        {
            try
            {
                int iKey = CurUnoDialog.getControlKey(EventObject.Source, CurUnoDialog.ControlList);
                String scontrolname = "";
                switch (iKey)
                {
                    //              case SOOPTQUERYMODE:
                    //                  getfilterstate();
                    case SO_FIRSTFIELDNAME:
                    case SO_SECONDFIELDNAME:
                    case SO_THIRDFIELDNAME:
                    case SO_FOURTHFIELDNAME:
                        scontrolname = getControlName(EventObject.Source);
                        String scontrolnameSuffix = sIncSuffix + "_" + scontrolname.substring(scontrolname.length() - 1, scontrolname.length());
                        XListBox xCurFieldListBox = (XListBox) UnoRuntime.queryInterface(XListBox.class, CurUnoDialog.xDlgContainer.getControl(scontrolname));
                        String CurDisplayFieldName = xCurFieldListBox.getSelectedItem();
                        FieldColumn CurFieldColumn = new FieldColumn(oQueryMetaData, CurDisplayFieldName);
                        XControl xValueControl = CurUnoDialog.xDlgContainer.getControl("txtValue" + scontrolnameSuffix);
                        XInterface xValueModel = (XInterface) UnoDialog.getModel(xValueControl);
                        Helper.setUnoPropertyValue(xValueModel, "TreatAsNumber", new Boolean(CurFieldColumn.bIsNumberFormat));
                        final NumberFormatter aNumberFormatter = oQueryMetaData.getNumberFormatter();
                        aNumberFormatter.setNumberFormat(xValueModel, CurFieldColumn.DBFormatKey, aNumberFormatter);
                        break;
                    case SO_FIRSTCONDITION:
                    case SO_SECONDCONDITION:
                    case SO_THIRDCONDITION:
                    case SO_FOURTHCONDITION:
                        scontrolname = getControlName(EventObject.Source);
                        break;
                    case SOOPTORMODE:
                    case SOOPTANDMODE:
                        // getfilterstate();
                        return;
                    default:
                        break;
                }
                togglefollowingControlRow(scontrolname);
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.out);
            }
        }

        public void disposing(com.sun.star.lang.EventObject eventObject)
        {
        }
    }

    class TextListenerImpl implements com.sun.star.awt.XTextListener
    {

        public void textChanged(TextEvent EventObject)
        {
            try
            {
                String sName = getControlName(EventObject.Source);
                togglefollowingControlRow(sName);
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.out);
            }
        }

        public void disposing(EventObject EventObject)
        {
        }
    }

    public void fieldconditionchanged(ItemEvent EventObject)
    {
        String sName = getControlName(EventObject.Source);
        togglefollowingControlRow(sName);
    }

    public void disposing(com.sun.star.lang.EventObject eventObject)
    {
    }

    class ActionListenerImpl implements com.sun.star.awt.XActionListener
    {

        public void disposing(EventObject eventObject)
        {
        }

        public void actionPerformed(com.sun.star.awt.ActionEvent actionEvent)
        {
            getfilterstate();
        }
    }

    private void togglefollowingControlRow(String _scurName)
    {
        // String sNameSuffix = sIncSuffix + "_" + _scurName.substring(_scurName.length() - 1, _scurName.length());
        int Index = Integer.valueOf(_scurName.substring(_scurName.length() - 1, _scurName.length())).intValue();
        if (Index < oControlRows.length)
        {
            boolean bValue = oControlRows[Index - 1].isComplete();
            oControlRows[Index].setEnabled(bValue);
        }
    }

    private String getControlName(Object _oSourceevent)
    {
        try
        {
            XControl xControl = (XControl) UnoRuntime.queryInterface(XControl.class, _oSourceevent);
            XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xControl.getModel());
            final String sName = AnyConverter.toString(xPSet.getPropertyValue("Name"));
            return sName;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
        return "";
    }

    public PropertyValue[][] getFilterConditions()
    {
        ControlRow CurControlRow;
        Object curValue;
        // getfilterstate();
        int filtercount = getFilterCount();
        if (filtercount > 0)
        {
            if (this.getfilterstate() == this.SOI_MATCHALL)
            {
                filterconditions = new PropertyValue[1][filtercount];
            }
            else
            {
                filterconditions = new PropertyValue[filtercount][1];
            }
            int a = 0;
            for (int i = 0; i < RowCount; i++)
            {
                CurControlRow = oControlRows[i];
                if (CurControlRow.isEnabled())
                {
                    if (CurControlRow.isComplete())
                    {
                        String curFieldName = CurControlRow.getSelectedFieldName();
                        int curOperator = (int) CurControlRow.getSelectedOperator() + 1;
                        FieldColumn CurFieldColumn = oQueryMetaData.getFieldColumnByDisplayName(curFieldName);
                        if (CurFieldColumn.StandardFormatKey == oQueryMetaData.getNumberFormatter().getTextFormatKey())
                        {
                            curValue = "'" + CurControlRow.getValue() + "'";
                        }
//// TODO the following code is bound to be deprecated as soon as the query composer is able to handle date/time values as numbers
                        else if ((CurFieldColumn.StandardFormatKey == oQueryMetaData.getNumberFormatter().getDateFormatKey()) ||
                                (CurFieldColumn.StandardFormatKey == oQueryMetaData.getNumberFormatter().getDateTimeFormatKey()))
                        {
                            String sDate = CurControlRow.getDateTimeString(true);
                            curValue = "{D '" + sDate + "' }";  // FormatsSupplier
                        }
                        else if (CurFieldColumn.StandardFormatKey == oQueryMetaData.getNumberFormatter().getTimeFormatKey())
                        {
                            String sTime = CurControlRow.getDateTimeString(true);
                            curValue = "'{T '" + sTime + "' }";
                        }
                        else
                        {
                            curValue = CurControlRow.getValue();
                            switch (CurFieldColumn.FieldType)
                            {
                                case DataType.TINYINT:
                                case DataType.BIGINT:
                                case DataType.INTEGER:
                                case DataType.SMALLINT:
                                    curValue = String.valueOf(((Double) curValue).intValue());
                                    break;
                                case DataType.BIT:
                                case DataType.BOOLEAN:
                                    // curValue = CurControlRow.getText(); // wrong! (creates something like 'WAHR'/'FALSCH' if german locale is used.

                                    // double dblvalue = ((Double) curValue).doubleValue();
                                    //curValue = new Boolean(dblvalue == 1.0); // wrong! we need a string, not a boolean value

                                    // converts the '1.0'/'0.0' (EffectiveValue) to a 'boolean' String like 'true'/'false'
                                    curValue = String.valueOf(((Double) curValue).intValue() == 1);
                                    break;
                                default:
                                    curValue = String.valueOf(curValue);
                                    break;
                            }
                        }

                        PropertyValue oPropertyValue = Properties.createProperty(curFieldName, curValue, curOperator);
                        if (getfilterstate()/*this.ifilterstate*/ == this.SOI_MATCHALL)
                        {
                            if (i == 0)
                            {
                                filterconditions[0] = new PropertyValue[filtercount];
                            }
                            filterconditions[0][a] = oPropertyValue;
                        }
                        else
                        {
                            filterconditions[a][0] = oPropertyValue;
                        }
                        a++;
                    }
                }
            }
            int[] iduplicate = JavaTools.getDuplicateFieldIndex(filterconditions);
            if (iduplicate[0] != -1)
            {
                PropertyValue aduplicatecondition = filterconditions[iduplicate[0]][iduplicate[1]];
                String smsgDuplicateCondition = getDisplayCondition(sDuplicateCondition, aduplicatecondition, null);
                CurUnoDialog.showMessageBox("WarningBox", VclWindowPeerAttribute.OK, smsgDuplicateCondition);
                CurUnoDialog.vetoableChange(new java.beans.PropertyChangeEvent(CurUnoDialog, "Steps", new Integer(1), new Integer(2)));
                return new PropertyValue[][]
                        {
                        };
            }
        }
        else
        {
            filterconditions = new PropertyValue[0][0];
        }
        return filterconditions;
    }

    public static String getDisplayCondition(String _BaseString, PropertyValue _filtercondition, QueryMetaData _CurDBMetaData)
    {
        try
        {
            String FieldName;
            if (_CurDBMetaData != null)
            {
                FieldColumn CurDBFieldColumn = _CurDBMetaData.getFieldColumnByDisplayName(_filtercondition.Name);
                FieldName = CurDBFieldColumn.getFieldTitle();
            }
            else
            {
                FieldName = _filtercondition.Name;
            }
            String sreturn = JavaTools.replaceSubString(_BaseString, FieldName, "<FIELDNAME>");
            String soperator = sLogicOperators[_filtercondition.Handle - 1];
            sreturn = JavaTools.replaceSubString(sreturn, soperator, "<LOGICOPERATOR>");
            String sDisplayValue = AnyConverter.toString(_filtercondition.Value);
            sreturn = JavaTools.replaceSubString(sreturn, sDisplayValue, "<VALUE>");
            return sreturn;
        }
        catch (IllegalArgumentException e)
        {
            e.printStackTrace(System.out);
        }
        return "";
    }

    private int getfilterstate()
    {
        boolean bisany = true;
        int ifilterstate = SOI_MATCHALL;
        bisany = (this.optMatchAny.getState()) == true;
        if (bisany)
        {
            ifilterstate = SOI_MATCHANY;
        }
        else
        {
            ifilterstate = SOI_MATCHALL;
        }
        return ifilterstate;
    }

    private void addfiltercondition(int _index, String _curFieldName, Object _curValue, int _curOperator)
    {
        try
        {
            String ValString = String.valueOf(_curValue);
            PropertyValue oPropertyValue = Properties.createProperty(_curFieldName, ValString, _curOperator);
            getfilterstate();
            if (getfilterstate() == this.SOI_MATCHALL)
            {
                if (_index == 0)
                {
                    filterconditions[0] = new PropertyValue[getFilterCount()];
                }
                filterconditions[0][_index] = new PropertyValue();
                filterconditions[0][_index] = oPropertyValue;
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    private int getFilterCount()
    {
        int a = 0;
        for (int i = 0; i < RowCount; i++)
        {
            if (oControlRows[i].isEnabled() && oControlRows[i].isComplete())
            {
                a++;
            }
        }
        return a;
    // FilterCount = a;
    // return FilterCount;
    }

    /** Creates a new instance of FilterComponent
     * @param CurUnoDialog
     * @param _xMSF
     * @param iStep
     * @param iPosX
     * @param iPosY
     * @param iWidth
     * @param FilterCount
     * @param _oQueryMetaData
     * @param _firstHelpID
     */
    public FilterComponent(WizardDialog CurUnoDialog, XMultiServiceFactory _xMSF, int iStep, int iPosX, int iPosY, int iWidth, int FilterCount, QueryMetaData _oQueryMetaData, int _firstHelpID)
    {
        try
        {
            this.curHelpID = _firstHelpID;
            this.xMSF = _xMSF;
            this.IStep = new Integer(iStep);

            curtabindex = UnoDialog.setInitialTabindex(iStep);
            this.CurUnoDialog = CurUnoDialog;
            this.RowCount = FilterCount;
            this.oQueryMetaData = _oQueryMetaData;
            boolean bEnabled;
            sIncSuffix = com.sun.star.wizards.common.Desktop.getIncrementSuffix(CurUnoDialog.xDlgNameAccess, "optMatchAll");
            iStartPosX = iPosX;
            iStartPosY = iPosY;

            String soptMatchAll = CurUnoDialog.m_oResource.getResText(BaseID + 9);
            String soptMatchAny = CurUnoDialog.m_oResource.getResText(BaseID + 10);
            slblFieldNames = CurUnoDialog.m_oResource.getResText(BaseID + 17);
            slblOperators = CurUnoDialog.m_oResource.getResText(BaseID + 24);
            slblValue = CurUnoDialog.m_oResource.getResText(BaseID + 25);
            sLogicOperators = CurUnoDialog.m_oResource.getResArray(BaseID + 26, 7);

            sDuplicateCondition = CurUnoDialog.m_oResource.getResText(BaseID + 89);

            optMatchAll = CurUnoDialog.insertRadioButton("optMatchAll" + sIncSuffix, SOOPTANDMODE, new ItemListenerImpl(),
                    new String[]
                    {
                        "Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"
                    },
                    new Object[]
                    {
                        new Integer(9), "HID:" + curHelpID++, soptMatchAll, new Integer(iPosX), new Integer(iStartPosY), new Short((short) 1), IStep, new Short(curtabindex++), new Integer(203)
                    });
            optMatchAny = CurUnoDialog.insertRadioButton("optMatchAny" + sIncSuffix, SOOPTORMODE, new ItemListenerImpl(),
                    new String[]
                    {
                        "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"
                    },
                    new Object[]
                    {
                        new Integer(9), "HID:" + curHelpID++, soptMatchAny, new Integer(iPosX), new Integer(iStartPosY + 12), IStep, new Short(curtabindex++), new Integer(203)
                    });
            getfilterstate();
            oControlRows = new ControlRow[FilterCount];
            for (int i = 0; i < FilterCount; i++)
            {
                bEnabled = (i == 0);
                oControlRows[i] = new ControlRow(iStartPosY + 20, i, bEnabled, (this.curHelpID + (i * 3)));
                iStartPosY += 43;
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public void initialize(PropertyValue[][] _filterconditions, String[] _fieldnames)
    {
        for (int i = 0; i < RowCount; i++)
        {
            oControlRows[i].setFieldNames(_fieldnames);
        }
        this.filterconditions = _filterconditions;
        PropertyValue[] curfilterconditions;
        int a;
        if (_filterconditions.length == 1)
        {
            curfilterconditions = filterconditions[0];
            for (a = 0; a < curfilterconditions.length; a++)
            {
                oControlRows[a].setCondition(filterconditions[0][a]);
            }
        }
        else
        {
            for (a = 0; a < filterconditions.length; a++)
            {
                oControlRows[a].setCondition(filterconditions[a][0]);
            }
        }
        while (a < oControlRows.length)
        {
            oControlRows[a].settovoid();
            boolean bdoenable;
            if (a > 0)
            {
                bdoenable = oControlRows[a - 1].isComplete();
            }
            else
            {
                bdoenable = true;
            }
            oControlRows[a].setEnabled(bdoenable);
            a++;
        }
    }

    public void addNumberFormats()
    {
        try
        {
            iDateFormat = oQueryMetaData.getNumberFormatter().defineNumberFormat("YYYY-MM-DD");
            iTimeFormat = oQueryMetaData.getNumberFormatter().defineNumberFormat("HH:MM:SS");
            iDateTimeFormat = oQueryMetaData.getNumberFormatter().defineNumberFormat("YYYY-MM-DD HH:MM:SS");
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    class ControlRow
    {

        protected XInterface[] ControlElements = new XInterface[6];
        private boolean bEnabled;
        String[] FieldNames;

        protected ControlRow(int iCompPosY, int Index, boolean _bEnabled, int _firstRowHelpID)
        {
            try
            {
                String sCompSuffix = sIncSuffix + "_" + String.valueOf(Index + 1);
                this.bEnabled = _bEnabled;
                ControlElements[0] = (XInterface) CurUnoDialog.insertLabel("lblFieldNames" + sCompSuffix,
                        new String[]
                        {
                            "Enabled", "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"
                        },
                        new Object[]
                        {
                            new Boolean(bEnabled), new Integer(9), slblFieldNames, new Integer(iStartPosX + 10), new Integer(iCompPosY + 13), IStep, new Short(curtabindex++), new Integer(55)
                        });
                ControlElements[1] = (XInterface) CurUnoDialog.insertLabel("lblOperators" + sCompSuffix,
                        new String[]
                        {
                            "Enabled", "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"
                        },
                        new Object[]
                        {
                            new Boolean(bEnabled), new Integer(9), slblOperators, new Integer(iStartPosX + 87), new Integer(iCompPosY + 13), IStep, new Short(curtabindex++), new Integer(52)
                        });
                ControlElements[2] = (XInterface) CurUnoDialog.insertLabel("lblValue" + sCompSuffix,
                        new String[]
                        {
                            "Enabled", "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"
                        },
                        new Object[]
                        {
                            new Boolean(bEnabled), new Integer(9), slblValue, new Integer(iStartPosX + 162), new Integer(iCompPosY + 13), IStep, new Short(curtabindex++), new Integer(44)
                        });
                ControlElements[3] = (XInterface) CurUnoDialog.insertListBox("lstFieldName" + sCompSuffix, SO_FIELDNAMELIST[Index], null, new ItemListenerImpl(),
                        new String[]
                        {
                            "Enabled", "Dropdown", "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width"
                        },
                        new Object[]
                        {
                            new Boolean(bEnabled), Boolean.TRUE, new Integer(13), "HID:" + _firstRowHelpID++, new Integer(iStartPosX + 10), new Integer(iCompPosY + 23), IStep, new Short(curtabindex++), new Integer(71)
                        });
                ControlElements[4] = (XInterface) CurUnoDialog.insertListBox("lstOperator" + sCompSuffix, SO_CONDITIONLIST[Index], null, new ItemListenerImpl(),
                        new String[]
                        {
                            "Enabled", "Dropdown", "Height", "HelpURL", "LineCount", "PositionX", "PositionY", "Step", "StringItemList", "TabIndex", "Width"
                        },
                        new Object[]
                        {
                            new Boolean(bEnabled), Boolean.TRUE, new Integer(13), "HID:" + _firstRowHelpID++, new Short((short) 7), new Integer(iStartPosX + 87), new Integer(iCompPosY + 23), IStep, sLogicOperators, new Short(curtabindex++), new Integer(70)
                        });
                ControlElements[5] = (XInterface) CurUnoDialog.insertFormattedField("txtValue" + sCompSuffix, SO_TEXTFIELDLIST[Index], new TextListenerImpl(),
                        new String[]
                        {
                            "Enabled", "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width"
                        },
                        new Object[]
                        {
                            new Boolean(bEnabled), new Integer(13), "HID:" + _firstRowHelpID++, new Integer(iStartPosX + 162), new Integer(iCompPosY + 23), IStep, new Short(curtabindex++), new Integer(44)
                        });
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.out);
            }
        }

        boolean isComplete()
        {
            try
            {
                short[] SelFields = (short[]) AnyConverter.toArray(Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTFIELDNAME]), "SelectedItems"));
                if (SelFields.length > 0)
                {
                    short[] SelOperator = (short[]) AnyConverter.toArray(Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTOPERATOR]), "SelectedItems"));
                    if (SelOperator.length > 0)
                    {
                        Object oValue = Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), "EffectiveValue");
                        if (!AnyConverter.isVoid(oValue))
                        {
                            String sValue = (String.valueOf(oValue));
                            return (!sValue.equals(""));
                        }
                    }
                }
                return false;
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.out);
                return false;
            }
        }

        public void fieldnamechanged(ItemEvent EventObject)
        {
            int i = 0;
        }

        protected void setCondition(PropertyValue _filtercondition)
        {
            try
            {
                int ikey;
                XListBox xFieldsListBox = (XListBox) UnoRuntime.queryInterface(XListBox.class, ControlElements[SOLSTFIELDNAME]);
                xFieldsListBox.selectItem(_filtercondition.Name, true);
                XListBox xOperatorListBox = (XListBox) UnoRuntime.queryInterface(XListBox.class, ControlElements[SOLSTOPERATOR]);
                xOperatorListBox.selectItemPos((short) (_filtercondition.Handle - 1), true);

                if (AnyConverter.isString(_filtercondition.Value))
                {
                    String sValue = AnyConverter.toString(_filtercondition.Value);
                    if (sValue.indexOf("{D '") > -1)
                    {
                        sValue = JavaTools.replaceSubString(sValue, "", "{D '");
                        sValue = JavaTools.replaceSubString(sValue, "", "' }");
                        oQueryMetaData.getNumberFormatter().convertStringToNumber(iDateFormat, sValue);
                    }
                    else if (sValue.indexOf("{T '") > -1)
                    {
                        sValue = JavaTools.replaceSubString(sValue, "", "{T '");
                        sValue = JavaTools.replaceSubString(sValue, "", "' }");
                        oQueryMetaData.getNumberFormatter().convertStringToNumber(iTimeFormat, sValue);
                    }
                }
                else if (AnyConverter.isBoolean(_filtercondition.Value))
                {
                    boolean b = AnyConverter.toBoolean(_filtercondition.Value);
                    double nValue = 0.0;
                    if (b)
                    {
                        nValue = 1.0;
                    }
                    Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), "EffectiveValue", nValue);
                }
                else
                {
                    Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), "EffectiveValue", _filtercondition.Value);
                }
            }
            catch (Exception e)
            {
                e.printStackTrace(System.out);
            }
        }

        protected void setFieldNames(String[] _FieldNames)
        {
            Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTFIELDNAME]), "StringItemList", _FieldNames);
            FieldNames = _FieldNames;
        }

        protected boolean isEnabled()
        {
            return bEnabled;
        }

        protected void settovoid()
        {
            CurUnoDialog.deselectListBox(ControlElements[SOLSTFIELDNAME]);
            CurUnoDialog.deselectListBox(ControlElements[SOLSTOPERATOR]);
            Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), "EffectiveValue", com.sun.star.uno.Any.VOID);
        }

        protected void setEnabled(boolean _bEnabled)
        {
            for (int i = 0; i < ControlElements.length; i++)
            {
                Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[i]), "Enabled", new Boolean(_bEnabled));
            }
            bEnabled = _bEnabled;
            if (bEnabled)
            {
                short[] iselected = new short[]
                {
                };
                try
                {
                    iselected = (short[]) AnyConverter.toArray(Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTOPERATOR]), "SelectedItems"));
                }
                catch (Exception e)
                {
                    e.printStackTrace(System.out);
                }
                if ((iselected.length) == 0)
                {
                    Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTOPERATOR]), "SelectedItems", new short[]
                            {
                                0
                            });
                }
            }
            else if (!isComplete())
            {
                CurUnoDialog.deselectListBox(ControlElements[SOLSTOPERATOR]);
            }
        }

        protected String getSelectedFieldName()
        {
            try
            {
                short[] SelFields = (short[]) AnyConverter.toArray(Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTFIELDNAME]), "SelectedItems"));
                return FieldNames[SelFields[0]];
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.out);
            }
            return null;
        }

        // TODO: check if it is really useful to match the indices of the listbox the API constants
        protected short getSelectedOperator()
        {
            try
            {
                short[] SelFields = (short[]) AnyConverter.toArray(Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTOPERATOR]), "SelectedItems"));
                return SelFields[0];
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.out);
            }
            return -1;
        }

        // TODO make a difference between Text and Numbers
        protected Object getValue()
        {
            try
            {
                return (Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), "EffectiveValue"));
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.out);
            }
            return null;
        }

        protected Object getText()
        {
            try
            {
                return (Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), "Text"));
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.out);
            }
            return null;
        }

        protected String getDateTimeString(boolean bgetDate)
        {
            try
            {
                double dblValue = ((Double) getValue()).doubleValue();
                NumberFormatter oNumberFormatter = oQueryMetaData.getNumberFormatter();
                return oNumberFormatter.convertNumberToString(iDateTimeFormat, dblValue);
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.out);
                return null;
            }
        }
    }
}//  com.sun.star.sdb.SQLFilterOperator.EQUAL
//  com.sun.star.sdb.SQLFilterOperator.NOT_EQUAL
//  com.sun.star.sdb.SQLFilterOperator.LESS
//  com.sun.star.sdb.SQLFilterOperator.GREATER
//  com.sun.star.sdb.SQLFilterOperator.LESS_EQUAL
//  com.sun.star.sdb.SQLFilterOperator.GREATER_EQUAL
//  com.sun.star.sdb.SQLFilterOperator.LIKE
//  com.sun.star.sdb.SQLFilterOperator.NOT_LIKE
//  com.sun.star.sdb.SQLFilterOperator.SQLNULL
//  com.sun.star.sdb.SQLFilterOperator.NOT_SQLNULL

/*  constants SQLFilterOperator
{
/// equal to
const long EQUAL            = 1;
/// not equal to
const long NOT_EQUAL        = 2;
/// less than
const long LESS             = 3;
/// greater than
const long GREATER          = 4;
/// less or eqal than
const long LESS_EQUAL       = 5;
/// greater or eqal than
const long GREATER_EQUAL    = 6;
/// like
const long LIKE             = 7;
/// not like
const long NOT_LIKE         = 8;
/// is null
const long SQLNULL          = 9;
/// is not null
const long NOT_SQLNULL      = 10;
}; */
