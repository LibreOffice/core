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

import java.beans.*;

import com.sun.star.wizards.ui.event.CommonListener;
import com.sun.star.wizards.ui.event.EventNames;
import com.sun.star.wizards.ui.event.MethodInvocation;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.awt.*;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.XInterface;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.Resource;
import com.sun.star.container.XIndexContainer;
import com.sun.star.frame.XTerminateListener;
import com.sun.star.frame.TerminationVetoException;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.beans.*;
import com.sun.star.wizards.common.HelpIds;
import com.sun.star.wizards.common.PropertyNames;

public abstract class WizardDialog extends UnoDialog2 implements VetoableChangeListener, XTerminateListener, XCompletion
{

    private static final String NEXT_ACTION_PERFORMED = "gotoNextAvailableStep";
    private static final String BACK_ACTION_PERFORMED = "gotoPreviousAvailableStep";
    private static final String FINISH_ACTION_PERFORMED = "finishWizard_1";
    private static final String CANCEL_ACTION_PERFORMED = "cancelWizard_1";
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
    private XSingleServiceFactory xSSFRoadmap;
    public XIndexContainer xIndexContRoadmap;
    private Resource oWizardResource;
    public String sMsgEndAutopilot;
    private int hid;
    private boolean bTerminateListenermustberemoved = true;

    /** Creates a new instance of WizardDialog
     * the hid is used as following :
     * "HID:(hid)"   - the dialog
     * "HID:(hid+1)  - the help button
     * "HID:(hid+2)" - the back button
     * "HID:(hid+3)" - the next button
     * "HID:(hid+4)" - the create button
     * "HID:(hid+5)" - the cancel button
     * @param xMSF
     * @param hid_
     */
    public WizardDialog(XMultiServiceFactory xMSF, int hid_)
    {
        super(xMSF);
        hid = hid_;
        oWizardResource = new Resource(xMSF, "Common", "dbw");
        sMsgEndAutopilot = oWizardResource.getResText(UIConsts.RID_DB_COMMON + 33);

    //new Resource(xMSF,"Common","com");
    }

    /**
     *
     * @return
     */
    public Resource getResource()
    {
        return oWizardResource;
    }

    public void activate()
    {
        try
        {
            XTopWindow top = UnoRuntime.queryInterface(XTopWindow.class, xWindow);
            if (top != null)
            {
                top.toFront();
            }
        }
        catch (Exception ex)
        {
            // do nothing;
        }
    }

    public void setMaxStep(int i)
    {
        nMaxStep = i;
    }

    public int getMaxStep()
    {
        return nMaxStep;
    }

    public void setOldStep(int i)
    {
        nOldStep = i;
    }

    public int getOldStep()
    {
        return nOldStep;
    }

    public void setNewStep(int i)
    {
        nNewStep = i;
    }

    public int getNewStep()
    {
        return nNewStep;
    }

    /**
     * @see java.beans.VetoableChangeListener#vetoableChange(java.beans.PropertyChangeEvent)
     */
    public void vetoableChange(java.beans.PropertyChangeEvent arg0)
    {
        nNewStep = nOldStep;
    }

