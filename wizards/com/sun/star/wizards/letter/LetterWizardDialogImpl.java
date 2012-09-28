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
package com.sun.star.wizards.letter;

import java.util.List;
import java.util.ArrayList;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.XTextComponent;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.document.MacroExecMode;
import com.sun.star.document.XDocumentProperties;
import com.sun.star.document.XDocumentPropertiesSupplier;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.task.XInteractionHandler;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextFrame;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.RuntimeException;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.CloseVetoException;
import com.sun.star.util.XCloseable;
import com.sun.star.wizards.common.Configuration;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.HelpIds;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.NoValidPathException;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.common.SystemDialog;
import com.sun.star.wizards.document.OfficeDocument;
import com.sun.star.wizards.text.TextFieldHandler;
import com.sun.star.wizards.text.TextFrameHandler;
import com.sun.star.wizards.text.ViewHandler;
import com.sun.star.wizards.ui.PathSelection;
import com.sun.star.wizards.ui.XPathSelectionListener;
import com.sun.star.wizards.ui.event.DataAware;
import com.sun.star.wizards.ui.event.RadioDataAware;
import com.sun.star.wizards.ui.event.UnoDataAware;

import com.sun.star.util.XSearchable;
import com.sun.star.util.XSearchDescriptor;
import com.sun.star.container.XIndexAccess;
import com.sun.star.text.*;
import com.sun.star.wizards.common.TextElement;
import com.sun.star.wizards.common.PlaceholderTextElement;

import com.sun.star.beans.XPropertySet;

public class LetterWizardDialogImpl extends LetterWizardDialog
{

    protected void enterStep(int OldStep, int NewStep)
    {
    }

    protected void leaveStep(int OldStep, int NewStep)
    {
    }
    static LetterDocument myLetterDoc;
    static boolean running;
    XMultiServiceFactory xmsf;
    XTextDocument xTextDocument;
    PathSelection myPathSelection;
    CGLetterWizard myConfig;
    ArrayList<DataAware> mainDA = new ArrayList<DataAware>();
    ArrayList<DataAware> letterDA = new ArrayList<DataAware>();
    ArrayList<DataAware> businessDA = new ArrayList<DataAware>();
    String[][] BusinessFiles;
    String[][] OfficialFiles;
    String[][] PrivateFiles;
    String sTemplatePath;
    String sUserTemplatePath;
    String sBitmapPath;
    String sLetterPath;
    String sLetterLangPackPath;
    String sWorkPath;
    String sPath;
    boolean bEditTemplate;
    boolean bSaveSuccess = false;
    private boolean filenameChanged = false;
    LetterDocument.BusinessPaperObject BusCompanyLogo = null;
    LetterDocument.BusinessPaperObject BusCompanyAddress = null;
    LetterDocument.BusinessPaperObject BusCompanyAddressReceiver = null;
    LetterDocument.BusinessPaperObject BusFooter = null;
    final static int RM_TYPESTYLE = 1;
    final static int RM_BUSINESSPAPER = 2;
    final static int RM_ELEMENTS = 3;
    final static int RM_SENDERRECEIVER = 4;
    final static int RM_FOOTER = 5;
    final static int RM_FINALSETTINGS = 6;

    List<XTextRange> constRangeList = new ArrayList<XTextRange>();
    XTextRange trSubjectconst;
    TextElement teSubjectconst;

    public LetterWizardDialogImpl(XMultiServiceFactory xmsf)
    {
        super(xmsf);
        this.xmsf = xmsf;
    }

