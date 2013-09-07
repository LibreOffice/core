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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.GregorianCalendar;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.sheet.XCellRangeMovement;
import com.sun.star.sheet.XFunctionAccess;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.table.CellAddress;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.table.XColumnRowRange;
import com.sun.star.table.XTableRows;
import com.sun.star.text.XSimpleText;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextField;
import com.sun.star.text.XTextRange;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import org.openoffice.XToDo;

/** This class capsulates the class, that implements the minimal component, a
 * factory for creating the service (<CODE>__getServiceFactory</CODE>) and a
 * method, that writes the information into the given registry key
 * (<CODE>__writeRegistryServiceInfo</CODE>).
 */
public class ToDo {

    /** This class implements the component. At least the interfaces
     * XInterface, XTypeProvider, and XWeak implemented by the helper class
     * WeakBase and XServiceInfo should be provided by the service.
     */
    public static class ToDoImpl extends WeakBase implements XServiceInfo, XToDo {

        /** The service name, that must be used to get an instance of this service.
         */
        private static final String __serviceName = "org.openoffice.ToDo";

        /** The initial component contextr, that gives access to
         * the service manager, supported singletons, ...
         * It's often later used
         */
        private XComponentContext m_cmpCtx;

        /** The service manager, that gives access to all registered services.
         * It's often later used
         */
        private XMultiComponentFactory m_xMCF;

        // Implementation helper variables
        static private final int INT_COLUMN_FEATURE = 0;
        static private final int INT_COLUMN_COMMENT = 1;
        static private final int INT_COLUMN_NEEDEDDAYS = 2;
        static private final int INT_COLUMN_STARTDATE = 3;
        static private final int INT_COLUMN_START_DAY_OF_WEEK = 4;
        static private final int INT_COLUMN_ENDDATE = 5;
        static private final int INT_COLUMN_END_DAY_OF_WEEK = 6;
        static private final int INT_COLUMN_DUEDATE = 7;
        static private final int INT_COLUMN_STATUS = 8;

        static private final int INT_ROW_FROM = 14; // 8

        static private final int INT_ROW_HOLIDAYS_START = 4;
        static private final int INT_COLUMN_HOLIDAYS_START = 7; // 10

        static private final String STRING_SEPARATOR = "/";


        /** The constructor of the inner class has a XComponenContext parameter.
         * @param xCompContext the initial component context
         */
        public ToDoImpl(XComponentContext xCompContext) {
            try {
                m_cmpCtx = xCompContext;
                m_xMCF = m_cmpCtx.getServiceManager();
            }
            catch( Exception e ) {
                e.printStackTrace(System.err);
            }
        }

        /** This method returns an array of all supported service names.
         * @return Array of supported service names.
         */
        public String[] getSupportedServiceNames() {
            return getServiceNames();
        }

        public static String[] getServiceNames() {
            String[] sSupportedServiceNames = { __serviceName };
            return sSupportedServiceNames;
        }

        /** This method returns true, if the given service will be
         * supported by the component.
         * @param sService Service name.
         * @return True, if the given service name will be supported.
         */
        public boolean supportsService(String sServiceName) {
            return sServiceName.equals( __serviceName );
        }

        /** Return the class name of the component.
         * @return Class name of the component.
         */
        public String getImplementationName() {
            return ToDoImpl.class.getName();
        }

