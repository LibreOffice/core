/*************************************************************************
 *
 *  $RCSfile: FilterComponent.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-05-19 13:04:50 $
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
 *  Contributor(s): _____________________________________
 *
 */

package com.sun.star.wizards.ui;

import java.util.Vector;

import com.sun.star.awt.TextEvent;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.XButton;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XListBox;
import com.sun.star.awt.XTextComponent;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.NotNumericException;
import com.sun.star.util.XNumberFormatsSupplier;
import com.sun.star.util.XNumberFormatter;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.common.SystemDialog;
import com.sun.star.wizards.db.FieldColumn;
import com.sun.star.wizards.db.QueryMetaData;


public class FilterComponent {
    Integer IStep;
    int iStartPosX;
    int iStartPosY;
    int Count;
    int RowCount;
    int FilterCount;
    static String[] sLogicOperators;
    public String[] FieldNames;
    XButton optMatchAll;
    XButton optMatchAny;
    private String soptMatchAll;
    private String soptMatchAny;
    private String[] sHeadLines;
    private String slblFieldNames;
    private String slblOperators;
    private String slblValue;
    WizardDialog CurUnoDialog;
    private int BaseID = 2300;
    private String sIncSuffix;
    private ControlRow[] oControlRows;
    private Vector FilterNames;
    private String sDuplicateCondition;
    final int SOLSTFIELDNAME = 3;
    final int SOLSTOPERATOR = 4;
    final int SOTXTVALUE = 5;
    String OPTIONBUTTONMATCHANY_ITEM_CHANGED;
    String TEXTFIELDVALUE_TEXT_CHANGED;
    String LISTBOXOPERATORS_ACTION_PERFORMED;
    String LISTBOXOPERATORS_ITEM_CHANGED;
    String LISTBOXFIELDNAME_ACTION_PERFORMED;
    String LISTBOXFIELDNAME_ITEM_CHANGED;
    QueryMetaData oQueryMetaData;
    int iDateFormat;
    int iTimeFormat;
    PropertyValue[][] filterconditions;
    short curtabindex;
    XMultiServiceFactory xMSF;
    XNumberFormatsSupplier xNumberFormatsSupplier;
    XNumberFormatter xNumberFormatter;

    final int SOFIRSTFIELDNAME = 1;
    final int SOSECFIELDNAME = 2;
    final int SOTHIRDFIELDNAME = 3;
    final int SOFOURTHFIELDNAME = 4;
    int[] SOFIELDNAMELIST = new int[] { SOFIRSTFIELDNAME, SOSECFIELDNAME, SOTHIRDFIELDNAME, SOFOURTHFIELDNAME };

    final int SOFIRSTTEXTFIELD = 1;
    final int SOSECTEXTFIELD = 2;
    final int SOTHIRDTEXTFIELD = 3;
    final int SOFOURTHTEXTFIELD = 4;
    int[] SOTEXTFIELDLIST = new int[] { SOFIRSTTEXTFIELD, SOSECTEXTFIELD, SOTHIRDTEXTFIELD, SOFOURTHTEXTFIELD };
    final int SOOPTQUERYMODE = 5;
    int SOIMATCHALL = 0;
    int SOIMATCHANY = 1;
    int ifilterstate = SOIMATCHALL;
    int curHelpID;

    class ItemListenerImpl implements com.sun.star.awt.XItemListener {

        public void itemStateChanged(com.sun.star.awt.ItemEvent EventObject) {
            try {
                int iKey = CurUnoDialog.getControlKey(EventObject.Source, CurUnoDialog.ControlList);
                switch (iKey) {
                    //              case SOOPTQUERYMODE:
                    //                  getfilterstate();
                    default :
                        XControl xControl = (XControl) UnoRuntime.queryInterface(XControl.class, EventObject.Source);
                        XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xControl.getModel());
                        String sName = (String) xPSet.getPropertyValue("Name");
                        String sNameSuffix = sIncSuffix + "_" + sName.substring(sName.length() - 1, sName.length());
                        XListBox xCurFieldListBox = (XListBox) UnoRuntime.queryInterface(XListBox.class, CurUnoDialog.xDlgContainer.getControl(sName));
                        String CurDisplayFieldName = xCurFieldListBox.getSelectedItem();
                        FieldColumn CurFieldColumn = new FieldColumn(oQueryMetaData, CurDisplayFieldName);
                        XControl xValueControl = CurUnoDialog.xDlgContainer.getControl("txtValue" + sNameSuffix);
                        XInterface xValueModel = (XInterface) UnoDialog.getModel(xValueControl);
                        Helper.setUnoPropertyValue(xValueModel, "TreatAsNumber", new Boolean(CurFieldColumn.bIsNumberFormat));
                        Desktop.setNumberFormat(xValueModel, oQueryMetaData.NumberFormats, CurFieldColumn.DBFormatKey);
                        break;
                }
            } catch (Exception exception) {
                exception.printStackTrace(System.out);
            }
        }

