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

import com.sun.star.uno.*;
import com.sun.star.beans.XPropertySet;
import com.sun.star.sdbc.*;

public class Sales
{
    private final XConnection con;

    public Sales(XConnection connection )
    {
        con = connection;
    }
    // create the table sales.
    public void createSalesTable() throws com.sun.star.uno.Exception
    {
        String createTableSales = "CREATE TABLE SALES " +
                "(SALENR INTEGER NOT NULL, " +
                " COS_NR INTEGER, " +
                " SNR INTEGER, " +
                " NAME VARCHAR(50)," +
                " SALEDATE DATE," +
                " PRICE FLOAT(10), " +
                " PRIMARY KEY(SALENR)" +
                " )";
        XStatement stmt = con.createStatement();
        stmt.executeUpdate( createTableSales );
    }

    // drop the table sales.
    public void dropSalesTable() throws com.sun.star.uno.Exception
    {
        String createTableSalesman = "DROP TABLE SALES ";
        XStatement stmt = con.createStatement();
        stmt.executeUpdate( createTableSalesman );
    }

    // insert data into the table sales.
    public void insertDataIntoSales() throws com.sun.star.uno.Exception
    {
        XStatement stmt = con.createStatement();
        stmt.executeUpdate("INSERT INTO SALES " +
                "VALUES (1, '100', '1','Linux','2001-02-12',15)");
        stmt.executeUpdate("INSERT INTO SALES " +
                "VALUES (2, '101', '2','Beef','2001-10-18',15.78)");
        stmt.executeUpdate("INSERT INTO SALES " +
                "VALUES (3, '104', '4','orange juice','2001-08-09',1.5)");
    }

    // update the table sales with a prepared statement.
    public void updateSales() throws com.sun.star.uno.Exception
    {
        XStatement stmt = con.createStatement();
        String updateString =   "UPDATE SALES " +
                                "SET PRICE = 30 " +
                                "WHERE SALENR = 1";
        stmt.executeUpdate(updateString);
    }

    // retrieve the data of the table sales.
    public void retrieveSalesData() throws com.sun.star.uno.Exception
    {
        XStatement stmt = con.createStatement();
        String query =  "SELECT NAME, PRICE FROM SALES " +
                        "WHERE SALENR = 1";
        XResultSet rs = stmt.executeQuery(query);
        XRow      row = UnoRuntime.queryInterface(XRow.class, rs);
        while (rs.next()) {
                String s = row.getString(1);
                float  n = row.getFloat(2);
                System.out.println("The current price for " + s + " is: $" + n + ".");
        }
    }



    // displays the column names
    public void displayColumnNames() throws com.sun.star.uno.Exception
    {
        XStatement stmt = con.createStatement();
        XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class,stmt);
        xProp.setPropertyValue("ResultSetType", Integer.valueOf(ResultSetType.SCROLL_INSENSITIVE));
        xProp.setPropertyValue("ResultSetConcurrency", Integer.valueOf(ResultSetConcurrency.READ_ONLY));
        XResultSet rs = stmt.executeQuery("SELECT NAME, PRICE FROM SALES");
        XResultSetMetaDataSupplier xRsMetaSup = UnoRuntime.queryInterface(XResultSetMetaDataSupplier.class,rs);
        XResultSetMetaData xRsMetaData =  xRsMetaSup.getMetaData();
        int nColumnCount =  xRsMetaData.getColumnCount();
        for(int i=1 ; i <= nColumnCount ; ++i)
        {
           System.out.println("Name: " + xRsMetaData.getColumnName(i) + " Type: " +
                              xRsMetaData.getColumnType(i));
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
