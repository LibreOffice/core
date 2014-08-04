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

import lib.MultiMethodTest;

import com.sun.star.container.XIndexContainer;
import com.sun.star.drawing.XGluePointsSupplier;

/**
* Testing <code>com.sun.star.drawing.XGluePointsSupplier</code>
* interface methods :
* <ul>
*  <li><code> getGluePoints()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.drawing.XGluePointsSupplier
*/
public class _XGluePointsSupplier extends MultiMethodTest {
    public XGluePointsSupplier oObj = null;

    /**
    * Gets glue points collection. <p>
    * Has <b> OK </b> status if the value returned is not null. <p>
    */
    public void _getGluePoints(){
        boolean result = true;
        XIndexContainer points = null;

        log.println("testing getDrawPages() ... ");

        points = oObj.getGluePoints();
        result = points != null;

        tRes.tested("getGluePoints()", result);
    }

}

