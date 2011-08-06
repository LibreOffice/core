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

package ifc.document;


import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.PropertyValue;
import com.sun.star.document.XExporter;
import com.sun.star.document.XFilter;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.document.XFilter</code>
* interface methods :
* <ul>
*  <li><code> filter()</code></li>
*  <li><code> cancel()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'MediaDescriptor'</code> (of type <code>PropertyValue[]</code>):
*   the property set described in
*   <code>com.sun.star.document.MediaDescriptor</code>
*  </li>
*  <li> <code>'XFilter.Checker'</code> <b>(optional)</b> (of type
*   <code>ifc.document._XFilter.FilterChecker</code>) : implementation
*   of interface must allow checking that document was exported successfully.
*   If the relation doesn't exist then by default successfull filtering
*   assumed.
*  </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.document.XFilter
*/
public class _XFilter extends MultiMethodTest {

    public static interface FilterChecker {
        boolean checkFilter() ;
    }

    public XFilter oObj = null;
    protected PropertyValue[] mDesc = null ;
    protected FilterChecker checker = null ;
    protected XComponent sourceDoc = null;
    protected boolean dummy = false;

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.<br>
    * In case of Importers only a dummy implementation exists
    * therefore the methods of this interface will be skipped
    * in this case
    */
    public void before() {
        String name = tEnv.getTestCase().getObjectName();
        if (name.indexOf("Importer")>0) {
            log.println(name+" contains only a dummy implementation");
            log.println("therefore all methods are skipped");
            dummy = true;
        }
        mDesc = (PropertyValue[]) tEnv.getObjRelation("MediaDescriptor") ;
        checker = (FilterChecker) tEnv.getObjRelation("XFilter.Checker") ;
        if (mDesc == null && !dummy) throw new StatusException(
                                    Status.failed("Relation not found.")) ;
            sourceDoc = (XComponent)tEnv.getObjRelation("SourceDocument");
        try {
            if (sourceDoc != null) {
                XExporter xEx = (XExporter)UnoRuntime.queryInterface(
                                                    XExporter.class,oObj);
                xEx.setSourceDocument(sourceDoc);
            }
        }
        catch (com.sun.star.lang.IllegalArgumentException e) {}
    }

    public void after() {
        if (dummy) {
            throw new StatusException(Status.skipped(true));
        }
    }

    /**
    * Just calls the method. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _filter() {
        if (dummy) {
            tRes.tested("filter()", true);
            return;
        }
        boolean result = true ;
        result = oObj.filter(mDesc) ;

        if (checker == null) {
            log.println("!!! Warning : cann't check filter as no relation found");
        } else {
            result &= checker.checkFilter() ;
        }

        tRes.tested("filter()", result) ;
    }

    /**
    * Just calls the method. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _cancel() {
        if (dummy) {
            tRes.tested("cancel()",true);
            return;
        }
        requiredMethod("filter()");
        if (tEnv.getObjRelation("NoFilter.cancel()") != null) {
            System.out.println("Cancel not tested.");
            log.println("Method 'cancel()' is not working and therefore "+
                        "not tested.\nResult is set to SKIPPED.OK");
            tRes.tested("cancel()", Status.skipped(true));
            return;
        }

        boolean result = false ;
        FilterThread newFilter = new FilterThread(oObj);
        newFilter.mdesc = mDesc;
        newFilter.start();
        oObj.cancel();
        while (newFilter.isAlive()) {
        }
        result = !newFilter.filterRes;
        tRes.tested("cancel()", result) ;
    }

    /**
    * Calls <code>filter()</code> method in a separate thread.
    * Necessary to check if the cancel method works
    */
    protected class FilterThread extends Thread {

        public boolean filterRes = true ;
        private XFilter Filter = null ;
        public PropertyValue[] mdesc = null;

        public FilterThread(XFilter Filter) {
            this.Filter = Filter ;
        }

        public void run() {
            filterRes = Filter.filter(mdesc);
        }
    }

}



