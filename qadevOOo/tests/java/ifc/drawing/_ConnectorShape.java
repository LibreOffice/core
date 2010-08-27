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

