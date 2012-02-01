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
//          Step 3: enter a example text
//          Step 4: replace some english spelled words with US spelled
//***************************************************************************


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
            com.sun.star.util.XSearchDescriptor xSearchDescriptor = null;
            com.sun.star.util.XReplaceable xReplaceable = null;

            xReplaceable = (com.sun.star.util.XReplaceable)
                UnoRuntime.queryInterface(
                    com.sun.star.util.XReplaceable.class, xTextDocument);

            // You need a descriptor to set properies for Replace
            xReplaceDescr = (com.sun.star.util.XReplaceDescriptor)
                xReplaceable.createReplaceDescriptor();

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
            xTextCursor = (com.sun.star.text.XTextCursor)
                xTextDocument.getText().createTextCursor();
            com.sun.star.text.XText xText = (com.sun.star.text.XText)
                xTextDocument.getText();

            xText.insertString( xTextCursor,
                "He nervously looked all around. Suddenly he saw his ", false );

            xText.insertString( xTextCursor, "neighbour ", true );
            com.sun.star.beans.XPropertySet xCPS = (com.sun.star.beans.XPropertySet)
                UnoRuntime.queryInterface(
                    com.sun.star.beans.XPropertySet.class, xTextCursor);
            // Set the word blue
            xCPS.setPropertyValue( "CharColor", new Integer( 255 ) );
            // Go to last character
            xTextCursor.gotoEnd(false);
            xCPS.setPropertyValue( "CharColor", new Integer( 0 ) );

            xText.insertString( xTextCursor, "in the alley. Like lightening he darted off to the left and disappeared between the two warehouses almost falling over the trash can lying in the ", false  );

            xText.insertString( xTextCursor, "centre ", true );
            xCPS = (com.sun.star.beans.XPropertySet)UnoRuntime.queryInterface(
                com.sun.star.beans.XPropertySet.class, xTextCursor);
            // Set the word blue
            xCPS.setPropertyValue( "CharColor", new Integer( 255 ) );
            // Go to last character
            xTextCursor.gotoEnd(false);
            xCPS.setPropertyValue( "CharColor", new Integer( 0 ) );

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
