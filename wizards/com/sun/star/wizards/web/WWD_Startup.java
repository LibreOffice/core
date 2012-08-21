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
package com.sun.star.wizards.web;

//import com.sun.star.awt.ItemEvent;
//import com.sun.star.awt.XItemListener;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XItemListener;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.ConfigSet;
import com.sun.star.wizards.common.Configuration;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.common.SystemDialog;
import com.sun.star.wizards.document.OfficeDocument;
import com.sun.star.wizards.ui.DocumentPreview;
import com.sun.star.wizards.ui.event.DataAware;
import com.sun.star.wizards.ui.event.ListModelBinder;
import com.sun.star.wizards.ui.event.RadioDataAware;
import com.sun.star.wizards.ui.event.SimpleDataAware;
import com.sun.star.wizards.ui.event.Task;
import com.sun.star.wizards.ui.event.UnoDataAware;
import com.sun.star.wizards.web.data.CGContent;
import com.sun.star.wizards.web.data.CGDocument;
import com.sun.star.wizards.web.data.CGIconSet;
import com.sun.star.wizards.web.data.CGPublish;
import com.sun.star.wizards.web.data.CGSession;
import com.sun.star.wizards.web.data.CGSessionName;
import com.sun.star.wizards.web.data.CGSettings;
import com.sun.star.wizards.web.data.CGStyle;

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
public abstract class WWD_Startup extends WWD_General
{

    SimpleDataAware sda = null;
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
    protected List<DataAware> docAware = new ArrayList<DataAware>();
    /**
     * The Vector containing DataAware objects
     * which relay on the session's generalInfo
     * Object (CGSession.cp_GeneralInfo) as
     * DataObject (or model).
     */
    protected List<DataAware> genAware = new ArrayList<DataAware>();
    /**
     * The vector containing DataAware objectscm25npd ..
     * which relay on the session's Design Object
     * (CGSession.cp_Design) as DataObject
     * (or model).
     *
     */
    protected List<DataAware> designAware = new ArrayList<DataAware>();
    /**
     * A Vector containig the DataAware objects
     * which relay on Publishing Objects.
     * Differently from other Vectors, each
     * DataAware object in this Vector relays
     * on a <b>different</b> CGPublish object,
     * So they are handled with more care.
     */
    protected List<DataAware> pubAware = new ArrayList<DataAware>(3);
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
    /**
     * If ftp proxies are on, ftp is disabled, and
     * the true/false of the FTP publisher is set to false.
     * In order to save it correctly when saving the session
     * at the end, the original loaded value is saved to this variable;
     */
    boolean __ftp;
    /**
     * When the wizard starts, a new document opens.
     * The backgroundDoc memeber contains the TextDocument
     * instance used for that purpose.
     */
    protected XFrame myFrame;


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
    public WWD_Startup(XMultiServiceFactory xmsf) throws Exception
    {
        super(xmsf);

        proxies = getOOProxies();

        String soTemplateDir = FileAccess.getOfficePath(xmsf, "Template", "share", "/wizard");

        String exclamationURL = FileAccess.connectURLs(soTemplateDir, "../wizard/bitmap/caution_16.png");
        this.drawNaviBar();
        this.buildStep1();
        this.buildStep2();
        this.buildStep3();

        this.buildStep4();
        this.buildStep5();
        this.buildStep6();
        this.buildStep7(proxies, exclamationURL);
        buildStepX();

        this.xMSF = xmsf;
        XDesktop xDesktop = Desktop.getDesktop(xMSF);
        myFrame = OfficeDocument.createNewFrame(xMSF, this);
        Object doc = OfficeDocument.createNewDocument(myFrame, "swriter", false, true);

        loadSettings(doc);
        setSaveSessionName(settings.cp_DefaultSession);

        ilLayouts.setListModel(settings.cp_Layouts);
        ilLayouts.create(this);

        checkContent(settings.cp_DefaultSession.cp_Content, new Task(PropertyNames.EMPTY_STRING, PropertyNames.EMPTY_STRING, 99999), this.xControl);

        //saved sessions, styles, combobox save session.
        // also set the chosen saved session...
        fillLists();
        makeDataAware();
        // change the display to correspond to the current session.
        updateUI();
        // fill the documents listbox.
        mount(settings.cp_DefaultSession.cp_Content);


        if (proxies)
        {
            setEnabled(btnFTP, false);
            setEnabled(chkFTP, false);
        }
    }

