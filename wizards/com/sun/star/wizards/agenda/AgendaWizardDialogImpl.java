/*************************************************************************
*
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AgendaWizardDialogImpl.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-21 16:13:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/package com.sun.star.wizards.agenda;

import java.util.Vector;

import com.sun.star.awt.ItemEvent;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.XItemListener;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.CloseVetoException;
import com.sun.star.util.XCloseable;
import com.sun.star.wizards.common.Configuration;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.NoValidPathException;
import com.sun.star.wizards.common.SystemDialog;
import com.sun.star.wizards.document.OfficeDocument;
import com.sun.star.wizards.text.ViewHandler;
import com.sun.star.wizards.ui.PathSelection;
import com.sun.star.wizards.ui.XPathSelectionListener;
import com.sun.star.wizards.ui.event.DataAware;
import com.sun.star.wizards.ui.event.RadioDataAware;
import com.sun.star.wizards.ui.event.UnoDataAware;

/**
 * This class is the dialog implementation class -
 * there is not much business logic here - but mostley
 * event methods.
 * Some event methods are also implemented in TopicsControl and TopicsControl.ControlRow.
 * @author rp143992
 */
public class AgendaWizardDialogImpl extends AgendaWizardDialog
{
    /**
     * used to prevent a double start of the wizard.
     */
    static boolean running;

    /**
     * the preview document controller.
     */
    AgendaTemplate agendaTemplate;
    /**
     * the data model, read from the OOo configuration.
     * (live synchronized: when the user changes the gui,
     * the data model changes, except for topics).
     */
    private CGAgenda agenda;

    /**
     * the topics control, a gui element which
     * manipulates the topics data according to the
     * user's input.
     */
    private TopicsControl topicsControl;

    /**
     * an array with two array memebers:
     * agendaTemplates[0] contains an array with
     * UI template names.
     * agendaTemplates[1] contains an array with
     * corresponding URLs.
     */
    private String[][] agendaTemplates;

    PathSelection myPathSelection;

    String sTemplatePath;
    String sUserTemplatePath;
    String sBitmapPath;

    String sPath;

    /** constructor */
    public AgendaWizardDialogImpl(XMultiServiceFactory xmsf)
    {
        super(xmsf);
    }


    /**
     * read the configuration data, open the specified template,
     * initialize the template controller (AgendaTemplate) and
     * set the status of the displayed template to the one
     * read from the configuration.
     * build the dialog.
     * Synchronize the dialog to the same status (read from
     * the configuration).
     * show the dialog.
     */
    public void startWizard() {
        running = true;
        try  {
            // read configuration data.
            agenda = new CGAgenda();

            Object root = Configuration.getConfigurationRoot(xMSF, "/org.openoffice.Office.Writer/Wizards/Agenda", false);
            agenda.readConfiguration(root,"cp_");

            // initialize the agenda template
            agendaTemplate = new AgendaTemplate(xMSF, agenda, resources, this);
            initializeTemplates();

            agendaTemplate.load(agendaTemplates[1][agenda.cp_AgendaType] , new Vector());

            // build the dialog.
            buildStep1();
            buildStep2();
            buildStep3();
            buildStep4();
            buildStep5();
            topicsControl = new TopicsControl(this,xMSF, agenda);
            buildStep6();
            drawNaviBar();

            initializePaths();

            //special Control for setting the save Path:
            insertPathSelectionControl();

            // create the peer
            XWindow xw = agendaTemplate.xFrame.getContainerWindow();
            XWindowPeer xWindowPeer = (XWindowPeer) UnoRuntime.queryInterface(XWindowPeer.class, xw);
            this.createWindowPeer( xWindowPeer );

            // initialize roadmap
            this.addRoadmap();
            this.insertRoadMapItems(
                    new String[] { resources.resStep1, resources.resStep2, resources.resStep3, resources.resStep4, resources.resStep5, resources.resStep6, } ,
                    new int[] { 1,2,3,4,5,6},
                    new boolean[] { true,true,true,true,true,true }
                    );
            this.setMaxStep(6);

            // synchronize GUI and CGAgenda object.
            makeDA();
            if(myPathSelection.xSaveTextBox.getText().equalsIgnoreCase("")) {myPathSelection.initializePath();}

        }
        catch (Exception ex) {
            ex.printStackTrace();
            running=false;
            return;
        }

        // show dialog.
        xWindow.setVisible(true);

    }

