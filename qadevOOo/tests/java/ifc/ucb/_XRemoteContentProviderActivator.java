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
 * Test is <b> NOT </b> multithread compliant. <p>
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
        xRCPA.addRemoteContentProvider("ContentID",tParam.getMSF(),template,null);
        XContentProviderManager CPM = oObj.activateRemoteContentProviders();
        res = (CPM != null);
        xRCPA.removeRemoteContentProvider("ContentID");

        tRes.tested("activateRemoteContentProviders()",res);
    }

}

