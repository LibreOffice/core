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
package ifc.registry;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;
import util.RegistryTools;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.registry.InvalidRegistryException;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.registry.XSimpleRegistry;


/**
* Testing <code>com.sun.star.registry.XSimpleRegistry</code>
* interface methods :
* <ul>
*  <li><code> getURL()</code></li>
*  <li><code> open()</code></li>
*  <li><code> isValid()</code></li>
*  <li><code> close()</code></li>
*  <li><code> destroy()</code></li>
*  <li><code> getRootKey()</code></li>
*  <li><code> isReadOnly()</code></li>
*  <li><code> mergeKey()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'NR'</code> <b>optional</b> (of type <code>String</code>):
*   if this object relation isn't null than the testing component
*   doesn't support some methods of the interface
*   (<code>open(), close(), destroy()</code>)</li>
*  <li> <code>'XSimpleRegistry.open'</code> (of type <code>String</code>):
*    The full system path to the registry file which is opened and modified.
*  </li>
*  <li> <code>'XSimpleRegistry.destroy'</code> (of type <code>String</code>):
*    The full system path to the registry fiel which is destroyed.
*  </li>
*  <li> <code>'XSimpleRegistry.merge'</code> (of type <code>String</code>):
*    The full system path to the registry file which is merged with the
*    registry tested.
*  </li>
* </ul> <p>
* @see com.sun.star.registry.XSimpleRegistry
*/
public class _XSimpleRegistry extends MultiMethodTest {
    public XSimpleRegistry oObj = null;
    protected String nr = null;
    protected boolean configuration = false;
    protected String openF = null;
    protected String destroyF = null;
    protected String mergeF = null;

    /**
    * Retrieves object relations.
    * @throws StatusException If one of required relations not found.
    */
    protected void before() {
        if (tEnv.getObjRelation("configuration") != null) {
            configuration = true;
        }

        nr = (String) tEnv.getObjRelation("NR");

        openF = (String) tEnv.getObjRelation("XSimpleRegistry.open");

        if (openF == null) {
            throw new StatusException(Status.failed(
                                              "Relation 'XSimpleRegistry.open' not found"));
        }

        destroyF = (String) tEnv.getObjRelation("XSimpleRegistry.destroy");

        if (destroyF == null) {
            throw new StatusException(Status.failed(
                                              "Relation 'XSimpleRegistry.destroy' not found"));
        }

        mergeF = (String) tEnv.getObjRelation("XSimpleRegistry.merge");

        if (mergeF == null) {
            throw new StatusException(Status.failed(
                                              "Relation 'XSimpleRegistry.merge' not found"));
        }
    }

    /**
    * If the method is supported opens the registry key with the URL
    * from <code>'XSimpleRegistry.open'</code> relation, then closes it. <p>
    *
    * Has <b> OK </b> status if the method isn't supported by the component
    * (the object relation <code>'NR'</code> isn't null) or no exceptions were
    * thrown during open/close operations. <p>
    */
    public void _open() {
        if (nr != null) {
            log.println("'open()' isn't supported by '" + nr + "'");
            tRes.tested("open()", true);

            return;
        }

        log.println("Trying to open registry :" + openF);

        try {
            oObj.open(openF, false, true);
            oObj.close();
        } catch (InvalidRegistryException e) {
            e.printStackTrace(log);
            tRes.tested("open()", false);

            return;
        }

        tRes.tested("open()", true);
    }

    /**
    * Test opens the registry key with the URL from
    * <code>'XSimpleRegistry.open'</code> relation not only for read,
    * calls the method, checks returned value and closes the registry. <p>
    *
    * Has <b> OK </b> status if returned value is false and no exceptions were
    * thrown. <p>
    */
    public void _isReadOnly() {
        boolean result = false;

        try {
            openReg(oObj, openF, false, true);
            result = !oObj.isReadOnly();
            closeReg(oObj);
        } catch (InvalidRegistryException e) {
            e.printStackTrace(log);
            result = false;
        }

        tRes.tested("isReadOnly()", result);
    }

    /**
    * Test opens the registry key with the URL from
    * <code>'XSimpleRegistry.open'</code> relation, calls the method,
    * checks returned value and closes the registry key. <p>
    *
    * Has <b>OK</b> status if returned value isn't null and no exceptions were
    * thrown. <p>
    */
    public void _getRootKey() {
        boolean result = false;

        try {
            openReg(oObj, openF, false, true);

            XRegistryKey rootKey = oObj.getRootKey();
            result = rootKey != null;
            closeReg(oObj);
        } catch (InvalidRegistryException e) {
            e.printStackTrace(log);
            result = false;
        }

        tRes.tested("getRootKey()", result);
    }