    private class myPathSelectionListener implements XPathSelectionListener {
        public void validatePath() {
            if (myPathSelection.usedPathPicker) {
                filenameChanged = true;
            }
            myPathSelection.usedPathPicker = false;
        }
    }


    public void insertPathSelectionControl() {
        myPathSelection = new PathSelection(xMSF, this, PathSelection.TransferMode.SAVE, PathSelection.DialogTypes.FILE);
        myPathSelection.insert(6, 97, 70, 205, (short) 45, resources.reslblTemplatePath_value, true, "HID:" + ( HID + 47 ), "HID:" + ( HID + 48 ));
        myPathSelection.sDefaultDirectory = sUserTemplatePath;
        myPathSelection.sDefaultName = "myAgendaTemplate.ott";
        myPathSelection.sDefaultFilter = "writer8_template";
        myPathSelection.addSelectionListener(new myPathSelectionListener());
    }

    private void initializePaths() {
        try {
            sTemplatePath = FileAccess.getOfficePath(xMSF, "Template", "share");
            sUserTemplatePath = FileAccess.getOfficePath(xMSF, "Template", "user");
            sBitmapPath = FileAccess.combinePaths(xMSF, sTemplatePath, "/wizard/bitmap");
        } catch (NoValidPathException e) {
            e.printStackTrace();
        }
    }

    private void checkSavePath() {
        if (agenda.cp_TemplatePath == null ||
                agenda.cp_TemplatePath.equals("") ||
                !getFileAccess().exists(FileAccess.getParentDir(agenda.cp_TemplatePath),false) ||
                !getFileAccess().isDirectory(FileAccess.getParentDir(agenda.cp_TemplatePath )))
            {
                try  {
                    agenda.cp_TemplatePath =
                        FileAccess.connectURLs(
                            FileAccess.getOfficePath(xMSF, "Work", "") ,
                            resources.resDefaultFilename
                        );
                }
                catch (Exception ex) {
                    ex.printStackTrace();
                }
            }
    }

