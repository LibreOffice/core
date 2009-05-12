/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SvxUnoNumberingRules.java,v $
 * $Revision: 1.7.8.1 $
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
package mod._svx;

import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.DrawTools;


public class SvxUnoNumberingRules extends TestCase {
    static XComponent xDrawDoc;

    protected void initialize(TestParameters tParam, PrintWriter log) {
        log.println("creating a drawdoc");
        xDrawDoc = DrawTools.createDrawDoc(
                           (XMultiServiceFactory) tParam.getMSF());
    }

    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                    PrintWriter log) {
        XMultiServiceFactory docMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(
                                              XMultiServiceFactory.class,
                                              xDrawDoc);
        XInterface oObj = null;

        try {
            oObj = (XInterface) docMSF.createInstance(
                           "com.sun.star.text.NumberingRules");
        } catch (Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

        TestEnvironment tEnv = new TestEnvironment(oObj);

        log.println("Implementationname: " + util.utils.getImplName(oObj));

        Object NewRules=null;

        try{
        XIndexAccess xIA = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, oObj);
        NewRules = xIA.getByIndex(2);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Couldn't get new Rules for XIndexReplace");
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't get new Rules for XIndexReplace");
        }

        tEnv.addObjRelation("INSTANCE1",NewRules);

        return tEnv;
    }

    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xDrawDoc ");
        util.DesktopTools.closeDoc(xDrawDoc);
    }
}
