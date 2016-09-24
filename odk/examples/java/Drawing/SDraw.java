/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


// comment: Step 1: bootstrap UNO and get the remote component context
//          Step 2: open an empty text document
//          Step 3: get the drawpage an insert some shapes



import com.sun.star.uno.UnoRuntime;

public class SDraw  {


    public static void main(String args[]) {

        //oooooooooooooooooooooooooooStep 1oooooooooooooooooooooooooooooooooooooooo
        // bootstrap UNO and get the remote component context. The context can
        // be used to get the service manager

        com.sun.star.uno.XComponentContext xContext = null;

        try {
            // get the remote office component context
            xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            System.out.println("Connected to a running office ...");
        }
        catch( Exception e) {
            e.printStackTrace(System.err);
            System.exit(1);
        }

        com.sun.star.lang.XComponent xDrawDoc = null;
        com.sun.star.drawing.XDrawPage xDrawPage = null;

        //oooooooooooooooooooooooooooStep 2oooooooooooooooooooooooooooooooooooooooo
        // open an empty document. In this case it's a draw document.
        // For this purpose an instance of com.sun.star.frame.Desktop
        // is created. It's interface XDesktop provides the XComponentLoader,
        // which is used to open the document via loadComponentFromURL


        //Open document
        //Draw
        System.out.println("Opening an empty Draw document ...");
        xDrawDoc = openDraw(xContext);

        //oooooooooooooooooooooooooooStep 3oooooooooooooooooooooooooooooooooooooooo
        // get the drawpage an insert some shapes.
        // the documents DrawPageSupplier supplies the DrawPage vi IndexAccess
        // To add a shape get the MultiServiceFaktory of the document, create an
        // instance of the ShapeType and add it to the Shapes-container
        // provided by the drawpage



        // get the drawpage of drawing here
        try {
            System.out.println( "getting Drawpage" );
            com.sun.star.drawing.XDrawPagesSupplier xDPS =
                UnoRuntime.queryInterface(
                com.sun.star.drawing.XDrawPagesSupplier.class, xDrawDoc);
            com.sun.star.drawing.XDrawPages xDPn = xDPS.getDrawPages();
            com.sun.star.container.XIndexAccess xDPi =
                UnoRuntime.queryInterface(
                com.sun.star.container.XIndexAccess.class, xDPn);
            xDrawPage = UnoRuntime.queryInterface(
                com.sun.star.drawing.XDrawPage.class, xDPi.getByIndex(0));
        } catch ( Exception e ) {
            System.err.println( "Couldn't create document"+ e );
            e.printStackTrace(System.err);
        }

        createSequence(xDrawDoc, xDrawPage);

        //put something on the drawpage
        System.out.println( "inserting some Shapes" );
        com.sun.star.drawing.XShapes xShapes = UnoRuntime.queryInterface(
            com.sun.star.drawing.XShapes.class, xDrawPage);
        xShapes.add(createShape(xDrawDoc,2000,1500,1000,1000,"Line",0));
        xShapes.add(createShape(xDrawDoc,3000,4500,15000,1000,"Ellipse",16711680));
        xShapes.add(createShape(xDrawDoc,5000,3500,7500,5000,"Rectangle",6710932));


        System.out.println("done");
        System.exit(0);
    }

    public static com.sun.star.lang.XComponent openDraw(
        com.sun.star.uno.XComponentContext xContext)
    {
        com.sun.star.frame.XComponentLoader xCLoader;
        com.sun.star.lang.XComponent xComp = null;

        try {
            // get the remote office service manager
            com.sun.star.lang.XMultiComponentFactory xMCF =
                xContext.getServiceManager();

            Object oDesktop = xMCF.createInstanceWithContext(
                                        "com.sun.star.frame.Desktop", xContext);

            xCLoader = UnoRuntime.queryInterface(com.sun.star.frame.XComponentLoader.class,
                                      oDesktop);
            com.sun.star.beans.PropertyValue szEmptyArgs[] =
                new com.sun.star.beans.PropertyValue[0];
            String strDoc = "private:factory/sdraw";
            xComp = xCLoader.loadComponentFromURL(strDoc, "_blank", 0, szEmptyArgs);

        } catch(Exception e){
            System.err.println(" Exception " + e);
            e.printStackTrace(System.err);
        }

        return xComp;
    }