    /**
     * return true if http proxies or other proxies
     * which do not enable ftp to function correctly are set.
     * @return true if (http) proxies are on.
     * @throws Exception
     */
    private boolean getOOProxies() throws Exception
    {
        Object node = Configuration.getConfigurationRoot(xMSF, "org.openoffice.Inet/Settings", false);
        int i = Configuration.getInt("ooInetProxyType", node);
        //System.out.println("WWD:Startup:getOOProxies:" + i);
        switch (i)
        {
            case 0: //no proxies
                return false;
            case 2: //http proxies
                return true;
            default:
                return true;
        }
    }

    /**
     * calculates the first available session name,
     * and sets the current session name to it.
     * The combobox text in step 7 will be updated
     * automatically when updateUI() is called.
     */
    protected void setSaveSessionName(CGSession session)
    {
        int max = 0;
        int len = resources.resSessionName.length();
        // traverse between the sessions and find the one that
        // has the biggest number.
        for (int i = 0; i < settings.cp_SavedSessions.getSize(); i++)
        {
            String sessionName = ((CGSessionName) settings.cp_SavedSessions.getElementAt(i)).cp_Name;
            if (sessionName.startsWith(resources.resSessionName))
            {
                max = max(max, Integer.valueOf(sessionName.substring(len)).intValue());
            }
        }

        session.cp_Name = resources.resSessionName + ++max;

    }

    private int max(int i1, int i2)
    {
        if (i1 > i2)
        {
            return i1;
        }
        else
        {
            return i2;
        }
    }

