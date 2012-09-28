/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XComponentLoader;

import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.XComponentContext;


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
        // get the remote office component context
        xRemoteContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
        System.out.println("Connected to a running office ...");
        // get the remote service manager
        xRemoteServiceManager = xRemoteContext.getServiceManager();

        Object desktop = xRemoteServiceManager.createInstanceWithContext(
            "com.sun.star.frame.Desktop", xRemoteContext);
        XComponentLoader xComponentLoader = (XComponentLoader)
            UnoRuntime.queryInterface(XComponentLoader.class, desktop);

        PropertyValue[] loadProps = new PropertyValue[0];
        XComponent xDrawComponent = xComponentLoader.loadComponentFromURL(
            "private:factory/sdraw", "_blank", 0, loadProps);

        // get draw page by index
        com.sun.star.drawing.XDrawPagesSupplier xDrawPagesSupplier =
            (com.sun.star.drawing.XDrawPagesSupplier)UnoRuntime.queryInterface(
                com.sun.star.drawing.XDrawPagesSupplier.class, xDrawComponent );
        com.sun.star.drawing.XDrawPages xDrawPages =
            xDrawPagesSupplier.getDrawPages();
        Object drawPage = xDrawPages.getByIndex(0);
        com.sun.star.drawing.XDrawPage xDrawPage = (com.sun.star.drawing.XDrawPage)
            UnoRuntime.queryInterface(com.sun.star.drawing.XDrawPage.class,
                                      drawPage);

        com.sun.star.lang.XMultiServiceFactory xDocumentFactory =
            (com.sun.star.lang.XMultiServiceFactory)UnoRuntime.queryInterface(
                com.sun.star.lang.XMultiServiceFactory.class, xDrawComponent);

        com.sun.star.beans.XPropertySet xPageProps =
            (com.sun.star.beans.XPropertySet)UnoRuntime.queryInterface(
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
                shapeX = horCenter - (levelCount[level] * shapeWidth +
                         (levelCount[level] - 1) * horSpace) / 2
                         + i * shapeWidth + i * horSpace;
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
                    com.sun.star.beans.XPropertySet xConnectorProps =
                        (com.sun.star.beans.XPropertySet)UnoRuntime.queryInterface(
                            com.sun.star.beans.XPropertySet.class, connector);
                com.sun.star.drawing.XShape xConnector =
                    (com.sun.star.drawing.XShape)UnoRuntime.queryInterface(
                        com.sun.star.drawing.XShape.class, connector);
                    xDrawPage.add(xConnector);
                    xConnectorProps.setPropertyValue("StartShape", xStartShape);
                    xConnectorProps.setPropertyValue("EndShape", xShape);
                    xConnectorProps.setPropertyValue("StartGluePointIndex",
                                      new Integer(2)); // 2 = bottom glue point
                    xConnectorProps.setPropertyValue("EndGluePointIndex",
                                      new Integer(0));   // 0 = top glue point
                }
            }
        }
    }
}
