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

package org.libreoffice.example.java_scripts;

//import com.sun.star.frame.XComponentLoader;
import java.io.*;
import com.sun.star.lang.XComponent;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.XStorable;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XModel;
import com.sun.star.script.framework.runtime.XScriptContext;

// for debug only
import javax.swing.JOptionPane;

public class OfficeAttachment {

    private StatusWindow status = null;
    private XStorable storedDoc = null;
    private File htmlFile = null;
    private File officeFile = null;
    private boolean isHtmlDoc = false;
    private boolean isOfficeDoc = false;
    private String templocationURL = "";
    private String templocationSystem = "";
    private String attachmentName = "";
    private String statusLine = "";

    public OfficeAttachment(XScriptContext xsc, StatusWindow sw, boolean html,
                            boolean office) {
        status = sw;
        isHtmlDoc = html;
        isOfficeDoc = office;

        templocationSystem = templocationURL = System.getProperty("user.home");

        if (System.getProperty("os.name").indexOf("Windows") != -1) {
            while (templocationURL.indexOf("\\") != -1) {
                int sepPos = templocationURL.indexOf("\\");
                String firstPart = templocationURL.substring(0, sepPos);
                String lastPart = templocationURL.substring(sepPos + 1,
                                  templocationURL.length());
                templocationURL = firstPart + "/" + lastPart;
            }
        }

        try {
            statusLine = "Querying Office for current document";
            status.setStatus(1, statusLine);
            XScriptContext scriptcontext = xsc;
            XModel xmodel = scriptcontext.getDocument();
            storedDoc = (XStorable) UnoRuntime.queryInterface(XStorable.class, xmodel);
            // find document name from storedDoc
            attachmentName = storedDoc.getLocation();
        } catch (Exception e) {
            //UNO error
            status.setStatus(1, "Error: " + statusLine);
        }

        if (attachmentName.equalsIgnoreCase("")) {
            attachmentName = "Attachment";
        } else {
            int lastSep = attachmentName.lastIndexOf("/");
            attachmentName = attachmentName.substring(lastSep + 1, attachmentName.length());
            int dot = attachmentName.indexOf(".");
            attachmentName = attachmentName.substring(0, dot);
        }
    }


    public boolean createTempDocs() {
        String filenameURL = "file:///" + templocationURL +  "/" + attachmentName;

        try {
            if (isHtmlDoc) {
                statusLine = "Saving doc in HTML format";
                status.setStatus(4, statusLine);
                PropertyValue[] propertyvalue_html = new PropertyValue[2];
                propertyvalue_html[0] = new PropertyValue();
                propertyvalue_html[0].Name = "Overwrite";
                propertyvalue_html[0].Value = Boolean.TRUE;
                propertyvalue_html[1] = new PropertyValue();
                propertyvalue_html[1].Name = ("FilterName");
                propertyvalue_html[1].Value = "swriter: HTML (StarWriter)";
                storedDoc.storeAsURL(filenameURL + ".html", propertyvalue_html);

                File homedir = new File(templocationSystem);
                File homefiles[] = homedir.listFiles();
                String file = "";

                for (int i = 0; i < homefiles.length; i++) {
                    if (homefiles[i].getName().equals(attachmentName + ".html")) {
                        file = homefiles[i].getAbsolutePath();
                    }
                }

                htmlFile = new File(file);
            }

            if (isOfficeDoc) {
                statusLine = "Saving doc in .sxw format";
                status.setStatus(4, statusLine);
                PropertyValue[] propertyvalue_sxw = new PropertyValue[2];
                propertyvalue_sxw[0] = new PropertyValue();
                propertyvalue_sxw[0].Name = "Overwrite";
                propertyvalue_sxw[0].Value = Boolean.TRUE;
                propertyvalue_sxw[1] = new PropertyValue();
                propertyvalue_sxw[1].Name = "Overwrite";
                propertyvalue_sxw[1].Value = Boolean.TRUE;
                storedDoc.storeAsURL(filenameURL + ".sxw", propertyvalue_sxw);

                File homedir = new File(templocationSystem);

                File homefiles[] = homedir.listFiles();
                String file = "";

                for (int i = 0; i < homefiles.length; i++) {
                    if (homefiles[i].getName().equals(attachmentName + ".sxw")) {
                        file = homefiles[i].getAbsolutePath();
                    }
                }

                officeFile = new File(file);
            }

        } catch (SecurityException se) {
            status.setStatus(4, "Error: " + statusLine);
            System.out.println("Security error while saving temporary Document(s). Check file permissions in home directory.");
            se.printStackTrace();
            htmlFile = null;
            officeFile = null;
            return false;
        } catch (Exception e) {
            status.setStatus(4, "Error: " + statusLine);
            System.out.println("Error saving temporary Document(s)");
            e.printStackTrace();
            htmlFile = null;
            officeFile = null;
            return false;
        }

        return true;
    }


    public boolean removeTempDocs() {
        /*
        if( !htmlFile.exists() && !officeFile.exists() )
                {
            System.out.println("Error: Document(s) have not been saved." );
        }
        */

        statusLine = "Removing temp docs";
        status.setStatus(13, statusLine);

        try {
            if (isOfficeDoc && isHtmlDoc) {
                htmlFile.delete();
                officeFile.delete();
            } else {
                if (isOfficeDoc) {
                    officeFile.delete();
                } else {
                    htmlFile.delete();
                }
            }
        } catch (SecurityException se) {
            status.setStatus(13, "Error: " + statusLine);
            System.out.println("Security Error while deleting temporary Document(s). Check file permissions in home directory.");
            se.printStackTrace();
            return false;
        }

        return true;
    }


    public void cleanUpOnError() {
        try {
            if (isOfficeDoc && isHtmlDoc) {
                htmlFile.delete();
                officeFile.delete();
            } else {
                if (isOfficeDoc) {
                    officeFile.delete();
                } else {
                    htmlFile.delete();
                }
            }
        } catch (SecurityException se) {
            System.out.println("Security Error while deleting temporary Document(s). Check file permissions in home directory.");
            se.printStackTrace();
        }
    }


    public File[] getAttachments() {

        statusLine = "Retrieving temp docs";
        status.setStatus(8, statusLine);

        File attachments[] = null;

        if (isOfficeDoc && isHtmlDoc) {
            attachments = new File[2];
            attachments[0] = htmlFile;
            attachments[1] = officeFile;
        } else {
            if (isOfficeDoc) {
                attachments = new File[1];
                attachments[0] = officeFile;
            } else {
                attachments = new File[1];
                attachments[0] = htmlFile;
            }
        }

        return attachments;
    }


    public boolean isHtmlAttachment() {
        return isHtmlDoc;
    }


    public boolean isOfficeAttachment() {
        return isOfficeDoc;
    }

}
