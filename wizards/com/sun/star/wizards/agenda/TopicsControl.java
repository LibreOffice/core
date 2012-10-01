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
package com.sun.star.wizards.agenda;

import java.util.List;
import java.util.ArrayList;
import com.sun.star.wizards.common.HelpIds;

import com.sun.star.awt.FocusEvent;
import com.sun.star.awt.Key;
import com.sun.star.awt.KeyEvent;
import com.sun.star.awt.KeyModifier;
import com.sun.star.awt.Selection;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XFocusListener;
import com.sun.star.awt.XKeyListener;
import com.sun.star.awt.XTextComponent;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.ui.ControlScroller;
import com.sun.star.wizards.ui.UnoDialog2;
import com.sun.star.wizards.ui.event.EventNames;
import com.sun.star.wizards.ui.event.MethodInvocation;

/**
 * This class implements the UI functionality of the topics scroller control.
 * <br/>
 * During developement, there has been a few changes which were not *fully* done -
 * mainly in converting the topics and time boxes from combobox and time box to normal textboxes,
 * so in the code they might be referenced as combobox or timebox. This should be
 * rather understood as topicstextbox and timetextbox.
 * <br/>
 * <br/>
 * Important behaiviour of this control is that there is always a
 * blank row at the end, in which the user can enter data.<br/>
 * Once the row is not blank (thus, the user entered data...),
 * a new blank row is added.<br/>
 * Once the user removes the last *unempty* row, by deleteing its data, it becomes
 * the *last empty row* and the one after is being automatically removed.<br/>
 * <br/>
 * The contorl shows 5 rows at a time.<br/>
 * If, for example, only 2 rows exist (from which the 2ed one is empty...)
 * then the other three rows, which do not exist in the data model, are disabled.
 * <br/>
 * The following other functionality is implemented:
 * <br/>
 * 0. synchroniting data between controls, data model and live preview.
 * 1. Tab scrolling.<br/>
 * 2. Keyboard scrolling.<br/>
 * 3. Removing rows and adding new rows.<br/>
 * 4. Moving rows up and down. <br/>
 * <br/>
 * This control relays on the ControlScroller control which uses the following
 * Data model:<br/>
 * 1. It uses a vector, whos members are arrays of PropertyValue.<br/>
 * 2. Each array represents a row.<br/>
 * (Note: the Name and Value memebrs of the PropertyValue object are being used...)
 * 3. Each property Value represents a value for a single control with the following rules:<br/>
 * 3. a. the Value of the property is used for as value of the controls (usually text).<br/>
 * 3. b. the Name of the property is used to map values to UI controls in the following manner:<br/>
 * 3. b. 1. only the Name of the first X Rows is regarded, where X is the number of visible rows
 * (in the agenda wizard this would be 5, since 5 topic rows are visible on the dialog).<br/>
 * 3. b. 2. The Names of the first X (or 5...) rows are the names of the UI Controls to
 * hold values. When the control scroller scrolls, it looks at the first 5 rows and uses
 * the names specified there to map the current values to the specified controls.
 * <br/>
 * This data model makes the following limitations on the implementation:
 * When moving rows, only the values should be moved. The Rows objects, which contain
 * also the Names of the controls should not be switched. <br/>
 * also when deleting or inserting rows, attention should be paid that no rows should be removed
 * or inserted. Instead, only the Values should rotate.
 * <br/>
 * <br/>
 * To save the topics in the registry a ConfigSet of objects of type CGTopic is
 * being used.
 * This one is not synchronized "live", since it is unnecessary... instead, it is
 * synchronized on call, before the settings should be saved.
 */
public class TopicsControl extends ControlScroller implements XFocusListener
{

    /**
     * The name prefix of the number (label) controls
     */
    public static final String LABEL = "lblTopicCnt_";
    /**
     * The name prefix of the topic (text) controls
     */
    public static final String TOPIC = "txtTopicTopic_";
    /**
     * The name prefix of the responsible (text) controls
     */
    public static final String RESP = "cbTopicResp_";
    /**
     * The name prefix of the time (text) controls
     */
    public static final String TIME = "txtTopicTime_";
    Object lastFocusControl;
    int lastFocusRow;
    /**
     * the last
     * topic text box.
     * When pressing tab on this one a scroll down *may* be performed.
     */
    private Object firstTopic;
    /**
     * the first time box.
     * When pressing shift-tab on this control, a scroll up *may* be performed.
     */
    private Object lastTime;
    /**
     * is used when constructing to track the tab index
     * of the created control rows.
     */
    private int tabIndex = 520;

    //This maintains for each topic, if something has been written in it.
    private ArrayList<Boolean> rowUsedArray = new ArrayList<Boolean>();

