/*************************************************************************
 *
 *  $RCSfile: _XSingleServiceFactory.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:46:17 $
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

package ifc.lang;

import lib.MultiMethodTest;

import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.uno.UnoRuntime;


/**
/**
* Testing <code>com.sun.star.</code>
* interface methods :lang.XSingleServiceFactory
* <ul>
*  <li><code> createInstance()</code></li>
*  <li><code> createInstanceWithArguments()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XSingleServiceFactory.createInstance.negative'</code> :
*   <code>String</code> relation; If its value 'true' then
*   <code>createInstance</code> method for the object isn't
*   supported. </li>
*  <li> <code>'XSingleServiceFactory.arguments'</code> <b>(optional)</b>:
*   has <code>Object[]</code> type. This relation is used as
*   a parameter for <code>createInstanceWithArguments</code>
*   method call. If this relation doesn't exist test pass
*   zerro length array as argument. </li>
*  <li> <code>'XSingleServiceFactory.MustSupport'</code> <b>(optional)</b>:
*   of type <code>java.lang.Class[]</code>. This is an array of UNO
*   interface classes which must be supported by created instance.
*  </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* After test completion object environment has to be recreated.
* @see com.sun.star.lang.XSingleServiceFactory
*/
public class _XSingleServiceFactory extends MultiMethodTest {

    public XSingleServiceFactory oObj = null;
    private Class[] mustSupport = null ;

    public void before() {
        mustSupport = (Class[]) tEnv.getObjRelation
            ("XSingleServiceFactory.MustSupport") ;
    }

    /**
    * Just calls the method and check the value returned. <p>
    *
    * Has <b>OK</b> status in case if this method is supported
    * by object and non null value is returned, or if
    * this method isn't supported then the method call must
    * rise an exception or return <code>null</code> value.
    * If the relation exists which specifies required interfaces
    * supported by created instance then status is <b>OK</b>
    * if all these interfaces are supported.
    */
    public void _createInstance() {
        // for some objects the method should fail.
        // If thi is required the property is set to true.
        String negStr = (String)tEnv.getObjRelation(
                "XSingleServiceFactory.createInstance.negative");
        boolean negative = (negStr != null && negStr.equalsIgnoreCase("true"));

        if (negative) {
            log.println("Negative test: createInstance should fail");
        }

        try {
            log.println("Creating Instance: ");
            Object Inst = oObj.createInstance();
            boolean bOK = Inst != null ;

            if (mustSupport != null && bOK) {
                for (int i = 0; i < mustSupport.length; i++) {
                    Object ifc = UnoRuntime.queryInterface(mustSupport[i], Inst) ;
                    if (ifc == null) {
                        log.println(" !!! Created instance doesn't support " +
                            mustSupport[i].toString()) ;
                    }
                    bOK &= ifc != null ;
                }
            }

            tRes.tested("createInstance()",
                (negative && Inst == null) || (!negative && bOK));
        } catch (com.sun.star.uno.Exception ex) {
            log.println("Exception occured during createInstance()");
            if (negative) {
                ex.printStackTrace(log);
            }
            tRes.tested("createInstance()", negative);
        }
    }

    /**
    * Calls the method and checks the value returned. If relation
    * with method argument doesn't exist new zerro length array
    * is created. <p>
    * Has <b>OK</b> status if non null value is returned.
    * If the relation exists which specifies required interfaces
    * supported by created instance then status is <b>OK</b>
    * if all these interfaces are supported.
    */
    public void _createInstanceWithArguments() {
        Object[] arg = (Object[])tEnv.getObjRelation(
                "XSingleServiceFactory.arguments");

        if (arg == null) {
            arg = new Object[0];
        }

        try {
            boolean bOK = true ;
            log.println("Creating Instance with Argument");
            Object Inst = oObj.createInstanceWithArguments(arg);
            bOK &= Inst != null ;

            if (mustSupport != null) {
                for (int i = 0; i < mustSupport.length; i++) {
                    Object ifc = UnoRuntime.queryInterface(mustSupport[i], Inst) ;
                    if (ifc == null) {
                        log.println(" !!! Created instance doesn't support " +
                            mustSupport[i].toString()) ;
                    }
                    bOK &= ifc != null ;
                }
            }

            tRes.tested("createInstanceWithArguments()", bOK);
        }
        catch (com.sun.star.uno.Exception ex) {
            log.println("Exception occured during createInstanceWithArguments()");
            ex.printStackTrace(log);
            tRes.tested("createInstanceWithArguments()",false);
        }
    }

}  // finish class _XSingleServiceFactory


