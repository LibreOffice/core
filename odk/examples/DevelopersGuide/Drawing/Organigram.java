/*************************************************************************
 *
 *  $RCSfile: Organigram.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:21:55 $
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
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.beans.PropertyValue;

/*
 * OpenQuery.java
 *
 * Created on 6. Juli 2002, 10:25
 */

/**
 *
 * @author  dschulten
 */
public class Organigram {

    private XComponentContext xRemoteContext = null;
    private XMultiComponentFactory xRemoteServiceManager = null;

    /** Creates a new instance of OpenQuery */
    public Organigram() {
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        Organigram organigram1 = new Organigram();
        try {
            organigram1.drawOrganigram();
        }
        catch (java.lang.Exception e){
            e.printStackTrace();
        }
        finally {
            System.exit(0);
        }
    }
    public void drawOrganigram() throws java.lang.Exception {
        xRemoteServiceManager = this.getRemoteServiceManager(
                "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager");
        Object desktop = xRemoteServiceManager.createInstanceWithContext(
            "com.sun.star.frame.Desktop", xRemoteContext);
        XComponentLoader xComponentLoader = (XComponentLoader)UnoRuntime.queryInterface(
            XComponentLoader.class, desktop);

        PropertyValue[] loadProps = new PropertyValue[0];
        XComponent xDrawComponent = xComponentLoader.loadComponentFromURL("private:factory/sdraw", "_blank", 0, loadProps);

        // get draw page by index
        com.sun.star.drawing.XDrawPagesSupplier xDrawPagesSupplier = (com.sun.star.drawing.XDrawPagesSupplier)UnoRuntime.queryInterface(
            com.sun.star.drawing.XDrawPagesSupplier.class, xDrawComponent );
        com.sun.star.drawing.XDrawPages xDrawPages = xDrawPagesSupplier.getDrawPages();
        Object drawPage = xDrawPages.getByIndex(0);
        com.sun.star.drawing.XDrawPage xDrawPage = (com.sun.star.drawing.XDrawPage)
            UnoRuntime.queryInterface(com.sun.star.drawing.XDrawPage.class, drawPage);

        com.sun.star.lang.XMultiServiceFactory xDocumentFactory = (com.sun.star.lang.XMultiServiceFactory)
            UnoRuntime.queryInterface(
                com.sun.star.lang.XMultiServiceFactory.class, xDrawComponent);

        com.sun.star.beans.XPropertySet xPageProps = (com.sun.star.beans.XPropertySet)
            UnoRuntime.queryInterface(
                com.sun.star.beans.XPropertySet.class, xDrawPage);

        int pageWidth = AnyConverter.toInt(xPageProps.getPropertyValue("Width"));
        int pageHeight = AnyConverter.toInt(xPageProps.getPropertyValue("Height"));
        int pageBorderTop = AnyConverter.toInt(xPageProps.getPropertyValue("BorderTop"));
        int pageBorderLeft = AnyConverter.toInt(xPageProps.getPropertyValue("BorderLeft"));
        int pageBorderRight = AnyConverter.toInt(xPageProps.getPropertyValue("BorderRight"));
        int drawWidth = pageWidth - pageBorderLeft - pageBorderRight;
        int horCenter = pageBorderLeft + drawWidth / 2;

        String[][] orgUnits = new String[2][4];
        orgUnits[0][0] = "Management";
        orgUnits[1][0] = "Production";
        orgUnits[1][1] = "Purchasing";
        orgUnits[1][2] = "IT Services";
        orgUnits[1][3] = "Sales";
        int[] levelCount = {1, 4};

        int horSpace = 300;
        int verSpace = 3000;

        int shapeWidth = (drawWidth - (levelCount[1] - 1) * horSpace) / levelCount[1];
        int shapeHeight = pageHeight / 20;
        int shapeX = pageWidth / 2 - shapeWidth / 2;
        int shapeY = pageBorderTop;

        int levelY;
        int levelX;

        com.sun.star.drawing.XShape xStartShape = null;

        for (int level = 0; level <= 1; level++) {
            levelY = pageBorderTop + 2000 + level * (shapeHeight + verSpace);
            for (int i = levelCount[level] - 1; i > -1; i--) {
                shapeX = horCenter - (levelCount[level] * shapeWidth + (levelCount[level] - 1) * horSpace) / 2 + i * shapeWidth + i * horSpace;
                Object shape = xDocumentFactory.createInstance("com.sun.star.drawing.RectangleShape");
                com.sun.star.drawing.XShape xShape = (com.sun.star.drawing.XShape)
                    UnoRuntime.queryInterface(
                        com.sun.star.drawing.XShape.class, shape);
                xShape.setPosition(new com.sun.star.awt.Point(shapeX, levelY));
                xShape.setSize(new com.sun.star.awt.Size(shapeWidth, shapeHeight));
                xDrawPage.add(xShape);

                com.sun.star.text.XText xText = (com.sun.star.text.XText)
                    UnoRuntime.queryInterface(
                        com.sun.star.text.XText.class, xShape);

                xText.setString(orgUnits[level][i]);

                // memorize the root shape
                if (level == 0 && i == 0)
                    xStartShape = xShape;

                if (level == 1) {
                    Object connector = xDocumentFactory.createInstance("com.sun.star.drawing.ConnectorShape");
                    com.sun.star.beans.XPropertySet xConnectorProps = (com.sun.star.beans.XPropertySet)
                        UnoRuntime.queryInterface(
                            com.sun.star.beans.XPropertySet.class, connector);
                com.sun.star.drawing.XShape xConnector = (com.sun.star.drawing.XShape)
                    UnoRuntime.queryInterface(
                        com.sun.star.drawing.XShape.class, connector);
                    xDrawPage.add(xConnector);
                    xConnectorProps.setPropertyValue("StartShape", xStartShape);
                    xConnectorProps.setPropertyValue("EndShape", xShape);
                    xConnectorProps.setPropertyValue("StartGluePointIndex", new Integer(2)); // 2 = bottom glue point
                    xConnectorProps.setPropertyValue("EndGluePointIndex", new Integer(0));   // 0 = top glue point
                }

            }
        }
    }



