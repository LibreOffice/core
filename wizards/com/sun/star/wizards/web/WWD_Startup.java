/*************************************************************************
 *
 *  $RCSfile: WWD_Startup.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $  $Date: 2004-05-19 13:15:03 $
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
 *  Contributor(s): _______________________________________
 *
 */
package com.sun.star.wizards.web;

import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.io.FileNotFoundException;
import java.util.List;
import java.util.Vector;

import com.sun.star.awt.KeyEvent;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.XKeyListener;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XEnumeration;
import com.sun.star.frame.FrameSearchFlag;
import com.sun.star.frame.XController;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;

import com.sun.star.uno.Any;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.document.OfficeDocument;
import com.sun.star.wizards.ui.*;
import com.sun.star.wizards.ui.event.*;
import com.sun.star.wizards.web.data.*;

/**
 * Web Wizard Dialog implementation : Startup.
 * This class contains the startup implementation and session mounting methods.<p/>
 * <h2> Data Aware Concept </h2>
 * <p> I use DataAware objects, which synchronize
 * UI and Data. A DataAware object a kind of interface
 * between a Value and a UI element.
 * </P>
 * <p>
 * A Value is either a JavaBean property with
 * a Getter and a Setter or a public class Memeber.
 * </P>
 * When the UI Control changes, the Value changes correspondingly.
 * This depends on settings a Listener which calls the updateData()
 * method od the DataAware object.
 * When the Value changes, the UI Control changes respopndingly.
 * This also depends on settings a Listener which calls the updateUI()
 * method of the DataAware object.
 * </P>
 * <P>
 * here I use only UI Listeners, and keep calling
 * the updateUI() method whenever I change the Value.
 * </P>
 * To contain the Data, I use the Configuration Data Objects
 * which read themselfs out of the Configuration. they are all located under
 * the data package.
 * <p/>
 * Different groups of DataAware objects are grouped into vectors.
 * Each group (all elements of a vector) uses the same DataObject,
 * so when I change the underlying dataObject, I change the whole vector's DataObject,
 * thus actualizing UI.<br/>
 * This is what happends when a session loads: I Load the session into
 * a tree of DataObjects, and change the DataAware's DataObject. <br/>
 * One could also look at this as a kind of "View-Model" relationship.
 * The controls are the View, The DataObjects are the Model, containing the
 * Data rad directly from the configuration, and the DataAware objects
 * are the controller inbetween.
 *
 * @author rpiterman
 *
 */

public abstract class WWD_Startup extends WWD_General {

    /**
     * This is currently not used.
     * It should have been a Listener which invoces
     * an Update to the Preview-Document, only
     * We did not implement the live-preview document,
     * so this became obsolete.
     */
    protected DataAware.Listener refresh;
    /**
     * Is called whenever a field is changed which
     * could affect the "create" button enable state,
     * like publishing-checkboxes, save-session check- and textbox.
     */
    protected DataAware.Listener checkPublish = new CheckPublish();

    /**
     * The Vector containing DataAware objects
     * which relay on the selected document
     * as DataObject (or Model).
     */
    protected List docAware = new Vector();
    /**
     * The Vector containing DataAware objects
     * which relay on the session's generalInfo
     * Object (CGSession.cp_GeneralInfo) as
     * DataObject (or model).
     */
    protected List genAware = new Vector();
    /**
     * The vector containing DataAware objectscm25npd ..
     * which relay on the session's Design Object
     * (CGSession.cp_Design) as DataObject
     * (or model).
     *
     */
    protected List designAware = new Vector();
    /**
     * A Vector containig the DataAware objects
     * which relay on Publishing Objects.
     * Differently from other Vectors, each
     * DataAware object in this Vector relays
     * on a <b>different</b> CGPublish object,
     * So they are handled with more care.
     */
    protected List pubAware = new Vector(3);

    /**
     * The DataAware object which handles
     * the documents-list events.
     * This will call the set/getDocument() method
     * implemented in WWD_General.
     */
    protected UnoDataAware docListDA;
    /**
     * DataAware object which synchronizes the session
     * name in step 7 with the CGSession.cp_Name
     */
    protected UnoDataAware sessionNameDA;
    /**
     * I Seperate this one because I need
     * to call it exclusivly (updateUI()) when
     * setting the FavIcon URL.
     */
    protected UnoDataAware favIconDA;

