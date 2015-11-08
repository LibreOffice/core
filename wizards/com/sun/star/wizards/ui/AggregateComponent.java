/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package com.sun.star.wizards.ui;

import java.util.ArrayList;
import java.util.Vector;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.db.*;
import com.sun.star.lang.EventObject;
import com.sun.star.beans.*;
import com.sun.star.sdbc.SQLException;
import com.sun.star.uno.AnyConverter;

import com.sun.star.awt.*;

public class AggregateComponent extends ControlScroller
{

    String[] sFunctions;
    String[] sFunctionOperators = new String[]
    {
        "SUM", "AVG", "MIN", "MAX", "COUNT"
    };
    QueryMetaData CurDBMetaData;
    XButton optDetailQuery;
    XButton optSummaryQuery;
    String soptDetailQuery;
    String soptSummaryQuery;
    String slblAggregate;
    String slblFieldNames;
    String sDuplicateAggregateFunction;
    int Count;
    int iQueryType;
    final int SOADDROW = 1;
    final int SOREMOVEROW = 2;
    final int CONTROLROWDIST = 18;
    Vector ControlRowVector;
    String OPTIONBUTTONDETAILQUERY_ITEM_CHANGED = "toggleComponent";
    String OPTIONBUTTONSUMMARYQUERY_ITEM_CHANGED = "toggleComponent";
    String LISTBOXFUNCTIONS_ACTION_PERFORMED;
    String LISTBOXFUNCTIONS_ITEM_CHANGED;
    String LISTBOXFIELDNAMES_ACTION_PERFORMED;
    String LISTBOXFIELDNAMES_ITEM_CHANGED;
    String COMMANDBUTTONPLUS_ACTION_PERFORMED = "addRow";
    String COMMANDBUTTONMINUS_ACTION_PERFORMED = "removeRow";
    Vector ControlRows;
    int curHelpID;
    int lastHelpIndex;

