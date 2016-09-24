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

package ifc.frame;

import lib.MultiMethodTest;
import util.utils;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XStorable;
import com.sun.star.io.IOException;

/**
* Testing <code>com.sun.star.frame.XStorable</code>
* interface methods:
* <ul>
*  <li><code> getLocation() </code></li>
*  <li><code> hasLocation() </code></li>
*  <li><code> isReadonly() </code></li>
*  <li><code> storeAsURL() </code></li>
*  <li><code> storeToURL() </code></li>
*  <li><code> store() </code></li>
* </ul><p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.frame.XStorable
*/
public class _XStorable extends MultiMethodTest {
    public XStorable oObj = null; // oObj filled by MultiMethodTest
    String storeUrl;
    boolean stored;

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status in three cases:
    * <ol>
    *  <li>An object has location stored in. Then if method does not return
    *  null, it has <b> OK </b> status</li>
    *  <li>An object has no location stored in. Then method storeAsURL() is
    *  called, and if url is not null and equals to a url where component
    *  was stored, method has <b> OK </b> status</li>
    *  <li>An object has no location stored in. Then method storeAsURL() is
    *  called, and if url is null and method returns null too, method
    *  has <b> OK </b> status </li>
    * </ol><p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> storeAsURL() </code> : stores the object's persistent data
    *  to a URL and lets the object become the representation of this new
    *  URL</li>
    * </ul>
    */
    public void _getLocation() {
        if (oObj.hasLocation()) {
            // if it has location it should know it
            tRes.tested("getLocation()", oObj.getLocation() != null);
        } else {
            // else try to obtain location
            requiredMethod("storeAsURL()");
            if (storeUrl != null) {
                // if stored successfully - check location
                log.println(oObj.getLocation() + "--" + storeUrl);
                tRes.tested("getLocation()",
                    storeUrl.equals(oObj.getLocation()));
            } else {
                // if not - it should not have a location
                tRes.tested("getLocation()", oObj.getLocation() == null);
            }
        }
    }

    /**
    * Test calls the method, then result is checked. <p>
    * Has <b> OK </b> status if stored url is not null and method does not
    * return null or if stored url is null and the method returns null.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> storeAsURL() </code>: stores the object's persistent data
    *  to a URL and lets the object become the representation of this new
    *  URL</li>
    * </ul>
    */
    public void _hasLocation() {
        requiredMethod("storeAsURL()");
        if (storeUrl != null) {
            // if stored successfully - it should have a location
            tRes.tested("hasLocation()", oObj.hasLocation());
        } else {
            // if not - it should not
            tRes.tested("hasLocation()", !oObj.hasLocation());
        }
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if value, returned by the method is not equal to
    * 'stored' variable. ( If it's readonly it should not have been stored. )
    * <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> store() </code> : stores data to the URL from which it
    *  was loaded </li>
    * </ul>
    */
    public void _isReadonly() {
        requiredMethod("store()");
        tRes.tested("isReadonly()", oObj.isReadonly() != stored);
    }

    /**
    * Object is stored into temporary directory. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _storeAsURL() {
        // getting an url to store
        String url = utils.getOfficeTemp(
                                tParam.getMSF());

        if (url != null) {
            url += "xstorable.store.as.test";
            log.println("store as '" + url + "'");
            try {
                oObj.storeAsURL(url, new PropertyValue[0]);
                storeUrl = url;
                tRes.tested("storeAsURL()", true);
            } catch (IOException e) {
                log.println("Couldn't store as "+url+" : "+e.getMessage());
                e.printStackTrace(log);
                storeUrl = null;
                tRes.tested("storeAsURL()", false);
            }
        } else {
            log.println("an url to store is not found");
        }
    }

    /**
    * Object is stored into temporary directory. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _storeToURL() {
        // getting an url to store
        String url = utils.getOfficeTemp(
                                tParam.getMSF());

        if (url != null) {
            url += "xstorable.store.as.test";
            log.println("store to '" + url + "'");
            try {
                oObj.storeToURL(url, new PropertyValue[0]);
                tRes.tested("storeToURL()", true);
            } catch (IOException e) {
                log.println("Couldn't store to "+url+" : "+e.getMessage());
                e.printStackTrace(log);
                tRes.tested("storeToURL()", false);
            }
        } else {
            log.println("an url to store is not found");
        }
    }

    /**
    * Test calls the method. Then result is checked.<p>
    * Has <b> OK </b> status if:
    * <ol>
    *  <li>component was stored, object is not readonly and has location</li>
    *  <li>exception occurred because of component is readonly
    *  and wasn't stored</li>
    * </ol>
    */
    public void _store() {
        IOException ioE = null;

        try {
            oObj.store();
            stored = true;
        } catch (IOException e) {
            stored = false;
            ioE = e;
        }
        if (oObj.hasLocation() && !oObj.isReadonly()) {
            tRes.tested("store()", stored);
            if (!stored) {
                log.println("Couldn't store : " + ioE.getMessage());
                ioE.printStackTrace(log);
            }
        } else {
            tRes.tested("store()", !stored);
            if (stored) {
                if (!oObj.hasLocation()) {
                    log.println("Shouldn't store successfully"
                            + " a document without location");
                } else {
                    log.println("Shouldn't store successfully"
                            + " a read-only document");
                }
            }
        }
    }

}// finished class _XStorable

