/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _ConnectorShape.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:31:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

