/*************************************************************************
*
*  $RCSfile: OfficeDocument.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: kz $ $Date: 2004-05-19 12:41:59 $
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

package com.sun.star.wizards.document;

import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.container.XNameAccess;
import com.sun.star.document.XEventsSupplier;
import com.sun.star.document.XTypeDetection;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.wizards.common.*;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.sheet.XCellRangeData;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.table.XCellRange;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStorable;
import com.sun.star.util.XCloseable;
import com.sun.star.util.XModifiable;

public class OfficeDocument {

    /** Creates a new instance of OfficeDocument */
    public OfficeDocument() {
    }

    public static void attachEventCall(XComponent xComponent, String EventName, String EventType, String EventURL) {
        try {
            XEventsSupplier xEventsSuppl = (XEventsSupplier) UnoRuntime.queryInterface(XEventsSupplier.class, xComponent);
            PropertyValue[] oEventProperties = new PropertyValue[2];
            oEventProperties[0] = new PropertyValue();
            oEventProperties[0].Name = "EventType";
            oEventProperties[0].Value = EventType; // "Service", "StarBasic"
            oEventProperties[1] = new PropertyValue();
            oEventProperties[1].Name = "Script"; //"URL";
            oEventProperties[1].Value = EventURL;
            xEventsSuppl.getEvents().replaceByName(EventName, oEventProperties);
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public static void dispose(XMultiServiceFactory xMSF, XComponent xComponent) {
    try {
        if (xComponent != null) {
            XModifiable xModified = (XModifiable) UnoRuntime.queryInterface(XModifiable.class, xComponent);
            XModel xModel = (XModel) UnoRuntime.queryInterface(XModel.class, xComponent);
            XFrame xFrame = xModel.getCurrentController().getFrame();
            if (xModified.isModified())
                xModified.setModified(false);
            Desktop.dispatchURL(xMSF, ".uno:CloseDoc", xFrame);
        }
    } catch (PropertyVetoException exception) {
        exception.printStackTrace(System.out);
    }}

    public static Object createNewDocument(XDesktop xDesktop, String sDocumentType) {
        //        XComponent xComponent = null;
        Object oDocument = null;
        PropertyValue xValues[] = new PropertyValue[1];
        com.sun.star.frame.XComponentLoader xComponentLoader = null;
        XInterface xInterface = null;
        try {
            String sURL = "private:factory/" + sDocumentType;
            PropertyValue[] xEmptyArgs = new PropertyValue[0];
            xComponentLoader = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class, xDesktop);
            com.sun.star.lang.XComponent xComponent = xComponentLoader.loadComponentFromURL(sURL, "_default", 0, xEmptyArgs);
            if (sDocumentType == "swriter")
                oDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, xComponent);
            else if (sDocumentType == "scalc")
                oDocument = (XSpreadsheetDocument) UnoRuntime.queryInterface(XSpreadsheetDocument.class, xComponent);

            //TODO:
            //                else if (sDocumentType == "simpress")
            //                else if (sDocumentType == "sdraw")
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
        return oDocument;
    }

    public static XFrame createNewFrame(XMultiServiceFactory xMSF) {
        XFrame xF = (XFrame) UnoRuntime.queryInterface(XFrame.class, Desktop.getDesktop(xMSF));
        XFrame xFrame = xF.findFrame("_blank", 0);
        return xFrame;
    }


    public static Object load(XInterface xInterface, String sURL, String sFrame, PropertyValue[] xValues) {
        //        XComponent xComponent = null;
        Object oDocument = null;
        com.sun.star.frame.XComponentLoader xComponentLoader = null;
        //XInterface xInterface = null;
        try {
            xComponentLoader = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class, xInterface);
            com.sun.star.lang.XComponent xComponent = xComponentLoader.loadComponentFromURL(sURL, sFrame, 0, xValues);

            XServiceInfo xComponentService = (XServiceInfo) UnoRuntime.queryInterface(XServiceInfo.class, xComponent);
            if (xComponentService.supportsService("com.sun.star.text.TextDocument"))
                oDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, xComponent);

            //TODO: write if clauses for Calc, Impress and Draw

        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
        return oDocument;
    }


    public static boolean store(XMultiServiceFactory xMSF, XComponent xComponent, String StorePath, String FilterName, boolean bStoreToUrl, String sMsgSavingImpossible) {
        try {
            XStorable xStoreable = (XStorable) UnoRuntime.queryInterface(XStorable.class, xComponent);
            PropertyValue[] oStoreProperties;
            if (FilterName.length() > 0) {
                oStoreProperties = new PropertyValue[1];
                oStoreProperties[0] = new PropertyValue();
                oStoreProperties[0].Name = "FilterName";
                oStoreProperties[0].Value = FilterName;
            } else
                oStoreProperties = new PropertyValue[0];
            if (bStoreToUrl == true)
                xStoreable.storeToURL(StorePath, oStoreProperties);
            else
                xStoreable.storeAsURL(StorePath, oStoreProperties);
            return true;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            SystemDialog.showMessageBox(xMSF, "ErrorBox", VclWindowPeerAttribute.OK, sMsgSavingImpossible);
            return false;
        }
    }


    public static boolean close(XComponent xComponent) {
        boolean bState = false;
        XModel xModel = (XModel) UnoRuntime.queryInterface(XModel.class, xComponent);

        if (xModel != null) {
            XCloseable xCloseable = (XCloseable) UnoRuntime.queryInterface(XCloseable.class, xModel);

            if (xCloseable != null) {
                try {
                    xCloseable.close(true);
                    bState = true;
                } catch (com.sun.star.util.CloseVetoException exCloseVeto) {
                    System.out.println("could not close doc");
                    bState = false;
                }
            } else {
                XComponent xDisposeable = (XComponent) UnoRuntime.queryInterface(XComponent.class, xModel);
                xDisposeable.dispose();
                bState = true;
            }
        }
        return bState;
    }


    public static void ArraytoCellRange(Object[][] datalist, Object oTable, int xpos, int ypos){
        try {
            int rowcount = datalist.length;
            if (rowcount > 0){
                int colcount = datalist[0].length;
                if (colcount > 0){
                    XCellRange xCellRange = (XCellRange) UnoRuntime.queryInterface(XCellRange.class, oTable);
                    XCellRange xNewRange = xCellRange.getCellRangeByPosition(xpos, ypos, (colcount+xpos)-1, (rowcount+ypos)-1);
                    XCellRangeData xDataArray = (XCellRangeData) UnoRuntime.queryInterface(XCellRangeData.class, xNewRange);
                    xDataArray.setDataArray(datalist);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }


    public static PropertyValue[] getFileMediaDecriptor(XMultiServiceFactory xmsf, String url)
        throws Exception
    {
        Object typeDetect = xmsf.createInstance("com.sun.star.document.TypeDetection");

        PropertyValue[][] mediaDescr = new PropertyValue[1][1];
        mediaDescr[0][0] = new PropertyValue();
        mediaDescr[0][0].Name = "URL";
        mediaDescr[0][0].Value = url;

        String type = ((XTypeDetection)UnoRuntime.queryInterface(XTypeDetection.class, typeDetect)).queryTypeByDescriptor(mediaDescr, true);

        XNameAccess xNameAccess = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class,typeDetect);
        if (type.equals(""))
            return null;
        else
            return (PropertyValue[]) xNameAccess.getByName(type);
    }


    public static PropertyValue[] getTypeMediaDescriptor(XMultiServiceFactory xmsf, String type)
        throws Exception
    {
        Object typeDetect = xmsf.createInstance("com.sun.star.document.TypeDetection");
        XNameAccess xNameAccess = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class,typeDetect);
        return (PropertyValue[]) xNameAccess.getByName(type);
    }

    /**
     * returns the count of slides in a presentation,
     * or the count of pages in a draw document.
     * @param model a presentation or a draw document
     * @return the number of slides/pages in the given document.
     */
    public static int getSlideCount(Object model) {
        XDrawPagesSupplier xDrawPagesSupplier = (XDrawPagesSupplier) UnoRuntime.queryInterface(XDrawPagesSupplier.class,model);
        return xDrawPagesSupplier.getDrawPages().getCount();
    }

}
