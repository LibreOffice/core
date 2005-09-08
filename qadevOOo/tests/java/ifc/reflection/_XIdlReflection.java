/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XIdlReflection.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:21:54 $
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

package ifc.reflection;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.reflection.XIdlClass;
import com.sun.star.reflection.XIdlReflection;
import com.sun.star.uno.TypeClass;
import lib.MultiMethodTest;

/**
* Testing <code>com.sun.star.reflection.XIdlReflection</code>
* interface methods :
* <ul>
*  <li><code> forName()</code></li>
*  <li><code> getType()</code></li>
* </ul> <p>
* @see com.sun.star.reflection.XIdlReflection
*/
public class _XIdlReflection extends MultiMethodTest{
    public XIdlReflection oObj = null;
    protected final static String typeName = "com.sun.star.container.XNameAccess";

    /**
    * Test calls the method and checks returned interface
    * <code>com.sun.star.container.XNameAccess</code>: gets the name and the
    * type and checks it. <p>
    * Has <b> OK </b> status if returned name is equal to the name of the
    * interface that was passed as parameter in the method call and if returned
    * type is equal to <code>com.sun.star.uno.TypeClass.INTERFACE</code>. <p>
    */
    public void _forName() {
        boolean result = true;
        XIdlClass cls = oObj.forName(typeName);

        if (cls != null) {
            log.println("Class name: " + cls.getName());
            result &= cls.getTypeClass() == TypeClass.INTERFACE;
            result &= typeName.equals(cls.getName());
        } else {
            log.println("Method returned null");
            result = false;
        }

        tRes.tested("forName()", result);
    }

    /**
    * Test creates the instance of <code>com.sun.star.io.Pipe</code>,
    * calls the method using this instance as parameter and checks returned
    * value. <p>
    * Has <b> OK </b> status if the instance was created successfully, if
    * returned value isn't null and no exceptions were thrown. <p>
    */
    public void _getType() {
        boolean result = true;
        Object obj = null;

        try {
            obj = ((XMultiServiceFactory)tParam.getMSF()).
                                    createInstance("com.sun.star.io.Pipe") ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Can't create object");
            tRes.tested("getType()", false);
            return;
        }

        if (obj == null) {
            result = false;
            log.println("Object wasn't created !");
            tRes.tested("getType()", false);
        }

        XIdlClass cls = oObj.getType(obj);

        log.println("The name is " + cls.getName());

        tRes.tested("getType()", cls != null);
    }
}


