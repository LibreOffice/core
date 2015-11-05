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
package com.sun.star.wizards.ui;

import com.sun.star.awt.TextEvent;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XListBox;
import com.sun.star.awt.XRadioButton;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertyContainer;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.sdb.SQLFilterOperator;
import com.sun.star.sdbc.DataType;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.wizards.common.NumberFormatter;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.HelpIds;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.db.FieldColumn;
import com.sun.star.wizards.db.QueryMetaData;
import com.sun.star.wizards.db.SQLQueryComposer;

import java.util.logging.Level;
import java.util.logging.Logger;

public class FilterComponent
{

    private final Integer IStep;
    private final int RowCount;
    private static String[] sLogicOperators;
    private final XRadioButton optMatchAny;
    private final String slblFieldNames;
    private final String slblOperators;
    private final String slblValue;
    private WizardDialog CurUnoDialog;
    private static final int BaseID = 2300;
    private final String sIncSuffix;
    private final ControlRow[] oControlRows;
    private final String sDuplicateCondition;
    private static final int SOOPTORMODE = 100;
    private static final int SOOPTANDMODE = 101;
    private QueryMetaData oQueryMetaData;
    private int iDateFormat;
    private int iTimeFormat;
    private PropertyValue[][] filterconditions;
    private short curtabindex;
    private static final int SO_FIRSTFIELDNAME = 1;
    private static final int SO_SECONDFIELDNAME = 2;
    private static final int SO_THIRDFIELDNAME = 3;
    private static final int SO_FOURTHFIELDNAME = 4;
    private int[] SO_FIELDNAMELIST = new int[]
    {
        SO_FIRSTFIELDNAME, SO_SECONDFIELDNAME, SO_THIRDFIELDNAME, SO_FOURTHFIELDNAME
    };
    private static final int SO_FIRSTCONDITION = 5;
    private static final int SO_SECONDCONDITION = 6;
    private static final int SO_THIRDCONDITION = 7;
    private static final int SO_FOURTHCONDITION = 8;
    private int[] SO_CONDITIONLIST = new int[]
    {
        SO_FIRSTCONDITION, SO_SECONDCONDITION, SO_THIRDCONDITION, SO_FOURTHCONDITION
    };
    private static final int SO_FIRSTTEXTFIELD = 1;
    private static final int SO_SECONDTEXTFIELD = 2;
    private static final int SO_THIRDTEXTFIELD = 3;
    private static final int SO_FOURTHTEXTFIELD = 4;
    private int[] SO_TEXTFIELDLIST = new int[]
    {
        SO_FIRSTTEXTFIELD, SO_SECONDTEXTFIELD, SO_THIRDTEXTFIELD, SO_FOURTHTEXTFIELD
    };
    private static final int SO_FIRSTBOOLFIELDNAME = 256 + 1;
    private static final int SO_SECONDBOOLFIELDNAME = 256 + 2;
    private static final int SO_THIRDBOOLFIELDNAME = 256 + 3;
    private static final int SO_FOURTHBOOLFIELDNAME = 256 + 4;
    private int SOI_MATCHALL = 0;
    private int SOI_MATCHANY = 1;

    class ItemListenerImpl implements com.sun.star.awt.XItemListener
    {

