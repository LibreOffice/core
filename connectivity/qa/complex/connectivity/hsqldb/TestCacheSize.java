/*
 * This file is part of the LibreOffice project.
 *
 * Copyright (c) 2001-2004, The HSQL Development Group
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of the HSQL Development Group nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL HSQL DEVELOPMENT GROUP, HSQLDB.ORG,
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


package complex.connectivity.hsqldb;



import org.hsqldb.lib.StopWatch;

import java.util.Random;
import com.sun.star.lang.*;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.sdbc.*;

/**
 * Test large cached tables by setting up a cached table of 100000 records
 * or more and a much smaller memory table with about 1/100th rows used.
 * Populate both tables so that an indexed column of the cached table has a
 * foreign key reference to the main table.
 *
 * This database can be used to demonstrate efficient queries to retrieve
 * the data from the cached table.
 *
 * 1.7.1 insert timings for 100000 rows, cache scale 12:
 * simple table, no extra index: 52 s
 * with index on lastname only: 56 s
 * with index on zip only: 211 s
 * foreign key, referential_integrity true: 216 s
 *
 * The above have improved a lot in 1.7.2
 *
 * This test now incorporates the defunct TestTextTables
 *
 * @version 1.7.2
 * @since 1.7.0
 */
public class TestCacheSize {

    // program can edit the *.properties file to set cache_size
    protected boolean filedb = true;

    // shutdown performed mid operation - not for mem: or hsql: URL's
    protected boolean shutdown = true;

    // fixed
    protected String url = "sdbc:embedded:hsqldb";

    // frequent reporting of progress
    boolean reportProgress = false;

    XMultiServiceFactory servicefactory = null;

    // type of the big table {MEMORY | CACHED | TEXT}
    String tableType      = "CACHED";
    int    cacheScale     = 17;
    int    cacheSizeScale = 8;

    // script format {TEXT, BINARY, COMPRESSED}
    String logType       = "TEXT";
    int     writeDelay    = 60;
    boolean indexZip      = true;
    boolean indexLastName = false;
    boolean addForeignKey = false;
    boolean refIntegrity  = true;

    // speeds up inserts when tableType=="CACHED"
    boolean createTempTable = false;

    // introduces fragmentation to the .data file during insert
    boolean deleteWhileInsert         = false;
    int     deleteWhileInsertInterval = 10000;

    // size of the tables used in test
    int bigrows   = 10000;
    int smallrows = 0xfff;

    // if the extra table needs to be created and filled up
    boolean multikeytable = false;

    //
    String     user;
    String     password;
    XStatement  sStatement;
    XConnection cConnection;
    XDataSource ds;
    XDriver drv;
    com.sun.star.beans.PropertyValue[] info;

    public TestCacheSize(XMultiServiceFactory _xmulti,com.sun.star.beans.PropertyValue[] _info,XDriver _drv){
        servicefactory = _xmulti;
        drv = _drv;
        info = _info;
    }

    public void setURL(String _url){
        url = _url;
    }

    public void setUp() {

        user     = "sa";
        password = "";

        try {
            sStatement  = null;
            cConnection = null;

            //Class.forName("org.hsqldb.jdbcDriver");

            if (filedb) {

                cConnection = drv.connect(url,info);
                sStatement = cConnection.createStatement();

                sStatement.execute("SET SCRIPTFORMAT " + logType);
                sStatement.execute("SET LOGSIZE " + 0);
                sStatement.execute("SHUTDOWN");
                cConnection.close();
//                props.setProperty("hsqldb.cache_scale", "" + cacheScale);
//                props.setProperty("hsqldb.cache_size_scale",
                                  //"" + cacheSizeScale);
            }
        } catch (Exception e) {
            e.printStackTrace();
            System.out.println("TestSql.setUp() error: " + e.getMessage());
        }
    }