    /**
     * create a new TopicControl. Since this is used specifically for the
     * agenda dialog, I use step 5, and constant location - and need no paramter...
     * @param dialog the parent dialog
     * @param xmsf service factory
     * @param agenda the Agenda configuration data (contains the current topics data).
     */
    public TopicsControl(AgendaWizardDialog dialog, XMultiServiceFactory xmsf, CGAgenda agenda)
    {
        super(dialog, xmsf, 5, 92, 38, 212, 5, 18, AgendaWizardDialogConst.LAST_HID);
        initializeScrollFields(agenda);
        initialize(agenda.cp_Topics.getSize() + 1);
        for(int i=0; i < agenda.cp_Topics.getSize(); ++i) rowUsedArray.add(false);

        // set some focus listeners for TAB scroll down and up...
        try
        {

            // prepare scroll down on tab press...
            Object lastTime = ((ControlRow) ControlGroupVector.get(nblockincrement - 1)).timebox;

            MethodInvocation mi = new MethodInvocation("lastControlKeyPressed", this, KeyEvent.class);
            dialog.getGuiEventListener().add(TIME + (nblockincrement - 1), EventNames.EVENT_KEY_PRESSED, mi);

            addKeyListener(lastTime, (XKeyListener) dialog.getGuiEventListener());

            //prepare scroll up on tab press...
            firstTopic = ((ControlRow) ControlGroupVector.get(0)).textbox;

            mi = new MethodInvocation("firstControlKeyPressed", this, KeyEvent.class);
            dialog.getGuiEventListener().add(TOPIC + 0, EventNames.EVENT_KEY_PRESSED, mi);

            addKeyListener(firstTopic, (XKeyListener) dialog.getGuiEventListener());

        }
        catch (NoSuchMethodException ex)
        {
            ex.printStackTrace();
        }

    }

    /**
     * Is used to add a keylistener to different controls...
     */
    static void addKeyListener(Object control, XKeyListener listener)
    {
        XWindow xlastControl = UnoRuntime.queryInterface(XWindow.class,
                control);
        xlastControl.addKeyListener(listener);
    }

    /**
     * Is used to add a focuslistener to different controls...
     */
    static void addFocusListener(Object control, XFocusListener listener)
    {
        XWindow xlastControl = UnoRuntime.queryInterface(XWindow.class,
                control);
        xlastControl.addFocusListener(listener);
    }

    /**
     * Implementation of the parent class...
     */
    protected void initializeScrollFields()
    {
    }

    /**
     * initializes the data of the control.
     * @param agenda
     */
    protected void initializeScrollFields(CGAgenda agenda)
    {
        // create a row for each topic with the given values....
        for (int i = 0; i < agenda.cp_Topics.getSize(); i++)
        {
            PropertyValue[] row = newRow(i);
            ((CGTopic) agenda.cp_Topics.getElementAt(i)).setDataToRow(row);
            // a parent class method
            registerControlGroup(row, i);
            this.updateDocumentRow(i);
        }
        // inserts a blank row at the end...
        insertRowAtEnd();
    }

    /**
     * Insert a blank (empty) row
     * as last row of the control.
     * The control has always a blank row at the
     * end, which enables the user to enter data...
     */
    protected void insertRowAtEnd()
    {
        int l = scrollfields.size();
        registerControlGroup(newRow(l), l);
        setTotalFieldCount(l + 1);

        // if the new row is visible, it must have been disabled
        // so it should be now enabled...
        if (l - nscrollvalue < nblockincrement)
        {
            ((ControlRow) ControlGroupVector.get(l - nscrollvalue)).setEnabled(true);
        }
    }

    /**
     * The Topics Set in the CGAgenda object is synchronized to
     * the current content of the topics.
     * @param agenda
     */
    void saveTopics(CGAgenda agenda)
    {
        agenda.cp_Topics.clear();
        for (int i = 0; i < scrollfields.size() - 1; i++)
        {
            agenda.cp_Topics.add(i,
                    new CGTopic(scrollfields.get(i)));
        }
    }

    /**
     * overrides the parent class method to also enable the
     * row whenever data is written to it.
     * @param guiRow
     */
    protected void fillupControls(int guiRow)
    {
        super.fillupControls(guiRow);
        ((ControlRow) ControlGroupVector.get(guiRow)).setEnabled(true);
    }

    /**
     * remove the last row
     */
    protected void removeLastRow()
    {
        int l = scrollfields.size();

        // if we should scroll up...
        if ((l - nscrollvalue >= 1) && (l - nscrollvalue <= nblockincrement) && nscrollvalue > 0)
        {
            while ((l - nscrollvalue >= 1) && (l - nscrollvalue <= nblockincrement) && nscrollvalue > 0)
            {
                setScrollValue(nscrollvalue - 1);
            }
        }
        // if we should disable a row...
        else if (nscrollvalue == 0 && l - 1 < nblockincrement)
        {
            ControlRow cr = (ControlRow) ControlGroupVector.get(l - 1);
            cr.setEnabled(false);
        }

        unregisterControlGroup(l - 1);
        setTotalFieldCount(l - 1);
    }