    public static void main(String args[])
    {
        //only being called when starting wizard remotely
        String ConnectStr = "uno:socket,host=127.0.0.1,port=8100;urp,negotiate=0,forcesynchronous=1;StarOffice.ServiceManager";
        XMultiServiceFactory xLocMSF = null;
        try
        {
            xLocMSF = Desktop.connect(ConnectStr);
        }
        catch (RuntimeException e)
        {
            e.printStackTrace();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        catch (java.lang.Exception e)
        {
            e.printStackTrace();
        }
        LetterWizardDialogImpl lw = new LetterWizardDialogImpl(xLocMSF);
        lw.startWizard(xLocMSF, null);

    }

    public void startWizard(XMultiServiceFactory xMSF, Object[] CurPropertyValue)
    {

        running = true;
        try
        {
            //Number of steps on WizardDialog:
            setMaxStep(6);

            //instatiate The Document Frame for the Preview
            myLetterDoc = new LetterDocument(xMSF, this);

            //create the dialog:
            drawNaviBar();
            buildStep1();
            buildStep2();
            buildStep3();
            buildStep4();
            buildStep5();
            buildStep6();

            initializePaths();
            initializeSalutation();
            initializeGreeting();


            //special Control for setting the save Path:
            insertPathSelectionControl();

            //load the last used settings from the registry and apply listeners to the controls:
            initConfiguration();

            initializeTemplates(xMSF);

            if (myConfig.cp_BusinessLetter.cp_Greeting.equals(PropertyNames.EMPTY_STRING))
            {
                myConfig.cp_BusinessLetter.cp_Greeting = resources.GreetingLabels[0];
            }
            if (myConfig.cp_BusinessLetter.cp_Salutation.equals(PropertyNames.EMPTY_STRING))
            {
                myConfig.cp_BusinessLetter.cp_Salutation = resources.SalutationLabels[0];
            }
            if (myConfig.cp_PrivateOfficialLetter.cp_Greeting.equals(PropertyNames.EMPTY_STRING))
            {
                myConfig.cp_PrivateOfficialLetter.cp_Greeting = resources.GreetingLabels[1];
            }
            if (myConfig.cp_PrivateOfficialLetter.cp_Salutation.equals(PropertyNames.EMPTY_STRING))
            {
                myConfig.cp_PrivateOfficialLetter.cp_Salutation = resources.SalutationLabels[1];
            }
            if (myConfig.cp_PrivateLetter.cp_Greeting.equals(PropertyNames.EMPTY_STRING))
            {
                myConfig.cp_PrivateLetter.cp_Greeting = resources.GreetingLabels[2];
            }
            if (myConfig.cp_PrivateLetter.cp_Salutation.equals(PropertyNames.EMPTY_STRING))
            {
                myConfig.cp_PrivateLetter.cp_Salutation = resources.SalutationLabels[2];
            }

            //update the dialog UI according to the loaded Configuration
            updateUI();

            if (myPathSelection.xSaveTextBox.getText().equalsIgnoreCase(PropertyNames.EMPTY_STRING))
            {
                myPathSelection.initializePath();
            }

            XWindow xContainerWindow = myLetterDoc.xFrame.getContainerWindow();
            XWindowPeer xWindowPeer = UnoRuntime.queryInterface(XWindowPeer.class, xContainerWindow);
            createWindowPeer(xWindowPeer);

            //add the Roadmap to the dialog:
            insertRoadmap();

            //load the last used document and apply last used settings:
            setConfiguration();

            //If the configuration does not define Greeting/Salutation yet choose a default
            setDefaultForGreetingAndSalutation();

            //disable funtionality that is not supported by the template:
            initializeElements();

            drawConstants();

            //disable the document, so that the user cannot change anything:
            myLetterDoc.xFrame.getComponentWindow().setEnable(false);

            executeDialog(myLetterDoc.xFrame);
            removeTerminateListener();
            closeDocument();
            running = false;

        }
        catch (Exception exception)
        {
            removeTerminateListener();
            exception.printStackTrace(System.err);
            running = false;
        }
    }

    public void cancelWizard()
    {
        xDialog.endExecute();
        running = false;
    }

    public boolean finishWizard()
    {
        switchToStep(getCurrentStep(), getMaxStep());
        try
        {
            //myLetterDoc.xTextDocument.lockControllers();
            FileAccess fileAccess = new FileAccess(xMSF);
            sPath = myPathSelection.getSelectedPath();
            if (sPath.equals(PropertyNames.EMPTY_STRING))
            {
                myPathSelection.triggerPathPicker();
                sPath = myPathSelection.getSelectedPath();
            }
            sPath = fileAccess.getURL(sPath);

            //first, if the filename was not changed, thus
            //it is coming from a saved session, check if the
            // file exists and warn the user.
            if (!filenameChanged)
            {
                if (fileAccess.exists(sPath, true))
                {

                    int answer = SystemDialog.showMessageBox(xMSF, xControl.getPeer(), "MessBox", VclWindowPeerAttribute.YES_NO + VclWindowPeerAttribute.DEF_NO, resources.resOverwriteWarning);
                    if (answer == 3) // user said: no, do not overwrite....
                    {
                        return false;
                    }
                }
            }
            myLetterDoc.setWizardTemplateDocInfo(resources.resLetterWizardDialog_title, resources.resTemplateDescription);
            myLetterDoc.killEmptyUserFields();
            myLetterDoc.keepLogoFrame = (chkUseLogo.getState() != 0);
            if ((chkBusinessPaper.getState() != 0) && (chkPaperCompanyLogo.getState() != 0))
            {
                myLetterDoc.keepLogoFrame = false;
            }
            myLetterDoc.keepBendMarksFrame = (chkUseBendMarks.getState() != 0);
            myLetterDoc.keepLetterSignsFrame = (chkUseSigns.getState() != 0);
            myLetterDoc.keepSenderAddressRepeatedFrame = (chkUseAddressReceiver.getState() != 0);

            if (optBusinessLetter.getState())
            {
                if ((chkBusinessPaper.getState() != 0) && (chkCompanyReceiver.getState() != 0))
                {
                    myLetterDoc.keepSenderAddressRepeatedFrame = false;
                }
                if ((chkBusinessPaper.getState() != 0) && (chkPaperCompanyAddress.getState() != 0))
                {
                    myLetterDoc.keepAddressFrame = false;
                }
            }

            myLetterDoc.killEmptyFrames();


            bSaveSuccess = OfficeDocument.store(xMSF, xTextDocument, sPath, "writer8_template", false);

            if (bSaveSuccess)
            {
                saveConfiguration();
                XInteractionHandler xIH = UnoRuntime.queryInterface(XInteractionHandler.class, xMSF.createInstance("com.sun.star.comp.uui.UUIInteractionHandler"));
                PropertyValue loadValues[] = new PropertyValue[4];
                loadValues[0] = new PropertyValue();
                loadValues[0].Name = "AsTemplate";
                loadValues[1] = new PropertyValue();
                loadValues[1].Name = "MacroExecutionMode";
                loadValues[1].Value = new Short(MacroExecMode.ALWAYS_EXECUTE);
                loadValues[2] = new PropertyValue();
                loadValues[2].Name = "UpdateDocMode";
                loadValues[2].Value = new Short(com.sun.star.document.UpdateDocMode.FULL_UPDATE);
                loadValues[3] = new PropertyValue();
                loadValues[3].Name = "InteractionHandler";
                loadValues[3].Value = xIH;

                if (bEditTemplate)
                {
                    loadValues[0].Value = Boolean.FALSE;
                }
                else
                {
                    loadValues[0].Value = Boolean.TRUE;
                }
                Object oDoc = OfficeDocument.load(Desktop.getDesktop(xMSF), sPath, "_default", loadValues);
                XTextDocument xTextDocument = (com.sun.star.text.XTextDocument) oDoc;
                XMultiServiceFactory xDocMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);
                ViewHandler myViewHandler = new ViewHandler(xDocMSF, xTextDocument);
                myViewHandler.setViewSetting("ZoomType", new Short(com.sun.star.view.DocumentZoomType.OPTIMAL));
            }
            else
            {
                //TODO: Error Handling
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        finally
        {
            xDialog.endExecute();
            running = false;
        }
        return true;
    }

    public void closeDocument()
    {
        try
        {
            //xComponent.dispose();
            XCloseable xCloseable = UnoRuntime.queryInterface(XCloseable.class, myLetterDoc.xFrame);
            xCloseable.close(false);
        }
        catch (CloseVetoException e)
        {
            e.printStackTrace();
        }
    }

    public void drawConstants()
    {
            constRangeList = searchFillInItems(1);

            XTextRange item = null;

            for (int i = 0; i < constRangeList.size(); i++)
            {
                item = constRangeList.get(i);
                String text = item.getString().trim().toLowerCase();
                if (text.equals(resources.resSubjectconstPlaceHolder))
                {
                    teSubjectconst = new PlaceholderTextElement(item, resources.resSubjectconstPlaceHolder_value, "hint", myLetterDoc.xMSF);
                    trSubjectconst = item;
                    constRangeList.remove(i--);
                    writeTitle(teSubjectconst,trSubjectconst,resources.resSubjectconstPlaceHolder_value);
                }
            }
    }

    public void clearConstants()
    {
        constRangeList.clear();
        trSubjectconst = null;
        teSubjectconst = null;
    }

    private void writeTitle(TextElement te, XTextRange tr, String text)
    {
        te.setText(text == null ? PropertyNames.EMPTY_STRING : text);
        te.write(tr);
    }

    public List<XTextRange> searchFillInItems(int type)
    {
      try
      {
            XSearchable xSearchable = UnoRuntime.queryInterface(XSearchable.class, xTextDocument);
            XSearchDescriptor sd = xSearchable.createSearchDescriptor();

            if(type == 0)
            {
              sd.setSearchString("<[^>]+>");
            }
            else if(type == 1)
            {
              sd.setSearchString("#[^#]+#");
            }
            sd.setPropertyValue("SearchRegularExpression", Boolean.TRUE);
            sd.setPropertyValue("SearchWords", Boolean.TRUE);

            XIndexAccess ia = xSearchable.findAll(sd);

            List<XTextRange> l = new ArrayList<XTextRange>(ia.getCount());
            for (int i = 0; i < ia.getCount(); i++)
            {
                try
                {
                    l.add(UnoRuntime.queryInterface(XTextRange.class, ia.getByIndex(i)));
                }
                catch (Exception ex)
                {
                    System.err.println("Nonfatal Error in finding fillins.");
                }
            }
            return l;
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
            throw new IllegalArgumentException("Fatal Error: Loading template failed: searching fillins failed");
        }
    }

    public void optBusinessLetterItemChanged()
    {
        DataAware.setDataObject(letterDA, myConfig.cp_BusinessLetter, true);
        setControlProperty("lblBusinessStyle", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("lstBusinessStyle", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("chkBusinessPaper", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("lblPrivOfficialStyle", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("lstPrivOfficialStyle", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("lblPrivateStyle", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("lstPrivateStyle", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        lstBusinessStyleItemChanged();
        enableSenderReceiver();
        setPossibleFooter(true);
        if (myPathSelection.xSaveTextBox.getText().equalsIgnoreCase(PropertyNames.EMPTY_STRING))
        {
            myPathSelection.initializePath();
        }
    }

    public void optPrivOfficialLetterItemChanged()
    {
        DataAware.setDataObject(letterDA, myConfig.cp_PrivateOfficialLetter, true);
        setControlProperty("lblBusinessStyle", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("lstBusinessStyle", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("chkBusinessPaper", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("lblPrivOfficialStyle", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("lstPrivOfficialStyle", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("lblPrivateStyle", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("lstPrivateStyle", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        lstPrivOfficialStyleItemChanged();
        disableBusinessPaper();
        enableSenderReceiver();
        setPossibleFooter(true);
        if (myPathSelection.xSaveTextBox.getText().equalsIgnoreCase(PropertyNames.EMPTY_STRING))
        {
            myPathSelection.initializePath();
        }
    }

    public void optPrivateLetterItemChanged()
    {
        DataAware.setDataObject(letterDA, myConfig.cp_PrivateLetter, true);
        setControlProperty("lblBusinessStyle", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("lstBusinessStyle", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("chkBusinessPaper", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("lblPrivOfficialStyle", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("lstPrivOfficialStyle", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("lblPrivateStyle", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("lstPrivateStyle", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        lstPrivateStyleItemChanged();
        disableBusinessPaper();
        disableSenderReceiver();
        setPossibleFooter(false);
        if (myPathSelection.xSaveTextBox.getText().equalsIgnoreCase(PropertyNames.EMPTY_STRING))
        {
            myPathSelection.initializePath();
        }
    }

    public void optSenderPlaceholderItemChanged()
    {
        setControlProperty("lblSenderName", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("lblSenderStreet", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("lblPostCodeCity", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("txtSenderName", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("txtSenderStreet", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("txtSenderPostCode", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("txtSenderState", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("txtSenderCity", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        myLetterDoc.fillSenderWithUserData();
    }

    public void optSenderDefineItemChanged()
    {
        setControlProperty("lblSenderName", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("lblSenderStreet", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("lblPostCodeCity", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("txtSenderName", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("txtSenderStreet", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("txtSenderPostCode", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("txtSenderState", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("txtSenderCity", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        txtSenderNameTextChanged();
        txtSenderStreetTextChanged();
        txtSenderPostCodeTextChanged();
        txtSenderStateTextChanged();
        txtSenderCityTextChanged();
    }

    public void optCreateLetterItemChanged()
    {
        bEditTemplate = false;
    }

    public void optMakeChangesItemChanged()
    {
        bEditTemplate = true;
    }

    public void optReceiverPlaceholderItemChanged()
    {
        OfficeDocument.attachEventCall(xTextDocument, "OnNew", "StarBasic", "macro:///Template.Correspondence.Placeholder()");
    }

    public void optReceiverDatabaseItemChanged()
    {
        OfficeDocument.attachEventCall(xTextDocument, "OnNew", "StarBasic", "macro:///Template.Correspondence.Database()");
    }

    public void lstBusinessStyleItemChanged()
    {
        xTextDocument = myLetterDoc.loadAsPreview(BusinessFiles[1][lstBusinessStyle.getSelectedItemPos()], false);
        myLetterDoc.xTextDocument.lockControllers();
        initializeElements();
        chkBusinessPaperItemChanged();
        setElements(false);
        clearConstants();
        drawConstants();
        myLetterDoc.xTextDocument.unlockControllers();
        activate();
    }

    public void lstPrivOfficialStyleItemChanged()
    {
        xTextDocument = myLetterDoc.loadAsPreview(OfficialFiles[1][lstPrivOfficialStyle.getSelectedItemPos()], false);
        myLetterDoc.xTextDocument.lockControllers();
        initializeElements();
        setPossibleSenderData(true);
        setElements(false);
        clearConstants();
        drawConstants();
        myLetterDoc.xTextDocument.unlockControllers();
        activate();
    }

    public void lstPrivateStyleItemChanged()
    {
        xTextDocument = myLetterDoc.loadAsPreview(PrivateFiles[1][lstPrivateStyle.getSelectedItemPos()], false);
        myLetterDoc.xTextDocument.lockControllers();
        initializeElements();
        setElements(true);
        clearConstants();
        drawConstants();
        myLetterDoc.xTextDocument.unlockControllers();
        activate();
    }

    public void numLogoHeightTextChanged()
    {
        BusCompanyLogo.iHeight = (int) (numLogoHeight.getValue() * 1000);
        BusCompanyLogo.setFramePosition();
    }

    public void numLogoWidthTextChanged()
    {
        BusCompanyLogo.iWidth = (int) (numLogoWidth.getValue() * 1000);
        BusCompanyLogo.setFramePosition();
    }

    public void numLogoXTextChanged()
    {
        BusCompanyLogo.iXPos = (int) (numLogoX.getValue() * 1000);
        BusCompanyLogo.setFramePosition();
    }

    public void numLogoYTextChanged()
    {
        BusCompanyLogo.iYPos = (int) (numLogoY.getValue() * 1000);
        BusCompanyLogo.setFramePosition();
    }

    public void numAddressWidthTextChanged()
    {
        BusCompanyAddress.iWidth = (int) (numAddressWidth.getValue() * 1000);
        BusCompanyAddress.setFramePosition();
    }

    public void numAddressXTextChanged()
    {
        BusCompanyAddress.iXPos = (int) (numAddressX.getValue() * 1000);
        BusCompanyAddress.setFramePosition();
    }

    public void numAddressYTextChanged()
    {
        BusCompanyAddress.iYPos = (int) (numAddressY.getValue() * 1000);
        BusCompanyAddress.setFramePosition();
    }

    public void numAddressHeightTextChanged()
    {
        BusCompanyAddress.iHeight = (int) (numAddressHeight.getValue() * 1000);
        BusCompanyAddress.setFramePosition();
    }

    public void numFooterHeightTextChanged()
    {
        BusFooter.iHeight = (int) (numFooterHeight.getValue() * 1000);
        BusFooter.iYPos = myLetterDoc.DocSize.Height - BusFooter.iHeight;
        BusFooter.setFramePosition();
    }

    public void chkPaperCompanyLogoItemChanged()
    {
        if (chkPaperCompanyLogo.getState() != 0)
        {
            //minimal value is required, otherwise the frame creation fails
            if (numLogoWidth.getValue() == 0)
            {
                numLogoWidth.setValue(0.1);
            }
            if (numLogoHeight.getValue() == 0)
            {
                numLogoHeight.setValue(0.1);
            }
            BusCompanyLogo = myLetterDoc.new BusinessPaperObject("Company Logo", (int) (numLogoWidth.getValue() * 1000), (int) (numLogoHeight.getValue() * 1000), (int) (numLogoX.getValue() * 1000), (int) (numLogoY.getValue() * 1000));
            setControlProperty("numLogoHeight", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
            setControlProperty("lblCompanyLogoHeight", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
            setControlProperty("numLogoWidth", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
            setControlProperty("lblCompanyLogoWidth", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
            setControlProperty("numLogoX", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
            setControlProperty("lblCompanyLogoX", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
            setControlProperty("numLogoY", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
            setControlProperty("lblCompanyLogoY", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
            setPossibleLogo(false);
        }
        else
        {
            if (BusCompanyLogo != null)
            {
                BusCompanyLogo.removeFrame();
            }
            setControlProperty("numLogoHeight", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
            setControlProperty("lblCompanyLogoHeight", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
            setControlProperty("numLogoWidth", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
            setControlProperty("lblCompanyLogoWidth", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
            setControlProperty("numLogoX", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
            setControlProperty("lblCompanyLogoX", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
            setControlProperty("numLogoY", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
            setControlProperty("lblCompanyLogoY", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
            setPossibleLogo(true);
        }
    }

    public void chkPaperCompanyAddressItemChanged()
    {
        if (chkPaperCompanyAddress.getState() != 0)
        {
//          minimal value is required, otherwise the frame creation fails
            if (numAddressWidth.getValue() == 0)
            {
                numAddressWidth.setValue(0.1);
            }
            if (numAddressHeight.getValue() == 0)
            {
                numAddressHeight.setValue(0.1);
            }
            BusCompanyAddress = myLetterDoc.new BusinessPaperObject("Company Address", (int) (numAddressWidth.getValue() * 1000), (int) (numAddressHeight.getValue() * 1000), (int) (numAddressX.getValue() * 1000), (int) (numAddressY.getValue() * 1000));
            setControlProperty("numAddressHeight", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
            setControlProperty("lblCompanyAddressHeight", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
            setControlProperty("numAddressWidth", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
            setControlProperty("lblCompanyAddressWidth", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
            setControlProperty("numAddressX", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
            setControlProperty("lblCompanyAddressX", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
            setControlProperty("numAddressY", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
            setControlProperty("lblCompanyAddressY", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
            if (myLetterDoc.hasElement("Sender Address"))
            {
                myLetterDoc.switchElement("Sender Address", (false));
            }
            if (chkCompanyReceiver.getState() != 0)
            {
                setPossibleSenderData(false);
            }
        }
        else
        {
            if (BusCompanyAddress != null)
            {
                BusCompanyAddress.removeFrame();
            }
            setControlProperty("numAddressHeight", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
            setControlProperty("lblCompanyAddressHeight", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
            setControlProperty("numAddressWidth", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
            setControlProperty("lblCompanyAddressWidth", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
            setControlProperty("numAddressX", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
            setControlProperty("lblCompanyAddressX", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
            setControlProperty("numAddressY", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
            setControlProperty("lblCompanyAddressY", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
            if (myLetterDoc.hasElement("Sender Address"))
            {
                myLetterDoc.switchElement("Sender Address", (true));
            }
            setPossibleSenderData(true);
            if (optSenderDefine.getState())
            {
                optSenderDefineItemChanged();
            }
            if (optSenderPlaceholder.getState())
            {
                optSenderPlaceholderItemChanged();
            }
        }
    }

    public void chkCompanyReceiverItemChanged()
    {
        XTextFrame xReceiverFrame = null;

        if (chkCompanyReceiver.getState() != 0)
        {
            try
            {
                xReceiverFrame = TextFrameHandler.getFrameByName("Receiver Address", xTextDocument);
                Integer FrameWidth = (Integer) Helper.getUnoPropertyValue(xReceiverFrame, PropertyNames.PROPERTY_WIDTH);
                int iFrameWidth = FrameWidth.intValue();
                Integer FrameX = (Integer) Helper.getUnoPropertyValue(xReceiverFrame, "HoriOrientPosition");
                int iFrameX = FrameX.intValue();
                Integer FrameY = (Integer) Helper.getUnoPropertyValue(xReceiverFrame, "VertOrientPosition");
                int iFrameY = FrameY.intValue();

                //Height of the Company Address in the Receiver Field
                int iReceiverHeight = (int) (0.5 * 1000);
                BusCompanyAddressReceiver = myLetterDoc.new BusinessPaperObject(PropertyNames.SPACE, iFrameWidth, iReceiverHeight, iFrameX, (iFrameY - iReceiverHeight));

                setPossibleAddressReceiver(false);
            }
            catch (NoSuchElementException e)
            {
                // TODO Error Message: Template modified!
                e.printStackTrace();
            }
            catch (WrappedTargetException e)
            {
                e.printStackTrace();
            }
            if (chkPaperCompanyAddress.getState() != 0)
            {
                setPossibleSenderData(false);
            }
        }
        else
        {
            if (BusCompanyAddressReceiver != null)
            {
                BusCompanyAddressReceiver.removeFrame();
            }
            setPossibleAddressReceiver(true);
            setPossibleSenderData(true);
            if (optSenderDefine.getState())
            {
                optSenderDefineItemChanged();
            }
            if (optSenderPlaceholder.getState())
            {
                optSenderPlaceholderItemChanged();
            }
        }
    }

    public void chkPaperFooterItemChanged()
    {

        if (chkPaperFooter.getState() != 0)
        {
            //minimal value is required, otherwise the frame creation fails
            if (numFooterHeight.getValue() == 0)
            {
                numFooterHeight.setValue(0.1);
            }
            BusFooter = myLetterDoc.new BusinessPaperObject("Footer", myLetterDoc.DocSize.Width, (int) (numFooterHeight.getValue() * 1000), 0, (int) (myLetterDoc.DocSize.Height - (numFooterHeight.getValue() * 1000)));
            this.setControlProperty("numFooterHeight", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
            this.setControlProperty("lblFooterHeight", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
            setPossibleFooter(false);
        }
        else
        {
            if (BusFooter != null)
            {
                BusFooter.removeFrame();
            }
            setControlProperty("numFooterHeight", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
            setControlProperty("lblFooterHeight", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
            setPossibleFooter(true);
        }
    }

    //switch Elements on/off -------------------------------------------------------
    public void chkUseLogoItemChanged()
    {
        try
        {
            if (myLetterDoc.hasElement("Company Logo"))
            {
                boolean logostatus = AnyConverter.toBoolean(getControlProperty("chkUseLogo", PropertyNames.PROPERTY_ENABLED)) && (chkUseLogo.getState() != 0);
                myLetterDoc.switchElement("Company Logo", logostatus);
            }
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
            throw new IllegalArgumentException("Fatal Error: Loading template failed: searching fillins failed");
        }
    }

    public void chkUseAddressReceiverItemChanged()
    {
        try
        {
            if (myLetterDoc.hasElement("Sender Address Repeated"))
            {
                boolean rstatus = AnyConverter.toBoolean(getControlProperty("chkUseAddressReceiver", PropertyNames.PROPERTY_ENABLED)) && (chkUseAddressReceiver.getState() != 0);
                myLetterDoc.switchElement("Sender Address Repeated", rstatus);
            }
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
            throw new IllegalArgumentException("Fatal Error: Loading template failed: searching fillins failed");
        }
    }

    public void chkUseSignsItemChanged()
    {
        if (myLetterDoc.hasElement("Letter Signs"))
        {
            myLetterDoc.switchElement("Letter Signs", (chkUseSigns.getState() != 0));
        }
    }

    public void chkUseSubjectItemChanged()
    {
        if (myLetterDoc.hasElement("Subject Line"))
        {
            myLetterDoc.switchElement("Subject Line", (chkUseSubject.getState() != 0));
        }
    }

    public void chkUseBendMarksItemChanged()
    {
        if (myLetterDoc.hasElement("Bend Marks"))
        {
            myLetterDoc.switchElement("Bend Marks", (chkUseBendMarks.getState() != 0));
        }
    }

    public void chkUseFooterItemChanged()
    {
        try
        {
            boolean bFooterPossible = (chkUseFooter.getState() != 0) && AnyConverter.toBoolean(getControlProperty("chkUseFooter", PropertyNames.PROPERTY_ENABLED));

            if (chkFooterNextPages.getState() != 0)
            {
                myLetterDoc.switchFooter("First Page", false, (chkFooterPageNumbers.getState() != 0), txtFooter.getText());
                myLetterDoc.switchFooter("Standard", bFooterPossible, (chkFooterPageNumbers.getState() != 0), txtFooter.getText());
            }
            else
            {
                myLetterDoc.switchFooter("First Page", bFooterPossible, (chkFooterPageNumbers.getState() != 0), txtFooter.getText());
                myLetterDoc.switchFooter("Standard", bFooterPossible, (chkFooterPageNumbers.getState() != 0), txtFooter.getText());
            }

            //enable/disable roadmap item for footer page
            XInterface BPaperItem = getRoadmapItemByID(RM_FOOTER);
            Helper.setUnoPropertyValue(BPaperItem, PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bFooterPossible));

        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public void chkFooterNextPagesItemChanged()
    {
        chkUseFooterItemChanged();
    }

    public void chkFooterPageNumbersItemChanged()
    {
        chkUseFooterItemChanged();
    }

    private void setPossibleFooter(boolean bState)
    {
        setControlProperty("chkUseFooter", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bState));
        chkUseFooterItemChanged();
    }

    private void setPossibleAddressReceiver(boolean bState)
    {
        if (myLetterDoc.hasElement("Sender Address Repeated"))
        {
            setControlProperty("chkUseAddressReceiver", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bState));
            chkUseAddressReceiverItemChanged();
        }
    }

    private void setPossibleLogo(boolean bState)
    {
        if (myLetterDoc.hasElement("Company Logo"))
        {
            setControlProperty("chkUseLogo", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bState));
            chkUseLogoItemChanged();
        }
    }

    public void txtFooterTextChanged()
    {
        chkUseFooterItemChanged();
    }

    public void txtSenderNameTextChanged()
    {
        TextFieldHandler myFieldHandler = new TextFieldHandler(myLetterDoc.xMSF, xTextDocument);
        myFieldHandler.changeUserFieldContent("Company", txtSenderName.getText());
    }

    public void txtSenderStreetTextChanged()
    {
        TextFieldHandler myFieldHandler = new TextFieldHandler(myLetterDoc.xMSF, xTextDocument);
        myFieldHandler.changeUserFieldContent("Street", txtSenderStreet.getText());
    }

    public void txtSenderCityTextChanged()
    {
        TextFieldHandler myFieldHandler = new TextFieldHandler(myLetterDoc.xMSF, xTextDocument);
        myFieldHandler.changeUserFieldContent("City", txtSenderCity.getText());
    }

    public void txtSenderPostCodeTextChanged()
    {
        TextFieldHandler myFieldHandler = new TextFieldHandler(myLetterDoc.xMSF, xTextDocument);
        myFieldHandler.changeUserFieldContent("PostCode", txtSenderPostCode.getText());
    }

    public void txtSenderStateTextChanged()
    {
        TextFieldHandler myFieldHandler = new TextFieldHandler(myLetterDoc.xMSF, xTextDocument);
        myFieldHandler.changeUserFieldContent(PropertyNames.PROPERTY_STATE, txtSenderState.getText());
    }

    public void txtTemplateNameTextChanged()
    {
        XDocumentPropertiesSupplier xDocPropsSuppl = UnoRuntime.queryInterface(XDocumentPropertiesSupplier.class, xTextDocument);
        XDocumentProperties xDocProps = xDocPropsSuppl.getDocumentProperties();
        String TitleName = txtTemplateName.getText();
        xDocProps.setTitle(TitleName);
    }

    public void chkUseSalutationItemChanged()
    {
        XTextComponent xTextComponent = UnoRuntime.queryInterface(XTextComponent.class, lstSalutation);
        myLetterDoc.switchUserField("Salutation", xTextComponent.getText(), (chkUseSalutation.getState() != 0));
        setControlProperty("lstSalutation", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(chkUseSalutation.getState() != 0));
    }

    public void lstSalutationItemChanged()
    {
        XTextComponent xTextComponent = UnoRuntime.queryInterface(XTextComponent.class, lstSalutation);
        myLetterDoc.switchUserField("Salutation", xTextComponent.getText(), (chkUseSalutation.getState() != 0));
    }

    public void lstSalutationTextChanged()
    {
    }

    public void chkUseGreetingItemChanged()
    {
        XTextComponent xTextComponent = UnoRuntime.queryInterface(XTextComponent.class, lstGreeting);
        myLetterDoc.switchUserField("Greeting", xTextComponent.getText(), (chkUseGreeting.getState() != 0));
        setControlProperty("lstGreeting", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(chkUseGreeting.getState() != 0));
    }

    private void setDefaultForGreetingAndSalutation()
    {
        XTextComponent xTextComponent;
        xTextComponent = UnoRuntime.queryInterface(XTextComponent.class, lstSalutation);
        if (xTextComponent.getText().equals(PropertyNames.EMPTY_STRING))
        {
            xTextComponent.setText(resources.SalutationLabels[0]);
        }
        xTextComponent = UnoRuntime.queryInterface(XTextComponent.class, lstGreeting);
        if (xTextComponent.getText().equals(PropertyNames.EMPTY_STRING))
        {
            xTextComponent.setText(resources.GreetingLabels[0]);
        }
    }

    public void lstGreetingItemChanged()
    {
        XTextComponent xTextComponent = UnoRuntime.queryInterface(XTextComponent.class, lstGreeting);
        myLetterDoc.switchUserField("Greeting", xTextComponent.getText(), (chkUseGreeting.getState() != 0));
    }

    public void lstGreetingTextChanged()
    {
    }

    //  ----------------------------------------------------------------------------
    public void chkBusinessPaperItemChanged()
    {
        //enable/disable Roadmap Entry Business Paper
        if (chkBusinessPaper.getState() != 0)
        {
            enableBusinessPaper();
        }
        else
        {
            disableBusinessPaper();
            setPossibleSenderData(true);
        }
    }

    private void setPossibleSenderData(boolean bState)
    {
        setControlProperty("optSenderDefine", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bState));
        setControlProperty("optSenderPlaceholder", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bState));
        setControlProperty("lblSenderAddress", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bState));
        if (!bState)
        {
            setControlProperty("txtSenderCity", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bState));
            setControlProperty("txtSenderName", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bState));
            setControlProperty("txtSenderPostCode", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bState));
            setControlProperty("txtSenderStreet", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bState));
            setControlProperty("txtSenderCity", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bState));
            setControlProperty("txtSenderState", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bState));
            setControlProperty("lblSenderName", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bState));
            setControlProperty("lblSenderStreet", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bState));
            setControlProperty("lblPostCodeCity", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bState));
        }
    }

    private void enableSenderReceiver()
    {
        XInterface BPaperItem = getRoadmapItemByID(RM_SENDERRECEIVER);
        Helper.setUnoPropertyValue(BPaperItem, PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
    }

    private void disableSenderReceiver()
    {
        XInterface BPaperItem = getRoadmapItemByID(RM_SENDERRECEIVER);
        Helper.setUnoPropertyValue(BPaperItem, PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
    }

    private void enableBusinessPaper()
    {
        XInterface BPaperItem = getRoadmapItemByID(RM_BUSINESSPAPER);
        Helper.setUnoPropertyValue(BPaperItem, PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        chkPaperCompanyLogoItemChanged();
        chkPaperCompanyAddressItemChanged();
        chkPaperFooterItemChanged();
        chkCompanyReceiverItemChanged();
    }

    private void disableBusinessPaper()
    {
        XInterface BPaperItem = getRoadmapItemByID(RM_BUSINESSPAPER);
        Helper.setUnoPropertyValue(BPaperItem, PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        if (BusCompanyLogo != null)
        {
            BusCompanyLogo.removeFrame();
        }
        if (BusCompanyAddress != null)
        {
            BusCompanyAddress.removeFrame();
        }
        if (BusFooter != null)
        {
            BusFooter.removeFrame();
        }
        if (BusCompanyAddressReceiver != null)
        {
            BusCompanyAddressReceiver.removeFrame();
        }
        setPossibleAddressReceiver(true);
        setPossibleFooter(true);
        setPossibleLogo(true);
        if (myLetterDoc.hasElement("Sender Address"))
        {
            myLetterDoc.switchElement("Sender Address", (true));
        }

    }

    public void initializeSalutation()
    {
        setControlProperty("lstSalutation", PropertyNames.STRING_ITEM_LIST, resources.SalutationLabels);
    }

    public void initializeGreeting()
    {
        setControlProperty("lstGreeting", PropertyNames.STRING_ITEM_LIST, resources.GreetingLabels);
    }

    private CGLetter getCurrentLetter()
    {
        switch (myConfig.cp_LetterType)
        {
            case 0:
                return myConfig.cp_BusinessLetter;
            case 1:
                return myConfig.cp_PrivateOfficialLetter;
            case 2:
                return myConfig.cp_PrivateLetter;
            default:
                return null;
        }
    }

    private void initializePaths()
    {
        try
        {
            XInterface xPathInterface = (XInterface) xMSF.createInstance("com.sun.star.util.PathSettings");
            XPropertySet xPropertySet = UnoRuntime.queryInterface(XPropertySet.class, xPathInterface);
            sTemplatePath = ((String[]) xPropertySet.getPropertyValue("Template_user"))[0];
            sUserTemplatePath = FileAccess.getOfficePath(xMSF, "Template", "user", PropertyNames.EMPTY_STRING);
            sBitmapPath = FileAccess.combinePaths(xMSF, sTemplatePath, "/../wizard/bitmap");
        }
        catch (NoValidPathException e)
        {
            e.printStackTrace();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    public boolean initializeTemplates(XMultiServiceFactory xMSF)
    {

      try
      {
        String sLetterPath = FileAccess.combinePaths(xMSF, sTemplatePath, "/../common/wizard/letter");

        BusinessFiles = FileAccess.getFolderTitles(xMSF, "bus", sLetterPath);
        OfficialFiles = FileAccess.getFolderTitles(xMSF, "off", sLetterPath);
        PrivateFiles = FileAccess.getFolderTitles(xMSF, "pri", sLetterPath);

        exchangeBusinessTitlesToLocalizedOnes();
        exchangeOfficialTitlesToLocalizedOnes();
        exchangePrivateTitlesToLocalizedOnes();

        setControlProperty("lstBusinessStyle", PropertyNames.STRING_ITEM_LIST, BusinessFiles[0]);
        setControlProperty("lstPrivOfficialStyle", PropertyNames.STRING_ITEM_LIST, OfficialFiles[0]);
        setControlProperty("lstPrivateStyle", PropertyNames.STRING_ITEM_LIST, PrivateFiles[0]);

        setControlProperty("lstBusinessStyle", PropertyNames.SELECTED_ITEMS, new short[]
                {
                    0
                });
        setControlProperty("lstPrivOfficialStyle", PropertyNames.SELECTED_ITEMS, new short[]
                {
                    0
                });
        setControlProperty("lstPrivateStyle", PropertyNames.SELECTED_ITEMS, new short[]
                {
                    0
                });
      }
      catch (com.sun.star.wizards.common.NoValidPathException e)
      {
        return false;
      }
      return true;
    }

    public void initializeElements()
    {
        setControlProperty("chkUseLogo", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(myLetterDoc.hasElement("Company Logo")));
        setControlProperty("chkUseBendMarks", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(myLetterDoc.hasElement("Bend Marks")));
        setControlProperty("chkUseAddressReceiver", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(myLetterDoc.hasElement("Sender Address Repeated")));
        setControlProperty("chkUseSubject", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(myLetterDoc.hasElement("Subject Line")));
        setControlProperty("chkUseSigns", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(myLetterDoc.hasElement("Letter Signs")));
        myLetterDoc.updateDateFields();
    }

    public void setConfiguration()
    {
        //set correct Configuration tree:

        if (optBusinessLetter.getState())
        {
            optBusinessLetterItemChanged();
        }
        if (optPrivOfficialLetter.getState())
        {
            optPrivOfficialLetterItemChanged();
        }
        if (optPrivateLetter.getState())
        {
            optPrivateLetterItemChanged();
        }
    }

    public void setElements(boolean privLetter)
    {
        //UI relevant:
        if (optSenderDefine.getState())
        {
            optSenderDefineItemChanged();
        }
        if (optSenderPlaceholder.getState())
        {
            optSenderPlaceholderItemChanged();
        }
        chkUseSignsItemChanged();
        chkUseSubjectItemChanged();
        chkUseSalutationItemChanged();
        chkUseGreetingItemChanged();
        chkUseBendMarksItemChanged();
        chkUseAddressReceiverItemChanged();
        txtTemplateNameTextChanged();

        //not UI relevant:
        if (optReceiverDatabase.getState() && !privLetter)
        {
            optReceiverDatabaseItemChanged();
        }
        if (optReceiverPlaceholder.getState() && !privLetter)
        {
            optReceiverPlaceholderItemChanged();
        }
        if (optCreateLetter.getState())
        {
            optCreateLetterItemChanged();
        }
        if (optMakeChanges.getState())
        {
            optMakeChangesItemChanged();
        }
    }

    public void insertRoadmap()
    {
        addRoadmap();
        int i = 0;
        i = insertRoadmapItem(0, true, resources.RoadmapLabels[RM_TYPESTYLE], RM_TYPESTYLE);
        i = insertRoadmapItem(i, false, resources.RoadmapLabels[RM_BUSINESSPAPER], RM_BUSINESSPAPER);
        i = insertRoadmapItem(i, true, resources.RoadmapLabels[RM_ELEMENTS], RM_ELEMENTS);
        i = insertRoadmapItem(i, true, resources.RoadmapLabels[RM_SENDERRECEIVER], RM_SENDERRECEIVER);
        i = insertRoadmapItem(i, false, resources.RoadmapLabels[RM_FOOTER], RM_FOOTER);
        i = insertRoadmapItem(i, true, resources.RoadmapLabels[RM_FINALSETTINGS], RM_FINALSETTINGS);
        setRoadmapInteractive(true);
        setRoadmapComplete(true);
        setCurrentRoadmapItemID((short) 1);
    }

    private class myPathSelectionListener implements XPathSelectionListener
    {

        public void validatePath()
        {
            if (myPathSelection.usedPathPicker)
            {
                filenameChanged = true;
            }
            myPathSelection.usedPathPicker = false;
        }
    }

    public void insertPathSelectionControl()
    {
        myPathSelection = new PathSelection(xMSF, this, PathSelection.TransferMode.SAVE, PathSelection.DialogTypes.FILE);
        myPathSelection.insert(6, 97, 70, 205, (short) 45, resources.reslblTemplatePath_value, true, HelpIds.getHelpIdString(HID + 47), HelpIds.getHelpIdString(HID + 48));
        myPathSelection.sDefaultDirectory = sUserTemplatePath;
        myPathSelection.sDefaultName = "myLetterTemplate.ott";
        myPathSelection.sDefaultFilter = "writer8_template";
        myPathSelection.addSelectionListener(new myPathSelectionListener());
    }

    public void initConfiguration()
    {
        try
        {
            myConfig = new CGLetterWizard();
            Object root = Configuration.getConfigurationRoot(xMSF, "/org.openoffice.Office.Writer/Wizards/Letter", false);
            myConfig.readConfiguration(root, "cp_");
            mainDA.add(RadioDataAware.attachRadioButtons(myConfig, "cp_LetterType", new Object[]
                    {
                        optBusinessLetter, optPrivOfficialLetter, optPrivateLetter
                    }, null, true));
            mainDA.add(UnoDataAware.attachListBox(myConfig.cp_BusinessLetter, "cp_Style", lstBusinessStyle, null, true));
            mainDA.add(UnoDataAware.attachListBox(myConfig.cp_PrivateOfficialLetter, "cp_Style", lstPrivOfficialStyle, null, true));
            mainDA.add(UnoDataAware.attachListBox(myConfig.cp_PrivateLetter, "cp_Style", lstPrivateStyle, null, true));
            mainDA.add(UnoDataAware.attachCheckBox(myConfig.cp_BusinessLetter, "cp_BusinessPaper", chkBusinessPaper, null, true));

            CGLetter cgl = myConfig.cp_BusinessLetter;

            CGPaperElementLocation cgpl = myConfig.cp_BusinessLetter.cp_CompanyLogo;
            CGPaperElementLocation cgpa = myConfig.cp_BusinessLetter.cp_CompanyAddress;

            businessDA.add(UnoDataAware.attachCheckBox(cgpl, "cp_Display", chkPaperCompanyLogo, null, true));
            businessDA.add(UnoDataAware.attachNumericControl(cgpl, "cp_Width", numLogoWidth, null, true));
            businessDA.add(UnoDataAware.attachNumericControl(cgpl, "cp_Height", numLogoHeight, null, true));
            businessDA.add(UnoDataAware.attachNumericControl(cgpl, "cp_X", numLogoX, null, true));
            businessDA.add(UnoDataAware.attachNumericControl(cgpl, "cp_Y", numLogoY, null, true));
            businessDA.add(UnoDataAware.attachCheckBox(cgpa, "cp_Display", chkPaperCompanyAddress, null, true));
            businessDA.add(UnoDataAware.attachNumericControl(cgpa, "cp_Width", numAddressWidth, null, true));
            businessDA.add(UnoDataAware.attachNumericControl(cgpa, "cp_Height", numAddressHeight, null, true));
            businessDA.add(UnoDataAware.attachNumericControl(cgpa, "cp_X", numAddressX, null, true));
            businessDA.add(UnoDataAware.attachNumericControl(cgpa, "cp_Y", numAddressY, null, true));

            businessDA.add(UnoDataAware.attachCheckBox(cgl, "cp_PaperCompanyAddressReceiverField", chkCompanyReceiver, null, true));
            businessDA.add(UnoDataAware.attachCheckBox(cgl, "cp_PaperFooter", chkPaperFooter, null, true));
            businessDA.add(UnoDataAware.attachNumericControl(cgl, "cp_PaperFooterHeight", numFooterHeight, null, true));

            letterDA.add(UnoDataAware.attachCheckBox(cgl, "cp_PrintCompanyLogo", chkUseLogo, null, true));
            letterDA.add(UnoDataAware.attachCheckBox(cgl, "cp_PrintCompanyAddressReceiverField", chkUseAddressReceiver, null, true));
            letterDA.add(UnoDataAware.attachCheckBox(cgl, "cp_PrintLetterSigns", chkUseSigns, null, true));
            letterDA.add(UnoDataAware.attachCheckBox(cgl, "cp_PrintSubjectLine", chkUseSubject, null, true));
            letterDA.add(UnoDataAware.attachCheckBox(cgl, "cp_PrintSalutation", chkUseSalutation, null, true));
            letterDA.add(UnoDataAware.attachCheckBox(cgl, "cp_PrintBendMarks", chkUseBendMarks, null, true));
            letterDA.add(UnoDataAware.attachCheckBox(cgl, "cp_PrintGreeting", chkUseGreeting, null, true));
            letterDA.add(UnoDataAware.attachCheckBox(cgl, "cp_PrintFooter", chkUseFooter, null, true));
            letterDA.add(UnoDataAware.attachEditControl(cgl, "cp_Salutation", lstSalutation, null, true));
            letterDA.add(UnoDataAware.attachEditControl(cgl, "cp_Greeting", lstGreeting, null, true));
            letterDA.add(RadioDataAware.attachRadioButtons(cgl, "cp_SenderAddressType", new Object[]
                    {
                        optSenderDefine, optSenderPlaceholder
                    }, null, true));
            letterDA.add(UnoDataAware.attachEditControl(cgl, "cp_SenderCompanyName", txtSenderName, null, true));
            letterDA.add(UnoDataAware.attachEditControl(cgl, "cp_SenderStreet", txtSenderStreet, null, true));
            letterDA.add(UnoDataAware.attachEditControl(cgl, "cp_SenderPostCode", txtSenderPostCode, null, true));
            letterDA.add(UnoDataAware.attachEditControl(cgl, "cp_SenderState", txtSenderState, null, true));
            letterDA.add(UnoDataAware.attachEditControl(cgl, "cp_SenderCity", txtSenderCity, null, true));
            letterDA.add(RadioDataAware.attachRadioButtons(cgl, "cp_ReceiverAddressType", new Object[]
                    {
                        optReceiverDatabase, optReceiverPlaceholder
                    }, null, true));
            letterDA.add(UnoDataAware.attachEditControl(cgl, "cp_Footer", txtFooter, null, true));
            letterDA.add(UnoDataAware.attachCheckBox(cgl, "cp_FooterOnlySecondPage", chkFooterNextPages, null, true));
            letterDA.add(UnoDataAware.attachCheckBox(cgl, "cp_FooterPageNumbers", chkFooterPageNumbers, null, true));
            letterDA.add(RadioDataAware.attachRadioButtons(cgl, "cp_CreationType", new Object[]
                    {
                        optCreateLetter, optMakeChanges
                    }, null, true));
            letterDA.add(UnoDataAware.attachEditControl(cgl, "cp_TemplateName", txtTemplateName, null, true));
            letterDA.add(UnoDataAware.attachEditControl(cgl, "cp_TemplatePath", myPathSelection.xSaveTextBox, null, true));

        }
        catch (Exception exception)
        {
            exception.printStackTrace();
        }

    }

    private void updateUI()
    {
        UnoDataAware.updateUI(mainDA);
        UnoDataAware.updateUI(letterDA);
        UnoDataAware.updateUI(businessDA);
    }

    public void saveConfiguration()
    {
        try
        {
            Object root = Configuration.getConfigurationRoot(xMSF, "/org.openoffice.Office.Writer/Wizards/Letter", true);
            myConfig.writeConfiguration(root, "cp_");
            Configuration.commit(root);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    private void exchangeBusinessTitlesToLocalizedOnes()
    {
      for(int i = 0; i < BusinessFiles[0].length; ++i)
      {
        if( BusinessFiles[0][i].equals("Elegant") )
        {
          BusinessFiles[0][i] = resources.resBusinessElegantTitle;
        }
        else if( BusinessFiles[0][i].equals("Modern") )
        {
          BusinessFiles[0][i] = resources.resBusinessModernTitle;
        }
        else if( BusinessFiles[0][i].equals("Office") )
        {
          BusinessFiles[0][i] = resources.resBusinessOfficeTitle;
        }
      }
    }

    private void exchangeOfficialTitlesToLocalizedOnes()
    {
      for(int i = 0; i < OfficialFiles[0].length; ++i)
      {
        if( OfficialFiles[0][i].equals("Elegant") )
        {
          OfficialFiles[0][i] = resources.resOfficialElegantTitle;
        }
        else if( OfficialFiles[0][i].equals("Modern") )
        {
          OfficialFiles[0][i] = resources.resOfficialModernTitle;
        }
        else if( OfficialFiles[0][i].equals("Office") )
        {
          OfficialFiles[0][i] = resources.resOfficialOfficeTitle;
        }
      }
    }
 
    private void exchangePrivateTitlesToLocalizedOnes()
    {
      for(int i = 0; i < PrivateFiles[0].length; ++i)
      {
        if( PrivateFiles[0][i].equals("Bottle") )
        {
          PrivateFiles[0][i] = resources.resPrivateBottleTitle;
        }
        else if( PrivateFiles[0][i].equals("Mail") )
        {
          PrivateFiles[0][i] = resources.resPrivateMailTitle;
        }
        else if( PrivateFiles[0][i].equals("Marine") )
        {
          PrivateFiles[0][i] = resources.resPrivateMarineTitle;
        }
        else if( PrivateFiles[0][i].equals("Red Line") )
        {
          PrivateFiles[0][i] = resources.resPrivateRedLineTitle;
        }
      }
    }

}