    protected XMultiComponentFactory getRemoteServiceManager(String unoUrl) throws java.lang.Exception {
        if (xRemoteContext == null) {
            // First step: create local component context, get local servicemanager and
            // ask it to create a UnoUrlResolver object with an XUnoUrlResolver interface
            XComponentContext xLocalContext =
            com.sun.star.comp.helper.Bootstrap.createInitialComponentContext(null);

            XMultiComponentFactory xLocalServiceManager = xLocalContext.getServiceManager();

            Object urlResolver  = xLocalServiceManager.createInstanceWithContext(
            "com.sun.star.bridge.UnoUrlResolver", xLocalContext );
            // query XUnoUrlResolver interface from urlResolver object
            XUnoUrlResolver xUnoUrlResolver = (XUnoUrlResolver) UnoRuntime.queryInterface(
            XUnoUrlResolver.class, urlResolver );
            //XUnoUrlResolver xUnoUrlResolver = (XUnoUrlResolver)urlResolver;

            // Second step: use xUrlResolver interface to import the remote StarOffice.ServiceManager,
            // retrieve its property DefaultContext and get the remote servicemanager
            Object initialObject = xUnoUrlResolver.resolve( unoUrl );
            XPropertySet xPropertySet = (XPropertySet)UnoRuntime.queryInterface(
            XPropertySet.class, initialObject);
            Object context = xPropertySet.getPropertyValue("DefaultContext");
            xRemoteContext = (XComponentContext)UnoRuntime.queryInterface(
            XComponentContext.class, context);
        }
        return xRemoteContext.getServiceManager();
    }

}
