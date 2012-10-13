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

import com.sun.star.awt.XWindowPeer;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.util.XStringSubstitution;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.HelpIds;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.common.SystemDialog;
import com.sun.star.wizards.ui.event.ListModelBinder;
import com.sun.star.wizards.web.data.CGDocument;
import com.sun.star.wizards.web.data.CGFilter;
import com.sun.star.wizards.web.data.CGPublish;
import com.sun.star.wizards.web.data.CGSettings;

/**
 * This class implements general methods, used by different sub-classes (either WWD_Sturtup, or WWD_Events)
 * or both.
 */
public abstract class WWD_General extends WebWizardDialog
{

    private FileAccess fileAccess;
    private SystemDialog docAddDialog,  folderDialog,  favIconDialog,  zipDialog;
    protected FTPDialog ftpDialog;
    protected CGSettings settings;
    /**
     * true if proxies are on, which means, ftp is disabled.
     */
    protected boolean proxies;
    private XStringSubstitution xStringSubstitution;

    protected StatusDialog getStatusDialog()
    {

        StatusDialog statusDialog = new StatusDialog(xMSF, StatusDialog.STANDARD_WIDTH, resources.resLoadingSession, false, new String[]
                {
                    resources.prodName, PropertyNames.EMPTY_STRING, PropertyNames.EMPTY_STRING, PropertyNames.EMPTY_STRING, PropertyNames.EMPTY_STRING, PropertyNames.EMPTY_STRING
                }, HelpIds.getHelpIdString(HID0_STATUS_DIALOG));
        try
        {
            statusDialog.createWindowPeer(xControl.getPeer());
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return statusDialog;
    }

    /**
     * @param xmsf
     */
    public WWD_General(XMultiServiceFactory xmsf)
    {
        super(xmsf);
        xStringSubstitution = SystemDialog.createStringSubstitution(xmsf);
    }

    /*
     *  File Dialog methods 
     */
    protected SystemDialog getDocAddDialog()
    {
        docAddDialog = SystemDialog.createOpenDialog(xMSF);
        for (int i = 0; i < settings.cp_Filters.getSize(); i++)
        {
            CGFilter f = ((CGFilter) settings.cp_Filters.getElementAt(i));
            docAddDialog.addFilter(
                    JavaTools.replaceSubString(f.cp_Name, resources.prodName, "%PRODNAME"), f.cp_Filter, i == 0);
        }
        return docAddDialog;
    }

    protected SystemDialog getZipDialog()
    {
        if (zipDialog == null)
        {
            zipDialog = SystemDialog.createStoreDialog(xMSF);
            zipDialog.addFilter(resources.resZipFiles, "*.zip", true);
        }
        return zipDialog;
    }

    protected FTPDialog getFTPDialog(CGPublish pub)
    {
        if (ftpDialog == null)
        {
            try
            {
                ftpDialog = new FTPDialog(xMSF, pub);
                ftpDialog.createWindowPeer(xControl.getPeer());
            }
            catch (Exception ex)
            {
                ex.printStackTrace();
            }
        }
        return ftpDialog;
    }

    protected String showFolderDialog(String title, String description, String dir)
    {
        if (folderDialog == null)
        {
            folderDialog = SystemDialog.createFolderDialog(xMSF);
        }
        return folderDialog.callFolderDialog(title, description, dir);
    }

    protected FileAccess getFileAccess()
    {
        if (fileAccess == null)
        {
            try
            {
                fileAccess = new FileAccess(xMSF);
            }
            catch (Exception ex)
            {
                ex.printStackTrace();
            }
        }
        return fileAccess;
    }

    /**
     * returns the document specified 
     * by the given short array.
     * @param s
     * @return
     */
    protected CGDocument getDoc(short[] s)
    {
        if (s.length == 0)
        {
            return null;
        }
        else if (settings.cp_DefaultSession.cp_Content.cp_Documents.getSize() <= s[0])
        {
            return null;
        }
        else
        {
            return (CGDocument) settings.cp_DefaultSession.cp_Content.cp_Documents.getElementAt(s[0]);
        }
    }

    /**
     * how many documents are in the list?
     * @return the number of documents in the docs list.
     */
    protected int getDocsCount()
    {
        return settings.cp_DefaultSession.cp_Content.cp_Documents.getSize();
    }

    /**
     * fills the export listbox.
     * @param listContent
     */
    protected void fillExportList(Object[] listContent)
    {
        ListModelBinder.fillList(lstDocTargetType, listContent, null);
    }

    /**
     * returns a publisher object for the given name 
     * @param name one of the WebWizardConst constants : FTP 
     * @return
     */
    protected CGPublish getPublisher(String name)
    {
        return (CGPublish) settings.cp_DefaultSession.cp_Publishing.getElement(name);
    }

    /**
     * @return true if the checkbox "save session" is checked.
     */
    protected boolean isSaveSession()
    {
        return (((Number) Helper.getUnoPropertyValue(
                getModel(chkSaveSettings), PropertyNames.PROPERTY_STATE)).intValue() == 1);
    }

    /**
     * @return the name to save the session (step 7)
     */
    protected String getSessionSaveName()
    {
        return (String) Helper.getUnoPropertyValue(
                getModel(cbSaveSettings), "Text");
    }

    /**
     * This method checks the status of the wizards and
     * enables or disables the 'next' and the 'create' button.
     *
     */
    protected void checkSteps()
    {
        /* first I check the document list.
         * If it is empty, then step3 and on are disabled.
         */
        if (checkDocList())
        {
            checkPublish();
        }
    }

    /**
     * enables/disables the steps 3 to 7)
     * @param enabled true = enabled, false = disabled.
     */
    private void enableSteps(boolean enabled)
    {

        if (!enabled && !isStepEnabled(3))
        {
            return;
        /*
         * disbale steps 3-7 
         */
        }
        for (int i = 3; i < 8; i++)
        {
            setStepEnabled(i, enabled, true);
        /* in this place i just disable the finish button.
         * later, in the checkPublish, which is only performed if
         * this one is true, it will be enabled (if the check 
         * is positive)
         */
        }
        if (!enabled)
        {
            enableFinishButton(false);
        }
    }

    /**
     * Checks if the documents list is
     * empty. If it is, disables the steps 3-7, and the
     * create button.
     * @return
     */
    protected boolean checkDocList()
    {
        if (settings.cp_DefaultSession.cp_Content.cp_Documents.getSize() == 0)
        {
            enableSteps(false);
            return false;
        }
        else
        {
            enableSteps(true);
            return true;
        }
    }

    /**
     * check if the save-settings input is ok.
     * (eather the checkbox is unmarked, or,
     * if it is marked, a session name exists.
     *
     */
    public boolean checkSaveSession()
    {
        return (!isSaveSession() ||
                !getSessionSaveName().equals(PropertyNames.EMPTY_STRING));

    }

    /**
     * @return false if this publisher is not active, or, if it
     * active, returns true if the url is not empty...
     * if the url is empty, throws an exception
     */
    private boolean checkPublish(String s, Object text, String property)
    {
        CGPublish p = getPublisher(s);
        if (p.cp_Publish)
        {
            String url = (String) Helper.getUnoPropertyValue(getModel(text), property);
            if ((url == null) || (url.equals(PropertyNames.EMPTY_STRING)))
            {
                throw new IllegalArgumentException();
            }
            else
            {
                return true;
            }
        }
        else
        {
            return false;
        }
    }

    /**
     * @return false either if publishing input is wrong or there
     * are no publishing targets chosen. returns true when at least 
     * one target is chosen, *and* all
     * which are chosen are legal.
     * If proxies are on, ftp publisher is ignored.
     */
    private boolean checkPublish_()
    {
        try
        {
            return (checkPublish(LOCAL_PUBLISHER, txtLocalDir, "Text") | (!proxies && checkPublish(FTP_PUBLISHER, lblFTP, PropertyNames.PROPERTY_LABEL)) | checkPublish(ZIP_PUBLISHER, txtZip, "Text")) && checkSaveSession();
        }
        catch (IllegalArgumentException ex)
        {
            return false;
        }
    }

    /**
     * This method checks if the publishing
     * input is ok, and enables and disables 
     * the 'create' button.
     * public because it is called from
     * an event listener object.
     */
    public void checkPublish()
    {
        enableFinishButton(checkPublish_());
    }

    /**
     * shows a message box "Unexpected Error... " :-)
     * @param ex
     */
    protected void unexpectedError(Exception ex)
    {
        ex.printStackTrace();
        XWindowPeer peer = xControl.getPeer();
        AbstractErrorHandler.showMessage(xMSF, peer, resources.resErrUnexpected, ErrorHandler.ERROR_PROCESS_FATAL);
    }

    /**
     * substitutes path variables with the corresponding values. 
     * @param path a path, which might contain OOo path variables. 
     * @return the path, after substituing path variables.
     */
    protected String substitute(String path)
    {
        try
        {
            return xStringSubstitution.substituteVariables(path, false);
        }
        catch (Exception ex)
        {
            return path;
        }
    }
}
