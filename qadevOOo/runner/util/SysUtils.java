/*************************************************************************
 *
 *  $RCSfile: SysUtils.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Date: 2003-01-27 16:26:51 $
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
 *
 ************************************************************************/

package util;

import java.io.InputStream;
import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;
import java.io.PrintWriter;
import java.io.LineNumberReader;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;

import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.datatransfer.clipboard.*;
import com.sun.star.datatransfer.*;

public class SysUtils {

    public static String getJavaPath() {
        String cp = (String) System.getProperty("java.class.path");
        String jh = (String) System.getProperty("java.home");
        String fs = (String) System.getProperty("file.separator");
        jh = jh + fs + "bin" + fs;
        jh = jh + "java -classpath "+cp;
        return jh;
    }

  static ArrayList files = new ArrayList();

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
        XDesktop xDesk = (XDesktop) UnoRuntime.queryInterface(XDesktop.class,desk);
        ac = xDesk.getCurrentComponent();
    } catch (com.sun.star.uno.Exception e) {
        System.out.println("Couldn't get active Component");
    }
    return ac;
  }

  public static XFrame getActiveFrame(XMultiServiceFactory msf) {
    try {
        Object desk = msf.createInstance("com.sun.star.frame.Desktop");
        XDesktop xDesk = (XDesktop) UnoRuntime.queryInterface(XDesktop.class,desk);
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

    XClipboard xCB = (XClipboard) UnoRuntime.queryInterface
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