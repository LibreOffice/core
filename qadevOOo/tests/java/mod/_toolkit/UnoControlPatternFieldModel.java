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
package mod._toolkit;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.utils;


public class UnoControlPatternFieldModel extends TestCase {
    public synchronized TestEnvironment createTestEnvironment(TestParameters Param,
                                                              PrintWriter log) {
        XInterface oObj = null;

        try {
            oObj = (XInterface) ((XMultiServiceFactory) Param.getMSF()).createInstance(
                           "com.sun.star.awt.UnoControlPatternFieldModel");
        } catch (Exception e) {
        }

        log.println(
                "creating a new environment for UnoControlPatternFieldModel object");

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("OBJNAME", "stardiv.vcl.controlmodel.PatternField");
        System.out.println("ImplementationName: " + utils.getImplName(oObj));

        return tEnv;
    } // finish method getTestEnvironment
} // finish class UnoControlPatternFieldModel