    /**
     * Binds a ListModel to the UnoControlListBox.
     */
    protected ListModelBinder docsBinder;

    /**
     *  a DocumentPreview object is
     * the UI OO Document preview
     * in the Dialog
     */
    protected DocumentPreview dpStylePreview;
    /**
     * An Object that implements the
     * File-copy and refresh of the
     * style preview
     */
    protected StylePreview stylePreview;

    /**
     * the currently selected document.
     */
    protected short[] selectedDoc = new short[0];

    protected XFrame myOwnFrame;
    protected XFrame desktopFrame;
    /* ******************************************
     *  ****************************************
     *             General Methods
     *  ****************************************
     * ******************************************  */

    /*
     * GENERAL Initialization  methods
     */

    /**
     * He - my constructor !
     * I call/do here in this order: <br/>
     * Check if ftp http proxy is set, and warn user he can not use
     * ftp if  it is.<br/>
     * draw NaviBar and steps 1 to 7, incl. Step X,
     * which is the Layouts ImageList on step 3.<br/>
     * load the settings from the configuration. <br/>
     * set the default save session name to the first
     * available one (sessionX or something like that). <br/>
     * set the Layouts ImageList ListModel, and call
     * its "create()" method. <br/>
     * check the content (documents) specified in the default session.<br/>
     * fill the list boxes (saved sessions, styles, combobox save session). <br/>
     * make data aware. <br/>
     * updateUI (refreshes the display to crrespond to the data in
     * the default session.)<br/>
     * fill the documents listbox. <br/>
     * if proxies are set, disable the ftp controls in step 7.
     */
    public WWD_Startup(XMultiServiceFactory xmsf) throws Exception {
        super(xmsf);

        checkProxies();

        this.drawNaviBar();
        this.buildStep1();
        this.buildStep2();
        this.buildStep3();

        this.buildStep4();
        this.buildStep5();
        this.buildStep6();
        this.buildStep7();
        buildStepX();

        loadSettings();
        setSaveSessionName();

        ilLayouts.setListModel(settings.cp_Layouts);
        ilLayouts.create(this);

        checkContent(settings.cp_DefaultSession.cp_Content, new Task("", "", 99999));

        //saved sessions, styles, combobox save session.
        fillLists();
        makeDataAware();
        // change the display to correspond to the current session.
        updateUI();
        // fill the documents listbox.
        mount(settings.cp_DefaultSession.cp_Content);


        if (proxies) {
            setEnabled(btnFTP,false);
            setEnabled(chkFTP,false);
        }
    }