    /**
    * Merges the current registry with the registry from URL got from
    * <code>'XSimpleRegistry.merge'</code> relation under 'MergeKey' key.
    * Then the keys of these two registries retrieved :
    * <ul>
    *  <li> Root key from 'XSimpleRegistry.merge' registry </li>
    *  <li> 'MergeKey' key from the current registry </li>
    * </ul>
    * Then these two keys are recursively compared. <p>
    *
    * Has <b> OK </b> status if the method isn't supported by the component
    * (the object relation <code>'NR'</code> isn't null)
    * or
    * if it's supported and after successfull merging the keys mentioned
    * above are recursively equal. <p>
    */
    public void _mergeKey() {
        if (configuration) {
            log.println(
                    "You can't merge into this registry. It's just a wrapper for a configuration node, which has a fixed structure which can not be modified");
            tRes.tested("mergeKey()", true);

            return;
        }

        if (nr != null) {
            log.println("'mergeKey()' isn't supported by '" + nr + "'");
            tRes.tested("mergeKey()", true);

            return;
        }

        openReg(oObj, openF, false, true);

        try {
            RegistryTools.printRegistryInfo(oObj.getRootKey(), log);
            oObj.mergeKey("MergeKey", mergeF);
            RegistryTools.printRegistryInfo(oObj.getRootKey(), log);
        } catch (com.sun.star.registry.MergeConflictException e) {
            e.printStackTrace(log);
            tRes.tested("mergeKey()", false);

            return;
        } catch (com.sun.star.registry.InvalidRegistryException e) {
            e.printStackTrace(log);
            tRes.tested("mergeKey()", false);

            return;
        }

        boolean isEqual = false;
        XSimpleRegistry reg = null;

        try {
            reg = RegistryTools.createRegistryService((XMultiServiceFactory) tParam.getMSF());
        } catch (com.sun.star.uno.Exception e) {
            log.print("Can't create registry service: ");
            e.printStackTrace(log);
            tRes.tested("mergeKey()", false);

            return;
        }

        openReg(reg, mergeF, false, true);

        try {
            XRegistryKey key = oObj.getRootKey().openKey("MergeKey");
            XRegistryKey mergeKey = reg.getRootKey();
            isEqual = RegistryTools.compareKeyTrees(key, mergeKey);
        } catch (com.sun.star.registry.InvalidRegistryException e) {
            log.print("Can't get root key: ");
            e.printStackTrace(log);
            tRes.tested("mergeKey()", false);

            return;
        }

        closeReg(reg);
        closeReg(oObj);

        tRes.tested("mergeKey()", isEqual);
    }

    /**
    * Test opens the registry key with the URL from
    * <code>'XSimpleRegistry.open'</code> relation, calls the method,
    * checks returned value and closes the registry key. <p>
    *
    * Has <b> OK </b> status if returned value isn't null and if length of the
    * returned string is greater than 0. <p>
    */
    public void _getURL() {
        openReg(oObj, openF, false, true);

        String url = oObj.getURL();
        closeReg(oObj);
        log.println("Getting URL: " + url+";");
        tRes.tested("getURL()", (url != null));
    }

    /**
    * Test checks value returned by the object relation <code>'NR'</code>,
    * opens the registry key with the URL from
    * <code>XSimpleRegistry.open'</code> relation, calls the method
    * and checks the validity of the registry key. <p>
    *
    * Has <b> OK </b> status if the registry key isn't valid after the method
    * call, or if the method isn't supported by the component (the object
    * relation <code>'NR'</code> isn't null). <p>
    */
    public void _close() {
        if (nr != null) {
            log.println("'close()' isn't supported by '" + nr + "'");
            tRes.tested("close()", true);

            return;
        }

        try {
            oObj.open(openF, false, true);
            oObj.close();
        } catch (com.sun.star.registry.InvalidRegistryException e) {
            e.printStackTrace(log);
            tRes.tested("close()", false);

            return;
        }

        tRes.tested("close()", !oObj.isValid());
    }

    /**
    * Test checks value returned by the object relation <code>'NR'</code>,
    * opens the registry key with the URL from
    * <code>'XSimpleRegistry.destroy'</code> relation, calls the method
    * and checks the validity of the registry key. <p>
    *
    * Has <b> OK </b> status if the registry key isn't valid after the method
    * call, or if the method isn't supported by the component (the object
    * relation <code>'NR'</code> isn't null). <p>
    */
    public void _destroy() {
        if (configuration) {
            log.println(
                    "This registry is a wrapper for a configuration access. It can not be destroyed.");
            tRes.tested("destroy()", true);

            return;
        }

        if (nr != null) {
            log.println("'destroy()' isn't supported by '" + nr + "'");
            tRes.tested("destroy()", true);

            return;
        }

        try {
            oObj.open(destroyF, false, true);
            oObj.destroy();
        } catch (com.sun.star.registry.InvalidRegistryException e) {
            e.printStackTrace(log);
            tRes.tested("destroy()", false);

            return;
        }

        tRes.tested("destroy()", !oObj.isValid());
    }

    /**
    * Test opens the registry key with the URL from
    * <code>'XSimpleRegistry.open'</code> relation, calls the method,
    * checks returned value and closes the registry key. <p>
    * Has <b> OK </b> status if returned value is true. <p>
    */
    public void _isValid() {
        boolean valid = true;

        openReg(oObj, openF, false, true);
        valid = oObj.isValid();
        closeReg(oObj);

        tRes.tested("isValid()", valid);
    }

    /**
    * Method calls <code>close()</code> of the interface
    * <code>com.sun.star.registry.XRegistryKey</code>. <p>
    * @param reg interface <code>com.sun.star.registry.XRegistryKey</code>
    * @param url specifies the complete URL to access the data source
    * @param arg1 specifies if the data source should be opened for read only
    * @param arg2 specifies if the data source should be created if it does not
    * already exist
    */
    public void openReg(XSimpleRegistry reg, String url, boolean arg1,
                        boolean arg2) {
        if (nr == null) {
            try {
                reg.open(url, arg1, arg2);
            } catch (com.sun.star.registry.InvalidRegistryException e) {
                log.print("Couldn't open registry:");
                e.printStackTrace(log);
            }
        }
    }

    /**
    * Method calls <code>close()</code> of the interface
    * <code>com.sun.star.registry.XRegistryKey</code>. <p>
    * @param reg <code>com.sun.star.registry.XRegistryKey</code>
    */
    public void closeReg(XSimpleRegistry reg) {
        if (nr == null) {
            try {
                reg.close();
            } catch (com.sun.star.registry.InvalidRegistryException e) {
                log.print("Couldn't close registry:");
                e.printStackTrace(log);
            }
        }
    }
}