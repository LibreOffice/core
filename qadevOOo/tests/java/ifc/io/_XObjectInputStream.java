/*************************************************************************
 *
 *  $RCSfile: _XObjectInputStream.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:43:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package ifc.io;

import lib.MultiMethodTest;
import lib.Status;
import util.ValueComparer;

import com.sun.star.beans.Property;
import com.sun.star.beans.XPropertySet;
import com.sun.star.io.XObjectInputStream;
import com.sun.star.io.XObjectOutputStream;
import com.sun.star.io.XPersistObject;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.io.XObjectInputStream</code>
* interface methods:
* <ul>
*   <li><code>readObject()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'PersistObject'</code> (of type <code>Object</code>):
*   object that supports interface <code>XPersistObject</code> </li>
* <ul> <p>
* After test completion object environment has to be recreated.
* @see com.sun.star.io.XObjectInputStream
* @see com.sun.star.io.XPersistObject
*/
public class _XObjectInputStream extends MultiMethodTest {

    public XObjectInputStream oObj = null;
    private Object objRead = null ;
    private Object objWrite = null ;

    /**
    * Test reads perisist object from stream and compares properties
    * of the object with properties of persist object obtained
    * from relation <code>'PersistObject'</code> <p>
    * Has <b> OK </b> status if returned value isn't null and values
    * of objects properties are equal. <p>
    */
    public void _readObject() {
        objWrite = tEnv.getObjRelation("PersistObject") ;
        if (objWrite == null) {
            log.println("PersistObject not found in relations") ;
            tRes.tested("readObject()", false) ;
            return ;
        }

        // write the object
        try {
            XObjectOutputStream oStream = (XObjectOutputStream)
                                    tEnv.getObjRelation("StreamWriter");
            oStream.writeObject((XPersistObject)objWrite);
        } catch(com.sun.star.io.IOException e) {
            log.println("Couldn't write object to stream");
            e.printStackTrace(log);
            tRes.tested("readObject()", Status.skipped(false));
            return;
        }

        try {
            objRead = oObj.readObject() ;
        } catch(com.sun.star.io.IOException e) {
            log.println("Couldn't read object from stream");
            e.printStackTrace(log);
            tRes.tested("readObject()", false) ;
            return ;
        }

        if (objRead == null) {
            log.println("No object was read.") ;
            tRes.tested("readObject()", false) ;
            return ;
        }

        XPropertySet props1 = null ;
        XPropertySet props2 = null ;

        props1 = (XPropertySet) UnoRuntime.queryInterface
            (XPropertySet.class, objRead) ;

        props2 = (XPropertySet) UnoRuntime.queryInterface
            (XPropertySet.class, objWrite) ;

        if (props1 == null) {
            log.println("Object read doesn't implement XPropertySet") ;
            tRes.tested("readObject()", false) ;
            return ;
        }
        if (props2 == null) {
            log.println("Object written doesn't implement XPropertySet") ;
            tRes.tested("readObject()", false) ;
            return ;
        }

        tRes.tested("readObject()",
            compareProperties(props1, props2)) ;
    }

    protected boolean compareProperties(XPropertySet props1,
        XPropertySet props2)  {

        Property[] p1 = props1.getPropertySetInfo().getProperties() ;
        Property[] p2 = props2.getPropertySetInfo().getProperties() ;

        if (p1.length != p2.length) {
            log.println("Number of properties differs") ;
            return false ;
        }

        boolean result = true ;

        for (int i = 0; i < p1.length; i++) {
            String propName = p1[i].Name ;

            log.print("Comparing property '" + propName + "' ...") ;
            boolean res = false ;
            try {
                res = ValueComparer.equalValue
                    (props1.getPropertyValue(propName),
                     props2.getPropertyValue(propName)) ;
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                log.println("Not found !") ;
            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println(e) ;
            }

            if (res)
                log.println("OK.") ;
            else
                log.println("Different !") ;

            result &= res ;
        }

        return result ;
    }

    /**
    * Forces object environment recreation.
    */
    public void after() {
        tEnv.dispose() ;
    }
}