    /**
     * Fill up the cache
     *
     *
     */
    public void testFillUp() {

        StopWatch sw = new StopWatch();
        String ddl1 = "DROP TABLE test IF EXISTS;"
                      + "DROP TABLE zip IF EXISTS;";
        String ddl2 = "CREATE CACHED TABLE zip( zip INT IDENTITY );";
        String ddl3 = "CREATE " + tableType + " TABLE test( id INT IDENTITY,"
                      + " firstname VARCHAR, " + " lastname VARCHAR, "
                      + " zip INTEGER, " + " filler VARCHAR); ";
        String ddl31 = "SET TABLE test SOURCE \"test.csv;cache_scale="
                       + cacheScale + "\";";

        // adding extra index will slow down inserts a bit
        String ddl4 = "CREATE INDEX idx1 ON TEST (lastname);";

        // adding this index will slow down  inserts a lot
        String ddl5 = "CREATE INDEX idx2 ON TEST (zip);";

        // referential integrity checks will slow down inserts a bit
        String ddl6 =
            "ALTER TABLE test add constraint c1 FOREIGN KEY (zip) REFERENCES zip(zip);";
        String ddl7 = "CREATE TEMP TABLE temptest( id INT,"
                      + " firstname VARCHAR, " + " lastname VARCHAR, "
                      + " zip INTEGER, " + " filler VARCHAR); ";
        String filler =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
            + "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ";
        String mddl1 = "DROP TABLE test2 IF EXISTS;";
        String mddl2 = "CREATE " + tableType
                       + " TABLE test2( id1 INT, id2 INT,"
                       + " firstname VARCHAR, " + " lastname VARCHAR, "
                       + " zip INTEGER, " + " filler VARCHAR, "
                       + " PRIMARY KEY (id1,id2) ); ";
        String mdd13 = "SET TABLE test2 SOURCE \"test2.csv;cache_scale="
                       + cacheScale + "\";";

        try {
            System.out.println("Connecting");
            sw.zero();

            cConnection = null;
            sStatement  = null;
            cConnection = drv.connect(url,info);

            System.out.println("connected: " + sw.elapsedTime());
            sw.zero();

            sStatement = cConnection.createStatement();

            java.util.Random randomgen = new java.util.Random();

            sStatement.execute("SET WRITE_DELAY " + writeDelay);
            sStatement.execute(ddl1);
            sStatement.execute(ddl2);
            sStatement.execute(ddl3);

            if (tableType.equals("TEXT")) {
                sStatement.execute(ddl31);
            }

            System.out.println("test table with no index");

            if (indexLastName) {
                sStatement.execute(ddl4);
                System.out.println("create index on lastname");
            }

            if (indexZip) {
                sStatement.execute(ddl5);
                System.out.println("create index on zip");
            }

            if (addForeignKey) {
                sStatement.execute(ddl6);
                System.out.println("add foreign key");
            }

            if (createTempTable) {
                sStatement.execute(ddl7);
                System.out.println("temp table");
            }

            if (multikeytable) {
                sStatement.execute(mddl1);
                sStatement.execute(mddl2);

                if (tableType.equals("TEXT")) {
                    sStatement.execute(mdd13);
                }

                System.out.println("multi key table");
            }

//            sStatement.execute("CREATE INDEX idx3 ON tempTEST (zip);");
            System.out.println("Setup time: " + sw.elapsedTime());
            fillUpBigTable(filler, randomgen);

            if (multikeytable) {
                fillUpMultiTable(filler, randomgen);
            }

            sw.zero();

            if (shutdown) {
                sStatement.execute("SHUTDOWN");
                System.out.println("Shutdown Time: " + sw.elapsedTime());
            }

            cConnection.close();
        } catch (SQLException e) {
            System.out.println(e.getMessage());
        }
    }

    private void fillUpBigTable(String filler,
                                Random randomgen) throws SQLException {

        StopWatch sw = new StopWatch();
        int       i;

        for (i = 0; i <= smallrows; i++) {
            sStatement.execute("INSERT INTO zip VALUES(null);");
        }

        sStatement.execute("SET REFERENTIAL_INTEGRITY " + this.refIntegrity
                           + ";");

        XPreparedStatement ps = cConnection.prepareStatement(
            "INSERT INTO test (firstname,lastname,zip,filler) VALUES (?,?,?,?)");

        XParameters para = UnoRuntime.queryInterface(XParameters.class,ps);
        para.setString(1, "Julia");
        para.setString(2, "Clancy");

        for (i = 0; i < bigrows; i++) {
            para.setInt(3, randomgen.nextInt(smallrows));

            long nextrandom   = randomgen.nextLong();
            int  randomlength = (int) nextrandom & 0x7f;

            if (randomlength > filler.length()) {
                randomlength = filler.length();
            }

            String varfiller = filler.substring(0, randomlength);

            para.setString(4, nextrandom + varfiller);
            ps.execute();

            if (reportProgress && (i + 1) % 10000 == 0) {
                System.out.println("Insert " + (i + 1) + " : "
                                   + sw.elapsedTime());
            }

            // delete and add 4000 rows to introduce fragmentation
            if (deleteWhileInsert && i != 0
                    && i % deleteWhileInsertInterval == 0) {
                sStatement.execute("CALL IDENTITY();");

                XMultipleResults mrs = UnoRuntime.queryInterface(XMultipleResults.class,sStatement);
                XResultSet rs = mrs.getResultSet();

                rs.next();

                XRow row = UnoRuntime.queryInterface(XRow.class,rs);
                int lastId = row.getInt(1);

                sStatement.execute(
                    "SELECT * INTO TEMP tempt FROM test WHERE id > "
                    + (lastId - 4000) + " ;");
                sStatement.execute("DELETE FROM test WHERE id > "
                                   + (lastId - 4000) + " ;");
                sStatement.execute("INSERT INTO test SELECT * FROM tempt;");
                sStatement.execute("DROP TABLE tempt;");
            }
        }

//            sStatement.execute("INSERT INTO test SELECT * FROM temptest;");
//            sStatement.execute("DROP TABLE temptest;");
//            sStatement.execute(ddl7);
        System.out.println("Total insert: " + i);
        System.out.println("Insert time: " + sw.elapsedTime() + " rps: "
                           + (i * 1000 / (sw.elapsedTime() + 1)));
    }

