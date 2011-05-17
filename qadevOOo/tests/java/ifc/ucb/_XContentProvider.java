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
            log.println("Didn't work with differnt IDs");
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


