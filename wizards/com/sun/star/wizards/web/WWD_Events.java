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

import javax.swing.ListModel;

import com.sun.star.awt.KeyEvent;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XKeyListener;
import com.sun.star.awt.XWindow;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCloseable;
import com.sun.star.wizards.common.Configuration;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.SystemDialog;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.ui.UnoDialog;
import com.sun.star.wizards.ui.event.ListModelBinder;
import com.sun.star.wizards.ui.event.Task;
import com.sun.star.wizards.web.data.CGDocument;
import com.sun.star.wizards.web.data.CGPublish;
import com.sun.star.wizards.web.data.CGSession;
import com.sun.star.wizards.web.data.CGSessionName;

/**
 * This class implements the ui-events of the
 * web wizard.
 * it is therfore sorted to steps.
 * not much application-logic here - just plain
 * methods which react to events.
 * The only exception are the finish methods with the save
 * session methods. 
 */
public abstract class WWD_Events extends WWD_Startup
{

    private static final short[] EMPTY_SHORT_ARRAY = new short[0];
    /**
     * Tracks the current loaded session.
     * If PropertyNames.EMPTY_STRING - it means the current session is the default one (empty)
     * If a session is loaded, this will be the name of the loaded session.
     */
    protected String currentSession = PropertyNames.EMPTY_STRING;

