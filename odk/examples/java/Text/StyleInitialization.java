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
//          Step 3: enter a example text
//          Step 4: use the paragraph collection
//          Step 5: apply a different paragraph style on the paragraphs


import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;

public class StyleInitialization {

    public static void main(String args[]) {
        // You need the desktop to create a document
        // The getDesktop method does the UNO bootstrapping, gets the
        // remote servie manager and the desktop object.
        com.sun.star.frame.XDesktop xDesktop = null;
        xDesktop = getDesktop();

        try {
            // BEGIN: 'Style basics' Section from the Tutorial

            // create text document
            com.sun.star.text.XTextDocument xTextDocument = null;
            xTextDocument = createTextdocument( xDesktop );

            // the text interface contains all methods and properties to
            // manipulate the content from a text document
            com.sun.star.text.XText xText = null;
            xText = xTextDocument.getText();

            String sMyText = "A very short paragraph for illustration only";

            // you can travel with the cursor through the text document.
            // you travel only at the model, not at the view. The cursor that you can
            // see on the document doesn't change the position
            com.sun.star.text.XTextCursor xTextCursor = null;
            xTextCursor = xTextDocument.getText().createTextCursor();

            com.sun.star.beans.XPropertySet oCPS = UnoRuntime.queryInterface(
                com.sun.star.beans.XPropertySet.class, xTextCursor);
            try {
                oCPS.setPropertyValue("CharFontName","Helvetica");
            }
            catch (Exception ex) {

            }

            xText.insertString( xTextCursor, "Headline", false );

            try {
                oCPS.setPropertyValue("CharFontName","Times");
            }
            catch (Exception ex) {

            }
            xText.insertControlCharacter(xTextCursor,
                      com.sun.star.text.ControlCharacter.PARAGRAPH_BREAK, false);

            xText.insertString( xTextCursor, sMyText, false );

            com.sun.star.text.XTextRange xTextRange = null;
            com.sun.star.beans.XPropertySet xPropertySet = null;

            // the text range not the cursor contains the 'parastyle' property
            xTextRange = xText.getEnd();
            xPropertySet = UnoRuntime.queryInterface(
                com.sun.star.beans.XPropertySet.class, xTextRange );

            // To run the sample with StarOffice 5.2 you'll have to change
            // 'ParaStyleName' to 'ParaStyle' in the next line
            System.out.println( "Current Parastyle : "
                                + xPropertySet.getPropertyValue("ParaStyleName") );

            // END: 'Style basics' Section from the Tutorial

            // There are two way to travel through the paragraphs, with a
            // paragraph cursor, or a enumeration.
            // You find both ways in this example

            // The first way, with the paragraph cursor
            com.sun.star.text.XParagraphCursor xParagraphCursor = null;
            xParagraphCursor = UnoRuntime.queryInterface(
                com.sun.star.text.XParagraphCursor.class, xTextRange );

            xParagraphCursor.gotoStart( false );
            xParagraphCursor.gotoEndOfParagraph( true );

            // The second way, with the paragraph enumeration
            com.sun.star.container.XEnumerationAccess xEnumerationAccess = null;
            xEnumerationAccess = UnoRuntime.queryInterface(
                com.sun.star.container.XEnumerationAccess.class, xText );

            // the enumeration contains all paragraph form the document
            com.sun.star.container.XEnumeration xParagraphEnumeration = null;
            xParagraphEnumeration = xEnumerationAccess.createEnumeration();

            com.sun.star.text.XTextContent xParagraph = null;
            com.sun.star.text.XTextRange xWord = null;

            com.sun.star.container.XEnumerationAccess xParaEnumerationAccess = null;
            com.sun.star.container.XEnumeration xPortionEnumeration = null;

            // check if a paragraph is available
            while ( xParagraphEnumeration.hasMoreElements() ) {
                // get the next paragraph
                xParagraph = UnoRuntime.queryInterface(
                    com.sun.star.text.XTextContent.class,
                    xParagraphEnumeration.nextElement());

                // you need the method getAnchor to a TextRange -> to manipulate
                // the paragraph
                String sText = xParagraph.getAnchor().getString();

                // create a cursor from this paragraph
                com.sun.star.text.XTextCursor xParaCursor = null;
                xParaCursor = xParagraph.getAnchor().getText().createTextCursor();

                // goto the start and end of the paragraph
                xParaCursor.gotoStart( false );
                xParaCursor.gotoEnd( true );

                // The enumeration from the paragraphs contain parts from the
                // paragraph with a different attributes.
                xParaEnumerationAccess = UnoRuntime.queryInterface(
                    com.sun.star.container.XEnumerationAccess.class, xParagraph);
                xPortionEnumeration = xParaEnumerationAccess.createEnumeration();

                while ( xPortionEnumeration.hasMoreElements() ) {
                    // output of all parts from the paragraph with different attributes
                    xWord = UnoRuntime.queryInterface(
                        com.sun.star.text.XTextRange.class,
                        xPortionEnumeration.nextElement());
                    String sWordString = xWord.getString();
                    System.out.println( "Content of the paragraph : " + sWordString );
                }
            }

            // BEGIN: 'Finding a suitable style' Section from the Tutorial

            // create a supplier to get the styles-collection
            com.sun.star.style.XStyleFamiliesSupplier xSupplier = null;
            xSupplier = UnoRuntime.queryInterface(
                com.sun.star.style.XStyleFamiliesSupplier.class, xTextDocument );

            // use the name access from the collection
            com.sun.star.container.XNameAccess xNameAccess = null;
            xNameAccess = xSupplier.getStyleFamilies();

            com.sun.star.container.XNameContainer xParaStyleCollection = null;
            xParaStyleCollection = UnoRuntime.queryInterface(
                com.sun.star.container.XNameContainer.class, xNameAccess.getByName( "ParagraphStyles" ));

            // create a array from strings with the name of all paragraph styles from the text document
            String[] sElementNames = xParaStyleCollection.getElementNames();
            int iElementCount = sElementNames.length;

            for( int iCounter = 0;  iCounter < iElementCount; iCounter++ ) {
                // specify one paragraph style
                com.sun.star.style.XStyle xStyle = null;
                xStyle = UnoRuntime.queryInterface(
                    com.sun.star.style.XStyle.class,
                    xParaStyleCollection.getByName( sElementNames[iCounter] ));

                // create a property set of all properties from the style
                xPropertySet = UnoRuntime.queryInterface(
                    com.sun.star.beans.XPropertySet.class, xStyle );

                String sFontname = AnyConverter.toString(xPropertySet.getPropertyValue("CharFontName"));
                sFontname = sFontname.toLowerCase();

                // if the style use the font 'Albany', apply it to the current paragraph
                if( sFontname.equals("albany") ) {
                    // create a property set from the current paragraph, to change the paragraph style
                    xPropertySet = UnoRuntime.queryInterface(
                        com.sun.star.beans.XPropertySet.class, xTextRange );

                    // To run the sample with StarOffice 5.2 you'll have to change 'ParaStyleName'
                    // to 'ParaStyle' in the next line
                    xPropertySet.setPropertyValue("ParaStyleName", sElementNames[iCounter] );
                    System.out.println( "Apply the paragraph style : " + sElementNames[iCounter] );
                    break;
                }
            }
            // END: 'Finding a suitable style' Section from the Tutorial
        }
        catch( Exception e) {
            e.printStackTrace(System.err);
        }


        System.out.println("Done");

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
