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

package util;

import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;

import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.datatransfer.clipboard.*;
import com.sun.star.datatransfer.*;

public class SysUtils {

    public static String getJavaPath() {
        String cp = System.getProperty("java.class.path");
        String jh = System.getProperty("java.home");
        String fs = System.getProperty("file.separator");
        jh = jh + fs + "bin" + fs;
        jh = jh + "java -classpath "+cp;
        return jh;
    }

  static ArrayList<String> files = new ArrayList<String>();

  public static Object[] traverse( String afileDirectory ) {

    File fileDirectory = new File(afileDirectory);
    // Testing, if the file is a directory, and if so, it throws an exception
    if ( !fileDirectory.isDirectory() ) {
      throw new IllegalArgumentException(
      "not a directory: " + fileDirectory.getName()
      );
    }

    // Getting all files and directories in the current directory
    File[] entries = fileDirectory.listFiles(
    new FileFilter() {
      public boolean accept( File pathname ) {
        return true;
      }
    }
    );

    // Iterating for each file and directory
    for ( int i = 0; i < entries.length; ++i ) {
      // Testing, if the entry in the list is a directory
      if ( entries[ i ].isDirectory() ) {
        // Recursive call for the new directory
        traverse( entries[ i ].getAbsolutePath() );
      } else {
        // adding file to List
        try {
          // Composing the URL by replacing all backslashs
          String stringUrl = "file:///"
          + entries[ i ].getAbsolutePath().replace( '\\', '/' );
          files.add(stringUrl);
        }
        catch( Exception exception ) {
          exception.printStackTrace();
        }

      }
    }
    return files.toArray();
  }

  public static XComponent getActiveComponent(XMultiServiceFactory msf) {
    XComponent ac = null;
    try {
        Object desk = msf.createInstance("com.sun.star.frame.Desktop");
        XDesktop xDesk = UnoRuntime.queryInterface(XDesktop.class,desk);
        ac = xDesk.getCurrentComponent();
    } catch (com.sun.star.uno.Exception e) {
        System.out.println("Couldn't get active Component");
    }
    return ac;
  }

  public static XFrame getActiveFrame(XMultiServiceFactory msf) {
    try {
        Object desk = msf.createInstance("com.sun.star.frame.Desktop");
        XDesktop xDesk = UnoRuntime.queryInterface(XDesktop.class,desk);
        return xDesk.getCurrentFrame();
    } catch (com.sun.star.uno.Exception e) {
        System.out.println("Couldn't get active Component");
    }

    return null;
  }

  /**
   * Tries to obtain text data from cliboard if such one exists.
   * The method iterates through all 'text/plain' supported data
   * flavors and returns the first non-null String value.
   *
   * @param msf MultiserviceFactory
   * @return First found string clipboard contents or null if no
   *    text contents were found.
   * @throws com.sun.star.uno.Exception if system clipboard is not accessible.
   */
  public static String getSysClipboardText(XMultiServiceFactory msf)
        throws com.sun.star.uno.Exception {

    XClipboard xCB = UnoRuntime.queryInterface
        (XClipboard.class, msf.createInstance
        ("com.sun.star.datatransfer.clipboard.SystemClipboard"));

    XTransferable xTrans = xCB.getContents();

    DataFlavor[] dfs = xTrans.getTransferDataFlavors();

    for (int i = 0; i < dfs.length; i++) {
        if (dfs[i].MimeType.startsWith("text/plain")) {
            Object data = xTrans.getTransferData(dfs[i]);
            if (data != null && data instanceof String) {
                return (String) data;
            }
        }
    }

    return null;
  }
}