    /**
     * He - my constructor !
     * I add a window listener, which, when
     * the window closes, deltes the temp directory.
     */
    public WWD_Events(XMultiServiceFactory xmsf) throws Exception
    {
        super(xmsf);
        Create c = new Create();
        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, chkFTP);
        xWindow.addKeyListener(c);
        xWindow = UnoRuntime.queryInterface(XWindow.class, chkLocalDir);
        xWindow.addKeyListener(c);
        xWindow = UnoRuntime.queryInterface(XWindow.class, chkZip);
        xWindow.addKeyListener(c);
    }

    /* *********************************************************
     *  *******************************************************
     *          EVENT and UI METHODS
     *  *******************************************************
     * *********************************************************/
    protected void leaveStep(int nOldStep, int nNewStep)
    {
        if (nOldStep == 1 && nNewStep == 2)
        {
            // 1. check if the selected session is the same as the current one.
        }
    }

    protected void enterStep(int old, int newStep)
    {
        if ((old == 1))
        {
            String sessionToLoad = PropertyNames.EMPTY_STRING;
            short[] s = (short[]) Helper.getUnoPropertyValue(getModel(lstLoadSettings), PropertyNames.SELECTED_ITEMS);
            if (s.length == 0 || s[0] == 0)
            {
                sessionToLoad = PropertyNames.EMPTY_STRING;
            }
            else
            {
                sessionToLoad = ((CGSessionName) settings.cp_SavedSessions.getElementAt(s[0])).cp_Name;
            }
            if (!sessionToLoad.equals(currentSession))
            {
                loadSession(sessionToLoad);
            }
        }
        if (newStep == 5)
        {
        }
    }

    /* ********************************* 
     *  STEP 1
     */
    /**
     * Called from the Uno event dispatcher when the
     * user selects a saved session.
     */
    public void sessionSelected()
    {
        short[] s = (short[]) Helper.getUnoPropertyValue(getModel(lstLoadSettings), PropertyNames.SELECTED_ITEMS);
        setEnabled(btnDelSession, s.length > 0 && s[0] > 0);
    }

    /**
     * Ha ! the session should be loaded :-)
     */
    public void loadSession(final String sessionToLoad)
    {
        try
        {
            final StatusDialog sd = getStatusDialog();

            final Task task = new Task("LoadDocs", PropertyNames.EMPTY_STRING, 10);

            sd.execute(this, task, resources.resLoadingSession);
            task.start();

            setSelectedDoc(EMPTY_SHORT_ARRAY);
            Helper.setUnoPropertyValue(getModel(lstDocuments), PropertyNames.SELECTED_ITEMS, EMPTY_SHORT_ARRAY);
            Helper.setUnoPropertyValue(getModel(lstDocuments), PropertyNames.STRING_ITEM_LIST, EMPTY_STRING_ARRAY);

            Object view = null;

            if (sessionToLoad.equals(PropertyNames.EMPTY_STRING))
            {
                view = Configuration.getConfigurationRoot(xMSF, CONFIG_PATH + "/DefaultSession", false);
            }
            else
            {
                view = Configuration.getConfigurationRoot(xMSF, CONFIG_PATH + "/SavedSessions", false);
                view = Configuration.getNode(sessionToLoad, view);
            }

            CGSession session = new CGSession();
            session.setRoot(settings);
            session.readConfiguration(view, CONFIG_READ_PARAM);
            task.setMax(session.cp_Content.cp_Documents.getSize() * 5 + 7);
            task.advance(true);

            if (sessionToLoad.equals(PropertyNames.EMPTY_STRING))
            {
                setSaveSessionName(session);
            }
            mount(session, task, false, sd.xControl);

            checkSteps();
            currentSession = sessionToLoad;

            while (task.getStatus() <= task.getMax())
            {
                task.advance(false);
            }
            task.removeTaskListener(sd);
        }
        catch (Exception ex)
        {
            unexpectedError(ex);
        }

        try
        {
            refreshStylePreview();
            updateIconsetText();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    /**
     * hmm. the user clicked the delete button.
     */
    public void delSession()
    {
        short[] selected = (short[]) Helper.getUnoPropertyValue(getModel(lstLoadSettings), PropertyNames.SELECTED_ITEMS);
        if (selected.length == 0)
        {
            return;
        }
        if (selected[0] == 0)
        {
            return;
        }
        boolean confirm = AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(), resources.resDelSessionConfirm, ErrorHandler.ERROR_QUESTION_NO);
        if (confirm)
        {
            try
            {
                String name = (String) settings.cp_SavedSessions.getKey(selected[0]);
                // first delete the session from the registry/configuration.

                Configuration.removeNode(xMSF, CONFIG_PATH + "/SavedSessions", name);

                // then delete the session from the java-set (settings.cp_SavedSessions)
                settings.cp_SavedSessions.remove(selected[0]);
                settings.savedSessions.remove(selected[0] - 1);

                short[] nextSelected = new short[]
                {
                    (short) 0
                };
                // We try to select the same item index again, if possible 
                if (settings.cp_SavedSessions.getSize() > selected[0])
                {
                    nextSelected[0] = selected[0];
                }
                else
                // this will always be available because
                // the user can not remove item 0.
                {
                    nextSelected[0] = (short) (selected[0] - 1);                // if the <none> session will be selected, disable the remove button...
                }
                if (nextSelected[0] == 0)
                {
                    Helper.setUnoPropertyValue(getModel(btnDelSession), PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);                // select...
                }
                Helper.setUnoPropertyValue(getModel(lstLoadSettings), PropertyNames.SELECTED_ITEMS, nextSelected);
            }
            catch (Exception ex)
            {
                ex.printStackTrace();
                unexpectedError(ex);
            }
        }
    }

    /* ********************************
     * STEP 2
     */
    /**
     * A method used by the UnoDataAware attached
     * to the Documents listbox.
     * See the concept of the DataAware objects to undestand
     * why it is there...
     */
    public short[] getSelectedDoc()
    {
        return selectedDoc;
    }
    private static String[] EMPTY_STRING_ARRAY = new String[0];

    /**
     * when the user clicks another document
     * in the listbox, this method is called,
     * and couses the display in
     * the textboxes title,description, author and export format 
     * to change
     */
    public void setSelectedDoc(short[] s)
    {
        CGDocument oldDoc = getDoc(selectedDoc);
        CGDocument doc = getDoc(s);

        if (doc == null)
        {
            fillExportList(EMPTY_STRING_ARRAY);
        //I try to avoid refreshing the export list if 
        //the same type of document is chosen.
        }
        else if (oldDoc == null || (!oldDoc.appType.equals(doc.appType)))
        {
            fillExportList(settings.getExporters(doc.appType));
        }


        selectedDoc = s;

        mount(doc, docAware);
        disableDocUpDown();
    }

    /**
     * The user clicks the "Add" button.
     * This will open a "FileOpen" dialog,
     * and, if the user chooses more than one file,
     * will open a status dialog, when validating each document.
     */
    public void addDocument()
    {

        final String[] files = getDocAddDialog().callOpenDialog(true, settings.cp_DefaultSession.cp_InDirectory);
        if (files == null)
        {
            return;
        }
        final Task task = new Task(PropertyNames.EMPTY_STRING, PropertyNames.EMPTY_STRING, files.length * 5);

        /*
         * If more than a certain number
         * of documents have been added,
         * open the status dialog.
         */
        if (files.length > MIN_ADD_FILES_FOR_DIALOG)
        {
            StatusDialog sd = getStatusDialog();
            sd.setLabel(resources.resValidatingDocuments);
            sd.execute(this, task, resources.prodName);
            LoadDocs oLoadDocs = new LoadDocs(this.xControl, files, task);
            oLoadDocs.loadDocuments();
            task.removeTaskListener(sd);
        }
        /*
         * When adding a single document, do not use a 
         * status dialog... 
         */
        else
        {
            LoadDocs oLoadDocs = new LoadDocs(this.xControl, files, task);
            oLoadDocs.loadDocuments();
        }

    }

    /**
     * The user clicked delete.
     */
    public void removeDocument()
    {
        if (selectedDoc.length == 0)
        {
            return;
        }
        settings.cp_DefaultSession.cp_Content.cp_Documents.remove(selectedDoc[0]);

        // update the selected document
        while (selectedDoc[0] >= getDocsCount())
        {
            selectedDoc[0]--;        // if there are no documents...
        }
        if (selectedDoc[0] == -1)
        {
            selectedDoc = EMPTY_SHORT_ARRAY;        // update the list to show the right selection.
        }
        docListDA.updateUI();
        // disables all the next steps, if the list of docuemnts
        // is empty.
        checkSteps();
    }

    /**
     * doc up.
     */
    public void docUp()
    {
        Object doc = settings.cp_DefaultSession.cp_Content.cp_Documents.getElementAt(selectedDoc[0]);
        settings.cp_DefaultSession.cp_Content.cp_Documents.remove(selectedDoc[0]);
        settings.cp_DefaultSession.cp_Content.cp_Documents.add(--selectedDoc[0], doc);
        docListDA.updateUI();
        disableDocUpDown();
    }

    /**
     * doc down
     */
    public void docDown()
    {
        Object doc = settings.cp_DefaultSession.cp_Content.cp_Documents.getElementAt(selectedDoc[0]);
        settings.cp_DefaultSession.cp_Content.cp_Documents.remove(selectedDoc[0]);
        settings.cp_DefaultSession.cp_Content.cp_Documents.add(++selectedDoc[0], doc);
        docListDA.updateUI();
        disableDocUpDown();
    }

    /* ******************************
     * STEP 5
     */
    /**
     * invoked when the user clicks "Choose backgrounds" button.
     */
    private ImageListDialog bgDialog;

    /**
     * the user clicked the "backgrounds" button
     */
    public void chooseBackground()
    {
        try
        {
            setEnabled(btnBackgrounds, false);
            if (bgDialog == null)
            {
                bgDialog = new BackgroundsDialog(xMSF, settings.cp_BackgroundImages, resources);
                bgDialog.createWindowPeer(xControl.getPeer());
            }
            bgDialog.setSelected(settings.cp_DefaultSession.cp_Design.cp_BackgroundImage);
            short i = bgDialog.executeDialog((UnoDialog) WWD_Events.this);
            if (i == 1) //ok
            {
                setBackground(bgDialog.getSelected());
            }
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
        finally
        {
            setEnabled(btnBackgrounds, true);
        }
    }

    /**
     * invoked when the BackgorundsDialog is "OKed".
     */
    public void setBackground(Object background)
    {
        if (background == null)
        {
            background = PropertyNames.EMPTY_STRING;
        }
        settings.cp_DefaultSession.cp_Design.cp_BackgroundImage = (String) background;
        refreshStylePreview();
    }
    private IconsDialog iconsDialog;

    /**
     * is called when the user clicks "Icon sets" button.
     *
     */
    public void chooseIconset()
    {
        try
        {
            setEnabled(btnIconSets, false);
            if (iconsDialog == null)
            {
                iconsDialog = new IconsDialog(xMSF, settings.cp_IconSets, resources);
                iconsDialog.createWindowPeer(xControl.getPeer());
            }

            iconsDialog.setIconset(settings.cp_DefaultSession.cp_Design.cp_IconSet);

            short i = iconsDialog.executeDialog((UnoDialog) WWD_Events.this);
            if (i == 1) //ok
            {
                setIconset(iconsDialog.getIconset());
            }
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
        finally
        {
            setEnabled(btnIconSets, true);
        }
    }

    /**
     * invoked when the Iconsets Dialog is OKed.
     */
    public void setIconset(String icon)
    {
        settings.cp_DefaultSession.cp_Design.cp_IconSet = icon;
        updateIconsetText();
    }

    /* ******************************
     * STEP 7
     */
    /**
     * sets the publishing url of either a local/zip or ftp publisher.
     * updates the ui....
     */
    private CGPublish setPublishUrl(String publisher, String url, int number)
    {
        if (url == null)
        {
            return null;
        }
        CGPublish p = getPublisher(publisher);
        p.cp_URL = url;
        p.cp_Publish = true;
        updatePublishUI(number);
        p.overwriteApproved = true;
        return p;
    }

    /**
     * updates the ui of a certain publisher 
     * (the text box url)
     * @param number
     */
    private void updatePublishUI(int number)
    {
        pubAware.get(number).updateUI();
        pubAware.get(number + 1).updateUI();
        checkPublish();
    }

    /**
     * The user clicks the local "..." button.
     *
     */
    public void setPublishLocalDir()
    {
        String dir = showFolderDialog("Local destination directory", PropertyNames.EMPTY_STRING, settings.cp_DefaultSession.cp_OutDirectory);
        //if ok was pressed...
        setPublishUrl(LOCAL_PUBLISHER, dir, 0);

    }

    /**
     * The user clicks the "Configure" FTP button.
     *
     */
    public void setFTPPublish()
    {
        if (showFTPDialog(getPublisher(FTP_PUBLISHER)))
        {
            getPublisher(FTP_PUBLISHER).cp_Publish = true;
            updatePublishUI(2);
        }
    }

    /**
     * show the ftp dialog
     * @param pub
     * @return true if OK was pressed, otherwise false.
     */
    private boolean showFTPDialog(CGPublish pub)
    {
        try
        {
            return getFTPDialog(pub).execute(this) == 1;
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
            return false;
        }
    }

    /**
     * the user clicks the zip "..." button.
     * Choose a zip file...
     */
    public void setZipFilename()
    {
        SystemDialog sd = getZipDialog();
        String zipFile = sd.callStoreDialog(settings.cp_DefaultSession.cp_OutDirectory, resources.resDefaultArchiveFilename);
        setPublishUrl(ZIP_PUBLISHER, zipFile, 4);
        getPublisher(ZIP_PUBLISHER).overwriteApproved = true;
    }
    private TOCPreview docPreview;

    /**
     * the user clicks the "Preview" button.
     */
    public void documentPreview()
    {
        try
        {
            if (docPreview == null)
            {
                docPreview = new TOCPreview(xMSF, settings, resources, stylePreview.tempDir, myFrame);
            }
            docPreview.refresh(settings);
        }
        catch (Exception ex)
        {
            unexpectedError(ex);
        }
    }

    /* **********************
     * FINISH
     */
    /**
     * This method checks if the given target's path, added the pathExtension argument,
     * exists, and asks the user what to do about it.
     * If the user says its all fine, then the target will
     * be replaced.
     * @return true if "create" should continue. false if "create" should abort.
     */
    private boolean publishTargetApproved()
    {
        boolean result = true;
        // 1. check local publish target

        CGPublish p = getPublisher(LOCAL_PUBLISHER);

        // should publish ?
        if (p.cp_Publish)
        {
            String path = getFileAccess().getPath(p.url, null);
            // target exists? 
            if (getFileAccess().exists(p.url, false))
            {
                //if its a directory
                if (getFileAccess().isDirectory(p.url))
                {
                    //check if its empty
                    String[] files = getFileAccess().listFiles(p.url, true);
                    if (files.length > 0)
                    {
                        /* it is not empty :-(
                         * it either a local publisher or an ftp (zip uses no directories
                         * as target...)
                         */
                        String message = JavaTools.replaceSubString(resources.resLocalTragetNotEmpty,
                                path, "%FILENAME");
                        result = AbstractErrorHandler.showMessage(
                                xMSF, xControl.getPeer(), message,
                                ErrorHandler.MESSAGE_WARNING, ErrorHandler.BUTTONS_YES_NO,
                                ErrorHandler.DEF_NO, ErrorHandler.RESULT_YES);

                        if (!result)
                        {
                            return result;
                        }
                    }
                }
                else
                {//not a directory, but still exists 
                    String message = JavaTools.replaceSubString(resources.resLocalTargetExistsAsfile,
                            path, "%FILENAME");
                    AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(), message, ErrorHandler.ERROR_PROCESS_FATAL);
                    return false;
                }

            // try to write to the path...
            }
            else
            {
                // the local target directory does not exist.
                String message = JavaTools.replaceSubString(resources.resLocalTargetCreate,
                        path, "%FILENAME");
                try
                {
                    result = AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(), message,
                            ErrorHandler.ERROR_QUESTION_YES);
                }
                catch (Exception ex)
                {
                    ex.printStackTrace();
                }

                if (!result)
                {
                    return result;
                // try to create the directory...
                }
                try
                {
                    getFileAccess().fileAccess.createFolder(p.cp_URL);
                }
                catch (Exception ex)
                {
                    message = JavaTools.replaceSubString(resources.resLocalTargetCouldNotCreate,
                            path, "%FILENAME");
                    AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(), message,
                            ErrorHandler.ERROR_PROCESS_FATAL);
                    return false;
                }
            }
        }

        // 2. Check ZIP
        // should publish ?
        p = getPublisher(ZIP_PUBLISHER);

        if (p.cp_Publish)
        {

            String path = getFileAccess().getPath(p.cp_URL, null);
            // target exists? 
            if (getFileAccess().exists(p.cp_URL, false))
            {
                //if its a directory
                if (getFileAccess().isDirectory(p.cp_URL))
                {
                    String message = JavaTools.replaceSubString(resources.resZipTargetIsDir,
                            path, "%FILENAME");
                    AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(), message,
                            ErrorHandler.ERROR_PROCESS_FATAL);
                    return false;
                }
                else
                {//not a directory, but still exists ( a file...) 
                    if (!p.overwriteApproved)
                    {
                        String message = JavaTools.replaceSubString(resources.resZipTargetExists,
                                path, "%FILENAME");
                        result = AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(), message,
                                ErrorHandler.ERROR_QUESTION_YES);
                        if (!result)
                        {
                            return false;
                        }
                    }
                }
            }
        }

        // 3. check FTP
        p = getPublisher(FTP_PUBLISHER);

        // should publish ?
        if (p.cp_Publish)
        {

            String path = getFileAccess().getPath(p.cp_URL, null);

            // target exists? 
            if (getFileAccess().exists(p.url, false))
            {
                //if its a directory
                if (getFileAccess().isDirectory(p.url))
                {
                    //check if its empty
                    String[] files = getFileAccess().listFiles(p.url, true);
                    if (files.length > 0)
                    {
                        /* it is not empty :-(
                         * it either a local publisher or an ftp (zip uses no directories
                         * as target...)
                         */
                        String message = JavaTools.replaceSubString(resources.resFTPTargetNotEmpty,
                                path, "%FILENAME");
                        result = AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(), message,
                                ErrorHandler.ERROR_QUESTION_CANCEL);
                        if (!result)
                        {
                            return result;
                        }
                    }
                }
                else
                {//not a directory, but still exists (as a file)
                    String message = JavaTools.replaceSubString(resources.resFTPTargetExistsAsfile,
                            path, "%FILENAME");
                    AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(), message,
                            ErrorHandler.ERROR_PROCESS_FATAL);
                    return false;
                }

            // try to write to the path...
            }
            else
            {
                // the ftp target directory does not exist.
                String message = JavaTools.replaceSubString(resources.resFTPTargetCreate,
                        path, "%FILENAME");
                result = AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(), message,
                        ErrorHandler.ERROR_QUESTION_YES);
                if (!result)
                {
                    return result;
                // try to create the directory...
                }
                try
                {
                    getFileAccess().fileAccess.createFolder(p.url);
                }
                catch (Exception ex)
                {
                    message = JavaTools.replaceSubString(resources.resFTPTargetCouldNotCreate,
                            path, "%FILENAME");
                    AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(), message,
                            ErrorHandler.ERROR_PROCESS_FATAL);
                    return false;
                }
            }
        }
        return true;
    }

    /*
     * return false if "create" should be aborted. true if everything is fine.
     */
    private boolean saveSession()
    {
        try
        {
            Object node = null;
            String name = getSessionSaveName();

            //set documents index field.
            ListModel docs = settings.cp_DefaultSession.cp_Content.cp_Documents;

            for (int i = 0; i < docs.getSize(); i++)
            {
                ((CGDocument) docs.getElementAt(i)).cp_Index = i;
            }
            Object conf = Configuration.getConfigurationRoot(xMSF, CONFIG_PATH + "/SavedSessions", true);
            // first I check if a session with the given name exists
            try
            {
                node = Configuration.getNode(name, conf);
                if (node != null)
                {
                    if (!AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(),
                            JavaTools.replaceSubString(resources.resSessionExists, name, "${NAME}"),
                            ErrorHandler.ERROR_NORMAL_IGNORE))
                    {
                        return false;                    //remove the old session
                    }
                }
                Configuration.removeNode(conf, name);

            }
            catch (NoSuchElementException nsex)
            {
            }

            settings.cp_DefaultSession.cp_Index = 0;
            node = Configuration.addConfigNode(conf, name);
            settings.cp_DefaultSession.cp_Name = name;
            settings.cp_DefaultSession.writeConfiguration(node, CONFIG_READ_PARAM);
            settings.cp_SavedSessions.reindexSet(conf, name, "Index");
            Configuration.commit(conf);

            // now I reload the sessions to actualize the list/combo boxes load/save sessions.
            settings.cp_SavedSessions.clear();

            Object confView = Configuration.getConfigurationRoot(xMSF, CONFIG_PATH + "/SavedSessions", false);
            settings.cp_SavedSessions.readConfiguration(confView, CONFIG_READ_PARAM);

            settings.cp_LastSavedSession = name;
            currentSession = name;
            // now save the name of the last saved session...

            settings.cp_LastSavedSession = name;

            // TODO add the <none> session...
            prepareSessionLists();
            ListModelBinder.fillList(lstLoadSettings, settings.cp_SavedSessions.items(), null);
            ListModelBinder.fillComboBox(cbSaveSettings, settings.savedSessions.items(), null);
            selectSession();

            currentSession = settings.cp_LastSavedSession;

            return true;
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
            return false;
        }
    }

    private String targetStringFor(String publisher)
    {
        CGPublish p = getPublisher(publisher);
        if (p.cp_Publish)
        {
            return "\n" + getFileAccess().getPath(p.cp_URL, null);
        }
        else
        {
            return PropertyNames.EMPTY_STRING;
        }
    }

    /**
     * this method will be called when the Status Dialog 
     * is hidden. 
     * It checks if the "Process" was successfull, and if so,
     * it closes the wizard dialog. 
     */
    public void finishWizardFinished()
    {
        if (process.getResult())
        {
            String targets =
                    targetStringFor(LOCAL_PUBLISHER) +
                    targetStringFor(ZIP_PUBLISHER) +
                    targetStringFor(FTP_PUBLISHER);
            String message = JavaTools.replaceSubString(resources.resFinishedSuccess, targets, "%FILENAME");

            AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(), message, ErrorHandler.ERROR_MESSAGE);
            if (exitOnCreate)
            {
                this.xDialog.endExecute();
            }
        }
        else
        {
            AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(), resources.resFinishedNoSuccess, ErrorHandler.ERROR_WARNING);
        }
    }

    public void cancel()
    {
        xDialog.endExecute();
    }
    private Process process;
    private boolean exitOnCreate = true;

    /**
     * the user clicks the finish/create button.
     */
    public boolean finishWizard()
    {
        finishWizard(true);
        return true;
    }

    /**
     * finish the wizard
     * @param exitOnCreate_ should the wizard close after
     * a successfull create.
     * Default is true,
     * I have a hidden feature which enables false here 
     */
    public void finishWizard(boolean exitOnCreate_)
    {

        exitOnCreate = exitOnCreate_;

        /**
         * First I check if ftp password was set, if not - the ftp dialog pops up...
         * This may happen when a session is loaded, since the
         * session saves the ftp url and username, but not the password.
         */
        final CGPublish p = getPublisher(FTP_PUBLISHER);
        // if ftp is checked, and no proxies are set, and password is empty...
        if (p.cp_Publish && (!proxies) && (p.password == null || p.password.equals(PropertyNames.EMPTY_STRING)))
        {
            if (showFTPDialog(p))
            {
                updatePublishUI(2);
                //now continue...
                finishWizard2();
            }
        }
        else
        {
            finishWizard2();
        }
    }

    /**
     * this method is only called
     * if ftp-password was eather set, or
     * the user entered one in the FTP Dialog which
     * popped up when clicking "Create". 
     *
     */
    private void finishWizard2()
    {

        CGPublish p = getPublisher(LOCAL_PUBLISHER);
        p.url = p.cp_URL;

        /*
         * zip publisher is using another url form...
         */
        p = getPublisher(ZIP_PUBLISHER);
        //replace the '%' with '%25'
        String url1 = JavaTools.replaceSubString(p.cp_URL, "%25", "%");
        //replace all '/' with '%2F'
        url1 = JavaTools.replaceSubString(url1, "%2F", "/");

        p.url = "vnd.sun.star.zip://" + url1 + "/";

        /*
         * and now ftp...
         */
        p = getPublisher(FTP_PUBLISHER);
        p.url = FTPDialog.getFullURL(p);


        /* first we check the publishing targets. If they exist we warn and ask
         * what to do. a False here means the user said "cancel" (or rather: clicked...)
         */
        if (!publishTargetApproved())
        {
            return;
        /*
         * In order to save the session correctly,
         * I return the value of the ftp publisher cp_Publish
         * property to its original value...
         */
        }
        p.cp_Publish = __ftp;

        //if the "save settings" checkbox is on...
        if (isSaveSession())
        {
            // if canceled by user
            if (!saveSession())
            {
                return;
            }
        }
        else
        {
            settings.cp_LastSavedSession = PropertyNames.EMPTY_STRING;
        }
        try
        {
            Object conf = Configuration.getConfigurationRoot(xMSF, CONFIG_PATH, true);
            Configuration.set(
                    settings.cp_LastSavedSession,
                    "LastSavedSession", conf);
            Configuration.commit(conf);
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }

        /*
         * again, if proxies are on, I disable ftp before the creation process
         * starts. 
         */
        if (proxies)
        {
            p.cp_Publish = false;

        /*
         * There is currently a bug, which crashes office when
         * writing folders to an existing zip file, after deleting
         * its content, so I "manually" delete it here...
         */
        }
        p = getPublisher(ZIP_PUBLISHER);
        if (getFileAccess().exists(p.cp_URL, false))
        {
            getFileAccess().delete(p.cp_URL);
        }
        try
        {

            ErrorHandler eh = new ProcessErrorHandler(xMSF, xControl.getPeer(), resources);

            process = new Process(settings, xMSF, eh);

            StatusDialog pd = getStatusDialog();

            pd.setRenderer(new ProcessStatusRenderer(resources));
            pd.execute(this, process.myTask, resources.prodName);  //process,
            process.runProcess();
            finishWizardFinished();
            process.myTask.removeTaskListener(pd);

        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }

    }

    /**
     * implements a hidden feature for "finishing" without
     * closing the wizard.
     * press "&%" quite fast when the focus is on one
     * of the last steps' checkboxes.
     * @author rp143992
     */
    private class Create implements XKeyListener
    {

        long time = 0;
        int count = 0;

        /* (non-Javadoc)
         * @see com.sun.star.awt.XKeyListener#keyPressed(com.sun.star.awt.KeyEvent)
         */
        public void keyPressed(KeyEvent ke)
        {
            if (ke.KeyChar == '&')
            {
                time = System.currentTimeMillis();
            }
            else if (ke.KeyChar == '%' && ((System.currentTimeMillis() - time) < 300))
            {
                Boolean b = (Boolean) getControlProperty("btnWizardFinish", PropertyNames.PROPERTY_ENABLED);
                if (b.booleanValue())
                {
                    finishWizard(false);
                }
            }
        }

        public void keyReleased(KeyEvent arg0)
        {
        }

        public void disposing(EventObject arg0)
        {
        }
    }

    /**
     * is called on the WindowHidden event, 
     * deletes the temporary directory.
     */
    public void cleanup()
    {


        try
        {
            dpStylePreview.dispose();
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }

        stylePreview.cleanup();

        try
        {
            if (bgDialog != null)
            {
                bgDialog.xComponent.dispose();
            }
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }

        try
        {
            if (iconsDialog != null)
            {
                iconsDialog.xComponent.dispose();
            }
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }

        try
        {
            if (ftpDialog != null)
            {
                ftpDialog.xComponent.dispose();
            }
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }

        try
        {
            xComponent.dispose();
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }

        try
        {
            XCloseable xCloseable = UnoRuntime.queryInterface(XCloseable.class, myFrame);
            if (xCloseable != null)
            {
                xCloseable.close(false);
            }
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }

    }

    public class LoadDocs
    {

        private XControl xC;
        String[] files;
        Task task;

        public LoadDocs(XControl xC_, String[] files_, Task task_)
        {
            xC = xC_;
            files = files_;
            task = task_;
        }

        public void loadDocuments()
        {
            // where the documents are added to in the list (offset)
            int offset = (getSelectedDoc().length > 0 ? selectedDoc[0] + 1 : getDocsCount());

            /* if the user chose one file, the list starts at 0, 
             * if he chose more than one, the first entry is a directory name,
             * all the others are filenames.
             */
            int start = (files.length > 1 ? 1 : 0);
            /*
             * Number of documents failed to validate. 
             */
            int failed = 0;

            // store the directory
            settings.cp_DefaultSession.cp_InDirectory = start == 1 ? files[0] : FileAccess.getParentDir(files[0]);

            /*
             * Here i go through each file, and validate it.
             * If its ok, I add it to the ListModel/ConfigSet
             */
            for (int i = start; i < files.length; i++)
            {
                CGDocument doc = new CGDocument();
                doc.setRoot(settings);

                doc.cp_URL = (start == 0) ? files[i] : FileAccess.connectURLs(files[0], files[i]);

                /* so - i check each document and if it is ok I add it.
                 * The failed variable is used only to calculate the place to add -
                 * Error reporting to the user is (or should (-:  )done in the checkDocument(...) method
                 */
                if (checkDocument(doc, task, xC))
                {
                    settings.cp_DefaultSession.cp_Content.cp_Documents.add(offset + i - failed - start, doc);
                }
                else
                {
                    failed++;
                }
            }

            // if any documents where added,
            // set the first one to be the current-selected document.
            if (files.length > start + failed)
            {
                setSelectedDoc(new short[]
                        {
                            (short) offset
                        });
            }
            // update the ui...
            docListDA.updateUI();
            // this enables/disables the next steps.
            // when no documents in the list, all next steps are disabled
            checkSteps();
            /* a small insurance that the status dialog will
             * really close...
             */
            while (task.getStatus() < task.getMax())
            {
                task.advance(false);
            }
        }
    }
}

