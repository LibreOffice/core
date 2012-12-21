/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package ifc.ucb;

import lib.MultiMethodTest;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XRemoteContentProviderAcceptor;
import com.sun.star.ucb.XRemoteContentProviderDoneListener;

/**
 * Testing <code>com.sun.star.ucb.XRemoteContentProviderAcceptor</code>
 * interface methods :
 * <ul>
 *  <li><code> addRemoteContentProvider()</code></li>
 *  <li><code> removeRemoteContentProvider()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.ucb.XRemoteContentProviderAcceptor
 */
public class _XRemoteContentProviderAcceptor extends MultiMethodTest {

    public XRemoteContentProviderAcceptor oObj = null;

    /**
     * The simple <code>XRemoteContentProviderDoneListener</code>
     * implementation.
     */
    public class DoneListener implements XRemoteContentProviderDoneListener {

        public void doneWithRemoteContentProviders
                (XRemoteContentProviderAcceptor xRCPA) {
        }
        public void disposing (com.sun.star.lang.EventObject obj) {}

    }

    XRemoteContentProviderDoneListener aDoneListener = new DoneListener();

    /**
     * Adds a remote provider. <p>
     * Has <b> OK </b> status if the method returns <code>true</code>.
     */
    public void _addRemoteContentProvider() {
        boolean res = false;

        String[] template = new String[]{"file"};
        res = oObj.addRemoteContentProvider("ContentID",(XMultiServiceFactory)tParam.getMSF(),
            template,aDoneListener);

        tRes.tested("addRemoteContentProvider()",res);
    }

    /**
     * Removes the remote provider added before. <p>
     * Has <b> OK </b> status if the method returns <code>true</code>.
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> addRemoteContentProvider() </code> : rovider must
     *  be added first </li>
     * </ul>
     */
    public void _removeRemoteContentProvider() {
        requiredMethod("addRemoteContentProvider()") ;

        boolean res = false;

        res = oObj.removeRemoteContentProvider("ContentID");
        tRes.tested("removeRemoteContentProvider()",res);
    }


}