    /**
     * fills the road map, and sets the necessary properties,
     * like MaxStep, Complete, Interactive-
     * Disables the finbihButton.
     */
    private void addRoadMapItems()
    {
        insertRoadMapItems(new String[]
                {
                    resources.resStep1, resources.resStep2, resources.resStep3, resources.resStep4, resources.resStep5, resources.resStep6, resources.resStep7
                }, new int[]
                {
                    1, 2, 3, 4, 5, 6, 7
                }, new boolean[]
                {
                    true, true, false, false, false, false, false
                });

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
    private void updateUI()
    {
        DataAware.updateUI(designAware);
        DataAware.updateUI(genAware);
        DataAware.updateUI(pubAware);
        sessionNameDA.updateUI();
        checkPublish();
    }

    private XFrame getFrame(Object model)
    {
        XModel xmodel = UnoRuntime.queryInterface(XModel.class, model);
        return xmodel.getCurrentController().getFrame();
    }

    /**
     * create the peer, add roadmap,
     * add roadmap items, add style-preview,
     * disable the steps 3 to 7 if no documents are
     * on the list, and... show the dialog!
     */
    public void show()
    {
        try
        {


            /* myFrame.initialize(docWindow);
             * */

            //desktopFrame = Desktop.findAFrame(xMSF, myFrame, desktopFrame);

            //XWindow xContainerWindow = myFrame.getContainerWindow();

            XWindow xContainerWindow = myFrame.getComponentWindow();
            XWindowPeer xWindowPeer = UnoRuntime.queryInterface(XWindowPeer.class, xContainerWindow);

            createWindowPeer(xWindowPeer);

            addRoadmap();
            addRoadMapItems();
            addStylePreview();
            checkSteps();

            executeDialog(myFrame);
            removeTerminateListener();

        }
        catch (java.lang.Exception jexception)
        {
            jexception.printStackTrace(System.err);
        }
    }

    /**
     * initializes the style preview.
     */
    private void addStylePreview()
    {
        try
        {
            dpStylePreview = new DocumentPreview(xMSF, imgPreview);
            stylePreview = new StylePreview(xMSF, settings.workPath);
            stylePreview.refresh(settings.cp_DefaultSession.getStyle(), settings.cp_DefaultSession.cp_Design.cp_BackgroundImage);
            dpStylePreview.setDocument(stylePreview.htmlFilename, DocumentPreview.PREVIEW_MODE);

        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }

    /**
     * Loads the web wizard settings from the registry.
     */
    private void loadSettings(Object document)
    {
        try
        {
            // instanciate
            String[] settingsResources = new String[]
            {
                resources.resPages,
                resources.resSlides,
                resources.resCreatedTemplate,
                resources.resUpdatedTemplate,
                resources.resSizeTemplate
            };

            settings = new CGSettings(xMSF, settingsResources, document);

            // get configuration view
            Object confRoot = Configuration.getConfigurationRoot(xMSF, CONFIG_PATH, false);
            // read
            settings.readConfiguration(confRoot, CONFIG_READ_PARAM);

            ConfigSet set = settings.cp_DefaultSession.cp_Publishing;

            // now if path variables are used in publisher paths, they
            // are getting replaced here...
            for (int i = 0; i < set.getSize(); i++)
            {
                CGPublish p = (CGPublish) set.getElementAt(i);
                p.cp_URL = substitute(p.cp_URL);
            }

            // initialize the settings.
            settings.configure(xMSF);

            // set resource needed for web page.

            // sort the styles alphabetically
            settings.cp_Styles.sort(new StylesComparator());

            prepareSessionLists();

            if (proxies)
            {
                __ftp = getPublisher(FTP_PUBLISHER).cp_Publish;
                getPublisher(FTP_PUBLISHER).cp_Publish = false;

            }

        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }

    protected void prepareSessionLists()
    {
        // now copy the sessions list...
        Object[] sessions = settings.cp_SavedSessions.items();
        settings.savedSessions.clear();
        for (int i = 0; i < sessions.length; i++)
        {
            settings.savedSessions.add(i, sessions[i]);        // add an empty session to the saved session list which apears in step 1
        }
        CGSessionName sn = new CGSessionName();
        sn.cp_Name = resources.resSessionNameNone;
        settings.cp_SavedSessions.add(0, sn);
    }

    /**
     * fills the saved session list, the styles list,
     * and save session combo box.
     * Also set the selected "load" session to the last session
     * which was saved.
     */
    private void fillLists()
    {
        // fill the saved session list.
        ListModelBinder.fillList(lstLoadSettings, settings.cp_SavedSessions.items(), null);

        // set the selected session to load. (step 1)
        selectSession();

        // fill the styles list.
        ListModelBinder.fillList(lstStyles, settings.cp_Styles.items(), null);

        // fill the save session combobox (step 7)
        ListModelBinder.fillComboBox(cbSaveSettings, settings.savedSessions.items(), null);


    }

    protected void selectSession()
    {
        int selectedSession = 0;
        if (settings.cp_LastSavedSession != null && !settings.cp_LastSavedSession.equals(PropertyNames.EMPTY_STRING))
        {

            Object ses = settings.cp_SavedSessions.getElement(settings.cp_LastSavedSession);
            if (ses != null)
            {
                selectedSession = settings.cp_SavedSessions.getIndexOf(ses);
            }
        }
        Helper.setUnoPropertyValue(getModel(lstLoadSettings), PropertyNames.SELECTED_ITEMS, new short[]
                {
                    (short) selectedSession
                });

    }

    public class SimpleDataawareUpdater implements XItemListener
    {
        /* (non-Javadoc)
         * @see com.sun.star.lang.XEventListener#disposing(com.sun.star.lang.EventObject)
         */

        public void disposing(EventObject arg0)
        {
            // TODO Auto-generated method stub
        }

        public void itemStateChanged(com.sun.star.awt.ItemEvent itemEvent)
        {
            sda.updateData();
        //TODO xf uncomment
        //refresh.eventPerformed(ie);
        }
    }

    /**
     * attaches to each ui-data-control (like checkbox, groupbox or
     * textbox, no buttons though), a DataObject's JavaBean Property,
     * or class member.
     */
    private void makeDataAware()
    {
        //page 1
        new ListModelBinder(lstLoadSettings, settings.cp_SavedSessions);

        //page 2 : document properties

        docListDA = UnoDataAware.attachListBox(this, "SelectedDoc", lstDocuments, null, false);
        docListDA.disableControls(new Object[]
                {
                    /*btnDocDown, btnDocUp, */
                    lnDocsInfo, btnRemoveDoc, lblDocTitle, txtDocTitle, lblDocInfo, txtDocInfo, lblDocAuthor, txtDocAuthor, lblDocExportFormat, lstDocTargetType
                });
        docListDA.updateUI();

        CGDocument doc = new CGDocument(); //dummy

        docsBinder = new ListModelBinder(lstDocuments, settings.cp_DefaultSession.cp_Content.cp_Documents);

        docAware.add(UnoDataAware.attachEditControl(doc, "cp_Title", txtDocTitle, refresh, true));
        docAware.add(UnoDataAware.attachEditControl(doc, "cp_Description", txtDocInfo, refresh, true));
        docAware.add(UnoDataAware.attachEditControl(doc, "cp_Author", txtDocAuthor, refresh, true));
        docAware.add(UnoDataAware.attachListBox(doc, "Exporter", lstDocTargetType, refresh, false));

        //page 3 : Layout
        Object design = settings.cp_DefaultSession.cp_Design;
        sda = new SimpleDataAware(design, new DataAware.PropertyValue("Layout", design), ilLayouts, new DataAware.PropertyValue("Selected", ilLayouts));
        ilLayouts.addItemListener(new SimpleDataawareUpdater());
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
        designAware.add(RadioDataAware.attachRadioButtons(settings.cp_DefaultSession.cp_Design, "cp_OptimizeDisplaySize", new Object[]
                {
                    optOptimize640x480, optOptimize800x600, optOptimize1024x768
                }, refresh, true));


        //page 5 : Style
        /*
         * note : on style change, i do not call here refresh ,but rather on
         * a special method which will perform some display, background and Iconsets changes.
         */
        designAware.add(UnoDataAware.attachListBox(settings.cp_DefaultSession.cp_Design, "Style", lstStyles, null, false));

        //page 6 : site general props
        genAware.add(UnoDataAware.attachEditControl(settings.cp_DefaultSession.cp_GeneralInfo, "cp_Title", txtSiteTitle, refresh, true));
        genAware.add(UnoDataAware.attachEditControl(settings.cp_DefaultSession.cp_GeneralInfo, "cp_Description", txtSiteDesc, refresh, true));

        genAware.add(UnoDataAware.attachDateControl(settings.cp_DefaultSession.cp_GeneralInfo, "CreationDate", dateSiteCreated, refresh, false));
        genAware.add(UnoDataAware.attachDateControl(settings.cp_DefaultSession.cp_GeneralInfo, "UpdateDate", dateSiteUpdate, refresh, false));
        genAware.add(UnoDataAware.attachEditControl(settings.cp_DefaultSession.cp_GeneralInfo, "cp_Email", txtEmail, refresh, true));
        genAware.add(UnoDataAware.attachEditControl(settings.cp_DefaultSession.cp_GeneralInfo, "cp_Copyright", txtCopyright, refresh, true));

        //page 7 : publishing

        pubAware(LOCAL_PUBLISHER, chkLocalDir, txtLocalDir, false);
        pubAware(FTP_PUBLISHER, chkFTP, lblFTP, true);
        pubAware(ZIP_PUBLISHER, chkZip, txtZip, false);

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
    private void pubAware(String publish, Object checkbox, Object textbox, boolean isLabel)
    {
        Object p = settings.cp_DefaultSession.cp_Publishing.getElement(publish);
        UnoDataAware uda = UnoDataAware.attachCheckBox(p, "cp_Publish", checkbox, checkPublish, true);
        uda.setInverse(true);

        uda.disableControls(new Object[]
                {
                    textbox
                });
        pubAware.add(uda);
        pubAware.add(
                isLabel ? UnoDataAware.attachLabel(p, PropertyNames.URL, textbox, checkPublish, false)
                : UnoDataAware.attachEditControl(p, PropertyNames.URL, textbox, checkPublish, false));

    }

    /*
     * Session load methods
     * (are used both on the start of the wizard and
     * when the user loads a session)
     */
    /**
     * Is called when a new session/settings is
     * loaded. <br/>
     * Checks the documents (validate), fills the
     * documents listbox, and changes the
     * DataAware data objects to the
     * new session's objects.<br/>
     * Task advances 4 times in the mount method,
     * and ??? times for each document in the session.
     *
     */
    protected void mount(CGSession session, Task task, boolean refreshStyle, XControl xC)
    {
        /* This checks the documents. If the user
         * chooses to cancel, the session is not loaded.
         */
        checkContent(session.cp_Content, task, xC);

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
        Helper.setUnoPropertyValue(getModel(chkSaveSettings), PropertyNames.PROPERTY_STATE, new Short((short) 1));
        docListDA.updateUI();

        task.advance(true);

        if (refreshStyle)
        {
            refreshStylePreview();
            updateIconsetText();
        }

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
    private void mount(Object data, int i)
    {
        pubAware.get(i * 2).setDataObject(data, true);
        pubAware.get(i * 2 + 1).setDataObject(data, true);
    }

    /**
     * Fills the documents listbox.
     * @param root the CGContent object
     * that contains the documents (a ListModel)
     */
    private void mount(CGContent root)
    {
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
    protected void mount(Object data, List<DataAware> list)
    {
        for (int i = 0; i < list.size(); i++)
        {
            list.get(i).setDataObject(data, true);
        }
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
    protected boolean checkDocument(CGDocument doc, Task task, XControl xC)
    {
        try
        {
            doc.validate(xMSF, task);
            return true;
        }
        catch (FileNotFoundException ex)
        {

            int relocate = SystemDialog.showMessageBox(xMSF, xC.getPeer(), "WarningBox", VclWindowPeerAttribute.YES_NO + VclWindowPeerAttribute.DEF_NO, getFileAccess().getPath(doc.cp_URL, PropertyNames.EMPTY_STRING) + "\n\n" + resources.resSpecifyNewFileLocation);

            if (relocate == 2)
            {
                String[] file = getDocAddDialog().callOpenDialog(false, FileAccess.getParentDir(doc.cp_URL));
                if (file == null)
                {
                    return false;
                }
                else
                {
                    doc.cp_URL = file[0];
                    return checkDocument(doc, task, xC);
                }
            }
            else
            {
                return false;
            }
        }
        catch (IllegalArgumentException iaex)
        {
            //file is a directory
            AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(),
                    JavaTools.replaceSubString(resources.resErrIsDirectory,
                    getFileAccess().getPath(doc.cp_URL, PropertyNames.EMPTY_STRING),
                    "%FILENAME"),
                    ErrorHandler.ERROR_PROCESS_FATAL);
            return false;
        }
        catch (Exception exp)
        {
            //something went wrong.
            exp.printStackTrace();
            AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(),
                    JavaTools.replaceSubString(resources.resErrDocValidate,
                    getFileAccess().getPath(doc.cp_URL, PropertyNames.EMPTY_STRING),
                    "%FILENAME"), ErrorHandler.ERROR_PROCESS_FATAL);
            return false;
        }

    }

    /**
     * Checks the documents contained in this content.
     * @param content
     * @param task
     */
    private void checkContent(CGContent content, Task task, XControl xC)
    {
        for (int i = 0; i < content.cp_Documents.getSize(); i++)
        {
            if (!checkDocument((CGDocument) content.cp_Documents.getElementAt(i), task, xC))
            // I use here 'i--' since, when the document is removed
            // an index change accures
            {
                content.cp_Documents.remove(i--);
            /*for (Iterator i = content.cp_Contents.childrenMap.values().iterator(); i.hasNext();)
            checkContent((CGContent)i.next(),task);*/
            }
        }
    }

    /**
     * A Listener which is called whenever
     * a Publish checkbox/textbox state changes, and
     * changes the "create" button enable state accordingly.
     * @author rpiterman
     */
    private class CheckPublish implements DataAware.Listener
    {

        public void eventPerformed(Object event)
        {
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
    protected void disableDocUpDown()
    {
        try
        {
            setEnabled(btnDocUp, selectedDoc.length == 0 ? Boolean.FALSE : (selectedDoc[0] == 0 ? Boolean.FALSE : Boolean.TRUE));
            setEnabled(btnDocDown, selectedDoc.length == 0 ? Boolean.FALSE : (selectedDoc[0] + 1 < settings.cp_DefaultSession.cp_Content.cp_Documents.getSize() ? Boolean.TRUE : Boolean.FALSE));
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    void updateBackgroundText()
    {
        String bg = settings.cp_DefaultSession.cp_Design.cp_BackgroundImage;
        if (bg == null || bg.equals(PropertyNames.EMPTY_STRING))
        {
            bg = resources.resBackgroundNone;
        }
        else
        {
            bg = FileAccess.getPathFilename(getFileAccess().getPath(bg, null));
        }
        Helper.setUnoPropertyValue(getModel(txtBackground), PropertyNames.PROPERTY_LABEL, bg);
    }

    void updateIconsetText()
    {
        String iconset = settings.cp_DefaultSession.cp_Design.cp_IconSet;
        String iconsetName;
        if (iconset == null || iconset.equals(PropertyNames.EMPTY_STRING))
        {
            iconsetName = resources.resIconsetNone;
        }
        else
        {
            CGIconSet is = (CGIconSet) settings.cp_IconSets.getElement(iconset);
            if (is == null)
            {
                iconsetName = resources.resIconsetNone;
            }
            else
            {
                iconsetName = is.cp_Name;
            }
        }

        Helper.setUnoPropertyValue(getModel(txtIconset), PropertyNames.PROPERTY_LABEL, iconsetName);
    }

    /**
     * refreshes the style preview.
     * I also call here "updateBackgroundtext", because always
     * when the background is changed, this method
     * has to be called, so I am walking on the safe side here...
     */
    public void refreshStylePreview()
    {
        try
        {
            updateBackgroundText();
            stylePreview.refresh(settings.cp_DefaultSession.getStyle(), settings.cp_DefaultSession.cp_Design.cp_BackgroundImage);
            dpStylePreview.reload(xMSF);
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }

    private class StylesComparator implements Comparator<Object>
    {

        /* (non-Javadoc)
         * @see java.util.Comparator#compare(java.lang.Object, java.lang.Object)
         */
        public int compare(Object o1, Object o2)
        {
            // TODO Auto-generated method stub
            if (o1 instanceof CGStyle && o2 instanceof CGStyle)
            {
                return ((CGStyle) o1).cp_Name.compareTo(
                        ((CGStyle) o2).cp_Name);
            }
            else
            {
                throw new IllegalArgumentException("Cannot compare objects which are not CGStyle.");
            }
        }
    }
}
