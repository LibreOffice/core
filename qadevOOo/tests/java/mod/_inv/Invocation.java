/*************************************************************************
 *
 *  $RCSfile: Invocation.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 11:57:11 $
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

package mod._inv;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.script.Invocation</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::lang::XSingleServiceFactory</code></li>
* </ul>
* @see com.sun.star.script.Invocation
* @see com.sun.star.lang.XSingleServiceFactory
* @see ifc.lang._XSingleServiceFactory
*/
public class Invocation extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates service <code>com.sun.star.script.Invocation</code>.
    *     Object relations created :
    * <ul>
    *  <li> <code>'XSingleServiceFactory.createInstance.negative'</code> :
    *   for interface {@link _ifc.lang.XSingleServiceFactory} ;
    *   <code>String</code> relation; If its value 'true' then
    *   <code>createInstance</code> method for the object isn't
    *   supported. In this case object doesn't support this method.</li>
    *  <li> <code>'XSingleServiceFactory.arguments'</code> :
    *   for interface {@link _ifc.lang.XSingleServiceFactory} ;
    *   has <code>Object[]</code> type. This relation is used as
    *   a parameter for <code>createInstanceWithArguments</code>
    *   method call. If this relation doesn't exist test pass
    *   zerro length array as argument. Here
    *   <code>com.sun.star.io.Pipe</code> instance is passed.</li>
    *  <li> <code>'XSingleServiceFactory.MustSupport'</code> :
    *   for interface {@link _ifc.lang.XSingleServiceFactory}.
    *   Specifies that created instance must support
    *   <code>com.sun.star.script.XInvocation</code> interface.
    * </ul>
    */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();

        try {
            XInterface xInt = (XInterface)xMSF.createInstance(
                    "com.sun.star.script.Invocation");

            TestEnvironment tEnv = new TestEnvironment(xInt);

            // the createInstance should fail according to the documentation
            tEnv.addObjRelation(
                    "XSingleServiceFactory.createInstance.negative", "true");

            // creating parameters to createInstanceWithArguments
            Object[] args = new Object[1];

            args[0] = xMSF.createInstance("com.suns.star.io.Pipe");

            tEnv.addObjRelation(
                    "XSingleServiceFactory.arguments", args);

            tEnv.addObjRelation("XSingleServiceFactory.MustSupport",
                new Class[] {com.sun.star.script.XInvocation.class});

            return tEnv;
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }
    }
}
