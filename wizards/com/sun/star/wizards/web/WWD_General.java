/*************************************************************************
 *
 *  $RCSfile: WWD_General.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $  $Date: 2004-05-19 13:14:51 $
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

import com.sun.star.awt.XWindowPeer;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.util.XStringSubstitution;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.ui.event.ListModelBinder;
import com.sun.star.wizards.web.data.*;

/**
 * @author rpiterman
 * This class implements general methods, used by different sub-classes (either WWD_Sturtup, or WWD_Events)
 * or both.
 */
public abstract class WWD_General extends WebWizardDialog {


    private FileAccess fileAccess;

    private SystemDialog docAddDialog, folderDialog, favIconDialog, zipDialog;

    protected FTPDialog ftpDialog;

    protected StatusDialog statusDialog;

    protected CGSettings settings;


    /**
     * true if proxies are on, which means, ftp is disabled.
     */
    protected boolean proxies;
    private XStringSubstitution xStringSubstitution ;

    protected StatusDialog getStatusDialog() {
        if (statusDialog == null) {
            statusDialog = new StatusDialog(xMSF, StatusDialog.STANDARD_WIDTH,  resources.resLoadingSession , false , new String[] { resources.prodName, "", "", "", "", "" }, "HID:"+ HID0_STATUS_DIALOG);
            try {
                statusDialog.createWindowPeer(xControl.getPeer());
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return statusDialog;
    }


    /**
     * @param xmsf
     */
    public WWD_General(XMultiServiceFactory xmsf) {
        super(xmsf);
        xStringSubstitution = SystemDialog.createStringSubstitution(xmsf);
    }

    /*
     *  File Dialog methods
     */


    protected SystemDialog getDocAddDialog() {
        //if (docAddDialog == null) {
            docAddDialog = SystemDialog.createOpenDialog(xMSF);
            docAddDialog.addFilter(resources.resSODocs, "*.sxw;*.sxc;*.sxd;*.sxi;*.sdw;*.sdc;*.sdd;*.sdi;*.sda;*.sdp"  ,true);
            docAddDialog.addFilter(resources.resMSDocs, "*.doc;*.xls;*.ppt;*.pps",false);
            docAddDialog.addFilter(resources.resImages, "*.jpg;*.gif;*.png;*.bmp;*.tiff;*.jpeg;*.jpe",false);
            docAddDialog.addFilter(resources.resAllFiles,"*.*",false);
        //}
        return docAddDialog;
    }

    protected SystemDialog getFavIconDialog() {
        if (favIconDialog == null) {
            favIconDialog = SystemDialog.createOpenDialog(xMSF);
            favIconDialog.addFilter(resources.resIconFiles, "*.ico", true);
        }
        return favIconDialog ;
    }


    protected SystemDialog getZipDialog() {
        if (zipDialog==null) {
            zipDialog = SystemDialog.createStoreDialog(xMSF);
            zipDialog.addFilter(resources.resZipFiles,"*.zip",true);
        }
        return zipDialog;
    }

    protected FTPDialog getFTPDialog(CGPublish pub) {
        if (ftpDialog == null) {
            try {
                ftpDialog = new FTPDialog(xMSF,pub);
                ftpDialog.createWindowPeer(xControl.getPeer());
            }
            catch (Exception ex) {
                ex.printStackTrace();
            }
        }
        return ftpDialog;
    }


    protected String showFolderDialog(String title, String description,String dir) {
        if (folderDialog == null)
            folderDialog = SystemDialog.createFolderDialog(xMSF);
        return folderDialog.callFolderDialog(title,description,dir);
    }


    protected FileAccess getFileAccess() {
        if (fileAccess == null)
          try {
              fileAccess = new FileAccess(xMSF);
          }
          catch (Exception ex) {
              ex.printStackTrace();
          }

        return fileAccess;
    }

    /**
     * returns the document specified
     * by the given short array.
     * @param s
     * @return
     */
    protected CGDocument getDoc(short[] s) {
        if (s.length==0)
          return null;
        else return (CGDocument)settings.cp_DefaultSession.cp_Content.cp_Documents.getElementAt(s[0]);
    }

    /**
     * how many documents are in the list?
     * @return the number of documents in the docs list.
     */
    protected int getDocsCount() {
        return settings.cp_DefaultSession.cp_Content.cp_Documents.getSize();
    }


    /**
     * fills the export listbox.
     * @param listContent
     */
    protected void fillExportList(Object[] listContent) {
        ListModelBinder.fillList(lstDocTargetType,listContent,null);
    }

    /**
     * returns a publisher object for the given name
     * @param name one of the WebWizardConst constants : FTP
     * @return
     */
    protected CGPublish getPublisher(String name) {
        return (CGPublish)settings.cp_DefaultSession.cp_Publishing.getElement(name);
    }

    /**
     * @return true if the checkbox "save session" is checked.
     */
    protected boolean isSaveSession() {
        return (((Number)Helper.getUnoPropertyValue(
            getModel(chkSaveSettings),"State")).intValue()==1);
    }

    /**
     * @return the name to save the session (step 7)
     */
    protected String getSessionSaveName() {
        return (String)Helper.getUnoPropertyValue(
            getModel(cbSaveSettings),"Text");
    }


    /**
     * This method checks the status of the wizards and
     * enables or disables the 'next' and the 'create' button.
     *
     */
    protected void checkSteps() {
        /* first I check the document list.
         * If it is empty, then step3 and on are disabled.
         */
        if (checkDocList())
          checkPublish();
    }


    /**
     * enables/disables the steps 3 to 7)
     * @param enabled true = enabled, false = disabled.
     */
    private void enableSteps(boolean enabled) {

        if (!enabled && !isStepEnabled(3))
          return;
        /*
         * disbale steps 3-7
         */
        for (int i = 3; i<8; i++)
            setStepEnabled(i,enabled,true);

        /* in this place i just disable the finish button.
         * later, in the checkPublish, which is only performed if
         * this one is true, it will be enabled (if the check
         * is positive)
         */
        if (!enabled)
            enableFinishButton(false);
    }

    /**
     * Checks if the documents list is
     * empty. If it is, disables the steps 3-7, and the
     * create button.
     * @return
     */
    protected boolean checkDocList() {
        if (settings.cp_DefaultSession.cp_Content.cp_Documents.getSize() == 0) {
            enableSteps(false);
            return false;
        }
        else {
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
    public boolean checkSaveSession() {
        return (!isSaveSession() ||
          !getSessionSaveName().equals(""));

    }


    /**
     * @return false if this publisher is not active, or, if it
     * active, returns true if the url is not empty...
     * if the url is empty, throws an exception
     */
    private boolean checkPublish(String s) {
        CGPublish p = getPublisher(s);
        if (p.cp_Publish) {
            if ((p.cp_URL == null) || (p.cp_URL.equals("")))
                throw new IllegalArgumentException();
            else return true;
        }
        else return false;
    }



    /**
     *
     * @return false either if publishing input is wrong or there
     * are no publishing targets chosen. returns true when at least
     * one target is chosen, *and* all
     * which are chosen are legal.
     * If proxies are on, ftp publisher is ignored.
     */
    private boolean checkPublish_() {
        try {


            return (checkPublish(LOCAL_PUBLISHER)
                | ( !proxies && checkPublish(FTP_PUBLISHER))
                | checkPublish(ZIP_PUBLISHER)) && checkSaveSession();
        }
        catch (IllegalArgumentException ex) {
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
    public void checkPublish() {
        enableFinishButton(checkPublish_());
    }

    /**
     * shows a message box "Unexpected Error... " :-)
     * @param ex
     */
    protected void unexpectedError(Exception ex) {
        ex.printStackTrace();
        XWindowPeer peer = xControl.getPeer();
        AbstractErrorHandler.showMessage(xMSF,peer,resources.resErrUnexpected, ErrorHandler.ERROR_PROCESS_FATAL);
    }

    /**
     * substitutes path variables with the corresponding values.
     * @param path a path, which might contain OOo path variables.
     * @return the path, after substituing path variables.
     */
    protected String substitute(String path) {
        try {
            return xStringSubstitution.substituteVariables(path,false);
        }
        catch (Exception ex) {
            return path;
        }
    }



}
