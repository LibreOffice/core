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
//          Step 3: create an enumeration of all paragraphs
//          Step 4: create an enumeration of all text portions


import com.sun.star.uno.UnoRuntime;

public class TextDocumentStructure {

    public static void main(String args[]) {
        com.sun.star.uno.XComponentContext xContext = null;

        try {
            // get the remote office component context
            xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            System.out.println("Connected to a running office ...");

            // get the remote service manager
            com.sun.star.lang.XMultiComponentFactory xMCF =
                xContext.getServiceManager();

            // create a new instance of the desktop
            Object oDesktop = xMCF.createInstanceWithContext(
                                        "com.sun.star.frame.Desktop", xContext);

            // get the component laoder from the desktop to create a new
            // text document
            com.sun.star.frame.XComponentLoader xCLoader =
                UnoRuntime.queryInterface(
                com.sun.star.frame.XComponentLoader.class,oDesktop);
            com.sun.star.beans.PropertyValue [] szEmptyArgs =
                new com.sun.star.beans.PropertyValue [0];
            String strDoc = "private:factory/swriter";

            System.out.println("create new text document");

            com.sun.star.lang.XComponent xComp = xCLoader.loadComponentFromURL(
                strDoc, "_blank", 0, szEmptyArgs);

            // query the new document for the XTextDocument interface
            com.sun.star.text.XTextDocument xTextDocument =
                UnoRuntime.queryInterface(
                com.sun.star.text.XTextDocument.class, xComp);

            // create some example data
            com.sun.star.text.XText xText = xTextDocument.getText();
            createExampleData( xText );

            // Begin section 'The structure of text documents' of the Tutorial

            com.sun.star.container.XEnumeration xParagraphEnumeration = null;
            com.sun.star.container.XEnumerationAccess xParaEnumerationAccess = null;
            com.sun.star.container.XEnumeration xTextPortionEnum;
            com.sun.star.text.XTextContent xTextElement = null;

            System.out.println("create an enumeration of all paragraphs");
            // create an enumeration access of all paragraphs of a document
            com.sun.star.container.XEnumerationAccess xEnumerationAccess =
                UnoRuntime.queryInterface(
                com.sun.star.container.XEnumerationAccess.class, xText);
            xParagraphEnumeration = xEnumerationAccess.createEnumeration();

            // Loop through all paragraphs of the document
            while ( xParagraphEnumeration.hasMoreElements() ) {
                xTextElement = UnoRuntime.queryInterface(
                    com.sun.star.text.XTextContent.class,
                    xParagraphEnumeration.nextElement());
                com.sun.star.lang.XServiceInfo xServiceInfo =
                    UnoRuntime.queryInterface(
                    com.sun.star.lang.XServiceInfo.class, xTextElement);

                // check ifs the current paragraph really a paragraph or an
                // anchor of a frame or picture
                if( xServiceInfo.supportsService("com.sun.star.text.Paragraph") ) {
                    com.sun.star.text.XTextRange xTextRange =
                        xTextElement.getAnchor();
                    System.out.println( "This is a Paragraph" );

                    // create another enumeration to get all text portions of
                    // the paragraph
                    xParaEnumerationAccess =
                        UnoRuntime.queryInterface(
                            com.sun.star.container.XEnumerationAccess.class,
                            xTextElement);
                    xTextPortionEnum = xParaEnumerationAccess.createEnumeration();

                    while ( xTextPortionEnum.hasMoreElements() ) {
                        com.sun.star.text.XTextRange xTextPortion =
                            UnoRuntime.queryInterface(
                            com.sun.star.text.XTextRange.class,
                            xTextPortionEnum.nextElement());
                        System.out.println( "Text from the portion : "
                                            + xTextPortion.getString() );

                        com.sun.star.beans.XPropertySet xPropertySet =
                            UnoRuntime.queryInterface(
                             com.sun.star.beans.XPropertySet.class,
                             xTextPortion);
                        System.out.println( "Name of the font : "
                            + xPropertySet.getPropertyValue( "CharFontName" ) );

                        // PropertyState status of each text portion.
                        com.sun.star.beans.XPropertyState xPropertyState =
                            UnoRuntime.queryInterface(
                            com.sun.star.beans.XPropertyState.class,
                            xTextPortion);

                        if( xPropertyState.getPropertyState("CharWeight").equals(
                                com.sun.star.beans.PropertyState.AMBIGUOUS_VALUE) )
                            System.out.println( "-  The text range contains more than one different attributes" );

                        if( xPropertyState.getPropertyState( "CharWeight" ).equals(
                                com.sun.star.beans.PropertyState.DIRECT_VALUE ) )
                            System.out.println( " - The text range contains hard formats" );

                        if( xPropertyState.getPropertyState( "CharWeight" ).equals(
                                com.sun.star.beans.PropertyState.DEFAULT_VALUE ) )
                            System.out.println( " - The text range doesn't contains hard formats" );
                    }
                }
                else
                    System.out.println( "The text portion isn't a text paragraph" );
                // End section 'The structure of text documents' of the Tutorial
            }
        }
        catch( Exception e) {
            e.printStackTrace(System.err);
            System.exit(1);
        }

        System.out.println("done");
        System.exit(0);
    }

    public static void createExampleData( com.sun.star.text.XText xText ) {

        try {
            xText.setString( "This is an example sentence" );

            com.sun.star.text.XWordCursor xWordCursor =
                UnoRuntime.queryInterface(
                com.sun.star.text.XWordCursor.class, xText.getStart());

            xWordCursor.gotoNextWord(false);
            xWordCursor.gotoNextWord(false);
            xWordCursor.gotoEndOfWord(true);

            com.sun.star.beans.XPropertySet xPropertySet =
                UnoRuntime.queryInterface(
                com.sun.star.beans.XPropertySet.class, xWordCursor );
            xPropertySet.setPropertyValue("CharWeight",
                             new Float( com.sun.star.awt.FontWeight.BOLD ));

            System.out.println("create example data");
        }
        catch( Exception e) {
            e.printStackTrace(System.err);
        }


    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
