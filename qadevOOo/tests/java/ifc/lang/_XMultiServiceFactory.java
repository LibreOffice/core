/*************************************************************************
 *
 *  $RCSfile: _XMultiServiceFactory.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:45:44 $
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

import com.sun.star.lang.XMultiServiceFactory;


/**
* Testing <code>com.sun.star.lang.XMultiServiceFactory</code>
* interface methods:
* <ul>
*   <li><code>createInstance()</code></li>
*   <li><code>createInstanceWithArguments()</code></li>
*   <li><code>getAvailableServiceNames()</code></li>
* </ul> <p>
*
* This test needs the following object relations :
* <ul>
*  <li> <code>'XMSF.serviceNames'</code> (of type <code>String[]</code>)
*    <b>optional</b>:
*    the relation used when service names are obtained the way
*    other than calling <code>getAvailableServiceNames()</code>
*    method.
*  </li>
*  <li> <code>'XMSF.serviceNamesWithArgs'</code> (of type <code>String[]</code>)
*    <b>optional</b>:
*    the relation used when service names are obtained the way
*    other than calling <code>getAvailableServiceNames()</code>
*    method for testing <code>createInstanceWithArguments</code> method.
*  </li>
*  <li> <code>'XMSF.Args'</code> (of type <code>Object[][]</code>)
*    <b>optional</b>:
*    if this relation exists than the method
*    <code>createInstanceWithArguments</code> is tested. This relation
*    supplies arguments for creating instances. If the relation
*    <code>'XMSF.serviceNamesWithArgs'</code> is also specified
*    then for each service name from that relation appropriate arguments
*    are used from arguments array. If not than arguments with index
*    0 are used for services creation obtained by
*    <code>getAvailableServiceNames</code> method.
*  </li>
* </ul> <p>
*
* @see com.sun.star.lang.XMultiServiceFactory
*/
public class _XMultiServiceFactory extends MultiMethodTest {
    public XMultiServiceFactory oObj = null;
    public String[] services = null;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getAvailableServiceNames() {
        services = oObj.getAvailableServiceNames();

        for (int i = 0; i < services.length; i++) {
            log.println("Service" + i + ": " + services[i]);
        }

        tRes.tested("getAvailableServiceNames()", services != null);
    }

    /**
     * Test creates instance of the first service from names array
     * get by <code>getAvailableServiceNames()</code>. If the array
     * is empty than test looks for names from relation. <p>
     *
     * Has <b> OK </b> status if created instance isn't null. <p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getAvailableServiceNames() </code> : to have list of
     *  supported services </li>
     * </ul>
     */
    public void _createInstance() {
        requiredMethod("getAvailableServiceNames()");

        if (services.length == 0) {
            services = (String[]) tEnv.getObjRelation("XMSF.serviceNames");

            if (services == null) {
                log.println("No service to create.");
                tRes.tested("createInstance()", true);

                return;
            }
        }

        String needArgs = (String) tEnv.getObjRelation("needArgs");

        if (needArgs != null) {
            log.println("The " + needArgs +
                        " doesn't support createInstance without arguments");
            tRes.tested("createInstance()", true);

            return;
        }

        boolean res = true;

        for (int k = 0; k < services.length; k++) {
            try {
                log.println("Creating Instance: " + services[k]);

                Object Inst = oObj.createInstance(services[k]);
                res = (Inst != null);
            } catch (com.sun.star.uno.Exception ex) {
                log.println("Exception occured during createInstance()");
                ex.printStackTrace(log);
                res = false;
            }
        }

        tRes.tested("createInstance()", res);
    }

    /**
     * If the relation with arguments is not specified test does nothing.
     * In other case it tries to create instance by its name from
     * relation of from array <code>getAvailableServiceNames()</code>
     * method supplied. <p>
     *
     * Has <b> OK </b> status if the created instance is not null. <p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getAvailableServiceNames() </code> : to have list of
     *  supported services </li>
     * </ul>
     */
    public void _createInstanceWithArguments() {
        requiredMethod("getAvailableServiceNames()");

        Object[][] args = (Object[][]) tEnv.getObjRelation("XMSF.Args");
        String[] sNames = (String[]) tEnv.getObjRelation(
                                    "XMSF.serviceNamesWithArgs");

        if (args == null) {
            log.println("Relation 'XMSF.serviceNamesWithArgs' not found");
            log.println("The component assumed not support " +
                        "createInstanceWithArguments()");
            tRes.tested("createInstanceWithArguments()", true);
        } else {
            if (sNames == null) {
                sNames = services;
            }

            boolean res = true;

            for (int k = 0; k < sNames.length; k++) {
                log.println("Creating service '" + sNames[k] +
                            "' with arguments");

                try {
                    Object Inst = oObj.createInstanceWithArguments(sNames[k],
                                                                   args[k]);
                    res &= (Inst != null);
                } catch (com.sun.star.uno.Exception ex) {
                    log.println(
                            "Exception occured during createInstanceWithArguments()");
                    ex.printStackTrace(log);
                    res = false;
                }
            }

            tRes.tested("createInstanceWithArguments()", res);
        }
    }
} // finish class _XMultiServiceFactory