    private void fillUpMultiTable(String filler,
                                  Random randomgen) throws SQLException {

        StopWatch sw = new StopWatch();
        int       i;
        XPreparedStatement ps = cConnection.prepareStatement(
            "INSERT INTO test2 (id1, id2, firstname,lastname,zip,filler) VALUES (?,?,?,?,?,?)");

        XParameters para = UnoRuntime.queryInterface(XParameters.class,ps);
        para.setString(3, "Julia");
        para.setString(4, "Clancy");

        int id1 = 0;

        for (i = 0; i < bigrows; i++) {
            int id2 = randomgen.nextInt(Integer.MAX_VALUE);

            if (i % 1000 == 0) {
                id1 = randomgen.nextInt(Integer.MAX_VALUE);
            }

            para.setInt(1, id1);
            para.setInt(2, id2);
            para.setInt(5, randomgen.nextInt(smallrows));

            long nextrandom   = randomgen.nextLong();
            int  randomlength = (int) nextrandom & 0x7f;

            if (randomlength > filler.length()) {
                randomlength = filler.length();
            }

            String varfiller = filler.substring(0, randomlength);

            para.setString(6, nextrandom + varfiller);

            try {
                ps.execute();
            } catch (SQLException e) {
                e.printStackTrace();
            }

            if (reportProgress && (i + 1) % 10000 == 0) {
                System.out.println("Insert " + (i + 1) + " : "
                                   + sw.elapsedTime());
            }
        }

        System.out.println("Multi Key Total insert: " + i);
        System.out.println("Insert time: " + sw.elapsedTime() + " rps: "
                           + (i * 1000 / (sw.elapsedTime() + 1)));
    }

    public void tearDown() {}

