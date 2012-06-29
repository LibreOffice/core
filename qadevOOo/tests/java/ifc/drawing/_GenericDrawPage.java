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

package ifc.drawing;

import lib.MultiPropertyTest;


/**
* Testing <code>com.sun.star.drawing.GenericDrawPage</code>
* service properties :
* <ul>
*  <li><code> BorderBottom</code></li>
*  <li><code> BorderLeft</code></li>
*  <li><code> BorderRight</code></li>
*  <li><code> BorderTop</code></li>
*  <li><code> Height</code></li>
*  <li><code> Width</code></li>
*  <li><code> Number</code></li>
*  <li><code> Orientation</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.drawing.GenericDrawPage
*/
public class _GenericDrawPage extends MultiPropertyTest {

    public void _Number() {
        log.println("Number started");
        boolean res = false;
        log.println("trying to get the value");
        try {
            Short wat = (Short) oObj.getPropertyValue("Number");
            if (wat == null) {
                log.println("it is null");
            } else {
                log.println("it isn't null");
                res=true;
            }
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("an UnknownPropertyException occurred");

        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("an WrappedTargetException occurred");
        }

        tRes.tested("Number",res);
    }

    public void _UserDefinedAttributes() {
        log.println("Userdefined started");
        boolean res = false;
        log.println("trying to get the value");
        try {
            Object wat = oObj.getPropertyValue("UserDefinedAttributes");
            if (wat == null) {
                log.println("it is null");
            } else {
                log.println("it isn't null");
                res=true;
            }
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("an UnknownPropertyException occurred");

        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("an WrappedTargetException occurred");
        }

        tRes.tested("UserDefinedAttributes",res);
    }

} // end of GenericDrawPage