    public void itemStateChanged(com.sun.star.awt.ItemEvent itemEvent)
    {
        try
        {
            nNewStep = itemEvent.ItemId;
            nOldStep = AnyConverter.toInt(Helper.getUnoPropertyValue(xDialogModel, PropertyNames.PROPERTY_STEP));
            if (nNewStep != nOldStep)
            {
                switchToStep();
            }
        }
        catch (com.sun.star.lang.IllegalArgumentException exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public void setRoadmapInteractive(boolean _bInteractive)
    {
        Helper.setUnoPropertyValue(oRoadmap, "Activated", Boolean.valueOf(_bInteractive));
    }

    public void setRoadmapComplete(boolean bComplete)
    {
        Helper.setUnoPropertyValue(oRoadmap, "Complete", Boolean.valueOf(bComplete));
    }

    public boolean isRoadmapComplete()
    {
        try
        {
            return AnyConverter.toBoolean(Helper.getUnoPropertyValue(oRoadmap, "Complete"));
        }
        catch (IllegalArgumentException exception)
        {
            exception.printStackTrace(System.err);
            return false;
        }
    }

    public void setCurrentRoadmapItemID(short ID)
    {
        if (oRoadmap != null)
        {
            int nCurItemID = getCurrentRoadmapItemID();
            if (nCurItemID != ID)
            {
                Helper.setUnoPropertyValue(oRoadmap, "CurrentItemID", new Short(ID));
            }
        }
    }

    public int getCurrentRoadmapItemID()
    {
        try
        {
            return AnyConverter.toInt(Helper.getUnoPropertyValue(oRoadmap, "CurrentItemID"));
        }
        catch (com.sun.star.lang.IllegalArgumentException exception)
        {
            exception.printStackTrace(System.err);
            return -1;
        }
    }

    public void addRoadmap()
    {
        try
        {
            int iDialogHeight = ((Integer) Helper.getUnoPropertyValue(this.xDialogModel, PropertyNames.PROPERTY_HEIGHT)).intValue();

            // the roadmap control has got no real TabIndex ever
            // that is not correct, but changing this would need time, so it is used
            // without TabIndex as before
            oRoadmap = insertControlModel("com.sun.star.awt.UnoControlRoadmapModel", "rdmNavi",
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT,
                        PropertyNames.PROPERTY_POSITION_X,
                        PropertyNames.PROPERTY_POSITION_Y,
                        PropertyNames.PROPERTY_STEP,
                        PropertyNames.PROPERTY_TABINDEX,
                        "Tabstop",
                        PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        new Integer(iDialogHeight - 26),
                        0,
                        0,
                        0,
                        new Short((short)0),
                        Boolean.TRUE,
                        85
                    });
            XPropertySet xPSet = UnoRuntime.queryInterface(XPropertySet.class, oRoadmap);
            xPSet.setPropertyValue(PropertyNames.PROPERTY_NAME, "rdmNavi");

            xSSFRoadmap = UnoRuntime.queryInterface(XSingleServiceFactory.class, oRoadmap);
            xIndexContRoadmap = UnoRuntime.queryInterface(XIndexContainer.class, oRoadmap);
            //    XPropertySet xPropRoadmapModel = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oRoadmap);
            //    xPropRoadmapModel.addPropertyChangeListener("CurrentItemID", new WizardDialog.RoadmapItemListener(this.xDialogModel));

            MethodInvocation mi = new MethodInvocation("itemStateChanged", this, com.sun.star.awt.ItemEvent.class);
            getGuiEventListener().add("rdmNavi", EventNames.EVENT_ITEM_CHANGED, mi);
            xRoadmapControl = this.xDlgContainer.getControl("rdmNavi");
            xRoadmapBroadcaster = UnoRuntime.queryInterface(XItemEventBroadcaster.class, xRoadmapControl);
            xRoadmapBroadcaster.addItemListener((XItemListener) getGuiEventListener());

            //     xRoadmapControl = this.xDlgContainer.getControl("rdmNavi");
            //     xRoadmapBroadcaster.addItemListener(new RoadmapItemListener());
            Helper.setUnoPropertyValue(oRoadmap, "Text", oWizardResource.getResText(UIConsts.RID_COMMON + 16));
        }
        catch (NoSuchMethodException ex)
        {
            Resource.showCommonResourceError(xMSF);
        }
        catch (java.lang.Exception jexception)
        {
            jexception.printStackTrace(System.err);
        }
    }

    public void setRMItemLabels(Resource _oResource, int StartResID)
    {
        sRMItemLabels = _oResource.getResArray(StartResID, nMaxStep);
    }

    public String[] getRMItemLabels()
    {
        return sRMItemLabels;
    }

    /*    public void insertRoadmapItems(int StartIndex, int RMCount)
    {
    Object oRoadmapItem;
    boolean bEnabled;
    for (int i = StartIndex; i < (StartIndex + RMCount); i++)
    insertSingleRoadmapItem(i, true, sRMItemLabels[i], i);
    }*/
    public int insertRoadmapItem(int _Index, boolean _bEnabled, int _LabelID, int _CurItemID)
    {
        return insertRoadmapItem(_Index, _bEnabled, sRMItemLabels[_LabelID], _CurItemID);
    }

    public int insertRoadmapItem(int Index, boolean _bEnabled, String _sLabel, int _CurItemID)
    {
        try
        {
            Object oRoadmapItem = xSSFRoadmap.createInstance();
            Helper.setUnoPropertyValue(oRoadmapItem, PropertyNames.PROPERTY_LABEL, _sLabel);
            Helper.setUnoPropertyValue(oRoadmapItem, PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(_bEnabled));
            Helper.setUnoPropertyValue(oRoadmapItem, "ID", new Integer(_CurItemID));
            xIndexContRoadmap.insertByIndex(Index, oRoadmapItem);
            return Index + 1;
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.err);
            return -1;
        }
    }

