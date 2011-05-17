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

package ifc.lang;


import lib.MultiMethodTest;

import com.sun.star.lang.XMain;


/**
 * Testing <code>com.sun.star.lang.XMain</code>
 * interface methods :
 * <ul>
 *  <li><code> run()</code></li>
 * </ul> <p>
 *
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'ARGS'</code> (of type <code>String[]</code>):
 *   used as argument for <code>run</code> method. </li>
 * <ul> <p>
 *
 * @see com.sun.star.lang.XMain
 */
public class _XMain extends MultiMethodTest{

    public static XMain oObj = null;

    /**
     * Just calls the method. <p>
     *
     * Has <b> OK </b> status if no runtime exceptions occurred.
     */
    public void _run() {
        String[] args = (String[])tEnv.getObjRelation("ARGS");

        log.println("Running with arguments:");
        for (int i=0; i< args.length; i++)
            log.println("#" + i + ": " + args[i]);

        oObj.run(args);

        tRes.tested("run()", true);
    }

}
