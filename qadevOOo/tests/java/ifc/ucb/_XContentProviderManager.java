/*************************************************************************
 *
 *  $RCSfile: _XContentProviderManager.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 11:24:22 $
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
* Test is <b> NOT </b> multithread compilant. <p>
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
     * First content provider. It will be hidden by <code>contentProvider
     * </code>, registred with the same <code>myScheme</code> to test
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
    public void before() {
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();

        log.println("creating testing content providers");
        try {
            firstContentProvider = (XContentProvider)UnoRuntime.queryInterface(
                    XContentProvider.class, xMSF.createInstance(
                            "com.sun.star.ucb.FileContentProvider"));

            contentProvider = (XContentProvider)UnoRuntime.queryInterface(
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
        // querying providfers info before inserting them, to verify results
        initialProvidersInfo = oObj.queryContentProviders();

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
            if (result.equals(providersInfo[i].ContentProvider)
                    /*&& myScheme.equals(providersInfo[i].Scheme)*/) {
                found = true;
                break;
            }
        }

//        boolean gotTheRightOne = util.ValueComparer.equalValue
//            (result,contentProvider);

        tRes.tested("queryContentProvider()", found);

//        gotTheRightOne = result.equals(contentProvider);

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

        // verifying that no provider is returned
        tRes.tested("deregisterContentProvider()", res == null);
    }
}
