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

//  import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.*;
import com.sun.star.util.Date;
import com.sun.star.sdbc.*;

public class SalesMan
{
    private XConnection con;

    public SalesMan(XConnection connection )
    {
        con = connection;
    }
    // create the table salesman.
    public void createSalesManTable() throws com.sun.star.uno.Exception
    {
        String createTableSalesman = "CREATE TABLE SALESMAN " +
                "(SNR INTEGER NOT NULL, "+
                " FIRSTNAME VARCHAR(50)," +
                " LASTNAME VARCHAR(100)," +
                " STREET VARCHAR(50)," +
                " STATE VARCHAR(50)," +
                " ZIP INTEGER," +
                " BIRTHDATE DATE," +
                " PRIMARY KEY(SNR)" +
                " )";
        XStatement stmt = con.createStatement();
        stmt.executeUpdate( createTableSalesman );
    }

    // drop the table salesman
    public void dropSalesManTable() throws com.sun.star.uno.Exception
    {
        String createTableSalesman = "DROP TABLE SALESMAN ";
        XStatement stmt = con.createStatement();
        stmt.executeUpdate( createTableSalesman );
    }

    // insert data into the table salesman
    public void insertDataIntoSalesMan() throws com.sun.star.uno.Exception
    {
        XStatement stmt = con.createStatement();
        stmt.executeUpdate("INSERT INTO SALESMAN " +
                "VALUES (1, 'Joseph', 'Smith','Bond Street','CA',95460,"
                + "'1946-07-02')");
        stmt.executeUpdate("INSERT INTO SALESMAN " +
                "VALUES (2, 'Frank', 'Jones','Lake Silver','CA',95460,"
                + "'1963-12-24')");
        stmt.executeUpdate("INSERT INTO SALESMAN " +
                "VALUES (3, 'Jane', 'Esperansa','23 Hollywood drive','CA',95460,"
                + "'1972-04-01')");
        stmt.executeUpdate("INSERT INTO SALESMAN " +
                "VALUES (4, 'George', 'Flint','12 Washington street','CA',95460,"
                + "'1953-02-13')");
        stmt.executeUpdate("INSERT INTO SALESMAN " +
                "VALUES (5, 'Bob', 'Meyers','2 Moon way','CA',95460,"
                + "'1949-09-07')");
    }

    // update the table sales man with a prepared statement.
    public void updateSalesMan() throws com.sun.star.uno.Exception
    {
        XPreparedStatement updateStreet = con.prepareStatement(
            "UPDATE SALESMAN SET STREET = ? WHERE SNR = ?");
        XParameters setPara = (XParameters)UnoRuntime.queryInterface(XParameters.class,updateStreet);
        setPara.setString(1, "34 Main Road");
        setPara.setInt(2, 1);
        updateStreet.executeUpdate();

        setPara.setString(1, "Marryland");
        setPara.setInt(2, 4);
        updateStreet.executeUpdate();
        // changes STREET column of salesman George to Marryland
        setPara.setString(1, "Michigan road");
        updateStreet.executeUpdate();
        // changes again STREET column of salesman George to
        // Michigan road
        // parameter 2 stayed 4, and the first parameter was reset
        // to "Michigan road")

        setPara.setString(1, "Bond Street");
        setPara.setInt(2, 3);
        int n = updateStreet.executeUpdate();
        System.out.println("executeUpdate returns: " + n);
        // n = 1 because one row had a change in it
    }

    // retrieve the data of the table salesman
    public void retrieveSalesManData() throws com.sun.star.uno.Exception
    {
        XStatement stmt = con.createStatement();
        XResultSet rs   = stmt.executeQuery("SELECT FIRSTNAME, LASTNAME, BIRTHDATE FROM SALESMAN");
        XRow row = (XRow)UnoRuntime.queryInterface(XRow.class,rs);
        while ( rs != null && rs.next() ) {
            String fn = row.getString( 1 );
            String ln = row.getString( 2 );
            Date   dt = row.getDate( 3 );
            System.out.println(fn + "    " + ln + "    " + dt.Month + "/" + dt.Day + "/" + dt.Year);
        }
    }
}

