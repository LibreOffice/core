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

//***************************************************************************
// comment: Step 1: get the Desktop object from the office
//          Step 2: open an empty text document
//          Step 3: enter a example text
//          Step 4: get some text attributes
//          Step 5: check the PropertyState from the selection
//
//          Chapter 4.1.4 Hard formatting
//***************************************************************************

import com.sun.star.uno.UnoRuntime;

public class HardFormatting {

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

            // the text interface contains all methods and properties to
            // manipulate the content from a text document
            com.sun.star.text.XText xText = null;
            xText = xTextDocument.getText();

            String sMyText = "A very short paragraph for illustration only";

            // you can travel with the cursor throught the text document.
            // you travel only at the model, not at the view. The cursor that you can
            // see on the document doesn't change the position
            com.sun.star.text.XTextCursor xTextCursor = null;
            xTextCursor = (com.sun.star.text.XTextCursor)
                xTextDocument.getText().createTextCursor();

            xText.insertString( xTextCursor, "Headline", false );
            xText.insertControlCharacter(xTextCursor,
                      com.sun.star.text.ControlCharacter.PARAGRAPH_BREAK, false);

            xText.insertString(xTextCursor, sMyText, false);

            com.sun.star.text.XTextRange xTextRange = null;
            com.sun.star.beans.XPropertySet xPropertySet = null;

            // BEGIN: 'Hard formating'
            // the text range not the cursor contains the 'parastyle' property
            xTextRange = xText.getEnd();
            xPropertySet = (com.sun.star.beans.XPropertySet)
                UnoRuntime.queryInterface(
                    com.sun.star.beans.XPropertySet.class, xTextRange);

            // create a paragraph cursor to travel throught the paragraphs
            com.sun.star.text.XParagraphCursor xParagraphCursor = null;
            xParagraphCursor = (com.sun.star.text.XParagraphCursor)
                UnoRuntime.queryInterface(
                    com.sun.star.text.XParagraphCursor.class, xTextRange);

            xParagraphCursor.gotoStart( false );
            xParagraphCursor.gotoEndOfParagraph( true );
            xTextRange = xParagraphCursor.getText().getStart();

            // create a WordCursor to travel into the paragraph
            com.sun.star.text.XWordCursor xWordCursor = null;
            xWordCursor = (com.sun.star.text.XWordCursor) UnoRuntime.queryInterface(
                com.sun.star.text.XWordCursor.class, xTextRange);

            // the PropertySet from the cursor contains the text attributes
            xPropertySet = (com.sun.star.beans.XPropertySet)
                UnoRuntime.queryInterface(
                    com.sun.star.beans.XPropertySet.class, xWordCursor);
            System.out.println(
                "Parastyle : "
                +xPropertySet.getPropertyValue("ParaStyleName").toString()
                + "\nFontname : "
                + xPropertySet.getPropertyValue("CharFontName").toString()
                + "\nWeight : "
                + xPropertySet.getPropertyValue("CharWeight").toString() );

            xWordCursor.gotoNextWord(false);
            xWordCursor.gotoNextWord(false);
            xWordCursor.gotoEndOfWord(true);

            xPropertySet = (com.sun.star.beans.XPropertySet)
                UnoRuntime.queryInterface(
                    com.sun.star.beans.XPropertySet.class, xWordCursor);
            xPropertySet.setPropertyValue("CharWeight",
                                          new Float(com.sun.star.awt.FontWeight.BOLD));
            xPropertySet.setPropertyValue("CharColor", new Integer( 255 ) );

            System.out.println(
                "Parastyle : "
                + xPropertySet.getPropertyValue("ParaStyleName").toString()
                + "\nFontname : "
                + xPropertySet.getPropertyValue("CharFontName").toString()
                + "\nWeight : "
                + xPropertySet.getPropertyValue("CharWeight").toString() );

            // the PropertyState contains information where the attribute is set,
            // is a text part hard formated or not.
            com.sun.star.beans.XPropertyState xPropertyState = null;
            xPropertyState = (com.sun.star.beans.XPropertyState)
                UnoRuntime.queryInterface(
                    com.sun.star.beans.XPropertyState.class, xWordCursor);

            com.sun.star.beans.PropertyState xPropertyStateValue =
                xPropertyState.getPropertyState("CharWeight");

            checkPropertyState( xWordCursor, xPropertyStateValue );

            xWordCursor.goRight( (short) 3 , true );
            xPropertyStateValue = xPropertyState.getPropertyState("CharWeight");

            System.out.println("Increase the selection with three characters");
            checkPropertyState(xWordCursor, xPropertyStateValue);

            xPropertyState.setPropertyToDefault("CharWeight");

            System.out.println("Set the default value on the selection");
            xPropertyStateValue = xPropertyState.getPropertyState("CharWeight");
            checkPropertyState(xWordCursor, xPropertyStateValue);

            // END: 'Hard formating' Section from the Cookbook
        }
        catch( Exception e) {
            e.printStackTrace(System.err);
            System.exit(1);
        }


        System.out.println("Done");

        System.exit(0);

    }


    public static void checkPropertyState(
        com.sun.star.text.XWordCursor xWordCursor,
        com.sun.star.beans.PropertyState xPropertyStateValue )
    {
        switch( xPropertyStateValue.getValue() ) {
            case com.sun.star.beans.PropertyState.DIRECT_VALUE_value:  {
                System.out.println( "-> The selection '"
                                    + xWordCursor.getString()
                                    + "' completely hard formated" );
                break;
            }

            case com.sun.star.beans.PropertyState.DEFAULT_VALUE_value: {
                System.out.println( "-> The selection '"
                                    + xWordCursor.getString()
                                    + "' isn't hard formated" );
                break;
            }

            case com.sun.star.beans.PropertyState.AMBIGUOUS_VALUE_value: {
                System.out.println( "-> The selection '"
                                    + xWordCursor.getString()
                                    + "' isn't completely hard formated" );
                break;
            }

            default:
                System.out.println( "No PropertyState found" );
        }
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
