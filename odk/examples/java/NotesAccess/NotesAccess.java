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

// Lotus Notes Domino API
import lotus.domino.NotesThread;
import lotus.domino.Session;
import lotus.domino.Database;
import lotus.domino.DocumentCollection;
import lotus.domino.Document;
import lotus.domino.NotesFactory;

import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.beans.PropertyValue;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.table.XCell;

/** This class creates an OpenOffice.org Calc spreadsheet document and fills it
 * with existing values of documents from a Lotus Notes database.
 */
public class NotesAccess implements Runnable {

    /** Host server of the Domino Directory.
     */
    static String stringHost = "";

    /** User in the host's Domino Directory.
     */
    static String stringUser = "";

    /** Password for the user in the host's Domino Directory.
     */
    static String stringPassword = "";

    /** Database with documents to get data from.
     */
    static String stringDatabase = "";

    /** Reading the arguments and constructing the thread.
     * @param argv Holding values for the host, user, and the password of the user.
     */
    public static void main( String args[] ) {
        Thread thread;

        if ( args.length < 4 ) {
            System.out.println(
                "usage: java -jar NotesAccess.jar \"<Domino Host>\" \"<User>\" " +
                "\"<Password>\" \"<Database>\"" );
            System.out.println( "\ne.g.:" );
            System.out.println(
                "java -jar NotesAccess.jar \"\" \"\" \"\" \"Stocks.nsf\"" );
            System.exit( 1 );
        }

        if ( !args[ 0 ].trim().equals( "" ) ) {
            stringHost = args[ 0 ].trim();
        }
        if ( !args[ 1 ].trim().equals( "" ) ) {
            stringUser = args[ 1 ].trim();
        }
        stringPassword = args[ 2 ].trim();

        try {
            java.io.File sourceFile = new java.io.File(args[ 3 ].trim());
            stringDatabase = sourceFile.getCanonicalPath();
        } catch (java.io.IOException e) {
            System.out.println("Error: Please check the name or path to your database file.");
            e.printStackTrace();
            System.exit( 1 );
        }

        if ( stringHost.equals( "" ) ) {
            // Initializing.
            NotesAccess notesaccess = new NotesAccess();

            // Allowing only local calls to the Domino classes.
            thread = new NotesThread( ( Runnable ) notesaccess );
        }
        else {
            // Extracting the host, user, and password.
            NotesAccess notesaccess = new NotesAccess();

            // Allowing remote calls to the Domino classes.
            thread = new Thread( ( Runnable ) notesaccess );
        }

        // Starting the thread.
        thread.start();
    }

    /** This is the default constructor without arguments.
     */
    public NotesAccess() {
    }

    /** Reading all documents from the given database and writing the data to
     * an OpenOffice.org Calc spreadsheet document.
     */
    public void run() {
        try {
            // get the remote office component context
            XComponentContext xContext =
                com.sun.star.comp.helper.Bootstrap.bootstrap();

            System.out.println("Connected to a running office ...");

            XMultiComponentFactory xMCF = xContext.getServiceManager();

            /* A desktop environment contains tasks with one or more
               frames in which components can be loaded. Desktop is the
               environment for components which can instanciate within
               frames. */
            XComponentLoader xLoader = ( XComponentLoader )
                UnoRuntime.queryInterface(XComponentLoader.class,
                    xMCF.createInstanceWithContext(
                        "com.sun.star.frame.Desktop", xContext));

            // Load a Writer document, which will be automaticly displayed
            XComponent xComponent = xLoader.loadComponentFromURL(
                "private:factory/scalc", "_blank", 0,
                new PropertyValue[0] );

            // Querying for the interface XSpreadsheetDocument
            XSpreadsheetDocument xSpreadsheetDoc =
                (XSpreadsheetDocument) UnoRuntime.queryInterface(
                    XSpreadsheetDocument.class, xComponent);

            // Getting all sheets from the spreadsheet document.
            XSpreadsheets xSpreadsheets = xSpreadsheetDoc.getSheets() ;

            // Querying for the interface XIndexAccess.
            XIndexAccess xIndexAccess = (XIndexAccess) UnoRuntime.queryInterface(
                XIndexAccess.class, xSpreadsheets);

            // Getting the first spreadsheet.
            XSpreadsheet xSpreadsheet = (XSpreadsheet) UnoRuntime.queryInterface(
                XSpreadsheet.class, xIndexAccess.getByIndex(0));

            Session session;
            if ( !stringHost.equals( "" ) ) {
                // Creating a Notes session for remote calls to the Domino classes.
                session = NotesFactory.createSession(stringHost, stringUser,
                                                     stringPassword);
            }
            else {
                // Creating a Notes session for only local calls to the
                // Domino classes.
                session = NotesFactory.createSession();
            }

            // Getting the specified Notes database.
            Database database = session.getDatabase( "", stringDatabase );

            // Getting a collection of all documents from the database.
            DocumentCollection documentCollection = database.getAllDocuments();

            // Getting the first document from the database
            Document document = documentCollection.getFirstDocument();

            // Start to write to cells at this row.
            int intRowToStart = 0;

            // The current row.
            int intRow = intRowToStart;

            // The current column.
            int intColumn = 0;

            // Process all documents
            while ( document != null ) {
                // Getting the name of the stock.
                String stringName = document.getItemValueString("Name");

                // Inserting the name to a specified cell.
                insertIntoCell(intColumn, intRow, stringName, xSpreadsheet, "");

                // Getting the number of stocks.
                double intNumber = document.getItemValueInteger( "Number" );

                // Inserting the number of stocks to a specified cell.
                insertIntoCell( intColumn + 1, intRow, String.valueOf(intNumber),
                                xSpreadsheet, "V" );

                // Getting current share price.
                double doubleSharePrice = document.getItemValueDouble("SharePrice");

                // Inserting the current share price to a specified cell.
                insertIntoCell(intColumn + 2, intRow,
                               String.valueOf(doubleSharePrice),
                               xSpreadsheet, "V");

                // Inserting the total value.
                insertIntoCell(intColumn + 3, intRow, "=B"
                               + String.valueOf( intRow + 1 )
                               + "*C" + String.valueOf(intRow + 1),
                               xSpreadsheet, "");

                // Increasing the current row.
                intRow++;

                // Getting the next document from the collection.
                document = documentCollection.getNextDocument();
            }

            // Summing all specific amounts.
            insertIntoCell(intColumn + 3, intRow, "=sum(D"
                           + String.valueOf( intRowToStart + 1 ) + ":D"
                           + String.valueOf( intRow ),
                           xSpreadsheet, "");

            xContext = null;

            // Leaving the program.
            System.exit(0);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }

    /** Inserting a value or formula to a cell defined by the row and column.
     * @param intCellX Row.
     * @param intCellY Column.
     * @param stringValue This value will be written to the cell.
     * @param xSpreadsheet Write the value to the cells of this spreadsheet.
     * @param stringFlag If this string contains "V", the value will be written,
     *                   otherwise the formula.
   */
    public static void insertIntoCell(int intCellX, int intCellY,
                                      String stringValue,
                                      XSpreadsheet xSpreadsheet,
                                      String stringFlag)
    {
        XCell xCell = null;

        try {
            xCell = xSpreadsheet.getCellByPosition( intCellX, intCellY );
        } catch ( com.sun.star.lang.IndexOutOfBoundsException exception ) {
            System.out.println( "Could not get Cell" );
        }
        if ( stringFlag.equals( "V" )) {
            xCell.setValue((new Float(stringValue)).floatValue());
        }
        else {
            xCell.setFormula(stringValue);
        }
    }
}