        /** For every bug/feature listed in a spreadsheet document this method
         * calculates the start date, day of week of the start date, the end date
         * and the day of week of the end date. All calculations are dependent
         * on the values of "Needed Days", "Due Date" and "Status". The columns
         * "Needed Days" and "Status" are mandatory. The first feature/bug should
         * be placed in row nine. The date to start the calculation should be
         * placed in cell C6. The private holidays should be placed in cell K4/K5
         * and below. All rows will be calculated up to the first empty cell in
         * the first column. If a cell in the column "Due Date" will be colored
         * red, you should take a look at your entries.
         * @param aInstance Spreadsheet document.
         * @throws com.sun.star.uno.RuntimeException This exception could occur
         *                                           at every interface method.
         */
        public void recalc( java.lang.Object aInstance )
            throws com.sun.star.uno.RuntimeException {
            try {
                // Querying for the interface XSpreadsheetDocument
                XSpreadsheetDocument xspreadsheetdocument =
                    UnoRuntime.queryInterface(
                    XSpreadsheetDocument.class, aInstance );

                // Querying for the interface XIndexAccess
                XIndexAccess xindexaccess = UnoRuntime.queryInterface( XIndexAccess.class,
                                             xspreadsheetdocument.getSheets() );

                // Getting the first XSpreadsheet
                XSpreadsheet xspreadsheet = UnoRuntime.queryInterface(
                    XSpreadsheet.class, xindexaccess.getByIndex( 0 ));

                // Querying for the interface XCellRange on the XSpeadsheet
                XCellRange xcellrange = UnoRuntime.queryInterface( XCellRange.class, xspreadsheet );

                /* Getting the gregorian calendar with the date on which to start
                   the calculation */
                GregorianCalendar gregCalAbsoluteStartDate =
                    this.getGregorianCalendarFromString(this.getStringFromCell(
                                                            xcellrange, 5, 2 ) );
                gregCalAbsoluteStartDate.add( Calendar.DATE, -1 );

                // Set the start date with the absolute start date
                GregorianCalendar gregCalStartDate =
                (GregorianCalendar) gregCalAbsoluteStartDate.clone();

                /* Creating the service FunctionAccess, which allows generic
                   access to all spreadsheet functions */
                Object objectFunctionAccess =
                    m_xMCF.createInstanceWithContext(
                        "com.sun.star.sheet.FunctionAccess", m_cmpCtx );

                // Querying for the interface XFunctionAccess on service
                // FunctionAccess
                XFunctionAccess xfunctionaccess = UnoRuntime.queryInterface(XFunctionAccess.class,
                                                  objectFunctionAccess );

                // Creating vector for holidays
                ArrayList<Object> vectorHolidays = new ArrayList<Object>();

                // Get the Official Holidays
                this.getOfficialHolidays( vectorHolidays, xcellrange,
                                          xfunctionaccess,
                                          gregCalStartDate.get(
                                              Calendar.YEAR ) );

                // Get the private holidays
                this.getPrivateHolidays(vectorHolidays, xcellrange,
                                        xfunctionaccess);

                // Getting the object array of holidays
                Object[] objectSortedHolidays = vectorHolidays.toArray();

                // Sorting the holidays
                Arrays.sort( objectSortedHolidays );

                // Collect the Official Holidays and the private holidays
                Object [][]objectHolidays =
                new Object[][] { objectSortedHolidays };

                // Row index
                int intRowTo = ToDoImpl.INT_ROW_FROM - 1;

                // Getting the feature of the first cell
                String sFeature = this.getStringFromCell(xcellrange,
                                                         intRowTo + 1,
                                                         ToDoImpl.INT_COLUMN_FEATURE);

                // Determine the last row with an entry in the first column
                while ( ( sFeature != null ) &&
                        ( !sFeature.equals( "" ) ) ) {
                    intRowTo++;
                    sFeature = this.getStringFromCell( xcellrange,
                    intRowTo + 1, ToDoImpl.INT_COLUMN_FEATURE );
                }

                // Setting the last row to be calculated
                final int INT_ROW_TO = intRowTo + 1;

                // Deleting cells which will be recalculated
                for ( int intRow = ToDoImpl.INT_ROW_FROM; intRow < INT_ROW_TO + 5;
                      intRow++ ) {
                    for ( int intColumn = ToDoImpl.INT_COLUMN_STARTDATE;
                          intColumn <= ToDoImpl.INT_COLUMN_END_DAY_OF_WEEK;
                          intColumn++ ) {
                        this.setStringToCell(xcellrange, intRow, intColumn, "");
                    }
                }

                /* Clearing the background color of the due date cells and setting
                   the hyperlink to the bugtracker */
                for (int intRow = ToDoImpl.INT_ROW_FROM; intRow < INT_ROW_TO; intRow++)
                {
                    // Querying for the interface XPropertySet for the cell
                    // providing the due date
                    XPropertySet xpropertyset = UnoRuntime.queryInterface(XPropertySet.class,
                                                  xcellrange.getCellByPosition(
                                                  ToDoImpl.INT_COLUMN_DUEDATE,
                                                  intRow ));

                    // Changing the background color of the cell to white
                    xpropertyset.setPropertyValue( "CellBackColor",
                    new Integer( 16777215 ) );

                    // Getting the cell of the bug id
                    XCell xcell = xcellrange.getCellByPosition(
                    ToDoImpl.INT_COLUMN_FEATURE, intRow );

                    // Querying for the interface XSimpleText
                    XSimpleText xsimpletext = UnoRuntime.queryInterface( XSimpleText.class, xcell );

                    // Getting the text cursor
                    XTextCursor xtextcursor = xsimpletext.createTextCursor();

                    // Querying for the interface XTextRange
                    XTextRange xtextrange = UnoRuntime.queryInterface( XTextRange.class, xtextcursor );

                    // Getting the bug ID from the cell
                    String sBugID = xtextrange.getString();
                    if ( !sBugID.startsWith(
                             "http://www.openoffice.org/issues/show_bug.cgi?id=") ) {
                        String sBugIDLink =
                            "http://www.openoffice.org/issues/show_bug.cgi?id=" + sBugID;

                        // Querying for the interface XMultiServiceFactory
                        XMultiServiceFactory xMSFTextField =
                            UnoRuntime.queryInterface(
                            XMultiServiceFactory.class, aInstance );

                        // Creating an instance of the text field URL
                        Object objectTextField =
                            xMSFTextField.createInstance(
                                "com.sun.star.text.TextField.URL" );

                        // Querying for the interface XTextField
                        XTextField xtextfield = UnoRuntime.queryInterface( XTextField.class,
                                                   objectTextField );

                        // Querying for the interface XPropertySet
                        XPropertySet xpropertysetTextField = UnoRuntime.queryInterface( XPropertySet.class,
                                                   xtextfield );

                        // Setting the URL
                        xpropertysetTextField.setPropertyValue( "URL",
                                                                sBugIDLink );

                        // Setting the representation of the URL
                        xpropertysetTextField.setPropertyValue( "Representation",
                                                                sBugID );

                        // Querying for the interface XText
                        XText xtext = UnoRuntime.queryInterface(
                            XText.class, xcell );

                        // Delete cell content
                        xtextrange.setString( "" );

                        // Inserting the text field URL to the cell
                        xtext.insertTextContent( xtextrange, xtextfield, false );
                    }
                }

                // Processing all features/bugs in the table
                for (int intRow = ToDoImpl.INT_ROW_FROM; intRow < INT_ROW_TO; intRow++)
                {
                    // Getting the cell of the column "Needed Days" in the
                    // current row
                    XCell xcell = xcellrange.getCellByPosition(
                        INT_COLUMN_NEEDEDDAYS, intRow );

                    // Getting the number of needed days to perform the feature
                    int intNeededDays = (int) Math.round( xcell.getValue() );

                    // Getting the content of a specified cell
                    String sStatus = this.getStringFromCell( xcellrange,
                    intRow, ToDoImpl.INT_COLUMN_STATUS );

                    /* Testing if the number of needed days is greater than
                       zero and if
                       the status is not "done" */
                    if ( ( intNeededDays > 0 )
                         && !( sStatus.toLowerCase().trim().equals("done")) ) {
                        // Getting the start date after a specified number of
                        // workdays
                        gregCalStartDate = this.getWorkday(
                            gregCalStartDate, 1, objectHolidays,
                            xfunctionaccess );

                        // Getting a string with the date format jjjj-mm-dd from
                        // the gregorian calendar
                        String sDate = this.getStringFromGregorianCalendar(
                            gregCalStartDate );

                        // Set the start date in the specified cell of the table
                        this.setStringToCell(xcellrange, intRow,
                                             ToDoImpl.INT_COLUMN_STARTDATE, sDate);

                        // For the start day set the day of week in the specified
                        // cell of the table
                        this.setDayOfWeek( gregCalStartDate,
                                           xcellrange, intRow,
                                           ToDoImpl.INT_COLUMN_START_DAY_OF_WEEK );

                        // Getting the end date after a specified number of workdays
                        GregorianCalendar gregCalEndDate =
                            this.getWorkday( gregCalStartDate,
                                             intNeededDays - 1,
                                             objectHolidays, xfunctionaccess );

                        // Creating a string with the date format jjjj-mm-dd
                        sDate = this.getStringFromGregorianCalendar(
                            gregCalEndDate );

                        // Set the end date in the specified cell of the table
                        this.setStringToCell( xcellrange, intRow,
                                              ToDoImpl.INT_COLUMN_ENDDATE, sDate );

                        // For the end day set the day of week in the specified
                        // cell of the table
                        this.setDayOfWeek(gregCalEndDate, xcellrange,
                                          intRow, ToDoImpl.INT_COLUMN_END_DAY_OF_WEEK);

                        // Set the initial date for the next loop
                        gregCalStartDate = ( GregorianCalendar )
                            gregCalEndDate.clone();

                        // Get the due date from the table
                        String sDueDate = this.getStringFromCell(
                            xcellrange, intRow, ToDoImpl.INT_COLUMN_DUEDATE );

                        // Testing if the due date is not empty
                        if ( !sDueDate.equals( "" ) ) {
                            GregorianCalendar gregCalDueDate =
                                this.getGregorianCalendarFromString(sDueDate);

                            // Testing if the due date is before the calculated
                            // end date
                            if ( gregCalDueDate.before(
                                     gregCalEndDate ) ) {
                                /* Getting the date when the processing of the
                                   feature/bug should
                                   be started at the latest */
                                GregorianCalendar gregCalLatestDateToStart =
                                    this.getWorkday(gregCalDueDate,
                                                    -( intNeededDays - 1 ),
                                                    objectHolidays,
                                                    xfunctionaccess);

                                // Begin with the current row
                                int intRowToInsert = intRow;

                                // Get the start date for the feature/bug in the
                                // current row
                                GregorianCalendar gregCalPreviousStartDate =
                                    this.getGregorianCalendarFromString(
                                        this.getStringFromCell(
                                            xcellrange, intRowToInsert,
                                            ToDoImpl.INT_COLUMN_STARTDATE ) );

                                // Testing if we have to search for an earlier date
                                // to begin
                                while ((gregCalLatestDateToStart.before(
                                            gregCalPreviousStartDate)) &&
                                        (INT_ROW_FROM != intRowToInsert)) {
                                    // Decrease the row
                                    intRowToInsert--;

                                    // Get the start date for the feature/bug in
                                    // the current row
                                    String sStartDate = this.getStringFromCell(
                                        xcellrange, intRowToInsert,
                                        ToDoImpl.INT_COLUMN_STARTDATE );

                                    // Search until a valid start date is found
                                    while ( sStartDate.equals( "" ) ) {
                                        // Decrease the row
                                        intRowToInsert--;

                                        // Get the start date for the feature/bug
                                        // in the current row
                                        sStartDate = this.getStringFromCell(
                                            xcellrange, intRowToInsert,
                                            ToDoImpl.INT_COLUMN_STARTDATE );
                                    }

                                    // Get the GregorianCalender format for the
                                    // start date
                                    gregCalPreviousStartDate =
                                        this.getGregorianCalendarFromString(
                                            sStartDate );
                                }

                                // Getting the cell of the column "Needed Days"
                                // in the row where to insert
                                XCell xcellNeededDaysWhereToInsert =
                                    xcellrange.getCellByPosition(
                                        INT_COLUMN_NEEDEDDAYS, intRowToInsert );
                                // Getting the number of needed days to perform
                                // the feature
                                int intNeededDaysWhereToInsert = (int)
                                    Math.round(
                                        xcellNeededDaysWhereToInsert.getValue());

                                GregorianCalendar gregCalPreviousNewEndDate =
                                    this.getWorkday(gregCalPreviousStartDate,
                                                    intNeededDays - 1 +
                                                    intNeededDaysWhereToInsert,
                                                    objectHolidays,
                                                    xfunctionaccess);
                                String sPreviousDueDate = this.getStringFromCell(
                                    xcellrange, intRowToInsert,
                                    ToDoImpl.INT_COLUMN_DUEDATE );

                                GregorianCalendar gregCalPreviousDueDate = null;

                                if ( !sPreviousDueDate.equals( "" ) ) {
                                    gregCalPreviousDueDate =
                                        this.getGregorianCalendarFromString(
                                            sPreviousDueDate );
                                }

                                if ( ( intRowToInsert == intRow ) ||
                                     ( gregCalPreviousNewEndDate.after(
                                         gregCalPreviousDueDate ) ) ) {
                                    // Querying for the interface XPropertySet for
                                    // the cell providing the due date
                                    XPropertySet xpropertyset = UnoRuntime.queryInterface(
                                            XPropertySet.class,
                                            xcellrange.getCellByPosition(
                                            ToDoImpl.INT_COLUMN_DUEDATE,
                                            intRow ) );

                                    // Changing the background color of the cell
                                    // to red
                                    xpropertyset.setPropertyValue(
                                        "CellBackColor", new Integer( 16711680 ) );
                                } else {
                                    // Querying for the interface XColumnRowRange
                                    // on the XCellRange
                                    XColumnRowRange xcolumnrowrange =
                                        UnoRuntime.queryInterface(
                                        XColumnRowRange.class, xcellrange );
                                    // Inserting one row to the table
                                    XTableRows xTableRows =
                                        xcolumnrowrange.getRows();
                                    xTableRows.insertByIndex( intRowToInsert, 1 );

                                    // Querying for the interface
                                    // XCellRangeMovement on XCellRange
                                    XCellRangeMovement xcellrangemovement =
                                      UnoRuntime.queryInterface(
                                          XCellRangeMovement.class, xcellrange );

                                    // Creating the cell address of the destination
                                    CellAddress celladdress = new CellAddress();
                                    celladdress.Sheet = 0;
                                    celladdress.Column = 0;
                                    celladdress.Row = intRowToInsert;

                                    // Creating the cell range of the source
                                    CellRangeAddress cellrangeaddress =
                                    new CellRangeAddress();
                                    cellrangeaddress.Sheet = 0;
                                    cellrangeaddress.StartColumn = 0;
                                    cellrangeaddress.StartRow = intRow + 1;
                                    cellrangeaddress.EndColumn = 8;
                                    cellrangeaddress.EndRow = intRow + 1;

                                    // Moves the cell range to another position in
                                    // the document
                                    xcellrangemovement.moveRange(celladdress,
                                                                 cellrangeaddress);

                                    // Removing the row not needed anymore
                                    xcolumnrowrange.getRows().removeByIndex(intRow
                                                                            + 1, 1);

                                    // Set the current row, because we want to
                                    // recalculate all rows below
                                    intRow = intRowToInsert - 1;

                                    // Tests at which line we want to insert
                                    if ( intRow >= ToDoImpl.INT_ROW_FROM ) {
                                        // Get the start date
                                        gregCalStartDate =
                                            this.getGregorianCalendarFromString(
                                                this.getStringFromCell( xcellrange,
                                                   intRow,ToDoImpl.INT_COLUMN_ENDDATE));
                                    }
                                    else {
                                        // Set the start date with the absolute s
                                        // tart date
                                        gregCalStartDate = (GregorianCalendar)
                                            gregCalAbsoluteStartDate.clone();
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch( Exception exception ) {
                showExceptionMessage( exception );
            }
        }

        /** Getting a string from a gregorian calendar.
         * @param gregCal Date to be converted.
         * @return string (converted gregorian calendar).
         */
        public String getStringFromGregorianCalendar( GregorianCalendar gregCal ) {
            String sDate = ( gregCal.get( Calendar.MONTH ) + 1 )
                + STRING_SEPARATOR + gregCal.get( Calendar.DATE )
//                + STRING_SEPARATOR + ( gregCal.get( Calendar.MONTH ) + 1 )
                + STRING_SEPARATOR + gregCal.get( Calendar.YEAR );

            return  sDate;
        }

        /** Getting a GregorianCalendar from a string.
         * @param sDate String to be converted.
         * @return The result of the converting of the string.
         */
        public GregorianCalendar getGregorianCalendarFromString( String sDate ) {
            int []intDateValue = this.getDateValuesFromString( sDate );

            return( new GregorianCalendar( intDateValue[ 2 ], intDateValue[ 0 ],
                                           intDateValue[ 1 ] ) );
        }

        /** Getting the day, month and year from a string.
         * @param sDate String to be parsed.
         * @return Returns an array of integer variables.
         */
        public int[] getDateValuesFromString( String sDate) {
            int[] intDateValues = new int[ 3 ];

            int intPositionFirstTag = sDate.indexOf( STRING_SEPARATOR );
            int intPositionSecondTag = sDate.indexOf(STRING_SEPARATOR,
                                                     intPositionFirstTag + 1);

            // Getting the value of the month
            intDateValues[ 0 ] = Integer.parseInt(
                sDate.substring(0, intPositionFirstTag)) - 1;
            // Getting the value of the day
            intDateValues[ 1 ] = Integer.parseInt(
                sDate.substring(intPositionFirstTag + 1, intPositionSecondTag));
            // Getting the value of the year
            intDateValues[ 2 ] = Integer.parseInt(
                sDate.substring(intPositionSecondTag + 1, sDate.length()));

            return intDateValues;
        }

        /** Getting a content from a specified cell.
         * @param xcellrange Providing access to cells.
         * @param intRow Number of row.
         * @param intColumn Number of column.
         * @return String from the specified cell.
         */
        public String getStringFromCell( XCellRange xcellrange, int intRow,
                                         int intColumn ) {
            XTextRange xtextrangeStartDate = null;

            try {
                // Getting the cell holding the information about the start date
                XCell xcellStartDate = xcellrange.getCellByPosition(intColumn,
                                                                    intRow);
                // Querying for the interface XTextRange on the XCell
                xtextrangeStartDate = UnoRuntime.queryInterface(XTextRange.class, xcellStartDate);
            }
            catch( Exception exception ) {
                this.showExceptionMessage( exception );
            }

            // Getting the start date
            return  xtextrangeStartDate.getString().trim();
        }

        /** Writing a specified string to a specified cell.
         * @param xcellrange Providing access to the cells.
         * @param intRow Number of row.
         * @param intColumn Number of column.
         * @param sDate Date to write to the cell.
         */
        public void setStringToCell( XCellRange xcellrange, int intRow,
                                     int intColumn, String sDate ) {
            try {
                // Getting the cell holding the information on the day to start
                XCell xcellStartDate = xcellrange.getCellByPosition(intColumn,
                                                                    intRow);
                // Querying for the interface XTextRange on the XCell
                XTextRange xtextrange = UnoRuntime.queryInterface(XTextRange.class, xcellStartDate);
                // Setting the new start date
                xtextrange.setString( sDate );
            }
            catch( Exception exception ) {
                this.showExceptionMessage( exception );
            }
        }

        /** Calculates the week of day and calls the method "setStringToCell".
         * @param gregCal Day to be written to the cell.
         * @param xcellrange Providing access to the cells.
         * @param intRow Number of row.
         * @param intColumn Number of column.
         */
        public void setDayOfWeek( GregorianCalendar gregCal,
                                  XCellRange xcellrange, int intRow,
                                  int intColumn) {
            int intDayOfWeek = gregCal.get( Calendar.DAY_OF_WEEK );
            String sDayOfWeek = "";
            if ( intDayOfWeek == Calendar.MONDAY ) {
                sDayOfWeek = "MON";
            } else if ( intDayOfWeek == Calendar.TUESDAY ) {
                sDayOfWeek = "TUE";
            } else if ( intDayOfWeek == Calendar.WEDNESDAY ) {
                sDayOfWeek = "WED";
            } else if ( intDayOfWeek == Calendar.THURSDAY ) {
                sDayOfWeek = "THU";
            } else if ( intDayOfWeek == Calendar.FRIDAY ) {
                sDayOfWeek = "FRI";
            }

            this.setStringToCell( xcellrange, intRow, intColumn,
            sDayOfWeek );
        }

        /** Calculates the dates of the official holidays with help of Calc
         * functions.
         * @param vectorHolidays Holding all holidays.
         * @param xcellrange Providing the cells.
         * @param xfunctionaccess Provides access to functions of the Calc.
         * @param intYear Year to calculate the official holidays.
         */
        public void getOfficialHolidays(
        ArrayList<Object> vectorHolidays,
        XCellRange xcellrange,
        XFunctionAccess xfunctionaccess,
        int intYear ) {
            try {
                // Official Holidays for how many years?
                final int intHowManyYears = 2;

                // Get the Official Holiday for two years
                for ( int intNumberOfYear = 0;
                      intNumberOfYear <= ( intHowManyYears - 1 );
                      intNumberOfYear++ ) {
                    intYear += intNumberOfYear;

                    // Getting the Easter sunday
                    Double dEasterSunday = ( Double )
                        xfunctionaccess.callFunction(
                            "EASTERSUNDAY", new Object[] { new Integer(intYear) });

                    int intEasterSunday = (int)Math.round(
                        dEasterSunday.doubleValue());

                    // New-year
                    vectorHolidays.add( xfunctionaccess.callFunction(
                                                   "DATE",
                                                   new Object[] {
                                                       new Integer( intYear ),
                                                       new Integer( 1 ),
                                                       new Integer( 1 ) } ));

                    // Good Friday
                    vectorHolidays.add(
                    new Double( intEasterSunday - 2 ) );

                    // Easter monday
                    vectorHolidays.add(
                    new Double( intEasterSunday + 1 ) );

                    // Labour Day
                    vectorHolidays.add( xfunctionaccess.callFunction(
                                                   "DATE",
                                                   new Object[] {
                                                       new Integer( intYear ),
                                                       new Integer( 5 ),
                                                       new Integer( 1 ) } ));

                    // Ascension Day
                    vectorHolidays.add(new Double(intEasterSunday + 39 ));

                    // Pentecost monday
                    vectorHolidays.add(new Double(intEasterSunday + 50 ));

                    // German Unification
                    vectorHolidays.add( xfunctionaccess.callFunction(
                                                   "DATE",
                                                   new Object[] {
                                                       new Integer( intYear ),
                                                       new Integer( 10 ),
                                                       new Integer( 3 ) } ));

                    // Christmas Day First
                    vectorHolidays.add( xfunctionaccess.callFunction(
                                                   "DATE",
                                                   new Object[] {
                                                       new Integer( intYear ),
                                                       new Integer( 12 ),
                                                       new Integer( 25 ) } ));

                    // Christmas Day Second
                    vectorHolidays.add( xfunctionaccess.callFunction(
                                                   "DATE",
                                                   new Object[] {
                                                       new Integer( intYear ),
                                                       new Integer( 12 ),
                                                       new Integer( 26 ) } ));
                }
            }
            catch( Exception exception ) {
                this.showExceptionMessage( exception );
            }
        }

        /** Returns the serial number of the date before or after a specified
         * number of workdays.
         * @param gregCalStartDate Date to start with the calculation.
         * @param intDays Number of workdays (e.g. 5 or -3).
         * @param objectHolidays Private and public holidays to take into account.
         * @param xfunctionaccess Allows to call functions from the Calc.
         * @return The gregorian date before or after a specified number of
         *         workdays.
         */
        public GregorianCalendar getWorkday(
            GregorianCalendar gregCalStartDate,
            int intDays, Object[][] objectHolidays,
            XFunctionAccess xfunctionaccess ) {
            GregorianCalendar gregCalWorkday = null;

            try {
                // Getting the value of the start date
                Double dDate = ( Double ) xfunctionaccess.callFunction(
                    "DATE",
                    new Object[] {
                        new Integer( gregCalStartDate.get( Calendar.YEAR ) ),
                        new Integer( gregCalStartDate.get( Calendar.MONTH ) + 1 ),
                        new Integer( gregCalStartDate.get( Calendar.DATE ) )
                    } );

                Double dWorkday = ( Double ) xfunctionaccess.callFunction(
                "com.sun.star.sheet.addin.Analysis.getWorkday",
                new Object[] { dDate, new Integer( intDays ), objectHolidays } );

                Double dYear = ( Double ) xfunctionaccess.callFunction(
                    "YEAR", new Object[] { dWorkday } );
                Double dMonth = ( Double ) xfunctionaccess.callFunction(
                    "MONTH", new Object[] { dWorkday } );
                Double dDay = ( Double ) xfunctionaccess.callFunction(
                    "DAY", new Object[] { dWorkday } );

                gregCalWorkday = new GregorianCalendar(
                dYear.intValue(),
                dMonth.intValue() - 1,
                dDay.intValue() );
            }
            catch( Exception exception ) {
                this.showExceptionMessage( exception );
            }

            return gregCalWorkday;
        }

        /** Getting the holidays from the spreadsheet.
         * @param vectorHolidays Holding all holidays.
         * @param xcellrange Providing the cells.
         * @param xfunctionaccess Provides the access to functions of the Calc.
         */
        public void getPrivateHolidays( ArrayList<Object> vectorHolidays,
                                        XCellRange xcellrange,
                                        XFunctionAccess xfunctionaccess ) {
            try {
                int intRow = ToDoImpl.INT_ROW_HOLIDAYS_START;
                int intColumn = ToDoImpl.INT_COLUMN_HOLIDAYS_START;

                double dHolidayStart = xcellrange.getCellByPosition(
                    intColumn, intRow ).getValue();

                double dHolidayEnd = xcellrange.getCellByPosition(
                    intColumn + 1, intRow ).getValue();

                while ( dHolidayStart != 0 ) {
                    if ( dHolidayEnd == 0 ) {
                        vectorHolidays.add(
                            new Integer( (int) Math.round(
                                             dHolidayStart ) ) );
                    }
                    else {
                        for ( int intHoliday = (int) Math.round(
                                  dHolidayStart );
                              intHoliday <= (int) Math.round( dHolidayEnd );
                              intHoliday++ ) {
                            vectorHolidays.add( new Double( intHoliday ) );
                        }
                    }

                    intRow++;
                    dHolidayStart = xcellrange.getCellByPosition(
                        intColumn, intRow).getValue();
                    dHolidayEnd = xcellrange.getCellByPosition(
                        intColumn + 1, intRow).getValue();
                }
            }
            catch( Exception exception ) {
                this.showExceptionMessage( exception );
            }
        }

        /** Showing the stack trace in a JOptionPane.
         * @param sMessage The message to show.
         */
        public void showMessage( String sMessage ) {
            javax.swing.JFrame jframe = new javax.swing.JFrame();
            jframe.setLocation(100, 100);
            jframe.setSize(300, 200);
            jframe.setVisible(true);
            javax.swing.JOptionPane.showMessageDialog(
                jframe, sMessage, "Debugging information",
                javax.swing.JOptionPane.INFORMATION_MESSAGE);
            jframe.dispose();
        }

        /** Writing the stack trace from an exception to a string and calling
         * the method showMessage() with this string.
         * @param exception The occurred exception.
         * @see showMessage
         */
        public void showExceptionMessage( Exception exception ) {
            java.io.StringWriter swriter = new java.io.StringWriter();
            java.io.PrintWriter printwriter =
                new java.io.PrintWriter( swriter );
            exception.printStackTrace( printwriter);
            System.err.println( exception );
            this.showMessage( swriter.getBuffer().substring(0) );
        }
    }

    /**
     * Gives a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns a <code>XSingleComponentFactory</code> for creating
     *          the component
     * @param   sImplName the name of the implementation for which a
     *          service is desired
     * @see     com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleComponentFactory __getComponentFactory(String sImplName) {
        XSingleComponentFactory xFactory = null;

        if ( sImplName.equals( ToDoImpl.class.getName() ) )
            xFactory = Factory.createComponentFactory(ToDoImpl.class,
                                                      ToDoImpl.getServiceNames());

        return xFactory;
    }

    /**
     * Writes the service information into the given registry key.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns true if the operation succeeded
     * @param   regKey the registryKey
     * @see     com.sun.star.comp.loader.JavaLoader
     */
    // This method not longer necessary since OOo 3.4 where the component registration
    // was changed to passive component registration. For more details see
    // http://wiki.openoffice.org/wiki/Passive_Component_Registration

//     public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
//         return Factory.writeRegistryServiceInfo(ToDoImpl.class.getName(),
//                                                 ToDoImpl.getServiceNames(), regKey);
//     }
}

