/*************************************************************************
 *
 *  $RCSfile: AggregateComponent.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-05-19 13:03:55 $
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
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.db.*;
import com.sun.star.lang.EventObject;
//import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.beans.*;
import com.sun.star.sdbc.SQLException;
import com.sun.star.uno.AnyConverter;

import com.sun.star.awt.*;


public class AggregateComponent extends ControlScroller{

    String[] sFunctions;
    String[] sFunctionOperators = new String[] {"SUM", "AVG", "MIN", "MAX"};;
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
    String COMMANDBUTTONPLUS_ACTION_PERFORMED    = "addRow";
    String COMMANDBUTTONMINUS_ACTION_PERFORMED  = "removeRow";
    Vector ControlRows;
    int curHelpID;
    int lastHelpIndex;



    /** Creates a new instance of AggrgateComponent */
    public AggregateComponent(WizardDialog _CurUnoDialog, QueryMetaData _CurDBMetaData, int _iStep, int _iPosX, int _iPosY, int _iWidth, int _uitextfieldcount, int _firstHelpID){
    super(_CurUnoDialog, _CurDBMetaData.xMSF,  _iStep, _iPosX, _iPosY, _iWidth, _uitextfieldcount, 18, _firstHelpID+2);
    try{
        curHelpID = _firstHelpID;
        this.CurDBMetaData = _CurDBMetaData;;
        Count = 1;
        optDetailQuery = CurUnoDialog.insertRadioButton("optDetailQuery", 0, new ActionListenerImpl(),
            new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"},
            new Object[] { new Integer(8), "HID:" + curHelpID, soptDetailQuery, new Integer(iCompPosX), new Integer(iCompPosY-34), new Short((short)1), IStep, new Short(curtabindex++), new Integer(iCompWidth)}
        );

        optSummaryQuery = CurUnoDialog.insertRadioButton("optSummaryQuery", 0, new ActionListenerImpl(),
            new String[] {"Height", "HelpURL","Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
            new Object[] { new Integer(10), "HID:" + (curHelpID+1), soptSummaryQuery, new Integer(iCompPosX), new Integer(iCompPosY-24), IStep, new Short(curtabindex++), new Integer(iCompWidth)}
        );
        CurUnoDialog.insertLabel("lblAggregate",
            new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
            new Object[] { new Integer(8),slblAggregate, new Integer( iCompPosX + 5), new Integer(iCompPosY-10), IStep, new Short(curtabindex++), new Integer(50)}
        );
        CurUnoDialog.insertLabel("lblFieldnames",
            new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
            new Object[] {new Integer(8),slblFieldNames, new Integer(iCompPosX + 107), new Integer(iCompPosY-10),IStep, new Short(curtabindex++), new Integer(57)}
        );
        this.setTotalFieldCount(1);
        FontDescriptor oFontDescriptor = new FontDescriptor();
        oFontDescriptor.Weight = com.sun.star.awt.FontWeight.BOLD;
        oFontDescriptor.Height = (short) 14;

        int iButtonPosY = iCompPosY + iCompHeight + 3;
        CurUnoDialog.insertButton("btnplus", SOADDROW, new ActionListenerImpl(),
            new String[] {"FontDescriptor", "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
            new Object[] {oFontDescriptor, new Integer(14), "HID:" + (lastHelpIndex+1), "+", new Integer(_iPosX + iCompWidth - 36), new Integer(iButtonPosY), IStep, new Short((curtabindex++)), new Integer(16)}
        );
        CurUnoDialog.insertButton("btnminus", SOREMOVEROW, new ActionListenerImpl(),
            new String[] {"FontDescriptor", "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
            new Object[] {oFontDescriptor, new Integer(14), "HID:" + (lastHelpIndex+2), "-" , new Integer(_iPosX + iCompWidth - 16), new Integer(iButtonPosY), IStep, new Short(curtabindex++), new Integer(16)}
        );
        CurDBMetaData.Type = getQueryType();
    }
    catch(com.sun.star.uno.Exception exception){
        Resource.showCommonResourceError(CurDBMetaData.xMSF);
    }}


    public int getQueryType(){
        if (((Short) CurUnoDialog.getControlProperty("optDetailQuery", "State")).intValue() == 1)
            return QueryMetaData.QueryType.SODETAILQUERY;
        else
            return QueryMetaData.QueryType.SOSUMMARYQUERY;
    }



    class ActionListenerImpl implements com.sun.star.awt.XActionListener{

        public void disposing(EventObject eventObject){
        }

        public void actionPerformed(com.sun.star.awt.ActionEvent actionEvent){
        try{
            int iKey  =  CurUnoDialog.getControlKey(actionEvent.Source, CurUnoDialog.ControlList);
            switch (iKey){
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
        catch( Exception exception ){
            exception.printStackTrace(System.out);
        }}
    }


    public int getCount(){
        return Count;
    }


    protected void insertControlGroup(int i, int ypos){
        if (i == 0){
            int BaseID = 2300;
            soptDetailQuery = CurUnoDialog.oResource.getResText(BaseID + 11);
            soptSummaryQuery = CurUnoDialog.oResource.getResText(BaseID + 12);
            slblAggregate = CurUnoDialog.oResource.getResText(BaseID + 16);
            slblFieldNames = CurUnoDialog.oResource.getResText(BaseID + 17);
            sFunctions = CurUnoDialog.oResource.getResArray(BaseID + 40, 4);

            sDuplicateAggregateFunction = CurUnoDialog.oResource.getResText(BaseID + 90);
        }
        if (ControlRowVector == null)
            ControlRowVector = new Vector();
        int locHelpID = curHelpIndex + (i * 2);
        ControlRow oControlRow = new ControlRow(i, ypos, locHelpID);
        ControlRowVector.add(oControlRow);
    }


    protected void setControlGroupVisible(int _index, boolean _bIsVisible){
        ControlRow oControlRow = (ControlRow) ControlRowVector.elementAt(_index);
        oControlRow.setVisible(_bIsVisible);
        if (_index >= (this.CurDBMetaData.AggregateFieldNames.length))
            oControlRow.settovoid();
    }


    protected void addRow(){
        int fieldcount = super.getTotalFieldCount();
        registerControlGroupAtIndex(fieldcount);
        if ( fieldcount < super.getBlockIncrementation()){
            ControlRow oControlRow = (ControlRow) ControlRowVector.elementAt(fieldcount);
            oControlRow.setVisible(true);
        }
        else{
             ControlRow oControlRow = (ControlRow) ControlRowVector.elementAt(super.getBlockIncrementation()-1);
            super.setScrollValue(getScrollValue() + 1, (fieldcount+1));
            oControlRow.settovoid();
        }
        fieldcount++;
        super.setTotalFieldCount(fieldcount);
        toggleButtons();
    }


    protected void removeRow(){
        int fieldcount = super.getTotalFieldCount();
        if ( fieldcount > 1){
            ControlRow oControlRow;
            fieldcount--;
            if ((fieldcount+1) <= super.getBlockIncrementation()){
                oControlRow = (ControlRow) ControlRowVector.elementAt(fieldcount);
                oControlRow.setVisible(false);
            }
            super.setScrollValue(getScrollValue()-1, (fieldcount));
            super.unregisterControlGroup(fieldcount);

        }
        toggleButtons();
    }


    protected void toggleButtons(){
        CurDBMetaData.Type = getQueryType();
        CurUnoDialog.setControlProperty("btnminus", "Enabled",  new Boolean((super.getTotalFieldCount()> 1) && (CurDBMetaData.Type == QueryMetaData.QueryType.SOSUMMARYQUERY)));
        ControlRow curcontrolrow = (ControlRow) ControlRowVector.elementAt(super.getCurFieldCount()-1);
        CurUnoDialog.setControlProperty("btnplus", "Enabled",  new Boolean((curcontrolrow.isComplete()) && (CurDBMetaData.Type == QueryMetaData.QueryType.SOSUMMARYQUERY)));
    }


    public void toggleComponent(){
        CurDBMetaData.Type = getQueryType();
        boolean benableComponent = (CurDBMetaData.Type == QueryMetaData.QueryType.SOSUMMARYQUERY);
        CurUnoDialog.setControlProperty("lblAggregate", "Enabled", new Boolean(benableComponent));
        CurUnoDialog.setControlProperty("lblFieldnames", "Enabled", new Boolean(benableComponent));
        toggleButtons();
        super.toggleComponent(benableComponent);
        super.toggleControls(benableComponent);
        togglefollowingDialogSteps(benableComponent);
    }


    private void togglefollowingDialogSteps(boolean _benableComponent){
    try {
        boolean benableGroupPage = _benableComponent && CurDBMetaData.xDBMetaData.supportsGroupBy() && hascompletefunctions();
        CurUnoDialog.setStepEnabled(UIConsts.SOGROUPSELECTIONPAGE, benableGroupPage);
        CurUnoDialog.setStepEnabled(UIConsts.SOGROUPFILTERPAGE, benableGroupPage && CurDBMetaData.GroupFieldNames.length > 0);
        boolean benabletherest = (!_benableComponent) || (_benableComponent && hascompletefunctions());
        CurUnoDialog.enablefromStep(UIConsts.SOTITLESPAGE, benabletherest);
    } catch (SQLException e) {
        e.printStackTrace();
    }}

    private void hideControlRowsfromindex(int _index){
        if (_index < this.ControlRowVector.size()){
            for (int i = _index; i < ControlRowVector.size();i++){
                ControlRow oControlRow = (ControlRow) ControlRowVector.elementAt(i);
                if (i == _index)
                    oControlRow.settovoid();
                else
                    oControlRow.setVisible(false);
            }
        }

    }

    public String[][] getAggregateFieldNames(){
    try{
        CurDBMetaData.Type = getQueryType();
        if (CurDBMetaData.Type == QueryMetaData.QueryType.SOSUMMARYQUERY){
            Vector aggregatevector = new Vector();
            PropertyValue[][] aggregatelist = this.getScrollFieldValues();
            PropertyValue[] currowproperties;
            PropertyValue curaggregateproperty;
            int a = 0;
            if (CurDBMetaData.AggregateFieldNames != null){
                for (int i = 0; i < aggregatelist.length; i++){
                    currowproperties = aggregatelist[i];
                    if ((currowproperties[0].Value != null) && (currowproperties[1].Value != null)){
                        short[] iselfield = (short[]) AnyConverter.toArray(currowproperties[1].Value);
                        short[] iselfunction = (short[]) AnyConverter.toArray(currowproperties[0].Value);
                        if ((iselfield.length > 0) && (iselfunction.length > 0)){
                            String[] curaggregatename = new String[2];
                            curaggregatename[0] = CurDBMetaData.NumericFieldNames[iselfield[0]];
                            curaggregatename[1] = this.sFunctionOperators[iselfunction[0]];
                            aggregatevector.add(curaggregatename);
                            a++;
                        }
                    }
                }
            }
            CurDBMetaData.AggregateFieldNames = new String[a][2];
            aggregatevector.toArray(CurDBMetaData.AggregateFieldNames);
         }

        int iduplicate = JavaTools.getDuplicateFieldIndex(CurDBMetaData.AggregateFieldNames);
        if (iduplicate != -1){
            sDuplicateAggregateFunction = JavaTools.replaceSubString(sDuplicateAggregateFunction, CurDBMetaData.AggregateFieldNames[iduplicate][0], "<NUMERICFIELD>");
            int index = JavaTools.FieldInList(sFunctionOperators, CurDBMetaData.AggregateFieldNames[iduplicate][1]);
            String sDisplayFunction = sFunctions[index];
            sDuplicateAggregateFunction = JavaTools.replaceSubString(sDuplicateAggregateFunction, sDisplayFunction, "<FUNCTION>");
            SystemDialog.showMessageBox(CurDBMetaData.xMSF, "WarningBox", VclWindowPeerAttribute.OK, sDuplicateAggregateFunction);
            CurUnoDialog.vetoableChange(new java.beans.PropertyChangeEvent(CurUnoDialog, "Steps", new Integer(1), new Integer(2)));
            return new String[][]{};
        }
        else
            return CurDBMetaData.AggregateFieldNames;
    }
    catch (Exception exception){
        exception.printStackTrace(System.out);
        return null;
    }}


    public void initialize(){
        CurDBMetaData.setNumericFields();
        initializeScrollFields();
        super.initialize(CurDBMetaData.AggregateFieldNames.length+1);
        hideControlRowsfromindex(CurDBMetaData.AggregateFieldNames.length);
        toggleComponent();
    }


    protected void initializeScrollFields(){
        ControlRow curControlRow;
        if (CurDBMetaData.AggregateFieldNames != null){
            for (int i = 0; i < this.getBlockIncrementation(); i++){
                curControlRow = (ControlRow) ControlRowVector.elementAt(i);
                curControlRow.insertFieldNames();
            }
            for (int i = 0; i <= CurDBMetaData.AggregateFieldNames.length; i++)
                registerControlGroupAtIndex(i);
        }
    }


    private void registerControlGroupAtIndex(int _index){
        short[] iselfunctionlist = new short[]{};
        short[] iselfieldslist = new short[]{};
        PropertyValue[] currowproperties = new PropertyValue[2];
        if (_index < CurDBMetaData.AggregateFieldNames.length){
            short iselfieldsindex = (short) JavaTools.FieldInList(CurDBMetaData.NumericFieldNames, CurDBMetaData.AggregateFieldNames[_index][0] );
            iselfieldslist = new short[]{iselfieldsindex};
            short iselfunctionindex = (short) JavaTools.FieldInList(sFunctionOperators, CurDBMetaData.AggregateFieldNames[_index][1] );
            iselfunctionlist = new short[]{iselfunctionindex};
        }
        currowproperties[0] = Properties.createProperty( getFunctionControlName(_index), iselfunctionlist, _index);
        currowproperties[1] = Properties.createProperty( getFieldsControlName(_index), iselfieldslist, _index);     //getTitleName(i)
        super.registerControlGroup(currowproperties, _index);
    }


    private String getFunctionControlName(int _index){
        String namesuffix = "_" + String.valueOf(_index + 1);
        return "lstfunctions" + namesuffix;
    }

    private String getFieldsControlName(int _index){
        String namesuffix = "_" + String.valueOf(_index + 1);
        return "lstFieldnames" + namesuffix;
    }


    private boolean hascompletefunctions(){
        int maxfieldcount = super.getCurFieldCount();
        if (maxfieldcount > 0){
            ControlRow curcontrolrow = (ControlRow) this.ControlRowVector.elementAt(0);
            return curcontrolrow.isComplete();
        }
        else
            return false;
    }



    protected class ControlRow{
        private String NameSuffix;
        private XListBox xFieldListBox;
        private XListBox xFunctionListBox;
        private int index;

        protected ControlRow(int _index, int ypos, int _curHelpID){
        try{
            this.index = _index;
            xFunctionListBox = CurUnoDialog.insertListBox(getFunctionControlName(index), 1, null, new ItemListenerImpl(),
                new String[] { "Dropdown", "Height", "HelpURL", "PositionX", "PositionY", "Step", "StringItemList", "TabIndex", "Width"},
                new Object[] { Boolean.TRUE, new Integer(12), "HID:" + _curHelpID++, new Integer(iCompPosX+4), new Integer(ypos), UIConsts.INVISIBLESTEP, sFunctions, new Short(curtabindex++), new Integer(94)}
            );

            xFieldListBox = CurUnoDialog.insertListBox(getFieldsControlName(index),1, null, new ItemListenerImpl(),
                new String[] {"Dropdown", "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                new Object[] { Boolean.TRUE, new Integer(12), "HID:" + _curHelpID++, new Integer(iCompPosX + 104), new Integer(ypos), UIConsts.INVISIBLESTEP, new Short(curtabindex++), new Integer(90)}
            );
            lastHelpIndex = _curHelpID-1;
        }
        catch( Exception exception ){
            exception.printStackTrace(System.out);
        }}


        private String getSelectedFieldName(){
            return xFieldListBox.getSelectedItem();
        }


        private String getSelectedFunction(){
            return xFunctionListBox.getSelectedItem();
        }


        private void setVisible(boolean bHidden){
            CurUnoDialog.setControlVisible(getFunctionControlName(index), bHidden);
            CurUnoDialog.setControlVisible(getFieldsControlName(index), bHidden);
            CurUnoDialog.repaintDialogStep();
        }


        private void insertFieldNames(){
            Helper.setUnoPropertyValue(UnoDialog.getModel(xFieldListBox), "StringItemList", CurDBMetaData.NumericFieldNames);
        }

        private boolean isComplete(){
            boolean bfieldnameisselected = (Helper.getUnoArrayPropertyValue(UnoDialog.getModel(xFieldListBox), "SelectedItems") != null);
            boolean bfunctionisselected = (Helper.getUnoArrayPropertyValue(UnoDialog.getModel(xFunctionListBox), "SelectedItems") != null);
            return (bfieldnameisselected && bfunctionisselected);
        }


        private void settovoid(){
            CurUnoDialog.deselectListBox(xFieldListBox);
            CurUnoDialog.deselectListBox(xFunctionListBox);
        }


        protected class ItemListenerImpl implements com.sun.star.awt.XItemListener{

            public void itemStateChanged(com.sun.star.awt.ItemEvent EventObject){
            try{
                toggleButtons();
                togglefollowingDialogSteps(true);
            }
            catch (Exception exception){
                exception.printStackTrace(System.out);
            }}

            public void disposing(com.sun.star.lang.EventObject eventObject) {
            }
        }

    }
}