    /**
     * in order to use the "move up", "downPropertyNames.SPACEinsert" and "remove" buttons,
     * we track the last control the gained focus, in order to know which
     * row should be handled.
     * @param fe
     */
    public void focusGained(FocusEvent fe)
    {
        XControl xc = UnoRuntime.queryInterface(XControl.class, fe.Source);
        focusGained(xc);
    }

    /**
     * Sometimes I set the focus programatically to a control
     * (for example when moving a row up or down, the focus should move
     * with it).
     * In such cases, no VCL event is being triggered so it must
     * be called programtically.
     * This is done by this method.
     * @param control
     */
    private void focusGained(XControl control)
    {
        try
        {
            //calculate in which row we are...
            String name = (String) Helper.getUnoPropertyValue(UnoDialog2.getModel(control), PropertyNames.PROPERTY_NAME);
            int i = name.indexOf("_");
            String num = name.substring(i + 1);
            lastFocusRow = Integer.valueOf(num).intValue() + nscrollvalue;
            lastFocusControl = control;
            // enable/disable the buttons...
            enableButtons();
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }

    /**
     * enable or disable the buttons according to the
     * current row we are in.
     */
    private void enableButtons()
    {
        UnoDialog2.setEnabled(getAD().btnInsert, (lastFocusRow < scrollfields.size() - 1 ? Boolean.TRUE : Boolean.FALSE));
        UnoDialog2.setEnabled(getAD().btnRemove, (lastFocusRow < scrollfields.size() - 1 ? Boolean.TRUE : Boolean.FALSE));
        UnoDialog2.setEnabled(getAD().btnUp, (lastFocusRow > 0 ? Boolean.TRUE : Boolean.FALSE));
        UnoDialog2.setEnabled(getAD().btnDown, (lastFocusRow < scrollfields.size() - 1 ? Boolean.TRUE : Boolean.FALSE));
    }

    /**
     * compolsary implementation of FocusListener.
     * @param fe
     */
    public void focusLost(FocusEvent fe)
    {
    }

    /**
     * compolsary implementation of FocusListener.
     * @param o
     */
    public void disposing(EventObject o)
    {
    }

    /**
     * Convenience method. Is used to get a reference of
     * the template controller (live preview in background).
     * @return the parent dialog, casted to AgendaWizardDialog.
     */
    private AgendaWizardDialog getAD()
    {
        return (AgendaWizardDialog) this.CurUnoDialog;
    }

    /**
     * move the current row up
     */
    public void rowUp()
    {
        rowUp(lastFocusRow - nscrollvalue, lastFocusControl);

        //swap the items in rowUsedArray
        boolean temp = rowUsedArray.get(lastFocusRow - nscrollvalue);
        rowUsedArray.set(lastFocusRow - nscrollvalue, rowUsedArray.get(lastFocusRow));
        rowUsedArray.set(lastFocusRow, temp);
        ((AgendaWizardDialogImpl)CurUnoDialog).agendaTemplate.refreshTopicConstants();
    }

    /**
     * move the current row down.
     */
    public void rowDown()
    {
        rowDown(lastFocusRow - nscrollvalue, lastFocusControl);

        //swap the items in rowUsedArray
        boolean temp = rowUsedArray.get(lastFocusRow - nscrollvalue);
        rowUsedArray.set(lastFocusRow - nscrollvalue, rowUsedArray.get(lastFocusRow));
        rowUsedArray.set(lastFocusRow, temp);
        ((AgendaWizardDialogImpl)CurUnoDialog).agendaTemplate.refreshTopicConstants();
    }

    private void lockDoc()
    {
        //((AgendaWizardDialogImpl)CurUnoDialog).agendaTemplate.xTextDocument.lockControllers();
    }

    private void unlockDoc()
    {
        //((AgendaWizardDialogImpl)CurUnoDialog).agendaTemplate.xTextDocument.unlockControllers();
    }

    /**
     * Removes the current row.
     * See general class documentation explanation about the
     * data model used and the limitations which explain the implementation here.
     */
    public void removeRow()
    {
        lockDoc();
        for (int i = lastFocusRow; i < scrollfields.size() - 1; i++)
        {
            PropertyValue[] pv1 = scrollfields.get(i);
            PropertyValue[] pv2 = scrollfields.get(i + 1);
            pv1[1].Value = pv2[1].Value;
            pv1[2].Value = pv2[2].Value;
            pv1[3].Value = pv2[3].Value;
            updateDocumentRow(i);
            if (i - nscrollvalue < nblockincrement)
            {
                fillupControls(i - nscrollvalue);
            }
        }
        removeLastRow();

        ((AgendaWizardDialogImpl)CurUnoDialog).agendaTemplate.refreshTopicConstants();
        rowUsedArray.remove(lastFocusRow);

        // update the live preview background document
        reduceDocumentToTopics();

        // the focus should return to the edit control
        focus(lastFocusControl);

        unlockDoc();
    }

    /**
     * Inserts a row before the current row.
     * See general class documentation explanation about the
     * data model used and the limitations which explain the implementation here.
     */
    public void insertRow()
    {
        lockDoc();
        insertRowAtEnd();
        for (int i = scrollfields.size() - 2; i > lastFocusRow; i--)
        {
            PropertyValue[] pv1 = scrollfields.get(i);
            PropertyValue[] pv2 = scrollfields.get(i - 1);
            pv1[1].Value = pv2[1].Value;
            pv1[2].Value = pv2[2].Value;
            pv1[3].Value = pv2[3].Value;
            updateDocumentRow(i);
            if (i - nscrollvalue < nblockincrement)
            {
                fillupControls(i - nscrollvalue);
            }
        }

        // after rotating all the properties from this row on,
        // we clear the row, so it is practically a new one...
        PropertyValue[] pv1 = scrollfields.get(lastFocusRow);
        pv1[1].Value = PropertyNames.EMPTY_STRING;
        pv1[2].Value = PropertyNames.EMPTY_STRING;
        pv1[3].Value = PropertyNames.EMPTY_STRING;

        // update the preview document.
        updateDocumentRow(lastFocusRow);

        rowUsedArray.add(lastFocusRow,false);

        fillupControls(lastFocusRow - nscrollvalue);

        focus(lastFocusControl);

        unlockDoc();
    }

    /**
     * create a new row with the given index.
     * The index is important because it is used in the
     * Name member of the PropertyValue objects.
     * To know why see general class documentation above (data model explanation).
     * @param i the index of the new row
     * @return
     */
    private PropertyValue[] newRow(int i)
    {
        PropertyValue[] pv = new PropertyValue[4];
        pv[0] = Properties.createProperty(LABEL + i, PropertyNames.EMPTY_STRING + (i + 1) + ".");
        pv[1] = Properties.createProperty(TOPIC + i, PropertyNames.EMPTY_STRING);
        pv[2] = Properties.createProperty(RESP + i, PropertyNames.EMPTY_STRING);
        pv[3] = Properties.createProperty(TIME + i, PropertyNames.EMPTY_STRING);

        ((AgendaWizardDialogImpl)CurUnoDialog).agendaTemplate.refreshTopicConstants();
        rowUsedArray.add(i,false);

        return pv;
    }

    /**
     * Implementation of ControlScroller
     * This is a UI method which inserts a new row to the control.
     * It uses the child-class ControlRow. (see below).
     * @param _index
     * @param npos
     * @see ControlRow
     */
    protected void insertControlGroup(int _index, int npos)
    {
        ControlRow oControlRow = new ControlRow((AgendaWizardDialog) CurUnoDialog, iCompPosX, npos, _index, tabIndex);
        ControlGroupVector.add(oControlRow);
        tabIndex += 4;
    }

    /**
     * Implementation of ControlScroller
     * This is a UI method which makes a row visibele.
     * As far as I know it is never called.
     * @param _index
     * @param _bIsVisible
     * @see ControlRow
     */
    protected void setControlGroupVisible(int _index, boolean _bIsVisible)
    {
        ((ControlRow) ControlGroupVector.get(_index)).setVisible(_bIsVisible);

    }

    /**
     * Checks if a row is empty.
     * This is used when the last row is changed.
     * If it is empty, the next row (which is always blank) is removed.
     * If it is not empty, a next row must exist.
     * @param row the index number of the row to check.
     * @return true if empty. false if not.
     */
    protected boolean isRowEmpty(int row)
    {
        PropertyValue[] data = getTopicData(row);

        // now - is this row empty?
        return data[1].Value.equals(PropertyNames.EMPTY_STRING) &&
                data[2].Value.equals(PropertyNames.EMPTY_STRING) &&
                data[3].Value.equals(PropertyNames.EMPTY_STRING);

    }
    /**
     * is used for data tracking.
     */
    private Object[] oldData;

    /**
     * update the preview document and
     * remove/insert rows if needed.
     * @param guiRow
     * @param column
     */
    synchronized void fieldChanged(int guiRow, int column)
    {
        synchronized(this)
        {

            try
            {
                // First, I update the document
                PropertyValue[] data = getTopicData(guiRow + nscrollvalue);

                if (data == null)
                {
                    return;
                }
                boolean equal = true;

                if (oldData != null)
                {
                    for (int i = 0; i < data.length && equal; i++)
                    {
                        equal = (equal & data[i].Value.equals(oldData[i]));
                    }
                    if (equal)
                    {
                        return;
                    }
                }
                else
                {
                    oldData = new Object[4];
                }
                for (int i = 0; i < data.length; i++)
                {
                    oldData[i] = data[i].Value;
                }
                updateDocumentCell(guiRow + nscrollvalue, column, data);

                if (isRowEmpty(guiRow + nscrollvalue))
                {
                    /* if this is the row before the last one
                     * (the last row is always empty)
                     * delete the last row...
                     */
                    if (guiRow + nscrollvalue == scrollfields.size() - 2)
                    {
                        removeLastRow();

                        /*
                         * now consequentially check the last two rows,
                         * and remove the last one if they are both empty.
                         * (actually I check always the "before last" row,
                         * because the last one is always empty...
                         */
                        while (scrollfields.size() > 1 && isRowEmpty(scrollfields.size() - 2))
                        {
                            removeLastRow();
                        }
                        ControlRow cr = (ControlRow) ControlGroupVector.get(scrollfields.size() - nscrollvalue - 1);

                        // if a remove was performed, set focus to the last row with some data in it...
                        focus(getControl(cr, column));

                        // update the preview document.
                        reduceDocumentToTopics();
                    }

                }
                else
                { // row contains data
                    // is this the last row?
                    if ((guiRow + nscrollvalue + 1) == scrollfields.size())
                    {
                        insertRowAtEnd();
                    }
                }
                if(rowUsedArray.get(guiRow) == false){
                  ((AgendaWizardDialogImpl)CurUnoDialog).agendaTemplate.refreshTopicConstants();
                  rowUsedArray.set(guiRow,true);
                }
            }
            catch (Exception e)
            {
                e.printStackTrace();
            }

        }
    }

    /**
     * return the corresponding row data for the given index.
     * @param topic index of the topic to get.
     * @return a PropertyValue array with the data for the given topic.
     */
    public PropertyValue[] getTopicData(int topic)
    {
        if (topic < scrollfields.size())
        {
            return scrollfields.get(topic);
        }
        else
        {
            return null;
        }
    }

    /**
     * If the user presses tab on the last control, and
     * there *are* more rows in the model, scroll down.
     * @param event
     */
    public void lastControlKeyPressed(KeyEvent event)
    {
        // if tab without shift was pressed...
        if ((event.KeyCode == Key.TAB) && (event.Modifiers == 0))
        // if there is another row...
        {
            if ((nblockincrement + nscrollvalue) < scrollfields.size())
            {
                setScrollValue(nscrollvalue + 1);
                //focus(firstTopic);
                focus(getControl((ControlRow) ControlGroupVector.get(4), 1));

            }
        }
    }

    /**
     * If the user presses shift-tab on the first control, and
     * there *are* more rows in the model, scroll up.
     * @param event
     */
    public void firstControlKeyPressed(KeyEvent event)
    {
        // if tab with shift was pressed...
        if ((event.KeyCode == Key.TAB) && (event.Modifiers == KeyModifier.SHIFT))
        {
            if (nscrollvalue > 0)
            {
                setScrollValue(nscrollvalue - 1);
                focus(lastTime);
            }
        }
    }

    /**
     * sets focus to the given control.
     * @param textControl
     */
    private void focus(Object textControl)
    {
        UnoRuntime.queryInterface(XWindow.class, textControl).setFocus();
        XTextComponent xTextComponent = UnoRuntime.queryInterface(XTextComponent.class, textControl);
        String text = xTextComponent.getText();
        xTextComponent.setSelection(new Selection(0, text.length()));
        XControl xc = UnoRuntime.queryInterface(XControl.class, textControl);
        focusGained(xc);
    }

    /**
     * moves the given row one row down.
     * @param guiRow the gui index of the row to move.
     * @param control the control to gain focus after moving.
     */
    synchronized void rowDown(int guiRow, Object control)
    {
        // only perform if this is not the last row.
        int actuallRow = guiRow + nscrollvalue;
        if (actuallRow + 1 < scrollfields.size())
        {
            // get the current selection
            Selection selection = getSelection(control);

            // the last row should scroll...
            boolean scroll = guiRow == (nblockincrement - 1);
            if (scroll)
            {
                setScrollValue(nscrollvalue + 1);
            }
            int scroll1 = nscrollvalue;

            switchRows(guiRow, guiRow + (scroll ? -1 : 1));

            if (nscrollvalue != scroll1)
            {
                guiRow += (nscrollvalue - scroll1);
            }
            setSelection(guiRow + (scroll ? 0 : 1), control, selection);
        }
    }

    synchronized void rowUp(int guiRow, Object control)
    {
        // only perform if this is not the first row
        int actuallRow = guiRow + nscrollvalue;
        if (actuallRow > 0)
        {
            // get the current selection
            Selection selection = getSelection(control);

            // the last row should scroll...
            boolean scroll = (guiRow == 0);
            if (scroll)
            {
                setScrollValue(nscrollvalue - 1);
            }
            switchRows(guiRow, guiRow + (scroll ? 1 : -1));

            setSelection(guiRow - (scroll ? 0 : 1), control, selection);
        }
    }

    /**
     * moves the cursor up.
     * @param guiRow
     * @param control
     */
    synchronized void cursorUp(int guiRow, Object control)
    {
        // is this the last full row ?
        int actuallRow = guiRow + nscrollvalue;
        //if this is the first row
        if (actuallRow == 0)
        {
            return;
        // the first row should scroll...
        }
        boolean scroll = (guiRow == 0);
        ControlRow upperRow;
        if (scroll)
        {
            setScrollValue(nscrollvalue - 1);
            upperRow = (ControlRow) ControlGroupVector.get(guiRow);
        }
        else
        {
            upperRow = (ControlRow) ControlGroupVector.get(guiRow - 1);
        }
        focus(getControl(upperRow, control));

    }

    /**
     * moves the cursor down
     * @param guiRow
     * @param control
     */
    synchronized void cursorDown(int guiRow, Object control)
    {
        // is this the last full row ?
        int actuallRow = guiRow + nscrollvalue;
        //if this is the last row, exit
        if (actuallRow == scrollfields.size() - 1)
        {
            return;
        // the first row should scroll...
        }
        boolean scroll = (guiRow == nblockincrement - 1);
        ControlRow lowerRow;
        if (scroll)
        {
            setScrollValue(nscrollvalue + 1);
            lowerRow = (ControlRow) ControlGroupVector.get(guiRow);
        }
        // if we scrolled we are done...
        //otherwise...
        else
        {
            lowerRow = (ControlRow) ControlGroupVector.get(guiRow + 1);
        }
        focus(getControl(lowerRow, control));
    }

    /**
     * changes the values of the given rows with eachother
     * @param row1 one can figure out what this parameter is...
     * @param row2 one can figure out what this parameter is...
     */
    private void switchRows(int row1, int row2)
    {
        PropertyValue[] o1 = scrollfields.get(row1 + nscrollvalue);
        PropertyValue[] o2 = scrollfields.get(row2 + nscrollvalue);

        Object temp = null;
        for (int i = 1; i < o1.length; i++)
        {
            temp = o1[i].Value;
            o1[i].Value = o2[i].Value;
            o2[i].Value = temp;
        }

        fillupControls(row1);
        fillupControls(row2);

        updateDocumentRow(row1 + nscrollvalue, o1);
        updateDocumentRow(row2 + nscrollvalue, o2);

        /*
         * if we changed the last row, add another one...
         */
        if ((row1 + nscrollvalue + 1 == scrollfields.size()) ||
                (row2 + nscrollvalue + 1 == scrollfields.size()))
        {
            insertRowAtEnd();
        /*
         * if we did not change the last row but
         * we did change the one before - check if we
         * have two empty rows at the end.
         * If so, delete the last one...
         */
        }
        else if ((row1 + nscrollvalue) + (row2 + nscrollvalue) == (scrollfields.size() * 2 - 5))
        {
            if (isRowEmpty(scrollfields.size() - 2) && isRowEmpty(scrollfields.size() - 1))
            {
                removeLastRow();
                reduceDocumentToTopics();
            }
        }
    }

    /**
     * returns the current Selection of a text field
     * @param control a text field from which the Selection object
     * should be gotten.
     * @return the selection object.
     */
    private Selection getSelection(Object control)
    {
        return UnoRuntime.queryInterface(XTextComponent.class, control).getSelection();
    }

    /**
     * sets a text selection to a given control.
     * This is used when one moves a row up or down.
     * After moving row X to X+/-1, the selection (or cursor position) of the
     * last focused control should be restored.
     * The control's row is the given guiRow.
     * The control's column is detecte4d according to the given event.
     * This method is called as subsequent to different events,
     * thus it is comfortable to use the event here to detect the column,
     * rather than in the different event methods.
     * @param guiRow the row of the control to set the selection to.
     * @param eventSource helps to detect the control's column to set the selection to.
     * @param s the selection object to set.
     */
    private void setSelection(int guiRow, Object eventSource, Selection s)
    {
        ControlRow cr = (ControlRow) ControlGroupVector.get(guiRow);
        Object control = getControl(cr, eventSource);
        UnoRuntime.queryInterface(XWindow.class, control).setFocus();
        UnoRuntime.queryInterface(XTextComponent.class, control).setSelection(s);
    }

    /**
     * returns a control out of the given row, according to a column number.
     * @param cr control row object.
     * @param column the column number.
     * @return the control...
     */
    private Object getControl(ControlRow cr, int column)
    {
        switch (column)
        {
            case 0:
                return cr.label;
            case 1:
                return cr.textbox;
            case 2:
                return cr.combobox;
            case 3:
                return cr.timebox;
            default:
                throw new IllegalArgumentException("No such column");
        }
    }

    /**
     * returns a control out of the given row, which is
     * in the same column as the given control.
     * @param cr control row object
     * @param control a control indicating a column.
     * @return
     */
    private Object getControl(ControlRow cr, Object control)
    {
        int column = getColumn(control);
        return getControl(cr, column);
    }

    /**
     * returns the column number of the given control.
     * @param control
     * @return
     */
    private int getColumn(Object control)
    {
        String name = (String) Helper.getUnoPropertyValue(UnoDialog2.getModel(control), PropertyNames.PROPERTY_NAME);
        if (name.startsWith(TOPIC))
        {
            return 1;
        }
        if (name.startsWith(RESP))
        {
            return 2;
        }
        if (name.startsWith(TIME))
        {
            return 3;
        }
        if (name.startsWith(LABEL))
        {
            return 0;
        }
        return -1;
    }

    /**
     * updates the given row in the preview document.
     * @param row
     */
    private void updateDocumentRow(int row)
    {
        updateDocumentRow(row, scrollfields.get(row));
    }

    /**
     * update the given row in the preview document with the given data.
     * @param row
     * @param data
     */
    private void updateDocumentRow(int row, PropertyValue[] data)
    {
        try
        {
            ((AgendaWizardDialogImpl) CurUnoDialog).agendaTemplate.topics.write(row, data);
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }

    /**
     * updates a single cell in the preview document.
     * Is called when a single value is changed, since we really
     * don't have to update the whole row for one small changhe...
     * @param row the data row to update (topic number).
     * @param column the column to update (a gui column, not a document column).
     * @param data the data of the entire row.
     */
    private void updateDocumentCell(int row, int column, PropertyValue[] data)
    {
        try
        {
            ((AgendaWizardDialogImpl) CurUnoDialog).agendaTemplate.topics.writeCell(row, column, data);
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }

    /**
     * when removeing rows, this method updates
     * the preview document to show the number of rows
     * according to the data model.
     */
    private void reduceDocumentToTopics()
    {
        try
        {
            ((AgendaWizardDialogImpl) CurUnoDialog).agendaTemplate.topics.reduceDocumentTo(scrollfields.size() - 1);
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }

    /**
     * needed to make this data poblic.
     * @return the List containing the topics data.
     */
    public List<PropertyValue[]> getTopicsData()
    {
        return scrollfields;
    }
    /**
     * A static member used for the child-class ControlRow (GUI Constant)
     */
    private static Integer I_12 = 12;
    /**
     * A static member used for the child-class ControlRow (GUI Constant)
     */
    private static Integer I_8 = 8;
    /**
     * A static member used for the child-class ControlRow (GUI Constant)
     */
    private static final String[] LABEL_PROPS = new String[]
    {
        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
    };
    /**
     * A static member used for the child-class ControlRow (GUI Constant)
     */
    private static final String[] TEXT_PROPS = new String[]
    {
        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
    };

    /**
     * A class represting a single GUI row.
     * Note that the instance methods of this class
     * are being called and handle controls of
     * a single row.
     */
    public class ControlRow implements XKeyListener
    {

        /**
         * the number (label) control
         */
        Object label;
        /**
         * the topic (text) control
         */
        Object textbox;
        /**
         * the responsible (text, yes, text) control
         */
        Object combobox;
        /**
         * the time (text, yes, text) control
         */
        Object timebox;
        /**
         * the row offset of this instance (0 = first gui row)
         */
        int offset;

        /**
         * called through an event listener when the
         * topic text is changed by the user.
         * updates the data model and the preview document.
         */
        public void topicTextChanged()
        {
            try
            {
                // update the data model
                fieldInfo(offset, 1);
                // update the preview document
                fieldChanged(offset, 1);
            }
            catch (Exception ex)
            {
                ex.printStackTrace();
            }
        }

        /**
         * called through an event listener when the
         * responsible text is changed by the user.
         * updates the data model and the preview document.
         */
        public void responsibleTextChanged()
        {
            try
            {
                // update the data model
                fieldInfo(offset, 2);
                // update the preview document
                fieldChanged(offset, 2);
            }
            catch (Exception ex)
            {
                ex.printStackTrace();
            }
        }

        /**
         * called through an event listener when the
         * time text is changed by the user.
         * updates the data model and the preview document.
         */
        public void timeTextChanged()
        {
            try
            {
                // update the data model
                fieldInfo(offset, 3);
                // update the preview document
                fieldChanged(offset, 3);
            }
            catch (Exception ex)
            {
                ex.printStackTrace();
            }
        }

        /**
         * constructor. Create the row in the given dialog given cordinates,
         * with the given offset (row number) and tabindex.
         * Note that since I use this specifically for the agenda wizard,
         * the step and all control coordinates inside the
         * row are constant (5).
         * @param dialog the agenda dialog
         * @param x x coordinates
         * @param y y coordinates
         * @param i the gui row index
         * @param tabindex first tab index for this row.
         */
        public ControlRow(AgendaWizardDialog dialog, int x, int y, int i, int tabindex)
        {

            offset = i;

            Integer y_ = new Integer(y);

            label = dialog.insertLabel(LABEL + i,
                    LABEL_PROPS,
                    new Object[]
                    {
                        I_8, PropertyNames.EMPTY_STRING + (i + 1) + ".", new Integer(x + 4), new Integer(y + 2), IStep, new Short((short) tabindex), 10
                    });

            textbox = dialog.insertTextField(TOPIC + i, "topicTextChanged", this,
                    TEXT_PROPS,
                    new Object[]
                    {
                        I_12, HelpIds.getHelpIdString(curHelpIndex + i * 3 + 1), new Integer(x + 15), y_, IStep, new Short((short) (tabindex + 1)), 84
                    });

            combobox = dialog.insertTextField(RESP + i, "responsibleTextChanged", this,
                    TEXT_PROPS,
                    new Object[]
                    {
                        I_12, HelpIds.getHelpIdString(curHelpIndex + i * 3 + 2), new Integer(x + 103), y_, IStep, new Short((short) (tabindex + 2)), 68
                    });

            timebox = dialog.insertTextField(TIME + i, "timeTextChanged", this,
                    TEXT_PROPS,
                    new Object[]
                    {
                        I_12, HelpIds.getHelpIdString(curHelpIndex + i * 3 + 3), new Integer(x + 175), y_, IStep, new Short((short) (tabindex + 3)), 20
                    });

            setEnabled(false);
            addKeyListener(textbox, this);
            addKeyListener(combobox, this);
            addKeyListener(timebox, this);

            addFocusListener(textbox, TopicsControl.this);
            addFocusListener(combobox, TopicsControl.this);
            addFocusListener(timebox, TopicsControl.this);

        }

        /**
         * not implemented.
         * @param visible
         */
        public void setVisible(boolean visible)
        {
            //  Helper.setUnoPropertyValue(UnoDialog2.getModel(button),"Visible", visible ? Boolean.TRUE : Boolean.FASLE);
        }

        /**
         * enables/disables the row.
         * @param enabled true for enable, false for disable.
         */
        public void setEnabled(boolean enabled)
        {
            Boolean b = enabled ? Boolean.TRUE : Boolean.FALSE;
            UnoDialog2.setEnabled(label, b);
            UnoDialog2.setEnabled(textbox, b);
            UnoDialog2.setEnabled(combobox, b);
            UnoDialog2.setEnabled(timebox, b);
        }

        /**
         * Impelementation of XKeyListener.
         * Optionally performs the one of the following:
         * cursor up, or down, row up or down
         */
        public void keyPressed(KeyEvent event)
        {
            if (isMoveDown(event))
            {
                rowDown(offset, event.Source);
            }
            else if (isMoveUp(event))
            {
                rowUp(offset, event.Source);
            }
            else if (isDown(event))
            {
                cursorDown(offset, event.Source);
            }
            else if (isUp(event))
            {
                cursorUp(offset, event.Source);
            }
            enableButtons();
        }

        /**
         * returns the column number of the given control.
         * The given control must belong to this row or
         * an IllegalArgumentException will accure.
         * @param control
         * @return an int columnd number of the given control (0 to 3).
         */
        private int getColumn(Object control)
        {
            if (control == textbox)
            {
                return 1;
            }
            else if (control == combobox)
            {
                return 2;
            }
            else if (control == timebox)
            {
                return 3;
            }
            else if (control == label)
            {
                return 0;
            }
            else
            {
                throw new IllegalArgumentException("Control is not part of this ControlRow");
            }
        }

        private boolean isMoveDown(KeyEvent e)
        {
            return (e.KeyCode == Key.DOWN) && (e.Modifiers == KeyModifier.MOD1);
        }

        private boolean isMoveUp(KeyEvent e)
        {
            return (e.KeyCode == Key.UP) && (e.Modifiers == KeyModifier.MOD1);
        }

        private boolean isDown(KeyEvent e)
        {
            return (e.KeyCode == Key.DOWN) && (e.Modifiers == 0);
        }

        private boolean isUp(KeyEvent e)
        {
            return (e.KeyCode == Key.UP) && (e.Modifiers == 0);
        }

        public void keyReleased(KeyEvent arg0)
        {
        }


        /* (non-Javadoc)
         * @see com.sun.star.lang.XEventListener#disposing(com.sun.star.lang.EventObject)
         */
        public void disposing(EventObject arg0)
        {
        }
    }
}