    /**
     * if FTP http proxy is set, notify the user.
     * @throws StoppedByUserException if the user chooses not
     * to start the wizard.
     */
    private void checkProxies() throws Exception {
        if (getOOProxies()) {
            boolean b = AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(),resources.resErrProxies, ErrorHandler.ERROR_NORMAL_IGNORE);
            if (!b)
                throw new StoppedByUserException("OOProxies not allowed");
            else
                proxies = true;
        }
    }

    /**
     * return true if http proxies or other proxies
     * which do not enable ftp to function correctly are set.
     * @return true if (http) proxies are on.
     * @throws Exception
     */
    private boolean getOOProxies() throws Exception {
        Object node = Configuration.getConfigurationRoot(xMSF,"org.openoffice.Inet/Settings",false);
         int i = Configuration.getInt("ooInetProxyType",node);
         //System.out.println("WWD:Startup:getOOProxies:" + i);
         switch (i) {
             case 0 : //no proxies
                 return false;
             case 2 : //http proxies
                 return true;
             default :
                 return true;
         }
    }


    /**
     * calculates the first available session name,
     * and sets the current session name to it.
     * The combobox text in step 7 will be updated
     * automatically when updateUI() is called.
     */
    private void setSaveSessionName() {
        int max = 0;
        int len = resources.resSessionName.length();
        // traverse between the sessions and find the one that
        // has the biggest number.
        for (int i = 0; i < settings.cp_SavedSessions.getSize(); i++) {
            String sessionName = ((CGSessionName) settings.cp_SavedSessions.getElementAt(i)).cp_Name;
            if (sessionName.startsWith(resources.resSessionName))
                max = max(max, Integer.valueOf(sessionName.substring(len)).intValue());
        }

        settings.cp_DefaultSession.cp_Name = resources.resSessionName + ++max;

    }


    private int max(int i1, int i2) {
        if (i1 > i2)
            return i1;
        else
            return i2;
    }

    /**
     * fills the road map, and sets the necessary properties,
     * like MaxStep, Complete, Interactive-
     * Disables the finbihButton.
     */
    private void addRoadMapItems() {
        insertRoadMapItems(new String[] { resources.resStep1, resources.resStep2, resources.resStep3, resources.resStep4, resources.resStep5, resources.resStep6, resources.resStep7  }, new int[] { 1, 2, 3, 4, 5, 6, 7 }, new boolean[] { true, true, false, false, false, false, false });

        setRoadmapInteractive(true);
        setRoadmapComplete(true);
        setCurrentRoadmapItemID((short) 1);
        setMaxStep(7);
        enableFinishButton(false);

    }

    /**
     * This method goes through all
     * the DataAware vectors and objects and
     * calls their updateUI() method.
     * In response, the UI is synchronized to
     * the state of the corresponding data objects.
     *
     */
    private void updateUI() {
        DataAware.updateUI(designAware);
        DataAware.updateUI(genAware);
        DataAware.updateUI(pubAware);
        sessionNameDA.updateUI();
        favIconDA.updateUI();
        checkPublish();
    }



    /**
     * create the peer, add roadmap,
     * add roadmap items, add style-preview,
     * disable the steps 3 to 7 if no documents are
     * on the list, and... show the dialog!
     */
    public void show() {
        try {

            desktopFrame = Desktop.getActiveFrame(xMSF);

            myOwnFrame = OfficeDocument.createNewFrame(xMSF);

            desktopFrame = Desktop.findAFrame(xMSF, myOwnFrame, desktopFrame);



            XWindow xContainerWindow = myOwnFrame.getContainerWindow();
            XWindowPeer xWindowPeer = (XWindowPeer) UnoRuntime.queryInterface(XWindowPeer.class, xContainerWindow);
            createWindowPeer(xWindowPeer);

            addRoadmap();
            addRoadMapItems();
            addStylePreview();
            checkSteps();

            executeDialog(desktopFrame);

        } catch (java.lang.Exception jexception) {
            jexception.printStackTrace(System.out);
        }
    }

    /**
     * initializes the style preview.
     */
    private void addStylePreview() {
        try {
            dpStylePreview = new DocumentPreview(xMSF, imgPreview);
            stylePreview = new StylePreview(xMSF, settings.workPath);
            stylePreview.refresh(settings.cp_DefaultSession.getStyle(), settings.cp_DefaultSession.cp_Design.cp_BackgroundImage);
            dpStylePreview.setDocument(stylePreview.htmlFilename, DocumentPreview.PREVIEW_MODE);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    /**
     * Loads the web wizard settings from the registry.
     */
    private void loadSettings() {
        try {
            settings = new CGSettings(xMSF);

            Object confRoot = Configuration.getConfigurationRoot(xMSF, CONFIG_PATH, false);
            settings.readConfiguration(confRoot, CONFIG_READ_PARAM);

            ConfigSet set = settings.cp_DefaultSession.cp_Publishing;

            for (int i = 0; i<set.getSize(); i++) {
                CGPublish p =(CGPublish)set.getElementAt(i);
                p.cp_URL = substitute(p.cp_URL);
            }

            settings.configure(xMSF);
            settings.resCreated = resources.resCreatedTemplate;
            settings.resUpdated = resources.resUpdatedTemplate;
            settings.resPages = resources.resPages;
            settings.resSlides = resources.resSlides;

        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    /**
     * fills the saved session list, the styles list,
     * and the save session combo box.
     */
    private void fillLists() {
        ListModelBinder.fillList(lstLoadSettings, settings.cp_SavedSessions.items(), null);
        ListModelBinder.fillList(lstStyles, settings.cp_Styles.items(), null);
        ListModelBinder.fillComboBox(cbSaveSettings, settings.cp_SavedSessions.items(), null);
    }

    /**
     * attaches to each ui-data-control (like checkbox, groupbox or
     * textbox, no buttons though), a DataObject's JavaBean Property,
     * or class member.
     */
    private void makeDataAware() {
        //page 1
        new ListModelBinder(lstLoadSettings, settings.cp_SavedSessions);

        //page 2 : document properties

        docListDA = (UnoDataAware) UnoDataAware.attachListBox(this, "SelectedDoc", lstDocuments, null, false);
        docListDA.disableControls(new Object[] {
            /*btnDocDown, btnDocUp, */
            lnDocsInfo, btnRemoveDoc, lblDocTitle, txtDocTitle, lblDocInfo, txtDocInfo, lblDocAuthor, txtDocAuthor, lblDocExportFormat, lstDocTargetType });
        docListDA.updateUI();

        CGDocument doc = new CGDocument(); //dummy

        docsBinder = new ListModelBinder(lstDocuments, settings.cp_DefaultSession.cp_Content.cp_Documents);

        docAware.add(UnoDataAware.attachEditControl(doc, "cp_Title", txtDocTitle, refresh, true ));
        docAware.add(UnoDataAware.attachEditControl(doc, "cp_Description", txtDocInfo, refresh, true));
        docAware.add(UnoDataAware.attachEditControl(doc, "cp_Author", txtDocAuthor, refresh, true));
        docAware.add(UnoDataAware.attachListBox(doc, "Exporter", lstDocTargetType, refresh, false));

        //page 3 : Layout
        Object design = settings.cp_DefaultSession.cp_Design;
        final SimpleDataAware sda = new SimpleDataAware(design,
                new DataAware.PropertyValue("Layout",design), ilLayouts,
                new DataAware.PropertyValue("Selected",ilLayouts));
            ilLayouts.addItemListener(new ItemListener() {
                public void itemStateChanged(ItemEvent ie) {
                    sda.updateData();
                    //TODO xf uncomment
                    //refresh.eventPerformed(ie);
                }
            });
            designAware.add(sda);

        //page 4 : layout 2
        designAware.add(UnoDataAware.attachCheckBox(design, "cp_DisplayDescription", chbDocDesc, refresh, true));
        designAware.add(UnoDataAware.attachCheckBox(design, "cp_DisplayAuthor", chbDocAuthor, refresh, true));
        designAware.add(UnoDataAware.attachCheckBox(design, "cp_DisplayCreateDate", chkDocCreated, refresh, true));
        designAware.add(UnoDataAware.attachCheckBox(design, "cp_DisplayUpdateDate", chkDocChanged, refresh, true));
        designAware.add(UnoDataAware.attachCheckBox(design, "cp_DisplayFilename", chkDocFilename, refresh, true));
        designAware.add(UnoDataAware.attachCheckBox(design, "cp_DisplayFileFormat", chkDocFormat, refresh, true));
        designAware.add(UnoDataAware.attachCheckBox(design, "cp_DisplayFormatIcon", chkDocFormatIcon, refresh, true));
        designAware.add(UnoDataAware.attachCheckBox(design, "cp_DisplayPages", chkDocPages, refresh, true));
        designAware.add(UnoDataAware.attachCheckBox(design, "cp_DisplaySize", chkDocSize, refresh, true));
        designAware.add(RadioDataAware.attachRadioButtons(settings.cp_DefaultSession.cp_Design, "cp_OptimizeDisplaySize", new Object[] { optOptimize640x480, optOptimize800x600, optOptimize1024x768 }, refresh, true));


        //page 5 : Style
        /*
         * note : on style change, i do not call here refresh ,but rather on
         * a special method which will perform some display, background and Iconsets changes.
         */
        designAware.add(UnoDataAware.attachListBox(settings.cp_DefaultSession.cp_Design, "Style", lstStyles, null, false));

        //page 6 : site general props
        genAware.add(UnoDataAware.attachEditControl(settings.cp_DefaultSession.cp_GeneralInfo, "cp_Title", txtSiteTitle, refresh, true));
        genAware.add(UnoDataAware.attachEditControl(settings.cp_DefaultSession.cp_GeneralInfo, "cp_Description", txtSiteDesc, refresh, true));

        //note: FavIcon textbox is handeled seperatly from all other "general properties".
        try {
            favIconDA = UnoDataAware.attachEditControl(this, "Icon", txtFavIcon, null, false);
            MethodInvocation mi1 = new MethodInvocation("removeFavIcon",this,KeyEvent.class);
            guiEventListener.add("txtFavIcon",EventNames.EVENT_KEY_PRESSED,mi1);
            ((XWindow)UnoRuntime.queryInterface(XWindow.class,txtFavIcon)).addKeyListener((XKeyListener)guiEventListener);
        }
        catch (Exception ex) {
            ex.printStackTrace();
        }

        genAware.add(UnoDataAware.attachEditControl(settings.cp_DefaultSession.cp_GeneralInfo, "cp_Keywords", txtSiteKeywords, null, true));
        genAware.add(UnoDataAware.attachDateControl(settings.cp_DefaultSession.cp_GeneralInfo, "CreationDate", dateSiteCreated, refresh, false));
        genAware.add(UnoDataAware.attachDateControl(settings.cp_DefaultSession.cp_GeneralInfo, "UpdateDate", dateSiteUpdate, refresh, false));
        genAware.add(UnoDataAware.attachNumericControl(settings.cp_DefaultSession.cp_GeneralInfo, "cp_RevisitAfter", txtRevisitAfter, null, true));
        genAware.add(UnoDataAware.attachEditControl(settings.cp_DefaultSession.cp_GeneralInfo, "cp_Email", txtEmail, refresh, true));
        genAware.add(UnoDataAware.attachEditControl(settings.cp_DefaultSession.cp_GeneralInfo, "cp_Copyright", txtCopyright, refresh, true));

        //page 7 : publishing

        pubAware(LOCAL_PUBLISHER, chkLocalDir, txtLocalDir);
        pubAware(FTP_PUBLISHER, chkFTP, txtFTP);
        pubAware(ZIP_PUBLISHER, chkZip, txtZip);

        sessionNameDA = UnoDataAware.attachEditControl(settings.cp_DefaultSession, "cp_Name", cbSaveSettings, null, true);

        //cleanup when exiting wizard.
        //guiEventListener.add("WebWizardDialog",EventNames.EVENT_WINDOW_HIDDEN, "cleanup", this);
        //xWindow.addWindowListener((XWindowListener)guiEventListener);
    }


    /**
     * A help method to attach a Checkbox and a TextBox to
     * a CGPublish object properties/class members,
     * @param publish
     * @param checkbox
     * @param textbox
     */
    private void pubAware(String publish, Object checkbox, Object textbox) {
        Object p = settings.cp_DefaultSession.cp_Publishing.getElement(publish);
        UnoDataAware uda = UnoDataAware.attachCheckBox(p, "cp_Publish", checkbox, checkPublish, true);
        uda.setInverse(true);

        uda.disableControls(new Object[] { textbox });
        pubAware.add(uda);
        pubAware.add(UnoDataAware.attachEditControl(p, "URL", textbox, checkPublish, false));
    }

    /*
     * Session load methods
     * (are used both on the start of the wizard and
     * when the user loads a session)
     */

    /**
     * If called when a new session/settings is
     * loaded. <br/>
     * Checks the documents (validate), fills the
     * documents listbox, and changes the
     * DataAware data objects to the
     * new session's objects.<br/>
     * Task advances 4 times in the mount method,
     * and ??? times for each document in the session.
     *
     */
    protected void mount(CGSession session, Task task) {
        /* This checks the documents. If the user
         * chooses to cancel, the session is not loaded.
         */
        checkContent(session.cp_Content, task);

        settings.cp_DefaultSession = session;

        mount(session.cp_Content);

        task.advance(true);

        mount(session.cp_Design, designAware);
        mount(session.cp_GeneralInfo, genAware);

        task.advance(true);

        mount(session.cp_Publishing.getElement(LOCAL_PUBLISHER), 0);
        mount(session.cp_Publishing.getElement(FTP_PUBLISHER), 1);
        mount(session.cp_Publishing.getElement(ZIP_PUBLISHER), 2);

        task.advance(true);

        sessionNameDA.setDataObject(session, true);
        Helper.setUnoPropertyValue(getModel(chkSaveSettings), "State", new Short((short) 1));
        favIconDA.updateUI();
        docListDA.updateUI();

        task.advance(true);

        refreshStylePreview();

        //updateUI();
    }

    /**
     * used to mount the publishing objects. This is somehow cryptic -
     * there are 6 DataAware objects, 2 for each Publishing Object (CGPublish).
     * All 6 reside on one Vector (pubAware).
     * Since, otherwise than other DataAware objects, different
     * Objects on the same Vector need different DataObjectBinding,
     * I use this method...
     * @param data the CGPublish object
     * @param i the number of the object (0 = local, 1 = ftp, 2 = zip)
     */
    private void mount(Object data, int i) {
        ((DataAware) pubAware.get(i * 2)).setDataObject(data, true);
        ((DataAware) pubAware.get(i * 2 + 1)).setDataObject(data, true);
    }

    /**
     * Fills the documents listbox.
     * @param root the CGContent object
     * that contains the documents (a ListModel)
     */
    private void mount(CGContent root) {
        ListModelBinder.fillList(lstDocuments, root.cp_Documents.items(), null);
        docsBinder.setListModel(root.cp_Documents);
        disableDocUpDown();

    }

    /**
     * changes the DataAwares Objects' (in
     * the gioen list) DataObject to the
     * @param data
     * @param list
     */
    protected void mount(Object data, List list) {
        for (int i = 0; i < list.size(); i++)
             ((DataAware) list.get(i)).setDataObject(data, true);
    }

    /**
     * Checks if the document specified by the given CGDocument
     * object (cp_URL) exists. Calls also the validate(...) method
     * of this document.
     * If the document does not exist, it asks the user whether
     * to specify a new URL, or forget about this document.
     * @param doc the document to check.
     * @param task while loading a session, a status is
     * displayed, using the Task object to monitor progress.
     * @return true if the document is ok (a file exists in the given url).
     */
    protected boolean checkDocument(CGDocument doc, Task task) {
        try {
            doc.validate(xMSF, task);
            return true;
        } catch (FileNotFoundException ex) {

            int relocate = SystemDialog.showMessageBox(xMSF, "warningbox", VclWindowPeerAttribute.YES_NO + VclWindowPeerAttribute.DEF_NO, getFileAccess().getPath(doc.cp_URL,"") + "\n \n File not found. Would you like to specify a new file location?");

            if (relocate == 2) {
                String[] file = getDocAddDialog().callOpenDialog(false, FileAccess.getParentDir(doc.cp_URL));
                if (file == null)
                    return false;
                else {
                    doc.cp_URL = file[0];
                    return checkDocument(doc, task);
                }
            } else
                return false;
        }
        catch (IllegalArgumentException iaex) {
            //file is a directory
            AbstractErrorHandler.showMessage(xMSF,xControl.getPeer(),
                JavaTools.replaceSubString(resources.resErrIsDirectory,
                    getFileAccess().getPath(doc.cp_URL,""),
                    "%FILENAME"),
                ErrorHandler.ERROR_PROCESS_FATAL);
            return false;
        }
        catch (Exception exp) {
            //something went wrong.
            exp.printStackTrace();
            AbstractErrorHandler.showMessage(xMSF,xControl.getPeer(),
            JavaTools.replaceSubString(resources.resErrDocValidate,
                getFileAccess().getPath(doc.cp_URL,""),
                "%FILENAME"),ErrorHandler.ERROR_PROCESS_FATAL);
            return false;
        }

    }

    /**
     * Checks the documents contained in this content.
     * @param content
     * @param task
     */
    private void checkContent(CGContent content, Task task) {
        for (int i = 0; i < content.cp_Documents.getSize(); i++)
            if (!checkDocument((CGDocument) content.cp_Documents.getElementAt(i), task))
                // I use here 'i--' since, when the document is removed
                // an index change accures
                content.cp_Documents.remove(i--);
        /*for (Iterator i = content.cp_Contents.childrenMap.values().iterator(); i.hasNext();)
          checkContent((CGContent)i.next(),task);*/
    }

    /**
     * A Listener which is called whenever
     * a Publish checkbox/textbox state changes, and
     * changes the "create" button enable state accordingly.
     * @author rpiterman
     */
    private class CheckPublish implements DataAware.Listener {
        public void eventPerformed(Object event) {
            checkPublish();
        }
    }

    /**
     * Disables/enables the docUpDown buttons (step 2)
     * according to the currently selected document
     * (no doc selected - both disbaled, last doc selected,
     * down disabled and so on...)
     *
     */
    protected void disableDocUpDown() {
        try {
            setEnabled(btnDocUp, selectedDoc.length == 0 ? Boolean.FALSE : (selectedDoc[0] == 0 ? Boolean.FALSE : Boolean.TRUE));
            setEnabled(btnDocDown, selectedDoc.length == 0 ? Boolean.FALSE : (selectedDoc[0] + 1 < settings.cp_DefaultSession.cp_Content.cp_Documents.getSize() ? Boolean.TRUE : Boolean.FALSE));
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * refreshes the style preview.
     */
    public void refreshStylePreview() {
        try {
            stylePreview.refresh(settings.cp_DefaultSession.getStyle(), settings.cp_DefaultSession.cp_Design.cp_BackgroundImage);
            dpStylePreview.reload(xMSF);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }


}