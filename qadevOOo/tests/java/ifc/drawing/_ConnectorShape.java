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

import com.sun.star.drawing.XShape;
/**
* Testing <code>com.sun.star.drawing.ConnectorShape</code>
* service properties :
* <ul>
*  <li><code> EdgeLine1Delta</code></li>
*  <li><code> EdgeLine2Delta</code></li>
*  <li><code> EdgeLine3Delta</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.drawing.ConnectorShape
*/
public class _ConnectorShape extends MultiPropertyTest {
    public XShape[] conShapes = null;

    @Override
    protected void before() {
        conShapes = (XShape[]) tEnv.getObjRelation("XConnectorShape.Shapes");
    }

    public void _EndShape() {
        Object oldProp = null;
        try {
            oldProp = oObj.getPropertyValue("EndShape");
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace();
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace();
        }
        testProperty("EndShape",oldProp,conShapes[0]);
    }

    public void _StartShape() {
        Object oldProp = null;
        log.println("Special for StartShape");
        try {
            oldProp = oObj.getPropertyValue("StartShape");
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace();
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace();
        }
        testProperty("StartShape",oldProp,conShapes[1]);
    }


}

