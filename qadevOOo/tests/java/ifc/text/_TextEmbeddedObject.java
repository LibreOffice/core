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
package ifc.text;

import lib.MultiPropertyTest;


/**
 * Testing <code>com.sun.star.text.TextEmbeddedObject</code>
 * service properties :
 * <ul>
 *  <li><code> CLSID</code></li>
 *  <li><code> Model</code></li>
 *  <li><code> Component</code></li>
 * </ul> <p>
 * Properties testing is automated by <code>lib.MultiPropertyTest</code>.
 * @see com.sun.star.text.TextEmbeddedObject
 */
public class _TextEmbeddedObject extends MultiPropertyTest {

    /**
     * This property can't be set, as soon as the object is inserted
     * so the set method will be skipped.
     */
    public void _CLSID() {
        boolean result = false;

        try {
            String clsid = (String) oObj.getPropertyValue("CLSID");
            log.println("Getting " + clsid);
            log.println(
                    "According to the idl-description this property can't be set as soon as the Object is inserted in the document, so setting will be skipped");
            result=true;
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("Exception while getting Property 'CLSID' " +
                        e.getMessage());
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception while getting Property 'CLSID' " +
                        e.getMessage());
        }

        tRes.tested("CLSID", result);
    }
} // finish class _TextEmbeddedObject
