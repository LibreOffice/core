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

package ifc.ucb;

import com.sun.star.beans.Property;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.ucb.Command;
import com.sun.star.ucb.NumberedSortingInfo;
import com.sun.star.ucb.OpenCommandArgument2;
import com.sun.star.ucb.OpenMode;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.ucb.XContent;
import com.sun.star.ucb.XContentIdentifier;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.ucb.XContentProvider;
import com.sun.star.ucb.XDynamicResultSet;
import com.sun.star.ucb.XSortedDynamicResultSetFactory;
import com.sun.star.uno.UnoRuntime;
import lib.MultiMethodTest;

import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;

/**
* Testing <code>com.sun.star.ucb.XSortedDynamicResultSetFactory</code>
* interface methods :
* <ul>
*  <li><code> createSortedDynamicResultSet()</code></li>
* </ul> <p>
* The following predefined files needed to complete the test:
* <ul>
*  <li> <code>solibrary.jar</code> : is used to retrieve
*   content of its root directory.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.ucb.XSortedDynamicResultSetFactory
*/
public class _XSortedDynamicResultSetFactory extends MultiMethodTest {

    /**
     * Conatins the tested object.
     */
    public XSortedDynamicResultSetFactory oObj;

    /**
     * Creates sorted dynamic result set from result set. For this
     * a dynamic result set is to be created. It is created by
     * retrieving content list from JAR archive.
     * Has <b>OK</b> status if numbers of rows are equal and they are
     * greater then 0 (because JAR file contains at least one entry).
     */
    public void _createSortedDynamicResultSet() {
        boolean result = true ;

        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
        XDynamicResultSet dynResSet = null ;
        try {
            Object oUCB = xMSF.createInstanceWithArguments
                ("com.sun.star.ucb.UniversalContentBroker",
                new Object[0]) ;

            XContentIdentifierFactory ciFac = UnoRuntime.queryInterface
                (XContentIdentifierFactory.class,oUCB) ;

            String url = util.utils.getFullTestURL("SwXTextEmbeddedObject.sxw") ;
            String escUrl = "" ;

            // In base URL of a JAR file in content URL all directory
            // separators ('/') must be replaced with escape symbol '%2F'.
            int idx = url.indexOf("/") ;
            int lastIdx = -1 ;
            while (idx >= 0) {
                escUrl += url.substring(lastIdx + 1, idx) + "%2F" ;
                lastIdx = idx ;
                idx = url.indexOf("/", idx + 1) ;
            }
            escUrl += url.substring(lastIdx + 1) ;
            String cntUrl = "vnd.sun.star.pkg://" + escUrl + "/" ;

            XContentIdentifier CI = ciFac.createContentIdentifier(cntUrl) ;

            XContentProvider cntProv = UnoRuntime.queryInterface(XContentProvider.class, oUCB) ;

            XContent cnt = cntProv.queryContent(CI) ;

            XCommandProcessor cmdProc = UnoRuntime.queryInterface(XCommandProcessor.class, cnt) ;

            Property prop = new Property() ;
            prop.Name = "Title" ;

            Command cmd = new Command("open", -1, new OpenCommandArgument2
                (OpenMode.ALL, 10000, null, new Property[] {prop},
                 new NumberedSortingInfo[0])) ;

            dynResSet = (XDynamicResultSet) AnyConverter.toObject(
                new Type(XDynamicResultSet.class),cmdProc.execute(cmd, 0, null));
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
        }

        XDynamicResultSet sortedSet = oObj.createSortedDynamicResultSet
            (dynResSet, new NumberedSortingInfo[0], null) ;

        int rowCount = -1 ;
        if (sortedSet != null) {
            XResultSet set = null ;
            try {
                set = sortedSet.getStaticResultSet() ;
            } catch (com.sun.star.ucb.ListenerAlreadySetException e) {
                e.printStackTrace(log);
            }

            try {
                set.last() ;
                rowCount = set.getRow();
                log.println("Number of rows in result set: " + rowCount);
            } catch (com.sun.star.sdbc.SQLException e) {
                log.println("Exception occurred while accessing "+
                    "sorted result set :");
                e.printStackTrace(log);
            }
        } else {
            log.println("Null returned !!!");
            result &= false ;
        }

        result &= rowCount > 1 ;

        tRes.tested("createSortedDynamicResultSet()", result) ;
    }


}
