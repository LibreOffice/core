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
import com.sun.star.ucb.XContentProviderManager;
import com.sun.star.ucb.XRemoteContentProviderAcceptor;
import com.sun.star.ucb.XRemoteContentProviderActivator;

/**
 * Testing <code>com.sun.star.ucb.XRemoteContentProviderActivator</code>
 * interface methods :
 * <ul>
 *  <li><code> activateRemoteContentProviders()</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'RCPA'</code>
 *   (of type <code>XRemoteContentProviderAcceptor</code>):
 *   this acceptor is used to add a provider first before
 *   its activation. </li>
 * <ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.ucb.XRemoteContentProviderActivator
 */
public class _XRemoteContentProviderActivator extends MultiMethodTest {

    public XRemoteContentProviderActivator oObj = null;

    /**
     * First the relation is retrieved and a remote provider is
     * added to the acceptor. Then it is activated and
     * removed. <p>
     * Has <b> OK </b> status if <code>activateRemoteContentProviders</code>
     * method returns not <code>null</code> value. <p>
     */
    public void _activateRemoteContentProviders() {
        boolean res = false;

        XRemoteContentProviderAcceptor xRCPA = (XRemoteContentProviderAcceptor)
                                                    tEnv.getObjRelation("RCPA");
        String[] template = new String[]{"file"};
        xRCPA.addRemoteContentProvider("ContentID",(XMultiServiceFactory)tParam.getMSF(),template,null);
        XContentProviderManager CPM = oObj.activateRemoteContentProviders();
        res = (CPM != null);
        xRCPA.removeRemoteContentProvider("ContentID");

        tRes.tested("activateRemoteContentProviders()",res);
    }

}

