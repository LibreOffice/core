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

package mod._cfgmgr2;

import com.sun.star.beans.NamedValue;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

public class LocalHierarchyBrowser extends TestCase {


    /** Called to create an instance of <code>TestEnvironment</code> with an
     * object to test and related objects. Subclasses should implement this
     * method to provide the implementation and related objects. The method is
     * called from <code>getTestEnvironment()</code>.
     *
     * @param tParam test parameters
     * @param log writer to log information while testing
     *
     * @see TestEnvironment
     * @see #getTestEnvironment()
     *
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                    PrintWriter log) {
        XInterface oObj = null;

        try {
            oObj = (XInterface) ((XMultiServiceFactory) tParam.getMSF()).createInstance(
                           "com.sun.star.comp.configuration.backend.LocalHierarchyBrowser");
        } catch (com.sun.star.uno.Exception e) {
        }

        String filename = util.utils.getOfficeURL(
                                  (XMultiServiceFactory) tParam.getMSF()) +
                          "/../share/registry/data/org/openoffice";

        String filename2 = util.utils.getOfficeURL(
                                  (XMultiServiceFactory) tParam.getMSF()) +
                          "/../share/registry/schema/org/openoffice/";

        Object[] vXJobArgs = new Object[2];
        NamedValue[] vXJobArg0 = new NamedValue[1];
        vXJobArg0[0] = new NamedValue("LayerDataUrl", filename);
        NamedValue[] vXJobArg1 = new NamedValue[1];
        vXJobArg1[0] = new NamedValue("SchemaDataUrl", filename2);
        vXJobArgs[0] = vXJobArg0;
        vXJobArgs[1] = vXJobArg1;


        log.println("Implementation name: " + util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);
        tEnv.addObjRelation("XJobArgs", vXJobArgs);

        return tEnv;
    }

}
