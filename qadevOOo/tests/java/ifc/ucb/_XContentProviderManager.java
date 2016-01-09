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

package ifc.ucb;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.ContentProviderInfo;
import com.sun.star.ucb.DuplicateProviderException;
import com.sun.star.ucb.XContentProvider;
import com.sun.star.ucb.XContentProviderManager;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;

/**
* Tests XContentProviderManager. The test registers two ContentProviders, calls
* queryXXX methods to verify results, and deregisters them.
*
* Testing <code>com.sun.star.ucb.XContentProviderManager</code>
* interface methods :
* <ul>
*  <li><code> registerContentProvider()</code></li>
*  <li><code> deregisterContentProvider()</code></li>
*  <li><code> queryContentProviders()</code></li>
*  <li><code> queryContentProvider()</code></li>
* </ul> <p>
* The test registers two ContentProviders, calls
* queryXXX methods to verify results, and deregisters them. <p>
*
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.ucb.XContentProviderManager
*/
public class _XContentProviderManager extends MultiMethodTest {
    /**
     * Contains the tested object.
     */
    public XContentProviderManager oObj;

    /**
     * The test scheme name.
     */
    static final String myScheme = "test-scheme";

    /**
     * Any preexisting content provider. If it exists it will be hidden by
     * <code>firstContentProvider</code>, registered with the same
     * <code>myScheme</code>. Typically there is no preexisting content
     * provider, unless the catch-all providers GnomeVFSContentProvider or
     * GIOContentProvider is installed
     */
    XContentProvider preexistingContentProvider;

    /**
     * First content provider. It will be hidden by <code>contentProvider
     * </code>, registered with the same <code>myScheme</code> to test
     * the "hiding" behaviour.
     */
    XContentProvider firstContentProvider;

    /**
     * The main content provider.
     */
    XContentProvider contentProvider;

    /**
     * <code>ContentProvider</code>s information which are in the manager
     * before registering the testing providers.
     */
    ContentProviderInfo[] initialProvidersInfo;

    /**
     * Creates two testing providers.
     *
     * @see #firstContentProvider
     * @see #contentProvider
     */
    @Override
    public void before() {
        XMultiServiceFactory xMSF = tParam.getMSF();

        log.println("creating testing content providers");
        try {
            firstContentProvider = UnoRuntime.queryInterface(
                    XContentProvider.class, xMSF.createInstance(
                            "com.sun.star.ucb.FileContentProvider"));

            contentProvider = UnoRuntime.queryInterface(
                    XContentProvider.class, xMSF.createInstance(
                            "com.sun.star.ucb.FileContentProvider"));
        } catch (Exception e) {
            log.println("Can't create content providers " + e.getMessage());
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }
    }

    /**
     * At the beginning call <code>queryContentProviders</code> method
     *
     * to have info about providers existing before new adding.
     * It adds two testing contents providers, both for the same scheme.
     * The second one is added two times: first, in non-replacing mode, to test
     * <code>DuplicateProviderException</code>, and second, in replacing mode,
     * to hide the first provider. <p>
     *
     * The evaluation of results are performed later, in
     * <code>queryContentProvider()</code>.
     *
     * Has <b> OK </b> status if in the first provider is registered
     * without exceptions, the second throws
     * <code>DuplicateProviderException</code> in non-replacing mode,
     * and no exceptions in replacing mode. <p>
     *
     * @see #_queryContentProvider
     */
    public void _registerContentProvider() {
        // querying providers info before inserting them, to verify results
        initialProvidersInfo = oObj.queryContentProviders();

        // GnomeVFSContentProvider or GIOContentProvider ?, typically null
        preexistingContentProvider = oObj.queryContentProvider(myScheme);

        log.println("registering the first provider");
        try {
            oObj.registerContentProvider(firstContentProvider, myScheme,false);
        } catch (DuplicateProviderException e) {
            log.println("Unexpected exception thrown " + e.getMessage());
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception ", e);
        }

        log.println("registering the second provider in non-replacing mode");
        try {
            oObj.registerContentProvider(contentProvider, myScheme, false);
            Status.failed("registerContentProvider(.., .., false)");
        } catch (DuplicateProviderException e) {
            log.println("DuplicateProviderException thrown - OK");
        }

        XContentProvider result;

        log.println("registering the second provider in the replace mode");
        try {
            result = oObj.registerContentProvider(contentProvider, myScheme, true);
        } catch (DuplicateProviderException e) {
            log.println("Unexpected exception thrown " + e.getMessage());
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception ", e);
        }

        // check the result is the first provider
        tRes.tested("registerContentProvider()",
            result.equals(firstContentProvider));
    }

    /**
     * It calls the method (after registering providers) and compares
     * its result with the result  before registering.
     *
     * Has <b> OK </b> status if the number of providers increases
     * by one after registering custom provider.
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> registerContentProvider() </code> : to compare number
     *  of providers. </li>
     * </ul>
     * @see #_registerContentProvider
     */
    public void _queryContentProviders() {
        executeMethod("registerContentProvider()");

        ContentProviderInfo[] providersInfo = oObj.queryContentProviders();
        // verifying that the number of providers increased by 1
        tRes.tested("queryContentProviders()",
                providersInfo.length == initialProvidersInfo.length + 1);
    }

    /**
     * It queries for added custom provider using its scheme
     * and verifies its result with
     * <code>queryContentProviders()</code> result and with
     * custom provider created in <code>registerContentProvider()</code>.
     * Also verifies <code>registerContentProvider()</code>. <p>
     *
     * Has <b>OK</b> status if the provider returned is found within
     * all providers and is equal to provider created before.
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> registerContentProvider() </code> </li>
     * </ul>
     */
    public void _queryContentProvider() {
        executeMethod("registerContentProvider()");

        XContentProvider result = oObj.queryContentProvider
            ("http://www.sun.com");

        log.println("finding queryContentProvider() result");
        boolean found = false;

        ContentProviderInfo[] providersInfo = oObj.queryContentProviders();

        for (int i = 0; i < providersInfo.length; i++) {
            if (result.equals(providersInfo[i].ContentProvider)) {
                found = true;
                break;
            }
        }

        tRes.tested("queryContentProvider()", found);
    }

    /**
     * At first one provider is deregistered, after that provider
     * is queried, the second provider must be returned for the
     * specified scheme. Then the second provider is deregistered.
     * Now <code>null</code> value must be retruned by the method
     * <code>queryContentProvider</code> on the specified scheme. <p>
     *
     * Has <b>OK</b> status if in the first case the second provider
     * remains registered, and after its removing no providers remain
     * registered for the scheme specified.
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> registerContentProvider() </code> : two providers
     *  must be registered. </li>
     * </ul>
     * The following method tests are to be executed before :
     * <ul>
     *  <li> <code> queryContentProvider() </code> : to run this test
     *   finally. </li>
     *  <li> <code> queryContentProviders() </code> : to run this test
     *   finally.  </li>
     * </ul>
     */
    public void _deregisterContentProvider() {
        executeMethod("queryContentProvider()");
        executeMethod("queryContentProviders()");
        requiredMethod("registerContentProvider()");

        log.println("deregistering the second provider");
        oObj.deregisterContentProvider(contentProvider, myScheme);

        XContentProvider res = oObj.queryContentProvider(myScheme);

        log.println("deregistering the first provider");
        oObj.deregisterContentProvider(firstContentProvider, myScheme);

        res = oObj.queryContentProvider(myScheme);

	// verifying that the original provider (typically none) is returned.
        tRes.tested("deregisterContentProvider()", res == preexistingContentProvider);
    }
}
