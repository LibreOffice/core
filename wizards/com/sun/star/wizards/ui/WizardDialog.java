/*************************************************************************
 *
 *  $RCSfile: WizardDialog.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-05-19 13:06:53 $
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
 *  Contributor(s): Berend Cornelius
 *
 */

package com.sun.star.wizards.ui;

import java.beans.*;

import com.sun.star.wizards.ui.event.EventNames;
import com.sun.star.wizards.ui.event.MethodInvocation;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.awt.XItemEventBroadcaster;
import com.sun.star.awt.*;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.XInterface;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.Resource;
import com.sun.star.container.XIndexContainer;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.beans.*;

public abstract class WizardDialog extends UnoDialog2 implements VetoableChangeListener {

    private static final String NEXT_ACTION_PERFORMED = "gotoNextAvailableStep";
    private static final String BACK_ACTION_PERFORMED = "gotoPreviousAvailableStep";
    private static final String FINISH_ACTION_PERFORMED = "finishWizard";
    private static final String CANCEL_ACTION_PERFORMED = "cancelWizard";
    private static final String HELP_ACTION_PERFORMED = "callHelp";
    public VetoableChangeSupport vetos = new VetoableChangeSupport(this);
    private String[] sRightPaneHeaders;
    private int iButtonWidth = 50;
    private int nNewStep = 1;
    private int nOldStep = 1;
    private int nMaxStep = 1;
    protected XItemListener RoadmapItemListener;
    protected XControl xRoadmapControl;
    XItemEventBroadcaster xRoadmapBroadcaster;
    String[] sRMItemLabels;
    private Object oRoadmap;
    private Object oRoadmapControl;
    private XSingleServiceFactory xSSFRoadmap;
    public XIndexContainer xIndexContRoadmap;
    private Resource oWizardResource;
    public String sMsgEndAutopilot;
    private int hid;


    /** Creates a new instance of WizardDialog
     * the hid is used as following :
     * "HID:(hid)"   - the dialog
     * "HID:(hid+1)  - the help button
     * "HID:(hid+2)" - the back button
     * "HID:(hid+3)" - the next button
     * "HID:(hid+4)" - the create button
     * "HID:(hid+5)" - the cancel button
     */
    public WizardDialog(XMultiServiceFactory xMSF, int hid_) {
        super(xMSF);
        hid = hid_;
        oWizardResource = new Resource(xMSF, "Common", "dbw");
        sMsgEndAutopilot = oWizardResource.getResText(UIConsts.RID_DB_COMMON + 33);

        //new Resource(xMSF,"Common","com");
    }

    public void setMaxStep(int i) {
        nMaxStep = i;
    }

    public int getMaxStep() {
        return nMaxStep;
    }

    public void setOldStep(int i) {
        nOldStep = i;
    }

    public int getOldStep() {
        return nOldStep;
    }

    public void setNewStep(int i) {
        nNewStep = i;
    }

    public int getNewStep() {
        return nNewStep;
    }

    /**
     * @see java.beans.VetoableChangeListener#vetoableChange(java.beans.PropertyChangeEvent)
     */
    public void vetoableChange(java.beans.PropertyChangeEvent arg0) {
        nNewStep = nOldStep;
    }

    public void itemStateChanged(com.sun.star.awt.ItemEvent itemEvent) {
        try {
            nNewStep = itemEvent.ItemId;
            nOldStep = AnyConverter.toInt(Helper.getUnoPropertyValue(xDialogModel, "Step"));
            if (nNewStep != nOldStep)
                switchToStep();
        } catch (com.sun.star.lang.IllegalArgumentException exception) {
            exception.printStackTrace(System.out);
        }
    }

    public void setRoadmapInteractive(boolean _bInteractive) {
//      Helper.setUnoPropertyValue(oRoadmap, "Activated", new Boolean(_bInteractive));
    }

    public void setRoadmapComplete(boolean bComplete) {
        Helper.setUnoPropertyValue(oRoadmap, "Complete", new Boolean(bComplete));
    }

    public boolean isRoadmapComplete() {
        try {
            return AnyConverter.toBoolean(Helper.getUnoPropertyValue(oRoadmap, "Complete"));
        } catch (IllegalArgumentException exception) {
            exception.printStackTrace(System.out);
            return false;
        }
    }

    public void setCurrentRoadmapItemID(short ID) {
        if (oRoadmap != null) {
            int nCurItemID = getCurrentRoadmapItemID();
            if (nCurItemID != ID)
                Helper.setUnoPropertyValue(oRoadmap, "CurrentItemID", new Short(ID));
        }
    }

    public int getCurrentRoadmapItemID() {
        try {
            return AnyConverter.toInt(Helper.getUnoPropertyValue(oRoadmap, "CurrentItemID"));
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace(System.out);
            return -1;
        }
    }

