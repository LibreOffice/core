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
package com.sun.star.wizards.fax;

import java.util.ArrayList;

import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.XTextComponent;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.document.MacroExecMode;
import com.sun.star.document.XDocumentProperties;
import com.sun.star.document.XDocumentPropertiesSupplier;
import com.sun.star.graphic.XGraphic;
import com.sun.star.graphic.XGraphicProvider;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.task.XInteractionHandler;
import com.sun.star.text.XTextDocument;
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
import com.sun.star.wizards.text.*;
import com.sun.star.wizards.common.TextElement;
import com.sun.star.wizards.common.PlaceholderTextElement;

import java.util.List;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.XInterface;

public class FaxWizardDialogImpl extends FaxWizardDialog
{

    protected void enterStep(int OldStep, int NewStep)
    {
    }

    protected void leaveStep(int OldStep, int NewStep)
    {
    }
    static FaxDocument myFaxDoc;
    static boolean running;
    XTextDocument xTextDocument;
    PathSelection myPathSelection;
    CGFaxWizard myConfig;
    ArrayList<DataAware> mainDA = new ArrayList<DataAware>();
    ArrayList<DataAware> faxDA = new ArrayList<DataAware>();
    String[][] BusinessFiles;
    String[][] PrivateFiles;
    String sTemplatePath;
    String sUserTemplatePath;
    String sBitmapPath;
    String sFaxPath;
    String sWorkPath;
    String sPath;
    boolean bEditTemplate;
    boolean bSaveSuccess = false;
    private boolean filenameChanged = false;
    final static int RM_TYPESTYLE = 1;
    final static int RM_ELEMENTS = 2;
    final static int RM_SENDERRECEIVER = 3;
    final static int RM_FOOTER = 4;
    final static int RM_FINALSETTINGS = 5;

    List<XTextRange> constRangeList = new ArrayList<XTextRange>();
    XTextRange trTo, trFrom, trFaxconst, trTelconst, trEmailconst, trConsist1, trConsist2, trConsist3;
    TextElement teTo, teFrom, teFaxconst, teTelconst, teEmailconst, teConsist1, teConsist2, teConsist3;

    public FaxWizardDialogImpl(XMultiServiceFactory xmsf)
    {
        super(xmsf);
    }

