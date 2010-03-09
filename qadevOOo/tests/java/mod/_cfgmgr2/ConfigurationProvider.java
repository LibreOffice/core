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

import java.io.PrintWriter;
import java.util.Vector;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.beans.PropertyState;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;


public class ConfigurationProvider extends TestCase {

    /** Called to create an instance of <code>TestEnvironment</code> with an
     * object to test and related objects.
     *
     * For this an instance of com.sun.star.comp.configuration.ConfigurationProvider
     * is created at the MultiServiceFactory of the Office
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
            oObj = (XInterface) ((XMultiServiceFactory)tParam.getMSF())
                                      .createInstance("com.sun.star.comp.configuration.ConfigurationProvider");
        } catch (com.sun.star.uno.Exception e) {
        }

        util.dbg.printInterfaces(oObj);

        PropertyValue[] nodeArgs = new PropertyValue[1];
        PropertyValue nodepath = new PropertyValue();
        nodepath.Name = "nodepath";
        nodepath.Value = "org.openoffice.Setup";
        nodepath.Handle = -1;
        nodepath.State = PropertyState.DEFAULT_VALUE;
        nodeArgs[0] = nodepath;

        Vector args = new Vector();

        args.add(0, nodeArgs);
        args.add(0, nodeArgs);
        args.add(0, nodeArgs);
        args.add(0, nodeArgs);
        args.add(0, nodeArgs);
        args.add(0, nodeArgs);
        args.add(0, nodeArgs);

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("XMSF.Args",
                            args.toArray(new Object[args.size()][]));
        tEnv.addObjRelation("needArgs", "ConfigurationProvider");

        return tEnv;
    }
}