    public void addRoadmap() {
        try {
            oRoadmap = insertControlModel("com.sun.star.awt.UnoControlRoadmapModel", "rdmNavi",
                                        new String[] { "Height", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                        new Object[] { new Integer(184), new Integer(0), new Integer(0), new Integer(0), new Short((short) 0), new Integer(85)});
            XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oRoadmap);
            xPSet.setPropertyValue("Name", "rdmNavi");

            xSSFRoadmap = (XSingleServiceFactory) UnoRuntime.queryInterface(XSingleServiceFactory.class, oRoadmap);
            xIndexContRoadmap = (XIndexContainer) UnoRuntime.queryInterface(XIndexContainer.class, oRoadmap);
            //    XPropertySet xPropRoadmapModel = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oRoadmap);
            //    xPropRoadmapModel.addPropertyChangeListener("CurrentItemID", new WizardDialog.RoadmapItemListener(this.xDialogModel));

            MethodInvocation mi = new MethodInvocation("itemStateChanged", this, com.sun.star.awt.ItemEvent.class);
            guiEventListener.add("rdmNavi", EventNames.EVENT_ITEM_CHANGED, mi);
            xRoadmapControl = this.xDlgContainer.getControl("rdmNavi");
            xRoadmapBroadcaster = (XItemEventBroadcaster) UnoRuntime.queryInterface(XItemEventBroadcaster.class, xRoadmapControl);
            xRoadmapBroadcaster.addItemListener((XItemListener) guiEventListener);

            //     xRoadmapControl = this.xDlgContainer.getControl("rdmNavi");
            //     xRoadmapBroadcaster.addItemListener(new RoadmapItemListener());
            Helper.setUnoPropertyValue(oRoadmap, "Text", oWizardResource.getResText(UIConsts.RID_COMMON + 16));
        } catch (NoSuchMethodException ex) {
            Resource.showCommonResourceError(xMSF);
        } catch (java.lang.Exception jexception) {
            jexception.printStackTrace(System.out);
        }
    }

    public void setRMItemLabels(Resource _oResource, int StartResID) {
        sRMItemLabels = _oResource.getResArray(StartResID, nMaxStep);
    }

    public String[] getRMItemLabels() {
        return sRMItemLabels;
    }

    /*    public void insertRoadmapItems(int StartIndex, int RMCount)
       {
           Object oRoadmapItem;
           boolean bEnabled;
           for (int i = StartIndex; i < (StartIndex + RMCount); i++)
               insertSingleRoadmapItem(i, true, sRMItemLabels[i], i);
       }*/

    public int insertRoadmapItem(int _Index, boolean _bEnabled, int _LabelID, int _CurItemID) {
        return insertRoadmapItem(_Index, _bEnabled, sRMItemLabels[_LabelID], _CurItemID);
    }

    public int insertRoadmapItem(int Index, boolean _bEnabled, String _sLabel, int _CurItemID) {
        try {
            Object oRoadmapItem = xSSFRoadmap.createInstance();
            Helper.setUnoPropertyValue(oRoadmapItem, "Label", _sLabel);
            Helper.setUnoPropertyValue(oRoadmapItem, "Enabled", new Boolean(_bEnabled));
            Helper.setUnoPropertyValue(oRoadmapItem, "ID", new Integer(_CurItemID));
            xIndexContRoadmap.insertByIndex(Index, oRoadmapItem);
            int NextIndex = Index + 1;
            return NextIndex;
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace(System.out);
            return -1;
        }
    }

    public int getRMItemCount() {
        return xIndexContRoadmap.getCount();
    }

