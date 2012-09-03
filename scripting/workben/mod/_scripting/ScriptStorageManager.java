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

package mod._scripting;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

public class ScriptStorageManager extends TestCase {

    public void initialize( TestParameters tParam, PrintWriter log ) {
    }

    public synchronized TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log ) throws StatusException {

        XInterface oObj = null;
    Object oInterface = null;

    try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory) Param.getMSF();
            XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(
                                                XPropertySet.class, xMSF);
            // get context
            XComponentContext xContext = (XComponentContext)
                            UnoRuntime.queryInterface(XComponentContext.class,
                            xProp.getPropertyValue("DefaultContext"));
            // get the script storage manager from context
            oInterface = xContext.getValueByName("/singletons/" +
                        "drafts.com.sun.star.script.framework.storage.theScriptStorageManager");
        }
        catch( Exception e ) {
            log.println("ScriptStorageManager singleton service not available" );
        e.printStackTrace();
        }
        oObj = (XInterface) oInterface;

        TestEnvironment tEnv = new TestEnvironment(oObj) ;
        TestDataLoader.setupData(tEnv, "ScriptStorageManager");

        return tEnv ;
    }

    public synchronized void disposeTestEnvironment( TestEnvironment tEnv,
            TestParameters tParam) {
    }
}


