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

