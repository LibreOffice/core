/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



//***************************************************************************
// comment: Step 1: get the Desktop object from the office
//          Step 2: open an empty text document
//          Step 3: create a new Paragraph style
//          Step 4: apply the Paragraph style
//
//          Chapter 4.1.3 Defining Your Own Style
//***************************************************************************

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

            // the service '..ParagraphStyle' is context dependend, you need
            // the multi service factory from the document to use the service
            com.sun.star.lang.XMultiServiceFactory xDocMSF =
                (com.sun.star.lang.XMultiServiceFactory)UnoRuntime.queryInterface(
                    com.sun.star.lang.XMultiServiceFactory.class, xTextDocument);

            // use the service 'com.sun.star.style.ParagraphStyle'
            com.sun.star.uno.XInterface xInterface = (com.sun.star.uno.XInterface)
                xDocMSF.createInstance("com.sun.star.style.ParagraphStyle");

            // create a supplier to get the Style family collection
            com.sun.star.style.XStyleFamiliesSupplier xSupplier =
                (com.sun.star.style.XStyleFamiliesSupplier)UnoRuntime.queryInterface(
                    com.sun.star.style.XStyleFamiliesSupplier.class, xTextDocument );

            // get the NameAccess interface from the Style family collection
            com.sun.star.container.XNameAccess xNameAccess =
                xSupplier.getStyleFamilies();

            // select the Paragraph styles, you get the Paragraph style collection
            com.sun.star.container.XNameContainer xParaStyleCollection =
                (com.sun.star.container.XNameContainer) UnoRuntime.queryInterface(
                    com.sun.star.container.XNameContainer.class,
                    xNameAccess.getByName("ParagraphStyles"));

            // create a PropertySet to set the properties for the new Paragraphstyle
            com.sun.star.beans.XPropertySet xPropertySet =
                (com.sun.star.beans.XPropertySet) UnoRuntime.queryInterface(
                    com.sun.star.beans.XPropertySet.class, xInterface );
            System.out.println( "create a PropertySet to set the properties for the new Paragraphstyle" );

            // set some properties from the Paragraph style
            xPropertySet.setPropertyValue("CharFontName", new String( "Helvetica" ) );
            System.out.println( "set name of the font to 'Helvetica'" );

            xPropertySet.setPropertyValue("CharHeight", new Float( 36 ) );
            System.out.println( "Change the height of th font to 36" );

            xPropertySet.setPropertyValue("CharWeight",
                new Float( com.sun.star.awt.FontWeight.BOLD ) );
            System.out.println( "set the font attribute 'Bold'" );

            xPropertySet.setPropertyValue("CharAutoKerning", new Boolean( true ) );
            System.out.println( "set the paragraph attribute 'AutoKerning'" );
            xPropertySet.setPropertyValue("ParaAdjust",
                new Integer( com.sun.star.style.ParagraphAdjust.CENTER_value ) );
            System.out.println( "set the paragraph adjust to LEFT" );

            xPropertySet.setPropertyValue("ParaFirstLineIndent", new Integer( 0 ) );
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
                (com.sun.star.beans.XPropertySet)UnoRuntime.queryInterface(
                    com.sun.star.beans.XPropertySet.class, xTextRange );
            // change the value from the property 'ParaStyle' to apply the
            // Paragraph style
            // To run the sample with StarOffice 5.2 you'll have to change
            // 'ParaStyleName' to 'ParaStyle' in the next line
            xParagraphPropertySet.setPropertyValue("ParaStyleName",
                new String( "myheading" ) );
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
                xDesktop = (com.sun.star.frame.XDesktop) UnoRuntime.queryInterface(
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
            aTextDocument = (com.sun.star.text.XTextDocument)
                UnoRuntime.queryInterface(
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
        com.sun.star.beans.PropertyValue xValues[] =
            new com.sun.star.beans.PropertyValue[1];
        com.sun.star.beans.PropertyValue xEmptyArgs[] =
            new com.sun.star.beans.PropertyValue[0];

        try {
            xComponentLoader = (com.sun.star.frame.XComponentLoader)
                UnoRuntime.queryInterface(
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