    public XInterface getRoadmapItemByID(int _ID) {
        try {
            int CurID;
            XInterface CurRoadmapItem;
            for (int i = 0; i < xIndexContRoadmap.getCount(); i++) {
                CurRoadmapItem = (XInterface) xIndexContRoadmap.getByIndex(i);
                CurID = AnyConverter.toInt(Helper.getUnoPropertyValue(CurRoadmapItem, "ID"));
                if (CurID == _ID)
                    return CurRoadmapItem;
            }
            return null;
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    public boolean switchToStep(int _nOldStep, int _nNewStep) {
        nOldStep = _nOldStep;
        nNewStep = _nNewStep;
        return switchToStep();
    }

    private boolean switchToStep() {
        leaveStep(nOldStep, nNewStep);
        if (nNewStep != nOldStep) {
            changeToStep(nNewStep);
            enterStep(nOldStep, nNewStep);
            return true;
        }
        return false;
    }

    protected void leaveStep(int nOldStep, int nNewStep) {
    }

    protected void enterStep(int nOldStep, int nNewStep) {
    }

    protected void changeToStep(int nNewStep) {
        Helper.setUnoPropertyValue(xDialogModel, "Step", new Integer(nNewStep));
        setCurrentRoadmapItemID((short) (nNewStep));
        enableNextButton(getNextAvailableStep() > 0);
        enableBackButton(nNewStep != 1);
    }

    public void drawNaviBar() {

        try {
            short curtabindex = UIConsts.SOFIRSTWIZARDNAVITABINDEX;
            Integer IButtonWidth = new Integer(iButtonWidth);
            int iButtonHeight = 14;
            Integer IButtonHeight = new Integer(iButtonHeight);
            Integer ICurStep = new Integer(0);
            int iDialogHeight = ((Integer) Helper.getUnoPropertyValue(this.xDialogModel, "Height")).intValue();
            int iDialogWidth = ((Integer) Helper.getUnoPropertyValue(this.xDialogModel, "Width")).intValue();
            int iHelpPosX = 8;
            int iBtnPosY = iDialogHeight - iButtonHeight - 6;
            int iCancelPosX = iDialogWidth - iButtonWidth - 6;
            int iFinishPosX = iCancelPosX - 6 - iButtonWidth;
            int iNextPosX = iFinishPosX - 6 - iButtonWidth;
            int iBackPosX = iNextPosX - 3 - iButtonWidth;

            insertControlModel("com.sun.star.awt.UnoControlFixedLineModel", "lnNaviSep",
                                new String[] { "Height", "Orientation", "PositionX", "PositionY", "Step", "Width" },
                                new Object[] { new Integer(1), new Integer(0), new Integer(0), new Integer(184), ICurStep, new Integer(iDialogWidth)});

            insertControlModel("com.sun.star.awt.UnoControlFixedLineModel", "lnRoadSep",
                                new String[] { "Height", "Orientation", "PositionX", "PositionY", "Step", "Width" },
                                new Object[] { new Integer(iBtnPosY - 6), new Integer(1), new Integer(85), new Integer(0), ICurStep, new Integer(1)});

            String[] propNames = new String[] { "Enabled", "Height", "HelpURL", "Label", "PositionX", "PositionY", "PushButtonType", "Step", "TabIndex", "Width" };

            Helper.setUnoPropertyValue(super.xDialogModel, "HelpURL", "HID:" + hid);

            insertButton("btnWizardHelp", HELP_ACTION_PERFORMED, propNames ,
                                new Object[] { new Boolean(true), IButtonHeight, "HID:" + hid + 1, oWizardResource.getResText(UIConsts.RID_COMMON + 15), new Integer(iHelpPosX), new Integer(iBtnPosY), new Short((short)PushButtonType.HELP_value), ICurStep, new Short(curtabindex++), IButtonWidth });

            insertButton("btnWizardBack", BACK_ACTION_PERFORMED, propNames ,
                                new Object[] { new Boolean(false), IButtonHeight, "HID:" + (hid + 2) ,oWizardResource.getResText(UIConsts.RID_COMMON + 13), new Integer(iBackPosX), new Integer(iBtnPosY), new Short((short)PushButtonType.STANDARD_value), ICurStep, new Short(curtabindex++), IButtonWidth });

            insertButton("btnWizardNext", NEXT_ACTION_PERFORMED, propNames ,
                                new Object[] { new Boolean(true), IButtonHeight, "HID:" + (hid + 3) ,oWizardResource.getResText(UIConsts.RID_COMMON + 14), new Integer(iNextPosX), new Integer(iBtnPosY), new Short((short)PushButtonType.STANDARD_value), ICurStep, new Short(curtabindex++), IButtonWidth });

            insertButton("btnWizardFinish", FINISH_ACTION_PERFORMED, propNames,
                                new Object[] { new Boolean(true), IButtonHeight, "HID:" + (hid + 4), oWizardResource.getResText(UIConsts.RID_COMMON + 12), new Integer(iFinishPosX), new Integer(iBtnPosY), new Short((short)PushButtonType.STANDARD_value), ICurStep, new Short(curtabindex++), IButtonWidth });

            insertButton("btnWizardCancel", CANCEL_ACTION_PERFORMED, propNames,
                                new Object[] { new Boolean(true), IButtonHeight, "HID:" + (hid + 5), oWizardResource.getResText(UIConsts.RID_COMMON + 11), new Integer(iCancelPosX), new Integer(iBtnPosY), new Short((short)PushButtonType.STANDARD_value), ICurStep, new Short(curtabindex++), IButtonWidth });

            //          vetos.addVetoableChangeListener(new ChangesImpl());

        } catch (java.lang.Exception jexception) {
            jexception.printStackTrace(System.out);
        }
    }

    protected void insertRoadMapItems(String[] items, int[] steps, boolean[] enabled) {
        for (int i = 0; i < items.length; i++)
            insertRoadmapItem(i, enabled[i], items[i], steps[i]);
    }

    /** This method also enables and disables the "next" button,
    * if the step currently dis/enabled is the one of the next steps.
    * @param _nStep
    * @param bEnabled
    * @param enableNextButton
    */
    public void setStepEnabled(int _nStep, boolean bEnabled, boolean enableNextButton) {
        setStepEnabled(_nStep, bEnabled);
        if (getNextAvailableStep() > 0)
            enableNextButton(bEnabled);
    }

    public void enableNavigationButtons(boolean _bEnableBack, boolean _bEnableNext, boolean _bEnableFinish) {
        enableBackButton(_bEnableBack);
        enableNextButton(_bEnableNext);
        enableFinishButton(_bEnableFinish);
    }

    public void enableBackButton(boolean enabled) {
        setControlProperty("btnWizardBack", "Enabled", enabled ? Boolean.TRUE : Boolean.FALSE);
    }

    public void enableNextButton(boolean enabled) {
        setControlProperty("btnWizardNext", "Enabled", enabled ? Boolean.TRUE : Boolean.FALSE);
    }

    public void enableFinishButton(boolean enabled) {
        setControlProperty("btnWizardFinish", "Enabled", enabled ? Boolean.TRUE : Boolean.FALSE);
    }

    public void setStepEnabled(int _nStep, boolean bEnabled) {
        XInterface xRoadmapItem = getRoadmapItemByID(_nStep);
        Helper.setUnoPropertyValue(xRoadmapItem, "Enabled", new Boolean(bEnabled));
    }

    public void enablefromStep(int _iStep, boolean _bDoEnable) {
        if (_iStep <= this.nMaxStep) {
            for (int i = _iStep; i <= nMaxStep; i++)
                setStepEnabled(i, _bDoEnable);
            enableFinishButton(_bDoEnable);
            enableNextButton(_bDoEnable);
        }
    }

    public boolean isStepEnabled(int _nStep) {
        try {
            boolean bIsEnabled;
            XInterface xRoadmapItem = getRoadmapItemByID(_nStep);
            if (xRoadmapItem == null)
                // Todo: In this case an exception should be thrown
                return false;
            bIsEnabled = AnyConverter.toBoolean(Helper.getUnoPropertyValue(xRoadmapItem, "Enabled"));
            return bIsEnabled;
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace(System.out);
            return false;
        }
    }

    public synchronized void gotoPreviousAvailableStep() {
        boolean bIsEnabled;
        if (nNewStep > 1) {
            nOldStep = nNewStep;
            nNewStep--;
            while (nNewStep > 0) {
                bIsEnabled = isStepEnabled(nNewStep);
                if (bIsEnabled)
                    break;
                nNewStep--;
            }
            if (nNewStep == 0) // Exception???
                nNewStep = nOldStep;
            switchToStep();
        }
    }

    //TODO discuss with rp
    protected int getNextAvailableStep() {
        if (isRoadmapComplete())
            for (int i = nNewStep + 1; i <= nMaxStep; i++)
                if (isStepEnabled(i))
                    return i;
        return -1;
    }

    public synchronized void gotoNextAvailableStep() {
        nOldStep = nNewStep;
        nNewStep = getNextAvailableStep();
        if (nNewStep > -1)
            switchToStep();
    }

    public abstract void finishWizard();

    public int getMaximalStep() {
        return this.nMaxStep;
    }

    public int getCurrentStep() {
        try {
            return AnyConverter.toInt(Helper.getUnoPropertyValue(this.MSFDialogModel, "Step"));
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace(System.out);
            return -1;
        }
    }

    public void setRightPaneHeaders(Resource _oResource, int StartResID, int _nMaxStep) {
        String[] sRightPaneHeaders = _oResource.getResArray(StartResID, _nMaxStep);
        setRightPaneHeaders(sRightPaneHeaders);
    }

    public void setRightPaneHeaders(String[] _sRightPaneHeaders) {
        this.nMaxStep = _sRightPaneHeaders.length;
        this.sRightPaneHeaders = _sRightPaneHeaders;
        FontDescriptor oFontDesc = new FontDescriptor();
        oFontDesc.Weight = com.sun.star.awt.FontWeight.BOLD;

        for (int i = 0; i < sRightPaneHeaders.length; i++) {
            insertLabel("lblQueryTitle" + String.valueOf(i),
                        new String[] { "FontDescriptor", "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                        new Object[] { oFontDesc, new Integer(8), sRightPaneHeaders[i], new Integer(91), new Integer(8), new Integer(i + 1), new Short((short) 12), new Integer(212)});
        }
    }

    public void cancelWizard() {
        //can be overwritten by extending class
        xDialog.endExecute();
    }

    public void callHelp() {
        //should be overwritten by extending class
    }

}
