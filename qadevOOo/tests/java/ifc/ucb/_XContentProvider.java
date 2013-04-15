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

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.ucb.XContent;
import com.sun.star.ucb.XContentIdentifier;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.ucb.XContentProvider;

/**
* Testing <code>com.sun.star.ucb.XContentProvider</code>
* interface methods :
* <ul>
*  <li><code> queryContent()</code></li>
*  <li><code> compareContentIds()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'FACTORY'</code> (of type
*   <code>com.sun.star.ucb.XContentIdentifierFactory</code>):
*   a suitable factory which can produce content identifiers </li>
*  <li> <code>'CONTENT1'</code> (<b>optional</b>) (of type <code>String</code>):
*   name of the suitable content for provider tested. If relation
*   is not specified the 'vnd.sun.star.help://' name will be used.</li>
*  <li> <code>'CONTENT2'</code> (<b>optional</b>) (of type <code>String</code>):
*   another name of the suitable content for provider tested. If relation
*   is not specified the 'vnd.sun.star.writer://' name will be used.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.ucb.XContentProvider
*/
public class _XContentProvider extends MultiMethodTest {

    public static XContentProvider oObj = null;
    protected XContentIdentifierFactory CIF = null ;
    protected String content1 = "vnd.sun.star.help://" ;
    protected String content2 = "vnd.sun.star.writer://" ;

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.
    */
    public void before() {
        CIF = (XContentIdentifierFactory) tEnv.getObjRelation("FACTORY");
        String tmp = (String) tEnv.getObjRelation("CONTENT1") ;
        if (tmp != null) content1 = tmp ;
        tmp = (String) tEnv.getObjRelation("CONTENT2") ;
        if (tmp != null) content2 = tmp ;

        if (CIF == null) throw new StatusException(
            Status.failed("'FACTORY' relation is not found.")) ;
    }

    /**
    * Tries to query for some content suitable for this provider. <p>
    * Has <b>OK</b> status if not null value is returned.
    */
    public void _queryContent() {
        try {
            XContentIdentifierFactory CIF = (XContentIdentifierFactory)
                                                tEnv.getObjRelation("FACTORY");
            String aURL = content1;
            log.println("Trying to query "+aURL);
            XContentIdentifier CI = CIF.createContentIdentifier(aURL);
            XContent aContent = oObj.queryContent(CI);
            boolean res = true;
            Object nc = tEnv.getObjRelation("NoCONTENT");
            if (nc == null) {
                res = aContent != null;
            }
            tRes.tested("queryContent()",res);
        } catch (com.sun.star.ucb.IllegalIdentifierException e) {
            log.println("Exception while checking 'queryContent'");
            e.printStackTrace(log);
            tRes.tested("queryContent()",false);
        }
    }

    /**
    * Creates two different content identifiers. First two different
    * identifiers compared, then two same identifiers. <p>
    * Has <b>OK</b> status if in the first case <code>false</code>
    * returned, and in the second - <code>true</code>.
    */
    public void _compareContentIds() {
        XContentIdentifierFactory CIF = (XContentIdentifierFactory)
                                            tEnv.getObjRelation("FACTORY");
        String aURL = content1 ;
        XContentIdentifier CI = CIF.createContentIdentifier(aURL);
        aURL = content2 ;
        XContentIdentifier CI2 = CIF.createContentIdentifier(aURL);
        int compare = oObj.compareContentIds(CI,CI2);
        boolean res = (compare != 0);
        if (!res) {
            log.println("Didn't work with different IDs");
            log.println(compare+" was returned");
        }
        compare = oObj.compareContentIds(CI,CI);
        res &= (compare == 0);
        if (!res) {
            log.println("Didn't work with equal IDs");
            log.println(compare+" was returned");
        }
        tRes.tested("compareContentIds()",res);
    }

}


