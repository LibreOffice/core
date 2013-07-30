/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
     * Receives the object relations:
    * <ul>
    *  <li><code>XSingleSelectQueryComposer xCompoer</code></li>
    * </ul> <p>
     * @see com.sun.star.sdb.XSingleSelectQueryComposer
     */
    protected void before() {

        xComposer = UnoRuntime.queryInterface(XSingleSelectQueryComposer.class,
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