    public void checkResults() {

        try {
            StopWatch sw = new StopWatch();
            XResultSet rs;

            cConnection = drv.connect(url,info);

            System.out.println("Reopened database: " + sw.elapsedTime());
            sw.zero();

            sStatement = cConnection.createStatement();

            sStatement.execute("SET WRITE_DELAY " + writeDelay);

            // the tests use different indexes
            // use primary index
            sStatement.execute("SELECT count(*) from TEST");

            XMultipleResults mrs = UnoRuntime.queryInterface(XMultipleResults.class,sStatement);
            rs = mrs.getResultSet();
            XRow row = UnoRuntime.queryInterface(XRow.class,rs);

            rs.next();
            System.out.println("Row Count: " + row.getInt(1));
            System.out.println("Time to count: " + sw.elapsedTime());

            // use index on zip
            sw.zero();
            sStatement.execute("SELECT count(*) from TEST where zip > -1");

            rs = mrs.getResultSet();

            rs.next();
            System.out.println("Row Count: " + row.getInt(1));
            System.out.println("Time to count: " + sw.elapsedTime());
            checkSelects();
            checkUpdates();
            checkSelects();
            sw.zero();
            sStatement.execute("SELECT count(*) from TEST where zip > -1");

            rs = mrs.getResultSet();

            rs.next();
            System.out.println("Row Count: " + row.getInt(1));
            System.out.println("Time to count: " + sw.elapsedTime());
            sw.zero();

            if (shutdown) {
                sStatement.execute("SHUTDOWN");
                System.out.println("Shutdown Time: " + sw.elapsedTime());
            }

            cConnection.close();
            System.out.println("Closed database: " + sw.elapsedTime());
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    private void checkSelects() {

        StopWatch        sw        = new StopWatch();
        java.util.Random randomgen = new java.util.Random();
        int              i         = 0;
        boolean          slow      = false;

        try {
            XPreparedStatement ps = cConnection.prepareStatement(
                "SELECT TOP 1 firstname,lastname,zip,filler FROM test WHERE zip = ?");
            XParameters para = UnoRuntime.queryInterface(XParameters.class,ps);

            for (; i < bigrows; i++) {
                para.setInt(1, randomgen.nextInt(smallrows));
                ps.execute();

                if ((i + 1) == 100 && sw.elapsedTime() > 5000) {
                    slow = true;
                }

                if (reportProgress && (i + 1) % 10000 == 0
                        || (slow && (i + 1) % 100 == 0)) {
                    System.out.println("Select " + (i + 1) + " : "
                                       + sw.elapsedTime() + " rps: "
                                       + (i * 1000 / (sw.elapsedTime() + 1)));
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }

        System.out.println("Select random zip " + i + " rows : "
                           + sw.elapsedTime() + " rps: "
                           + (i * 1000 / (sw.elapsedTime() + 1)));
        sw.zero();

        try {
            XPreparedStatement ps = cConnection.prepareStatement(
                "SELECT firstname,lastname,zip,filler FROM test WHERE id = ?");
            XParameters para = UnoRuntime.queryInterface(XParameters.class,ps);

            for (i = 0; i < bigrows; i++) {
                para.setInt(1, randomgen.nextInt(bigrows - 1));
                ps.execute();

                if (reportProgress && (i + 1) % 10000 == 0
                        || (slow && (i + 1) % 100 == 0)) {
                    System.out.println("Select " + (i + 1) + " : "
                                       + (sw.elapsedTime() + 1));
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }

        System.out.println("Select random id " + i + " rows : "
                           + sw.elapsedTime() + " rps: "
                           + (i * 1000 / (sw.elapsedTime() + 1)));
        sw.zero();

        try {
            XPreparedStatement ps = cConnection.prepareStatement(
                "SELECT zip FROM zip WHERE zip = ?");
            XParameters para = UnoRuntime.queryInterface(XParameters.class,ps);

            for (i = 0; i < bigrows; i++) {
                para.setInt(1, randomgen.nextInt(smallrows - 1));
                ps.execute();

                if (reportProgress && (i + 1) % 10000 == 0
                        || (slow && (i + 1) % 100 == 0)) {
                    System.out.println("Select " + (i + 1) + " : "
                                       + (sw.elapsedTime() + 1));
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }

        System.out.println("Select random zip from zip table " + i
                           + " rows : " + sw.elapsedTime() + " rps: "
                           + (i * 1000 / (sw.elapsedTime() + 1)));
    }

    private void checkUpdates() {

        StopWatch        sw        = new StopWatch();
        java.util.Random randomgen = new java.util.Random();
        int              i         = 0;
        boolean          slow      = false;
        int              count     = 0;

        try {
            XPreparedStatement ps = cConnection.prepareStatement(
                "UPDATE test SET filler = filler || zip WHERE zip = ?");
            XParameters para = UnoRuntime.queryInterface(XParameters.class,ps);

            for (; i < smallrows; i++) {
                int random = randomgen.nextInt(smallrows - 1);

                para.setInt(1, random);

                count += ps.executeUpdate();

                if (reportProgress && count % 10000 < 20) {
                    System.out.println("Update " + count + " : "
                                       + (sw.elapsedTime() + 1));
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }

        System.out.println("Update with random zip " + i
                           + " UPDATE commands, " + count + " rows : "
                           + sw.elapsedTime() + " rps: "
                           + (count * 1000 / (sw.elapsedTime() + 1)));
        sw.zero();

        try {
            XPreparedStatement ps = cConnection.prepareStatement(
                "UPDATE test SET zip = zip + 1 WHERE id = ?");
            XParameters para = UnoRuntime.queryInterface(XParameters.class,ps);

            for (i = 0; i < bigrows; i++) {
                int random = randomgen.nextInt(bigrows - 1);

                para.setInt(1, random);
                ps.execute();

                if (reportProgress && (i + 1) % 10000 == 0
                        || (slow && (i + 1) % 100 == 0)) {
                    System.out.println("Update " + (i + 1) + " : "
                                       + sw.elapsedTime() + " rps: "
                                       + (i * 1000 / (sw.elapsedTime() + 1)));
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }

        System.out.println("Update with random id " + i + " rows : "
                           + sw.elapsedTime() + " rps: "
                           + (i * 1000 / (sw.elapsedTime() + 1)));
    }
}