    public int getRMItemCount()
    {
        return xIndexContRoadmap.getCount();
    }

    public XInterface getRoadmapItemByID(int _ID)
    {
        try
        {
            int CurID;
            XInterface CurRoadmapItem;
            for (int i = 0; i < xIndexContRoadmap.getCount(); i++)
            {
                CurRoadmapItem = (XInterface) xIndexContRoadmap.getByIndex(i);
                CurID = AnyConverter.toInt(Helper.getUnoPropertyValue(CurRoadmapItem, "ID"));
                if (CurID == _ID)
                {
                    return CurRoadmapItem;
                }
            }
            return null;
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.err);
            return null;
        }
    }

    public boolean switchToStep(int _nOldStep, int _nNewStep)
    {
        nOldStep = _nOldStep;
        nNewStep = _nNewStep;
        return switchToStep();
    }

    private boolean switchToStep()
    {
        leaveStep(nOldStep, nNewStep);
        if (nNewStep != nOldStep)
        {
            if (nNewStep == nMaxStep)
            {
                setControlProperty("btnWizardNext", "DefaultButton", Boolean.FALSE);
                setControlProperty("btnWizardFinish", "DefaultButton", Boolean.TRUE);
            }
            else
            {
                setControlProperty("btnWizardNext", "DefaultButton", Boolean.TRUE);
                setControlProperty("btnWizardFinish", "DefaultButton", Boolean.FALSE);
            }
            changeToStep(nNewStep);
            enterStep(nOldStep, nNewStep);
            return true;
        }
        return false;
    }

    abstract protected void leaveStep(int nOldStep, int nNewStep);

    abstract protected void enterStep(int nOldStep, int nNewStep);

    protected void changeToStep(int nNewStep)
    {
        Helper.setUnoPropertyValue(xDialogModel, PropertyNames.PROPERTY_STEP, new Integer(nNewStep));
        setCurrentRoadmapItemID((short) (nNewStep));
        enableNextButton(getNextAvailableStep() > 0);
        enableBackButton(nNewStep != 1);
    }


    /* (non-Javadoc)
     * @see com.sun.star.wizards.ui.XCompletion#iscompleted(int)
     */
    /**
     *
     * @param _ndialogpage
     * @return
     */
    public boolean iscompleted(int _ndialogpage)
    {
        return false;
    }
    /* (non-Javadoc)
     * @see com.sun.star.wizards.ui.XCompletion#ismodified(int)
     */

    /**
     *
     * @param _ndialogpage
     * @return
     */
    public boolean ismodified(int _ndialogpage)
    {
        return false;
    }
    /* (non-Javadoc)
     * @see com.sun.star.wizards.ui.XCompletion#setcompleted(int, boolean)
     */

    /**
     *
     * @param _ndialogpage
     * @param _biscompleted
     */
    public void setcompleted(int _ndialogpage, boolean _biscompleted)
    {
    }
    /* (non-Javadoc)
     * @see com.sun.star.wizards.ui.XCompletion#setmodified(int, java.lang.Object, java.lang.Object)
     */

    /**
     *
     * @param _ndialogpage
     * @param ooldValue
     * @param onewValue
     */
    public void setmodified(int _ndialogpage, Object ooldValue, Object onewValue)
    {
    }

    public void drawNaviBar()
    {

        try
        {
            short curtabindex = UIConsts.SOFIRSTWIZARDNAVITABINDEX;
            Integer IButtonWidth = new Integer(iButtonWidth);
            int iButtonHeight = 14;
            Integer IButtonHeight = new Integer(iButtonHeight);
            Integer ICurStep = 0;
            int iDialogHeight = ((Integer) Helper.getUnoPropertyValue(this.xDialogModel, PropertyNames.PROPERTY_HEIGHT)).intValue();
            int iDialogWidth = ((Integer) Helper.getUnoPropertyValue(this.xDialogModel, PropertyNames.PROPERTY_WIDTH)).intValue();
            int iHelpPosX = 8;
            int iBtnPosY = iDialogHeight - iButtonHeight - 6;
            int iCancelPosX = iDialogWidth - iButtonWidth - 6;
            int iFinishPosX = iCancelPosX - 6 - iButtonWidth;
            int iNextPosX = iFinishPosX - 6 - iButtonWidth;
            int iBackPosX = iNextPosX - 3 - iButtonWidth;

            insertControlModel("com.sun.star.awt.UnoControlFixedLineModel", "lnNaviSep",
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.ORIENTATION, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        1, 0, 0, new Integer(iDialogHeight - 26), ICurStep, new Integer(iDialogWidth)
                    });

            insertControlModel("com.sun.star.awt.UnoControlFixedLineModel", "lnRoadSep",
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.ORIENTATION, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        new Integer(iBtnPosY - 6), 1, 85, 0, ICurStep, 1
                    });

            String[] propNames = new String[]
            {
                PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "PushButtonType", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
            };

            Helper.setUnoPropertyValue(super.xDialogModel, PropertyNames.PROPERTY_HELPURL, HelpIds.getHelpIdString(hid));
            insertButton("btnWizardHelp", HELP_ACTION_PERFORMED, new String[]
                    {
                        PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "PushButtonType", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        true, IButtonHeight, oWizardResource.getResText(UIConsts.RID_COMMON + 15), new Integer(iHelpPosX), new Integer(iBtnPosY), new Short((short) PushButtonType.HELP_value), ICurStep, new Short(curtabindex++), IButtonWidth
                    });
            insertButton("btnWizardBack", BACK_ACTION_PERFORMED, propNames,
                    new Object[]
                    {
                        false, IButtonHeight, HelpIds.getHelpIdString(hid + 2), oWizardResource.getResText(UIConsts.RID_COMMON + 13), new Integer(iBackPosX), new Integer(iBtnPosY), new Short((short) PushButtonType.STANDARD_value), ICurStep, new Short(curtabindex++), IButtonWidth
                    });

            insertButton("btnWizardNext", NEXT_ACTION_PERFORMED, propNames,
                    new Object[]
                    {
                        true, IButtonHeight, HelpIds.getHelpIdString(hid + 3), oWizardResource.getResText(UIConsts.RID_COMMON + 14), new Integer(iNextPosX), new Integer(iBtnPosY), new Short((short) PushButtonType.STANDARD_value), ICurStep, new Short(curtabindex++), IButtonWidth
                    });

            insertButton("btnWizardFinish", FINISH_ACTION_PERFORMED, propNames,
                    new Object[]
                    {
                        true, IButtonHeight, HelpIds.getHelpIdString(hid + 4), oWizardResource.getResText(UIConsts.RID_COMMON + 12), new Integer(iFinishPosX), new Integer(iBtnPosY), new Short((short) PushButtonType.STANDARD_value), ICurStep, new Short(curtabindex++), IButtonWidth
                    });

            insertButton("btnWizardCancel", CANCEL_ACTION_PERFORMED, propNames,
                    new Object[]
                    {
                        true, IButtonHeight, HelpIds.getHelpIdString(hid + 5), oWizardResource.getResText(UIConsts.RID_COMMON + 11), new Integer(iCancelPosX), new Integer(iBtnPosY), new Short((short) PushButtonType.STANDARD_value), ICurStep, new Short(curtabindex++), IButtonWidth
                    });

            setControlProperty("btnWizardNext", "DefaultButton", Boolean.TRUE);
            // add a window listener, to know
            // if the user used "escape" key to
            // close the dialog.
            MethodInvocation windowHidden = new MethodInvocation("windowHidden", this);
            xWindow.addWindowListener((CommonListener) getGuiEventListener());
            String dialogName = (String) Helper.getUnoPropertyValue(xDialogModel, PropertyNames.PROPERTY_NAME);
            getGuiEventListener().add(dialogName, EVENT_ACTION_PERFORMED, windowHidden);

        }
        catch (java.lang.Exception jexception)
        {
            jexception.printStackTrace(System.err);
        }
    }

    protected void insertRoadMapItems(String[] items, int[] steps, boolean[] enabled)
    {
        for (int i = 0; i < items.length; i++)
        {
            insertRoadmapItem(i, enabled[i], items[i], steps[i]);
        }
    }

    /** This method also enables and disables the "next" button,
     * if the step currently dis/enabled is the one of the next steps.
     * @param _nStep
     * @param bEnabled
     * @param enableNextButton
     */
    public void setStepEnabled(int _nStep, boolean bEnabled, boolean enableNextButton)
    {
        setStepEnabled(_nStep, bEnabled);
        if (getNextAvailableStep() > 0)
        {
            enableNextButton(bEnabled);
        }
    }

    public void enableNavigationButtons(boolean _bEnableBack, boolean _bEnableNext, boolean _bEnableFinish)
    {
        enableBackButton(_bEnableBack);
        enableNextButton(_bEnableNext);
        enableFinishButton(_bEnableFinish);
    }

    public void enableBackButton(boolean enabled)
    {
        setControlProperty("btnWizardBack", PropertyNames.PROPERTY_ENABLED, enabled ? Boolean.TRUE : Boolean.FALSE);
    }

    public void enableNextButton(boolean enabled)
    {
        setControlProperty("btnWizardNext", PropertyNames.PROPERTY_ENABLED, enabled ? Boolean.TRUE : Boolean.FALSE);
    }

    public void enableFinishButton(boolean enabled)
    {
        setControlProperty("btnWizardFinish", PropertyNames.PROPERTY_ENABLED, enabled ? Boolean.TRUE : Boolean.FALSE);
    }

    public void setStepEnabled(int _nStep, boolean bEnabled)
    {
        XInterface xRoadmapItem = getRoadmapItemByID(_nStep);
        if (xRoadmapItem != null)
        {
            Helper.setUnoPropertyValue(xRoadmapItem, PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bEnabled));
        }
    }

    public void enablefromStep(int _iStep, boolean _bDoEnable)
    {
        if (_iStep <= this.nMaxStep)
        {
            for (int i = _iStep; i <= nMaxStep; i++)
            {
                setStepEnabled(i, _bDoEnable);
            }
            enableFinishButton(_bDoEnable);
            if (!_bDoEnable)
            {
                enableNextButton(_iStep > getCurrentStep() + 1);
            }
            else
            {
                enableNextButton(!(getCurrentStep() == nMaxStep));
            }
        }
    }

    public boolean isStepEnabled(int _nStep)
    {
        try
        {
            boolean bIsEnabled;
            XInterface xRoadmapItem = getRoadmapItemByID(_nStep);
            if (xRoadmapItem == null)
            // Todo: In this case an exception should be thrown
            {
                return false;
            }
            bIsEnabled = AnyConverter.toBoolean(Helper.getUnoPropertyValue(xRoadmapItem, PropertyNames.PROPERTY_ENABLED));
            return bIsEnabled;
        }
        catch (com.sun.star.lang.IllegalArgumentException exception)
        {
            exception.printStackTrace(System.err);
            return false;
        }
    }

    public synchronized void gotoPreviousAvailableStep()
    {
        boolean bIsEnabled;
        if (nNewStep > 1)
        {
            nOldStep = nNewStep;
            nNewStep--;
            while (nNewStep > 0)
            {
                bIsEnabled = isStepEnabled(nNewStep);
                if (bIsEnabled)
                {
                    break;
                }
                nNewStep--;
            }
            if (nNewStep == 0) // Exception???
            {
                nNewStep = nOldStep;
            }
            switchToStep();
        }
    }

    //TODO discuss with rp
    protected int getNextAvailableStep()
    {
        if (isRoadmapComplete())
        {
            for (int i = nNewStep + 1; i <= nMaxStep; i++)
            {
                if (isStepEnabled(i))
                {
                    return i;
                }
            }
        }
        return -1;
    }

    public synchronized void gotoNextAvailableStep()
    {
        nOldStep = nNewStep;
        nNewStep = getNextAvailableStep();
        if (nNewStep > -1)
        {
            switchToStep();
        }
    }

    public abstract boolean finishWizard();

    /**
     * This function will call if the finish button is pressed on the UI.
     */
    public void finishWizard_1()
    {
        enableFinishButton(false);
        boolean success = false;
        try
        {
            success = finishWizard();
        }
        finally
        {
            if ( !success )
                enableFinishButton( true );
        }
        if ( success )
            removeTerminateListener();
    }

    public int getMaximalStep()
    {
        return this.nMaxStep;
    }

    public int getCurrentStep()
    {
        try
        {
            return AnyConverter.toInt(Helper.getUnoPropertyValue(this.MSFDialogModel, PropertyNames.PROPERTY_STEP));
        }
        catch (com.sun.star.lang.IllegalArgumentException exception)
        {
            exception.printStackTrace(System.err);
            return -1;
        }
    }

    public void setCurrentStep(int _nNewstep)
    {
        nNewStep = _nNewstep;
        changeToStep(nNewStep);
    }

    public void setRightPaneHeaders(Resource _oResource, int StartResID, int _nMaxStep)
    {
        String[] sRightPaneHeaders = _oResource.getResArray(StartResID, _nMaxStep);
        setRightPaneHeaders(sRightPaneHeaders);
    }

    public void setRightPaneHeaders(String[] _sRightPaneHeaders)
    {
        this.nMaxStep = _sRightPaneHeaders.length;
        this.sRightPaneHeaders = _sRightPaneHeaders;
        FontDescriptor oFontDesc = new FontDescriptor();
        oFontDesc.Weight = com.sun.star.awt.FontWeight.BOLD;

        for (int i = 0; i < sRightPaneHeaders.length; i++)
        {
            insertLabel("lblQueryTitle" + String.valueOf(i),
                    new String[]
                    {
                        PropertyNames.FONT_DESCRIPTOR, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        oFontDesc, 16, sRightPaneHeaders[i], Boolean.TRUE, 91, 8, new Integer(i + 1), new Short((short) 12), 212
                    });
        }
    }

    public void cancelWizard()
    {
        //can be overwritten by extending class
        xDialog.endExecute();
    }

    public void callHelp()
    {
        //should be overwritten by extending class
    }

    public void removeTerminateListener()
    {
        if (bTerminateListenermustberemoved)
        {
            Desktop.getDesktop(xMSF).removeTerminateListener(this);
            bTerminateListenermustberemoved = false;
        }
    }

    /**
     * called by the cancel button and
     * by the window hidden event.
     * if this method was not called before,
     * perform a cancel.
     */
    public void cancelWizard_1()
    {
        cancelWizard();
        removeTerminateListener();
    }

    public void windowHidden()
    {
        cancelWizard_1();
    }

    public void notifyTermination(EventObject arg0)
    {
        cancelWizard_1();
    }

    public void queryTermination(EventObject arg0)
            throws TerminationVetoException
    {
        activate();
        throw new TerminationVetoException();
    }

    public void disposing(EventObject arg0)
    {
        cancelWizard_1();
    }
}