        public void itemStateChanged(com.sun.star.awt.ItemEvent EventObject)
        {
            int iKey = CurUnoDialog.getControlKey(EventObject.Source, CurUnoDialog.ControlList);
            String sControlName = PropertyNames.EMPTY_STRING;
            switch (iKey)
            {
                case SO_FIRSTFIELDNAME:
                case SO_SECONDFIELDNAME:
                case SO_THIRDFIELDNAME:
                case SO_FOURTHFIELDNAME:
                    sControlName = getControlName(EventObject.Source);
                    String sControlNameSuffix = sIncSuffix + "_" + getIndexNumber(sControlName);
                    XListBox xCurFieldListBox = UnoRuntime.queryInterface(XListBox.class, CurUnoDialog.xDlgContainer.getControl(sControlName));
                    String CurDisplayFieldName = xCurFieldListBox.getSelectedItem();
                    FieldColumn CurFieldColumn = oQueryMetaData.getFieldColumnByDisplayName(CurDisplayFieldName);

                    String sControlNameTextValue = "txtValue" + sControlNameSuffix;
                    XControl xValueControl = CurUnoDialog.xDlgContainer.getControl(sControlNameTextValue);
                    XInterface xValueModel = (XInterface) UnoDialog.getModel(xValueControl);
                    Helper.setUnoPropertyValue(xValueModel, "TreatAsNumber", Boolean.valueOf(CurFieldColumn.isNumberFormat()));
                    final NumberFormatter aNumberFormatter = oQueryMetaData.getNumberFormatter();
                    aNumberFormatter.setNumberFormat(xValueModel, CurFieldColumn.getDBFormatKey(), aNumberFormatter);

                    break;

                case SO_FIRSTCONDITION:
                case SO_SECONDCONDITION:
                case SO_THIRDCONDITION:
                case SO_FOURTHCONDITION:
                case SO_FIRSTBOOLFIELDNAME:
                case SO_SECONDBOOLFIELDNAME:
                case SO_THIRDBOOLFIELDNAME:
                case SO_FOURTHBOOLFIELDNAME:
                    sControlName = getControlName(EventObject.Source);
                    break;

                case SOOPTORMODE:
                case SOOPTANDMODE:
                    return;

                default:
                    break;
            }
            togglefollowingControlRow(sControlName);
        }

        public void disposing(com.sun.star.lang.EventObject eventObject)
        {
        }
    }

    class TextListenerImpl implements com.sun.star.awt.XTextListener
    {

        public void textChanged(TextEvent EventObject)
        {
            String sName = getControlName(EventObject.Source);
            togglefollowingControlRow(sName);
        }

        public void disposing(EventObject eventObject)
        {
        }


    }

    private static String getIndexNumber(String _sStr)
    {
        return _sStr.substring(_sStr.length() - 1, _sStr.length());
    }

    /**
     * Enable the next ControlRow if the Condition is complete in the current line
     *
     * @param _scurName every control name ends with something like _1, _2, _3 this is the index number
     *        we try to interpret to check which line we currently use. (number - 1)
     */
    private void togglefollowingControlRow(String _scurName)
    {
        int Index = Integer.parseInt(getIndexNumber(_scurName));
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
            XControl xControl = UnoRuntime.queryInterface(XControl.class, _oSourceevent);
            XPropertySet xPSet = UnoRuntime.queryInterface(XPropertySet.class, xControl.getModel());
            return AnyConverter.toString(xPSet.getPropertyValue(PropertyNames.PROPERTY_NAME));
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
        return PropertyNames.EMPTY_STRING;
    }

