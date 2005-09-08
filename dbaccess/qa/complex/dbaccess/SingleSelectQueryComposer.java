/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SingleSelectQueryComposer.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:49:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
package complex.dbaccess;

import com.sun.star.awt.XWindow;
import com.sun.star.frame.XModel;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCloseable;
import com.sun.star.beans.*;
import com.sun.star.sdbcx.*;
import com.sun.star.sdbc.*;
import com.sun.star.sdb.*;
import com.sun.star.container.*;
import com.sun.star.lang.XMultiServiceFactory;

import complexlib.ComplexTestCase;

import java.io.PrintWriter;

import util.utils;
import util.dbg;


public class SingleSelectQueryComposer extends ComplexTestCase {

    private String complexFilter = "( \"Identifier\" = '1' AND \"Type\" = '4' ) OR ( \"Identifier\" = '2' AND \"Type\" = '5' ) OR ( \"Identifier\" = '3' AND \"Type\" = '6' AND \"Address\" = '7' ) OR ( \"Address\" = '8' ) OR ( \"Type\" = '9' )";

    public String[] getTestMethodNames() {
        return new String[] { "testSingleSelectQueryComposer" };
    }

    public String getTestObjectName() {
        return "SingleSelectQueryComposer";
    }

    public void testSingleSelectQueryComposer() {
        log.println("testing SingleSelectQueryComposer");

    try
    {
          XNameAccess xNameAccess = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class,
                                            ((XMultiServiceFactory)param.getMSF()).createInstance("com.sun.star.sdb.DatabaseContext"));
        // we use the first datasource
        XDataSource xDS = (XDataSource)UnoRuntime.queryInterface(XDataSource.class,
                                            xNameAccess.getByName( "Bibliography" ));

                log.println("check XMultiServiceFactory");
        XMultiServiceFactory xConn = (XMultiServiceFactory)UnoRuntime.queryInterface(XMultiServiceFactory.class, xDS.getConnection(new String(),new String()));

                log.println("check getAvailableServiceNames");
        String[] sServiceNames = xConn.getAvailableServiceNames();
                assure("Service 'SingleSelectQueryComposer' not supported" ,sServiceNames[0].equals("com.sun.star.sdb.SingleSelectQueryComposer"));
                XSingleSelectQueryComposer xComposerAndAnalyzer = (XSingleSelectQueryComposer)
                                            UnoRuntime.queryInterface(XSingleSelectQueryComposer.class,xConn.createInstance( sServiceNames[0]));

                log.println("check setQuery");
                xComposerAndAnalyzer.setQuery("SELECT * FROM \"biblio\"");
                assure("Query not identical", xComposerAndAnalyzer.getQuery().equals("SELECT * FROM \"biblio\""));

                // XSingleSelectQueryComposer
                log.println("check setFilter");
                // filter
                xComposerAndAnalyzer.setFilter("\"Identifier\" = 'BOR02b'");
                assure("Query not identical:" + xComposerAndAnalyzer.getFilter() + " -> \"Identifier\" = 'BOR02b'", xComposerAndAnalyzer.getFilter().equals("\"Identifier\" = 'BOR02b'"));

                log.println("check setGroup");
                // group by
                xComposerAndAnalyzer.setGroup("\"Identifier\"");
                assure("Query not identical:" + xComposerAndAnalyzer.getGroup() + " -> \"Identifier\"", xComposerAndAnalyzer.getGroup().equals("\"Identifier\""));

                log.println("check setOrder");
                // order by
                xComposerAndAnalyzer.setOrder("\"Identifier\"");
                assure("Query not identical:" + xComposerAndAnalyzer.getOrder() + " -> \"Identifier\"", xComposerAndAnalyzer.getOrder().equals("\"Identifier\""));

                log.println("check setHavingClause");
                // having
                xComposerAndAnalyzer.setHavingClause("\"Identifier\" = 'BOR02b'");
                assure("Query not identical:" + xComposerAndAnalyzer.getHavingClause() + " -> \"Identifier\" = 'BOR02b'", xComposerAndAnalyzer.getHavingClause().equals("\"Identifier\" = 'BOR02b'"));

                log.println("check getOrderColumns");
                // order by columns
                XIndexAccess xOrderColumns = xComposerAndAnalyzer.getOrderColumns();
                assure("Order columns doesn't exist -> \"Identifier\"", xOrderColumns != null && xOrderColumns.getCount() == 1 && xOrderColumns.getByIndex(0) != null);

                log.println("check getGroupColumns");
                // group by columns
                XIndexAccess xGroupColumns = xComposerAndAnalyzer.getGroupColumns();
                assure("Group columns doesn't exist -> \"Identifier\"", xGroupColumns != null && xGroupColumns.getCount() == 1 && xGroupColumns.getByIndex(0) != null);

                log.println("check getColumns");
                // XColumnsSupplier
                XColumnsSupplier xSelectColumns = (XColumnsSupplier)
                        UnoRuntime.queryInterface(XColumnsSupplier.class,xComposerAndAnalyzer);
                assure("Select columns doesn't exist", xSelectColumns != null && xSelectColumns.getColumns() != null && xSelectColumns.getColumns().getElementNames().length != 0);

                log.println("check structured filter");
                // structured filter
                xComposerAndAnalyzer.setQuery("SELECT \"Identifier\", \"Type\", \"Address\" FROM \"biblio\" \"biblio\"");
                xComposerAndAnalyzer.setFilter(complexFilter);
                PropertyValue[][] aStructuredFilter = xComposerAndAnalyzer.getStructuredFilter();
                xComposerAndAnalyzer.setFilter("");
                xComposerAndAnalyzer.setStructuredFilter(aStructuredFilter);
                assure("Structured Filter not identical" , xComposerAndAnalyzer.getFilter().equals(complexFilter));

                log.println("check structured having");
                // structured having clause
                xComposerAndAnalyzer.setHavingClause(complexFilter);
                PropertyValue[][] aStructuredHaving = xComposerAndAnalyzer.getStructuredHavingClause();
                xComposerAndAnalyzer.setHavingClause("");
                xComposerAndAnalyzer.setStructuredHavingClause(aStructuredHaving);
                assure("Structured Having Clause not identical" , xComposerAndAnalyzer.getHavingClause().equals(complexFilter));
    }
    catch(Exception e)
    {
            assure("Exception catched: " + e,false);
    }
    }
}