    public static void main(String args[])
    {
        //only being called when starting wizard remotely

        try
        {
            String ConnectStr = "uno:socket,host=127.0.0.1,port=8100;urp,negotiate=0,forcesynchronous=1;StarOffice.ServiceManager";
            XMultiServiceFactory xLocMSF = Desktop.connect(ConnectStr);
            FaxWizardDialogImpl lw = new FaxWizardDialogImpl(xLocMSF);
            lw.startWizard(xLocMSF, null);
        }
        catch (RuntimeException e)
        {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        catch (Exception e)
        {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        catch (java.lang.Exception e)
        {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

    }

    public void startWizard(XMultiServiceFactory xMSF, Object[] CurPropertyValue)
    {

        running = true;
        try
        {
            //Number of steps on WizardDialog:
            setMaxStep(5);

            //instatiate The Document Frame for the Preview
            myFaxDoc = new FaxDocument(xMSF, this);

            //create the dialog:
            drawNaviBar();
            buildStep1();
            buildStep2();
            buildStep3();
            buildStep4();
            buildStep5();

            initializeSalutation();
            initializeGreeting();
            initializeCommunication();
            initializePaths();

            //special Control for setting the save Path:
            insertPathSelectionControl();

            //load the last used settings from the registry and apply listeners to the controls:
            initConfiguration();

            initializeTemplates(xMSF);


            //update the dialog UI according to the loaded Configuration
            updateUI();

            if (myPathSelection.xSaveTextBox.getText().equalsIgnoreCase(PropertyNames.EMPTY_STRING))
            {
                myPathSelection.initializePath();
            }

            XWindow xContainerWindow = myFaxDoc.xFrame.getContainerWindow();
            XWindowPeer xWindowPeer = UnoRuntime.queryInterface(XWindowPeer.class, xContainerWindow);
            createWindowPeer(xWindowPeer);

            //add the Roadmap to the dialog:
            insertRoadmap();

            //load the last used document and apply last used settings:
            //TODO:
            setConfiguration();

            //If the configuration does not define Greeting/Salutation/CommunicationType yet choose a default
            setDefaultForGreetingAndSalutationAndCommunication();

            //disable funtionality that is not supported by the template:
            initializeElements();

            //disable the document, so that the user cannot change anything:
            myFaxDoc.xFrame.getComponentWindow().setEnable(false);

            executeDialog(myFaxDoc.xFrame);
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
        myFaxDoc.setWizardTemplateDocInfo(resources.resFaxWizardDialog_title, resources.resTemplateDescription);
        try
        {
            //myFaxDoc.xTextDocument.lockControllers();
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
            myFaxDoc.setWizardTemplateDocInfo(resources.resFaxWizardDialog_title, resources.resTemplateDescription);
            myFaxDoc.killEmptyUserFields();
            myFaxDoc.keepLogoFrame = (chkUseLogo.getState() != 0);
            myFaxDoc.keepTypeFrame = (chkUseCommunicationType.getState() != 0);
            myFaxDoc.killEmptyFrames();


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
            XCloseable xCloseable = UnoRuntime.queryInterface(XCloseable.class, myFaxDoc.xFrame);
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
                if (text.equals(resources.resToPlaceHolder))
                {
                    teTo = new PlaceholderTextElement(item, resources.resToPlaceHolder_value, "hint", myFaxDoc.xMSF);
                    trTo = item;
                    constRangeList.remove(i--);
                    writeTitle(teTo,trTo,resources.resToPlaceHolder_value);
                }
                else if (text.equals(resources.resFromPlaceHolder))
                {
                    teFrom = new PlaceholderTextElement(item, resources.resFromPlaceHolder_value, "hint", myFaxDoc.xMSF);
                    trFrom = item;
                    constRangeList.remove(i--);
                    writeTitle(teFrom,trFrom,resources.resFromPlaceHolder_value);
                }
                else if (text.equals(resources.resFaxconstPlaceHolder))
                {
                    teFaxconst = new PlaceholderTextElement(item, resources.resFaxconstPlaceHolder_value, "hint", myFaxDoc.xMSF);
                    trFaxconst = item;
                    constRangeList.remove(i--);
                    writeTitle(teFaxconst,trFaxconst,resources.resFaxconstPlaceHolder_value);
                }
                else if (text.equals(resources.resTelconstPlaceHolder))
                {
                    teTelconst = new PlaceholderTextElement(item, resources.resTelconstPlaceHolder_value, "hint", myFaxDoc.xMSF);
                    trTelconst = item;
                    constRangeList.remove(i--);
                    writeTitle(teTelconst,trTelconst,resources.resTelconstPlaceHolder_value);
                }
                else if (text.equals(resources.resEmailconstPlaceHolder))
                {
                    teEmailconst = new PlaceholderTextElement(item, resources.resEmailconstPlaceHolder_value, "hint", myFaxDoc.xMSF);
                    trEmailconst = item;
                    constRangeList.remove(i--);
                    writeTitle(teEmailconst,trEmailconst,resources.resEmailconstPlaceHolder_value);
                }
                else if (text.equals(resources.resConsist1PlaceHolder))
                {
                    teConsist1 = new PlaceholderTextElement(item, resources.resConsist1PlaceHolder_value, "hint", myFaxDoc.xMSF);
                    trConsist1 = item;
                    constRangeList.remove(i--);
                    writeTitle(teConsist1,trConsist1,resources.resConsist1PlaceHolder_value);
                }
                else if (text.equals(resources.resConsist2PlaceHolder))
                {
                    teConsist2 = new PlaceholderTextElement(item, resources.resConsist2PlaceHolder_value, "hint", myFaxDoc.xMSF);
                    trConsist2 = item;
                    constRangeList.remove(i--);
                    writeTitle(teConsist2,trConsist2,resources.resConsist2PlaceHolder_value);
                }
                else if (text.equals(resources.resConsist3PlaceHolder))
                {
                    teConsist3 = new PlaceholderTextElement(item, resources.resConsist3PlaceHolder_value, "hint", myFaxDoc.xMSF);
                    trConsist3 = item;
                    constRangeList.remove(i--);
                    writeTitle(teConsist3,trConsist3,resources.resConsist3PlaceHolder_value);
                }

            }
    }

    public void clearConstants()
    {
        constRangeList.clear();
        trTo = null;
        trFrom = null;
        trFaxconst = null;
        trTelconst = null;
        trEmailconst = null;
        trConsist1 = null;
        trConsist2 = null;
        trConsist3 = null;
        teTo = null;
        teFrom = null;
        teFaxconst = null;
        teTelconst = null;
        teEmailconst = null;
        teConsist1 = null;
        teConsist2 = null;
        teConsist3 = null;
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

    private void exchangeTitlesToLocalizedOnes()
    {
      for(int i = 0; i < BusinessFiles[0].length; ++i)
      {
        if( BusinessFiles[0][i].equals("Classic Fax") )
        {
          BusinessFiles[0][i] = resources.resBusinessFaxClassic;
        }
        else if( BusinessFiles[0][i].equals("Classic Fax from Private") )
        {
          BusinessFiles[0][i] = resources.resBusinessFaxClassicPrivate;
        }
        else if( BusinessFiles[0][i].equals("Modern Fax") )
        {
          BusinessFiles[0][i] = resources.resBusinessFaxModern;
        }
        else if( BusinessFiles[0][i].equals("Modern Fax from Private") )
        {
          BusinessFiles[0][i] = resources.resBusinessFaxModernPrivate;
        }
      }

      for(int i = 0; i < PrivateFiles[0].length; ++i)
      {
        if( PrivateFiles[0][i].equals("Bottle") )
        {
          PrivateFiles[0][i] = resources.resPrivateFaxBottle;
        }
        else if( PrivateFiles[0][i].equals("Lines") )
        {
          PrivateFiles[0][i] = resources.resPrivateFaxLines;
        }
        else if( PrivateFiles[0][i].equals("Marine") )
        {
          PrivateFiles[0][i] = resources.resPrivateFaxMarine;
        }
      }
    }

    public void insertRoadmap()
    {
        addRoadmap();
        int i = 0;
        i = insertRoadmapItem(0, true, resources.RoadmapLabels[RM_TYPESTYLE], RM_TYPESTYLE);
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
        myPathSelection.insert(5, 97, 70, 205, (short) 45, resources.reslblTemplatePath_value, true, HelpIds.getHelpIdString(HID + 34), HelpIds.getHelpIdString(HID + 35));
        myPathSelection.sDefaultDirectory = sUserTemplatePath;
        myPathSelection.sDefaultName = "myFaxTemplate.ott";
        myPathSelection.sDefaultFilter = "writer8_template";
        myPathSelection.addSelectionListener(new myPathSelectionListener());
    }

    private void insertIcons()
    {
        try
        {
            Object oGS = xMSF.createInstance("com.sun.star.graphic.GraphicProvider");
            XGraphicProvider xGraphicProvider = UnoRuntime.queryInterface(XGraphicProvider.class, oGS);

            PropertyValue GraphicValues[] = new PropertyValue[1];
            GraphicValues[0] = new PropertyValue();
            GraphicValues[0].Name = PropertyNames.URL;
            GraphicValues[0].Value = "private:resource/svx/imagelist/18000/18022";
            XGraphic xGraphic = xGraphicProvider.queryGraphic(GraphicValues);

        }
        catch (Exception e)
        {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

    }

    private void updateUI()
    {
        UnoDataAware.updateUI(mainDA);
        UnoDataAware.updateUI(faxDA);
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
            String sFaxSubPath = "/wizard/fax";
            sFaxPath = FileAccess.combinePaths(xMSF, sTemplatePath, sFaxSubPath);
            sWorkPath = FileAccess.getOfficePath(xMSF, "Work", PropertyNames.EMPTY_STRING, PropertyNames.EMPTY_STRING);

            BusinessFiles = FileAccess.getFolderTitles(xMSF, "bus", sFaxPath);
            PrivateFiles = FileAccess.getFolderTitles(xMSF, "pri", sFaxPath);

            exchangeTitlesToLocalizedOnes();

            setControlProperty("lstBusinessStyle", PropertyNames.STRING_ITEM_LIST, BusinessFiles[0]);
            setControlProperty("lstPrivateStyle", PropertyNames.STRING_ITEM_LIST, PrivateFiles[0]);

            setControlProperty("lstBusinessStyle", PropertyNames.SELECTED_ITEMS, new short[]
                    {
                        0
                    });
            setControlProperty("lstPrivateStyle", PropertyNames.SELECTED_ITEMS, new short[]
                    {
                        0
                    });

            return true;
        }
        catch (NoValidPathException e)
        {
            // TODO Auto-generated catch block
            e.printStackTrace();
            return false;
        }
    }

    public void initializeElements()
    {
        setControlProperty("chkUseLogo", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(myFaxDoc.hasElement("Company Logo")));
        setControlProperty("chkUseSubject", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(myFaxDoc.hasElement("Subject Line")));
        setControlProperty("chkUseDate", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(myFaxDoc.hasElement("Date")));
        myFaxDoc.updateDateFields();
    }

    public void initializeSalutation()
    {
        setControlProperty("lstSalutation", PropertyNames.STRING_ITEM_LIST, resources.SalutationLabels);
    }

    public void initializeGreeting()
    {
        setControlProperty("lstGreeting", PropertyNames.STRING_ITEM_LIST, resources.GreetingLabels);
    }

    public void initializeCommunication()
    {
        setControlProperty("lstCommunicationType", PropertyNames.STRING_ITEM_LIST, resources.CommunicationLabels);
    }

    private void setDefaultForGreetingAndSalutationAndCommunication()
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
        xTextComponent = UnoRuntime.queryInterface(XTextComponent.class, lstCommunicationType);
        if (xTextComponent.getText().equals(PropertyNames.EMPTY_STRING))
        {
            xTextComponent.setText(resources.CommunicationLabels[0]);
        }
    }

    public void initConfiguration()
    {
        try
        {
            myConfig = new CGFaxWizard();
            Object root = Configuration.getConfigurationRoot(xMSF, "/org.openoffice.Office.Writer/Wizards/Fax", false);
            myConfig.readConfiguration(root, "cp_");
            mainDA.add(RadioDataAware.attachRadioButtons(myConfig, "cp_FaxType", new Object[]
                    {
                        optBusinessFax, optPrivateFax
                    }, null, true));
            mainDA.add(UnoDataAware.attachListBox(myConfig.cp_BusinessFax, "cp_Style", lstBusinessStyle, null, true));
            mainDA.add(UnoDataAware.attachListBox(myConfig.cp_PrivateFax, "cp_Style", lstPrivateStyle, null, true));

            CGFax cgl = myConfig.cp_BusinessFax;

            faxDA.add(UnoDataAware.attachCheckBox(cgl, "cp_PrintCompanyLogo", chkUseLogo, null, true));
            faxDA.add(UnoDataAware.attachCheckBox(cgl, "cp_PrintSubjectLine", chkUseSubject, null, true));
            faxDA.add(UnoDataAware.attachCheckBox(cgl, "cp_PrintSalutation", chkUseSalutation, null, true));
            faxDA.add(UnoDataAware.attachCheckBox(cgl, "cp_PrintDate", chkUseDate, null, true));
            faxDA.add(UnoDataAware.attachCheckBox(cgl, "cp_PrintCommunicationType", chkUseCommunicationType, null, true));
            faxDA.add(UnoDataAware.attachCheckBox(cgl, "cp_PrintGreeting", chkUseGreeting, null, true));
            faxDA.add(UnoDataAware.attachCheckBox(cgl, "cp_PrintFooter", chkUseFooter, null, true));
            faxDA.add(UnoDataAware.attachEditControl(cgl, "cp_Salutation", lstSalutation, null, true));
            faxDA.add(UnoDataAware.attachEditControl(cgl, "cp_Greeting", lstGreeting, null, true));
            faxDA.add(UnoDataAware.attachEditControl(cgl, "cp_CommunicationType", lstCommunicationType, null, true));
            faxDA.add(RadioDataAware.attachRadioButtons(cgl, "cp_SenderAddressType", new Object[]
                    {
                        optSenderDefine, optSenderPlaceholder
                    }, null, true));
            faxDA.add(UnoDataAware.attachEditControl(cgl, "cp_SenderCompanyName", txtSenderName, null, true));
            faxDA.add(UnoDataAware.attachEditControl(cgl, "cp_SenderStreet", txtSenderStreet, null, true));
            faxDA.add(UnoDataAware.attachEditControl(cgl, "cp_SenderPostCode", txtSenderPostCode, null, true));
            faxDA.add(UnoDataAware.attachEditControl(cgl, "cp_SenderState", txtSenderState, null, true));
            faxDA.add(UnoDataAware.attachEditControl(cgl, "cp_SenderCity", txtSenderCity, null, true));
            faxDA.add(UnoDataAware.attachEditControl(cgl, "cp_SenderFax", txtSenderFax, null, true));
            faxDA.add(RadioDataAware.attachRadioButtons(cgl, "cp_ReceiverAddressType", new Object[]
                    {
                        optReceiverDatabase, optReceiverPlaceholder
                    }, null, true));
            faxDA.add(UnoDataAware.attachEditControl(cgl, "cp_Footer", txtFooter, null, true));
            faxDA.add(UnoDataAware.attachCheckBox(cgl, "cp_FooterOnlySecondPage", chkFooterNextPages, null, true));
            faxDA.add(UnoDataAware.attachCheckBox(cgl, "cp_FooterPageNumbers", chkFooterPageNumbers, null, true));
            faxDA.add(RadioDataAware.attachRadioButtons(cgl, "cp_CreationType", new Object[]
                    {
                        optCreateFax, optMakeChanges
                    }, null, true));
            faxDA.add(UnoDataAware.attachEditControl(cgl, "cp_TemplateName", txtTemplateName, null, true));
            faxDA.add(UnoDataAware.attachEditControl(cgl, "cp_TemplatePath", myPathSelection.xSaveTextBox, null, true));

        }
        catch (Exception exception)
        {
            exception.printStackTrace();
        }

    }

    public void saveConfiguration()
    {
        try
        {
            Object root = Configuration.getConfigurationRoot(xMSF, "/org.openoffice.Office.Writer/Wizards/Fax", true);
            myConfig.writeConfiguration(root, "cp_");
            Configuration.commit(root);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    public void setConfiguration()
    {
        //set correct Configuration tree:
        if (optBusinessFax.getState())
        {
            optBusinessFaxItemChanged();
        }
        if (optPrivateFax.getState())
        {
            optPrivateFaxItemChanged();
        }
    }

    public void optBusinessFaxItemChanged()
    {
        DataAware.setDataObject(faxDA, myConfig.cp_BusinessFax, true);
        setControlProperty("lblBusinessStyle", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("lstBusinessStyle", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("lblPrivateStyle", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("lstPrivateStyle", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        lstBusinessStyleItemChanged();
        enableSenderReceiver();
        setPossibleFooter(true);
    }

    public void lstBusinessStyleItemChanged()
    {
        xTextDocument = myFaxDoc.loadAsPreview(BusinessFiles[1][lstBusinessStyle.getSelectedItemPos()], false);
        initializeElements();
        setElements();
        clearConstants();
        drawConstants();
    }

    public void optPrivateFaxItemChanged()
    {
        DataAware.setDataObject(faxDA, myConfig.cp_PrivateFax, true);
        setControlProperty("lblBusinessStyle", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("lstBusinessStyle", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("lblPrivateStyle", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("lstPrivateStyle", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        lstPrivateStyleItemChanged();
        disableSenderReceiver();
        setPossibleFooter(false);
    }

    public void lstPrivateStyleItemChanged()
    {
        xTextDocument = myFaxDoc.loadAsPreview(PrivateFiles[1][lstPrivateStyle.getSelectedItemPos()], false);
        initializeElements();
        setElements();
    }

    public void txtTemplateNameTextChanged()
    {
        XDocumentPropertiesSupplier xDocPropsSuppl = UnoRuntime.queryInterface(XDocumentPropertiesSupplier.class, xTextDocument);
        XDocumentProperties xDocProps = xDocPropsSuppl.getDocumentProperties();
        String TitleName = txtTemplateName.getText();
        xDocProps.setTitle(TitleName);
    }

    public void optSenderPlaceholderItemChanged()
    {
        setControlProperty("lblSenderName", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("lblSenderStreet", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("lblPostCodeCity", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("lblSenderFax", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("txtSenderName", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("txtSenderStreet", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("txtSenderPostCode", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("txtSenderState", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("txtSenderCity", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        setControlProperty("txtSenderFax", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        myFaxDoc.fillSenderWithUserData();
    }

    public void optSenderDefineItemChanged()
    {
        setControlProperty("lblSenderName", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("lblSenderStreet", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("lblPostCodeCity", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("lblSenderFax", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("txtSenderName", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("txtSenderStreet", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("txtSenderPostCode", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("txtSenderState", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("txtSenderCity", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        setControlProperty("txtSenderFax", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        txtSenderNameTextChanged();
        txtSenderStreetTextChanged();
        txtSenderPostCodeTextChanged();
        txtSenderStateTextChanged();
        txtSenderCityTextChanged();
        txtSenderFaxTextChanged();
    }

    public void optReceiverPlaceholderItemChanged()
    {
        OfficeDocument.attachEventCall(xTextDocument, "OnNew", "StarBasic", "macro:///Template.Correspondence.Placeholder()");
    }

    public void optReceiverDatabaseItemChanged()
    {
        OfficeDocument.attachEventCall(xTextDocument, "OnNew", "StarBasic", "macro:///Template.Correspondence.Database()");
    }

    public void optCreateFaxItemChanged()
    {
        bEditTemplate = false;
    }

    public void optMakeChangesItemChanged()
    {
        bEditTemplate = true;
    }

    public void txtSenderNameTextChanged()
    {
        TextFieldHandler myFieldHandler = new TextFieldHandler(myFaxDoc.xMSF, xTextDocument);
        myFieldHandler.changeUserFieldContent("Company", txtSenderName.getText());
    }

    public void txtSenderStreetTextChanged()
    {
        TextFieldHandler myFieldHandler = new TextFieldHandler(myFaxDoc.xMSF, xTextDocument);
        myFieldHandler.changeUserFieldContent("Street", txtSenderStreet.getText());
    }

    public void txtSenderCityTextChanged()
    {
        TextFieldHandler myFieldHandler = new TextFieldHandler(myFaxDoc.xMSF, xTextDocument);
        myFieldHandler.changeUserFieldContent("City", txtSenderCity.getText());
    }

    public void txtSenderPostCodeTextChanged()
    {
        TextFieldHandler myFieldHandler = new TextFieldHandler(myFaxDoc.xMSF, xTextDocument);
        myFieldHandler.changeUserFieldContent("PostCode", txtSenderPostCode.getText());
    }

    public void txtSenderStateTextChanged()
    {
        TextFieldHandler myFieldHandler = new TextFieldHandler(myFaxDoc.xMSF, xTextDocument);
        myFieldHandler.changeUserFieldContent(PropertyNames.PROPERTY_STATE, txtSenderState.getText());
    }

    public void txtSenderFaxTextChanged()
    {
        TextFieldHandler myFieldHandler = new TextFieldHandler(myFaxDoc.xMSF, xTextDocument);
        myFieldHandler.changeUserFieldContent("Fax", txtSenderFax.getText());
    }
    //switch Elements on/off -------------------------------------------------------
    public void setElements()
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
        chkUseLogoItemChanged();
        chkUseSubjectItemChanged();
        chkUseSalutationItemChanged();
        chkUseGreetingItemChanged();
        chkUseCommunicationItemChanged();
        chkUseDateItemChanged();
        chkUseFooterItemChanged();
        txtTemplateNameTextChanged();

        //not UI relevant:
        if (optReceiverDatabase.getState())
        {
            optReceiverDatabaseItemChanged();
        }
        if (optReceiverPlaceholder.getState())
        {
            optReceiverPlaceholderItemChanged();
        }
        if (optCreateFax.getState())
        {
            optCreateFaxItemChanged();
        }
        if (optMakeChanges.getState())
        {
            optMakeChangesItemChanged();
        }
    }

    public void chkUseLogoItemChanged()
    {
        if (myFaxDoc.hasElement("Company Logo"))
        {
            myFaxDoc.switchElement("Company Logo", (chkUseLogo.getState() != 0));
        }
    }

    public void chkUseSubjectItemChanged()
    {
        if (myFaxDoc.hasElement("Subject Line"))
        {
            myFaxDoc.switchElement("Subject Line", (chkUseSubject.getState() != 0));
        }
    }

    public void chkUseDateItemChanged()
    {
        if (myFaxDoc.hasElement("Date"))
        {
            myFaxDoc.switchElement("Date", (chkUseDate.getState() != 0));
        }
    }

    public void chkUseFooterItemChanged()
    {
        try
        {
            boolean bFooterPossible = (chkUseFooter.getState() != 0) && AnyConverter.toBoolean(getControlProperty("chkUseFooter", PropertyNames.PROPERTY_ENABLED));

            if (chkFooterNextPages.getState() != 0)
            {
                myFaxDoc.switchFooter("First Page", false, (chkFooterPageNumbers.getState() != 0), txtFooter.getText());
                myFaxDoc.switchFooter("Standard", bFooterPossible, (chkFooterPageNumbers.getState() != 0), txtFooter.getText());
            }
            else
            {
                myFaxDoc.switchFooter("First Page", bFooterPossible, (chkFooterPageNumbers.getState() != 0), txtFooter.getText());
                myFaxDoc.switchFooter("Standard", bFooterPossible, (chkFooterPageNumbers.getState() != 0), txtFooter.getText());
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

    public void txtFooterTextChanged()
    {
        chkUseFooterItemChanged();
    }

    public void chkUseSalutationItemChanged()
    {
        XTextComponent xTextComponent = UnoRuntime.queryInterface(XTextComponent.class, lstSalutation);
        myFaxDoc.switchUserField("Salutation", xTextComponent.getText(), (chkUseSalutation.getState() != 0));
        setControlProperty("lstSalutation", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(chkUseSalutation.getState() != 0));
    }

    public void lstSalutationItemChanged()
    {
        XTextComponent xTextComponent = UnoRuntime.queryInterface(XTextComponent.class, lstSalutation);
        myFaxDoc.switchUserField("Salutation", xTextComponent.getText(), (chkUseSalutation.getState() != 0));
    }

    public void lstSalutationTextChanged()
    {
    }

    public void chkUseCommunicationItemChanged()
    {
        XTextComponent xTextComponent = UnoRuntime.queryInterface(XTextComponent.class, lstCommunicationType);
        myFaxDoc.switchUserField("CommunicationType", xTextComponent.getText(), (chkUseCommunicationType.getState() != 0));
        setControlProperty("lstCommunicationType", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(chkUseCommunicationType.getState() != 0));
    }

    public void lstCommunicationItemChanged()
    {
        XTextComponent xTextComponent = UnoRuntime.queryInterface(XTextComponent.class, lstCommunicationType);
        myFaxDoc.switchUserField("CommunicationType", xTextComponent.getText(), (chkUseCommunicationType.getState() != 0));
    }

    public void lstCommunicationTextChanged()
    {
    }

    public void chkUseGreetingItemChanged()
    {
        XTextComponent xTextComponent = UnoRuntime.queryInterface(XTextComponent.class, lstGreeting);
        myFaxDoc.switchUserField("Greeting", xTextComponent.getText(), (chkUseGreeting.getState() != 0));
        setControlProperty("lstGreeting", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(chkUseGreeting.getState() != 0));
    }

    public void lstGreetingItemChanged()
    {
        XTextComponent xTextComponent = UnoRuntime.queryInterface(XTextComponent.class, lstGreeting);
        myFaxDoc.switchUserField("Greeting", xTextComponent.getText(), (chkUseGreeting.getState() != 0));
    }

    public void lstGreetingTextChanged()
    {
    }

    private void setPossibleFooter(boolean bState)
    {
        setControlProperty("chkUseFooter", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bState));
        if (!bState)
        {
            chkUseFooter.setState((short) 0);
        }
        chkUseFooterItemChanged();
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
}





