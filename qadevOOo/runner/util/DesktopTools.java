/*************************************************************************
 *
 *  $RCSfile: DesktopTools.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2005-02-02 13:59:31 $
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

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDesktop;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.UnoRuntime;

// access the implementations via names
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;
import com.sun.star.util.XModifiable;
import java.util.Vector;


/**
 * contains helper methods for the Desktop
 */
public class DesktopTools {
    /**
     * Queries the XComponentLoader
     *
     * @param xMSF the MultiServiceFactory
     * @return the gained XComponentLoader
    */
    public static XComponentLoader getCLoader(XMultiServiceFactory xMSF) {
        XDesktop oDesktop = (XDesktop) UnoRuntime.queryInterface(
                                    XDesktop.class, createDesktop(xMSF));

        XComponentLoader oCLoader = (XComponentLoader) UnoRuntime.queryInterface(
                                            XComponentLoader.class, oDesktop);

        return oCLoader;
    } // finish getCLoader

    /**
     * Creates an Instance of the Desktop service
     *
     * @param xMSF the MultiServiceFactory
     * @return the gained Object
    */
    public static Object createDesktop(XMultiServiceFactory xMSF) {
        Object oInterface;

        try {
            oInterface = xMSF.createInstance("com.sun.star.frame.Desktop");
        } catch (com.sun.star.uno.Exception e) {
            throw new IllegalArgumentException("Desktop Service not available");
        }

        return oInterface;
    } //finish createDesktop

    /**
     * returns a XEnumeration containing all components containing on the desktop
     */
    public static XEnumeration getAllComponents(XMultiServiceFactory xMSF) {
        XDesktop xDesktop = (XDesktop) UnoRuntime.queryInterface(
                                    XDesktop.class, createDesktop(xMSF));
        return xDesktop.getComponents().createEnumeration();
    }

    /*
     * returns an object arrary of all open documents
     */
    public static Object[] getAllOpenDocuments(XMultiServiceFactory xMSF) {
        Vector components = new Vector();
        XDesktop xDesktop = (XDesktop) UnoRuntime.queryInterface(
                                    XDesktop.class, createDesktop(xMSF));

        XEnumeration allComp = getAllComponents(xMSF);

        while (allComp.hasMoreElements()){
            try{
                XComponent xComponent = (XComponent) UnoRuntime.queryInterface(
                                       XComponent.class, allComp.nextElement());

                if (getDocumentType(xComponent) != null) components.add(xComponent);

            } catch (com.sun.star.container.NoSuchElementException e) {
            } catch ( com.sun.star.lang.WrappedTargetException e) {}
        }
        return components.toArray();
    }

    public static String getDocumentType(XComponent xComponent) {
        XServiceInfo sInfo = (XServiceInfo)UnoRuntime.queryInterface(
                XServiceInfo.class, xComponent);

        if (sInfo == null) {
            return "";
        } else if (sInfo.supportsService("com.sun.star.sheet.SpreadsheetDocument")) {
            return "scalc";
        } else if (sInfo.supportsService("com.sun.star.text.TextDocument")) {
            return "swriter";
        } else if (sInfo.supportsService("com.sun.star.drawing.DrawingDocument")) {
            return "sdraw";
        } else if (sInfo.supportsService("com.sun.star.formula.FormulaProperties")) {
            return "smath";
        } else {
            return null;
        }
    }


    /**
     * Opens a new document of a given kind
     * with arguments
     * @param xMSF the MultiServiceFactory
     * @return the XComponent Interface of the document
    */
    public static XComponent openNewDoc(XMultiServiceFactory xMSF, String kind,
                                        PropertyValue[] Args) {
        XComponent oDoc = null;

        try {
            oDoc = getCLoader(xMSF)
                       .loadComponentFromURL("private:factory/" + kind,
                                             "_blank", 0, Args);
        } catch (com.sun.star.uno.Exception e) {
            throw new IllegalArgumentException("Document could not be opened");
        }

        return oDoc;
    } //finish openNewDoc

    /**
     * loads a document of from a given url
     * with arguments
     * @param xMSF the MultiServiceFactory
     * @return the XComponent Interface of the document
    */
    public static XComponent loadDoc(XMultiServiceFactory xMSF, String url,
                                     PropertyValue[] Args) {
        XComponent oDoc = null;

        try {
            oDoc = getCLoader(xMSF)
                       .loadComponentFromURL(url, "_blank", 0, Args);
        } catch (com.sun.star.uno.Exception e) {
            throw new IllegalArgumentException("Document could not be loaded");
        }

        return oDoc;
    } //finish openNewDoc

    /**
     * closes a given document
     * @param DocumentToClose
     */
    public static void closeDoc(XInterface DocumentToClose) {
        String kd = System.getProperty("KeepDocument");
        if (kd != null ) {
            System.out.println("The property 'KeepDocument' is set and so the document won't be disposed");
            return;
        }
        XModifiable modified = (XModifiable) UnoRuntime.queryInterface(
                                       XModifiable.class, DocumentToClose);
        XCloseable closer = (XCloseable) UnoRuntime.queryInterface(
                                    XCloseable.class, DocumentToClose);

        try {
            modified.setModified(false);
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            System.out.println("Couldn't close document");
        } catch (com.sun.star.lang.DisposedException e) {
            System.out.println("Couldn't close document");
        } catch (java.lang.NullPointerException e) {
            System.out.println("Couldn't close document");
        } catch (com.sun.star.beans.PropertyVetoException e) {
            System.out.println("Couldn't close document");
        }
    }
}