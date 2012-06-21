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

package ifc.form;

import lib.MultiMethodTest;

import com.sun.star.awt.XImageProducer;
import com.sun.star.form.XImageProducerSupplier;

/**
* Testing <code>com.sun.star.form.XImageProducerSupplier</code>
* interface methods :
* <ul>
*  <li><code> getImageProducer()</code></li>
* </ul>
* @see com.sun.star.form.XImageProducerSupplier
*/
public class _XImageProducerSupplier extends MultiMethodTest {

    public XImageProducerSupplier oObj = null;

    /**
    * Test calls the method and checks return value and that
    * no exceptions were thrown. <p>
    * Has <b> OK </b> status if the method returns non null value
    * and no exceptions were thrown. <p>
    */
    public void _getImageProducer() {
        boolean bResult = false;
        XImageProducer ip = oObj.getImageProducer();
        bResult = ip!=null;
        tRes.tested("getImageProducer()", bResult);
    }
}