    /** Creates a new instance of AggrgateComponent */
    public AggregateComponent(WizardDialog _CurUnoDialog, QueryMetaData _CurDBMetaData, int _iStep, int _iPosX, int _iPosY, int _iWidth, int _uitextfieldcount, int _firstHelpID)
    {
        super(_CurUnoDialog, _CurDBMetaData.xMSF, _iStep, _iPosX + 10, _iPosY, _iWidth - 12, _uitextfieldcount, 18, _firstHelpID + 2);
        try
        {
            curHelpID = _firstHelpID;
            this.CurDBMetaData = _CurDBMetaData;
            Count = 1;
            optDetailQuery = CurUnoDialog.insertRadioButton("optDetailQuery", 0, new ActionListenerImpl(),
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        8, HelpIds.getHelpIdString(curHelpID), soptDetailQuery, new Integer(_iPosX), new Integer(iCompPosY - 42), new Short((short) 1), IStep, new Short(curtabindex++), new Integer(iCompWidth)
                    });

            optSummaryQuery = CurUnoDialog.insertRadioButton("optSummaryQuery", 0, new ActionListenerImpl(),
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        16, HelpIds.getHelpIdString(curHelpID + 1), soptSummaryQuery, Boolean.TRUE, new Integer(_iPosX), new Integer(iCompPosY - 32), IStep, new Short(curtabindex++), new Integer(iCompWidth)
                    });
            CurUnoDialog.insertLabel("lblAggregate",
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        8, slblAggregate, new Integer(iCompPosX + 5), new Integer(iCompPosY - 10), IStep, new Short(curtabindex++), 90
                    });
            CurUnoDialog.insertLabel("lblFieldnames",
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        8, slblFieldNames, new Integer(iCompPosX + 101), new Integer(iCompPosY - 10), IStep, new Short(curtabindex++), 90
                    });
            this.setTotalFieldCount(1);
            FontDescriptor oFontDescriptor = new FontDescriptor();
            oFontDescriptor.Weight = com.sun.star.awt.FontWeight.BOLD;
            oFontDescriptor.Height = (short) 14;

            int iButtonPosY = iCompPosY + iCompHeight + 3;
            CurUnoDialog.insertButton("btnplus", SOADDROW, new ActionListenerImpl(),
                    new String[]
                    {
                        PropertyNames.FONT_DESCRIPTOR, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        oFontDescriptor, 14, HelpIds.getHelpIdString(lastHelpIndex + 1), "+", new Integer(_iPosX + iCompWidth - 36), new Integer(iButtonPosY), IStep, new Short((curtabindex++)), 16
                    });
            CurUnoDialog.insertButton("btnminus", SOREMOVEROW, new ActionListenerImpl(),
                    new String[]
                    {
                        PropertyNames.FONT_DESCRIPTOR, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        oFontDescriptor, 14, HelpIds.getHelpIdString(lastHelpIndex + 2), "-", new Integer(_iPosX + iCompWidth - 16), new Integer(iButtonPosY), IStep, new Short(curtabindex++), 16
                    });
            CurDBMetaData.Type = getQueryType();
        }
        catch (com.sun.star.uno.Exception exception)
        {
            Resource.showCommonResourceError(CurDBMetaData.xMSF);
        }
    }

    public int getQueryType()
    {
        if (((Short) CurUnoDialog.getControlProperty("optDetailQuery", PropertyNames.PROPERTY_STATE)).intValue() == 1)
        {
            return QueryMetaData.QueryType.SODETAILQUERY;
        }
        else
        {
            return QueryMetaData.QueryType.SOSUMMARYQUERY;
        }
    }

    class ActionListenerImpl implements com.sun.star.awt.XActionListener
    {

        public void disposing(EventObject eventObject)
        {
        }

        public void actionPerformed(com.sun.star.awt.ActionEvent actionEvent)
        {
            try
            {
                int iKey = CurUnoDialog.getControlKey(actionEvent.Source, CurUnoDialog.ControlList);
                switch (iKey)
                {
                    case SOADDROW:
                        addRow();
                        break;

                    case SOREMOVEROW:
                        removeRow();
                        break;

                    default:
                        toggleComponent();
                        break;
                }
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.out);
            }
        }
    }

    public int getCount()
    {
        return Count;
    }

    protected void insertControlGroup(int i, int ypos)
    {
        if (i == 0)
        {
            soptDetailQuery = CurUnoDialog.m_oResource.getResText(UIConsts.RID_QUERY + 11);
            soptSummaryQuery = CurUnoDialog.m_oResource.getResText(UIConsts.RID_QUERY + 12);
            slblAggregate = CurUnoDialog.m_oResource.getResText(UIConsts.RID_QUERY + 16);
            slblFieldNames = CurUnoDialog.m_oResource.getResText(UIConsts.RID_QUERY + 17);
            sFunctions = CurUnoDialog.m_oResource.getResArray(UIConsts.RID_QUERY + 40, 5);

            sDuplicateAggregateFunction = CurUnoDialog.m_oResource.getResText(UIConsts.RID_QUERY + 90);
        }
        if (ControlRowVector == null)
        {
            ControlRowVector = new Vector();
        }
        int locHelpID = curHelpIndex + (i * 2);
        ControlRow oControlRow = new ControlRow(i, ypos, locHelpID);
        ControlRowVector.add(oControlRow);
    }

    protected void setControlGroupVisible(int _index, boolean _bIsVisible)
    {
        ControlRow oControlRow = (ControlRow) ControlRowVector.elementAt(_index);
        oControlRow.setVisible(_bIsVisible);
        if (_index >= (this.CurDBMetaData.AggregateFieldNames.length))
        {
            oControlRow.settovoid();
        }
    }

    protected void addRow()
    {
        int fieldcount = super.getTotalFieldCount();
        registerControlGroupAtIndex(fieldcount);
        if (fieldcount < super.getBlockIncrementation())
        {
            ControlRow oControlRow = (ControlRow) ControlRowVector.elementAt(fieldcount);
            oControlRow.setVisible(true);
            oControlRow.settovoid();
        }
        else
        {
            ControlRow oControlRow = (ControlRow) ControlRowVector.elementAt(super.getBlockIncrementation() - 1);
            super.setScrollValue(getScrollValue() + 1, (fieldcount + 1));
            oControlRow.settovoid();
        }
        fieldcount++;
        super.setTotalFieldCount(fieldcount);
        toggleButtons();
        CurUnoDialog.repaintDialogStep();
    }

    protected void removeRow()
    {
        int fieldcount = super.getTotalFieldCount();
        if (fieldcount > 0)
        {
            ControlRow oControlRow;
            fieldcount--;
            if ((fieldcount + 1) <= super.getBlockIncrementation())
            {
                oControlRow = (ControlRow) ControlRowVector.elementAt(fieldcount);
                oControlRow.setVisible(false);
            }
            super.setScrollValue(getScrollValue() - 1, (fieldcount));
            super.unregisterControlGroup(fieldcount);

        }
        toggleButtons();
        CurUnoDialog.repaintDialogStep();
    }

    protected void toggleButtons()
    {
        ControlRow curcontrolrow = null;
        boolean biscomplete = true;
        CurDBMetaData.Type = getQueryType();
        CurUnoDialog.setControlProperty("btnminus", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf((super.getTotalFieldCount() > 0) && (CurDBMetaData.Type == QueryMetaData.QueryType.SOSUMMARYQUERY)));
        int fieldcount = super.getCurFieldCount();
        if (fieldcount > 0)
        {
            curcontrolrow = (ControlRow) ControlRowVector.elementAt(super.getCurFieldCount() - 1);
            biscomplete = curcontrolrow.isComplete();
        }
        CurUnoDialog.setControlProperty("btnplus", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(biscomplete && (CurDBMetaData.Type == QueryMetaData.QueryType.SOSUMMARYQUERY)));
        togglefollowingDialogSteps();
    }

    public void toggleComponent()
    {
        CurDBMetaData.Type = getQueryType();
        boolean benableComponent = isAggregateComponentEnabled();
        CurUnoDialog.setControlProperty("lblAggregate", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(benableComponent));
        CurUnoDialog.setControlProperty("lblFieldnames", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(benableComponent));
        toggleButtons();
        super.toggleComponent(benableComponent);
        super.toggleControls(benableComponent);
        togglefollowingDialogSteps();
    }

    private boolean isAggregateComponentEnabled()
    {
        return (CurDBMetaData.Type == QueryMetaData.QueryType.SOSUMMARYQUERY);
    }

    public boolean isGroupingpossible()
    {
        try
        {
            boolean benableGroupPage = isAggregateComponentEnabled() && CurDBMetaData.xDBMetaData.supportsGroupBy() && hasonlycompletefunctions();
            String[][] sAggregateFieldNames = this.getAggregateFieldNames();
            if (benableGroupPage)
            {
                for (int i = 0; i < CurDBMetaData.NumericFieldNames.length; i++)
                {
                    boolean bisthere = (JavaTools.FieldInTable(sAggregateFieldNames, CurDBMetaData.NumericFieldNames[i]) > -1);
                    if (!bisthere)
                    {
                        return true;
                    }
                }
            }
        }
        catch (SQLException e)
        {
            e.printStackTrace(System.out);
        }
        return false;
    }

    private void togglefollowingDialogSteps()
    {
        boolean benabletherest = true;
        boolean benableGroupPage = isGroupingpossible();
        CurUnoDialog.setStepEnabled(UIConsts.SOGROUPSELECTIONPAGE, benableGroupPage);
        CurUnoDialog.setStepEnabled(UIConsts.SOGROUPFILTERPAGE, benableGroupPage && CurDBMetaData.GroupFieldNames.length > 0);
        if (isAggregateComponentEnabled())
        {
            benabletherest = hasonlycompletefunctions();
        }
        CurUnoDialog.enablefromStep(UIConsts.SOTITLESPAGE, benabletherest);
        CurUnoDialog.enableNextButton(benabletherest);
    }

    private void hideControlRowsfromindex(int _index)
    {
        if (_index < this.ControlRowVector.size())
        {
            for (int i = _index; i < ControlRowVector.size(); i++)
            {
                ControlRow oControlRow = (ControlRow) ControlRowVector.elementAt(i);
//              if (i == _index)
//                  oControlRow.settovoid();
//              else
                oControlRow.setVisible(false);
            }
        }

    }

    public String[][] getAggregateFieldNames()
    {
        try
        {
            CurDBMetaData.Type = getQueryType();
            if (CurDBMetaData.Type == QueryMetaData.QueryType.SOSUMMARYQUERY)
            {
                ArrayList<String[]> aggregatevector = new ArrayList<String[]>();
                PropertyValue[][] aggregatelist = this.getScrollFieldValues();
                PropertyValue[] currowproperties;
                PropertyValue curaggregateproperty;
                if (CurDBMetaData.AggregateFieldNames != null)
                {
                    for (int i = 0; i < aggregatelist.length; i++)
                    {
                        currowproperties = aggregatelist[i];
                        if ((currowproperties[0].Value != null) && (currowproperties[1].Value != null))
                        {
                            short[] iselfield = (short[]) AnyConverter.toArray(currowproperties[1].Value);
                            short[] iselfunction = (short[]) AnyConverter.toArray(currowproperties[0].Value);
                            if ((iselfield.length > 0) && (iselfunction.length > 0))
                            {
                                String[] curaggregatename = new String[2];
                                curaggregatename[0] = CurDBMetaData.NumericFieldNames[iselfield[0]];
                                curaggregatename[1] = this.sFunctionOperators[iselfunction[0]];
                                aggregatevector.add(curaggregatename);
                            }
                        }
                    }
                }
                CurDBMetaData.AggregateFieldNames = new String[aggregatevector.size()][2];
                aggregatevector.toArray(CurDBMetaData.AggregateFieldNames);
            }

            int iduplicate = JavaTools.getDuplicateFieldIndex(CurDBMetaData.AggregateFieldNames);
            if (iduplicate != -1)
            {
                sDuplicateAggregateFunction = JavaTools.replaceSubString(sDuplicateAggregateFunction, CurDBMetaData.AggregateFieldNames[iduplicate][0], "<NUMERICFIELD>");
                int index = JavaTools.FieldInList(sFunctionOperators, CurDBMetaData.AggregateFieldNames[iduplicate][1]);
                String sDisplayFunction = sFunctions[index];
                sDuplicateAggregateFunction = JavaTools.replaceSubString(sDuplicateAggregateFunction, sDisplayFunction, "<FUNCTION>");
                CurUnoDialog.showMessageBox("WarningBox", VclWindowPeerAttribute.OK, sDuplicateAggregateFunction);
                CurUnoDialog.vetoableChange(new java.beans.PropertyChangeEvent(CurUnoDialog, "Steps", 1, 2));
                return new String[][]
                        {
                        };
            }
            else
            {
                return CurDBMetaData.AggregateFieldNames;
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    public void initialize()
    {
        CurDBMetaData.setNumericFields();
        initializeScrollFields();
        int icount = CurDBMetaData.AggregateFieldNames.length;
        if (icount == 0)
        {
            super.initialize(1);
            hideControlRowsfromindex(1);
        }
        else
        {
            super.initialize(icount);
            hideControlRowsfromindex(icount);
        }
        toggleComponent();
        CurUnoDialog.repaintDialogStep();
    }

    protected void initializeScrollFields()
    {
        ControlRow curControlRow;
        if (CurDBMetaData.AggregateFieldNames != null)
        {
            for (int i = 0; i < this.getBlockIncrementation(); i++)
            {
                curControlRow = (ControlRow) ControlRowVector.elementAt(i);
                curControlRow.insertFieldNames();
            }
            for (int i = 0; i <= CurDBMetaData.AggregateFieldNames.length; i++)
            {
                registerControlGroupAtIndex(i);
            }
        }
    }

    private void registerControlGroupAtIndex(int _index)
    {
        short[] iselfunctionlist = new short[]
        {
        };
        short[] iselfieldslist = new short[]
        {
        };
        PropertyValue[] currowproperties = new PropertyValue[2];
        if (_index < CurDBMetaData.AggregateFieldNames.length)
        {
            short iselfieldsindex = (short) JavaTools.FieldInList(CurDBMetaData.NumericFieldNames, CurDBMetaData.AggregateFieldNames[_index][0]);
            iselfieldslist = new short[]
                    {
                        iselfieldsindex
                    };
            short iselfunctionindex = (short) JavaTools.FieldInList(sFunctionOperators, CurDBMetaData.AggregateFieldNames[_index][1]);
            iselfunctionlist = new short[]
                    {
                        iselfunctionindex
                    };
        }
        currowproperties[0] = Properties.createProperty(getFunctionControlName(_index), iselfunctionlist, _index);
        currowproperties[1] = Properties.createProperty(getFieldsControlName(_index), iselfieldslist, _index);     //getTitleName(i)
        super.registerControlGroup(currowproperties, _index);
    }

    private String getFunctionControlName(int _index)
    {
        String namesuffix = "_" + String.valueOf(_index + 1);
        return "lstfunctions" + namesuffix;
    }

    private String getFieldsControlName(int _index)
    {
        String namesuffix = "_" + String.valueOf(_index + 1);
        return "lstFieldnames" + namesuffix;
    }

    private boolean hasonlycompletefunctions()
    {
        int maxfieldcount = super.getCurFieldCount();
        if (maxfieldcount > 0)
        {
            ControlRow curcontrolrow = (ControlRow) this.ControlRowVector.elementAt(maxfieldcount - 1);
            return curcontrolrow.isComplete();
        }
        else
        {
            return false;
        }
    }

    protected class ControlRow
    {

        private XListBox xFieldListBox;
        private XListBox xFunctionListBox;
        private int index;

        protected ControlRow(int _index, int ypos, int _curHelpID)
        {
            try
            {
                this.index = _index;
                xFunctionListBox = CurUnoDialog.insertListBox(getFunctionControlName(index), 1, null, new ItemListenerImpl(),
                        new String[]
                        {
                            "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.STRING_ITEM_LIST, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                        },
                        new Object[]
                        {
                            Boolean.TRUE, 12, HelpIds.getHelpIdString(_curHelpID++), new Integer(iCompPosX + 4), new Integer(ypos), UIConsts.INVISIBLESTEP, sFunctions, new Short(curtabindex++), 88
                        });

                xFieldListBox = CurUnoDialog.insertListBox(getFieldsControlName(index), 1, null, new ItemListenerImpl(),
                        new String[]
                        {
                            "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                        },
                        new Object[]
                        {
                            Boolean.TRUE, 12, HelpIds.getHelpIdString(_curHelpID++), new Integer(iCompPosX + 98), new Integer(ypos), UIConsts.INVISIBLESTEP, new Short(curtabindex++), 86
                        });
                lastHelpIndex = _curHelpID - 1;
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.out);
            }
        }

        private String getSelectedFieldName()
        {
            return xFieldListBox.getSelectedItem();
        }

        private String getSelectedFunction()
        {
            return xFunctionListBox.getSelectedItem();
        }

        private void setVisible(boolean _bvisible)
        {
            CurUnoDialog.setControlVisible(getFunctionControlName(index), _bvisible);
            CurUnoDialog.setControlVisible(getFieldsControlName(index), _bvisible);
        }

        private void insertFieldNames()
        {
            Helper.setUnoPropertyValue(UnoDialog.getModel(xFieldListBox), PropertyNames.STRING_ITEM_LIST, CurDBMetaData.NumericFieldNames);
        }

        private boolean isComplete()
        {
            boolean bfieldnameisselected = (Helper.getUnoArrayPropertyValue(UnoDialog.getModel(xFieldListBox), PropertyNames.SELECTED_ITEMS) != null);
            boolean bfunctionisselected = (Helper.getUnoArrayPropertyValue(UnoDialog.getModel(xFunctionListBox), PropertyNames.SELECTED_ITEMS) != null);
            return (bfieldnameisselected && bfunctionisselected);
        }

        private void settovoid()
        {
            CurUnoDialog.deselectListBox(xFieldListBox);
            CurUnoDialog.deselectListBox(xFunctionListBox);
        }

        protected class ItemListenerImpl implements com.sun.star.awt.XItemListener
        {

            public void itemStateChanged(com.sun.star.awt.ItemEvent EventObject)
            {
                try
                {
                    toggleButtons();
                    togglefollowingDialogSteps();
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
    }
}