    /**
     * bind controls to the agenda member (DataAware model)
     */
    private void makeDA() {

        setControlProperty("listPageDesign", "StringItemList", agendaTemplates[0]);

        checkSavePath();
        //setFilename(agenda.cp_TemplatePath);

        UnoDataAware.attachListBox( agenda, "cp_AgendaType", listPageDesign , null, true ).updateUI();
        UnoDataAware.attachCheckBox( agenda, "cp_IncludeMinutes", chkMinutes, null, true).updateUI();

        UnoDataAware.attachEditControl(agenda, "cp_Title", txtTitle, agendaTemplate , true).updateUI();
        UnoDataAware.attachDateControl(agenda, "cp_Date", txtDate, agendaTemplate , true).updateUI();
        UnoDataAware.attachTimeControl(agenda, "cp_Time", txtTime, agendaTemplate , true).updateUI();
        UnoDataAware.attachEditControl(agenda, "cp_Location", cbLocation, agendaTemplate , true).updateUI();

        UnoDataAware.attachCheckBox(agenda,"cp_ShowMeetingType", chkMeetingTitle , new RedrawListener( TemplateConsts.FILLIN_MEETING_TYPE ) , true);
        UnoDataAware.attachCheckBox(agenda,"cp_ShowRead", chkRead , new RedrawListener( TemplateConsts.FILLIN_READ ) , true).updateUI();
        UnoDataAware.attachCheckBox(agenda,"cp_ShowBring", chkBring , new RedrawListener( TemplateConsts.FILLIN_BRING ) , true).updateUI();
        UnoDataAware.attachCheckBox(agenda,"cp_ShowNotes", chkNotes , new RedrawListener( TemplateConsts.FILLIN_NOTES ) , true).updateUI();

        UnoDataAware.attachCheckBox(agenda,"cp_ShowCalledBy", chkConvenedBy , new RedrawListener( TemplateConsts.FILLIN_CALLED_BY ) , true).updateUI();
        UnoDataAware.attachCheckBox(agenda,"cp_ShowFacilitator", chkPresiding , new RedrawListener( TemplateConsts.FILLIN_FACILITATOR ) , true).updateUI();
        UnoDataAware.attachCheckBox(agenda,"cp_ShowNotetaker", chkNoteTaker , new RedrawListener( TemplateConsts.FILLIN_NOTETAKER ) , true).updateUI();
        UnoDataAware.attachCheckBox(agenda,"cp_ShowTimekeeper", chkTimekeeper , new RedrawListener( TemplateConsts.FILLIN_TIMEKEEPER ) , true).updateUI();
        UnoDataAware.attachCheckBox(agenda,"cp_ShowAttendees", chkAttendees , new RedrawListener( TemplateConsts.FILLIN_PARTICIPANTS ) , true).updateUI();
        UnoDataAware.attachCheckBox(agenda,"cp_ShowObservers", chkObservers , new RedrawListener( TemplateConsts.FILLIN_OBSERVERS ) , true).updateUI();
        UnoDataAware.attachCheckBox(agenda,"cp_ShowResourcePersons", chkResourcePersons , new RedrawListener( TemplateConsts.FILLIN_RESOURCE_PERSONS ) , true).updateUI();

        UnoDataAware.attachEditControl(agenda, "cp_TemplateName", txtTemplateName, null, true).updateUI();
        RadioDataAware.attachRadioButtons( agenda, "cp_ProceedMethod",
                new Object[] {optCreateAgenda, optMakeChanges} ,null,true).updateUI();

        listPageDesign.addItemListener(new XItemListener() {
            public void itemStateChanged(ItemEvent ie) {
                pageDesignChanged(ie);
            }
            public void disposing(EventObject eo) {}
        });
    }

    /** used in developement to start the wizard */
    public static void main(String args[])
    {
        String ConnectStr = "uno:socket,host=127.0.0.1,port=8100;urp,negotiate=0,forcesynchronous=1;StarOffice.NamingService"; //localhost
                                                                                                                               // ;Lo-1.Germany.sun.com;
                                                                                                                               // 10.16.65.155
        try {
            XMultiServiceFactory xLocMSF = Desktop.connect(ConnectStr);
            AgendaWizardDialogImpl wizard = new AgendaWizardDialogImpl(xLocMSF);
            wizard.startWizard();

        }
        catch (Exception exception) {
            exception.printStackTrace();
        }
    }


    /*
    private void initializePaths() {
        try {

            sUserTemplatePath = FileAccess.getOfficePath(xMSF, "Template", "user");
            sBitmapPath = FileAccess.combinePaths(xMSF, sTemplatePath, "/wizard/bitmap");
        } catch (NoValidPathException e) {
            e.printStackTrace();
        }
    }*/

    /**
     * read the available agenda wizard templates.
     */
    public boolean initializeTemplates() {
        try {
             String sTemplatePath = FileAccess.getOfficePath(xMSF, "Template", "share");

            //sCurrentNorm = Norms[getCurrentLetter().cp_Norm];
            String sAgendaPath = FileAccess.combinePaths(xMSF, sTemplatePath, "/wizard/agenda" );

            agendaTemplates = FileAccess.getFolderTitles(xMSF, "aw" , sAgendaPath);

            return true;
        } catch (NoValidPathException nopathexception) {
            nopathexception.printStackTrace();
            return false;
        } catch (Exception exception) {
            exception.printStackTrace();
            return false;
        }
    }

