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
//          Step 4: replace some english spelled words with US spelled



import com.sun.star.uno.UnoRuntime;

public class TextReplace {

    public static void main(String args[]) {
        // You need the desktop to create a document
        // The getDesktop method does the UNO bootstrapping, gets the
        // remote servie manager and the desktop object.
        com.sun.star.frame.XDesktop xDesktop = null;
        xDesktop = getDesktop();

        com.sun.star.text.XTextDocument xTextDocument =
            createTextdocument( xDesktop );

        createExampleData( xTextDocument );

        String mBritishWords[] = {"colour", "neighbour", "centre", "behaviour",
                                  "metre", "through" };
        String mUSWords[] = { "color", "neighbor", "center", "behavior",
                              "meter", "thru" };

        try {
            com.sun.star.util.XReplaceDescriptor xReplaceDescr = null;
            com.sun.star.util.XReplaceable xReplaceable = null;

            xReplaceable = UnoRuntime.queryInterface(
                com.sun.star.util.XReplaceable.class, xTextDocument);

            // You need a descriptor to set properties for Replace
            xReplaceDescr = xReplaceable.createReplaceDescriptor();

            System.out.println("Change all occurrences of ...");
            for( int iArrayCounter = 0; iArrayCounter < mBritishWords.length;
                 iArrayCounter++ )
            {
                System.out.println(mBritishWords[iArrayCounter] +
                    " -> " + mUSWords[iArrayCounter]);
                // Set the properties the replace method need
                xReplaceDescr.setSearchString(mBritishWords[iArrayCounter] );
                xReplaceDescr.setReplaceString(mUSWords[iArrayCounter] );

                // Replace all words
                xReplaceable.replaceAll( xReplaceDescr );
            }

        }
        catch( Exception e) {
            e.printStackTrace(System.err);
        }

        System.out.println("Done");

        System.exit(0);

    }

    protected static void createExampleData(
        com.sun.star.text.XTextDocument xTextDocument )
    {
        // Create textdocument and insert example text
        com.sun.star.text.XTextCursor xTextCursor = null;

        try {
            xTextCursor = xTextDocument.getText().createTextCursor();
            com.sun.star.text.XText xText = xTextDocument.getText();

            xText.insertString( xTextCursor,
                "He nervously looked all around. Suddenly he saw his ", false );

            xText.insertString( xTextCursor, "neighbour ", true );
            com.sun.star.beans.XPropertySet xCPS = UnoRuntime.queryInterface(
                com.sun.star.beans.XPropertySet.class, xTextCursor);
            // Set the word blue
            xCPS.setPropertyValue( "CharColor", Integer.valueOf( 255 ) );
            // Go to last character
            xTextCursor.gotoEnd(false);
            xCPS.setPropertyValue( "CharColor", Integer.valueOf( 0 ) );

            xText.insertString( xTextCursor, "in the alley. Like lightning he darted off to the left and disappeared between the two warehouses almost falling over the trash can lying in the ", false  );

            xText.insertString( xTextCursor, "centre ", true );
            xCPS = UnoRuntime.queryInterface(
                com.sun.star.beans.XPropertySet.class, xTextCursor);
            // Set the word blue
            xCPS.setPropertyValue( "CharColor", Integer.valueOf( 255 ) );
            // Go to last character
            xTextCursor.gotoEnd(false);
            xCPS.setPropertyValue( "CharColor", Integer.valueOf( 0 ) );

            xText.insertString( xTextCursor, "of the sidewalk.", false );

            xText.insertControlCharacter( xTextCursor,
                      com.sun.star.text.ControlCharacter.PARAGRAPH_BREAK, false );
            xText.insertString( xTextCursor, "He tried to nervously tap his way along in the inky darkness and suddenly stiffened: it was a dead-end, he would have to go back the way he had come.", false );

            xTextCursor.gotoStart(false);
        }
        catch( Exception e) {
            e.printStackTrace(System.err);
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