    public static com.sun.star.drawing.XShape createShape(
        com.sun.star.lang.XComponent xDocComp, int height, int width, int x,
        int y, String kind, int col)
    {
        //possible values for kind are 'Ellipse', 'Line' and 'Rectangle'
        com.sun.star.awt.Size size = new com.sun.star.awt.Size();
        com.sun.star.awt.Point position = new com.sun.star.awt.Point();
        com.sun.star.drawing.XShape xShape = null;

        //get MSF
        com.sun.star.lang.XMultiServiceFactory xDocMSF =
            UnoRuntime.queryInterface(
            com.sun.star.lang.XMultiServiceFactory.class, xDocComp );

        try {
            Object oInt = xDocMSF.createInstance("com.sun.star.drawing."
                                                 +kind + "Shape");
            xShape = UnoRuntime.queryInterface(
                com.sun.star.drawing.XShape.class, oInt);
            size.Height = height;
            size.Width = width;
            position.X = x;
            position.Y = y;
            xShape.setSize(size);
            xShape.setPosition(position);

        } catch ( Exception e ) {
            System.err.println( "Couldn't create instance "+ e );
            e.printStackTrace(System.err);
        }

        com.sun.star.beans.XPropertySet xSPS = UnoRuntime.queryInterface(
            com.sun.star.beans.XPropertySet.class, xShape);

        try {
            xSPS.setPropertyValue("FillColor", Integer.valueOf(col));
        } catch (Exception e) {
            System.err.println("Can't change colors " + e);
            e.printStackTrace(System.err);
        }

        return xShape;
    }

    public static com.sun.star.drawing.XShape createSequence(
        com.sun.star.lang.XComponent xDocComp, com.sun.star.drawing.XDrawPage xDP)
    {
        com.sun.star.awt.Size size = new com.sun.star.awt.Size();
        com.sun.star.awt.Point position = new com.sun.star.awt.Point();
        com.sun.star.drawing.XShape xShape = null;
        com.sun.star.drawing.XShapes xShapes = UnoRuntime.queryInterface(com.sun.star.drawing.XShapes.class, xDP);
        int height = 3000;
        int width = 3500;
        int x = 1900;
        int y = 20000;
        Object oInt = null;
        int r = 40;
        int g = 0;
        int b = 80;

        //get MSF
        com.sun.star.lang.XMultiServiceFactory xDocMSF =
            UnoRuntime.queryInterface(
            com.sun.star.lang.XMultiServiceFactory.class, xDocComp );

        for (int i=0; i<370; i=i+25) {
            try{
                oInt = xDocMSF.createInstance("com.sun.star.drawing.EllipseShape");
                xShape = UnoRuntime.queryInterface(
                    com.sun.star.drawing.XShape.class, oInt);
                size.Height = height;
                size.Width = width;
                position.X = (x+(i*40));
                position.Y =
                    (new Float(y+(Math.sin((i*Math.PI)/180))*5000)).intValue();
                xShape.setSize(size);
                xShape.setPosition(position);

            } catch ( Exception e ) {
                // Some exception occurs.FAILED
                System.err.println( "Couldn't get Shape "+ e );
                e.printStackTrace(System.err);
            }

            b=b+8;

            com.sun.star.beans.XPropertySet xSPS = UnoRuntime.queryInterface(com.sun.star.beans.XPropertySet.class,
                                      xShape);

            try {
                xSPS.setPropertyValue("FillColor", Integer.valueOf(getCol(r,g,b)));
                xSPS.setPropertyValue("Shadow", Boolean.TRUE);
            } catch (Exception e) {
                System.err.println("Can't change colors " + e);
                e.printStackTrace(System.err);
            }
            xShapes.add(xShape);
        }

        com.sun.star.drawing.XShapeGrouper xSGrouper =
            UnoRuntime.queryInterface(
            com.sun.star.drawing.XShapeGrouper.class, xDP);

        xShape = xSGrouper.group(xShapes);

        return xShape;
    }

    public static int getCol(int r, int g, int b) {
        return r*65536+g*256+b;
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