    /* ******************************
     * Event methods
     * ******************************
     */

    /**
     * first page, page design listbox changed.
     */
    public void pageDesignChanged(ItemEvent ie) {
        int selected = ie.Selected;
        try {
            agendaTemplate.load( agendaTemplates[1][selected] , topicsControl.getTopicsData());
        }
        catch (Exception ex) {
            SystemDialog.showMessageBox(xMSF, "ErrBox", VclWindowPeerAttribute.OK, resources.resErrOpenTemplate);
            ex.printStackTrace();
        }
    }

    /**
     * last page, template title changed...
     */
    public void templateTitleChanged() {
        String title = (String)Helper.getUnoPropertyValue( getModel(txtTemplateName), "Text" );
        agendaTemplate.setTemplateTitle( title );
    }


    private FileAccess fileAccess1;
    /** convenience method.
     *  instead of creating a FileAccess object every time
     *  it is needed, I have a FileAccess object memeber.
     *  the first time it is needed it will be created, and
     *  then be reused...
     * @return the FileAccess memeber object.
     */
    private FileAccess getFileAccess() {
        if (fileAccess1 == null)
            try {
                fileAccess1 = new FileAccess(xMSF);
            } catch (Exception e) {
                e.printStackTrace();
            }
        return fileAccess1;

    }
    /**
     * indicates if the filename was changed by the user through
     * the "save as" dialog.
     * If it is so, one needs not warn the user
     * upon overwrite, since she was already warned.
     */
    private boolean filenameChanged = false;

    /**
     * last page, "browse" ("...") button was clicked...
     */
    public void saveAs() {
        try {

            checkSavePath();

            SystemDialog saveAs = SystemDialog.createStoreDialog(xMSF);
            saveAs.addFilterToDialog("ott","writer8_template",true);
            // call the saveAs dialog.
            String url = saveAs.callStoreDialog(
                FileAccess.getParentDir(agenda.cp_TemplatePath),
                FileAccess.getFilename(agenda.cp_TemplatePath));

            if (url != null) {
                agenda.cp_TemplatePath = url;
                setFilename(url);
                filenameChanged = true;
            }
        }
        catch (Exception ex) {
            ex.printStackTrace();
        }


    }

    /**
     * is called when the user
     * changes the path through the "save as" dialog.
     * The path displayed is a translated, user-friendly, platform dependant path.
     * @param url the new save url.
     */
    private void setFilename(String url) {
        try {
            String path = getFileAccess().getPath(url,"");
            Helper.setUnoPropertyValue( getModel(myPathSelection.xSaveTextBox), "Text", path);
        }
        catch (Exception ex) {
            ex.printStackTrace();
        }

    }

    public void insertRow() {
        topicsControl.insertRow();
    }

    public void removeRow() {
        topicsControl.removeRow();
    }

    public void rowUp() {
        topicsControl.rowUp();
    }

    public void rowDown() {
        topicsControl.rowDown();
    }


    /* ************************
     * Navigation bar methods
     * ************************
     */

    public void cancelWizard() {
        xWindow.setVisible(false);
        closeDocument();
        running = false;
    }

