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
package ifc.container;

import com.sun.star.container.XHierarchicalName;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;


public class _XHierarchicalName extends MultiMethodTest {
    public XHierarchicalName oObj;

    public void _composeHierarchicalName() {
        String expName = (String) tEnv.getObjRelation("HierachicalName");
        String element = (String) tEnv.getObjRelation("ElementName");
        boolean res = false;

        try {
            String hn = oObj.composeHierarchicalName(element);
            res = hn.startsWith(expName);

            if (!res) {
                log.println("Getting : " + hn +
                            " but expected it to start with " + expName);
            }
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception " + e.getMessage());
        } catch (com.sun.star.lang.NoSupportException e) {
            log.println("Exception " + e.getMessage());
        }

        tRes.tested("composeHierarchicalName()", res);
    }

    public void _getHierarchicalName() {
        String hName = oObj.getHierarchicalName();
        String expName = (String) tEnv.getObjRelation("HierachicalName");
        boolean res = true;
System.out.println("### "+hName);
        if (expName != null) {
            res = hName.startsWith(expName);

            if (!res) {
                log.println("Expected the name to start with " + expName);
                log.println("got " + hName);
            }
        } else {
            throw new StatusException(Status.failed(
                                              "ObjectRelation 'HierachicalName' missing"));
        }

        tRes.tested("getHierarchicalName()", res);
    }
}