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
package com.sun.star.comp.helper;

import java.util.HashMap;
import java.util.logging.Level;
import java.util.logging.Logger;

import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import org.junit.Test;

public class ComponentContext_Test {

    private static final Logger logger = Logger.getLogger(ComponentContext_Test.class.getName());

    @Test public void test() throws Exception {
        logger.log(Level.INFO, "Testing ComponentContext");
        HashMap<String, Object> table = new HashMap<String, Object>();
        table.put("bla1", new ComponentContextEntry(null, Integer.valueOf(1)));
        XComponentContext xInitialContext = Bootstrap.createInitialComponentContext(table);

        table = new HashMap<String, Object>();
        table.put("bla2", new ComponentContextEntry(Integer.valueOf(2)));
        table.put("bla3", Integer.valueOf(3));
        XComponentContext xContext = new ComponentContext(table, xInitialContext);

        XMultiComponentFactory xSMgr = xContext.getServiceManager();

        assertNotNull(xSMgr.createInstanceWithContext("com.sun.star.loader.Java", xContext));
        assertNotNull(xSMgr.createInstanceWithContext("com.sun.star.bridge.BridgeFactory", xContext));
        assertNotNull(xSMgr.createInstanceWithContext("com.sun.star.bridge.UnoUrlResolver", xContext));
        assertNotNull(xSMgr.createInstanceWithContext("com.sun.star.connection.Connector", xContext));
        assertNotNull(xSMgr.createInstanceWithContext("com.sun.star.connection.Acceptor", xContext));
//        assertNotNull(xSMgr.createInstanceWithContext("com.sun.star.lang.ServiceManager", xContext));

        assertNotNull(xContext.getValueByName("bla1"));
        assertNotNull(xContext.getValueByName("bla2"));
        assertNotNull(xContext.getValueByName("bla3"));
        assertNotNull(xInitialContext.getValueByName("bla2"));
        assertNotNull(xInitialContext.getValueByName("bla3"));

        assertEquals(((Integer) xContext.getValueByName("bla1")).intValue(), 1);
        assertEquals(((Integer) xContext.getValueByName("bla2")).intValue(), 2);
        assertEquals(((Integer) xContext.getValueByName("bla3")).intValue(), 3);
        assertEquals(((Integer) xInitialContext.getValueByName("bla1")).intValue(), 1);

        XComponent xComp = UnoRuntime.queryInterface(
                XComponent.class, xInitialContext);
        xComp.dispose();
    }
}