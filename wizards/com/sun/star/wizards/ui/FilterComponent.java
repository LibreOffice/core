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
    // private int iStartPosX;
    // private int iStartPosY;
    // int Count;
    private int RowCount;
    // private int FilterCount;
    private static String[] sLogicOperators;
    private static String[] sBooleanValues;
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
    final int SO_FIRSTBOOLFIELDNAME = 256 + 1;
    final int SO_SECONDBOOLFIELDNAME = 256 + 2;
    final int SO_THIRDBOOLFIELDNAME = 256 + 3;
    final int SO_FOURTHBOOLFIELDNAME = 256 + 4;
    int SO_BOOLEANLIST[] = {
        SO_FIRSTBOOLFIELDNAME, SO_SECONDBOOLFIELDNAME, SO_THIRDBOOLFIELDNAME, SO_FOURTHBOOLFIELDNAME
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
                String sControlName = "";
                switch (iKey)
                {
                    //              case SOOPTQUERYMODE:
                    //                  getfilterstate();
                    case SO_FIRSTFIELDNAME:
                    case SO_SECONDFIELDNAME:
                    case SO_THIRDFIELDNAME:
                    case SO_FOURTHFIELDNAME:
                        sControlName = getControlName(EventObject.Source);
                        String sControlNameSuffix = sIncSuffix + "_" + getIndexNumber(sControlName);
                        XListBox xCurFieldListBox = (XListBox) UnoRuntime.queryInterface(XListBox.class, CurUnoDialog.xDlgContainer.getControl(sControlName));
                        String CurDisplayFieldName = xCurFieldListBox.getSelectedItem();
                        FieldColumn CurFieldColumn = new FieldColumn(oQueryMetaData, CurDisplayFieldName);

                        String sControlNameTextValue = "txtValue" + sControlNameSuffix;
//                        String sControlNameBooleanList = "lstBoolean" + sControlNameSuffix;
//                        if (aFieldColumn.FieldType == DataType.BOOLEAN)
//                        {
//                            // scheint aufgrund eines Fehlers in Toolkit nicht zu funktionieren
//                            CurUnoDialog.setControlVisible(sControlNameTextValue, false);
//                            CurUnoDialog.setControlVisible(sControlNameBooleanList, true);
//                        }
//                        else
//                        {
//                            CurUnoDialog.setControlVisible(sControlNameTextValue, true);
//                            CurUnoDialog.setControlVisible(sControlNameBooleanList, false);

                            XControl xValueControl = CurUnoDialog.xDlgContainer.getControl(sControlNameTextValue);
                            XInterface xValueModel = (XInterface) UnoDialog.getModel(xValueControl);
                            Helper.setUnoPropertyValue(xValueModel, "TreatAsNumber", Boolean.valueOf(CurFieldColumn.isNumberFormat()));
                            final NumberFormatter aNumberFormatter = oQueryMetaData.getNumberFormatter();
                            aNumberFormatter.setNumberFormat(xValueModel, CurFieldColumn.getDBFormatKey(), aNumberFormatter);
//                         }

                        break;
                    case SO_FIRSTCONDITION:
                    case SO_SECONDCONDITION:
                    case SO_THIRDCONDITION:
                    case SO_FOURTHCONDITION:
                        sControlName = getControlName(EventObject.Source);
                        break;
                    case SOOPTORMODE:
                    case SOOPTANDMODE:
                        // getfilterstate();
                        return;

                case SO_FIRSTBOOLFIELDNAME:
                case SO_SECONDBOOLFIELDNAME:
                case SO_THIRDBOOLFIELDNAME:
                case SO_FOURTHBOOLFIELDNAME:
                        sControlName = getControlName(EventObject.Source);
                    break;

                    default:
                        break;
                }
                togglefollowingControlRow(sControlName);
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

    public static String getIndexNumber(String _sStr)
        {
            String sLastNumber = _sStr.substring(_sStr.length() - 1, _sStr.length());
            return sLastNumber;
        }

    /**
     * Enable the next ControlRow if the Condition is complete in the current line
     *
     * @param _scurName every control name ends with something like _1, _2, _3 this is the index number
     *        we try to interpret to check which line we currently use. (number - 1)
     */
    private void togglefollowingControlRow(String _scurName)
    {
        // String sNameSuffix = sIncSuffix + "_" + _scurName.substring(_scurName.length() - 1, _scurName.length());
        int Index = Integer.valueOf(getIndexNumber(_scurName)).intValue();
        if (Index < oControlRows.length)
        {
            boolean bValue = oControlRows[Index - 1].isConditionComplete();
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
        // ControlRow CurControlRow;
        // getfilterstate();
        int nFilterCount = getFilterCount();
        if (nFilterCount > 0)
        {
            if (this.getfilterstate() == this.SOI_MATCHALL)
            {
                filterconditions = new PropertyValue[1][nFilterCount];
            }
            else
            {
                filterconditions = new PropertyValue[nFilterCount][1];
            }
            int a = 0;
            for (int i = 0; i < RowCount; i++)
            {
                ControlRow CurControlRow = oControlRows[i];
                if (CurControlRow.isEnabled())
                {
                    if (CurControlRow.isConditionComplete())
                    {
                        Object aValue;
                        String sFieldName = CurControlRow.getSelectedFieldName();
                        int nOperator = (int) CurControlRow.getSelectedOperator();
                        FieldColumn aFieldColumn = oQueryMetaData.getFieldColumnByDisplayName(sFieldName);
                        if (aFieldColumn.getStandardFormatKey() == oQueryMetaData.getNumberFormatter().getTextFormatKey())
                        {
                            aValue = "'" + CurControlRow.getValue() + "'";
                        }
//// TODO the following code is bound to be deprecated as soon as the query composer is able to handle date/time values as numbers
                        else if ((aFieldColumn.getStandardFormatKey() == oQueryMetaData.getNumberFormatter().getDateFormatKey()) ||
                                (aFieldColumn.getStandardFormatKey() == oQueryMetaData.getNumberFormatter().getDateTimeFormatKey()))
                        {
                            String sDate = CurControlRow.getDateTimeString(true);
                            aValue = "{D '" + sDate + "' }";  // FormatsSupplier
                        }
                        else if (aFieldColumn.getStandardFormatKey() == oQueryMetaData.getNumberFormatter().getTimeFormatKey())
                        {
                            String sTime = CurControlRow.getDateTimeString(true);
                            aValue = "'{T '" + sTime + "' }";
                        }
                        else
                        {
                            aValue = CurControlRow.getValue();
                            // if void
                            if (! AnyConverter.isVoid(aValue))
                            {
                                switch (aFieldColumn.getFieldType())
                                {
                                    case DataType.TINYINT:
                                    case DataType.BIGINT:
                                    case DataType.INTEGER:
                                    case DataType.SMALLINT:
                                        if ( AnyConverter.isDouble(aValue) )
                                            aValue = String.valueOf(((Double) aValue).intValue());
                                        break;
                                    case DataType.BIT:
                                    case DataType.BOOLEAN:
                                        // curValue = CurControlRow.getText(); // wrong! (creates something like 'WAHR'/'FALSCH' if german locale is used.

                                        // double dblvalue = ((Double) curValue).doubleValue();
                                        //curValue = new Boolean(dblvalue == 1.0); // wrong! we need a string, not a boolean value

                                        // converts the '1.0'/'0.0' (EffectiveValue) to a 'boolean' String like 'true'/'false'
                                        if ( AnyConverter.isDouble(aValue) )
                                            aValue = String.valueOf(((Double) aValue).intValue() == 1);
                                        break;
                                    default:
                                        aValue = String.valueOf(aValue);
                                        break;
                                }
                            }
                        }

                        PropertyValue oPropertyValue = Properties.createProperty(sFieldName, aValue, nOperator);
                        if (getfilterstate()/*this.ifilterstate*/ == this.SOI_MATCHALL)
                        {
                            if (i == 0)
                            {
                                filterconditions[0] = new PropertyValue[nFilterCount];
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
                CurUnoDialog.vetoableChange(new java.beans.PropertyChangeEvent(CurUnoDialog, "Steps", Integer.valueOf(1), Integer.valueOf(2)));
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
            String sDisplayValue = "";
            if ( !AnyConverter.isVoid(_filtercondition.Value) )
                sDisplayValue = AnyConverter.toString(_filtercondition.Value);
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
            if (oControlRows[i].isEnabled() && oControlRows[i].isConditionComplete())
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
            this.IStep = Integer.valueOf(iStep);

            curtabindex = UnoDialog.setInitialTabindex(iStep);
            this.CurUnoDialog = CurUnoDialog;
            this.RowCount = FilterCount;
            this.oQueryMetaData = _oQueryMetaData;
            boolean bEnabled;
            sIncSuffix = com.sun.star.wizards.common.Desktop.getIncrementSuffix(CurUnoDialog.getDlgNameAccess(), "optMatchAll");
            // iStartPosX = iPosX;
            // iStartPosY = iPosY;

            String soptMatchAll = CurUnoDialog.m_oResource.getResText(BaseID + 9);
            String soptMatchAny = CurUnoDialog.m_oResource.getResText(BaseID + 10);
            slblFieldNames = CurUnoDialog.m_oResource.getResText(BaseID + 17);
            slblOperators = CurUnoDialog.m_oResource.getResText(BaseID + 24);
            slblValue = CurUnoDialog.m_oResource.getResText(BaseID + 25);
            sLogicOperators = CurUnoDialog.m_oResource.getResArray(BaseID + 26, 10 /* 7 */); // =, <>, <, >, <=, >=, like, !like, is null, !is null
            sBooleanValues = CurUnoDialog.m_oResource.getResArray(BaseID + 36, 2); // true, false

            sDuplicateCondition = CurUnoDialog.m_oResource.getResText(BaseID + 89);

            // create Radiobuttons
            // * match all
            // * match one
            optMatchAll = CurUnoDialog.insertRadioButton("optMatchAll" + sIncSuffix, SOOPTANDMODE, new ItemListenerImpl(),
                    new String[]
                    {
                        "Height",
                        "HelpURL",
                        "Label",
                        "PositionX",
                        "PositionY",
                        "State",
                        "Step",
                        "TabIndex",
                        "Width"
                    },
                    new Object[]
                    {
                        Integer.valueOf(9),
                        "HID:" + curHelpID++,
                        soptMatchAll,
                        Integer.valueOf(iPosX),
                        Integer.valueOf(iPosY),
                        Short.valueOf((short) 1),
                        IStep,
                        Short.valueOf(curtabindex++),
                        Integer.valueOf(203)
                    });
            optMatchAny = CurUnoDialog.insertRadioButton("optMatchAny" + sIncSuffix, SOOPTORMODE, new ItemListenerImpl(),
                    new String[]
                    {
                        "Height",
                        "HelpURL",
                        "Label",
                        "PositionX",
                        "PositionY",
                        "Step",
                        "TabIndex",
                        "Width"
                    },
                    new Object[]
                    {
                        Integer.valueOf(9),
                        "HID:" + curHelpID++,
                        soptMatchAny,
                        Integer.valueOf(iPosX),
                        Integer.valueOf(iPosY + 12),
                        IStep,
                        Short.valueOf(curtabindex++),
                        Integer.valueOf(203)
                    });
            getfilterstate();

            oControlRows = new ControlRow[FilterCount];
            for (int i = 0; i < FilterCount; i++)
            {
                bEnabled = (i == 0);
                oControlRows[i] = new ControlRow(iPosX, iPosY + 20, i, bEnabled, (this.curHelpID + (i * 3)));
                iPosY += 43;
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public void initialize(PropertyValue[][] _filterconditions, String[] _fieldnames)
    {
        // String aFieldNamesWithAdditionalEmpty[] = new String[_fieldnames.length + 1];
        // for (int i = 0; i < _fieldnames.length; i++)
        // {
        //     aFieldNamesWithAdditionalEmpty[i] = _fieldnames[i];
        // }
        // aFieldNamesWithAdditionalEmpty[_fieldnames.length] = "";

        int i;
        for (i = 0; i < RowCount; i++)
        {
            oControlRows[i].setFieldNames(_fieldnames);
            // oControlRows[i].setFieldNames(aFieldNamesWithAdditionalEmpty);
        }
        this.filterconditions = _filterconditions;
        PropertyValue[] curfilterconditions;
        // int a;
        if (_filterconditions.length == 1)
        {
            curfilterconditions = filterconditions[0];
            for (i = 0; i < curfilterconditions.length; i++)
            {
                oControlRows[i].setCondition(filterconditions[0][i]);
            }
        }
        else
        {
            for (i = 0; i < filterconditions.length; i++)
            {
                oControlRows[i].setCondition(filterconditions[i][0]);
            }
        }
        while (i < oControlRows.length)
        {
            oControlRows[i].settovoid();
            boolean bdoenable;
            if (i > 0)
            {
                bdoenable = oControlRows[i - 1].isConditionComplete();
            }
            else
            {
                bdoenable = true;
            }
            oControlRows[i].setEnabled(bdoenable);
            i++;
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

    // -------------------------------------------------------------------------
    //
    //
    // -------------------------------------------------------------------------

    class ControlRow
    {
        private final static int SOLSTFIELDNAME = 3;
        private final static int SOLSTOPERATOR = 4;
        private final static int SOTXTVALUE = 5;

        protected XInterface[] ControlElements = new XInterface[6];
        private boolean m_bEnabled;
        String[] FieldNames;

        protected ControlRow(int iCompPosX, int iCompPosY, int Index, boolean _bEnabled, int _firstRowHelpID)
        {
            int nFieldWidth = 71;
            int nOperatorWidth = 70;
            int nValueWidth = 44;

            int nPosX1 = iCompPosX + 10;
            int nPosX2 = iCompPosX + 10 + nFieldWidth + 6; // 87
            int nPosX3 = iCompPosX + 10 + nFieldWidth + 6 + nOperatorWidth + 6;


            try
            {
                String sCompSuffix = sIncSuffix + "_" + String.valueOf(Index + 1);
                m_bEnabled = _bEnabled;

                // Label Field
                ControlElements[0] = (XInterface) CurUnoDialog.insertLabel("lblFieldNames" + sCompSuffix,
                        new String[]
                        {
                            "Enabled",
                            "Height",
                            "Label",
                            "PositionX",
                            "PositionY",
                            "Step",
                            "TabIndex",
                            "Width"
                        },
                        new Object[]
                        {
                            Boolean.valueOf(isEnabled()),
                            Integer.valueOf(9),
                            slblFieldNames,
                            Integer.valueOf(nPosX1),
                            Integer.valueOf(iCompPosY + 13),
                            IStep,
                            Short.valueOf(curtabindex++),
                            Integer.valueOf(nFieldWidth)
                        });

                // Label Operator
                ControlElements[1] = (XInterface) CurUnoDialog.insertLabel("lblOperators" + sCompSuffix,
                        new String[]
                        {
                            "Enabled",
                            "Height",
                            "Label",
                            "PositionX",
                            "PositionY",
                            "Step",
                            "TabIndex",
                            "Width"
                        },
                        new Object[]
                        {
                            Boolean.valueOf(isEnabled()),
                            Integer.valueOf(9),
                            slblOperators,
                            Integer.valueOf(nPosX2),
                            Integer.valueOf(iCompPosY + 13),
                            IStep,
                            Short.valueOf(curtabindex++),
                            Integer.valueOf(nOperatorWidth)
                        });

                // Label Value
                ControlElements[2] = (XInterface) CurUnoDialog.insertLabel("lblValue" + sCompSuffix,
                        new String[]
                        {
                            "Enabled",
                            "Height",
                            "Label",
                            "PositionX",
                            "PositionY",
                            "Step",
                            "TabIndex",
                            "Width"
                        },
                        new Object[]
                        {
                            Boolean.valueOf(isEnabled()),
                            Integer.valueOf(9),
                            slblValue,
                            Integer.valueOf(nPosX3),
                            Integer.valueOf(iCompPosY + 13),
                            IStep,
                            Short.valueOf(curtabindex++),
                            Integer.valueOf(nValueWidth)
                        });

                // Listbox Fields
                ControlElements[SOLSTFIELDNAME] = (XInterface) CurUnoDialog.insertListBox("lstFieldName" + sCompSuffix, SO_FIELDNAMELIST[Index], null, new ItemListenerImpl(),
                        new String[]
                        {
                            "Enabled",
                            "Dropdown",
                            "Height",
                            "HelpURL",
                            "LineCount",
                            "PositionX",
                            "PositionY",
                            "Step",
                            "TabIndex",
                            "Width"
                        },
                        new Object[]
                        {
                            Boolean.valueOf(isEnabled()),
                            Boolean.TRUE,
                            Integer.valueOf(13),
                            "HID:" + _firstRowHelpID++,
                            Short.valueOf(UnoDialog.getListBoxLineCount() /* 7 */) ,
                            Integer.valueOf(nPosX1),
                            Integer.valueOf(iCompPosY + 23),
                            IStep,
                            Short.valueOf(curtabindex++),
                            Integer.valueOf(nFieldWidth)
                        });

                // Listbox Operators
                ControlElements[SOLSTOPERATOR] = (XInterface) CurUnoDialog.insertListBox("lstOperator" + sCompSuffix, SO_CONDITIONLIST[Index], null, new ItemListenerImpl(),
                        new String[]
                        {
                            "Enabled",
                            "Dropdown",
                            "Height",
                            "HelpURL",
                            "LineCount",
                            "PositionX",
                            "PositionY",
                            "Step",
                            "StringItemList",
                            "TabIndex",
                            "Width"
                        },
                        new Object[]
                        {
                            Boolean.valueOf(isEnabled()),
                            Boolean.TRUE,
                            Integer.valueOf(13),
                            "HID:" + _firstRowHelpID++,
                            Short.valueOf((short) sLogicOperators.length /* 7 */ ),
                            Integer.valueOf(nPosX2),
                            Integer.valueOf(iCompPosY + 23),
                            IStep,
                            sLogicOperators,
                            Short.valueOf(curtabindex++),
                            Integer.valueOf(nOperatorWidth)
                        });
                ControlElements[SOTXTVALUE] = (XInterface) CurUnoDialog.insertFormattedField("txtValue" + sCompSuffix, SO_TEXTFIELDLIST[Index], new TextListenerImpl(),
                        new String[]
                        {
                            "Enabled",
                            "Height",
                            "HelpURL",
                            "PositionX",
                            "PositionY",
                            "Step",
                            "TabIndex",
                            "Width"
                        },
                        new Object[]
                        {
                            Boolean.valueOf(isEnabled()),
                            Integer.valueOf(13),
                            "HID:" + _firstRowHelpID++,
                            Integer.valueOf(nPosX3),
                            Integer.valueOf(iCompPosY + 23),
                            IStep,
                            Short.valueOf(curtabindex++),
                            Integer.valueOf(nValueWidth)
                        });

//                ControlElements[6] = CurUnoDialog.insertListBox((new StringBuilder()).append("lstBoolean").append(sCompSuffix).toString(), SO_BOOLEANLIST[Index], null, new ItemListenerImpl(), new String[] {
//                    "Enabled",
//                    "Dropdown",
//                    "Height",
//                    "HelpURL",
//                    "LineCount",
//                    "PositionX", "PositionY",
//                    "Step",
//                    "StringItemList",
//                    "TabIndex",
//                    "Width"
//                }, new Object[] {
//                    new Boolean(bEnabled),
//                    Boolean.TRUE,
//                    new Integer(13),
//                    "HID:" + _firstRowHelpID++,
//                    new Short((short) 2),
//                    new Integer(nPosX3 + 44), new Integer(iCompPosY + 23),
//                    IStep,
//                    FilterComponent.sBooleanValues,
//                    new Short(curtabindex++),
//                    new Integer(nValueWidth)
//                });
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.out);
            }
        }

        /**
         * @returns true if the current condition is complete, all needed fields are filled with values.
         * So we can enable the next.
         */
        boolean isConditionComplete()
        {
            try
            {
                short[] SelFields = (short[]) AnyConverter.toArray(Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTFIELDNAME]), "SelectedItems"));
                if (SelFields.length > 0)
                {
                    int nSelOperator = getSelectedOperator();
                    // short[] SelOperator = (short[]) AnyConverter.toArray(Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTOPERATOR]), "SelectedItems"));
                    if (nSelOperator == com.sun.star.sdb.SQLFilterOperator.SQLNULL ||   /* is null */
                        nSelOperator == com.sun.star.sdb.SQLFilterOperator.NOT_SQLNULL) /* is not null */
                    {
                        // disable value field
                        Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[2]), "Enabled", Boolean.FALSE);
                        Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), "Enabled", Boolean.FALSE);

                        return true;
                    }
                    else
                    {
                        // enable value field
                        Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[2]), "Enabled", Boolean.TRUE);
                        Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), "Enabled", Boolean.TRUE);
                    }
                    if (nSelOperator != -1)
                    {
                        Object oValue = Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), "EffectiveValue");
                        if (!AnyConverter.isVoid(oValue))
                        {
                            String sValue = (String.valueOf(oValue));
                            return (!sValue.equals(""));
                        }
//                        String sBoolValue="";
//                        short aSelectedBoolValue[] = (short[])Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[6]), "SelectedItems");
//                        if (aSelectedBoolValue.length > 0)
//                        {
//                            sBoolValue = String.valueOf(aSelectedBoolValue[0] == 1);
//                            return !sBoolValue.equals("");
//                        }
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

        private void fieldnamechanged(ItemEvent EventObject)
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
            return m_bEnabled;
        }

        private void settovoid()
        {
            CurUnoDialog.deselectListBox(ControlElements[SOLSTFIELDNAME]);
            CurUnoDialog.deselectListBox(ControlElements[SOLSTOPERATOR]);
            Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), "EffectiveValue", com.sun.star.uno.Any.VOID);
        }

        private void setEnabled(boolean _bEnabled)
        {
            // enable all Controls Fieldname, operator, value
            for (int i = 0; i < ControlElements.length; i++)
            {
                Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[i]), "Enabled", Boolean.valueOf(_bEnabled));
            }
            m_bEnabled = _bEnabled;
            if (isEnabled())
            {
                short[] iselected = new short[] {};
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
                    Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTOPERATOR]), "SelectedItems", new short[] { 0 });
                }
            }
            else if (!isConditionComplete())
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
        // =, <>, <, >, <=, >=, like, !like, is null, !is null
        protected short getSelectedOperator()
        {
            try
            {
                short[] SelFields = (short[]) AnyConverter.toArray(Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTOPERATOR]), "SelectedItems"));
                switch (SelFields[0])
                {
                case 0:
                    return com.sun.star.sdb.SQLFilterOperator.EQUAL;
                case 1:
                    return com.sun.star.sdb.SQLFilterOperator.NOT_EQUAL;
                case 2:
                    return com.sun.star.sdb.SQLFilterOperator.LESS;
                case 3:
                    return com.sun.star.sdb.SQLFilterOperator.GREATER;
                case 4:
                    return com.sun.star.sdb.SQLFilterOperator.LESS_EQUAL;
                case 5:
                    return com.sun.star.sdb.SQLFilterOperator.GREATER_EQUAL;
                case 6:
                    return com.sun.star.sdb.SQLFilterOperator.LIKE;
                case 7:
                    return com.sun.star.sdb.SQLFilterOperator.NOT_LIKE;
                case 8:
                    return com.sun.star.sdb.SQLFilterOperator.SQLNULL;
                case 9:
                    return com.sun.star.sdb.SQLFilterOperator.NOT_SQLNULL;
                default:
                    return -1;
                }
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
}
//  com.sun.star.sdb.SQLFilterOperator.EQUAL
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