    public PropertyValue[][] getFilterConditions()
    {
        int nFilterCount = getFilterCount();
        if (nFilterCount > 0)
        {
            final SQLQueryComposer composer = oQueryMetaData.getSQLQueryComposer();
            try
            {
                final String serviceName = "com.sun.star.beans.PropertyBag";
                final XPropertyContainer column = UnoRuntime.queryInterface(XPropertyContainer.class, oQueryMetaData.xMSF.createInstance(serviceName));

                column.addProperty("Type", PropertyAttribute.BOUND, DataType.VARCHAR);
                column.addProperty(PropertyNames.PROPERTY_NAME, PropertyAttribute.BOUND, PropertyNames.EMPTY_STRING);
                column.addProperty("Value", (short) (PropertyAttribute.MAYBEVOID | PropertyAttribute.REMOVABLE), null);
                final XPropertySet columnSet = UnoRuntime.queryInterface(XPropertySet.class, column);

                if (composer.getQuery().length() == 0)
                {
                    StringBuilder sql = new StringBuilder();
                    sql.append(composer.getSelectClause(true));
                    sql.append(' ');
                    sql.append(composer.getFromClause());
                    composer.getQueryComposer().setElementaryQuery(sql.toString());
                }
                composer.getQueryComposer().setStructuredFilter(new PropertyValue[][]
                        {
                        });
                for (int i = 0; i < RowCount; i++)
                {
                    ControlRow currentControlRow = oControlRows[i];
                    if (currentControlRow.isEnabled() && currentControlRow.isConditionComplete())
                    {
                        String sFieldName = currentControlRow.getSelectedFieldName();
                        int nOperator = currentControlRow.getSelectedOperator();
                        FieldColumn aFieldColumn = oQueryMetaData.getFieldColumnByDisplayName(sFieldName);
                        columnSet.setPropertyValue(PropertyNames.PROPERTY_NAME, aFieldColumn.getFieldName());
                        columnSet.setPropertyValue("Type", aFieldColumn.getXColumnPropertySet().getPropertyValue("Type"));
                        Object value = currentControlRow.getValue();
                        switch (aFieldColumn.getFieldType())
                        {
                            case DataType.TIMESTAMP:
                            case DataType.DATE:
                                value = ((Double) value) - oQueryMetaData.getNullDateCorrection();
                                break;
                                // TODO: TIME case?
                        }
                        column.removeProperty("Value");
                        final short operator = currentControlRow.getSelectedOperator();
                        if ((operator == SQLFilterOperator.SQLNULL)
                                || (operator == SQLFilterOperator.NOT_SQLNULL)
                                || AnyConverter.isVoid(value))
                        {
                            column.addProperty("Value", (short) (PropertyAttribute.MAYBEVOID | PropertyAttribute.REMOVABLE), "");
                            value = new Any(new Type(TypeClass.VOID), null);
                        }
                        else
                        {
                            column.addProperty("Value", (short) (PropertyAttribute.MAYBEVOID | PropertyAttribute.REMOVABLE), value);
                        }
                        columnSet.setPropertyValue("Value", value);
                        composer.getQueryComposer().appendFilterByColumn(columnSet, getfilterstate() == this.SOI_MATCHALL, nOperator);
                    }
                }
                filterconditions = composer.getNormalizedStructuredFilter();
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
            catch (Exception ex)
            {
                Logger.getLogger(FilterComponent.class.getName()).log(Level.SEVERE, null, ex);
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
                FieldColumn CurDBFieldColumn = _CurDBMetaData.getFieldColumnByFieldName(_filtercondition.Name);
                FieldName = CurDBFieldColumn.getFieldTitle();
            }
            else
            {
                FieldName = _filtercondition.Name;
            }
            String sreturn = JavaTools.replaceSubString(_BaseString, FieldName, "<FIELDNAME>");
            String soperator = sLogicOperators[_filtercondition.Handle - 1];
            sreturn = JavaTools.replaceSubString(sreturn, soperator, "<LOGICOPERATOR>");
            String sDisplayValue = PropertyNames.EMPTY_STRING;
            if ((_filtercondition.Handle != SQLFilterOperator.SQLNULL)
                    && (_filtercondition.Handle != SQLFilterOperator.NOT_SQLNULL)
                    && !AnyConverter.isVoid(_filtercondition.Value))
            {
                sDisplayValue = AnyConverter.toString(_filtercondition.Value);
            }
            sreturn = JavaTools.replaceSubString(sreturn, sDisplayValue, "<VALUE>");
            return sreturn;
        }
        catch (IllegalArgumentException e)
        {
            e.printStackTrace(System.err);
        }
        return PropertyNames.EMPTY_STRING;
    }

    private int getfilterstate()
    {
        boolean bisany = true;
        int ifilterstate = SOI_MATCHALL;
        bisany = this.optMatchAny.getState();
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
    }

    /** Creates a new instance of FilterComponent
     */
    public FilterComponent(WizardDialog CurUnoDialog, int iStep, int iPosX, int iPosY, int FilterCount, QueryMetaData _oQueryMetaData, int _firstHelpID)
    {
        int curHelpID = _firstHelpID;
        this.IStep = Integer.valueOf(iStep);

        curtabindex = UnoDialog.setInitialTabindex(iStep);
        this.CurUnoDialog = CurUnoDialog;
        this.RowCount = FilterCount;
        this.oQueryMetaData = _oQueryMetaData;
        boolean bEnabled;
        sIncSuffix = com.sun.star.wizards.common.Desktop.getIncrementSuffix(CurUnoDialog.getDlgNameAccess(), "optMatchAll");

        String soptMatchAll = CurUnoDialog.m_oResource.getResText(BaseID + 9);
        String soptMatchAny = CurUnoDialog.m_oResource.getResText(BaseID + 10);
        slblFieldNames = CurUnoDialog.m_oResource.getResText(BaseID + 17);
        slblOperators = CurUnoDialog.m_oResource.getResText(BaseID + 24);
        slblValue = CurUnoDialog.m_oResource.getResText(BaseID + 25);
        sLogicOperators = CurUnoDialog.m_oResource.getResArray(BaseID + 26, 10 /* 7 */); // =, <>, <, >, <=, >=, like, !like, is null, !is null

        sDuplicateCondition = CurUnoDialog.m_oResource.getResText(BaseID + 89);

        // create Radiobuttons
        // * match all
        // * match one
        CurUnoDialog.insertRadioButton("optMatchAll" + sIncSuffix, SOOPTANDMODE, new ItemListenerImpl(),
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT,
                    PropertyNames.PROPERTY_HELPURL,
                    PropertyNames.PROPERTY_LABEL,
                    PropertyNames.PROPERTY_POSITION_X,
                    PropertyNames.PROPERTY_POSITION_Y,
                    PropertyNames.PROPERTY_STATE,
                    PropertyNames.PROPERTY_STEP,
                    PropertyNames.PROPERTY_TABINDEX,
                    PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Integer.valueOf(9),
                    HelpIds.getHelpIdString(curHelpID++),
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
                    PropertyNames.PROPERTY_HEIGHT,
                    PropertyNames.PROPERTY_HELPURL,
                    PropertyNames.PROPERTY_LABEL,
                    PropertyNames.PROPERTY_POSITION_X,
                    PropertyNames.PROPERTY_POSITION_Y,
                    PropertyNames.PROPERTY_STEP,
                    PropertyNames.PROPERTY_TABINDEX,
                    PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Integer.valueOf(9),
                    HelpIds.getHelpIdString(curHelpID++),
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
            oControlRows[i] = new ControlRow(iPosX, iPosY + 20, i, bEnabled, (curHelpID + (i * 3)));
            iPosY += 43;
        }
    }

    public void initialize(PropertyValue[][] _filterconditions, String[] _fieldnames)
    {
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
        iDateFormat = oQueryMetaData.getNumberFormatter().defineNumberFormat("YYYY-MM-DD");
        iTimeFormat = oQueryMetaData.getNumberFormatter().defineNumberFormat("HH:MM:SS");
        oQueryMetaData.getNumberFormatter().defineNumberFormat("YYYY-MM-DD HH:MM:SS");
    }





    private final class ControlRow
    {

        private static final int SOLSTFIELDNAME = 3;
        private static final int SOLSTOPERATOR = 4;
        private static final int SOTXTVALUE = 5;
        private XInterface[] ControlElements = new XInterface[6];
        private boolean m_bEnabled;
        private String[] FieldNames;

        private ControlRow(int iCompPosX, int iCompPosY, int Index, boolean _bEnabled, int _firstRowHelpID)
        {
            int nFieldWidth = 71;
            int nOperatorWidth = 70;
            int nValueWidth = 44;

            int nPosX1 = iCompPosX + 10;
            int nPosX2 = iCompPosX + 10 + nFieldWidth + 6; // 87
            int nPosX3 = iCompPosX + 10 + nFieldWidth + 6 + nOperatorWidth + 6;


            try
            {
                String sCompSuffix = sIncSuffix + "_" + (Index + 1);
                m_bEnabled = _bEnabled;

                // Label Field
                ControlElements[0] = CurUnoDialog.insertLabel("lblFieldNames" + sCompSuffix,
                        new String[]
                        {
                            PropertyNames.PROPERTY_ENABLED,
                            PropertyNames.PROPERTY_HEIGHT,
                            PropertyNames.PROPERTY_LABEL,
                            PropertyNames.PROPERTY_POSITION_X,
                            PropertyNames.PROPERTY_POSITION_Y,
                            PropertyNames.PROPERTY_STEP,
                            PropertyNames.PROPERTY_TABINDEX,
                            PropertyNames.PROPERTY_WIDTH
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
                ControlElements[1] = CurUnoDialog.insertLabel("lblOperators" + sCompSuffix,
                        new String[]
                        {
                            PropertyNames.PROPERTY_ENABLED,
                            PropertyNames.PROPERTY_HEIGHT,
                            PropertyNames.PROPERTY_LABEL,
                            PropertyNames.PROPERTY_POSITION_X,
                            PropertyNames.PROPERTY_POSITION_Y,
                            PropertyNames.PROPERTY_STEP,
                            PropertyNames.PROPERTY_TABINDEX,
                            PropertyNames.PROPERTY_WIDTH
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
                ControlElements[2] = CurUnoDialog.insertLabel("lblValue" + sCompSuffix,
                        new String[]
                        {
                            PropertyNames.PROPERTY_ENABLED,
                            PropertyNames.PROPERTY_HEIGHT,
                            PropertyNames.PROPERTY_LABEL,
                            PropertyNames.PROPERTY_POSITION_X,
                            PropertyNames.PROPERTY_POSITION_Y,
                            PropertyNames.PROPERTY_STEP,
                            PropertyNames.PROPERTY_TABINDEX,
                            PropertyNames.PROPERTY_WIDTH
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
                ControlElements[SOLSTFIELDNAME] = CurUnoDialog.insertListBox("lstFieldName" + sCompSuffix, SO_FIELDNAMELIST[Index], null, new ItemListenerImpl(),
                        new String[]
                        {
                            PropertyNames.PROPERTY_ENABLED,
                            "Dropdown",
                            PropertyNames.PROPERTY_HEIGHT,
                            PropertyNames.PROPERTY_HELPURL,
                            "LineCount",
                            PropertyNames.PROPERTY_POSITION_X,
                            PropertyNames.PROPERTY_POSITION_Y,
                            PropertyNames.PROPERTY_STEP,
                            PropertyNames.PROPERTY_TABINDEX,
                            PropertyNames.PROPERTY_WIDTH
                        },
                        new Object[]
                        {
                            Boolean.valueOf(isEnabled()),
                            Boolean.TRUE,
                            Integer.valueOf(13),
                            HelpIds.getHelpIdString(_firstRowHelpID++),
                            Short.valueOf(UnoDialog.getListBoxLineCount() /* 7 */),
                            Integer.valueOf(nPosX1),
                            Integer.valueOf(iCompPosY + 23),
                            IStep,
                            Short.valueOf(curtabindex++),
                            Integer.valueOf(nFieldWidth)
                        });

                // Listbox Operators
                ControlElements[SOLSTOPERATOR] = CurUnoDialog.insertListBox("lstOperator" + sCompSuffix, SO_CONDITIONLIST[Index], null, new ItemListenerImpl(),
                        new String[]
                        {
                            PropertyNames.PROPERTY_ENABLED,
                            "Dropdown",
                            PropertyNames.PROPERTY_HEIGHT,
                            PropertyNames.PROPERTY_HELPURL,
                            "LineCount",
                            PropertyNames.PROPERTY_POSITION_X,
                            PropertyNames.PROPERTY_POSITION_Y,
                            PropertyNames.PROPERTY_STEP,
                            PropertyNames.STRING_ITEM_LIST,
                            PropertyNames.PROPERTY_TABINDEX,
                            PropertyNames.PROPERTY_WIDTH
                        },
                        new Object[]
                        {
                            Boolean.valueOf(isEnabled()),
                            Boolean.TRUE,
                            Integer.valueOf(13),
                            HelpIds.getHelpIdString(_firstRowHelpID++),
                            Short.valueOf((short) sLogicOperators.length /* 7 */),
                            Integer.valueOf(nPosX2),
                            Integer.valueOf(iCompPosY + 23),
                            IStep,
                            sLogicOperators,
                            Short.valueOf(curtabindex++),
                            Integer.valueOf(nOperatorWidth)
                        });
                ControlElements[SOTXTVALUE] = CurUnoDialog.insertFormattedField("txtValue" + sCompSuffix, SO_TEXTFIELDLIST[Index], new TextListenerImpl(),
                        new String[]
                        {
                            PropertyNames.PROPERTY_ENABLED,
                            PropertyNames.PROPERTY_HEIGHT,
                            PropertyNames.PROPERTY_HELPURL,
                            PropertyNames.PROPERTY_POSITION_X,
                            PropertyNames.PROPERTY_POSITION_Y,
                            PropertyNames.PROPERTY_STEP,
                            PropertyNames.PROPERTY_TABINDEX,
                            PropertyNames.PROPERTY_WIDTH
                        },
                        new Object[]
                        {
                            Boolean.valueOf(isEnabled()),
                            Integer.valueOf(13),
                            HelpIds.getHelpIdString(_firstRowHelpID++),
                            Integer.valueOf(nPosX3),
                            Integer.valueOf(iCompPosY + 23),
                            IStep,
                            Short.valueOf(curtabindex++),
                            Integer.valueOf(nValueWidth)
                        });
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.err);
            }
        }

        /**
         * @return true if the current condition is complete, all needed fields are filled with values.
         * So we can enable the next.
         */
        private boolean isConditionComplete()
        {
            try
            {
                short[] SelFields = (short[]) AnyConverter.toArray(Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTFIELDNAME]), PropertyNames.SELECTED_ITEMS));
                if (SelFields.length > 0)
                {
                    int nSelOperator = getSelectedOperator();
                    // short[] SelOperator = (short[]) AnyConverter.toArray(Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTOPERATOR]), PropertyNames.SELECTED_ITEMS));
                    if (nSelOperator == com.sun.star.sdb.SQLFilterOperator.SQLNULL
                            || /* is null */ nSelOperator == com.sun.star.sdb.SQLFilterOperator.NOT_SQLNULL) /* is not null */

                    {
                        // disable value field
                        Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[2]), PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
                        Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);

                        return true;
                    }
                    else
                    {
                        // enable value field
                        Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[2]), PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
                        Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
                    }
                    if (nSelOperator != -1)
                    {
                        Object oValue = Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), "EffectiveValue");
                        if (!AnyConverter.isVoid(oValue))
                        {
                            String sValue = String.valueOf(oValue);
                            return !sValue.equals(PropertyNames.EMPTY_STRING);
                        }
                    }
                }
                return false;
            }
            catch (IllegalArgumentException exception)
            {
                exception.printStackTrace(System.err);
                return false;
            }
        }

        private void setCondition(PropertyValue _filtercondition)
        {
            try
            {
                XListBox xFieldsListBox = UnoRuntime.queryInterface(XListBox.class, ControlElements[SOLSTFIELDNAME]);
                xFieldsListBox.selectItem(_filtercondition.Name, true);
                XListBox xOperatorListBox = UnoRuntime.queryInterface(XListBox.class, ControlElements[SOLSTOPERATOR]);
                xOperatorListBox.selectItemPos((short) (_filtercondition.Handle - 1), true);

                if (AnyConverter.isString(_filtercondition.Value))
                {
                    String sValue = AnyConverter.toString(_filtercondition.Value);
                    if (sValue.indexOf("{D '") > -1)
                    {
                        // TODO: this seems to never happen?
                        sValue = JavaTools.replaceSubString(sValue, PropertyNames.EMPTY_STRING, "{D '");
                        sValue = JavaTools.replaceSubString(sValue, PropertyNames.EMPTY_STRING, "' }");
                        try
                        {
                            //Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), "EffectiveValue", );
                            oQueryMetaData.getNumberFormatter().convertStringToNumber(iDateFormat, sValue);
                        }
                        catch (java.lang.Exception ex)
                        {
                            Logger.getLogger(FilterComponent.class.getName()).log(Level.SEVERE, null, ex);
                        }
                    }
                    else if (sValue.indexOf("{T '") > -1)
                    {
                        // TODO: this seems to never happen?
                        sValue = JavaTools.replaceSubString(sValue, PropertyNames.EMPTY_STRING, "{T '");
                        sValue = JavaTools.replaceSubString(sValue, PropertyNames.EMPTY_STRING, "' }");
                        try
                        {
                            //Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), "EffectiveValue", );
                            oQueryMetaData.getNumberFormatter().convertStringToNumber(iTimeFormat, sValue);
                        }
                        catch (java.lang.Exception ex)
                        {
                            Logger.getLogger(FilterComponent.class.getName()).log(Level.SEVERE, null, ex);
                        }
                    }
                    // TODO: TS case?
                    //TODO: other datetime formats?
                    else
                    {
                        //Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), "EffectiveValue", sValue);
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
            catch (IllegalArgumentException e)
            {
                e.printStackTrace(System.err);
            }
        }

        private void setFieldNames(String[] _FieldNames)
        {
            Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTFIELDNAME]), PropertyNames.STRING_ITEM_LIST, _FieldNames);
            FieldNames = _FieldNames;
        }

        private boolean isEnabled()
        {
            return m_bEnabled;
        }

        private void settovoid()
        {
            WizardDialog.deselectListBox(ControlElements[SOLSTFIELDNAME]);
            WizardDialog.deselectListBox(ControlElements[SOLSTOPERATOR]);
            Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), "EffectiveValue", com.sun.star.uno.Any.VOID);
        }

        private void setEnabled(boolean _bEnabled)
        {
            // enable all Controls Fieldname, operator, value
            for (int i = 0; i < ControlElements.length; i++)
            {
                Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[i]), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(_bEnabled));
            }
            m_bEnabled = _bEnabled;
            if (isEnabled())
            {
                short[] iselected = new short[]
                {
                };
                try
                {
                    iselected = (short[]) AnyConverter.toArray(Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTOPERATOR]), PropertyNames.SELECTED_ITEMS));
                }
                catch (IllegalArgumentException e)
                {
                    e.printStackTrace(System.err);
                }
                if ((iselected.length) == 0)
                {
                    Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTOPERATOR]), PropertyNames.SELECTED_ITEMS, new short[]
                            {
                                0
                            });
                }
            }
            else if (!isConditionComplete())
            {
                WizardDialog.deselectListBox(ControlElements[SOLSTOPERATOR]);
            }
        }

        private String getSelectedFieldName()
        {
            try
            {
                short[] SelFields = (short[]) AnyConverter.toArray(Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTFIELDNAME]), PropertyNames.SELECTED_ITEMS));
                return FieldNames[SelFields[0]];
            }
            catch (IllegalArgumentException exception)
            {
                exception.printStackTrace(System.err);
            }
            return null;
        }

        // TODO: check if it is really useful to match the indices of the listbox the API constants
        // =, <>, <, >, <=, >=, like, !like, is null, !is null
        private short getSelectedOperator()
        {
            try
            {
                short[] SelFields = (short[]) AnyConverter.toArray(Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTOPERATOR]), PropertyNames.SELECTED_ITEMS));
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
            catch (IllegalArgumentException exception)
            {
                exception.printStackTrace(System.err);
            }
            return -1;
        }

        // TODO make a difference between Text and Numbers
        private Object getValue()
        {
            return Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), "EffectiveValue");
        }




    }
}
