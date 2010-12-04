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
                log.println("Exception occurred during createInstance()");
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
                            "Exception occurred during createInstanceWithArguments()");
                    ex.printStackTrace(log);
                    res = false;
                }
            }

            tRes.tested("createInstanceWithArguments()", res);
        }
    }
} // finish class _XMultiServiceFactory
