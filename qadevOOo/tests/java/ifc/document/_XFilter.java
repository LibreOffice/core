/*************************************************************************
 *
 *  $RCSfile: _XFilter.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:26:55 $
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
    * Has <b> OK </b> status if no runtime exceptions occured
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
    * Has <b> OK </b> status if no runtime exceptions occured
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



