/*************************************************************************
 *
 *  $RCSfile: WWD_Events.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $  $Date: 2004-05-19 13:14:40 $
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

import javax.swing.ListModel;

import com.sun.star.awt.Key;
import com.sun.star.awt.KeyEvent;
import com.sun.star.awt.KeyFunction;
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
import com.sun.star.wizards.ui.event.DataAware;
import com.sun.star.wizards.ui.event.ListModelBinder;
import com.sun.star.wizards.ui.event.MethodInvocation;
import com.sun.star.wizards.ui.event.Task;
import com.sun.star.wizards.web.data.CGDocument;
import com.sun.star.wizards.web.data.CGPublish;
import com.sun.star.wizards.web.data.CGSession;

/**
 * This class implements the ui-events of the
 * web wizard.
 * it is therfore sorted to steps.
 * not much application-logic here - just plain
 * methods which react to events.
 * The only exception are the finish methods with the save
 * session methods.
 */
public abstract class WWD_Events extends WWD_Startup {

    /**
     * He - my constructor !
     * I add a window listener, which, when
     * the window closes, deltes the temp directory.
     */
    public WWD_Events(XMultiServiceFactory xmsf) throws Exception {
        super(xmsf);
        Create c = new Create();
        XWindow xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class,chkFTP);
        xWindow.addKeyListener(c);
        xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class,chkLocalDir);
        xWindow.addKeyListener(c);
        xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class,chkZip);
        xWindow.addKeyListener(c);
    }

    /* *********************************************************
     *  *******************************************************
     *          EVENT and UI METHODS
     *  *******************************************************
     * *********************************************************/

    protected void enterStep(int old, int newStep) {
        if ((old == 1) && (newStep == 2))
            checkDocList();

    }

    /* *********************************
     *  STEP 1
     */

    /**
     * Called from the Uno event dispatcher when the
     * user selects a saved session.
     */
    public void sessionSelected() {
        short[] s = (short[]) Helper.getUnoPropertyValue(getModel(lstLoadSettings), "SelectedItems");
        setEnabled(btnDelSession, s.length > 0);
        setEnabled(btnLoadSession, s.length > 0);
    }

    /**
     * Ha ! the user clicked the
     * Load button !
     */
    public void loadSession() {
        StatusDialog sd = getStatusDialog();
        sd.setLabel(resources.resLoadingSession);

        final Task task = new Task("LoadDocs", "", 10);

        sd.execute(this, task, new Runnable() {
            public void run() {
                try {
                    task.start();
                    short[] selected = (short[]) Helper.getUnoPropertyValue(getModel(lstLoadSettings), "SelectedItems");
                    String name = (String) settings.cp_SavedSessions.getKey(selected[0]);

                    Object view = Configuration.getConfigurationRoot(xMSF, CONFIG_PATH + "/SavedSessions", false);
                    view = Configuration.getNode(name, view);
                    CGSession session = new CGSession();
                    session.setRoot(settings);
                    session.readConfiguration(view, CONFIG_READ_PARAM);
                    task.setMax(session.cp_Content.cp_Documents.getSize() * 5 + 5);
                    task.advance(true);

                    mount(session, task);
                    checkSteps();
                } catch (Exception ex) {
                    unexpectedError(ex);
                }

                while (task.getStatus() <= task.getMax())
                    task.advance(false);
            }
        });
    }

    /**
     * hmm. the user clicked the delete button.
     */
    public void delSession() {
        boolean confirm = AbstractErrorHandler.showMessage(xMSF,xControl.getPeer(), resources.resDelSessionConfirm, ErrorHandler.ERROR_QUESTION_NO);
        if (confirm) {
            try {
                short[] selected = (short[]) Helper.getUnoPropertyValue(getModel(lstLoadSettings), "SelectedItems");
                String name = (String) settings.cp_SavedSessions.getKey(selected[0]);
                // first delete the session from the registry/configuration.

                Configuration.removeNode(xMSF, CONFIG_PATH + "/SavedSessions", name);

                // then delete the session from the java-set (settings.cp_SavedSessions)
                settings.cp_SavedSessions.remove(selected[0]);
                //disable buttons
                Helper.setUnoPropertyValue(getModel(btnDelSession), "Enabled", Boolean.FALSE);
                Helper.setUnoPropertyValue(getModel(btnLoadSession), "Enabled", Boolean.FALSE);

                ListModelBinder.fillComboBox(cbSaveSettings, settings.cp_SavedSessions.items(), null);

            } catch (Exception ex) {
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
    public short[] getSelectedDoc() {
        return selectedDoc;
    }

    private static String[] EMPTY_STRING_ARRAY = new String[0];

    /*  public void loadSessionSelected() {
            UIHelper.setEnabled(btnLoadSession,true);
            UIHelper.setEnabled(btnDelSession,true);
        }
    */

    /**
     * when the user clicks another document
     * in the listbox, this method is called,
     * and couses the display in
     * the textboxes title,description, author and export format
     * to change
     */
    public void setSelectedDoc(short[] s) {
        CGDocument oldDoc = getDoc(selectedDoc);
        CGDocument doc = getDoc(s);

        if (doc == null)
            fillExportList(EMPTY_STRING_ARRAY);
        //I try to avoid refreshing the export list if
        //the same type of document is chosen.
        else if (oldDoc == null || (!oldDoc.appType.equals(doc.appType)))
            fillExportList(settings.getExporters(doc.appType));
        else
            ; // do nothing

        selectedDoc = s;

        mount(doc, docAware);
        disableDocUpDown();
    }

    private static final String[] EXTENSIONS = { "StarOffice XML (Writer)", "StarOffice XML (Calc)", "sxi", "sxd", "*" };

    /**
     * The user clicks the "Add" button.
     * This will open a "FileOpen" dialog,
     * and, if the user chooses more than one file,
     * will open a status dialog, when validating each document.
     */
    public void addDocument() {

        final String[] files = getDocAddDialog().callOpenDialog(true, settings.cp_DefaultSession.cp_InDirectory);
        if (files == null)
            return;

        final Task task = new Task("", "", files.length * 5);


        Runnable loadDocs = new Runnable() {
            public void run() {
                //LogTaskListener lts = new LogTaskListener();
                //task.addTaskListener(lts);

                task.start();

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
                for (int i = start; i < files.length; i++) {
                    CGDocument doc = new CGDocument();
                    doc.setRoot(settings);

                    doc.cp_URL = (start == 0) ? files[i] : FileAccess.connectURLs(files[0], files[i]);

                    /* so - i check each document and if it is ok I add it.
                     * The failed variable is used only to calculate the place to add -
                     * Error reporting to the user is (or should (-:  )done in the checkDocument(...) method
                     */
                    if (checkDocument(doc, task))
                        settings.cp_DefaultSession.cp_Content.cp_Documents.add(offset + i - failed - start, doc);
                    else
                        failed++;

                }

                // if any documents where added,
                // set the first one to be the current-selected document.
                if (files.length > start + failed) {
                    setSelectedDoc(new short[] {(short) offset });
                }
                // update the ui...
                docListDA.updateUI();
                // this enables/disables the next steps.
                // when no documents in the list, all next steps are disabled
                checkSteps();
                /* a small insurance that the status dialog will
                 * really close...
                 */
                while(task.getStatus() < task.getMax())
                    task.advance(false);
            }
        };

        /*
         * If more than a certain number
         * of documents have been added,
         * open the status dialog.
         */
        if (files.length > MIN_ADD_FILES_FOR_DIALOG) {
            StatusDialog sd = getStatusDialog();
            sd.setLabel(resources.resValidatingDocuments);

            sd.execute(this, task, loadDocs);
        }
        /*
         * When adding a single document, do not use a
         * status dialog...
         */
        else
            loadDocs.run();
    }

    /**
     * The user clicked delete.
     */
    public void removeDocument() {
        settings.cp_DefaultSession.cp_Content.cp_Documents.remove(selectedDoc[0]);

        // update the selected document
        while (selectedDoc[0] >= getDocsCount())
            selectedDoc[0]--;

        // if there are no documents...
        if (selectedDoc[0] == -1)
            selectedDoc = new short[0];

        // update the list to show the right selection.
        docListDA.updateUI();
        // disables all the next steps, if the list of docuemnts
        // is empty.
        checkSteps();
    }



    /**
     * doc up.
     */
    public void docUp() {
        Object doc = settings.cp_DefaultSession.cp_Content.cp_Documents.getElementAt(selectedDoc[0]);
        settings.cp_DefaultSession.cp_Content.cp_Documents.remove(selectedDoc[0]);
        settings.cp_DefaultSession.cp_Content.cp_Documents.add(--selectedDoc[0], doc);
        docListDA.updateUI();
        disableDocUpDown();
    }

    /**
     * doc down
     */
    public void docDown() {
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
    public void chooseBackground() {
        new Thread() {
            public void run() {
                try {
                    setEnabled(btnBackgrounds, false);
                    if (bgDialog == null) {
                        bgDialog = new BackgroundsDialog(xMSF, settings.cp_BackgroundImages, resources );
                        bgDialog.createWindowPeer(xControl.getPeer());
                    }
                    bgDialog.setSelected(settings.cp_DefaultSession.cp_Design.cp_BackgroundImage);
                        short i = bgDialog.executeDialog(WWD_Events.this);
                        if (i == 1) //ok
                            setBackground(bgDialog.getSelected());
                } catch (Exception ex) {
                    ex.printStackTrace();
                } finally {
                    setEnabled(btnBackgrounds, true);
                }
            }
        }
        .start();

    }

    /**
     * invoked when the BackgorundsDialog is "OKed".
     */
    public void setBackground(Object background) {
        if (background == null)
            background = "";
        settings.cp_DefaultSession.cp_Design.cp_BackgroundImage = (String) background;
        refreshStylePreview();
    }

    private IconsDialog iconsDialog;

    /**
     * is called when the user clicks "Icon sets" button.
     *
     */
    public void chooseIconset() {
        new Thread() {
            public void run() {
                try {
                    setEnabled(btnIconSets, false);
                    if (iconsDialog == null) {
                        iconsDialog = new IconsDialog(xMSF, settings.cp_IconSets, resources);
                        iconsDialog.createWindowPeer(xControl.getPeer());
                    }

                    iconsDialog.setIconset(settings.cp_DefaultSession.cp_Design.cp_IconSet);

                    short i = iconsDialog.executeDialog(WWD_Events.this);
                    if (i == 1) //ok
                        setIconset(iconsDialog.getIconset());
                } catch (Exception ex) {
                    ex.printStackTrace();
                } finally {
                    setEnabled(btnIconSets, true);
                }
            }
        }
        .start();
    }

    /**
        * invoked when the Iconsets Dialog is OKed.
        */
    public void setIconset(String icon) {
        settings.cp_DefaultSession.cp_Design.cp_BackgroundImage = icon;
    }

    /* ******************************
     * STEP 6
     */
    public void chooseFavIcon() {
        String[] files = getFavIconDialog().callOpenDialog(false, settings.cp_DefaultSession.cp_InDirectory);
        if (files == null) //if user canceled.
            return;

        //store the directory.
        settings.cp_DefaultSession.cp_InDirectory = FileAccess.getParentDir(files[0]);

        //store the fav icon
        settings.cp_DefaultSession.cp_GeneralInfo.cp_Icon = files[0];

        //and display it...
        favIconDA.updateUI();
    }

    public void setIcon(String icon) { /*dummy - neverCalled*/
    }

    /**
     * When the user presses "delete" or "backspace"
     * on the FavIcon textbox, the content is deleted,
     * @param event
     */
    public void removeFavIcon(KeyEvent event) {
        if (event.KeyFunc == KeyFunction.DELETE || event.KeyCode == Key.BACKSPACE) {
            settings.cp_DefaultSession.cp_GeneralInfo.cp_Icon = "";
            //and display it...
            favIconDA.updateUI();
        }
    }

    public String getIcon() {
        return getFileAccess().getPath(settings.cp_DefaultSession.cp_GeneralInfo.cp_Icon, "");
    }

    /* ******************************
     * STEP 7
     */

    /**
     * sets the publishing url of either a local/zip or ftp publisher.
     * updates the ui....
     */
    private CGPublish setPublishUrl(String publisher, String url, int number) {
        if (url == null)
            return null;
        CGPublish p = getPublisher(publisher);
        p.cp_URL = url;
        p.cp_Publish = true;
        updatePublishUI(number);
        return p;
    }

    /**
     * updates the ui of a certain publisher
     * (the text box url)
     * @param number
     */
    private void updatePublishUI(int number) {
        ((DataAware) pubAware.get(number)).updateUI();
        ((DataAware) pubAware.get(number + 1)).updateUI();
        checkPublish();
    }

    /**
     * The user clicks the local "..." button.
     *
     */
    public void setPublishLocalDir() {
        String dir = showFolderDialog("Local destination directory", "", settings.cp_DefaultSession.cp_OutDirectory);
        //if ok was pressed...
        setPublishUrl(LOCAL_PUBLISHER, dir, 0);
    }

    /**
     * The user clicks the "Configure" FTP button.
     *
     */
    public void setFTPPublish() {
        new Thread(new Runnable() {
            public void run() {
                if (showFTPDialog(getPublisher(FTP_PUBLISHER))) {
                    getPublisher(FTP_PUBLISHER).cp_Publish = true;
                    updatePublishUI(2);
                }

            }
        }).start();
    }

    /**
     * show the ftp dialog
     * @param pub
     * @return true if OK was pressed, otherwise false.
     */
    private boolean showFTPDialog(CGPublish pub) {
        try {
            return getFTPDialog(pub).execute(this) == 1;
        } catch (Exception ex) {
            ex.printStackTrace();
            return false;
        }
    }

    /**
     * the user clicks the zip "..." button.
     * Choose a zip file...
     */
    public void setZipFilename() {
        SystemDialog sd = getZipDialog();
        String zipFile = sd.callStoreDialog(settings.cp_DefaultSession.cp_OutDirectory, resources.resDefaultArchiveFilename);
        setPublishUrl(ZIP_PUBLISHER, zipFile, 4);
    }

    private TOCPreview docPreview;

    /**
     * the user clicks the "Preview" button.
     */
    public void documentPreview() {
        try {
            if (docPreview == null)
                docPreview = new TOCPreview(xMSF, settings, resources, stylePreview.tempDir);
            docPreview.refresh(settings);
        } catch (Exception ex) {
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
    private boolean publishTargetExists(String publisher) {
        CGPublish p = getPublisher(publisher);
        //well, if this publisher does not publish, then its ok...
        if (!p.cp_Publish)
            return true;

        String path = p.url;

        // if the target exists we're in trouble...
        if (getFileAccess().exists(path, false)) {
            //if its a directory
            if (getFileAccess().isDirectory(path)) {
                //check if its empty
                String[] files = getFileAccess().listFiles(path, true);
                if (files.length > 0) {
                    /* it is not empty :-(
                     * it either a local publisher or an ftp (zip uses no directories
                     * as target...)
                     */
                    String message = getExistsMessage(publisher);

                    return AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(), message, ErrorHandler.ERROR_QUESTION_CANCEL);
                }
            } else //not a directory, but still exists
                {
                if (publisher.equals(ZIP_PUBLISHER))
                    return AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(),resources.resZipTargetExists, ErrorHandler.ERROR_QUESTION_CANCEL);
                /*
                 * This is an interessting option:
                 * a file exists with the name of the directory that
                 * should be created - so we can't really go on...
                 */
                else if (publisher.equals(LOCAL_PUBLISHER))
                    return AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(),resources.resLocalTargetExistsAsfile, ErrorHandler.ERROR_PROCESS_FATAL);
                else if (publisher.equals(FTP_PUBLISHER))
                    return AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(),resources.resFTPTargetExistsAsfile, ErrorHandler.ERROR_PROCESS_FATAL);
                else throw new IllegalArgumentException("Illegal publisher name");
            }
        }
        return true;
    }

    private String getExistsMessage(String publisher) {
        if (publisher.equals(LOCAL_PUBLISHER))
            return resources.resLocalTragetNotEmpty;
        else if (publisher.equals(ZIP_PUBLISHER))
            return resources.resZipTargetExists;
        else if (publisher.equals(FTP_PUBLISHER))
            return resources.resFTPTargetNotEmpty;
        else throw new IllegalArgumentException("Illegal publisher name");
    }

    /*
     * return false if "create" should be aborted. true if everything is fine.
     */
    private boolean saveSession() {
        try {
            Object node = null;
            String name = getSessionSaveName();

            //set documents index field.

            ListModel docs = settings.cp_DefaultSession.cp_Content.cp_Documents;

            for (int i = 0; i < docs.getSize(); i++)
                 ((CGDocument) docs.getElementAt(i)).cp_Index = i;

            Object conf = Configuration.getConfigurationRoot(xMSF, CONFIG_PATH + "/SavedSessions", true);
            // first I check if a session with the given name exists
            try {
                node = Configuration.getNode(name, conf);
                if (node != null)
                    if (!AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(),
                        JavaTools.replaceSubString(resources.resSessionExists, name, "${NAME}"),
                        ErrorHandler.ERROR_NORMAL_IGNORE))

                            return false;

                //remove the old session
                Configuration.removeNode(conf, name);

            } catch (NoSuchElementException nsex) {}

            settings.cp_DefaultSession.cp_Index = 0;
            node = Configuration.addConfigNode(conf, name);
            settings.cp_DefaultSession.cp_Name = name;
            settings.cp_DefaultSession.writeConfiguration(node, CONFIG_READ_PARAM);
            settings.cp_SavedSessions.reindexSet(conf, name, "Index");
            Configuration.commit(conf);

            // now I reload the sessions to actualize the list/combo boxes load/save sessions.
            while (settings.cp_SavedSessions.getSize() > 0)
                settings.cp_SavedSessions.remove(0);

            Object confView = Configuration.getConfigurationRoot(xMSF, CONFIG_PATH + "/SavedSessions", false);
            settings.cp_SavedSessions.readConfiguration(confView,CONFIG_READ_PARAM);

            Object[] o = settings.cp_SavedSessions.items();
            ListModelBinder.fillList(lstLoadSettings,o, null);
            ListModelBinder.fillComboBox(cbSaveSettings,o, null);

            return true;
        } catch (Exception ex) {
            ex.printStackTrace();
            return false;
        }
    }

    /**
     * this method will be called when the Status Dialog
     * is hidden.
     * It checks if the "Process" was successfull, and if so,
     * it closes the wizard dialog.
     */
    public void finishWizardFinished() {
        if (process.getResult()) {
            AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(),resources.resFinishedSuccess, ErrorHandler.ERROR_MESSAGE);
            if (exitOnCreate)
                this.xDialog.endExecute();
        } else
            AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(),resources.resFinishedNoSuccess, ErrorHandler.ERROR_WARNING);

    }

    public void cancel() {
        xDialog.endExecute();
    }

    private Process process;
    private boolean exitOnCreate = true;

    /**
     * the user clicks the finish/create button.
     */
    public void finishWizard() {
        finishWizard(true);
    }

    /**
     * finish the wizard
     * @param exitOnCreate_ should the wizard close after
     * a successfull create.
     * Default is true,
     * I have a hidden feature which enables false here
     */
    public void finishWizard(boolean exitOnCreate_) {

        exitOnCreate = exitOnCreate_;

        /**
         * First I check if ftp password was set, if not - the ftp dialog pops up...
         * This may happen when a session is loaded, since the
         * session saves the ftp url and username, but not the password.
         */
        final CGPublish p = getPublisher(FTP_PUBLISHER);
        // if ftp is checked, and no proxies are set, and password is empty...
        if (p.cp_Publish && (!proxies) && (p.password == null || p.password.equals("")))
            new Thread(new Runnable() {
                public void run() {
                    if (showFTPDialog(p)) {
                        updatePublishUI(2);
                        //now continue...
                        finishWizard2();
                    }
                }
            }).start();
        else
            finishWizard2();
    }


    /**
     * this method is only called
     * if ftp-password was eather set, or
     * the user entered one in the FTP Dialog which
     * popped up when clicking "Create".
     *
     */
    private void finishWizard2() {
        /* local publisher is publishing on a subdirectory of
         * the chosen directory.
         */
        CGPublish p = getPublisher(LOCAL_PUBLISHER);
        p.url = FileAccess.connectURLs(p.cp_URL , resources.resPublishDir);
        /*
         * zip publisher is using another url form...
         */
        p = getPublisher(ZIP_PUBLISHER);
        //replace the '%' with '%25'
        String url1 = JavaTools.replaceSubString(p.cp_URL,"%25","%");
        //replace all '/' with '%2F'
        url1 = JavaTools.replaceSubString(url1, "%2F", "/");

        p.url = "vnd.sun.star.pkg://" + url1 + "/";

        /*
         * and now ftp...
         */
        p = getPublisher(FTP_PUBLISHER);
        p.url = FTPDialog.getFullURL(p);

        /*
         * Now, if proxies disable ftp,
         * I save the cp_Publish value, so it will be saved correctly, and disable
         * it for the target check that follows.
         */
        boolean ftp__ = p.cp_Publish;
        if (proxies)
            p.cp_Publish = false;

        /* first we check the publishing targets. If they exist we warn and ask
         * what to do. a False here means the user said "cancel" (or rather: clicked...)
         */
        if (!(publishTargetExists(LOCAL_PUBLISHER) && publishTargetExists(ZIP_PUBLISHER) && publishTargetExists(FTP_PUBLISHER)))
            return;

        /*
         * In order to save the session correctly,
         * I return the value of the ftp publisher cp_Publish
         * property to its original value...
         */
        p.cp_Publish = ftp__;

        //if the "save settings" checkbox is on...
        if (isSaveSession())
            // if canceled by user
            if (!saveSession())
                return;

        /*
         * again, if proxies are on, I disable ftp before the creation process
         * starts.
         */
        if (proxies)
             p.cp_Publish = false;

        /*
         * There is currently a bug, which crashes office when
         * writing folders to an existing zip file, after deleting
         * its content, so I "manually" delete it here...
         */
        p = getPublisher(ZIP_PUBLISHER);
        if (getFileAccess().exists(p.cp_URL,false))
            getFileAccess().delete(p.cp_URL);

        try {

            ErrorHandler eh = new ProcessErrorHandler(xMSF, xControl.getPeer(),resources);

            process = new Process(settings, xMSF, eh);

            StatusDialog pd = getStatusDialog();

            pd.setRenderer(new ProcessStatusRenderer(resources));
            pd.setFinishedMethod(new MethodInvocation("finishWizardFinished", this));


            pd.execute(this, process.myTask, process);

        } catch (Exception ex) {
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
    private class Create implements XKeyListener {
        long time = 0;
        int count = 0;

        /* (non-Javadoc)
         * @see com.sun.star.awt.XKeyListener#keyPressed(com.sun.star.awt.KeyEvent)
         */
        public void keyPressed(KeyEvent ke) {
            if (ke.KeyChar == '&' )
                 time = System.currentTimeMillis();
            else if (ke.KeyChar == '%' && ( (System.currentTimeMillis() - time) < 300) )
                {
                    Boolean b = (Boolean)getControlProperty("btnWizardFinish","Enabled");
                    if (b.booleanValue())
                        finishWizard(false);
                }
        }
        public void keyReleased(KeyEvent arg0) {}
        public void disposing(EventObject arg0) {}

    }

    /**
     * is called on the WindowHidden event,
     * deletes the temporary directory.
     */
    public void cleanup() {

        try {
            dpStylePreview.dispose();
        }
        catch (Exception ex) {ex.printStackTrace();}

        stylePreview.cleanup();

        try {
            if (bgDialog != null)
                bgDialog.xComponent.dispose();
        }
        catch (Exception ex) {ex.printStackTrace();}

        try {
            if (iconsDialog != null)
                iconsDialog.xComponent.dispose();
        }
        catch (Exception ex) {ex.printStackTrace();}

        try {
            if (ftpDialog != null)
                ftpDialog.xComponent.dispose();
        }
        catch (Exception ex) {ex.printStackTrace();}

        try {
            if (statusDialog != null)
                statusDialog.xComponent.dispose();
        }
        catch (Exception ex) {ex.printStackTrace();}

        try {
            xComponent.dispose();
        }
        catch (Exception ex) {ex.printStackTrace();}

        try {
            XCloseable xCloseable = (XCloseable) UnoRuntime.queryInterface(XCloseable.class, myOwnFrame);
            xCloseable.close(false);
        }
        catch (Exception ex) {ex.printStackTrace();}
    }
}