/*************************************************************************
 *
 *  $RCSfile: SingleSelectQueryComposer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-03-02 12:39:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
                XSingleSelectQueryAnalyzer xQueryAna = (XSingleSelectQueryAnalyzer)
                                            UnoRuntime.queryInterface(XSingleSelectQueryAnalyzer.class,xConn.createInstance( sServiceNames[0]));

                log.println("check setQuery");
                xQueryAna.setQuery("SELECT * FROM \"biblio\"");
                assure("Query not identical", xQueryAna.getQuery().equals("SELECT * FROM \"biblio\""));

                // XSingleSelectQueryComposer
                XSingleSelectQueryComposer xComposer = (XSingleSelectQueryComposer)
                        UnoRuntime.queryInterface(XSingleSelectQueryComposer.class,xQueryAna);

                log.println("check setFilter");
                // filter
                xComposer.setFilter("\"Identifier\" = 'BOR02b'");
                assure("Query not identical:" + xQueryAna.getFilter() + " -> \"Identifier\" = 'BOR02b'", xQueryAna.getFilter().equals("\"Identifier\" = 'BOR02b'"));

                log.println("check setGroup");
                // group by
                xComposer.setGroup("\"Identifier\"");
                assure("Query not identical:" + xQueryAna.getGroup() + " -> \"Identifier\"", xQueryAna.getGroup().equals("\"Identifier\""));

                log.println("check setOrder");
                // order by
                xComposer.setOrder("\"Identifier\"");
                assure("Query not identical:" + xQueryAna.getOrder() + " -> \"Identifier\"", xQueryAna.getOrder().equals("\"Identifier\""));

                log.println("check setHavingClause");
                // having
                xComposer.setHavingClause("\"Identifier\" = 'BOR02b'");
                assure("Query not identical:" + xQueryAna.getHavingClause() + " -> \"Identifier\" = 'BOR02b'", xQueryAna.getHavingClause().equals("\"Identifier\" = 'BOR02b'"));

                log.println("check getOrderColumns");
                // order by columns
                XIndexAccess xOrderColumns = xQueryAna.getOrderColumns();
                assure("Order columns doesn't exist -> \"Identifier\"", xOrderColumns != null && xOrderColumns.getCount() == 1 && xOrderColumns.getByIndex(0) != null);

                log.println("check getGroupColumns");
                // group by columns
                XIndexAccess xGroupColumns = xQueryAna.getGroupColumns();
                assure("Group columns doesn't exist -> \"Identifier\"", xGroupColumns != null && xGroupColumns.getCount() == 1 && xGroupColumns.getByIndex(0) != null);

                log.println("check getColumns");
                // XColumnsSupplier
                XColumnsSupplier xSelectColumns = (XColumnsSupplier)
                        UnoRuntime.queryInterface(XColumnsSupplier.class,xQueryAna);
                assure("Select columns doesn't exist", xSelectColumns != null && xSelectColumns.getColumns() != null && xSelectColumns.getColumns().getElementNames().length != 0);

                log.println("check structured filter");
                // structured filter
                xQueryAna.setQuery("SELECT \"Identifier\", \"Type\", \"Address\" FROM \"biblio\" \"biblio\"");
                xComposer.setFilter(complexFilter);
                PropertyValue[][] aStructuredFilter = xQueryAna.getStructuredFilter();
                xComposer.setFilter("");
                xComposer.setStructuredFilter(aStructuredFilter);
                assure("Structured Filter not identical" , xQueryAna.getFilter().equals(complexFilter));

                log.println("check structured having");
                // structured having clause
                xComposer.setHavingClause(complexFilter);
                PropertyValue[][] aStructuredHaving = xQueryAna.getStructuredHavingFilter();
                xComposer.setHavingClause("");
                xComposer.setStructuredHavingFilter(aStructuredHaving);
                assure("Structured Having Clause not identical" , xQueryAna.getHavingClause().equals(complexFilter));
    }
    catch(Exception e)
    {
            assure("Exception catched: " + e,false);
    }
    }
}
