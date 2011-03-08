/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package ifc.sdb;

import com.sun.star.sdb.XSingleSelectQueryAnalyzer;
import lib.MultiMethodTest;
import com.sun.star.sdb.XSingleSelectQueryComposer;
import com.sun.star.uno.UnoRuntime;
import lib.StatusException;
import lib.Status;
import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XIndexAccess;

/**
* Testing <code>com.sun.star.sdb.XSingleSelectQueryAnalyzer</code>
* interface methods :
* <ul>
*  <li><code>getQuery()</code></li>
*  <li><code>setQuery()</code></li>
*  <li><code>getFilter()</code></li>
*  <li><code>getStructuredFilter()</code></li>
*  <li><code>getGroup()</code></li>
*  <li><code>getGroupColumns()</code></li>
*  <li><code>getHavingClause()</code></li>
*  <li><code>getStructuredHavingClause()</code></li>
*  <li><code>getOrder()</code></li>
*  <li><code>getOrderColumns()</code></li>

* </ul> <p>
* @see com.sun.star.sdb.XSingleSelectQueryAnalyzer
*/
public class _XSingleSelectQueryAnalyzer extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XSingleSelectQueryAnalyzer oObj = null ;

    private String queryString = "SELECT * FROM \"biblio\"";

    private XSingleSelectQueryComposer xComposer = null;

    /**
     * Recieves the object relations:
    * <ul>
    *  <li><code>XSingleSelectQueryComposer xCompoer</code></li>
    * </ul> <p>
     * @see om.sun.star.sdb.XSingleSelectQueryComposer
     */
    protected void before() {

        xComposer = (XSingleSelectQueryComposer)
                      UnoRuntime.queryInterface(XSingleSelectQueryComposer.class,
                      tEnv.getObjRelation("xComposer"));

        if (xComposer == null) {
            throw new StatusException(Status.failed(
           "Couldn't get object relation 'xComposer'. Test must be modified"));

        }

    }
    /**
    * call <code>setQuery()</code> once with valid query, once with invalid
    * query. Has ok if only on sceond call <code>SQLException</code> was thrwon
    */
    public void _setQuery() {

        try{
            oObj.setQuery("This is an invalid SQL query");
        } catch (com.sun.star.sdbc.SQLException e){
            log.println("expected Exception. ");
        }

        try{
            oObj.setQuery(queryString);
        } catch (com.sun.star.sdbc.SQLException e){
            log.println("unexpected Exception: " + e.toString());
            tRes.tested("setQuery()", false);
        }
        tRes.tested("setQuery()", true);
    }

    /**
    * checks of the returned value of <code>getQuery()</code>
    * equals the string which was set by <code>setQuery()</code>
    * <p>
    * required methods:
    *<ul>
    * <li><code>setQuery</code></li>
    *</ul>
    */
    public void _getQuery() {
        this.requiredMethod("setQuery()");

        boolean res = false;

        res = oObj.getQuery().equals(queryString);

        tRes.tested("getQuery()", res);
    }


    /**
    * Object relation <code>xComposer</code> set a filter. This filter
    * must returned while calling <code>getFilter</code>
    */
    public void _getFilter() {
        try{
            String filter = "\"Identifier\" = 'BOR02b'";
            xComposer.setFilter(filter);
            tRes.tested("getFilter()", (oObj.getFilter().equals(filter)));

        } catch (com.sun.star.sdbc.SQLException e){
            log.println("unexpected Exception: " + e.toString());
            tRes.tested("getFilter()", false);
        }
    }

    /**
    * Object relation <code>xComposer</code> set a complex filter with method
    . <code>setFilter</code>. Then <code>getStructuredFilter</code> returns a
    * sequenze of <code>PropertyValue</code> which was set with method
    * <code>setStructuredFilter</code> from <xComposer>.
    * Then test has ok status if <getFilter> returns the complex filter.
    * <p>
    * required methods:
    *<ul>
    * <li><code>setQuery</code></li>
    * <li><code>getFilter</code></li>
    *</ul>
    */
    public void _getStructuredFilter() {
        requiredMethod("setQuery()");
        requiredMethod("getFilter()");
        try{
            oObj.setQuery("SELECT \"Identifier\", \"Type\", \"Address\" FROM \"biblio\" \"biblio\"");
            String complexFilter = "( \"Identifier\" = '1' AND \"Type\" = '4' ) OR ( \"Identifier\" = '2' AND \"Type\" = '5' ) OR ( \"Identifier\" = '3' AND \"Type\" = '6' AND \"Address\" = '7' ) OR ( \"Address\" = '8' ) OR ( \"Type\" = '9' )";
            xComposer.setFilter(complexFilter);
            PropertyValue[][] aStructuredFilter = oObj.getStructuredFilter();
            xComposer.setFilter("");
            xComposer.setStructuredFilter(aStructuredFilter);
            tRes.tested("getStructuredFilter()", (oObj.getFilter().equals(complexFilter)));

        } catch (com.sun.star.sdbc.SQLException e){
            log.println("unexpected Exception: " + e.toString());
            tRes.tested("getStructuredFilter()", false);
        } catch (com.sun.star.lang.IllegalArgumentException e){
            log.println("unexpected Exception: " + e.toString());
            tRes.tested("getStructuredFilter()", false);
        }
    }

    /**
    * Object relation <code>xComposer</code> set a goup. This group
    * must returned while calling <code>getGroup</code>
    */
    public void _getGroup() {
        try{
            String group = "\"Identifier\"";
            xComposer.setGroup(group);
            tRes.tested("getGroup()", (oObj.getGroup().equals(group)));

        } catch (com.sun.star.sdbc.SQLException e){
            log.println("unexpected Exception: " + e.toString());
            tRes.tested("getGroup()", false);
        }
    }

    /**
    * Method <code>getGroupColumns</code> retunrs a <code>XIndexAccess</code>
    * Test has ok status if returned value is an useable <code>XIndexAccess</code>
    */
    public void _getGroupColumns() {
        try{
           XIndexAccess xGroupColumns = oObj.getGroupColumns();

           tRes.tested("getGroupColumns()", (xGroupColumns != null &&
                                            xGroupColumns.getCount() == 1 &&
                                        xGroupColumns.getByIndex(0) != null));

        } catch (com.sun.star.lang.IndexOutOfBoundsException e){
            log.println("unexpected Exception: " + e.toString());
            tRes.tested("getGroupColumns()", false);
        } catch (com.sun.star.lang.WrappedTargetException e){
            log.println("unexpected Exception: " + e.toString());
            tRes.tested("getGroupColumns()", false);
        }
    }

    /**
    * Object relation <code>xComposer</code> set a clause. This clause
    * must returned while calling <code>getHavingClause</code>
    */
    public void _getHavingClause() {
        try{
            String clause = "\"Identifier\" = 'BOR02b'";
            xComposer.setHavingClause(clause);
            tRes.tested("getHavingClause()", (
                                        oObj.getHavingClause().equals(clause)));

        } catch (com.sun.star.sdbc.SQLException e){
            log.println("unexpected Exception: " + e.toString());
            tRes.tested("getHavingClause()", false);
        }
    }

    /**
    * Object relation <code>xComposer</code> set a clause. This clause
    * must returned while calling <code>getHavingClause</code>
    * <p>
    * required methods:
    *<ul>
    * <li><code>setQuery</code></li>
    * <li><code>getFilter</code></li>
    * <li><code>getStructuredFilter</code></li>
    *</ul>
    */
    public void _getStructuredHavingClause() {
        requiredMethod("setQuery()");
        requiredMethod("getFilter()");
        executeMethod("getStructuredFilter()");
        String complexFilter = "( \"Identifier\" = '1' AND \"Type\" = '4' ) OR ( \"Identifier\" = '2' AND \"Type\" = '5' ) OR ( \"Identifier\" = '3' AND \"Type\" = '6' AND \"Address\" = '7' ) OR ( \"Address\" = '8' ) OR ( \"Type\" = '9' )";

        try{
           xComposer.setHavingClause(complexFilter);
           PropertyValue[][] aStructuredHaving = oObj.getStructuredHavingClause();
           xComposer.setHavingClause("");
           xComposer.setStructuredHavingClause(aStructuredHaving);
           tRes.tested("getStructuredHavingClause()",
                                (oObj.getHavingClause().equals(complexFilter)));

        } catch (com.sun.star.sdbc.SQLException e){
            log.println("unexpected Exception: " + e.toString());
            tRes.tested("getStructuredHavingClause()", false);
        }
    }

    /**
    * Object relation <code>xComposer</code> set an order. This order
    * must returned while calling <code>getOrder</code>
    */
    public void _getOrder() {
        try{
            String order = "\"Identifier\"";
            xComposer.setOrder(order);
            tRes.tested("getOrder()", (oObj.getOrder().equals(order)));

        } catch (com.sun.star.sdbc.SQLException e){
            log.println("unexpected Exception: " + e.toString());
            tRes.tested("getOrder()", false);
        }
    }

    /**
    * Method <code>getGroupColumns</code> retunrs a <code>XIndexAccess</code>
    * Test has ok status if returned value is an useable <code>XIndexAccess</code>
    */
    public void _getOrderColumns() {
        try{
            XIndexAccess xOrderColumns = oObj.getOrderColumns();
            tRes.tested("getOrderColumns()", (xOrderColumns != null &&
                                              xOrderColumns.getCount() == 1 &&
                                         xOrderColumns.getByIndex(0) != null));

        } catch (com.sun.star.lang.IndexOutOfBoundsException e){
            log.println("unexpected Exception: " + e.toString());
            tRes.tested("getOrderColumns()", false);
        } catch (com.sun.star.lang.WrappedTargetException e){
            log.println("unexpected Exception: " + e.toString());
            tRes.tested("getOrderColumns()", false);
        }
    }


}  // finish class _XSingleSelectQueryAnalyzer
