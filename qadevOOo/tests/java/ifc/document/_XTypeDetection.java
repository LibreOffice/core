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

package ifc.document;

import lib.MultiMethodTest;
import util.utils;

import com.sun.star.beans.PropertyValue;
import com.sun.star.document.XTypeDetection;

/**
 * Testing <code>com.sun.star.document.XTypeDetection</code>
 * interface methods :
 * <ul>
 *  <li><code> queryTypeByURL()</code></li>
 *  <li><code> queryTypeByDescriptor()</code></li>
 * </ul> <p>
 *
 * The following predefined files needed to complete the test:
 * <ul>
 *  <li> <code>XTypeDetection.sxw</code> : <b>Calc</b>
 *      document which extension is <b>sxw</b>. </li>
 * <ul> <p>
 *
 * @see com.sun.star.document.XTypeDetection
 */
public class _XTypeDetection extends MultiMethodTest {
    public XTypeDetection oObj = null;

    private String docURL = null;
    private String bookmarkURL = null;

    /**
     * Get the document URL.
     */
    @Override
    public void before() {
        docURL = utils.getFullTestURL("XTypeDetection.sxw");
        bookmarkURL =  (String) tEnv.getObjRelation("XTypeDetection.bookmarkDoc");
    }

    /**
      * Tries to detect type by writer document URL. <p>
      *
      * Has <b> OK </b> status if type returned contains
      * 'writer' as substring.
      */
    public void _queryTypeByURL() {

        boolean result = true ;
        String type = oObj.queryTypeByURL(docURL) ;
        result &= type.indexOf("writer") > -1;

        tRes.tested("queryTypeByURL()", result) ;
    }

    /**
     * Tries to detect type of the document using <i>flat</i>
     * and <i>deep</i> detection. For flat detection a writer type
     * must be returned (file has writer extension), but deep
     * detection must return calc type (document has calc contents)<p>
     *
     * Has <b> OK </b> status if for the first case type contains
     * 'writer' string and for the second 'calc' string.
     */
    public void _queryTypeByDescriptor() {
        boolean result = true ;
        boolean ok = true;
        log.println("test document with wrong extension");
        log.println("the document '" + docURL + "' is not what it seems to be ;-)");
        PropertyValue[][] mediaDescr = new PropertyValue[1][1];
        mediaDescr[0][0] = new PropertyValue();
        mediaDescr[0][0].Name = "URL";
        mediaDescr[0][0].Value = docURL;

        String type = oObj.queryTypeByDescriptor(mediaDescr, false);
        ok = type.indexOf("writer") > -1;
        result &= ok;
        log.println("flat detection should detect a writer and has detected '"+ type +"': " + ok);

        type = oObj.queryTypeByDescriptor(mediaDescr, true);
        ok = type.indexOf("calc") > -1;
        result &= ok;
        log.println("deep detection should detect a calc and has detected '"+ type +"': " + ok);

        log.println("test dokument with bookmark: " + bookmarkURL);
        mediaDescr = new PropertyValue[1][1];
        mediaDescr[0][0] = new PropertyValue();
        mediaDescr[0][0].Name = "URL";
        mediaDescr[0][0].Value = bookmarkURL;
        type = oObj.queryTypeByDescriptor(mediaDescr, true);
        ok = type.indexOf("writer") > -1;
        result &= ok;
        log.println("deep detection should detect a writer and has detected '"+ type +"': " + ok);

        tRes.tested("queryTypeByDescriptor()", result) ;
    }
}

