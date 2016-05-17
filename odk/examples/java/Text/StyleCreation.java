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


// comment: Step 1: get the Desktop object from the office
//          Step 2: open an empty text document
//          Step 3: create a new Paragraph style
//          Step 4: apply the Paragraph style

//          Chapter 4.1.3 Defining Your Own Style


import com.sun.star.uno.UnoRuntime;


public class StyleCreation {
    public static void main(String args[]) {
        // You need the desktop to create a document
        // The getDesktop method does the UNO bootstrapping, gets the
        // remote servie manager and the desktop object.
        com.sun.star.frame.XDesktop xDesktop = null;
        xDesktop = getDesktop();

        try {
            // create text document
            com.sun.star.text.XTextDocument xTextDocument = null;
            xTextDocument = createTextdocument(xDesktop);

            // the service '..ParagraphStyle' is context dependent, you need
            // the multi service factory from the document to use the service
            com.sun.star.lang.XMultiServiceFactory xDocMSF =
                UnoRuntime.queryInterface(
                com.sun.star.lang.XMultiServiceFactory.class, xTextDocument);

            // use the service 'com.sun.star.style.ParagraphStyle'
            com.sun.star.uno.XInterface xInterface = (com.sun.star.uno.XInterface)
                xDocMSF.createInstance("com.sun.star.style.ParagraphStyle");

            // create a supplier to get the Style family collection
            com.sun.star.style.XStyleFamiliesSupplier xSupplier =
                UnoRuntime.queryInterface(
                com.sun.star.style.XStyleFamiliesSupplier.class, xTextDocument );

            // get the NameAccess interface from the Style family collection
            com.sun.star.container.XNameAccess xNameAccess =
                xSupplier.getStyleFamilies();

            // select the Paragraph styles, you get the Paragraph style collection
            com.sun.star.container.XNameContainer xParaStyleCollection =
                UnoRuntime.queryInterface(
                com.sun.star.container.XNameContainer.class,
                xNameAccess.getByName("ParagraphStyles"));

            // create a PropertySet to set the properties for the new Paragraphstyle
            com.sun.star.beans.XPropertySet xPropertySet =
                UnoRuntime.queryInterface(
                com.sun.star.beans.XPropertySet.class, xInterface );
            System.out.println( "create a PropertySet to set the properties for the new Paragraphstyle" );

            // set some properties from the Paragraph style
            xPropertySet.setPropertyValue("CharFontName", "Helvetica" );
            System.out.println( "set name of the font to 'Helvetica'" );

            xPropertySet.setPropertyValue("CharHeight", new Float( 36 ) );
            System.out.println( "Change the height of th font to 36" );

            xPropertySet.setPropertyValue("CharWeight",
                new Float( com.sun.star.awt.FontWeight.BOLD ) );
            System.out.println( "set the font attribute 'Bold'" );

            xPropertySet.setPropertyValue("CharAutoKerning", Boolean.TRUE );
            System.out.println( "set the paragraph attribute 'AutoKerning'" );
            xPropertySet.setPropertyValue("ParaAdjust",
                Integer.valueOf( com.sun.star.style.ParagraphAdjust.CENTER_value ) );
            System.out.println( "set the paragraph adjust to LEFT" );

            xPropertySet.setPropertyValue("ParaFirstLineIndent", Integer.valueOf( 0 ) );
            System.out.println( "set the first line indent to 0 cm" );

            xPropertySet.setPropertyValue("BreakType",
                com.sun.star.style.BreakType.PAGE_AFTER );
            System.out.println( "set the paragraph attribute Breaktype to PageAfter" );

            // insert the new Paragraph style in the Paragraph style collection
            xParaStyleCollection.insertByName( "myheading", xPropertySet );
            System.out.println( "create new paragraph style, with the values from the Propertyset");

            // get the Textrange from the document
            com.sun.star.text.XTextRange xTextRange =
                xTextDocument.getText().getStart();

            // get the PropertySet from the current paragraph
            com.sun.star.beans.XPropertySet xParagraphPropertySet =
                UnoRuntime.queryInterface(
                com.sun.star.beans.XPropertySet.class, xTextRange );
            // change the value from the property 'ParaStyle' to apply the
            // Paragraph style
            // To run the sample with StarOffice 5.2 you'll have to change
            // 'ParaStyleName' to 'ParaStyle' in the next line
            xParagraphPropertySet.setPropertyValue("ParaStyleName",
                "myheading" );
            System.out.println( "apply the new paragraph style");
        }
        catch( Exception e) {
            e.printStackTrace(System.err);
            System.exit(1);
        }

        System.out.println("done");

        System.exit(0);
    }


    public static com.sun.star.frame.XDesktop getDesktop() {
        com.sun.star.frame.XDesktop xDesktop = null;
        com.sun.star.lang.XMultiComponentFactory xMCF = null;

        try {
            com.sun.star.uno.XComponentContext xContext = null;

            // get the remote office component context
            xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();

            // get the remote office service manager
            xMCF = xContext.getServiceManager();
            if( xMCF != null ) {
                System.out.println("Connected to a running office ...");

                Object oDesktop = xMCF.createInstanceWithContext(
                    "com.sun.star.frame.Desktop", xContext);
                xDesktop = UnoRuntime.queryInterface(
                    com.sun.star.frame.XDesktop.class, oDesktop);
            }
            else
                System.out.println( "Can't create a desktop. No connection, no remote office servicemanager available!" );
        }
        catch( Exception e) {
            e.printStackTrace(System.err);
            System.exit(1);
        }


        return xDesktop;
    }

    public static com.sun.star.text.XTextDocument createTextdocument(
        com.sun.star.frame.XDesktop xDesktop )
    {
        com.sun.star.text.XTextDocument aTextDocument = null;

        try {
            com.sun.star.lang.XComponent xComponent = CreateNewDocument(xDesktop,
                                                                        "swriter");
            aTextDocument = UnoRuntime.queryInterface(
                com.sun.star.text.XTextDocument.class, xComponent);
        }
        catch( Exception e) {
            e.printStackTrace(System.err);
        }

        return aTextDocument;
    }


    protected static com.sun.star.lang.XComponent CreateNewDocument(
        com.sun.star.frame.XDesktop xDesktop,
        String sDocumentType )
    {
        String sURL = "private:factory/" + sDocumentType;

        com.sun.star.lang.XComponent xComponent = null;
        com.sun.star.frame.XComponentLoader xComponentLoader = null;
        com.sun.star.beans.PropertyValue xEmptyArgs[] =
            new com.sun.star.beans.PropertyValue[0];

        try {
            xComponentLoader = UnoRuntime.queryInterface(
                com.sun.star.frame.XComponentLoader.class, xDesktop);

            xComponent  = xComponentLoader.loadComponentFromURL(
                sURL, "_blank", 0, xEmptyArgs);
        }
        catch( Exception e) {
            e.printStackTrace(System.err);
        }

        return xComponent ;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
