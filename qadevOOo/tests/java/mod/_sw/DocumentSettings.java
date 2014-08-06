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
package mod._sw;

import com.sun.star.text.XTextDocument;
import com.sun.star.uno.XInterface;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.SOfficeFactory;
import util.WriterTools;

import java.io.PrintWriter;


public class DocumentSettings extends TestCase
{
    XTextDocument xTextDoc = null;

    /**
    * Creates text document.
    */
    @Override
    protected void initialize(TestParameters tParam, PrintWriter log)
    {
        log.println("    opening xTextDoc ");
        xTextDoc =
            WriterTools.createTextDoc(tParam.getMSF());
    }

    /**
    * Disposes text document.
    */
    @Override
    protected void cleanup(TestParameters tParam, PrintWriter log)
    {
        log.println("    disposing xTextDoc ");
        util.DesktopTools.closeDoc(xTextDoc);
    }

    @Override
    protected TestEnvironment createTestEnvironment(
        TestParameters tParam, PrintWriter log
    )
    {
        SOfficeFactory SOF =
            SOfficeFactory.getFactory(tParam.getMSF());
        XInterface oObj = null;

        log.println("creating a test environment");

        oObj =
            (XInterface) SOF.createInstance(
                xTextDoc, "com.sun.star.text.DocumentSettings"
            );

        TestEnvironment tEnv = new TestEnvironment(oObj);

        return tEnv;
    }
}