    public void finishWizard() {
        boolean bSaveSuccess = false; // pesimistic :(
        XTextDocument xTextDocument;

        try {
            FileAccess fileAccess = new FileAccess(xMSF);
            sPath = myPathSelection.getSelectedPath();
            if (sPath.equals("")) {
                myPathSelection.triggerPathPicker();
                sPath = myPathSelection.getSelectedPath();
            }
            sPath = fileAccess.getURL(sPath);

            //first, if the filename was not changed, thus
            //it is coming from a saved session, check if the
            // file exists and warn the user.
            if (!filenameChanged)
                if (fileAccess.exists(sPath, true)) {

                    int answer = SystemDialog.showMessageBox(xMSF, xControl.getPeer(), "MessBox", VclWindowPeerAttribute.YES_NO + VclWindowPeerAttribute.DEF_NO, resources.resFileExists);
                    if (answer == 3) // user said: no, do not overwrite....
                        return;
                }

            agendaTemplate.xTextDocument.lockControllers();

            xTextDocument = (XTextDocument)UnoRuntime.queryInterface(XTextDocument.class,agendaTemplate.document);

            bSaveSuccess = OfficeDocument.store(xMSF, xTextDocument, sPath , "writer8_template", false, resources.resErrSaveTemplate );
        } catch (Exception e) {
            SystemDialog.showMessageBox(xMSF, xControl.getPeer(), "ErrBox", VclWindowPeerAttribute.OK, resources.resErrSaveTemplate);
            //e.printStackTrace();
        }

        if (bSaveSuccess) {
            try {
                topicsControl.saveTopics(agenda);
                Object root = Configuration.getConfigurationRoot(xMSF, "/org.openoffice.Office.Writer/Wizards/Agenda", true);
                agenda.writeConfiguration(root,"cp_");
                Configuration.commit(root);
            }
            catch (Exception ex) {
                ex.printStackTrace();
            }

            agendaTemplate.finish( topicsControl.getTopicsData());
            try {
                XStorable xStoreable = (XStorable) UnoRuntime.queryInterface(XStorable.class, agendaTemplate.document);
                xStoreable.store();
            }
            catch (Exception ex) {
                SystemDialog.showMessageBox(xMSF, "ErrBox", VclWindowPeerAttribute.OK, resources.resErrSaveTemplate);
                ex.printStackTrace();
            }

            xWindow.setVisible(false);
            running = false;
            closeDocument();
            agendaTemplate.xTextDocument.unlockControllers();

            PropertyValue loadValues[] = new PropertyValue[1];
            loadValues[0] = new PropertyValue();
            loadValues[0].Name = "AsTemplate";
            if (agenda.cp_ProceedMethod == 1) {
                loadValues[0].Value = Boolean.TRUE;
            } else {
                loadValues[0].Value = Boolean.FALSE;
            }
            Object oDoc = OfficeDocument.load(Desktop.getDesktop(xMSF), sPath, "_default", new PropertyValue[0]);
            xTextDocument = (com.sun.star.text.XTextDocument) oDoc;
            XMultiServiceFactory xDocMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);
            ViewHandler myViewHandler = new ViewHandler(xDocMSF, xTextDocument);
            try {
                myViewHandler.setViewSetting("ZoomType", new Short(com.sun.star.view.DocumentZoomType.OPTIMAL));
            }
            catch (Exception ex) {
                ex.printStackTrace();
            }
        } else {
            agendaTemplate.xTextDocument.unlockControllers();
            return;
        }
    }

    private void closeDocument() {
        try {
            xComponent.dispose();
            XCloseable xCloseable = (XCloseable) UnoRuntime.queryInterface(XCloseable.class, agendaTemplate.xFrame);
            xCloseable.close(false);
        } catch (CloseVetoException e) {
            e.printStackTrace();
        }
    }

    /* ********************
     * Sub Classes
     * ********************
     */


    /**
     * this class is used to redraw an item's table when
     * the user clicks one of the checkboxes in step 3 or 4.
     */
    private class RedrawListener implements DataAware.Listener {

        private String itemName;

        public RedrawListener(String itemName_) {
            itemName = itemName_;
        }

        /* (non-Javadoc)
         * @see com.sun.star.wizards.ui.event.DataAware.Listener#eventPerformed(java.lang.Object)
         */
        public void eventPerformed(Object event) {
            agendaTemplate.xTextDocument.lockControllers();
            agendaTemplate.redraw(itemName);
            agendaTemplate.xTextDocument.unlockControllers();
        }
    }


}
    