        public void disposing(com.sun.star.lang.EventObject eventObject) {
        }
    }

    class TextListenerImpl implements com.sun.star.awt.XTextListener {

        public void textChanged(TextEvent EventObject) {
            try {
                // TODO this redundancy to "itemStateChanged" shoud be removed asap
                XControl xControl = (XControl) UnoRuntime.queryInterface(XControl.class, EventObject.Source);
                XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xControl.getModel());
                String sName = (String) xPSet.getPropertyValue("Name");
                String sNameSuffix = sIncSuffix + "_" + sName.substring(sName.length() - 1, sName.length());
                int Index = Integer.valueOf(sName.substring(sName.length() - 1, sName.length())).intValue();
                XTextComponent xValueTextBox = (XTextComponent) UnoRuntime.queryInterface(XTextComponent.class, CurUnoDialog.xDlgContainer.getControl(sName));
                if (Index < oControlRows.length)
                    oControlRows[Index].setEnabled(oControlRows[Index - 1].isComplete());
            } catch (Exception exception) {
                exception.printStackTrace(System.out);
            }
        }

        public void disposing(EventObject EventObject) {
        }
    }

    class ActionListenerImpl implements com.sun.star.awt.XActionListener {

        public void disposing(EventObject eventObject) {
        }
        public void actionPerformed(com.sun.star.awt.ActionEvent actionEvent) {
            getfilterstate();
        }
    }


    public PropertyValue[][] getFilterConditions() {
        ControlRow CurControlRow;
        Object curValue;
        getfilterstate();
        int filtercount = getFilterCount();
        if (filtercount > 0) {
            if (this.ifilterstate == this.SOIMATCHALL)
                filterconditions = new PropertyValue[1][filtercount];
            else
                filterconditions = new PropertyValue[filtercount][1];
            int a = 0;
            for (int i = 0; i < RowCount; i++) {
                CurControlRow = oControlRows[i];
                if (CurControlRow.isEnabled()) {
                    if (CurControlRow.isComplete()) {
                        String curFieldName = CurControlRow.getSelectedFieldName();
                        int curOperator = (int) CurControlRow.getSelectedOperator() + 1;
                        FieldColumn CurFieldColumn = oQueryMetaData.getFieldColumnByDisplayName(curFieldName);
                        if (CurFieldColumn.StandardFormatKey == oQueryMetaData.iTextFormatKey)
                            curValue = "'" + CurControlRow.getValue() + "'";
//// TODO the following code is bound to be deprecated as soon as the query composer is able to handle date/time values as numbers
                        else if ((CurFieldColumn.StandardFormatKey == oQueryMetaData.iDateFormatKey) || (CurFieldColumn.StandardFormatKey == oQueryMetaData.iDateTimeFormatKey)){
                            String sDate = CurControlRow.getDateTimeString(true);
                            curValue = "{D '" + sDate + "' }";  // FormatsSupplier

                        }
                        else if (CurFieldColumn.StandardFormatKey == oQueryMetaData.iTimeFormatKey){
                            String sTime = CurControlRow.getDateTimeString(true);
                            curValue = "'{T '" + sTime + "' }";
                        }
                        else
                            curValue = CurControlRow.getValue();
                        String sval = String.valueOf(curValue);
                        PropertyValue oPropertyValue = Properties.createProperty(curFieldName, sval, curOperator);
                        if (this.ifilterstate == this.SOIMATCHALL) {
                            if (i == 0)
                                filterconditions[0] = new PropertyValue[filtercount];
                            filterconditions[0][a] = oPropertyValue;
                        } else
                            filterconditions[a][0] = oPropertyValue;
                        a++;
                    }
                }
            }
            int[] iduplicate = JavaTools.getDuplicateFieldIndex(filterconditions);
            if (iduplicate[0] != -1) {
                PropertyValue aduplicatecondition = filterconditions[iduplicate[0]][iduplicate[1]];
                String smsgDuplicateCondition = getDisplayCondition(sDuplicateCondition, aduplicatecondition, null);
                SystemDialog.showMessageBox(CurUnoDialog.xMSF, "WarningBox", VclWindowPeerAttribute.OK, smsgDuplicateCondition);
                CurUnoDialog.vetoableChange(new java.beans.PropertyChangeEvent(CurUnoDialog, "Steps", new Integer(1), new Integer(2)));
                return new PropertyValue[][] {
                };
            }
        }
        return filterconditions;
    }

    public static String getDisplayCondition(String _BaseString, PropertyValue _filtercondition, QueryMetaData _CurDBMetaData) {
        try {
            String FieldName;
            if (_CurDBMetaData != null) {
                FieldColumn CurDBFieldColumn = _CurDBMetaData.getFieldColumnByDisplayName(_filtercondition.Name);
                FieldName = CurDBFieldColumn.AliasName;
            } else
                FieldName = _filtercondition.Name;
            String sreturn = JavaTools.replaceSubString(_BaseString, FieldName, "<FIELDNAME>");
            String soperator = sLogicOperators[_filtercondition.Handle - 1];
            sreturn = JavaTools.replaceSubString(sreturn, soperator, "<LOGICOPERATOR>");
            String sDisplayValue = AnyConverter.toString(_filtercondition.Value);
            sreturn = JavaTools.replaceSubString(sreturn, sDisplayValue, "<VALUE>");
            return sreturn;
        } catch (IllegalArgumentException e) {
            e.printStackTrace(System.out);
            return "";
        }
    }

    private void getfilterstate() {
        if (CurUnoDialog.getControlProperty("optMatchAny", "State") == Boolean.TRUE)
            ifilterstate = SOIMATCHANY;
        else
            ifilterstate = SOIMATCHALL;
    }

    private void addfiltercondition(int _index, String _curFieldName, Object _curValue, int _curOperator) {
        try {
            String ValString = String.valueOf(_curValue);
            PropertyValue oPropertyValue = Properties.createProperty(_curFieldName, ValString, _curOperator);
            getfilterstate();
            if (this.ifilterstate == this.SOIMATCHALL) {
                if (_index == 0)
                    filterconditions[0] = new PropertyValue[getFilterCount()];
                filterconditions[0][_index] = new PropertyValue();
                filterconditions[0][_index] = oPropertyValue;
            }
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    private int getFilterCount() {
        int a = 0;
        for (int i = 0; i < RowCount; i++)
            if (oControlRows[i].isEnabled() && oControlRows[i].isComplete())
                a++;
        FilterCount = a;
        return FilterCount;
    }

    /** Creates a new instance of FilterComponent */
    public FilterComponent(WizardDialog CurUnoDialog, XMultiServiceFactory _xMSF, int iStep, int iPosX, int iPosY, int iWidth, int FilterCount, QueryMetaData _oQueryMetaData, int _firstHelpID) {
        try {
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

            soptMatchAll = CurUnoDialog.oResource.getResText(BaseID + 9);
            soptMatchAny = CurUnoDialog.oResource.getResText(BaseID + 10);
            slblFieldNames = CurUnoDialog.oResource.getResText(BaseID + 17);
            slblOperators = CurUnoDialog.oResource.getResText(BaseID + 24);
            slblValue = CurUnoDialog.oResource.getResText(BaseID + 25);
            sLogicOperators = CurUnoDialog.oResource.getResArray(BaseID + 26, 7);

            sDuplicateCondition = CurUnoDialog.oResource.getResText(BaseID + 89);

            optMatchAll = CurUnoDialog.insertRadioButton("optMatchAll" + sIncSuffix, SOOPTQUERYMODE, new ActionListenerImpl(),
                                            new String[] { "Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width" },
                                            new Object[] { new Integer(9), "HID:" + curHelpID++, soptMatchAll, new Integer(iPosX), new Integer(iStartPosY), new Short((short) 1), IStep, new Short(curtabindex++), new Integer(103)});
            optMatchAny = CurUnoDialog.insertRadioButton("optMatchAny" + sIncSuffix, SOOPTQUERYMODE, new ActionListenerImpl(),
                                            new String[] { "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                            new Object[] { new Integer(9), "HID:" + curHelpID++, soptMatchAny, new Integer(iPosX), new Integer(iStartPosY + 12), IStep, new Short(curtabindex++), new Integer(103)});
            getfilterstate();
            oControlRows = new ControlRow[FilterCount];
            for (int i = 0; i < FilterCount; i++) {
                bEnabled = (i == 0);
                oControlRows[i] = new ControlRow(iStartPosY + 28, i, bEnabled, (this.curHelpID + (i * 3)));
                iStartPosY += 43;
            }
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }


    public void initialize(PropertyValue[][] _filterconditions, String[] _fieldnames) {
        for (int i = 0; i < RowCount; i++)
            oControlRows[i].setFieldNames(_fieldnames);
        this.filterconditions = _filterconditions;
        PropertyValue[] curfilterconditions;
        int a;
        if (_filterconditions.length == 1) {
            curfilterconditions = filterconditions[0];
            for (a = 0; a < curfilterconditions.length; a++)
                oControlRows[a].setCondition(filterconditions[0][a]);
        } else
            for (a = 0; a < filterconditions.length; a++)
                oControlRows[a].setCondition(filterconditions[a][0]);
        while (a < oControlRows.length) {
            oControlRows[a].settovoid();
            if (a > 0)
                oControlRows[a].setEnabled(oControlRows[a - 1].isComplete());
            a++;
        }
    }


    public void addNumberFormatsSupplier(XNumberFormatsSupplier _xNumberFormatsSupplier){
        try {
            xNumberFormatsSupplier = _xNumberFormatsSupplier;
            iDateFormat = Desktop.defineNumberFormat(xNumberFormatsSupplier.getNumberFormats(), "YYYY-MM-DD");
            iTimeFormat = Desktop.defineNumberFormat(xNumberFormatsSupplier.getNumberFormats(), "HH:MM:SS");
            xNumberFormatter = Desktop.createNumberFormatter(xMSF, xNumberFormatsSupplier);
        } catch (NotNumericException e) {
        } catch (Exception e) {
            e.printStackTrace(System.out);
        }
    }



        class ControlRow {
            protected XInterface[] ControlElements = new XInterface[6];
            private boolean bEnabled;
            String[] FieldNames;

            protected ControlRow(int iCompPosY, int Index, boolean _bEnabled, int _firstRowHelpID) {
            try {
                String sCompSuffix = sIncSuffix + "_" + String.valueOf(Index + 1);
                this.bEnabled = _bEnabled;
                ControlElements[0] = (XInterface) CurUnoDialog.insertLabel("lblFieldNames" + sCompSuffix,
                                        new String[] { "Enabled", "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                        new Object[] { new Boolean(bEnabled), new Integer(9), slblFieldNames, new Integer(iStartPosX + 6), new Integer(iCompPosY + 13), IStep, new Short(curtabindex++), new Integer(55)});
                ControlElements[1] = (XInterface) CurUnoDialog.insertLabel("lblOperators" + sCompSuffix,
                                        new String[] { "Enabled", "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                        new Object[] { new Boolean(bEnabled), new Integer(9), slblOperators, new Integer(iStartPosX + 83), new Integer(iCompPosY + 13), IStep, new Short(curtabindex++), new Integer(52)});
                ControlElements[2] = (XInterface) CurUnoDialog.insertLabel("lblValue" + sCompSuffix,
                                        new String[] { "Enabled", "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                        new Object[] { new Boolean(bEnabled), new Integer(9), slblValue, new Integer(iStartPosX + 158), new Integer(iCompPosY + 13), IStep, new Short(curtabindex++), new Integer(32)});
                ControlElements[3] = (XInterface) CurUnoDialog.insertListBox("lstFieldName" + sCompSuffix, SOFIELDNAMELIST[Index], null,
                                        new ItemListenerImpl(), new String[] { "Enabled", "Dropdown", "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                        new Object[] { new Boolean(bEnabled), Boolean.TRUE, new Integer(13), "HID:" + _firstRowHelpID++, new Integer(iStartPosX + 6), new Integer(iCompPosY + 23), IStep, new Short(curtabindex++), new Integer(71)});
                ControlElements[4] = (XInterface) CurUnoDialog.insertListBox("lstOperator" + sCompSuffix, LISTBOXOPERATORS_ACTION_PERFORMED,    LISTBOXOPERATORS_ITEM_CHANGED,
                                        new String[] { "Enabled", "Dropdown", "Height", "HelpURL", "LineCount", "PositionX", "PositionY", "Step", "StringItemList", "TabIndex", "Width" },
                                        new Object[] { new Boolean(bEnabled), Boolean.TRUE, new Integer(13), "HID:" + _firstRowHelpID++, new Short((short) 7), new Integer(iStartPosX + 83), new Integer(iCompPosY + 23), IStep, sLogicOperators, new Short(curtabindex++), new Integer(70)});

                ControlElements[5] = (XInterface) CurUnoDialog.insertFormattedField("txtValue" + sCompSuffix, SOTEXTFIELDLIST[Index], new TextListenerImpl(),
                                        new String[] { "Enabled", "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                        new Object[] { new Boolean(bEnabled), new Integer(13), "HID:" + _firstRowHelpID++, new Integer(iStartPosX + 158), new Integer(iCompPosY + 23), IStep, new Short(curtabindex++), new Integer(43)});
            } catch (Exception exception) {
                exception.printStackTrace(System.out);
            }}

            boolean isComplete() {
                try {
                    short[] SelFields = (short[]) AnyConverter.toArray(Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTFIELDNAME]), "SelectedItems"));
                    if (SelFields.length > 0) {
                        short[] SelOperator = (short[]) AnyConverter.toArray(Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTOPERATOR]), "SelectedItems"));
                        if (SelOperator.length > 0) {
                            return (!AnyConverter.isVoid(Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), "EffectiveValue")));
                        }
                    }
                    return false;
                } catch (Exception exception) {
                    exception.printStackTrace(System.out);
                    return false;
                }
            }

            protected void setCondition(PropertyValue _filtercondition) {
            try {
                int ikey;
                XListBox xFieldsListBox = (XListBox) UnoRuntime.queryInterface(XListBox.class, ControlElements[SOLSTFIELDNAME]);
                xFieldsListBox.selectItem(_filtercondition.Name, true);
                XListBox xOperatorListBox = (XListBox) UnoRuntime.queryInterface(XListBox.class, ControlElements[SOLSTOPERATOR]);
                xOperatorListBox.selectItemPos((short) (_filtercondition.Handle - 1), true);
                if (AnyConverter.isString(_filtercondition.Value)){
                    String sValue = AnyConverter.toString(_filtercondition.Value);
                    if (sValue.indexOf("{D '") > -1){
                        sValue = JavaTools.replaceSubString(sValue, "","{D '");
                        sValue = JavaTools.replaceSubString(sValue, "","' }");
                        xNumberFormatter.convertStringToNumber(iDateFormat, sValue);
                    }
                    else if (sValue.indexOf("{T '") > -1){
                        sValue = JavaTools.replaceSubString(sValue, "","{T '");
                        sValue = JavaTools.replaceSubString(sValue, "","' }");
                        xNumberFormatter.convertStringToNumber(iTimeFormat, sValue);
                    }
                }
                else
                    Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), "EffectiveValue", _filtercondition.Value);
            } catch (Exception e) {
                e.printStackTrace(System.out);
            }}


            protected void setFieldNames(String[] _FieldNames) {
                Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTFIELDNAME]), "StringItemList", _FieldNames);
                FieldNames = _FieldNames;
            }

            protected boolean isEnabled() {
                return bEnabled;
            }

            protected void settovoid() {
                CurUnoDialog.deselectListBox(ControlElements[SOLSTFIELDNAME]);
                CurUnoDialog.deselectListBox(ControlElements[SOLSTOPERATOR]);
                Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), "EffectiveValue", com.sun.star.uno.Any.VOID);
            }

            protected void setEnabled(boolean _bEnabled) {
                for (int i = 0; i < ControlElements.length; i++)
                    Helper.setUnoPropertyValue(UnoDialog.getModel(ControlElements[i]), "Enabled", new Boolean(_bEnabled));
                bEnabled = _bEnabled;
            }

            protected String getSelectedFieldName() {
                try {
                    short[] SelFields = (short[]) AnyConverter.toArray(Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTFIELDNAME]), "SelectedItems"));
                    return FieldNames[SelFields[0]];
                } catch (Exception exception) {
                    exception.printStackTrace(System.out);
                    return null;
                }
            }

            // TODO: check if it is really useful to match the indices of the listbox the API constants
            protected short getSelectedOperator() {
                try {
                    short[] SelFields = (short[]) AnyConverter.toArray(Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOLSTOPERATOR]), "SelectedItems"));
                    return SelFields[0];
                } catch (Exception exception) {
                    exception.printStackTrace(System.out);
                    return -1;
                }
            }

            // TODO make a difference between Text and Numbers
            protected Object getValue() {
                try {
                    return (Helper.getUnoPropertyValue(UnoDialog.getModel(ControlElements[SOTXTVALUE]), "EffectiveValue"));
                } catch (Exception exception) {
                    exception.printStackTrace(System.out);
                    return null;
                }
            }

            protected String getDateTimeString(boolean bgetDate) {
            try {
                double dblValue = ((Double) getValue()).doubleValue();
                return Desktop.convertNumberToString(xNumberFormatter, iDateFormat, dblValue);
            } catch (Exception exception) {
                exception.printStackTrace(System.out);
                return null;
            }}
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
