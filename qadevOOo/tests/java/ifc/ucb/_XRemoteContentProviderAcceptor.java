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

    };

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

