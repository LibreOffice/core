/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ResourceManager.java,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package org.openoffice.setup;

import org.openoffice.setup.SetupData.SetupDataProvider;
import java.io.File;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Locale;
import java.util.MissingResourceException;
import java.util.PropertyResourceBundle;
import java.util.ResourceBundle;
import javax.swing.ImageIcon;

public class ResourceManager {

    static PropertyResourceBundle stringResourceBundle;
    static PropertyResourceBundle fileNameResourceBundle;
    static HashMap setupFiles = new HashMap();  // required, because it is not possible to set values in fileNameResourceBundle

    private ResourceManager() {
    }

    static public void checkFileExistence(File htmlDirectory) {

        for (Enumeration e = fileNameResourceBundle.getKeys(); e.hasMoreElements(); ) {
            String key = (String) e.nextElement();
            String fileName = (String)(fileNameResourceBundle.getObject(key));

            if ( ! fileName.endsWith("html") ) {
                // no check of existence for non-html files
                setupFiles.put(key, fileName);
                // System.err.println("Using file: " + fileName);
            }

            if ( fileName.endsWith("html") ) {
                boolean fileExists = true;

                File file = new File(htmlDirectory, fileName);
                File newFile = null;

                if ( file.exists() ) {
                    setupFiles.put(key, fileName);
                    // System.err.println("Using file: " + fileName);
                } else {
                    fileExists = false;
                    // try to use english version
                    int pos1 = fileName.lastIndexOf("_");

                    if ( pos1 > 0 ) {
                        int pos2 = fileName.lastIndexOf(".");
                        String newFileName = fileName.substring(0, pos1) + fileName.substring(pos2, fileName.length());
                        newFile = new File(htmlDirectory, newFileName);
                        if ( newFile.exists() ) {
                            fileExists = true;
                            setupFiles.put(key, newFileName);
                            // System.err.println("Using file: " + fileName);
                        } else {
                            // Introducing fallback to a very special simple html page
                            String simplePage = "Excuse.html";
                            File simpleFile = new File(htmlDirectory, simplePage);
                            if ( simpleFile.exists() ) {
                                fileExists = true;
                                setupFiles.put(key, simplePage);
                                // System.err.println("Using file: " + fileName);
                            }
                        }
                    }
                }

                if ( ! fileExists ) {
                    if ( newFile != null ) {
                        System.err.println("ERROR: Neither file \"" + file.getPath() +
                                           "\" nor file \"" + newFile.getPath() + "\" do exist!");
                    } else {
                        System.err.println("ERROR: File \"" + file.getPath() + "\" does not exist!");
                    }
                    System.exit(1);
                }
            }
        }
    }

    static public String getString(String key) {
        String value = (String)(stringResourceBundle.getObject(key));
        if (value != null && (value.indexOf('$') >= 0)) {
            value = SetupDataProvider.replaceMacros(value);
        }
        return value;
    }

    static public String getFileName(String key) {
        String value = (String)setupFiles.get(key);
        // String value = (String)(fileNameResourceBundle.getObject(key));
        return value;
    }

    static public ImageIcon getIcon(String key) {

        String name = getFileName(key);

        try {
            Class c = Class.forName("org.openoffice.setup.ResourceManager");
            URL url = c.getResource(name);
            if (url != null) {
                return new ImageIcon(url);
            } else {
                System.err.println("Error: file not found: " + name);
            }
        } catch (ClassNotFoundException e) {
            System.err.println(e);
        }

        return new ImageIcon();
    }

    static public ImageIcon getIconFromPath(File file) {

        try {
            URL url = file.toURL();
            if (url != null) {
                return new ImageIcon(url);
            } else {
                System.err.println("Error: file not found: " + file.getPath());
            }
        } catch (MalformedURLException e) {
            System.err.println(e);
        }

        return new ImageIcon();
    }

    static {
        Locale locale = Locale.getDefault();
        System.err.println("System locale: " + locale );
        try {
            stringResourceBundle = (PropertyResourceBundle) ResourceBundle.getBundle("org.openoffice.setup.setupstrings", locale);
            fileNameResourceBundle = (PropertyResourceBundle) ResourceBundle.getBundle("org.openoffice.setup.setupfiles", locale);
        } catch (MissingResourceException ex) {
            ex.printStackTrace();
            System.exit(1);
        }
    }
}
